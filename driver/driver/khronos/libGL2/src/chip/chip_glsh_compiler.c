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


#include "gc_gl_context.h"
#include "gc_gl_bindtable.h"
#include "chip_context.h"
#include "gc_hal_user_shader.h"

#define _GC_OBJ_ZONE glvZONE_TRACE

/* Built-In Variables */
typedef struct _slsBUILT_IN_VARIABLE_INFO
{
    gctCONST_STRING     implSymbol;
    __GLSLUniformType   builtInUniformId;

} slsBUILT_IN_VARIABLE_INFO;

static GLenum samplerType2TexEnableDim[] = {
    1,          /* __GLSL_DATATYPE_SAMPLER_1D         */
    2,          /* __GLSL_DATATYPE_SAMPLER_2D         */
    3,          /* __GLSL_DATATYPE_SAMPLER_3D         */
    4,          /* __GLSL_DATATYPE_SAMPLER_CUBE       */
    5,          /* __GLSL_DATATYPE_SAMPLER_2D_RECT    */
    5,          /* __GLSL_DATATYPE_SAMPLER_2D_RECT_SHADOW */
    1,          /* __GLSL_DATATYPE_SAMPLER_1D_SHADOW  */
    2,          /* __GLSL_DATATYPE_SAMPLER_2D_SHADOW  */

    6,          /* __GLSL_DATATYPE_SAMPLER_1D_ARRAY */
    7,          /* __GLSL_DATATYPE_SAMPLER_2D_ARRAY */
    6,          /* __GLSL_DATATYPE_SAMPLER_1D_ARRAY_SHADOW */
    7,          /* __GLSL_DATATYPE_SAMPLER_2D_ARRAY_SHADOW */
    4,          /* __GLSL_DATATYPE_SAMPLER_CUBE_SHADOW */
    8,          /* __GLSL_DATATYPE_SAMPLER_BUFFER */

    1,          /* __GLSL_DATATYPE_INT_SAMPLER_1D */
    2,          /* __GLSL_DATATYPE_INT_SAMPLER_2D */
    3,          /* __GLSL_DATATYPE_INT_SAMPLER_3D */
    4,          /* __GLSL_DATATYPE_INT_SAMPLER_CUBE */
    5,          /* __GLSL_DATATYPE_INT_SAMPLER_2D_RECT */
    6,          /* __GLSL_DATATYPE_INT_SAMPLER_1D_ARRAY */
    7,          /* __GLSL_DATATYPE_INT_SAMPLER_2D_ARRAY */
    8,          /* __GLSL_DATATYPE_INT_SAMPLER_BUFFER */

    1,          /* __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_1D */
    2,          /* __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_2D */
    3,          /* __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_3D */
    4,          /* __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_CUBE */
    5,          /* __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_RECT */
    6,          /* __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_1D_ARRAY */
    7,          /* __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_ARRAY */
    8,          /* __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_BUFFER */
};

static const GLenum gc2glType[] =
{
    GL_FLOAT,               /* gcUNIFORM_FLOAT_X1 */
    GL_FLOAT_VEC2,          /* gcUNIFORM_FLOAT_X2 */
    GL_FLOAT_VEC3,          /* gcUNIFORM_FLOAT_X3 */
    GL_FLOAT_VEC4,          /* gcUNIFORM_FLOAT_X4 */
    GL_FLOAT_MAT2,          /* gcUNIFORM_FLOAT_2X2 */
    GL_FLOAT_MAT3,          /* gcUNIFORM_FLOAT_3X3 */
    GL_FLOAT_MAT4,          /* gcUNIFORM_FLOAT_4X4 */
    GL_BOOL,                /* gcUNIFORM_BOOLEAN_X1 */
    GL_BOOL_VEC2,           /* gcUNIFORM_BOOLEAN_X2 */
    GL_BOOL_VEC3,           /* gcUNIFORM_BOOLEAN_X3 */
    GL_BOOL_VEC4,           /* gcUNIFORM_BOOLEAN_X4 */
    GL_INT,                 /* gcUNIFORM_INTEGER_X1 */
    GL_INT_VEC2,            /* gcUNIFORM_INTEGER_X2 */
    GL_INT_VEC3,            /* gcUNIFORM_INTEGER_X3 */
    GL_INT_VEC4,            /* gcUNIFORM_INTEGER_X4 */
    GL_SAMPLER_1D,          /* gcUNIFORM_SAMPLER_1D */
    GL_SAMPLER_2D,          /* gcUNIFORM_SAMPLER_2D */
    GL_SAMPLER_3D,          /* gcUNIFORM_SAMPLER_3D */
    GL_SAMPLER_CUBE,        /* gcUNIFORM_SAMPLER_CUBIC */
    0,                      /* gcSHADER_FIXED_X1 */
    0,                      /* gcSHADER_FIXED_X2 */
    0,                      /* gcSHADER_FIXED_X3 */
    0,                      /* gcSHADER_FIXED_X4 */
    0,                      /* gcSHADER_IMAGE_2D */
    0,                      /* gcSHADER_IMAGE_3D */
    0,                      /* gcSHADER_SAMPLER */
    GL_FLOAT_MAT2x3,        /* gcSHADER_FLOAT_2X3 */
    GL_FLOAT_MAT2x4,        /* gcSHADER_FLOAT_2X4 */
    GL_FLOAT_MAT3x2,        /* gcSHADER_FLOAT_3X2 */
    GL_FLOAT_MAT3x4,        /* gcSHADER_FLOAT_3X4 */
    GL_FLOAT_MAT4x2,        /* gcSHADER_FLOAT_4X2 */
    GL_FLOAT_MAT4x3,        /* gcSHADER_FLOAT_4X3 */
    GL_INT_SAMPLER_2D_EXT,  /* gcSHADER_ISAMPLER_2D */
    GL_INT_SAMPLER_3D_EXT,  /* gcSHADER_ISAMPLER_3D */
    GL_INT_SAMPLER_CUBE_EXT,/* gcSHADER_ISAMPLER_CUBIC */
    GL_UNSIGNED_INT_SAMPLER_2D_EXT,     /* gcSHADER_USAMPLER_2D */
    GL_UNSIGNED_INT_SAMPLER_3D_EXT,    /* gcSHADER_USAMPLER_2D */
    GL_UNSIGNED_INT_SAMPLER_CUBE_EXT,  /* gcSHADER_USAMPLER_CUBIC */
};

const slsBUILT_IN_VARIABLE_INFO builtInUniformInfo[] = {
    {"#DepthRange.near",                            __GLSL_UNIFORM_DEPTHRANGE_NEAR},
    {"#DepthRange.far",                             __GLSL_UNIFORM_DEPTHRANGE_FAR},
    {"#DepthRange.diff",                            __GLSL_UNIFORM_DEPTHRANGE_DIFF},

    {"#ModelViewMatrix",                            __GLSL_UNIFORM_MODELVIEW},
    {"#ProjectionMatrix",                           __GLSL_UNIFORM_PROJECTION},
    {"#ModelViewProjectionMatrix",                  __GLSL_UNIFORM_MVP},
    /* Added by xch */
    {"#ff_MVP_Matrix",                  __GLSL_UNIFORM_MVP},
    {"#TextureMatrix",                              __GLSL_UNIFORM_TEXTURE_MATRIX},
    {"#NormalMatrix",                               __GLSL_UNIFORM_NORMAL_MATRIX},

    {"#ModelViewMatrixInverse",                     __GLSL_UNIFORM_MODELVIEW_INV},
    {"#ProjectionMatrixInverse",                    __GLSL_UNIFORM_PROJECTION_INV},
    {"#ModelViewProjectionMatrixInverse",           __GLSL_UNIFORM_MVP_INV},
    {"#TextureMatrixInverse",                       __GLSL_UNIFORM_TEXTURE_MATRIX_INV},

    {"#ModelViewMatrixTranspose",                   __GLSL_UNIFORM_MODELVIEW_TRANS},
    {"#ProjectionMatrixTranspose",                  __GLSL_UNIFORM_PROJECTION_TRANS},
    {"#ModelViewProjectionMatrixTranspose",         __GLSL_UNIFORM_MVP_TRANS},
    {"#TextureMatrixTranspose",                     __GLSL_UNIFORM_TEXTURE_MATRIX_TRANS},

    {"#ModelViewMatrixInverseTranspose",            __GLSL_UNIFORM_MODELVIEW_INVTRANS},
    {"#ProjectionMatrixInverseTranspose",           __GLSL_UNIFORM_PROJECTION_INVTRANS},
    {"#ModelViewProjectionMatrixInverseTranspose",  __GLSL_UNIFORM_MVP_INVTRANS},
    {"#TextureMatrixInverseTranspose",              __GLSL_UNIFORM_TEXTURE_MATRIX_INVTRANS},

    /* Normal scaling */
    {"#NormalScale",                                __GLSL_UNIFORM_NORMAL_SCALE},

    /* Clip plane */
    {"#ClipPlane",                                  __GLSL_UNIFORM_CLIP_PLANE},

    /* Point parameter */
    {"#Point.size",                                 __GLSL_UNIFORM_POINT_SIZE},
    {"#Point.sizeMin",                              __GLSL_UNIFORM_POINT_SIZE_MIN},
    {"#Point.sizeMax",                              __GLSL_UNIFORM_POINT_SIZE_MAX},
    {"#Point.fadeThresholdSize",                    __GLSL_UNIFORM_POINT_FADE_THRESHOLD_SIZE},
    {"#Point.distanceConstantAttenuation",          __GLSL_UNIFORM_POINT_DISTANCE_CONST_ATTENU},
    {"#Point.distanceLinearAttenuation",            __GLSL_UNIFORM_POINT_DISTANCE_LINEAR_ATTENU},
    {"#Point.distanceQuadraticAttenuation",         __GLSL_UNIFORM_POINT_DISTANCE_QUADRATIC_ATTENU},

    /* Material */
    {"#FrontMaterial.emission",                     __GLSL_UNIFORM_FRONT_MATERIAL_EMISSION},
    {"#FrontMaterial.ambient",                      __GLSL_UNIFORM_FRONT_MATERIAL_AMBIENT},
    {"#FrontMaterial.diffuse",                      __GLSL_UNIFORM_FRONT_MATERIAL_DIFFUSE},
    {"#FrontMaterial.specular",                     __GLSL_UNIFORM_FRONT_MATERIAL_SPECULAR},
    {"#FrontMaterial.shininess",                    __GLSL_UNIFORM_FRONT_MATERIAL_SHININESS},

    {"#BackMaterial.emission",                      __GLSL_UNIFORM_BACK_MATERIAL_EMISSION},
    {"#BackMaterial.ambient",                       __GLSL_UNIFORM_BACK_MATERIAL_AMBIENT},
    {"#BackMaterial.diffuse",                       __GLSL_UNIFORM_BACK_MATERIAL_DIFFUSE},
    {"#BackMaterial.specular",                      __GLSL_UNIFORM_BACK_MATERIAL_SPECULAR},
    {"#BackMaterial.shininess",                     __GLSL_UNIFORM_BACK_MATERIAL_SHININESS},

    /* Light source */
    {"#LightSource[0].ambient",                     __GLSL_UNIFORM_LIGHT0_AMBIENT},
    {"#LightSource[0].diffuse",                     __GLSL_UNIFORM_LIGHT0_DIFFUSE},
    {"#LightSource[0].specular",                    __GLSL_UNIFORM_LIGHT0_SPECULAR},
    {"#LightSource[0].position",                    __GLSL_UNIFORM_LIGHT0_POSITION},
    {"#LightSource[0].halfVector",                  __GLSL_UNIFORM_LIGHT0_HALF_VECTOR},
    {"#LightSource[0].spotDirection",               __GLSL_UNIFORM_LIGHT0_SPOT_DIRECTION},
    {"#LightSource[0].spotExponent",                __GLSL_UNIFORM_LIGHT0_SPOT_EXPONENT},
    {"#LightSource[0].spotCutoff",                  __GLSL_UNIFORM_LIGHT0_SPOT_CUTOFF},
    {"#LightSource[0].spotCosCutoff",               __GLSL_UNIFORM_LIGHT0_SPOT_COS_CUTOFF},
    {"#LightSource[0].constantAttenuation",         __GLSL_UNIFORM_LIGHT0_CONST_ATTENU},
    {"#LightSource[0].linearAttenuation",           __GLSL_UNIFORM_LIGHT0_LINEAR_ATTENU},
    {"#LightSource[0].quadraticAttenuation",        __GLSL_UNIFORM_LIGHT0_QUADRATIC_ATTENU},

    {"#LightSource[1].ambient",                     __GLSL_UNIFORM_LIGHT1_AMBIENT},
    {"#LightSource[1].diffuse",                     __GLSL_UNIFORM_LIGHT1_DIFFUSE},
    {"#LightSource[1].specular",                    __GLSL_UNIFORM_LIGHT1_SPECULAR},
    {"#LightSource[1].position",                    __GLSL_UNIFORM_LIGHT1_POSITION},
    {"#LightSource[1].halfVector",                  __GLSL_UNIFORM_LIGHT1_HALF_VECTOR},
    {"#LightSource[1].spotDirection",               __GLSL_UNIFORM_LIGHT1_SPOT_DIRECTION},
    {"#LightSource[1].spotExponent",                __GLSL_UNIFORM_LIGHT1_SPOT_EXPONENT},
    {"#LightSource[1].spotCutoff",                  __GLSL_UNIFORM_LIGHT1_SPOT_CUTOFF},
    {"#LightSource[1].spotCosCutoff",               __GLSL_UNIFORM_LIGHT1_SPOT_COS_CUTOFF},
    {"#LightSource[1].constantAttenuation",         __GLSL_UNIFORM_LIGHT1_CONST_ATTENU},
    {"#LightSource[1].linearAttenuation",           __GLSL_UNIFORM_LIGHT1_LINEAR_ATTENU},
    {"#LightSource[1].quadraticAttenuation",        __GLSL_UNIFORM_LIGHT1_QUADRATIC_ATTENU},

    {"#LightSource[2].ambient",                     __GLSL_UNIFORM_LIGHT2_AMBIENT},
    {"#LightSource[2].diffuse",                     __GLSL_UNIFORM_LIGHT2_DIFFUSE},
    {"#LightSource[2].specular",                    __GLSL_UNIFORM_LIGHT2_SPECULAR},
    {"#LightSource[2].position",                    __GLSL_UNIFORM_LIGHT2_POSITION},
    {"#LightSource[2].halfVector",                  __GLSL_UNIFORM_LIGHT2_HALF_VECTOR},
    {"#LightSource[2].spotDirection",               __GLSL_UNIFORM_LIGHT2_SPOT_DIRECTION},
    {"#LightSource[2].spotExponent",                __GLSL_UNIFORM_LIGHT2_SPOT_EXPONENT},
    {"#LightSource[2].spotCutoff",                  __GLSL_UNIFORM_LIGHT2_SPOT_CUTOFF},
    {"#LightSource[2].spotCosCutoff",               __GLSL_UNIFORM_LIGHT2_SPOT_COS_CUTOFF},
    {"#LightSource[2].constantAttenuation",         __GLSL_UNIFORM_LIGHT2_CONST_ATTENU},
    {"#LightSource[2].linearAttenuation",           __GLSL_UNIFORM_LIGHT2_LINEAR_ATTENU},
    {"#LightSource[2].quadraticAttenuation",        __GLSL_UNIFORM_LIGHT2_QUADRATIC_ATTENU},

    {"#LightSource[3].ambient",                     __GLSL_UNIFORM_LIGHT3_AMBIENT},
    {"#LightSource[3].diffuse",                     __GLSL_UNIFORM_LIGHT3_DIFFUSE},
    {"#LightSource[3].specular",                    __GLSL_UNIFORM_LIGHT3_SPECULAR},
    {"#LightSource[3].position",                    __GLSL_UNIFORM_LIGHT3_POSITION},
    {"#LightSource[3].halfVector",                  __GLSL_UNIFORM_LIGHT3_HALF_VECTOR},
    {"#LightSource[3].spotDirection",               __GLSL_UNIFORM_LIGHT3_SPOT_DIRECTION},
    {"#LightSource[3].spotExponent",                __GLSL_UNIFORM_LIGHT3_SPOT_EXPONENT},
    {"#LightSource[3].spotCutoff",                  __GLSL_UNIFORM_LIGHT3_SPOT_CUTOFF},
    {"#LightSource[3].spotCosCutoff",               __GLSL_UNIFORM_LIGHT3_SPOT_COS_CUTOFF},
    {"#LightSource[3].constantAttenuation",         __GLSL_UNIFORM_LIGHT3_CONST_ATTENU},
    {"#LightSource[3].linearAttenuation",           __GLSL_UNIFORM_LIGHT3_LINEAR_ATTENU},
    {"#LightSource[3].quadraticAttenuation",        __GLSL_UNIFORM_LIGHT3_QUADRATIC_ATTENU},

    {"#LightSource[4].ambient",                     __GLSL_UNIFORM_LIGHT4_AMBIENT},
    {"#LightSource[4].diffuse",                     __GLSL_UNIFORM_LIGHT4_DIFFUSE},
    {"#LightSource[4].specular",                    __GLSL_UNIFORM_LIGHT4_SPECULAR},
    {"#LightSource[4].position",                    __GLSL_UNIFORM_LIGHT4_POSITION},
    {"#LightSource[4].halfVector",                  __GLSL_UNIFORM_LIGHT4_HALF_VECTOR},
    {"#LightSource[4].spotDirection",               __GLSL_UNIFORM_LIGHT4_SPOT_DIRECTION},
    {"#LightSource[4].spotExponent",                __GLSL_UNIFORM_LIGHT4_SPOT_EXPONENT},
    {"#LightSource[4].spotCutoff",                  __GLSL_UNIFORM_LIGHT4_SPOT_CUTOFF},
    {"#LightSource[4].spotCosCutoff",               __GLSL_UNIFORM_LIGHT4_SPOT_COS_CUTOFF},
    {"#LightSource[4].constantAttenuation",         __GLSL_UNIFORM_LIGHT4_CONST_ATTENU},
    {"#LightSource[4].linearAttenuation",           __GLSL_UNIFORM_LIGHT4_LINEAR_ATTENU},
    {"#LightSource[4].quadraticAttenuation",        __GLSL_UNIFORM_LIGHT4_QUADRATIC_ATTENU},

    {"#LightSource[5].ambient",                     __GLSL_UNIFORM_LIGHT5_AMBIENT},
    {"#LightSource[5].diffuse",                     __GLSL_UNIFORM_LIGHT5_DIFFUSE},
    {"#LightSource[5].specular",                    __GLSL_UNIFORM_LIGHT5_SPECULAR},
    {"#LightSource[5].position",                    __GLSL_UNIFORM_LIGHT5_POSITION},
    {"#LightSource[5].halfVector",                  __GLSL_UNIFORM_LIGHT5_HALF_VECTOR},
    {"#LightSource[5].spotDirection",               __GLSL_UNIFORM_LIGHT5_SPOT_DIRECTION},
    {"#LightSource[5].spotExponent",                __GLSL_UNIFORM_LIGHT5_SPOT_EXPONENT},
    {"#LightSource[5].spotCutoff",                  __GLSL_UNIFORM_LIGHT5_SPOT_CUTOFF},
    {"#LightSource[5].spotCosCutoff",               __GLSL_UNIFORM_LIGHT5_SPOT_COS_CUTOFF},
    {"#LightSource[5].constantAttenuation",         __GLSL_UNIFORM_LIGHT5_CONST_ATTENU},
    {"#LightSource[5].linearAttenuation",           __GLSL_UNIFORM_LIGHT5_LINEAR_ATTENU},
    {"#LightSource[5].quadraticAttenuation",        __GLSL_UNIFORM_LIGHT5_QUADRATIC_ATTENU},

    {"#LightSource[6].ambient",                     __GLSL_UNIFORM_LIGHT6_AMBIENT},
    {"#LightSource[6].diffuse",                     __GLSL_UNIFORM_LIGHT6_DIFFUSE},
    {"#LightSource[6].specular",                    __GLSL_UNIFORM_LIGHT6_SPECULAR},
    {"#LightSource[6].position",                    __GLSL_UNIFORM_LIGHT6_POSITION},
    {"#LightSource[6].halfVector",                  __GLSL_UNIFORM_LIGHT6_HALF_VECTOR},
    {"#LightSource[6].spotDirection",               __GLSL_UNIFORM_LIGHT6_SPOT_DIRECTION},
    {"#LightSource[6].spotExponent",                __GLSL_UNIFORM_LIGHT6_SPOT_EXPONENT},
    {"#LightSource[6].spotCutoff",                  __GLSL_UNIFORM_LIGHT6_SPOT_CUTOFF},
    {"#LightSource[6].spotCosCutoff",               __GLSL_UNIFORM_LIGHT6_SPOT_COS_CUTOFF},
    {"#LightSource[6].constantAttenuation",         __GLSL_UNIFORM_LIGHT6_CONST_ATTENU},
    {"#LightSource[6].linearAttenuation",           __GLSL_UNIFORM_LIGHT6_LINEAR_ATTENU},
    {"#LightSource[6].quadraticAttenuation",        __GLSL_UNIFORM_LIGHT6_QUADRATIC_ATTENU},

    {"#LightSource[7].ambient",                     __GLSL_UNIFORM_LIGHT7_AMBIENT},
    {"#LightSource[7].diffuse",                     __GLSL_UNIFORM_LIGHT7_DIFFUSE},
    {"#LightSource[7].specular",                    __GLSL_UNIFORM_LIGHT7_SPECULAR},
    {"#LightSource[7].position",                    __GLSL_UNIFORM_LIGHT7_POSITION},
    {"#LightSource[7].halfVector",                  __GLSL_UNIFORM_LIGHT7_HALF_VECTOR},
    {"#LightSource[7].spotDirection",               __GLSL_UNIFORM_LIGHT7_SPOT_DIRECTION},
    {"#LightSource[7].spotExponent",                __GLSL_UNIFORM_LIGHT7_SPOT_EXPONENT},
    {"#LightSource[7].spotCutoff",                  __GLSL_UNIFORM_LIGHT7_SPOT_CUTOFF},
    {"#LightSource[7].spotCosCutoff",               __GLSL_UNIFORM_LIGHT7_SPOT_COS_CUTOFF},
    {"#LightSource[7].constantAttenuation",         __GLSL_UNIFORM_LIGHT7_CONST_ATTENU},
    {"#LightSource[7].linearAttenuation",           __GLSL_UNIFORM_LIGHT7_LINEAR_ATTENU},
    {"#LightSource[7].quadraticAttenuation",        __GLSL_UNIFORM_LIGHT7_QUADRATIC_ATTENU},

    /* Light model ambient */
    {"#LightModel.ambient",                         __GLSL_UNIFORM_LIGHTMODEL_AMBIENT},

    /* Light model product */
    {"#FrontLightModelProduct.sceneColor",          __GLSL_UNIFORM_FRONT_LIGHTMODEL_PRODUCT_SCENECOLOR},
    {"#BackLightModelProduct.sceneColor",           __GLSL_UNIFORM_BACK_LIGHTMODEL_PRODUCT_SCENECOLOR},

    /* Light product */
    {"#FrontLightProduct[0].ambient",               __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_AMBIENT},
    {"#FrontLightProduct[0].diffuse",               __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_DIFFUSE},
    {"#FrontLightProduct[0].specular",              __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_SPECULAR},
    {"#BackLightProduct[0].ambient",                __GLSL_UNIFORM_BACK_LIGHT0_PRODUCT_AMBIENT},
    {"#BackLightProduct[0].diffuse",                __GLSL_UNIFORM_BACK_LIGHT0_PRODUCT_DIFFUSE},
    {"#BackLightProduct[0].specular",               __GLSL_UNIFORM_BACK_LIGHT0_PRODUCT_SPECULAR},

    {"#FrontLightProduct[1].ambient",               __GLSL_UNIFORM_FRONT_LIGHT1_PRODUCT_AMBIENT},
    {"#FrontLightProduct[1].diffuse",               __GLSL_UNIFORM_FRONT_LIGHT1_PRODUCT_DIFFUSE},
    {"#FrontLightProduct[1].specular",              __GLSL_UNIFORM_FRONT_LIGHT1_PRODUCT_SPECULAR},
    {"#BackLightProduct[1].ambient",                __GLSL_UNIFORM_BACK_LIGHT1_PRODUCT_AMBIENT},
    {"#BackLightProduct[1].diffuse",                __GLSL_UNIFORM_BACK_LIGHT1_PRODUCT_DIFFUSE},
    {"#BackLightProduct[1].specular",               __GLSL_UNIFORM_BACK_LIGHT1_PRODUCT_SPECULAR},

    {"#FrontLightProduct[2].ambient",               __GLSL_UNIFORM_FRONT_LIGHT2_PRODUCT_AMBIENT},
    {"#FrontLightProduct[2].diffuse",               __GLSL_UNIFORM_FRONT_LIGHT2_PRODUCT_DIFFUSE},
    {"#FrontLightProduct[2].specular",              __GLSL_UNIFORM_FRONT_LIGHT2_PRODUCT_SPECULAR},
    {"#BackLightProduct[2].ambient",                __GLSL_UNIFORM_BACK_LIGHT2_PRODUCT_AMBIENT},
    {"#BackLightProduct[2].diffuse",                __GLSL_UNIFORM_BACK_LIGHT2_PRODUCT_DIFFUSE},
    {"#BackLightProduct[2].specular",               __GLSL_UNIFORM_BACK_LIGHT2_PRODUCT_SPECULAR},

    {"#FrontLightProduct[3].ambient",               __GLSL_UNIFORM_FRONT_LIGHT3_PRODUCT_AMBIENT},
    {"#FrontLightProduct[3].diffuse",               __GLSL_UNIFORM_FRONT_LIGHT3_PRODUCT_DIFFUSE},
    {"#FrontLightProduct[3].specular",              __GLSL_UNIFORM_FRONT_LIGHT3_PRODUCT_SPECULAR},
    {"#BackLightProduct[3].ambient",                __GLSL_UNIFORM_BACK_LIGHT3_PRODUCT_AMBIENT},
    {"#BackLightProduct[3].diffuse",                __GLSL_UNIFORM_BACK_LIGHT3_PRODUCT_DIFFUSE},
    {"#BackLightProduct[3].specular",               __GLSL_UNIFORM_BACK_LIGHT3_PRODUCT_SPECULAR},

    {"#FrontLightProduct[4].ambient",               __GLSL_UNIFORM_FRONT_LIGHT4_PRODUCT_AMBIENT},
    {"#FrontLightProduct[4].diffuse",               __GLSL_UNIFORM_FRONT_LIGHT4_PRODUCT_DIFFUSE},
    {"#FrontLightProduct[4].specular",              __GLSL_UNIFORM_FRONT_LIGHT4_PRODUCT_SPECULAR},
    {"#BackLightProduct[4].ambient",                __GLSL_UNIFORM_BACK_LIGHT4_PRODUCT_AMBIENT},
    {"#BackLightProduct[4].diffuse",                __GLSL_UNIFORM_BACK_LIGHT4_PRODUCT_DIFFUSE},
    {"#BackLightProduct[4].specular",               __GLSL_UNIFORM_BACK_LIGHT4_PRODUCT_SPECULAR},

    {"#FrontLightProduct[5].ambient",               __GLSL_UNIFORM_FRONT_LIGHT5_PRODUCT_AMBIENT},
    {"#FrontLightProduct[5].diffuse",               __GLSL_UNIFORM_FRONT_LIGHT5_PRODUCT_DIFFUSE},
    {"#FrontLightProduct[5].specular",              __GLSL_UNIFORM_FRONT_LIGHT5_PRODUCT_SPECULAR},
    {"#BackLightProduct[5].ambient",                __GLSL_UNIFORM_BACK_LIGHT5_PRODUCT_AMBIENT},
    {"#BackLightProduct[5].diffuse",                __GLSL_UNIFORM_BACK_LIGHT5_PRODUCT_DIFFUSE},
    {"#BackLightProduct[5].specular",               __GLSL_UNIFORM_BACK_LIGHT5_PRODUCT_SPECULAR},

    {"#FrontLightProduct[6].ambient",               __GLSL_UNIFORM_FRONT_LIGHT6_PRODUCT_AMBIENT},
    {"#FrontLightProduct[6].diffuse",               __GLSL_UNIFORM_FRONT_LIGHT6_PRODUCT_DIFFUSE},
    {"#FrontLightProduct[6].specular",              __GLSL_UNIFORM_FRONT_LIGHT6_PRODUCT_SPECULAR},
    {"#BackLightProduct[6].ambient",                __GLSL_UNIFORM_BACK_LIGHT6_PRODUCT_AMBIENT},
    {"#BackLightProduct[6].diffuse",                __GLSL_UNIFORM_BACK_LIGHT6_PRODUCT_DIFFUSE},
    {"#BackLightProduct[6].specular",               __GLSL_UNIFORM_BACK_LIGHT6_PRODUCT_SPECULAR},

    {"#FrontLightProduct[7].ambient",               __GLSL_UNIFORM_FRONT_LIGHT7_PRODUCT_AMBIENT},
    {"#FrontLightProduct[7].diffuse",               __GLSL_UNIFORM_FRONT_LIGHT7_PRODUCT_DIFFUSE},
    {"#FrontLightProduct[7].specular",              __GLSL_UNIFORM_FRONT_LIGHT7_PRODUCT_SPECULAR},
    {"#BackLightProduct[7].ambient",                __GLSL_UNIFORM_BACK_LIGHT7_PRODUCT_AMBIENT},
    {"#BackLightProduct[7].diffuse",                __GLSL_UNIFORM_BACK_LIGHT7_PRODUCT_DIFFUSE},
    {"#BackLightProduct[7].specular",               __GLSL_UNIFORM_BACK_LIGHT7_PRODUCT_SPECULAR},

    /* Texture environment color */
    {"#TextureEnvColor",                            __GLSL_UNIFORM_TEXENV_COLOR},

    /* Texture coordinate generation */
    {"#EyePlaneS",                                  __GLSL_UNIFORM_TEXGEN_EYE_PLANE_S},
    {"#EyePlaneT",                                  __GLSL_UNIFORM_TEXGEN_EYE_PLANE_T},
    {"#EyePlaneR",                                  __GLSL_UNIFORM_TEXGEN_EYE_PLANE_R},
    {"#EyePlaneQ",                                  __GLSL_UNIFORM_TEXGEN_EYE_PLANE_Q},
    {"#ObjectPlaneS",                               __GLSL_UNIFORM_TEXGEN_OBJECT_PLANE_S},
    {"#ObjectPlaneT",                               __GLSL_UNIFORM_TEXGEN_OBJECT_PLANE_T},
    {"#ObjectPlaneT",                               __GLSL_UNIFORM_TEXGEN_OBJECT_PLANE_R},
    {"#ObjectPlaneQ",                               __GLSL_UNIFORM_TEXGEN_OBJECT_PLANE_Q},

    /* Fog */
    {"#Fog.color",                                  __GLSL_UNIFORM_FOG_COLOR},
    {"#Fog.density",                                __GLSL_UNIFORM_FOG_DENSITY},
    {"#Fog.start",                                  __GLSL_UNIFORM_FOG_START},
    {"#Fog.end",                                    __GLSL_UNIFORM_FOG_END},
    {"#Fog.scale",                                  __GLSL_UNIFORM_FOG_SCALE}
};

typedef struct _bt_attribute
{
gctINT index;
char name[128];
}BT_ATTRIBUTE;

static const BT_ATTRIBUTE builtInAttributesInfo[]={
    {_GL_VERTEX_INDEX,"#Vertex"},
    {_GL_COLOR_INDEX,"#AttrColor"},
    {_GL_MULTITEX0_INDEX,"#MultiTexCoord0"},
    {_GL_MULTITEX1_INDEX,"#MultiTexCoord1"},
    {_GL_MULTITEX2_INDEX,"#MultiTexCoord2"},
    {_GL_MULTITEX3_INDEX,"#MultiTexCoord3"},
    {_GL_MULTITEX4_INDEX,"#MultiTexCoord4"},
    {_GL_MULTITEX5_INDEX,"#MultiTexCoord5"},
    {_GL_MULTITEX6_INDEX,"#MultiTexCoord6"},
    {_GL_MULTITEX7_INDEX,"#MultiTexCoord7"}
};

static void  tagBuiltInAttribute(IN __GLcontext * gc, IN gctINT index, IN gctCONST_STRING attrname)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gctINT i = 0;
    for ( i = 0; i < sizeof(builtInAttributesInfo)/sizeof(builtInAttributesInfo[0]); i++ )
    {
        if ( gcmIS_SUCCESS(gcoOS_StrCmp(attrname, builtInAttributesInfo[i].name)) )
        {
            chipCtx->builtinAttributeIndex[builtInAttributesInfo[i].index] = index;
        }
    }
}

static void vsInputMaskForBuiltIns(IN __GLcontext * gc, GLuint *vsInputMask)
{
    gctUINT i = 0;
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);

    if ( chipCtx->builtinAttributeIndex[_GL_VERTEX_INDEX] >=0 )
        (*vsInputMask) |= __GL_INPUT_VERTEX;

    if ( chipCtx->builtinAttributeIndex[_GL_COLOR_INDEX] >=0 )
        (*vsInputMask) |= __GL_INPUT_DIFFUSE;

    for( i = _GL_MULTITEX0_INDEX; i < _GL_BT_INDEX_MAX; i++ )
    {
        if ( chipCtx->builtinAttributeIndex[i] >=0 )
            (*vsInputMask) |= (__GL_ONE_32 << (__GL_INPUT_TEX0_INDEX + i - _GL_MULTITEX0_INDEX));
    }
}

static GLsizei
gcType2Bytes(
    gcSHADER_TYPE Type
    )
{
    switch (Type)
    {
    case gcSHADER_FLOAT_X1:
        return 1 * sizeof(GLfloat);

    case gcSHADER_FLOAT_X2:
        return 2 * sizeof(GLfloat);

    case gcSHADER_FLOAT_X3:
        return 3 * sizeof(GLfloat);

    case gcSHADER_FLOAT_X4:
        return 4 * sizeof(GLfloat);

    case gcSHADER_FLOAT_2X2:
        return 2 * 2 * sizeof(GLfloat);

    case gcSHADER_FLOAT_3X3:
        return 3 * 3 * sizeof(GLfloat);

    case gcSHADER_FLOAT_4X4:
        return 4 * 4 * sizeof(GLfloat);

    case gcSHADER_BOOLEAN_X1:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_SAMPLER_1D:
    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_CUBIC:
        return 1 * sizeof(GLfloat);

    case gcSHADER_BOOLEAN_X2:
    case gcSHADER_INTEGER_X2:
        return 2 * sizeof(GLfloat);

    case gcSHADER_BOOLEAN_X3:
    case gcSHADER_INTEGER_X3:
        return 3 * sizeof(GLfloat);

    case gcSHADER_BOOLEAN_X4:
    case gcSHADER_INTEGER_X4:
        return 4 * sizeof(GLfloat);

    default:
        gcmFATAL("Unsupported type: %d", Type);
        return 0;
    }
}

static void
float2Bool(
    GLfloat * Destination,
    const GLfloat * Source,
    GLsizei Count
    )
{
    GLsizei i;

    for (i = 0; i < Count; ++i)
    {
        Destination[i] = (Source[i] == 0.0f) ? 0.0f : 1.0f;
    }
}


static void
int2Bool(
    GLfloat * Destination,
    const GLint * Source,
    GLsizei Count
    )
{
    GLsizei i;

    for (i = 0; i < Count; ++i)
    {
        Destination[i] = (Source[i] == 0) ? 0.0f : 1.0f;
    }
}

static void
_Int2Float(
    GLfloat * Destination,
    const GLint * Source,
    GLsizei Count
    )
{
    GLsizei i;

    for (i = 0; i < Count; ++i)
    {
        Destination[i] = (GLfloat) Source[i];
    }
}

static void
float2Int(
    GLint * Destination,
    const GLfloat * Source,
    GLsizei Count
    )
{
    GLsizei i;

    for (i = 0; i < Count; ++i)
    {
        Destination[i] = (GLint) Source[i];
    }
}
extern gceSTATUS set_uModelView(
    __GLcontext * gc,
    gcUNIFORM Uniform
    );

extern gceSTATUS set_uProjection(
    __GLcontext * gc,
    gcUNIFORM Uniform
    );

extern gceSTATUS set_uModelViewProjection(
    __GLcontext * gc,
    gcUNIFORM Uniform
    );

extern gceSTATUS set_uModelViewInverse3x3Transposed(
    __GLcontext * gc,
    gcUNIFORM Uniform
    );

static void setBuiltInUniform(__GLcontext *gc)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLUniform Uniform;
    GLProgram program = chipCtx->currGLSLProgram;
    GLint i;

    for (i = 0; i < program->builtInUniformCount; i++)
    {
        Uniform = &program->builtInUniforms[i];
        switch(Uniform->uniformID)
        {
            case __GLSL_UNIFORM_MODELVIEW:
                set_uModelView(gc, Uniform->uniform[0]);
                if (Uniform->uniform[1]) {
                    set_uModelView(gc, Uniform->uniform[1]);
                }
                break;
            case __GLSL_UNIFORM_PROJECTION:
                set_uProjection(gc, Uniform->uniform[0]);
                if (Uniform->uniform[0]) {
                    set_uProjection(gc, Uniform->uniform[1]);
                }
                break;
            case __GLSL_UNIFORM_MVP:
                set_uModelViewProjection(gc, Uniform->uniform[0]);
                if (Uniform->uniform[1]) {
                    set_uModelViewProjection(gc, Uniform->uniform[1]);
                }
                break;
            case __GLSL_UNIFORM_TEXTURE_MATRIX:
                 break;
            case __GLSL_UNIFORM_NORMAL_MATRIX:
                 break;
            case __GLSL_UNIFORM_MODELVIEW_INV:
                set_uModelViewInverse3x3Transposed(gc, Uniform->uniform[0]);
                if (Uniform->uniform[1]) {
                    set_uModelViewInverse3x3Transposed(gc, Uniform->uniform[1]);
                }
                break;
            case __GLSL_UNIFORM_PROJECTION_INV:
                 break;
            case __GLSL_UNIFORM_MVP_INV:
                 break;
            case __GLSL_UNIFORM_TEXTURE_MATRIX_INV:
                 break;

            case __GLSL_UNIFORM_MODELVIEW_TRANS:
                 break;
            case __GLSL_UNIFORM_PROJECTION_TRANS:
                 break;
            case __GLSL_UNIFORM_MVP_TRANS:
                 break;
            case __GLSL_UNIFORM_TEXTURE_MATRIX_TRANS:
                 break;
            case __GLSL_UNIFORM_MODELVIEW_INVTRANS:
                set_uModelViewInverse3x3Transposed(gc, Uniform->uniform[0]);
                if (Uniform->uniform[1]) {
                    set_uModelViewInverse3x3Transposed(gc, Uniform->uniform[1]);
                }
                break;
            case __GLSL_UNIFORM_PROJECTION_INVTRANS:
                 break;
            case __GLSL_UNIFORM_MVP_INVTRANS:
                 break;
            case __GLSL_UNIFORM_TEXTURE_MATRIX_INVTRANS:
                 break;
            case __GLSL_UNIFORM_NORMAL_SCALE:
                {
                    GLfloat  scale = 0;
                    __GLtransform *tr = NULL;
                    tr = gc->transform.modelView;
                    scale = tr->matrix.matrix[1][3] * tr->matrix.matrix[1][3] +
                        tr->matrix.matrix[2][3] * tr->matrix.matrix[2][3] +
                        tr->matrix.matrix[3][3] * tr->matrix.matrix[3][3];
                    scale = (GLfloat)(1.0 / sqrt(scale));
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, &scale);
                    if (Uniform->uniform[1]) {
                        gcUNIFORM_SetValueF_Ex(Uniform->uniform[1], 1, program->hints, &scale);
                    }
                }
                break;
            case __GLSL_UNIFORM_DEPTHRANGE_NEAR:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.viewport.zNear);
                break;
            case __GLSL_UNIFORM_DEPTHRANGE_FAR:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.viewport.zFar);
                break;
            case __GLSL_UNIFORM_DEPTHRANGE_DIFF:
                {
                    GLfloat v;
                    v = gc->state.viewport.zFar - gc->state.viewport.zNear;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&v);
                }
                break;
                 break;
            case __GLSL_UNIFORM_CLIP_PLANE:
                 break;
            case __GLSL_UNIFORM_POINT_SIZE:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.point.requestedSize);
                break;
            case __GLSL_UNIFORM_POINT_SIZE_MIN:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.point.sizeMin);
                break;
            case __GLSL_UNIFORM_POINT_SIZE_MAX:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.point.sizeMax);
                break;
            case __GLSL_UNIFORM_POINT_FADE_THRESHOLD_SIZE:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.point.fadeThresholdSize);
                break;
            case __GLSL_UNIFORM_POINT_DISTANCE_CONST_ATTENU:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.point.distanceAttenuation[0]);
                break;
            case __GLSL_UNIFORM_POINT_DISTANCE_LINEAR_ATTENU:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.point.distanceAttenuation[1]);
                break;
            case __GLSL_UNIFORM_POINT_DISTANCE_QUADRATIC_ATTENU:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.point.distanceAttenuation[2]);
                break;
            case __GLSL_UNIFORM_FRONT_MATERIAL_EMISSION:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.front.emissive));
                break;
            case __GLSL_UNIFORM_FRONT_MATERIAL_AMBIENT:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.front.ambient));
                break;
            case __GLSL_UNIFORM_FRONT_MATERIAL_DIFFUSE:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.front.diffuse));
                break;
            case __GLSL_UNIFORM_FRONT_MATERIAL_SPECULAR:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.front.specular));
                break;
            case __GLSL_UNIFORM_FRONT_MATERIAL_SHININESS:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.front.specularExponent));
                break;
            case __GLSL_UNIFORM_BACK_MATERIAL_EMISSION:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.back.emissive));
                break;
            case __GLSL_UNIFORM_BACK_MATERIAL_AMBIENT:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.back.ambient));
                break;
            case __GLSL_UNIFORM_BACK_MATERIAL_DIFFUSE:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.back.diffuse));
                break;
            case __GLSL_UNIFORM_BACK_MATERIAL_SPECULAR:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.back.specular));
                break;
            case __GLSL_UNIFORM_BACK_MATERIAL_SHININESS:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.back.specularExponent));
                break;
            case __GLSL_UNIFORM_LIGHT0_AMBIENT:
            case __GLSL_UNIFORM_LIGHT1_AMBIENT:
            case __GLSL_UNIFORM_LIGHT2_AMBIENT:
            case __GLSL_UNIFORM_LIGHT3_AMBIENT:
            case __GLSL_UNIFORM_LIGHT4_AMBIENT:
            case __GLSL_UNIFORM_LIGHT5_AMBIENT:
            case __GLSL_UNIFORM_LIGHT6_AMBIENT:
            case __GLSL_UNIFORM_LIGHT7_AMBIENT:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_AMBIENT;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.light.source[index].ambient);
                }
                break;

            case __GLSL_UNIFORM_LIGHT0_DIFFUSE:
            case __GLSL_UNIFORM_LIGHT1_DIFFUSE:
            case __GLSL_UNIFORM_LIGHT2_DIFFUSE:
            case __GLSL_UNIFORM_LIGHT3_DIFFUSE:
            case __GLSL_UNIFORM_LIGHT4_DIFFUSE:
            case __GLSL_UNIFORM_LIGHT5_DIFFUSE:
            case __GLSL_UNIFORM_LIGHT6_DIFFUSE:
            case __GLSL_UNIFORM_LIGHT7_DIFFUSE:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_DIFFUSE;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints,  (gctFLOAT *)&gc->state.light.source[index].diffuse);
                }
                break;

            case __GLSL_UNIFORM_LIGHT0_SPECULAR:
            case __GLSL_UNIFORM_LIGHT1_SPECULAR:
            case __GLSL_UNIFORM_LIGHT2_SPECULAR:
            case __GLSL_UNIFORM_LIGHT3_SPECULAR:
            case __GLSL_UNIFORM_LIGHT4_SPECULAR:
            case __GLSL_UNIFORM_LIGHT5_SPECULAR:
            case __GLSL_UNIFORM_LIGHT6_SPECULAR:
            case __GLSL_UNIFORM_LIGHT7_SPECULAR:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_SPECULAR;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.light.source[index].specular);
                }
                break;
            case __GLSL_UNIFORM_LIGHT0_POSITION:
            case __GLSL_UNIFORM_LIGHT1_POSITION:
            case __GLSL_UNIFORM_LIGHT2_POSITION:
            case __GLSL_UNIFORM_LIGHT3_POSITION:
            case __GLSL_UNIFORM_LIGHT4_POSITION:
            case __GLSL_UNIFORM_LIGHT5_POSITION:
            case __GLSL_UNIFORM_LIGHT6_POSITION:
            case __GLSL_UNIFORM_LIGHT7_POSITION:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_POSITION;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.light.source[index].position);
                }
                break;
            case __GLSL_UNIFORM_LIGHT0_HALF_VECTOR:
            case __GLSL_UNIFORM_LIGHT1_HALF_VECTOR:
            case __GLSL_UNIFORM_LIGHT2_HALF_VECTOR:
            case __GLSL_UNIFORM_LIGHT3_HALF_VECTOR:
            case __GLSL_UNIFORM_LIGHT4_HALF_VECTOR:
            case __GLSL_UNIFORM_LIGHT5_HALF_VECTOR:
            case __GLSL_UNIFORM_LIGHT6_HALF_VECTOR:
            case __GLSL_UNIFORM_LIGHT7_HALF_VECTOR:
                 break;
            case __GLSL_UNIFORM_LIGHT0_SPOT_DIRECTION:
            case __GLSL_UNIFORM_LIGHT1_SPOT_DIRECTION:
            case __GLSL_UNIFORM_LIGHT2_SPOT_DIRECTION:
            case __GLSL_UNIFORM_LIGHT3_SPOT_DIRECTION:
            case __GLSL_UNIFORM_LIGHT4_SPOT_DIRECTION:
            case __GLSL_UNIFORM_LIGHT5_SPOT_DIRECTION:
            case __GLSL_UNIFORM_LIGHT6_SPOT_DIRECTION:
            case __GLSL_UNIFORM_LIGHT7_SPOT_DIRECTION:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_SPOT_DIRECTION;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.light.source[index].direction);
                }
                break;
            case __GLSL_UNIFORM_LIGHT0_SPOT_EXPONENT:
            case __GLSL_UNIFORM_LIGHT1_SPOT_EXPONENT:
            case __GLSL_UNIFORM_LIGHT2_SPOT_EXPONENT:
            case __GLSL_UNIFORM_LIGHT3_SPOT_EXPONENT:
            case __GLSL_UNIFORM_LIGHT4_SPOT_EXPONENT:
            case __GLSL_UNIFORM_LIGHT5_SPOT_EXPONENT:
            case __GLSL_UNIFORM_LIGHT6_SPOT_EXPONENT:
            case __GLSL_UNIFORM_LIGHT7_SPOT_EXPONENT:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_SPOT_EXPONENT;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.light.source[index].spotLightExponent);
                }
                break;
            case __GLSL_UNIFORM_LIGHT0_SPOT_CUTOFF:
            case __GLSL_UNIFORM_LIGHT1_SPOT_CUTOFF:
            case __GLSL_UNIFORM_LIGHT2_SPOT_CUTOFF:
            case __GLSL_UNIFORM_LIGHT3_SPOT_CUTOFF:
            case __GLSL_UNIFORM_LIGHT4_SPOT_CUTOFF:
            case __GLSL_UNIFORM_LIGHT5_SPOT_CUTOFF:
            case __GLSL_UNIFORM_LIGHT6_SPOT_CUTOFF:
            case __GLSL_UNIFORM_LIGHT7_SPOT_CUTOFF:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_SPOT_CUTOFF;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.light.source[index].spotLightCutOffAngle);
                }
                break;
            case __GLSL_UNIFORM_LIGHT0_SPOT_COS_CUTOFF:
            case __GLSL_UNIFORM_LIGHT1_SPOT_COS_CUTOFF:
            case __GLSL_UNIFORM_LIGHT2_SPOT_COS_CUTOFF:
            case __GLSL_UNIFORM_LIGHT3_SPOT_COS_CUTOFF:
            case __GLSL_UNIFORM_LIGHT4_SPOT_COS_CUTOFF:
            case __GLSL_UNIFORM_LIGHT5_SPOT_COS_CUTOFF:
            case __GLSL_UNIFORM_LIGHT6_SPOT_COS_CUTOFF:
            case __GLSL_UNIFORM_LIGHT7_SPOT_COS_CUTOFF:
                break;
            case __GLSL_UNIFORM_LIGHT0_CONST_ATTENU:
            case __GLSL_UNIFORM_LIGHT1_CONST_ATTENU:
            case __GLSL_UNIFORM_LIGHT2_CONST_ATTENU:
            case __GLSL_UNIFORM_LIGHT3_CONST_ATTENU:
            case __GLSL_UNIFORM_LIGHT4_CONST_ATTENU:
            case __GLSL_UNIFORM_LIGHT5_CONST_ATTENU:
            case __GLSL_UNIFORM_LIGHT6_CONST_ATTENU:
            case __GLSL_UNIFORM_LIGHT7_CONST_ATTENU:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_CONST_ATTENU;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.light.source[index].constantAttenuation);
                }
                break;
            case __GLSL_UNIFORM_LIGHT0_LINEAR_ATTENU:
            case __GLSL_UNIFORM_LIGHT1_LINEAR_ATTENU:
            case __GLSL_UNIFORM_LIGHT2_LINEAR_ATTENU:
            case __GLSL_UNIFORM_LIGHT3_LINEAR_ATTENU:
            case __GLSL_UNIFORM_LIGHT4_LINEAR_ATTENU:
            case __GLSL_UNIFORM_LIGHT5_LINEAR_ATTENU:
            case __GLSL_UNIFORM_LIGHT6_LINEAR_ATTENU:
            case __GLSL_UNIFORM_LIGHT7_LINEAR_ATTENU:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_LINEAR_ATTENU;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.light.source[index].spotLightExponent);
                }
                break;
            case __GLSL_UNIFORM_LIGHT0_QUADRATIC_ATTENU:
            case __GLSL_UNIFORM_LIGHT1_QUADRATIC_ATTENU:
            case __GLSL_UNIFORM_LIGHT2_QUADRATIC_ATTENU:
            case __GLSL_UNIFORM_LIGHT3_QUADRATIC_ATTENU:
            case __GLSL_UNIFORM_LIGHT4_QUADRATIC_ATTENU:
            case __GLSL_UNIFORM_LIGHT5_QUADRATIC_ATTENU:
            case __GLSL_UNIFORM_LIGHT6_QUADRATIC_ATTENU:
            case __GLSL_UNIFORM_LIGHT7_QUADRATIC_ATTENU:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_LIGHT0_QUADRATIC_ATTENU;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.light.source[index].quadraticAttenuation);
                }
                break;
            case __GLSL_UNIFORM_LIGHTMODEL_AMBIENT:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.light.model.ambient));
                break;
            case __GLSL_UNIFORM_FRONT_LIGHTMODEL_PRODUCT_SCENECOLOR:
                {
                    GLfloat v[4];
                    __GLmaterialState *m = &gc->state.light.front;
                    __GLcolor* ma = &(gc->state.light.model.ambient);
                    v[0] = m->emissive.r + m->ambient.r * ma->r;
                    v[1] = m->emissive.g + m->ambient.g * ma->g;
                    v[2] = m->emissive.b + m->ambient.b * ma->b;
                    v[3] = m->emissive.a + m->ambient.a * ma->a;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)v);
                }
                break;
            case __GLSL_UNIFORM_BACK_LIGHTMODEL_PRODUCT_SCENECOLOR:
                {
                    GLfloat v[4];
                    __GLmaterialState *m = &gc->state.light.back;
                    __GLcolor* ma = &(gc->state.light.model.ambient);
                    v[0] = m->emissive.r + m->ambient.r * ma->r;
                    v[1] = m->emissive.g + m->ambient.g * ma->g;
                    v[2] = m->emissive.b + m->ambient.b * ma->b;
                    v[3] = m->emissive.a + m->ambient.a * ma->a;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)v);
                }
                 break;
            case __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_FRONT_LIGHT1_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_FRONT_LIGHT2_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_FRONT_LIGHT3_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_FRONT_LIGHT4_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_FRONT_LIGHT5_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_FRONT_LIGHT6_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_FRONT_LIGHT7_PRODUCT_AMBIENT:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_AMBIENT;
                    GLfloat v[4];
                    __GLcolor* acm  = &(gc->state.light.front.ambient);
                    __GLcolor* acli = &gc->state.light.source[index].ambient;
                    v[0] = acm->r * acli->r;
                    v[1] = acm->g * acli->g;
                    v[2] = acm->b * acli->b;
                    v[0] = acm->a * acli->a;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)v);
                }
                break;
            case __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_FRONT_LIGHT1_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_FRONT_LIGHT2_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_FRONT_LIGHT3_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_FRONT_LIGHT4_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_FRONT_LIGHT5_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_FRONT_LIGHT6_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_FRONT_LIGHT7_PRODUCT_DIFFUSE:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_DIFFUSE;
                    GLfloat v[4];
                    __GLcolor* dcm  = &(gc->state.light.front.diffuse);
                    __GLcolor* dcli = &gc->state.light.source[index].diffuse;
                    v[0] = dcm->r * dcli->r;
                    v[1] = dcm->g * dcli->g;
                    v[2] = dcm->b * dcli->b;
                    v[3] = dcm->a * dcli->a;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)v);
                }
                break;
            case __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_FRONT_LIGHT1_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_FRONT_LIGHT2_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_FRONT_LIGHT3_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_FRONT_LIGHT4_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_FRONT_LIGHT5_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_FRONT_LIGHT6_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_FRONT_LIGHT7_PRODUCT_SPECULAR:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_DIFFUSE;
                    GLfloat v[4];
                    __GLcolor* scm  = &(gc->state.light.front.specular);
                    __GLcolor* scli = &gc->state.light.source[index].specular;
                    v[0] = scm->r * scli->r;
                    v[1] = scm->g * scli->g;
                    v[2] = scm->b * scli->b;
                    v[3] = scm->a * scli->a;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)v);
                }
                break;
            case __GLSL_UNIFORM_BACK_LIGHT0_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_BACK_LIGHT1_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_BACK_LIGHT2_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_BACK_LIGHT3_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_BACK_LIGHT4_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_BACK_LIGHT5_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_BACK_LIGHT6_PRODUCT_AMBIENT:
            case __GLSL_UNIFORM_BACK_LIGHT7_PRODUCT_AMBIENT:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_AMBIENT;
                    GLfloat v[4];
                    __GLcolor* acm  = &(gc->state.light.back.ambient);
                    __GLcolor* acli = &gc->state.light.source[index].ambient;
                    v[0] = acm->r * acli->r;
                    v[1] = acm->g * acli->g;
                    v[2] = acm->b * acli->b;
                    v[0] = acm->a * acli->a;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)v);
                }
                break;
            case __GLSL_UNIFORM_BACK_LIGHT0_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_BACK_LIGHT1_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_BACK_LIGHT2_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_BACK_LIGHT3_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_BACK_LIGHT4_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_BACK_LIGHT5_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_BACK_LIGHT6_PRODUCT_DIFFUSE:
            case __GLSL_UNIFORM_BACK_LIGHT7_PRODUCT_DIFFUSE:
                {
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_DIFFUSE;
                    GLfloat v[4];
                    __GLcolor* dcm  = &(gc->state.light.back.diffuse);
                    __GLcolor* dcli = &gc->state.light.source[index].diffuse;
                    v[0] = dcm->r * dcli->r;
                    v[1] = dcm->g * dcli->g;
                    v[2] = dcm->b * dcli->b;
                    v[3] = dcm->a * dcli->a;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)v);
                }
                }
                break;
            case __GLSL_UNIFORM_BACK_LIGHT0_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_BACK_LIGHT1_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_BACK_LIGHT2_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_BACK_LIGHT3_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_BACK_LIGHT4_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_BACK_LIGHT5_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_BACK_LIGHT6_PRODUCT_SPECULAR:
            case __GLSL_UNIFORM_BACK_LIGHT7_PRODUCT_SPECULAR:
                {
                    GLint index = Uniform->uniformID - __GLSL_UNIFORM_FRONT_LIGHT0_PRODUCT_DIFFUSE;
                    GLfloat v[4];
                    __GLcolor* scm  = &(gc->state.light.back.specular);
                    __GLcolor* scli = &gc->state.light.source[index].specular;
                    v[0] = scm->r * scli->r;
                    v[1] = scm->g * scli->g;
                    v[2] = scm->b * scli->b;
                    v[3] = scm->a * scli->a;
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)v);
                }
                break;
            case __GLSL_UNIFORM_TEXENV_COLOR:
                 break;
            case __GLSL_UNIFORM_TEXGEN_EYE_PLANE_S:
                 break;
            case __GLSL_UNIFORM_TEXGEN_EYE_PLANE_T:
                 break;
            case __GLSL_UNIFORM_TEXGEN_EYE_PLANE_R:
                 break;
            case __GLSL_UNIFORM_TEXGEN_EYE_PLANE_Q:
                 break;
            case __GLSL_UNIFORM_TEXGEN_OBJECT_PLANE_S:
                 break;
            case __GLSL_UNIFORM_TEXGEN_OBJECT_PLANE_T:
                 break;
            case __GLSL_UNIFORM_TEXGEN_OBJECT_PLANE_R:
                 break;
            case __GLSL_UNIFORM_TEXGEN_OBJECT_PLANE_Q:
                 break;
            case __GLSL_UNIFORM_FOG_COLOR:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&(gc->state.fog.color));
                break;
            case __GLSL_UNIFORM_FOG_DENSITY:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.fog.density);
                break;
            case __GLSL_UNIFORM_FOG_START:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.fog.start);
                break;
            case __GLSL_UNIFORM_FOG_END:
                gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&gc->state.fog.end);
                break;
            case __GLSL_UNIFORM_FOG_SCALE:
                {
                    float fogScale = 1.0f / (gc->state.fog.end - gc->state.fog.start);
                    gcUNIFORM_SetValueF_Ex(Uniform->uniform[0], 1, program->hints, (gctFLOAT *)&fogScale);
                }
                break;
        }
    }
}

static void
countUniforms(
    IN GLProgram Program,
    IN gcSHADER Shader,
    IN GLint Count
    )
{
    GLint i;

    for (i = 0; i < Count; ++i)
    {
        gcUNIFORM uniform;
        gctSIZE_T length;
        gctCONST_STRING name;

        gcmVERIFY_OK(gcSHADER_GetUniform(Shader, i, &uniform));

        gcmVERIFY_OK(gcUNIFORM_GetName(uniform, &length, &name));

        if (name[0] == '#')
        {
            ++ Program->builtInUniformCount;
        }
        else
        {
            ++ Program->uniformCount;

            if ((GLsizei) length > Program->uniformMaxLength)
            {
                Program->uniformMaxLength = length;
            }
        }
    }
}

static GLint findBuiltInUnformID(gctCONST_STRING name)
{
    GLint i;
    GLint id = -1;

    for (i = 0; i < (sizeof(builtInUniformInfo) / sizeof(builtInUniformInfo[0])); i++)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(builtInUniformInfo[i].implSymbol, name)))
        {
            id = builtInUniformInfo[i].builtInUniformId;
            break;
        }
    }
    return id;
}


static GLint
processBuiltInUniform(
    IN __GLcontext *gc
)
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gctCONST_STRING name;
    GLint i;
    GLProgram program = chipCtx->currGLSLProgram;
    GLint uniformID;

    for (i = 0; i < program->builtInUniformCount; i++)
    {
        gcUNIFORM_GetName(program->builtInUniforms[i].uniform[0], gcvNULL, &name);
        uniformID = findBuiltInUnformID(name);
        program->builtInUniforms[i].uniformID = uniformID;
    }
    return 0;
}



static GLint
processUniforms(
    IN __GLcontext * gc,
    IN OUT __GLshaderProgramObject* programObject,
    IN gcSHADER Shader,
    IN GLint Count,
    IN GLboolean Duplicates,
    IN OUT GLint * Index,
    IN OUT GLint * PrivateIndex,
    IN OUT GLint * SamplerIndex
    )
{
    GLint i, samplers = 0;
    GLint shaderType;
    GLProgram program = (GLProgram)programObject->privateData;
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);

    gcSHADER_GetType(Shader, (gcSHADER_KIND *)&shaderType);

    for (i = 0; i < Count; ++i)
    {
        gcUNIFORM uniform;
        gctCONST_STRING name;
        GLUniform slot;
        gcSHADER_TYPE type;
        gctSIZE_T length;
        gctSIZE_T bytes;
        gceSTATUS       status;

        gcmONERROR(gcSHADER_GetUniform(Shader, i, &uniform));

        gcmONERROR(gcUNIFORM_GetName(uniform, gcvNULL, &name));

        if (name[0] == '#')
        {
            gcmASSERT(*PrivateIndex < program->builtInUniformCount);
            slot = &program->builtInUniforms[*PrivateIndex];

            if (Duplicates)
            {
                GLint j;

                for (j = 0; j < *PrivateIndex; ++j)
                {
                    gctCONST_STRING name2;

                    gcmONERROR(gcUNIFORM_GetName(
                        program->builtInUniforms[j].uniform[0],
                        gcvNULL,
                        &name2));

                    if (gcmIS_SUCCESS(gcoOS_StrCmp(name, name2)))
                    {
                        slot = &program->builtInUniforms[j];
                        break;
                    }
                }
            }

            if (slot->uniform[0] == gcvNULL)
            {
                slot->uniform[0] = uniform;

                ++ (*PrivateIndex);
            }
            else
            {
                slot->uniform[1] = uniform;

                -- (program->builtInUniformCount);
            }

            continue;
        }

        gcmASSERT(*Index < program->uniformCount);
        slot = &program->uniforms[*Index];

        /* Get the uniform type. */
        gcmONERROR(gcUNIFORM_GetType(uniform, &type, &length));

        if (Duplicates)
        {
            GLint j;

            for (j = 0; j < *Index; ++j)
            {
                gctCONST_STRING name2;

                gcmONERROR(gcUNIFORM_GetName(program->uniforms[j].uniform[0],
                                               gcvNULL,
                                               &name2));

                if (gcmIS_SUCCESS(gcoOS_StrCmp(name, name2)))
                {
                    gcSHADER_TYPE type2;
                    gctSIZE_T length2;

                    gcmONERROR(
                        gcUNIFORM_GetType(program->uniforms[j].uniform[0],
                                          &type2,
                                          &length2));

                    gcmASSERT((type == type2) && (length == length2));

                    slot = &program->uniforms[j];
                    break;
                }
            }
        }

        switch (type)
        {
        case gcSHADER_SAMPLER_1D:
        case gcSHADER_SAMPLER_2D:
        case gcSHADER_SAMPLER_3D:
        case gcSHADER_SAMPLER_CUBIC:
        case gcSHADER_SAMPLER_EXTERNAL_OES:
            samplers += length;

            bytes = gcmSIZEOF(GLfloat) * length;

            while (length-- > 0)
            {
                gcmASSERT(*SamplerIndex < (GLint) gcmCOUNTOF(program->sampleMap));
                program->sampleMap[*SamplerIndex].shaderType = shaderType;
                program->sampleMap[*SamplerIndex].type = type;
                program->sampleMap[*SamplerIndex].unit = *SamplerIndex;
                program->samplerDirty |=( 1 << (*SamplerIndex) );
                chipCtx->samplerDirty |=( 1 << (*SamplerIndex) );
                programObject->bindingInfo.sampler2TexUnit[*SamplerIndex] = *SamplerIndex;
                ++ (*SamplerIndex);
            }

            break;

        case gcSHADER_FLOAT_X1:
        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_INTEGER_X1:
            bytes = 1 * gcmSIZEOF(GLfloat) * length;
            break;

        case gcSHADER_FLOAT_X2:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_INTEGER_X2:
            bytes = 2 * gcmSIZEOF(GLfloat) * length;
            break;

        case gcSHADER_FLOAT_X3:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_INTEGER_X3:
            bytes = 3 * gcmSIZEOF(GLfloat) * length;
            break;

        case gcSHADER_FLOAT_X4:
        case gcSHADER_BOOLEAN_X4:
        case gcSHADER_INTEGER_X4:
            bytes = 4 * gcmSIZEOF(GLfloat) * length;
            break;

        case gcSHADER_FLOAT_2X2:
            bytes = 2 * 2 * gcmSIZEOF(GLfloat) * length;
            break;

        case gcSHADER_FLOAT_3X3:
            bytes = 3 * 3 * gcmSIZEOF(GLfloat) * length;
            break;

        case gcSHADER_FLOAT_4X4:
            bytes = 4 * 4 * gcmSIZEOF(GLfloat) * length;
            break;

        default:
            gcmFATAL("Unknown shader type %d!", type);
            bytes = 0;
        }

        if (slot->uniform[0] == gcvNULL)
        {
            /* Assign primary uniform. */
            slot->uniform[0] = uniform;

            /* Allocate the data array. */
            gcmVERIFY_OK(gcoOS_Allocate(gcvNULL,
                                        bytes,
                                        (gctPOINTER*) &slot->data));

            gcoOS_ZeroMemory(slot->data, bytes);

            slot->dirty = GL_TRUE;

            ++ (*Index);
        }
        else
        {
            /* Assign secondary uniform. */
            slot->uniform[1] = uniform;

            -- (program->uniformCount);
        }
    }
OnError:
    return samplers;
}

static GLboolean glshLinkProgramAttributes(
    IN __GLcontext * gc,
    IN OUT __GLshaderProgramObject* programObject
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gcSHADER vertexShader, fragmentShader;
    GLuint i, j, location, index, map;
    gctSIZE_T length;
    gctSIZE_T vertexUniforms, fragmentUniforms;
    GLBinding binding;
    gctPOINTER pointer = gcvNULL;
    gceSTATUS       status;
    GLProgram program = (GLProgram)programObject->privateData;

    vertexShader = program->vertexShader;
    fragmentShader = program->fragmentShader;

    /* Get the number of vertex attributes. */
    gcmVERIFY_OK(gcSHADER_GetAttributeCount(vertexShader,
                                            &program->attributeCount));

    if (program->attributeCount > 0)
    {
        /* Allocate the pointer array for the vertex attributes. */
        if (gcmIS_ERROR(
                gcoOS_Allocate(chipCtx->os,
                               program->attributeCount * sizeof(gcATTRIBUTE),
                               (gctPOINTER *) &program->attributePointers)))
        {
            /* Error. */
            gcmFATAL("LinkShaders: gcoOS_Allocate failed");
            return GL_FALSE;
        }

        /* Zero out the maximum length of any attribute name. */
        program->attributeMaxLength = 0;

        /* Query all attributes. */
        for (i = index = location = map = 0; i < program->attributeCount; ++i)
        {
            gcATTRIBUTE attribute;
            gctCONST_STRING namex;
            /* Get the gcATTRIBUTE object pointer. */
            gcmVERIFY_OK(gcSHADER_GetAttribute(vertexShader, i, &attribute));

            if (attribute != gcvNULL)
            {
                gctBOOL enable = gcvFALSE;

                /* Save the attribute pointer. */
                program->attributePointers[index++] = attribute;

                /* Get the attribute name. */
                gcmVERIFY_OK(gcATTRIBUTE_GetName(vertexShader, attribute, gcvFALSE, &length, &namex));

                if (length > program->attributeMaxLength)
                {
                    /* Update maximum length of attribute name. */
                    program->attributeMaxLength = length;
                }

                /* Get the attribute array length. */
                gcmVERIFY_OK(gcATTRIBUTE_GetType(vertexShader, attribute, gcvNULL, &length));

                for (j = 0; j < length; ++j, ++location)
                {
                    if (location >= chipCtx->maxAttributes)
                    {
                        gcmFATAL("LinkShaders: Too many attributes");
                        return GL_FALSE;
                    }

                    /* Set attribute location. */
                    program->attributeLocation[location].attribute = attribute;
                    program->attributeLocation[location].index     = j;
                }

                /* Check whether attribute is enabled or not. */
                if (gcmIS_SUCCESS(gcATTRIBUTE_IsEnabled(attribute, &enable))
                &&  enable
                )
                {
                    program->attributeEnable |= 1 << i;
                    program->attributeMap[i]  = map++;
                    tagBuiltInAttribute(gc, i, namex);
                }

            }
        }

        /* Set actual number of attributes. */
        program->attributeCount = index;

        /* Zero out linkage and unused locaotions. */
        for (i = 0; i < chipCtx->maxAttributes; ++i)
        {
            program->attributeLinkage[i]           = -1;
            program->attributeLocation[i].assigned = GL_FALSE;

            if (i >= (GLint) location)
            {
                program->attributeLocation[i].attribute = gcvNULL;
                program->attributeLocation[i].index     = -1;
            }
        }

        /* Walk all bindings and assign the attributes. */
        for (binding = program->attributeBinding;
             binding != gcvNULL;
             binding = binding->next)
        {
            gctSIZE_T bindingLength;

            /* Get the length of the binding attribute. */
            gcoOS_StrLen(binding->name, &bindingLength);

            /* Walk all attribute locations. */
            for (i = 0; i < chipCtx->maxAttributes; ++i)
            {
                gcATTRIBUTE attribute;
                gctCONST_STRING name;

                /* Ignore if location is unavailable or not the start of an
                   array. */
                if (program->attributeLocation[i].index != 0)
                {
                    continue;
                }

                attribute = program->attributeLocation[i].attribute;

                /* Compare the name of the binding with the attribute. */
                gcmVERIFY_OK(gcATTRIBUTE_GetName(vertexShader, attribute, gcvFALSE, &length, &name));

                if ((bindingLength == length)
                &&  gcmIS_SUCCESS(gcoOS_MemCmp(binding->name, name, length))
                )
                {
                    /* Get the length of the attribute array. */
                    gcmVERIFY_OK(gcATTRIBUTE_GetType(vertexShader,
                                                     attribute,
                                                     gcvNULL,
                                                     &length));

                    /* Test for overflow. */
                    if (binding->index + length > chipCtx->maxAttributes)
                    {
                        gcmFATAL("Binding for %s overflows.", binding->name);
                        return GL_FALSE;
                    }

                    /* Copy the binding. */
                    for (j = 0; j < length; ++j)
                    {
                        /* Make sure the binding is not yet taken. */
                        if (program->attributeLinkage[binding->index + j] != -1)
                        {
                            gcmFATAL("Binding for %s occupied.", binding->name);
                            return GL_FALSE;
                        }

                        /* Assign the binding. */
                        program->attributeLinkage[binding->index + j] = i + j;
                        program->attributeLocation[i + j].assigned    = GL_TRUE;
                    }

                    break;
                }
            }
        }

        /* Walk all unassigned attributes. */
        for (i = 0; i < chipCtx->maxAttributes; ++i)
        {
            if (program->attributeLocation[i].assigned
            ||  (program->attributeLocation[i].attribute == gcvNULL)
            )
            {
                continue;
            }

            gcmVERIFY_OK(
                gcATTRIBUTE_GetType(vertexShader,
                                    program->attributeLocation[i].attribute,
                                    gcvNULL,
                                    &length));

            for (index = 0; index < chipCtx->maxAttributes; ++index)
            {
                for (j = 0; j < length; ++j)
                {
                    if (program->attributeLinkage[index + j] != -1)
                    {
                        break;
                    }
                }

                if (j == length)
                {
                    break;
                }
            }

            if (index == chipCtx->maxAttributes)
            {
                gcmFATAL("No room for attribute %d (x%d)", i, length);
                return GL_FALSE;
            }

            for (j = 0; j < length; ++j)
            {
                program->attributeLinkage[index + j]       = i + j;
                program->attributeLocation[i + j].assigned = GL_TRUE;
            }
        }
    }

    /***************************************************************************
    ** Uniform management.
    */

    /* Get the number of uniform attributes. */
    gcmONERROR(gcSHADER_GetUniformCount(vertexShader,   &vertexUniforms));
    gcmONERROR(gcSHADER_GetUniformCount(fragmentShader, &fragmentUniforms));

    /* Count vivsible and private uniforms. */
    program->vertexCount         = vertexUniforms;
    program->vertexSamplers      = 0;
    program->fragmentSamplers    = 0;
    program->uniformCount        = 0;
    program->builtInUniformCount = 0;
    program->uniformMaxLength    = 0;

    countUniforms(program, vertexShader,   vertexUniforms);
    countUniforms(program, fragmentShader, fragmentUniforms);

    if (program->uniformCount > 0)
    {
        /* Allocate the array of _GLUniform structures. */
        gctSIZE_T bytes = program->uniformCount * gcmSIZEOF(struct _GLUniform);

        if (gcmIS_ERROR(gcoOS_Allocate(gcvNULL,
                                       bytes,
                                       &pointer)))
        {
            /* Error. */
            gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
            return GL_FALSE;
        }

        program->uniforms = pointer;

        gcoOS_ZeroMemory(program->uniforms, bytes);
    }

    if (program->builtInUniformCount > 0)
    {
        /* Allocate the array of _GLUniform structures. */
        gctSIZE_T bytes = program->builtInUniformCount
                        * gcmSIZEOF(struct _GLUniform);

        if (gcmIS_ERROR(
                gcoOS_Allocate(gcvNULL,
                               bytes,
                               &pointer)))
        {
            /* Error. */
            gcmFATAL("%s(%d): gcoOS_Allocate failed", __FUNCTION__, __LINE__);
            return GL_FALSE;
        }

        program->builtInUniforms = pointer;

        gcoOS_ZeroMemory(program->builtInUniforms, bytes);
    }

    /* Process any uniforms. */
    if (program->uniformCount + program->builtInUniformCount > 0)
    {
        gctINT uniformIndex = 0;
        gctINT privateIndex = 0;
        gctINT samplerIndex = 0;
        gctUINT vsSamplesCount = 0;
        gctINT vssamplerIndex = 8;
                /* Verify the arguments. */
        gcmVERIFY_OK(gcoHAL_QueryTextureCaps(chipCtx->hal,
                                         gcvNULL,
                                         gcvNULL,
                                         gcvNULL,
                                         gcvNULL,
                                         gcvNULL,
                                         &vsSamplesCount,
                                         gcvNULL));
        if (vsSamplesCount == 16)
        {
            vssamplerIndex = 16;
        }
        /* Process vertex uniforms. */
        program->vertexSamplers = processUniforms(gc,
                                                   programObject,
                                                   vertexShader,
                                                   vertexUniforms,
                                                   GL_FALSE,
                                                   &uniformIndex,
                                                   &privateIndex,
                                                   &vssamplerIndex);

        /* Process fragment uniforms. */
        program->fragmentSamplers = processUniforms(gc,
                                                     programObject,
                                                     fragmentShader,
                                                     fragmentUniforms,
                                                     GL_TRUE,
                                                     &uniformIndex,
                                                     &privateIndex,
                                                     &samplerIndex);
    }

#if VIVANTE_PROFILER
    gcoHAL_ProfileEnd(chipCtx->hal, "BACK-END LINKER");
#endif
    /* Success. */
    return GL_TRUE;
OnError:
    return GL_FALSE;
}

static GLboolean
flushGLSLUniforms(
    IN __GLcontext * gc
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    GLProgram program = chipCtx->currGLSLProgram;
    GLint i;

    if (program == gcvNULL)
    {
        return GL_FALSE;
    }

    if ( chipCtx->currGLSLProgram )
        processBuiltInUniform(gc);
    /* Flush builtin uniforms */
    for (i = 0; i < program->builtInUniformCount; i++)
    {
        setBuiltInUniform(gc);
    }

    for (i = 0; i < program->uniformCount; ++i)
    {
        if (program->uniforms[i].dirty || chipCtx->programDirty)
        {
            gcSHADER_TYPE type;
            gctSIZE_T length;
            GLint j;

            if (gcmIS_ERROR(gcUNIFORM_GetType(program->uniforms[i].uniform[0],
                                              &type,
                                              &length)))
            {
                return GL_FALSE;
            }

            switch (type)
            {
            case gcSHADER_FLOAT_X1:
            case gcSHADER_FLOAT_X2:
            case gcSHADER_FLOAT_X3:
            case gcSHADER_FLOAT_X4:
            case gcSHADER_FLOAT_2X2:
            case gcSHADER_FLOAT_3X3:
            case gcSHADER_FLOAT_4X4:
                for (j = 0; j < 2; ++j)
                {
                    if ((program->uniforms[i].uniform[j] != gcvNULL)
                    &&  gcmIS_ERROR(
                            gcUNIFORM_SetValueF_Ex(program->uniforms[i].uniform[j],
                                                length,
                                                program->hints,
                                                program->uniforms[i].data))
                    )
                    {
                        return GL_FALSE;
                    }
                }
                break;

            case gcSHADER_BOOLEAN_X1:
            case gcSHADER_BOOLEAN_X2:
            case gcSHADER_BOOLEAN_X3:
            case gcSHADER_BOOLEAN_X4:
            case gcSHADER_INTEGER_X1:
            case gcSHADER_INTEGER_X2:
            case gcSHADER_INTEGER_X3:
            case gcSHADER_INTEGER_X4:
                for (j = 0; j < 2; ++j)
                {
                    if ((program->uniforms[i].uniform[j] != gcvNULL)
                    &&  gcmIS_ERROR(
                            gcUNIFORM_SetValue_Ex(program->uniforms[i].uniform[j],
                                               length,
                                               program->hints,
                                               program->uniforms[i].data))
                    )
                    {
                        return GL_FALSE;
                    }
                }
                break;

            default:
                break;
            }

            program->uniforms[i].dirty = GL_FALSE;
        }
    }

    chipCtx->programDirty = GL_FALSE;

    return GL_TRUE;
}

GLboolean glshLoadCompiler(IN glsCHIPCONTEXT_PTR  chipCtx)
{
    if (chipCtx->dll == gcvNULL)
    {
        do
        {
            union gluVARIANT
            {
                gceSTATUS           (*compiler)(IN gcoHAL Hal,
                                    IN gctINT ShaderType,
                                    IN gctSIZE_T SourceSize,
                                    IN gctCONST_STRING Source,
                                    OUT gcSHADER * Binary,
                                    OUT gctSTRING * Log);

                gctPOINTER ptr;
            } compiler;


            union gluINITIALIZER
            {
                gceSTATUS (*funcPtr)(IN  gcoHAL Hal, IN gcsGLSLCaps *Caps);
                gctPOINTER ptr;
            } intializer;

            gceSTATUS status;

            status = gcoOS_LoadLibrary(gcvNULL,
                                       "libGLSLC",
                                       &chipCtx->dll);
            if (gcmIS_ERROR(status))
            {
                break;
            }

            gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL,
                                              chipCtx->dll,
                                              "gcCompileShader",
                                              &compiler.ptr));

            gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL,
                                              chipCtx->dll,
                                              "gcInitializeCompiler",
                                              &intializer.ptr));

            chipCtx->compiler = compiler.compiler;

            /* Save the FE compiler ptr. */
            gcSetGLSLCompiler(chipCtx->compiler);

            gcmVERIFY_OK((*intializer.funcPtr)(chipCtx->hal, gcvNULL));
        }
        while (gcvFALSE);
    }

    return (chipCtx->compiler != gcvNULL);
}

void glshReleaseCompiler(IN glsCHIPCONTEXT_PTR  chipCtx)
{
    if (chipCtx->dll != gcvNULL)
    {
        union gluFINALIZER
        {
            gceSTATUS (*funcPtr)(IN  gcoHAL Hal);
            gctPOINTER ptr;
        } finalizer;

        gcmVERIFY_OK(gcoOS_GetProcAddress(gcvNULL, chipCtx->dll, "gcFinalizeCompiler", &finalizer.ptr));

        gcmVERIFY_OK((*finalizer.funcPtr)(chipCtx->hal));

        gcmVERIFY_OK(gcoOS_FreeLibrary(gcvNULL, chipCtx->dll));

        chipCtx->dll = gcvNULL;
    }
}


gceSTATUS glshLoadShader(
    IN __GLcontext * gc
    )
{
    glsCHIPCONTEXT_PTR chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    GLProgram program = chipCtx->currGLSLProgram;

    if ((gc->vertexStreams.primMode == GL_POINTS)
    && (program->hints->vsHasPointSize == 0))
    {
        /* If primitive type is GL_POINTS and gl_PointSize is not set,
           then return error. */
        gcmONERROR(gcvSTATUS_INVALID_DATA);
    }

    gcmONERROR(
        gco3D_SetAllEarlyDepthModes(chipCtx->hw,
                                    program->hints->psHasFragDepthOut));

    /* Check if program is dirty. */
    if (chipCtx->programDirty)
    {
        /* Load program. */
        gcmONERROR(gcLoadShaders(chipCtx->hal,
                                 program->statesSize,
                                 program->states,
                                 program->hints));
    }

    /* Check if primitive type has changed. */
    else if (chipCtx->lastPrimitiveType != gc->vertexStreams.primMode)
    {
        /* Reprogram hints based on primitive type. */
        gcmONERROR(gcLoadShaders(chipCtx->hal,
                                 0,
                                 gcvNULL,
                                 program->hints));
    }

    /* Check if primitive type has changed from or to a POINT_LIST. */
    if ((chipCtx->lastPrimitiveType != gc->vertexStreams.primMode)
    && (  (chipCtx->lastPrimitiveType ==  -1)
       || (chipCtx->lastPrimitiveType ==  GL_POINTS)
       || (gc->vertexStreams.primMode ==  GL_POINTS)
       )
    )
    {
        /* Enable point size for points. */
        gcmONERROR(
            gco3D_SetPointSizeEnable(chipCtx->hw,
                gc->vertexStreams.primMode == GL_POINTS));

        /* Enable point sprites for points. */
        gcmONERROR(
            gco3D_SetPointSprite(chipCtx->hw,
                gc->vertexStreams.primMode == GL_POINTS));
    }

    flushGLSLUniforms(gc);
    chipCtx->programDirty = GL_FALSE;

    /* Save current primitive type. */
    chipCtx->lastPrimitiveType = gc->vertexStreams.primMode;

    return gcvSTATUS_OK;

OnError:
   return status;
}

/************************************************************************/
/* Implementation for device GLSL API                                  */
/************************************************************************/
GLboolean __glChipCompileShader(__GLcontext * gc, __GLshaderObject* shaderObject)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    GLObjectType shaderType = GLObject_Unknown;

    if (shaderObject->shaderInfo.sourceSize == 0)
    {
        gcoOS_StrDup(gcvNULL,
                     "No source attached.",
                     (gctSTRING *)&shaderObject->shaderInfo.compiledLog);
        return GL_FALSE;
    }

    if (chipCtx->compiler == gcvNULL)
    {
        if (!glshLoadCompiler(chipCtx))
        {
            return GL_FALSE;
        }
    }

    switch (shaderObject->shaderInfo.shaderType) {
        case GL_VERTEX_SHADER:
            shaderType = GLObject_VertexShader;
            break;
        case GL_FRAGMENT_SHADER:
            shaderType = GLObject_FragmentShader;
            break;
        case GL_GEOMETRY_SHADER_EXT:
            shaderType = GLObject_Unknown;
            break;
    }

    if (shaderObject->shaderInfo.hShader) {
        gcmVERIFY_OK(gcSHADER_Destroy(shaderObject->shaderInfo.hShader));
        shaderObject->shaderInfo.hShader = NULL;
    }

    status = (*chipCtx->compiler)(chipCtx->hal,
                                  shaderType,
                                  shaderObject->shaderInfo.sourceSize,
                                  (gctCONST_STRING)shaderObject->shaderInfo.source,
                                  (gcSHADER *)&shaderObject->shaderInfo.hShader,
                                  (gctSTRING *)&shaderObject->shaderInfo.compiledLog);

    if (gcmIS_SUCCESS(status))
    {
        gcSHADER shader = (gcSHADER)shaderObject->shaderInfo.hShader;

        if (!GetShaderNeedPatchForCentroid(shader) &&
            !GetShaderHasIntrinsicBuiltin(shader) &&
            !gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(GetShaderOutputBlends(shader))
            )
        {
            /* Full optimization */
            gcSHADER_SetOptimizationOption(shader, gcvOPTIMIZATION_FULL & ~gcvOPTIMIZATION_LOADTIME_CONSTANT);

            status = gcOptimizeShader(shader, gcvNULL);
        }
    }

    return (status == gcvSTATUS_OK) ? GL_TRUE : GL_FALSE;
}

GLvoid __glChipDeleteShader(__GLcontext * gc, __GLshaderObject* shaderObject)
{
    if (shaderObject->shaderInfo.hShader) {
        gcmVERIFY_OK(gcSHADER_Destroy(shaderObject->shaderInfo.hShader));
        shaderObject->shaderInfo.hShader = NULL;
    }

    if (shaderObject->shaderInfo.compiledLog) {
        gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctSTRING)shaderObject->shaderInfo.compiledLog));
        shaderObject->shaderInfo.compiledLog = NULL;
    }
}

GLvoid __glChipDeleteShaderProgram(__GLcontext * gc, GLvoid ** programObject)
{
    GLProgram program = (GLProgram)*programObject;

    if (program) {
        if (program->fragmentShader) {
            gcmVERIFY_OK(gcSHADER_Destroy(program->fragmentShader));
            program->fragmentShader = NULL;
        }

        if (program->vertexShader) {
            gcmVERIFY_OK(gcSHADER_Destroy(program->vertexShader));
            program->vertexShader = NULL;
        }

        if (program->attributeLinkage) {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->attributeLinkage));
        }

        if (program->attributeLocation) {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->attributeLocation));
        }

        if (program->attributeMap) {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->attributeMap));
        }

        (*gc->imports.free)(gc, program);
        *programObject = NULL;
    }
}

static gceSTATUS
__glChipMapLinkError(
    __GLcontext *gc,
    __GLshaderProgramObject *programObject,
    gceSTATUS Status
    )
{
    gctSTRING logBuffer = programObject->programInfo.infoLog;
    gctUINT logOffset = 0;
    gceSTATUS status;

    if ( logBuffer )
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, programObject->programInfo.infoLog));
    }


    programObject->programInfo.infoLog = (char *)(*gc->imports.malloc)(gc, __GLSL_LOG_INFO_SIZE);
    logBuffer = programObject->programInfo.infoLog;

    switch (Status)
    {
    case gcvSTATUS_TOO_MANY_ATTRIBUTES:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Too many attributes.\n"));
        break;

    case gcvSTATUS_TOO_MANY_UNIFORMS:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Too many uniforms.\n"));
        break;

    case gcvSTATUS_TOO_MANY_VARYINGS:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Too many varyings.\n"));
        break;

    case gcvSTATUS_UNDECLARED_VARYING:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Undeclared varying.\n"));
        break;

    case gcvSTATUS_VARYING_TYPE_MISMATCH:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Input&Output mismatch.\n"));
        break;

    case gcvSTATUS_MISSING_MAIN:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Missing main function.\n"));
        break;

    case gcvSTATUS_UNIFORM_MISMATCH:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Uniform mismatch between Vertex and Fragment.\n"));
        break;

    case gcvSTATUS_TOO_MANY_SHADERS:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Too many shaders within one program.\n"));
        break;

    case gcvSTATUS_SHADER_VERSION_MISMATCH:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Shader version mismatch between Vertex and Fragment.\n"));
        break;

    case gcvSTATUS_TOO_MANY_INSTRUCTION:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Too many instructions.\n"));
        break;

    case gcvSTATUS_SSBO_MISMATCH:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: SSBO mismatch between Vertex and Fragment.\n"));
        break;

    case gcvSTATUS_TOO_MANY_OUTPUT:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Too many outputs.\n"));
        break;

    case gcvSTATUS_TOO_MANY_INPUT:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Too many inputs.\n"));
        break;

    case gcvSTATUS_NOT_SUPPORT_CL:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Can't support CL.\n"));
        break;

    default:
        break;
    }

#if (defined(_DEBUG) || defined(DEBUG))
    gcoOS_Print("%s", logBuffer);
#endif

    status = Status;

OnError:
    return status;
}

GLboolean __glChipLinkProgram(__GLcontext * gc, __GLshaderProgramObject* programObject)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    __GLshaderObject * vertexShaderObject = NULL;
    __GLshaderObject * fragmentShaderObject = NULL;
    gcSHADER vertexShader, fragmentShader;
    gctPOINTER pointer = gcvNULL;
    GLint i;
    gceSTATUS status;
    GLuint enable = 0;
    GLint link;
    GLProgram program = (GLProgram)programObject->privateData;

    for(i = 0; i < programObject->programInfo.attachedShadersTableSize; i++)
    {
        /* We need to consider multiple shaders case later */
        /* right now low level driver does not support this kind of linkage */
        if(programObject->programInfo.attachedShaders[i])
        {
            switch(programObject->programInfo.attachedShaders[i]->shaderInfo.shaderType){
            case GL_VERTEX_SHADER:
                vertexShaderObject = programObject->programInfo.attachedShaders[i];
                break;
            case GL_FRAGMENT_SHADER:
                fragmentShaderObject = programObject->programInfo.attachedShaders[i];
                break;
            case GL_GEOMETRY_SHADER_EXT:
                continue;
            }
        }
    }

    if ((vertexShaderObject->shaderInfo.hShader == 0) || (fragmentShaderObject->shaderInfo.hShader == 0)) {
        return GL_FALSE;
    }

    if (program == NULL) {
        program = (GLProgram)(*gc->imports.malloc)(gc, sizeof(struct _GLProgram));
        if ( program == gcvNULL )
                return GL_FALSE;
        programObject->privateData = program;
        /* Initialize GLProgram structure. */
        program->vertexShader   = gcvNULL;
        program->fragmentShader = gcvNULL;
        program->statesSize           = 0;
        program->states               = gcvNULL;
        program->hints                = gcvNULL;

        program->attributeCount       = 0;
        program->attributeMaxLength   = 0;
        program->attributePointers    = gcvNULL;

        program->attributeBinding     = gcvNULL;
        program->attributeLinkage     = gcvNULL;
        program->attributeLocation    = gcvNULL;
        program->attributeMap         = gcvNULL;
        program->attributeEnable = 0;
        program->vertexCount = 0;

        program->uniformCount         = 0;
        program->uniformMaxLength     = 0;
        program->uniforms             = gcvNULL;

        program->builtInUniformCount  = 0;
        program->builtInUniforms      = gcvNULL;
        program->vertexSamplers       = 0;
        program->fragmentSamplers     = 0;
        program->codeSeq = 0;
        program->samplerDirty = 0;

        gcoOS_ZeroMemory(program->sampleMap,
                                      gcmSIZEOF(program->sampleMap));

        program->valid = GL_FALSE;
        if (gcmIS_ERROR(
                gcoOS_Allocate(gcvNULL,
                               chipCtx->maxAttributes * gcmSIZEOF(GLuint),
                               &pointer)))
        {
            (*gc->imports.free)(gc, program);
            return GL_FALSE;
        }

        program->attributeLinkage = pointer;

        if (gcmIS_ERROR(
                gcoOS_Allocate(gcvNULL,
                               chipCtx->maxAttributes * gcmSIZEOF(GLLocation),
                               &pointer)))
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)program->attributeLinkage));
            (*gc->imports.free)(gc, program);
            return GL_FALSE;
        }

        program->attributeLocation = pointer;

        if (gcmIS_ERROR(
                gcoOS_Allocate(gcvNULL,
                               chipCtx->maxAttributes * gcmSIZEOF(GLuint),
                               &pointer)))
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)program->attributeLinkage));
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)program->attributeLocation));
            (*gc->imports.free)(gc, program);
            return GL_FALSE;
        }

        program->attributeMap = pointer;

        for (i = 0; i < chipCtx->maxAttributes; ++i)
        {
            program->attributeLinkage[i]            = (GLuint) 0xFFFFFFFF;
            program->attributeMap[i]                = (GLuint) 0xFFFFFFFF;
            program->attributeLocation[i].attribute = gcvNULL;
        }
    }

    /* Test if any of the shaders is dirty. */
    if (programObject->programInfo.codeSeq != program->codeSeq)
    {
        if (program->vertexShader == gcvNULL)
        {
            /* Construct a new vertex shader. */
            gcmONERROR(gcSHADER_Construct(chipCtx->hal,
                                          gcSHADER_TYPE_VERTEX,
                                          &program->vertexShader));
        }

        /* Copy the vertex shader binary from the shader to the program. */
        gcmONERROR(gcSHADER_Copy(program->vertexShader,
            (gcSHADER)vertexShaderObject->shaderInfo.hShader));

        if (program->fragmentShader == gcvNULL)
        {
            /* Construct a new fragment shader. */
            gcmONERROR(gcSHADER_Construct(chipCtx->hal,
                                          gcSHADER_TYPE_FRAGMENT,
                                          &program->fragmentShader));
        }

        /* Copy the fragment shader binary from the shader to the program. */
        gcmONERROR(gcSHADER_Copy(program->fragmentShader,
            (gcSHADER)fragmentShaderObject->shaderInfo.hShader));
        program->codeSeq = programObject->programInfo.codeSeq;


    }

    /* Get pointer to shader binaries. */
    vertexShader   = program->vertexShader;
    fragmentShader = program->fragmentShader;

    /* Call the HAL backend linker. */
    status = gcLinkShaders(vertexShader,
                           fragmentShader,
                           (gceSHADER_FLAGS)
                           ( gcvSHADER_DEAD_CODE
                           | gcvSHADER_RESOURCE_USAGE
                           | gcvSHADER_OPTIMIZER
                           | gcvSHADER_USE_GL_Z
                           | gcvSHADER_USE_GL_POINT_COORD
                           | gcvSHADER_USE_GL_POSITION
                           ),
                           &program->statesSize,
                           &program->states,
                           &program->hints);

    if (gcmIS_ERROR(status))
    {
        /* Error. */
        __glChipMapLinkError(gc, programObject, status);
        return GL_FALSE;
    }

    if (glshLinkProgramAttributes(gc,
        programObject))
    {
        programObject->bindingInfo.maxActiveAttribNameLength = program->attributeMaxLength + 1;
        programObject->bindingInfo.numActiveAttrib = program->attributeCount;

        programObject->bindingInfo.maxActiveUniformLength = program->uniformMaxLength + 1;
        programObject->bindingInfo.numActiveUniform = program->uniformCount + program->builtInUniformCount;
        programObject->bindingInfo.numUserUnifrom = program->uniformCount;

        /* Walk all unassigned attributes. */
        programObject->bindingInfo.vsInputMask = 0;
        vsInputMaskForBuiltIns( gc, &(programObject->bindingInfo.vsInputMask));
        for (i = 0; i < chipCtx->maxAttributes; ++i)
        {
            /* currently the compiler does not support build in input, so we only can */
            /* use generic attributes, immediate mode and conventional inputs won't work */
            link = program->attributeLinkage[i];
            if ((link != -1) && (link < chipCtx->maxAttributes)) {
                if (gcmIS_SUCCESS(gcATTRIBUTE_IsEnabled(program->attributeLocation[link].attribute,
                     (gctBOOL *)&enable))
                    && enable)
                {
                    programObject->bindingInfo.vsInputMask |= 1 << (i + __GL_VARRAY_ATT0_INDEX);
                }
            }
        }
        return gcvTRUE;
    } else {
        return gcvFALSE;
    }

OnError:
    gcmFATAL("%s(%d): gcSHADER out of memory", __FUNCTION__, __LINE__);
    return gcvFALSE;
}

GLboolean __glChipUseProgram(__GLcontext * gc, __GLshaderProgramObject* programObject, GLboolean * valid)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);

    if (programObject == NULL) {
        chipCtx->currGLSLProgram = NULL;
        chipCtx->programDirty = GL_TRUE;
    } else {
        chipCtx->currGLSLProgram = (GLProgram)programObject->privateData;
        chipCtx->programDirty = GL_TRUE;
        chipCtx->samplerDirty = chipCtx->currGLSLProgram->samplerDirty;
    }
    return GL_TRUE;
}

GLboolean __glChipGetUniformLocation(__GLcontext *gc, __GLshaderProgramObject *programObject,
                                  const GLchar *name, GLuint nameLen, GLuint arrayIdx,
                                  GLboolean bArray, GLint *location)
{
    GLProgram program = (GLProgram)programObject->privateData;
    gctSIZE_T uniformLength;
    gctCONST_STRING uniformName;
    gctSIZE_T arraySize;
    GLuint i = 0;

    /* Walk all uniform. */
    for (i = 0; i < program->uniformCount; i++)
    {
        /* Get the uniform name. */
        gcmVERIFY_OK(gcUNIFORM_GetName(program->uniforms[i].uniform[0],
                                       &uniformLength,
                                       &uniformName));
        gcmVERIFY_OK(gcUNIFORM_GetType(program->uniforms[i].uniform[0], gcvNULL, (gctSIZE_T *)&arraySize));

        /* See if the uniform matches the requested name. */
        if ((nameLen == uniformLength)
        &&  gcmIS_SUCCESS(gcoOS_MemCmp(name, uniformName, nameLen))
        )
        {
            if((gctSIZE_T)arrayIdx < arraySize)
            {
                /* Match, return position. */
                *location =  i + (arrayIdx << 16);
                return GL_TRUE;
            } else {
                *location = -1;
                return GL_FALSE;
            }
        }
    }

    *location = -1;
    return GL_FALSE;
}

GLvoid __glChipGetActiveUniform(__GLcontext *gc, __GLshaderProgramObject *programObject,
    GLuint index,
    GLsizei bufsize,
    GLsizei * length,
    GLint * size,
    GLenum * type,
    char * name)
{
    GLProgram program = (GLProgram)programObject->privateData;
    gcUNIFORM uniform;
    gctSIZE_T uniformNameLength;
    gctCONST_STRING uniformName;
    gcSHADER_TYPE uniformType = gcSHADER_FLOAT_X1;
    gceSTATUS status;


    /* Get gcUNIFORM object pointer. */
    uniform = program->uniforms[index].uniform[0];

    /* Get name of uniform. */
    gcmONERROR(gcUNIFORM_GetName(uniform,
                                   &uniformNameLength,
                                   &uniformName));

    /* Make sure it fits within the buffer. */
    if (uniformNameLength > (gctSIZE_T) bufsize - 1)
    {
        uniformNameLength = gcmMAX(bufsize - 1, 0);
    }

    if (name != gcvNULL)
    {
        if (uniformNameLength > 0)
        {
            /* Copy the name to the buffer. */
            gcoOS_MemCopy(name, uniformName, uniformNameLength);
        }

        name[uniformNameLength] = '\0';
    }

    if (length != gcvNULL)
    {
        /* Return number of characters copied. */
        *length = uniformNameLength + 1;
    }

    if (size != gcvNULL)
    {
        /* Get the type of the uniform. */
        gcmONERROR((gcUNIFORM_GetType(uniform, &uniformType, (gctSIZE_T *) size)));

    }

    /* Convert type to GL enumeration. */
    if (type != gcvNULL)
    {
        *type = gc2glType[uniformType];
    }
OnError:
    return;
}

GLboolean __glChipBindAttributeLocation(__GLcontext *gc, __GLshaderProgramObject *programObject, GLuint index, const GLchar * name)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    GLProgram program = (GLProgram)programObject->privateData;
    GLBinding binding = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    GLint i;

    if (index >= (GLuint) chipCtx->maxAttributes)
    {
        return GL_FALSE;
    }

    if (program == NULL) {
        program = (GLProgram)(*gc->imports.malloc)(gc, sizeof(struct _GLProgram));
        if ( program == gcvNULL )
                return GL_FALSE;
        programObject->privateData = program;
        /* Initialize GLProgram structure. */
        program->vertexShader   = gcvNULL;
        program->fragmentShader = gcvNULL;
        program->statesSize           = 0;
        program->states               = gcvNULL;
        program->hints                = gcvNULL;

        program->attributeCount       = 0;
        program->attributeMaxLength   = 0;
        program->attributePointers    = gcvNULL;

        program->attributeBinding     = gcvNULL;
        program->attributeLinkage     = gcvNULL;
        program->attributeLocation    = gcvNULL;
        program->attributeMap         = gcvNULL;
        program->attributeEnable = 0;
        program->vertexCount = 0;

        program->uniformCount         = 0;
        program->uniformMaxLength     = 0;
        program->uniforms             = gcvNULL;

        program->builtInUniformCount  = 0;
        program->builtInUniforms      = gcvNULL;
        program->vertexSamplers       = 0;
        program->fragmentSamplers     = 0;
        program->codeSeq = 0;
        program->samplerDirty = 0;

        gcoOS_ZeroMemory(program->sampleMap,
                                      gcmSIZEOF(program->sampleMap));

        program->valid = GL_FALSE;
        if (gcmIS_ERROR(
                gcoOS_Allocate(gcvNULL,
                               chipCtx->maxAttributes * gcmSIZEOF(GLuint),
                               &pointer)))
        {
            (*gc->imports.free)(gc, program);
            return GL_FALSE;
        }

        program->attributeLinkage = pointer;

        if (gcmIS_ERROR(
                gcoOS_Allocate(gcvNULL,
                               chipCtx->maxAttributes * gcmSIZEOF(GLLocation),
                               &pointer)))
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)program->attributeLinkage));
            (*gc->imports.free)(gc, program);
            return GL_FALSE;
        }

        program->attributeLocation = pointer;

        if (gcmIS_ERROR(
                gcoOS_Allocate(gcvNULL,
                               chipCtx->maxAttributes * gcmSIZEOF(GLuint),
                               &pointer)))
        {
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)program->attributeLinkage));
            gcmVERIFY_OK(gcoOS_Free(gcvNULL, (gctPOINTER)program->attributeLocation));
            (*gc->imports.free)(gc, program);
            return GL_FALSE;
        }

        program->attributeMap = pointer;

        for (i = 0; i < chipCtx->maxAttributes; ++i)
        {
            program->attributeLinkage[i]            = (GLuint) 0xFFFFFFFF;
            program->attributeMap[i]                = (GLuint) 0xFFFFFFFF;
            program->attributeLocation[i].attribute = gcvNULL;
        }
    }

    for (binding = program->attributeBinding;
         binding != gcvNULL;
         binding = binding->next)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp((gctCONST_STRING)binding->name, (gctCONST_STRING)name)))
        {
            binding->index = index;
            return GL_TRUE;;
        }
    }

    do
    {
        gceSTATUS status;
        gctPOINTER pointer = gcvNULL;

        gcmERR_BREAK(gcoOS_Allocate(gcvNULL,
                                    gcmSIZEOF(struct _GLBinding),
                                    &pointer));

        binding = pointer;

        gcmERR_BREAK(gcoOS_StrDup(gcvNULL, (gctCONST_STRING)name, (gctSTRING *)&binding->name));

        binding->index                  = index;
        binding->next                   = program->attributeBinding;
        program->attributeBinding = binding;

        /* Disable batch optmization. */
        chipCtx->batchDirty = GL_TRUE;

        return GL_TRUE;
    }
    while (gcvFALSE);

    if (binding != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, binding));
    }
    return GL_FALSE;
}

GLint __glChipGetAttributeLocation(__GLcontext *gc, __GLshaderProgramObject *programObject, const GLchar * name)
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    GLProgram program = (GLProgram)programObject->privateData;
    gctSIZE_T length = 0;
    GLuint i;
    gctSIZE_T attributeLength;
    gctCONST_STRING attributeName;

    gcoOS_StrLen((gctCONST_STRING)name, &length);

    /* Walk all attributes. */
    for (i = 0; i < (GLint)chipCtx->maxAttributes; ++i)
    {
        GLint linkage = program->attributeLinkage[i];

        if (linkage == -1)
        {
            continue;
        }

        /* Get the attribute name. */
        gcmVERIFY_OK(
            gcATTRIBUTE_GetName(
                program->vertexShader,
                program->attributeLocation[linkage].attribute,
                gcvFALSE,
                &attributeLength,
                &attributeName));

        /* See if the attribute matches the requested name. */
        if ((length == attributeLength)
        &&  gcmIS_SUCCESS(gcoOS_MemCmp(name, attributeName, length))
        )
        {
            return i;
        }
    }

    return -1;
}

GLint __glChipUniforms(
    __GLcontext *gc,
    __GLshaderProgramObject *programObject,
    GLint Location,
    GLint Type,
    GLsizei Count,
    const void * Values,
    GLboolean Transpose
    )
{
    glsCHIPCONTEXT_PTR  chipCtx = CHIP_CTXINFO(gc);
    GLint location, index;
    GLProgram program;
    GLUniform uniform;
    gcSHADER_TYPE type;
    gcSHADER_TYPE inputType;
    gctSIZE_T length;
    gctUINT32 sampler;
    GLenum error = GL_NO_ERROR;

    if (Values == gcvNULL)
    {
        return error;
    }

    program = (GLProgram)programObject->privateData;

    if (program == gcvNULL)
    {
        return GL_INVALID_OPERATION;
    }

    if (!glfConvertGLEnum(
            gc2glType,
            gcmCOUNTOF(gc2glType),
            &Type, glvINT,
            (GLuint *)&inputType
            ))
    {
        return GL_INVALID_OPERATION;
    }

    location = Location & 0xFFFF;
    index    = Location >> 16;

    /* Point to the GLUniform structure. */
    uniform = &program->uniforms[location];
    gcmASSERT(uniform != gcvNULL);

    if (gcmIS_ERROR(gcUNIFORM_GetType(uniform->uniform[0],
                                      &type,
                                      &length)))
    {
        return GL_INVALID_OPERATION;
    }

    if ( (length == 1) && (Count > 1) )
    {
        return GL_INVALID_OPERATION;
    }

    if ((gctSIZE_T) Count > length)
    {
        Count = length;
    }

    switch (type)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_4X4:
        if (type == inputType)
        {
            switch (type)
            {
            case gcSHADER_FLOAT_2X2:
                index *= 4;
                break;

            case gcSHADER_FLOAT_3X3:
                index *= 9;
                break;

            case gcSHADER_FLOAT_4X4:
                index *= 16;
                break;

            default:
                break;
            }

            gcoOS_MemCopy((GLfloat *) uniform->data + index,
                                       Values,
                                       Count * gcType2Bytes(inputType));
        }
        else
        {
            error = GL_INVALID_OPERATION;
        }
        break;

    case gcSHADER_BOOLEAN_X1:
        gcmASSERT(index == 0);
        switch (inputType)
        {
        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_INTEGER_X1:
            int2Bool(uniform->data, Values, Count);
            break;

        case gcSHADER_FLOAT_X1:
            float2Bool(uniform->data, Values, Count);
            break;

        default:
            error = GL_INVALID_OPERATION;
            break;
        }
        break;

    case gcSHADER_BOOLEAN_X2:
        gcmASSERT(index == 0);
        switch (inputType)
        {
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_INTEGER_X2:
            int2Bool(uniform->data, Values, 2 * Count);
            break;

        case gcSHADER_FLOAT_X2:
            float2Bool(uniform->data, Values, 2 * Count);
            break;

        default:
            error = GL_INVALID_OPERATION;
            break;
        }
        break;

    case gcSHADER_BOOLEAN_X3:
        gcmASSERT(index == 0);
        switch (inputType)
        {
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_INTEGER_X3:
            int2Bool(uniform->data, Values, 3 * Count);
            break;

        case gcSHADER_FLOAT_X3:
            float2Bool(uniform->data, Values, 3 * Count);
            break;

        default:
            error = GL_INVALID_OPERATION;
            break;
        }
        break;

    case gcSHADER_BOOLEAN_X4:
        gcmASSERT(index == 0);
        switch (inputType)
        {
        case gcSHADER_BOOLEAN_X4:
        case gcSHADER_INTEGER_X4:
            int2Bool(uniform->data, Values, 4 * Count);
            break;

        case gcSHADER_FLOAT_X4:
            float2Bool(uniform->data, Values, 4 * Count);
            break;

        default:
            error = GL_INVALID_OPERATION;
            break;
        }
        break;

    case gcSHADER_INTEGER_X1:
        gcmASSERT(index == 0);
        if (type == inputType)
        {
            _Int2Float(uniform->data, Values, 1 * Count);
        }
        else
        {
            error = GL_INVALID_OPERATION;
        }
        break;

    case gcSHADER_INTEGER_X2:
        gcmASSERT(index == 0);
        if (type == inputType)
        {
            _Int2Float(uniform->data, Values, 2 * Count);
        }
        else
        {
            error = GL_INVALID_OPERATION;
        }
        break;

    case gcSHADER_INTEGER_X3:
        gcmASSERT(index == 0);
        if (type == Type)
        {
            _Int2Float(uniform->data, Values, 3 * Count);
        }
        else
        {
            error = GL_INVALID_OPERATION;
        }
        break;

    case gcSHADER_INTEGER_X4:
        gcmASSERT(index == 0);
        if (type == inputType)
        {
            _Int2Float(uniform->data, Values, 4 * Count);
        }
        else
        {
            error = GL_INVALID_OPERATION;
        }
        break;

    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_EXTERNAL_OES:
        gcmASSERT(index == 0);

        /* Currently,we have problem of sampler array, will fix later */
        if(index > 0)
        {
            break;
        }

        if (inputType == gcSHADER_INTEGER_X1)
        {
            _Int2Float(uniform->data, Values, Count);

            /* Get sampler. */
            gcmVERIFY_OK(gcUNIFORM_GetSampler(uniform->uniform[0], &sampler));

            /* Map sampler. */
            program->sampleMap[sampler].unit = *(const GLint *) Values;
            programObject->bindingInfo.sampler2TexUnit[sampler] = *(const GLint *) Values;

            if ( program->sampleMap[sampler].shaderType == gcSHADER_TYPE_VERTEX )
            {
                __GL_SET_GLSL_SAMPLER_BIT(gc, 16);
            }

            if ( program->sampleMap[sampler].shaderType == gcSHADER_TYPE_FRAGMENT )
            {
                __GL_SET_GLSL_SAMPLER_BIT(gc, 0);
            }

            program->samplerDirty |=( 1 << sampler);
            chipCtx->samplerDirty |=( 1 << sampler);

            if ( uniform->uniform[1] )
            {
                gcmVERIFY_OK(gcUNIFORM_GetSampler(uniform->uniform[1], &sampler));
                program->sampleMap[sampler].unit = *(const GLint *) Values;
                programObject->bindingInfo.sampler2TexUnit[sampler] = *(const GLint *) Values;
                program->samplerDirty |=( 1 << sampler);
                chipCtx->samplerDirty |=( 1 << sampler);
            }

        }
        else
        {
            error = GL_INVALID_OPERATION;
        }
        break;

    default:
        error = GL_INVALID_OPERATION;
        break;
    }

    if (error == GL_NO_ERROR)
    {
        uniform->dirty = GL_TRUE;

        /* Disable batch optmization. */
        chipCtx->batchDirty = GL_TRUE;
    }

    return error;
}

GLboolean __glChipValidateProgram(__GLcontext *gc, __GLshaderProgramObject *programObject)
{
     GLProgram program = (GLProgram)programObject->privateData;
     GLint i;

   /* Free any existing log. */
     if (programObject->programInfo.infoLog != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, programObject->programInfo.infoLog));
        programObject->programInfo.infoLog = NULL;
    }

    programObject->programInfo.invalidFlag = 0;

    if (program->statesSize <= 0) {
        programObject->programInfo.invalidFlag |= GLSL_INVALID_LINK_BIT;
    }

    for (i = 0; i < programObject->programInfo.attachedShadersTableSize; i++) {
        if (programObject->programInfo.attachedShaders[i]) {
            if (programObject->programInfo.attachedShaders[i]->shaderInfo.compiledStatus) {
                programObject->programInfo.invalidFlag |= GLSL_INVALID_CODE_BIT;
            }
        }
    }

    return GL_TRUE;
}

GLboolean __glChipGetActiveAttribute(__GLcontext *gc, __GLshaderProgramObject *programObject,
    GLuint index,
    GLsizei bufsize,
    GLsizei * length,
    GLint * size,
    GLenum * type,
    char * name)
{
    GLProgram program = (GLProgram)programObject->privateData;
    gcATTRIBUTE attribute;
    gctSIZE_T attributeNameLength;
    gctCONST_STRING attributeName;
    gcSHADER_TYPE attributeType = gcSHADER_FLOAT_X1;
    gceSTATUS       status;

    /* Get gcATTRIBUTE object pointer. */
    attribute = program->attributePointers[index];

    if (attribute == gcvNULL)
    {
        return GL_FALSE;
    }

    /* Get name of attribute. */
    gcmONERROR(gcATTRIBUTE_GetName(program->vertexShader,
                                   attribute,
                                   gcvFALSE,
                                   &attributeNameLength,
                                   &attributeName));

    /* Make sure it fits within the buffer. */
    if (attributeNameLength > (gctSIZE_T) bufsize - 1)
    {
        attributeNameLength = gcmMAX(bufsize - 1, 0);
    }

    if (name != gcvNULL)
    {
        if (attributeNameLength > 0)
        {
            /* Copy the name to the buffer. */
            gcoOS_MemCopy(name, attributeName, attributeNameLength);
        }

        name[attributeNameLength] = '\0';
    }

    if (length != gcvNULL)
    {
        /* Return number of characters copied. */
        *length = attributeNameLength;
    }

    if (size != gcvNULL)
    {
        /* Get the type of the attribute. */
        gcmONERROR(gcATTRIBUTE_GetType(program->vertexShader,
                                       attribute,
                                       &attributeType,
                                       (gctSIZE_T *) size));
    }

    /* Convert type to GL enumeration. */
    if (type != gcvNULL)
    {
        *type = gc2glType[attributeType];
    }

    return GL_TRUE;

OnError:
    return GL_FALSE;
}

GLboolean __glChipGetUniforms(
    __GLcontext *gc,
    __GLshaderProgramObject *programObject,
    GLint Location,
    GLint Type,
    GLvoid * Values
    )
{

    GLProgram program = (GLProgram)programObject->privateData;
    GLUniform uniform;
    gcSHADER_TYPE type;
    gctSIZE_T length;
    GLsizei bytes;
    GLsizei count;

    uniform = &program->uniforms[Location];
    if (gcmIS_ERROR(gcUNIFORM_GetType(uniform->uniform[0], &type, (gctSIZE_T *)&length)))
    {
        return GL_FALSE;
    }

    if (Type == GL_FLOAT) {
        switch (type) {
            case gcSHADER_FLOAT_X1:
            case gcSHADER_FLOAT_X2:
            case gcSHADER_FLOAT_X3:
            case gcSHADER_FLOAT_X4:
            case gcSHADER_FLOAT_2X2:
            case gcSHADER_FLOAT_3X3:
            case gcSHADER_FLOAT_4X4:
            case gcSHADER_BOOLEAN_X1:
            case gcSHADER_BOOLEAN_X2:
            case gcSHADER_BOOLEAN_X3:
            case gcSHADER_BOOLEAN_X4:
            case gcSHADER_INTEGER_X1:
            case gcSHADER_INTEGER_X2:
            case gcSHADER_INTEGER_X3:
            case gcSHADER_INTEGER_X4:
                bytes = gcType2Bytes(type);
                gcoOS_MemCopy(Values, uniform->data, bytes);
                return GL_TRUE;
            default:
                return GL_FALSE;
        }
    } else {
        switch (type) {
            case gcSHADER_BOOLEAN_X1:
            case gcSHADER_INTEGER_X1:
            case gcSHADER_SAMPLER_1D:
            case gcSHADER_SAMPLER_2D:
            case gcSHADER_SAMPLER_3D:
            case gcSHADER_SAMPLER_CUBIC:
                count = 1;
                break;

            case gcSHADER_BOOLEAN_X2:
            case gcSHADER_INTEGER_X2:
                count = 2;
                break;

            case gcSHADER_BOOLEAN_X3:
            case gcSHADER_INTEGER_X3:
                count = 3;
                break;

            case gcSHADER_BOOLEAN_X4:
            case gcSHADER_INTEGER_X4:
                count = 4;
                break;
            default:
                return GL_FALSE;
        }
        float2Int(Values, uniform->data, count);
        return GL_TRUE;
    }
    return GL_FALSE;
}


GLvoid __glChipBuildTextureEnableDim(__GLcontext *gc)
{
    __GLshaderProgramObject *programObject = gc->shaderProgram.currentShaderProgram;
    GLProgram program = (GLProgram)programObject->privateData;
    GLint i;
    GLuint prevEnableDim[__GL_MAX_TEXTURE_UNITS];
    GLuint currentEnableDim;
    GLint unit;
    GLboolean bTexDimConflict = GL_FALSE;

    __GL_MEMZERO(&prevEnableDim[0], __GL_MAX_TEXTURE_UNITS*sizeof(GLuint));

    for (i=0; i < gcmCOUNTOF(program->sampleMap) ; i++)
    {

        if ( !( program->samplerDirty & ( 1 << i ) ) )
                continue;

        unit = program->sampleMap[i].unit;
        if (program->sampleMap[i].shaderType == gcSHADER_TYPE_VERTEX) {
            switch (program->sampleMap[i].type) {
                case gcSHADER_SAMPLER_1D:
                case gcSHADER_SAMPLER_2D:
                case gcSHADER_SAMPLER_3D:
                case gcSHADER_SAMPLER_CUBIC:
                    programObject->bindingInfo.vsTexEnableDim[unit] = currentEnableDim =
                        samplerType2TexEnableDim[program->sampleMap[i].type - gcSHADER_SAMPLER_1D];
                    if ((prevEnableDim[unit] != 0) && (currentEnableDim != prevEnableDim[unit]))
                    {
                        bTexDimConflict = GL_TRUE;
                    }
                    prevEnableDim[unit] = currentEnableDim;
                    break;
                default:
                    break;
            }
        }

        if (program->sampleMap[i].shaderType == gcSHADER_TYPE_FRAGMENT) {
            switch (program->sampleMap[i].type) {
                case gcSHADER_SAMPLER_1D:
                case gcSHADER_SAMPLER_2D:
                case gcSHADER_SAMPLER_3D:
                case gcSHADER_SAMPLER_CUBIC:
                    programObject->bindingInfo.psTexEnableDim[unit] = currentEnableDim =
                        samplerType2TexEnableDim[program->sampleMap[i].type - gcSHADER_SAMPLER_1D];
                    if ((prevEnableDim[unit] != 0) && (currentEnableDim != prevEnableDim[unit]))
                    {
                        bTexDimConflict = GL_TRUE;
                    }
                    prevEnableDim[unit] = currentEnableDim;
                    break;
                default:
                    break;
            }
        }
        /* Add GS when HW supports GS */
    }
    programObject->bindingInfo.texConflict = bTexDimConflict;
}

GLboolean __glChipCheckTextureConflict(__GLcontext *gc, __GLshaderProgramObject *programObject)
{
    GLProgram program = (GLProgram)programObject->privateData;
    GLint i;
    GLuint currentEnableDim[__GL_MAX_TEXTURE_UNITS];
    GLuint enableDim = 0;
    GLuint unit = 0;
    GLboolean bTexDimConflict = GL_FALSE;

    /*
    *** Note :
    ***     Now we only check the conflits as spec want :
    ***     1) Conflict between active GLSL samplers.
    ***     2) Conflict between GLSL VS and FF PS.
    ***     But for completeness, we should check the conflict between GLSL VS
    *** and all other PS.
    */
    __GL_MEMZERO(currentEnableDim, sizeof(currentEnableDim));

    if(programObject->programInfo.fragShaderEnable != GL_TRUE)
    {
        for (unit = 0; unit < __GL_MAX_TEXTURE_UNITS; unit++)
        {
            currentEnableDim[unit] = gc->state.enables.texUnits[unit].enabledDimension;
        }
    }

    for (i=0; i < gcmCOUNTOF(program->sampleMap); i++)
    {

        if ( !( program->samplerDirty & ( 1 << i ) ) )
                continue;

        unit = program->sampleMap[i].unit;
        switch (program->sampleMap[i].type) {
            case gcSHADER_SAMPLER_1D:
            case gcSHADER_SAMPLER_2D:
            case gcSHADER_SAMPLER_3D:
            case gcSHADER_SAMPLER_CUBIC:
                enableDim = samplerType2TexEnableDim[program->sampleMap[i].type - gcSHADER_SAMPLER_1D];
                if(currentEnableDim[unit] == 0)
                {
                    currentEnableDim[unit] = enableDim;
                } else {
                    if(currentEnableDim[unit] != enableDim)
                    {
                        bTexDimConflict = GL_TRUE;
                    }
                    break;
                }
                break;
            default:
                break;
        }
    }
    return bTexDimConflict;
}
