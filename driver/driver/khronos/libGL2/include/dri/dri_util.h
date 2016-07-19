/* $XFree86: xc/lib/GL/dri/dri_util.h,v 1.1 2002/02/22 21:32:52 dawes Exp $ */
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
 *   Brian Paul <brian@precisioninsight.com>
 */

#ifndef _DRI_UTIL_H_
#define _DRI_UTIL_H_


#define CAPI  /* XXX this should be globally defined somewhere */

#include <stdarg.h>
#include <drm.h>
#include <drm_sarea.h>
#include "drmgl.h"
#include "dri_interface.h"
#include "gl/gl_core.h"

#define DRM_VVT_PROCESS_EXIT  1

typedef struct __DRIdisplayPrivateRec  __DRIdisplayPrivate;
typedef struct __DRIscreenPrivateRec   __DRIscreenPrivate;
typedef struct __DRIcontextPrivateRec  __DRIcontextPrivate;
typedef struct __DRIdrawablePrivateRec __DRIdrawablePrivate;

typedef struct _drm_vvt_sarea {
    int ctx_owner;
} drm_vvt_sarea_t;

typedef struct _ScreenConfigRec {
    int virtualX;
    int virtualY;
} screenConfig;

typedef struct _vvtDeviceInfoRec {
    int bufBpp;
    int zbufBpp;
    int backOffset;
    int depthOffset;
    int sareaPrivOffset;
    screenConfig ScrnConf;
} vvtDeviceInfo;

#define DRI_VALIDATE_DRAWABLE_INFO_ONCE(pDrawPriv)              \
    do {                                                        \
        if (*(pDrawPriv->pStamp) != pDrawPriv->lastStamp) {     \
            __driUtilUpdateDrawableInfo(pDrawPriv);             \
        }                                                       \
    } while (0)


#define DRI_VALIDATE_EXTRADRAWABLE_INFO_ONCE(pDrawPriv)              \
    do {                                                        \
        if (*(pDrawPriv->pStamp) != pDrawPriv->lastStamp) {     \
            __driUtilUpdateExtraDrawableInfo(pDrawPriv);             \
        }                                                       \
    } while (0)



#define DRI_VALIDATE_DRAWABLE_INFO(psp, pdp)                            \
 do {                                                                   \
    while (*(pdp->pStamp) != pdp->lastStamp) {                          \
        DRMGL_UNLOCK(psp->fd, &psp->pSAREA->lock,                         \
               pdp->driContextPriv->hHWContext);                        \
        DRMGL_SPINLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);     \
        DRI_VALIDATE_EXTRADRAWABLE_INFO_ONCE(pdp);                           \
        DRMGL_SPINUNLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);   \
        DRMGL_LIGHT_LOCK(psp->fd, &psp->pSAREA->lock,                     \
              pdp->driContextPriv->hHWContext);                         \
    }                                                                   \
} while (0)

#define DRI_FULLSCREENINFO(psp, pdp)     \
 do {                                                                   \
        DRMGL_UNLOCK(psp->fd, &psp->pSAREA->lock,                         \
               pdp->driContextPriv->hHWContext);                        \
        DRMGL_SPINLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);     \
        __driUtilFullScreenCovered(pdp);                        \
        DRMGL_SPINUNLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);   \
        DRMGL_LIGHT_LOCK(psp->fd, &psp->pSAREA->lock,                     \
              pdp->driContextPriv->hHWContext);                         \
} while (0)


/**
 * Driver callback functions.
 *
 * Each DRI driver must have one of these structures with all the pointers set
 * to appropriate functions within the driver.
 *
 * When glXCreateContext() is called, for example, it'll call a helper function
 * dri_util.c which in turn will jump through the \a CreateContext pointer in
 * this structure.
 */
struct __DriverAPIRec {
    /**
     * Driver initialization callback
     */
    GLboolean (*InitDriver)(__DRIscreenPrivate *driScrnPriv);

    /**
     * Screen destruction callback
     */
    GLvoid (*DestroyScreen)(__DRIscreenPrivate *driScrnPriv);

    /**
     * Context creation callback
     */
    GLboolean (*CreateContext)(const __GLcontextModes *glVis,
                               __DRIcontextPrivate *driContextPriv,
                               GLvoid *sharedContextPrivate);

    /**
     * Context destruction callback
     */
    GLvoid (*DestroyContext)(__DRIcontextPrivate *driContextPriv);

    /**
     * Buffer (drawable) creation callback
     */
    GLboolean (*CreateBuffer)(__DRIscreenPrivate *driScrnPriv,
                              __DRIdrawablePrivate *driDrawPriv,
                              const __GLcontextModes *glVis,
                              GLboolean pixmapBuffer);

    /**
     * Buffer (drawable) destruction callback
     */
    GLvoid (*DestroyBuffer)(__DRIdrawablePrivate *driDrawPriv);

    /**
     * Buffer swapping callback
     */
    GLvoid (*SwapBuffers)(__DRIdrawablePrivate *driDrawPriv);

    /**
     * Context activation callback
     */
    GLboolean (*MakeCurrent)(__DRIcontextPrivate *driContextPriv,
                             __DRIdrawablePrivate *driDrawPriv,
                             __DRIdrawablePrivate *driReadPriv);

    /**
     * Context unbinding callback
     */
    GLboolean (*UnbindContext)(__DRIcontextPrivate *driContextPriv);

    /**
     * Copy Context callback
     */
    GLvoid (*CopyContext)(__DRIcontextPrivate *srcPriv,
                        __DRIcontextPrivate *dstPriv,
                        GLuint mask);
};

/**
 * Per-drawable private DRI driver information.
 */
struct __DRIdrawablePrivateRec {
    /**
     * Kernel drawable handle
     */
    drm_drawable_t hHWDrawable;

    /**
     * Driver's private drawable information.
     *
     * This structure is opaque.
     */
    GLvoid *driverPrivate;

    /**
     * X's drawable ID associated with this private drawable.
     */
    __DRIid draw;
    __DRIdrawable *pdraw;

    /**
     * Reference count for number of context's currently bound to this
     * drawable.
     *
     * Once it reaches zero, the drawable can be destroyed.
     *
     * \note This behavior will change with GLX 1.3.
     */
    int refcount;

    /**
     * Index of this drawable information in the SAREA.
     */
    GLuint index;

    /**
     * Pointer to the "drawable has changed ID" stamp in the SAREA.
     */
    GLuint *pStamp;

    /**
     * Last value of the stamp.
     *
     * If this differs from the value stored at __DRIdrawablePrivate::pStamp,
     * then the drawable information has been modified by the X server, and the
     * drawable information (below) should be retrieved from the X server.
     */
    GLuint lastStamp;

    /**
     * \name Drawable
     *
     * Drawable information used in software fallbacks.
     */
    /*@{*/
    int x;
    int y;
    int w;
    int h;
    int numClipRects;
    drm_clip_rect_t *pClipRects;
    /*@}*/

    /**
     * \name Back and depthbuffer
     *
     * Information about the back and depthbuffer where different from above.
     */
    /*@{*/
    int backX;
    int backY;
    int backClipRectType;
    int numBackClipRects;
    drm_clip_rect_t *pBackClipRects;
    /*@}*/

    /**
     * Pointer to context to which this drawable is currently bound.
     */
    __DRIcontextPrivate *driContextPriv;

    /**
     * Pointer to screen on which this drawable was created.
     */
    __DRIscreenPrivate *driScreenPriv;

    /**
     * \name Display and screen information.
     *
     * Basically just need these for when the locking code needs to call
     * \c __driUtilUpdateDrawableInfo.
     */
    /*@{*/
    __DRInativeDisplay *display;
    int screen;
    /*@}*/

    /**
     * Called via glXSwapBuffers().
     */
    GLvoid (*swapBuffers)(__DRIdrawablePrivate *dPriv );

    unsigned int wWidth;
    unsigned int wHeight;
    unsigned int xWOrigin;
    unsigned int yWOrigin;
    unsigned int nodeName;
    unsigned int backNode;
    unsigned int phyAddress;
    int fullscreenCovered;

#ifdef DRI_PIXMAPRENDER_GL
    GLvoid *wrapPixData;
    GLvoid *wrapSurface;
    GLvoid ( *doCPYToSCR)(__DRIdrawablePrivate *dPriv );
#endif
};


struct __DRIcontextPrivateRec {
    /*
    ** Kernel context handle used to access the device lock.
    */
    __DRIid contextID;

    /*
    ** Kernel context handle used to access the device lock.
    */
    drm_context_t hHWContext;

    /*
    ** Device driver's private context data.  This structure is opaque.
    */
    GLvoid *driverPrivate;

    /*
    ** This context's display pointer.
    */
    __DRInativeDisplay *display;

    /*
    ** Pointer to drawable currently bound to this context.
    */
    __DRIdrawablePrivate *driDrawablePriv;

    /*
    ** Pointer to screen on which this context was created.
    */
    __DRIscreenPrivate *driScreenPriv;
};

/**
 * Per-screen private driver information.
 */
struct __DRIscreenPrivateRec {
    /**
     * Display for this screen
     */
    __DRInativeDisplay *display;

    /**
     * Current screen's number
     */
    int myNum;

    /**
     * Callback functions into the hardware-specific DRI driver code.
     */
    struct __DriverAPIRec DriverAPI;

    /**
     * \name DDX version
     * DDX / 2D driver version information.
     * \todo Replace these fields with a \c __DRIversionRec.
     */
    /*@{*/
    int ddxMajor;
    int ddxMinor;
    int ddxPatch;
    /*@}*/

    /**
     * \name DRI version
     * DRI X extension version information.
     * \todo Replace these fields with a \c __DRIversionRec.
     */
    /*@{*/
    int driMajor;
    int driMinor;
    int driPatch;
    /*@}*/

    /**
     * \name DRM version
     * DRM (kernel module) version information.
     * \todo Replace these fields with a \c __DRIversionRec.
     */
    /*@{*/
    int drmMajor;
    int drmMinor;
    int drmPatch;
    /*@}*/

    /**
     * ID used when the client sets the drawable lock.
     *
     * The X server uses this value to detect if the client has died while
     * holding the drawable lock.
     */
    int drawLockID;

    /**
     * File descriptor returned when the kernel device driver is opened.
     *
     * Used to:
     *   - authenticate client to kernel
     *   - map the frame buffer, SAREA, etc.
     *   - close the kernel device driver
     */
    int fd;

    /**
     * SAREA pointer
     *
     * Used to access:
     *   - the device lock
     *   - the device-independent per-drawable and per-context(?) information
     */
    drm_sarea_t *pSAREA;

    /**
     * \name Direct frame buffer access information
     * Used for software fallbacks.
     */
    /*@{*/
    unsigned char *pFB;
    void * pLogicalAddr;
    int fbSize;
    int fbOrigin;
    int fbStride;
    int fbWidth;
    int fbHeight;
    int fbBPP;
    /*@}*/

    /**
     * \name Device-dependent private information (stored in the SAREA).
     *
     * This data is accessed by the client driver only.
     */
    /*@{*/
    GLvoid *pDevPriv;
    int devPrivSize;
    /*@}*/

    /**
     * Dummy context to which drawables are bound when not bound to any
     * other context.
     *
     * A dummy hHWContext is created for this context, and is used by the GL
     * core when a hardware lock is required but the drawable is not currently
     * bound (e.g., potentially during a SwapBuffers request).  The dummy
     * context is created when the first "real" context is created on this
     * screen.
     */
    __DRIcontextPrivate dummyContextPriv;

    /**
     * Hash table to hold the drawable information for this screen.
     */
    GLvoid *drawHash;

    /**
     * Device-dependent private information (not stored in the SAREA).
     *
     * This pointer is never touched by the DRI layer.
     */
    GLvoid *private;

    /**
     * GLX visuals / FBConfigs for this screen.  These are stored as a
     * linked list.
     *
     * \note
     * This field is \b only used in conjunction with the old interfaces.  If
     * the new interfaces are used, this field will be set to \c NULL and will
     * not be dereferenced.
     */
    __GLcontextModes *modes;

    /**
     * Pointer back to the \c __DRIscreen that contains this structure.
     */
    __DRIscreen *psc;

    /*
    ** The global Nop API dispatch pointer "__glNopDispatchTab" address so that
    ** glcore can initialize this global pointer with Nop dispatch tables.
    */
    __GLdispatchTable **nopDispatchPtrAddr;
};

struct __DRIdisplayPrivateRec {
    /*
    ** XFree86-DRI version information
    */
    int driMajor;
    int driMinor;
    int driPatch;

    /*
    ** Array of library handles [indexed by screen number]
    */
    GLvoid **libraryHandles;
};


extern GLvoid
__driUtilMessage(const char *f, ...);

extern GLvoid
__driUtilUpdateDrawableInfo(__DRIdrawablePrivate *pdp);

extern GLvoid
__driUtilUpdateExtraDrawableInfo(__DRIdrawablePrivate *pdp);

extern GLboolean
__driUtilFullScreenCovered(__DRIdrawablePrivate *pdp);

extern __DRIscreenPrivate * __driUtilCreateNewScreen( __DRInativeDisplay *dpy,
    int scrn, __DRIscreen *psc, __GLcontextModes * modes,
    const __DRIversion * ddx_version, const __DRIversion * dri_version,
    const __DRIversion * drm_version, const __DRIframebuffer * frame_buffer,
    GLvoid *pSAREA, int fd, int internal_api_version,
    const struct __DriverAPIRec *driverAPI );


#endif /* _DRI_UTIL_H_ */
