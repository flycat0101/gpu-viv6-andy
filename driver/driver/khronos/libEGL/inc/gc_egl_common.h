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


/*
 * Vivante specific definitions and declarations for EGL library.
 */

#ifndef __gc_egl_common_h_
#define __gc_egl_common_h_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifdef __cplusplus
extern "C" {
#endif


#if gcdSTATIC_LINK
#define __EGL_APICALL
#else
#define __EGL_APICALL EGLAPI
#endif

/* A generic function pointer. */
typedef void (* gltRENDER_FUNCTION)(void);
typedef void*(* gltCONTEXT_FUNCTION)(EGLenum);

/* Union of possible variables. */
typedef union gcuRENDER_VALUE
{
    gctINT              i;
    gctUINT             u;
    gctFLOAT            f;
    gctPOINTER          p;
}
gcuRENDER_VALUE;

/* One record in the message queue. */
typedef struct gcsRENDER_MESSAGE
{
    /* Function index. */
    gltRENDER_FUNCTION  function;

    /* Argument count. */
    gctUINT             argCount;

    /* Arguments. */
    gcuRENDER_VALUE     arg[9];

    /* Output. */
    gcuRENDER_VALUE     output;

    /* Memory usage. */
    gctSIZE_T           bytes[2];
}
gcsRENDER_MESSAGE;

#define gcdRENDER_MESSAGES      18000
#define gcdRENDER_MEMORY        4096

typedef struct gcsRENDER_THREAD gcsRENDER_THREAD;

/* Prototype for a render thread. */
typedef void (* veglRENDER_THREAD) (
    gcsRENDER_THREAD * ThreadInfo
    );

/* Thread information structure for a render thread. */
struct gcsRENDER_THREAD
{
    /* Thread handle. */
    gctHANDLE           thread;

    /* Owning context. */
    EGLContext          context;

    /* API context. */
    gctPOINTER          apiContext;

    /* Pointer to render thread handler. */
    veglRENDER_THREAD   handler;

    /* Thread status. */
    volatile gceSTATUS  status;

    /* Signal to start the thread. */
    gctSIGNAL           signalGo;

    /* Signal when the thread is done. */
    gctSIGNAL           signalDone;

    /* Signal when the thread has executed one message. */
    gctSIGNAL           signalHeartbeat;

    /* The message queue. */
    gcsRENDER_MESSAGE   queue[gcdRENDER_MESSAGES];

    /* Producer entry. */
    volatile gctUINT    producer;

    /* Consumer entry. */
    volatile gctUINT    consumer;

    /* The render thread local memory. */
    gctUINT8            memory[gcdRENDER_MEMORY];

    /* Producer memory offset. */
    volatile gctUINT    producerOffset;

    /* Consumer memory offset. */
    volatile gctUINT    consumerOffset;
};

gcsRENDER_MESSAGE *eglfGetRenderMessage(IN gcsRENDER_THREAD *Thread);

gcsRENDER_THREAD *eglfGetCurrentRenderThread(void);

void eglfWaitRenderThread(IN gcsRENDER_THREAD *Thread);

gctPOINTER eglfCopyRenderMemory(IN gctCONST_POINTER Memory, IN gctSIZE_T Bytes);

typedef struct __VEGLLock
{
    gctPOINTER lock;
    gctINT     usageCount;
} VEGLLock;

/* Private Drawable structure derived from eglSurface, which will be used by Halti driver */
typedef struct EGLDrawableRec
{
    gctPOINTER config;

    EGLint width;
    EGLint height;

    void * rtHandle;        /* gco surface handle of render target */
    void * prevRtHandle;    /* gco surface handle of prevous render target */
    void * depthHandle;     /* gco surface handle of depth */
    void * stencilHandle;   /* gco surface handle of stencil */

    void * private;         /* private handle point to API drawable */
    void (*destroyPrivate)(void*);
} EGLDrawable, *VEGLDrawable;

/* EGL image type enum. */
typedef enum _khrIMAGE_TYPE
{
    KHR_IMAGE_TEXTURE_2D            = 1,
    KHR_IMAGE_TEXTURE_CUBE,
    KHR_IMAGE_TEXTURE_3D,
    KHR_IMAGE_RENDER_BUFFER,
    KHR_IMAGE_VG_IMAGE,
    KHR_IMAGE_PIXMAP,
    KHR_IMAGE_ANDROID_NATIVE_BUFFER,
    KHR_IMAGE_WAYLAND_BUFFER,
    KHR_IMAGE_VIV_DEC,
    KHR_IMAGE_LINUX_DMA_BUF,
} khrIMAGE_TYPE;

#define KHR_EGL_IMAGE_MAGIC_NUM        0x47414D49  /* "IMAG" */

/* EGL Image */
typedef struct _khrEGL_IMAGE
{
    gctUINT32                   magic;
    khrIMAGE_TYPE               type;

    /* Mutex to protect image contents. */
    gctPOINTER                  mutex;

    /* Source sibling. */
    gcoSURF                     surface;

    /* Latest EGLImage sibling. */
    gcoSURF                     srcSurface;

    /*
     * Some type of EGLImage source sibling may may be changed implicitly
     * outside client driver, such as Pixmap, Android Native Buffer, etc.
     * This callback function is set by EGL for client driver to update
     * source sibling pixels. Returns true if pixels are updated.
     */
    gctBOOL                  (* update)(struct _khrEGL_IMAGE *);

    union
    {
        struct _khrEGL_IMAGE_TEXTURE
        {
            gctUINT32           width;
            gctUINT32           height;

            /* Format defined in GLES. */
            gctUINT32           format;
            gctUINT32           type;
            gctUINT32           internalFormat;

            gctUINT32           level;
            gctUINT32           face;
            gctUINT32           depth;

            /* Slice index in surface, for cubemap. */
            gctUINT32           sliceIndex;

            gctUINT32           texture;
            void *              object;

            /*Shadow surface, if exist we will always render to shadow surface for now */
            gcoSURF             shadowSurface;

            /* Master surface has content updated if TRUE */
            gctBOOL             masterDirty;
        } texture;

        struct _khrEGL_IMAGE_PIXMAP
        {
            void *              nativePixmap;
            void *              pixInfo;

            /* Native pixmap information. */
            gctUINT             width;
            gctUINT             height;

#ifdef EGL_API_XXX
            gctINT              seqNo;
#endif
        } pixmap;

        struct _khrEGL_IMAGE_VGIMAGE
        {
            gcoSURF             texSurface;

            gctUINT32           width;
            gctUINT32           height;
            gctUINT32           offset_x;
            gctUINT32           offset_y;

            gctUINT32           format;
            gctUINT32           allowedQuality;
            gctINT32            dirty;
            gctINT32_PTR        dirtyPtr;
            gctINT32            rootWidth;
            gctINT32            rootHeight;
            gctINT32            rootOffsetX;
            gctINT32            rootOffsetY;
        } vgimage;

        struct _khrEGL_IMAGE_ANDROID
        {
            void *              nativeBuffer;
            gceSURF_FORMAT      format;
            gctUINT64           timeStamp;
        } ANativeBuffer;

        struct _khrEGL_IMAGE_DECIMAGE
        {
            struct _gcoSURF *   texSurface;

            gctUINT32           width;
            gctUINT32           height;
            gctUINT32           color_buf;
            gctUINT32           tile_buf;

            gctUINT32           format;
            gctUINT32           reserved;
        } decimage;

        struct _khrEGL_IMAGE_DMA_BUF
        {
            gctINT              width;
            gctINT              height;
            gceSURF_FORMAT      format;
            gctINT              fd[3];
            gctINT              offset[3];
            gctINT              pitch[3];
            gceSURF_YUV_COLOR_SPACE   colorSpace;
            gceSURF_YUV_SAMPLE_RANGE  sampleRange;
            gceSURF_YUV_CHROMA_SITING siting[2];
        } dmaBuf;
        struct _khrEGL_IMAGE_WAYLAND_BUFFER
        {
            gctINT              width;
            gctINT              height;
        } wlbuffer;
    } u;
}
khrEGL_IMAGE;

typedef khrEGL_IMAGE * khrEGL_IMAGE_PTR;

/* Procedures which are imported by the GL from the surrounding "operating system".
** Math functions are not considered part of the "operating system".
*/
typedef struct __VEGLimports
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

    /* critical section lock import into gl core */
    void (*createMutex)(VEGLLock *lock);
    void (*destroyMutex)(VEGLLock *lock);
    void (*lockMutex)(VEGLLock *lock);
    void (*unlockMutex)(VEGLLock *lock);

    /* Pass context associated EGLConfig */
    void *config;

    /* EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT value */
    gctBOOL robustAccess;

    /* EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT value */
    gctINT resetNotification;

    gctBOOL debuggable;

} VEGLimports;



typedef void * (* veglCREATECONTEXT) (
    void * Thread,
#if gcdGC355_PROFILER
    gctUINT64 appStartTime,
    gctFILE apiTimeFile,
#endif
    gctINT ClientVersion,
    VEGLimports *Imports,
    gctPOINTER SharedContext
    );

typedef EGLBoolean (* veglDESTROYCONTEXT) (
    void * Thread,
    void * Context
    );

typedef EGLBoolean (* veglFLUSHCONTEXT) (
    void * Context
    );

typedef EGLBoolean (* veglMAKECURRENT) (
    void       * Thread,
    void       * Context,
    VEGLDrawable drawable,
    VEGLDrawable readable
    );

typedef EGLBoolean (* veglLOSECURRENT) (
    void       * Thread,
    void       * Context
    );

typedef EGLBoolean (* veglSETDRAWABLE) (
    void       * Thread,
    void       * Context,
    VEGLDrawable drawable,
    VEGLDrawable readable
    );

typedef gceSTATUS (* veglSETBUFFER) (
    gcoSURF Draw
    );

typedef void (* veglFLUSH) (
    void
    );

typedef void (* veglFINISH) (
    void
    );

typedef gcoSURF (* veglGETCLIENTBUFFER) (
    void * Context,
    EGLClientBuffer Buffer
    );

typedef EGLenum (* veglBINDTEXIMAGE) (
    void * Surface,
    EGLenum Format,
    EGLBoolean Mipmap,
    EGLint Level,
    EGLint Width,
    EGLint Height,
    void ** Binder
    );

typedef gctBOOL (* veglPROFILER) (
    void * Profiler,
    gctUINT32 Enum,
    gctHANDLE Value
    );

typedef void (* EGL_PROC)(void);
typedef EGL_PROC (* veglGETPROCADDR) (const char *procname);

typedef EGLenum (* veglCREATEIMAGETEXTURE) (
    void * Context,
    EGLenum Target,
    gctINT Texture,
    gctINT Level,
    gctINT Depth,
    gctPOINTER Image
    );

typedef EGLenum (* veglCREATEIMAGERENDERBUFFER) (
    void * Context,
    EGLenum Renderbuffer,
    gctPOINTER Image
    );

typedef    EGLenum (* veglCREATEIMAGEVGPARENTIMAGE) (
    void * Context,
    unsigned int vgImage,
    void ** Images,
    int * Count
    );

typedef EGLBoolean (* veglQUERYHWVG) (
    void
    );

typedef void (* gcfEGL_DO_SWAPBUFFER)(EGLDisplay Dpy, EGLSurface Draw);

typedef EGLBoolean (* veglSWAPBUFFER)(
    EGLDisplay Dpy,
    EGLSurface Draw,
    gcfEGL_DO_SWAPBUFFER Callback
    );

/* VG Resolve */
typedef void (* veglAPPENDVGRESOLVE) (
    void * Context,
    gcoSURF Target
    );

typedef void (* veglSyncImage) (
    void * Context
    );

/************************************************************************/

typedef struct _veglLOOKUP
{
    const char *                                name;
    __eglMustCastToProperFunctionPointerType    function;
}
veglLOOKUP;

#define eglMAKE_LOOKUP(function) \
    { #function, (__eglMustCastToProperFunctionPointerType) function }

#define eglMAKE_EXT_ENTRY(function) \
    { #function, (__eglMustCastToProperFunctionPointerType) function##_Entry }

typedef struct _veglDISPATCH
{
    veglCREATECONTEXT               createContext;
    veglDESTROYCONTEXT              destroyContext;
    veglMAKECURRENT                 makeCurrent;
    veglLOSECURRENT                 loseCurrent;
    veglSETDRAWABLE                 setDrawable;
    veglFLUSHCONTEXT                flushContext;

    veglFLUSH                       flush;
    veglFINISH                      finish;

    veglSETBUFFER                   setBuffer;
    veglGETCLIENTBUFFER             getClientBuffer;

    veglCREATEIMAGETEXTURE          createImageTexture;
    veglCREATEIMAGERENDERBUFFER     createImageRenderbuffer;
    veglCREATEIMAGEVGPARENTIMAGE    createImageParentImage;
    veglBINDTEXIMAGE                bindTexImage;

    veglPROFILER                    profiler;

    veglGETPROCADDR                 getProcAddr;

    veglQUERYHWVG                   queryHWVG;

    veglRENDER_THREAD               renderThread;

    veglSWAPBUFFER                  swapBuffer;

    veglAPPENDVGRESOLVE             resolveVG;  /* VG Resolve */

    veglSyncImage                   syncImage;
}
veglDISPATCH;

#ifdef __cplusplus
}
#endif

#endif /* __gc_egl_common_h_ */
