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


#include "gc_glsl_scanner.h"

#define T_RESERVED_KEYWORD        T_EOF
#define T_NOT_KEYWORD            T_IDENTIFIER

#define _MAX_IDENTIFIER_LENGTH_ 1024

typedef struct _slsKEYWORD
{
    gctCONST_STRING    symbol;
    gctINT             token;            /* token for slvEXTENSION_NON_HALTI */
    gctINT             disabled;         /* for extension not enabled */
    gctINT             enabled;          /* for extension enabled */
    sleEXTENSION       extension;
}
slsKEYWORD;

static slsKEYWORD KeywordTable[] =
{
    {"_viv_asm",               T_NOT_KEYWORD, T_NOT_KEYWORD, T_VIV_ASM, slvEXTENSION_VASM },
    {"active",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"asm",                    T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"atomic_uint",            T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_ATOMIC_UINT, slvEXTENSION_ES_31},
    {"attribute",              T_ATTRIBUTE, T_ATTRIBUTE, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"bool",                   T_BOOL, T_BOOL, T_BOOL, slvEXTENSION_HALTI},
    {"break",                  T_BREAK, T_BREAK, T_BREAK, slvEXTENSION_HALTI},
    {"buffer",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_BUFFER, slvEXTENSION_ES_31},
    {"bvec2",                  T_BVEC2, T_BVEC2, T_BVEC2, slvEXTENSION_HALTI},
    {"bvec3",                  T_BVEC3, T_BVEC3, T_BVEC3, slvEXTENSION_HALTI},
    {"bvec4",                  T_BVEC4, T_BVEC4, T_BVEC4, slvEXTENSION_HALTI},
    {"case",                   T_NOT_KEYWORD, T_NOT_KEYWORD, T_CASE, slvEXTENSION_HALTI},
    {"cast",                   T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"centroid",               T_NOT_KEYWORD, T_NOT_KEYWORD, T_CENTROID, slvEXTENSION_HALTI},
    {"class",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"coherent",               T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_COHERENT, slvEXTENSION_ES_31},
    {"common",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"const",                  T_CONST, T_CONST, T_CONST, slvEXTENSION_HALTI},
    {"continue",               T_CONTINUE, T_CONTINUE, T_CONTINUE, slvEXTENSION_HALTI},
    {"default",                T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_DEFAULT, slvEXTENSION_HALTI},
    {"discard",                T_DISCARD, T_DISCARD, T_DISCARD, slvEXTENSION_HALTI},
    {"do",                     T_DO, T_DO, T_DO, slvEXTENSION_HALTI},
    {"double",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"dvec2",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"dvec3",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"dvec4",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"else",                   T_ELSE, T_ELSE, T_ELSE, slvEXTENSION_HALTI},
    {"enum",                   T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"extern",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"external",               T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"filter",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"fixed",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"flat",                   T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_FLAT, slvEXTENSION_HALTI},
    {"float",                  T_FLOAT, T_FLOAT, T_FLOAT, slvEXTENSION_HALTI},
    {"for",                    T_FOR, T_FOR, T_FOR, slvEXTENSION_HALTI},
    {"fvec2",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"fvec3",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"fvec4",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"goto",                   T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"half",                   T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"highp",                  T_HIGH_PRECISION, T_HIGH_PRECISION, T_HIGH_PRECISION, slvEXTENSION_HALTI},
    {"hvec2",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"hvec3",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"hvec4",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"if",                     T_IF, T_IF, T_IF, slvEXTENSION_HALTI},
    {"iimage1D",               T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"iimage1DArray",          T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"iimage2D",               T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IIMAGE2D, slvEXTENSION_ES_31},
    {"iimage2DArray",          T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IIMAGE2DARRAY, slvEXTENSION_ES_31},
    {"iimage3D",               T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IIMAGE3D, slvEXTENSION_ES_31},
    {"iimageBuffer",           T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IIMAGEBUFFER, slvEXTENSION_EXT_TEXTURE_BUFFER},
    {"iimageCube",             T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IIMAGECUBE, slvEXTENSION_ES_31},
    {"iimageCubeArray",        T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IIMAGECUBEARRAY, slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY},
    {"image1D",                T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"image1DArray",           T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"image1DArrayShadow",     T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_NOT_KEYWORD, slvEXTENSION_ES_31},
    {"image1DShadow",          T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_NOT_KEYWORD, slvEXTENSION_ES_31},
    {"image2D",                T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IMAGE2D, slvEXTENSION_ES_31},
    {"image2DArray",           T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IMAGE2DARRAY, slvEXTENSION_ES_31},
    {"image2DArrayShadow",     T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_NOT_KEYWORD, slvEXTENSION_ES_31},
    {"image2DShadow",          T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_NOT_KEYWORD, slvEXTENSION_ES_31},
    {"image3D",                T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IMAGE3D, slvEXTENSION_ES_31},
    {"imageBuffer",            T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IMAGEBUFFER, slvEXTENSION_EXT_TEXTURE_BUFFER},
    {"imageCube",              T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IMAGECUBE, slvEXTENSION_ES_31},
    {"imageCubeArray",         T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_IMAGECUBEARRAY, slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY},
    {"in",                     T_IN, T_IN, T_IN, slvEXTENSION_HALTI},
    {"inline",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"inout",                  T_INOUT, T_INOUT, T_INOUT, slvEXTENSION_HALTI},
    {"input",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"int",                    T_INT, T_INT, T_INT, slvEXTENSION_HALTI},
    {"interface",              T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"invariant",              T_INVARIANT, T_INVARIANT, T_INVARIANT, slvEXTENSION_HALTI},
    {"isampler1D",             T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"isampler1DArray",        T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"isampler2D",             T_NOT_KEYWORD, T_NOT_KEYWORD, T_ISAMPLER2D, slvEXTENSION_HALTI},
    {"isampler2DArray",        T_NOT_KEYWORD, T_NOT_KEYWORD, T_ISAMPLER2DARRAY, slvEXTENSION_HALTI},
    {"isampler2DMS",           T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_ISAMPLER2DMS, slvEXTENSION_ES_31},
    {"isampler2DMSArray",      T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_ISAMPLER2DMSARRAY, slvEXTENSION_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY},
    {"isampler2DRect",         T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"isampler3D",             T_NOT_KEYWORD, T_NOT_KEYWORD, T_ISAMPLER3D, slvEXTENSION_HALTI},
    {"isamplerBuffer",         T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_ISAMPLERBUFFER, slvEXTENSION_EXT_TEXTURE_BUFFER},
    {"isamplerCube",           T_NOT_KEYWORD, T_NOT_KEYWORD, T_ISAMPLERCUBE, slvEXTENSION_HALTI},
    {"isamplerCubeArray",      T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_ISAMPLERCUBEARRAY, slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY},
    {"ivec2",                  T_IVEC2, T_IVEC2, T_IVEC2, slvEXTENSION_HALTI},
    {"ivec3",                  T_IVEC3, T_IVEC3, T_IVEC3, slvEXTENSION_HALTI},
    {"ivec4",                  T_IVEC4, T_IVEC4, T_IVEC4, slvEXTENSION_HALTI},
    {"layout",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_LAYOUT, slvEXTENSION_HALTI},
    {"long",                   T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"lowp",                   T_LOW_PRECISION, T_LOW_PRECISION, T_LOW_PRECISION, slvEXTENSION_HALTI},
    {"mat2",                   T_MAT2, T_MAT2, T_MAT2, slvEXTENSION_HALTI},
    {"mat2x2",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_MAT2, slvEXTENSION_HALTI},
    {"mat2x3",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_MAT2X3, slvEXTENSION_HALTI},
    {"mat2x4",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_MAT2X4, slvEXTENSION_HALTI},
    {"mat3",                   T_MAT3, T_MAT3, T_MAT3, slvEXTENSION_HALTI},
    {"mat3x2",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_MAT3X2, slvEXTENSION_HALTI},
    {"mat3x3",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_MAT3, slvEXTENSION_HALTI},
    {"mat3x4",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_MAT3X4, slvEXTENSION_HALTI},
    {"mat4",                   T_MAT4, T_MAT4, T_MAT4, slvEXTENSION_HALTI},
    {"mat4x2",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_MAT4X2, slvEXTENSION_HALTI},
    {"mat4x3",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_MAT4X3, slvEXTENSION_HALTI},
    {"mat4x4",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_MAT4, slvEXTENSION_HALTI},
    {"mediump",                T_MEDIUM_PRECISION, T_MEDIUM_PRECISION, T_MEDIUM_PRECISION, slvEXTENSION_HALTI},
    {"namespace",              T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"noinline",               T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"noperspective",          T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"out",                    T_OUT, T_OUT, T_OUT, slvEXTENSION_HALTI},
    {"output",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"packed",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"partition",              T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"patch",                  T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_PATCH, slvEXTENSION_TESSELLATION_SHADER},
    {"precise",                T_NOT_KEYWORD, T_NOT_KEYWORD, T_PRECISE, slvEXTENSION_GPU_SHADER5},
    {"precision",              T_PRECISION, T_PRECISION, T_PRECISION, slvEXTENSION_HALTI},
    {"public",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"readonly",               T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_READONLY, slvEXTENSION_ES_31},
    {"resource",               T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"restrict",               T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_RESTRICT, slvEXTENSION_ES_31},
    {"return",                 T_RETURN, T_RETURN, T_RETURN, slvEXTENSION_HALTI},
    {"sample",                 T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_SAMPLE, slvEXTENSION_TESSELLATION_SHADER | slvEXTENSION_EXT_GEOMETRY_SHADER | slvEXTENSION_SHADER_MULTISAMPLE_INTERPOLATION},
    {"sampler1D",              T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"sampler1DArray",         T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"sampler1DArrayShadow",   T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"sampler1DShadow",        T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"sampler2D",              T_SAMPLER2D, T_SAMPLER2D, T_SAMPLER2D, slvEXTENSION_HALTI},
    {"sampler2DArray",         T_NOT_KEYWORD, T_NOT_KEYWORD, T_SAMPLER2DARRAY, slvEXTENSION_HALTI},
    {"sampler2DArrayShadow",   T_NOT_KEYWORD, T_NOT_KEYWORD, T_SAMPLER2DARRAYSHADOW, slvEXTENSION_HALTI},
    {"sampler2DMS",            T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_SAMPLER2DMS, slvEXTENSION_ES_31},
    {"sampler2DMSArray",       T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_SAMPLER2DMSARRAY, slvEXTENSION_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY},
    {"sampler2DRect",          T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"sampler2DRectShadow",    T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"sampler2DShadow",        T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_SAMPLER2DSHADOW, slvEXTENSION_SHADOW_SAMPLER | slvEXTENSION_HALTI},
    {"sampler3D",              T_SAMPLER3D, T_SAMPLER3D, T_SAMPLER3D, slvEXTENSION_HALTI},
    {"sampler3DRect",          T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"samplerBuffer",          T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_SAMPLERBUFFER, slvEXTENSION_EXT_TEXTURE_BUFFER},
    {"samplerCube",            T_SAMPLERCUBE, T_SAMPLERCUBE, T_SAMPLERCUBE, slvEXTENSION_HALTI},
    {"samplerCubeArray",       T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_SAMPLERCUBEARRAY, slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY},
    {"samplerCubeArrayShadow", T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_SAMPLERCUBEARRAYSHADOW, slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY},
    {"samplerCubeShadow",      T_NOT_KEYWORD, T_NOT_KEYWORD, T_SAMPLERCUBESHADOW, slvEXTENSION_HALTI},
    {"samplerExternalOES",     T_NOT_KEYWORD, T_NOT_KEYWORD, T_SAMPLEREXTERNALOES, slvEXTENSION_EGL_IMAGE_EXTERNAL },
    {"shared",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_SHARED, slvEXTENSION_ES_31},
    {"short",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"sizeof",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"smooth",                 T_NOT_KEYWORD, T_NOT_KEYWORD, T_SMOOTH, slvEXTENSION_HALTI},
    {"static",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"struct",                 T_STRUCT, T_STRUCT, T_STRUCT, slvEXTENSION_HALTI},
    {"subroutine",             T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"superp",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"switch",                 T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_SWITCH, slvEXTENSION_HALTI},
    {"template",               T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"this",                   T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"typedef",                T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"uimage1D",               T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"uimage1DArray",          T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"uimage2D",               T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_UIMAGE2D, slvEXTENSION_ES_31},
    {"uimage2DArray",          T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_UIMAGE2DARRAY, slvEXTENSION_ES_31},
    {"uimage3D",               T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_UIMAGE3D, slvEXTENSION_ES_31},
    {"uimageBuffer",           T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_UIMAGEBUFFER, slvEXTENSION_EXT_TEXTURE_BUFFER},
    {"uimageCube",             T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_UIMAGECUBE, slvEXTENSION_ES_31},
    {"uimageCubeArray",        T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_UIMAGECUBEARRAY, slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY},
    {"uint",                   T_NOT_KEYWORD, T_NOT_KEYWORD, T_UINT, slvEXTENSION_HALTI},
    {"uniform",                T_UNIFORM, T_UNIFORM, T_UNIFORM, slvEXTENSION_HALTI},
    {"union",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"unsigned",               T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"usampler1D",             T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"usampler1DArray",        T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"usampler2D",             T_NOT_KEYWORD, T_NOT_KEYWORD, T_USAMPLER2D, slvEXTENSION_HALTI},
    {"usampler2DArray",        T_NOT_KEYWORD, T_NOT_KEYWORD, T_USAMPLER2DARRAY, slvEXTENSION_HALTI},
    {"usampler2DMS",           T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_USAMPLER2DMS, slvEXTENSION_ES_31},
    {"usampler2DMSArray",      T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_USAMPLER2DMSARRAY, slvEXTENSION_TEXTURE_STORAGE_MULTISAMPLE_2D_ARRAY},
    {"usampler2DRect",         T_NOT_KEYWORD, T_NOT_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"usampler3D",             T_NOT_KEYWORD, T_NOT_KEYWORD, T_USAMPLER3D, slvEXTENSION_HALTI},
    {"usamplerBuffer",         T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_USAMPLERBUFFER, slvEXTENSION_EXT_TEXTURE_BUFFER},
    {"usamplerCube",           T_NOT_KEYWORD, T_NOT_KEYWORD, T_USAMPLERCUBE, slvEXTENSION_HALTI},
    {"usamplerCubeArray",      T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_USAMPLERCUBEARRAY, slvEXTENSION_TEXTURE_CUBE_MAP_ARRAY},
    {"using",                  T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"uvec2",                  T_NOT_KEYWORD, T_NOT_KEYWORD, T_UVEC2, slvEXTENSION_HALTI},
    {"uvec3",                  T_NOT_KEYWORD, T_NOT_KEYWORD, T_UVEC3, slvEXTENSION_HALTI},
    {"uvec4",                  T_NOT_KEYWORD, T_NOT_KEYWORD, T_UVEC4, slvEXTENSION_HALTI},
    {"varying",                T_VARYING, T_VARYING, T_RESERVED_KEYWORD, slvEXTENSION_HALTI},
    {"vec2",                   T_VEC2, T_VEC2, T_VEC2, slvEXTENSION_HALTI},
    {"vec3",                   T_VEC3, T_VEC3, T_VEC3, slvEXTENSION_HALTI},
    {"vec4",                   T_VEC4, T_VEC4, T_VEC4, slvEXTENSION_HALTI},
    {"void",                   T_VOID, T_VOID, T_VOID, slvEXTENSION_HALTI},
    {"volatile",               T_RESERVED_KEYWORD, T_RESERVED_KEYWORD, T_VOLATILE, slvEXTENSION_ES_31},
    {"while",                  T_WHILE, T_WHILE, T_WHILE, slvEXTENSION_HALTI},
    {"writeonly",              T_NOT_KEYWORD, T_RESERVED_KEYWORD, T_WRITEONLY, slvEXTENSION_ES_31},
};

const gctUINT KeywordCount = sizeof(KeywordTable) / sizeof(slsKEYWORD);

#define _CHECK_KEYWORD_TABLE_SORTED  1
static gctBOOL _keywordTableChecked = gcvFALSE;
static void
_IsKeywordTableSorted(
    IN sloCOMPILER Compiler
)
{
    gctUINT i;
    int result;

    if(_keywordTableChecked) return;
    _keywordTableChecked = gcvTRUE;
    for(i=0; i < KeywordCount - 1; i++) {
        result = gcoOS_StrCmp(KeywordTable[i].symbol, KeywordTable[i + 1].symbol);
        if (result != gcvSTATUS_SMALLER) {
            gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                            0,
                            0,
                            slvREPORT_ERROR,
                            "keyword table not sorted at: '%s' and '%s'",
                            KeywordTable[i].symbol,
                            KeywordTable[i + 1].symbol));
            return;

        }
    }
}

static gctINT
_SearchKeyword(
    IN sloCOMPILER Compiler,
    IN gctCONST_STRING Symbol
    )
{
    gctINT    low, mid, high;
    gceSTATUS result;
    gctSIZE_T length = 0, i;

    length = gcoOS_StrLen(Symbol, gcvNULL);

    /* In addition,
    ** all identifiers containing two consecutive underscores (__) are reserved as possible future keywords.
    */
    if (length >= 2)
    {
        for (i = 0; i < length - 1; i++)
        {
            if (Symbol[i] == '_' && Symbol[i + 1] == '_')
            {
                return T_RESERVED_KEYWORD;
            }
        }
    }

#ifdef _CHECK_KEYWORD_TABLE_SORTED
    _IsKeywordTableSorted(Compiler);
#endif
    low = 0;
    high = KeywordCount - 1;

    while (low <= high)
    {
        mid = (low + high) / 2;

        result = gcoOS_StrCmp(Symbol, KeywordTable[mid].symbol);

        if (result == gcvSTATUS_SMALLER)
        {
            high    = mid - 1;
        }
        else if (result == gcvSTATUS_LARGER)
        {
            low        = mid + 1;
        }
        else
        {
            gcmASSERT(gcmIS_SUCCESS(result));

            if (sloCOMPILER_ExtensionEnabled(Compiler, KeywordTable[mid].extension))
            {
                return KeywordTable[mid].enabled;
            }
            else if (sloCOMPILER_ExtensionEnabled(Compiler, slvEXTENSION_NON_HALTI))
            {
                return KeywordTable[mid].token;
            }
            else
            {
                return KeywordTable[mid].disabled;
            }
        }
    }

    return T_NOT_KEYWORD;
}

gctINT
slScanIdentifier(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Symbol,
    OUT slsLexToken * Token
    )
{
    gceSTATUS       status;
    gctINT          tokenType;
    sleSHADER_TYPE  shaderType;
    sltPOOL_STRING  symbolInPool;
    slsNAME *       typeName;
    gctSIZE_T       length = 0;

    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_COMPILER, "LineNo=%u", LineNo);
    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_COMPILER, "StringNo=%u", StringNo);
    gcmTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_COMPILER, "Symbol=%s", gcmOPT_STRING(Symbol));

    gcmASSERT(Token);

    gcmVERIFY_OK(sloCOMPILER_GetShaderType(Compiler, &shaderType));

    gcoOS_ZeroMemory(Token, gcmSIZEOF(slsLexToken));
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    /* Check as a reserved keyword */
    if (sloCOMPILER_IsHaltiVersion(Compiler) &&
        sloCOMPILER_GetScannerState(Compiler) == slvSCANNER_NO_KEYWORD) {
            tokenType = T_NOT_KEYWORD;
    }
    else {
            tokenType = _SearchKeyword(Compiler, Symbol);
    }

    length = gcoOS_StrLen(Symbol, gcvNULL);

    if (sloCOMPILER_IsHaltiVersion(Compiler) && length > _MAX_IDENTIFIER_LENGTH_)
    {
        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        slvREPORT_ERROR,
                        "The maximum length of an identifier is 1024 characters'"));

        return T_RESERVED_KEYWORD;
    }

    if (tokenType == T_RESERVED_KEYWORD)
    {
        Token->type = T_RESERVED_KEYWORD;

        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        slvREPORT_ERROR,
                        "reserved keyword : '%s'",
                        Symbol));

        return T_RESERVED_KEYWORD;
    }
    else if (tokenType != T_NOT_KEYWORD)
    {
        Token->type = tokenType;

        switch (tokenType)
        {
        case T_CONST:
            Token->u.qualifiers.storage = slvSTORAGE_QUALIFIER_CONST;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_STORAGE;
            break;

        case T_UNIFORM:
            Token->u.qualifiers.storage = slvSTORAGE_QUALIFIER_UNIFORM;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_STORAGE;
            break;

        case T_BUFFER:
            Token->u.qualifiers.storage = slvSTORAGE_QUALIFIER_BUFFER;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_STORAGE;
            break;

        case T_SHARED:
            Token->u.qualifiers.storage = slvSTORAGE_QUALIFIER_SHARED;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_STORAGE;
            break;

        case T_ATTRIBUTE:
            if (shaderType != slvSHADER_TYPE_VERTEX)
            {
                gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                                LineNo,
                                StringNo,
                                slvREPORT_ERROR,
                                "'attribute' is only for the vertex shaders",
                                Symbol));
            }

            Token->u.qualifiers.storage = slvSTORAGE_QUALIFIER_ATTRIBUTE;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_STORAGE;
            break;

        case T_VARYING:
            Token->u.qualifiers.storage = (shaderType == slvSHADER_TYPE_VERTEX)?
                    slvSTORAGE_QUALIFIER_VARYING_OUT : slvSTORAGE_QUALIFIER_VARYING_IN;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_STORAGE;
            break;

        case T_INVARIANT:
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_INVARIANT;
            break;

        case T_PRECISE:
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_PRECISE;
            break;

        case T_IN:
            Token->u.qualifiers.storage = slvSTORAGE_QUALIFIER_IN;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_STORAGE;
            break;

        case T_OUT:
            Token->u.qualifiers.storage = slvSTORAGE_QUALIFIER_OUT;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_STORAGE;
            break;

        case T_INOUT:
            Token->u.qualifiers.storage = slvSTORAGE_QUALIFIER_INOUT;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_STORAGE;
            break;

        case T_CENTROID:
            slsQUALIFIERS_SET_AUXILIARY(&Token->u.qualifiers, slvAUXILIARY_QUALIFIER_CENTROID);
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_AUXILIARY;
            break;

        case T_SMOOTH:
            Token->u.qualifiers.interpolation = slvINTERPOLATION_QUALIFIER_SMOOTH;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_INTERPOLATION;
            break;

        case T_FLAT:
            Token->u.qualifiers.interpolation = slvINTERPOLATION_QUALIFIER_FLAT;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_INTERPOLATION;
            break;

        case T_HIGH_PRECISION:
            Token->u.qualifiers.precision    = slvPRECISION_QUALIFIER_HIGH;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_PRECISION;
            break;

        case T_MEDIUM_PRECISION:
            Token->u.qualifiers.precision    = slvPRECISION_QUALIFIER_MEDIUM;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_PRECISION;
            break;

        case T_LOW_PRECISION:
            Token->u.qualifiers.precision    = slvPRECISION_QUALIFIER_LOW;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_PRECISION;
            break;

        case T_COHERENT:
            Token->u.qualifiers.memoryAccess = slvMEMORY_ACCESS_QUALIFIER_COHERENT;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_MEMORY_ACCESS;
            break;

        case T_VOLATILE:
            Token->u.qualifiers.memoryAccess = slvMEMORY_ACCESS_QUALIFIER_VOLATILE;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_MEMORY_ACCESS;
            break;

        case T_RESTRICT:
            Token->u.qualifiers.memoryAccess = slvMEMORY_ACCESS_QUALIFIER_RESTRICT;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_MEMORY_ACCESS;
            break;

        case T_READONLY:
            Token->u.qualifiers.memoryAccess = slvMEMORY_ACCESS_QUALIFIER_READONLY;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_MEMORY_ACCESS;
            break;

        case T_WRITEONLY:
            Token->u.qualifiers.memoryAccess = slvMEMORY_ACCESS_QUALIFIER_WRITEONLY;
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_MEMORY_ACCESS;
            break;

        case T_PATCH:
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_PATCH;
            break;

        case T_SAMPLE:
            slsQUALIFIERS_SET_AUXILIARY(&Token->u.qualifiers, slvAUXILIARY_QUALIFIER_SAMPLE);
            Token->u.qualifiers.flags = slvQUALIFIERS_FLAG_AUXILIARY;
            break;
        }

        gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                        slvDUMP_SCANNER,
                        "<TOKEN line=\"%d\" string=\"%d\" type=\"keyword\" symbol=\"%s\" />",
                        LineNo,
                        StringNo,
                        Symbol));

        return tokenType;
    }

    status = sloCOMPILER_AllocatePoolString(
                                            Compiler,
                                            Symbol,
                                            &symbolInPool);

    if (gcmIS_ERROR(status)) return T_EOF;

    if (sloCOMPILER_GetScannerState(Compiler) == slvSCANNER_NORMAL)
    {
        /* Check as a type name */
        status = sloCOMPILER_SearchName(
                                        Compiler,
                                        symbolInPool,
                                        gcvTRUE,
                                        &typeName);

        if (status == gcvSTATUS_OK && typeName->type == slvSTRUCT_NAME)
        {
            Token->type            = T_TYPE_NAME;
            Token->u.typeName    = typeName;

            gcmVERIFY_OK(sloCOMPILER_Dump(
                                        Compiler,
                                        slvDUMP_SCANNER,
                                        "<TOKEN line=\"%d\" string=\"%d\" type=\"typeName\" symbol=\"%s\" />",
                                        LineNo,
                                        StringNo,
                                        symbolInPool));

            return T_TYPE_NAME;
        }
    }

    /* Treat as an identifier */
    Token->type            = T_IDENTIFIER;
    Token->u.identifier = symbolInPool;

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_SCANNER,
                                "<TOKEN line=\"%d\" string=\"%d\" type=\"identifier\" symbol=\"%s\" />",
                                LineNo,
                                StringNo,
                                Token->u.identifier));

    return T_IDENTIFIER;
}

gctINT
slScanSpecialIdentifier(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Symbol,
    OUT slsLexToken * Token
    )
{
    if (sloCOMPILER_IsHaltiVersion(Compiler)) {
        Token->type = T_RESERVED_KEYWORD;

        gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        slvREPORT_ERROR,
                        "reserved keyword : '%s'",
                        Symbol));

        return T_RESERVED_KEYWORD;
    }
    else {
        return slScanIdentifier(Compiler,
                    LineNo,
                    StringNo,
                    Symbol,
                    Token);
    }
}

gctINT
slScanConvToUnsignedType(
IN sloCOMPILER Compiler,
IN gctUINT LineNo,
IN gctUINT StringNo,
IN gctSTRING Symbol,
OUT slsLexToken * Token
)
{
    gcmASSERT(Token);
    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    if(sloCOMPILER_IsHaltiVersion(Compiler)) {
            /* unsigned is a reserved word in Halti */
        sloCOMPILER_Report(Compiler,
                               LineNo,
                               StringNo,
                               slvREPORT_ERROR,
                               "invalid syntax: '%s'",
                               Symbol);
        Token->type    = T_EOF;
    }
    /* Check as a reserved keyword */
    switch(_SearchKeyword(Compiler, Symbol)) {
    case T_INT:
        Token->type = T_UINT;
        break;

    default:
        gcmASSERT(0);
        Token->type    = T_EOF;
    }

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                      slvDUMP_SCANNER,
                      "<TOKEN line=\"%d\" string=\"%d\" type=\"keyword\" symbol=\"%s\" />",
                      LineNo,
                      StringNo,
                      Symbol));
    return Token->type;
}

gctINT
slScanBoolConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctBOOL Value,
    OUT slsLexToken * Token
    )
{
    gcmASSERT(Token);

    Token->lineNo            = LineNo;
    Token->stringNo            = StringNo;
    Token->type            = T_BOOLCONSTANT;
    Token->u.constant.boolValue    = Value;

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_SCANNER,
                                "<TOKEN line=\"%d\" string=\"%d\" type=\"boolConstant\" value=\"%s\" />",
                                LineNo,
                                StringNo,
                                (Token->u.constant.boolValue)? "true" : "false"));

    return T_BOOLCONSTANT;
}

#define SL_INT_MAX      2147483647
#define _sldUnsignedSuffix  "uU"
#define _sldFloatingSuffix  "fF"


static gctSTRING
_ScanStrpbrk(
IN gctCONST_STRING InStr,
IN gctCONST_STRING MatchChars
)
{

   gctSTRING strPtr;
   gctSTRING matchPtr;
   gctCHAR chr;

   if(InStr == gcvNULL || MatchChars == gcvNULL) return gcvNULL;
   strPtr = (gctSTRING) InStr;
   while(*strPtr)  {
     chr = *strPtr;
     matchPtr = (gctSTRING) MatchChars;
     while(*matchPtr) {
        if(chr != *matchPtr++) continue;
        return strPtr;
     }
     ++strPtr;
   }
   return gcvNULL;
}

static gctSTRING
_ScanIntConstantType(
IN gctSTRING ConstStr,
OUT gctINT *Type,
OUT gctBOOL *IsUnsigned
)
{
  gctINT type = T_INT;
  gctBOOL isUnsigned = gcvFALSE;
  gctSTRING endPtr;

  endPtr = _ScanStrpbrk(ConstStr, _sldUnsignedSuffix);
  if(endPtr) {
     gctSTRING cPtr = endPtr;
     char ch;

     while((ch = *cPtr++) != '\0') {
       switch(ch) {
       case 'u':
       case 'U':
          isUnsigned = gcvTRUE;
          break;
       }
     }
     if(isUnsigned) {
       type = T_UINT;
     }
     else {
       type = T_INT;
     }
  }
  *Type = type;
  *IsUnsigned = isUnsigned;
  return endPtr;
}

static gctSTRING
_ScanFloatConstantType(
IN gctSTRING ConstStr,
IN gctINT *Type
)
{
  gctSTRING endPtr;

  endPtr = _ScanStrpbrk(ConstStr, _sldFloatingSuffix);

  *Type = T_FLOAT;
  return endPtr;
}

static gctINT32
StringToIntConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING String,
    IN gctINT Base,
    IN OUT gctINT * Index
    )
{
    gctINT32 result = 0, digit = 0;
    gctCHAR ch;

    gcmASSERT(String);
    gcmASSERT(Index);
    gcmASSERT((Base == 8) || (Base == 10) || (Base == 16));

    for (; gcvTRUE; (*Index)++)
    {
        ch = String[*Index];

        switch (Base)
        {
        case 8:
            if ((ch >= '0') && (ch <= '7')) digit = ch - '0';
            else goto EXIT;
            break;

        case 10:
            if ((ch >= '0') && (ch <= '9')) digit = ch - '0';
            else goto EXIT;
            break;

        case 16:
            if ((ch >= 'a') && (ch <= 'f'))            digit = ch - 'a' + 10;
            else if ((ch >= 'A') && (ch <= 'F'))    digit = ch - 'A' + 10;
            else if ((ch >= '0') && (ch <= '9'))    digit = ch - '0';
            else goto EXIT;
            break;

        default: gcmASSERT(0);
        }

        if(!sloCOMPILER_IsHaltiVersion(Compiler)) {
            /* no need bound checking */
            result = result * Base + digit;
        }
        else {
            result = result * Base + digit;
        }
    }

EXIT:
    return result;
}

static gctSTRING
_ConvStringToUintConstant(
    IN gctSTRING String,
    IN gctINT Base,
    OUT gctUINT32 *UintConstant
    )
{
    gctUINT32 result = 0, digit = 0;
    gctCHAR ch;
    gctSTRING strEnd;

    gcmASSERT(String);
    gcmASSERT((Base == 8) || (Base == 10) || (Base == 16));

    strEnd = String;
    while(*strEnd != '\0') {
        ch = *strEnd++;
        switch (Base) {
        case 8:
            if ((ch >= '0') && (ch <= '7')) digit = ch - '0';
            else {
               strEnd = String;
                   result = 0;
               goto EXIT;
            }
            break;

        case 10:
            if ((ch >= '0') && (ch <= '9')) digit = ch - '0';
            else {
               strEnd = String;
                   result = 0;
               goto EXIT;
            }
            break;

        case 16:
            if ((ch >= 'a') && (ch <= 'f'))    digit = ch - 'a' + 10;
            else if ((ch >= 'A') && (ch <= 'F')) digit = ch - 'A' + 10;
            else if ((ch >= '0') && (ch <= '9')) digit = ch - '0';
            else {
               strEnd = String;
                   result = 0;
               goto EXIT;
            }
            break;

        default:
            gcmASSERT(0);
            strEnd = String;
            goto EXIT;
        }

                result = result * Base + digit;
    }

EXIT:  /*error invalid string */
    *UintConstant = result;
    return strEnd;
}

static gctUINT32
StringToUintConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING String,
    IN gctINT Base,
    IN OUT gctINT * Index
    )
{
    gctINT orgIndex = *Index;
    gctUINT32 result = 0;
    gctSTRING strEnd;

    strEnd = _ConvStringToUintConstant(String + orgIndex, Base, &result);
    if (strEnd == String + orgIndex)
    {
        /* Ignore octal-constant 0u, it is legal. */
        if (!(String[0] == '0' && String[1] == '\0' && Base == 8))
        {
           /*there is error*/
           gcmVERIFY_OK(sloCOMPILER_Report(Compiler,
                        LineNo,
                        StringNo,
                        slvREPORT_ERROR,
                        "invalid %s unsigned integer: %s",
                        (Base == 8)?
                        "octal" : ((Base == 10)? "decimal" : "hexadecimal"),
                        String + orgIndex));
        }
    }

    *Index = (gctINT) gcoOS_StrLen(String, gcvNULL);
    return result;
}

gctINT
slScanDecIntConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    )
{
    gctINT index = 0;
    char saveChr = '\0';
    gctSTRING endPtr = gcvNULL;
    gctBOOL isUnsigned = gcvFALSE;

    gcmASSERT(Token);

    Token->lineNo            = LineNo;
    Token->stringNo            = StringNo;

    if(sloCOMPILER_IsHaltiVersion(Compiler)) {
           endPtr = _ScanIntConstantType(Text, &Token->type, &isUnsigned);
       if(endPtr) {
             saveChr = *endPtr;
         *endPtr = '\0';
       }
    }

    if(isUnsigned) {
      Token->type = T_UINTCONSTANT;
      Token->u.constant.uintValue = StringToUintConstant(Compiler, LineNo, StringNo, Text, 10, &index);
      gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                        slvDUMP_SCANNER,
                        "<TOKEN line=\"%d\" string=\"%d\" type=\"uintConstant\""
                        " format=\"decimal\" value=\"%u\" />",
                        LineNo,
                        StringNo,
                        Token->u.constant.uintValue));
    }
    else {
      Token->type = T_INTCONSTANT;
      Token->u.constant.intValue = StringToIntConstant(Compiler, LineNo, StringNo, Text, 10, &index);
      gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                        slvDUMP_SCANNER,
                        "<TOKEN line=\"%d\" string=\"%d\" type=\"intConstant\""
                        " format=\"decimal\" value=\"%d\" />",
                        LineNo,
                        StringNo,
                        Token->u.constant.intValue));
    }

    if(endPtr) {
      *endPtr = saveChr;
    }

    return Token->type;
}

gctINT
slScanOctIntConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    )
{
    char saveChr = '\0';
    gctSTRING endPtr = gcvNULL;
    gctBOOL isUnsigned = gcvFALSE;
    gctINT index = 1;

    gcmASSERT(Token);

    Token->lineNo            = LineNo;
    Token->stringNo            = StringNo;

    if(sloCOMPILER_IsHaltiVersion(Compiler)) {
           endPtr = _ScanIntConstantType(Text, &Token->type, &isUnsigned);
       if(endPtr) {
             saveChr = *endPtr;
         *endPtr = '\0';
       }
    }

    if(isUnsigned) {
      Token->type = T_UINTCONSTANT;
      Token->u.constant.uintValue = StringToUintConstant(Compiler, LineNo, StringNo, Text, 8, &index);
      gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                        slvDUMP_SCANNER,
                        "<TOKEN line=\"%d\" string=\"%d\" type=\"uintConstant\""
                        " format=\"octal\" value=\"%u\" />",
                        LineNo,
                        StringNo,
                        Token->u.constant.uintValue));
    }
    else {
      Token->type = T_INTCONSTANT;
      Token->u.constant.intValue = StringToIntConstant(Compiler, LineNo, StringNo, Text, 8, &index);
      gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                        slvDUMP_SCANNER,
                        "<TOKEN line=\"%d\" string=\"%d\" type=\"intConstant\""
                        " format=\"octal\" value=\"%d\" />",
                        LineNo,
                        StringNo,
                        Token->u.constant.intValue));
    }

    if(endPtr) {
      *endPtr = saveChr;
    }

    return Token->type;
}

gctINT
slScanHexIntConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    )
{
    char saveChr = '\0';
    gctSTRING endPtr = gcvNULL;
    gctBOOL isUnsigned = gcvFALSE;
    gctINT index = 2;

    gcmASSERT(Token);

    Token->lineNo            = LineNo;
    Token->stringNo            = StringNo;

    if(sloCOMPILER_IsHaltiVersion(Compiler)) {
           endPtr = _ScanIntConstantType(Text, &Token->type, &isUnsigned);
       if(endPtr) {
             saveChr = *endPtr;
         *endPtr = '\0';
       }
    }

    if(isUnsigned) {
      Token->type = T_UINTCONSTANT;
      Token->u.constant.uintValue = StringToUintConstant(Compiler, LineNo, StringNo, Text, 16, &index);
      gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                        slvDUMP_SCANNER,
                        "<TOKEN line=\"%d\" string=\"%d\" type=\"uintConstant\""
                        " format=\"hexadecimal\" value=\"%u\" />",
                        LineNo,
                        StringNo,
                        Token->u.constant.uintValue));
    }
    else {
      Token->type = T_INTCONSTANT;
      Token->u.constant.intValue = StringToIntConstant(Compiler, LineNo, StringNo, Text, 16, &index);
      gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                        slvDUMP_SCANNER,
                        "<TOKEN line=\"%d\" string=\"%d\" type=\"intConstant\""
                        " format=\"hexadecimal\" value=\"%d\" />",
                        LineNo,
                        StringNo,
                        Token->u.constant.intValue));
    }

    if(endPtr) {
      *endPtr = saveChr;
    }

    return Token->type;
}

gctINT
slScanFloatConstant(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    )
{
    char saveChr ='\0';
    gctSTRING endPtr = gcvNULL;

    gcmASSERT(Token);

    Token->lineNo            = LineNo;
    Token->stringNo            = StringNo;
    Token->type            = T_FLOATCONSTANT;

        endPtr = _ScanFloatConstantType(Text, &Token->type);
    if(endPtr) {
       if(sloCOMPILER_IsHaltiVersion(Compiler)) {
              saveChr = *endPtr;
          *endPtr = '\0';
       }
           else {
              /* suffix not allowed in non-halti version */
          sloCOMPILER_Report(Compiler,
                                 LineNo,
                                 StringNo,
                                 slvREPORT_ERROR,
                                 "invalid syntax: '%s'",
                                 Text);
          return T_EOF;
           }
    }

    gcmVERIFY_OK(gcoOS_StrToFloat(Text, &Token->u.constant.floatValue));

    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                    slvDUMP_SCANNER,
                    "<TOKEN line=\"%d\" string=\"%d\" type=\"floatConstant\""
                    " value=\"%f\" />",
                    LineNo,
                    StringNo,
                    Token->u.constant.floatValue));

    if(endPtr) {
      *endPtr = saveChr;
    }
    return T_FLOATCONSTANT;
}

gctINT
slScanOperator(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    IN gctINT tokenType,
    OUT slsLexToken * Token
    )
{
    gcmASSERT(Token);

    Token->lineNo            = LineNo;
    Token->stringNo            = StringNo;
    Token->type            = tokenType;
    Token->u.operator        = tokenType;

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_SCANNER,
                                "<TOKEN line=\"%d\" string=\"%d\""
                                " type=\"operator\" symbol=\"%s\" />",
                                LineNo,
                                StringNo,
                                Text));

    return tokenType;
}


gctINT
slScanSpecialOperator(
    IN sloCOMPILER Compiler,
    IN sleEXTENSION Extention,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    IN gctINT tokenType,
    OUT slsLexToken * Token
    )
{
    if(sloCOMPILER_ExtensionEnabled(Compiler, Extention))
    {
        return slScanOperator(Compiler,
            LineNo,
            StringNo,
            Text,
            tokenType,
            Token);
    }

    return T_EOF;
}

gctINT
slScanFieldSelection(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Symbol,
    OUT slsLexToken * Token
    )
{
    gceSTATUS        status;
    sltPOOL_STRING    symbolInPool;

    gcmASSERT(Token);

    Token->lineNo    = LineNo;
    Token->stringNo    = StringNo;

    status = sloCOMPILER_AllocatePoolString(
                                            Compiler,
                                            Symbol,
                                            &symbolInPool);

    if (gcmIS_ERROR(status)) return T_EOF;

    Token->type = T_FIELD_SELECTION;
    Token->u.fieldSelection = symbolInPool;

    gcmVERIFY_OK(sloCOMPILER_Dump(
                                Compiler,
                                slvDUMP_SCANNER,
                                "<TOKEN line=\"%d\" string=\"%d\" type=\"fieldSelection\" symbol=\"%s\" />",
                                LineNo,
                                StringNo,
                                Token->u.fieldSelection));

    return T_FIELD_SELECTION;
}


gctINT
slScanLengthMethod(
    IN sloCOMPILER Compiler,
    IN gctUINT LineNo,
    IN gctUINT StringNo,
    IN gctSTRING Text,
    OUT slsLexToken * Token
    )
{
    gcmASSERT(Token);

    Token->lineNo            = LineNo;
    Token->stringNo            = StringNo;
    if(sloCOMPILER_IsHaltiVersion(Compiler)) {
        Token->type            = T_LENGTH_METHOD;
        Token->u.operator        = T_LENGTH_METHOD;
    }
    else {
            /* length() not supported in versions prior to Halti */
        sloCOMPILER_Report(Compiler,
                               LineNo,
                               StringNo,
                               slvREPORT_ERROR,
                               "unrecognizable syntax: '%s'", Text);
        Token->type            = T_EOF;
        Token->u.operator        = T_EOF;
    }
    gcmVERIFY_OK(sloCOMPILER_Dump(Compiler,
                    slvDUMP_SCANNER,
                    "<TOKEN line=\"%d\" string=\"%d\""
                    " type=\"method\" symbol=\"%s\" />",
                    LineNo,
                    StringNo,
                    Text));
    return Token->type;
}

int yywrap(void)
{
    return 1;
}
