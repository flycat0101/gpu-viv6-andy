/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_gl_consts_h_
#define __gc_gl_consts_h_

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
#define __gl4294965000  (4294965000.0f)
#define __gl4294967295  (4294967295.0f)        /* 2^32 - 1 */
#define __glFloatMax    3.402823466e+38f       /* max value */
#define __glFloatMin    -3.402823466e+38f      /* min value */

#define __glMaxByte     127.0f
#define __glMaxUbyte    255.0f
#define __glMaxShort    32767.0f
#define __glMaxUshort   65535.0f               /* 2^16 - 1 */
#if defined(_WIN64)
#define __glMaxInt        9223372036854775808.0    /* 2^63 - 1 double */
#define __glMaxUint       18446744073709551615.0    /* 2^64 - 1 double */
#else
#define __glMaxInt        2147483647.0    /* 2^31 - 1 double */
#define __glMinInt        -2147483647.0
#define __glMaxUint       4294967295.0    /* 2^32 - 1 double */
#endif

#define __glInvMaxByte     (0.007874015748031496062992125984252f)
#define __glInvMaxUbyte    (0.003921568627450980392156862745098f)
#define __glInvMaxShort    (3.0518509475997192297128208258309e-5f)
#define __glInvMaxUshort   (1.5259021896696421759365224689097e-5f)
#define __glInvMaxInt      (4.656612875245796924105750827168e-10)     /* double */
#define __glInvMaxUint     (2.3283064370807973754314699618685e-10)    /* double */
#define __glInvMaxUint24   (5.9604648328104515558750364705942e-8)



#define __GL_MAX_MAX_VIEWPORT                    2048
#define __GL_MAX_TEXTURE_COORDS                  8
#define __GL_MAX_TEXTURE_UNITS                   48
#define __GL_MAX_LIGHT_NUMBER                    32
#define __GL_MAX_CLIPPLANE_NUMBER                8

#define __GL_MAX_VERTEX_STREAMS                  16
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

#define __GL_TOTAL_VERTEX_ATTRIBUTES (__GL_MAX_PROGRAM_VERTEX_ATTRIBUTES + __GL_MAX_TEXTURE_COORDS + 8)

#define __GL_MAX_COLOR_ATTACHMENTS              (__GL_MAX_DRAW_BUFFERS)
#define __GL_MAX_ATTACHMENTS                    (__GL_MAX_COLOR_ATTACHMENTS+2)

#define __GL_ONE_32        ((GLuint)1)
#define __GL_ONE_64        ((GLuint64)1)

#define __GL_FLOAT      0       /* __GLfloat */
#define __GL_FLOAT32    1       /* api 32 bit float */
#define __GL_FLOAT64    2       /* api 64 bit float */
#define __GL_INT32      3       /* api 32 bit GLint */
#define __GL_BOOLEAN    4       /* api 8 bit boolean */
#define __GL_COLOR      5       /* unscaled color in __GLfloat */
#define __GL_SCOLOR     6       /* scaled color in __GLfloat */

/*
** Various constants.  Most of these will never change through the life
** of the context.
*/
typedef struct __GLdeviceConstantsRec {
    /* Stuff for glGetString */
    GLbyte *vendor;
    GLbyte *renderer;
    GLbyte *version;
    GLbyte *extensions;
    GLbyte *GLSLVersion;

    /* Specific size limits */
    GLuint numberOfLights;
    GLuint numberOfClipPlanes;
    GLuint numberofQueryCounterBits;
    GLuint maxViewportWidth;
    GLuint maxViewportHeight;
    GLuint maxDrawBuffers;

    /* Texture constants*/
    GLuint numberOfTextureUnits;
    GLuint maxTextureSize;
    GLuint maxNumTextureLevels;
    GLuint maxTextureLodBias;
    GLuint maxTextureMaxAnisotropy;
    GLuint maxTextureArraySize;
    GLuint maxTextureBufferSize;
    GLuint maxVSTextureImageUnits;
    GLuint maxCombinedTextureImageUnits;
    GLuint minProgramTexelOffset;
    GLuint maxProgramTexelOffset;

    /* Geometry shader related constants */
    GLuint maxGeometryTextureImageUnits;
    GLuint maxGeometryVaryingComponents;
    GLuint maxVertexVaryingComponents;
    GLuint maxVaryingComponents;
    GLuint maxGeometryUniformComponents;
    GLuint maxGeometryOutputVertices;
    GLuint maxGeometryTotalOuputComponents;
    GLuint maxVertexUniformComponents;
    GLuint maxFragmentUniformComponents;

    /* bindable uniform */
    GLuint maxVertexBindableUniform;
    GLuint maxFragmentBindableUniform;
    GLuint maxGeometryBindableUniform;
    GLuint maxBindableUniformSize;

    GLuint subpixelBits;
    GLuint maxListNesting;
    GLuint maxEvalOrder;
    GLuint maxAttribStackDepth;
    GLuint maxClientAttribStackDepth;
    GLuint maxNameStackDepth;
    GLuint maxModelViewStackDepth;
    GLuint maxProjectionStackDepth;
    GLuint maxTextureStackDepth;
    GLuint maxProgramStackDepth;
    GLuint maxColorStackDepth;
    GLuint maxElementsVertices;
    GLuint maxElementsIndices;
    GLuint maxPixelMapTable;
    GLuint maxConvolution1DWidth;
    GLuint maxConvolution2DWidth;
    GLuint maxConvolution2DHeight;
    GLuint maxSeparable2DWidth;
    GLuint maxSeparable2DHeight;
    GLuint maxVertexIndex;
    GLuint maxStreams;
    GLuint maxStreamStride;
    GLuint maxProgErrStrLen;

    GLfloat pointSizeMinimum;
    GLfloat pointSizeMaximum;
    GLfloat pointSizeGranularity;
    GLfloat lineWidthMinimum;
    GLfloat lineWidthMaximum;
    GLfloat lineWidthGranularity;

    /* program limitation */
    GLuint maxInstruction[__GL_NUMBER_OF_PROGRAM_TARGET];
    GLuint maxTemp[__GL_NUMBER_OF_PROGRAM_TARGET];
    GLuint maxParameter[__GL_NUMBER_OF_PROGRAM_TARGET];
    GLuint maxAttribute[__GL_NUMBER_OF_PROGRAM_TARGET];
    GLuint maxAddressRegister[__GL_NUMBER_OF_PROGRAM_TARGET];
    GLuint maxLocalParameter[__GL_NUMBER_OF_PROGRAM_TARGET];
    GLuint maxEnvParameter[__GL_NUMBER_OF_PROGRAM_TARGET];
    GLuint maxPSALUInstr;
    GLuint maxPSTexInstr;
    GLuint maxPSTexIndirectionInstr;

    /*
    ** Use texture to do DrawPixel/ReadPixel/CopyPixel simulation.
    */
    GLboolean pixelPipelineDrawSimulation;
    GLboolean pixelPipelineCopySimulation;

    /*
    ** Renderbuffer propertities limitation
    */
    GLuint maxRenderBufferWidth;
    GLuint maxRenderBufferHeight;
    GLuint maxSamples;

} __GLdeviceConstants;


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

#endif /* __gc_gl_consts_h_ */
