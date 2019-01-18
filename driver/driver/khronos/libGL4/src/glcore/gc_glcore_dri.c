/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#if defined(_LINUX_) && defined(DRI_PIXMAPRENDER_GL)

#include <stdio.h>
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xext.h>
#include <X11/extensions/Xdamage.h>
#include "gc_es_context.h"
#include "gc_es_device.h"
#include "g_asmoff.h"
//#include "../GLX/client/glxclient.h"
#include "GL/glxtokens.h"
#include "viv_lock.h"
#include <sys/types.h>
#include <unistd.h>
#if DEBUG_LOCKING
GLbyte *prevLockFile = NULL;
GLint prevLockLine = 0;
#endif

extern veglDISPATCH GL_DISPATCH_TABLE;
extern GLuint __glCopyContext(__GLcontext *dst, __GLcontext *src, GLuint mask);
GLboolean __glLoseCurrent(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable);
extern GLboolean __glMakeCurrent(__GLcontext *gc, __GLdrawablePrivate* drawable, __GLdrawablePrivate* readable, GLboolean flushDrawableChange);
extern GLvoid __glAssociateContext(__GLcontext *gc, __GLdrawablePrivate *drawable, __GLdrawablePrivate *readable);
extern GLuint __glDestroyContext(__GLcontext *gc);
extern GLuint __glShareContext(__GLcontext *gc, __GLcontext *gcShare);
extern GLvoid __glNotifyDrawableChange(__GLcontext *gc, GLuint mask);
extern __GLformatInfo __glFormatInfoTable[];
extern __GLdispatchTable __glNopFuncTable;
/*
*Later to add this if neccessary
extern __GLdispatchState __glNopDispatchFuncTable;
*/

extern GLboolean __glDpInitialize( __GLdeviceStruct *deviceEntry);

/* Mutex that guarantees only one thread access LINUX_LOCK_FRAMEBUFFER at a time.
*/
_glthread_Mutex __glDrmMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

/*
** The __glDevicePipeEntry[] stores an array of init entries for all
** framebuffer devices in one system. It can be dynamically initalized
** according to "chipID" by loading different DLLs for different devices.
*/
__GLdeviceStruct __glDevicePipeEntry[__GL_MAX_DEVICE_NUMBER];
__GLdeviceStruct *__glDevice = &__glDevicePipeEntry[0];


__GLthreadHashTable threadHashTable[__GL_MAXIMUM_THREAD_NUMBER];
GLboolean thrHashTabInit = GL_FALSE;

__GLcontext __glNopContext;

/*****************************************************************/
/* Imports Stuff (functions imported by the core rendering code) */

GLvoid
__vivError(const GLbyte *str)
{
    fprintf(stderr, "OGL ERR:%s\n", str);
}

GLvoid *
__vivImpMalloc(__GLcontext *gc, size_t size)
{
    return (GLvoid *)malloc(size);
}

GLvoid *
__vivImpCalloc(__GLcontext *gc, size_t numElements, size_t elementSize)
{
    return (GLvoid *)calloc(numElements, elementSize);
}

GLvoid *
__vivImpRealloc(__GLcontext *gc, GLvoid *oldPtr, size_t newSize)
{
    return (GLvoid *)realloc(oldPtr, newSize);
}

GLvoid
__vivImpFree(__GLcontext *gc, GLvoid *ptr)
{
    free(ptr);
}

GLvoid *
__vivMalloc(size_t size)
{
    return (GLvoid *)malloc(size);
}

GLvoid *
__vivCalloc(size_t numElements, size_t elementSize)
{
    return (GLvoid *)calloc(numElements, elementSize);
}

GLvoid *
__vivRealloc(GLvoid *oldPtr, size_t newSize)
{
    return (GLvoid *)realloc(oldPtr, newSize);
}

GLvoid
__vivFree(GLvoid *ptr)
{
    free(ptr);
}

GLvoid
__vivImpWarning(__GLcontext *gc, const GLbyte* msg)
{
    __vivError((GLbyte *) msg);
}

GLvoid
__vivImpFatal(__GLcontext *gc, const GLbyte* msg)
{
    __vivError((GLbyte *) msg);
    abort();
}

GLuint
__vivImpGetMemoryStatus(__GLmemoryStatus option)
{
    switch(option)
    {
        case __GL_MMUSAGE_PHYS_TOTAL:
            return 0x200000;
            break;
        case __GL_MMUSAGE_PHYS_AVAIL:
            return 0x200000;
            break;
        case __GL_MMUSAGE_VIRTUAL_TOTAL:
            return 0x200000;
            break;
        case __GL_MMUSAGE_VIRTUAL_AVAIL:
            return 0x200000;
            break;
    }
    return 0;
}

GLvoid *__vivImpGetHWND(__GLcontext *gc)
{
    return NULL;
}

GLvoid __vivImpNoop()
{
}

GLvoid __vivBltImageToScreen(__GLcontext *gc,
      GLint bitsAlignedWidth,
      GLint bitsAlignedHeight,
      GLint bitsPerPixel,
      GLvoid * bits,
      GLint left,
      GLint top,
      GLint width,
      GLint height)
{
    vivDriMirror *pDriMirror;
    __DRIdrawablePrivate *driDrawPriv;
    Display *dpy;
    GLint     status;
    GC xGc;
    Drawable d;
    XImage* xImage = NULL;
    Screen* xScreen;
    Visual* xVisual;
    GLuint xDepth;


    pDriMirror = (vivDriMirror *)gc->imports.other;
    driDrawPriv = pDriMirror->drawable;
    dpy = driDrawPriv->display;
    d = driDrawPriv->draw;

    xGc = XCreateGC(dpy, d, 0, NULL);

    /* Fetch defaults. */
    xScreen = DefaultScreenOfDisplay(dpy);
    xVisual = DefaultVisualOfScreen(xScreen);
    xDepth  = DefaultDepthOfScreen(xScreen);

    /* Create image from the bits. */
    xImage = XCreateImage(
        dpy,
        xVisual,
        xDepth,
        ZPixmap,
        0,
        (char*) bits,
        bitsAlignedWidth,
        bitsAlignedHeight,
        8,
        0
        );

    if (xImage) {
        /* Draw the image. */
        status = XPutImage(
            dpy,
            d,
            xGc,
            xImage,
            left, top,      /* Source origin. */
            left, top,      /* Destination origin. */
            width, height   /* Image size. */
            );

        if (status == Success)
        {
            /* Flush buffers. */
            status = XFlush(dpy);
        }

        /* Destroy the image. */
        if (xImage != NULL)
        {
           xImage->data = NULL;
           XDestroyImage(xImage);
        }


        /* Free graphics context. */
        XFreeGC(dpy, xGc);
    }
}

static GLvoid
__vivImpInternalSwapBuffers(__GLcontext *gc,  gctBOOL bSwapFront, gctBOOL finishMode)
{
    vivDriMirror *pDriMirror;
    __DRIdrawablePrivate *driDrawPriv;
    __GLdrawablePrivate *glPriv;
    XserverRegion region;
    XRectangle damagedRect[1];

    pDriMirror = (vivDriMirror *)gc->imports.other;
    driDrawPriv = pDriMirror->drawable;

    glPriv = driDrawPriv->driverPrivate;

    if ( driDrawPriv->pixtag && !finishMode ) return ;

    glPriv = driDrawPriv->driverPrivate;


     //LINUX_UNLOCK_FRAMEBUFFER(gc);


    /* Call DP SwapBuffers */
    (*glPriv->dp.swapBuffers)(gc, glPriv, GL_TRUE);

    /* Tell X server some area is damaged so that X can flip new flame onto on-screen,
    ** For Aps in which glxswapbuffers is applied, the next lines can be ignored cause glxswapbuffer will do the same thing.
    ** For Aps in which glxswapbuffers is not applied and glflush or glend is applied, notify x server of what happens and
    ** X server can flip new flame onto on-screen otherwise the flame can't perhaps display.
    */
    if ( (glPriv->fullScreenMode && driDrawPriv->fullscreenCovered) || (!glPriv->fullScreenMode) ) {
        damagedRect[0].x = 0;
        damagedRect[0].y = 0;
        damagedRect[0].width = driDrawPriv->w;
        damagedRect[0].height = driDrawPriv->h;
        region = XFixesCreateRegion (driDrawPriv->display, &damagedRect[0], 1);
        XDamageAdd (driDrawPriv->display, driDrawPriv->draw, region);
        XFixesDestroyRegion(driDrawPriv->display, region);

        /* If the size doesn't change, don't flush */
        if (gc->changeMask & __GL_DRAWABLE_PENDING_RESIZE)
        XFlush(driDrawPriv->display);

    }


     //LINUX_LOCK_FRAMEBUFFER(gc);

}


static void __vivsyncNative(void )
{

}
VEGLimports imports = {
    gcvNULL,
    gcvNULL,
    __vivsyncNative,
    gcvNULL,
    gcvNULL,
    __vivImpMalloc,
    __vivImpCalloc,
    __vivImpRealloc,
    __vivImpFree,
    __vivImpNoop,
    __vivImpNoop,
    __vivImpNoop,
    __vivImpNoop,
    gcvNULL,                 /* config */
    gcvFALSE,                /* robustAccess */
    0,                       /* resetNotification */
    gcvFALSE,                /* debuggable */
    0,
    gcvFALSE,
#if gcdGC355_PROFILER
    0,
    gcvNULL,
#endif
    __vivImpGetMemoryStatus,
    __vivImpWarning,
    __vivImpFatal,
    __vivImpNoop,
    __vivImpNoop,
    __vivImpNoop,
    __vivImpGetHWND,
    __vivImpNoop,
    __vivImpNoop,
    __vivImpInternalSwapBuffers,
    __vivImpNoop,
    __vivImpNoop,
    __vivImpNoop,
    __vivBltImageToScreen,
    gcvFALSE,
    0x14,
    &__glDevicePipeEntry[0],/*device*/
    0,/*deviceIndex*/
    gcvFALSE,
    gcvNULL,/*other*/
};


/************************************************************************/
/* Code for debugging glcore only */


/************************************************************************/

/* Update the hardware state.  This is called if another context has
 * grabbed the hardware lock, which includes the X server.  This
 * function also updates the driver's window state after the X server
 * moves, resizes or restacks a window -- the change will be reflected
 * in the drawable position and clip rects.  Since the X server grabs
 * the hardware lock when it changes the window state, this routine will
 * automatically be called after such a change.
 */
GLvoid vivGetLock( __GLcontext *gc, GLuint flags )
{
    vivDriMirror *pDriMirror = (vivDriMirror *)gc->imports.other;
    __DRIdrawablePrivate *dPriv = pDriMirror->drawable;
    __DRIscreenPrivate *sPriv = pDriMirror->screen;
    drm_vvt_sarea_t *sarea = pDriMirror->vvtSarea;
    __GLdrawablePrivate *glPriv;
    drm_clip_rect_t *drmClipRects;
    RECT *pClipRects;
    GLuint changeMask = 0;
    GLint x, y, w, h, i;
    vvtDeviceInfo *pDeviceInfo = sPriv->pDevPriv;

    if (sPriv->dri3 == GL_FALSE)
    {
        drmGetLock( pDriMirror->fd, pDriMirror->hwContext, flags );

        if (sarea->ctx_owner != pDriMirror->hwContext) {
            sarea->ctx_owner = pDriMirror->hwContext;
        }
    }

    if (dPriv) {
        glPriv = (__GLdrawablePrivate *)dPriv->driverPrivate;
    }
    else {
        return;
    }

    /* The window might have moved, so we might need to get new clip
    * rects.
    *
    * NOTE: This releases and regrabs the hw lock to allow the X server
    * to respond to the DRI protocol request for new drawable info.
    * Since the hardware state depends on having the latest drawable
    * clip rects, all state checking must be done _after_ this call.
    */
    if (sPriv->dri3)
    {
        DRI3_VALIDATE_DRAWABLE_INFO( sPriv, dPriv );
    }
    else
    {
        DRI_VALIDATE_DRAWABLE_INFO( sPriv, dPriv );
    }

    x = dPriv->x;
    y = dPriv->y;

    /* Overwrite some drawable info if returned drawable info from Xserver is not valid */
    if (dPriv->w < 0 || dPriv->h < 0) {
        dPriv->w = 0;
        dPriv->h = 0;
        dPriv->numClipRects = 0;
    }
    w = dPriv->w;
    h = dPriv->h;

    if ((x != glPriv->xOrigin) || (y != glPriv->yOrigin)) {
        glPriv->xOrigin = x;
        glPriv->yOrigin = y;
        changeMask |= __GL_DRAWABLE_PENDING_MOVE;
    }
    if ((w != glPriv->width) || (h != glPriv->height)) {
        glPriv->width = w;
        glPriv->height = h;
        changeMask |= __GL_DRAWABLE_PENDING_RESIZE;
    }

    if (sPriv->dri3)
    {
        glPriv->wWidth = dPriv->wWidth;
        glPriv->wHeight = dPriv->wHeight;
        glPriv->xWOrigin = dPriv->xWOrigin;
        glPriv->yWOrigin = dPriv->yWOrigin;
        goto DRI3_NEXT;
    }

    /* Check the number of clipRects */
    if (glPriv->numClipRects != dPriv->numClipRects) {
        glPriv->numClipRects = dPriv->numClipRects;
        changeMask |= __GL_DRAWABLE_PENDING_CLIPLIST;

        /* Check the size of clipRects buffer in __GLdrawablePrivate */
        if (glPriv->numClipRects > 16) {
            (*imports.free)(0, glPriv->clipRects);
            glPriv->clipRects = (GLuint *)(*imports.malloc)(0, glPriv->numClipRects * sizeof(RECT));
        }
    }

    if( (x == 0) && (y == 0) && (w == pDeviceInfo->ScrnConf.virtualX ) && (h == pDeviceInfo->ScrnConf.virtualY)) {
           DRI_FULLSCREENINFO(sPriv, dPriv);
    }

    /* Check the size of each clipRect */
    drmClipRects = dPriv->pClipRects;
    pClipRects = (RECT *)glPriv->clipRects;
    for (i = 0; i < dPriv->numClipRects; i++, drmClipRects++, pClipRects++) {
        if (drmClipRects->x1 != pClipRects->left || drmClipRects->x2 != pClipRects->right ||
            drmClipRects->y1 != pClipRects->top || drmClipRects->y2 != pClipRects->bottom) {
            pClipRects->left = drmClipRects->x1;
            pClipRects->right = drmClipRects->x2;
            pClipRects->top = drmClipRects->y1;
            pClipRects->bottom = drmClipRects->y2;
            changeMask |= __GL_DRAWABLE_PENDING_CLIPLIST;
        }
    }

    glPriv->wWidth = dPriv->wWidth;
    glPriv->wHeight = dPriv->wHeight;
    glPriv->xWOrigin = dPriv->xWOrigin;
    glPriv->yWOrigin = dPriv->yWOrigin;

    glPriv->backBufferPhysAddr = (GLuint)dPriv->phyAddress;
    glPriv->backNode = dPriv->backNode;

DRI3_NEXT:
    /* If window size or position changed, we need to update the drawable */
    if (changeMask) {
        pDeviceInfo = sPriv->pDevPriv;
        if( (x == 0) && (y == 0) && (w == pDeviceInfo->ScrnConf.virtualX ) && (h == pDeviceInfo->ScrnConf.virtualY)) {
            glPriv->fullScreenMode = GL_TRUE;
        }
        else {
            glPriv->fullScreenMode = GL_FALSE;
        }
        /* notify the device to update the device specific drawable information */
        if (glPriv->dp.updateDrawable) {
            (*glPriv->dp.updateDrawable)(glPriv);
        }

        __glNotifyDrawableChange(gc, changeMask);
    }
}

/************************************************************************/

GLuint getThreadHashId(GLuint threadId)
{
    GLuint thrHashId;
    GLuint currentId;
    GLuint tmpId;
    GLboolean found = GL_FALSE;

    thrHashId = __GL_THREAD_HASH_ID(threadId);

    if(threadHashTable[thrHashId].threadHashId == __GL_INVALID_THREAD_HASH_ID)
    {
        threadHashTable[thrHashId].threadHashId = thrHashId;
        threadHashTable[thrHashId].threadId = threadId;
    }
    else
    {
        if(threadHashTable[thrHashId].threadId != threadId)
        {
            currentId = thrHashId++;
            tmpId = threadId + 1;
            /* Find it is in other slot */
            while((!found) && (currentId != thrHashId))
            {
                if(threadHashTable[thrHashId].threadId == threadId)
                    found = GL_TRUE;
                else
                    thrHashId = __GL_THREAD_HASH_ID(tmpId ++);
            }

            if(!found)
            {
                thrHashId = currentId + 1;
                tmpId = threadId + 1;
                /* Find an empty slot */
                while((!found) && (currentId != thrHashId))
                {
                    if(threadHashTable[thrHashId].threadHashId == __GL_INVALID_THREAD_HASH_ID)
                        found = GL_TRUE;
                    else
                        thrHashId = __GL_THREAD_HASH_ID(tmpId ++);
                }

                if(!found)
                {
                    /* Table is full */
                    GL_ASSERT(0);
                }
                else
                {
                    threadHashTable[thrHashId].threadHashId = thrHashId;
                    threadHashTable[thrHashId].threadId = threadId;
                }
            }
        }
    }

    return thrHashId;
}

GLvoid removeThreadHashIdFromHashTable(GLuint thrHashId)
{
//    (*imports.lockMutex)();
    threadHashTable[thrHashId].threadHashId = __GL_INVALID_THREAD_HASH_ID;
    threadHashTable[thrHashId].threadId = 0;
//    (*imports.unlockMutex)();
}

/************************************************************************/

/* Create the device specific screen private data struct.
 */
__GLscreenPrivate * vivCreateScreen( __DRIscreenPrivate *sPriv )
{
    __GLdeviceStruct *__glDevice = &__glDevicePipeEntry[0];
    __GLscreenPrivate *vivScreen;
    GLint  i;

    /* Allocate the private area */
    vivScreen = (__GLscreenPrivate *)(*imports.calloc)(0, 1, sizeof(*vivScreen));
    if (!vivScreen) {
        __driUtilMessage("%s: CALLOC vivScreen struct failed", __FUNCTION__);
        return NULL;
    }

    /* Point to device specific information that returned from XF86DRIGetDeviceInfo().
    */
    vivScreen->pDevInfo = sPriv->pDevPriv;

    vivScreen->baseFBLinearAddress = sPriv->pLogicalAddr;
    vivScreen->baseFBPhysicalAddress = sPriv->pFB;
    vivScreen->stride = sPriv->fbStride;
    vivScreen->width = sPriv->fbWidth;
    vivScreen->height = sPriv->fbHeight;

    /* Initialize thread hash table.
    */
    if (thrHashTabInit == GL_FALSE) {
        thrHashTabInit = GL_TRUE;
        for(i = 0; i < __GL_MAXIMUM_THREAD_NUMBER; i++) {
            threadHashTable[i].threadId = 0;
            threadHashTable[i].threadHashId = __GL_INVALID_THREAD_HASH_ID;
            threadHashTable[i].thrArea = NULL;
        }
    }

    /* Call DP specific initialization function.
    */
    (*__glDevice->devInitialize)( vivScreen );

    return vivScreen;
}

/* Destroy the device specific screen private data struct.
 */
GLvoid vivDestroyScreen( __DRIscreenPrivate *sPriv )
{
    __GLdeviceStruct *__glDevice = &__glDevicePipeEntry[sPriv->myNum];
    __GLscreenPrivate *vivScreen = (__GLscreenPrivate *)sPriv->private;
    GLint  i;

    if (vivScreen) {

        LINUX_LOCK_FRAMEBUFFER_DUMMY( sPriv );

        /* Free all thread specific resources.
        */
        if (thrHashTabInit == GL_TRUE) {
            for (i = 0; i < __GL_MAXIMUM_THREAD_NUMBER; i++) {
                if (threadHashTable[i].threadHashId != __GL_INVALID_THREAD_HASH_ID) {
                    /* Perform any device-dependent detachment */
                    /* Fix me!
                    (*__glDevice->devThreadDetach)(threadHashTable[i].threadHashId, __vivFree);
                    */
                    removeThreadHashIdFromHashTable(threadHashTable[i].threadHashId);
                }
            }
        }

        /* Call DP specific deinitialization function.
        */
        (*__glDevice->devDeinitialize)( vivScreen );

        LINUX_UNLOCK_FRAMEBUFFER_DUMMY(sPriv);

        if (vivScreen->privateData) {
            (*imports.free)(0, vivScreen->privateData);
        }

        (*imports.free)(0, vivScreen);

        sPriv->private = NULL;
    }
}

/* Initialize the driver specific screen private data.
 */
static GLboolean
vivInitDriver( __DRIscreenPrivate *sPriv )
{
    /* Initialize the global Nop API dispatch pointer "__glNopDispatchTab".
    */
    memset((void *)&__glNopContext, 0, sizeof(__GLdispatchTable));
    __glNopContext.pEntryDispatch = &__glNopFuncTable;

    *sPriv->nopContext = &__glNopContext;


    /* FIXME, initialize HAL layer */
    /* Initialize dpGlobalInfo and device entry functions*/
    if (__glDpInitialize(&__glDevicePipeEntry[sPriv->myNum]) == GL_FALSE) {
        return GL_FALSE;
    }

    /* Create driver specific screen private data.
    */
    sPriv->private = (GLvoid *) vivCreateScreen( sPriv );
    if (!sPriv->private) {
        vivDestroyScreen( sPriv );
        return GL_FALSE;
    }

    return GL_TRUE;
}

/* Force the context `c' to be unbound from its buffer.
 */
static GLboolean
vivLoseCurrent( __DRIcontextPrivate *driContextPriv )
{
    __GLcontext *gc = (__GLcontext *)driContextPriv->driverPrivate;
    GLboolean retVal = GL_TRUE;

    retVal = __glLoseCurrent(gc, gc->drawablePrivate, gc->readablePrivate);
    if ( retVal )
    {
        memset((void *)&__glNopContext, 0, sizeof(__GLdispatchTable));
        __glNopContext.pEntryDispatch = &__glNopFuncTable;
        _glapi_set_context(&__glNopContext);
    }
    __glSetGLcontext(gcvNULL);
    return retVal;
}

/* Protect imports.other cause it can be accessed across multi-threads in one process at one time
*/
static _glthread_Mutex __vivCrtMutex = PTHREAD_MUTEX_INITIALIZER;

/************************************************************************/

/* Create the device specific context.
 */
static GLboolean
vivCreateContext( const __GLcontextModes *modes,
                     __DRIcontextPrivate *driContextPriv,
                     GLvoid *sharedContextPrivate )
{
    __DRIscreenPrivate *sPriv = driContextPriv->driScreenPriv;
    vvtDeviceInfo *pDevInfo = (vvtDeviceInfo *)sPriv->pDevPriv;
    vivDriMirror *pDriMirror;
    __GLcontext *gc;

    GL_ASSERT(modes);
    GL_ASSERT(driContextPriv);

    /* Attach DRI mirror state that is used for drm LOCK/UNLOCK op to ctx->imports.other
    */
    pDriMirror = (vivDriMirror *)(*imports.malloc)(0, sizeof(vivDriMirror));
    if (!pDriMirror) {
        __driUtilMessage("%s: MALLOC vivDriMirror struct failed", __FUNCTION__);
        return GL_FALSE;
    }
    memset(pDriMirror, 0, sizeof(vivDriMirror));
    /* Protect imports.other from being accessed by multi-threads */
    _glthread_LOCK_MUTEX(__vivCrtMutex);
    imports.other = pDriMirror;
    pDriMirror->context = driContextPriv;
    pDriMirror->screen = sPriv;
    pDriMirror->drawable = NULL; /* Set by vivMakeCurrent */
    pDriMirror->hwContext = driContextPriv->hHWContext;
    pDriMirror->hwLock = &sPriv->pSAREA->lock;
    pDriMirror->fd = sPriv->fd;
    pDriMirror->drmMinor = sPriv->drmMinor;
    pDriMirror->lockCnt = 0;
    pDriMirror->vvtSarea = (drm_vvt_sarea_t *)((GLubyte *)sPriv->pSAREA +
                                    pDevInfo->sareaPrivOffset);

    imports.deviceIndex = sPriv->myNum;

    /* set up imports (pointer back to __glDevice) */
    imports.device = (GLvoid *)__glDevice;

    imports.config = (void *)modes;

    /* create the core rendering context */
    gc = GL_DISPATCH_TABLE.createContext(gcvNULL, 0x14, (VEGLimports *)&imports, (gctPOINTER)sharedContextPrivate);

    _glthread_UNLOCK_MUTEX(__vivCrtMutex);

    driContextPriv->driverPrivate = gc;
    if (!gc) {
        __driUtilMessage("%s: __glCreateContext() failed", __FUNCTION__);
        return GL_FALSE;
    }

    if (sharedContextPrivate) {
        __glShareContext(gc, (__GLcontext *)sharedContextPrivate);
    }

    return GL_TRUE;
}


/* Destroy the device specific context.
 */
static GLvoid
vivDestroyContext( __DRIcontextPrivate *driContextPriv )
{
    __GLcontext *gc = (__GLcontext *)driContextPriv->driverPrivate;

    if (gc) {

        vivLoseCurrent(driContextPriv);
        /* There is no need to update drawable info in vivGetLock */

        __glDestroyContext(gc);

        driContextPriv->driverPrivate = NULL;
    }
}

/************************************************************************/

static GLvoid
vivInitDrawableBuffer(__GLdrawablePrivate *glPriv, __GLdrawableBuffer *buf, GLint bits)
{
    buf->depth = bits;
    buf->elementSize = ((bits-1) / 8) + 1;
    buf->height = 0;

    /* init privateData */
    if (glPriv->dp.bindRenderBuffer) {
        (*glPriv->dp.bindRenderBuffer)(glPriv, buf);
    }
}

static GLvoid
vivInitDrawable(__GLdrawablePrivate *glPriv)
{
    GLuint bits;

    bits = glPriv->modes.rgbaBits;
    vivInitDrawableBuffer(glPriv, &glPriv->frontBuffer, bits);
    vivInitDrawableBuffer(glPriv, &glPriv->frontBuffer2, bits);
    vivInitDrawableBuffer(glPriv, &glPriv->backBuffer[__GL_RESOLVE_BUFFER], bits);

    if (glPriv->modes.tripleBufferMode) {
        vivInitDrawableBuffer(glPriv, &glPriv->backBuffer[__GL_BACK_BUFFER0], bits);
        vivInitDrawableBuffer(glPriv, &glPriv->backBuffer[__GL_BACK_BUFFER1], bits);
    }
    else {
        if (glPriv->modes.doubleBufferMode) {
            vivInitDrawableBuffer(glPriv, &glPriv->backBuffer[__GL_BACK_BUFFER0], bits);
        }
    }

    if (glPriv->modes.haveDepthBuffer) {
        if (glPriv->modes.depthBits == 24)
            vivInitDrawableBuffer(glPriv, &glPriv->depthBuffer, 32);
        else
            vivInitDrawableBuffer(glPriv, &glPriv->depthBuffer, 16);
    }

    if (glPriv->modes.haveStencilBuffer) {
        vivInitDrawableBuffer(glPriv, &glPriv->stencilBuffer, 8);
    }

    if (glPriv->modes.haveAccumBuffer) {
        vivInitDrawableBuffer(glPriv, &glPriv->accumBuffer, bits);
    }

}

static GLvoid
vivGenerateDrawableInternalFormat(__GLdrawablePrivate * drawable, GLint dispColorDepth)
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
        drawable->internalFormatColorBuffer = GL_BGRA8_VIVPRIV;
        break;
    case 64:
        drawable->internalFormatColorBuffer = GL_RGBA16F_ARB;
        break;
    case 96:
        drawable->internalFormatColorBuffer = GL_RGB32F_ARB;
        break;
    case 128:
        drawable->internalFormatColorBuffer = GL_RGBA32F_ARB;
        break;
    default:
        GL_ASSERT(0);
        break;
    }

    switch(modes->depthBits)
    {
    case 16:
        if (modes->stencilBits > 0)
        {
            drawable->internalFormatDepthBuffer = GL_DEPTH24_STENCIL8_EXT;
            drawable->internalFormatStencilBuffer = GL_DEPTH24_STENCIL8_EXT;
        }
        else
        {
            drawable->internalFormatDepthBuffer = GL_DEPTH_COMPONENT16;
        }
        break;

    case 24:
        if (modes->stencilBits > 0)
        {
            drawable->internalFormatDepthBuffer = GL_DEPTH24_STENCIL8_EXT;
            drawable->internalFormatStencilBuffer = GL_DEPTH24_STENCIL8_EXT;
        }
        else
        {
            drawable->internalFormatDepthBuffer = GL_DEPTH_COMPONENT24;
        }
        break;
    case 0:
        break;

    default:
        GL_ASSERT(0);
        break;
    }

    switch(modes->accumBits)
    {
    case 0:
        break;

    case 32:
        drawable->internalFormatAccumBuffer = GL_RGBA8;
        break;

    default:
        GL_ASSERT(0);
        break;
    }

    switch (dispColorDepth) {
        case 16:
            drawable->internalFormatDisplayBuffer = GL_RGB5;
            break;

        case 32:
            drawable->internalFormatDisplayBuffer = GL_BGRA8_VIVPRIV;
            break;
    }

    return;
}

/* Create and initialize driver specific drawable buffer data.
 */
static GLboolean
vivCreateDrawable( __DRIscreenPrivate *driScrnPriv,
                    __DRIdrawablePrivate *driDrawPriv,
                    const __GLcontextModes *modes,
                    GLboolean isPixmap)
{
    __GLdeviceStruct *__glDevice = &__glDevicePipeEntry[driScrnPriv->myNum];
    __GLdrawablePrivate *glPriv;
    vvtDeviceInfo *pDevInfo = (vvtDeviceInfo*)driScrnPriv->pDevPriv;


    /* Allocate the private area */
    glPriv = (__GLdrawablePrivate *)(*imports.calloc)(0, 1, sizeof(__GLdrawablePrivate));
    if ( !glPriv ) {
        __driUtilMessage("%s: CALLOC __GLdrawablePrivate struct failed", __FUNCTION__);
        return GL_FALSE;
    }

    driDrawPriv->driverPrivate = glPriv;

    glPriv->modes = *modes;
    vivGenerateDrawableInternalFormat( glPriv, (pDevInfo->bufBpp == 2) ? 16 : 32 );

    switch (glPriv->modes.greenBits)
    {
    case 4:
        glPriv->rtFormatInfo = &__glFormatInfoTable[__GL_FMT_RGBA4];
        break;
    case 5:
        glPriv->rtFormatInfo = &__glFormatInfoTable[__GL_FMT_RGB5_A1];
        break;
    case 6:
        glPriv->rtFormatInfo = &__glFormatInfoTable[__GL_FMT_RGB565];
        break;
    case 8:
        glPriv->rtFormatInfo = glPriv->modes.alphaBits
                             ? &__glFormatInfoTable[__GL_FMT_RGBA8]
                             : &__glFormatInfoTable[__GL_FMT_RGB8];
        break;
    default:
        GL_ASSERT(0);
        glPriv->rtFormatInfo = gcvNULL;
    }

    switch (glPriv->modes.depthBits)
    {
        case 16:
            glPriv->dsFormatInfo = &__glFormatInfoTable[__GL_FMT_Z16];
            break;
        case 24:
            glPriv->dsFormatInfo = glPriv->modes.stencilBits
                                     ? &__glFormatInfoTable[__GL_FMT_Z24S8]
                                     : &__glFormatInfoTable[__GL_FMT_Z24];
            break;
        default:
            GL_ASSERT(0);
            glPriv->dsFormatInfo = gcvNULL;
            break;
    }


    glPriv->lock = NULL; //__vivCreateDrawableMutex();
    glPriv->other = driDrawPriv;

    glPriv->malloc = __vivMalloc;
    glPriv->calloc = __vivCalloc;
    glPriv->realloc = __vivRealloc;
    glPriv->free = __vivFree;
    glPriv->addSwapHintRectWIN = NULL;
    glPriv->clearSwapHintRectWIN = NULL;

    /* Allocate buffer for clipRects */
    glPriv->clipRects = (GLuint *)(*imports.malloc)(0, 16 * sizeof(RECT));

    {
        glPriv->yInverted = GL_FALSE;
    }

    glPriv->type = __GL_WINDOW;

    (*__glDevice->devCreateDrawable)(glPriv, 0);

    /* Initialize drawable */
    vivInitDrawable(glPriv);

    return GL_TRUE;
}

static GLvoid
vivDestroyDrawableBuffers(__GLdrawablePrivate *glPriv)
{
    /* Call device specific FreeBuffers function.*/
    if (glPriv->dp.freeBuffers) {
        (*glPriv->dp.freeBuffers)(glPriv, GL_FALSE);
    }


    if (glPriv->dp.deleteRenderBuffer) {
        (*glPriv->dp.deleteRenderBuffer)(glPriv, &glPriv->frontBuffer);
        (*glPriv->dp.deleteRenderBuffer)(glPriv, &glPriv->frontBuffer2);
        (*glPriv->dp.deleteRenderBuffer)(glPriv, &glPriv->backBuffer[__GL_BACK_BUFFER0]);
        (*glPriv->dp.deleteRenderBuffer)(glPriv, &glPriv->backBuffer[__GL_BACK_BUFFER1]);
        (*glPriv->dp.deleteRenderBuffer)(glPriv, &glPriv->backBuffer[__GL_RESOLVE_BUFFER]);
        (*glPriv->dp.deleteRenderBuffer)(glPriv, &glPriv->depthBuffer);
        (*glPriv->dp.deleteRenderBuffer)(glPriv, &glPriv->stencilBuffer);
        (*glPriv->dp.deleteRenderBuffer)(glPriv, &glPriv->accumBuffer);
    }
}

static GLvoid
vivDestroyDrawable(__DRIdrawablePrivate *driDrawPriv)
{
    __GLdrawablePrivate *glPriv = driDrawPriv->driverPrivate;
    __DRIscreenPrivate * sPriv = driDrawPriv->driScreenPriv;

    if (glPriv->clipRects) {
        (*imports.free)(0, glPriv->clipRects);
        glPriv->clipRects = NULL;
    }

    LINUX_LOCK_FRAMEBUFFER_DUMMY( sPriv );

    /* Free buffer and destroy drawable*/
    vivDestroyDrawableBuffers(glPriv);


    /* Free pbuffer texture structure.*/
    if (glPriv->pbufferTex) {
        (*imports.free)(0, glPriv->pbufferTex);
        glPriv->pbufferTex = NULL;
    }

    /* Free the device specific data structure attached to the drawable */
    if (glPriv->dp.destroyPrivateData) {
        (*glPriv->dp.destroyPrivateData)(glPriv);
    }

    LINUX_UNLOCK_FRAMEBUFFER_DUMMY(sPriv);

    /* Free the drawable private */
    (*imports.free)(0, glPriv);
    driDrawPriv->driverPrivate = NULL;
}

static GLvoid
vivSwapBuffers(__DRIdrawablePrivate *driDrawPriv)
{
    __GLdrawablePrivate *glPriv = driDrawPriv->driverPrivate;
    __DRIcontextPrivate *pcp = driDrawPriv->driContextPriv;
    __GLcontext *gc = (__GLcontext *)pcp->driverPrivate;


    __GL_VERTEX_BUFFER_FLUSH(gc);

    __glNotifyDrawableChange(gc, __GL_DRAWABLE_PENDING_SWAP);

    /* Call DP SwapBuffers */
    (*glPriv->dp.swapBuffers)(gc, glPriv, (gc->flags & __GL_DRAW_TO_FRONT));
}

/* Force the context `c' to be the current context and associate with buffer `b'.
 */
static GLboolean
vivMakeCurrent( __DRIcontextPrivate *driContextPriv,
                    __DRIdrawablePrivate *driDrawPriv,
                    __DRIdrawablePrivate *driReadPriv )
{
    __GLcontext *gc = (__GLcontext *)driContextPriv->driverPrivate;
    vivDriMirror *pDriMirror = (vivDriMirror *)gc->imports.other;

    GLboolean retVal = GL_FALSE;

    pDriMirror->drawable = driDrawPriv;

    __glNotifyDrawableChange(gc, __GL_DRAWABLE_PENDING_SWITCH);

    __glAssociateContext(gc, driDrawPriv->driverPrivate, driReadPriv->driverPrivate);

    /* Call GLcore makecurrent function */
    retVal=__glMakeCurrent(gc, (__GLdrawablePrivate *)driDrawPriv->driverPrivate, (__GLdrawablePrivate *)driReadPriv->driverPrivate, gcvFALSE);
    if ( retVal )
        _glapi_set_context(gc);
    __glSetGLcontext(gc);
    return retVal;
}

/* Copy GL states of a context to another.
 */
static GLvoid
vivCopyContext( __DRIcontextPrivate *srcPriv,
                __DRIcontextPrivate *dstPriv, GLuint mask )
{
    __GLcontext *srcGc = (__GLcontext *)srcPriv->driverPrivate;
    __GLcontext *dstGc = (__GLcontext *)dstPriv->driverPrivate;

    __glCopyContext(srcGc, dstGc, mask);
}

/************************************************************************/

static struct __DriverAPIRec vivAPI = {
    .InitDriver      = vivInitDriver,
    .DestroyScreen   = vivDestroyScreen,
    .CreateContext   = vivCreateContext,
    .DestroyContext  = vivDestroyContext,
    .CreateBuffer    = vivCreateDrawable,
    .DestroyBuffer   = vivDestroyDrawable,
    .SwapBuffers     = vivSwapBuffers,
    .MakeCurrent     = vivMakeCurrent,
    .UnbindContext   = vivLoseCurrent,
    .CopyContext     = vivCopyContext
};

static GLboolean
driFillInModes( __GLcontextModes ** ptr_to_modes,
        GLenum fb_format, GLenum fb_type,
        GLuint *depth_bits, GLuint *stencil_bits,
        GLuint depth_buffer_factor,
        GLuint back_buffer_factor, GLint visType )
{
    static const GLuint bits_table[3][4] = {
        /* R  G  B  A */
        { 5, 6, 5, 0 }, /* Any GL_UNSIGNED_SHORT_5_6_5 */
        { 8, 8, 8, 0 }, /* Any RGB with any GL_UNSIGNED_INT_8_8_8_8 */
        { 8, 8, 8, 8 }  /* Any RGBA with any GL_UNSIGNED_INT_8_8_8_8 */
    };

    /* The following arrays are all indexed by the fb_type masked with 0x07.
    * Given the four supported fb_type values, this results in valid array
    * indices of 3, 4, 5, and 7.
    */
    static const GLuint masks_table_rgb[8][4] = {
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 }, /* 5_6_5       */
        { 0x0000001F, 0x000007E0, 0x0000F800, 0x00000000 }, /* 5_6_5_REV   */
        { 0xFF000000, 0x00FF0000, 0x0000FF00, 0x00000000 }, /* 8_8_8_8     */
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 }  /* 8_8_8_8_REV */
    };

    static const GLuint masks_table_rgba[8][4] = {
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 }, /* 5_6_5       */
        { 0x0000001F, 0x000007E0, 0x0000F800, 0x00000000 }, /* 5_6_5_REV   */
        { 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF }, /* 8_8_8_8     */
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000 }, /* 8_8_8_8_REV */
    };

    static const GLuint masks_table_bgr[8][4] = {
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x0000001F, 0x000007E0, 0x0000F800, 0x00000000 }, /* 5_6_5       */
        { 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 }, /* 5_6_5_REV   */
        { 0x0000FF00, 0x00FF0000, 0xFF000000, 0x00000000 }, /* 8_8_8_8     */
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 }, /* 8_8_8_8_REV */
    };

    static const GLuint masks_table_bgra[8][4] = {
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x0000001F, 0x000007E0, 0x0000F800, 0x00000000 }, /* 5_6_5       */
        { 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 }, /* 5_6_5_REV   */
        { 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF }, /* 8_8_8_8     */
        { 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
        { 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 }, /* 8_8_8_8_REV */
    };

    static const GLuint bytes_per_pixel[8] = {
        0, 0, 0, 2, 2, 4, 0, 4
    };

    __GLcontextModes *modes = *ptr_to_modes;
    const GLint index = fb_type & 0x07;
    const GLuint *bits;
    const GLuint *masks;
    GLuint i, j, k;

    if ( bytes_per_pixel[ index ] == 0 ) {
        fprintf( stderr, "[%s:%u] Framebuffer type 0x%04x has 0 bytes per pixel.\n",
                __FUNCTION__, __LINE__, fb_type );
        return GL_FALSE;
    }

    /* Valid types are GL_UNSIGNED_SHORT_5_6_5 and GL_UNSIGNED_INT_8_8_8_8 and
    *  the _REV versions.
    *  Valid formats are GL_RGBA, GL_RGB, and GL_BGRA.
    */
    switch ( fb_format ) {
      case GL_RGB:
        bits = (bytes_per_pixel[ index ] == 2) ? bits_table[0] : bits_table[1];
        masks = masks_table_rgb[ index ];
        break;
      case GL_RGBA:
        bits = (bytes_per_pixel[ index ] == 2) ? bits_table[0] : bits_table[2];
        masks = masks_table_rgba[ index ];
        break;
      case GL_BGR:
        bits = (bytes_per_pixel[ index ] == 2) ? bits_table[0] : bits_table[1];
        masks = masks_table_bgr[ index ];
        break;
      case GL_BGRA:
        bits = (bytes_per_pixel[ index ] == 2) ? bits_table[0] : bits_table[2];
        masks = masks_table_bgra[ index ];
        break;
      default:
        fprintf( stderr, "[%s:%u] Framebuffer format 0x%04x is not GL_RGB, GL_RGBA, GL_BGR, or GL_BGRA.\n",
                __FUNCTION__, __LINE__, fb_format );
        return GL_FALSE;
    }

    for ( k = 0 ; k < depth_buffer_factor ; k++ ) {
        for ( i = 0 ; i < back_buffer_factor ; i++ ) {
            for ( j = 0 ; j < 2 ; j++ ) {

                modes->redBits   = bits[0];
                modes->greenBits = bits[1];
                modes->blueBits  = bits[2];
                modes->alphaBits = bits[3];
                modes->redMask   = masks[0];
                modes->greenMask = masks[1];
                modes->blueMask  = masks[2];
                modes->alphaMask = masks[3];
                modes->rgbaBits   = modes->redBits + modes->greenBits
                                  + modes->blueBits + modes->alphaBits;

                modes->accumRedBits   = 16 * j;
                modes->accumGreenBits = 16 * j;
                modes->accumBlueBits  = 16 * j;
                modes->accumAlphaBits = (masks[3] != 0) ? 16 * j : 0;
                modes->visualRating = (j == 0) ? GLX_NONE : GLX_SLOW_CONFIG;

                modes->stencilBits = stencil_bits[k];
                modes->depthBits = depth_bits[k];

                modes->visualType = visType;
                modes->renderType = GLX_RGBA_BIT;
                modes->drawableType = GLX_WINDOW_BIT;
                modes->rgbMode = GL_TRUE;

                modes->doubleBufferMode = (i == 0) ? GL_FALSE : GL_TRUE;

                modes->haveAccumBuffer = ((modes->accumRedBits +
                                           modes->accumGreenBits +
                                           modes->accumBlueBits +
                                           modes->accumAlphaBits) > 0);
                modes->haveDepthBuffer = (modes->depthBits > 0);
                modes->haveStencilBuffer = (modes->stencilBits > 0);

                modes = modes->next;
            }
        }
    }

    *ptr_to_modes = modes;
    return GL_TRUE;
}

extern __GLcontextModes *__glContextModesCreate( unsigned count );

static __GLcontextModes *vivFillInModes(
    GLuint pixel_bits)
{
    __GLcontextModes *modes;
    __GLcontextModes *m;
    GLuint num_modes;
    GLuint depth_buffer_factor;
    GLuint back_buffer_factor;
    GLenum fb_format, fb_type;
    GLuint depth_bits_array[4];
    GLuint stencil_bits_array[4];

    depth_bits_array[0] = 0;
    depth_bits_array[1] = 16;
    depth_bits_array[2] = 16;
    depth_bits_array[3] = 24;

    /* Just like with the accumulation buffer, always provide some modes
     * with a stencil buffer. It will be a sw fallback, but some apps won't
     * care about that.
     */
    stencil_bits_array[0] = 0;
    stencil_bits_array[1] = 0;
    stencil_bits_array[2] = 8;
    stencil_bits_array[3] = 8;
    depth_buffer_factor = 4;
    back_buffer_factor  = 2;

    num_modes = depth_buffer_factor * back_buffer_factor * 4;

    if (pixel_bits == 16) {
        fb_format = GL_RGB;
        fb_type = GL_UNSIGNED_SHORT_5_6_5;
#if ENABLE_32BIT_RT_ON_16BIT_DISPLAY
        num_modes *= 2;
#endif
    } else {
        fb_format = GL_BGRA; /* AGBG or BGRA? */
        fb_type = GL_UNSIGNED_INT_8_8_8_8_REV;
    }

    modes = __glContextModesCreate(num_modes);
    m = modes;

    if (!driFillInModes(&m, fb_format, fb_type,
                        depth_bits_array, stencil_bits_array, depth_buffer_factor,
                        back_buffer_factor, GLX_TRUE_COLOR)) {
        fprintf(stderr, "[%s:%u] Error creating FBConfig!\n", __func__, __LINE__);
        return NULL;
    }

    if (!driFillInModes(&m, fb_format, fb_type,
                        depth_bits_array, stencil_bits_array, depth_buffer_factor,
                        back_buffer_factor, GLX_DIRECT_COLOR)) {
        fprintf(stderr, "[%s:%u] Error creating FBConfig!\n", __func__, __LINE__);
        return NULL;
    }

#if ENABLE_32BIT_RT_ON_16BIT_DISPLAY
    if (pixel_bits == 16) {
        fb_format = GL_BGRA; /* AGBG or BGRA? */
        fb_type = GL_UNSIGNED_INT_8_8_8_8_REV;
        if (!driFillInModes(&m, fb_format, fb_type,
                            depth_bits_array, stencil_bits_array, depth_buffer_factor,
                            back_buffer_factor, GLX_TRUE_COLOR)) {
            fprintf(stderr, "[%s:%u] Error creating FBConfig!\n", __func__, __LINE__);
            return NULL;
        }

        if (!driFillInModes(&m, fb_format, fb_type,
                           depth_bits_array, stencil_bits_array, depth_buffer_factor,
                           back_buffer_factor, GLX_DIRECT_COLOR)) {
            fprintf(stderr, "[%s:%u] Error creating FBConfig!\n", __func__, __LINE__);
            return NULL;
        }
    }
#endif

    return modes;
}


/*
 * This is the bootstrap function for the driver.
 * The __driCreateScreen name is the symbol that libGL.so fetches.
 * Return:  pointer to a __DRIscreenPrivate.
 */
GLvoid * __driCreateNewScreen( Display *dpy, GLint scrn, __DRIscreen *psc,
                 const __GLcontextModes * modes,
                 const __DRIversion * ddx_version,
                 const __DRIversion * dri_version,
                 const __DRIversion * drm_version,
                 const __DRIframebuffer * frame_buffer,
                 drmAddress pSAREA, GLint fd,
                 GLint internal_api_version,
                 __GLcontextModes ** driver_modes )

{
    __DRIscreenPrivate *psp;

    psp = __driUtilCreateNewScreen(dpy, scrn, psc, NULL,
                  ddx_version, dri_version, drm_version,
                  frame_buffer, pSAREA, fd,
                  internal_api_version, &vivAPI);

    /* Initialize __GLcontextModes list with supported pixel formats.
     */
    if ( psp != NULL ) {
         vvtDeviceInfo *pDevInfo = (vvtDeviceInfo*)psp->pDevPriv;
         *driver_modes = vivFillInModes((pDevInfo->bufBpp == 2) ? 16 : 32);
    }

    return (GLvoid *) psp;
}
#endif
