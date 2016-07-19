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


#ifndef __gl_core_h_
#define __gl_core_h_

#include <sys/types.h>
#include "bldflags.h"
#ifdef _LINUX_
#include "win2unix.h"
#endif

/* Decide big endian or little endian
** Add more support to little endian if possible
** If big endian, it should be not defined
*/
#if defined(__arm__) || defined(i386) || defined(__i386__) || defined(__x86__) || defined(_M_IX86)\
 || defined(__x86_64__) || defined(_M_AMD64) || defined (_M_X64) || defined(__e2k__) || defined(_LITTLE_ENDIAN)
#define USE_LENDIAN
#endif

/*
** This file defines the interface between the GL core and the surrounding
** "operating system" that supports it (currently the GLX or WGL extensions).
**
** Members (data and function pointers) are documented as imported or
** exported according to how they are used by the core rendering functions.
** Imported members are initialized by the "operating system" and used by
** the core functions. Exported members are initialized by the core functions
** and used by the "operating system".
*/

typedef struct __GLcontextRec __GLcontext;
typedef struct __GLmipMapLevelRec  __GLmipMapLevel;
typedef struct __GLtexelRec __GLtexel;
typedef struct __GLtextureObjectRec __GLtextureObject;
typedef struct __GLdispatchStateRec __GLdispatchState;

typedef GLuint * GLuint_ptr;
#ifdef _LINUX_
typedef struct __GLscreenPrivateRec {

    /* Point to device specific information that returned from XF86DRIGetDeviceInfo() */
    GLvoid *pDevInfo;

    /* Drm file descriptor */
    GLint fd;

    /* Frame buffer base virtual address */
    GLvoid *baseFBLinearAddress;
    /* screen information */
    GLvoid *baseFBPhysicalAddress;
    GLint stride;
    GLint width;
    GLint height;

    /* Point to DP private data structure */
    GLvoid *privateData;

} __GLscreenPrivate;
#endif

/*
** Mode and limit information for a context. This information is
** kept around in the context so that values can be used during
** command execution, and for returning information about the
** context to the application.
*/
typedef struct __GLcontextModesRec {
    GLuint rgbMode;
    GLuint rgbFloatMode;
    GLuint doubleBufferMode;
    GLuint tripleBufferMode;
    GLuint stereoMode;
    GLuint haveAccumBuffer;
    GLuint haveDepthBuffer;
    GLuint haveStencilBuffer;

    /*multisample extend*/
    union
    {
        GLint  sampleQuality;
        GLint  samples;
    };

    union
    {
        GLuint haveMultiSampleBuffer;
        GLuint sampleBuffers;
    };

    GLint  redBits, greenBits, blueBits, alphaBits;
    GLuint redMask, greenMask, blueMask, alphaMask;
    GLint  rgbaBits; /* total bits for rgba */

    GLint  accumBits; /*total accumulation buffer bits */
    GLint  accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits;
    GLint  depthBits;
    GLint  stencilBits;

    GLint  numAuxBuffers;

    GLint  level;

#ifdef _LINUX_
    GLuint colorIndexMode;
    GLint indexBits;

    GLint  pixmapMode;

    /* GLX */
    GLint  visualID;
    GLint  visualType;

    /* visual_rating / GLX 1.2 */
    GLint  visualRating;

    /* visual_info / GLX 1.2 */
    GLint  transparentPixel;
    GLint  transparentRed, transparentGreen, transparentBlue, transparentAlpha;
    GLint  transparentIndex;

    /* fbconfig / GLX 1.3 */
    GLint  drawableType;
    GLint  renderType;
    GLint  xRenderable;
    GLint  fbconfigID;

    /* pbuffer / GLX 1.3 */
    GLint  maxPbufferWidth;
    GLint  maxPbufferHeight;
    GLint  maxPbufferPixels;
    GLint  optimalPbufferWidth, optimalPbufferHeight;  /* for SGIX_pbuffer */

    GLint  screen;

    GLint swapMethod;

    struct __GLcontextModesRec *next;
#endif

} __GLcontextModes;


/* all possible device supported buffer format */
typedef enum {
    __GL_DEVFMT_ALPHA8,
    __GL_DEVFMT_ALPHA16,
    __GL_DEVFMT_ALPHA24,
    __GL_DEVFMT_ALPHA32F,

    __GL_DEVFMT_LUMINANCE8,
    __GL_DEVFMT_LUMINANCE16,
    __GL_DEVFMT_LUMINANCE24,
    __GL_DEVFMT_LUMINANCE32F,

    __GL_DEVFMT_INTENSITY8,
    __GL_DEVFMT_INTENSITY16,
    __GL_DEVFMT_INTENSITY24,
    __GL_DEVFMT_INTENSITY32F,

    __GL_DEVFMT_LUMINANCE_ALPHA4,
    __GL_DEVFMT_LUMINANCE_ALPHA8,
    __GL_DEVFMT_LUMINANCE_ALPHA16,

    __GL_DEVFMT_BGR565,
    __GL_DEVFMT_BGRA4444,
    __GL_DEVFMT_BGRA5551,
    __GL_DEVFMT_BGRA8888,
    __GL_DEVFMT_RGBA16,
    __GL_DEVFMT_BGRX8888,
    __GL_DEVFMT_BGRA1010102,
    __GL_DEVFMT_Z16,
    __GL_DEVFMT_Z24,
    __GL_DEVFMT_Z24_STENCIL,
    __GL_DEVFMT_Z32F,
    __GL_DEVFMT_COMPRESSED_RGB_DXT1,
    __GL_DEVFMT_COMPRESSED_RGBA_DXT1,
    __GL_DEVFMT_COMPRESSED_RGBA_DXT3,
    __GL_DEVFMT_COMPRESSED_RGBA_DXT5,
    __GL_DEVFMT_ZHIGH24,
    __GL_DEVFMT_ZHIGH24_STENCIL,
    __GL_DEVFMT_COMPRESSED_LUMINANCE_LATC1,
    __GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_LATC1,
    __GL_DEVFMT_COMPRESSED_LUMINANCE_ALPHA_LATC2,
    __GL_DEVFMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2,
    __GL_DEVFMT_COMPRESSED_RED_RGTC1,
    __GL_DEVFMT_COMPRESSED_SIGNED_RED_RGTC1,
    __GL_DEVFMT_COMPRESSED_RED_GREEN_RGTC2,
    __GL_DEVFMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2,
    __GL_DEVFMT_RGBA8888,
    __GL_DEVFMT_RGBA8888_SIGNED,
    __GL_DEVFMT_RGBA32UI,
    __GL_DEVFMT_RGBA32I,
    __GL_DEVFMT_RGBA16UI,
    __GL_DEVFMT_RGBA16I,
    __GL_DEVFMT_RGBA8UI,
    __GL_DEVFMT_RGBA8I,
    __GL_DEVFMT_RGB32UI,
    __GL_DEVFMT_RGB32I,
    __GL_DEVFMT_RGBA16F,
    __GL_DEVFMT_RGBA32F,
    __GL_DEVFMT_SRGB_ALPHA,
    __GL_DEVFMT_COMPRESSED_SRGB_S3TC_DXT1,
    __GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1,
    __GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3,
    __GL_DEVFMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5,
    __GL_DEVFMT_RGB9_E5,
    __GL_DEVFMT_RGB32F,
    __GL_DEVFMT_R11G11B10F,
    __GL_DEVFMT_STENCIL,
    __GL_DEVFMT_MAX
} __GLdeviceFormat;


typedef struct __GLdeviceFormatInfoRec {
    __GLdeviceFormat devfmt;
    GLboolean compressed;

    GLuint redMask;
    GLuint greenMask;
    GLuint blueMask;
    GLuint alphaMask;
    GLuint depthMask;
    GLuint stencilMask;
    GLuint luminanceMask;
    GLuint intensityMask;

    GLint indexSize;
    GLint redSize;
    GLint greenSize;
    GLint blueSize;
    GLint alphaSize;
    GLint depthSize;
    GLint stencilSize;
    GLint luminanceSize;
    GLint intensitySize;
    GLint sharedSize;

    GLint redType;
    GLint greenType;
    GLint blueType;
    GLint alphaType;
    GLint depthType;
    GLint stencilType;
    GLint luminanceType;
    GLint intensityType;


    GLint bitsPerPixel;
    GLenum pxFormat;
    GLenum pxType;
    GLint pxAlignment;
}__GLdeviceFormatInfo;

/************************************************************************/

/*
** Structure used for allocating and freeing drawable private memory.
** (like software buffers, for example).
**
** The memory allocation routines are provided by the surrounding
** "operating system" code, and they are to be used for allocating
** software buffers and things which are associated with the drawable,
** and used by any context which draws to that drawable.  There are
** separate memory allocation functions for drawables and contexts
** since drawables and contexts can be created and destroyed independently
** of one another, and the "operating system" may want to use separate
** allocation arenas for each.
**
** The freePrivate function is filled in by the core routines when they
** allocates software buffers, and stick them in "private".  The freePrivate
** function will destroy anything allocated to this drawable (to be called
** when the drawable is destroyed).
*/

typedef struct __GLregionRectRec {
    /* lower left (inside the rectangle) */
    GLint x0, y0;
    /* upper right (outside the rectangle) */
    GLint x1, y1;
} __GLregionRect;

struct __GLdrawableRegionRec {
    GLint numRects;
    GLint maxNumRects;
    __GLregionRect *rects;
    __GLregionRect boundingRect;
    LPRGNDATA rgnData;
};

/************************************************************************/

/* Masks for the buffers */
#define __GL_FRONT_BUFFER_MASK          0x00000001
#define __GL_FRONT_LEFT_BUFFER_MASK     0x00000001
#define __GL_FRONT_RIGHT_BUFFER_MASK    0x00000002
#define __GL_BACK_BUFFER_MASK           0x00000004
#define __GL_BACK_LEFT_BUFFER_MASK      0x00000004
#define __GL_BACK_RIGHT_BUFFER_MASK     0x00000008
#define __GL_ACCUM_BUFFER_MASK          0x00000010
#define __GL_DEPTH_BUFFER_MASK          0x00000020
#define __GL_STENCIL_BUFFER_MASK        0x00000040
#define __GL_OWNERSHIP_BUFFER_MASK      0x00000080
#define __GL_TRIPLE_BUFFER_MASK         0x00000100
#define __GL_PIXEL_BUFFER_MASK          0x00000200
#define __GL_MULTISAMPLE_BUFFER_MASK    0x00000400
#define __GL_AUX_BUFFER_MASK(i)         (0x0000400 << (i))

#define __GL_ALL_BUFFER_MASK            0xffffffff


/***** Bitmasks defined for gc->flags ***********
*/
#define __GL_CONTEXT_UNINITIALIZED          0x1

#define __GL_FULL_SCREEN                    0x40
#define __GL_DRAW_TO_FRONT                  0x80


/*
** Reserved for debugging, only can be set manually. Cleared every draw.
**/
#define __GL_DISCARD_ONE_DRAW_MASK                     0x200

/*
** According to spec, in some cases no draw should apply when the 'draw' is issued.
** Setting the flag causes the following draws skipped.  To avoid redundant check,
** This flag is used together with DISCARD_ONE_DRAW, so during 'draw' only one flag
** is checked.  Seems can't use only 1 bit to represent it.
*/
#define __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_VS         0x2000
#define __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_PS         0x4000

/*
** For 8 bit mode or less, only set this bit to skip the drawing. However,
** the states need to be set and nopDispatch table should not be switched.
*/
#define __GL_DISCARD_FOLLOWING_DRAWS_UNSPPORTED_MODE            0x8000

/*
** Only for debugging, only can be set manually.
*/
#define __GL_DISCARD_FOLLOWING_DRAWS_DEBUG_SKIP                 0x10000

/*
** drawable failed to create for window minimize or mode change.
*/
#define __GL_DISCARD_FOLLOWING_DRAWS_NULL_RENDERBUFFER          0x20000

/*
** invalid GS shader
*/
#define __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_GS         0x40000

/*
** If both vs & ps enable same texture stage, but enable different target, the result is undefined.
** For such case, we shoose skip this draw.
*/
#define __GL_DISCARD_FOLLOWING_DRAWS_INVALID_TEXDIM             0x80000

/*
** Framebuffer is not complete
*/
#define __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE          0x100000

#define __GL_DISCARD_FOLLOWING_DRAWS_MASK \
                (__GL_DISCARD_FOLLOWING_DRAWS_DEBUG_SKIP|\
                 __GL_DISCARD_FOLLOWING_DRAWS_UNSPPORTED_MODE|\
                 __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_VS|\
                 __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_GS|\
                 __GL_DISCARD_FOLLOWING_DRAWS_INVALID_PROGRAM_PS|\
                 __GL_DISCARD_FOLLOWING_DRAWS_NULL_RENDERBUFFER|\
                 __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE)

#define __GL_DISCARD_DRAWS_MASK \
                (__GL_DISCARD_ONE_DRAW_MASK|__GL_DISCARD_FOLLOWING_DRAWS_MASK)

/**** End of "Bitmasks defined for gc->flags" ********
*/

/*
** Index for Back buffers
*/
#define __GL_BACK_BUFFER0                   0
#define __GL_BACK_BUFFER1                   1
#define __GL_RESOLVE_BUFFER                 6
#define __GL_MAX_BACK_BUFFERS               7
#define __GL_MAX_DRAW_BUFFERS               8

/* Index for Drawbuffers*/
#define __GL_DRAWBUFFER_FRONTLEFT_INDEX       0
#define __GL_DRAWBUFFER_FRONTRIGHT_INDEX      1
#define __GL_DRAWBUFFER_BACKLEFT_INDEX        2
#define __GL_DRAWBUFFER_BACKRIGHT_INDEX       3
#define __GL_DRAWBUFFER_AUX0_INDEX            4
#define __GL_DRAWBUFFER_AUX1_INDEX            5
#define __GL_DRAWBUFFER_AUX2_INDEX            6
#define __GL_DRAWBUFFER_AUX3_INDEX            7


typedef struct __GLdrawableRegionRec __GLdrawableRegion;
typedef struct __GLdrawableBufferRec __GLdrawableBuffer;
typedef struct __GLdrawablePrivateRec __GLdrawablePrivate;
typedef enum __GLdrawableTypeEnum {
    __GL_WINDOW,
    __GL_PBUFFER,
    __GL_PIXMAP,
} __GLdrawableType;

typedef struct __GLboundTextureRec {
    GLuint name;    /* The texture name which is bound to the colorbuffer */
    GLvoid *gc;     /* the render context in which the texture object exists. */
} __GLboundTexture;

/*for vertex buffer,texture and renderbuffer, this enum decides where is the vertex buffer or texture*/
typedef enum __GLresidenceTypeEnum {
    __GL_RESIDENT_FIRST = 0,
    __GL_RESIDENT_IN_SYSTEM = __GL_RESIDENT_FIRST,
    __GL_RESIDENT_IN_LOCALVIDMEM,
    __GL_RESIDENT_IN_NONLOCALVIDMEM,
    __GL_RESIDENT_IN_PCIEVIDEO,
    __GL_RESIDENT_IN_VIDEOMEMORY,   /* Local video memory */
    __GL_RESIDENT_LAST,
} __GLresidenceType;


struct __GLdrawableBufferRec {
    GLint width, height, depth;
    GLvoid * bufferData;       /* Only for Accum buffer: the real float buffer in device coordinate */
    GLint elementSize;                    /* Each element size in frame buffer */
    GLvoid * privateData;                 /* pointer to __GLdpRenderBufferInfo*/
    const __GLdeviceFormatInfo * deviceFormatInfo; /* pointer to detail information about the data layout*/
    __GLboundTexture boundTex;             /* Used by pbuffer to point the to the bound texture. */
};

typedef struct __GLpBufferTextureRec {
    GLboolean renderTexture;
    GLboolean needGenMipmap;
    GLenum target;
    GLboolean mipmap;
    GLenum internalFormat; /* RGB or RGBA */
    GLuint chosenFormat;

    GLuint face;
    GLuint level;
} __GLpBufferTexture;

typedef enum __GLdrawableInfoEnum {
    __GL_DRAW_FRONTBUFFER_ALLOCATION = 0,
    __GL_DRAW_BACKBUFFER0_ALLOCATION = 1,
    __GL_DRAW_BACKBUFFER1_ALLOCATION = 2,
    __GL_DRAW_PRIMARY_ALLOCATION = 3
} __GLdrawableInfo;

typedef enum  __GLLHSyncObjEnum {
    __GL_SYNCHRONIZATION_MUTEX = 1,
    __GL_SEMAPHORE             = 2
}__GLLHSyncObj;

#define __GL_DRAWABLE_PENDING_RESIZE        0x1
#define __GL_DRAWABLE_PENDING_MOVE          0x2
#define __GL_DRAWABLE_PENDING_DESTROY       0x4
#define __GL_DRAWABLE_PENDING_SWAP          0x8
#define __GL_DRAWABLE_PENDING_SWITCH        0x10
#define __GL_DRAWABLE_PENDING_RT_RESIDENT   0x20
#define __GL_DRAWABLE_PENDING_CLIPLIST      0x40
#define __GL_DRAWABLE_PENDING_PRIMARY_LOST  0x80

struct __GLdrawablePrivateRec {
    __GLcontextModes modes;
    GLint xOrigin;
    GLint yOrigin;
    /* Start position of client area related to Window */
    GLint xWOrigin;
    GLint yWOrigin;
    GLint width, height;

    /* width & height of Window */
    GLint wWidth;
    GLint wHeight;

    /* Seq Number used for sync to gc's clipbox */
    GLuint clipSeqNum;

    /* Back buffer information */
    GLuint  nodeName;
    GLuint  backNode;
    GLuint backBufferPhysAddr;
    GLuint * backBufferLogicalAddr;

#ifdef _LINUX_
    /* For Linux clip list information */
    GLint numClipRects;
    GLuint *clipRects;
#endif

    GLint yInverted;
    __GLdrawableType type;
    GLboolean fullScreenMode;
    GLboolean flipOn;
    GLboolean bFocus;
    GLboolean bExclusiveModeChanged;
    union {
        struct {
            /* 0: front_left, 1: front_right, 2: back_left, 3:back_right */
            /* 4-7: aux0 to aux3 */
            __GLdrawableBuffer drawBuffers[__GL_MAX_DRAW_BUFFERS];
        };
        struct {
            /* Keep these varibles to avoid changing lagecy driver */
            __GLdrawableBuffer frontBuffer;      /* front and front_left */
            __GLdrawableBuffer frontRightBuffer; /* front right */
            /* 0: back_left and back buffer 0, 1:back right and back buffer 1 for triple buffer mode */
            /* 2 to 5 for Aux0 to Aux3, 6 is for fake front */
            __GLdrawableBuffer backBuffer[__GL_MAX_BACK_BUFFERS];
        };
    };

    __GLdrawableBuffer frontBuffer2; /* For secondary front buffer in SAMM mode */
    __GLdrawableBuffer accumBuffer;
    __GLdrawableBuffer depthBuffer;
    __GLdrawableBuffer stencilBuffer;
    __GLdrawableBuffer multisampleBuffer;

    GLvoid *lock;        /* the actual lock for this drawablePrivate */

    /* imported */
    GLvoid *(*malloc)(size_t size);
    GLvoid *(*calloc)(size_t numElem, size_t elemSize);
    GLvoid *(*realloc)(GLvoid *oldAddr, size_t newSize);
    GLvoid (*free)(GLvoid *addr);
    GLvoid *(*addSwapHintRectWIN)(__GLdrawablePrivate *, GLint, GLint, GLsizei, GLsizei);
    GLvoid (*clearSwapHintRectWIN)(__GLdrawablePrivate *);

    GLvoid *other;

    /* Fallback software pipeline specific drawable information */
    struct {
        GLvoid *privateData;
        GLvoid (*destroyPrivateData)(__GLdrawablePrivate *);
        GLvoid (*updateDrawable)(__GLdrawablePrivate *);
        GLvoid (*freeBuffers)(__GLdrawablePrivate *);
    } swp;

    /* Device specific drawable information attached to "privateData" */
    struct {
        GLvoid *privateData;
        GLvoid (*destroyPrivateData)(__GLdrawablePrivate *);
        GLvoid (*updateDrawable)(__GLdrawablePrivate *);
        GLvoid (*freeBuffers)(__GLdrawablePrivate *, GLboolean);
        GLvoid (*restoreFrontBuffer)(__GLdrawablePrivate *);
        GLvoid (*clearShareData)(__GLdrawablePrivate *);
        GLvoid (*addSwapHintRectWIN)(__GLdrawablePrivate *,RECT * , GLuint );
        GLvoid (*clearSwapHintRectWIN)(__GLdrawablePrivate *);
        GLvoid (* bindRenderBuffer)(__GLdrawablePrivate *, __GLdrawableBuffer *);
        GLvoid (* deleteRenderBuffer)(__GLdrawablePrivate *, __GLdrawableBuffer *);
        GLvoid (* notifyBuffersSwapable)(__GLdrawablePrivate *);

        /* These three APIs are called in Window Vista driver, for XP they are NULL */
        GLboolean (*setDisplayMode)(__GLcontext *gc);
        GLboolean(*setExclusiveDisplay)(GLboolean bSet);
        GLvoid(*ExclusiveModeChange)(__GLcontext *gc);

        /* for DrvPresentBuffers to perform dp present buffers. NULL for XP*/
        GLboolean (*presentBuffers)(__GLcontext *gc, __GLdrawablePrivate *, GLvoid *, GLboolean, GLboolean, ULONGLONG presentToken);

        /* for DrvSwapBuffers to perform dp swap buffers, NULL for Vista*/
        GLboolean (*swapBuffers)(__GLcontext *, __GLdrawablePrivate *, GLboolean);
    } dp;

    /* For the pbuffer . */
    __GLpBufferTexture *pbufferTex;

    /* internal formats for drawable buffers */
    GLenum internalFormatColorBuffer;
    GLenum internalFormatDepthBuffer;
    GLenum internalFormatStencilBuffer;
    GLenum internalFormatAccumBuffer;
    GLenum internalFormatDisplayBuffer;

#if (defined(_DEBUG) || defined(DEBUG))
    GLuint drawCount;
    GLuint frameCount;
#endif
};


typedef enum __GLmemoryStatusEnum{
    __GL_MMUSAGE_PHYS_TOTAL = 0,
    __GL_MMUSAGE_PHYS_AVAIL = 1,
    __GL_MMUSAGE_VIRTUAL_TOTAL = 2,
    __GL_MMUSAGE_VIRTUAL_AVAIL = 3,
    }__GLmemoryStatus;

/*
** Procedures which are imported by the GL from the surrounding
** "operating system".  Math functions are not considered part of the
** "operating system".
*/
typedef struct __GLimportsRec {
    /* Memory management */
    GLvoid *(*malloc)(__GLcontext *gc, size_t size);
    GLvoid *(*calloc)(__GLcontext *gc, size_t numElem, size_t elemSize);
    GLvoid *(*realloc)(__GLcontext *gc, GLvoid *oldAddr, size_t newSize);
    GLvoid (*free)(__GLcontext *gc, GLvoid *addr);
    GLuint (*getMemoryStatus)(__GLmemoryStatus);

    /* Error handling */
    GLvoid (*warning)(__GLcontext *gc, const GLbyte *fmt);
    GLvoid (*fatal)(__GLcontext *gc, const GLbyte *fmt);

    /* Set window's GL api dispatch table */
    GLvoid (*setProcTable)(__GLcontext *gc, __GLdispatchState *state);

    /* Drawable mutex lock from gl core */
    GLvoid (*lockDrawable)(GLvoid *lock);
    GLvoid (*unlockDrawable)(GLvoid *lock);

    /*critical section lock import into gl core */
    GLvoid (*createMutex)(GLvoid *lock);
    GLvoid (*destroyMutex)(GLvoid *lock);
    GLvoid (*lockMutex)(GLvoid *lock);
    GLvoid (*unlockMutex)(GLvoid *lock);

    /* GetDC and GetHWND */
    GLvoid *(*getHWND)(__GLcontext *gc);
    GLvoid *(*getDC)(GLvoid *other);
    GLvoid (*releaseDC)(GLvoid *other, GLvoid *hDC);

    /* Blt back buffer content to front buffer */
    /* This function is called if renderring in front buffer */
    GLvoid (*internalSwapBuffers)(__GLcontext *gc, GLboolean bSwapFront);

    /* Get current display mode */
    GLvoid (*getDisplayMode)(GLuint *width, GLuint *height, GLuint *depth);

    /* Get the colorbuffer from the handle of pbuffer */
    __GLdrawableBuffer *(*getColorbufferPBuffer)(GLvoid *hPbuffer,GLenum iBuffer);

    /* Device configuration changed function */
    GLvoid (*deviceConfigChanged)(__GLcontext *gc);

    GLvoid (*bltImageToScreen)(__GLcontext *gc,
                              GLint bitsAlignedWidth,
                              GLint bitsAlignedHeight,
                              GLint bitsPerPixel,
                              GLvoid * bits,
                              GLint left,
                              GLint top,
                              GLint width,
                              GLint height);

    /* Device DLL interface */
    GLvoid *device;
    GLuint deviceIndex;

    /* Operating system dependent data goes here */
    GLvoid *other;

} __GLimports;

/*
** Procedures which are exported by the GL to the surrounding "operating
** system" so that it can manage multiple GL context's.
*/
typedef struct __GLexportsRec {
    /* Context management (return GL_FALSE on failure) */
    GLuint (*destroyContext)(__GLcontext *gc);
    GLuint (*loseCurrent)(__GLcontext *gc, GLvoid **thrArea);
    GLuint (*makeCurrent)(__GLcontext *gc, GLuint thrHashId, GLvoid **thrArea);
    GLuint (*shareContext)(__GLcontext *gc, __GLcontext *gcShare);
    GLuint (*copyContext)(__GLcontext *dst, __GLcontext *src, GLuint mask);

    GLvoid (*associateContext)(__GLcontext *gc, __GLdrawablePrivate *drawable, __GLdrawablePrivate *readable);
    GLvoid (*deassociateContext)(__GLcontext *gc);

    /* Drawing surface change notification */
    GLvoid (*notifyDrawableChange)(__GLcontext *gc, GLuint mask);

    GLvoid (*notifyMultiThread)(__GLcontext *gc, GLuint mtFlag);

    /* Display device configuration change notification */
    GLvoid (*deviceConfigurationChanged)(__GLcontext *gc);

    /* WGL_EXT_swap_control */
    GLboolean (*swapInterval)(__GLcontext *, GLint, GLuint);

    /* WGL_ARB_render_texture */
    GLboolean (*bindTexImageARB)(__GLcontext *, __GLdrawablePrivate *, GLvoid*,GLenum);
    GLboolean (*releaseTexImageARB)(__GLcontext *, __GLdrawablePrivate *, GLenum);
} __GLexports;

/************************************************************************/

/*
** This must be the first member of a __GLcontext structure.  This is the
** only part of a context that is exposed to the outside world; everything
** else is opaque.
*/
typedef struct __GLcontextInterfaceRec {
    __GLimports imports;
    __GLexports exports;
} __GLcontextInterface;

extern __GLcontext *__glCreateContext(__GLimports *, __GLcontextModes *);
extern GLvoid __glCoreNopDispatch(GLvoid);

extern GLvoid __glUnSupportModeEnter(GLvoid);

#endif /* __gl_core_h_ */
