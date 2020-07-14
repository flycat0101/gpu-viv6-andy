/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
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

/* Some internal platform types */
enum {
    /* khronos define platform */
    EGL_PLATFORM_DEFAULT_VIV = 0,
    EGL_PLATFORM_ANDROID_VIV,
    EGL_PLATFORM_GBM_VIV,
    EGL_PLATFORM_WAYLAND_VIV,
    EGL_PLATFORM_X11_VIV,

    /* internal platform */
    EGL_PLATFORM_DFB_VIV,
    EGL_PLATFORM_DRI_VIV,
    EGL_PLATFORM_DRI3_VIV,
    EGL_PLATFORM_FB_VIV,
    EGL_PLATFORM_NULLWS_VIV,
    EGL_PLATFORM_QNX_VIV,
    EGL_PLATFORM_WIN32_VIV,
};

/* EGL image type enum. */
typedef enum _khrIMAGE_TYPE
{
    KHR_IMAGE_TEXTURE_2D = 1,
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
            gctINT                    width;
            gctINT                    height;
            gceSURF_FORMAT            format;
            gctINT                    fd[3];
            gctINT                    offset[3];
            gctINT                    pitch[3];
            gceSURF_YUV_COLOR_SPACE   colorSpace;
            gceSURF_YUV_SAMPLE_RANGE  sampleRange;
            gceSURF_YUV_CHROMA_SITING siting[2];
        } dmaBuf;

        struct _khrEGL_IMAGE_WAYLAND_BUFFER
        {
            gctINT              width;
            gctINT              height;
            gctINT              fd;
        } wlbuffer;
    } u;
}
khrEGL_IMAGE;

typedef khrEGL_IMAGE * khrEGL_IMAGE_PTR;


/******************************************************************************/

typedef struct __VEGLLock
{
    gctPOINTER lock;
    gctINT     usageCount;
} VEGLLock;

/* Procedures which are imported by the GL from the surrounding "operating system".
** Math functions are not considered part of the "operating system".
*/
typedef struct __VEGLimports
{
    void      * (* getCurContext)(EGLenum);
    void        (* setCurContext)(EGLenum, void*);
    void        (* syncNative)(void);
    void        (* referenceImage)(khrEGL_IMAGE *);
    void        (* dereferenceImage)(khrEGL_IMAGE *);

    /* Memory management */
    gctPOINTER  (* malloc)(void * ctx, gctSIZE_T size);
    gctPOINTER  (* calloc)(void * ctx, gctSIZE_T numElem, gctSIZE_T elemSize);
    gctPOINTER  (* realloc)(void * ctx, gctPOINTER oldAddr, gctSIZE_T newSize);
    void        (* free)(void * ctx, gctPOINTER addr);

    /* critical section lock import into gl core */
    void        (* createMutex)(VEGLLock * lock);
    void        (* destroyMutex)(VEGLLock * lock);
    void        (* lockMutex)(VEGLLock * lock);
    void        (* unlockMutex)(VEGLLock * lock);

    /* Pass context associated EGLConfig */
    void      * config;

    /* EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT value */
    gctBOOL     robustAccess;

    /* EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT value */
    gctINT      resetNotification;

    gctBOOL     debuggable;

    /* EGL_CONTEST_FLAGS.*/
    gctINT      contextFlags;

    /* EGL_EXT_protected_content. */
    gctBOOL     protectedContext;

#if gcdGC355_PROFILER
    gctUINT64   appStartTime;
    gctFILE     apiTimeFile;
#endif

    gctUINT (*getMemoryStatus)(EGLenum);
        /* Error handling */
    void (*warning)(void *gc, const void *fmt);
    void (*fatal)(void *gc, const void *fmt);
    /* Set window's GL api dispatch table */

    void (*setProcTable)(void *gc, void *state);

        /* Drawable mutex lock from gl core */
    void (*lockDrawable)(void *lock);
    void (*unlockDrawable)(void *lock);

    /* GetDC and GetHWND */
    void *(*getHWND)(void *gc);
    void *(*getDC)(void *other);
    void (*releaseDC)(void *other, void *hDC);

    /* Blt back buffer content to front buffer */
    /* This function is called if renderring in front buffer */
    void (*internalSwapBuffers)(void *gc, gctBOOL bSwapFront, gctBOOL finishMode);

    /* Get current display mode */
    void (*getDisplayMode)(gctUINT *width, gctUINT *height, gctUINT *depth);

    /* Get the colorbuffer from the handle of pbuffer */
    void *(*getColorbufferPBuffer)(void  *hpbuffer,EGLenum iBuffer);

    /* Device configuration changed function */
    void (*deviceConfigChanged)(void *gc);

    void (*bltImageToScreen)(void *gc,
                              gctINT bitsAlignedWidth,
                              gctINT bitsAlignedHeight,
                              gctINT bitsPerPixel,
                              gctINT * bits,
                              gctINT left,
                              gctINT top,
                              gctINT width,
                              gctINT height);

    gctINT   version;
    /* Device DLL interface */
    void *device;
    gctUINT deviceIndex;
    /* Operating system dependent data goes here */
    void *other;

    gctBOOL  conformGLSpec;
    gctBOOL  coreProfile;
    gctBOOL  fromEGL;

} VEGLimports;


/******************************************************************************/

/* Private Drawable structure derived from eglSurface, which will be used by Halti driver */
typedef struct EGLDrawableRec
{
    gctPOINTER  config;

    EGLint      width;
    EGLint      height;

    void      * rtHandles[gcdMAX_DRAW_BUFFERS];        /* gco surface handle of render target */
    void      * prevRtHandles[gcdMAX_DRAW_BUFFERS];    /* gco surface handle of previous render target */
    void      * depthHandle;     /* gco surface handle of depth */
    void      * stencilHandle;   /* gco surface handle of stencil */

    void      * accumHandle;

    void      * private;         /* private handle point to API drawable */
    void        (* destroyPrivate)(void *);
} EGLDrawable, *VEGLDrawable;

typedef void * (* veglCREATECONTEXT) (
    void        * Thread,
    gctINT        ClientVersion,
    VEGLimports * Imports,
    gctPOINTER    SharedContext,
    gctINT        SharedContextClient
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

typedef void (* veglFLUSH) (
    void
    );

typedef void (* veglFINISH) (
    void
    );

typedef gcoSURF (* veglGETCLIENTBUFFER) (
    void          * Context,
    EGLClientBuffer Buffer
    );

typedef EGLenum (* veglCREATEIMAGETEXTURE) (
    void     * Context,
    EGLenum    Target,
    gctINT     Texture,
    gctINT     Level,
    gctINT     Depth,
    gctPOINTER Image
    );

typedef EGLenum (* veglCREATEIMAGERENDERBUFFER) (
    void     * Context,
    EGLenum    Renderbuffer,
    gctPOINTER Image
    );

typedef EGLenum (* veglCREATEIMAGEVGPARENTIMAGE) (
    void       * Context,
    unsigned int vgImage,
    void      ** Images,
    int        * Count
    );

typedef EGLenum (* veglBINDTEXIMAGE) (
    void     * Surface,
    EGLenum    Format,
    EGLBoolean Mipmap,
    EGLint     Level,
    EGLint     Width,
    EGLint     Height,
    void    ** Binder
    );

typedef gctBOOL (* veglPROFILER) (
    void    * Profiler,
    gctUINT32 Enum,
    gctHANDLE Value
    );

typedef void (* EGL_PROC)(
    void
    );

typedef EGL_PROC (* veglGETPROCADDR) (
    const char *procname
    );

typedef void (* gcfEGL_DO_SWAPBUFFER)(
    EGLDisplay Dpy,
    EGLSurface Draw
    );

typedef EGLBoolean (* veglSWAPBUFFERS)(
    EGLDisplay           Dpy,
    EGLSurface           Draw,
    gcfEGL_DO_SWAPBUFFER Callback
    );

typedef EGLBoolean (* veglQUERYHWVG) (
    void
    );

typedef void (* veglAPPENDVGRESOLVE) (
    void  * Context,
    gcoSURF Target
    );

typedef struct _veglDISPATCH
{
    veglCREATECONTEXT            createContext;
    veglDESTROYCONTEXT           destroyContext;
    veglMAKECURRENT              makeCurrent;
    veglLOSECURRENT              loseCurrent;
    veglSETDRAWABLE              setDrawable;
    veglFLUSH                    flush;
    veglFINISH                   finish;
    veglGETCLIENTBUFFER          getClientBuffer;
    veglCREATEIMAGETEXTURE       createImageTexture;
    veglCREATEIMAGERENDERBUFFER  createImageRenderbuffer;
    veglCREATEIMAGEVGPARENTIMAGE createImageParentImage;
    veglBINDTEXIMAGE             bindTexImage;
    veglPROFILER                 profiler;
    veglGETPROCADDR              getProcAddr;
    veglSWAPBUFFERS              swapBuffers;
    veglQUERYHWVG                queryHWVG;
    veglAPPENDVGRESOLVE          resolveVG;
}
veglDISPATCH;

#ifdef __cplusplus
}
#endif

#endif /* __gc_egl_common_h_ */
