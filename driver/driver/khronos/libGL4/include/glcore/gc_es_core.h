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


#ifndef __gc_es_core_h__
#define __gc_es_core_h__

#include <sys/types.h>
#ifdef _LINUX_
#include "win2unix.h"
#endif


#if defined(__arm__) || defined(i386) || defined(__i386__) || defined(__x86__) || defined(_M_IX86)\
 || defined(__x86_64__) || defined(_M_AMD64) || defined (_M_X64) || defined(__e2k__) || defined(_LITTLE_ENDIAN)|| defined(__arm64) || defined(__arm64__)
#define USE_LENDIAN
#endif

typedef struct __GLcontextRec __GLcontext;
typedef struct __GLmipMapLevelRec __GLmipMapLevel;
typedef struct __GLtextureObjectRec __GLtextureObject;
typedef struct __GLesDispatchTableRec __GLesDispatchTable;

typedef enum __GLdrawableTypeEnum {
    __GL_WINDOW,
    __GL_PBUFFER,
    __GL_PIXMAP,
} __GLdrawableType;


typedef enum __GLdrawableInfoEnum {
    __GL_DRAW_FRONTBUFFER_ALLOCATION = 0,
    __GL_DRAW_BACKBUFFER0_ALLOCATION = 1,
    __GL_DRAW_BACKBUFFER1_ALLOCATION = 2,
    __GL_DRAW_PRIMARY_ALLOCATION = 3
} __GLdrawableInfo;

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
    GLint  samples;

    GLuint sampleBuffers;

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

#define __GL_FULL_SCREEN                    0x40
#define __GL_DRAW_TO_FRONT                  0x80

/*
** All possible device supported buffer format
*/
typedef enum
{
    __GL_FMT_A8 = 0,
    __GL_FMT_L8,
    __GL_FMT_LA8,
    __GL_FMT_R8,
    __GL_FMT_R8_SNORM,
    __GL_FMT_RG8,
    __GL_FMT_RG8_SNORM,
    __GL_FMT_RGB8,
    __GL_FMT_RGB8_SNORM,
    __GL_FMT_RGB565,
    __GL_FMT_RGBA4,
    __GL_FMT_RGB5_A1,
    __GL_FMT_RGBA8,
    __GL_FMT_BGRA,
    __GL_FMT_RGBA8_SNORM,
    __GL_FMT_RGB10_A2,
    __GL_FMT_SRGB8,
    __GL_FMT_SRGB8_ALPHA8,
    __GL_FMT_R16F,
    __GL_FMT_RG16F,
    __GL_FMT_RGB16F,
    __GL_FMT_RGBA16F,
    __GL_FMT_R32F,
    __GL_FMT_RG32F,
    __GL_FMT_RGB32F,
    __GL_FMT_RGBA32F,
    __GL_FMT_R11F_G11F_B10F,
    __GL_FMT_RGB9_E5,
    __GL_FMT_R8I,
    __GL_FMT_R8UI,
    __GL_FMT_R16I,
    __GL_FMT_R16UI,
    __GL_FMT_R32I,
    __GL_FMT_R32UI,
    __GL_FMT_RG8I,
    __GL_FMT_RG8UI,
    __GL_FMT_RG16I,
    __GL_FMT_RG16UI,
    __GL_FMT_RG32I,
    __GL_FMT_RG32UI,
    __GL_FMT_RGB8I,
    __GL_FMT_RGB8UI,
    __GL_FMT_RGB16I,
    __GL_FMT_RGB16UI,
    __GL_FMT_RGB32I,
    __GL_FMT_RGB32UI,
    __GL_FMT_RGBA8I,
    __GL_FMT_RGBA8UI,
    __GL_FMT_RGBA16I,
    __GL_FMT_RGBA16UI,
    __GL_FMT_RGBA32I,
    __GL_FMT_RGBA32UI,
    __GL_FMT_RGB10_A2UI,

    __GL_FMT_ETC1_RGB8_OES,
    __GL_FMT_R11_EAC,
    __GL_FMT_SIGNED_R11_EAC,
    __GL_FMT_RG11_EAC,
    __GL_FMT_SIGNED_RG11_EAC,
    __GL_FMT_RGB8_ETC2,
    __GL_FMT_SRGB8_ETC2,
    __GL_FMT_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    __GL_FMT_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,
    __GL_FMT_RGBA8_ETC2_EAC,
    __GL_FMT_SRGB8_ALPHA8_ETC2_EAC,

    __GL_FMT_COMPRESSED_RGB_S3TC_DXT1_EXT,
    __GL_FMT_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    __GL_FMT_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    __GL_FMT_COMPRESSED_RGBA_S3TC_DXT5_EXT,

    __GL_FMT_PALETTE4_RGBA4_OES,
    __GL_FMT_PALETTE4_RGB5_A1_OES,
    __GL_FMT_PALETTE4_R5_G6_B5_OES,
    __GL_FMT_PALETTE4_RGB8_OES,
    __GL_FMT_PALETTE4_RGBA8_OES,
    __GL_FMT_PALETTE8_RGBA4_OES,
    __GL_FMT_PALETTE8_RGB5_A1_OES,
    __GL_FMT_PALETTE8_R5_G6_B5_OES,
    __GL_FMT_PALETTE8_RGB8_OES,
    __GL_FMT_PALETTE8_RGBA8_OES,


    /* Depth stencil formats */
    __GL_FMT_Z16,
    __GL_FMT_Z24,
    __GL_FMT_Z32F,
    __GL_FMT_Z24S8,
    __GL_FMT_Z32FS8,
    __GL_FMT_S1,
    __GL_FMT_S4,
    __GL_FMT_S8,

    /* ASTC formats. */
#if defined(GL_KHR_texture_compression_astc_ldr)
    __GL_FMT_COMPRESSED_RGBA_ASTC_4x4_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_5x4_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_5x5_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_6x5_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_6x6_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_8x5_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_8x6_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_8x8_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_10x5_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_10x6_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_10x8_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_10x10_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_12x10_KHR,
    __GL_FMT_COMPRESSED_RGBA_ASTC_12x12_KHR,

    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR,
    __GL_FMT_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,
#endif

    __GL_FMT_RGBX8,
    __GL_FMT_BGRX8,
 /*LUIMANCE ALPHA*/

    __GL_FMT_A32F,

    __GL_FMT_L32F,

    __GL_FMT_LA32F,


#ifdef OPENGL40
    __GL_FMT_A16,
    __GL_FMT_A24,
    __GL_FMT_L16,
    __GL_FMT_L24,
    __GL_FMT_I8,
    __GL_FMT_I16,
    __GL_FMT_I24,
    __GL_FMT_I32F,
    __GL_FMT_LA4,
    __GL_FMT_LA16,
    __GL_FMT_BGR565,
    __GL_FMT_BGRA4444,
    __GL_FMT_BGRA5551,
    __GL_FMT_RGBA16,
    __GL_FMT_BGRA1010102,
    __GL_FMT_ZHIGH24,
    __GL_FMT_ZHIGH24S8,
    __GL_FMT_COMPRESSED_LUMINANCE_LATC1,
    __GL_FMT_COMPRESSED_SIGNED_LUMINANCE_LATC1,
    __GL_FMT_COMPRESSED_LUMINANCE_ALPHA_LATC2,
    __GL_FMT_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2,
    __GL_FMT_COMPRESSED_RED_RGTC1,
    __GL_FMT_COMPRESSED_SIGNED_RED_RGTC1,
    __GL_FMT_COMPRESSED_RED_GREEN_RGTC2,
    __GL_FMT_COMPRESSED_SIGNED_RED_GREEN_RGTC2,
    __GL_FMT_RGBA8888_SIGNED,
    __GL_FMT_SRGB_ALPHA,
    __GL_FMT_COMPRESSED_SRGB_S3TC_DXT1,
    __GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT1,
    __GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT3,
    __GL_FMT_COMPRESSED_SRGB_ALPHA_S3TC_DXT5,
#endif

    __GL_FMT_MAX,

} __GLformat;


typedef struct __GLformatInfoRec
{
    /* Driver used indexed format, same as glFormat, just remapped to be sequential */
    __GLformat  drvFormat;

    /* Same as internal format.
    ** Actually, mostly it's a sized-internal-format, just a few cases is a unsized-internal-format
    ** If it's unsized-internal-format, it's same as base internal format
    **/
    GLenum      glFormat;

    /* Base internal format */
    GLenum      baseFormat;

    /* UNORM/SNORM/FLOAT/INT/UNIT */
    GLint       category;

    /* Compressed format? */
    GLboolean   compressed;

    /* Texture filterable? */
    GLboolean   filterable;

    /* Color/depth/stencil renderable? */
    GLboolean   renderable;

    /* For compressed formats, it's bitsPerBlock */
    GLint       bitsPerPixel;

    GLint       redSize;
    GLint       greenSize;
    GLint       blueSize;
    GLint       alphaSize;
    GLint       depthSize;
    GLint       stencilSize;

    /* Basically it's almost same as base format.
    ** But it differentiate integer or not
    */
    GLenum      dataFormat;

    /* Data type + dataFormat = sized-internal-format = glFormat; */
    GLenum      dataType;

    /* Shared component size */
    GLuint      sharedSize;

    /* Component type */
    GLint       redType;
    GLint       greenType;
    GLint       blueType;
    GLint       alphaType;
    GLint       depthType;

    /* Buffer encoding */
    GLint       encoding;
#ifdef OPENGL40
    GLint       luminanceType;
    GLint       intensityType;
    GLint       luminanceSize;
    GLint       intensitySize;
#endif
} __GLformatInfo;


/*
****** Context flag bitmasks defined for gc->flags ***********
*/
#define __GL_CONTEXT_UNINITIALIZED                          0x1



#define __GL_CONTEXT_FULL_SCREEN                            0x2

#define __GL_CONTEXT_DRAW_TO_FRONT                          0x4



#define __GL_CONTEXT_SKIP_DRAW_UNSPPORTED_MODE              0x10

#define __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER         0x20

#define __GL_CONTEXT_SKIP_DRAW_INSUFFICIENT_VERTEXCOUNT     0x40



#define __GL_CONTEXT_SKIP_PRESERVE_CLEAR_RECT               0x100





#define __GL_CONTEXT_SKIP_DRAW_MASK \
                (__GL_CONTEXT_SKIP_DRAW_UNSPPORTED_MODE | \
                 __GL_CONTEXT_SKIP_DRAW_INVALID_RENDERBUFFER | \
                 __GL_CONTEXT_SKIP_DRAW_INSUFFICIENT_VERTEXCOUNT)

/**** End of "Bitmasks defined for gc->flags" ****/

/*
** Index for Back buffers, must be no less than GL reported caps
*/
#define __GL_MAX_DRAW_BUFFERS               4

#define __GL_MAX_DRAW_BUFFERS_GL4

/* Drawable flags */
#define __GL_DRAWABLE_FLAG_ZERO_WH          0x00001

#ifdef OPENGL40
/* temporily define it to pass the compile */
typedef struct __GLdrawableBufferRec __GLdrawableBuffer;
struct __GLdrawableBufferRec {
    GLint width, height, depth;
    GLvoid * bufferData;       /* Only for Accum buffer: the real float buffer in device coordinate */
    GLint elementSize;                    /* Each element size in frame buffer */
    GLvoid * privateData;                 /* pointer to __GLdpRenderBufferInfo*/
    __GLformatInfo *deviceFormatInfo; /* pointer to detail information about the data layout*/
//    __GLboundTexture boundTex;             /* Used by pbuffer to point the to the bound texture. */
};
/*
** Index for Back buffers
*/
#define __GL_BACK_BUFFER0                   0
#define __GL_BACK_BUFFER1                   1
#define __GL_RESOLVE_BUFFER                 6
#define __GL_MAX_BACK_BUFFERS               7


/* Index for Drawbuffers*/
#define __GL_DRAWBUFFER_FRONTLEFT_INDEX       0
#define __GL_DRAWBUFFER_FRONTRIGHT_INDEX      1
#define __GL_DRAWBUFFER_BACKLEFT_INDEX        2
#define __GL_DRAWBUFFER_BACKRIGHT_INDEX       3
#define __GL_DRAWBUFFER_AUX0_INDEX            4
#define __GL_DRAWBUFFER_AUX1_INDEX            5
#define __GL_DRAWBUFFER_AUX2_INDEX            6
#define __GL_DRAWBUFFER_AUX3_INDEX            7

#define __GL_DISCARD_FOLLOWING_DRAWS_FRAMEBUFFER_NOT_COMPLETE          0x100000

#define __GL_DRAWABLE_PENDING_RESIZE        0x1
#define __GL_DRAWABLE_PENDING_MOVE          0x2
#define __GL_DRAWABLE_PENDING_DESTROY       0x4
#define __GL_DRAWABLE_PENDING_SWAP          0x8
#define __GL_DRAWABLE_PENDING_SWITCH        0x10
#define __GL_DRAWABLE_PENDING_RT_RESIDENT   0x20
#define __GL_DRAWABLE_PENDING_CLIPLIST      0x40
#define __GL_DRAWABLE_PENDING_PRIMARY_LOST  0x80

#endif
typedef struct __GLdrawablePrivateRec __GLdrawablePrivate;

struct __GLdrawablePrivateRec
{
    __GLcontext *gc;
    __GLcontextModes modes;

    GLint xOrigin;
    GLint yOrigin;
    /* Start position of client area related to Window */
    GLint xWOrigin;
    GLint yWOrigin;


    GLint width;
    GLint height;

    /* width & height of Window */
    GLint wWidth;
    GLint wHeight;

#ifdef OPENGL40
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
    GLboolean fullScreenMode;
    GLboolean flipOn;
    GLboolean bFocus;
    GLboolean bExclusiveModeChanged;

/* possibly the next fields are not needed for ES core, Fix me */
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
    __GLdrawableBuffer depthBuffer;
    __GLdrawableBuffer stencilBuffer;
#endif


#ifdef OPENGL40
    __GLdrawableBuffer accumBuffer;
    void * rtHandle[__GL_MAX_DRAW_BUFFERS];
#else
    void * rtHandle;
#endif
    void * prevRtHandle;
    void * depthHandle;
    void * stencilHandle;

    __GLformatInfo * rtFormatInfo;
    __GLformatInfo * dsFormatInfo;

#ifdef OPENGL40
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


    /* imported */
    GLvoid *(*malloc)(size_t size);
    GLvoid *(*calloc)(size_t numElem, size_t elemSize);
    GLvoid *(*realloc)(GLvoid *oldAddr, size_t newSize);
    GLvoid (*free)(GLvoid *addr);
    GLvoid *(*addSwapHintRectWIN)(__GLdrawablePrivate *, GLint, GLint, GLsizei, GLsizei);
    GLvoid (*clearSwapHintRectWIN)(__GLdrawablePrivate *);
#endif

    GLuint flags;
    __GLdrawableType type;
    __GLpBufferTexture *pbufferTex;
    GLvoid *lock;        /* the actual lock for this drawablePrivate */
    GLvoid *privateData;
    GLvoid *other;

#ifdef OPENGL40
    /* internal formats for drawable buffers */
    GLenum internalFormatColorBuffer;
    GLenum internalFormatDepthBuffer;
    GLenum internalFormatStencilBuffer;
    GLenum internalFormatAccumBuffer;
    GLenum internalFormatDisplayBuffer;
#endif

};



#endif /* __gc_es_core_h__ */
