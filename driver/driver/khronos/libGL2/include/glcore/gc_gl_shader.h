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


#ifndef __gc_gl_shader_h_
#define __gc_gl_shader_h_

/************************************************************************/
/* Definitions for object                                               */
/************************************************************************/
typedef enum __GL_SHADER_PROGRAM_TYPE {
    __GL_SHADER_OBJECT_TYPE = 0,
    __GL_PROGRAM_OBJECT_TYPE,
    __GL_SHADER_PROGRAM_TYPE_LAST,
}GL_SHADER_PROGRAM_TYPE;

#define __GL_MAX_ATTACHED_SHADERS    32

enum {
    __GLSL_CB_VS   = 0,
    __GLSL_CB_GS   = 1,
    __GLSL_CB_FS   = 2,
    __GLSL_CB_TYPES = 3,
};


/************************************************************************/
/* Definitions for uniform                                              */
/************************************************************************/
#define __GL_MAX_GLSL_VS_SAMPLERS       16
#define __GL_MAX_GLSL_GS_SAMPLERS       16
#define __GL_MAX_GLSL_PS_SAMPLERS       16

#define __GL_GLSL_PS_SAMPLER_SHIFT       0
#define __GL_GLSL_VS_SAMPLER_SHIFT       16
#define __GL_GLSL_GS_SAMPLER_SHIFT       32

#define __GL_GLSL_PS_SAMPLER_MASK       0x00000000ffff
#define __GL_GLSL_VS_SAMPLER_MASK       0x0000ffff0000
#define __GL_GLSL_GS_SAMPLER_MASK       0xffff00000000


#define __GL_MAX_GLSL_SAMPLERS \
    (__GL_MAX_GLSL_VS_SAMPLERS + __GL_MAX_GLSL_GS_SAMPLERS + __GL_MAX_GLSL_PS_SAMPLERS)

/* bit definition of uniform locations
** [15:  0] array index of the name. For scalar, it is 0.
** [29: 16] item index of the sampler table
** [31: 30] 00: uniform, 01: sampler
*/

#define __GL_LOCATION_UNIFORM_MASK (0x0L << 30)
#define __GL_LOCATION_SAMPLER_MASK (0x1L << 30)

#define __GEN_UNIFORM_LOCATION(itemIdx, arrayIdx) \
    (__GL_LOCATION_UNIFORM_MASK | ((itemIdx & 0x3fff) << 16) | (arrayIdx & 0xffff))
#define __GEN_SAMPLER_LOCATION(itemIdx, arrayIdx) \
    (__GL_LOCATION_SAMPLER_MASK | ((itemIdx & 0x3fff) << 16) | (arrayIdx & 0xffff))

#define __GET_UNIFORM_ITEM_INDEX(location)    ((location >> 16) & 0x3fff)
#define __GET_UNIFORM_ARRAY_INDEX(location)   (location & 0xffff)

/*
** Because SM4.0 think TRUE as 0xffffffff, we need transfer common TRUE value
** (none zero) to 0xfffffff, Otherwise we may get unexpected result when shader
** code use bit operation on bool value.
*/
#define __GLSL_TRUE     0xffffffff
#define __GLSL_FALSE    0x0

/************************************************************************/
/* Definitions for built-in state                                       */
/************************************************************************/
enum {
    __GLSL_USAGE_ALL_ATTRS              = 0,
    __GLSL_USAGE_TRANSFORM_ATTRS        = 1,
    __GLSL_USAGE_LIGHT_ATTRS            = 2,
    __GLSL_USAGE_TEXTURE_ATTRS          = 3,
    __GLSL_USAGE_MISC_ATTRS1            = 4,
    __GLSL_USAGE_MISC_ATTRS2            = 5,
    __GLSL_USAGE_END                    = 6,
};

/* Usage bits in globalAttrState[__GLSL_USAGE_TRANSFORM_ATTRS] */
enum {
    /* ModelView 4bits,
    * Projection 4bits,
    * MVP 4bits,
    * Normal matrix, 1bit
    * Normal Scale, 1bit
    */
    __GLSL_USAGE_MODELVIEW_BIT              = (1 << 0),
    __GLSL_USAGE_MODELVIEW_TRANS_BIT        = (1 << 1),
    __GLSL_USAGE_MODELVIEW_INV_BIT          = (1 << 2),
    __GLSL_USAGE_MODELVIEW_INVTRANS_BIT     = (1 << 3),

    __GLSL_USAGE_PROJECTION_BIT             = (1 << 4),
    __GLSL_USAGE_PROJECTION_TRANS_BIT       = (1 << 5),
    __GLSL_USAGE_PROJECTION_INV_BIT         = (1 << 6),
    __GLSL_USAGE_PROJECTION_INVTRANS_BIT    = (1 << 7),

    __GLSL_USAGE_MVP_BIT                    = (1 << 8),
    __GLSL_USAGE_MVP_TRANS_BIT              = (1 << 9),
    __GLSL_USAGE_MVP_INV_BIT                = (1 << 10),
    __GLSL_USAGE_MVP_INVTRANS_BIT           = (1 << 11),

    __GLSL_USAGE_NORMAL_MATRIX_BIT          = (1 << 12),
    __GLSL_USAGE_NORMAL_SCALE_BIT           = (1 << 13)
};

#define __GLSL_USAGE_MODELVIEW_BITS     ( __GLSL_USAGE_MODELVIEW_BIT |\
                                          __GLSL_USAGE_MODELVIEW_TRANS_BIT | \
                                          __GLSL_USAGE_MODELVIEW_INV_BIT | \
                                          __GLSL_USAGE_MODELVIEW_INVTRANS_BIT )

#define __GLSL_USAGE_PROJECTION_BITS    ( __GLSL_USAGE_PROJECTION_BIT |\
                                          __GLSL_USAGE_PROJECTION_TRANS_BIT |\
                                          __GLSL_USAGE_PROJECTION_INV_BIT |\
                                          __GLSL_USAGE_PROJECTION_INVTRANS_BIT )

#define __GLSL_USAGE_MVP_BITS   ( __GLSL_USAGE_MVP_BIT |\
                                  __GLSL_USAGE_MVP_TRANS_BIT |\
                                  __GLSL_USAGE_MVP_INV_BIT |\
                                  __GLSL_USAGE_MVP_INVTRANS_BIT )


/* Usage bits in lightAttrState[__GL_MAX_LIGHT_NUMBER] */
enum{
    /*
    * LightSource 12bits x 8
    * LightProduct 3bits x 2 x 8
    */
    __GLSL_USAGE_LIGHT_AMBIENT_BIT                  = (1 << 0),
    __GLSL_USAGE_LIGHT_DIFFUSE_BIT                  = (1 << 1),
    __GLSL_USAGE_LIGHT_SPECULAR_BIT                 = (1 << 2),
    __GLSL_USAGE_LIGHT_POSITION_BIT                 = (1 << 3),
    __GLSL_USAGE_LIGHT_HALF_VECTOR_BIT              = (1 << 4),
    __GLSL_USAGE_LIGHT_SPOT_DIRECTION_BIT           = (1 << 5),
    __GLSL_USAGE_LIGHT_SPOT_EXPONENT_BIT            = (1 << 6),
    __GLSL_USAGE_LIGHT_SPOT_CUTOFF_BIT              = (1 << 7),
    __GLSL_USAGE_LIGHT_SPOT_COS_CUTOFF_BIT          = (1 << 8),
    __GLSL_USAGE_LIGHT_CONST_ATTENU_BIT             = (1 << 9),
    __GLSL_USAGE_LIGHT_LINEAR_ATTENU_BIT            = (1 << 10),
    __GLSL_USAGE_LIGHT_QUADRATIC_ATTENU_BIT         = (1 << 11),

    __GLSL_USAGE_FRONT_LIGHT_PRODUCT_AMBIENT_BIT    = (1 << 12),
    __GLSL_USAGE_FRONT_LIGHT_PRODUCT_DIFFUSE_BIT    = (1 << 13),
    __GLSL_USAGE_FRONT_LIGHT_PRODUCT_SPECULAR_BIT   = (1 << 14),

    __GLSL_USAGE_BACK_LIGHT_PRODUCT_AMBIENT_BIT     = (1 << 15),
    __GLSL_USAGE_BACK_LIGHT_PRODUCT_DIFFUSE_BIT     = (1 << 16),
    __GLSL_USAGE_BACK_LIGHT_PRODUCT_SPECULAR_BIT    = (1 << 17),
};

#define __GLSL_USAGE_LIGHT_SRC_BITS     ( __GLSL_USAGE_LIGHT_AMBIENT_BIT |\
                                          __GLSL_USAGE_LIGHT_DIFFUSE_BIT |\
                                          __GLSL_USAGE_LIGHT_SPECULAR_BIT |\
                                          __GLSL_USAGE_LIGHT_POSITION_BIT |\
                                          __GLSL_USAGE_LIGHT_HALF_VECTOR_BIT |\
                                          __GLSL_USAGE_LIGHT_SPOT_DIRECTION_BIT |\
                                          __GLSL_USAGE_LIGHT_SPOT_EXPONENT_BIT |\
                                          __GLSL_USAGE_LIGHT_SPOT_CUTOFF_BIT |\
                                          __GLSL_USAGE_LIGHT_SPOT_COS_CUTOFF_BIT |\
                                          __GLSL_USAGE_LIGHT_CONST_ATTENU_BIT |\
                                          __GLSL_USAGE_LIGHT_LINEAR_ATTENU_BIT |\
                                          __GLSL_USAGE_LIGHT_QUADRATIC_ATTENU_BIT )

#define __GLSL_USAGE_LIGHT_PRODUCT_BITS ( __GLSL_USAGE_FRONT_LIGHT_PRODUCT_AMBIENT_BIT |\
                                          __GLSL_USAGE_FRONT_LIGHT_PRODUCT_DIFFUSE_BIT |\
                                          __GLSL_USAGE_FRONT_LIGHT_PRODUCT_SPECULAR_BIT |\
                                          __GLSL_USAGE_BACK_LIGHT_PRODUCT_AMBIENT_BIT |\
                                          __GLSL_USAGE_BACK_LIGHT_PRODUCT_DIFFUSE_BIT |\
                                          __GLSL_USAGE_BACK_LIGHT_PRODUCT_SPECULAR_BIT )

/* Usage bits in texAttrState[] */
enum{
    /* TexEnvColor 1bit x 8
    * TexGen 8bits x 8
    * TexTransform 4bits x 8
    */
    __GLSL_USAGE_TEXENV_COLOR_BIT                   = (1 << 0),

    __GLSL_USAGE_TEXGEN_EYE_PLANE_S_BIT             = (1 << 1),
    __GLSL_USAGE_TEXGEN_EYE_PLANE_T_BIT             = (1 << 2),
    __GLSL_USAGE_TEXGEN_EYE_PLANE_R_BIT             = (1 << 3),
    __GLSL_USAGE_TEXGEN_EYE_PLANE_Q_BIT             = (1 << 4),
    __GLSL_USAGE_TEXGEN_OBJECT_PLANE_S_BIT          = (1 << 5),
    __GLSL_USAGE_TEXGEN_OBJECT_PLANE_T_BIT          = (1 << 6),
    __GLSL_USAGE_TEXGEN_OBJECT_PLANE_R_BIT          = (1 << 7),
    __GLSL_USAGE_TEXGEN_OBJECT_PLANE_Q_BIT          = (1 << 8),

    __GLSL_USAGE_TEXTURE_MATRIX_BIT                 = (1 << 9),
    __GLSL_USAGE_TEXTURE_MATRIX_TRANS_BIT           = (1 << 10),
    __GLSL_USAGE_TEXTURE_MATRIX_INV_BIT             = (1 << 11),
    __GLSL_USAGE_TEXTURE_MATRIX_INVTRANS_BIT        = (1 << 12),
};

#define __GLSL_USAGE_TEX_BITS   ( __GLSL_USAGE_TEXENV_COLOR_BIT |\
                                  __GLSL_USAGE_TEXGEN_EYE_PLANE_S_BIT |\
                                  __GLSL_USAGE_TEXGEN_EYE_PLANE_T_BIT |\
                                  __GLSL_USAGE_TEXGEN_EYE_PLANE_R_BIT |\
                                  __GLSL_USAGE_TEXGEN_EYE_PLANE_Q_BIT |\
                                  __GLSL_USAGE_TEXGEN_OBJECT_PLANE_S_BIT |\
                                  __GLSL_USAGE_TEXGEN_OBJECT_PLANE_T_BIT |\
                                  __GLSL_USAGE_TEXGEN_OBJECT_PLANE_R_BIT |\
                                  __GLSL_USAGE_TEXGEN_OBJECT_PLANE_Q_BIT |\
                                  __GLSL_USAGE_TEXTURE_MATRIX_BIT |\
                                  __GLSL_USAGE_TEXTURE_MATRIX_TRANS_BIT |\
                                  __GLSL_USAGE_TEXTURE_MATRIX_INV_BIT |\
                                  __GLSL_USAGE_TEXTURE_MATRIX_INVTRANS_BIT )

#define __GLSL_USAGE_TEXGEN_BITS    ( __GLSL_USAGE_TEXGEN_EYE_PLANE_S_BIT |\
                                      __GLSL_USAGE_TEXGEN_EYE_PLANE_T_BIT |\
                                      __GLSL_USAGE_TEXGEN_EYE_PLANE_R_BIT |\
                                      __GLSL_USAGE_TEXGEN_EYE_PLANE_Q_BIT |\
                                      __GLSL_USAGE_TEXGEN_OBJECT_PLANE_S_BIT |\
                                      __GLSL_USAGE_TEXGEN_OBJECT_PLANE_T_BIT |\
                                      __GLSL_USAGE_TEXGEN_OBJECT_PLANE_R_BIT |\
                                      __GLSL_USAGE_TEXGEN_OBJECT_PLANE_Q_BIT )

#define __GLSL_USAGE_TEXTURE_MATRIX_BITS    (__GLSL_USAGE_TEXTURE_MATRIX_BIT |\
                                             __GLSL_USAGE_TEXTURE_MATRIX_TRANS_BIT |\
                                             __GLSL_USAGE_TEXTURE_MATRIX_INV_BIT |\
                                             __GLSL_USAGE_TEXTURE_MATRIX_INVTRANS_BIT )



/* Usage bits in globalAttrState[__GLSL_USAGE_MISC_ATTRS1] */
enum {
    /* DepthRange 3bits
    * ClipPlane 6bits
    * PointParam 7bis
    * FogParam 5bits
    */
    __GLSL_USAGE_DEPTHRANGE_NEAR_BIT                    = (1 << 0),
    __GLSL_USAGE_DEPTHRANGE_FAR_BIT                     = (1 << 1),
    __GLSL_USAGE_DEPTHRANGE_DIFF_BIT                    = (1 << 2),

    __GLSL_USAGE_CLIP_PLANE0_BIT                        = (1 << 3),
    __GLSL_USAGE_CLIP_PLANE1_BIT                        = (1 << 4),
    __GLSL_USAGE_CLIP_PLANE2_BIT                        = (1 << 5),
    __GLSL_USAGE_CLIP_PLANE3_BIT                        = (1 << 6),
    __GLSL_USAGE_CLIP_PLANE4_BIT                        = (1 << 7),
    __GLSL_USAGE_CLIP_PLANE5_BIT                        = (1 << 8),

    __GLSL_USAGE_POINT_SIZE_BIT                         = (1 << 9),
    __GLSL_USAGE_POINT_SIZE_MIN_BIT                     = (1 << 10),
    __GLSL_USAGE_POINT_SIZE_MAX_BIT                     = (1 << 11),
    __GLSL_USAGE_POINT_FADE_THRESHOLD_SIZE_BIT          = (1 << 12),
    __GLSL_USAGE_POINT_DISTANCE_CONST_ATTENU_BIT        = (1 << 13),
    __GLSL_USAGE_POINT_DISTANCE_LINEAR_ATTENU_BIT       = (1 << 14),
    __GLSL_USAGE_POINT_DISTANCE_QUADRATIC_ATTENU_BIT    = (1 << 15),

    __GLSL_USAGE_FOG_COLOR_BIT                          = (1 << 16),
    __GLSL_USAGE_FOG_DENSITY_BIT                        = (1 << 17),
    __GLSL_USAGE_FOG_START_BIT                          = (1 << 18),
    __GLSL_USAGE_FOG_END_BIT                            = (1 << 19),
    __GLSL_USAGE_FOG_SCALE_BIT                          = (1 << 20),
};

#define __GLSL_USAGE_DEPTHRANGE_BITS        ( __GLSL_USAGE_DEPTHRANGE_NEAR_BIT |\
                                              __GLSL_USAGE_DEPTHRANGE_FAR_BIT |\
                                              __GLSL_USAGE_DEPTHRANGE_DIFF_BIT )

#define __GLSL_USAGE_FOG_BITS   ( __GLSL_USAGE_FOG_COLOR_BIT |\
                                  __GLSL_USAGE_FOG_DENSITY_BIT |\
                                  __GLSL_USAGE_FOG_START_BIT |\
                                  __GLSL_USAGE_FOG_END_BIT |\
                                  __GLSL_USAGE_FOG_SCALE_BIT )

#define __GLSL_USAGE_CLIP_BITS  ( __GLSL_USAGE_CLIP_PLANE0_BIT |\
                                  __GLSL_USAGE_CLIP_PLANE1_BIT |\
                                  __GLSL_USAGE_CLIP_PLANE2_BIT |\
                                  __GLSL_USAGE_CLIP_PLANE3_BIT |\
                                  __GLSL_USAGE_CLIP_PLANE4_BIT |\
                                  __GLSL_USAGE_CLIP_PLANE5_BIT )

#define __GLSL_USAGE_POINT_BITS ( __GLSL_USAGE_POINT_SIZE_BIT |\
                                  __GLSL_USAGE_POINT_SIZE_MIN_BIT |\
                                  __GLSL_USAGE_POINT_SIZE_MAX_BIT |\
                                  __GLSL_USAGE_POINT_FADE_THRESHOLD_SIZE_BIT |\
                                  __GLSL_USAGE_POINT_DISTANCE_CONST_ATTENU_BIT |\
                                  __GLSL_USAGE_POINT_DISTANCE_LINEAR_ATTENU_BIT |\
                                  __GLSL_USAGE_POINT_DISTANCE_QUADRATIC_ATTENU_BIT )



/* Usage bits in globalAttrState[__GLSL_USAGE_MISC_ATTRS2] */
enum {
    /*
    * MaterialParam 5bits x 2
    * LightModelParam 1bit
    * LightModelProduct 1bit x 2
    */
    __GLSL_USAGE_FRONT_MATERIAL_EMISSION_BIT                = (1 << 0),
    __GLSL_USAGE_FRONT_MATERIAL_AMBIENT_BIT                 = (1 << 1),
    __GLSL_USAGE_FRONT_MATERIAL_DIFFUSE_BIT                 = (1 << 2),
    __GLSL_USAGE_FRONT_MATERIAL_SPECULAR_BIT                = (1 << 3),
    __GLSL_USAGE_FRONT_MATERIAL_SHININESS_BIT               = (1 << 4),

    __GLSL_USAGE_BACK_MATERIAL_EMISSION_BIT                 = (1 << 5),
    __GLSL_USAGE_BACK_MATERIAL_AMBIENT_BIT                  = (1 << 6),
    __GLSL_USAGE_BACK_MATERIAL_DIFFUSE_BIT                  = (1 << 7),
    __GLSL_USAGE_BACK_MATERIAL_SPECULAR_BIT                 = (1 << 8),
    __GLSL_USAGE_BACK_MATERIAL_SHININESS_BIT                = (1 << 9),

    __GLSL_USAGE_LIGHTMODEL_AMBIENT_BIT                     = (1 << 10),

    __GLSL_USAGE_FRONT_LIGHTMODEL_PRODUCT_SCENECOLOR_BIT    = (1 << 11),
    __GLSL_USAGE_BACK_LIGHTMODEL_PRODUCT_SCENECOLOR_BIT     = (1 << 12),
};

#define __GLSL_USAGE_FRONT_MATERIAL_BITS    ( __GLSL_USAGE_FRONT_MATERIAL_EMISSION_BIT |\
                                              __GLSL_USAGE_FRONT_MATERIAL_AMBIENT_BIT |\
                                              __GLSL_USAGE_FRONT_MATERIAL_DIFFUSE_BIT |\
                                              __GLSL_USAGE_FRONT_MATERIAL_SPECULAR_BIT |\
                                              __GLSL_USAGE_FRONT_MATERIAL_SHININESS_BIT )

#define __GLSL_USAGE_BACK_MATERIAL_BITS     ( __GLSL_USAGE_BACK_MATERIAL_EMISSION_BIT |\
                                              __GLSL_USAGE_BACK_MATERIAL_AMBIENT_BIT |\
                                              __GLSL_USAGE_BACK_MATERIAL_DIFFUSE_BIT |\
                                              __GLSL_USAGE_BACK_MATERIAL_SPECULAR_BIT |\
                                              __GLSL_USAGE_BACK_MATERIAL_SHININESS_BIT)

#define __GLSL_USAGE_LIGHTMODEL_BITS        ( __GLSL_USAGE_LIGHTMODEL_AMBIENT_BIT |\
                                              __GLSL_USAGE_FRONT_LIGHTMODEL_PRODUCT_SCENECOLOR_BIT |\
                                              __GLSL_USAGE_BACK_LIGHTMODEL_PRODUCT_SCENECOLOR_BIT )

/* Index for built-in state mapping table */
enum {
    __GLSL_STATE_LIGHT_AMBIENT_OFFSET                   = 0,
    __GLSL_STATE_LIGHT_DIFFUSE_OFFSET                   = 1,
    __GLSL_STATE_LIGHT_SPECULAR_OFFSET                  = 2,
    __GLSL_STATE_LIGHT_POSITION_OFFSET                  = 3,
    __GLSL_STATE_LIGHT_HALF_VECTOR_OFFSET               = 4,
    __GLSL_STATE_LIGHT_SPOT_DIRECTION_OFFSET            = 5,
    __GLSL_STATE_LIGHT_SPOT_EXPONENT_OFFSET             = 6,
    __GLSL_STATE_LIGHT_SPOT_CUTOFF_OFFSET               = 7,
    __GLSL_STATE_LIGHT_SPOT_COS_CUTOFF_OFFSET           = 8,
    __GLSL_STATE_LIGHT_CONST_ATTENU_OFFSET              = 9,
    __GLSL_STATE_LIGHT_LINEAR_ATTENU_OFFSET             = 10,
    __GLSL_STATE_LIGHT_QUADRATIC_ATTENU_OFFSET          = 11,

    __GLSL_STATE_FRONT_LIGHT_PRODUCT_AMBIENT_OFFSET     = 12,
    __GLSL_STATE_FRONT_LIGHT_PRODUCT_DIFFUSE_OFFSET     = 13,
    __GLSL_STATE_FRONT_LIGHT_PRODUCT_SPECULAR_OFFSET    = 14,

    __GLSL_STATE_BACK_LIGHT_PRODUCT_AMBIENT_OFFSET      = 15,
    __GLSL_STATE_BACK_LIGHT_PRODUCT_DIFFUSE_OFFSET      = 16,
    __GLSL_STATE_BACK_LIGHT_PRODUCT_SPECULAR_OFFSET     = 17,

    __GLSL_STATE_LIGHT_OFFSET_COUNT,
};

enum{
    __GLSL_STATE_TEXENV_COLOR_OFFSET                = 0,

    __GLSL_STATE_TEXGEN_EYE_PLANE_S_OFFSET          = 1,
    __GLSL_STATE_TEXGEN_EYE_PLANE_T_OFFSET          = 2,
    __GLSL_STATE_TEXGEN_EYE_PLANE_R_OFFSET          = 3,
    __GLSL_STATE_TEXGEN_EYE_PLANE_Q_OFFSET          = 4,
    __GLSL_STATE_TEXGEN_OBJECT_PLANE_S_OFFSET       = 5,
    __GLSL_STATE_TEXGEN_OBJECT_PLANE_T_OFFSET       = 6,
    __GLSL_STATE_TEXGEN_OBJECT_PLANE_R_OFFSET       = 7,
    __GLSL_STATE_TEXGEN_OBJECT_PLANE_Q_OFFSET       = 8,

    __GLSL_STATE_TEXTURE_MATRIX_OFFSET              = 9,
    __GLSL_STATE_TEXTURE_MATRIX_TRANS_OFFSET        = 10,
    __GLSL_STATE_TEXTURE_MATRIX_INV_OFFSET          = 11,
    __GLSL_STATE_TEXTURE_MATRIX_INVTRANS_OFFSET     = 12,

    __GLSL_STATE_TEXTURE_OFFSET_COUNT,
};

/* Transform  states*/
#define __GLSL_STATE_MODELVIEW_INDEX                0
#define __GLSL_STATE_MODELVIEW_TRANS_INDEX          1
#define __GLSL_STATE_MODELVIEW_INV_INDEX            2
#define __GLSL_STATE_MODELVIEW_INVTRANS_INDEX       3

#define __GLSL_STATE_PROJECTION_INDEX               4
#define __GLSL_STATE_PROJECTION_TRANS_INDEX         5
#define __GLSL_STATE_PROJECTION_INV_INDEX           6
#define __GLSL_STATE_PROJECTION_INVTRANS_INDEX      7

#define __GLSL_STATE_MVP_INDEX                      8
#define __GLSL_STATE_MVP_TRANS_INDEX                9
#define __GLSL_STATE_MVP_INV_INDEX                  10
#define __GLSL_STATE_MVP_INVTRANS_INDEX             11

#define __GLSL_STATE_NORMAL_MATRIX_INDEX            12
#define __GLSL_STATE_NORMAL_SCALE_INDEX             13

/* Light States*/
#define __GLSL_STATE_LIGHT0_BASE_INDEX      (__GLSL_STATE_NORMAL_SCALE_INDEX + 1)
#define __GLSL_STATE_LIGHT1_BASE_INDEX      (__GLSL_STATE_LIGHT0_BASE_INDEX + __GLSL_STATE_LIGHT_OFFSET_COUNT)
#define __GLSL_STATE_LIGHT2_BASE_INDEX      (__GLSL_STATE_LIGHT1_BASE_INDEX + __GLSL_STATE_LIGHT_OFFSET_COUNT)
#define __GLSL_STATE_LIGHT3_BASE_INDEX      (__GLSL_STATE_LIGHT2_BASE_INDEX + __GLSL_STATE_LIGHT_OFFSET_COUNT)
#define __GLSL_STATE_LIGHT4_BASE_INDEX      (__GLSL_STATE_LIGHT3_BASE_INDEX + __GLSL_STATE_LIGHT_OFFSET_COUNT)
#define __GLSL_STATE_LIGHT5_BASE_INDEX      (__GLSL_STATE_LIGHT4_BASE_INDEX + __GLSL_STATE_LIGHT_OFFSET_COUNT)
#define __GLSL_STATE_LIGHT6_BASE_INDEX      (__GLSL_STATE_LIGHT5_BASE_INDEX + __GLSL_STATE_LIGHT_OFFSET_COUNT)
#define __GLSL_STATE_LIGHT7_BASE_INDEX      (__GLSL_STATE_LIGHT6_BASE_INDEX + __GLSL_STATE_LIGHT_OFFSET_COUNT)

/* Texture States*/
#define __GLSL_STATE_TEXTURE0_BASE_INDEX    (__GLSL_STATE_LIGHT7_BASE_INDEX + __GLSL_STATE_LIGHT_OFFSET_COUNT)
#define __GLSL_STATE_TEXTURE1_BASE_INDEX    (__GLSL_STATE_TEXTURE0_BASE_INDEX + __GLSL_STATE_TEXTURE_OFFSET_COUNT)
#define __GLSL_STATE_TEXTURE2_BASE_INDEX    (__GLSL_STATE_TEXTURE1_BASE_INDEX + __GLSL_STATE_TEXTURE_OFFSET_COUNT)
#define __GLSL_STATE_TEXTURE3_BASE_INDEX    (__GLSL_STATE_TEXTURE2_BASE_INDEX + __GLSL_STATE_TEXTURE_OFFSET_COUNT)
#define __GLSL_STATE_TEXTURE4_BASE_INDEX    (__GLSL_STATE_TEXTURE3_BASE_INDEX + __GLSL_STATE_TEXTURE_OFFSET_COUNT)
#define __GLSL_STATE_TEXTURE5_BASE_INDEX    (__GLSL_STATE_TEXTURE4_BASE_INDEX + __GLSL_STATE_TEXTURE_OFFSET_COUNT)
#define __GLSL_STATE_TEXTURE6_BASE_INDEX    (__GLSL_STATE_TEXTURE5_BASE_INDEX + __GLSL_STATE_TEXTURE_OFFSET_COUNT)
#define __GLSL_STATE_TEXTURE7_BASE_INDEX    (__GLSL_STATE_TEXTURE6_BASE_INDEX + __GLSL_STATE_TEXTURE_OFFSET_COUNT)

/* Misc States 1 */
#define __GLSL_STATE_MISC1_BASE_INDEX       (__GLSL_STATE_TEXTURE7_BASE_INDEX + __GLSL_STATE_TEXTURE_OFFSET_COUNT)
#define __GLSL_STATE_DEPTHRANGE_NEAR_INDEX  (__GLSL_STATE_MISC1_BASE_INDEX + 0)
#define __GLSL_STATE_DEPTHRANGE_FAR_INDEX   (__GLSL_STATE_MISC1_BASE_INDEX + 1)
#define __GLSL_STATE_DEPTHRANGE_DIFF_INDEX  (__GLSL_STATE_MISC1_BASE_INDEX + 2)

#define __GLSL_STATE_CLIP_PLANE0_INDEX      (__GLSL_STATE_MISC1_BASE_INDEX + 3)
#define __GLSL_STATE_CLIP_PLANE1_INDEX      (__GLSL_STATE_MISC1_BASE_INDEX + 4)
#define __GLSL_STATE_CLIP_PLANE2_INDEX      (__GLSL_STATE_MISC1_BASE_INDEX + 5)
#define __GLSL_STATE_CLIP_PLANE3_INDEX      (__GLSL_STATE_MISC1_BASE_INDEX + 6)
#define __GLSL_STATE_CLIP_PLANE4_INDEX      (__GLSL_STATE_MISC1_BASE_INDEX + 7)
#define __GLSL_STATE_CLIP_PLANE5_INDEX      (__GLSL_STATE_MISC1_BASE_INDEX + 8)

#define __GLSL_STATE_POINT_SIZE_INDEX                       (__GLSL_STATE_MISC1_BASE_INDEX + 9)
#define __GLSL_STATE_POINT_SIZE_MIN_INDEX                   (__GLSL_STATE_MISC1_BASE_INDEX + 10)
#define __GLSL_STATE_POINT_SIZE_MAX_INDEX                   (__GLSL_STATE_MISC1_BASE_INDEX + 11)
#define __GLSL_STATE_POINT_FADE_THRESHOLD_SIZE_INDEX        (__GLSL_STATE_MISC1_BASE_INDEX + 12)
#define __GLSL_STATE_POINT_DISTANCE_CONST_ATTENU_INDEX      (__GLSL_STATE_MISC1_BASE_INDEX + 13)
#define __GLSL_STATE_POINT_DISTANCE_LINEAR_ATTENU_INDEX     (__GLSL_STATE_MISC1_BASE_INDEX + 14)
#define __GLSL_STATE_POINT_DISTANCE_QUADRATIC_ATTENU_INDEX  (__GLSL_STATE_MISC1_BASE_INDEX + 15)

#define __GLSL_STATE_FOG_COLOR_INDEX        (__GLSL_STATE_MISC1_BASE_INDEX + 16)
#define __GLSL_STATE_FOG_DENSITY_INDEX      (__GLSL_STATE_MISC1_BASE_INDEX + 17)
#define __GLSL_STATE_FOG_START_INDEX        (__GLSL_STATE_MISC1_BASE_INDEX + 18)
#define __GLSL_STATE_FOG_END_INDEX          (__GLSL_STATE_MISC1_BASE_INDEX + 19)
#define __GLSL_STATE_FOG_SCALE_INDEX        (__GLSL_STATE_MISC1_BASE_INDEX + 20)


/* Misc states 2 */
#define __GLSL_STATE_MISC2_BASE_INDEX   (__GLSL_STATE_FOG_SCALE_INDEX + 1)

#define __GLSL_STATE_FRONT_MATERIAL_EMISSION_INDEX  (__GLSL_STATE_MISC2_BASE_INDEX + 0)
#define __GLSL_STATE_FRONT_MATERIAL_AMBIENT_INDEX   (__GLSL_STATE_MISC2_BASE_INDEX + 1)
#define __GLSL_STATE_FRONT_MATERIAL_DIFFUSE_INDEX   (__GLSL_STATE_MISC2_BASE_INDEX + 2)
#define __GLSL_STATE_FRONT_MATERIAL_SPECULAR_INDEX  (__GLSL_STATE_MISC2_BASE_INDEX + 3)
#define __GLSL_STATE_FRONT_MATERIAL_SHININESS_INDEX (__GLSL_STATE_MISC2_BASE_INDEX + 4)

#define __GLSL_STATE_BACK_MATERIAL_EMISSION_INDEX   (__GLSL_STATE_MISC2_BASE_INDEX + 5)
#define __GLSL_STATE_BACK_MATERIAL_AMBIENT_INDEX    (__GLSL_STATE_MISC2_BASE_INDEX + 6)
#define __GLSL_STATE_BACK_MATERIAL_DIFFUSE_INDEX    (__GLSL_STATE_MISC2_BASE_INDEX + 7)
#define __GLSL_STATE_BACK_MATERIAL_SPECULAR_INDEX   (__GLSL_STATE_MISC2_BASE_INDEX + 8)
#define __GLSL_STATE_BACK_MATERIAL_SHININESS_INDEX  (__GLSL_STATE_MISC2_BASE_INDEX + 9)

#define __GLSL_STATE_LIGHTMODEL_AMBIENT_INDEX       (__GLSL_STATE_MISC2_BASE_INDEX + 10)
#define __GLSL_STATE_FRONT_LIGHTMODEL_PRODUCT_SCENECOLOR_INDEX  (__GLSL_STATE_MISC2_BASE_INDEX + 11)
#define __GLSL_STATE_BACK_LIGHTMODEL_PRODUCT_SCENECOLOR_INDEX   (__GLSL_STATE_MISC2_BASE_INDEX + 12)

#define __GLSL_STATE_INDEX_COUNT  (__GLSL_STATE_BACK_LIGHTMODEL_PRODUCT_SCENECOLOR_INDEX + 1)



/* Helper dirty bits for built in state */
#define  __GLSL_STATE_BITS  ( (1 << __GL_DIRTY_ATTRS_1) |\
                              (1 << __GL_DIRTY_ATTRS_2) |\
                              (1 << __GL_DIRTY_ATTRS_3) |\
                              (1 << __GL_LIGHTING_ATTRS) |\
                              (1 << __GL_LIGHT_SRC_ATTRS) |\
                              (1 << __GL_CLIP_ATTRS) |\
                              (1 << __GL_TEX_UNIT_ATTRS) )

/* State group 1 */
#define __GLSL_STATE_GROUP1_BITS  __GL_DEPTHRANGE_BIT


/* State group 2 */
#define __GLSL_STATE_FOG_BITS  ( __GL_FOGCOLOR_BIT |\
                                 __GL_FOGDENSITY_BIT |\
                                 __GL_FOGSTART_BIT |\
                                 __GL_FOGEND_BIT )

#define __GLSL_STATE_POINT_BITS  ( __GL_POINTSIZE_BIT | \
                __GL_POINT_SIZE_MIN_BIT | \
                __GL_POINT_SIZE_MAX_BIT | \
                __GL_POINT_FADE_THRESHOLD_SIZE_BIT | \
                __GL_POINT_DISTANCE_ATTENUATION_BIT)

#define __GLSL_STATE_GROUP2_BITS  ( __GLSL_STATE_FOG_BITS | __GLSL_STATE_POINT_BITS)


/* State group 3 */
#define __GLSL_STATE_GROUP3_BITS ( __GL_MODELVIEW_TRANSFORM_BIT |\
                                    __GL_PROJECTION_TRANSFORM_BIT )


/* State lighting */
#define __GLSL_STATE_FRONT_MATERIAL_BITS ( \
                __GL_MATERIAL_EMISSION_FRONT_BIT | \
                __GL_MATERIAL_SPECULAR_FRONT_BIT | \
                __GL_MATERIAL_SHININESS_FRONT_BIT | \
                __GL_MATERIAL_AMBIENT_FRONT_BIT | \
                __GL_MATERIAL_DIFFUSE_FRONT_BIT)

#define __GLSL_STATE_BACK_MATERIAL_BITS ( \
                __GL_MATERIAL_EMISSION_BACK_BIT | \
                __GL_MATERIAL_SPECULAR_BACK_BIT | \
                __GL_MATERIAL_SHININESS_BACK_BIT | \
                __GL_MATERIAL_AMBIENT_BACK_BIT | \
                __GL_MATERIAL_DIFFUSE_BACK_BIT)


#define __GLSL_STATE_LIGHTING_BITS  ( __GL_LIGHTMODEL_AMBIENT_BIT |\
                                      __GLSL_STATE_FRONT_MATERIAL_BITS |\
                                      __GLSL_STATE_BACK_MATERIAL_BITS )

/* State light src */
#define __GLSL_STATE_LIGHTSRC_BITS  ( __GL_LIGHT_AMBIENT_BIT |\
                                      __GL_LIGHT_DIFFUSE_BIT |\
                                      __GL_LIGHT_SPECULAR_BIT |\
                                      __GL_LIGHT_POSITION_BIT |\
                                      __GL_LIGHT_CONSTANTATT_BIT |\
                                      __GL_LIGHT_LINEARATT_BIT |\
                                      __GL_LIGHT_QUADRATICATT_BIT |\
                                      __GL_LIGHT_SPOTDIRECTION_BIT |\
                                      __GL_LIGHT_SPOTEXPONENT_BIT |\
                                      __GL_LIGHT_SPOTCUTOFF_BIT )

/* State clip */
#define __GLSL_STATE_CLIP_BITS  ( __GL_CLIPPLANE0_BIT |\
                                  __GL_CLIPPLANE1_BIT |\
                                  __GL_CLIPPLANE2_BIT |\
                                  __GL_CLIPPLANE3_BIT |\
                                  __GL_CLIPPLANE4_BIT |\
                                  __GL_CLIPPLANE5_BIT )

#define __GLSL_STATE_TEX_BITS   ( __GL_TEXTURE_TRANSFORM_BIT |\
                                  __GL_TEXGEN_S_BIT |\
                                  __GL_TEXGEN_T_BIT |\
                                  __GL_TEXGEN_R_BIT |\
                                  __GL_TEXGEN_Q_BIT |\
                                  __GL_TEXENV_COLOR_BIT )

#define __GLSL_STATE_TEXGEN_BITS    ( __GL_TEXGEN_S_BIT |\
                                      __GL_TEXGEN_T_BIT |\
                                      __GL_TEXGEN_R_BIT |\
                                      __GL_TEXGEN_Q_BIT )

/* State delay */
enum {
    __GLSL_DELAY_FOG_SCALE_BIT                          = (1 << 0),
    __GLSL_DELAY_MVP_BIT                                = (1 << 1),
    __GLSL_DELAY_FRONT_LIGHTMODEL_PRODUCT_SCENECOLOR    = (1 << 2),
    __GLSL_DELAY_BACK_LIGHTMODEL_PRODUCT_SCENECOLOR     = (1 << 3),
    __GLSL_DELAY_LIGHT0                                 = (1 << 4),
    __GLSL_DELAY_LIGHT1                                 = (1 << 5),
    __GLSL_DELAY_LIGHT2                                 = (1 << 6),
    __GLSL_DELAY_LIGHT3                                 = (1 << 7),
    __GLSL_DELAY_LIGHT4                                 = (1 << 8),
    __GLSL_DELAY_LIGHT5                                 = (1 << 9),
    __GLSL_DELAY_LIGHT6                                 = (1 << 10),
    __GLSL_DELAY_LIGHT7                                 = (1 << 11),
};

enum {
    __GLSL_DELAY_FRONT_LIGHT_PRODUCT_AMBIENT    = (1 << 0),
    __GLSL_DELAY_FRONT_LIGHT_PRODUCT_DIFFUSE    = (1 << 1),
    __GLSL_DELAY_FRONT_LIGHT_PRODUCT_SPECULAR   = (1 << 2),

    __GLSL_DELAY_BACK_LIGHT_PRODUCT_AMBIENT     = (1 << 3),
    __GLSL_DELAY_BACK_LIGHT_PRODUCT_DIFFUSE     = (1 << 4),
    __GLSL_DELAY_BACK_LIGHT_PRODUCT_SPECULAR    = (1 << 5),
};

#define __GLSL_DELAY_LIGHT_BITS     ( __GLSL_DELAY_LIGHT0 |\
                                      __GLSL_DELAY_LIGHT1 |\
                                      __GLSL_DELAY_LIGHT2 |\
                                      __GLSL_DELAY_LIGHT3 |\
                                      __GLSL_DELAY_LIGHT4 |\
                                      __GLSL_DELAY_LIGHT5 |\
                                      __GLSL_DELAY_LIGHT6 |\
                                      __GLSL_DELAY_LIGHT7 )

#define __GLSL_DELAY_FRONT_LIGHT_PRODUCT_BITS   ( __GLSL_DELAY_FRONT_LIGHT_PRODUCT_AMBIENT |\
                                                  __GLSL_DELAY_FRONT_LIGHT_PRODUCT_DIFFUSE |\
                                                  __GLSL_DELAY_FRONT_LIGHT_PRODUCT_SPECULAR )

#define __GLSL_DELAY_BACK_LIGHT_PRODUCT_BITS    ( __GLSL_DELAY_BACK_LIGHT_PRODUCT_AMBIENT |\
                                                  __GLSL_DELAY_BACK_LIGHT_PRODUCT_DIFFUSE |\
                                                  __GLSL_DELAY_BACK_LIGHT_PRODUCT_SPECULAR )



#define __GLSL_LOG_INFO_SIZE 512
/************************************************************************/
/* Structures                                                           */
/************************************************************************/
typedef struct __GLobjectInfoRec {
    /* indicate how many targets the object is currently bound to
    */
    GLuint bindCount;

    /* The seqNumber is increased by 1 whenever program string is changed.
    ** DP must recompile the program string if its internal copy of
    ** savedSeqNum is different from this seqNumber.
    */
    GLuint seqNumber;

    /* Internal flag for generic object management. */
    GLbitfield flag;

    GL_SHADER_PROGRAM_TYPE objectType;
    GLuint id;
} __GLobjectInfo;

typedef struct __GLshaderInfoRec {
    /* shader handle used in lower level driver */
    GLvoid * hShader;
    GLuint shaderType;
    GLboolean deleteStatus;
    GLboolean compiledStatus;
    GLubyte * compiledLog;
    GLubyte * source;
    GLuint sourceSize;
} __GLshaderInfo;


/* Shader object holds vertex or fragment shader related information */
typedef struct __GLshaderObjectRec {
    /* Generic object information */
    __GLobjectInfo objectInfo;
    __GLshaderInfo shaderInfo;

    /* program object count which this shader is attached */
    GLuint attachCount;

    /* This is the object privateData that can be shared by different contexts
    ** that are bound to the object.
    */
    GLvoid *privateData;

    /* These are vertex shader only states */
    GLboolean twoSidedColorEnabled;
    GLboolean pointSizeEnabled;
} __GLshaderObject;

typedef struct __GLunit2SamplerRec{
    GLuint mappedSampler[__GL_MAX_GLSL_SAMPLERS];
    GLuint numSamplers;
}__GLunit2Sampler;


#define GLSL_INVALID_LINK_BIT   0x00000001
#define GLSL_INVALID_CODE_BIT   0x00000002
#define GLSL_INVALID_TEX_BIT    0x00000004

typedef struct __GLshaderProgramInfoRec {
    GLboolean deletedStatus;
    GLboolean linkedStatus;
    char * infoLog;

    // GLboolean validatedStatus;
    GLuint invalidFlag;

    GLuint attachedShadersTableSize;
    GLuint count;
    __GLshaderObject **attachedShaders;

    /* set when link program */
    GLboolean vertShaderEnable;
    GLboolean geomShaderEnable;
    GLboolean fragShaderEnable;

    /* states of geometry shader */
    GLuint geomVerticesOut;
    GLenum geomInputType;
    GLenum geomOutputType;

    /* output type of current link */
    GLenum geomRealOutputType;

    GLuint codeSeq;     /* used for object share */
} __GLshaderProgramInfo;

typedef struct __GLSLBuiltInStateUsage{
    GLbitfield globalAttrState[__GLSL_USAGE_END];
    GLbitfield lightAttrState[__GL_MAX_LIGHT_NUMBER];
    GLbitfield texAttrState[__GL_MAX_TEXTURE_UNITS];
}__GLSLBuiltInStateUsage;

typedef struct __GLbindingInfoRec{

    /* Uniform */
    GLuint                  numUserUnifrom;

    /* Sampler */
    struct {
        GLuint sampler2TexUnit[__GL_MAX_GLSL_SAMPLERS];

        GLuint vsTexEnableDim[__GL_MAX_TEXTURE_UNITS];
        GLuint gsTexEnableDim[__GL_MAX_TEXTURE_UNITS];
        GLuint psTexEnableDim[__GL_MAX_TEXTURE_UNITS];

        GLuint64 samplerSeq;

        GLboolean texConflict;
    };


    /* info integrated of Uniform & Sampler */
    GLuint numActiveUniform;        /* number of active uniform */
    GLuint maxActiveUniformLength;  /* max name length */

    /* Attribute */
    GLuint numActiveAttrib;
    GLuint maxActiveAttribNameLength;

    struct {
        /* vs input mask will be built from output attribute table of GLSL compiler */
        GLuint vsInputMask;
        /* ps input mask will be built from output ps varing table of GLSL compiler */
        GLuint psInputMask;
    };

    /* Varing */
    struct {
        GLuint fragmentVaringOutTableSize;
        GLvoid *pFragmentVaringOutTable;
    };
}__GLbindingInfo;

/* Program object holds program relted information */
typedef struct __GLshaderProgramObjectRec {
    /* Generic object information */
    __GLobjectInfo objectInfo;

    /* Define the program related states information */
    __GLshaderProgramInfo programInfo;

    /* compiler output binding tables of the program */
    __GLbindingInfo bindingInfo;

    /* This is the object privateData that can be shared by different contexts
    ** that are bound to the object.
    */
    GLvoid *privateData;

} __GLshaderProgramObject;

typedef struct __GLshaderProgramMachineRec {
    __GLsharedObjectMachine *shared;

    /* Current executable program object */
    __GLshaderProgramObject *currentShaderProgram;

    GLboolean vertShaderEnable;     /* current enable */
    GLboolean geomShaderEnable;
    GLboolean fragShaderEnable;

    GLboolean vertShaderRealEnable; /* real enable */
    GLboolean geomShaderRealEnable;
    GLboolean fragShaderRealEnable;

    GLenum geomOutputType;          /* primitive type if gs enable */

    /* Dirty flag to indicate which sampler change. Used when validate samplers.
    ** If there's no program switch, it's just a copy of the dirtyState in programObject.
    ** If there's program switch, current */
    GLuint64 samplerDirtyState;

    /* when two or more contexts share the same program object, used for setting
    ** sampler dirty bits in each context.
    */
    GLuint64 samplerSeq;

    /* record commit state for validate */
    GLuint lastProgram;
    GLuint lastCodeSeq;
    GLboolean lastVertShaderEnable;
    GLboolean lastGeomShaderEnable;
    GLboolean lastFragShaderEnable;
    GLuint prevSampler2TexUnit[__GL_MAX_GLSL_SAMPLERS];
} __GLshaderProgramMachine;


#define __GL_SYMBOL_TYPE_VARIANT            0x20000000
#define __GL_SYMBOL_TYPE_INVARIANT          0x40000000
#define __GL_SYMBOL_TYPE_LOCALCONST         0x60000000
#define __GL_SYMBOL_TYPE_LOCAL              0x80000000

#define __GL_SYMBOL_TYPE_STATEVARIANT       0xA0000000
#define __GL_SYMBOL_TYPE_STATEINVARIANT     0xC0000000

#define __GL_SYMBOL_TYPE_OUTPUT             0xE0000000

#define __GL_SYMBOL_TYPE_SHIFT            29
#define __GL_SYMBOL_TYPE_MASK           0xE0000000
#define __GL_SYMBOL_INDEX_MASK          0x1FFFFFFF

/*
*** symbol definitions
*/
typedef struct __GLsymbolRec {
    GLuint dataType:2;
    GLuint storageType:3;
    GLuint normalized:1;
    GLuint readSwizzle:8;
    GLuint writeMask:4;
    GLuint size:3;
    GLuint used:1;
    GLuint regIndex:10;
#if defined(USE_LENDIAN)
    union{
        GLuint symbolCode;
        struct{
            GLushort virtualIndex;
            GLushort mask;
        };
    };
#else
    union{
        GLuint symbolCode;
        struct{
            GLushort mask;
            GLushort virtualIndex;
        };
    };
#endif

    GLushort dirtyBitOffset;
    GLubyte dirtyEntryOffset;
    GLubyte forwardCount;
    GLuint unit;
}__GLsymbol;

#endif /* __gc_gl_shader_h_ */

