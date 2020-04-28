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


#ifndef __gc_egl_h_
#define __gc_egl_h_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#if defined(LINUX)
/* Undef some badly named X defines. */
#   undef Status
#   undef Always
#   undef CurrentTime
#endif

#include "gc_hal.h"
#include "gc_hal_priv.h"
#include "gc_hal_driver.h"
#include "gc_hal_engine.h"
#include "gc_egl_common.h"

/* Duplicate 8888 configs to ARGB and ABGR visual. */
#if defined(ANDROID)
#   define VEGL_DUPLICATE_ABGR_EGL_CONFIGS      1
#else
#   define VEGL_DUPLICATE_ABGR_EGL_CONFIGS      0
#endif

/* Enable 4444 and 5551 EGL configs. */
#if defined(ANDROID) || defined(__QNXNTO__)
#   define VEGL_ENABLE_4444_5551_EGL_CONFIGS    0
#else
#   define VEGL_ENABLE_4444_5551_EGL_CONFIGS    1
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef struct eglDisplay         * VEGLDisplay;
typedef struct eglResObj          * VEGLResObj;
typedef struct eglContext         * VEGLContext;
typedef struct eglConfig          * VEGLConfig;
typedef struct eglSurface         * VEGLSurface;
typedef struct eglImage           * VEGLImage;
typedef struct eglSync            * VEGLSync;
typedef struct eglImageRef        * VEGLImageRef;
typedef struct eglThreadData      * VEGLThreadData;
typedef struct eglWorkerInfo      * VEGLWorkerInfo;
typedef struct eglBackBuffer      * VEGLBackBuffer;

/* Wraps up platform operations. */
typedef struct eglPlatform        * VEGLPlatform;

/* Wraps up native display/window/pixmap variables/operations. */

typedef struct eglDisplayInfo     * VEGLDisplayInfo;
typedef struct eglWindowInfo      * VEGLWindowInfo;
typedef struct eglPixmapInfo      * VEGLPixmapInfo;

#define VEGL_DISPLAY(p)             ((VEGLDisplay) (p))
#define VEGL_RESOBJ(p)              ((VEGLResObj)  (p))
#define VEGL_CONTEXT(p)             ((VEGLContext) (p))
#define VEGL_CONFIG(p)              ((VEGLConfig)  (p))
#define VEGL_SURFACE(p)             ((VEGLSurface) (p))
#define VEGL_IMAGE(p)               ((VEGLImage)   (p))
#define VEGL_SYNC(p)                ((VEGLSync)    (p))

#define EGL_DISPLAY_SIGNATURE       gcmCC('E','G','L','D')
#define EGL_SURFACE_SIGNATURE       gcmCC('E','G','L','S')
#define EGL_CONTEXT_SIGNATURE       gcmCC('E','G','L','C')
#define EGL_IMAGE_SIGNATURE         gcmCC('E','G','L','I')
#define EGL_SYNC_SIGNATURE          gcmCC('E','G','L','Y')

#define  __EGL_INVALID_CONFIG__     0

#define MAJOR_API_VER(x)            ((x) >> 4)
#define MINOR_API_VER(x)            ((x) & 0xF)


struct eglResObj
{
    /* Signature. */
    gctUINT32                   signature;

    /* Next eglResObj */
    VEGLResObj                  next;
};

typedef void * EGLResObj;

struct eglRegion
{
    EGLint                      numRects;
    EGLint                      maxNumRects;
    EGLint *                    rects;
};

struct eglBackBuffer
{
    gctPOINTER                  context;
    gcoSURF                     surface;
    gcsPOINT                    origin;
    gctBOOL                     flip;
};

struct eglWorkerInfo
{
    gctSIGNAL                   signal;
    gctSIGNAL                   targetSignal;

    /* Owner of this surface. */
    VEGLSurface                 draw;

    struct eglBackBuffer        backBuffer;

    /* Buffer damage region. */
    struct eglRegion            region;

    /* Surface damage region hints. */
    struct eglRegion            damageHint;

    /* Used by eglDisplay worker list. */
    VEGLWorkerInfo              prev;
    VEGLWorkerInfo              next;
};

#if defined(ANDROID)
#   define EGL_WORKER_COUNT     24
#elif defined(__QNXNTO__)
#   define EGL_WORKER_COUNT     1
#else
#   define EGL_WORKER_COUNT     16
#endif

typedef void (* VEGL_ESPrivDestructor) (gctPOINTER priv);

typedef enum _veglAPIINDEX
{
    vegl_EGL,
    vegl_OPENGL_ES11,
    vegl_OPENGL_ES20,
    vegl_OPENGL,
    vegl_OPENVG,

    vegl_API_LAST,
}
veglAPIINDEX;


struct eglThreadData
{
    /* Extends from gcsDRIVER_TLS structure, must be first field. */
    gcsDRIVER_TLS               base;

    /* Last known error code. */
    EGLenum                     error;

    /* Current API. */
    EGLenum                     api;

    /* Current context of current rendering API
    ** Shortcut to below three sorts of current contexts.
    */
    VEGLContext                 context;
    /* Current context of OES API*/
    VEGLContext                 esContext;
    /* Current context of VG API */
    VEGLContext                 vgContext;
    /* Current context of GL API */
    VEGLContext                 glContext;

    /* Private thread data pointer */
    gctPOINTER                  esPrivate;
    VEGL_ESPrivDestructor       destroyESPrivate;

    struct eglWorkerInfo      * worker;

#if gcdGC355_MEM_PRINT
    gctINT                      fbMemSize;
#endif

    /* Client extension string. */
    char *                      clientExtString;

    /* Dispatch tables of client APIs, non-null means client API available. */
    veglDISPATCH *              dispatchTables[vegl_API_LAST];

    /* Client handles, better to close when exit. */
    gctHANDLE                   clientHandles[vegl_API_LAST];

    /**************************************************************************/
    /* Fields below are hardware relevant. */

    gctINT32                    chipCount;
    gcsHAL_LIMITS               chipLimits[gcdCHIP_COUNT];

    /* Hardware capabilities. */
    gceCHIPMODEL                chipModel;
    EGLint                      maxWidth;
    EGLint                      maxHeight;
    EGLint                      maxSamples;
    gctBOOL                     openVGpipe;

    /* Hardware specific: fastMSAA and small MSAA only support 4x mode */
    gctBOOL                     fastMSAA;

    /* Hardware specific: Security feature. */
    gctBOOL                     security;

    /* A signal associated with a pending Android Native fence, for gbm only */
    gctSIGNAL                     pendingSignal;
};

struct eglImageRef
{
    void *                      pixmap;
    VEGLPixmapInfo              pixInfo;
    gcoSURF                     surface;
#ifdef EGL_API_DRI
    gcoSURF                     tmpSurface;
#endif
    VEGLImageRef                next;
};

struct eglDisplay
{
    /* Next EGLDisplay. */
    VEGLDisplay                 next;

    /* Platform operations. */
    VEGLPlatform                platform;

    /* Native screen and native display. */
    void                      * nativeScreen;
    void                      * nativeDisplay;

    /* Handle to device context. */
    void *                      hdc;

    /* Local info. */
    gctPOINTER                  localInfo;

    /* wayland global. */
    void *                      wl_global;

    gctBOOL                     releaseDpy;

    /* Process handle. */
    gctHANDLE                   process;
    gctHANDLE                   ownerThread;

    /* Number of configurations. */
    EGLint                      configCount;

    /* Pointer to configurations. */
    VEGLConfig                  config;

    /* What's the swap interval range supported */
    EGLint                      minSwapInterval;
    EGLint                      maxSwapInterval;

    /* access lock to access this display */
    gctPOINTER                  accessMutex;

    /* resource lock */
    gctPOINTER                  resourceMutex;

    /* Allocated resources. */
    VEGLSurface                 surfaceStack;
    VEGLContext                 contextStack;
    VEGLImage                   imageStack;
    VEGLImageRef                imageRefStack;
    VEGLSync                    syncStack;

    /* Initialize flag. */
    gctBOOL                     initialized;

    /* Hardware specific: Extension string. */
    char *                      extString;

    /* Worker thread for copying data. */
    gctHANDLE                   workerThread;
    gctSIGNAL                   startSignal;
    gctSIGNAL                   stopSignal;
    gctPOINTER                  suspendMutex;

#if defined(WL_EGL_PLATFORM) || defined(EGL_API_FB) || defined(__GBM__)
    /* Control flag for swap worker on wayland platform. */
    EGLint                      enableClient;
    EGLint                      enableServer;
#endif

    /* EGL Blob Cache Functions */
    EGLGetBlobFuncANDROID       blobCacheGet;
    EGLSetBlobFuncANDROID       blobCacheSet;

    /* Sentinel for a list of active workers that are queued up. */
    struct eglWorkerInfo        workerSentinel;

#if defined(ANDROID)
    /* Bool for EGL_ANDROID_framebuffer_target */
    gctBOOL                     supportFBTarget;
#endif
};

struct eglConfig
{
    EGLint                      bufferSize;
    EGLint                      configBufferSize;
    EGLint                      alphaSize;
    EGLint                      blueSize;
    EGLint                      greenSize;
    EGLint                      redSize;
    EGLint                      depthSize;
    EGLint                      stencilSize;
    EGLenum                     configCaveat;
    EGLint                      configId;
    EGLBoolean                  defaultConfig;
    EGLBoolean                  nativeRenderable;
    EGLint                      nativeVisualType;
    EGLint                      samples;
    EGLint                      sampleBuffers;
    EGLenum                     surfaceType;
    EGLBoolean                  bindToTetxureRGB;
    EGLBoolean                  bindToTetxureRGBA;
    EGLint                      luminanceSize;
    EGLint                      alphaMaskSize;
    EGLenum                     colorBufferType;
    EGLenum                     renderableType;
    EGLenum                     conformant;
    EGLenum                     matchFormat;
    EGLint                      matchNativePixmap;
    EGLint                      width;
    EGLint                      height;
    EGLint                      level;
    EGLint                      minSwapInterval;
    EGLint                      maxSwapInterval;
    EGLenum                     transparentType;
    EGLint                      transparentRedValue;
    EGLint                      transparentGreenValue;
    EGLint                      transparentBlueValue;
    /* Special config on android to return BGRA visual. */
    EGLBoolean                  swizzleRB;
    /* EGL_ANDROID_recordable extension. */
    EGLBoolean                  recordableConfig;

    EGLint rgbMode;
    EGLint rgbFloatMode;
    EGLint doubleBufferMode;
    EGLint tripleBufferMode;
    EGLint stereoMode;
    EGLint haveAccumBuffer;
    EGLint haveDepthBuffer;
    EGLint haveStencilBuffer;

    EGLint redMask, greenMask, blueMask, alphaMask;

    EGLint  accumBits; /*total accumulation buffer bits */
    EGLint  accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits;


    EGLint  numAuxBuffers;

#if defined(ANDROID)
    /* Bool for EGL_ANDROID_framebuffer_target */
    gctBOOL                     supportFBTarget;
    EGLint                      colorComponentType;
#endif
};

enum _VEGL_DIRECT_RENDERING_MODE
{
    VEGL_INDIRECT_RENDERING = 0,

    /* no-resolve, with tile status, but fc filled after swap. */
    VEGL_DIRECT_RENDERING_FCFILL,

    /* special no-resolve, do not allocate tile status. */
    VEGL_DIRECT_RENDERING_NOFC,

    /* no-resolve, with tile status, no compression. */
    VEGL_DIRECT_RENDERING_FC_NOCC,

    /* full no-resolve mode, tile status, compression. */
    VEGL_DIRECT_RENDERING
};

struct eglSurface
{
    /* Should be the first member */
    struct eglResObj            resObj;

    /* For buffer age. */
    EGLBoolean                   initialFrame;

    /*
     * Set if VG pipe is present and this surface was created when the current
     * API was set to OpenVG
     */
    gctBOOL                     openVG;

    /* Render target. */
    gcoSURF                     renderTarget;
    gceSURF_FORMAT              renderTargetFormat;
    gctBOOL                     rtFormatChanged;

    /* Previous render target. */
    gcoSURF                     prevRenderTarget;

    /*
     * Damage for this surface. For EGL_KHR_partial_update,
     * This is only useful for indirect render.
     * For direct render, we ignore this damage range.
     * For indirect render, we always report buffer age == 1 (initial buffer age is 0).
     * use this damage range to compute the difference of back buffer to internal RT
     */
    struct eglRegion            damage[EGL_WORKER_COUNT];
    gctUINT                     curDamage;

    EGLBoolean                  damageValid;
    EGLBoolean                  queriedAge;

    /* Depth buffer. */
    gcoSURF                     depthBuffer;
    gceSURF_FORMAT              depthFormat;

    /* For "render to texture" To bind texture. */
    gcoSURF                     texBinder;

    /* Surface Config Data. */
    struct eglConfig            config;

    /* Reference counter. */
    gcsATOM_PTR                 reference;

    /* Surface type. */
    EGLint                      type;
    EGLenum                     buffer;
    gceSURF_COLOR_TYPE          colorType;
    EGLint                      swapBehavior;
    EGLint                      multisampleResolve;
    EGLBoolean                  protectedContent;

    /* VG attribute */
    EGLenum                     vgColorSpace;
    EGLenum                     vgAlphaFormat;


    /* Window attributes. */
    void *                      hwnd;
    EGLBoolean                  bound;

    struct eglBackBuffer        backBuffer;

    /*
     * Wrap up native window specific variables/operations.
     * Do not access/modify outside egl platform layer.
     */
    VEGLWindowInfo              winInfo;


    /* Pixmap attributes. */
    void *                      pixmap;
    gcoSURF                     pixmapSurface;

    /* Wrap up native pixmap specific variables/operations. */
    VEGLPixmapInfo              pixInfo;

    /* PBuffer attributes. */
    EGLBoolean                  largestPBuffer;
    EGLBoolean                  mipmapTexture;
    EGLint                      mipmapLevel;
    EGLenum                     textureFormat;
    EGLenum                     textureTarget;


    /* Lock surface attributes. */
    EGLBoolean                  locked;
    gcoSURF                     lockBuffer;
    void *                      lockBits;
    gceSURF_FORMAT              lockBufferFormat;
    gctINT                      lockBufferStride;
    EGLBoolean                  lockPreserve;

    /*
     * To temporarily preserve the lockBuffer pixels when there's no
     * renderTarget. Render target is defered allocated for performance.
     */
    gcoSURF                     lockBufferMirror;

    /* Render mode and swap model, valid for window surface for now. */
    EGLint                      renderMode;
    EGLBoolean                  newSwapModel;

    /* Image swap workers. */
    struct eglWorkerInfo        workers[EGL_WORKER_COUNT];

    gctUINT                     totalWorkerCount;
    gctUINT                     freeWorkerCount;

    VEGLWorkerInfo              availableWorkers;
    VEGLWorkerInfo              lastSubmittedWorker;
    gctPOINTER                  workerMutex;

    /* Signaled when at least one worker is available. */
    gctSIGNAL                   workerAvaiableSignal;

    /* Signaled when all workers are free, ie, no worker in workerThread. */
    gctSIGNAL                   workerDoneSignal;

    /*
     * Rectangles to resolve out.
     * 1. Swap region
     * 2. clip with surface size
     * 3. clip with damage region (EGL_KHR_partial_update)
     * 4. clip with swap rect (obsolete EGL_ANDROID_swap_rectange)
     *
     * NOTICE:
     * It's safe to enlarge the rects when resolve out.
     *
     * clipRegion::rects: 4 elements for one group, each representing a single
     * rectangle in surface coordinates in the form {x, y, width, height}.
     */
    struct eglRegion            clipRegion;

    /* EGL_KHR_swap_buffers_with_damage. */
    struct eglRegion            damageHint;

#if defined(ANDROID)
    /* EGL_ANDROID_set_swap_rectangle. */
    struct
    {
        /* Current swap rectangle. */
        EGLint                  x;
        EGLint                  y;
        EGLint                  width;
        EGLint                  height;
    }
    swapRect;

    /* EGL_ANDROID_get_render_buffer. */
    /* Skip resolve if hwc composition is used. */
    EGLBoolean                  skipResolve;
#endif

    /* drawable handle which owned by API driver */
    EGLDrawable                 drawable;
};

struct eglImage
{
    /* Internal image struct pointer. Should be the first one */
    khrEGL_IMAGE                image;

    /* Signature. */
    gctUINT                     signature;

    /* Owner display. */
    VEGLDisplay                 display;

    /* Destroyed by eglDestroyImage.*/
    EGLBoolean                  destroyed;

    /* Reference counter. */
    gcsATOM_PTR                 reference;

    /* Protected Content */
    EGLBoolean                  protectedContent;

    /* Next eglImage. */
    VEGLImage                   next;
};

struct eglContext
{
    /* Should be the first member */
    struct eglResObj            resObj;

    /* Bounded thread. */
    VEGLThreadData              thread;

    /* Bounded API. */
    EGLenum                     api;
    EGLint                      client;

    /* Attached display. */
    VEGLDisplay                 display;

    /* Context Config Data. */
    struct eglConfig            config;

    /* Shared configuration. */
    VEGLContext                 sharedContext;

    /* Current read/draw surface */
    VEGLSurface                 read;
    VEGLSurface                 draw;

    /* Context for API. */
    void *                      context;

    /* Delete requested */
    gctBOOL                     deleteReq;

    /* EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR set when creating context */
    EGLint                      flags;

    /* EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT value */
    gctBOOL                     robustAccess;

    /* EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT value */
    gctINT                      resetNotification;

    /* Context protection, EGL_EXT_protected_content. */
    EGLBoolean                  protectedContent;

    EGLBoolean                  coreProfile;

#if gcdGC355_PROFILER
    gctUINT64                   appStartTime;
    gctFILE                     apiTimeFile;
#endif
};

struct eglSync
{
    /* Should be the first member */
    struct eglResObj            resObj;

    /* Type */
    EGLenum                     type;

    /* Type */
    EGLenum                     condition;

    /* Signal */
    gctSIGNAL                   signal;

#if defined(__linux__)
    /* Native sync fence fd. */
    EGLint                      fenceFD;
#endif
};

struct eglExtension
{
    const char *                string;
    EGLBoolean                  enabled;
};

typedef enum _VEGL_EXTID
{
    VEGL_EXTID_KHR_fence_sync,
    VEGL_EXTID_KHR_reusable_sync,
    VEGL_EXTID_KHR_wait_sync,
    VEGL_EXTID_KHR_image,
    VEGL_EXTID_KHR_image_base,
    VEGL_EXTID_KHR_image_pixmap,
    VEGL_EXTID_KHR_gl_texture_2D_image,
    VEGL_EXTID_KHR_gl_texture_cubemap_image,
    VEGL_EXTID_KHR_gl_renderbuffer_image,
    VEGL_EXTID_EXT_image_dma_buf_import,
    VEGL_EXTID_EXT_image_dma_buf_import_modifiers,
    VEGL_EXTID_KHR_lock_surface,
    VEGL_EXTID_KHR_create_context,
    VEGL_EXTID_KHR_no_config_context,
    VEGL_EXTID_KHR_surfaceless_context,
    VEGL_EXTID_KHR_get_all_proc_addresses,
    VEGL_EXTID_EXT_create_context_robustness,
    VEGL_EXTID_EXT_protected_surface,
    VEGL_EXTID_EXT_protected_content,
    VEGL_EXTID_EXT_buffer_age,
    VEGL_EXTID_ANDROID_image_native_buffer,
    VEGL_EXTID_ANDROID_swap_rectangle,
    VEGL_EXTID_ANDROID_blob_cache,
    VEGL_EXTID_ANDROID_recordable,
    VEGL_EXTID_ANDROID_native_fence_sync,
    VEGL_EXTID_WL_bind_wayland_display,
    VEGL_EXTID_WL_create_wayland_buffer_from_image,
    VEGL_EXTID_KHR_partial_update,
    VEGL_EXTID_EXT_swap_buffers_with_damage,
    VEGL_EXTID_KHR_swap_buffers_with_damage,

    VEGL_EXTID_COUNT,
}
VEGL_EXTID;

typedef enum _VEGL_COLOR_FORMAT
{
    VEGL_DEFAULT    = (1 << 0),
    VEGL_ALPHA      = (1 << 1),
    VEGL_444        = (1 << 2),
    VEGL_555        = (1 << 3),
    VEGL_565        = (1 << 4),
    VEGL_888        = (1 << 5),
    VEGL_YUV        = (1 << 6),

    VEGL_4444       = VEGL_ALPHA | VEGL_444,
    VEGL_5551       = VEGL_ALPHA | VEGL_555,
    VEGL_8888       = VEGL_ALPHA | VEGL_888
}
VEGL_COLOR_FORMAT;

typedef struct EGL_CONFIG_COLOR
{
    EGLint                      bufferSize;
    EGLint                      configBufferSize;

    EGLint                      redSize;
    EGLint                      greenSize;
    EGLint                      blueSize;
    EGLint                      alphaSize;

    VEGL_COLOR_FORMAT           formatFlags;
}
* VEGLConfigColor;

typedef struct EGL_CONFIG_DEPTH
{
    int depthSize;
    int stencilSize;
}
* VEGLConfigDepth;

typedef struct _veglClientApiEntry
{
    const char *                                name;
    __eglMustCastToProperFunctionPointerType    function;
}
veglClientApiEntry;

typedef struct _veglCommonEsApiDispatch
{
    const char *                                name;
    __eglMustCastToProperFunctionPointerType    es11func;
    __eglMustCastToProperFunctionPointerType    es2xfunc;
}
veglCommonEsApiDispatch;

/*******************************************************************************
** Thread API functions.
*/

VEGLThreadData
veglGetThreadData(
    void
    );

/*
 * GetThreadData will not initialize hardware relevant data.
 * Use this function after GetThreadData if hardware initialization required.
 */
EGLBoolean
veglInitDeviceThreadData(
    VEGLThreadData Thread
    );

/*******************************************************************************
** Display API functions.
*/

void
veglDereferenceDisplay(
    IN VEGLThreadData Thread,
    IN VEGLDisplay    Display,
    EGLBoolean Always
    );

VEGLDisplay
veglGetDisplay(
    IN EGLDisplay Dpy
    );

/*
 * Sync EGL objects to native objects.
 * This function is asyncrhonized.
 */
void
veglSyncNative(
    IN VEGLThreadData Thread,
    IN VEGLDisplay    Display
    );

/*******************************************************************************
** Surface API functions.
*/

EGLBoolean
veglReferenceSurface(
    IN VEGLThreadData Thread,
    IN VEGLDisplay    Display,
    IN VEGLSurface    Surface
    );

void
veglDereferenceSurface(
    IN VEGLThreadData Thread,
    IN VEGLDisplay    Display,
    IN VEGLSurface    Surface,
    IN EGLBoolean     Always
    );

EGLBoolean
veglDestroySurface(
    IN EGLDisplay Dpy,
    IN EGLSurface Surface
    );

EGLBoolean
veglCreateRenderTarget(
    IN VEGLThreadData Thread,
    IN VEGLSurface    Surface
    );

EGLint
veglResizeSurface(
    IN VEGLDisplay Display,
    IN VEGLSurface Surface,
    IN gctUINT     Width,
    IN gctUINT     Height
    );

/*******************************************************************************
** Context API functions.
*/
EGLBoolean
veglDestroyContext(
    IN EGLDisplay Dpy,
    IN EGLContext Ctx
    );

EGLBoolean
veglReleaseThread(
    IN VEGLThreadData Thread
    );

/*******************************************************************************
** eglImage API functions.
*/
void
veglDestroyImage(
    IN VEGLThreadData Thread,
    IN VEGLDisplay    Display,
    IN VEGLImage      Image
    );

void
veglReferenceImage(
    IN VEGLThreadData Thread,
    IN VEGLDisplay    Display,
    IN VEGLImage      Image
    );

void
veglDereferenceImage(
    IN VEGLThreadData Thread,
    IN VEGLDisplay    Display,
    IN VEGLImage      Image
    );

/*******************************************************************************
** eglSync API functions.
*/
EGLBoolean
veglDestroySync(
    IN EGLDisplay Dpy,
    IN EGLSync    Sync
    );

/*******************************************************************************
** Swap worker API functions.
*/

void
veglSuspendSwapWorker(
    VEGLDisplay Display
    );

void
veglResumeSwapWorker(
    VEGLDisplay Display
    );

VEGLWorkerInfo
veglGetWorker(
    IN VEGLThreadData Thread,
    IN VEGLDisplay    Display,
    IN VEGLSurface    Surface
    );

VEGLWorkerInfo
veglFreeWorker(
    VEGLWorkerInfo Worker
    );

gctBOOL
veglSubmitWorker(
    IN VEGLThreadData Thread,
    IN VEGLDisplay    Display,
    IN VEGLWorkerInfo Worker,
    IN gctBOOL        ScheduleSignals
    );

/*******************************************************************************
** Misc functions.
*/

void
veglSetEGLerror(
    VEGLThreadData Thread,
    EGLint         Error
    );

void
veglGetFormat(
    IN VEGLThreadData    Thread,
    IN VEGLConfig        Config,
    OUT gceSURF_FORMAT * RenderTarget,
    OUT gceSURF_FORMAT * DepthBuffer
    );

VEGLResObj
veglGetResObj(
    IN VEGLDisplay  Dpy,
    IN VEGLResObj * pResHead,
    IN EGLResObj    ResObj,
    IN gctUINT      ResSig
    );

void
veglPushResObj(
    IN VEGLDisplay     Dpy,
    INOUT VEGLResObj * pResHead,
    IN VEGLResObj      ResObj
    );

void
veglPopResObj(
    IN VEGLDisplay     Dpy,
    INOUT VEGLResObj * pResHead,
    IN VEGLResObj      ResObj
    );

EGLsizeiANDROID
veglGetBlobCache(
    const void    * key,
    EGLsizeiANDROID keySize,
    void          * value,
    EGLsizeiANDROID valueSize
    );

void
veglSetBlobCache(
    const void    * key,
    EGLsizeiANDROID keySize,
    const void    * value,
    EGLsizeiANDROID valueSize
    );

#if defined(_WIN32)
#define veglTHREAD_RETURN DWORD
#else
#define veglTHREAD_RETURN void *
#endif

#ifndef WINAPI
#define WINAPI
#endif

veglTHREAD_RETURN
WINAPI
veglSwapWorker(
    void * Display
    );

gctHANDLE
veglGetModule(
    IN gcoOS           Os,
    IN veglAPIINDEX    Index,
    IN gctCONST_STRING Name,
    IN veglDISPATCH ** Dispatch
    );

/*******************************************************************************
** Bridge functions.
*/

veglDISPATCH *
_GetDispatch(
    VEGLThreadData Thread,
    VEGLContext    Context
    );

void *
_CreateApiContext(
    VEGLThreadData Thread,
    VEGLContext    Context,
    VEGLConfig     Config,
    void         * SharedContext,
    EGLint         SharedContextClient
    );

EGLBoolean
_DestroyApiContext(
    VEGLThreadData Thread,
    VEGLContext    Context,
    void         * ApiContext
    );

EGLBoolean
_SetDrawable(
    VEGLThreadData Thread,
    VEGLContext    Context,
    VEGLDrawable   Draw,
    VEGLDrawable   Read
    );

EGLBoolean
_Flush(
    VEGLThreadData Thread
    );

EGLBoolean
_Finish(
    VEGLThreadData Thread
    );

EGLBoolean
_ProfilerCallback(
    VEGLThreadData Thread,
    IN gctUINT32   Enum,
    IN gctHANDLE   Value
    );

gcoSURF
_GetClientBuffer(
    VEGLThreadData Thread,
    void         * Context,
    EGLClientBuffer buffer
    );

EGLenum
_BindTexImage(
    VEGLThreadData Thread,
    gcoSURF        Surface,
    EGLenum        Format,
    EGLBoolean     Mipmap,
    EGLint         Level,
    EGLint         Width,
    EGLint         Height,
    gcoSURF      * BindTo
    );

EGLenum
_CreateImageTexture(
    VEGLThreadData Thread,
    VEGLContext    Context,
    EGLenum        Target,
    gctINT         Texture,
    gctINT         Level,
    gctINT         Depth,
    gctPOINTER     Image
    );

EGLenum
_CreateImageFromRenderBuffer(
    VEGLThreadData Thread,
    VEGLContext    Context,
    gctUINT        Framebuffer,
    gctPOINTER     Image
    );

EGLenum
_CreateImageFromVGParentImage(
    VEGLThreadData Thread,
    VEGLContext    Context,
    unsigned int   vgimage_obj,
    VEGLImage      eglimage
    );

EGLenum
_CreateImageFromANativeBuffer(
    VEGLThreadData Thread,
    EGLClientBuffer Buffer,
    VEGLImage Image
    );

EGLBoolean
_IsExtSuppored(
    EGLenum extId
    );

#define VEGL_TRACE_API(func) \
    if (veglTracerDispatchTable.func)(*veglTracerDispatchTable.func)

#define VEGL_TRACE_API_PRE(func) \
    if (veglTracerDispatchTable.func##_pre)(*veglTracerDispatchTable.func##_pre)

#define VEGL_TRACE_API_POST(func) \
    if (veglTracerDispatchTable.func##_post)(*veglTracerDispatchTable.func##_post)

#define VEGL_LOCK_DISPLAY(dpy)   \
    if (dpy->accessMutex) \
    {\
        gcoOS_AcquireMutex(gcvNULL, dpy->accessMutex, gcvINFINITE);\
    }\

#define VEGL_UNLOCK_DISPLAY(dpy)  \
    if (dpy->accessMutex) \
    {\
        gcoOS_ReleaseMutex(gcvNULL, dpy->accessMutex);\
    }\


#define VEGL_LOCK_DISPLAY_RESOURCE(dpy)   \
    if (dpy->resourceMutex) \
    {\
        gcoOS_AcquireMutex(gcvNULL, dpy->resourceMutex, gcvINFINITE);\
    }\

#define VEGL_UNLOCK_DISPLAY_RESOURCE(dpy)  \
    if (dpy->resourceMutex) \
    {\
        gcoOS_ReleaseMutex(gcvNULL, dpy->resourceMutex);\
    }\


/* EGL Tracer Dispatch Function Table */
typedef struct
{
    /* Note:
    ** The following 36 interfaces are used to link with vTracer EGL entry functions in libGLES_vlogger.so.
    ** So the following interfaces must match with vTracer EGL entry functions.
    */
    EGLint     (* GetError_pre)(void);
    EGLDisplay (* GetDisplay_post)(EGLNativeDisplayType display_id, EGLDisplay ret_dpy);
    EGLBoolean (* Initialize)(EGLDisplay dpy, EGLint *major, EGLint *minor);
    EGLBoolean (* Terminate)(EGLDisplay dpy);
    const char * (* QueryString_post)(EGLDisplay dpy, EGLint name, const char* str);
    EGLBoolean (* GetConfigs_pre)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean (* ChooseConfig_pre)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean (* GetConfigAttrib_post)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
    EGLSurface (* CreateWindowSurface_post)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list, EGLSurface ret_surface);
    EGLSurface (* CreatePbufferSurface_post)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list, EGLSurface ret_surface);
    EGLSurface (* CreatePixmapSurface_post)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list, EGLSurface ret_surface);
    EGLBoolean (* DestroySurface)(EGLDisplay dpy, EGLSurface surface);
    EGLBoolean (* QuerySurface_pre)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
    EGLBoolean (* BindAPI)(EGLenum api);
    EGLenum    (* QueryAPI_pre)(void);
    EGLBoolean (* WaitClient)(void);
    EGLBoolean (* ReleaseThread)(void);
    EGLSurface (* CreatePbufferFromClientBuffer_post)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list, EGLSurface ret_surface);
    EGLBoolean (* SurfaceAttrib)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
    EGLBoolean (* BindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
    EGLBoolean (* ReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
    EGLBoolean (* SwapInterval)(EGLDisplay dpy, EGLint interval);
    EGLContext (* CreateContext_post)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list, EGLContext ret_ctx);
    EGLBoolean (* DestroyContext)(EGLDisplay dpy, EGLContext ctx);
    EGLBoolean (* MakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
    EGLContext (* GetCurrentContext_post)(EGLContext ret_ctx);
    EGLSurface (* GetCurrentSurface_post)(EGLint readdraw, EGLSurface ret_surface);
    EGLDisplay (* GetCurrentDisplay_post)(EGLDisplay ret_dpy);
    EGLBoolean (* QueryContext_pre)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
    EGLBoolean (* WaitGL)(void);
    EGLBoolean (* WaitNative)(EGLint engine);
    EGLBoolean (* SwapBuffers)(EGLDisplay dpy, EGLSurface surface);
    EGLBoolean (* CopyBuffers)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
    void (* GetProcAddress_pre)(const char *procname);

    /* EGL 1.5 */
    EGLSync    (* CreateSync_post)(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list, EGLSync ret_sync);
    EGLBoolean (* DestroySync)(EGLDisplay dpy, EGLSync sync);
    EGLint     (* ClientWaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout);
    EGLBoolean (* GetSyncAttrib_post)(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value, EGLAttrib ret_value);
    EGLImage   (* CreateImage_post)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list, EGLImage ret_image);
    EGLBoolean (* DestroyImage)(EGLDisplay dpy, EGLImage image);
    EGLDisplay (* GetPlatformDisplay_post)(EGLenum platform, void *native_display, const EGLAttrib *attrib_list, EGLDisplay ret_dpy);
    EGLSurface (* CreatePlatformWindowSurface_post)(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list, EGLSurface ret_surface);
    EGLSurface (* CreatePlatformPixmapSurface_post)(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list, EGLSurface ret_surface);
    EGLBoolean (* WaitSync)(EGLDisplay dpy, EGLSync sync, EGLint flags);

    /* EGL_KHR_lock_surface. */
    EGLBoolean (* LockSurfaceKHR)(EGLDisplay dpy, EGLSurface surface, const EGLint *attrib_list);
    EGLBoolean (* UnlockSurfaceKHR)(EGLDisplay dpy, EGLSurface surface);

    /* EGL_KHR_image. */
    EGLImageKHR (* CreateImageKHR_post)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list, EGLImageKHR ret_image);
    EGLBoolean (* DestroyImageKHR)(EGLDisplay dpy, EGLImageKHR image);

    /* EGL_KHR_fence_sync. */
    EGLSyncKHR (* CreateSyncKHR_post)(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list, EGLSyncKHR ret_sync);
    EGLBoolean (* DestroySyncKHR)(EGLDisplay dpy, EGLSyncKHR sync);
    EGLint     (* ClientWaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
    EGLBoolean (* GetSyncAttribKHR_pre)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value);

    /* EGL_KHR_wait_sync. */
    EGLint     (* WaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags);

    /* EGL_KHR_reusable_sync. */
    EGLBoolean (* SignalSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLenum mode);

    /* EGL_ANDROID_native_fence_sync */
    EGLint     (* DupNativeFenceFDANDROID_post)(EGLDisplay dpy, EGLSyncKHR sync, EGLint ret_fd);

    /* EGL_EXT_platform_base. */
    EGLDisplay (* GetPlatformDisplayEXT_post)(EGLenum platform, void *native_display, const EGLint *attrib_list, EGLDisplay ret_dpy);
    EGLSurface (* CreatePlatformWindowSurfaceEXT_post)(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list, EGLSurface ret_surface);
    EGLSurface (* CreatePlatformPixmapSurfaceEXT_post)(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list, EGLSurface ret_surface);

    /* EGL_KHR_partial_update */
    EGLBoolean (* SetDamageRegionKHR)(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);

    /* EGL_KHR_swap_buffers_with_damage. */
    EGLBoolean (* SwapBuffersWithDamageKHR)(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);
    EGLBoolean (* SwapBuffersWithDamageEXT)(EGLDisplay dpy, EGLSurface surface, EGLint *rects, EGLint n_rects);

    /* EGL_EXT_image_dma_buf_import_modifiers */
    EGLBoolean (* QueryDmaBufFormatsEXT)(EGLDisplay dpy, EGLint max_formats, EGLint *formats, EGLint *num_formats);
    EGLBoolean (* QueryDmaBufModifiersEXT)(EGLDisplay dpy, EGLint format, EGLint max_modifiers, EGLuint64KHR *modifiers, EGLBoolean *external_only, EGLint *num_modifiers);

    /******  The above interfaces are used to link with external vTracer library libGLES_vlogger.so ******/

    EGLDisplay (* GetDisplay_pre)(EGLNativeDisplayType display_id);
    EGLBoolean (* GetConfigAttrib_pre)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
    EGLSurface (* CreateWindowSurface_pre)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
    EGLSurface (* CreatePbufferSurface_pre)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
    EGLSurface (* CreatePixmapSurface_pre)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
    EGLSurface (* CreatePbufferFromClientBuffer_pre)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
    EGLContext (* CreateContext_pre)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
    EGLContext (* GetCurrentContext_pre)();
    EGLSurface (* GetCurrentSurface_pre)(EGLint readdraw);
    EGLDisplay (* GetCurrentDisplay_pre)();
    void (* GetProcAddress_post)(const char *procname, __eglMustCastToProperFunctionPointerType func);

    EGLint     (* GetError_post)(EGLint err);
    const char * (* QueryString_pre)(EGLDisplay dpy, EGLint name);
    EGLBoolean (* GetConfigs_post)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean (* ChooseConfig_post)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
    EGLBoolean (* QuerySurface_post)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
    EGLenum    (* QueryAPI_post)(EGLenum api);
    EGLBoolean (* QueryContext_post)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);

    /* EGL 1.5 */
    EGLSync    (* CreateSync_pre)(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list);
    EGLBoolean (* GetSyncAttrib_pre)(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value);
    EGLImage   (* CreateImage_pre)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list);
    EGLDisplay (* GetPlatformDisplay_pre)(EGLenum platform, void *native_display, const EGLAttrib *attrib_list);
    EGLSurface (* CreatePlatformWindowSurface_pre)(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list);
    EGLSurface (* CreatePlatformPixmapSurface_pre)(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list);

    /* EGL_KHR_image */
    EGLImageKHR (* CreateImageKHR_pre)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);

    /* EGL_KHR_fence_sync*/
    EGLSyncKHR (* CreateSyncKHR_pre)(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list);
    EGLBoolean (* GetSyncAttribKHR_post)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value, EGLint ret_value);

    /* EGL_ANDROID_native_fence_sync */
    EGLint     (* DupNativeFenceFDANDROID_pre)(EGLDisplay dpy, EGLSyncKHR sync);

    /* EGL_EXT_platform_base. */
    EGLDisplay (* GetPlatformDisplayEXT_pre)(EGLenum platform, void *native_display, const EGLint *attrib_list);
    EGLSurface (* CreatePlatformWindowSurfaceEXT_pre)(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLint *attrib_list);
    EGLSurface (* CreatePlatformPixmapSurfaceEXT_pre)(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLint *attrib_list);
}
eglTracerDispatchTableStruct;

EGLBoolean
veglInitTracerDispatchTable(
    void
    );

extern eglTracerDispatchTableStruct veglTracerDispatchTable;
extern eglTracerDispatchTableStruct veglLogFunctionTable;
extern gceTRACEMODE veglTraceMode;

#ifdef __cplusplus
}
#endif

#endif /* __gc_egl_h_ */
