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


#ifndef __chip_shader_h__
#define __chip_shader_h__

#ifdef __cplusplus
extern "C" {
#endif

enum __gceRecompileOptions
{
    __GL_CHIP_RO_DEFAULT = 0,
    __GL_CHIP_RO_OUTPUT_HIGHP_CONVERSION = 1,
};

typedef struct __GLchipSLBindingRec
{
    struct __GLchipSLBindingRec *next;
    char      *name;
    GLint     index;
} __GLchipSLBinding;


/* Definition for uniform */
typedef enum __GLchipUniformUsageRec
{
    /* All user defined uniform will get this type */
    __GL_CHIP_UNIFORM_USAGE_USER_DEFINED = 0,

    /* All compiler generated uniform, including LTC dummy uniform,
       transform feedback state and buffer uniform, etc., get this type */
    __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED,

    /* Depth range */
    __GL_CHIP_UNIFORM_USAGE_DEPTHRANGE_NEAR,
    __GL_CHIP_UNIFORM_USAGE_DEPTHRANGE_FAR,
    __GL_CHIP_UNIFORM_USAGE_DEPTHRANGE_DIFF,
#ifdef OPENGL40
    __GL_CHIP_UNIFORM_USAGE_MODELVIEW,
    __GL_CHIP_UNIFORM_USAGE_PROJECTION,
    __GL_CHIP_UNIFORM_USAGE_MVP,
    __GL_CHIP_UNIFORM_USAGE_TEXTURE_MATRIX,
    __GL_CHIP_UNIFORM_USAGE_NORMAL_MATRIX,
    __GL_CHIP_UNIFORM_USAGE_MODELVIEW_INV,
    __GL_CHIP_UNIFORM_USAGE_PROJECTION_INV,
    __GL_CHIP_UNIFORM_USAGE_MVP_INV,
    __GL_CHIP_UNIFORM_USAGE_TEXTURE_MATRIX_INV,
    __GL_CHIP_UNIFORM_USAGE_MODELVIEW_TRANS,
    __GL_CHIP_UNIFORM_USAGE_PROJECTION_TRANS,
    __GL_CHIP_UNIFORM_USAGE_MVP_TRANS,
    __GL_CHIP_UNIFORM_USAGE_TEXTURE_MATRIX_TRANS,
    __GL_CHIP_UNIFORM_USAGE_MODELVIEW_INVTRANS,
    __GL_CHIP_UNIFORM_USAGE_PROJECTION_INVTRANS,
    __GL_CHIP_UNIFORM_USAGE_MVP_INVTRANS,
    __GL_CHIP_UNIFORM_USAGE_TEXTURE_MATRIX_INVTRANS,
    __GL_CHIP_UNIFORM_USAGE_NORMAL_SCALE,
    __GL_CHIP_UNIFORM_USAGE_ClIP_PLANE,
    __GL_CHIP_UNIFORM_USAGE_POINT_SIZE,
    __GL_CHIP_UNIFORM_USAGE_POINT_SIZE_MIN,
    __GL_CHIP_UNIFORM_USAGE_POINT_SIZE_MAX,
    __GL_CHIP_UNIFORM_USAGE_POINT_FADE_THRESHOLD_SIZE,
    __GL_CHIP_UNIFORM_USAGE_POINT_DISTANCE_CONST_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_POINT_DISTANCE_LINEAR_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_POINT_DISTANCE_QUADRATIC_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_FRONT_MATERIAL_EMISSION,
    __GL_CHIP_UNIFORM_USAGE_FRONT_MATERIAL_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_MATERIAL_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_FRONT_MATERIAL_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_FRONT_MATERIAL_SHININESS,
    __GL_CHIP_UNIFORM_USAGE_BACK_MATERIAL_EMISSION,
    __GL_CHIP_UNIFORM_USAGE_BACK_MATERIAL_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_BACK_MATERIAL_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_BACK_MATERIAL_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_BACK_MATERIAL_SHININESS,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_POSITION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_HALF_VECTOR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_SPOT_DIRECTION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_SPOT_EXPONENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_SPOT_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_SPOT_COS_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_CONST_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_LINEAR_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT0_QUADRATIC_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_POSITION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_HALF_VECTOR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_SPOT_DIRECTION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_SPOT_EXPONENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_SPOT_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_SPOT_COS_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_CONST_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_LINEAR_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT1_QUADRATIC_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_POSITION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_HALF_VECTOR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_SPOT_DIRECTION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_SPOT_EXPONENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_SPOT_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_SPOT_COS_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_CONST_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_LINEAR_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT2_QUADRATIC_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_POSITION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_HALF_VECTOR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_SPOT_DIRECTION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_SPOT_EXPONENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_SPOT_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_SPOT_COS_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_CONST_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_LINEAR_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT3_QUADRATIC_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_POSITION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_HALF_VECTOR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_SPOT_DIRECTION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_SPOT_EXPONENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_SPOT_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_SPOT_COS_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_CONST_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_LINEAR_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT4_QUADRATIC_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_POSITION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_HALF_VECTOR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_SPOT_DIRECTION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_SPOT_EXPONENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_SPOT_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_SPOT_COS_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_CONST_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_LINEAR_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT5_QUADRATIC_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_POSITION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_HALF_VECTOR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_SPOT_DIRECTION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_SPOT_EXPONENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_SPOT_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_SPOT_COS_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_CONST_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_LINEAR_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT6_QUADRATIC_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_POSITION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_HALF_VECTOR,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_SPOT_DIRECTION,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_SPOT_EXPONENT,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_SPOT_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_SPOT_COS_CUTOFF,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_CONST_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_LINEAR_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHT7_QUADRATIC_ATTENU,
    __GL_CHIP_UNIFORM_USAGE_LIGHTMODEL_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHTMODEL_PRODUCT_SCENECOLOR,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHTMODEL_PRODUCT_SCENECOLOR,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT0_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT0_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT0_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT0_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT0_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT0_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT1_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT1_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT1_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT1_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT1_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT1_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT2_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT2_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT2_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT2_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT2_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT2_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT3_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT3_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT3_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT3_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT3_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT3_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT4_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT4_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT4_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT4_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT4_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT4_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT5_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT5_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT5_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT5_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT5_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT5_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT6_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT6_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT6_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT6_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT6_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT6_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT7_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT7_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_FRONT_LIGHT7_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT7_PRODUCT_AMBIENT,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT7_PRODUCT_DIFFUSE,
    __GL_CHIP_UNIFORM_USAGE_BACK_LIGHT7_PRODUCT_SPECULAR,
    __GL_CHIP_UNIFORM_USAGE_TEXENV_COLOR,
    __GL_CHIP_UNIFORM_USAGE_TEXGEN_EYE_PLANE_S,
    __GL_CHIP_UNIFORM_USAGE_TEXGEN_EYE_PLANE_T,
    __GL_CHIP_UNIFORM_USAGE_TEXGEN_EYE_PLANE_R,
    __GL_CHIP_UNIFORM_USAGE_TEXGEN_EYE_PLANE_Q,
    __GL_CHIP_UNIFORM_USAGE_TEXGEN_OBJECT_PLANE_S,
    __GL_CHIP_UNIFORM_USAGE_TEXGEN_OBJECT_PLANE_T,
    __GL_CHIP_UNIFORM_USAGE_TEXGEN_OBJECT_PLANE_R,
    __GL_CHIP_UNIFORM_USAGE_TEXGEN_OBJECT_PLANE_Q,
    __GL_CHIP_UNIFORM_USAGE_FOG_COLOR,
    __GL_CHIP_UNIFORM_USAGE_FOG_DENSITY,
    __GL_CHIP_UNIFORM_USAGE_FOG_START,
    __GL_CHIP_UNIFORM_USAGE_FOG_END,
    __GL_CHIP_UNIFORM_USAGE_FOG_SCALE,
#endif
    __GL_CHIP_UNIFORM_USAGE_LAST
} __GLchipUniformUsage;

/*
** These uniforms have __GLSL_UNIFORM_USAGE_COMPILER_GENERATED.
** With subUsage, we can recognize which one it is.
*/
typedef enum __GLchipUniformSubUsageRec
{
    /* Only for some compiler generated unfirom, driver need recognize them,
    ** we need set its subUsage, others we don't care.
    */
    __GL_CHIP_UNIFORM_SUB_USAGE_NOT_CARE = 0,

    __GL_CHIP_UNIFORM_SUB_USAGE_XFB_ENABLE,
    __GL_CHIP_UNIFORM_SUB_USAGE_XFB_BUFFER,

    /* For yInverted rendering */
    __GL_CHIP_UNIFORM_SUB_USAGE_RTHEIGHT,

    __GL_CHIP_UNIFORM_SUB_USAGE_BASE_LEVEL_SIZE,
    __GL_CHIP_UNIFORM_SUB_USAGE_LOD_MIN_MAX,

    __GL_CHIP_UNIFORM_SUB_USAGE_LTC,

    __GL_CHIP_UNIFORM_SUB_USAGE_IMAGE_SIZE,
    __GL_CHIP_UNIFORM_SUB_USAGE_GROUPNUM,

    __GL_CHIP_UNIFORM_SUB_USAGE_STARTVERTEX,
    __GL_CHIP_UNIFORM_SUB_USAGE_VERTEXCOUNT_PER_INSTANCE,

    __GL_CHIP_UNIFORM_SUB_USAGE_MULTILAYER_SAMPLER,

    __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_SAMPLER,

    __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_STATE,

    __GL_CHIP_UNIFORM_SUB_USAGE_MULTILAYER_IMAGE,

    __GL_CHIP_UNIFORM_SUB_USAGE_RTWIDTH,

    __GL_CHIP_UNIFORM_SUB_USAGE_SAMPLEMASK_VALUE,

    __GL_CHIP_UNIFORM_SUB_USAGE_COVERAGE_INV,

    __GL_CHIP_UNIFORM_SUB_USAGE_SAMPLE_NUM,

    __GL_CHIP_UNIFORM_SUB_USAGE_TCS_PATCH_VERTICES_IN,

    __GL_CHIP_UNIFORM_SUB_USAGE_TES_PATCH_VERTICES_IN,

    __GL_CHIP_UNIFORM_SUB_USAGE_SAMPLE_LOCATIONS,

    __GL_CHIP_UNIFORM_SUB_USAGE_MULTISAMPLE_BUFFERS,

    __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_SAMPLER,

    __GL_CHIP_UNIFORM_SUB_USAGE_YINVERT,

    __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_RT_WH,

    __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_EQUATION,

    __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_FUNCTION,

    __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_CONSTANT_COLOR,

    __GL_CHIP_UNIFORM_SUB_USAGE_DEPTH_BIAS,

    __GL_CHIP_UNIFORM_SUB_USAGE_RT_IMAGE,

    __GL_CHIP_UNIFORM_SUB_USAGE_LAST
} __GLchipUniformSubUsage;

/* Definition for uniform block */
typedef enum __GLchipUbUsageRec
{
    __GL_CHIP_UB_USAGE_USER_DEFINED = 0,

    /* Store builtin and default user-defined uniforms */
    __GL_CHIP_UB_USAGE_DEFAULT,

    /* Store compile generated LTC or constant uniforms */
    __GL_CHIP_UB_USAGE_PRIVATE,

    __GL_CHIP_UB_USAGE_LAST
} __GLchipUbUsage;

typedef struct __GLchipUniformTypeInfoRec
{
    gcSHADER_TYPE   halType;        /* Compiler defined type */
    gctUINT         components;      /* e.g. 4 components each row */
    GLenum          glType;         /* GL defined type */
    gctSIZE_T       size;           /* Size in bytes */
} __GLchipUniformTypeInfo;


struct __GLchipSLUniformRec
{
    /* Record halUniform in its corresponding stage */
    gcUNIFORM           halUniform[__GLSL_STAGE_LAST];
    gctUINT32           stateAddress[__GLSL_STAGE_LAST];
    gctSTRING           name;
    gctSIZE_T           nameLen;
    gcSHADER_PRECISION  precision;
    gcSHADER_TYPE       dataType;
    gcSHADER_TYPE_KIND  category;
    gctBOOL             isArray;
    gctSIZE_T           arraySize;
    GLint               location;

    /* what's the binding for opaque uniforms? */
    GLint               binding;

    gctINT              numArraySize;
    gctUINT             entryIdx;
    gctINT             *arraySizes;

    __GLchipUniformUsage    usage;
    __GLchipUniformSubUsage subUsage;

    /* which uniform block it belongs, -1 means it was not in any UB. */
    GLint               ubIndex;

    /* which atomic_counter (block) it belongs, -1 means it was not an atomic_counter. */
    GLint               acbIndex;

    /* which kind of UB it belongs to? __GL_CHIP_UB_USAGE_LAST means it was not in any UB*/
    __GLchipUbUsage     ubUsage;
    gctINT              offset;
    /* always be -1 for uniforms in default block */
    gctINT              arrayStride;
    gctINT              matrixStride;
    gctBOOL             isRowMajor;
    GLvoid             *data;

    /* Whether this uniform has the initializer in the shader. */
    gctBOOL             hasInitializerInShader[__GLSL_STAGE_LAST];
    gctUINT8           *initializerData;

    /* The reg offset from a[0]...[0] */
    gctUINT32           regOffset;

    GLboolean           dirty;
#ifdef OPENGL40
    GLboolean           usrDef;
#endif
};

#define gcdUB_MAPPED_TO_MEM 0x1
#define gcdUB_MAPPED_TO_REG 0x2

typedef struct __GLchipSLUniformBlockRec
{
    gcsUNIFORM_BLOCK    halUB[__GLSL_STAGE_LAST];
    gcUNIFORM           halUniform[__GLSL_STAGE_LAST];
    gctUINT32           stateAddress[__GLSL_STAGE_LAST];
    GLboolean           refByStage[__GLSL_STAGE_LAST];
    GLbitfield          mapFlags[__GLSL_STAGE_LAST];
    /* Overall map flag for all shader stages */
    GLbitfield          mapFlag;

    gctCONST_STRING     name;
    gctSIZE_T           nameLen;
    gctSIZE_T           dataSize;
    __GLchipUbUsage     usage;
    gctSIZE_T           activeUniforms; /* active uniforms written to uniformIndices */
    gctSIZE_T           indexSize;      /* actual allocated size of uniformIndices */
    GLuint             *uniformIndices;

    GLuint              binding;

    /* For default/private UBOs, driver allocates storage internally. */
    gcoBUFOBJ           halBufObj;
    GLubyte*            bufBase;
} __GLchipSLUniformBlock;

/*
** Iterms for atomic_counter_buffer
*/
typedef struct __GLchipSLXfbVaryingRec
{
    gctCONST_STRING name;
    gctUINT         nameLen;
    gcSHADER_TYPE   type;
    gctBOOL         isArray;
    gctUINT         arraySize;
    gctUINT         stride;
} __GLchipSLXfbVarying;

/*
** Iterms for atomic_counter_buffer
*/
typedef struct __GLchipSLAtomCntBufRec
{
    gcUNIFORM           halUniform[__GLSL_STAGE_LAST];
    GLint               binding;
    gctSIZE_T           dataSize;
    GLint               activeACs;

    /* The allocated size of uniformIndices*/
    GLuint              *uniformIndices;
} __GLchipSLAtomCntBuf;

/*
** Iterms for buffer variables
*/
typedef struct __GLchipSLBufVariableRec
{
    gcVARIABLE          halBV[__GLSL_STAGE_LAST];
    GLboolean           refByStage[__GLSL_STAGE_LAST];

    gctSTRING           name;
    gctUINT             nameLen;

    gcSHADER_PRECISION  precision;
    gcSHADER_TYPE       dataType;
    gctBOOL             isArray;
    gctUINT             arraySize;
    gctINT              arrayStride;
    gctINT              matrixStride;
    gctBOOL             isRowMajor;

    gctINT              numArraySize;
    gctINT             *arraySizes;

    gctUINT             topLevelArraySize;
    gctUINT             topLevelArrayStride;

    gctINT              offset;
    gctUINT             ssbIndex;
} __GLchipSLBufVariable;

typedef struct __GLchipBVinfoRec
{
    GLuint          index;
    gctSTRING       name;
    gctINT          offset;
    gcVARIABLE      halBV[__GLSL_STAGE_LAST];
} __GLchipBVinfo;

/*
** Iterms for shader storage blocks
*/

typedef enum __GLchipSsbUsageRec
{
    __GL_CHIP_SSB_USAGE_USERDEF = 0,

    /* For compute shared variables */
    __GL_CHIP_SSB_USAGE_SHAREDVAR,

    /* For reg more than HW caps */
    __GL_CHIP_SSB_USAGE_EXTRAREG,

    __GL_CHIP_SSB_USAGE_LAST
} __GLchipSsbUsage;

typedef struct __GLchipSLStorageBlockRec
{
    gcsSTORAGE_BLOCK    halSB[__GLSL_STAGE_LAST];
    gcUNIFORM           halUniform[__GLSL_STAGE_LAST];
    gctUINT32           stateAddress[__GLSL_STAGE_LAST];
    GLboolean           refByStage[__GLSL_STAGE_LAST];

    gctCONST_STRING     name;
    gctUINT             nameLen;
    GLuint              dataSize;

    __GLchipSsbUsage    usage;

    /* Index to its pre-sibling slot, -1 if no pre-sibling. */
    GLint               preSiblingIdx;

    GLuint              activeBVs;  /* active buffer variables written to bvInfos */
    GLuint              bvSize;     /* actual allocated array size of bvInfos */
    __GLchipBVinfo     *bvInfos;

    GLuint              binding;

    /* For private SSBs, driver allocates storage internally. */
    gcoBUFOBJ           halBufObj;
    GLuint              groups;
} __GLchipSLStorageBlock;

/*
** Items of fragment varying output table
*/
typedef struct __GLchipSLOutputRec
{
    gctSTRING           name;
    gctUINT             nameLen;
    gcSHADER_TYPE       type;
    gcSHADER_PRECISION  precision;
    gctBOOL             isArray;
    gctINT              arraySize;
    gctINT              startIndex;
    GLint               location;
    gctINT              fieldIndex;
    gceLAYOUT_QUALIFIER layout;
    GLboolean           refByStage[__GLSL_STAGE_LAST];
    gctBOOL             isPerPatch;
} __GLchipSLOutput;

typedef struct __GLchipSLInputRec
{
    gcATTRIBUTE         halAttrib;
    gctSTRING           name;
    gctUINT             nameLen;
    gcSHADER_TYPE       type;
    gcSHADER_PRECISION  precision;
    gctBOOL             isArray;
    gctINT              arraySize;
    gctUINT             size;       /* size in basic vs input (attribute) units, i.e. matrix will be expanded */
    gctINT              location;   /* location which is set by shader text, -1 means not be set*/
    gctBOOL             isBuiltin;
    gctBOOL             isPosition;
    gctINT              fieldIndex; /* index inside a structure */
    GLboolean           refByStage[__GLSL_STAGE_LAST];
    gctBOOL             isPerPatch;
    gctBOOL             isSampleIn;
    gctBOOL             isDirectPosition;
} __GLchipSLInput;

typedef struct __GLchipSLLocationRec
{
    __GLchipSLInput      *pInput;
    GLint                 index;
    GLboolean             assigned;
} __GLchipSLLocation;

typedef struct __GLchipSLLinkage
{
    struct __GLchipSLLinkage *next;
    GLint                    attribLocation;
} __GLchipSLLinkage;

typedef enum __GL_CHIP_LTCBoolValue_enum
{
    __GL_CHIP_LTC_BoolValue_false,
    __GL_CHIP_LTC_BoolValue_true,
    __GL_CHIP_LTC_BoolValue_any           /* any value, don't care, it happens when the
                                     nested loop is dead code with previous condition */
} __GLchipLTCBoolValue;

#define __GL_CHIP_LTC_MAX_BRANCH_MULTIVERSION   2

typedef struct __GLchipCachedCodeRec
{
    __GLchipLTCBoolValue     value[__GL_CHIP_LTC_MAX_BRANCH_MULTIVERSION];
    gctSIZE_T                statesSize;
    gctPOINTER               states;
    gcsHINT_PTR              hints;
    struct __GLchipCachedCodeRec *  next;   /* next cached code */
} __GLchipCachedCode;

/* Program statekey, note that we need assure key is as small as possible to decrease time of hash hit.
   Meanwhile, key is also initialized to zero */
typedef union __GLchipPgStateKeyNP2AddrModeRec
{
    struct __GLaddrModeRec
    {
        GLubyte                         addrModeS : 2; /* Default is gcvTEXTURE_INVALID(0) */
        GLubyte                         addrModeT : 2; /* Default is gcvTEXTURE_INVALID(0) */
        GLubyte                         addrModeR : 2; /* Default is gcvTEXTURE_INVALID(0) */
        GLubyte                         reserved  : 2;
    } add;

    GLubyte                             value;
} __GLchipPgStateKeyNP2AddrMode;

typedef struct __GLchipPgStateKeyShadowMapCmpInfoRec
{
    struct __GLshadowMapCmpRec
    {
        GLubyte                         cmpMode   : 2; /* Default is gcvTEXTURE_COMPARE_MODE_INVALID(0) */
        GLubyte                         cmpOp     : 4; /* Default is gcvCOMPARE_INVALID(0) */
        GLubyte                         reserved  : 2;
    } cmp;

    GLuint                              texFmt;        /* Default is gcvSURF_UNKNOWN */

} __GLchipPgStateKeyShadowMapCmpInfo;

typedef struct __GLchipPgStateKeyTexPatchInfoRec
{
    gceTEXTURE_SWIZZLE swizzle[gcvTEXTURE_COMPONENT_NUM];
    GLuint  format;
    GLboolean needFormatConvert;
} __GLchipPgStateKeyTexPatchInfo;


typedef struct __GLchipProgramStateStaticKeyRec
{
    gceSURF_FORMAT                      rtPatchFmt[__GL_MAX_DRAW_BUFFERS];        /* Per draw buffer, default is gcvSURF_UNKNOWN */
    GLuint                              constTexcrdEnableMask;                    /* Per HW sampler slot, default is 0 */
    gctBOOL                             shaderBlendPatch;                         /* Default is disabled */
    gctBOOL                             removeAlpha[__GL_MAX_DRAW_BUFFERS];       /* Per draw buffer */
    gctBOOL                             drawYInverted;                            /* has yInverted-aware code */
    gctBOOL                             sampleMask;                               /* has sampleMask code */
    gctBOOL                             sampleCoverage;                           /* has sampleCoverage code */
    gctBOOL                             alpha2Coverage;                           /* has alpha2Coverage code */
    gctINT                              tcsPatchInVertices;                       /* real number of patchIn vertices for tcs */
    gctBOOL                             colorKill;
    gctBOOL                             alphaBlend;                               /* has blend code */
    gctBOOL                             ShaderPolygonOffset;                      /* has shader polygon offset */
    gctBOOL                             highpConversion;
}__GLchipProgramStateStaticKey;

typedef struct __GLchipProgramStateKeyRec
{
    __GLchipPgStateKeyNP2AddrMode       *NP2AddrMode;
    __GLchipPgStateKeyShadowMapCmpInfo  *shadowMapCmpInfo;
    __GLchipPgStateKeyTexPatchInfo      *texPatchInfo;
    __GLchipProgramStateStaticKey       *staticKey;

    /* the data of the above pointer.*/
    GLuint size;
    GLvoid *data;
}__GLchipProgramStateKey;

typedef union __GLchipProgramStateKeyMaskRec
{
    struct __GLchipProgramStateMaskRec
    {
        GLuint                          hasNP2AddrMode          : 1; /* Set when NP2 texture is set */
        GLuint                          hasShadowMapCmp         : 1; /* Set when shadow map compare parameters are set */
        GLuint                          hasTexPatchFmt          : 1; /* Set when texture format needs shader patch */
        GLuint                          hasRtPatchFmt           : 1; /* Set when rt format needs shader patch */
        GLuint                          hasConstTexcrd          : 1; /* Set when sampler is filtered by constant texcoord */
        GLuint                          hasShaderBlend          : 1; /* Set when using ETC2 RGBA8 texture with src blend  */
        GLuint                          hasRemoveAlpha          : 1; /* Set when we have opt to remove alpha assignment */
        GLuint                          hasYinverted            : 1; /* Set when we have yInverted-awared code in ps */
        GLuint                          hasSampleMask           : 1; /* Set when we have sample mask code in ps */
        GLuint                          hasSampleCov            : 1; /* Set when we have sample coverage code in ps */
        GLuint                          hasAlpha2Cov            : 1; /* Set when we have alpha to coverage code in ps */
        GLuint                          hasTcsPatchInVertices   : 1; /* Set when we have mismatched tcs patchIn vertices with predicted value */
        GLuint                          hasColorKill            : 1; /* Set when we found color.xyzw = 0 and blend function (1, 1-srcAlpha) to discard color */
        GLuint                          hasAlphaBlend           : 1; /* Set when we need do shader blend */
        GLuint                          hasPolygonOffset        : 1; /* Set when we do shader polygone offset */
        GLuint                          hasPSOutHighpConversion : 1; /*  */
        GLuint                          reserved                : 16;
    } s;

    GLuint                              value;
} __GLchipProgramStateKeyMask;

typedef struct __GLchipRecompileInfoRec
{
    gcPatchDirective* recompilePatchDirectivePtr;
} __GLchipRecompileInfo;

typedef struct __GLchipProgCmdStateKeyRec
{
    struct {
        GLuint progId;
        GLuint linkSeq;
        GLuint instanceId;
    } stages[__GLSL_STAGE_LAST];
} __GLchipProgCmdStateKey;

typedef struct __GLchipProgramFlagsRec
{
    gctUINT alphaKill               : 1;
    gctUINT noLTC                   : 1;
    gctUINT skipRecompile           : 1;
    gctUINT cube_UserLOD            : 1;
    gctUINT CTSMaxUBOSize           : 1;

    gctUINT disableDual16           : 1;
    gctUINT disableHP_RCP           : 1;
    gctUINT disableHP_FRAC          : 1;
    gctUINT disableHP_IMMED         : 1;
    gctUINT disableMP_IMMED         : 1;
    gctUINT disableHP_TEXLD_COORD   : 1;
    gctUINT enableHP_RCP            : 1;
    gctUINT enableHP_FRAC           : 1;
    gctUINT enableHP_IMMED          : 1;
    gctUINT enableMP_IMMED          : 1;
    gctUINT enableHP_TEXLD_COORD    : 1;

    gctUINT enableNetflix           : 1;
    gctUINT robustEnabled           : 1;
    gctUINT msaaOQ                  : 1;
    gctUINT alphaBlend              : 1;

    gctUINT outputHighpConversion   : 1;

    gctUINT VIRCGNone               : 1;    /* for compile-time purpose */
    gctUINT VIRCGOne                : 1;    /* for compile-time purpose */
    gctUINT disableInline           : 1;
    gctUINT deqpMinCompTime         : 1;    /* for compile-time purpose */

    gctUINT helperInvocationCheck   : 1;

    gctUINT reserved                : 10;


} __GLchipProgramFlags;



typedef struct __GLchipSamplerRec
{
    gcUNIFORM                       uniform;
    GLint                           arrayIndex;
    __GLSLStage                     stage;
    GLenum                          texDim;
    GLuint                          unit;
    GLboolean                       auxiliary;
    __GLchipUniformSubUsage         subUsage;
    GLboolean                       isInteger;
    __GLchipSLUniform               *slUniform;
    gcSHADER_KIND                   shaderType;
    gcSHADER_TYPE                   type;
}__GLchipSampler;


typedef struct __GLchipImageUnit2UniformRec
{
    GLuint numUniform;
    struct
    {
        __GLchipSLUniform           *slUniform;
        GLuint                       arrayIndex;
        gceIMAGE_FORMAT              formatQualifier;
        GLuint                       type;
        GLboolean                    auxiliary;
        __GLchipUniformSubUsage      subUsage;
    }uniforms[__GL_MAX_GLSL_IMAGE_UNIFORM];
}__GLchipImageUnit2Uniform;


/* Program instance */
typedef struct __GLchipSLProgramInstanceRec
{
    __GLchipUtilsObject                *ownerCacheObj;

    /* StateKey is calculated from context's keystate at draw time */
    __GLchipProgramStateKey            *pgStateKey;

    /* Unique id of this instance, same as CRCed state key */
    GLuint                              instanceId;

    gcSHADER                            binaries[__GLSL_STAGE_LAST];

    /* The saved binary before link for saving program */
    gcSHADER                            savedBinaries[__GLSL_STAGE_LAST];

    gcsPROGRAM_STATE                    programState;

    /* Per instance private uniforms */
    GLint                               privateUniformCount;
    __GLchipSLUniform*                  privateUniforms;
    gctBOOL                             hasLTC;

    /* Record XFB uniform info */
    __GLchipSLUniform                  *xfbActiveUniform;
    __GLchipSLUniform                  *xfbBufferUniforms[__GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS];
    __GLchipSLUniform                  *xfbVertexCountPerInstance;
    GLuint                              xfbBufferCount;

    /* sampler generated by recompilation or some speical case*/
    __GLchipSampler                     extraSamplerMap[__GL_MAX_GLSL_SAMPLERS];
    GLuint                              rtSampler;

    /* extra Image uniform */
    __GLchipImageUnit2Uniform extraImageUnit2Uniform[__GL_MAX_IMAGE_UNITS];
    GLuint extraImageUniformCount;

    /* covert gl_LastFragData to image uniform */
    __GLchipSLUniform                   *pLastFragData;

    /* Per instance private uniform blocks */
    GLint                               privateUbCount;
    __GLchipSLUniformBlock             *privateUBs;

    /* gl_NumWorkGroups uniform index (-1 means not exist)*/
    GLint                               groupNumUniformIdx;
    /* Driver needs this info to program extra resources that GPU pipeline needs */
    __GLchipRecompileInfo               recompilePatchInfo;

    /* TCS input patch vertices number from compiler, which is probably not true */
    GLint                               tcsPatchInVertices;

    /* advanced blend state uniform */
    __GLchipSLUniform                  *advBlendState;

    __GLchipProgramStateKeyMask         pgStateKeyMask;

    GLboolean                           master;

    GLboolean                           hasTXGatherSample;
} __GLchipSLProgramInstance;

#define __GL_PROG_INSTANCE_HASH_ENTRY_NUM   32
#define __GL_PROG_INSTANCE_HASH_ENTRY_SIZE  32
#define __GL_PROG_CMD_HASH_ENTRY_NUM        32
#define __GL_PROG_CMD_HASH_ENTRY_SIZE       32


struct __GLchipSLProgramRec
{
    /* Whether shader language is of version es30?
    ** In fact, it's better to record exact shader version.
    ** But compiler did not export that interface.
    */
    gctBOOL                             isHalti;
    GLboolean                           valid;
    GLuint                              codeSeq;

    /* Program Input */
    GLuint                              inCount;
    GLuint                              inMaxNameLen;
    __GLchipSLInput*                    inputs;

    __GLchipSLBinding*                  attribBinding;
    __GLchipSLLocation*                 attribLocation;
    /* The table maps app visible location to program attrib index */
    __GLchipSLLinkage**                 attribLinkage;

    /* Program Output */
    GLuint                              outMaxNameLen;
    GLuint                              outCount;
    __GLchipSLOutput*                   outputs;
    GLuint                              maxOutLoc;
    __GLchipSLOutput**                  loc2Out;

    /* Uniforms:
    ** Uniform indices are organized in "user-uniforms followed by builtIn uniforms" order
    */
    GLuint                              uniformMaxNameLen;
    GLint                               userDefUniformCount;
    GLint                               builtInUniformCount;
    GLint                               activeUniformCount;
    __GLchipSLUniform*                  uniforms;

    GLuint                              numSamplers[__GLSL_STAGE_LAST];

    /* Uniform location mapping table */
    GLint                               maxLocation;
    __GLchipSLUniform **                loc2Uniform;


    /* Bit-mask to record which samplers are of any shadow type, they need to be recompiled */
    __GLbitmask                         shadowSamplerMask;

    /* The array is indexed by HAL sampler physical index, which should be same as HW's sampler index */
    __GLchipSampler                     samplerMap[__GL_MAX_GLSL_SAMPLERS];

    /* Image uniform for program*/
    __GLchipImageUnit2Uniform imageUnit2Uniform[__GL_MAX_IMAGE_UNITS];
    GLuint imageUniformCount;

    /* Uniform Blocks
    ** UBs are organized in "userDef followed by default" order
    */
    gctSIZE_T                           ubMaxNameLen;
    GLint                               userDefUbCount;
    GLint                               defaultUbCount;
    GLint                               totalUbCount;
    __GLchipSLUniformBlock*             uniformBlocks;
    GLint                               maxActiveUniforms;
    /* Any userDef UB was mapped to reg? If yes, we cannot just flush
    ** when buffer binding changed, we also need to flush when buffer
    ** content changed. Bc current no buffer content dirty was tracked,
    ** we chose to flush every draw to make the case work.
    */
    gctBOOL                             ubMapToReg;

    /* Transform feedback Varyings */
    gctSIZE_T                           xfbMaxNameLen;
    gctUINT*                            xfbStride;   /* for interleaved mode */
    GLuint                              xfbCount;
    __GLchipSLXfbVarying*               xfbVaryings;

    /* Atomic Counter Buffer */
    GLint *                             acbBinding2SlotIdx; /* Mapping from acb binding to acbSlotIdx, -1 indicates none */
    GLint *                             acbBinding2NumACs;  /* Mapping from acb binding to number of atomic counter uniforms */
    GLuint                              acbCount;
    __GLchipSLAtomCntBuf *              acbs;
    GLint                               maxActiveACs;

    /* Buffer Variable */
    GLuint                              bvMaxNameLen;
    GLuint                              bufVariableCount;
    __GLchipSLBufVariable *             bufVariables;

    /* Shader Storage Block
    ** SSBs are organized in "userDef followed by private" order
    */
    GLuint                              ssbMaxNameLen;
    GLuint                              userDefSsbCount;
    GLuint                              privateSsbCount;
    GLuint                              totalSsbCount;
    __GLchipSLStorageBlock *            ssbs;
    GLint                               maxActiveBVs;

    /* Program instance cache of hash type */
    __GLchipUtilsHash*                  pgInstaceCache;

    /* Master instance which is the base of recompile */
    __GLchipSLProgramInstance*          masterPgInstance;

    /* Current program instance from instance pools for this program. All state validations must be dependent
       on current instance, not master instance. */
    __GLchipSLProgramInstance*          curPgInstance;

#if __GL_CHIP_PATCH_ENABLED
    /* Used for SW Culling patch */
    __GLchipSLUniform                   *mvpUniform;
    GLint                               aLocPosition;

    GLint                               aLocTexCoord;
    __GLchipSLUniform                   *isUserLodUniform;
    __GLchipSLUniform                   *userLodUniform;
    __GLchipSLUniform                   *biasUniform;

    GLint                               uboIndex;
    __GLchipSLUniform                   *indexUniform;

    /* Used for helper check.*/
    __GLchipSLUniform                   *pointXYUniform;
    __GLchipSLUniform                   *halfLineWidthUniform;
    __GLchipSLUniform                   *rtWHUniform;
    __GLchipSLUniform                   *bXMajorUniform;
    __GLchipSLUniform                   *bCheckHelperUniform;
#endif

    __GLchipProgramFlags                progFlags;

    gcePROGRAM_STAGE_BIT                stageBits;
};

typedef struct __GLchipProgramPipelineRec
{
    gcsPROGRAM_STATE programState;
}__GLchipProgramPipeline;


#ifdef OPENGL40

typedef struct _glsATTRIBUTEINFO * glsATTRIBUTEINFO_PTR;
typedef struct _glsATTRIBUTEINFO
{
    /* Current value. */
    glsVECTOR               currValue;
    GLboolean               dirty;

    /* Stream. */
    GLboolean               streamEnabled;
    gceVERTEX_FORMAT        format;
    GLboolean               normalize;
    GLuint                  components;
    gcSHADER_TYPE           attributeType;
    gcSHADER_TYPE           varyingType;
    gcSL_SWIZZLE            varyingSwizzle;
    GLsizei                 stride;
    GLsizei                 attributeSize;
    gctCONST_POINTER        pointer;
}
glsATTRIBUTEINFO;

typedef gceSTATUS (*glfUNIFORMSET) (
    __GLcontext *,
    gcUNIFORM
    );

typedef struct _glsUNIFORMWRAP * glsUNIFORMWRAP_PTR;
typedef struct _glsUNIFORMWRAP
{
    gcUNIFORM               uniform;
    glfUNIFORMSET           set;
}
glsUNIFORMWRAP;

typedef struct _glsATTRIBUTEWRAP * glsATTRIBUTEWRAP_PTR;
typedef struct _glsATTRIBUTEWRAP
{
    gcATTRIBUTE             attribute;
    glsATTRIBUTEINFO_PTR    info;
    gctINT                  binding;
}
glsATTRIBUTEWRAP;

typedef struct _glsSHADERCONTROL * glsSHADERCONTROL_PTR;
typedef struct _glsSHADERCONTROL
{
    gcSHADER                shader;
    glsUNIFORMWRAP_PTR      uniforms;
    glsATTRIBUTEWRAP_PTR    attributes;
    glsUNIFORMWRAP_PTR      texture[glvMAX_TEXTURES];
}
glsSHADERCONTROL;


typedef struct _glsPROGRAMINFO    * glsPROGRAMINFO_PTR;
typedef struct _glsPROGRAMINFO
{
    /* Timestamp. */
    gctUINT                 timestamp;

    /* Shader wrappers. */
    glsSHADERCONTROL        vs;
    glsSHADERCONTROL        fs;

    /* Shader program state. */
    gcsPROGRAM_STATE        programState;
}
glsPROGRAMINFO;

//typedef struct _glsFSCONTROL * glsFSCONTROL_PTR;


/******************************************************************************\
**************************** Shader generation macros. *************************
\******************************************************************************/

#define glmUSING_UNIFORM(Name) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl))

#define glmUSING_ATTRIBUTE(Name) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl))

#define glmUSING_INDEXED_ATTRIBUTE(Name, Index) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl, Index))

#define glmUSING_VARYING(Name) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl))

#define glmUSING_INDEXED_VARYING(Name, Index) \
    gcmERR_BREAK(using_##Name(gc, ShaderControl, Index))

#define glmDEFINE_TEMP(Name) \
    gctUINT16 Name

#define glmALLOCATE_LOCAL_TEMP(Name) \
    glmDEFINE_TEMP(Name) = allocateTemp(ShaderControl)

#define glmALLOCATE_TEMP(Name) \
    Name = allocateTemp(ShaderControl)

#define glmDEFINE_LABEL(Name) \
    gctUINT Name

#define glmALLOCATE_LOCAL_LABEL(Name) \
    glmDEFINE_LABEL(Name) = allocateLabel(ShaderControl)

#define glmALLOCATE_LABEL(Name) \
    Name = allocateLabel(ShaderControl)

#define glmASSIGN_OUTPUT(Name) \
    if (ShaderControl->Name != 0) \
    { \
        gcmERR_BREAK(assign_##Name( \
            gc, \
            ShaderControl, \
            ShaderControl->Name \
            )); \
    }

#define glmASSIGN_INDEXED_OUTPUT(Name, Index) \
    if (ShaderControl->Name[Index] != 0) \
    { \
        gcmERR_BREAK(assign_##Name( \
            gc, \
            ShaderControl, \
            Index \
            )); \
    }

#define glmOPCODE(Opcode, TempRegister, ComponentEnable) \
    gcmASSERT(TempRegister != 0); \
    gcmERR_BREAK(gcSHADER_AddOpcode( \
        ShaderControl->i->shader, \
        gcSL_##Opcode, \
        TempRegister, \
        gcSL_ENABLE_##ComponentEnable, \
        gcSL_FLOAT, \
        gcSHADER_PRECISION_LOW ,\
        0\
        ))

#define glmOPCODE_COND(Opcode, Condition, TempRegister, ComponentEnable) \
    gcmASSERT(TempRegister != 0); \
    gcmERR_BREAK(gcSHADER_AddOpcode2( \
        ShaderControl->i->shader, \
        gcSL_##Opcode, \
        gcSL_##Condition, \
        TempRegister, \
        gcSL_ENABLE_##ComponentEnable, \
        gcSL_FLOAT \
        ))

#define glmOPCODEV(Opcode, TempRegister, ComponentEnable) \
    gcmASSERT(TempRegister != 0); \
    gcmASSERT((ComponentEnable & ~gcSL_ENABLE_XYZW) == 0); \
    gcmERR_BREAK(gcSHADER_AddOpcode( \
        ShaderControl->i->shader, \
        gcSL_##Opcode, \
        TempRegister, \
        ComponentEnable, \
        gcSL_FLOAT, \
        gcSHADER_PRECISION_LOW, \
        0 \
        ))

#define glmOPCODE_BRANCH(Opcode, Condition, Target) \
    gcmERR_BREAK(gcSHADER_AddOpcodeConditional( \
        ShaderControl->i->shader, \
        gcSL_##Opcode, \
        gcSL_##Condition, \
        Target, \
        0 \
        ))

#define glmCONST(Value) \
    gcmERR_BREAK(gcSHADER_AddSourceConstant( \
        ShaderControl->i->shader, \
        (gctFLOAT) (Value) \
        ))

#define glmINT_CONST(Value) \
    gcmERR_BREAK(gcSHADER_AddSourceConstantFormatted( \
        ShaderControl->i->shader, \
        (gctINT*) (Value), \
        gcSL_INT32 \
        ))

#define glmUNIFORM_WRAP(Shader, Name) \
        ShaderControl->uniforms[glmUNIFORM_INDEX(Shader, Name)]

#define glmUNIFORM_WRAP_INDEXED(Shader, Name, Index) \
        ShaderControl->uniforms[glmUNIFORM_INDEX(Shader, Name) + Index]

#define glmUNIFORM(Shader, Name, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceUniform( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP(Shader, Name)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmUNIFORM_INDEXED(Shader, Name, Index, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceUniform( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP_INDEXED(Shader, Name, Index)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmUNIFORM_STATIC(Shader, Name, Swizzle, Index) \
    gcmERR_BREAK(gcSHADER_AddSourceUniform( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP(Shader, Name)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        Index \
        ))

#define glmUNIFORM_DYNAMIC(Shader, Name, Swizzle, IndexRegister) \
    gcmERR_BREAK(gcSHADER_AddSourceUniformIndexed( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP(Shader, Name)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        0, \
        gcSL_INDEXED_X, \
        IndexRegister \
        ))

#define glmUNIFORM_DYNAMIC_MATRIX(Shader, Name, Swizzle, IndexRegister, \
                                  Offset, IndexMode) \
    gcmERR_BREAK(gcSHADER_AddSourceUniformIndexed( \
        ShaderControl->i->shader, \
        glmUNIFORM_WRAP(Shader, Name)->uniform, \
        gcSL_SWIZZLE_##Swizzle, \
        Offset, \
        IndexMode, \
        IndexRegister \
        ))

#define glmATTRIBUTE_WRAP(Shader, Name) \
        ShaderControl->attributes[glmATTRIBUTE_INDEX(Shader, Name)]

#define glmATTRIBUTE_WRAP_INDEXED(Shader, Name, Index) \
        ShaderControl->attributes[glmATTRIBUTE_INDEX(Shader, Name) + Index]

#define glmATTRIBUTE(Shader, Name, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP(Shader, Name)->attribute, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmATTRIBUTE_INDEXED(Shader, Name, Index, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP_INDEXED(Shader, Name, Index)->attribute, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmATTRIBUTEV(Shader, Name, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP(Shader, Name)->attribute, \
        Swizzle, \
        0 \
        ))

#define glmVARYING(Shader, Name, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP(Shader, Name)->attribute, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmVARYING_INDEXED(Shader, Name, Index, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP_INDEXED(Shader, Name, Index)->attribute, \
        gcSL_SWIZZLE_##Swizzle, \
        0 \
        ))

#define glmVARYINGV_INDEXED(Shader, Name, Index, Swizzle) \
    gcmERR_BREAK(gcSHADER_AddSourceAttribute( \
        ShaderControl->i->shader, \
        glmATTRIBUTE_WRAP_INDEXED(Shader, Name, Index)->attribute, \
        Swizzle, \
        0 \
        ))

#define glmTEMP(TempRegister, Swizzle) \
    gcmASSERT(TempRegister != 0); \
    gcmERR_BREAK(gcSHADER_AddSource( \
        ShaderControl->i->shader, \
        gcSL_TEMP, \
        TempRegister, \
        gcSL_SWIZZLE_##Swizzle, \
        gcSL_FLOAT, \
        gcSHADER_PRECISION_LOW \
        ))

#define glmTEMPV(TempRegister, Swizzle) \
    gcmASSERT(TempRegister != 0); \
    gcmERR_BREAK(gcSHADER_AddSource( \
        ShaderControl->i->shader, \
        gcSL_TEMP, \
        TempRegister, \
        Swizzle, \
        gcSL_FLOAT, \
        gcSHADER_PRECISION_LOW \
        ))

#define glmADD_FUNCTION(FunctionName) \
    gcmERR_BREAK(gcSHADER_AddFunction( \
        ShaderControl->i->shader, \
        #FunctionName, \
        &ShaderControl->FunctionName \
        ))

#define glmBEGIN_FUNCTION(FunctionName) \
    gcmERR_BREAK(gcSHADER_BeginFunction( \
        ShaderControl->i->shader, \
        ShaderControl->FunctionName \
        ))

#define glmEND_FUNCTION(FunctionName) \
    gcmERR_BREAK(gcSHADER_EndFunction( \
        ShaderControl->i->shader, \
        ShaderControl->FunctionName \
        ))

#define glmRET() \
    gcmERR_BREAK(gcSHADER_AddOpcodeConditional( \
        ShaderControl->i->shader, \
        gcSL_RET, \
        gcSL_ALWAYS, \
        0, \
        0 \
        ))

#define glmLABEL(Label) \
    gcmERR_BREAK(gcSHADER_AddLabel( \
        ShaderControl->i->shader, \
        Label \
        ))



gceSTATUS glfUsingUniform(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctSIZE_T Length,
    IN glfUNIFORMSET UniformSet,
    IN glsUNIFORMWRAP_PTR* UniformWrap
    );

gceSTATUS glfUsingAttribute(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctSIZE_T Length,
    IN gctBOOL IsTexture,
    IN glsATTRIBUTEINFO_PTR AttributeInfo,
    IN glsATTRIBUTEWRAP_PTR* AttributeWrap,
    IN gctINT Binding,
    IN gcSHADER_SHADERMODE ShadingMode
    );

gceSTATUS glfUsingVarying(
    IN glsSHADERCONTROL_PTR ShaderControl,
    IN gctCONST_STRING Name,
    IN gcSHADER_TYPE Type,
    IN gctSIZE_T Length,
    IN gctBOOL IsTexture,
    IN glsATTRIBUTEWRAP_PTR* AttributeWrap,
    IN gcSHADER_SHADERMODE ShadingMode
    );

gceSTATUS glfUsing_uTexCoord(
    IN glsSHADERCONTROL_PTR ShaderControl,
    OUT glsUNIFORMWRAP_PTR* UniformWrap
    );

#endif

extern __GLchipUniformTypeInfo g_typeInfos[];

/* dynamic patch program */
extern gceSTATUS
gcChipDynamicPatchProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    gcPatchDirective* recompilePatchDirectivePtr,
    gctUINT32 Options
    );

extern GLboolean
__glChipCompileShader(
    __GLcontext *gc,
    __GLshaderObject *shaderObject
    );

extern GLvoid
__glChipDeleteShader(
    __GLcontext *gc,
    __GLshaderObject *shaderObject
    );

extern GLboolean
__glChipCreateProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject
    );

extern GLvoid
__glChipDeleteProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject
    );

extern GLboolean
__glChipLinkProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject
    );

extern GLboolean
__glChipUseProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLboolean *valid
    );

extern GLboolean
__glChipValidateProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLboolean callFromDraw
    );

extern GLboolean
__glChipBindAttributeLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    const GLchar *name
    );

extern GLboolean
__glChipGetProgramBinary_V1(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei bufSize,
    GLsizei* length,
    GLenum *binaryFormat,
    GLvoid* binary
    );

extern GLboolean
__glChipProgramBinary_V1(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLvoid *binary,
    GLsizei length
    );

extern GLboolean
__glChipShaderBinary(
    __GLcontext *gc,
    GLsizei n,
    __GLshaderObject **shadersObjects,
    GLenum binaryformat,
    const GLvoid *binary,
    GLsizei length
    );

extern GLint
__glChipGetAttributeLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *name
    );

extern GLboolean
__glChipGetActiveAttribute(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    GLsizei bufsize,
    GLsizei *length,
    GLint *size,
    GLenum *type,
    char *name
    );

extern GLint
__glChipGetFragDataLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *name
    );

#ifdef OPENGL40
extern GLvoid
__glChipBindFragDataLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint colorNumber,
    const GLchar *name
    );
#endif

extern GLint
__glChipGetUniformLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *name
    );

extern GLvoid
__glChipGetActiveUniform(
    __GLcontext *gc,
     __GLprogramObject *programObject,
     GLuint index,
     GLsizei bufsize,
     GLsizei *length,
     GLint *size,
     GLenum *type,
     GLchar *name
     );

extern GLvoid
__glChipGetActiveUniformsiv(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei uniformCount,
    const GLuint *uniformIndices,
    GLenum pname,
    GLint *params
    );

extern GLvoid
__glChipGetUniformIndices(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei uniformCount,
    const GLchar* const* uniformNames,
    GLuint *uniformIndices
    );

extern GLuint
__glChipGetUniformBlockIndex(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar* uniformBlockName
    );

extern GLvoid
__glChipGetActiveUniformBlockiv(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint uniformBlockIndex,
    GLenum pname,
    GLint *params
    );

extern GLvoid
__glChipActiveUniformBlockName(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint uniformBlockIndex,
    GLsizei bufSize,
    GLsizei *length,
    GLchar *uniformBlockName
    );

extern GLvoid
__glChipUniformBlockBinding(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint uniformBlockIndex,
    GLuint uniformBlockBinding
    );

extern GLboolean
__glChipSetUniformData(
    __GLcontext       *gc,
    __GLprogramObject *programObject,
    GLint              location,
    GLint              type,
    GLsizei            count,
    const GLvoid      *values,
    GLboolean          transpose
    );

extern GLboolean
__glChipGetUniformData(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLint location,
    GLint type,
    GLvoid *values
    );

extern GLsizei
__glChipGetUniformSize(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLint location
    );

extern GLvoid
__glChipBuildTexEnableDim(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipMarkUniformDirtyCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    );

gceSTATUS
gcChipClearUniformDirtyCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    );

extern gceSTATUS
gcChipFlushGLSLResourcesCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    );

extern gceSTATUS
gcChipFlushSingleUniform(
    __GLcontext *       gc,
    __GLchipSLProgram * program,
    __GLchipSLUniform * uniform,
    GLvoid *            data
    );

extern gceSTATUS
gcChipLoadCompiler(
    __GLcontext *gc
    );

extern gceSTATUS
gcChipReleaseCompiler(
    IN __GLcontext *gc
    );

extern __GLchipUtilsObject*
gcChipAddPgInstanceToCache(
    __GLcontext *gc,
    __GLchipSLProgram *program,
    GLuint key,
    GLboolean master
    );

extern GLuint
__glChipGetProgramResourceIndex(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLenum progInterface,
    const GLchar *name
    );

extern GLvoid
__glChipGetProgramResourceName(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLenum progInterface,
    GLuint index,
    GLsizei bufSize,
    GLsizei *length,
    GLchar *name
    );

extern GLvoid
__glChipGetProgramResourceiv(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLenum progInterface,
    GLuint index,
    GLsizei propCount,
    const GLenum *props,
    GLsizei bufSize,
    GLsizei *length,
    GLint *params
    );

GLboolean
__glChipValidateProgramPipeline(
    __GLcontext *gc,
    __GLprogramPipelineObject *ppObj,
    GLboolean callFromDraw
    );

gceSTATUS
gcChipPgStateKeyAlloc(
    __GLcontext *gc,
    __GLchipProgramStateKey **ppPgStateKey
    );

gceSTATUS
gcChipPgStateKeyFree(
    __GLcontext *gc,
    __GLchipProgramStateKey **ppPgStateKey
    );

gceSTATUS
gcChipPgStateKeyCopy(
    __GLcontext *gc,
    __GLchipProgramStateKey *pDst,
    __GLchipProgramStateKey *pSrc
    );

#ifdef OPENGL40
gceSTATUS glfGenerateVSFixedFunction(
    IN __GLcontext * gc
    );

gceSTATUS glfGenerateFSFixedFunction(
    IN __GLcontext * gc
    );
gceSTATUS
gcChipLoadFixFunctionShader(
    IN __GLcontext * gc
    );
#endif


#ifdef __cplusplus
}
#endif

#endif /* __chip_shader_h__ */
