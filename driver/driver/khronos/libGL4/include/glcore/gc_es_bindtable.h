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


#ifndef __gc_es_bindtable_h__
#define __gc_es_bindtable_h__

/* Stage */
typedef enum __GLSLStageRec
{
    __GLSL_STAGE_INVALID = -1,
    __GLSL_STAGE_VS   = 0,
    __GLSL_STAGE_TCS,
    __GLSL_STAGE_TES,
    __GLSL_STAGE_GS,
    __GLSL_STAGE_FS,
    __GLSL_STAGE_CS,

    __GLSL_STAGE_LAST
} __GLSLStage;


/* Data type */
typedef enum __GLSLDataTypeRec
{
    __GLSL_DATATYPE_INT = 0,
    __GLSL_DATATYPE_INT_VEC2,
    __GLSL_DATATYPE_INT_VEC3,
    __GLSL_DATATYPE_INT_VEC4,
    __GLSL_DATATYPE_BOOL,
    __GLSL_DATATYPE_BOOL_VEC2,
    __GLSL_DATATYPE_BOOL_VEC3,
    __GLSL_DATATYPE_BOOL_VEC4,
    __GLSL_DATATYPE_FLOAT,
    __GLSL_DATATYPE_FLOAT_VEC2,
    __GLSL_DATATYPE_FLOAT_VEC3,
    __GLSL_DATATYPE_FLOAT_VEC4,
    __GLSL_DATATYPE_FLOAT_MAT2,
    __GLSL_DATATYPE_FLOAT_MAT3,
    __GLSL_DATATYPE_FLOAT_MAT4,
    __GLSL_DATATYPE_FLOAT_MAT2X3,
    __GLSL_DATATYPE_FLOAT_MAT2X4,
    __GLSL_DATATYPE_FLOAT_MAT3X2,
    __GLSL_DATATYPE_FLOAT_MAT3X4,
    __GLSL_DATATYPE_FLOAT_MAT4X2,
    __GLSL_DATATYPE_FLOAT_MAT4X3,

    __GLSL_DATATYPE_UNSIGNED_INT,
    __GLSL_DATATYPE_UNSIGNED_INT_VEC2,
    __GLSL_DATATYPE_UNSIGNED_INT_VEC3,
    __GLSL_DATATYPE_UNSIGNED_INT_VEC4,

    __GLSL_DATATYPE_SAMPLER_2D,
    __GLSL_DATATYPE_SAMPLER_3D,
    __GLSL_DATATYPE_SAMPLER_CUBE,
    __GLSL_DATATYPE_SAMPLER_2D_SHADOW,

    __GLSL_DATATYPE_SAMPLER_2D_ARRAY,
    __GLSL_DATATYPE_SAMPLER_2D_ARRAY_SHADOW,
    __GLSL_DATATYPE_SAMPLER_CUBE_SHADOW,

    __GLSL_DATATYPE_INT_SAMPLER_2D,
    __GLSL_DATATYPE_INT_SAMPLER_3D,
    __GLSL_DATATYPE_INT_SAMPLER_CUBE,
    __GLSL_DATATYPE_INT_SAMPLER_2D_ARRAY,

    __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_2D,
    __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_3D,
    __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_CUBE,
    __GLSL_DATATYPE_UNSIGNED_INT_SAMPLER_2D_ARRAY,

    __GLSL_DATATYPE_STRUCTURE,

    __GLSL_DATATYPE_LAST
} __GLSLDataType;

#endif /* __gc_es_bindtable_h__ */
