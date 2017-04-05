/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __win_gl_h_
#define __win_gl_h_

#include "EGL/egl.h"
#include "gc_egl_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __GLmemoryStatusEnum{
    __GL_MMUSAGE_PHYS_TOTAL = 0,
    __GL_MMUSAGE_PHYS_AVAIL = 1,
    __GL_MMUSAGE_VIRTUAL_TOTAL = 2,
    __GL_MMUSAGE_VIRTUAL_AVAIL = 3,
}__GLmemoryStatus;

typedef struct __VEGLEXimports
{
    void* (*getCurContext)(EGLenum);
    void  (*setCurContext)(EGLenum, void*);
    void  (*syncNative)(void);
    void  (*referenceImage)(khrEGL_IMAGE *);
    void  (*dereferenceImage)(khrEGL_IMAGE *);

    /* Memory management */
    gctPOINTER (*malloc)(void *ctx, gctSIZE_T size);
    gctPOINTER (*calloc)(void *ctx, gctSIZE_T numElem, gctSIZE_T elemSize);
    gctPOINTER (*realloc)(void *ctx, gctPOINTER oldAddr, gctSIZE_T newSize);
    void       (*free)(void *ctx, gctPOINTER addr);
    gctUINT (*getMemoryStatus)(__GLmemoryStatus);


    /* Error handling */
    void (*warning)(__GLcontext *gc, const GLbyte *fmt);
    void (*fatal)(__GLcontext *gc, const GLbyte *fmt);
    /* Set window's GL api dispatch table */
    void (*setProcTable)(__GLcontext *gc, void *state);

    /* Drawable mutex lock from gl core */
    void (*lockDrawable)(void *lock);
    void (*unlockDrawable)(void *lock);

    /* critical section lock import into gl core */
    void (*createMutex)(VEGLLock *lock);
    void (*destroyMutex)(VEGLLock *lock);
    void (*lockMutex)(VEGLLock *lock);
    void (*unlockMutex)(VEGLLock *lock);

    /* GetDC and GetHWND */
    void *(*getHWND)(__GLcontext *gc);
    void *(*getDC)(void *other);
    void (*releaseDC)(void *other, void *hDC);

    /* Blt back buffer content to front buffer */
    /* This function is called if renderring in front buffer */
    void (*internalSwapBuffers)(__GLcontext *gc, gctBOOL bSwapFront, gctBOOL finishMode);

    /* Get current display mode */
    void (*getDisplayMode)(gctUINT *width, gctUINT *height, gctUINT *depth);

    /* Get the colorbuffer from the handle of pbuffer */
    void *(*getColorbufferPBuffer)(void  *hpbuffer,GLenum iBuffer);

    /* Device configuration changed function */
    void (*deviceConfigChanged)(__GLcontext *gc);

    void (*bltImageToScreen)(__GLcontext *gc,
                              gctINT bitsAlignedWidth,
                              gctINT bitsAlignedHeight,
                              gctINT bitsPerPixel,
                              gctINT * bits,
                              gctINT left,
                              gctINT top,
                              gctINT width,
                              gctINT height);

    /* Pass context associated EGLConfig */
    void *config;

    /* EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT value */
    gctBOOL robustAccess;

    /* EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT value */
    gctINT resetNotification;

    gctBOOL debuggable;

    /* Device DLL interface */
    void *device;
    gctUINT deviceIndex;

    /* Operating system dependent data goes here */
    void *other;

} VEGLEXimports;


#define __GL_DRAWABLE_PENDING_RESIZE        0x1
#define __GL_DRAWABLE_PENDING_MOVE          0x2
#define __GL_DRAWABLE_PENDING_DESTROY       0x4
#define __GL_DRAWABLE_PENDING_SWAP          0x8
#define __GL_DRAWABLE_PENDING_SWITCH        0x10
#define __GL_DRAWABLE_PENDING_RT_RESIDENT   0x20
#define __GL_DRAWABLE_PENDING_CLIPLIST      0x40
#define __GL_DRAWABLE_PENDING_PRIMARY_LOST  0x80

#ifdef __cplusplus
}
#endif

#endif
