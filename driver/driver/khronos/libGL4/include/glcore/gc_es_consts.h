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


#ifndef __gc_es_consts_h__
#define __gc_es_consts_h__

#include "gc_es_bindtable.h"

#define __glZero        (0.0f)
#define __glOne         (1.0f)
#define __glMinusOne    (-1.0f)
#define __glHalf        (0.5f)
#define __glMinusHalf   (-0.5f)
#define __glTwo         (2.0f)
#define __glDegreesToRadians    (0.017453292519943295769222222222222f)
#define __glPi          (3.14159265358979323846f)
#define __glSqrt2       (1.41421356237309504880f)
#define __glE           (2.7182818284590452354f)
#define __glLog2E       (1.442694f)
#define __gl255         (255.0f)
#define __gl65535       (65535.0f)
#define __glFloatMax    3.402823466e+38f       /* max value */
#define __glFloatMin    -3.402823466e+38f      /* min value */

#define __glMaxByte     127.0f
#define __glMaxUbyte    255.0f
#define __glMaxShort    32767.0f
#define __glMaxUshort   65535.0f
#define __glMaxInt        2147483647.0    /* 2^31 - 1 double */
#define __glMinInt       -2147483647.0    /* -(2^32 - 1) double */
#define __glMaxUint       4294967295.0    /* 2^32 - 1 double */

#define __glInvMaxByte     (0.007874015748031496062992125984252f)
#define __glInvMaxUbyte    (0.003921568627450980392156862745098f)
#define __glInvMaxShort    (3.0518509475997192297128208258309e-5f)
#define __glInvMaxUshort   (1.5259021896696421759365224689097e-5f)
#define __glInvMaxInt      (4.656612875245796924105750827168e-10)     /* double */
#define __glInvMaxUint     (2.3283064370807973754314699618685e-10)    /* double */
#define __glInvMaxUint24   (5.9604648328104515558750364705942e-8)     /* double */

#define __GL_MAX_MAX_VIEWPORT                   2048

#define __GL_MAX_TEXTURE_UNITS                  96

#define __GL_MAX_VERTEX_STREAMS                 16

#define __GL_MAX_IMAGE_UNITS                    40

#define __GL_MAX_VERTEX_ATTRIBUTES              32

#define __GL_MAX_VERTEX_ATTRIBUTE_BINDINGS      32

#define __GL_MAX_COLOR_ATTACHMENTS              (__GL_MAX_DRAW_BUFFERS)
#define __GL_MAX_ATTACHMENTS                    (__GL_MAX_COLOR_ATTACHMENTS + 2)

#define __GL_ONE_32        ((GLuint)1)
#define __GL_ONE_64        ((GLuint64)1)

#define __GL_FLOAT      0       /* __GLfloat */
#define __GL_FLOAT32    1       /* api 32 bit float */
#define __GL_FLOAT64    2       /* api 64 bit float */
#define __GL_INT32      3       /* api 32 bit GLint */
#define __GL_INT64      4       /* api 64 bit GLint */
#define __GL_BOOLEAN    5       /* api 8 bit boolean */
#define __GL_COLOR      6       /* unscaled color in __GLfloat */

#ifdef OPENGL40
#define __GL_MAX_TEXTURE_COORDS                  8
#define __GL_MAX_LIGHT_NUMBER                    32
#define __GL_MAX_CLIPPLANE_NUMBER                8

#define __GL_MAX_VERTEX_ELEMENTS                 16
#define __GL_MAX_WEIGHT_SUPPORT                  4
#define __GL_MAX_VERTEX_BUFFER_BINDINGS          6

#define __GL_MAX_PROGRAM_STACK_DEPTH             8
#define __GL_MAX_PROGRAM_VERTEX_ATTRIBUTES       16
#define __GL_MAX_PROGRAM_MATRICES                16
#define __GL_MAX_PROGRAM_LOCAL_PARAMETERS        256
#define __GL_MAX_PROGRAM_PARAMETERS              480
#define __GL_MAX_PROGRAM_ENV_PARAMETERS          256
#define __GL_MAX_PROGRAM_INSTRUCTIONS            128
#define __GL_PROGRAM_NATIVE_INSTRUCTIONS         128
#define __GL_MAX_PROGRAM_ADDRESS_REGISTERS       1
#define __GL_NUMBER_OF_PROGRAM_TARGET            2
#define __GL_MAX_PROGRAM_ERRORSTR_LENGTH         2048
#define __GL_PROGRAM_TABLE_STEP                  20

#define __GL_TOTAL_VERTEX_ATTRIBUTES (__GL_MAX_VERTEX_ATTRIBUTES + __GL_MAX_TEXTURE_COORDS + 8)

#define __GL_MAX_COLORS                  2

/* Indicies for color[] array in vertex */
#define __GL_FRONTFACE                   0
#define __GL_BACKFACE                    1
#define __GL_MAX_COLORS                  2
#define __GL_PRIMARY_COLOR               0
#define __GL_SECONDARY_COLOR             1


/*
** Vertex structure.  Each vertex contains enough state to properly
** render the active primitive.
*/
typedef struct __GLvertexRec {
    __GLcoord    winPos;
    GLfloat      clipW;
    __GLcolor   *color[__GL_MAX_COLORS];         /* Pointer to current primary and secondary colors*/
    __GLcolor    colors[__GL_MAX_COLORS];        /* 0: front face primary,   1: back face primary */
    __GLcolor    colors2[__GL_MAX_COLORS];       /* 0: front face secondary, 1: back face secondary */
    __GLcoord    texture[__GL_MAX_TEXTURE_COORDS];
    GLfloat      eyeDistance;
    GLfloat      pointSize;
    GLfloat      colorIndex;
    GLuint       boundaryEdge;
} __GLvertex;

#endif


/*internal data formats which aren't defined by spec, but supported by Vivante internal
**mainly they are for renderbuffers
*/
/* From 0x1FFFFF formats ,there are compressed formats*/
enum {
    __GL_MIN_FMT_TYPE                           = 0x0001FFFF,

    __GL_RGBX8                                  = __GL_MIN_FMT_TYPE,
    __GL_BGRX8                                  = 0x0002FFFF,
#ifdef OPENGL40
    __GL_DXT1_BLOCK              =      0x1FFFFF,
    __GL_MIN_TC_TYPE             =  __GL_DXT1_BLOCK,
    __GL_DXT1A_BLOCK             =      0x2FFFFF,
    __GL_DXT3_BLOCK              =      0x3FFFFF,
    __GL_DXT5_BLOCK              =      0x4FFFFF,
    __GL_LATC1_BLOCK             =      0x5FFFFF,
    __GL_SIGNED_LATC1_BLOCK      =      0x6FFFFF,
    __GL_LATC2_BLOCK             =      0x7FFFFF,
    __GL_SIGNED_LATC2_BLOCK      =      0x8FFFFF,
    __GL_RGTC1_BLOCK             =      0x9FFFFF,
    __GL_SIGNED_RGTC1_BLOCK      =      0xAFFFFF,
    __GL_RGTC2_BLOCK             =      0xBFFFFF,
    __GL_SIGNED_RGTC2_BLOCK      =      0xCFFFFF,
    __GL_MAX_TC_TYPE             =  __GL_SIGNED_RGTC2_BLOCK,
#else
    __GL_DXT1_BLOCK                             = 0x0003FFFF,
    __GL_DXT1A_BLOCK                            = 0x0004FFFF,
    __GL_DXT3_BLOCK                             = 0x0005FFFF,
    __GL_DXT5_BLOCK                             = 0x0006FFFF,
#endif
    __GL_ETC1_BLOCK                             = 0x0007FFFF,
    __GL_R11_EAC_BLOCK                          = 0x0008FFFF,
    __GL_SIGNED_R11_EAC_BLOCK                   = 0x0009FFFF,
    __GL_RG11_EAC_BLOCK                         = 0x000AFFFF,
    __GL_SIGNED_RG11_EAC_BLOCK                  = 0x000BFFFF,
    __GL_RGB8_ETC2_BLOCK                        = 0x000CFFFF,
    __GL_RGB8_PUNCHTHROUGH_ALPHA1_ETC2_BLOCK    = 0x000DFFFF,
    __GL_RGBA8_ETC2_EAC_BLOCK                   = 0x000EFFFF,

    __GL_PALETTE4_RGBA4                         = 0x000FFFFF,
    __GL_PALETTE4_RGB5_A1                       = 0x0010FFFF,
    __GL_PALETTE4_R5_G6_B5                      = 0x0011FFFF,
    __GL_PALETTE4_RGB8                          = 0x0012FFFF,
    __GL_PALETTE4_RGBA8                         = 0x0013FFFF,
    __GL_PALETTE8_RGBA4                         = 0x0014FFFF,
    __GL_PALETTE8_RGB5_A1                       = 0x0015FFFF,
    __GL_PALETTE8_R5_G6_B5                      = 0x0016FFFF,
    __GL_PALETTE8_RGB8                          = 0x0017FFFF,
    __GL_PALETTE8_RGBA8                         = 0x0018FFFF,


    __GL_ASTC_4x4_BLOCK                         = 0x001BFFFF,
    __GL_ASTC_5x4_BLOCK                         = 0x001CFFFF,
    __GL_ASTC_5x5_BLOCK                         = 0x001DFFFF,
    __GL_ASTC_6x5_BLOCK                         = 0x001EFFFF,
    __GL_ASTC_6x6_BLOCK                         = 0x001FFFFF,
    __GL_ASTC_8x5_BLOCK                         = 0x0020FFFF,
    __GL_ASTC_8x6_BLOCK                         = 0x0021FFFF,
    __GL_ASTC_8x8_BLOCK                         = 0x0022FFFF,
    __GL_ASTC_10x5_BLOCK                        = 0x0023FFFF,
    __GL_ASTC_10x6_BLOCK                        = 0x0024FFFF,
    __GL_ASTC_10x8_BLOCK                        = 0x0025FFFF,
    __GL_ASTC_10x10_BLOCK                       = 0x0026FFFF,
    __GL_ASTC_12x10_BLOCK                       = 0x0027FFFF,
    __GL_ASTC_12x12_BLOCK                       = 0x0028FFFF,

    __GL_MAX_FMT_TYPE                           = __GL_ASTC_12x12_BLOCK,
};

/* Enum for shader precisions */
enum
{
    __GLSL_TYPE_LOW_FLOAT = 0,
    __GLSL_TYPE_MEDIUM_FLOAT,
    __GLSL_TYPE_HIGH_FLOAT,
    __GLSL_TYPE_LOW_INT,
    __GLSL_TYPE_MEDIUM_INT,
    __GLSL_TYPE_HIGH_INT,

    __GLSL_TYPE_LAST,
};

#define __GL_MAX_VERSION_LEN 64

/*
** Various constants. Most of these will never change through the life
** of the context.
*/
typedef struct __GLdeviceConstantsRec
{
    /* Stuff for glGetString */
    GLchar *vendor;
    GLchar *renderer;
    GLchar version[__GL_MAX_VERSION_LEN];
    GLchar *GLSLVersion;
    GLchar *extensions;

    GLuint numExtensions;
    GLint  majorVersion;
    GLint  minorVersion;

    /* Specific size limits */
    GLuint numberofQueryCounterBits;
    GLuint maxViewportWidth;
    GLuint maxViewportHeight;

    /* Texture constants*/
    GLuint maxTextureSize;
    GLuint maxNumTextureLevels;
    GLuint numCompressedTextureFormats;
    GLint *pCompressedTexturesFormats;
    GLuint maxAnistropic;
    GLuint maxTextureLodBias;

    GLuint maxTextureArraySize;
    GLuint maxTextureDepthSize;
    /* stream caps */
    GLuint maxElementsVertices;
    GLuint maxElementsIndices;
    GLuint64 maxElementIndex;
    GLuint maxStreams;

    /* Rasterization caps */
    GLfloat pointSizeMin;
    GLfloat pointSizeMax;
    GLfloat lineWidthMin;
    GLfloat lineWidthMax;
    GLfloat lineWidthGranularity;

    /* Renderbuffer properties limits */
    GLuint maxRenderBufferSize;
    GLuint maxSamples;
    GLuint maxSampleMaskWords;
    GLuint maxSamplesInteger;

    /* vertex array limits */
    GLuint maxVertexAttribBindings;
    GLuint maxVertexAttribRelativeOffset;
    GLuint maxVertexAttribStride;

    /* Misc */
    GLuint subpixelBits;
    GLuint64 maxServerWaitTimeout;

    /* Shader precision */
    struct __GLshaderPrecisionRec
    {
        GLint rangeLow;
        GLint rangeHigh;
        GLint precision;
    } shaderPrecision[__GLSL_STAGE_LAST][__GLSL_TYPE_LAST];

    /* Shader/program binary formats */
    GLint numShaderBinaryFormats;
    GLint *pShaderBinaryFormats;
    GLint numProgramBinaryFormats;
    GLint *pProgramBinaryFormats;
    GLenum gsLayerProvokingVertex;

    /* Shader multisample interpolation caps. */
    GLfloat minFragmentInterpolationOffset;
    GLfloat maxFragmentInterpolationOffset;
    GLint fragmentInterpolationOffsetBits;

    /* Shader caps */
    gcsGLSLCaps shaderCaps;

    GLint maxTextureBufferSize;
    GLint textureBufferOffsetAlignment;

#ifdef OPENGL40
    GLuint numberOfClipPlanes;
    GLuint maxEvalOrder;
    GLuint maxListNesting;
    GLuint maxModelViewStackDepth;
    GLuint maxProjectionStackDepth;
    GLuint maxTextureStackDepth;
    GLuint maxProgramStackDepth;
    GLuint numberOfLights;
    GLuint maxAttribStackDepth;
    GLuint maxClientAttribStackDepth;
    GLuint maxNameStackDepth;
    GLuint maxDrawBuffers;
    GLuint maxGeometryVaryingComponents;
    GLuint maxVertexVaryingComponents;
    GLfloat pointSizeGranularity;
    GLuint maxPixelMapTable;
#endif
} __GLdeviceConstants;

#endif /* __gc_es_consts_h__ */
