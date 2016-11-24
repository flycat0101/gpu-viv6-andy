/*
 * Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
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
 * THE COPYRIGHT HOLDERS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file dri_interface.h
 *
 * This file contains all the types and functions that define the interface
 * between a DRI driver and driver loader.  Currently, the most common driver
 * loader is the XFree86 libGL.so.  However, other loaders do exist, and in
 * the future the server-side libglx.a will also be a loader.
 *
 * \author Kevin E. Martin <kevin@precisioninsight.com>
 * \author Ian Romanick <idr@us.ibm.com>
 */

#ifndef DRI_INTERFACE_H
#define DRI_INTERFACE_H

#include "drm.h"


/*
** The following structures define the interface between the GLX client
** side library and the DRI (direct rendering infrastructure).
*/
typedef struct __DRIdisplayRec  __DRIdisplay;
typedef struct __DRIscreenRec   __DRIscreen;
typedef struct __DRIcontextRec  __DRIcontext;
typedef struct __DRIdrawableRec __DRIdrawable;
typedef struct __DRIdriverRec   __DRIdriver;
typedef struct __DRIframebufferRec __DRIframebuffer;
typedef struct __DRIversionRec     __DRIversion;
typedef unsigned long __DRIid;
typedef GLvoid __DRInativeDisplay;


typedef GLvoid *(*CreateScreenFunc)(__DRInativeDisplay *dpy, int scrn, __DRIscreen *psc,
                                  int numConfigs, GLvoid *config);

typedef GLvoid *(*CreateNewScreenFunc)(__DRInativeDisplay *dpy, int scrn, __DRIscreen *psc,
    const __GLcontextModes * modes, const __DRIversion * ddx_version,
    const __DRIversion * dri_version, const __DRIversion * drm_version,
    const __DRIframebuffer * frame_buffer, GLvoid * pSAREA,
    int fd, int internal_api_version, __GLcontextModes ** driver_modes);


/**
 * Stored version of some component (i.e., server-side DRI module, kernel-side
 * DRM, etc.).
 */
struct __DRIversionRec {
    int    major;        /**< Major version number. */
    int    minor;        /**< Minor version number. */
    int    patch;        /**< Patch-level. */
};

/**
 * Framebuffer information record.  Used by libGL to communicate information
 * about the framebuffer to the driver's \c __driCreateNewScreen function.
 *
 * In XFree86, most of this information is derrived from data returned by
 * calling \c XF86DRIGetDeviceInfo.
 */
struct __DRIframebufferRec {
    unsigned char *base;    /**< Framebuffer base address in the CPU's
                             * address space.  This value is calculated by
                             * calling \c drmMap on the framebuffer handle
                             * returned by \c XF86DRIGetDeviceInfo (or a
                             * similar function).
                             */
    int size;               /**< Framebuffer size, in bytes. */
    int stride;             /**< Number of bytes from one line to the next. */
    int width;              /**< Pixel width of the framebuffer. */
    int height;             /**< Pixel height of the framebuffer. */
    int dev_priv_size;      /**< Size of the driver's dev-priv structure. */
    GLvoid *dev_priv;         /**< Pointer to the driver's dev-priv structure. */
};


/*
** We keep a linked list of these structures, one per DRI device driver.
*/
struct __DRIdriverRec {
   const char *name;
   GLvoid *handle;
   CreateScreenFunc createScreenFunc;
   CreateNewScreenFunc createNewScreenFunc;
   struct __DRIdriverRec *next;
    int refCount;
};


/*
** Display dependent methods.  This structure is initialized during the
** driCreateDisplay() call.
*/
struct __DRIdisplayRec {
    /*
    ** Method to destroy the private DRI display data.
    */
    GLvoid (*destroyDisplay)(__DRInativeDisplay *dpy, GLvoid *displayPrivate);

    /**
     * Methods to create the private DRI screen data and initialize the
     * screen dependent methods.
     * This is an array [indexed by screen number] of function pointers.
     *
     * \deprecated  This array of function pointers has been replaced by
     *              \c __DRIdisplayRec::createNewScreen.
     * \sa __DRIdisplayRec::createNewScreen
     */
    CreateScreenFunc * createScreen;

    /**
     * Opaque pointer to private per display direct rendering data.
     * \c NULL if direct rendering is not supported on this display.
     */
    struct __DRIdisplayPrivateRec *private;

    /**
     * Array of pointers to methods to create and initialize the private DRI
     * screen data.
     *
     * \sa __DRIdisplayRec::createScreen
     */
    CreateNewScreenFunc * createNewScreen;
};

/*
** Screen dependent methods.  This structure is initialized during the
** (*createScreen)() call.
*/
struct __DRIscreenRec {
    /*
    ** Method to destroy the private DRI screen data.
    */
    GLvoid (*destroyScreen)(__DRInativeDisplay *dpy, int scrn, GLvoid *screenPrivate);

    /**
     * Method to create the private DRI context data and initialize the
     * context dependent methods.
     */
    GLvoid *(*createNewContext)(__DRInativeDisplay *dpy, const __GLcontextModes *modes,
                    int render_type,
                    GLvoid *sharedPrivate, __DRIcontext *pctx);

    /**
     * Method to copy GL states of a DRI context to another DRI context.
     */
    GLvoid *(*copyContext)(__DRInativeDisplay *dpy,
                    __DRIcontext *src, __DRIcontext *dst, GLuint mask);

    /**
     * Method to create the private DRI drawable data and initialize the
     * drawable dependent methods.
     */
    GLvoid *(*createNewDrawable)(__DRInativeDisplay *dpy, const __GLcontextModes *modes,
                    __DRIid draw, __DRIdrawable *pdraw,
                    int renderType, const int *attrs);

    /*
    ** Method to return a pointer to the DRI drawable data.
    */
    __DRIdrawable *(*getDrawable)(__DRInativeDisplay *dpy, __DRIid draw,
                    GLvoid *drawablePrivate);

    /*
    ** Opaque pointer to private per screen direct rendering data.  NULL
    ** if direct rendering is not supported on this screen.  Never
    ** dereferenced in libGL.
    */
    GLvoid *private;

    /**
     * Opaque pointer that points back to the containing
     * \c __GLXscreenConfigs.  This data structure is shared with DRI drivers
     * but \c __GLXscreenConfigs is not. However, they are needed by some GLX
     * functions called by DRI drivers.
     */
    GLvoid *screenConfigs;

};

/*
** Context dependent methods.  This structure is initialized during the
** (*createContext)() call.
*/
struct __DRIcontextRec {
    /*
    ** Method to destroy the private DRI context data.
    */
    GLvoid (*destroyContext)(__DRInativeDisplay *dpy, int scrn, GLvoid *contextPrivate);

    /*
    ** Method to bind a DRI drawable and readable to a DRI graphics context.
    */
    GLboolean (*bindContext)(__DRInativeDisplay *dpy, int scrn, __DRIid draw,
                __DRIid read, __DRIcontext *ctx);

    /*
    ** Method to unbind a DRI drawable to a DRI graphics context.
    */
    GLboolean (*unbindContext)(__DRInativeDisplay *dpy, int scrn, __DRIid draw,
               __DRIid read, __DRIcontext *ctx);

    /*
    ** Opaque pointer to private per context direct rendering data.
    ** NULL if direct rendering is not supported on the display or
    ** screen used to create this context.  Never dereferenced in libGL.
    */
    GLvoid *private;

    /**
     * Pointer to the mode used to create this context.
     */
    const __GLcontextModes * mode;
};

/*
** Drawable dependent methods.  This structure is initialized during the
** (*createDrawable)() call.  createDrawable() is not called by libGL at
** this time.  It's currently used via the dri_util.c utility code instead.
*/
struct __DRIdrawableRec {
    /*
    ** Method to destroy the private DRI drawable data.
    */
    GLvoid (*destroyDrawable)(__DRInativeDisplay *dpy, GLvoid *drawablePrivate);

    /*
    ** Method to swap the front and back buffers.
    */
    GLvoid (*swapBuffers)(__DRInativeDisplay *dpy, GLvoid *drawablePrivate);

    /*
    ** Opaque pointer to private per drawable direct rendering data.
    ** NULL if direct rendering is not supported on the display or
    ** screen used to create this drawable.  Never dereferenced in libGL.
    */
    GLvoid *private;
};

#endif
