/**************************************************************************

Copyright 2000, 2001 VA Linux Systems Inc., Fremont, California.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial
portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT OWNER(S) AND/OR ITS SUPPLIERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

#ifndef __VIV_LOCK_H__
#define __VIV_LOCK_H__

#if defined(_LINUX_) && defined(DRI_PIXMAPRENDER_GL)

#include "dri_util.h"
extern GLvoid vivGetLock( __GLcontext *gc, GLuint flags );

typedef struct vvtDriMirrorRec {
    __DRIcontextPrivate     *context;   /* DRI context */
    __DRIscreenPrivate      *screen;    /* DRI screen */
    __DRIdrawablePrivate    *drawable;  /* DRI drawable bound to this ctx */

    drm_context_t hwContext;
    drm_hw_lock_t *hwLock;
    int fd;
    int drmMinor;
    int lockCnt;
    GLboolean bDrawableInitied;

    /*drm_vvt_sarea_t *vvtSarea; */
    drm_vvt_sarea_t *vvtSarea;
} vivDriMirror;

/* Turn DEBUG_LOCKING on to find locking conflicts.
 */
#if DEBUG_LOCKING
extern GLbyte *prevLockFile;
extern int prevLockLine;

#define DEBUG_LOCK()                            \
    do {                                        \
        prevLockFile = (__FILE__);              \
        prevLockLine = (__LINE__);              \
    } while (0)

#define DEBUG_RESET()                           \
    do {                                        \
        prevLockFile = 0;                       \
        prevLockLine = 0;                       \
    } while (0)

#define DEBUG_CHECK_LOCK()                      \
    do {                                        \
        if ( prevLockFile ) {                   \
            fprintf( stderr,                    \
                "LOCK SET!\n\tPrevious %s:%d\n\tCurrent: %s:%d\n",  \
                prevLockFile, prevLockLine, __FILE__, __LINE__ );   \
            GL_ASSERT(0);\
            exit( 1 );                          \
        }                                       \
    } while (0)

#else

#define DEBUG_LOCK()
#define DEBUG_RESET()
#define DEBUG_CHECK_LOCK()

#endif

/*
 * !!! We may want to separate locks from locks with validation.  This
 * could be used to improve performance for those things commands that
 * do not do any drawing !!!
 */
extern _glthread_Mutex __glDrmMutex;

/* Lock the hardware and validate drawable state.
 * LOCK_FRAMEBUFFER( gc ) and UNLOCK_FRAMEBUFFER( gc ) should be called whenever
 * OpenGL driver needs to access framebuffer hardware.
 */
#define LINUX_LOCK_FRAMEBUFFER( gc )                            \
    if (!gc->imports.fromEGL) {                                 \
        GLbyte __ret = 0;                                       \
        vivDriMirror *dri = (vivDriMirror *)gc->imports.other;  \
        DEBUG_CHECK_LOCK();                                     \
        _glthread_LOCK_MUTEX(__glDrmMutex);                     \
        if (dri->screen->dri3)                                  \
        {                                                       \
            vivGetLock( gc, 0 );                                \
        }                                                       \
        else                                                    \
        {                                                       \
          if (dri->lockCnt++ == 0) {                            \
            if (!dri->bDrawableInitied) {                       \
                vivGetLock( gc, 0 );                            \
                dri->bDrawableInitied = 1;                      \
            } else {                                            \
                DRMGL_CAS( dri->hwLock, dri->hwContext,         \
                   (DRM_LOCK_HELD | dri->hwContext), __ret );   \
                if ( __ret)                                     \
                    vivGetLock( gc, 0 );                        \
            }                                                   \
          }                                                     \
        }                                                       \
        DEBUG_LOCK();                                           \
    }

#define LINUX_UNLOCK_FRAMEBUFFER( gc )                          \
    if (!gc->imports.fromEGL) {                                 \
        vivDriMirror *dri = (vivDriMirror *)gc->imports.other;  \
        if (!dri->screen->dri3)                                 \
        {                                                       \
          if (--dri->lockCnt == 0) {                            \
            DRMGL_UNLOCK( dri->fd,                              \
              dri->hwLock,                                      \
              dri->hwContext );                                 \
          }                                                     \
        }                                                       \
        _glthread_UNLOCK_MUTEX(__glDrmMutex);                   \
        DEBUG_RESET();                                          \
    }

#define LINUX_LOCK_FRAMEBUFFER_DUMMY( sPriv ) \
    do { \
        if ( sPriv->dri3) break;    \
        if ( sPriv->dummyContextPriv.driScreenPriv && sPriv->dummyContextPriv.hHWContext) { \
            DRMGL_LOCK(sPriv->fd, &sPriv->pSAREA->lock, sPriv->dummyContextPriv.hHWContext, 0); \
            DEBUG_LOCK(); \
        } \
    }while(0)

#define LINUX_UNLOCK_FRAMEBUFFER_DUMMY( sPriv ) \
    do { \
        if ( sPriv->dri3) break;    \
        if ( sPriv->dummyContextPriv.driScreenPriv && sPriv->dummyContextPriv.hHWContext) { \
            DRMGL_UNLOCK(sPriv->fd, &sPriv->pSAREA->lock, sPriv->dummyContextPriv.hHWContext); \
            DEBUG_RESET(); \
        } \
    }while(0)
#else /* _LINUX_ */

#define LINUX_LOCK_FRAMEBUFFER( gc )
#define LINUX_UNLOCK_FRAMEBUFFER( gc )
#define LINUX_LOCK_FRAMEBUFFER_DUMMY( sPriv )
#define LINUX_UNLOCK_FRAMEBUFFER_DUMMY( sPriv )

#endif /* _LINUX_ */

#endif  /* __VIV_LOCK_H__ */

