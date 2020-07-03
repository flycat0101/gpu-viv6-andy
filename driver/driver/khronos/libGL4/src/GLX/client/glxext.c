/* $XFree86: xc/lib/GL/glx/glxext.c,v 1.16 2003/01/20 21:37:19 tsi Exp $ */

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

/*                                                            <
 * Direct rendering support added by Precision Insight, Inc.  <
 *                                                            <
 * Authors:                                                   <
 *   Kevin E. Martin <kevin@precisioninsight.com>             <
 *                                                            <
 */

#include "packrender.h"
#include <stdio.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/extutil.h>
#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include "xf86dri.h"
#include "sarea.h"
#include "dri_util.h"
#include "glcore/glos.h"
#include "glcore/gc_es_dispatch.h"
#if defined(PTHREADS)
#include <pthread.h> /* POSIX threads headers */
#include <errno.h>
#endif
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xdamage.h>
#ifndef MAP_FAILED
#define MAP_FAILED ((GLvoid *)-1)
#endif

#ifdef DEBUG
GLvoid __glXDumpDrawBuffer(__GLXcontext *ctx);
#endif


#if USE_DRM_MAP
#else
struct fb_fix_screeninfo {
    char id[16];                /* identification string eg "TT Builtin" */
    char *smem_start;           /* Start of frame buffer mem */
                                /* (physical address) */
    unsigned int smem_len;      /* Length of frame buffer mem */
    unsigned int type;          /* see FB_TYPE_*        */
    unsigned int type_aux;      /* Interleave for interleaved Planes */
    unsigned int visual;        /* see FB_VISUAL_*      */
    unsigned short xpanstep;    /* zero if no hardware panning  */
    unsigned short ypanstep;    /* zero if no hardware panning  */
    unsigned short ywrapstep;   /* zero if no hardware ywrap    */
    unsigned int line_length;   /* length of a line in bytes    */
    char *mmio_start;           /* Start of Memory Mapped I/O   */
                                /* (physical address) */
    unsigned int mmio_len;      /* Length of Memory Mapped I/O  */
    unsigned int accel;         /* Type of acceleration available */
    unsigned short reserved[3]; /* Reserved for future compatibility */
};

struct fb_bitfield {
    unsigned int offset;            /* beginning of bitfield    */
    unsigned int length;            /* length of bitfield       */
    unsigned int msb_right;         /* != 0 : Most significant bit is */
                                    /* right */
};

struct fb_var_screeninfo {
    unsigned int  xres;         /* visible resolution       */
    unsigned int  yres;
    unsigned int  xres_virtual;     /* virtual resolution       */
    unsigned int  yres_virtual;
    unsigned int  xoffset;          /* offset from virtual to visible */
    unsigned int  yoffset;          /* resolution           */

    unsigned int  bits_per_pixel;       /* guess what           */
    unsigned int  grayscale;        /* != 0 Graylevels instead of colors */

    struct fb_bitfield red;     /* bitfield in fb mem if true color, */
    struct fb_bitfield green;   /* else only length is significant */
    struct fb_bitfield blue;
    struct fb_bitfield transp;  /* transparency         */

    unsigned int  nonstd;           /* != 0 Non standard pixel format */

    unsigned int  activate;         /* see FB_ACTIVATE_*        */

    unsigned int  height;           /* height of picture in mm    */
    unsigned int  width;            /* width of picture in mm     */

    unsigned int  accel_flags;      /* acceleration flags (hints)   */

    /* Timing: All values in pixclocks, except pixclock (of course) */
    unsigned int  pixclock;         /* pixel clock in ps (pico seconds) */
    unsigned int  left_margin;      /* time from sync to picture    */
    unsigned int  right_margin;     /* time from picture to sync    */
    unsigned int  upper_margin;     /* time from sync to picture    */
    unsigned int  lower_margin;
    unsigned int  hsync_len;        /* length of horizontal sync    */
    unsigned int  vsync_len;        /* length of vertical sync  */
    unsigned int  sync;         /* see FB_SYNC_*        */
    unsigned int  vmode;            /* see FB_VMODE_*       */
    unsigned int  reserved[6];      /* Reserved for future compatibility */
};
#endif

extern GLvoid _glapi_set_dispatch(GLvoid *dispatch);
extern GLvoid _glapi_check_multithread(GLvoid);

/************************************************************************/
/*
** We setup some dummy structures here so that the API can be used
** even if no context is current.
*/

static GLubyte dummyBuffer[__GLX_BUFFER_LIMIT_SIZE];

/*
** Dummy context used by small commands when there is no current context.
** All the
** gl and glx entry points are designed to operate as nop's when using
** the dummy context structure.
*/
static __GLXcontext dummyContext = {
    &dummyBuffer[0],
    &dummyBuffer[0],
    &dummyBuffer[0],
    &dummyBuffer[__GLX_BUFFER_LIMIT_SIZE],
    sizeof(dummyBuffer),
};

#if defined( PTHREADS )

pthread_mutex_t __glXmutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_once_t once_control = PTHREAD_ONCE_INIT;

/**
 * Per-thread data key.
 *
 * Once \c init_thread_data has been called, the per-thread data key will
 * take a value of \c NULL.  As each new thread is created the default
 * value, in that thread, will be \c NULL.
 */
static pthread_key_t _glXContextTSD;

/**
 * Initialize the per-thread data key.
 *
 * This function is called \b exactly once per-process (not per-thread!) to
 * initialize the per-thread data key.  This is ideally done using the
 * \c pthread_once mechanism.
 */
static GLvoid init_thread_data( GLvoid )
{
    if ( pthread_key_create( & _glXContextTSD, NULL ) != 0 ) {
        perror( "pthread_key_create" );
        exit( -1 );
    }
}

GLvoid __glXSetCurrentContext( __GLXcontext * c )
{
    pthread_once( & once_control, init_thread_data );
    pthread_setspecific( _glXContextTSD, c );
}

__GLXcontext * __glXGetCurrentContext( GLvoid )
{
    GLvoid * v;

    pthread_once( & once_control, init_thread_data );
    v = pthread_getspecific( _glXContextTSD );
    return (v == NULL) ? & dummyContext : (__GLXcontext *) v;
}

#else

/* not thread safe */
__GLXcontext *__glXcurrentContext = &dummyContext;

#endif


/*
** Point to glcore's global __glNopDispatchFuncTable.dispatch table.
*/
__GLdispatchTable *__glNopDispatchTab = NULL;
__GLcontext *__glxNopContext = NULL;

/*
** You can set this cell to 1 to force the gl drawing stuff to be
** one command per packet
*/
int __glXDebug = 0;

/*
** forward prototype declarations
*/
int __glXCloseDisplay(Display *dpy, XExtCodes *codes);

/************************************************************************/

/* Global variable that save a pointer to GLX extension private data */
XExtData *__glXExtensionPrivate = NULL;

/* Global variable that indicates if X display is closed */
Bool __glXDisplayIsClosed = GL_FALSE;


/* Extension required boiler plate */

static char *__glXExtensionName = GLX_EXTENSION_NAME;
XExtensionInfo *__glXExtensionInfo = NULL;

static /* const */ char *error_list[] = {
    "GLXBadContext",
    "GLXBadContextState",
    "GLXBadDrawable",
    "GLXBadPixmap",
    "GLXBadContextTag",
    "GLXBadCurrentWindow",
    "GLXBadRenderRequest",
    "GLXBadLargeRequest",
    "GLXUnsupportedPrivateRequest",
};

int __glXCloseDisplay(Display *dpy, XExtCodes *codes)
{
    __GLXdisplayPrivate *priv;
    GLXContext gc;

    gc = __glXGetCurrentContext();
    if (dpy == gc->currentDpy) {
        __glXMutexLock();
        __glXSetCurrentContext(&dummyContext);
        _glapi_set_context((GLvoid *)__glxNopContext);
        priv = __glXInitialize(dpy);
        __glXFreeContext(priv, gc);
        __glXMutexUnlock();
    }

    return XextRemoveDisplay(__glXExtensionInfo, dpy);
}


static XEXT_GENERATE_ERROR_STRING(__glXErrorString, __glXExtensionName,
                  __GLX_NUMBER_ERRORS, error_list)

static /* const */ XExtensionHooks __glXExtensionHooks = {
    NULL,                /* create_gc */
    NULL,                /* copy_gc */
    NULL,                /* flush_gc */
    NULL,                /* free_gc */
    NULL,                /* create_font */
    NULL,                /* free_font */
    __glXCloseDisplay,   /* close_display */
    NULL,                /* wire_to_event */
    NULL,                /* event_to_wire */
    NULL,                /* error */
    __glXErrorString,    /* error_string */
};

static
XEXT_GENERATE_FIND_DISPLAY(__glXFindDisplay, __glXExtensionInfo,
               __glXExtensionName, &__glXExtensionHooks,
               __GLX_NUMBER_EVENTS, NULL)

/************************************************************************/

/**
 * Convert an X visual type to a GLX visual type.
 *
 * \param visualType X visual type (i.e., \c TrueColor, \c StaticGray, etc.)
 *        to be converted.
 * \return If \c visualType is a valid X visual type, a GLX visual type will
 *         be returned.  Otherwise \c GLX_NONE will be returned.
 */
#define NUM_VISUAL_TYPES   6
GLint
__glConvertFromXvisualType( int visualType )
{
    static const int glx_visual_types[ NUM_VISUAL_TYPES ] = {
            GLX_STATIC_GRAY,  GLX_GRAY_SCALE,
            GLX_STATIC_COLOR, GLX_PSEUDO_COLOR,
            GLX_TRUE_COLOR,   GLX_DIRECT_COLOR
    };

    return ( (unsigned) visualType < NUM_VISUAL_TYPES )
        ? glx_visual_types[ visualType ] : GLX_NONE;
}


/**
 * Destroy a linked list of \c __GLcontextModes structures created by
 * \c __glContextModesCreate.
 *
 * \param modes  Linked list of structures to be destroyed.  All structres
 *               in the list will be freed.
 */
GLvoid
__glContextModesDestroy( __GLcontextModes * modes )
{
    while ( modes != NULL ) {
        __GLcontextModes * const next = modes->next;
        Xfree( modes );
        modes = next;
    }
}

/**
 * Allocate a linked list of \c __GLcontextModes structures.  The fields of
 * each structure will be initialized to "reasonable" default values.  In
 * most cases this is the default value defined by table 3.4 of the GLX
 * 1.3 specification.  This means that most values are either initialized to
 * zero or \c GLX_DONT_CARE (which is -1).  As support for additional
 * extensions is added, the new values will be initialized to appropriate
 * values from the extension specification.
 *
 * \param count         Number of structures to allocate.
 * \param minimum_size  Minimum size of a structure to allocate.  This allows
 *                      for differences in the version of the
 *                      \c __GLcontextModes stucture used in libGL and in a
 *                      DRI-based driver.
 * \returns A pointer to the first element in a linked list of \c count
 *          stuctures on success, or \c NULL on failure.
 *
 * \warning Use of \c minimum_size does \b not guarantee binary compatibility.
 *          The fundamental assumption is that if the \c minimum_size
 *          specified by the driver and the size of the \c __GLcontextModes
 *          structure in libGL is the same, then the meaning of each byte in
 *          the structure is the same in both places.  \b Be \b careful!
 *          Basically this means that fields have to be added in libGL and
 *          then propagated to drivers.  Drivers should \b never arbitrarilly
 *          extend the \c __GLcontextModes data-structure.
 */
__GLcontextModes *
__glContextModesCreate( unsigned count )
{
    const size_t size = sizeof( __GLcontextModes );
    __GLcontextModes * base = NULL;
    __GLcontextModes ** next;
    unsigned   i;

    next = & base;
    for ( i = 0 ; i < count ; i++ ) {
        *next = (__GLcontextModes *) Xmalloc( size );
        if ( *next == NULL ) {
            __glContextModesDestroy( base );
            base = NULL;
            break;
        }

        (GLvoid) memset( *next, 0, size );
        (*next)->visualID = GLX_DONT_CARE;
        (*next)->visualType = GLX_DONT_CARE;
        (*next)->visualRating = GLX_NONE;
        (*next)->transparentPixel = GLX_NONE;
        (*next)->transparentRed = GLX_DONT_CARE;
        (*next)->transparentGreen = GLX_DONT_CARE;
        (*next)->transparentBlue = GLX_DONT_CARE;
        (*next)->transparentAlpha = GLX_DONT_CARE;
        (*next)->transparentIndex = GLX_DONT_CARE;
        (*next)->xRenderable = GLX_DONT_CARE;
        (*next)->fbconfigID = GLX_DONT_CARE;

        next = & ((*next)->next);
    }

    return base;
}

/**
 * Determine if two context-modes are the same.  This is intended to be used
 * by libGL implementations to compare to sets of driver generated FBconfigs.
 *
 * \param a  Context-mode to be compared.
 * \param b  Context-mode to be compared.
 * \returns \c GL_TRUE if the two context-modes are the same.  \c GL_FALSE is
 *          returned otherwise.
 */
Bool
__glContextModesAreSame( const __GLcontextModes * a,
                                     const __GLcontextModes * b )
{
    return( (a->rgbMode == b->rgbMode) &&
            (a->doubleBufferMode == b->doubleBufferMode) &&
            (a->stereoMode == b->stereoMode) &&
            (a->redBits == b->redBits) &&
            (a->greenBits == b->greenBits) &&
            (a->blueBits == b->blueBits) &&
            (a->alphaBits == b->alphaBits) &&
            (a->rgbaBits == b->rgbaBits) &&
            (a->accumRedBits == b->accumRedBits) &&
            (a->accumGreenBits == b->accumGreenBits) &&
            (a->accumBlueBits == b->accumBlueBits) &&
            (a->accumAlphaBits == b->accumAlphaBits) &&
            (a->depthBits == b->depthBits) &&
            (a->stencilBits == b->stencilBits) &&
            (a->numAuxBuffers == b->numAuxBuffers) &&
            (a->level == b->level) &&
            (a->pixmapMode == b->pixmapMode) &&
            (a->visualRating == b->visualRating) &&
            (a->transparentPixel == b->transparentPixel) &&

            ((a->transparentPixel != GLX_TRANSPARENT_RGB) ||
             ((a->transparentRed == b->transparentRed) &&
              (a->transparentGreen == b->transparentGreen) &&
              (a->transparentBlue == b->transparentBlue) &&
              (a->transparentAlpha == b->transparentAlpha))) &&

            ((a->transparentPixel != GLX_TRANSPARENT_INDEX) ||
             (a->transparentIndex == b->transparentIndex)) &&

            (a->sampleBuffers == b->sampleBuffers) &&
            (a->samples == b->samples) &&
            ((a->drawableType & b->drawableType) != 0) &&
            (a->renderType == b->renderType) &&
            (a->maxPbufferWidth == b->maxPbufferWidth) &&
            (a->maxPbufferHeight == b->maxPbufferHeight) &&
            (a->maxPbufferPixels == b->maxPbufferPixels) );
}


/************************************************************************/

/*
** Free the per screen configs data as well as the array of
** __glXScreenConfigs.
*/
static GLvoid FreeScreenConfigs(__GLXdisplayPrivate *priv)
{
    __GLXscreenConfigs *psc;
    GLint i, screens;

    /* Free screen configuration information */
    psc = priv->screenConfigs;
    screens = ScreenCount(priv->dpy);
    for (i = 0; i < screens; i++, psc++) {
        if (psc->configs) {
            __glContextModesDestroy( psc->configs );
            psc->configs = NULL;

            if(psc->effectiveGLXexts)
                Xfree(psc->effectiveGLXexts);

            if ( psc->old_configs != NULL ) {
                Xfree( psc->old_configs );
                psc->old_configs = NULL;
                psc->numOldConfigs = 0;
            }
        }

        /* Free the direct rendering per screen data */
        if (psc->driScreen.private) {
            (*psc->driScreen.destroyScreen)(priv->dpy, i, psc->driScreen.private);
        }
        psc->driScreen.private = NULL;
    }
    XFree((char*) priv->screenConfigs);
}

/*
** Release the private memory referred to in a display private
** structure.  The caller will free the extension structure.
*/
static int __glXFreeDisplayPrivate(XExtData *extension)
{
    __GLXdisplayPrivate *priv = (__GLXdisplayPrivate*) extension->private_data;
    __GLXscreenConfigs *psc = priv->screenConfigs;
    __GLXcontext *ctx;
    int i;

    for (i = 0; i < ScreenCount(priv->dpy); i++, psc++) {
        if (psc->driScreen.private) {
            __DRIscreenPrivate *psp = (__DRIscreenPrivate *)(psc->driScreen.private);

            if (psp->dri3) {
                continue;
            }

#if USE_DRM_MAP
            (GLvoid)drmUnmap(psp->pLogicalAddr, psp->fbSize);
#else
            munmap(psp->pLogicalAddr, psp->fbSize);
#endif

            (GLvoid)drmUnmap((drmAddress)psp->pSAREA, SAREA_MAX);
            (GLvoid)drmClose(psp->fd);
            break;
        }
    }

    __glXDisplayIsClosed = GL_TRUE;

    __glXMutexLock();

    /* Force destroy all contexts on priv->contextList */
    ctx = priv->contextList;
    while (ctx) {
        /* Destroy the direct rendering context */
        if (ctx->isDirect) {
            if (ctx->driContext.private) {
                (*ctx->driContext.destroyContext)(priv->dpy, ctx->screen,
                                 ctx->driContext.private);
                ctx->driContext.private = NULL;
            }
        }
        /* Free the __GLXcontext */
        __glXFreeContext(priv, ctx);
        ctx = priv->contextList;
    }

    __glXMutexUnlock();

    FreeScreenConfigs(priv);
    if(priv->serverGLXvendor) {
        Xfree((char*)priv->serverGLXvendor);
        priv->serverGLXvendor = 0x0; /* to protect against double free's */
    }
    if(priv->serverGLXversion) {
        Xfree((char*)priv->serverGLXversion);
        priv->serverGLXversion = 0x0; /* to protect against double free's */
    }

    /* Free the direct rendering per display data */
    if (priv->driDisplay.private)
        (*priv->driDisplay.destroyDisplay)(priv->dpy,
                            priv->driDisplay.private);
    priv->driDisplay.private = NULL;

    XFree(priv->driDisplay.createScreen);

    Xfree((char*) priv);

    __glXExtensionPrivate = NULL;

    return 0;
}

/************************************************************************/

/*
** Query the version of the GLX extension.  This procedure works even if
** the client extension is not completely set up.
*/
static Bool QueryVersion(Display *dpy, int opcode, int *major, int *minor)
{
    xGLXQueryVersionReq *req;
    xGLXQueryVersionReply reply;

    /* Send the glXQueryVersion request */
    LockDisplay(dpy);
    GetReq(GLXQueryVersion,req);
    req->reqType = opcode;
    req->glxCode = X_GLXQueryVersion;
    req->majorVersion = GLX_MAJOR_VERSION;
    req->minorVersion = GLX_MINOR_VERSION;
    _XReply(dpy, (xReply*) &reply, 0, False);
    UnlockDisplay(dpy);
    SyncHandle();

    if (reply.majorVersion != GLX_MAJOR_VERSION) {
        /*
        ** The server does not support the same major release as this
        ** client.
        */
        return GL_FALSE;
    }
    *major = reply.majorVersion;
    *minor = min(reply.minorVersion, GLX_MINOR_VERSION);
    return GL_TRUE;
}


GLvoid
__glXInitializeVisualConfigFromTags( __GLcontextModes *config, int count,
                     const INT32 *bp, Bool tagged_only,
                     Bool fbconfig_style_tags )
{
    int i;

    if (!tagged_only) {
        /* Copy in the first set of properties */
        config->visualID = *bp++;

        config->visualType = __glConvertFromXvisualType( *bp++ );

        config->rgbMode = *bp++;

        config->redBits = *bp++;
        config->greenBits = *bp++;
        config->blueBits = *bp++;
        config->alphaBits = *bp++;
        config->accumRedBits = *bp++;
        config->accumGreenBits = *bp++;
        config->accumBlueBits = *bp++;
        config->accumAlphaBits = *bp++;

        config->doubleBufferMode = *bp++;
        config->stereoMode = *bp++;

        config->rgbaBits = *bp++;
        config->depthBits = *bp++;
        config->stencilBits = *bp++;
        config->numAuxBuffers = *bp++;
        config->level = *bp++;

        count -= __GLX_MIN_CONFIG_PROPS;
    }

    /*
    ** Additional properties may be in a list at the end
    ** of the reply.  They are in pairs of property type
    ** and property value.
    */

#define FETCH_OR_SET(tag) \
    config-> tag = ( fbconfig_style_tags ) ? *bp++ : 1

    for (i = 0; i < count; i += 2 ) {
        switch(*bp++) {
          case GLX_RGBA:
            FETCH_OR_SET( rgbMode );
            break;
          case GLX_BUFFER_SIZE:
            config->rgbaBits = *bp++;
            break;
          case GLX_LEVEL:
            config->level = *bp++;
            break;
          case GLX_DOUBLEBUFFER:
            FETCH_OR_SET( doubleBufferMode );
            break;
          case GLX_STEREO:
            FETCH_OR_SET( stereoMode );
            break;
          case GLX_AUX_BUFFERS:
            config->numAuxBuffers = *bp++;
            break;
          case GLX_RED_SIZE:
            config->redBits = *bp++;
            break;
          case GLX_GREEN_SIZE:
            config->greenBits = *bp++;
            break;
          case GLX_BLUE_SIZE:
            config->blueBits = *bp++;
            break;
          case GLX_ALPHA_SIZE:
            config->alphaBits = *bp++;
            break;
          case GLX_DEPTH_SIZE:
            config->depthBits = *bp++;
            break;
          case GLX_STENCIL_SIZE:
            config->stencilBits = *bp++;
            break;
          case GLX_ACCUM_RED_SIZE:
            config->accumRedBits = *bp++;
            break;
          case GLX_ACCUM_GREEN_SIZE:
            config->accumGreenBits = *bp++;
            break;
          case GLX_ACCUM_BLUE_SIZE:
            config->accumBlueBits = *bp++;
            break;
          case GLX_ACCUM_ALPHA_SIZE:
            config->accumAlphaBits = *bp++;
            break;
          case GLX_VISUAL_CAVEAT_EXT:
            config->visualRating = *bp++;
            break;
          case GLX_X_VISUAL_TYPE:
            config->visualType = *bp++;
            break;
          case GLX_TRANSPARENT_TYPE:
            config->transparentPixel = *bp++;
            break;
          case GLX_TRANSPARENT_INDEX_VALUE:
            config->transparentIndex = *bp++;
            break;
          case GLX_TRANSPARENT_RED_VALUE:
            config->transparentRed = *bp++;
            break;
          case GLX_TRANSPARENT_GREEN_VALUE:
            config->transparentGreen = *bp++;
            break;
          case GLX_TRANSPARENT_BLUE_VALUE:
            config->transparentBlue = *bp++;
            break;
          case GLX_TRANSPARENT_ALPHA_VALUE:
            config->transparentAlpha = *bp++;
            break;
          case GLX_VISUAL_ID:
            config->visualID = *bp++;
            break;
          case GLX_DRAWABLE_TYPE:
            config->drawableType = *bp++;
            break;
          case GLX_RENDER_TYPE:
            config->renderType = *bp++;
            break;
          case GLX_X_RENDERABLE:
            config->xRenderable = *bp++;
            break;
          case GLX_FBCONFIG_ID:
            config->fbconfigID = *bp++;
            break;
          case GLX_MAX_PBUFFER_WIDTH:
            config->maxPbufferWidth = *bp++;
            break;
          case GLX_MAX_PBUFFER_HEIGHT:
            config->maxPbufferHeight = *bp++;
            break;
          case GLX_MAX_PBUFFER_PIXELS:
            config->maxPbufferPixels = *bp++;
            break;
          case GLX_SAMPLE_BUFFERS_SGIS:
            config->sampleBuffers = *bp++;
            break;
          case GLX_SAMPLES_SGIS:
            config->samples = *bp++;
            break;
          case None:
            i = count;
            break;
          default:
            break;
        }
    }

    /* Force no aux buffer support */
    config->numAuxBuffers = 0;

    config->renderType = (config->rgbMode) ? GLX_RGBA_BIT : GLX_COLOR_INDEX_BIT;

    config->haveAccumBuffer = ((config->accumRedBits +
                   config->accumGreenBits +
                   config->accumBlueBits +
                   config->accumAlphaBits) > 0);
    config->haveDepthBuffer = (config->depthBits > 0);
    config->haveStencilBuffer = (config->stencilBits > 0);
}

static unsigned
FilterGlContextModes( __GLcontextModes ** server_modes,
          const __GLcontextModes * driver_modes )
{
    __GLcontextModes * m;
    __GLcontextModes ** prev_next;
    const __GLcontextModes * check;
    unsigned modes_count = 0;

    if ( driver_modes == NULL ) {
        fprintf(stderr, "libGL warning: 3D driver returned no fbconfigs.\n");
        return 0;
    }

    /* For each mode in server_modes, check to see if a matching mode exists
     * in driver_modes.  If not, then the mode is not available.
     */
    prev_next = server_modes;
    for ( m = *prev_next ; m != NULL ; m = *prev_next ) {
        Bool do_delete = GL_TRUE;

        for ( check = driver_modes ; check != NULL ; check = check->next ) {
            if ( __glContextModesAreSame( m, check ) ) {
                /*
                ** A temp solution to set the masks
                */
                switch (m->rgbaBits )
                {
                    case 32:
                        m->alphaMask = 0xFF000000;
                        m->redMask = 0x00FF0000;
                        m->greenMask = 0x0000FF00;
                        m->blueMask= 0x000000FF;
                        break;
                    case 16:
                        m->alphaMask = 0;
                        m->redMask = 0x0000f800;
                        m->greenMask= 0x000007e0;
                        m->blueMask = 0x0000001f;
                        break;
                    default:
                        assert(0);
                        break;
                }
                do_delete = GL_FALSE;
                break;
            }
        }

        /* The 3D has to support all the modes that match the GLX visuals
         * sent from the X server.
         */
        if ( do_delete ) {
            *prev_next = m->next;

            m->next = NULL;
            __glContextModesDestroy( m );
        }
        else {
            modes_count++;
            prev_next = & m->next;
        }
    }

    return modes_count;
}


/**
 * Perform the required libGL-side initialization and call the client-side
 * driver's \c __driCreateNewScreen function.
 *
 * \param dpy    Display pointer.
 * \param scrn   Screen number on the display.
 * \param psc    DRI screen information.
 * \param driDpy DRI display information.
 * \param createNewScreen  Pointer to the client-side driver's
 *               \c __driCreateNewScreen function.
 * \returns A pointer to the \c __DRIscreenPrivate structure returned by
 *          the client-side driver on success, or \c NULL on failure.
 */
static GLvoid *
CallCreateNewScreen_dri1(Display *dpy, int scrn, __DRIscreen *psc,
            __DRIdisplay * driDpy,
            CreateNewScreenFunc createNewScreen)
{
    __DRIscreenPrivate *psp = NULL;
    drm_handle_t hSAREA;
    drmAddress pSAREA = MAP_FAILED;
    char *BusID;
    __DRIversion   ddx_version;
    __DRIversion   dri_version;
    __DRIversion   drm_version;
    __DRIframebuffer  framebuffer;
#if USE_DRM_MAP
#else
    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
#endif
    int   fd = -1;
#if USE_DRM_MAP
#else
    char *map = NULL;
    int   fbfd = -1;
    int         k;
#endif
    int   status;
    const char * err_msg;
    const char * err_extra;
    int api_ver = 1.0; // __glXGetInternalVersion();

    dri_version.major = driDpy->private->driMajor;
    dri_version.minor = driDpy->private->driMinor;
    dri_version.patch = driDpy->private->driPatch;
    dri_version.dri3 = driDpy->private->dri3;

    err_msg = "XF86DRIOpenConnection";
    err_extra = NULL;

    memset (&framebuffer, 0, sizeof (framebuffer));
    framebuffer.base = MAP_FAILED;

    if (XF86DRIOpenConnection(dpy, scrn, &hSAREA, &BusID)) {

        fd = drmOpen(NULL,BusID);

        Xfree(BusID); /* No longer needed */

        err_msg = "open DRM";
        err_extra = strerror( -fd );

        if (fd >= 0) {
            drm_magic_t magic;

            err_msg = "drmGetMagic";
            err_extra = NULL;

            if (!drmGetMagic(fd, &magic))
            {
                drmVersionPtr version = drmGetVersion(fd);
                if (version)
                {
                    drm_version.major = version->version_major;
                    drm_version.minor = version->version_minor;
                    drm_version.patch = version->version_patchlevel;
                    drmFreeVersion(version);
                }
                else
                {
                    drm_version.major = -1;
                    drm_version.minor = -1;
                    drm_version.patch = -1;
                }

                err_msg = "XF86DRIAuthConnection";
                if (XF86DRIAuthConnection(dpy, scrn, magic))
                {
                    char *driverName;

                    /*
                     * Get device name (like "tdfx") and the ddx version numbers.
                     * We'll check the version in each DRI driver's "createScreen"
                     * function.
                    */
                    err_msg = "XF86DRIGetClientDriverName";
                    if (XF86DRIGetClientDriverName(dpy, scrn,
                           &ddx_version.major,
                           &ddx_version.minor,
                           &ddx_version.patch,
                           &driverName))
                    {
                        drm_handle_t  hFB;
                        int        junk;

                        /* No longer needed. */
                        Xfree( driverName );


                        /*
                         * Get device-specific info.  pDevPriv will point to a struct
                         * (such as DRIRADEONRec in xfree86/driver/ati/radeon_dri.h)
                         * that has information about the screen size, depth, pitch,
                         * ancilliary buffers, DRM mmap handles, etc.
                        */
                        err_msg = "XF86DRIGetDeviceInfo";
                        if (XF86DRIGetDeviceInfo(dpy, scrn,
                             &hFB,
                             &junk,
                             &framebuffer.size,
                             &framebuffer.stride,
                             &framebuffer.dev_priv_size,
                             &framebuffer.dev_priv))
                        {
#if USE_DRM_MAP
                            /* Set on screen frame buffer physical address */
                            framebuffer.base = (unsigned char *)hFB;
                            framebuffer.width = DisplayWidth(dpy, scrn);
                            framebuffer.height = DisplayHeight(dpy, scrn);
                            status = drmMap(fd, hFB, framebuffer.size,
                                     &framebuffer.dev_priv);
                            if (status == 0) {
#else
                            const char *fb = getenv("__GL_DEV_FB");
                            if (!fb)
                            {
                                fb = "/dev/fb0";
                            }

                            if ((fbfd = open(fb, O_RDWR)) < 0)
                            {
                                (GLvoid)XF86DRICloseConnection(dpy, scrn);
                                 return NULL;
                            }

                            /* quiet valgrind */
                            memset (&fix, '\0', sizeof (fix));
                            if ((k=ioctl(fbfd, FBIOGET_FSCREENINFO, &fix)) < 0)
                            {
                                (GLvoid)XF86DRICloseConnection(dpy, scrn);
                                close (fd);
                                return NULL;
                            }
                            framebuffer.base = fix.smem_start;

                            memset (&var, '\0', sizeof (var));
                            if ((k=ioctl(fbfd, FBIOGET_VSCREENINFO, &var)) < 0)
                            {
                                close (fd);
                                return NULL;
                            }
                            map = mmap(0, fix.smem_len, PROT_READ | PROT_WRITE,  MAP_FILE | MAP_SHARED, fbfd, 0);

                            framebuffer.dev_priv = map;
                            framebuffer.size = fix.smem_len;

                            close(fbfd);
#endif
                            /*
                             * Map the SAREA region.  Further mmap regions may be setup in
                             * each DRI driver's "createScreen" function.
                            */
                            status = drmMap(fd, hSAREA, SAREA_MAX,
                                     &pSAREA);

                            err_msg = "drmMap of sarea";
                            err_extra = strerror( -status );

                            if ( status == 0 ) {
                                __GLcontextModes * driver_modes = NULL;
                                __GLXscreenConfigs *configs = psc->screenConfigs;

                                err_msg = "InitDriver";
                                err_extra = NULL;
                                psp = (*createNewScreen)(dpy, scrn,
                                             psc,
                                             configs->configs,
                                             & ddx_version,
                                             & dri_version,
                                             & drm_version,
                                             & framebuffer,
                                             pSAREA,
                                             fd,
                                             api_ver,
                                             & driver_modes );
                                FilterGlContextModes( & configs->configs, driver_modes );
                                __glContextModesDestroy( driver_modes );
                            }
#if USE_DRM_MAP
                         }
#endif
                        }
                    }
                }
            }
        }
    }

    if ( psp == NULL ) {
        if ( pSAREA != MAP_FAILED ) {
            (GLvoid)drmUnmap(pSAREA, SAREA_MAX);
        }
#if USE_DRM_MAP
        if ( framebuffer.dev_priv != MAP_FAILED ) {
            (GLvoid)drmUnmap(framebuffer.dev_priv, framebuffer.size);
        }
#endif
        if ( fd >= 0 ) {
            (GLvoid)drmClose(fd);
        }

        (GLvoid)XF86DRICloseConnection(dpy, scrn);

        if ( err_extra != NULL ) {
            fprintf(stderr, "libGL error: %s failed (%s)\n", err_msg,
                err_extra);
        }
        else {
            fprintf(stderr, "libGL error: %s failed\n", err_msg );
        }

        fprintf(stderr, "libGL error: reverting to (slow) indirect rendering\n");
    }

    return psp;
}

#ifdef X11_DRI3

#include <X11/Xlib-xcb.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xdamage.h>
#include <xcb/xcb.h>
#include <xcb/dri3.h>
#include <xcb/present.h>

static int
dri3_open(Display *dpy,
          Window root,
          CARD32 provider)
{
   xcb_dri3_open_cookie_t       cookie;
   xcb_dri3_open_reply_t        *reply;
   xcb_connection_t             *c = XGetXCBConnection(dpy);
   int                          fd;

   cookie = xcb_dri3_open(c,
                          root,
                          None);

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
static GLvoid *
CallCreateNewScreen_dri3(Display *dpy, int scrn, __DRIscreen *psc,
            __DRIdisplay * driDpy,
            CreateNewScreenFunc createNewScreen)
{
    __DRIscreenPrivate *psp = NULL;
    __DRIversion   ddx_version;
    __DRIversion   dri_version;
    __DRIversion   drm_version;

    int api_ver = 1.0;
    int fd;

    dri_version.major = driDpy->private->driMajor;
    dri_version.minor = driDpy->private->driMinor;
    dri_version.patch = driDpy->private->driPatch;
    dri_version.dri3 = driDpy->private->dri3;
    {
        __GLcontextModes * driver_modes = NULL;
        __GLXscreenConfigs *configs = psc->screenConfigs;
        fd = dri3_open(dpy, RootWindow(dpy, scrn), None);
        psp = (*createNewScreen)(dpy, scrn,
                     psc,
                     configs->configs,
                     & ddx_version,
                     & dri_version,
                     & drm_version,
                     NULL,
                     NULL,
                     fd,
                     api_ver,
                     & driver_modes );
        FilterGlContextModes( & configs->configs, driver_modes );
        __glContextModesDestroy( driver_modes );
    }


    if ( psp == NULL ) {
        fprintf(stderr, "libGL error: reverting to (slow) indirect rendering\n");
    }

    return psp;
}
#else
static GLvoid *
CallCreateNewScreen_dri3(Display *dpy, int scrn, __DRIscreen *psc,
            __DRIdisplay * driDpy,
            CreateNewScreenFunc createNewScreen)
{
    return NULL;
}
#endif


static GLvoid *
CallCreateNewScreen(Display *dpy, int scrn, __DRIscreen *psc,
            __DRIdisplay * driDpy,
            CreateNewScreenFunc createNewScreen)
{
    gcmASSERT((driDpy != gcvNULL));

    if (driDpy->private->dri3)
    {
        return CallCreateNewScreen_dri3(dpy, scrn, psc, driDpy, createNewScreen);
    }

    return CallCreateNewScreen_dri1(dpy, scrn, psc, driDpy, createNewScreen);
}

/*
** Allocate the memory for the per screen configs for each screen.
** If that works then fetch the per screen configs data.
*/
static Bool AllocAndFetchScreenConfigs(Display *dpy, __GLXdisplayPrivate *priv)
{
    xGLXGetVisualConfigsReq *req;
    xGLXGetFBConfigsReq *fb_req;
    xGLXVendorPrivateWithReplyReq *vpreq;
    xGLXGetFBConfigsSGIXReq *sgi_req;
    xGLXGetVisualConfigsReply reply;
    __GLXscreenConfigs *psc;
    __GLcontextModes *config;
    GLint i, j, nprops, screens;
    INT32 buf[__GLX_TOTAL_CONFIG], *props;
    unsigned supported_request = 0;
    unsigned prop_size;

    /*
    ** First allocate memory for the array of per screen configs.
    */
    screens = ScreenCount(dpy);
    psc = (__GLXscreenConfigs*) Xmalloc(screens * sizeof(__GLXscreenConfigs));
    if (!psc) {
        return GL_FALSE;
    }
    memset(psc, 0, screens * sizeof(__GLXscreenConfigs));
    priv->screenConfigs = psc;

    priv->serverGLXversion = __glXGetStringFromServer(dpy, priv->majorOpcode,
                     X_GLXQueryServerString,
                     0, GLX_VERSION);
    if ( priv->serverGLXversion == NULL ) {
        FreeScreenConfigs(priv);
        return GL_FALSE;
    }

    if ( atof( priv->serverGLXversion ) >= 1.3 ) {
        supported_request = 1;
    }

    /*
    ** Now fetch each screens configs structures.  If a screen supports
    ** GL (by returning a numVisuals > 0) then allocate memory for our
    ** config structure and then fill it in.
    */
    for (i = 0; i < screens; i++, psc++) {
        if ( supported_request != 1 ) {
            psc->serverGLXexts = __glXGetStringFromServer(dpy, priv->majorOpcode,
                                        X_GLXQueryServerString,
                                        i, GLX_EXTENSIONS);
            if ( strstr( psc->serverGLXexts, "GLX_SGIX_fbconfig" ) != NULL ) {
                supported_request = 2;
            }
            else {
                supported_request = 3;
            }
        }

        LockDisplay(dpy);
        switch( supported_request ) {
          case 1:
            GetReq(GLXGetFBConfigs,fb_req);
            fb_req->reqType = priv->majorOpcode;
            fb_req->glxCode = X_GLXGetFBConfigs;
            fb_req->screen = i;
            break;

          case 2:
            GetReqExtra(GLXVendorPrivateWithReply,
                sz_xGLXGetFBConfigsSGIXReq-sz_xGLXVendorPrivateWithReplyReq,vpreq);
            sgi_req = (xGLXGetFBConfigsSGIXReq *) vpreq;
            sgi_req->reqType = priv->majorOpcode;
            sgi_req->glxCode = X_GLXVendorPrivateWithReply;
            sgi_req->vendorCode = X_GLXvop_GetFBConfigsSGIX;
            sgi_req->screen = i;
            break;

          case 3:
            GetReq(GLXGetVisualConfigs,req);
            req->reqType = priv->majorOpcode;
            req->glxCode = X_GLXGetVisualConfigs;
            req->screen = i;
            break;
         }

        if (!_XReply(dpy, (xReply*) &reply, 0, False)) {
            /* Something is busted. Punt. */
            UnlockDisplay(dpy);
            FreeScreenConfigs(priv);
            return GL_FALSE;
        }
        UnlockDisplay(dpy);

        if (!reply.numVisuals) {
            /* This screen does not support GL rendering */
            UnlockDisplay(dpy);
            continue;
        }

        /* Check number of properties */
        nprops = reply.numProps;
        if ((nprops < __GLX_MIN_CONFIG_PROPS) ||
            (nprops > __GLX_MAX_CONFIG_PROPS)) {
            /* Huh?  Not in protocol defined limits.  Punt */
            UnlockDisplay(dpy);
            SyncHandle();
            FreeScreenConfigs(priv);
            return GL_FALSE;
        }

        /* Allocate memory for our config structure */
        psc->configs = __glContextModesCreate(reply.numVisuals);
        if (!psc->configs) {
            UnlockDisplay(dpy);
            SyncHandle();
            FreeScreenConfigs(priv);
            return GL_FALSE;
        }

        /* Allocate memory for the properties, if needed */
        if ( supported_request != 3 ) {
            nprops *= 2;
        }

        prop_size = nprops * __GLX_SIZE_INT32;

        if (prop_size <= sizeof(buf)) {
            props = buf;
        } else {
            props = (INT32 *) Xmalloc(prop_size);
        }

        /* Read each config structure and convert it into our format */
        config = psc->configs;
        for (j = 0; j < reply.numVisuals; j++) {
            assert( config != NULL );
            _XRead(dpy, (char *)props, prop_size);

            if ( supported_request != 3 ) {
                config->rgbMode = GL_TRUE;
                config->drawableType = GLX_WINDOW_BIT;
            }
            else {
                config->drawableType = GLX_WINDOW_BIT | GLX_PIXMAP_BIT;
            }

            __glXInitializeVisualConfigFromTags( config, nprops, props,
                            (supported_request != 3), GL_TRUE );
            if ( config->fbconfigID == GLX_DONT_CARE ) {
                config->fbconfigID = config->visualID;
            }
            config->screen = i;
            config = config->next;
        }
        if (props != buf) {
            Xfree((char *)props);
        }
        UnlockDisplay(dpy);

        /* Initialize per screen dynamic client GLX extensions */
        psc->ext_list_first_time = GL_TRUE;

        /* Initialize the direct rendering per screen data and functions */
        if (priv->driDisplay.private != NULL) {
            if (priv->driDisplay.createNewScreen &&
                priv->driDisplay.createNewScreen[i]) {

                psc->driScreen.screenConfigs = (GLvoid *)psc;
                psc->driScreen.private =
                    CallCreateNewScreen(dpy, i, & psc->driScreen,
                            & priv->driDisplay,
                            priv->driDisplay.createNewScreen[i] );
            }
        }
    }

    SyncHandle();
    return GL_TRUE;
}

/*
** Declare this function as the constructor when libGL.so is dlopened.
*/
GLvoid __attribute__((constructor)) __glXFirstInit(GLvoid)
{
    XInitThreads();
}

/*
** Declare this function as the destructor when libGL.so is dlclosed.
*/
GLvoid __attribute__((destructor)) __glXFinalCleanUp(GLvoid)
{
    /* Clean up resources before exiting.
    */
    if (__glXExtensionPrivate) {
        printf("Perform final cleanup before process termination!\n");
        __glXFreeDisplayPrivate(__glXExtensionPrivate);
    }
}

/*
** Initialize the client side extension code.
*/
__GLXdisplayPrivate *__glXInitialize(Display* dpy)
{
    XExtDisplayInfo *info = __glXFindDisplay(dpy);
    XExtData **privList, *private, *found;
    __GLXdisplayPrivate *dpyPriv;
    XEDataObject dataObj;
    int major, minor;
    int event_base, error_base;

    if (!XextHasExtension(info)) {
        /* No GLX extension supported by this server. Oh well. */
        XMissingExtension(dpy, __glXExtensionName);
        return 0;
    }

    /* See if a display private already exists.  If so, return it */
    dataObj.display = dpy;
    privList = XEHeadOfExtensionList(dataObj);
    found = XFindOnExtensionList(privList, info->codes->extension);
    if (found) {
        return (__GLXdisplayPrivate *) found->private_data;
    }

    /* See if the versions are compatible */
    if (!QueryVersion(dpy, info->codes->major_opcode, &major, &minor)) {
        /* The client and server do not agree on versions.  Punt. */
        return 0;
    }

    /* Verify fixes extensions is available */
    if (!XFixesQueryExtension(dpy, &event_base, &error_base)) {
        printf("The X server does not have Xfixes extensions!\n");
        return 0;
    }

    /* Verify damage extensions is available */
    if (!XDamageQueryExtension(dpy, &event_base, &error_base)) {
        printf("The X server does not have Xdamage extensions!\n");
        return 0;
    }

    /*
    ** Allocate memory for all the pieces needed for this buffer.
    */
    private = (XExtData *) Xmalloc(sizeof(XExtData));
    if (!private) {
        return 0;
    }
    dpyPriv = (__GLXdisplayPrivate *) Xmalloc(sizeof(__GLXdisplayPrivate));
    if (!dpyPriv) {
        Xfree((char*) private);
        return 0;
    }

    memset((char *) dpyPriv, 0, sizeof(__GLXdisplayPrivate));

    /*
    ** Init the display private and then read in the screen config
    ** structures from the server.
    */
    dpyPriv->majorOpcode = info->codes->major_opcode;
    dpyPriv->majorVersion = major;
    dpyPriv->minorVersion = minor;
    dpyPriv->dpy = dpy;

    dpyPriv->serverGLXvendor = 0x0;
    dpyPriv->serverGLXversion = 0x0;

    /*
    ** Initialize the direct rendering per display data and functions.
    ** Note: This _must_ be done before calling any other DRI routines
    ** (e.g., those called in AllocAndFetchScreenConfigs).
    */
    if (getenv("LIBGL_ALWAYS_INDIRECT")) {
        /* Assinging zero here assures we'll never go direct */
        /* NOT supported currently */
        GL_ASSERT(0);
        dpyPriv->driDisplay.private = 0;
        dpyPriv->driDisplay.destroyDisplay = 0;
        dpyPriv->driDisplay.createScreen = 0;
    }
    else {
        dpyPriv->driDisplay.private =
            driCreateDisplay(dpy, &dpyPriv->driDisplay);
    }

    if (!AllocAndFetchScreenConfigs(dpy, dpyPriv)) {
        Xfree((char*) dpyPriv);
        Xfree((char*) private);
        return 0;
    }

    /*
    ** Fill in the private structure.  This is the actual structure that
    ** hangs off of the Display structure.  Our private structure is
    ** referred to by this structure.  Got that?
    */
    private->number = info->codes->extension;
    private->next = 0;
    private->free_private = __glXFreeDisplayPrivate;
    private->private_data = (char *) dpyPriv;
    XAddToExtensionList(privList, private);

    /* Cache a GLX ext private pointer that will be used in __glXFinalCleanUp.
    */
    __glXExtensionPrivate = private;

    _glapi_set_context((GLvoid *)__glxNopContext);

    dummyContext.currentDpy = 0;

    if (dpyPriv->majorVersion == 1 && dpyPriv->minorVersion >= 1) {
        __glXClientInfo(dpy, dpyPriv->majorOpcode);
    }

    return dpyPriv;
}

/*
** Setup for sending a GLX command on dpy.  Make sure the extension is
** initialized.  Try to avoid calling __glXInitialize as its kinda slow.
*/
CARD8 __glXSetupForCommand(Display *dpy)
{
    GLXContext gc;
    __GLXdisplayPrivate *priv;

    /* If this thread has a current context, flush its rendering commands */
    gc = __glXGetCurrentContext();
    if (gc->currentDpy) {
        /* Flush rendering buffer of the current context, if any */
        (GLvoid) __glXFlushRenderBuffer(gc, gc->pc);

        if (gc->currentDpy == dpy) {
            /* Use opcode from gc because its right */
            return gc->majorOpcode;
        } else {
            /*
            ** Have to get info about argument dpy because it might be to
            ** a different server
            */
        }
    }

    /* Forced to lookup extension via the slow initialize route */
    priv = __glXInitialize(dpy);
    if (!priv) {
        return 0;
    }
    return priv->majorOpcode;
}

/*
** Flush the drawing command transport buffer.
*/
GLubyte *__glXFlushRenderBuffer(__GLXcontext *ctx, GLubyte *pc)
{
    Display *dpy;
#if defined(INDIRECT_SUPPORT)
    xGLXRenderReq *req;
    GLint size;
#endif

    if (!(dpy = ctx->currentDpy)) {
        /* Using the dummy context */
        ctx->pc = ctx->buf;
        return ctx->pc;
    }

#if defined(INDIRECT_SUPPORT)
    size = pc - ctx->buf;
    if (size) {
        /* Send the entire buffer as an X request */
        LockDisplay(dpy);
        GetReq(GLXRender,req);
        req->reqType = ctx->majorOpcode;
        req->glxCode = X_GLXRender;
        req->contextTag = ctx->currentContextTag;
        req->length += (size + 3) >> 2;
        _XSend(dpy, (char *)ctx->buf, size);
        UnlockDisplay(dpy);
        SyncHandle();
    }
#endif

    /* Reset pointer and return it */
    ctx->pc = ctx->buf;
    return ctx->pc;
}


/**
 * Send a portion of a GLXRenderLarge command to the server.  The advantage of
 * this function over \c __glXSendLargeCommand is that callers can use the
 * data buffer in the GLX context and may be able to avoid allocating an
 * extra buffer.  The disadvantage is the clients will have to do more
 * GLX protocol work (i.e., calculating \c totalRequests, etc.).
 *
 * \sa __glXSendLargeCommand
 *
 * \param gc             GLX context
 * \param requestNumber  Which part of the whole command is this?  The first
 *                       request is 1.
 * \param totalRequests  How many requests will there be?
 * \param data           Command data.
 * \param dataLen        Size, in bytes, of the command data.
 */
GLvoid __glXSendLargeChunk(__GLXcontext *gc, GLint requestNumber,
             GLint totalRequests,
             const GLvoid * data, GLint dataLen)
{
#if defined(INDIRECT_SUPPORT)
    Display *dpy = gc->currentDpy;
    xGLXRenderLargeReq *req;

    if ( requestNumber == 1 ) {
        LockDisplay(dpy);
    }

    GetReq(GLXRenderLarge,req);
    req->reqType = gc->majorOpcode;
    req->glxCode = X_GLXRenderLarge;
    req->contextTag = gc->currentContextTag;
    req->length += (dataLen + 3) >> 2;
    req->requestNumber = requestNumber;
    req->requestTotal = totalRequests;
    req->dataBytes = dataLen;
    Data(dpy, data, dataLen);

    if ( requestNumber == totalRequests ) {
        UnlockDisplay(dpy);
        SyncHandle();
    }
#endif
}


/**
 * Send a command that is too large for the GLXRender protocol request.
 *
 * Send a large command, one that is too large for some reason to
 * send using the GLXRender protocol request.  One reason to send
 * a large command is to avoid copying the data.
 *
 * \param ctx        GLX context
 * \param header     Header data.
 * \param headerLen  Size, in bytes, of the header data.  It is assumed that
 *                   the header data will always be small enough to fit in
 *                   a single X protocol packet.
 * \param data       Command data.
 * \param dataLen    Size, in bytes, of the command data.
 */
GLvoid __glXSendLargeCommand(__GLXcontext *ctx,
               const GLvoid *header, GLint headerLen,
               const GLvoid *data, GLint dataLen)
{
#if defined(INDIRECT_SUPPORT)
    GLint maxSize;
    GLint totalRequests, requestNumber;

    /*
    ** Calculate the maximum amount of data can be stuffed into a single
    ** packet.  sz_xGLXRenderReq is added because bufSize is the maximum
    ** packet size minus sz_xGLXRenderReq.
    */
    maxSize = (ctx->bufSize + sz_xGLXRenderReq) - sz_xGLXRenderLargeReq;
    totalRequests = 1 + (dataLen / maxSize);
    if (dataLen % maxSize) totalRequests++;

    /*
    ** Send all of the command, except the large array, as one request.
    */
    assert( headerLen <= maxSize );
    __glXSendLargeChunk(ctx, 1, totalRequests, header, headerLen);

    /*
    ** Send enough requests until the whole array is sent.
    */
    for ( requestNumber = 2 ; requestNumber <= (totalRequests - 1) ; requestNumber++ ) {
        __glXSendLargeChunk(ctx, requestNumber, totalRequests, data, maxSize);
        data = (const GLvoid *) (((const GLubyte *) data) + maxSize);
        dataLen -= maxSize;
        assert( dataLen > 0 );
    }

    assert( dataLen <= maxSize );
    __glXSendLargeChunk(ctx, requestNumber, totalRequests, data, dataLen);
#endif
}

/************************************************************************/

GLXContext glXGetCurrentContext(GLvoid)
{
    GLXContext cx = __glXGetCurrentContext();

    if (cx == &dummyContext) {
        return NULL;
    } else {
        return cx;
    }
}

GLXDrawable glXGetCurrentDrawable(GLvoid)
{
    GLXContext gc = __glXGetCurrentContext();
    return gc->currentDrawable;
}


/************************************************************************/

/* Return the DRI per screen structure */
__DRIscreen *__glXFindDRIScreen(Display *dpy, int scrn)
{
    __DRIscreen *pDRIScreen = NULL;
    XExtDisplayInfo *info = __glXFindDisplay(dpy);
    XExtData **privList, *found;
    __GLXdisplayPrivate *dpyPriv;
    XEDataObject dataObj;

    dataObj.display = dpy;
    privList = XEHeadOfExtensionList(dataObj);
    found = XFindOnExtensionList(privList, info->codes->extension);

    if (found) {
        dpyPriv = (__GLXdisplayPrivate *)found->private_data;
        pDRIScreen = &dpyPriv->screenConfigs[scrn].driScreen;
    }

    return pDRIScreen;
}

/************************************************************************/

static Bool SendMakeCurrentRequest( Display *dpy, CARD8 opcode,
    GLXContextID gc, GLXContextTag old_gc, GLXDrawable draw, GLXDrawable read,
    xGLXMakeCurrentReply * reply );

/**
 * Sends a GLX protocol message to the specified display to make the context
 * and the drawables current.
 *
 * \param dpy     Display to send the message to.
 * \param opcode  Major opcode value for the display.
 * \param gc_id   Context tag for the context to be made current.
 * \param draw    Drawable ID for the "draw" drawable.
 * \param read    Drawable ID for the "read" drawable.
 * \param reply   Space to store the X-server's reply.
 *
 * \warning
 * This function assumes that \c dpy is locked with \c LockDisplay on entry.
 */
static Bool SendMakeCurrentRequest( Display *dpy, CARD8 opcode,
                    GLXContextID gc_id, GLXContextTag gc_tag,
                    GLXDrawable draw, GLXDrawable read,
                    xGLXMakeCurrentReply * reply )
{
#if defined(INDIRECT_SUPPORT)
    if ( draw == read ) {
        xGLXMakeCurrentReq *req;

        GetReq(GLXMakeCurrent,req);
        req->reqType = opcode;
        req->glxCode = X_GLXMakeCurrent;
        req->drawable = draw;
        req->context = gc_id;
        req->oldContextTag = gc_tag;
    }
    else {
        xGLXMakeContextCurrentReq *req;

        GetReq(GLXMakeContextCurrent,req);
        req->reqType = opcode;
        req->glxCode = X_GLXMakeContextCurrent;
        req->drawable = draw;
        req->readdrawable = read;
        req->context = gc_id;
        req->oldContextTag = gc_tag;
    }

    return _XReply(dpy, (xReply*) reply, 0, False);
#else
    return GL_TRUE;
#endif
}


/*
** Make a particular context current.
** NOTE: this is in this file so that it can access dummyContext.
*/
static Bool MakeContextCurrent(Display *dpy,
                   GLXDrawable draw, GLXDrawable read,
                   GLXContext gc)
{
    xGLXMakeCurrentReply reply = {0};
    GLXContext oldGC;
    CARD8 opcode, oldOpcode = 0;
    Bool sentRequestToOldDpy = False;
    Bool bindReturnValue = True;
    Bool status = GL_TRUE;

    /*
    ** Make sure that the new context has a nonzero ID.  In the request,
    ** a zero context ID is used only to mean that we bind to no current
    ** context.
    */
    if ((gc != NULL) && (gc->xid == None)) {
        return GL_FALSE;
    }

    oldGC = __glXGetCurrentContext();

    /* Skip MakeContextCurrent if App tries to rebind gc with the same drawable.
    */
    if (oldGC->currentDpy == dpy && oldGC == gc &&
        oldGC->currentDrawable == draw && oldGC->currentReadable == read) {
        return GL_TRUE;
    }


    if ((dpy != oldGC->currentDpy || (gc && gc->isDirect)) &&
        !oldGC->isDirect && oldGC != &dummyContext) {

        oldOpcode = __glXSetupForCommand(dpy);
        if (!oldOpcode) {
            return GL_FALSE;
        }

        /*
        ** We are either switching from one dpy to another and have to
        ** send a request to the previous dpy to unbind the previous
        ** context, or we are switching away from a indirect context to
        ** a direct context and have to send a request to the dpy to
        ** unbind the previous context.
        */
        sentRequestToOldDpy = True;
        LockDisplay(oldGC->currentDpy);

        if ( ! SendMakeCurrentRequest( oldGC->currentDpy, oldOpcode, None,
                           oldGC->currentContextTag, None, None,
                           &reply ) ) {
            /* The make current failed.  Just return GL_FALSE. */
            UnlockDisplay(oldGC->currentDpy);
            SyncHandle();
            return GL_FALSE;
        }

        oldGC->currentContextTag = 0;
    }

    _glapi_check_multithread();

    __glXMutexLock();

    /* Unbind the old direct rendering context */
    if (oldGC->isDirect) {
        if (oldGC->driContext.private) {
            if (! (*oldGC->driContext.unbindContext)(oldGC->currentDpy, oldGC->screen,
                        oldGC->currentDrawable, oldGC->currentDrawable,
                        &oldGC->driContext) ) {
                /* The make current failed.  Just return GL_FALSE. */
                status = GL_FALSE;
                goto MakeContextCurrent_exit;
            }
        }
        oldGC->currentContextTag = 0;
    }

    /* Bind the direct rendering context to the drawable */
    if (gc && gc->isDirect) {
        if (gc->driContext.private) {
            bindReturnValue =
                (*gc->driContext.bindContext)(dpy, gc->screen, draw, read, &gc->driContext);
        }
    } else {
        opcode = __glXSetupForCommand(dpy);
        if (!opcode) {
            status = GL_FALSE;
            goto MakeContextCurrent_exit;
        }

        /* Send a glXMakeCurrent request to bind the new context. */
        LockDisplay(dpy);
        bindReturnValue = SendMakeCurrentRequest( dpy, opcode,
                              gc ? gc->xid : None,
                              oldGC->currentContextTag,
                              draw, read, &reply );
        UnlockDisplay(dpy);
    }

    if (!bindReturnValue) {
        /* The make current failed. */
        if (gc && !gc->isDirect) {
            SyncHandle();
        }

        /* If the old context was direct rendering, then re-bind to it. */
        if (oldGC->isDirect) {
            if (oldGC->driContext.private) {
                if (! (*oldGC->driContext.bindContext)(oldGC->currentDpy, oldGC->screen,
                        oldGC->currentDrawable, oldGC->currentDrawable,
                        &oldGC->driContext) ) {
                    /*
                    ** The request failed; this cannot happen with the
                    ** current API.  If in the future the API is
                    ** extended to allow context sharing between
                    ** clients, then this may fail (because another
                    ** client may have grabbed the context); in that
                    ** case, we cannot undo the previous request, and
                    ** cannot adhere to the "no-op" behavior.
                    */
                }
            }
        }
        /*
        ** If we had just sent a request to a previous dpy, we have to
        ** undo that request (because if a command fails, it should act
        ** like a no-op) by making current to the previous context and
        ** drawable.
        */
        else if (sentRequestToOldDpy) {
            if ( !SendMakeCurrentRequest( oldGC->currentDpy, oldOpcode,
                          oldGC->xid, 0,
                          oldGC->currentDrawable,
                          oldGC->currentReadable, &reply ) ) {
                UnlockDisplay(oldGC->currentDpy);
                SyncHandle();
                /*
                ** The request failed; this cannot happen with the
                ** current API.  If in the future the API is extended to
                ** allow context sharing between clients, then this may
                ** fail (because another client may have grabbed the
                ** context); in that case, we cannot undo the previous
                ** request, and cannot adhere to the "no-op" behavior.
                */
            }
            else {
                UnlockDisplay(oldGC->currentDpy);
            }
            oldGC->currentContextTag = reply.contextTag;
        }

        status = GL_FALSE;
        goto MakeContextCurrent_exit;
    }

    /* Update our notion of what is current */
    if (gc == oldGC) {
        /*
        ** Even though the contexts are the same the drawable might have
        ** changed.  Note that gc cannot be the dummy, and that oldGC
        ** cannot be NULL, therefore if they are the same, gc is not
        ** NULL and not the dummy.
        */
        gc->currentDrawable = draw;
        gc->currentReadable = read;
    } else {
        if (oldGC != &dummyContext) {
            /* Old current context is no longer current to anybody */
            oldGC->currentDpy = 0;
            oldGC->currentDrawable = None;
            oldGC->currentReadable = None;
            oldGC->currentContextTag = 0;

            if (oldGC->xid == None) {
                __GLXdisplayPrivate *priv;
                /*
                ** We are switching away from a context that was
                ** previously destroyed, so we need to free the memory
                ** for the old handle.
                */

                /* Destroy the old direct rendering context */
                if (oldGC->isDirect) {
                    if (oldGC->driContext.private) {
                        (*oldGC->driContext.destroyContext)
                            (dpy, oldGC->screen, oldGC->driContext.private);
                        oldGC->driContext.private = NULL;
                    }
                }
                priv = __glXInitialize(dpy);
                __glXFreeContext(priv, oldGC);
            }
        }
        if (gc) {
            __glXSetCurrentContext(gc);
            if (!gc->isDirect) {
                /* For indirect mode, set table as empty function */
                _glapi_set_context((GLvoid *)__glxNopContext);
            }

            gc->currentDpy = dpy;
            gc->currentDrawable = draw;
            gc->currentReadable = read;

            if (gc->isDirect) reply.contextTag = -1;
            gc->currentContextTag = reply.contextTag;
        } else {
            __glXSetCurrentContext(&dummyContext);
            _glapi_set_context((GLvoid *)__glxNopContext);

        }
    }

MakeContextCurrent_exit:

    __glXMutexUnlock();

    return status;
}

Bool GLX_PREFIX(glXMakeCurrent)(Display *dpy, GLXDrawable draw, GLXContext gc)
{
    return MakeContextCurrent( dpy, draw, draw, gc );
}

Bool GLX_PREFIX(glXMakeContextCurrent)(Display *dpy, GLXDrawable d, GLXDrawable r, GLXContext ctx)
{
    return MakeContextCurrent( dpy, d, r, ctx );
}

/*
** GLX_SGI_make_current_read
*/
Bool GLX_PREFIX(glXMakeCurrentReadSGI)(Display *dpy, GLXDrawable d, GLXDrawable r, GLXContext ctx)
{
    return MakeContextCurrent( dpy, d, r, ctx );
}


#ifdef DEBUG
GLvoid __glXDumpDrawBuffer(__GLXcontext *ctx)
{
    GLubyte *p = ctx->buf;
    GLubyte *end = ctx->pc;
    GLushort opcode, length;

    while (p < end) {
        /* Fetch opcode */
        opcode = *((GLushort*) p);
        length = *((GLushort*) (p + 2));
        printf("%2x: %5d: ", opcode, length);
        length -= 4;
        p += 4;
        while (length > 0) {
            printf("%08x ", *((unsigned *) p));
            p += 4;
            length -= 4;
        }
        printf("\n");
    }
}
#endif
