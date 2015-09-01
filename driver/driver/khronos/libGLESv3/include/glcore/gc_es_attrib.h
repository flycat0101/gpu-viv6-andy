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


#ifndef __gc_es_attrib_h__
#define __gc_es_attrib_h__

/*
** The current state for Vertex.
*/
typedef struct __GLcurrentStateRec
{
    __GLcoord attribute[__GL_MAX_VERTEX_ATTRIBUTES];
} __GLcurrentState;

/*
** Line state.  Contains all the client controllable line state.
*/
typedef struct __GLlineStateRec
{
    GLfloat  requestedWidth;
    GLint    aliasedWidth;
} __GLlineState;

/*
** Polygon state.  Contains all the user controllable polygon state
*/
typedef struct __GLpolygonStateRec
{
    GLenum cullFace;
    GLenum frontFace;

    /* Polygon offset */
    GLfloat factor;
    GLfloat units;
} __GLpolygonState;

/*
** Depth state.  Contains all the user settable depth state.
*/
typedef struct __GLdepthStateRec
{
    /*
    ** Depth buffer test function.  The z value is compared using zFunction
    ** against the current value in the zbuffer.  If the comparison
    ** succeeds the new z value is written into the z buffer masked
    ** by the z write mask.
    */
    GLenum testFunc;

    /*
    ** Writemask enable.  When GL_TRUE writing to the depth buffer is
    ** allowed.
    */
    GLboolean writeEnable;

    /*
    ** Value used to clear the z buffer when glClear is called.
    */
    GLfloat clear;

    /*
    ** Depthrange parameters from user.
    */
    GLfloat zNear, zFar;
} __GLdepthState;

typedef struct __GLstencilFaceStateRec
{
    GLenum testFunc;
    GLint  reference;
    GLuint mask;
    GLenum fail;
    GLenum depthFail;
    GLenum depthPass;

    GLint writeMask;
} __GLstencilFaceState;

typedef struct __GLstencilStateRec
{
    __GLstencilFaceState front;
    __GLstencilFaceState back;

    GLint clear;
}__GLstencilState;

/*
** Viewport state.
*/
typedef struct __GLviewportRec
{
    /* Viewport parameters from user, as integers. */
    GLint x, y;
    GLsizei width, height;
} __GLviewport;

/*
** Raster state.
*/
typedef struct __GLrasterStateRec
{
    /* Alpha blending source and destination factors, blend op,
    ** and blend color.
    */
    GLenum    blendEquationRGB[__GL_MAX_DRAW_BUFFERS];
    GLenum    blendEquationAlpha[__GL_MAX_DRAW_BUFFERS];

    GLenum    blendSrcRGB[__GL_MAX_DRAW_BUFFERS];
    GLenum    blendDstRGB[__GL_MAX_DRAW_BUFFERS];
    GLenum    blendSrcAlpha[__GL_MAX_DRAW_BUFFERS];
    GLenum    blendDstAlpha[__GL_MAX_DRAW_BUFFERS];

    __GLcolor blendColor;

    /*
    ** Color to fill the color portion of the framebuffer when clear
    ** is called.
    */
    union __GLclearColorUnion
    {
        __GLcolor    clear;
        __GLcolorIui clearIui;
        __GLcolorIi  clearIi;
    } clearColor;

    /*
    ** RGB write masks.  These booleans enable or disable writing of
    ** the r, g, b, and a components.
    */
    struct __GLcolorMaskRec
    {
        GLboolean redMask;
        GLboolean greenMask;
        GLboolean blueMask;
        GLboolean alphaMask;
    } colorMask[__GL_MAX_DRAW_BUFFERS];

    /* This state variable tracks which buffers are drawn into. */
    GLenum    drawBuffers[__GL_MAX_DRAW_BUFFERS];
    GLboolean mrtEnable;

    GLenum    readBuffer;
} __GLrasterState;

/*
** Pixel state
*/
typedef struct __GLpixelPackModeRec
{
    GLuint alignment;
    GLuint lineLength;
    GLuint imageHeight;
    GLuint skipPixels;
    GLuint skipLines;
    GLuint skipImages;
} __GLpixelPackMode;

typedef struct __GLclientPixelStateRec
{
    __GLpixelPackMode packModes;
    __GLpixelPackMode unpackModes;
} __GLclientPixelState;

/*
** Hint state.  Contains all the user controllable hint state.
*/
typedef struct __GLhitStateRec
{
    GLenum generateMipmap;
    GLenum fsDerivative;
} __GLhintState;

/*
** Scissor state from user.
*/
typedef struct __GLscissorRec
{
    GLint scissorX;
    GLint scissorY;
    GLint scissorWidth;
    GLint scissorHeight;
} __GLscissor;

/*
** Multisample state from user.
*/
typedef struct __GLmultisampleStateRec
{
    GLboolean coverageInvert;
    GLfloat   coverageValue;
    GLbitfield sampleMaskValue;
    GLfloat   minSampleShadingValue;
} __GLmultisampleState;

typedef struct __GLcolorBufferStateRec
{
    GLboolean dither;
    GLboolean blend[__GL_MAX_DRAW_BUFFERS];
} __GLColorBufferEnableState;

typedef struct __GLpolygonEnableStateRec
{
    GLboolean cullFace;
    GLboolean polygonOffsetFill;
} __GLPolygonEnableState;

typedef struct __GLmultisampleEnableStateRec
{
    GLboolean alphaToCoverage;
    GLboolean coverage;
    GLboolean sampleMask;
    GLboolean sampleShading;
} __GLMultisampleEnableState;

typedef struct __GLenableStateRec
{
    __GLColorBufferEnableState colorBuffer;
    __GLPolygonEnableState     polygon;
    __GLMultisampleEnableState multisample;
    GLboolean                  scissorTest;
    GLboolean                  depthTest;
    GLboolean                  stencilTest;
    GLboolean                  primitiveRestart;
    GLboolean                  rasterizerDiscard;
} __GLenableState;

typedef struct __GLprimBoundStateRec
{
    GLfloat minX;
    GLfloat minY;
    GLfloat minZ;
    GLfloat minW;
    GLfloat maxX;
    GLfloat maxY;
    GLfloat maxZ;
    GLfloat maxW;
} __GLprimBoundState;


/* Record the last samper2TexUnit mapping */
typedef struct __GLprogramStateRec
{
    GLuint sampler2TexUnit[__GL_MAX_GLSL_SAMPLERS];
} __GLprogramState;

/*
***********************************************************************
*/
typedef struct __GLattributeRec
{
    __GLcurrentState current;
    __GLlineState line;
    __GLpolygonState polygon;
    __GLdepthState depth;
    __GLstencilState stencil;
    __GLviewport viewport;
    __GLenableState enables;
    __GLrasterState raster;
    __GLhintState hints;
    __GLscissor scissor;
    __GLmultisampleState multisample;
    __GLtextureState texture;
    __GLprogramState program;
    __GLimageState image;
    __GLprimBoundState primBound;
} __GLattribute;

typedef struct __GLclientAttributeRec
{
    __GLclientPixelState pixel;
} __GLclientAttribute;


#ifdef __cplusplus
extern "C" {
#endif

extern GLvoid __glOverturnCommitStates(__GLcontext *gc);
extern GLvoid __glSetAttributeStatesDirty(__GLcontext *gc);

#ifdef __cplusplus
}
#endif

#endif /* __gc_es_attrib_h__ */
