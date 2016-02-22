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


#ifndef __gc_gl_enable_h_
#define __gc_gl_enable_h_

typedef struct {
    GLboolean normalize;
    GLboolean rescaleNormal;
    GLuint    clipPlanesMask;
} __GLTransformEnableState;

typedef struct {
    GLboolean lighting;
    GLboolean colorMaterial;
    GLboolean light[__GL_MAX_LIGHT_NUMBER];
} __GLLightingEnableState;

typedef struct {
    GLboolean map1[__GL_MAP_RANGE_COUNT];
    GLboolean map2[__GL_MAP_RANGE_COUNT];
    GLboolean autonormal;
} __GLEvalEnableState;

typedef struct {
    GLboolean texGen[4];

    GLboolean texture1D;
    GLboolean texture2D;
    GLboolean texture3D;
    GLboolean textureCubeMap;
    GLboolean textureRec;

    /* Texture enable dimension based on texture1D, 2D, 3D, CubeMap flags.
    */
    GLuint enabledDimension;    /* fix function ps enable dimension */
    GLuint programVSEnabledDimension; /* programmable vs enable dimension */
    GLuint programGSEnabledDimension; /* programmable gs enable dimension */
    GLuint programPSEnabledDimension; /* programmable ps enable dimension */
} __GLTextureEnableState;

typedef struct {
    GLboolean alphaTest;
    GLboolean blend[__GL_MAX_DRAW_BUFFERS];
    GLboolean dither;
    GLboolean logicOp;
    GLboolean colorLogicOp, indexLogicOp;
} __GLColorBufferEnableState;

typedef struct {
    GLboolean smooth;
    GLboolean stipple;
    GLboolean cullFace;
    GLboolean polygonOffsetPoint, polygonOffsetLine, polygonOffsetFill;
} __GLPolygonEnableState;

typedef struct {
    GLboolean test;
} __GLDepthEnableState;

typedef struct {
    GLboolean smooth;
    GLboolean stipple;
    GLboolean stippleRequested;
} __GLLineEnableState;

typedef struct {
    GLboolean convolution1D;
    GLboolean convolution2D;
    GLboolean separable2D;
} __GLConvolutionEnableState;

typedef struct {
    GLboolean multisampleOn;
    GLboolean alphaToCoverage;
    GLboolean alphaToOne;
    GLboolean coverage;
} __GLMultisampleEnableState;

typedef struct  {
    GLboolean vpPointSize;
    GLboolean vpTwoSize;
    GLboolean vertexProgram;
    GLboolean fragmentProgram;
}__GLProgramEnableState;

typedef struct __GLenableStateRec {
    __GLTransformEnableState   transform;
    __GLLightingEnableState    lighting;
    __GLEvalEnableState        eval;
    __GLTextureEnableState     texUnits[__GL_MAX_TEXTURE_UNITS];
    __GLColorBufferEnableState colorBuffer;
    __GLPolygonEnableState     polygon;
    __GLDepthEnableState       depthBuffer;
    __GLLineEnableState        line;
    __GLConvolutionEnableState convolution;
    __GLMultisampleEnableState multisample;
    __GLProgramEnableState     program;
    GLboolean                  pointSmooth;
    GLboolean                  fog;
    GLboolean                  scissor;
    GLboolean                  stencilTest;
    GLboolean                  stencilTestTwoSideExt;
    GLboolean                  colorSum;
    GLboolean                  colorTable;
    GLboolean                  postConvColorTable;
    GLboolean                  postColorMatrixColorTable;
    GLboolean                  histogram;
    GLboolean                  minmax;
    GLboolean                  depthBoundTest;
      GLboolean                pointSprite;
} __GLenableState;

#endif /* __gc_gl_enable_h_ */
