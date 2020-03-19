/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "gc_es_context.h"

extern GLvoid __gllc_InvalidValue(__GLcontext *gc);
extern GLvoid __gllc_InvalidEnum(__GLcontext *gc);
extern GLvoid __gllc_InvalidOperation(__GLcontext *gc);

/*
*** Fog parameter size
*/
GLint __glFog_size(GLenum pname)
{
    switch (pname) {
      case GL_FOG_COLOR:    return 4;
      case GL_FOG_DENSITY:  return 1;
      case GL_FOG_END:      return 1;
      case GL_FOG_MODE:     return 1;
      case GL_FOG_INDEX:    return 1;
      case GL_FOG_START:    return 1;
      case GL_FOG_COORD_SRC:return 1;
      default:
        return -1;
    }
}

/*
*** Light parameter size
*/
GLint __glLight_size(GLenum pname)
{
    switch (pname) {
      case GL_SPOT_EXPONENT:        return 1;
      case GL_SPOT_CUTOFF:      return 1;
      case GL_AMBIENT:          return 4;
      case GL_DIFFUSE:          return 4;
      case GL_SPECULAR:         return 4;
      case GL_POSITION:         return 4;
      case GL_SPOT_DIRECTION:       return 3;
      case GL_CONSTANT_ATTENUATION: return 1;
      case GL_LINEAR_ATTENUATION:   return 1;
      case GL_QUADRATIC_ATTENUATION:    return 1;
      default:
        return -1;
    }
}

GLint __glLightModel_size(GLenum pname)
{
    switch (pname) {
      case GL_LIGHT_MODEL_LOCAL_VIEWER:     return 1;
      case GL_LIGHT_MODEL_TWO_SIDE:     return 1;
      case GL_LIGHT_MODEL_COLOR_CONTROL: return 1;
      case GL_LIGHT_MODEL_AMBIENT: return 4;
      default:
        return -1;
    }
}

/*
*** Material parameter size
*/
GLint __glMaterial_size(GLenum pname)
{
    switch (pname) {
      case GL_SHININESS:        return 1;
      case GL_EMISSION:         return 4;
      case GL_AMBIENT:          return 4;
      case GL_DIFFUSE:          return 4;
      case GL_SPECULAR:         return 4;
      case GL_AMBIENT_AND_DIFFUSE:  return 4;
      case GL_COLOR_INDEXES:        return 3;
      default:
        return -1;
    }
}

/*
*** Utility functions to calculate texture related state parameter size.
*/
GLint __glTexParameter_size(GLenum e)
{
    switch (e)
    {
    case GL_TEXTURE_WRAP_S:
    case GL_TEXTURE_WRAP_T:
    case GL_TEXTURE_WRAP_R:
    case GL_TEXTURE_MIN_FILTER:
    case GL_TEXTURE_MAG_FILTER:
    case GL_TEXTURE_PRIORITY:
    case GL_TEXTURE_MIN_LOD:
    case GL_TEXTURE_MAX_LOD:
    case GL_TEXTURE_LOD_BIAS:
    case GL_TEXTURE_BASE_LEVEL:
    case GL_TEXTURE_MAX_LEVEL:
    case GL_GENERATE_MIPMAP_SGIS:
    case GL_DEPTH_TEXTURE_MODE_ARB:
    case GL_TEXTURE_COMPARE_MODE_ARB:
    case GL_TEXTURE_COMPARE_FUNC_ARB:
    case GL_TEXTURE_COMPARE_FAIL_VALUE_ARB:
    case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        return(1);
    case GL_TEXTURE_BORDER_COLOR:
        return(4);
    default:
        return(-1);
    }
}

GLint __glTexEnv_size(GLenum e)
{
    switch (e)
    {
    case GL_TEXTURE_ENV_MODE:
    case GL_COMBINE_RGB:
    case GL_COMBINE_ALPHA:
    case GL_SOURCE0_RGB:
    case GL_SOURCE1_RGB:
    case GL_SOURCE2_RGB:
    case GL_SOURCE0_ALPHA:
    case GL_SOURCE1_ALPHA:
    case GL_SOURCE2_ALPHA:
    case GL_OPERAND0_RGB:
    case GL_OPERAND1_RGB:
    case GL_OPERAND2_RGB:
    case GL_OPERAND0_ALPHA:
    case GL_OPERAND1_ALPHA:
    case GL_OPERAND2_ALPHA:
    case GL_RGB_SCALE:
    case GL_ALPHA_SCALE:
        return(1);
    case GL_TEXTURE_ENV_COLOR:
        return(4);
    default:
        return(-1);
    }
}

GLint __glTexGen_size(GLenum e)
{
    switch (e)
    {
    case GL_TEXTURE_GEN_MODE:
        return(1);
    case GL_OBJECT_PLANE:
    case GL_EYE_PLANE:
        return(4);
    default:
        return(-1);
    }
}

GLint __glPointParameter_size(GLenum pname)
{
    switch (pname) {
    case GL_POINT_SIZE_MIN_EXT:
    case GL_POINT_SIZE_MAX_EXT:
    case GL_POINT_FADE_THRESHOLD_SIZE_EXT:
    case GL_POINT_SPRITE_COORD_ORIGIN:
        return 1;
    case GL_DISTANCE_ATTENUATION_EXT:
        return 3;
    default:
        return -1;
    }
}

/*
*** get info for copy image data from user space to driver space
*** mainly for copying compressed pixle format.
*/
GLboolean __glTexImagCopyInfo(__GLcontext *gc, GLenum format, GLenum type, GLenum *ret_format, GLenum *ret_type)
{
    GLboolean index;

    *ret_format = format;
    *ret_type = type;

    switch (format)
    {
    case GL_COLOR_INDEX:
        index = GL_TRUE;
        break;
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
    case GL_ABGR_EXT:
    case GL_BGR_EXT:
    case GL_BGRA_EXT:
        index = GL_FALSE;
        break;
    default:
        __gllc_InvalidEnum(gc);
        return GL_FALSE;
    }

    switch (type)
    {
    case GL_BITMAP:
        if (!index)
        {
            __gllc_InvalidEnum(gc);
            return GL_FALSE;
        }
        break;
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
        break;
    case GL_UNSIGNED_BYTE_3_3_2:
    case GL_UNSIGNED_SHORT_5_6_5:
    case GL_UNSIGNED_BYTE_2_3_3_REV:
    case GL_UNSIGNED_SHORT_5_6_5_REV:
        switch (format)
        {
        case GL_RGB:
        case GL_BGR_EXT:
            *ret_format= GL_LUMINANCE; /* or anything that's 1 component */
            *ret_type= GL_UNSIGNED_BYTE;
            break;
        default:
            __gllc_InvalidOperation(gc);
            return GL_FALSE;
        }
        break;
    case GL_UNSIGNED_SHORT_4_4_4_4:
    case GL_UNSIGNED_SHORT_5_5_5_1:
    case GL_UNSIGNED_INT_8_8_8_8:
    case GL_UNSIGNED_INT_10_10_10_2:
    case GL_UNSIGNED_SHORT_4_4_4_4_REV:
    case GL_UNSIGNED_SHORT_1_5_5_5_REV:
    case GL_UNSIGNED_INT_8_8_8_8_REV:
    case GL_UNSIGNED_INT_2_10_10_10_REV:
        switch (format)
        {
        case GL_RGBA:
        case GL_ABGR_EXT:
        case GL_BGRA_EXT:
            *ret_format= GL_LUMINANCE; /* or anything that's 1 component */
            *ret_type= GL_UNSIGNED_SHORT;
            if (type == GL_UNSIGNED_INT_8_8_8_8 ||
                type == GL_UNSIGNED_INT_10_10_10_2 ||
                type == GL_UNSIGNED_INT_8_8_8_8_REV ||
                type == GL_UNSIGNED_INT_2_10_10_10_REV)
                *ret_type= GL_UNSIGNED_INT;
            break;
        default:
            __gllc_InvalidOperation(gc);
            return GL_FALSE;
        }
        break;
    default:
        __gllc_InvalidEnum(gc);
        return GL_FALSE;
    }
    return GL_TRUE;
}

/*
** Check input parameter of material
*/
GLenum __glErrorCheckMaterial(GLenum face, GLenum p, GLfloat pv0)
{
    switch (face)
    {
    case GL_FRONT:
    case GL_BACK:
    case GL_FRONT_AND_BACK:
        break;
    default:
        return(GL_INVALID_ENUM);
    }
    switch (p)
    {
    case GL_COLOR_INDEXES:
    case GL_EMISSION:
    case GL_SPECULAR:
    case GL_AMBIENT:
    case GL_DIFFUSE:
    case GL_AMBIENT_AND_DIFFUSE:
    case GL_INDEX_OFFSET:
    case GL_INDEX_SHIFT:
        break;
    case GL_SHININESS:
        if (pv0 < 0 || pv0 > 128)
        {
            return(GL_INVALID_VALUE);
        }
        break;
    default:
        return(GL_INVALID_ENUM);
    }
    return(GL_NO_ERROR);
}

