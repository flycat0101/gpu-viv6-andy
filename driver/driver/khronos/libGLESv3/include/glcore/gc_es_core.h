/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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


typedef struct __GLcontextRec __GLcontext;
typedef struct __GLmipMapLevelRec __GLmipMapLevel;
typedef struct __GLtextureObjectRec __GLtextureObject;
typedef struct __GLesDispatchTableRec __GLesDispatchTable;

/*
** Mode and limit information for a context. This information is
** kept around in the context so that values can be used during
** command execution, and for returning information about the
** context to the application.
*/
typedef struct __GLcontextModesRec
{
    GLuint    rgbMode;
    GLuint    rgbFloatMode;
    GLuint    doubleBufferMode;
    GLuint    tripleBufferMode;
    GLuint    stereoMode;
    GLuint    haveDepthBuffer;
    GLuint    haveStencilBuffer;

    /* Multisample extend */
    GLint     samples;
    GLuint    sampleBuffers;

    GLint     redBits, greenBits, blueBits, alphaBits;
    GLint     rgbaBits; /* total bits for rgba */

    GLint     depthBits;
    GLint     stencilBits;

} __GLcontextModes;

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
    __GL_FMT_ARGB4,
    __GL_FMT_ABGR4,
    __GL_FMT_XRGB4,
    __GL_FMT_XBGR4,
 /*LUIMANCE ALPHA*/
    __GL_FMT_A32F,
    __GL_FMT_L32F,
    __GL_FMT_LA32F,
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

    /* Compression block size, non-conpressed size is 1 */
    GLint       blockWidth;
    GLint       blockHeight;

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

/* Drawable flags */
#define __GL_DRAWABLE_FLAG_ZERO_WH          0x00001

typedef struct __GLdrawablePrivateRec
{
    __GLcontext *gc;
    __GLcontextModes modes;

    GLint width;
    GLint height;

    void * rtHandle;
    void * prevRtHandle;
    void * depthHandle;
    void * stencilHandle;

    __GLformatInfo * rtFormatInfo;
    __GLformatInfo * dsFormatInfo;

    GLuint flags;

    GLvoid *privateData;

} __GLdrawablePrivate;

#endif /* __gc_es_core_h__ */
