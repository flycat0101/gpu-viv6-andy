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
#include "gc_chip_context.h"

#define _GC_OBJ_ZONE gcdZONE_ES30_SHADER

#define MAX_ALIASED_ATTRIB_COUNT 2

gcePROGRAM_STAGE __glChipGLShaderStageToHAL[] =
{
    gcvPROGRAM_STAGE_VERTEX,   /* __GLSL_STAGE_VS   */
    gcvPROGRAM_STAGE_TCS,      /* __GLSL_STAGE_TCS  */
    gcvPROGRAM_STAGE_TES,      /* __GLSL_STAGE_TES  */
    gcvPROGRAM_STAGE_GEOMETRY, /* __GLSL_STAGE_GS   */
    gcvPROGRAM_STAGE_FRAGMENT, /* __GLSL_STAGE_FS   */
    gcvPROGRAM_STAGE_COMPUTE,  /* __GLSL_STAGE_CS   */
    gcvPROGRAM_STAGE_LAST,     /* __GLSL_STAGE_LAST */
};

gcSHADER_KIND __glChipGLShaderStageToShaderKind[] =
{
    gcSHADER_TYPE_VERTEX,   /* __GLSL_STAGE_VS   */
    gcSHADER_TYPE_TCS,      /* __GLSL_STAGE_TCS  */
    gcSHADER_TYPE_TES,      /* __GLSL_STAGE_TES  */
    gcSHADER_TYPE_GEOMETRY, /* __GLSL_STAGE_GS   */
    gcSHADER_TYPE_FRAGMENT, /* __GLSL_STAGE_FS   */
    gcSHADER_TYPE_COMPUTE,  /* __GLSL_STAGE_CS   */
    gcSHADER_TYPE_UNKNOWN,  /* __GLSL_STAGE_LAST */
};

__GLSLStage  __glChipHALShaderStageToGL[] =
{
    __GLSL_STAGE_VS,   /* gcvPROGRAM_STAGE_VERTEX */
    __GLSL_STAGE_TCS,  /* gcvPROGRAM_STAGE_TCS */
    __GLSL_STAGE_TES,  /* gcvPROGRAM_STAGE_TES */
    __GLSL_STAGE_GS,   /* gcvPROGRAM_STAGE_GEOMETRY */
    __GLSL_STAGE_FS,   /* gcvPROGRAM_STAGE_FRAGMENT */
    __GLSL_STAGE_CS,   /* gcvPROGRAM_STAGE_COMPUTE */
    __GLSL_STAGE_LAST, /* gcvPROGRAM_STAGE_OPENCL */
    __GLSL_STAGE_LAST, /* gcvPROGRAM_STAGE_LAST */
};

/* Built-In or compiler-generated uniform info */
typedef struct __GLchipNonUserDefUniformInfoRec
{
    gctCONST_STRING     implSymbol;
    __GLchipUniformUsage    usage;
    __GLchipUniformSubUsage subUsage;
    gctCONST_STRING     outputSymbol;
} __GLchipNonUserDefUniformInfo;


typedef struct __GLchipProgramBinaryV1HeaderRec
{
    GLuint signature1;
    GLuint signature2;
    GLuint remainSize;
} __GLchipProgramBinaryV1Header;

#define CHIP_PROG_V1_SIG1 gcmCC('C','H','I','P')
#define CHIP_PROG_V1_SIG2 gcmCC('P','G','V','1')

static gctBOOL
gcChipIsMatrixType(
    IN gcSHADER_TYPE DataType
    )
{
    switch (DataType)
    {
    case gcSHADER_FLOAT_2X2:
    case gcSHADER_FLOAT_2X3:
    case gcSHADER_FLOAT_2X4:
    case gcSHADER_FLOAT_3X2:
    case gcSHADER_FLOAT_3X3:
    case gcSHADER_FLOAT_3X4:
    case gcSHADER_FLOAT_4X2:
    case gcSHADER_FLOAT_4X3:
    case gcSHADER_FLOAT_4X4:
        return gcvTRUE;

    default:
        return gcvFALSE;
    }
}

__GLchipUniformTypeInfo g_typeInfos[] =
{
    /*halType                                glType                                              size              */
    {gcSHADER_FLOAT_X1,                      GL_FLOAT,                                           1*sizeof(GLfloat)},                /* 0x00 */
    {gcSHADER_FLOAT_X2,                      GL_FLOAT_VEC2,                                      2*sizeof(GLfloat)},                /* 0x01 */
    {gcSHADER_FLOAT_X3,                      GL_FLOAT_VEC3,                                      3*sizeof(GLfloat)},                /* 0x02 */
    {gcSHADER_FLOAT_X4,                      GL_FLOAT_VEC4,                                      4*sizeof(GLfloat)},                /* 0x03 */
    {gcSHADER_FLOAT_2X2,                     GL_FLOAT_MAT2,                                      2*2*sizeof(GLfloat)},              /* 0x04 */
    {gcSHADER_FLOAT_3X3,                     GL_FLOAT_MAT3,                                      3*3*sizeof(GLfloat)},              /* 0x05 */
    {gcSHADER_FLOAT_4X4,                     GL_FLOAT_MAT4,                                      4*4*sizeof(GLfloat)},              /* 0x06 */
    {gcSHADER_BOOLEAN_X1,                    GL_BOOL,                                            1*sizeof(GLint)},                  /* 0x07 */
    {gcSHADER_BOOLEAN_X2,                    GL_BOOL_VEC2,                                       2*sizeof(GLint)},                  /* 0x08 */
    {gcSHADER_BOOLEAN_X3,                    GL_BOOL_VEC3,                                       3*sizeof(GLint)},                  /* 0x09 */
    {gcSHADER_BOOLEAN_X4,                    GL_BOOL_VEC4,                                       4*sizeof(GLint)},                  /* 0x0A */
    {gcSHADER_INTEGER_X1,                    GL_INT,                                             1*sizeof(GLint)},                  /* 0x0B */
    {gcSHADER_INTEGER_X2,                    GL_INT_VEC2,                                        2*sizeof(GLint)},                  /* 0x0C */
    {gcSHADER_INTEGER_X3,                    GL_INT_VEC3,                                        3*sizeof(GLint)},                  /* 0x0D */
    {gcSHADER_INTEGER_X4,                    GL_INT_VEC4,                                        4*sizeof(GLint)},                  /* 0x0E */
    {gcSHADER_SAMPLER_1D,                    GL_NONE,                                            0},                                /* 0x0F */
    {gcSHADER_SAMPLER_2D,                    GL_SAMPLER_2D,                                      1*sizeof(GLint)},                  /* 0x10 */
    {gcSHADER_SAMPLER_3D,                    GL_SAMPLER_3D,                                      1*sizeof(GLint)},                  /* 0x11 */
    {gcSHADER_SAMPLER_CUBIC,                 GL_SAMPLER_CUBE,                                    1*sizeof(GLint)},                  /* 0x12 */
    {gcSHADER_FIXED_X1,                      GL_NONE,                                            0},                                /* 0x13 */
    {gcSHADER_FIXED_X2,                      GL_NONE,                                            0},                                /* 0x14 */
    {gcSHADER_FIXED_X3,                      GL_NONE,                                            0},                                /* 0x15 */
    {gcSHADER_FIXED_X4,                      GL_NONE,                                            0},                                /* 0x16 */

    /* For OCL. */
    {gcSHADER_IMAGE_1D_T,                    GL_NONE,                                            0},                                /* 0x17 */
    {gcSHADER_IMAGE_1D_BUFFER_T,             GL_NONE,                                            0},                                /* 0x18 */
    {gcSHADER_IMAGE_1D_ARRAY_T,              GL_NONE,                                            0},                                /* 0x19 */
    {gcSHADER_IMAGE_2D,                      GL_NONE,                                            0},                                /* 0x1A */
    {gcSHADER_IMAGE_2D_ARRAY_T,              GL_NONE,                                            0},                                /* 0x1B */
    {gcSHADER_IMAGE_3D,                      GL_NONE,                                            0},                                /* 0x1C */
    {gcSHADER_VIV_GENERIC_IMAGE_T,           GL_NONE,                                            0},                                /* 0x1D */
    {gcSHADER_SAMPLER_T,                     GL_NONE,                                            0},                                /* 0x1E */

    {gcSHADER_FLOAT_2X3,                     GL_FLOAT_MAT2x3,                                    2*3*sizeof(GLfloat)},              /* 0x1F */
    {gcSHADER_FLOAT_2X4,                     GL_FLOAT_MAT2x4,                                    2*4*sizeof(GLfloat)},              /* 0x20 */
    {gcSHADER_FLOAT_3X2,                     GL_FLOAT_MAT3x2,                                    3*2*sizeof(GLfloat)},              /* 0x21 */
    {gcSHADER_FLOAT_3X4,                     GL_FLOAT_MAT3x4,                                    3*4*sizeof(GLfloat)},              /* 0x22 */
    {gcSHADER_FLOAT_4X2,                     GL_FLOAT_MAT4x2,                                    4*2*sizeof(GLfloat)},              /* 0x23 */
    {gcSHADER_FLOAT_4X3,                     GL_FLOAT_MAT4x3,                                    4*3*sizeof(GLfloat)},              /* 0x24 */
    {gcSHADER_ISAMPLER_2D,                   GL_INT_SAMPLER_2D,                                  1*sizeof(GLint)},                  /* 0x25 */
    {gcSHADER_ISAMPLER_3D,                   GL_INT_SAMPLER_3D,                                  1*sizeof(GLint)},                  /* 0x26 */
    {gcSHADER_ISAMPLER_CUBIC,                GL_INT_SAMPLER_CUBE,                                1*sizeof(GLint)},                  /* 0x27 */
    {gcSHADER_USAMPLER_2D,                   GL_UNSIGNED_INT_SAMPLER_2D,                         1*sizeof(GLint)},                  /* 0x28 */
    {gcSHADER_USAMPLER_3D,                   GL_UNSIGNED_INT_SAMPLER_3D,                         1*sizeof(GLint)},                  /* 0x29 */
    {gcSHADER_USAMPLER_CUBIC,                GL_UNSIGNED_INT_SAMPLER_CUBE,                       1*sizeof(GLint)},                  /* 0x2A */
    {gcSHADER_SAMPLER_EXTERNAL_OES,          GL_SAMPLER_EXTERNAL_OES,                            1*sizeof(GLint)},                  /* 0x2B */

    {gcSHADER_UINT_X1,                       GL_UNSIGNED_INT,                                    1*sizeof(GLuint)},                 /* 0x2C */
    {gcSHADER_UINT_X2,                       GL_UNSIGNED_INT_VEC2,                               2*sizeof(GLuint)},                 /* 0x2D */
    {gcSHADER_UINT_X3,                       GL_UNSIGNED_INT_VEC3,                               3*sizeof(GLuint)},                 /* 0x2E */
    {gcSHADER_UINT_X4,                       GL_UNSIGNED_INT_VEC4,                               4*sizeof(GLuint)},                 /* 0x2F */

    {gcSHADER_SAMPLER_2D_SHADOW,             GL_SAMPLER_2D_SHADOW,                               1*sizeof(GLuint)},                 /* 0x30 */
    {gcSHADER_SAMPLER_CUBE_SHADOW,           GL_SAMPLER_CUBE_SHADOW,                             1*sizeof(GLuint)},                 /* 0x31 */

    {gcSHADER_SAMPLER_1D_ARRAY,              GL_NONE,                                            0},    /* 1DArray types are invalid in OES, while defined by compiler *//* 0x32 */
    {gcSHADER_SAMPLER_1D_ARRAY_SHADOW,       GL_NONE,                                            0},                                /* 0x33 */
    {gcSHADER_SAMPLER_2D_ARRAY,              GL_SAMPLER_2D_ARRAY,                                1*sizeof(GLuint)},                 /* 0x34 */
    {gcSHADER_ISAMPLER_2D_ARRAY,             GL_INT_SAMPLER_2D_ARRAY,                            1*sizeof(GLuint)},                 /* 0x35 */
    {gcSHADER_USAMPLER_2D_ARRAY,             GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,                   1*sizeof(GLuint)},                 /* 0x36 */
    {gcSHADER_SAMPLER_2D_ARRAY_SHADOW,       GL_SAMPLER_2D_ARRAY_SHADOW,                         1*sizeof(GLuint)},                 /* 0x37 */

    {gcSHADER_SAMPLER_2D_MS,                 GL_SAMPLER_2D_MULTISAMPLE,                          1*sizeof(GLuint)},                 /* 0x38 */
    {gcSHADER_ISAMPLER_2D_MS,                GL_INT_SAMPLER_2D_MULTISAMPLE,                      1*sizeof(GLuint)},                 /* 0x39 */
    {gcSHADER_USAMPLER_2D_MS,                GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE,             1*sizeof(GLuint)},                 /* 0x3A */
    {gcSHADER_SAMPLER_2D_MS_ARRAY,           GL_SAMPLER_2D_MULTISAMPLE_ARRAY_OES,                1*sizeof(GLuint)},                 /* 0x3B */
    {gcSHADER_ISAMPLER_2D_MS_ARRAY,          GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES,            1*sizeof(GLuint)},                 /* 0x3C */
    {gcSHADER_USAMPLER_2D_MS_ARRAY,          GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY_OES,   1*sizeof(GLuint)},                 /* 0x3D */

    {gcSHADER_IMAGE_2D,                      GL_IMAGE_2D,                                    4*sizeof(GLuint)},                     /* 0x3E */
    {gcSHADER_IIMAGE_2D,                     GL_INT_IMAGE_2D,                                    4*sizeof(GLuint)},                 /* 0x3F */
    {gcSHADER_UIMAGE_2D,                     GL_UNSIGNED_INT_IMAGE_2D,                           4*sizeof(GLuint)},                 /* 0x40 */
    {gcSHADER_IMAGE_3D,                      GL_IMAGE_3D,                                    4*sizeof(GLuint)},                     /* 0x41 */
    {gcSHADER_IIMAGE_3D,                     GL_INT_IMAGE_3D,                                    4*sizeof(GLuint)},                 /* 0x42 */
    {gcSHADER_UIMAGE_3D,                     GL_UNSIGNED_INT_IMAGE_3D,                           4*sizeof(GLuint)},                 /* 0x43 */
    {gcSHADER_IMAGE_CUBE,                    GL_IMAGE_CUBE,                                      4*sizeof(GLuint)},                 /* 0x44 */
    {gcSHADER_IIMAGE_CUBE,                   GL_INT_IMAGE_CUBE,                                  4*sizeof(GLuint)},                 /* 0x45 */
    {gcSHADER_UIMAGE_CUBE,                   GL_UNSIGNED_INT_IMAGE_CUBE,                         4*sizeof(GLuint)},                 /* 0x46 */
    {gcSHADER_IMAGE_2D_ARRAY,                GL_IMAGE_2D_ARRAY,                                  4*sizeof(GLuint)},                 /* 0x47 */
    {gcSHADER_IIMAGE_2D_ARRAY,               GL_INT_IMAGE_2D_ARRAY,                              4*sizeof(GLuint)},                 /* 0x48 */
    {gcSHADER_UIMAGE_2D_ARRAY,               GL_UNSIGNED_INT_IMAGE_2D_ARRAY,                     4*sizeof(GLuint)},                 /* 0x49 */
    {gcSHADER_VIV_GENERIC_GL_IMAGE,          GL_NONE,                                            4*sizeof(GLuint)},                 /* 0x4A */

    {gcSHADER_ATOMIC_UINT,                   GL_UNSIGNED_INT_ATOMIC_COUNTER,                     1*sizeof(GLuint)},                 /* 0x4B */

    /* GL_EXT_texture_cube_map_array */
    {gcSHADER_SAMPLER_CUBEMAP_ARRAY,         GL_SAMPLER_CUBE_MAP_ARRAY_EXT,                      1*sizeof(GLuint)},                 /* 0x4C */
    {gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW,  GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_EXT,               1*sizeof(GLuint)},                 /* 0x4D */
    {gcSHADER_ISAMPLER_CUBEMAP_ARRAY,        GL_INT_SAMPLER_CUBE_MAP_ARRAY_EXT,                  1*sizeof(GLuint)},                 /* 0x4E */
    {gcSHADER_USAMPLER_CUBEMAP_ARRAY,        GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_EXT,         1*sizeof(GLuint)},                 /* 0x4F */
    {gcSHADER_IMAGE_CUBEMAP_ARRAY,           GL_IMAGE_CUBE_MAP_ARRAY_EXT,                        4*sizeof(GLuint)},                 /* 0x50 */
    {gcSHADER_IIMAGE_CUBEMAP_ARRAY,          GL_INT_IMAGE_CUBE_MAP_ARRAY_EXT,                    4*sizeof(GLuint)},                 /* 0x51 */
    {gcSHADER_UIMAGE_CUBEMAP_ARRAY,          GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY_EXT,           4*sizeof(GLuint)},                 /* 0x52 */

    {gcSHADER_INT64_X1,                      GL_NONE,                                            0},                                /* 0x53 */
    {gcSHADER_INT64_X2,                      GL_NONE,                                            0},                                /* 0x54 */
    {gcSHADER_INT64_X3,                      GL_NONE,                                            0},                                /* 0x55 */
    {gcSHADER_INT64_X4,                      GL_NONE,                                            0},                                /* 0x56 */
    {gcSHADER_UINT64_X1,                     GL_NONE,                                            0},                                /* 0x57 */
    {gcSHADER_UINT64_X2,                     GL_NONE,                                            0},                                /* 0x58 */
    {gcSHADER_UINT64_X3,                     GL_NONE,                                            0},                                /* 0x59 */
    {gcSHADER_UINT64_X4,                     GL_NONE,                                            0},                                /* 0x5A */

    /* texture buffer extension type. */
    {gcSHADER_SAMPLER_BUFFER,                GL_SAMPLER_BUFFER_OES,                              4*sizeof(GLint)},                  /* 0x5B */
    {gcSHADER_ISAMPLER_BUFFER,               GL_INT_SAMPLER_BUFFER_OES,                          4*sizeof(GLint)},                  /* 0x5C */
    {gcSHADER_USAMPLER_BUFFER,               GL_UNSIGNED_INT_SAMPLER_BUFFER_OES,                 4*sizeof(GLint)},                  /* 0x5D */
    {gcSHADER_IMAGE_BUFFER,                  GL_IMAGE_BUFFER_OES,                                4*sizeof(GLint)},                  /* 0x5E */
    {gcSHADER_IIMAGE_BUFFER,                 GL_INT_IMAGE_BUFFER_OES,                            4*sizeof(GLint)},                  /* 0x5F */
    {gcSHADER_UIMAGE_BUFFER,                 GL_UNSIGNED_INT_IMAGE_BUFFER_OES,                   4*sizeof(GLint)},                  /* 0x60 */
    {gcSHADER_VIV_GENERIC_GL_SAMPLER,        GL_NONE,                                            4*sizeof(GLint)},                  /* 0x61 */

    /* float16 */
    { gcSHADER_FLOAT16_X1,  GL_NONE,                       0},                                                                      /* 0x62 half2 */
    { gcSHADER_FLOAT16_X2,  GL_NONE,                       0},                                                                      /* 0x63 half2 */
    { gcSHADER_FLOAT16_X3,  GL_NONE,                       0},                                                                      /* 0x64 half3 */
    { gcSHADER_FLOAT16_X4,  GL_NONE,                       0},                                                                      /* 0x65 half4 */
    { gcSHADER_FLOAT16_X8,  GL_NONE,                       0},                                                                      /* 0x66 half8 */
    { gcSHADER_FLOAT16_X16, GL_NONE,                       0},                                                                      /* 0x67 half16 */
    { gcSHADER_FLOAT16_X32, GL_NONE,                       0},                                                                      /* 0x68 half32 */

    /*  boolean  */
    { gcSHADER_BOOLEAN_X8,  GL_NONE,                       0},                                                                      /* 0x69 */
    { gcSHADER_BOOLEAN_X16, GL_NONE,                       0},                                                                      /* 0x6A */
    { gcSHADER_BOOLEAN_X32, GL_NONE,                       0},                                                                      /* 0x6B */

    /* uchar vectors  */
    { gcSHADER_UINT8_X1,    GL_NONE,                       0},                                                                      /* 0x6C */
    { gcSHADER_UINT8_X2,    GL_NONE,                       0},                                                                      /* 0x6D */
    { gcSHADER_UINT8_X3,    GL_NONE,                       0},                                                                      /* 0x6E */
    { gcSHADER_UINT8_X4,    GL_NONE,                       0},                                                                      /* 0x6F */
    { gcSHADER_UINT8_X8,    GL_NONE,                       0},                                                                      /* 0x70 */
    { gcSHADER_UINT8_X16,   GL_NONE,                       0},                                                                      /* 0x71 */
    { gcSHADER_UINT8_X32,   GL_NONE,                       0},                                                                      /* 0x72 */

    /* char vectors  */
    { gcSHADER_INT8_X1,     GL_NONE,                       0},                                                                      /* 0x73 */
    { gcSHADER_INT8_X2,     GL_NONE,                       0},                                                                      /* 0x74 */
    { gcSHADER_INT8_X3,     GL_NONE,                       0},                                                                      /* 0x75 */
    { gcSHADER_INT8_X4,     GL_NONE,                       0},                                                                      /* 0x76 */
    { gcSHADER_INT8_X8,     GL_NONE,                       0},                                                                      /* 0x77 */
    { gcSHADER_INT8_X16,    GL_NONE,                       0},                                                                      /* 0x78 */
    { gcSHADER_INT8_X32,    GL_NONE,                       0},                                                                      /* 0x79 */

    /* ushort vectors */
    { gcSHADER_UINT16_X1,   GL_NONE,                       0},                                                                      /* 0x7A */
    { gcSHADER_UINT16_X2,   GL_NONE,                       0},                                                                      /* 0x7B */
    { gcSHADER_UINT16_X3,   GL_NONE,                       0},                                                                      /* 0x7C */
    { gcSHADER_UINT16_X4,   GL_NONE,                       0},                                                                      /* 0x7D */
    { gcSHADER_UINT16_X8,   GL_NONE,                       0},                                                                      /* 0x7E */
    { gcSHADER_UINT16_X16,  GL_NONE,                       0},                                                                      /* 0x7F */
    { gcSHADER_UINT16_X32,  GL_NONE,                       0},                                                                      /* 0x80 */

    /* short vectors */
    { gcSHADER_INT16_X1,    GL_NONE,                       0},                                                                      /* 0x81 */
    { gcSHADER_INT16_X2,    GL_NONE,                       0},                                                                      /* 0x82 */
    { gcSHADER_INT16_X3,    GL_NONE,                       0},                                                                      /* 0x83 */
    { gcSHADER_INT16_X4,    GL_NONE,                       0},                                                                      /* 0x84 */
    { gcSHADER_INT16_X8,    GL_NONE,                       0},                                                                      /* 0x85 */
    { gcSHADER_INT16_X16,   GL_NONE,                       0},                                                                      /* 0x86 */
    { gcSHADER_INT16_X32,   GL_NONE,                       0},                                                                      /* 0x87 */

    /* packed data type */
    /* packed float16 (2 bytes per element) */
    { gcSHADER_FLOAT16_P2,  GL_NONE,                       0},                                                                      /* 0x88 half2 */
    { gcSHADER_FLOAT16_P3,  GL_NONE,                       0},                                                                      /* 0x89 half3 */
    { gcSHADER_FLOAT16_P4,  GL_NONE,                       0},                                                                      /* 0x8A half4 */
    { gcSHADER_FLOAT16_P8,  GL_NONE,                       0},                                                                      /* 0x8B half8 */
    { gcSHADER_FLOAT16_P16, GL_NONE,                       0},                                                                      /* 0x8C half16 */
    { gcSHADER_FLOAT16_P32, GL_NONE,                       0},                                                                      /* 0x8D half32 */

    /* packed boolean (1 byte per element) */
    { gcSHADER_BOOLEAN_P2,  GL_NONE,                       0},                                                                      /* 0x8E bool2 bvec2 */
    { gcSHADER_BOOLEAN_P3,  GL_NONE,                       0},                                                                      /* 0x8F */
    { gcSHADER_BOOLEAN_P4,  GL_NONE,                       0},                                                                      /* 0x90 */
    { gcSHADER_BOOLEAN_P8,  GL_NONE,                       0},                                                                      /* 0x91 */
    { gcSHADER_BOOLEAN_P16, GL_NONE,                       0},                                                                      /* 0x92 */
    { gcSHADER_BOOLEAN_P32, GL_NONE,                       0},                                                                      /* 0x93 */

    /* uchar vectors (1 byte per element) */
    { gcSHADER_UINT8_P2,    GL_NONE,                       0},                                                                      /* 0x94 */
    { gcSHADER_UINT8_P3,    GL_NONE,                       0},                                                                      /* 0x95 */
    { gcSHADER_UINT8_P4,    GL_NONE,                       0},                                                                      /* 0x96 */
    { gcSHADER_UINT8_P8,    GL_NONE,                       0},                                                                      /* 0x97 */
    { gcSHADER_UINT8_P16,   GL_NONE,                       0},                                                                      /* 0x98 */
    { gcSHADER_UINT8_P32,   GL_NONE,                       0},                                                                      /* 0x99 */

    /* char vectors (1 byte per element) */
    { gcSHADER_INT8_P2,     GL_NONE,                       0},                                                                      /* 0x9A */
    { gcSHADER_INT8_P3,     GL_NONE,                       0},                                                                      /* 0x9B */
    { gcSHADER_INT8_P4,     GL_NONE,                       0},                                                                      /* 0x9C */
    { gcSHADER_INT8_P8,     GL_NONE,                       0},                                                                      /* 0x9D */
    { gcSHADER_INT8_P16,    GL_NONE,                       0},                                                                      /* 0x9E */
    { gcSHADER_INT8_P32,    GL_NONE,                       0},                                                                      /* 0x9F */

    /* ushort vectors (2 bytes per element) */
    { gcSHADER_UINT16_P2,   GL_NONE,                       0},                                                                      /* 0xA0 */
    { gcSHADER_UINT16_P3,   GL_NONE,                       0},                                                                      /* 0xA1 */
    { gcSHADER_UINT16_P4,   GL_NONE,                       0},                                                                      /* 0xA2 */
    { gcSHADER_UINT16_P8,   GL_NONE,                       0},                                                                      /* 0xA3 */
    { gcSHADER_UINT16_P16,  GL_NONE,                       0},                                                                      /* 0xA4 */
    { gcSHADER_UINT16_P32,  GL_NONE,                       0},                                                                      /* 0xA5 */

    /* short vectors (2 bytes per element) */
    { gcSHADER_INT16_P2,    GL_NONE,                       0},                                                                      /* 0xA6 */
    { gcSHADER_INT16_P3,    GL_NONE,                       0},                                                                      /* 0xA7 */
    { gcSHADER_INT16_P4,    GL_NONE,                       0},                                                                      /* 0xA8 */
    { gcSHADER_INT16_P8,    GL_NONE,                       0},                                                                      /* 0xA9 */
    { gcSHADER_INT16_P16,   GL_NONE,                       0},                                                                      /* 0xAA */
    { gcSHADER_INT16_P32,   GL_NONE,                       0},                                                                      /* 0xAB */

    { gcSHADER_INTEGER_X8,   GL_NONE,                      0},                                                                      /* 0xAC */
    { gcSHADER_INTEGER_X16,  GL_NONE,                      0},                                                                      /* 0xAD */
    { gcSHADER_UINT_X8,      GL_NONE,                      0},                                                                      /* 0xAE */
    { gcSHADER_UINT_X16,     GL_NONE,                      0},                                                                      /* 0xAF */
    { gcSHADER_FLOAT_X8,     GL_NONE,                      0},                                                                      /* 0xB0 */
    { gcSHADER_FLOAT_X16,    GL_NONE,                      0},                                                                      /* 0xB1 */
    { gcSHADER_INT64_X8,     GL_NONE,                      0},                                                                      /* 0xB2 */
    { gcSHADER_INT64_X16,    GL_NONE,                      0},                                                                      /* 0xB3 */
    { gcSHADER_UINT64_X8,    GL_NONE,                      0},                                                                      /* 0xB4 */
    { gcSHADER_UINT64_X16,   GL_NONE,                      0},                                                                      /* 0xB5 */

    /*halType                glType                     size    */
    {gcSHADER_FLOAT64_X1,    GL_NONE,                      0},
    {gcSHADER_FLOAT64_X2,    GL_NONE,                      0},
    {gcSHADER_FLOAT64_X3,    GL_NONE,                      0},
    {gcSHADER_FLOAT64_X4,    GL_NONE,                      0},
    {gcSHADER_FLOAT64_2X2,   GL_NONE,                      0},
    {gcSHADER_FLOAT64_3X3,   GL_NONE,                      0},
    {gcSHADER_FLOAT64_4X4,   GL_NONE,                      0},
    {gcSHADER_FLOAT64_2X3,   GL_NONE,                      0},
    {gcSHADER_FLOAT64_2X4,   GL_NONE,                      0},
    {gcSHADER_FLOAT64_3X2,   GL_NONE,                      0},
    {gcSHADER_FLOAT64_3X4,   GL_NONE,                      0},
    {gcSHADER_FLOAT64_4X2,   GL_NONE,                      0},
    {gcSHADER_FLOAT64_4X3,   GL_NONE,                      0},
    {gcSHADER_FLOAT64_X8,    GL_NONE,                      0},
    {gcSHADER_FLOAT64_X16,   GL_NONE,                      0},

    /* OpenGL 4.0 types */
    { gcSHADER_SAMPLER_2D_RECT,         GL_NONE,           0},
    { gcSHADER_ISAMPLER_2D_RECT,        GL_NONE,           0},
    { gcSHADER_USAMPLER_2D_RECT,        GL_NONE,           0},
    { gcSHADER_SAMPLER_2D_RECT_SHADOW,  GL_NONE,           0},
    { gcSHADER_ISAMPLER_1D_ARRAY,       GL_NONE,           0},
    { gcSHADER_USAMPLER_1D_ARRAY,       GL_NONE,           0},
    { gcSHADER_ISAMPLER_1D,             GL_NONE,           0},
    { gcSHADER_USAMPLER_1D,             GL_NONE,           0},
    { gcSHADER_SAMPLER_1D_SHADOW,       GL_NONE,           0},

    {gcSHADER_UNKONWN_TYPE,  GL_NONE,                      0},
};
/* compile-time assertion if the g_typeInfos is not the same length as gcSHADER_TYPE_COUNT */
const gctINT _verify_gTypeInfo[sizeof(g_typeInfos)/sizeof(__GLchipUniformTypeInfo) == gcSHADER_TYPE_COUNT] = { 0 };

/*
** Non-user defined uniform info, including build-in and compiler-generated.
** For compiler-generated, we have subusage to differentiate them.
*/
const __GLchipNonUserDefUniformInfo nonUserDefUniformInfo[] =
{
    {"#DepthRange.near", __GL_CHIP_UNIFORM_USAGE_DEPTHRANGE_NEAR,    __GL_CHIP_UNIFORM_SUB_USAGE_NOT_CARE,     "gl_DepthRange.near"},
    {"#DepthRange.far",  __GL_CHIP_UNIFORM_USAGE_DEPTHRANGE_FAR,     __GL_CHIP_UNIFORM_SUB_USAGE_NOT_CARE,     "gl_DepthRange.far"},
    {"#DepthRange.diff", __GL_CHIP_UNIFORM_USAGE_DEPTHRANGE_DIFF,    __GL_CHIP_UNIFORM_SUB_USAGE_NOT_CARE,     "gl_DepthRange.diff"},
    {"#sh_rtHeight",     __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_RTHEIGHT,     gcvNULL},
    {"#num_group",       __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_GROUPNUM,     gcvNULL},
    {"#sh_startVertex",  __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_STARTVERTEX,                           gcvNULL},
    {"#sh_totalVtxCountPerInstance",  __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_VERTEXCOUNT_PER_INSTANCE, gcvNULL},
    {"#sh_multiLayerTex", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_MULTILAYER_SAMPLER,                   gcvNULL},
    {"#sh_blend_sampler", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_SAMPLER,               gcvNULL},
    {"#sh_blend_enable_mode", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_STATE,             gcvNULL},
    {"#sh_imageExtraLayer", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_MULTILAYER_IMAGE,                   gcvNULL},
    {"#sh_rtWidth",         __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_RTWIDTH,     gcvNULL},
    {"#sampleMaskValue",    __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_SAMPLEMASK_VALUE,     gcvNULL},
    {"#sampleCoverageValue_Invert",    __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_COVERAGE_INV,     gcvNULL},
    {"#NumSamples",    __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_SAMPLE_NUM,     gcvNULL},
    {"#TcsPatchVerticesIn",    __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_TCS_PATCH_VERTICES_IN,     gcvNULL},
    {"#TesPatchVerticesIn",    __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_TES_PATCH_VERTICES_IN,     gcvNULL},
    { "#sh_alphaBlendEquation", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_EQUATION, gcvNULL },
    { "#sh_alphaBlendFunction", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_FUNCTION, gcvNULL },
    { "#sh_rt_WidthHeight", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_RT_WH, gcvNULL },
    { "#sh_blendConstColor", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_CONSTANT_COLOR, gcvNULL },
    { "#sh_alphablend_sampler", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_SAMPLER, gcvNULL },
    { "#sh_yInvert", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_YINVERT, gcvNULL },
    { "#sh_DepthBias", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_DEPTH_BIAS, gcvNULL },
    { "#sh_rtImage", __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED, __GL_CHIP_UNIFORM_SUB_USAGE_RT_IMAGE, gcvNULL }

};

/* unique program ID */
GLuint g_progId = 0;

/************************************************************************/
/* Implementation for internal functions                                */
/************************************************************************/

#if TEMP_SHADER_PATCH

__GL_INLINE void
_FlushChannelOfPatchedUniformImm(
    IN  __GLcontext *gc,
    IN GLint hwPatchedConstRegAddr,
    IN gctUINT8 patchedSingleChannel,
    IN gctUINT32 channelData
    )
{
    gctUINT32 address, psBase;

    gcoHAL_QueryUniformBase(gcvNULL, gcvNULL, &psBase);

    psBase = psBase * 4;

    address = psBase + hwPatchedConstRegAddr * 16 + patchedSingleChannel * 4;

    gcoSHADER_BindUniform(gcvNULL, address, hwPatchedConstRegAddr, 1, 1, 1, gcvFALSE, 4, 4,
                         (gctPOINTER)&channelData, gcvUNIFORMCVT_NONE, gcSHADER_TYPE_FRAGMENT);
}

__GL_INLINE void
gcChipTmpPatchFlushUniforms(
    IN  __GLcontext *gc,
    IN __GLchipSLProgram *program
    )
{
    /* CALL _FlushChannelOfPatchedUniform FOR EACH CHANNEL OF PATCHED UNIFORM */
    switch (program->curPgInstance->programState.hints->pachedShaderIdentifier)
    {
    case gcvMACHINECODE_GLB27_RELEASE_0:
        _FlushChannelOfPatchedUniformImm(gc, 2, gcSL_SWIZZLE_X, 0x3D888889); /*0.066667*/
        _FlushChannelOfPatchedUniformImm(gc, 2, gcSL_SWIZZLE_Y, 0x3E000000); /*0.125000*/
        _FlushChannelOfPatchedUniformImm(gc, 2, gcSL_SWIZZLE_Z, 0x3D088889); /*0.033333*/
        _FlushChannelOfPatchedUniformImm(gc, 2, gcSL_SWIZZLE_W, 0x3D800000); /*0.062500*/
        _FlushChannelOfPatchedUniformImm(gc, 3, gcSL_SWIZZLE_X, 0x3E800000); /*0.250000*/
        break;

    default:
        break;
    }
}

#endif

/*
** Float2Bool cannot combined with int2Bool, because float -0.0 (0x80000000)
** will be treated as false, while int (0x80000000) will not be.
*/
__GL_INLINE void
gcChipUtilFloat2Bool(
    GLuint *dst,
    const GLfloat *src,
    gctSIZE_T count
    )
{
    gctSIZE_T i;
    for (i = 0; i < count; ++i)
    {
        dst[i] = (src[i] == 0.0f) ? 0 : 1;
    }
}

__GL_INLINE void
gcChipUtilInt2Bool(
    GLuint *dst,
    const GLint *src,
    gctSIZE_T count
    )
{
    gctSIZE_T i;
    for (i = 0; i < count; ++i)
    {
        dst[i] = (src[i] == 0) ? 0 : 1;
    }
}

__GL_INLINE void
gcChipUtilTransposeMatrix(
    GLfloat *dst,
    const GLfloat *src,
    gctSIZE_T count,
    gctSIZE_T rows,
    gctSIZE_T cols
    )
{
    gctSIZE_T i, x, y;
    for (i = 0; i < count; ++i)
    {
        for (x = 0; x < rows; ++x)
        {
            for (y = 0; y < cols; ++y)
            {
                dst[i * (rows * cols) + x * cols + y] = src[i * (rows * cols) + y * rows + x];
            }
        }
    }
}

__GL_INLINE void
gcChipUtilInt2Float(
    GLfloat *dst,
    const GLint *src,
    gctSIZE_T count
    )
{
    gctSIZE_T i;
    for (i = 0; i < count; ++i)
    {
        dst[i] = (GLfloat)src[i];
    }
}


__GL_INLINE gceUNIFORMCVT
gcChipQueryUniformConvert(
    __GLchipSLProgram *program,
    gcSHADER_TYPE      dataType,
    gctBOOL            userDefUB,
    gctBOOL            hasIntegerSupport
    )
{
    gceUNIFORMCVT convert = gcvUNIFORMCVT_NONE;

    gcmHEADER_ARG("program=0x%x dataType=%d, userDefUB=%d", program, dataType, userDefUB);

    /*
    ** For es20 shaders, compiler always uses float in the machine code.
    ** It is needed to convert app-specified INT data into float before send to HW
    ** Note: 1) Bool was already converted to INT 0/1 when specifying
    **       2) There is no uint data in es20 shaders
    ** For es30 shaders, if a boolean of user-def UB was mapped to reg. It need to
    ** be converted to 0/1 before send to HW. Booleans in default/private UB were
    ** already converted when specifying.
    */
    switch (dataType)
    {
        case gcSHADER_BOOLEAN_X1:
        case gcSHADER_BOOLEAN_X2:
        case gcSHADER_BOOLEAN_X3:
        case gcSHADER_BOOLEAN_X4:
            if (
#if TREAT_ES20_INTEGER_AS_FLOAT
                !program->isHalti ||
#endif
                !hasIntegerSupport)
            {
                convert = gcvUNIFORMCVT_TO_FLOAT;
            }
            else
            {
                if (userDefUB)
                {
                    convert = gcvUNIFORMCVT_TO_BOOL;
                }
            }
            break;

        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
        case gcSHADER_SAMPLER_2D:
        case gcSHADER_SAMPLER_3D:
        case gcSHADER_SAMPLER_CUBIC:
            if (
#if TREAT_ES20_INTEGER_AS_FLOAT
                !program->isHalti ||
#endif
                !hasIntegerSupport)
            {
                convert = gcvUNIFORMCVT_TO_FLOAT;
            }
            break;

        default:
            break;
    }

    gcmFOOTER_ARG("return=%d", convert);
    return convert;
}

static __GLchipUniformUsage
gcChipUtilFindUniformUsage(
    gcSHADER Shader,
    gcUNIFORM uniform,
    gctCONST_STRING *name,
    __GLchipUniformSubUsage *pSubUsage
    )
{
    GLint i;
    __GLchipUniformUsage usage;
    __GLchipUniformSubUsage subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_NOT_CARE;

    gcmHEADER_ARG("uniform=0x%x name=%s subUsage=0x%x", uniform, gcmOPT_STRING(*name), subUsage);

    if ((*name)[0] != '#')
    {
        usage = __GL_CHIP_UNIFORM_USAGE_USER_DEFINED;
        subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_NOT_CARE;
    }
    else if (isUniformTransfromFeedbackState(uniform))
    {
        usage = __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED;
        subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_XFB_ENABLE;
    }
    else if (isUniformTransfromFeedbackBuffer(uniform))
    {
        usage = __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED;
        subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_XFB_BUFFER;
    }
    else if (isUniformLoadtimeConstant(uniform))
    {
        usage = __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED;
        subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_LTC;
    }
    else if (isUniformLevelBaseSize(uniform))
    {
        gcUNIFORM parentUniform = gcvNULL;

        gcSHADER_GetUniform(Shader, uniform->parent, &parentUniform);
        gcmASSERT(parentUniform);
        usage = __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED;

        if (isSamplerType(parentUniform->u.type))
        {
            subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_BASE_LEVEL_SIZE;
        }
        else
        {
            gcmASSERT(isOGLImageType(parentUniform->u.type));
            subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_IMAGE_SIZE;
        }
    }
    else if (isUniformLodMinMax(uniform))
    {
        usage = __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED;
        subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_LOD_MIN_MAX;
    }
    else if (isUniformSampleLocation(uniform))
    {
        usage = __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED;
        subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_SAMPLE_LOCATIONS;
    }
    else if (isUniformMultiSampleBuffers(uniform))
    {
        usage = __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED;
        subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_MULTISAMPLE_BUFFERS;
    }
    else
    {
        usage = __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED;
        subUsage = __GL_CHIP_UNIFORM_SUB_USAGE_NOT_CARE;

        for (i = 0; i < __GL_TABLE_SIZE(nonUserDefUniformInfo); i++)
        {
            gctSIZE_T strLen;
            gcoOS_StrLen(nonUserDefUniformInfo[i].implSymbol, &strLen);
            if (gcmIS_SUCCESS(gcoOS_StrNCmp(nonUserDefUniformInfo[i].implSymbol, *name, strLen)))
            {
                usage = nonUserDefUniformInfo[i].usage;
                subUsage = nonUserDefUniformInfo[i].subUsage;
                if (nonUserDefUniformInfo[i].outputSymbol != gcvNULL)
                {
                    *name = nonUserDefUniformInfo[i].outputSymbol;
                }
                break;
            }
        }
    }

    if (pSubUsage)
    {
        *pSubUsage = subUsage;
    }

    gcmFOOTER_ARG("return=%d", usage);
    return usage;
}

static __GLchipUbUsage
gcChipUtilFindUbUsage(
    gctCONST_STRING name
    )
{
    __GLchipUbUsage usage = __GL_CHIP_UB_USAGE_USER_DEFINED;

    gcmHEADER_ARG("name=%s ", name);

    if (gcmIS_SUCCESS(gcoOS_StrNCmp(name, "#ConstantUBO", 12)))
    {
        usage = __GL_CHIP_UB_USAGE_PRIVATE;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrNCmp(name, "#DefaultUBO", 11)))
    {
        usage = __GL_CHIP_UB_USAGE_DEFAULT;
    }

    gcmFOOTER_ARG("return=%d", usage);
    return usage;
}

static __GLchipSsbUsage
gcChipUtilFindSsbUsage(
    gctCONST_STRING name
    )
{
    __GLchipSsbUsage usage = __GL_CHIP_SSB_USAGE_USERDEF;

    gcmHEADER_ARG("name=%s ", name);

    if (gcmIS_SUCCESS(gcoOS_StrCmp(name, _sldSharedVariableStorageBlockName)))
    {
        usage = __GL_CHIP_SSB_USAGE_SHAREDVAR;
    }
    else if (gcmIS_SUCCESS(gcoOS_StrCmp(name, "#sh_extraReg")))
    {
        usage = __GL_CHIP_SSB_USAGE_EXTRAREG;
    }

    gcmFOOTER_ARG("return=%d", usage);
    return usage;
}

static gcSHADER_PRECISION
gcChipUtilFixPrecison(
    __GLchipSLProgram *program,
    gcSHADER_PRECISION precision,
    gcSHADER_TYPE dataType,
    __GLSLStage stageIdx
    )
{
    /* Get real precision if default.
    ** In fact, the code should be handled in compiler. Compiler should expand "default" to
    ** one of "lowp", "mediump" or "highp".
    */
    gcmHEADER_ARG("program=0x%x precision=%d dataType=%d stageIdx=%d",
                   program, precision, dataType, stageIdx);

    if (gcSHADER_PRECISION_DEFAULT == precision)
    {
        switch (dataType)
        {
        case gcSHADER_FLOAT_X1:
        case gcSHADER_FLOAT_X2:
        case gcSHADER_FLOAT_X3:
        case gcSHADER_FLOAT_X4:
        case gcSHADER_FLOAT_2X2:
        case gcSHADER_FLOAT_3X3:
        case gcSHADER_FLOAT_4X4:
        case gcSHADER_FLOAT_2X3:
        case gcSHADER_FLOAT_2X4:
        case gcSHADER_FLOAT_3X2:
        case gcSHADER_FLOAT_3X4:
        case gcSHADER_FLOAT_4X2:
        case gcSHADER_FLOAT_4X3:
            /* change "default" to "highp" for VS float or PS float in ES20 */
            if (0 == program->isHalti || __GLSL_STAGE_VS == stageIdx)
            {
                precision = gcSHADER_PRECISION_HIGH;
            }
            break;

        case gcSHADER_INTEGER_X1:
        case gcSHADER_INTEGER_X2:
        case gcSHADER_INTEGER_X3:
        case gcSHADER_INTEGER_X4:
        case gcSHADER_UINT_X1:
        case gcSHADER_UINT_X2:
        case gcSHADER_UINT_X3:
        case gcSHADER_UINT_X4:
            /* change "default" to "highp" for VS int */
            if (__GLSL_STAGE_VS == stageIdx)
            {
                precision = gcSHADER_PRECISION_HIGH;
            }
            /* change "default" to "mediump" for PS int */
            else if (__GLSL_STAGE_FS == stageIdx)
            {
                precision = gcSHADER_PRECISION_MEDIUM;
            }
            break;

        case gcSHADER_SAMPLER_2D:
        case gcSHADER_SAMPLER_CUBIC:
            /* "lowp" for sampler2D and samplercube as spec requires */
            precision = gcSHADER_PRECISION_LOW;
            break;

        default:
            break;
        }
    }

    gcmFOOTER_ARG("return=%d", precision);
    return precision;
}

__GL_INLINE __GLSLStage
gcChipGetShaderStage(
    gcSHADER shader
)
{
    __GLSLStage stageIdx = __GLSL_STAGE_LAST;
    gcSHADER_KIND shaderType = gcSHADER_TYPE_UNKNOWN;

    gcmHEADER_ARG("shader=0x%x", shader);

    gcSHADER_GetType(shader, &shaderType);

    switch (shaderType)
    {
    case gcSHADER_TYPE_VERTEX:
        stageIdx = __GLSL_STAGE_VS;
        break;
    case gcSHADER_TYPE_FRAGMENT:
        stageIdx = __GLSL_STAGE_FS;
        break;
    case gcSHADER_TYPE_COMPUTE:
        stageIdx = __GLSL_STAGE_CS;
        break;
    case gcSHADER_TYPE_TCS:
        stageIdx = __GLSL_STAGE_TCS;
        break;
    case gcSHADER_TYPE_TES:
        stageIdx = __GLSL_STAGE_TES;
        break;
    case gcSHADER_TYPE_GEOMETRY:
        stageIdx = __GLSL_STAGE_GS;
        break;
    default:
        GL_ASSERT(0);
    }

    gcmFOOTER_ARG("return=%u", stageIdx);
    return stageIdx;
}

__GL_INLINE __GLSLStage
gcChipGetFirstStage(
    __GLchipSLProgram *program
    )
{
    __GLSLStage stage;
    __GLchipSLProgramInstance* masterPgInstance = program->masterPgInstance;

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (masterPgInstance->binaries[stage])
        {
            break;
        }
    }

    return stage;
}

__GL_INLINE __GLSLStage
gcChipGetLastStage(
    __GLchipSLProgram *program
    )
{
    __GLSLStage iter;
    __GLSLStage stage = __GLSL_STAGE_LAST;
    __GLchipSLProgramInstance* masterPgInstance = program->masterPgInstance;

    for (iter = __GLSL_STAGE_LAST - 1; iter >= __GLSL_STAGE_VS; --iter)
    {
        if (masterPgInstance->binaries[iter])
        {
            stage = iter;
            break;
        }
    }

    return stage;
}

__GL_INLINE __GLSLStage
gcChipGetLastNonFragStage(
    __GLchipSLProgram *program
    )
{
    __GLSLStage iter;
    __GLSLStage stage = __GLSL_STAGE_LAST;
    __GLchipSLProgramInstance* masterPgInstance = program->masterPgInstance;

    for (iter = __GLSL_STAGE_LAST - 1; iter >= __GLSL_STAGE_VS; --iter)
    {
        if (iter != __GLSL_STAGE_FS && iter != __GLSL_STAGE_CS && masterPgInstance->binaries[iter])
        {
            stage = iter;
            break;
        }
    }

    return stage;
}

__GL_INLINE gcsUNIFORM_BLOCK
gcChipUBGetPrevActiveSibling(
    gcSHADER Shader,
    gcsUNIFORM_BLOCK uniformBlock
    )
{
    gcsUNIFORM_BLOCK prevActiveUB = gcvNULL;
    gctINT sibling = GetSVIPrevSibling(GetUBShaderVarInfo(uniformBlock));

    while (sibling != -1)
    {
        gceSTATUS status;
        gcsUNIFORM_BLOCK prevUB;
        gcUNIFORM prevUniform;

        gcmERR_BREAK(gcSHADER_GetUniformBlock(Shader, sibling, &prevUB));
        gcmERR_BREAK(gcSHADER_GetUniform(Shader, GetUBIndex(prevUB), &prevUniform));

        if (!isUniformInactive(prevUniform))
        {
            prevActiveUB = prevUB;
            break;
        }

        sibling = GetSVIPrevSibling(GetUBShaderVarInfo(prevUB));
    }

    return prevActiveUB;
}

__GL_INLINE gcsSTORAGE_BLOCK
gcChipSSBGetPrevActiveSibling(
    gcSHADER Shader,
    gcsSTORAGE_BLOCK storageBlock
    )
{
    gcsSTORAGE_BLOCK prevActiveSSB = gcvNULL;
    gctINT sibling = GetSVIPrevSibling(GetSBShaderVarInfo(storageBlock));

    while (sibling != -1)
    {
        gceSTATUS status;
        gcsSTORAGE_BLOCK prevSB;
        gcUNIFORM prevUniform;

        gcmERR_BREAK(gcSHADER_GetStorageBlock(Shader, sibling, &prevSB));
        gcmERR_BREAK(gcSHADER_GetUniform(Shader, GetSBIndex(prevSB), &prevUniform));

        if (!isUniformInactive(prevUniform))
        {
            prevActiveSSB = prevSB;
            break;
        }

        sibling = GetSVIPrevSibling(GetSBShaderVarInfo(prevSB));
    }

    return prevActiveSSB;
}

/* Return how many uniform entries will be reported to app */
__GL_INLINE GLuint
gcChipGetUniformArrayInfo(
    gcUNIFORM uniform,
    gctCONST_STRING name,
    gctUINT *maxNameLen,    /* max possible name len of this entry, exclude bottom-level "[0]" */
    gctBOOL *isArray,
    gctUINT *arraySize
)
{
    gctINT j;
    gctUINT32 length;
    GLuint entries = 1;

    if (name)
    {
        length = (gctUINT32)gcoOS_StrLen(name, gcvNULL);
    }
    else
    {
        gcUNIFORM_GetName(uniform, &length, gcvNULL);
    }

    /* Multiple entries will be reported for array of arrays.
    ** If a uniform is an array, or array of array, its name length should be added
    ** with several dim of array indices, like "name[x]...[x][0]".
    */
    for (j = 0; j < uniform->arrayLengthCount - 1; ++j)
    {
        gctUINT decimalLen = 1;
        gctINT arrayLen = uniform->arrayLengthList[j];

        GL_ASSERT(arrayLen > 0);
        entries *= arrayLen;

        arrayLen--;    /* Get max arrayIndex */
        while (arrayLen >= 10)
        {
            ++decimalLen;
            arrayLen /= 10;
        }

        length += (decimalLen + 2);
    }

    if (maxNameLen)
    {
        *maxNameLen = length;
    }

    if (isArray)
    {
        *isArray = uniform->arrayLengthCount > 0 ? gcvTRUE : gcvFALSE;
    }

    if (arraySize)
    {
        *arraySize = uniform->arrayLengthCount > 0
                   ? (uniform->arrayLengthList[uniform->arrayLengthCount - 1] > 0 ?
                      uniform->arrayLengthList[uniform->arrayLengthCount - 1] : 0)
                   : 1;
    }

    return entries;
}

static gctBOOL
gcChipCheckUniformActive(
    IN     __GLcontext *gc,
    IN     __GLchipSLProgram *program,
    IN     gcSHADER Shader,
    IN     gcUNIFORM HalUniform
    )
{
    gctBOOL isActive = gcvTRUE;

    if (!HalUniform || isUniformImmediate(HalUniform) ||
        (isUniformInactive(HalUniform) && HalUniform->location == -1))
    {
        isActive = gcvFALSE;
    }


    return isActive;
}

static GLuint
gcChipCountUniforms(
    IN     __GLcontext *gc,
    IN     __GLchipSLProgram *program,
    IN     gcSHADER Shader,
    IN     GLint Count,
    IN     GLboolean recompile,
    IN     GLint *Index,
    IN OUT gctCONST_STRING *Names
    )
{
    GLint i, j;
    GLint prevIdx = *Index;
    GLuint activeUniforms = 0;
    __GLchipSLProgramInstance* pgInstance = program->curPgInstance;

    gcmHEADER_ARG("gc=0x%x program=0x%x Shader=0x%x Count=%d recompile=%d Index=0x%x Name=0x%x",
                  gc, program, Shader, Count, recompile, Index, Names);

    for (i = 0; i < Count; ++i)
    {
        gcUNIFORM uniform;
        gctUINT length;
        gctCONST_STRING name;
        gctBOOL isArray;
        gcSHADER_TYPE_KIND category = gceTK_UNKOWN;
        GLboolean duplicated = GL_FALSE;
        __GLchipUniformUsage usage;
        GLuint numEntries = 1;

        gcmVERIFY_OK(gcSHADER_GetUniform(Shader, i, &uniform));

        /* Skip inactive and not located uniforms */
        if (!gcChipCheckUniformActive(gc, program, Shader, uniform))
        {
            continue;
        }

        /* Skip uniform struct/block entries */
        if (!isUniformNormal(uniform) &&
            !isUniformBlockMember(uniform) &&
            !isUniformLevelBaseSize(uniform) &&
            !isUniformLodMinMax(uniform) &&
            !isUniformSampleLocation(uniform) &&
            !isUniformMultiSampleBuffers(uniform))
        {
            continue;
        }

        /* uniform block member */
        if (GetUniformBlockID(uniform) != -1)
        {
            gcsUNIFORM_BLOCK uniformBlock;

            gcmVERIFY_OK(gcSHADER_GetUniformBlock(Shader,
                                                  GetUniformBlockID(uniform),
                                                  &uniformBlock));

            if (!uniformBlock || GetUBIndex(uniformBlock) == -1)
            {
                continue;
            }

            /* skip non-zeroth element of uniform block array */
            if (gcChipUBGetPrevActiveSibling(Shader, uniformBlock))
            {
                continue;
            }
        }

        gcmVERIFY_OK(gcUNIFORM_GetName(uniform, &length, &name));
        usage = gcChipUtilFindUniformUsage(Shader, uniform, &name, gcvNULL);
        numEntries = gcChipGetUniformArrayInfo(uniform, name, &length, &isArray, gcvNULL);

        if (usage != __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED)
        {
            /* Active uniforms count separately in VS and PS */
            activeUniforms += numEntries;
        }

        /* For arrays, its name length should add the length of "[0]". */
        if (isArray)
        {
            length += 3;
        }

        /* Check duplicated uniforms in previous shader stage */
        for (j = 0; j < prevIdx; ++j)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(name, Names[j])))
            {
                duplicated = GL_TRUE;
                break;
            }
        }

        /* Skip duplicated uniforms */
        if (duplicated)
        {
            continue;
        }

        gcmVERIFY_OK(gcUNIFORM_GetTypeEx(uniform, gcvNULL, &category, gcvNULL, gcvNULL));
        if (!recompile && category == gceTK_ATOMIC)
        {
            GLint binding = GetUniformBinding(uniform);

            GL_ASSERT(binding >= 0 && binding < (GLint)gc->constants.shaderCaps.maxAtomicCounterBufferBindings);

            /* Allocate acbIdx2Slot table if first detect active atomic counter */
            if (!program->acbBinding2SlotIdx)
            {
                GLuint i;
                gctSIZE_T bytes = gcmSIZEOF(GLint) * gc->constants.shaderCaps.maxCombinedAtomicCounters;

                GL_ASSERT(!program->acbBinding2NumACs);
                gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*)&program->acbBinding2SlotIdx));
                gcmVERIFY_OK(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*)&program->acbBinding2NumACs));

                for (i = 0; i < gc->constants.shaderCaps.maxCombinedAtomicCounters; ++i)
                {
                    /* Initialize there is no acb used in shader for any index */
                    program->acbBinding2SlotIdx[i] = -1;
                    program->acbBinding2NumACs[i] = 0;
                }
            }

            program->acbBinding2NumACs[binding] += numEntries;
            if (program->acbBinding2SlotIdx[binding] == -1)
            {
                program->acbBinding2SlotIdx[binding] = program->acbCount++;
            }
        }

        switch (usage)
        {
        case __GL_CHIP_UNIFORM_USAGE_USER_DEFINED:
            program->userDefUniformCount += numEntries;
            program->uniformMaxNameLen = __GL_MAX(program->uniformMaxNameLen, length + 1);
            break;
        case __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED:
            pgInstance->privateUniformCount += numEntries;
            break;
        default:
            program->builtInUniformCount += numEntries;
            /* Spec requires reported uniforms include built-in ones. */
            length = (gctUINT32)gcoOS_StrLen(name, gcvNULL);
            program->uniformMaxNameLen = __GL_MAX(program->uniformMaxNameLen, length + 1);
            break;
        }

        /* Store them for later duplicate check */
        Names[(*Index)++] = name;
    }

    gcmFOOTER_ARG("return=%u", activeUniforms);
    return activeUniforms;
}

static GLuint
gcChipCountUniformBlocks(
    IN     __GLchipSLProgram *program,
    IN     gcSHADER Shader,
    IN     GLint Count,
    IN     GLint *Index,
    IN OUT gctCONST_STRING *Names
    )
{
    GLint i, j;
    GLint prevIdx = *Index;
    GLuint activeUBs = 0;
    __GLchipSLProgramInstance* pgInstance = program->curPgInstance;

    gcmHEADER_ARG("program=0x%x Shader=0x%x Count=%d Index=0x%x Names=0x%x",
                  program, Shader, Count, Index, Names);

    for (i = 0; i < Count; ++i)
    {
        GLboolean duplicated = GL_FALSE;
        gcsUNIFORM_BLOCK uniformBlock;
        gcUNIFORM ubUniform;
        __GLchipUbUsage ubUsage;

        gcmVERIFY_OK(gcSHADER_GetUniformBlock(Shader, i, &uniformBlock));

        if (!uniformBlock || GetUBBlockIndex(uniformBlock) == -1)
        {
            continue;
        }

        gcSHADER_GetUniform(Shader, GetUBIndex(uniformBlock), &ubUniform);

        /* Skip inactive uniform blocks */
        if (isUniformInactive(ubUniform))
        {
            continue;
        }

        ubUsage = gcChipUtilFindUbUsage(GetUniformName(ubUniform));
        if (ubUsage == __GL_CHIP_UB_USAGE_USER_DEFINED)
        {
            /* Active uniform blocks count separately in VS and PS */
            ++activeUBs;
        }

        /* Check duplicated uniform blocks in previous shader stage */
        for (j = 0; j < prevIdx; ++j)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(GetUBName(uniformBlock), Names[j])))
            {
                duplicated = GL_TRUE;
                break;
            }
        }
        /* Skip duplicated uniform blocks */
        if (duplicated)
        {
            continue;
        }

        switch (ubUsage)
        {
        case __GL_CHIP_UB_USAGE_USER_DEFINED:
            ++program->userDefUbCount;
            program->ubMaxNameLen = __GL_MAX(program->ubMaxNameLen, (gctSIZE_T)GetUBNameLength(uniformBlock) + 1);
            break;
        case __GL_CHIP_UB_USAGE_DEFAULT:
            ++program->defaultUbCount;
            break;
        case __GL_CHIP_UB_USAGE_PRIVATE:
            ++pgInstance->privateUbCount;
            break;
        default:
            GL_ASSERT(0);
            break;
        }

        /* Store them for later duplicate check */
        Names[(*Index)++] = GetUBName(uniformBlock);
    }

    gcmFOOTER_ARG("return=%u", activeUBs);
    return activeUBs;
}

static gceSTATUS
gcChipProcessUniforms(
    IN     __GLcontext *gc,
    IN OUT __GLprogramObject *programObject,
    IN     gcSHADER Shader,
    IN     GLint Count,
    IN     GLboolean recompiled,
    IN OUT GLint *UserDefIndex,
    IN OUT GLint *BuildInIndex,
    IN OUT GLint *PrivateIndex,
    OUT    GLint *UniformHALIdx2GL,
    OUT    GLuint *numSamplers
    )
{
#if (defined(_DEBUG) || defined(DEBUG))
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
#endif
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLProgramInstance* pgInstance = program->curPgInstance;
    GLint i, j, samplers = 0;
    __GLSLStage stageIdx = __GLSL_STAGE_LAST;
    GLint prevUserDefIndex = *UserDefIndex;
    GLint prevBuiltInIndex = *BuildInIndex;
    GLint prevPrivateIndex = *PrivateIndex;
    gceSTATUS status = gcvSTATUS_OK;
    gctSTRING tmpName = gcvNULL;
    gcSHADER_KIND shaderType = gcSHADER_TYPE_UNKNOWN;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x Shader=0x%x Count=%d recompiled=%d UserDefIndex=0x%x "
                  "BuildInIndex=0x%x PrivateIndex=0x%x UniformHALIdx2GL=0x%x, numSampler=0x%x",
                   gc, programObject, Shader, Count, recompiled, UserDefIndex, BuildInIndex,
                   PrivateIndex, UniformHALIdx2GL, numSamplers);

    stageIdx = gcChipGetShaderStage(Shader);
    gcSHADER_GetType(Shader, &shaderType);

    for (i = 0; i < Count; ++i)
    {
        gcUNIFORM uniform;
        __GLchipSLUniform *slot;
        gctCONST_STRING name;
        gctUINT32 nameLen, maxLen;
        gcSHADER_TYPE dataType;
        gcSHADER_TYPE_KIND category;
        gcSHADER_PRECISION precision;
        gctUINT32 arraySize;
        gctBOOL isArray;
        gctBOOL isRowMajor;
        gctINT offset, arrayStride, matrixStride;
        gctSIZE_T bytes;
        gctUINT16 slotIdx = (gctUINT16)-1;
        GLint location = -1;
        GLint binding = 0;
        __GLchipUniformUsage usage;
        __GLchipUniformSubUsage subUsage;
        gctBOOL duplicate = gcvFALSE;
        GLuint numEntries = 1;
        GLuint entryIdx;
        gctUINT32 regOffset = 0;
        gctUINT32 rows = 0;

        gcmONERROR(gcSHADER_GetUniform(Shader, i, &uniform));

        /* Suppose the uniform->index are assigned in order */
        GL_ASSERT((GLint)GetUniformIndex(uniform) == i);

        /* By default, map HAL uniform to none GL uniform */
        if (UniformHALIdx2GL)
        {
            UniformHALIdx2GL[i] = -1;
        }

        /* Skip inactive and not located uniforms */
        if (!gcChipCheckUniformActive(gc, program, Shader, uniform))
        {
            continue;
        }

        /* Skip uniform struct/block entries */
        if (!isUniformNormal(uniform) &&
            !isUniformBlockMember(uniform) &&
            !isUniformLevelBaseSize(uniform) &&
            !isUniformLodMinMax(uniform) &&
            !isUniformSampleLocation(uniform) &&
            !isUniformMultiSampleBuffers(uniform))
        {
            continue;
        }

        /* uniform block member */
        if (GetUniformBlockID(uniform) != -1)
        {
            gcsUNIFORM_BLOCK uniformBlock;

            gcmONERROR(gcSHADER_GetUniformBlock(Shader,
                                                  GetUniformBlockID(uniform),
                                                  &uniformBlock));

            if (!uniformBlock || GetUBIndex(uniformBlock) == -1)
            {
                continue;
            }

            /* skip non-zeroth element of uniform block array */
            if (gcChipUBGetPrevActiveSibling(Shader, uniformBlock))
            {
                continue;
            }
        }

        gcmONERROR(gcUNIFORM_GetName(uniform, &nameLen, &name));
        usage = gcChipUtilFindUniformUsage(Shader, uniform, &name, &subUsage);
        gcmONERROR(gcUNIFORM_GetTypeEx(uniform, &dataType, &category, &precision, gcvNULL));
        GL_ASSERT(category > gceTK_UNKOWN && category < gceTK_OTHER);
        precision = gcChipUtilFixPrecison(program, precision, dataType, stageIdx);
        isRowMajor = gcChipIsMatrixType(dataType) ? GetUniformIsRowMajor(uniform) : gcvFALSE;
        gcTYPE_GetTypeInfo(dataType, gcvNULL, &rows, gcvNULL);

        offset       = (gctINT)GetUniformOffset(uniform);
        arrayStride  = (gctINT)GetUniformArrayStride(uniform);
        matrixStride = (gctINT)GetUniformMatrixStride(uniform);
        binding      = GetUniformBinding(uniform);

        location     = uniform->location;
        GL_ASSERT(location == -1 || location >= 0);

        numEntries = gcChipGetUniformArrayInfo(uniform, name, &maxLen, &isArray, &arraySize);
        maxLen++; /* count in null-terminator */

        tmpName = (gctSTRING)gc->imports.calloc(gc, maxLen, sizeof(gctCHAR));
        gcoOS_StrCopySafe(tmpName, maxLen, name);

        for (entryIdx = 0; entryIdx < numEntries; ++entryIdx)
        {
            /* Print name string of this entry */
            if (uniform->arrayLengthCount > 1)
            {
                GLuint tmpIdx = entryIdx;
                gctUINT tmpOffset = (gctUINT)gcoOS_StrLen(name, gcvNULL);
                gctINT_PTR arrayIndices = (gctINT_PTR)gc->imports.calloc(gc, uniform->arrayLengthCount - 1, sizeof(gctINT));

                for (j = uniform->arrayLengthCount - 2; j >= 0; --j)
                {
                    arrayIndices[j] = tmpIdx % uniform->arrayLengthList[j];
                    tmpIdx /= uniform->arrayLengthList[j];
                }

                for (j = 0; j < uniform->arrayLengthCount - 1; ++j)
                {
                    gcoOS_PrintStrSafe(tmpName, maxLen, &tmpOffset, "[%d]", arrayIndices[j]);
                }
                nameLen = (gctUINT)gcoOS_StrLen(tmpName, gcvNULL);
                gc->imports.free(gc, arrayIndices);
            }

            switch (usage)
            {
            case __GL_CHIP_UNIFORM_USAGE_USER_DEFINED:
                slotIdx = (gctUINT16)(*UserDefIndex);

                /* Check duplicates for PS uniforms */
                for (j = 0; j < prevUserDefIndex; ++j)
                {
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(tmpName, program->uniforms[j].name)))
                    {
                        duplicate = gcvTRUE;
                        slotIdx = (gctUINT16)j;
                        break;
                    }
                }

                GL_ASSERT(slotIdx < program->userDefUniformCount);
                slot = &program->uniforms[slotIdx];

                if (!duplicate)
                {
                    ++(*UserDefIndex);
                }
                break;

            case __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED:
                slotIdx = (gctUINT16)(*PrivateIndex);

                /* Check duplicates for PS private uniforms */
                for (j = 0; j < prevPrivateIndex; ++j)
                {
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(tmpName, pgInstance->privateUniforms[j].name)))
                    {
                        duplicate = gcvTRUE;
                        slotIdx = (gctUINT16)j;
                        break;
                    }
                }

                GL_ASSERT(slotIdx < pgInstance->privateUniformCount);
                slot = &pgInstance->privateUniforms[slotIdx];

                if (!duplicate)
                {
                    ++(*PrivateIndex);
                }

                /* gcUNIFORM::glUniformIndex think private index was after "userDef + buildin" */
                slotIdx += (gctUINT16)program->activeUniformCount;

                switch (subUsage)
                {
                case __GL_CHIP_UNIFORM_SUB_USAGE_XFB_ENABLE:
                    GL_ASSERT(stageIdx == __GLSL_STAGE_VS);
                    GL_ASSERT(chipCtx->chipFeature.hwFeature.hasHwTFB == GL_FALSE);
                    pgInstance->xfbActiveUniform = slot;
                    break;
                case __GL_CHIP_UNIFORM_SUB_USAGE_XFB_BUFFER:
                    GL_ASSERT(stageIdx == __GLSL_STAGE_VS);
                    GL_ASSERT(pgInstance->xfbBufferCount < __GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS);
                    GL_ASSERT(chipCtx->chipFeature.hwFeature.hasHwTFB == GL_FALSE);
                    pgInstance->xfbBufferUniforms[pgInstance->xfbBufferCount++] = slot;
                    break;
                case __GL_CHIP_UNIFORM_SUB_USAGE_STARTVERTEX:
                    GL_ASSERT(stageIdx == __GLSL_STAGE_VS);
                    break;
                case __GL_CHIP_UNIFORM_SUB_USAGE_VERTEXCOUNT_PER_INSTANCE:
                    GL_ASSERT(stageIdx == __GLSL_STAGE_VS);
                    pgInstance->xfbVertexCountPerInstance = slot;
                    break;
                case __GL_CHIP_UNIFORM_SUB_USAGE_LTC:
                    pgInstance->hasLTC = gcvTRUE;
                    break;
                case __GL_CHIP_UNIFORM_SUB_USAGE_GROUPNUM:
                    pgInstance->groupNumUniformIdx = uniform->physical;
                    /* If this uniform is used in shader, not only LTC, we need this assertion. */
                    if (isUniformUsedInShader(uniform))
                    {
                        GL_ASSERT(pgInstance->groupNumUniformIdx != -1);
                    }
                    break;
                case __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_STATE:
                    pgInstance->advBlendState = slot;
                    break;
                case __GL_CHIP_UNIFORM_SUB_USAGE_RT_IMAGE:
                    GL_ASSERT(stageIdx == __GLSL_STAGE_FS);
                    pgInstance->pLastFragData = slot;
                    break;
                default:
                    break;
                }
                break;

            default:    /* Buildin uniforms */
                slotIdx = (gctUINT16)(*BuildInIndex);

                /* Check duplicates for PS buildin uniforms */
                for (j = program->userDefUniformCount; j < prevBuiltInIndex; ++j)
                {
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(tmpName, program->uniforms[j].name)))
                    {
                        duplicate = gcvTRUE;
                        slotIdx = (gctUINT16)j;
                        break;
                    }
                }

                /* Refresh nameLen in case client change it to spec format */
                nameLen = (gctUINT32)gcoOS_StrLen(tmpName, gcvNULL);

                GL_ASSERT(slotIdx < program->activeUniformCount);
                slot = &program->uniforms[slotIdx];

                if (!duplicate)
                {
                    ++(*BuildInIndex);
                }
                break;
            }

            gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(pgInstance->programState.hints->hwConstRegBases,
                                                              uniform,
                                                              &slot->stateAddress[stageIdx]));

            /* If in recompile stage, only process private uniforms */
            if (recompiled && usage != __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED)
            {
                continue;
            }

            /* make sure the table was not broken due to definition change */
            GL_ASSERT(g_typeInfos[dataType].halType == dataType);
            bytes = g_typeInfos[dataType].size * arraySize;

            slot->halUniform[stageIdx] = uniform;

            if (!duplicate)
            {
                /* Assign primary uniform. */
                gcoOS_Allocate(gcvNULL, nameLen + 1, (gctPOINTER*)&slot->name);
                gcoOS_StrCopySafe(slot->name, nameLen + 1, tmpName);

                slot->nameLen       = nameLen;
                slot->usage         = usage;
                slot->subUsage      = subUsage;
                slot->precision     = precision;
                slot->dataType      = dataType;
                slot->category      = category;
                slot->isArray       = isArray;
                slot->arraySize     = arraySize;
                slot->location      = location;
                slot->binding       = binding;
                slot->entryIdx      = entryIdx;

                slot->numArraySize  = uniform->arrayLengthCount;
                if (slot->numArraySize > 0)
                {
                    gcoOS_Allocate(gcvNULL, slot->numArraySize * gcmSIZEOF(gctINT), (gctPOINTER*)&slot->arraySizes);
                    gcoOS_MemCopy(slot->arraySizes, uniform->arrayLengthList, slot->numArraySize * gcmSIZEOF(gctINT));
                }

                slot->ubIndex       = -1;
                slot->acbIndex      = -1;
                slot->ubUsage       = __GL_CHIP_UB_USAGE_LAST;
                slot->offset        = offset;
                slot->arrayStride   = arrayStride;
                slot->matrixStride  = matrixStride;
                slot->isRowMajor    = isRowMajor;
                slot->regOffset     = regOffset;

                /* no need to allocate storage for uniforms in named ub */
                if (slot->offset == -1)
                {
                    /* Allocate the data array. */
                    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*)&slot->data));
                    gcoOS_ZeroMemory(slot->data, bytes);
                }

                slot->dirty = GL_TRUE;
            }
            else
            {
                /* If any shader stage explicitly specify uniform location */
                if (location != -1)
                {
                    slot->location = location;
                }
            }

            /* For user-defined sampler type uniforms */
            if (category == gceTK_SAMPLER)
            {
                gctUINT32 samplerIdx;
                GLenum texDim = __GL_MAX_TEXTURE_BINDINGS;
                GLboolean isInteger = GL_FALSE;
                gctUINT samplerBase = 0;

                samplerBase = gcHINTS_GetSamplerBaseOffset(pgInstance->programState.hints, Shader);
                gcUNIFORM_GetSampler(uniform, &samplerIdx);

                samplerIdx += samplerBase;
                samplerIdx += (entryIdx * arraySize);   /* Get start sampler index of this entry */

                switch (dataType)
                {
                case gcSHADER_ISAMPLER_2D:
                case gcSHADER_USAMPLER_2D:
                    isInteger = GL_TRUE;
                    /* fall through */
                case gcSHADER_SAMPLER_2D:
                case gcSHADER_SAMPLER_2D_SHADOW:
                    texDim = __GL_TEXTURE_2D_INDEX;
                    break;
                case gcSHADER_ISAMPLER_CUBIC:
                case gcSHADER_USAMPLER_CUBIC:
                    isInteger = GL_TRUE;
                    /* fall throug */
                case gcSHADER_SAMPLER_CUBIC:
                case gcSHADER_SAMPLER_CUBE_SHADOW:
                    texDim = __GL_TEXTURE_CUBEMAP_INDEX;
                    break;
                case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:
                case gcSHADER_USAMPLER_CUBEMAP_ARRAY:
                    isInteger = GL_TRUE;
                    /* fall through */
                case gcSHADER_SAMPLER_CUBEMAP_ARRAY:
                case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:
                    texDim = __GL_TEXTURE_CUBEMAP_ARRAY_INDEX;
                    break;
                case gcSHADER_ISAMPLER_2D_ARRAY:
                case gcSHADER_USAMPLER_2D_ARRAY:
                    isInteger = GL_TRUE;
                    /* fall through */
                case gcSHADER_SAMPLER_2D_ARRAY:
                case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:
                    texDim = __GL_TEXTURE_2D_ARRAY_INDEX;
                    break;
                case gcSHADER_USAMPLER_3D:
                case gcSHADER_ISAMPLER_3D:
                    isInteger = GL_TRUE;
                    /*fall through */
                case gcSHADER_SAMPLER_3D:
                    texDim = __GL_TEXTURE_3D_INDEX;
                    break;
                case gcSHADER_SAMPLER_EXTERNAL_OES:
                    texDim = __GL_TEXTURE_EXTERNAL_INDEX;
                    break;
                case gcSHADER_ISAMPLER_2D_MS:
                case gcSHADER_USAMPLER_2D_MS:
                    isInteger = GL_TRUE;
                    /* fall through */
                case gcSHADER_SAMPLER_2D_MS:
                    texDim = __GL_TEXTURE_2D_MS_INDEX;
                    break;
                case gcSHADER_ISAMPLER_2D_MS_ARRAY:
                case gcSHADER_USAMPLER_2D_MS_ARRAY:
                    isInteger = GL_TRUE;
                    /*fall through */
                case gcSHADER_SAMPLER_2D_MS_ARRAY:
                    texDim = __GL_TEXTURE_2D_MS_ARRAY_INDEX;
                    break;

                case gcSHADER_ISAMPLER_BUFFER:
                case gcSHADER_USAMPLER_BUFFER:
                    isInteger = GL_TRUE;
                case gcSHADER_SAMPLER_BUFFER:
                    texDim = __GL_TEXTURE_BINDING_BUFFER_EXT;
                    break;

                default:
                    GL_ASSERT(0);
                    break;
                }

                if (usage == __GL_CHIP_UNIFORM_USAGE_USER_DEFINED)
                {
                    samplers += (GLint)arraySize;
                    GL_ASSERT(binding == -1 || binding + arraySize <= gc->constants.shaderCaps.maxCombinedTextureImageUnits);

                    if (isUniformUsedAsTexGatherSampler(uniform))
                    {
                        pgInstance->hasTXGatherSample = gcvTRUE;
                    }

                    for (j = 0; j < (GLint)arraySize; ++j)
                    {
                        GL_ASSERT(samplerIdx < (GLint)gcmCOUNTOF(program->samplerMap));
                        program->samplerMap[samplerIdx].shaderType = shaderType;
                        program->samplerMap[samplerIdx].type = dataType;
                        program->samplerMap[samplerIdx].slUniform = slot;
                        program->samplerMap[samplerIdx].uniform    = uniform;
                        program->samplerMap[samplerIdx].arrayIndex = j + (entryIdx * arraySize);
                        program->samplerMap[samplerIdx].texDim     = texDim;
                        program->samplerMap[samplerIdx].stage      = stageIdx;

                        /* By default, all samplers will reference texture unit 0 if not explicitly bound by shader */
                        program->samplerMap[samplerIdx].unit = (binding >= 0) ? (binding + j) : 0;
                        program->samplerMap[samplerIdx].auxiliary = GL_FALSE;
                        program->samplerMap[samplerIdx].subUsage = subUsage;
                        program->samplerMap[samplerIdx].isInteger = isInteger;

                        if (GetUniformResOpFlags(uniform) == gcUNIFORM_RES_OP_FLAG_FETCH ||
                            GetUniformResOpFlags(uniform) == gcUNIFORM_RES_OP_FLAG_FETCH_MS)
                        {
                            __glBitmaskSet(&program->texelFetchSamplerMask, samplerIdx);
                        }

                        if (binding >= 0)
                        {
                            GL_ASSERT(slot->data);
                            ((GLuint*)slot->data)[j] = binding + j;
                        }

                        if (gcSHADER_SAMPLER_2D_SHADOW == dataType       ||
                            gcSHADER_SAMPLER_2D_ARRAY_SHADOW == dataType ||
                            gcSHADER_SAMPLER_CUBE_SHADOW == dataType     ||
                            gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW == dataType)
                        {
                            __glBitmaskSet(&program->shadowSamplerMask, samplerIdx);
                        }
                        programObject->maxUnit = gcmMAX(program->samplerMap[samplerIdx].unit + 1, programObject->maxUnit);
                        ++samplerIdx;
                    }
                }
                else
                {
                    GL_ASSERT(usage == __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED);
                    GL_ASSERT(binding == -1 || binding + arraySize <= gc->constants.shaderCaps.maxCombinedTextureImageUnits);
                    for (j = 0; j < (GLint)arraySize; ++j)
                    {
                        GL_ASSERT(samplerIdx < (GLint)gcmCOUNTOF(pgInstance->extraSamplerMap));
                        pgInstance->extraSamplerMap[samplerIdx].shaderType = shaderType;
                        pgInstance->extraSamplerMap[samplerIdx].type = dataType;
                        pgInstance->extraSamplerMap[samplerIdx].uniform = uniform;
                        pgInstance->extraSamplerMap[samplerIdx].texDim = texDim;
                        pgInstance->extraSamplerMap[samplerIdx].stage = stageIdx;
                        /* By default, all samplers will reference texture unit 0 if not explicitly bound by shader */
                        pgInstance->extraSamplerMap[samplerIdx].unit = (binding >= 0) ? (binding + j) : 0;
                        pgInstance->extraSamplerMap[samplerIdx].auxiliary = (subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_MULTILAYER_SAMPLER);
                        pgInstance->extraSamplerMap[samplerIdx].subUsage = subUsage;
                        pgInstance->extraSamplerMap[samplerIdx].isInteger = isInteger;
                        if(subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_SAMPLER)
                        {
                            pgInstance->rtSampler = samplerIdx;
                        }
                        programObject->maxUnit = gcmMAX(pgInstance->extraSamplerMap[samplerIdx].unit + 1, programObject->maxUnit);
                        ++samplerIdx;
                    }
                }

                programObject->maxSampler = gcmMAX(samplerIdx, programObject->maxSampler);
            }
            /* For user-defined atomic_counter uniforms */
            else if (category == gceTK_ATOMIC && usage == __GL_CHIP_UNIFORM_USAGE_USER_DEFINED)
            {
                GLint acbSlotIdx;
                __GLchipSLAtomCntBuf *acb;

                GL_ASSERT(binding >= 0 && binding < (GLint)gc->constants.shaderCaps.maxAtomicCounterBufferBindings);
                GL_ASSERT(program->acbBinding2SlotIdx && program->acbBinding2NumACs);
                acbSlotIdx = program->acbBinding2SlotIdx[binding];
                GL_ASSERT(acbSlotIdx >= 0 && acbSlotIdx < (GLint)program->acbCount);
                acb = &program->acbs[acbSlotIdx];

                if (!duplicate)
                {
                    gctSIZE_T requiredSize = offset + (isArray ? (arraySize * arrayStride)
                                                               : g_typeInfos[dataType].size);

                    slot->acbIndex = acbSlotIdx;
                    acb->binding = binding;

                    if (acb->dataSize < requiredSize)
                    {
                        acb->dataSize = requiredSize;
                    }

                    if (!acb->uniformIndices)
                    {
                        gcmONERROR(gcoOS_Allocate(gcvNULL,
                                                  program->acbBinding2NumACs[binding] * sizeof(GLuint),
                                                  (gctPOINTER*)&acb->uniformIndices));
                    }
                    GL_ASSERT(acb->activeACs < program->acbBinding2NumACs[binding]);
                    acb->uniformIndices[acb->activeACs++] = slotIdx;

                    if (program->maxActiveACs < acb->activeACs)
                    {
                        program->maxActiveACs = acb->activeACs;
                    }
                }

                acb->halUniform[stageIdx] = uniform;
            }
            else if (category == gceTK_IMAGE)
            {
                gceIMAGE_FORMAT formatQualifier = GetUniformImageFormat(uniform);
                GLuint type = __GL_IMAGE_2D;

                switch (dataType)
                {
                case gcSHADER_IMAGE_2D:
                case gcSHADER_IIMAGE_2D:
                case gcSHADER_UIMAGE_2D:
                    type = __GL_IMAGE_2D;
                    break;
                case gcSHADER_IMAGE_3D:
                case gcSHADER_IIMAGE_3D:
                case gcSHADER_UIMAGE_3D:
                    type = __GL_IMAGE_3D;
                    break;
                case gcSHADER_IMAGE_CUBE:
                case gcSHADER_IIMAGE_CUBE:
                case gcSHADER_UIMAGE_CUBE:
                    type = __GL_IMAGE_CUBE;
                    break;
                case gcSHADER_IMAGE_CUBEMAP_ARRAY:
                case gcSHADER_IIMAGE_CUBEMAP_ARRAY:
                case gcSHADER_UIMAGE_CUBEMAP_ARRAY:
                    type = __GL_IMAGE_CUBE_ARRAY;
                    break;
                case gcSHADER_IMAGE_2D_ARRAY:
                case gcSHADER_IIMAGE_2D_ARRAY:
                case gcSHADER_UIMAGE_2D_ARRAY:
                    type = __GL_IMAGE_2D_ARRAY;
                    break;
                case gcSHADER_IMAGE_BUFFER:
                case gcSHADER_IIMAGE_BUFFER:
                case gcSHADER_UIMAGE_BUFFER:
                    type = __GL_IMAGE_BUFFER;
                    break;
                default:
                    GL_ASSERT(0);
                    break;
                }

                GL_ASSERT(program->imageUniformCount + pgInstance->extraImageUniformCount + arraySize <= gc->constants.shaderCaps.maxCombinedImageUniform);
                GL_ASSERT(binding != -1 && (binding + arraySize) <= gc->constants.shaderCaps.maxImageUnit);

                if (!duplicate)
                {
                    if (usage == __GL_CHIP_UNIFORM_USAGE_USER_DEFINED)
                    {
                        program->imageUniformCount += arraySize;
                        for (j = 0; j < (GLint)arraySize; ++j)
                        {
                            __GLchipImageUnit2Uniform *pImageUnit2Uniform = &program->imageUnit2Uniform[binding+j];
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].slUniform = slot;
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].type = type;
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].formatQualifier = formatQualifier;
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].arrayIndex = j;
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].auxiliary = GL_FALSE;
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].subUsage = subUsage;
                            pImageUnit2Uniform->numUniform++;
                        }
                    }
                    else if (subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_MULTILAYER_IMAGE)
                    {
                        GL_ASSERT(usage == __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED);
                        pgInstance->extraImageUniformCount += arraySize;
                        for (j = 0; j < (GLint)arraySize; ++j)
                        {
                            __GLchipImageUnit2Uniform *pImageUnit2Uniform = &pgInstance->extraImageUnit2Uniform[binding+j];
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].slUniform = slot;
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].type = type;
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].formatQualifier = formatQualifier;
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].arrayIndex = j;
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].auxiliary = (subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_MULTILAYER_IMAGE);
                            pImageUnit2Uniform->uniforms[pImageUnit2Uniform->numUniform].subUsage = subUsage;
                            pImageUnit2Uniform->numUniform++;
                        }
                    }
                }
            }

            /* Hal uniform only ref to 1st entry of GLuniform */
            if (entryIdx == 0)
            {
                if (UniformHALIdx2GL)
                {
                    UniformHALIdx2GL[i] = (GLint)slotIdx;
                }

                /* the app might link program multiple times */
                GL_ASSERT(GetUniformGlUniformIndex(uniform) == ((gctINT16)-1) ||
                          GetUniformGlUniformIndex(uniform) == (gctINT16)slotIdx ||
                          usage == __GL_CHIP_UNIFORM_USAGE_COMPILER_GENERATED);

                SetUniformGlUniformIndex(uniform, slotIdx);
            }

            regOffset += ((4 * rows * arraySize) << 2);

            /* Increase offset of uniform block */
            if (offset != -1)
            {
                offset += (arrayStride * arraySize);
            }

            if (location != -1)
            {
                location += arraySize;
            }
        }
        gc->imports.free(gc, tmpName);
        tmpName = gcvNULL;
    }

OnError:
    /* recompile stage will not update the sampler count */
    if (numSamplers && !recompiled)
    {
        *numSamplers = gcmIS_SUCCESS(status) ? (GLuint)samplers : 0;
    }
    if (tmpName != gcvNULL)
    {
        gc->imports.free(gc, tmpName);
    }
    gcmFOOTER();
    return status;
}

static GLvoid
gcChipProcessUniformBlocks(
    IN     __GLcontext *gc,
    IN OUT __GLprogramObject *programObject,
    IN     gcSHADER Shader,
    IN     GLint Count,
    IN     GLboolean recompiled,
    IN     GLint *UniformHALIdx2GL,
    IN OUT GLint *UserDefIndex,
    IN OUT GLint *DefaultIndex,
    IN OUT GLint *PrivateIndex
    )
{
    GLint i, j;
    __GLSLStage stageIdx = __GLSL_STAGE_LAST;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLProgramInstance* pgInstance = program->curPgInstance;
    GLint prevUserDefIndex = *UserDefIndex;
    GLint prevDefaultIndex = *DefaultIndex;
    GLint prevPrivateIndex = *PrivateIndex;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x Shader=0x%x Count=%d recompiled = %d"
                  "UniformHALIdx2GL=0x%x UserDefIndex=0x%x DefaultIndex=0x%x PrivateIndex=0x%x",
                  gc, programObject, Shader, Count, recompiled, UniformHALIdx2GL, UserDefIndex,
                  DefaultIndex, PrivateIndex);

    stageIdx = gcChipGetShaderStage(Shader);

    for (i = 0; i < Count; ++i)
    {
        GLint ubSlotIdx                = -1;
        GLuint binding                 = 0;
        __GLchipSLUniformBlock *ubSlot = gcvNULL;
        gcsUNIFORM_BLOCK uniformBlock;
        gcsUNIFORM_BLOCK prevActiveUB  = gcvNULL;
        gcUNIFORM ubUniform            = gcvNULL;
        GLbitfield mapFlags            = 0;
        gctUINT32 numUniforms          = 0;
        GLuint totalEntries            = 0;
        gctUINT32 prevActiveUniforms   = 0;
        gctBOOL duplicate              = gcvFALSE;
        __GLchipUbUsage ubUsage;

        gcmVERIFY_OK(gcSHADER_GetUniformBlock(Shader, i, &uniformBlock));
        if (!uniformBlock || GetUBBlockIndex(uniformBlock) == -1)
        {
            continue;
        }

        gcmVERIFY_OK(gcSHADER_GetUniform(Shader, GetUBIndex(uniformBlock), &ubUniform));

        /* Skip inactive uniform blocks */
        if (isUniformInactive(ubUniform) ||
            (GetUniformArraySize(ubUniform) > 1 && GetUBArrayIndex(uniformBlock) >= GetUniformUsedArraySize(ubUniform)))
        {
            continue;
        }

        if (GetUniformPhysical(ubUniform) != -1)
        {
            /* Part of the UB was mapped to video memory. */
            mapFlags |= gcdUB_MAPPED_TO_MEM;
        }

        ubUsage = gcChipUtilFindUbUsage(GetUniformName(ubUniform));
        binding = GetUBBinding(uniformBlock);
        switch (ubUsage)
        {
        case __GL_CHIP_UB_USAGE_USER_DEFINED:
            ubSlotIdx = *UserDefIndex;
            /* Tried to found whether the UniformBlock was defined in previous shader */
            for (j = 0; j < prevUserDefIndex; ++j)
            {
                if (gcmIS_SUCCESS(gcoOS_StrCmp(GetUBName(uniformBlock), program->uniformBlocks[j].name)))
                {
                    /* If both VS and PS define the same UB, they must report same dataSize */
                    if (GetUBBlockSize(uniformBlock) != program->uniformBlocks[j].dataSize ||
                        binding                      != program->uniformBlocks[j].binding)
                    {
                        gcmFATAL("%s(%d): PS didn't report same UB dataSize or binding as VS did", __FUNCTION__, __LINE__);
                    }

                    duplicate = GL_TRUE;
                    ubSlotIdx = j;
                    break;
                }
                /* Unlike uniforms, HAL compiler guaranteed vs/ps match for uniform blocks */
            }

            GL_ASSERT(ubSlotIdx < program->userDefUbCount);
            ubSlot = &program->uniformBlocks[ubSlotIdx];

            if (!duplicate)
            {
                ++(*UserDefIndex);
            }
            break;

        case __GL_CHIP_UB_USAGE_DEFAULT:
            ubSlotIdx = *DefaultIndex;
            for (j = program->userDefUbCount; j < prevDefaultIndex; ++j)
            {
                if (gcmIS_SUCCESS(gcoOS_StrCmp(GetUBName(uniformBlock), program->uniformBlocks[j].name)))
                {
                    duplicate = GL_TRUE;
                    ubSlotIdx = j;
                    break;
                }
            }

            GL_ASSERT(ubSlotIdx < program->totalUbCount);
            ubSlot = &program->uniformBlocks[ubSlotIdx];

            if (!duplicate)
            {
                ++(*DefaultIndex);
            }
            break;

        case __GL_CHIP_UB_USAGE_PRIVATE:
            /* Private UBO should always be mapped to vidmem */
            GL_ASSERT(GetUniformPhysical(ubUniform) != -1);

            ubSlotIdx = *PrivateIndex;
            for (j = 0; j < prevPrivateIndex; ++j)
            {
                if (gcmIS_SUCCESS(gcoOS_StrCmp(GetUBName(uniformBlock), pgInstance->privateUBs[j].name)))
                {
                    duplicate = GL_TRUE;
                    ubSlotIdx = j;
                    break;
                }
            }

            GL_ASSERT(ubSlotIdx < pgInstance->privateUbCount);
            ubSlot = &pgInstance->privateUBs[ubSlotIdx];

            if (!duplicate)
            {
                ++(*PrivateIndex);
            }
            break;

        default:
            GL_ASSERT(0);
            break;
        }

        gcmVERIFY_OK(gcSHADER_ComputeUniformPhysicalAddress(pgInstance->programState.hints->hwConstRegBases,
                                                            ubUniform,
                                                            &ubSlot->stateAddress[stageIdx]));

        /* If in recompile stage, only process private uniforms */
        if (recompiled && ubUsage != __GL_CHIP_UB_USAGE_PRIVATE)
        {
            continue;
        }

        gcmVERIFY_OK(gcSHADER_GetUniformBlockUniformCount(Shader, uniformBlock, &numUniforms));
        prevActiveUB = gcChipUBGetPrevActiveSibling(Shader, uniformBlock);

        /* Private UBO doesn't contain uniform and cannot be arrays */
        GL_ASSERT((ubUsage != __GL_CHIP_UB_USAGE_PRIVATE) ||
                  (!prevActiveUB && numUniforms == 0));

        /* Check how many GLuniform entries will be reported for this block */
        for (j = 0; j < (GLint)numUniforms; ++j)
        {
            gcUNIFORM uniform = gcvNULL;
            gcmVERIFY_OK(gcSHADER_GetUniformBlockUniform(Shader, uniformBlock, j, &uniform));

            /* Skip inactive uniforms */
            if (!uniform || isUniformInactive(uniform))
            {
                continue;
            }

            totalEntries += gcChipGetUniformArrayInfo(uniform, gcvNULL, gcvNULL, gcvNULL, gcvNULL);
        }

        /* Resize the index buffer if needed */
        prevActiveUniforms = (gctUINT32)ubSlot->activeUniforms;
        if (ubSlot->indexSize < prevActiveUniforms + totalEntries)
        {
            GLuint *prevInidces = ubSlot->uniformIndices;
            gctSIZE_T prevIndexSize = ubSlot->indexSize;

            ubSlot->indexSize = prevActiveUniforms + totalEntries;

            ubSlot->uniformIndices = (GLuint*)gc->imports.calloc(gc, 1, ubSlot->indexSize * sizeof(GLuint));

            if (prevIndexSize)
            {
                __GL_MEMCOPY(ubSlot->uniformIndices, prevInidces, prevIndexSize * sizeof(GLuint));
                gc->imports.free(gc, prevInidces);
            }
        }

        /* check for non-zeroth uniform block array element */
        /* copy the uniform indices from sibling of uniform block array element into it */
        if (prevActiveUB)
        {
            GLint siblingUbSlotIdx                = 0;
            __GLchipSLUniformBlock *siblingUbSlot = gcvNULL;

            /* Whether UB[1] was mapped to reg should be check independent from UB[0]. */
            for (j = 0; j < (GLint)numUniforms; ++j)
            {
                gcUNIFORM uniform = gcvNULL;

                gcmVERIFY_OK(gcSHADER_GetUniformBlockUniform(Shader, uniformBlock, j, &uniform));

                /* Skip inactive uniforms */
                if (!uniform || isUniformInactive(uniform))
                {
                    continue;
                }

                if (GetUniformPhysical(uniform) != -1)
                {
                    /* Part of the UB was mapped to reg/state buffer. */
                    mapFlags |= gcdUB_MAPPED_TO_REG;
                }
            }

            siblingUbSlotIdx = __glChipGetUniformBlockIndex(gc,
                                                            programObject,
                                                            GetUBName(prevActiveUB));
            gcmASSERT(siblingUbSlotIdx != -1);
            siblingUbSlot = &program->uniformBlocks[siblingUbSlotIdx];

            ubSlot->activeUniforms = siblingUbSlot->activeUniforms;
            if (siblingUbSlot->activeUniforms)
            {
                __GL_MEMCOPY(ubSlot->uniformIndices,
                             siblingUbSlot->uniformIndices,
                             siblingUbSlot->activeUniforms * sizeof(GLuint));
            }
        }
        else
        {
            /* Iterate all active uniforms in the uniform block. */
            for (j = 0; j < (GLint)numUniforms; ++j)
            {
                GLint uSlotIdx = 0;
                GLuint k, entries;
                gcUNIFORM uniform = gcvNULL;

                gcmVERIFY_OK(gcSHADER_GetUniformBlockUniform(Shader, uniformBlock, j, &uniform));

                /* Skip inactive uniforms */
                if (!uniform || isUniformInactive(uniform))
                {
                    continue;
                }

                if (GetUniformPhysical(uniform) != -1)
                {
                    /* Part of the UB was mapped to reg/state buffer. */
                    mapFlags |= gcdUB_MAPPED_TO_REG;
                }

                uSlotIdx = UniformHALIdx2GL[GetUniformIndex(uniform)];

                /* The hal uniform wasn't mapped to GLuniform entries */
                if (uSlotIdx == -1)
                {
                    continue;
                }

                entries = gcChipGetUniformArrayInfo(uniform, gcvNULL, gcvNULL, gcvNULL, gcvNULL);

                for (k = 0; k < entries; ++k, ++uSlotIdx)
                {
                    gctSIZE_T l              = 0;
                    GLboolean duplicated     = GL_FALSE;
                    __GLchipSLUniform *uSlot = &program->uniforms[uSlotIdx];

                    uSlot->ubIndex  = ubSlotIdx;
                    uSlot->ubUsage  = ubUsage;
                    uSlot->entryIdx = k;

                    /* Per Spec, stride should be -1 if not in user-defined UB */
                    if (ubUsage != __GL_CHIP_UB_USAGE_USER_DEFINED)
                    {
                        uSlot->arrayStride = -1;
                        uSlot->matrixStride = -1;
                    }

                    /* Whether block already recorded the uniform in previous stage? */
                    for (l = 0; l < prevActiveUniforms; ++l)
                    {
                        __GLchipSLUniform *uSlotPrev = &program->uniforms[ubSlot->uniformIndices[l]];
                        if (gcmIS_SUCCESS(gcoOS_StrCmp(uSlot->name, uSlotPrev->name)))
                        {
                            duplicated = GL_TRUE;
                            break;
                        }
                    }

                    if (!duplicated)
                    {
                        ubSlot->uniformIndices[ubSlot->activeUniforms++] = uSlotIdx;
                    }
                }
            }
        }

        if (ubUsage == __GL_CHIP_UB_USAGE_USER_DEFINED)
        {
            program->maxActiveUniforms = __GL_MAX(program->maxActiveUniforms, (GLint)ubSlot->activeUniforms);
        }

        ubSlot->halUB[stageIdx]      = uniformBlock;
        ubSlot->halUniform[stageIdx] = ubUniform;
        ubSlot->refByStage[stageIdx] = HasIBIFlag(GetUBInterfaceBlockInfo(uniformBlock), gceIB_FLAG_STATICALLY_USED);
        ubSlot->mapFlags[stageIdx]   = mapFlags;
        ubSlot->mapFlag             |= mapFlags;

        if (mapFlags & gcdUB_MAPPED_TO_REG)
        {
            program->ubMapToReg = gcvTRUE;
        }


        if (!duplicate)
        {
            ubSlot->name        = GetUBName(uniformBlock);
            ubSlot->nameLen     = GetUBNameLength(uniformBlock);
            ubSlot->dataSize    = GetUBBlockSize(uniformBlock);
            ubSlot->usage       = ubUsage;
            ubSlot->binding     = binding;
        }
    }
    gcmFOOTER_NO();
}

static GLvoid
gcChipDumpGLUniform(
    __GLchipSLUniform *Uniform,
    gcSHADER_TYPE     Type,
    gctSIZE_T         Count,
    GLuint            Index
    )
{
    gctUINT32 rows;
    gctUINT32 components;
    GLfloat   *data = (GLfloat *) Uniform->data + Index;
    gctSIZE_T i;
    gctBOOL   singleValue;
    gctCHAR   buffer[512];
    gctUINT   offset      = 0;
    gctBOOL   printArrayIndex = 0;
    gctSIZE_T arrayIndex;
    gctCONST_STRING typeName;

    gcmHEADER_ARG("Uniform=0x%x Type=%d Count=%d Index=%d", Uniform, Type, Count, Index);


    gcmASSERT(Uniform != gcvNULL && Type < gcSHADER_TYPE_COUNT);

    gcTYPE_GetTypeInfo(Type, &components, &rows, &typeName);
    singleValue = (rows*components*Count == 1);
    arrayIndex = Index / (rows * components);

    /* print uniform type */
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                           "uniform %s", typeName));

    /* print array size if it has one */
    if (Uniform->arraySize > 1)
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "[%d]", Uniform->arraySize));
        printArrayIndex = gcvTRUE;
    }

    /* print uniform name */
    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                           " %s", Uniform->name));

    /* print index value */
    if (printArrayIndex)
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               "[%d]", arrayIndex));
    }

    gcmVERIFY_OK(
        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset," = "));

    if (Uniform->category == gceTK_SAMPLER ||
        Uniform->category == gceTK_IMAGE)
    {
        gcmVERIFY_OK(
            gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                               " %6.4f;", *data));
        gcoOS_Print("%s", buffer);
    }
    else
    {
        if (!singleValue)
            gcmVERIFY_OK(
                gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                   "{ \n"));

        for (i=0; i < Count; ++i)
        {
            gctSIZE_T j;
            if (Count > 1)
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "\t\t{"));

            /* print rows */
            for (j=0; j < rows; j++)
            {
                gctSIZE_T k;
                if (rows > 1)
                   gcmVERIFY_OK(
                       gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                          "\t\t\t{"));
                /* print print components */
                for (k = 0; k < components; k++)
                {
                    if (Uniform->category == gceTK_FLOAT)
                    {
                        gcmVERIFY_OK(
                           gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                              " %10.6f", *data++));
                    }
                    else
                    {
                        gcmVERIFY_OK(
                           gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                              " 0x%x", *((gctUINT*)data)));
                        data++;
                    }
                    if (k < components-1)
                       gcmVERIFY_OK(
                           gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                              ","));
                }
                if (rows > 1)
                    gcmVERIFY_OK(
                        gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                           "  },\n"));
            }

            if (Count > 1)
            {
                gcmVERIFY_OK(
                    gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                       "} "));
                if (i != Count -1)
                   gcmVERIFY_OK(
                       gcoOS_PrintStrSafe(buffer, gcmSIZEOF(buffer), &offset,
                                          ",\n\t\t"));
            }
            gcoOS_Print("%s", buffer);
            offset = 0;
        }
        if (!singleValue)
            gcoOS_Print("};\n");
        else
            gcoOS_Print(";\n");
    }

    gcmFOOTER_NO();
    return;
}

__GL_INLINE GLboolean
gcChipIsUniformLocatable(
    __GLchipSLUniform *uniform
    )
{
    /* User-defined uniforms which are of sampler or image types, or resident in defaultUBO. */
    return uniform->usage == __GL_CHIP_UNIFORM_USAGE_USER_DEFINED &&
           uniform->category != gceTK_ATOMIC &&
           (uniform->ubIndex == -1 ||
            uniform->ubUsage == __GL_CHIP_UB_USAGE_DEFAULT ||
            uniform->category == gceTK_SAMPLER ||
            uniform->category == gceTK_IMAGE);
}

__GL_INLINE __GLchipSLUniform *
gcChipGetUniformByName(
    __GLcontext *gc,
    __GLchipSLProgram *program,
    const GLchar *name,
    gctSIZE_T nameLen
)
{
    GLint i;
    __GLchipSLUniform *uniform = gcvNULL;

    gcmHEADER_ARG("gc=0x%x program=0x%x name=%s", gc, program, name);

    GL_ASSERT(name);

    if (nameLen == (gctSIZE_T)-1)
    {
        gcoOS_StrLen(name, &nameLen);
    }

    for (i = 0; i < program->userDefUniformCount; ++i)
    {
        __GLchipSLUniform *iter = &program->uniforms[i];

        if (gcChipIsUniformLocatable(iter) &&
            nameLen == iter->nameLen &&
            gcmIS_SUCCESS(gcoOS_MemCmp(name, iter->name, nameLen))
           )
        {
            uniform = iter;
            break;
        }
    }

    gcmFOOTER_ARG("return=0x%x", -1);
    return uniform;
}

static gceSTATUS
gcChipBuildUniformLocationMapping(
    __GLcontext *gc,
    __GLchipSLProgram *program,
    gctSTRING logBuf,
    gctUINT *logOffset
    )
{
    GLint i, j, loc;
    gctSIZE_T bytes;
    __GLchipSLUniform *uniform;
    __GLchipSLUniform **tmpLoc2Uniform = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x program=0x%x", gc, program);

    /* Allocate location table with maximum allowed size */
    bytes = gc->constants.shaderCaps.maxUniformLocations * gcmSIZEOF(__GLchipSLUniform*);
    gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*)&tmpLoc2Uniform));
    gcoOS_ZeroMemory(tmpLoc2Uniform, bytes);

    /* Step1: walk through for explicit shader assigned locations */
    for (i = 0; i < program->userDefUniformCount; ++i)
    {
        uniform = &program->uniforms[i];

        if (gcChipIsUniformLocatable(uniform) && uniform->location >= 0)
        {
            if (uniform->location + (GLint)uniform->arraySize > gc->constants.shaderCaps.maxUniformLocations)
            {
                gcmONERROR(gcoOS_PrintStrSafe(logBuf, __GLSL_LOG_INFO_SIZE, logOffset,
                    "LinkShaders: explicit uniform %s (size=%d) location %d exceeds max allowed %d\n",
                     uniform->name, uniform->arraySize, uniform->location, gc->constants.shaderCaps.maxUniformLocations));
                gcmTRACE(gcvLEVEL_ERROR, "LinkShaders: explicit uniform location exceeds");
                gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
            }

            for (j = 0; j < (GLint)uniform->arraySize; ++j)
            {
                loc = uniform->location + j;
                if (tmpLoc2Uniform[loc])
                {
                    gcmONERROR(gcoOS_PrintStrSafe(logBuf, __GLSL_LOG_INFO_SIZE, logOffset,
                        "LinkShaders: explicit uniform %s[%d] location %d was already occupied by uniform %s\n",
                        uniform->name, j, loc, tmpLoc2Uniform[loc]->name));
                    gcmTRACE(gcvLEVEL_ERROR, "LinkShaders: explicit uniform locations conflicts");
                    gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
                }
                tmpLoc2Uniform[loc] = uniform;
            }
            program->maxLocation = __GL_MAX(program->maxLocation, uniform->location + (GLint)uniform->arraySize);
        }
    }

    /* Step2: walk through for implicit uniform locations */
    for (i = 0; i < program->userDefUniformCount; ++i)
    {
        uniform = &program->uniforms[i];

        if (gcChipIsUniformLocatable(uniform) && uniform->location == -1)
        {
            /* Find continuous available locations */
            for (loc = 0; loc < gc->constants.shaderCaps.maxUniformLocations; loc += (j + 1))
            {
                for (j = 0; j < (GLint)uniform->arraySize; ++j)
                {
                    if (tmpLoc2Uniform[loc + j])
                    {
                        break;
                    }
                }

                /* Found them out */
                if (j == (GLint)uniform->arraySize)
                {
                    break;
                }
            }

            if (loc >= gc->constants.shaderCaps.maxUniformLocations)
            {
                gcmONERROR(gcoOS_PrintStrSafe(logBuf, __GLSL_LOG_INFO_SIZE, logOffset,
                    "LinkShaders: cannot find continuous locations for uniform %s\n",
                    uniform->name));
                gcmTRACE(gcvLEVEL_ERROR, "LinkShaders: cannot find continuous uniform locations");
                gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
            }

            uniform->location = loc;
            for (j = 0; j < (GLint)uniform->arraySize; ++j)
            {
                tmpLoc2Uniform[uniform->location + j] = uniform;
            }
            program->maxLocation = __GL_MAX(program->maxLocation, uniform->location + (GLint)uniform->arraySize);
        }
    }

    /* Step3: copy useful location mapping table to program object */
    if (program->maxLocation > 0)
    {
        bytes = program->maxLocation * gcmSIZEOF(__GLchipSLUniform*);
        gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*)&program->loc2Uniform));
        gcoOS_MemCopy(program->loc2Uniform, tmpLoc2Uniform, bytes);
    }

OnError:
    if (tmpLoc2Uniform)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, tmpLoc2Uniform));
    }
    gcmFOOTER();
    return status;
}

/* Return how many uniform entries will be reported to app */
__GL_INLINE GLuint
gcChipGetBufVariableArrayInfo(
    gcVARIABLE variable,
    gctUINT *maxNameLen,    /* max possible name len of this entry, exclude bottom-level "[0]" */
    gctBOOL *isArray,
    gctUINT *arraySize
)
{
    gctINT j, startLevel = isVariableStructMember(variable) ? 0 : 1;
    gctUINT32 length = GetVariableNameLength(variable);
    GLuint entries = 1;

    /* Multiple entries will be reported for array of arrays.
    ** If a variable is an array, or array of array, its name length should be added
    ** with several dim of array indices, like "name[0][x]...[x][0]".
    ** But if this array is a struct member, then we need to split to "name[x][x]...[x][]".
    */
    for (j = startLevel; j < variable->arrayLengthCount - 1; ++j)
    {
        gctUINT decimalLen = 1;
        gctINT arrayLen = variable->arrayLengthList[j];

        GL_ASSERT(arrayLen > 0);
        entries *= arrayLen;

        arrayLen--;    /* Get max arrayIndex */
        while (arrayLen >= 10)
        {
            ++decimalLen;
            arrayLen /= 10;
        }

        length += (decimalLen + 2);
    }

    if (maxNameLen)
    {
        *maxNameLen = length;
    }

    if (isArray)
    {
        *isArray = variable->arrayLengthCount > 0 ? gcvTRUE : gcvFALSE;
    }

    if (arraySize)
    {
        *arraySize = variable->arrayLengthCount > 0
                   ? (variable->arrayLengthList[variable->arrayLengthCount - 1] > 0 ?
                      variable->arrayLengthList[variable->arrayLengthCount - 1] : 0)
                   : 1;
    }

    return entries;
}

static GLuint
gcChipCountStorageBlocks(
    IN     __GLchipSLProgram *program,
    IN     gcSHADER Shader,
    IN     GLint Count,
    IN     GLint *Index,
    IN OUT gctCONST_STRING *Names
    )
{
    GLint i, j;
    GLint prevIdx = *Index;
    GLuint activeSBs = 0;

    gcmHEADER_ARG("program=0x%x Shader=0x%x Count=%d Index=0x%x Names=0x%x",
                  program, Shader, Count, Index, Names);

    for (i = 0; i < Count; ++i)
    {
        GLboolean duplicated = GL_FALSE;
        gcsSTORAGE_BLOCK storageBlock;
        __GLchipSsbUsage sbUsage;
        gcUNIFORM sbUniform;
        gctSTRING sbName;

        gcmVERIFY_OK(gcSHADER_GetStorageBlock(Shader, i, &storageBlock));

        if (!storageBlock || GetSBBlockIndex(storageBlock) == -1)
        {
            continue;
        }

        gcSHADER_GetUniform(Shader, GetSBIndex(storageBlock), &sbUniform);

        /* Skip inactive storage blocks */
        if (isUniformInactive(sbUniform))
        {
            continue;
        }

        sbName = GetSBName(storageBlock);
        /* Check duplicated storage blocks in previous shader stage */
        for (j = 0; j < prevIdx; ++j)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(sbName, Names[j])))
            {
                duplicated = GL_TRUE;
                break;
            }
        }

        /* Skip duplicated storage blocks */
        if (duplicated)
        {
            continue;
        }

        sbUsage = gcChipUtilFindSsbUsage(sbName);
        switch (sbUsage)
        {
        case __GL_CHIP_SSB_USAGE_USERDEF:
            program->ssbMaxNameLen = __GL_MAX(program->ssbMaxNameLen, (gctSIZE_T)GetSBNameLength(storageBlock) + 1);
            ++program->userDefSsbCount;
            ++activeSBs;
            break;
        case __GL_CHIP_SSB_USAGE_SHAREDVAR:
        case __GL_CHIP_SSB_USAGE_EXTRAREG:
            ++program->privateSsbCount;
            break;
        default:
            GL_ASSERT(0);
            break;
        }

        /* Store them for later duplicate check */
        Names[(*Index)++] = GetUBName(storageBlock);
    }

    gcmFOOTER_ARG("return=%u", activeSBs);
    return activeSBs;
}

static GLvoid
gcChipUpdateBaseAddrUniformForStorageBlocks(
    IN     __GLcontext *gc,
    IN OUT __GLprogramObject *progObj,
    IN     gcSHADER Shader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgram *program = (__GLchipSLProgram *)progObj->privateData;
    __GLchipSLStorageBlock *sbSlot = gcvNULL;
    __GLSLStage stageIdx = __GLSL_STAGE_LAST;
    gcUNIFORM newBaseAddrUniform = gcvNULL;
    GLuint i;

    gcmHEADER_ARG("gc=0x%x progObj=0x%x Shader=0x%x", gc, progObj, Shader);

    stageIdx = gcChipGetShaderStage(Shader);

    for (i = 0; i < program->totalSsbCount; i++)
    {
        sbSlot = &program->ssbs[i];

        if (sbSlot == gcvNULL || sbSlot->halUniform[stageIdx] == gcvNULL)
        {
            continue;
        }

        /* Get the new base address uniform. */
        gcmONERROR(gcSHADER_GetUniform(Shader,
                                       GetUniformIndex(sbSlot->halUniform[stageIdx]),
                                       &newBaseAddrUniform));
        gcmASSERT(GetUniformKind(newBaseAddrUniform) == gcvUNIFORM_KIND_STORAGE_BLOCK_ADDRESS);

        /* Set the new base address uniform. */
        sbSlot->halUniform[stageIdx] = newBaseAddrUniform;

        gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(program->curPgInstance->programState.hints->hwConstRegBases,
                                                          newBaseAddrUniform,
                                                          &sbSlot->stateAddress[stageIdx]));
    }

OnError:
    gcmFOOTER_NO();
}

static GLvoid
gcChipProcessStorageBlocks(
    IN     __GLcontext *gc,
    IN OUT __GLprogramObject *progObj,
    IN     gcSHADER Shader,
    IN     GLint Count,
    IN OUT GLint *UserDefIndex,
    IN OUT GLint *PrivateIndex
    )
{
    GLint i, j;
    gctSTRING tmpName = gcvNULL;
    __GLSLStage stageIdx = __GLSL_STAGE_LAST;
    GLint prevUserDefIdx = *UserDefIndex;
    GLint prevPrivateIdx = *PrivateIndex;
    __GLchipSLProgram *program = (__GLchipSLProgram *)progObj->privateData;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x progObj=0x%x Shader=0x%x Count=%d UserDefIndex=0x%x PrivateIndex=0x%x",
                  gc, progObj, Shader, Count, UserDefIndex, PrivateIndex);

    stageIdx = gcChipGetShaderStage(Shader);

    for (i = 0; i < Count; ++i)
    {
        GLint sbSlotIdx = -1;
        __GLchipSLStorageBlock *sbSlot = gcvNULL;
        gctBOOL duplicate = gcvFALSE;
        gcsSTORAGE_BLOCK storageBlock;
        gcsSTORAGE_BLOCK prevActiveSSB = gcvNULL;
        __GLchipSsbUsage sbUsage;
        gcUNIFORM sbUniform;
        gctSTRING sbName;

        gcmONERROR(gcSHADER_GetStorageBlock(Shader, i, &storageBlock));
        if (!storageBlock || GetSBBlockIndex(storageBlock) == -1)
        {
            continue;
        }

        gcmONERROR(gcSHADER_GetUniform(Shader, GetSBIndex(storageBlock), &sbUniform));

        /* Skip inactive storage block */
        if (isUniformInactive(sbUniform))
        {
            continue;
        }

        sbName = GetSBName(storageBlock);
        sbUsage = gcChipUtilFindSsbUsage(sbName);

        switch (sbUsage)
        {
        case __GL_CHIP_SSB_USAGE_USERDEF:
            sbSlotIdx = *UserDefIndex;

            /* Check duplicated storage blocks in previous shader stage */
            for (j = 0; j < prevUserDefIdx; ++j)
            {
                if (gcmIS_SUCCESS(gcoOS_StrCmp(sbName, program->ssbs[j].name)))
                {
                   /* If multiple shaders define the same UB, they must report same dataSize */
                    if (GetSBBlockSize(storageBlock) != program->ssbs[j].dataSize ||
                        GetSBBinding(storageBlock)   != (GLint)program->ssbs[j].binding)
                    {
                        gcmFATAL("%s(%d): multiple declaration didn't report same SB dataSize or binding", __FUNCTION__, __LINE__);
                        gcmONERROR(gcvSTATUS_VARYING_TYPE_MISMATCH);
                    }
                    duplicate = GL_TRUE;
                    sbSlotIdx = j;
                    break;
                }
            }

            if (!duplicate)
            {
                ++(*UserDefIndex);
            }

            GL_ASSERT(sbSlotIdx < (GLint)program->userDefSsbCount);
            sbSlot = &program->ssbs[sbSlotIdx];

            /* Skip check for non-zeroth storage block array element.
            ** It index table will be copied form pre-sibling later.
            */
            prevActiveSSB = gcChipSSBGetPrevActiveSibling(Shader, storageBlock);
            if (prevActiveSSB)
            {
                sbSlot->preSiblingIdx = __glChipGetProgramResourceIndex(gc, progObj, GL_SHADER_STORAGE_BLOCK, GetSBName(prevActiveSSB));
                gcmASSERT(sbSlot->preSiblingIdx != -1);
            }
            else
            {
                gctUINT numBVs = 0;
                gctUINT prevActiveBVs = 0;
                gctUINT totalEntries = 0;
                gcVARIABLE variable = gcvNULL;

                gcmONERROR(gcSHADER_GetStorageBlockVariableCount(Shader, storageBlock, &numBVs));
                GL_ASSERT(numBVs > 0);

                for (j = 0; j < (GLint)numBVs; ++j)
                {
                    gcmONERROR(gcSHADER_GetStorageBlockVariable(Shader, storageBlock, j, &variable));
                    /* Skip inactive variables */
                    if (!variable || isVariableInactive(variable))
                    {
                        continue;
                    }
                    /* Skip auxiliary variables */
                    if (!isVariableSimple(variable))
                    {
                        continue;
                    }
                    totalEntries += gcChipGetBufVariableArrayInfo(variable, gcvNULL, gcvNULL, gcvNULL);
                }

                sbSlot->preSiblingIdx = -1;
                prevActiveBVs = sbSlot->activeBVs;

                /* Resize the bvInfo buffer of this block if needed */
                if (sbSlot->bvSize < prevActiveBVs + totalEntries)
                {
                    GLuint prevBvSize = sbSlot->bvSize;
                    __GLchipBVinfo *prevBvInfos = sbSlot->bvInfos;

                    sbSlot->bvSize = prevActiveBVs + totalEntries;
                    sbSlot->bvInfos = (__GLchipBVinfo*)gc->imports.calloc(gc, 1, sbSlot->bvSize * sizeof(__GLchipBVinfo));

                    if (prevBvInfos)
                    {
                        __GL_MEMCOPY(sbSlot->bvInfos, prevBvInfos, prevBvSize * sizeof(__GLchipBVinfo));
                        gc->imports.free(gc, prevBvInfos);
                    }
                }

                /* Iterate all active variables in the storage block. */
                for (j = 0; j < (GLint)numBVs; ++j)
                {
                    gctUINT bvIdx;
                    gctUINT numEntries;
                    gctUINT maxLen;
                    gctUINT arraySize;
                    gctUINT entryIdx;
                    gctINT offset;

                    gcmONERROR(gcSHADER_GetStorageBlockVariable(Shader, storageBlock, j, &variable));

                    /* Skip inactive variables */
                    if (!variable || isVariableInactive(variable))
                    {
                        continue;
                    }
                    /* Skip auxiliary variables */
                    if (!isVariableSimple(variable))
                    {
                        continue;
                    }

                    offset = (gctINT)GetVariableOffset(variable);
                    numEntries = gcChipGetBufVariableArrayInfo(variable, &maxLen, gcvNULL, &arraySize);
                    if (isVariableArraysOfArrays(variable))
                    {
                        maxLen += 3;
                    }
                    maxLen++; /* count in null-terminator */

                    tmpName = (gctSTRING)gc->imports.calloc(gc, maxLen, sizeof(gctCHAR));
                    gcoOS_StrCopySafe(tmpName, maxLen, GetVariableName(variable));

                    if (isVariableArraysOfArrays(variable) && !isVariableStructMember(variable))
                    {
                        gcoOS_StrCatSafe(tmpName, maxLen, "[0]");
                    }

                    for (entryIdx = 0; entryIdx < numEntries; ++entryIdx)
                    {
                        /* Print name string of this entry */
                        if ((GetVariableArrayLengthCount(variable) > 1 && isVariableStructMember(variable)) ||
                            (GetVariableArrayLengthCount(variable) > 2 && !isVariableStructMember(variable)))
                        {
                            GLint k, startIndex = isVariableStructMember(variable) ? 0 : 1;
                            GLuint tmpIdx = entryIdx;
                            gctUINT tmpOffset = (gctUINT)gcoOS_StrLen(GetVariableName(variable), gcvNULL);
                            gctINT_PTR arrayIndices = (gctINT_PTR)gc->imports.calloc(gc, variable->arrayLengthCount - 1 - startIndex, sizeof(gctINT));

                            if (!isVariableStructMember(variable))
                            {
                                tmpOffset += 3;
                            }

                            for (k = variable->arrayLengthCount - 2; k >= startIndex; --k)
                            {
                                arrayIndices[k - startIndex] = tmpIdx % variable->arrayLengthList[k];
                                tmpIdx /= variable->arrayLengthList[k];
                            }

                            for (k = startIndex; k < variable->arrayLengthCount - 1; ++k)
                            {
                                gcoOS_PrintStrSafe(tmpName, maxLen, &tmpOffset, "[%d]", arrayIndices[k - startIndex]);
                            }
                            gc->imports.free(gc, arrayIndices);
                        }

                        for (bvIdx = 0; bvIdx < prevActiveBVs; ++bvIdx)
                        {
                            if (gcmIS_SUCCESS(gcoOS_StrCmp(sbSlot->bvInfos[bvIdx].name, tmpName)))
                            {
                                if (offset != sbSlot->bvInfos[bvIdx].offset)
                                {
                                    gcoOS_StrCatSafe(progObj->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                                        "PS didn't report same variable offset as VS did");
                                    gcmONERROR(gcvSTATUS_UNIFORM_MISMATCH);
                                }
                                break;
                            }
                        }

                        /* If the variable was not found in previous shader stage */
                        if (bvIdx == prevActiveBVs)
                        {
                            gctSIZE_T nameSize = gcoOS_StrLen(tmpName, gcvNULL) + 1;

                            bvIdx = sbSlot->activeBVs++;
                            gcoOS_Allocate(gcvNULL, nameSize, (gctPOINTER*)&sbSlot->bvInfos[bvIdx].name);
                            gcoOS_StrCopySafe(sbSlot->bvInfos[bvIdx].name, nameSize, tmpName);
                            sbSlot->bvInfos[bvIdx].offset = offset;
                        }
                        sbSlot->bvInfos[bvIdx].halBV[stageIdx] = variable;

                        offset += (GetVariableArrayStride(variable) * arraySize);
                    }

                    gc->imports.free(gc, tmpName);
                    tmpName = gcvNULL;
                }
            }

            program->maxActiveBVs = __GL_MAX(program->maxActiveBVs, (GLint)sbSlot->activeBVs);
            break;

        case __GL_CHIP_SSB_USAGE_SHAREDVAR:
        case __GL_CHIP_SSB_USAGE_EXTRAREG:
            sbSlotIdx = *PrivateIndex;
            /* Check duplicated storage blocks in previous shader stage */
            for (j = program->userDefSsbCount; j < prevPrivateIdx; ++j)
            {
                if (gcmIS_SUCCESS(gcoOS_StrCmp(sbName, program->ssbs[j].name)))
                {
                   /* If multiple shaders define the same UB, they must report same dataSize */
                    if (GetSBBlockSize(storageBlock) != program->ssbs[j].dataSize)
                    {
                        gcmFATAL("%s(%d): multiple declaration didn't report same SB dataSize", __FUNCTION__, __LINE__);
                        gcmONERROR(gcvSTATUS_VARYING_TYPE_MISMATCH);
                    }
                    duplicate = GL_TRUE;
                    sbSlotIdx = j;
                    break;
                }
            }

            if (!duplicate)
            {
                ++(*PrivateIndex);
            }

            GL_ASSERT(sbSlotIdx < (GLint)program->totalSsbCount);
            sbSlot = &program->ssbs[sbSlotIdx];
            break;

        default:
            GL_ASSERT(0);
            break;
        }

        sbSlot->halSB[stageIdx] = storageBlock;
        sbSlot->halUniform[stageIdx] = sbUniform;
        sbSlot->refByStage[stageIdx] = HasIBIFlag(GetSBInterfaceBlockInfo(storageBlock),gceIB_FLAG_STATICALLY_USED);

        gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(program->curPgInstance->programState.hints->hwConstRegBases,
                                                          sbUniform,
                                                          &sbSlot->stateAddress[stageIdx]));

        if (!duplicate)
        {
            sbSlot->name        = sbName;
            sbSlot->nameLen     = GetSBNameLength(storageBlock);
            sbSlot->dataSize    = GetSBBlockSize(storageBlock);
            sbSlot->binding     = GetSBBinding(storageBlock);
            sbSlot->usage       = sbUsage;
        }
    }

OnError:
    if (tmpName)
    {
        gc->imports.free(gc, tmpName);
        tmpName = gcvNULL;
    }
    gcmFOOTER_NO();
}

static GLuint
gcChipCountBufferVariables(
    IN __GLchipSLProgram *program
    )
{
    GLuint i;

    gcmHEADER_ARG("program=0x%x", program);

    for (i = 0; i < program->userDefSsbCount; ++i)
    {
        __GLchipSLStorageBlock *sbSlot = &program->ssbs[i];

        /* Only count block[0] if it's defined as array. */
        if (sbSlot->preSiblingIdx == -1)
        {
            program->bufVariableCount += sbSlot->activeBVs;
        }
    }

    gcmFOOTER_ARG("return=%u", program->bufVariableCount);
    return program->bufVariableCount;
}

static GLvoid
gcChipProcessBufferVariables(
    IN __GLcontext *gc,
    IN OUT __GLprogramObject *progObj
)
{
    GLuint i, j;
    GLuint bvCount = 0;
    __GLchipSLProgram *program = (__GLchipSLProgram *)progObj->privateData;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x progObj=0x%x", gc, progObj);

    for (i = 0; i < program->userDefSsbCount; ++i)
    {
        __GLchipSLStorageBlock *sbSlot = &program->ssbs[i];

        if (sbSlot->preSiblingIdx == -1)
        {
            for (j = 0; j < sbSlot->activeBVs; ++j)
            {
                __GLSLStage stage;
                GLboolean duplicate = GL_FALSE;
                GLuint bvSlotIdx = bvCount++;
                __GLchipSLBufVariable *bvSlot = &program->bufVariables[bvSlotIdx];

                bvSlot->ssbIndex = i;

                for (stage = 0; stage < __GLSL_STAGE_LAST; ++stage)
                {
                    gcVARIABLE variable;
                    gcSHADER_TYPE dataType;
                    gcSHADER_PRECISION precision;
                    gctBOOL isRowMajor;
                    gctUINT arraySize;
                    gctUINT maxLen;
                    gctBOOL isArray;

                    variable = sbSlot->bvInfos[j].halBV[stage];

                    if (!variable)
                    {
                        continue;
                    }

                    gcChipGetBufVariableArrayInfo(variable, &maxLen, &isArray, &arraySize);

                    dataType = GetVariableType(variable);
                    precision = gcChipUtilFixPrecison(program, GetVariablePrecision(variable), dataType, stage);
                    isRowMajor = gcChipIsMatrixType(dataType) ? GetVariableIsRowMajor(variable) : gcvFALSE;

                    bvSlot->halBV[stage] = variable;
                    bvSlot->refByStage[stage] = IsVariableStaticallyUsed(variable);
                    bvSlot->name = sbSlot->bvInfos[j].name;

                    if (duplicate)
                    {
                        if (bvSlot->precision    != precision ||
                            bvSlot->dataType     != dataType ||
                            bvSlot->isRowMajor   != isRowMajor ||
                            bvSlot->isArray      != isArray ||
                            bvSlot->arraySize    != arraySize ||
                            bvSlot->arrayStride  != (gctINT)GetVariableArrayStride(variable) ||
                            bvSlot->matrixStride != (gctINT)GetVariableMatrixStride(variable) ||
                            bvSlot->numArraySize != variable->arrayLengthCount ||
                            (bvSlot->arraySizes && variable->arrayLengthList &&
                             !gcmIS_SUCCESS(gcoOS_MemCmp(bvSlot->arraySizes, variable->arrayLengthList,
                                                         bvSlot->numArraySize * gcmSIZEOF(gctINT)))
                            )
                           )
                        {
                            /* Compiler didn't check variable match when link, driver check it. */
                            gcoOS_StrCatSafe(progObj->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                                "PS didn't report same variable parameters as VS did");
                            gcmONERROR(gcvSTATUS_UNIFORM_MISMATCH);
                        }
                    }
                    else
                    {
                        gctINT k;
                        GLuint nameLen = (GLuint)gcoOS_StrLen(bvSlot->name, gcvNULL);

                        bvSlot->nameLen      = nameLen;
                        bvSlot->precision    = precision;
                        bvSlot->dataType     = dataType;
                        bvSlot->isRowMajor   = isRowMajor;
                        bvSlot->isArray      = isArray;
                        bvSlot->arraySize    = arraySize;
                        bvSlot->arrayStride  = (gctINT)GetVariableArrayStride(variable);
                        bvSlot->matrixStride = (gctINT)GetVariableMatrixStride(variable);
                        bvSlot->offset       = sbSlot->bvInfos[j].offset;

                        bvSlot->numArraySize = variable->arrayLengthCount;
                        if (bvSlot->numArraySize > 0)
                        {
                            gcoOS_Allocate(gcvNULL, bvSlot->numArraySize * gcmSIZEOF(gctINT), (gctPOINTER*)&bvSlot->arraySizes);
                            gcoOS_MemCopy(bvSlot->arraySizes, variable->arrayLengthList, bvSlot->numArraySize * gcmSIZEOF(gctINT));
                        }

                        bvSlot->topLevelArraySize = GetVariableTopLevelArraySize(variable);
                        bvSlot->topLevelArrayStride = GetVariableTopLevelArrayStride(variable);

                        /* If this array is a struct member, then don't multiply array size. */
                        if (!isVariableStructMember(variable))
                        {
                            for (k = 1; k < bvSlot->numArraySize; ++k)
                            {
                                bvSlot->topLevelArrayStride *= bvSlot->arraySizes[k];
                            }
                        }

                        if (bvSlot->isArray)
                        {
                            nameLen += 3;
                        }

                        program->bvMaxNameLen = __GL_MAX(program->bvMaxNameLen, nameLen + 1);
                    }

                    duplicate = GL_TRUE;
                }

                sbSlot->bvInfos[j].index = bvSlotIdx;
            }
        }
        else
        {
            /* Per spec, Block[i+1] contained variables should be same as Block[i] */
            __GLchipSLStorageBlock *preSbSlot = &program->ssbs[sbSlot->preSiblingIdx];
            gctSIZE_T bytes = preSbSlot->activeBVs * sizeof(__GLchipBVinfo);

            /* Make sure this SB was not allocated yet */
            GL_ASSERT(sbSlot->activeBVs == 0 && !sbSlot->bvInfos);
            /* Make sure it's pre-sibling be allocated */
            GL_ASSERT(preSbSlot->activeBVs > 0 && preSbSlot->bvInfos);

            sbSlot->activeBVs = sbSlot->bvSize = preSbSlot->activeBVs;
            sbSlot->bvInfos = (__GLchipBVinfo*)gc->imports.malloc(gc, bytes);
            __GL_MEMCOPY(sbSlot->bvInfos, preSbSlot->bvInfos, bytes);
        }
    }

    GL_ASSERT(bvCount == program->bufVariableCount);

OnError:
    gcmFOOTER_NO();
}

static void
gcChipPgInstanceCleanBindingInfo(
    __GLcontext *gc,
    __GLchipSLProgramInstance* pgInstance
    )
{
    GLint i;
    gcmHEADER_ARG("pgInstance=0x%x", pgInstance);

    /* Free uniforms */
    if (pgInstance->privateUniforms != gcvNULL)
    {
        for (i = 0; i < pgInstance->privateUniformCount; ++i)
        {
            __GLchipSLUniform *uniform = &pgInstance->privateUniforms[i];
            __GLSLStage stage;
            gctBOOL emptyUniform = gcvTRUE;

            for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
            {
                if (uniform->halUniform[stage] != gcvNULL)
                {
                    emptyUniform = gcvFALSE;
                    break;
                }
            }

            if (emptyUniform)
            {
                continue;
            }

            GL_ASSERT(uniform->name);
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, uniform->name));

            if (uniform->arraySizes)
            {
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, uniform->arraySizes));
            }

            /* No need to free if uniform was put in UBO, its data was either NULL or mapped from UBO */
            if (uniform->data && uniform->offset == -1)
            {
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, uniform->data));
            }
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, pgInstance->privateUniforms));
        pgInstance->privateUniforms = gcvNULL;
    }
    pgInstance->privateUniformCount = 0;
    pgInstance->hasLTC = gcvFALSE;

    pgInstance->xfbActiveUniform = gcvNULL;
    for (i = 0; i < (GLint)pgInstance->xfbBufferCount; ++i)
    {
        pgInstance->xfbBufferUniforms[i] = gcvNULL;
    }
    pgInstance->xfbVertexCountPerInstance = gcvNULL;
    pgInstance->xfbBufferCount = 0;

    pgInstance->pLastFragData = gcvNULL;

    /* Free uniform blocks */
    if (pgInstance->privateUBs != gcvNULL)
    {
        for (i = 0; i < pgInstance->privateUbCount; ++i)
        {
            __GLchipSLUniformBlock *ub = &pgInstance->privateUBs[i];

            if (ub->uniformIndices)
            {
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, ub->uniformIndices));
            }

            if (ub->bufBase)
            {
                gcoOS_Free(gcvNULL, ub->bufBase);
                ub->bufBase = gcvNULL;
            }

            if (ub->halBufObj)
            {
                gcmVERIFY_OK(gcoBUFOBJ_Destroy(ub->halBufObj));
                ub->halBufObj = gcvNULL;
            }
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, pgInstance->privateUBs));
        pgInstance->privateUBs = gcvNULL;
    }
    pgInstance->privateUbCount = 0;

    pgInstance->groupNumUniformIdx = -1;
    pgInstance->advBlendState = gcvNULL;
    pgInstance->tcsPatchInVertices = 0;

    for (i = 0; i < (GLint)gc->constants.shaderCaps.maxTextureSamplers; ++i)
    {
        /* Reset sampleMap table to invalid shader stage by default */
        pgInstance->extraSamplerMap[i].stage = __GLSL_STAGE_LAST;
        pgInstance->extraSamplerMap[i].shaderType = gcSHADER_TYPE_UNKNOWN;
    }

    pgInstance->extraImageUniformCount = 0;
    gcoOS_ZeroMemory(pgInstance->extraImageUnit2Uniform, gcmSIZEOF(__GLchipImageUnit2Uniform) * __GL_MAX_IMAGE_UNITS);

    gcmFOOTER_NO();
}

static void
gcChipProgramCleanBindingInfo(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    GLint i;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLProgramInstance* masterPgInstance = program->masterPgInstance;
    __GLSLStage stage;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x", gc, programObject);

#if __GL_CHIP_PATCH_ENABLED
    program->aLocPosition = -1;
    program->aLocTexCoord = -1;
#endif

    /* Free attribute pointers. */
    for (i = 0; i < (GLint)program->inCount; i++)
    {
        if (program->inputs[i].name != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->inputs[i].name));
        }
    }
    if (program->inputs != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->inputs));
    }

    /* Zero existing attribute buffer. */
    program->inCount = 0;
    program->inMaxNameLen = 0;

    for (i = 0; i < (GLint)gc->constants.shaderCaps.maxUserVertAttributes; ++i)
    {
        if (program->attribLinkage[i])
        {
            __GLchipSLLinkage* attribLinkage = program->attribLinkage[i];
            while (attribLinkage)
            {
                __GLchipSLLinkage* tmp = attribLinkage->next;
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, attribLinkage));
                attribLinkage = tmp;
            }
            program->attribLinkage[i] = gcvNULL;
        }
    }

    for (i = 0; i < (GLint)gc->constants.shaderCaps.maxVertAttributes * MAX_ALIASED_ATTRIB_COUNT; ++i)
    {
        program->attribLocation[i].pInput   = gcvNULL;
        program->attribLocation[i].index    = (GLuint)-1;
        program->attribLocation[i].assigned = GL_FALSE;
    }
    programObject->bindingInfo.maxInputNameLen = 0;
    programObject->bindingInfo.numActiveInput = 0;
    programObject->bindingInfo.vsInputArrayMask = 0;

    __glBitmaskSetAll(&program->shadowSamplerMask, GL_FALSE);
    __glBitmaskSetAll(&program->texelFetchSamplerMask, GL_FALSE);

    /* Free uniforms */
    if (program->uniforms)
    {
        for (i = 0; i < program->activeUniformCount; ++i)
        {
            __GLchipSLUniform *uniform = &program->uniforms[i];

            GL_ASSERT(uniform->name);
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, uniform->name));

            if (uniform->arraySizes)
            {
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, uniform->arraySizes));
            }

            /* No need to free if uniform was put in UBO, its data was either NULL or mapped from UBO */
            if (uniform->data && uniform->offset == -1)
            {
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, uniform->data));
            }
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->uniforms));
        program->uniforms = gcvNULL;
    }
    program->uniformMaxNameLen    = 0;
    program->userDefUniformCount  = 0;
    program->builtInUniformCount  = 0;
    program->activeUniformCount   = 0;
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        program->numSamplers[stage] = 0;
    }
    programObject->bindingInfo.maxUniformNameLen = 0;
    programObject->bindingInfo.numActiveUniform = 0;

    if (program->loc2Uniform)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->loc2Uniform));
        program->loc2Uniform = gcvNULL;
    }
    program->maxLocation = 0;

    /* Free uniform blocks */
    if (program->uniformBlocks)
    {
        for (i = 0; i < program->totalUbCount; ++i)
        {
            __GLchipSLUniformBlock *ub = &program->uniformBlocks[i];

            if (ub->uniformIndices)
            {
                gc->imports.free(gc, ub->uniformIndices);
            }

            if (ub->bufBase)
            {
                gcoOS_Free(gcvNULL, ub->bufBase);
                ub->bufBase = gcvNULL;
            }

            if (ub->halBufObj)
            {
                gcmVERIFY_OK(gcoBUFOBJ_Destroy(ub->halBufObj));
                ub->halBufObj = gcvNULL;
            }
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->uniformBlocks));
        program->uniformBlocks = gcvNULL;
    }
    program->ubMaxNameLen   = 0;
    program->userDefUbCount = 0;
    program->defaultUbCount = 0;
    program->totalUbCount   = 0;
    program->maxActiveUniforms = 0;
    programObject->bindingInfo.numActiveUB = 0;
    programObject->bindingInfo.maxUBNameLen = 0;
    programObject->bindingInfo.maxActiveUniforms = 0;

    /* Transform feedback varyings */
    if (program->xfbVaryings)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->xfbVaryings));
        program->xfbVaryings = gcvNULL;
    }
    program->xfbCount = 0;
    program->xfbMaxNameLen = 0;
    program->xfbStride = 0;
    programObject->bindingInfo.xfbMode = GL_INTERLEAVED_ATTRIBS;
    programObject->bindingInfo.numActiveXFB  = 0;
    programObject->bindingInfo.maxXFBNameLen = 0;

    if (program->acbs)
    {
        for (i = 0; i < (GLint)program->acbCount; ++i)
        {
            __GLchipSLAtomCntBuf* acb = &program->acbs[i];
            if (acb->uniformIndices)
            {
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, acb->uniformIndices));
                acb->uniformIndices = gcvNULL;
            }
        }
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->acbs));
        program->acbs = gcvNULL;

        GL_ASSERT(program->acbBinding2SlotIdx && program->acbBinding2NumACs);
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->acbBinding2SlotIdx));
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->acbBinding2NumACs));
        program->acbBinding2SlotIdx = gcvNULL;
        program->acbBinding2NumACs = gcvNULL;
    }
    program->acbCount = 0;
    program->maxActiveACs = 0;
    programObject->bindingInfo.numActiveACBs = 0;
    programObject->bindingInfo.maxActiveACs = 0;

    /* Free outputs. */
    for (i = 0; i < (GLint)program->outCount; i++)
    {
        if (program->outputs[i].name != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->outputs[i].name));
        }
    }
    if (program->outputs)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->outputs));
        program->outputs = gcvNULL;
    }
    if (program->loc2Out)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->loc2Out));
        program->loc2Out = gcvNULL;
    }
    program->outCount = 0;
    program->outMaxNameLen = 0;
    program->maxOutLoc = 0;
    programObject->bindingInfo.numActiveOutput = 0;
    programObject->bindingInfo.maxOutputNameLen = 0;

    for (i = 0; i < (GLint)gc->constants.shaderCaps.maxTextureSamplers; ++i)
    {
        /* Reset sampleMap table to invalid shader stage by default */
        program->samplerMap[i].stage = __GLSL_STAGE_LAST;
    }

    if (program->bufVariables)
    {
        for (i = 0; i < (GLint)program->bufVariableCount; ++i)
        {
            __GLchipSLBufVariable *variable = &program->bufVariables[i];

            GL_ASSERT(variable->name);
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, variable->name));

            if (variable->arraySizes)
            {
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, variable->arraySizes));
            }
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->bufVariables));
        program->bufVariables = gcvNULL;
    }
    program->bufVariableCount = 0;
    program->bvMaxNameLen = 0;
    programObject->bindingInfo.numActiveBV = 0;
    programObject->bindingInfo.maxBVNameLen = 0;

    if (program->ssbs)
    {
        for (i = 0; i < (GLint)program->totalSsbCount; ++i)
        {
            __GLchipSLStorageBlock* ssb = &program->ssbs[i];
            if (ssb->bvInfos)
            {
                gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, ssb->bvInfos));
                ssb->bvInfos = gcvNULL;
            }
            if (ssb->halBufObj)
            {
                gcmVERIFY_OK(gcoBUFOBJ_Destroy(ssb->halBufObj));
                ssb->halBufObj = gcvNULL;
            }
        }

        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->ssbs));
        program->ssbs = gcvNULL;
    }
    program->userDefSsbCount = 0;
    program->privateSsbCount = 0;
    program->totalSsbCount = 0;
    program->ssbMaxNameLen = 0;
    program->maxActiveBVs = 0;
    programObject->bindingInfo.numActiveSSB = 0;
    programObject->bindingInfo.maxSSBNameLen = 0;
    programObject->bindingInfo.maxActiveBVs = 0;

    programObject->bindingInfo.tessOutPatchSize = 0;
    programObject->bindingInfo.tessGenMode = GL_QUADS_EXT;
    programObject->bindingInfo.tessSpacing = GL_EQUAL;
    programObject->bindingInfo.tessVertexOrder = GL_CCW;
    programObject->bindingInfo.tessPointMode = GL_FALSE;

    programObject->bindingInfo.gsOutVertices = 0;
    programObject->bindingInfo.gsInputType = GL_TRIANGLES;
    programObject->bindingInfo.gsOutputType = GL_TRIANGLE_STRIP;
    programObject->bindingInfo.gsInvocationCount = 1;

    __GL_MEMZERO(programObject->bindingInfo.workGroupSize, 3 * sizeof(GLuint));

    program->imageUniformCount = 0;
    gcoOS_ZeroMemory(program->imageUnit2Uniform, gcmSIZEOF(__GLchipImageUnit2Uniform) * __GL_MAX_IMAGE_UNITS);

    if (masterPgInstance)
    {
        gcChipPgInstanceCleanBindingInfo(gc, masterPgInstance);
    }
    gcmFOOTER_NO();
}

static GLboolean
gcChipProgramBindingRecompiledInfo(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLProgramInstance* pgInstance = program->curPgInstance;
    gcSHADER *pBinaries = gcvNULL;
    gctUINT resCounts[__GLSL_STAGE_LAST] = {0};
    gctUINT combinedResCount = 0;
    __GLSLStage stage;
    gceSTATUS status = gcvSTATUS_OK;
    GLint i;

#if (defined(_DEBUG) || defined(DEBUG))
    GLint  prevUserDefUniformCount = program->userDefUniformCount;
    GLint  prevBuiltInUniformCount = program->builtInUniformCount;
    GLint  prevUserDefUbCount      = program->userDefUbCount;
    GLint  prevDefaultUbCount      = program->defaultUbCount;
#endif

    gcmHEADER_ARG("gc=0x%x programObject=0x%x", gc, programObject);

    if (pgInstance)
    {
        pBinaries = pgInstance->binaries;
    }
    else
    {
        gcmONERROR(gcvSTATUS_INVALID_ADDRESS);
    }

    /***************************************************************************
    ** Uniforms management.
    */
    program->uniformMaxNameLen    = 0;
    program->userDefUniformCount  = 0;
    program->builtInUniformCount  = 0;
    program->activeUniformCount   = 0;

    /* Get the number of uniforms for each stage */
    combinedResCount = 0;
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (pBinaries[stage])
        {
            gcmONERROR(gcSHADER_GetUniformCount(pBinaries[stage], &resCounts[stage]));
        }
        else
        {
            resCounts[stage] = 0;
        }
        combinedResCount += resCounts[stage];
    }

    if (combinedResCount > 0)
    {
        GLint index = 0;
        gctCONST_STRING *names = (gctCONST_STRING*)gc->imports.malloc(gc, combinedResCount * sizeof(gctCONST_STRING));

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                gcChipCountUniforms(gc, program, pBinaries[stage], resCounts[stage], GL_TRUE, &index, names);
            }
        }

        gc->imports.free(gc, (gctPOINTER)names);

        program->activeUniformCount = program->userDefUniformCount + program->builtInUniformCount;
    }

    /*
    ** Rebuild uniform table only(including the base address of SSBOs),
    ** supposed recompile only affect uniform, not UBO, attribute, output etc.
    */
    if (pgInstance->privateUniformCount > 0)
    {
        GLint userDefIndex = 0;
        GLint builtInIndex = program->userDefUniformCount;
        GLint privateIndex = 0;

        /* Allocate the array of _GLUniform structures. */
        gctSIZE_T bytes = pgInstance->privateUniformCount * gcmSIZEOF(__GLchipSLUniform);
        gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*)&pgInstance->privateUniforms));
        gcoOS_ZeroMemory(pgInstance->privateUniforms, bytes);

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                gcmONERROR(gcChipProcessUniforms(gc,
                                                 programObject,
                                                 pBinaries[stage],
                                                 resCounts[stage],
                                                 GL_TRUE,
                                                 &userDefIndex,
                                                 &builtInIndex,
                                                 &privateIndex,
                                                 gcvNULL,
                                                 gcvNULL));
            }
        }
    }

    /***************************************************************************
    ** Uniform blocks management.
    */
    program->userDefUbCount = 0;
    program->defaultUbCount = 0;

    /* Get the number of uniform block for each stage */
    combinedResCount = 0;
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (pBinaries[stage])
        {
            gcmONERROR(gcSHADER_GetUniformBlockCount(pBinaries[stage], &resCounts[stage]));
        }
        else
        {
            resCounts[stage] = 0;
        }
        combinedResCount += resCounts[stage];
    }

    if (combinedResCount > 0)
    {
        GLint ubIndex = 0;
        gctCONST_STRING *names = (gctCONST_STRING*)gc->imports.malloc(gc, combinedResCount * sizeof(gctCONST_STRING));

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                gcChipCountUniformBlocks(program, pBinaries[stage], resCounts[stage], &ubIndex, names);
            }
        }

        program->totalUbCount = program->userDefUbCount + program->defaultUbCount;

        gc->imports.free(gc, (gctPOINTER)names);
    }

    if (pgInstance->privateUbCount > 0)
    {
        GLint userDefIndex = 0;
        GLint defaultIndex = program->userDefUbCount;
        GLint privateIndex = 0;

        /* Allocate the array of _GLUniform structures. */
        if (pgInstance->privateUbCount)
        {
            gctSIZE_T bytes = pgInstance->privateUbCount * gcmSIZEOF(__GLchipSLUniformBlock);
            gcmONERROR(gcoOS_Allocate(gcvNULL, bytes, (gctPOINTER*)&pgInstance->privateUBs));
            gcoOS_ZeroMemory(pgInstance->privateUBs, bytes);
        }

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                gcChipProcessUniformBlocks(gc,
                                           programObject,
                                           pBinaries[stage],
                                           resCounts[stage],
                                           GL_TRUE,
                                           gcvNULL,
                                           &userDefIndex,
                                           &defaultIndex,
                                           &privateIndex);
            }
        }
    }

    /* Allocate and upload private UBOs (currently only const ones) */
    for (i = 0; i < pgInstance->privateUbCount; ++i)
    {
        gctPOINTER data = gcvNULL;
        __GLchipSLUniformBlock *uBlock = &pgInstance->privateUBs[i];

        GL_ASSERT(uBlock->halBufObj == gcvNULL);
        gcmONERROR(gcoBUFOBJ_Construct(chipCtx->hal, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &uBlock->halBufObj));

        GL_ASSERT(uBlock->activeUniforms == 0);

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (uBlock->halUB[stage])
            {
                data = GetShaderConstUBOData(pBinaries[stage]);
                break;
            }
        }
        gcmONERROR(gcoBUFOBJ_Upload(uBlock->halBufObj, data, 0, uBlock->dataSize, gcvBUFOBJ_USAGE_STATIC_READ));
    }

    /* The base address uniform of a SSBO may be changed, so we need to update this uniform. */
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (pBinaries[stage])
        {
            gcChipUpdateBaseAddrUniformForStorageBlocks(gc, programObject, pBinaries[stage]);
        }
    }

    if (pBinaries[__GLSL_STAGE_TCS])
    {
        gcmONERROR(gcSHADER_GetTCSPatchInputVertices(pBinaries[__GLSL_STAGE_TCS], &pgInstance->tcsPatchInVertices));
    }

#if (defined(_DEBUG) || defined(DEBUG))
    /* Assumption: the recompile process cannot change the shader required user+buildin uniforms/UBs.
    ** It only can change compiler generated private uniforms/UBs.
    */
    GL_ASSERT(prevUserDefUniformCount == program->userDefUniformCount);
    GL_ASSERT(prevBuiltInUniformCount == program->builtInUniformCount);
    GL_ASSERT(prevUserDefUbCount      == program->userDefUbCount);
    GL_ASSERT(prevDefaultUbCount      == program->defaultUbCount);
#endif

OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipPgInstanceCleanBindingInfo(gc, pgInstance);
    }
    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipMapLinkError(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    gceSTATUS Status
    )
{
    gctSTRING logBuffer = programObject->programInfo.infoLog;
    gctUINT logOffset = 0;
    gceSTATUS status;

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

    case gcvSTATUS_LOCATION_ALIASED:
        gcmONERROR(gcoOS_PrintStrSafe(logBuffer,
            __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Location aliased.\n"));
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

static gceSTATUS
gcChipProgramBuildBindingInfo(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLProgramInstance* masterPgInstance = program->masterPgInstance;
    GLuint i, j;
    GLint index = 0;
    __GLSLStage stage, firstStage, lastStage, lastNonFragStage;
    gcSHADER *pBinaries = masterPgInstance->binaries;
    /* Array to record how HAL level stage uniform indices are mapped to GL user uniform ones */
    GLint* uniformHALIdx2GL[__GLSL_STAGE_LAST] = {gcvNULL};
    gctUINT resCounts[__GLSL_STAGE_LAST] = {0};
    gctUINT resCount, combinedResCount = 0;
    gctSTRING logBuffer = programObject->programInfo.infoLog;
    gctUINT logOffset = 0;
    gctSIZE_T bytes;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT   userInputCount;
    gctUINT   activeUserInputCount = 0;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x", gc, programObject);

    /* Linker will make all stage have same shader language version */
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (pBinaries[stage])
        {
            program->isHalti = gcSHADER_IsHaltiCompiler(pBinaries[stage]);
            break;
        }
    }

    firstStage = gcChipGetFirstStage(program);
    GL_ASSERT(firstStage < __GLSL_STAGE_LAST);

    /* Get the number of shader inputs. */
    gcmONERROR(gcSHADER_GetAttributeAndBuiltinInputCount(pBinaries[firstStage], &resCount));
    gcmONERROR(gcSHADER_GetAttributeCount(pBinaries[firstStage], &userInputCount));

    /* Query how many active inputs */
    for (i = 0; i < resCount; ++i)
    {
        gcATTRIBUTE attribute;
        gctBOOL enable = gcvFALSE;

        gcmONERROR(gcSHADER_GetAttribute(pBinaries[firstStage], i, &attribute));

        /* skip NULL inputs, do not know why NULL one was reported by HAL */
        if (gcvNULL == attribute)
        {
            continue;
        }
        /* If the input was inactive, skip it */
        if (!gcmIS_SUCCESS(gcATTRIBUTE_IsEnabled(attribute, &enable)) || !enable)
        {
            continue;
        }

        if (GetATTRIOBlockArrayIndex(attribute))
        {
            continue;
        }

        if (i >= userInputCount)
        {
            activeUserInputCount = program->inCount;
        }
        program->inCount++;
    }

    if (program->inCount > 0)
    {
        GLuint count = 0;
        __GLchipSLBinding *binding;
        __GLchipSLInput *input;
        gctBOOL bGLSL1_0 = (firstStage == __GLSL_STAGE_VS)
                         ? gcSHADER_IsES11Compiler(pBinaries[firstStage])
                         : gcvFALSE;
        gctUINT locationIndex = 0;
        gctUINT aliasedIndex = 0;
        gctUINT aliasedCount = 0;

        /* Allocate the array for the vertex attributes. */
        bytes = program->inCount * sizeof(__GLchipSLInput);
        gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&program->inputs));
        gcoOS_ZeroMemory(program->inputs, bytes);

        /* Query name/type of each input, and expand vs inputs (attrib) if matrix */
        for (i = 0; i < resCount; ++i)
        {
            gcATTRIBUTE attribute;
            gctBOOL enable = gcvFALSE;
            gctUINT nameLen = 0;
            gctBOOL bAliased = gcvFALSE;

            gcmONERROR(gcSHADER_GetAttribute(pBinaries[firstStage], i, &attribute));
            /* skip NULL inputs, do not know why NULL one was reported by HAL */
            if (gcvNULL == attribute)
            {
                continue;
            }
            /* If the input was inactive, skip it */
            if (!gcmIS_SUCCESS(gcATTRIBUTE_IsEnabled(attribute, &enable)) || !enable)
            {
                continue;
            }

            if (GetATTRIOBlockArrayIndex(attribute))
            {
                continue;
            }

            input = &program->inputs[count++];
            input->halAttrib = attribute;
            gcmONERROR(gcATTRIBUTE_GetNameEx(pBinaries[firstStage], attribute, &input->nameLen, &input->name));
            gcmONERROR(gcATTRIBUTE_GetType(pBinaries[firstStage], attribute, &input->type, (gctUINT_PTR)&input->arraySize));
            input->isArray = (GetATTRIsArray(attribute) ||
                (gcmATTRIBUTE_isPerVertexArray(attribute) && GetATTRNameLength(attribute) > 0));
            input->isBuiltin = gcmIS_SUCCESS(gcoOS_StrNCmp(input->name, "gl_", 3));
            input->fieldIndex = GetATTRFieldIndex(attribute);
            input->isPosition = gcmATTRIBUTE_isPosition(attribute);
            input->isDirectPosition = gcmATTRIBUTE_isDirectPosition(attribute);
            input->isLocationSetByDriver = gcmATTRIBUTE_isLocSetByDriver(attribute);
            gcmONERROR(gcATTRIBUTE_GetLocation(attribute, &input->location));
            gcmONERROR(gcATTRIBUTE_GetPrecision(attribute, &input->precision));
            gcmONERROR(gcATTRIBUTE_IsPerPatch(attribute, &input->isPerPatch));
            gcmONERROR(gcATTRIBUTE_IsSample(attribute, &input->isSampleIn));

            input->size = gcmATTRIBUTE_isPerVertexArray(attribute) ? 1 : input->arraySize;

            nameLen = input->isArray ? input->nameLen + 3 : input->nameLen;
            program->inMaxNameLen = __GL_MAX(program->inMaxNameLen, nameLen + 1);

            /* Expand the attribute for matrix type */
            switch (input->type)
            {
            case gcSHADER_FLOAT_2X2:
            case gcSHADER_FLOAT_2X3:
            case gcSHADER_FLOAT_2X4:
                input->size *= 2;
                break;
            case gcSHADER_FLOAT_3X2:
            case gcSHADER_FLOAT_3X3:
            case gcSHADER_FLOAT_3X4:
                input->size *= 3;
                break;
            case gcSHADER_FLOAT_4X2:
            case gcSHADER_FLOAT_4X3:
            case gcSHADER_FLOAT_4X4:
                input->size *= 4;
                break;
            default:
                break;
            }
            input->refByStage[firstStage] = GL_TRUE;

            /* Set attribute location. */
            if (input->isLocationSetByDriver && index > 0)
            {
                for ( locationIndex = 0; (gctINT)locationIndex < index;)
                {
                    if (input->location != -1 &&
                        program->attribLocation[locationIndex].pInput->isLocationSetByDriver &&
                        program->attribLocation[locationIndex].pInput->location == input->location)
                    {
                        bAliased = gcvTRUE;
                        break;
                    }

                    locationIndex += program->attribLocation[locationIndex].pInput->size;
                }
            }

            /* expand the attribute, if it is a matrix, to basic types */
            for (j = 0; j < input->size; ++j)
            {
                if (i < activeUserInputCount && index >= (GLint)gc->constants.shaderCaps.maxUserVertAttributes)
                {
                    gcmONERROR(gcoOS_PrintStrSafe(logBuffer, __GLSL_LOG_INFO_SIZE, &logOffset, "LinkShaders: Too many attributes\n"));
                    gcmTRACE(gcvLEVEL_ERROR, "LinkShaders: Too many attributes");
                    gcmONERROR(gcvSTATUS_TOO_MANY_ATTRIBUTES);
                }

                if (bAliased)
                {
                    /* Current attriLocation is not a list, we just use a array and only support
                    ** one aliased attribute for each slot.
                    ** the data structure is:
                    **
                    ** ------------------------------------------
                    **| 0~15 attribute slot| 16~31 aliased attrib|
                    ** ------------------------------------------
                    */
                    aliasedIndex = j + gc->constants.shaderCaps.maxVertAttributes + locationIndex;
                    gcmASSERT( program->attribLocation[aliasedIndex].pInput == gcvNULL);

                    program->attribLocation[aliasedIndex].pInput = input;
                    program->attribLocation[aliasedIndex].index  = (GLint)j;
                    continue;
                }

                program->attribLocation[index].pInput = input;
                program->attribLocation[index].index  = (GLint)j;
                ++index;
            }
        }

        /* Step1: Walk all attribute locations to set all binding defined in shader text by APP*/
        for (i = 0; i < gc->constants.shaderCaps.maxVertAttributes; ++i)
        {
            /* Ignore if location is unavailable or not the start of an array. */
            if ((!program->attribLocation[i].pInput) || (program->attribLocation[i].index != 0))
            {
                continue;
            }

            input = program->attribLocation[i].pInput;

            if (input->location != -1 && !(input->isLocationSetByDriver))
            {
                /* Test for overflow. */
                if (input->location + (GLuint)input->size > gc->constants.shaderCaps.maxUserVertAttributes)
                {
                    gcmONERROR(gcoOS_PrintStrSafe(logBuffer, __GLSL_LOG_INFO_SIZE, &logOffset, "Binding for %s overflows\n", input->name));
                    gcmTRACE(gcvLEVEL_ERROR, "Binding for %s overflows.", input->name);
                    gcmONERROR(gcvSTATUS_TOO_MANY_ATTRIBUTES);
                }

                /* Copy the binding. */
                for (j = 0; j < input->size; ++j)
                {
                    __GLchipSLLinkage* attribLinkage = gcvNULL;
                    /* Make sure the binding is not yet taken. */
                    if (program->attribLinkage[input->location + j])
                    {
                        gcmONERROR(gcoOS_PrintStrSafe(logBuffer, __GLSL_LOG_INFO_SIZE, &logOffset, "Binding for %s occupied\n", input->name));
                        gcmTRACE(gcvLEVEL_ERROR, "Binding for %s occupied.", input->name);
                        gcmONERROR(gcvSTATUS_TOO_MANY_ATTRIBUTES);
                    }

                    /* Assign the binding. */
                    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(__GLchipSLLinkage), (gctPOINTER *)&attribLinkage));
                    attribLinkage->attribLocation = (GLuint)(i + j);
                    attribLinkage->next = gcvNULL;
                    program->attribLinkage[input->location + j] = attribLinkage;
                    program->attribLocation[i + j].assigned = GL_TRUE;
                }
            }
        }

        /* Only assign location for VS attributes */
        if (firstStage == __GLSL_STAGE_VS)
        {
            /* Step2: Walk all bindings and assign the attributes which is specified by API */
            for (binding = program->attribBinding; binding != gcvNULL;  binding = binding->next)
            {
                /* Walk all attribute locations. */
                for (i = 0; i < gc->constants.shaderCaps.maxVertAttributes; ++i)
                {
                    for ( aliasedCount = 0; aliasedCount < MAX_ALIASED_ATTRIB_COUNT; ++aliasedCount)
                    {
                        aliasedIndex = i + aliasedCount * gc->constants.shaderCaps.maxVertAttributes;
                        /* Ignore if location is unavailable or not the start of an array. */
                        if ((!program->attribLocation[aliasedIndex].pInput) || (program->attribLocation[aliasedIndex].index != 0))
                        {
                            continue;
                        }

                        if (program->attribLocation[aliasedIndex].assigned)
                        {
                            continue;
                        }
                        input = program->attribLocation[aliasedIndex].pInput;

                        if (gcmIS_SUCCESS(gcoOS_StrCmp(binding->name, input->name)))
                        {
                            /* Test for overflow. */
                            if (binding->index + (GLuint)input->size > gc->constants.shaderCaps.maxUserVertAttributes)
                            {
                                gcmONERROR(gcoOS_PrintStrSafe(logBuffer, __GLSL_LOG_INFO_SIZE, &logOffset, "Binding for %s overflows\n", input->name));
                                gcmTRACE(gcvLEVEL_ERROR, "Binding for %s overflows.", binding->name);
                                gcmONERROR(gcvSTATUS_TOO_MANY_ATTRIBUTES);
                            }

                            /* Copy the binding. */
                            for (j = 0; j < input->size; ++j)
                            {
                                __GLchipSLLinkage* attribLinkage = gcvNULL;

                                /*
                                **    Make sure the binding is not yet taken.
                                **    Binding more than one attribute name to the same location is referred to as
                                **    aliasing, and is not permitted in OpenGL ES Shading Language 3.00 or later vertex
                                **    shaders. LinkProgram will fail when this condition exists. However, aliasing
                                **    is possible in OpenGL ES Shading Language 1.00 vertex shaders.
                                **
                                */
                                if (!bGLSL1_0 && program->attribLinkage[binding->index + j])
                                {
                                    gcmONERROR(gcoOS_PrintStrSafe(logBuffer, __GLSL_LOG_INFO_SIZE, &logOffset, "Binding for %s occupied.", input->name));
                                    gcmTRACE(gcvLEVEL_ERROR, "Binding for %s occupied.", binding->name);
                                    gcmONERROR(gcvSTATUS_TOO_MANY_ATTRIBUTES);
                                }

                                /* Assign the binding. */
                                gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(__GLchipSLLinkage), (gctPOINTER *)&attribLinkage));
                                attribLinkage->attribLocation = (GLuint)(i + j);

                                attribLinkage->next = gcvNULL;

                                attribLinkage->next = program->attribLinkage[binding->index + j];
                                program->attribLinkage[binding->index + j] = attribLinkage;

                                program->attribLocation[aliasedIndex + j].assigned = GL_TRUE;
                            }
                            input->location = binding->index;
                            break;
                        }
                    }
                }
            }

            /* Step3: Walk all unassigned attributes, which is decided by driver/compiler */
            for (i = 0; i < gc->constants.shaderCaps.maxVertAttributes; ++i)
            {
                gctSIZE_T size;

                /* Ignore if location is unavailable or not the start of an array. */
                if ((!program->attribLocation[i].pInput) || (program->attribLocation[i].index != 0))
                {
                    continue;
                }

                if (program->attribLocation[i].assigned)
                {
                    continue;
                }

                input = program->attribLocation[i].pInput;
                /* Skip assign location for builtin attributes */
                if (input->isBuiltin)
                {
                    continue;
                }

                size = input->size;
                /* Find continuous un-occupied slot */
                for (index = 0; index < (GLint)gc->constants.shaderCaps.maxUserVertAttributes; ++index)
                {
                    for (j = 0; j < size; ++j)
                    {
                        if (program->attribLinkage[index + j])
                        {
                            break;
                        }
                    }

                    if (j == size)
                    {
                        break;
                    }
                }

                if (index >= (GLint)gc->constants.shaderCaps.maxUserVertAttributes)
                {
                    gcmONERROR(gcoOS_PrintStrSafe(logBuffer, __GLSL_LOG_INFO_SIZE, &logOffset, "No room for attribute %d (x%d).\n", i, size));
                    gcmTRACE(gcvLEVEL_ERROR, "No room for attribute %d (x%d)", i, size);
                    gcmONERROR(gcvSTATUS_TOO_MANY_ATTRIBUTES);
                }

                for (j = 0; j < size; ++j)
                {
                    __GLchipSLLinkage* attribLinkage = gcvNULL;
                    gcmONERROR(gcoOS_Allocate(gcvNULL, gcmSIZEOF(__GLchipSLLinkage), (gctPOINTER *)&attribLinkage));
                    attribLinkage->attribLocation = (GLuint)(i + j);
                    attribLinkage->next = gcvNULL;
                    program->attribLinkage[index + j] = attribLinkage;
                    program->attribLocation[i + j].assigned = GL_TRUE;
                }
                input->location = index;
            }
        }
    }

    /***************************************************************************
    ** Uniform management.
    */

    /* Get the number of uniforms for each stage */
    combinedResCount = 0;
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (pBinaries[stage])
        {
            gcmONERROR(gcSHADER_GetUniformCount(pBinaries[stage], &resCounts[stage]));
        }
        else
        {
            resCounts[stage] = 0;
        }
        combinedResCount += resCounts[stage];
    }

    if (combinedResCount > 0)
    {
        gctCONST_STRING *names = (gctCONST_STRING*)gc->imports.calloc(gc, combinedResCount, sizeof(gctCONST_STRING));

        index = 0;
        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                gcChipCountUniforms(gc, program, pBinaries[stage], resCounts[stage], GL_FALSE, &index, names);
            }
        }

        gc->imports.free(gc, (gctPOINTER)names);

        /* Calc auxiliary values */
        program->activeUniformCount = program->userDefUniformCount + program->builtInUniformCount;
    }

    if (program->acbCount > 0)
    {
        bytes = program->acbCount * gcmSIZEOF(__GLchipSLAtomCntBuf);
        gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&program->acbs));
        gcoOS_ZeroMemory(program->acbs, bytes);
    }

    if (program->activeUniformCount + masterPgInstance->privateUniformCount > 0)
    {
        GLint userDefIndex = 0;
        GLint builtInIndex = program->userDefUniformCount;
        GLint privateIndex = 0;

        /* Allocate the array of _GLUniform structures. */
        if (program->activeUniformCount > 0)
        {
            bytes = program->activeUniformCount * gcmSIZEOF(__GLchipSLUniform);
            gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&program->uniforms));
            gcoOS_ZeroMemory(program->uniforms, bytes);
        }

        if (masterPgInstance->privateUniformCount > 0)
        {
            bytes = masterPgInstance->privateUniformCount * gcmSIZEOF(__GLchipSLUniform);
            gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&masterPgInstance->privateUniforms));
            gcoOS_ZeroMemory(masterPgInstance->privateUniforms, bytes);
        }

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                /* Process vertex uniforms. */
                uniformHALIdx2GL[stage] = (GLint*)gc->imports.calloc(gc, 1, resCounts[stage]*sizeof(GLint));
                gcmONERROR(gcChipProcessUniforms(gc,
                                                 programObject,
                                                 pBinaries[stage],
                                                 resCounts[stage],
                                                 GL_FALSE,
                                                 &userDefIndex,
                                                 &builtInIndex,
                                                 &privateIndex,
                                                 uniformHALIdx2GL[stage],
                                                 &program->numSamplers[stage]));
            }
        }
    }

    if (pBinaries[__GLSL_STAGE_CS])
    {
        if (-1 != masterPgInstance->groupNumUniformIdx)
        {
            gctPOINTER inputScratch;
            gctUINT newInputCount = (program->inCount + 1);
            __GLchipSLInput *newInput;

            gcmONERROR(gcoOS_Allocate(chipCtx->os, newInputCount * gcmSIZEOF(__GLchipSLInput), &inputScratch));
            gcoOS_ZeroMemory(inputScratch, newInputCount * gcmSIZEOF(__GLchipSLInput));
            if (program->inCount)
            {
                gcoOS_MemCopy(inputScratch, program->inputs, program->inCount * gcmSIZEOF(__GLchipSLInput));
                gcmOS_SAFE_FREE(chipCtx->os, program->inputs);
            }
            program->inputs = (__GLchipSLInput *)inputScratch;
            newInput = &program->inputs[program->inCount];
            program->inCount++;

            newInput->arraySize = 1;
            newInput->fieldIndex = -1;
            newInput->halAttrib = gcvNULL;
            newInput->isBuiltin = gcvTRUE;
            newInput->location = -1;
            gcoOS_StrDup(chipCtx->os, "gl_NumWorkGroups", &newInput->name);
            gcoOS_StrLen(newInput->name, (gctSIZE_T*)&newInput->nameLen);
            newInput->precision = gcSHADER_PRECISION_DEFAULT;
            newInput->refByStage[__GLSL_STAGE_CS] = gcvTRUE;
            newInput->size = 1;
            newInput->type = gcSHADER_UINT_X3;

            program->inMaxNameLen = __GL_MAX(program->inMaxNameLen, newInput->nameLen + 1);
        }
    }
    /***************************************************************************
    ** Uniform blocks management.
    */

    /* Get the number of uniform blocks for each stage */
    combinedResCount = 0;
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (pBinaries[stage])
        {
            gcmONERROR(gcSHADER_GetUniformBlockCount(pBinaries[stage], &resCounts[stage]));
        }
        else
        {
            resCounts[stage] = 0;
        }
        combinedResCount += resCounts[stage];
    }

    if (combinedResCount > 0)
    {
        GLint ubIndex = 0;
        GLuint combinedUBs = 0;
        GLuint activeUBs[__GLSL_STAGE_LAST] = {0};
        gctCONST_STRING *names = (gctCONST_STRING*)gc->imports.calloc(gc, combinedResCount, sizeof(gctCONST_STRING));

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                activeUBs[stage] = gcChipCountUniformBlocks(program, pBinaries[stage], resCounts[stage], &ubIndex, names);
                combinedUBs += activeUBs[stage];
            }
        }

        program->totalUbCount = program->userDefUbCount + program->defaultUbCount;

        gc->imports.free(gc, (gctPOINTER)names);

        /* ES30 conform used more than maxVertUniformBlocks/maxFragUniformBlocks/maxCombinedUniformBlocks
        ** and expected compiler pass, while link fail.
        */
        if (activeUBs[__GLSL_STAGE_VS] > gc->constants.shaderCaps.maxVertUniformBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "VS active uniform blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
        }
        if (activeUBs[__GLSL_STAGE_FS] > gc->constants.shaderCaps.maxFragUniformBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "PS active uniform blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
        }
        if (activeUBs[__GLSL_STAGE_CS] > gc->constants.shaderCaps.maxCmptUniformBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "CS active uniform blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
        }
        if (activeUBs[__GLSL_STAGE_TCS] > gc->constants.shaderCaps.maxTcsUniformBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "TCS active uniform blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
        }
        if (activeUBs[__GLSL_STAGE_TES] > gc->constants.shaderCaps.maxTesUniformBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "TES active uniform blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
        }
        if (activeUBs[__GLSL_STAGE_GS] > gc->constants.shaderCaps.maxGsUniformBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "GS active uniform blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
        }
        if (combinedUBs > gc->constants.shaderCaps.maxCombinedUniformBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "Combined active uniform blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_TOO_MANY_UNIFORMS);
        }

        /* The last index must equal to totalUniformCount */
        GL_ASSERT(ubIndex == program->totalUbCount + masterPgInstance->privateUbCount);
    }

    if (program->totalUbCount + masterPgInstance->privateUbCount > 0)
    {
        GLint userDefIndex = 0;
        GLint defaultIndex = program->userDefUbCount;
        GLint privateIndex = 0;

        /* Allocate the array of _GLUniform structures. */
        if (program->totalUbCount > 0)
        {
            bytes = program->totalUbCount * gcmSIZEOF(__GLchipSLUniformBlock);
            gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&program->uniformBlocks));
            gcoOS_ZeroMemory(program->uniformBlocks, bytes);
        }

        if (masterPgInstance->privateUbCount)
        {
            bytes = masterPgInstance->privateUbCount * gcmSIZEOF(__GLchipSLUniformBlock);
            gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&masterPgInstance->privateUBs));
            gcoOS_ZeroMemory(masterPgInstance->privateUBs, bytes);
        }

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                gcChipProcessUniformBlocks(gc,
                                           programObject,
                                           pBinaries[stage],
                                           resCounts[stage],
                                           GL_FALSE,
                                           uniformHALIdx2GL[stage],
                                           &userDefIndex,
                                           &defaultIndex,
                                           &privateIndex);
            }
        }
    }

    /* build uniform location mapping table after UB was processed */
    if (program->userDefUniformCount > 0)
    {
        gcmONERROR(gcChipBuildUniformLocationMapping(gc, program, logBuffer, &logOffset));
    }

    /* Allocate default UBOs */
    for (i = (GLuint)program->userDefUbCount; i < (GLuint)program->totalUbCount; ++i)
    {
        __GLchipSLUniformBlock *uBlock = &program->uniformBlocks[i];

        GL_ASSERT(uBlock->bufBase == gcvNULL);
        /* Allocate cacheable CPU memory for default uniform storage. */
        gcmONERROR(gcoOS_Allocate(chipCtx->os, uBlock->dataSize, (gctPOINTER*)&uBlock->bufBase));

        /* By default, all uniforms need to set as 0 */
        gcoOS_ZeroMemory(uBlock->bufBase, uBlock->dataSize);

        for (j = 0; j < uBlock->activeUniforms; ++j)
        {
            __GLchipSLUniform *uniform;
            GLuint uSlotIndex = uBlock->uniformIndices[j];

            GL_ASSERT(uSlotIndex < (GLuint)program->activeUniformCount);
            uniform = &program->uniforms[uSlotIndex];
            GL_ASSERT(uniform->data == gcvNULL && uniform->offset != -1);
            uniform->data = (GLvoid*)(uBlock->bufBase + uniform->offset);
        }

        /* If any shader stage mapped any part of UB to mem, allocate additional video memory. */
        if (uBlock->mapFlag & gcdUB_MAPPED_TO_MEM)
        {
            GL_ASSERT(uBlock->halBufObj == gcvNULL);
            gcmONERROR(gcoBUFOBJ_Construct(chipCtx->hal, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &uBlock->halBufObj));
            gcmONERROR(gcoBUFOBJ_Upload(uBlock->halBufObj, gcvNULL, 0, uBlock->dataSize, gcvBUFOBJ_USAGE_STATIC_READ));
        }
    }

    /* Allocate and upload private UBOs (currently only const ones) */
    for (i = 0; i < (GLuint)masterPgInstance->privateUbCount; ++i)
    {
        gctPOINTER data[__GLSL_STAGE_LAST] = {gcvNULL};
        __GLSLStage firstStage = __GLSL_STAGE_LAST;

        __GLchipSLUniformBlock *uBlock = &masterPgInstance->privateUBs[i];

        GL_ASSERT(uBlock->halBufObj == gcvNULL);
        GL_ASSERT(uBlock->activeUniforms == 0);

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (uBlock->halUB[stage])
            {
                data[stage] = GetShaderConstUBOData(pBinaries[stage]);

                if (__GLSL_STAGE_LAST == firstStage)
                {
                    firstStage = stage;
                }
                else
                {
                    GLuint j;
                    for (j = 0; j < uBlock->dataSize; j++)
                    {
                        ((GLubyte *)data[firstStage])[j] |= ((GLubyte *)data[stage])[j];
                    }
                }
            }
        }
        GL_ASSERT(data[firstStage]);
        gcmONERROR(gcoBUFOBJ_Construct(chipCtx->hal, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &uBlock->halBufObj));
        gcmONERROR(gcoBUFOBJ_Upload(uBlock->halBufObj, data[firstStage], 0, uBlock->dataSize, gcvBUFOBJ_USAGE_STATIC_READ));
    }

    /* Update GLcore object binding info */
    programObject->bindingInfo.maxInputNameLen = (GLuint)program->inMaxNameLen;
    programObject->bindingInfo.numActiveInput = (GLuint)program->inCount;
    programObject->bindingInfo.vsInputArrayMask = 0;

    for (i = 0; i < gc->constants.shaderCaps.maxUserVertAttributes; ++i)
    {
        if (program->attribLinkage[i])
        {
            programObject->bindingInfo.vsInputArrayMask |= (__GL_ONE_32 << i);
        }
    }

    programObject->bindingInfo.maxUniformNameLen = (GLuint)program->uniformMaxNameLen;
    programObject->bindingInfo.numActiveUniform = program->activeUniformCount;
    programObject->bindingInfo.numActiveUB = program->userDefUbCount;
    programObject->bindingInfo.maxUBNameLen = (GLuint)program->ubMaxNameLen;
    programObject->bindingInfo.maxActiveUniforms = program->maxActiveUniforms;

    programObject->bindingInfo.numActiveACBs = program->acbCount;
    programObject->bindingInfo.maxActiveACs = program->maxActiveACs;


    /***************************************************************************
    ** Transform feedback varying table.
    */
    lastNonFragStage = gcChipGetLastNonFragStage(program);
    if (pBinaries[lastNonFragStage])
    {
        gcmONERROR(gcSHADER_GetTransformFeedbackVaryingCount(pBinaries[lastNonFragStage], (gctUINT32_PTR)&program->xfbCount));
    }
    else
    {
        program->xfbCount = 0;
    }

    if (program->xfbCount > 0)
    {
        bytes = program->xfbCount * sizeof(__GLchipSLXfbVarying);
        gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&program->xfbVaryings));

        for (i = 0; i < program->xfbCount; ++i)
        {
            __GLchipSLXfbVarying *xfb = &program->xfbVaryings[i];

            gcmONERROR(gcSHADER_GetTransformFeedbackVarying(pBinaries[lastNonFragStage],
                                                            i,
                                                            &xfb->name,
                                                            &xfb->nameLen,
                                                            &xfb->type,
                                                            &xfb->isArray,
                                                            &xfb->arraySize));

            program->xfbMaxNameLen = __GL_MAX(program->xfbMaxNameLen, xfb->nameLen + 1);

            gcmONERROR(gcSHADER_GetTransformFeedbackVaryingStrideSeparate(pBinaries[lastNonFragStage], i, &xfb->stride));
        }

        gcmONERROR(gcSHADER_GetTransformFeedbackVaryingStride(pBinaries[lastNonFragStage], &program->xfbStride));
    }
    programObject->bindingInfo.xfbMode = programObject->xfbMode;
    programObject->bindingInfo.numActiveXFB = program->xfbCount;
    programObject->bindingInfo.maxXFBNameLen = (GLuint)program->xfbMaxNameLen;


    /**********************************************************************************
    **
    ** Collect output binding information from program. Fragment shader only for now.
    */

    lastStage = gcChipGetLastStage(program);
    GL_ASSERT(lastStage < __GLSL_STAGE_LAST);

    gcmONERROR(gcSHADER_GetOutputCount(pBinaries[lastStage], &resCount));
    if (resCount)
    {
        gctCONST_STRING *names = (gctCONST_STRING*)gc->imports.calloc(gc, resCount, sizeof(gctCONST_STRING));
        for (i = 0; i < resCount; ++i)
        {
            gcOUTPUT output;
            gcmONERROR(gcSHADER_GetOutput(pBinaries[lastStage], i, &output));
            if (output)
            {
                gctUINT location;
                gctCONST_STRING outName;
                gcmONERROR(gcOUTPUT_GetName(pBinaries[lastStage], output, gcvFALSE, gcvNULL, &outName));
                gcmONERROR(gcOUTPUT_GetLocation(output, &location));

                if (location != (gctUINT)-1 && program->maxOutLoc < location + 1)
                {
                    program->maxOutLoc = location + 1;
                }

                if (GetOutputIOBlockArrayIndex(output))
                {
                    continue;
                }

                /* Skip other array element if already record. */
                for (j = 0; j < program->outCount; ++j)
                {
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(outName, names[j])))
                    {
                        break;
                    }
                }

                if (j == program->outCount)
                {
                    names[program->outCount++] = outName;
                }
            }
        }
        gc->imports.free(gc, (gctPOINTER)names);
    }
    else
    {
        program->outCount = 0;
    }

    if (program->outCount > 0)
    {
        GLuint outIndex = 0;
        bytes = program->outCount * sizeof(__GLchipSLOutput);

        gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&program->outputs));
        gcoOS_ZeroMemory(program->outputs, bytes);

        if (program->maxOutLoc > 0)
        {
            bytes = program->maxOutLoc * sizeof(__GLchipSLOutput*);
            gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&program->loc2Out));
            gcoOS_ZeroMemory(program->outputs, bytes);
        }

        for (i = 0; i < resCount; ++i)
        {
            gcOUTPUT halOut;
            gcmONERROR(gcSHADER_GetOutput(pBinaries[lastStage], i, &halOut));

            if (halOut)
            {
                gctSTRING name = gcvNULL;
                gctUINT32 nameLen = 0;
                gctUINT location = 0;
                __GLchipSLOutput *output = gcvNULL;

                if (GetOutputIOBlockArrayIndex(halOut))
                {
                    continue;
                }

                gcmONERROR(gcOUTPUT_GetNameEx(pBinaries[lastStage], halOut, &nameLen, &name));

                for (j = 0; j < outIndex; ++j)
                {
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(name, program->outputs[j].name)))
                    {
                        break;
                    }
                }

                gcmONERROR(gcOUTPUT_GetLocation(halOut, &location));
                if (j == outIndex)
                {
                    output = &program->outputs[outIndex++];
                    output->location   = location;
                    output->name       = name;
                    output->nameLen    = nameLen;
                    output->type       = halOut->type;
                    output->precision  = halOut->precision;
                    output->isArray    = (gcmOUTPUT_isArray(halOut) ||
                                          (gcmOUTPUT_isPerVertexArray(halOut) && GetOutputNameLength(halOut) > 0));
                    output->arraySize  = halOut->arraySize;
                    output->startIndex = halOut->arrayIndex;
                    output->layout     = halOut->layoutQualifier;
                    output->fieldIndex = halOut->fieldIndex;
                    output->refByStage[lastStage] = GL_TRUE;
                    output->isPerPatch = gcmOUTPUT_isPerPatch(halOut);

                    if (output->isArray)
                    {
                        nameLen += 3;
                    }

                    program->outMaxNameLen = __GL_MAX(program->outMaxNameLen, nameLen + 1);
                }
                else
                {
                    output = &program->outputs[j];

                    /* Update startIndex and location if found less indexed array elements */
                    if (output->startIndex > halOut->arrayIndex)
                    {
                        output->location   = location;
                        output->startIndex = halOut->arrayIndex;
                    }

                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, name));
                }

                if (location != (gctUINT)-1 && location < program->maxOutLoc)
                {
                    program->loc2Out[location] = output;
                }
            }
        }
        GL_ASSERT(program->outCount == outIndex);
    }
    programObject->bindingInfo.numActiveOutput = program->outCount;
    programObject->bindingInfo.maxOutputNameLen = program->outMaxNameLen;


    /***************************************************************************
    ** Buffer Variable and Shader Storage Block management.
    */
    /* Get the number of storage blocks for each stage */
    combinedResCount = 0;
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (pBinaries[stage])
        {
            gcmONERROR(gcSHADER_GetStorageBlockCount(pBinaries[stage], &resCounts[stage]));
        }
        else
        {
            resCounts[stage] = 0;
        }
        combinedResCount += resCounts[stage];
    }

    if (combinedResCount > 0)
    {
        GLint sbIndex = 0;
        GLuint combinedSBs = 0;
        GLuint activeSBs[__GLSL_STAGE_LAST] = {0};
        gctCONST_STRING *names = (gctCONST_STRING*)gc->imports.calloc(gc, combinedResCount, sizeof(gctCONST_STRING));

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                activeSBs[stage] = gcChipCountStorageBlocks(program, pBinaries[stage], resCounts[stage], &sbIndex, names);
                combinedSBs += activeSBs[stage];
            }
        }
        program->totalSsbCount = program->userDefSsbCount + program->privateSsbCount;

        gc->imports.free(gc, (gctPOINTER)names);

        /* ES30 conform used more than maxVertUniformBlocks/maxFragUniformBlocks/maxCombinedUniformBlocks
        ** and expected compiler pass, while link fail.
        */
        if (activeSBs[__GLSL_STAGE_VS] > gc->constants.shaderCaps.maxVertShaderStorageBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "VS active shader storage blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }
        if (activeSBs[__GLSL_STAGE_FS] > gc->constants.shaderCaps.maxFragShaderStorageBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "PS active shader storage blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }
        if (activeSBs[__GLSL_STAGE_CS] > gc->constants.shaderCaps.maxCmptShaderStorageBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "CS active shader storage blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }
        if (activeSBs[__GLSL_STAGE_TCS] > gc->constants.shaderCaps.maxTcsShaderStorageBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "TCS active shader storage blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }
        if (activeSBs[__GLSL_STAGE_TES] > gc->constants.shaderCaps.maxTesShaderStorageBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "TES active shader storage blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }
        if (activeSBs[__GLSL_STAGE_GS] > gc->constants.shaderCaps.maxGsShaderStorageBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "GS active shader storage blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }
        if (combinedSBs > gc->constants.shaderCaps.maxCombinedShaderStorageBlocks)
        {
            gcoOS_StrCatSafe(programObject->programInfo.infoLog, __GLSL_LOG_INFO_SIZE,
                             "Combined active shader storage blocks exceeded max supported\n");
            gcmONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        /* The last index must equal to totalUniformCount */
        GL_ASSERT(sbIndex == (GLint)program->totalSsbCount);
    }

    if (program->totalSsbCount > 0)
    {
        GLint userDefIndex = 0;
        GLint privateIndex = program->userDefSsbCount;

        bytes = program->totalSsbCount * gcmSIZEOF(__GLchipSLStorageBlock);
        gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&program->ssbs));
        gcoOS_ZeroMemory(program->ssbs, bytes);

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (pBinaries[stage] && resCounts[stage])
            {
                gcChipProcessStorageBlocks(gc, programObject, pBinaries[stage], resCounts[stage], &userDefIndex, &privateIndex);
            }
        }

        /* Count buffer variables in SSB and build buffer variable table */
        gcChipCountBufferVariables(program);

        if(program->bufVariableCount)
        {
            GL_ASSERT(program->bufVariableCount);

            bytes = program->bufVariableCount * gcmSIZEOF(__GLchipSLBufVariable);
            gcmONERROR(gcoOS_Allocate(chipCtx->os, bytes, (gctPOINTER*)&program->bufVariables));
            gcoOS_ZeroMemory(program->bufVariables, bytes);
            /* Here we don't handle struct array, compiler would do this. */
            gcChipProcessBufferVariables(gc, programObject);
        }
    }

    /* Allocate private SSBOs */
    for (i = program->userDefSsbCount; i < program->totalSsbCount; ++i)
    {
        __GLchipSLStorageBlock *ssb = &program->ssbs[i];

        switch (ssb->usage)
        {
        case __GL_CHIP_SSB_USAGE_SHAREDVAR:
        case __GL_CHIP_SSB_USAGE_EXTRAREG:
            GL_ASSERT(ssb->halBufObj == gcvNULL);
            gcmONERROR(gcoBUFOBJ_Construct(chipCtx->hal, gcvBUFOBJ_TYPE_GENERIC_BUFFER, &ssb->halBufObj));
            gcmONERROR(gcoBUFOBJ_Upload(ssb->halBufObj, gcvNULL, 0, ssb->dataSize, gcvBUFOBJ_USAGE_STATIC_READ));
            ssb->groups = 1;
            break;
        default:
            GL_ASSERT(0);
            break;
        }
    }

    programObject->bindingInfo.numActiveBV = program->bufVariableCount;
    programObject->bindingInfo.maxBVNameLen = program->bvMaxNameLen;
    programObject->bindingInfo.numActiveSSB = program->userDefSsbCount;
    programObject->bindingInfo.maxSSBNameLen = program->ssbMaxNameLen;
    programObject->bindingInfo.maxActiveBVs = program->maxActiveBVs;


    /***************************************************************************
    ** Compute shader only info
    */
    if (pBinaries[__GLSL_STAGE_CS])
    {
        __GL_MEMCOPY(programObject->bindingInfo.workGroupSize,
                     GetShaderWorkGroupSize(pBinaries[__GLSL_STAGE_CS]),
                     3 * sizeof(GLuint));
    }
    /*******************************************************************************
    ** TS only info
    */
    if (pBinaries[__GLSL_STAGE_TCS])
    {
        gctINT outPatchVertices;
        gcmONERROR(gcSHADER_GetTCSPatchOutputVertices(pBinaries[__GLSL_STAGE_TCS], &outPatchVertices));
        programObject->bindingInfo.tessOutPatchSize = (GLuint)outPatchVertices;

        gcmONERROR(gcSHADER_GetTCSPatchInputVertices(pBinaries[__GLSL_STAGE_TCS], &masterPgInstance->tcsPatchInVertices));

    }

    if (pBinaries[__GLSL_STAGE_TES])
    {
        gcTessPrimitiveMode primMode;
        gcTessOrdering order;
        gcTessVertexSpacing spacing;
        gctBOOL pointMode;
        GLenum tessPrimitiveMode;
        static GLenum xlateTessMode[] = { GL_TRIANGLES, GL_QUADS_EXT, GL_ISOLINES_EXT };
        static GLenum xlateOrder[]= {GL_CCW, GL_CW};
        static GLenum xlateSpacing[] = {GL_EQUAL, GL_FRACTIONAL_EVEN_EXT, GL_FRACTIONAL_ODD_EXT};
        gcmONERROR(gcSHADER_GetTESPrimitiveMode(pBinaries[__GLSL_STAGE_TES], &primMode));
        gcmONERROR(gcSHADER_GetTESOrdering(pBinaries[__GLSL_STAGE_TES], &order));
        gcmONERROR(gcSHADER_GetTESVertexSpacing(pBinaries[__GLSL_STAGE_TES], &spacing));
        gcmONERROR(gcSHADER_GetTESPointMode(pBinaries[__GLSL_STAGE_TES], &pointMode));
        GL_ASSERT(primMode < gcmCOUNTOF(xlateTessMode));
        GL_ASSERT((size_t) order < gcmCOUNTOF(xlateOrder));
        GL_ASSERT(spacing < gcmCOUNTOF(xlateSpacing));
        programObject->bindingInfo.tessGenMode = xlateTessMode[primMode];
        programObject->bindingInfo.tessVertexOrder = xlateOrder[order];
        programObject->bindingInfo.tessSpacing = xlateSpacing[spacing];
        programObject->bindingInfo.tessPointMode = (GLboolean)pointMode;

        if (pointMode)
        {
            tessPrimitiveMode = GL_POINTS;
        }
        else
        {
            if (xlateTessMode[primMode] == GL_ISOLINES_EXT)
            {
                tessPrimitiveMode = GL_LINES;
            }
            else
            {
                tessPrimitiveMode = GL_TRIANGLES;
            }
        }
        programObject->bindingInfo.tessPrimitiveMode = tessPrimitiveMode;
    }

    /*******************************************************************************
    ** GS only info
    */
    if (pBinaries[__GLSL_STAGE_GS])
    {
        gcGEOLayout gsLayout;
        static const GLenum xlatePrimType[] =
            {
                GL_POINTS,
                GL_LINES,
                GL_LINES_ADJACENCY_EXT,
                GL_TRIANGLES,
                GL_TRIANGLES_ADJACENCY_EXT,
                GL_LINE_STRIP,
                GL_TRIANGLE_STRIP,
            };

        gcmONERROR(gcSHADER_GetGSLayout(pBinaries[__GLSL_STAGE_GS], &gsLayout));

        programObject->bindingInfo.gsOutVertices = (GLuint)gsLayout.geoMaxVertices;
        programObject->bindingInfo.gsInputType = xlatePrimType[gsLayout.geoInPrimitive];
        programObject->bindingInfo.gsOutputType = xlatePrimType[gsLayout.geoOutPrimitive];
        programObject->bindingInfo.gsInvocationCount = (GLuint)gsLayout.geoInvocations;
    }


#if __GL_CHIP_PATCH_ENABLED
    if (chipCtx->patchInfo.patchFlags.swClip)
    {
        program->aLocPosition = __glChipGetAttributeLocation(gc, programObject, chipCtx->patchInfo.posAttribName);

        /* If we have a bone matrix. Then we cannot do trivial reject. */
        if (gcChipGetUniformByName(gc, program, "bones", (gctSIZE_T)-1) == gcvNULL &&
            gcChipGetUniformByName(gc, program, "bone_orientations", (gctSIZE_T)-1) == gcvNULL &&
            gcChipGetUniformByName(gc, program, "particle_data", (gctSIZE_T)-1) == gcvNULL
            )
        {
            program->mvpUniform = gcChipGetUniformByName(gc, program, chipCtx->patchInfo.mvpUniformName, (gctSIZE_T)-1);
        }
    }

    if (chipCtx->patchInfo.patchFlags.discardDraw)
    {
        chipCtx->patchInfo.uniformDiscard = gcChipGetUniformByName(gc, program, "bDiscard", (gctSIZE_T)-1);
    }

    if (chipCtx->patchInfo.youilabs && chipCtx->patchInfo.patchCleanupProgram == program)
    {
        chipCtx->patchInfo.uniformTime = gcChipGetUniformByName(gc, program, "time", (gctSIZE_T)-1);
        chipCtx->patchInfo.uniformTemp = gcChipGetUniformByName(gc, program, "slope", (gctSIZE_T)-1);
    }

    if (chipCtx->patchInfo.patchFlags.countDraws)
    {
        chipCtx->patchInfo.uniformDrawCount = gcChipGetUniformByName(gc, program, "drawCount", (gctSIZE_T)-1);
    }

    if (program->progFlags.cube_UserLOD)
    {
        program->isUserLodUniform = gcChipGetUniformByName(gc, program, "u_isUserLod", (gctSIZE_T)-1);
        program->userLodUniform   = gcChipGetUniformByName(gc, program, "u_userLod",   (gctSIZE_T)-1);
        program->biasUniform      = gcChipGetUniformByName(gc, program, "u_bias",      (gctSIZE_T)-1);

        program->aLocPosition = __glChipGetAttributeLocation(gc, programObject, "a_position");
        program->aLocTexCoord = __glChipGetAttributeLocation(gc, programObject, "a_texCoord");
    }

    if (program->progFlags.helperInvocationCheck)
    {
        program->pointXYUniform = gcChipGetUniformByName(gc, program, "u_pointXY", (gctSIZE_T)-1);
        program->bXMajorUniform = gcChipGetUniformByName(gc, program, "u_bXMajor", (gctSIZE_T)-1);
        program->bCheckHelperUniform = gcChipGetUniformByName(gc, program, "u_bCheckHelper", (gctSIZE_T)-1);
        program->rtWHUniform = gcChipGetUniformByName(gc, program, "u_rtWH", (gctSIZE_T)-1);
        program->halfLineWidthUniform = gcChipGetUniformByName(gc, program, "u_halfLineWidth", (gctSIZE_T)-1);

        program->aLocPosition = __glChipGetAttributeLocation(gc, programObject, "a_position");
    }

    if (program->progFlags.wideLineFix)
    {
        program->aLocPosition = __glChipGetAttributeLocation(gc, programObject, "a_position");
    }

    if (program->progFlags.CTSMaxUBOSize)
    {
        program->indexUniform = gcChipGetUniformByName(gc, program, "index", (gctSIZE_T)-1);
    }
#endif

    /* Success. */

OnError:
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (uniformHALIdx2GL[stage])
        {
            gc->imports.free(gc, uniformHALIdx2GL[stage]);
        }
    }

    gcmFOOTER();
    return status;
}

 /* _dumpGLUniform */

gceSTATUS
gcChipSetUniformData(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    __GLchipSLProgram *program,
    __GLchipSLUniform *uniform,
    GLenum type,
    gctSIZE_T count,
    GLuint index,
    const void *values,
    GLboolean transpose)
{
    gcSHADER_TYPE halType;
    gctSIZE_T bytes  = 0;
    gctPOINTER buf = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x program=0x%x uniform=0x%x type=0x%04x "
                  "count=%d index=%d values=0x%x transpose=%d",
                   gc, programObject, program, uniform, type, count, index, values, transpose);

    gcmASSERT(uniform != gcvNULL);

    if ((count > 1) && (uniform->isArray == gcvFALSE))
    {
        __GL_ERROR(GL_INVALID_OPERATION);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (count > uniform->arraySize - (gctSIZE_T)index)
    {
        count = uniform->arraySize - (gctSIZE_T)index;
    }

    halType = uniform->dataType;

    /* make sure the table was not broken due to definition change */
    GL_ASSERT(g_typeInfos[halType].halType == halType);

    bytes = g_typeInfos[halType].size;
    buf   = (gctPOINTER)((gctINT8_PTR)uniform->data + (gctSIZE_T)index * bytes);

    switch (halType)
    {
    case gcSHADER_FLOAT_X1:
    case gcSHADER_FLOAT_X2:
    case gcSHADER_FLOAT_X3:
    case gcSHADER_FLOAT_X4:
    case gcSHADER_INTEGER_X1:
    case gcSHADER_INTEGER_X2:
    case gcSHADER_INTEGER_X3:
    case gcSHADER_INTEGER_X4:
    case gcSHADER_UINT_X1:
    case gcSHADER_UINT_X2:
    case gcSHADER_UINT_X3:
    case gcSHADER_UINT_X4:
        if (g_typeInfos[halType].glType == type)
        {
#if __GL_CHIP_PATCH_ENABLED
            if (program->progFlags.CTSMaxUBOSize &&
                (uniform == program->indexUniform))
            {
                program->uboIndex = *(GLint*)values;
                gcoOS_ZeroMemory(buf, count * bytes);
            }
            else
#endif
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_FLOAT_2X2:
        if (g_typeInfos[halType].glType == type)
        {
            if (transpose)
            {
                gcChipUtilTransposeMatrix(buf, values, count, 2, 2);
            }
            else
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_FLOAT_2X3:
        if (g_typeInfos[halType].glType == type)
        {
            if (transpose)
            {
                gcChipUtilTransposeMatrix(buf, values, count, 2, 3);
            }
            else
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_FLOAT_2X4:
        if (g_typeInfos[halType].glType == type)
        {
            if (transpose)
            {
                gcChipUtilTransposeMatrix(buf, values, count, 2, 4);
            }
            else
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_FLOAT_3X2:
        if (g_typeInfos[halType].glType == type)
        {
            if (transpose)
            {
                gcChipUtilTransposeMatrix(buf, values, count, 3, 2);
            }
            else
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_FLOAT_3X3:
        if (g_typeInfos[halType].glType == type)
        {
            if (transpose)
            {
                gcChipUtilTransposeMatrix(buf, values, count, 3, 3);
            }
            else
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_FLOAT_3X4:
        if (g_typeInfos[halType].glType == type)
        {
            if (transpose)
            {
                gcChipUtilTransposeMatrix(buf, values, count, 3, 4);
            }
            else
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_FLOAT_4X2:
        if (g_typeInfos[halType].glType == type)
        {
            if (transpose)
            {
                gcChipUtilTransposeMatrix(buf, values, count, 4, 2);
            }
            else
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_FLOAT_4X3:
        if (g_typeInfos[halType].glType == type)
        {
            if (transpose)
            {
                gcChipUtilTransposeMatrix(buf, values, count, 4, 3);
            }
            else
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_FLOAT_4X4:
        if (g_typeInfos[halType].glType == type)
        {
            if (transpose)
            {
                gcChipUtilTransposeMatrix(buf, values, count, 4, 4);
            }
            else
            {
                gcoOS_MemCopy(buf, values, count * bytes);
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_BOOLEAN_X1:
        switch (type)
        {
        case GL_INT:
        case GL_UNSIGNED_INT:
            /* int/unit can share the same converting function */
            gcChipUtilInt2Bool(buf, values, count);
            break;
        case GL_FLOAT:
            gcChipUtilFloat2Bool(buf, values, count);
            break;
        case GL_BOOL:
            gcoOS_MemCopy(buf, values, count * bytes);
            break;
        default:
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            break;
        }
        break;

    case gcSHADER_BOOLEAN_X2:
        switch (type)
        {
        case GL_INT_VEC2:
        case GL_UNSIGNED_INT_VEC2:
            /* int/unit can share the same converting function */
            gcChipUtilInt2Bool(buf, values, count * 2);
            break;
        case GL_FLOAT_VEC2:
            gcChipUtilFloat2Bool(buf, values, count * 2);
            break;
        case GL_BOOL_VEC2:
            gcoOS_MemCopy(buf, values, count * bytes);
            break;
        default:
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            break;
        }
        break;

    case gcSHADER_BOOLEAN_X3:
        switch (type)
        {
        case GL_INT_VEC3:
        case GL_UNSIGNED_INT_VEC3:
            /* int/unit can share the same converting function */
            gcChipUtilInt2Bool(buf, values, count * 3);
            break;
        case GL_FLOAT_VEC3:
            gcChipUtilFloat2Bool(buf, values, count * 3);
            break;
        case GL_BOOL_VEC3:
            gcoOS_MemCopy(buf, values, count * bytes);
            break;
        default:
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            break;
        }
        break;

    case gcSHADER_BOOLEAN_X4:
        switch (type)
        {
        case GL_INT_VEC4:
        case GL_UNSIGNED_INT_VEC4:
            /* int/unit can share the same converting function */
            gcChipUtilInt2Bool(buf, values, count * 4);
            break;
        case GL_FLOAT_VEC4:
            gcChipUtilFloat2Bool(buf, values, count * 4);
            break;
        case GL_BOOL_VEC4:
            gcoOS_MemCopy(buf, values, count * bytes);
            break;
        default:
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            break;
        }
        break;

    case gcSHADER_SAMPLER_2D:
    case gcSHADER_SAMPLER_2D_SHADOW:
    case gcSHADER_SAMPLER_2D_ARRAY:
    case gcSHADER_SAMPLER_2D_ARRAY_SHADOW:
    case gcSHADER_SAMPLER_3D:
    case gcSHADER_SAMPLER_CUBIC:
    case gcSHADER_SAMPLER_CUBE_SHADOW:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_SAMPLER_CUBEMAP_ARRAY_SHADOW:
    case gcSHADER_ISAMPLER_2D:
    case gcSHADER_ISAMPLER_2D_ARRAY:
    case gcSHADER_ISAMPLER_3D:
    case gcSHADER_ISAMPLER_CUBIC:
    case gcSHADER_ISAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_USAMPLER_2D:
    case gcSHADER_USAMPLER_2D_ARRAY:
    case gcSHADER_USAMPLER_3D:
    case gcSHADER_USAMPLER_CUBIC:
    case gcSHADER_USAMPLER_CUBEMAP_ARRAY:
    case gcSHADER_SAMPLER_EXTERNAL_OES:
    case gcSHADER_SAMPLER_2D_MS:
    case gcSHADER_SAMPLER_2D_MS_ARRAY:
    case gcSHADER_ISAMPLER_2D_MS:
    case gcSHADER_ISAMPLER_2D_MS_ARRAY:
    case gcSHADER_USAMPLER_2D_MS:
    case gcSHADER_USAMPLER_2D_MS_ARRAY:
    case gcSHADER_SAMPLER_BUFFER:
    case gcSHADER_ISAMPLER_BUFFER:
    case gcSHADER_USAMPLER_BUFFER:
    case gcSHADER_IMAGE_BUFFER:
    case gcSHADER_IIMAGE_BUFFER:
    case gcSHADER_UIMAGE_BUFFER:
        if (type == GL_INT)
        {
            gctUINT i;
            __GLSLStage stageIdx;

            /* Check value of out range */
            for (i = 0; i < (gctUINT)count; ++i)
            {
                GLint unit = *((const GLint *)values + i);
                if (unit < 0 || unit >= (GLint)gc->constants.shaderCaps.maxCombinedTextureImageUnits)
                {
                    __GL_ERROR(GL_INVALID_VALUE);
                    gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
            }

            gcoOS_MemCopy(buf, values, count * bytes);

            for (stageIdx = 0; stageIdx < __GLSL_STAGE_LAST; ++stageIdx)
            {
                if (uniform->halUniform[stageIdx])
                {
                    gctUINT sampler = 0;
                    gctUINT32 samplerBase = 0;
                    gctUINT arraySize = (gctUINT)(uniform->halUniform[stageIdx]->arrayLengthCount > 0 ?
                                                    uniform->halUniform[stageIdx]->arrayLengthList[uniform->halUniform[stageIdx]->arrayLengthCount - 1]
                                                    : 1);

                    gcmVERIFY_OK(gcUNIFORM_GetSampler(uniform->halUniform[stageIdx], &sampler));
                    samplerBase = gcHINTS_GetSamplerBaseOffset(program->curPgInstance->programState.hints,
                                                               program->curPgInstance->binaries[stageIdx]);
                    sampler += samplerBase;

                    for (i = 0; i < (gctUINT)count; ++i)
                    {
                        GLuint unit = *((const GLuint *)values + i);
                        if (program->samplerMap[sampler + i + index + arraySize * uniform->entryIdx].unit != unit)
                        {
                            program->samplerMap[sampler + i + index + arraySize * uniform->entryIdx].unit = unit;
                            if (__glIsStageProgramActive(gc, programObject, GL_ALL_SHADER_BITS))
                            {
                                __GL_SET_GLSL_SAMPLER_BIT(gc, (sampler + i + index + arraySize * uniform->entryIdx));
                                gc->shaderProgram.samplerSeq++;
                            }
                            programObject->samplerSeq++;
                        }
                        programObject->maxUnit = gcmMAX(unit + 1, programObject->maxUnit);
                    }
                }
            }
        }
        else
        {
            __GL_ERROR(GL_INVALID_OPERATION);
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case gcSHADER_IMAGE_2D:
    case gcSHADER_IIMAGE_2D:
    case gcSHADER_UIMAGE_2D:
    case gcSHADER_IMAGE_3D:
    case gcSHADER_IIMAGE_3D:
    case gcSHADER_UIMAGE_3D:
    case gcSHADER_IMAGE_CUBE:
    case gcSHADER_IIMAGE_CUBE:
    case gcSHADER_UIMAGE_CUBE:
    case gcSHADER_IMAGE_2D_ARRAY:
    case gcSHADER_IIMAGE_2D_ARRAY:
    case gcSHADER_UIMAGE_2D_ARRAY:
        __GL_ERROR(GL_INVALID_OPERATION);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        break;

    default:
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        break;
    }

    if (uniform->usage == __GL_CHIP_UNIFORM_USAGE_USER_DEFINED)
    {
        /* __GL_DIRTY_GLSL_UNIFORM only used for user-def uniforms */
        __GL_SET_ATTR_DIRTY_BIT(gc, __GL_PROGRAM_ATTRS, __GL_DIRTY_GLSL_UNIFORM);
    }
    uniform->dirty = GL_TRUE;

    if (gcmOPT_DUMP_UNIFORM())
    {
        gcChipDumpGLUniform(uniform, uniform->dataType, count, index);
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE GLboolean
gcChipIsTextureInfoState(
    IN __GLchipSLUniform *Uniform
    )
{
    return (isUniformLevelBaseSize(Uniform->halUniform[0]) ||
            isUniformLodMinMax(Uniform->halUniform[0]));
}

static GLvoid*
gcChipUniformMapStorage(
    IN  __GLcontext *gc,
    IN  __GLchipSLProgram *program,
    IN  gcSHADER shader,
    IN  gcUNIFORM halUniform,
    IN  __GLchipSLUniform *uniform,
    OUT gcoBUFOBJ *bufObj
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLvoid *data = gcvNULL;
    gctINT ubIndex = -1;
    gctCONST_STRING uniformBlockName;
    gctINT i;

    gcmHEADER_ARG("gc=0x%x program=0x%x shader=0x%x halUniform=0x%x uniform=0x%x bufObj=0x%x",
                  __GL_PTR2INT(gc), __GL_PTR2INT(program), shader, halUniform,
                  __GL_PTR2INT(uniform), __GL_PTR2INT(bufObj));

    if (uniform == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

    /* If the uniform is a block member of a uniform block array,
    ** we need to find the right uniform block index because we only save the first element of the array on the driver uniform.
    */
    if (isUniformBlockMember(halUniform))
    {
        gcmONERROR(
            gcUNIFORM_BLOCK_GetName(GetShaderUniformBlock(shader, GetUniformBlockID(halUniform)), gcvNULL, &uniformBlockName));

        /* Walk all UB to find the match one. */
        for (i = 0; i < program->userDefUbCount; ++i)
        {
            /* See if the uniform block matches the requested name. */
            if (gcmIS_SUCCESS(gcoOS_StrCmp(uniformBlockName, program->uniformBlocks[i].name)))
            {
                /* Match, return position. */
                ubIndex = i;
                break;
            }
        }
    }
    else
    {
        ubIndex = uniform->ubIndex;
    }

    if (ubIndex == -1)
    {
        /* If not from named UB, they must have CPU memory allocated for storage */
        GL_ASSERT(uniform->data);
        data = uniform->data;
    }
    else
    {
        __GLchipSLUniformBlock *ub = &program->uniformBlocks[ubIndex];
        __GLBufBindPoint *pBindingPoint;
        __GLchipVertexBufferInfo *bufInfo;
        GLubyte *baseAddr = gcvNULL;
        gctSIZE_T bytes;

        GL_ASSERT(uniform->offset >= 0);
        if (ub->binding >= gc->constants.shaderCaps.maxUniformBufferBindings)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        pBindingPoint = &gc->bufferObject.bindingPoints[__GL_UNIFORM_BUFFER_INDEX][ub->binding];
        if (!pBindingPoint->boundBufObj)
        {
            /* No buffer object was not bound to it */
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        bytes = g_typeInfos[uniform->dataType].size * uniform->arraySize;
        bufInfo = (__GLchipVertexBufferInfo*)(pBindingPoint->boundBufObj->privateData);
        if (!bufInfo || !bufInfo->bufObj ||
            (gctSIZE_T)pBindingPoint->bufOffset + uniform->offset + bytes > bufInfo->size)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, gcvNULL, (gctPOINTER*)&baseAddr));
        if (bufObj)
        {
            *bufObj = bufInfo->bufObj;
        }

        data = baseAddr + pBindingPoint->bufOffset + uniform->offset;
    }

OnError:
    gcmFOOTER_ARG("return=0x%x", __GL_PTR2INT(data));
    return data;
}

static gceSTATUS
gcChipUniformUnmapStorage(
    IN  __GLcontext *gc,
    IN  __GLchipSLProgram *program,
    IN  __GLchipSLUniform *uniform,
    OUT gcoBUFOBJ bufObj
    )

{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x program=0x%x uniform=0x%x, bufObj=0x%x",
                  __GL_PTR2INT(gc), __GL_PTR2INT(program),
                  __GL_PTR2INT(uniform), __GL_PTR2INT(bufObj));

    if (bufObj)
    {
        gcmONERROR(gcoBUFOBJ_Unlock(bufObj));
    }

OnError:
    gcmFOOTER();
    return status;
}

/* return the swizzle for Component in the Source */
static
gctUINT
gcChipLTCSwizzleSourceComponent(
    IN gctUINT Source,
    IN gctUINT Component
    )
{
    gctUINT ret;
    gcmHEADER_ARG("Source=%d Component=%d", Source, Component);
    /* Select on swizzle. */
    switch ((gcSL_SWIZZLE) Component)
    {
    case gcSL_SWIZZLE_X:
        /* Select swizzle x. */
        ret = gcmSL_SOURCE_GET(Source, SwizzleX);
        break;

    case gcSL_SWIZZLE_Y:
        /* Select swizzle y. */
        ret = gcmSL_SOURCE_GET(Source, SwizzleY);
        break;

    case gcSL_SWIZZLE_Z:
        /* Select swizzle z. */
        ret = gcmSL_SOURCE_GET(Source, SwizzleZ);
        break;

    case gcSL_SWIZZLE_W:
        /* Select swizzle w. */
        ret = gcmSL_SOURCE_GET(Source, SwizzleW);
        break;

    default:
        gcmASSERT(gcvFALSE);
        ret = (gctUINT16) 0xFFFF;
        break;
    }
    gcmFOOTER_ARG("return=%u", ret);
    return ret;
} /* _SwizzleSourceComponent */

/*
 * get value from Instructions' SourceId, the value must be from app's uniform
 */
static gceSTATUS
gcChipLTCGetUserUniformSourceValue(
    IN __GLcontext          *gc,
    IN __GLchipContext      *chipCtx,
    IN __GLchipSLProgram    *program,
    IN gcSHADER              Shader,
    IN gctUINT               InstructionIndex,
    IN gctINT                SourceId,
    IN LTCValue             *Results,
    IN OUT LTCValue         *SourceValue,
    OUT GLboolean           *IsUserUniform)
{
    gceSTATUS                  status = gcvSTATUS_OK;
    __GLchipSLProgramInstance* pgInstance = (__GLchipSLProgramInstance*)program->curPgInstance;
    gctINT                     i;
    gcUNIFORM                  uniform = gcvNULL;
    gctINT                     combinedOffset = 0;
    gctINT                     constOffset = 0;
    gctINT                     indexedOffset = 0;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x program=0x%x Shader=0x%x InstructionIndex=0x%x "
                  "SourceId=%d Results=0x%x SourceValue=0x%x IsUserUniform=0x%x",
                  gc, chipCtx, program, Shader, InstructionIndex, SourceId, Results,
                  SourceValue, IsUserUniform);

    /* set value type */
    *IsUserUniform = gcvFALSE;
    do
    {
        gcmONERROR(gcOPT_GetUniformSrcLTC(Shader, InstructionIndex, SourceId, Results,
                                          &uniform, &combinedOffset, &constOffset,
                                          &indexedOffset, SourceValue));

        if (uniform)
        {
            gctCONST_STRING     uniformName;
            __GLchipSLUniform   *glUniform = gcvNULL;
            GLvoid *            uniformStorage;
            gctFLOAT *          data;
            gctUINT32           rows, components, arrayStride;
            gcoBUFOBJ           bufObj = NULL;
            gctSIZE_T           stride;
            gctINT16            glUniformIndex = GetUniformGlUniformIndex(uniform);
            gctINT16            arraySize = 1;

            /* If it is compile time initialzied uniform, just get the data from this uniform. */
            if (isUniformCompiletimeInitialized(uniform))
            {
                /* get the data from GLUniform one component at a time */
                for (i = 0; i < MAX_LTC_COMPONENTS; i++)
                {
                    gctUINT sourceComponent;

                    /* get the swizzled source component for channel i (corresponding to target ith component) */
                    sourceComponent = gcChipLTCSwizzleSourceComponent(SourceValue->sourceInfo, (gctUINT16)i);

                    if (SourceValue->elementType == gcSL_FLOAT)
                    {
                        SourceValue->v[i].f32 = GetUniformInitializer(uniform).f32_v4[sourceComponent];
                    }
                    else if (SourceValue->elementType == gcSL_INTEGER)
                    {
                        /* integer value */
                        SourceValue->v[i].i32 = GetUniformInitializer(uniform).i32_v4[sourceComponent];
                    }
                    else if (SourceValue->elementType == gcSL_UINT32)
                    {
                        /* uint value */
                        SourceValue->v[i].u32 = GetUniformInitializer(uniform).u32_v4[sourceComponent];
                    }
                    else if (SourceValue->elementType == gcSL_BOOLEAN)
                    {
                        /* bool value */
                        gctINT temp = GetUniformInitializer(uniform).u32_v4[sourceComponent];
                        SourceValue->v[i].b = (temp == 0) ? gcvFALSE : gcvTRUE;
                    }
                    else
                    {
                        /* Error. */
                        status = gcvSTATUS_INVALID_DATA;
                        break;
                    }
                }
                *IsUserUniform = gcvTRUE;
                break;
            }


            gcTYPE_GetTypeInfo(GetUniformType(uniform), &components, &rows, 0);

            if (uniform->arrayLengthCount > 1)
            {
                arraySize = (gctINT16)uniform->arrayLengthList[uniform->arrayLengthCount - 1] * (gctINT16)rows;

                glUniformIndex += (gctINT16)(combinedOffset / arraySize);
            }

            if (glUniformIndex >= program->activeUniformCount + pgInstance->privateUniformCount)
            {
                /* the access is out of bounds */
                status = gcvSTATUS_INVALID_DATA;
                break;
            }

            if (glUniformIndex != (gctINT16)-1)
            {
                if (glUniformIndex < program->activeUniformCount)
                {
                    /* It's an userDef or buildin uniform */

                    glUniform = &program->uniforms[glUniformIndex];
                    if (uniform->arrayLengthCount > 1)
                    {
                        gctINT i;

                        for (i = 0; i < __GLSL_STAGE_LAST; i++)
                        {
                            if (glUniform->halUniform[i] != gcvNULL)
                            {
                                gcmASSERT(glUniform->halUniform[i] ==
                                  program->uniforms[glUniformIndex - (gctINT16)(combinedOffset / arraySize)].halUniform[i]);
                            }
                        }
                        combinedOffset = combinedOffset % arraySize;
                    }
                }
                else
                {
                    /* It's a private uniform, get the index in that array */
                    glUniformIndex -= (gctINT16)program->activeUniformCount;
                    glUniform = &program->curPgInstance->privateUniforms[glUniformIndex];
                }
            }
            else
            {
                /* check if the combined indexed offset is out of bounds of the uniform */
                gcmONERROR(gcUNIFORM_GetName(uniform, gcvNULL, &uniformName));

                /* find the uniform data in the  program */
                for (i = 0; i < program->activeUniformCount; ++i)
                {
                    /* See if the uniform matches the requested name. */
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(uniformName, program->uniforms[i].name)))
                    {
                        /* Match, return position. */
                        glUniform = &program->uniforms[i];
                        break;
                    }
                }
            }

            if (glUniform == gcvNULL)
            {
                /* the access is out of bounds */
                status = gcvSTATUS_INVALID_DATA;
                break;
            }

            /* check if the combined indexed offset is out of bounds of the uniform */
            if (GetUniformArraySize(uniform) == 1 || GetUniformArrayStride(uniform) == -1)
            {
                arrayStride = rows;
            }
            else
            {
                arrayStride = (gctSIZE_T)GetUniformArrayStride(uniform);
            }

            if (combinedOffset >= (gctINT)(GetUniformArraySize(uniform) * arrayStride))
            {
                /* the uniform is not found in program */
                status = gcvSTATUS_INVALID_DATA;
                break;
            }

            uniformStorage = gcChipUniformMapStorage(gc, program, Shader, uniform, glUniform, &bufObj);
            gcmASSERT(uniformStorage != gcvNULL);

            if(uniformStorage == gcvNULL)
            {
                status = gcvSTATUS_INVALID_DATA;
                break;
            }

            data = (gctFLOAT*)uniformStorage;

            if (glUniform->matrixStride != 0 && glUniform->matrixStride != -1)
            {
                stride = glUniform->matrixStride / 4;
            }
            else if (glUniform->arrayStride != 0 && glUniform->arrayStride != -1)
            {
                stride = glUniform->arrayStride / 4;
            }
            else
            {
                if (glUniform->isRowMajor)
                {
                    stride = rows;
                }
                else
                {
                    stride = components;
                }
            }

            /* get the data from GLUniform one component at a time */
            for (i = 0; i < MAX_LTC_COMPONENTS; i++)
            {
                 gctUINT sourceComponent;
                 gctUINT index;

                /* get the swizzled source component for channel i
                 * (corresponding to target ith component) */
                sourceComponent = gcChipLTCSwizzleSourceComponent(SourceValue->sourceInfo, (gctUINT16)i);

                if (glUniform->isRowMajor)
                {
                    if (rows > 1)
                    {
                        index = (gctUINT)((sourceComponent * stride) + (combinedOffset / rows) * (stride * components) + combinedOffset % rows);
                    }
                    else
                    {
                        index = (gctUINT)((sourceComponent * stride) + combinedOffset);
                    }
                }
                else
                {
                    index = (gctUINT)((combinedOffset * stride) + sourceComponent);
                }

                if (SourceValue->elementType == gcSL_FLOAT)
                {
                    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
                    __GLchipFeature *chipFeature = &chipCtx->chipFeature;

                    /* float value */
                    if (gcChipQueryUniformConvert(program, GetUniformType(uniform), gcvFALSE, chipFeature->haltiLevel > __GL_CHIP_HALTI_LEVEL_0) == gcvUNIFORMCVT_TO_FLOAT)
                    {
                        SourceValue->v[i].f32 = (gctFLOAT)((GLint*)data)[index];
                    }
                    else
                    {
                        SourceValue->v[i].f32 = data[index];
                    }
                }
                else if (SourceValue->elementType == gcSL_INTEGER)
                {
                    /* integer value */
                    SourceValue->v[i].i32 = ((gctINT*)data)[index];
                }
                else if (SourceValue->elementType == gcSL_UINT32)
                {
                    /* uint value */
                    SourceValue->v[i].u32 = ((gctUINT32*)data)[index];
                }
                else if (SourceValue->elementType == gcSL_BOOLEAN)
                {
                    /* bool value */
                    gctINT temp = ((gctINT*)data)[index];
                    SourceValue->v[i].b = (temp == 0) ? gcvFALSE : gcvTRUE;
                }
                else
                {
                    /* Error. */
                    status = gcvSTATUS_INVALID_DATA;
                    break;
                }
            }

            gcmONERROR(gcChipUniformUnmapStorage(gc, program, glUniform, bufObj));

            *IsUserUniform = gcvTRUE;
        }
    }while(gcvFALSE);

 OnError:
    /* error handling */
    gcmASSERT(gcvSTATUS_INVALID_DATA != status);
    /* Return the status. */
    gcmFOOTER();
    return status;
} /* _GetSourceValue */

static gctINT
gcChipLTCGetFirstComponent(
       IN gcSL_ENABLE enable
       )
{
    gctINT firstComponent = 0;
    gctINT i = 0;

    for (i = 0; i < 4; i++)
    {
        if ((enable >> i) & 1)
        {
            firstComponent = i;
            break;
        }
    }
    return firstComponent;
}

/* Get source value from a UBO. */
static gceSTATUS
gcChipLTCGetSourceValueFromUBO(
    IN __GLcontext           *gc,
    IN __GLchipContext       *chipCtx,
    IN __GLchipSLProgram    *program,
    IN gcSHADER              Shader,
    IN gctUINT               InstructionIndex,
    IN OUT LTCValue          *SourceValue
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLUniformBlock * ub = gcvNULL;
    __GLchipVertexBufferInfo *bufInfo;
    __GLBufBindPoint *pBindingPoint;
    GLubyte * baseAddr = gcvNULL;
    gctUINT8_PTR data = gcvNULL;
    gctINT i, firstComponent;
    gctCONST_STRING uniformName;
    /* Hal variables. */
    gcUNIFORM uniform = gcvNULL;
    gcSL_INSTRUCTION inst = GetShaderLtcExpression(Shader, InstructionIndex);
    gcSL_FORMAT format = gcmSL_TARGET_GET(GetInstTemp(inst), Format);
    gcSL_ENABLE enable = gcmSL_TARGET_GET(GetInstTemp(inst), Enable);
    gctINT enableCount[16] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
    gctUINT constIndex = gcmSL_INDEX_GET(GetInstSource0Index(inst), ConstValue) + GetInstSource0Indexed(inst);

    /* Get the hal UBO. */
    gcmONERROR(gcSHADER_GetUniform(Shader, gcmSL_INDEX_GET(GetInstSource0Index(inst), Index), &uniform));
    gcmASSERT(isUniformUBOAddress(uniform));

    gcmONERROR(gcUNIFORM_GetName(uniform, gcvNULL, &uniformName));

    for (i = 0; i < program->userDefUbCount; ++i)
    {
        /* See if the uniform matches the requested name. */
        if (gcmIS_SUCCESS(gcoOS_StrCmp(uniformName, program->uniformBlocks[i].name)))
        {
            /* Match, return position. */
            ub = &program->uniformBlocks[i + constIndex];
            break;
        }
    }

    gcmASSERT(ub);

    if (ub->binding >= gc->constants.shaderCaps.maxUniformBufferBindings)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    pBindingPoint = &gc->bufferObject.bindingPoints[__GL_UNIFORM_BUFFER_INDEX][ub->binding];
    if (!pBindingPoint->boundBufObj)
    {
        /* No buffer object was not bound to it */
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    bufInfo = (__GLchipVertexBufferInfo*)(pBindingPoint->boundBufObj->privateData);
    if (!bufInfo || !bufInfo->bufObj)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Lock memory. */
    gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, gcvNULL, (gctPOINTER*)&baseAddr));

    /* Get the data start address. */
    data = (gctUINT8_PTR)(baseAddr + pBindingPoint->bufOffset) + GetInstSource1Index(inst);

    SourceValue->elementType = format;
    SourceValue->enable = enable;

    firstComponent = gcChipLTCGetFirstComponent(enable);

    for (i = 0; i < enableCount[enable]; i++)
    {
        switch(format)
        {
        case gcSL_FLOAT:
            SourceValue->v[firstComponent + i].f32 = *((gctFLOAT *)data+i);
            break;

        case gcSL_INTEGER:
            SourceValue->v[firstComponent + i].i32 = *((gctINT *)data+i);
            break;

        case gcSL_UINT32:
            SourceValue->v[firstComponent + i].u32 = *((gctUINT32 *)data+i);
            break;

        case gcSL_BOOLEAN:
            {
                gctINT value = *((gctINT *)data+i);
                SourceValue->v[firstComponent + i].b = (value == 0 ? gcvFALSE : gcvTRUE);
                break;
            }

        default:
            /* Right now, we only support these 4 formats. */
            gcmASSERT(gcvFALSE);
            break;
        }
    }

    /* Unlock memory. */
    gcmONERROR(gcoBUFOBJ_Unlock(bufInfo->bufObj));

OnError:
    return status;
}

/* Allocate memory from the heap. */
static gceSTATUS
gcChipLTCGetResultArray(
    IN __GLchipContext   * chipCtx,
    IN gcoOS               Os,
    IN gctSIZE_T           Size,
    OUT gctPOINTER       * Memory
    )
{
    gceSTATUS status;
    gcmHEADER_ARG("chipCtx=0x%x Os=0x%x Size=%d Memory=0x%x",chipCtx, Os, Size, Memory);

    if (chipCtx->curLTCResultArraySize >= Size)
    {
        /* cached result array is larger than requested */
        gcmASSERT(chipCtx->cachedLTCResultArray != gcvNULL);
        *Memory = chipCtx->cachedLTCResultArray;
        status = gcvSTATUS_OK;
        gcmFOOTER();
        return status;
    }

    if (chipCtx->cachedLTCResultArray != gcvNULL)
    {
        /* requested size is larger than cached size, reallocate */
        gcoOS_Free(Os, chipCtx->cachedLTCResultArray);
        chipCtx->cachedLTCResultArray = gcvNULL;
        chipCtx->curLTCResultArraySize = 0;
    }

    /* allocate a result array at least for 100 instructions */
    gcmASSERT(chipCtx->curLTCResultArraySize == 0);
    chipCtx->curLTCResultArraySize = Size > sizeof(LTCValue)*100 ? Size : sizeof(LTCValue)*100;
    status = gcoOS_Allocate(Os,
                            chipCtx->curLTCResultArraySize,
                            (gctPOINTER *)&chipCtx->cachedLTCResultArray);
    if(status == gcvSTATUS_OK)
    {
        gcoOS_ZeroMemory(chipCtx->cachedLTCResultArray, chipCtx->curLTCResultArraySize);
        *Memory = chipCtx->cachedLTCResultArray;
    }

    gcmFOOTER();
    return  status;
}

gceSTATUS
gcChipLTCReleaseResultArray(
    IN __GLchipContext   * chipCtx,
    IN gcoOS               Os
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("chipCtx=0x%x Os=0x%x",chipCtx, Os);

    if (chipCtx->cachedLTCResultArray != gcvNULL)
    {
        /* requested size is larger than cached size, reallocate */
        gcmONERROR(gcoOS_Free(Os, chipCtx->cachedLTCResultArray));
        chipCtx->cachedLTCResultArray = gcvNULL;
        chipCtx->curLTCResultArraySize = 0;
    }

    gcmASSERT(chipCtx->curLTCResultArraySize == 0);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/* store the Value to Shader's uniform at LtcUniformIndex */
static gceSTATUS
gcChipLTCStoreValueToDummyUniform(
    IN __GLcontext          *gc,
    IN __GLchipContext      *chipCtx,
    IN __GLchipSLProgram    *program,
    IN gcSHADER              Shader,
    IN LTCValue             *Value,
    IN gctINT                LtcUniformIndex
    )
{
    gceSTATUS      status = gcvSTATUS_OK;
    __GLchipSLProgramInstance* pgInstance = program->curPgInstance;
    __GLchipSLUniform *glUniform;
    gctINT         glUniformIndex;
    gctINT         i = 0;
    gcSL_ENABLE    enable = Value->enable;
    gcUNIFORM      uniform = gcvNULL;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x Shader=0x%x Value=0x%x LtcUniformIndex=%d",
                   gc, chipCtx, Shader, Value, LtcUniformIndex);

    gcmASSERT(LtcUniformIndex >= 0 &&
              LtcUniformIndex < (gctINT)GetShaderUniformCount(Shader));
    gcmASSERT(enable != gcSL_ENABLE_NONE);

    uniform = GetShaderUniform(Shader, LtcUniformIndex);
    glUniformIndex = GetUniformGlUniformIndex(uniform);
    GL_ASSERT(glUniformIndex != -1);

    /* The dst dummy uniform must be a private one */
    GL_ASSERT(glUniformIndex >= program->activeUniformCount);
    /* Get the index in private uniform table */
    glUniformIndex -= program->activeUniformCount;
    GL_ASSERT(glUniformIndex < pgInstance->privateUniformCount);
    glUniform = &pgInstance->privateUniforms[glUniformIndex];

    gcmASSERT(uniform == glUniform->halUniform[__GLSL_STAGE_VS]  ||
              uniform == glUniform->halUniform[__GLSL_STAGE_FS]  ||
              uniform == glUniform->halUniform[__GLSL_STAGE_CS]  ||
              uniform == glUniform->halUniform[__GLSL_STAGE_TCS] ||
              uniform == glUniform->halUniform[__GLSL_STAGE_TES] ||
              uniform == glUniform->halUniform[__GLSL_STAGE_GS]);

    /* make sure the table was not broken due to definition change */
    gcmASSERT(g_typeInfos[glUniform->dataType].halType == glUniform->dataType);

    if (Value->elementType == gcSL_FLOAT)
    {
        gctFLOAT f32[4] = {0.0};
        if (0 != (enable & gcSL_ENABLE_X))
            f32[i++] = Value->v[0].f32;

        if (0 != (enable & gcSL_ENABLE_Y))
            f32[i++] = Value->v[1].f32;

        if (0 != (enable & gcSL_ENABLE_Z))
            f32[i++] = Value->v[2].f32;

        if (0 != (enable & gcSL_ENABLE_W))
            f32[i++] = Value->v[3].f32;

        gcmONERROR(gcChipSetUniformData(gc,
                                        gcvNULL,
                                        program,
                                        glUniform,
                                        g_typeInfos[glUniform->dataType].glType,
                                        1  /* Count */,
                                        0  /* Index */,
                                        f32,
                                        GL_FALSE));
    }
    else if (Value->elementType == gcSL_INTEGER)
    {
        gctINT i32[4] = {0};
        if (0 != (enable & gcSL_ENABLE_X))
            i32[i++] = Value->v[0].i32;

        if (0 != (enable & gcSL_ENABLE_Y))
            i32[i++] = Value->v[1].i32;

        if (0 != (enable & gcSL_ENABLE_Z))
            i32[i++] = Value->v[2].i32;

        if (0 != (enable & gcSL_ENABLE_W))
            i32[i++] = Value->v[3].i32;

        gcmONERROR(gcChipSetUniformData(gc,
                                        gcvNULL,
                                        program,
                                        glUniform,
                                        g_typeInfos[glUniform->dataType].glType,
                                        1  /* Count */,
                                        0  /* Index */,
                                        i32,
                                        GL_FALSE));
    }
    else if (Value->elementType == gcSL_UINT32)
    {
        gctINT u32[4] = {0};
        if (0 != (enable & gcSL_ENABLE_X))
            u32[i++] = Value->v[0].u32;

        if (0 != (enable & gcSL_ENABLE_Y))
            u32[i++] = Value->v[1].u32;

        if (0 != (enable & gcSL_ENABLE_Z))
            u32[i++] = Value->v[2].u32;

        if (0 != (enable & gcSL_ENABLE_W))
            u32[i++] = Value->v[3].u32;

        gcmONERROR(gcChipSetUniformData(gc,
                                        gcvNULL,
                                        program,
                                        glUniform,
                                        g_typeInfos[glUniform->dataType].glType,
                                        1  /* Count */,
                                        0  /* Index */,
                                        u32,
                                        GL_FALSE));
    }
    else if (Value->elementType == gcSL_BOOLEAN)
    {
        gctBOOL boolean[4] = {0};
        if (0 != (enable & gcSL_ENABLE_X))
            boolean[i++] = Value->v[0].b;

        if (0 != (enable & gcSL_ENABLE_Y))
            boolean[i++] = Value->v[1].b;

        if (0 != (enable & gcSL_ENABLE_Z))
            boolean[i++] = Value->v[2].b;

        if (0 != (enable & gcSL_ENABLE_W))
            boolean[i++] = Value->v[3].b;

        gcmONERROR(gcChipSetUniformData(gc,
                                        gcvNULL,
                                        program,
                                        glUniform,
                                        g_typeInfos[glUniform->dataType].glType,
                                        1  /* Count */,
                                        0  /* Index */,
                                        boolean,
                                        GL_FALSE));
    }
    else
    {
        gcmASSERT(gcvFALSE);
    } /* if */

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
} /* _StoreValueToDummyUniform */

/***************************************************************************
**                   _EvaluateLoadtimeConstantExpresion
****************************************************************************
**
**   For instruction number 'InstructionIndex' in the Shader, evaluate the
**   instruction and store the evaluated constant value into Results
**
*/
static gceSTATUS
gcChipLTCEvaluateLoadtimeConstantExpresion(
    IN __GLcontext          *gc,
    IN __GLchipContext      *chipCtx,
    IN __GLchipSLProgram    *program,
    IN gcSHADER              Shader,
    IN gctUINT               InstructionIndex,
    IN LTCValue             *Results
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gctINT      ltcUniformIndex;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x program=0x%x shader=0x%x InstructionIndex=%d Results=0x%x",
                   gc, chipCtx, program, Shader, InstructionIndex, Results);

    gcmASSERT(InstructionIndex <= GetShaderLtcInstructionCount(Shader));

    ltcUniformIndex = GetShaderLtcCodeUniformIndex(Shader, InstructionIndex);

    /* for each load time constant, mark it. */
    do
    {
        /* get the source0 and source1 */
        LTCValue  source0Value;
        LTCValue  source1Value;
        LTCValue  source2Value;
        LTCValue  resultValue;
        GLboolean isSrc0UserUniform = gcvFALSE;
        GLboolean isSrc1UserUniform = gcvFALSE;
        GLboolean isSrc2UserUniform = gcvFALSE;
        gctBOOL   hasSource2 = gcvFALSE;

        if (GetInstOpcode(GetShaderLtcExpression(Shader, InstructionIndex)) == gcSL_LOAD)
        {
            gcmONERROR(gcChipLTCGetSourceValueFromUBO(gc, chipCtx, program, Shader, InstructionIndex, &source0Value));
            isSrc0UserUniform = gcvTRUE;
        }
        else if (GetInstOpcode(GetShaderLtcExpression(Shader, InstructionIndex)) == gcSL_SET)
        {
            hasSource2 = gcvTRUE;
            gcmONERROR(gcChipLTCGetUserUniformSourceValue(gc, chipCtx, program, Shader, InstructionIndex,
                                                          0, Results, &source0Value, &isSrc0UserUniform));
            gcmONERROR(gcChipLTCGetUserUniformSourceValue(gc, chipCtx, program, Shader, InstructionIndex,
                                                          1, Results, &source1Value, &isSrc1UserUniform));
            gcmONERROR(gcChipLTCGetUserUniformSourceValue(gc, chipCtx, program, Shader, InstructionIndex + 1,
                                                          1, Results, &source2Value, &isSrc2UserUniform));
        }
        else
        {
            gcmONERROR(gcChipLTCGetUserUniformSourceValue(gc, chipCtx, program, Shader, InstructionIndex,
                                                          0, Results, &source0Value, &isSrc0UserUniform));
            gcmONERROR(gcChipLTCGetUserUniformSourceValue(gc, chipCtx, program, Shader, InstructionIndex,
                                                          1, Results, &source1Value, &isSrc1UserUniform));
        }


        /* Call compiler code to do LTC constant folding */
        gcmONERROR(gcOPT_DoConstantFoldingLTC(Shader, InstructionIndex,
                                              isSrc0UserUniform ? &source0Value : gcvNULL,
                                              isSrc1UserUniform ? &source1Value : gcvNULL,
                                              isSrc2UserUniform ? &source2Value : gcvNULL,
                                              hasSource2,
                                              &resultValue, Results));

        /* check if the result need to store to dummy uniform */
        /* If this is a JMP instruction, we can't need to create a dummy uniform. */
        if (ltcUniformIndex != -1 && GetInstOpcode(GetShaderLtcExpression(Shader, InstructionIndex)) != gcSL_JMP)
        {
            gcmASSERT(ltcUniformIndex >= 0 && ltcUniformIndex < (gctINT)GetShaderUniformCount(Shader));

            /* If a LTC uniform is not used, don't need to update the data. */
            if (GetShaderUniform(Shader, ltcUniformIndex) && isUniformInactive(GetShaderUniform(Shader, ltcUniformIndex)))
            {
                gcmFOOTER();
                return status;
            }

            /* the instruction's result is a dummy uniform value ,
             * need to store the value to the uniform */
            gcmONERROR(gcChipLTCStoreValueToDummyUniform(gc,
                                                         chipCtx,
                                                         program,
                                                         Shader,
                                                         &resultValue,
                                                         ltcUniformIndex));
        }
        /*  dump evaluated uniform value */
    } while (gcvFALSE);

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                          gcEvaluateLoadtimeConstantExpresions
********************************************************************************
**
**  evaluate the LTC expression with the uniform input
**
**
*/
gceSTATUS
gcChipLTCEvaluateLoadtimeConstantExpresions(
    IN __GLcontext        *gc,
    IN __GLchipContext    *chipCtx,
    IN __GLchipSLProgram  *program,
    IN gcSHADER            shader
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    LTCValue    *results = gcvNULL;
    gctUINT     instructions;
    gctUINT     i;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x program=0x%x shader=0x%x", gc, chipCtx, program, shader);

    /* do nothing if there is no Loadtime Constant Expression */
    if (GetShaderLtcUniformCount(shader) == 0)
    {
        gcmFOOTER();
        return status;
    }

    instructions = GetShaderLtcInstructionCount(shader);

    GL_ASSERT(instructions && GetShaderLtcExpressions(shader));

    /* allocate result array */
    gcmONERROR(gcChipLTCGetResultArray(chipCtx,
                                       gcvNULL,
                                       instructions * sizeof(LTCValue),
                                       (gctPOINTER*)&results));

    /* evaluate all instructions */
    for (i = 0; i < instructions; ++i)
    {
        results[i].instructionIndex = i;

        gcmONERROR(gcChipLTCEvaluateLoadtimeConstantExpresion(gc,
                                                              chipCtx,
                                                              program,
                                                              shader,
                                                              i,
                                                              results));

        /* If this is a JMP instruction,
        ** we need to jump to the right position according to the result.
        */
        if (GetInstOpcode(GetShaderLtcExpression(shader, i)) == gcSL_JMP)
        {
            if (results[i].v[0].b)
            {
                i = GetInstTempIndex(GetShaderLtcExpression(shader, i)) - 1;
            }
        }
        else if (GetInstOpcode(GetShaderLtcExpression(shader, i)) == gcSL_SET)
        {
            gctINT ltcUniformIndex;

            results[i + 1] = results[i];
            i++;

            ltcUniformIndex = GetShaderLtcCodeUniformIndex(shader, i);

            if (ltcUniformIndex != -1)
            {
                gcmASSERT(ltcUniformIndex >= 0 && ltcUniformIndex < (gctINT)GetShaderUniformCount(shader));

                /* If a LTC uniform is not used, don't need to update the data. */
                if (GetShaderUniform(shader, ltcUniformIndex) && isUniformInactive(GetShaderUniform(shader, ltcUniformIndex)))
                {
                    gcmFOOTER();
                    return status;
                }

                /* the instruction's result is a dummy uniform value ,
                 * need to store the value to the uniform */
                gcmONERROR(gcChipLTCStoreValueToDummyUniform(gc,
                                                             chipCtx,
                                                             program,
                                                             shader,
                                                             &results[i],
                                                             ltcUniformIndex));
            }
        }
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

/*******************************************************************************
**                          gcComputeLoadtimeConstant
********************************************************************************
**
**  compute the loadtime constant in the Context
**
**
*/
static gceSTATUS
gcChipLTCComputeLoadtimeConstant(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLchipSLProgram *program
    )
{
    __GLSLStage stage;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x", gc, chipCtx);

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        gcSHADER binary = program->curPgInstance->binaries[stage];
        if (binary)
        {
            gcmONERROR(gcChipLTCEvaluateLoadtimeConstantExpresions(gc, chipCtx, program, binary));
        }
    }

OnError:
    /* Return the status. */
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipFlushSingleUniform(
    __GLcontext *       gc,
    __GLchipSLProgram * program,
    __GLchipSLUniform * uniform,
    GLvoid *            data
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x program=0x%x uniform=0x%x data=0x%x", gc, program, uniform, data);

    /* Only upload to state buffer if
    ** 1. It was not put in UBO, and
    ** 2. It was not any sampler/image/atomic_counter type
    */
    if (uniform->ubIndex == -1 &&
        ((uniform->category < gceTK_SAMPLER) ||
        ((uniform->dataType >= gcSHADER_SAMPLER_BUFFER) && (uniform->dataType <= gcSHADER_UIMAGE_BUFFER)))
        )
    {
        __GLSLStage stageIdx;
        gctUINT32 columns, rows;
        gctSIZE_T arraySize;
        gctUINT32 matrixStride, arrayStride;
        __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
        __GLchipFeature *chipFeature = &chipCtx->chipFeature;
        gceUNIFORMCVT convert = gcChipQueryUniformConvert(program, uniform->dataType, gcvFALSE, chipFeature->haltiLevel > __GL_CHIP_HALTI_LEVEL_0);

        gcTYPE_GetTypeInfo(uniform->dataType, &columns, &rows, gcvNULL);

        /* Uniforms of default UBO were stored closely in column major */
        matrixStride = columns * 4;
        arrayStride = matrixStride * rows;

        for (stageIdx = 0; stageIdx < __GLSL_STAGE_LAST; ++stageIdx)
        {
            gcUNIFORM halUniform = uniform->halUniform[stageIdx];

            if (halUniform && isUniformUsedInShader(halUniform))
            {
                gctINT32 index;
                GL_ASSERT(GetUniformPhysical(halUniform) != -1);

                arraySize = GetUniformUsedArraySize(halUniform);
                if (isUniformArraysOfArrays(halUniform))
                {
                    arraySize = GetUniformSingleLevelArraySzie(halUniform, halUniform->arrayLengthCount - 1);
                }

                if (gc->shaderProgram.boundPPO ||chipCtx->chipDirty.uDefer.sDefer.pgInsChanged)
                {
                    gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(chipCtx->activeProgState->hints->hwConstRegBases,
                                                                      halUniform,
                                                                      &uniform->stateAddress[stageIdx]));
                }

                /* For es20 and es11 shaders, compiler always uses float in the machine code.
                ** So driver convert INT data into float before send to HW,
                ** but for XFB buffer address, it should not do the conversion.
                ** Compiler will refine it later.
                */
                if (uniform->subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_XFB_BUFFER)
                {
                    convert = gcvUNIFORMCVT_NONE;
                }

                index = GetUniformPhysical(halUniform) + (uniform->regOffset >> 4);

                gcmONERROR(gcoSHADER_BindUniform(gcvNULL, uniform->stateAddress[stageIdx] + uniform->regOffset, index,
                                                 columns, rows, arraySize, gcvFALSE, matrixStride,
                                                 arrayStride, data, convert, GetUniformShaderKind(halUniform)));

                if (gcmOPT_DUMP_UNIFORM())
                {
                    gcChipDumpGLUniform(uniform, uniform->dataType, 1, 0);
                }
           }
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE GLboolean
gcChipUtilParseUniformName(
    const GLchar *name,
    gctSIZE_T *nameLen,
    gctSIZE_T *arrayIdx,
    GLboolean *isArray
    )
{
    gctSIZE_T origLen = 0;
    GLboolean ret = GL_FALSE;

    gcmHEADER_ARG("name=%s nameLen=0x%x arrayIdx=0x%x isArray=0x%x",
                   name, nameLen, arrayIdx, isArray);

    GL_ASSERT(name);
    GL_ASSERT(nameLen);
    GL_ASSERT(arrayIdx);

    origLen = gcoOS_StrLen(name, gcvNULL);

    if ((origLen >= 4) && (name[origLen - 1] == ']'))
    {
        const GLchar* end   = name + origLen - 1;
        const GLchar* start = end - 1;

        while (*start != '[' && start > name)
        {
            --start;
        }

        if (start > name && start < end - 1)
        {
            GLuint index = 0;
            const GLchar* p = gcvNULL;
            for (p = start+1; p < end; p++)
            {
                if (!__GL_ISDIGIT(*p))
                {
                    goto OnError;
                }

                /* Names cannot have extra leading zeroes */
                if (*p == '0' && index == 0 && p != end - 1)
                {
                    goto OnError;
                }

                index = index*10 + ((*p)-'0');
            }

            *nameLen = (GLuint)(start - name);
            *arrayIdx = index;
            if (isArray)
            {
                *isArray = GL_TRUE;
            }
        }
    }
    else
    {
        *nameLen  = origLen;
        *arrayIdx = 0;
        if (isArray)
        {
            *isArray  = GL_FALSE;
        }
    }
    ret = GL_TRUE;

OnError:
    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}

/* Check if the Loadtime Constant Optimization is turned on for Program */
GLboolean
gcChipIsLTCEnabled(
    IN __GLchipSLProgram *Program
    )
{
    GLboolean ret;
    gcePATCH_ID patchId = gcvPATCH_INVALID;

    gcmHEADER_ARG("Program=0x%x", Program);

    gcoHAL_GetPatchID(gcvNULL, &patchId);

    /* general disable LTC */
    if (patchId == gcvPATCH_INVALID)
    {
        gcmFOOTER_ARG("return=0x%04x", gcvFALSE);
        return gcvFALSE;
    }

    /* Disable LTC optimization for real racing */
    if (patchId == gcvPATCH_REALRACING || patchId == gcvPATCH_NENAMARK
        || patchId == gcvPATCH_LEANBACKSCROLLING)
    {
        gcmFOOTER_ARG("return=0x%04x", gcvFALSE);
        return gcvFALSE;
    }

    /* by default turn on the LTC optimization */
    ret = (GLboolean)gcmOPT_EnableLTC();
    gcmFOOTER_ARG("return=0x%04x", ret);
    return ret;
}


/************************************************************************/
/* Implementation for EXPORTED FUNCTIONS                                */
/************************************************************************/
GLboolean
__glChipCompileShader(
    __GLcontext *gc,
    __GLshaderObject *shaderObject
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gcSHADER_KIND shaderType = gcSHADER_TYPE_UNKNOWN;
    gceSTATUS status;
    gcsGLSLCaps *glslCaps = &gc->constants.shaderCaps;

    gcmHEADER_ARG("gc=0x%x shaderObject=0x%x", gc, shaderObject);

    if (shaderObject->shaderInfo.sourceSize == 0)
    {
        gcoOS_StrDup(gcvNULL, "No source attached.", (gctSTRING*)&shaderObject->shaderInfo.compiledLog);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (chipCtx->pfCompile == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    switch (shaderObject->shaderInfo.shaderType)
    {
    case GL_VERTEX_SHADER:
        shaderType = gcSHADER_TYPE_VERTEX_DEFAULT_UBO;
        break;
    case GL_FRAGMENT_SHADER:
        shaderType = gcSHADER_TYPE_FRAGMENT_DEFAULT_UBO;
        break;
    case GL_COMPUTE_SHADER:
        shaderType = gcSHADER_TYPE_COMPUTE;
        break;
    case GL_TESS_CONTROL_SHADER_EXT:
        shaderType = gcSHADER_TYPE_TCS;
        break;
    case GL_TESS_EVALUATION_SHADER_EXT:
        shaderType = gcSHADER_TYPE_TES;
        break;
    case GL_GEOMETRY_SHADER_EXT:
        shaderType = gcSHADER_TYPE_GEOMETRY;
        break;
    }

    if (shaderObject->shaderInfo.hBinary)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(shaderObject->shaderInfo.hBinary));
        shaderObject->shaderInfo.hBinary = gcvNULL;
    }

    /* Enable it for CTS in nanoUltra3. */
    if ((chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_OESCTS || chipCtx->patchId == gcvPATCH_GTFES30) &&
        (chipCtx->chipModel == gcv880 && chipCtx->chipRevision == 0x5124))
    {
        gcOPT_SetFeature(FB_TREAT_CONST_ARRAY_AS_UNIFORM);
    }

    /* re-initialize CompilerCaps ,avoid muti-context ,the shaderCaps.extensions that compiler uses has been released when other context destory */
    (*chipCtx->pfInitCompilerCaps)(glslCaps);

    gcmONERROR((*chipCtx->pfCompile)(shaderType,
                                     shaderObject->shaderInfo.sourceSize,
                                     shaderObject->shaderInfo.source,
                                     (gcSHADER*)&shaderObject->shaderInfo.hBinary,
                                     (gctSTRING*)&shaderObject->shaderInfo.compiledLog));

    if ((chipCtx->patchId == gcvPATCH_DEQP || chipCtx->patchId == gcvPATCH_OESCTS || chipCtx->patchId == gcvPATCH_GTFES30) &&
        (chipCtx->chipModel == gcv880 && chipCtx->chipRevision == 0x5124))
    {
        gcOPT_ResetFeature(FB_TREAT_CONST_ARRAY_AS_UNIFORM);
    }

    gcmFOOTER_ARG("return=%d", gcvTRUE);
    return gcvTRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLvoid
__glChipDeleteShader(
    __GLcontext *gc,
    __GLshaderObject *shaderObject
    )
{
    gcmHEADER_ARG("gc=0x%x shaderObject=0x%x", gc, shaderObject);
    if (shaderObject->shaderInfo.hBinary)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(shaderObject->shaderInfo.hBinary));
        shaderObject->shaderInfo.hBinary = gcvNULL;
    }

    if (shaderObject->shaderInfo.compiledLog)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, shaderObject->shaderInfo.compiledLog));
        shaderObject->shaderInfo.compiledLog = gcvNULL;
    }
    gcmFOOTER_NO();
}

static GLvoid
gcChipPgInstanceInitialize(
    __GLcontext *gc,
    __GLchipSLProgramInstance* pgInstance,
    GLuint key
    )
{
    __GLSLStage stage;

    gcmHEADER_ARG("gc=0x%x pgInstance=0x%x", gc, pgInstance);

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        pgInstance->binaries[stage] = gcvNULL;
        pgInstance->savedBinaries[stage] = gcvNULL;
    }

    pgInstance->ownerCacheObj       = gcvNULL;
    pgInstance->instanceId          = key;
    pgInstance->programState.stateBufferSize = 0;
    pgInstance->programState.stateBuffer     = gcvNULL;
    pgInstance->programState.hints           = gcvNULL;

    pgInstance->privateUniformCount = 0;
    pgInstance->privateUniforms     = gcvNULL;
    pgInstance->privateUbCount      = 0;
    pgInstance->privateUBs          = gcvNULL;
    pgInstance->groupNumUniformIdx  = -1;
    pgInstance->advBlendState       = gcvNULL;
    pgInstance->extraImageUniformCount   = 0;

    pgInstance->recompilePatchInfo.recompilePatchDirectivePtr = gcvNULL;
    pgInstance->pgStateKeyMask.value = 0;

    gcChipPgStateKeyAlloc(gc, &pgInstance->pgStateKey);

    pgInstance->master = GL_FALSE;

    gcmFOOTER_NO();
}

static GLvoid
gcChipPgInstanceDeinitialize(
    __GLcontext *gc,
    GLvoid* pgInstanceToDel
    )
{
    __GLSLStage stage;
    __GLchipSLProgramInstance* pgInstance = (__GLchipSLProgramInstance*)pgInstanceToDel;

    gcmHEADER_ARG("gc=0x%x pgInstanceToDel=0x%x", gc, pgInstanceToDel);

    gcChipPgInstanceCleanBindingInfo(gc, pgInstance);

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (pgInstance->binaries[stage])
        {
            gcmVERIFY_OK(gcSHADER_Destroy(pgInstance->binaries[stage]));
            pgInstance->binaries[stage] = gcvNULL;
        }

        if (pgInstance->savedBinaries[stage])
        {
            gcmVERIFY_OK(gcSHADER_Destroy(pgInstance->savedBinaries[stage]));
            pgInstance->savedBinaries[stage] = gcvNULL;
        }
    }

    /* Free any states. */
    gcmVERIFY_OK(gcFreeProgramState(pgInstance->programState));

    if (pgInstance->recompilePatchInfo.recompilePatchDirectivePtr)
    {
        gcmVERIFY_OK(gcDestroyPatchDirective(&pgInstance->recompilePatchInfo.recompilePatchDirectivePtr));
        pgInstance->recompilePatchInfo.recompilePatchDirectivePtr = gcvNULL;
    }

    gcChipPgStateKeyFree(gc, &pgInstance->pgStateKey);

    (*gc->imports.free)(gc, pgInstance);
    gcmFOOTER_NO();
}

GLvoid
gcChipProgFreeCmdInstance(
    __GLcontext *gc,
    GLvoid *cmdInstance
    )
{
    gcmHEADER_ARG("gc=0x%x cmdInstance=0x%x", gc, cmdInstance);

    if (cmdInstance)
    {
        gcsPROGRAM_STATE *stateBuf = (gcsPROGRAM_STATE*)cmdInstance;

        gcFreeProgramState(*stateBuf);

        gc->imports.free(gc, stateBuf);
    }

    gcmFOOTER_NO();
}

GLboolean
__glChipCreateProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status;
    gctPOINTER pointer = gcvNULL;
    __GLchipSLProgram *program;
    __GLSLStage stage;
    GLuint i;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x", gc, programObject);

    program = (__GLchipSLProgram *)(*gc->imports.calloc)(gc, 1, sizeof(__GLchipSLProgram));
    programObject->privateData    = program;

    /* Initialize GLProgram structure. */
    program->isHalti              = gcvFALSE;
    program->codeSeq              = 0;
    program->valid                = GL_FALSE;
    program->inCount              = 0;
    program->inMaxNameLen         = 0;
    program->inputs               = gcvNULL;

    program->mayHasAliasedAttrib  = gcvFALSE;
    program->attribBinding        = gcvNULL;
    program->attribLinkage        = gcvNULL;
    program->attribLocation       = gcvNULL;

    program->uniformMaxNameLen    = 0;
    program->userDefUniformCount  = 0;
    program->builtInUniformCount  = 0;
    program->activeUniformCount   = 0;
    program->uniforms             = gcvNULL;

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        program->numSamplers[stage] = 0;
    }

    program->ubMaxNameLen         = 0;
    program->userDefUbCount       = 0;
    program->defaultUbCount       = 0;
    program->totalUbCount         = 0;
    program->uniformBlocks        = gcvNULL;

    program->maxLocation          = 0;
    program->loc2Uniform          = gcvNULL;

    gcoOS_ZeroMemory(program->samplerMap, gcmSIZEOF(program->samplerMap));
    for (i = 0; i < gc->constants.shaderCaps.maxTextureSamplers; ++i)
    {
        program->samplerMap[i].stage = __GLSL_STAGE_LAST;
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              gc->constants.shaderCaps.maxUserVertAttributes * gcmSIZEOF(__GLchipSLLinkage*),
                              &pointer));

    program->attribLinkage = (__GLchipSLLinkage**) pointer;

    for (i = 0; i < gc->constants.shaderCaps.maxUserVertAttributes; ++i)
    {
        program->attribLinkage[i] = gcvNULL;
    }

    gcmONERROR(gcoOS_Allocate(gcvNULL,
                              gc->constants.shaderCaps.maxVertAttributes * MAX_ALIASED_ATTRIB_COUNT * gcmSIZEOF(__GLchipSLLocation),
                              &pointer));

    program->attribLocation = pointer;

    for (i = 0; i < gc->constants.shaderCaps.maxVertAttributes * MAX_ALIASED_ATTRIB_COUNT; ++i)
    {
        program->attribLocation[i].pInput = gcvNULL;
    }

    program->pgInstaceCache = gcChipUtilsHashCreate(gc,
                                                    __GL_PROG_INSTANCE_HASH_ENTRY_NUM,
                                                    __GL_PROG_INSTANCE_HASH_ENTRY_SIZE,
                                                    gcChipPgInstanceDeinitialize);

    gcmASSERT(program->pgInstaceCache != gcvNULL);

    program->masterPgInstance = gcvNULL;
    program->curPgInstance = gcvNULL;
    program->stageBits = 0;

    __glBitmaskInitAllZero(&program->shadowSamplerMask, gc->constants.shaderCaps.maxTextureSamplers);
    __glBitmaskInitAllZero(&program->texelFetchSamplerMask, gc->constants.shaderCaps.maxTextureSamplers);

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLvoid
__glChipDeleteProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLBinding *binding;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x", gc, programObject);

    if (program)
    {
#if __GL_CHIP_PATCH_ENABLED
        gcChipPatchCleanUpProgram(gc, program);
#endif
        gcChipProgramCleanBindingInfo(gc, programObject);

        while (program->attribBinding)
        {
            binding = program->attribBinding;
            program->attribBinding = binding->next;
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, binding->name));
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, binding));
        }

        if (program->attribLinkage)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->attribLinkage));
        }

        if (program->attribLocation)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, program->attribLocation));
        }

        if (program->pgInstaceCache)
        {
            /* We are going to delete cache, so deref the current instance */
            if (program->curPgInstance)
            {
                gcChipUtilsObjectReleaseRef(program->curPgInstance->ownerCacheObj);
            }

            gcChipUtilsHashDestory(gc, program->pgInstaceCache);

            program->masterPgInstance = gcvNULL;
            program->curPgInstance = gcvNULL;
        }


        (*gc->imports.free)(gc, program);
        programObject->privateData = gcvNULL;
    }
    gcmFOOTER_NO();
}

__GLchipUtilsObject*
gcChipAddPgInstanceToCache(
    __GLcontext *gc,
    __GLchipSLProgram *program,
    GLuint key,
    GLboolean master
    )
{
    __GLchipSLProgramInstance* pgInstance = gcvNULL;
    __GLchipUtilsObject*       pgInstanceObj = gcvNULL;

    gcmHEADER_ARG("gc=0x%x program=0x%x key=%u master=%d", gc, program, key, master);

    /* For master program instance creation, delete all instances program currently holds */
    if (master)
    {
        if (program->masterPgInstance != gcvNULL)
        {
            /* We are going to delete content of cache, so deref the current instance */
            if (program->curPgInstance)
            {
                gcChipUtilsObjectReleaseRef(program->curPgInstance->ownerCacheObj);
            }

            gcChipUtilsHashDeleteAllObjects(gc, program->pgInstaceCache);

            program->masterPgInstance = gcvNULL;
            program->curPgInstance = gcvNULL;
        }
    }
    else
    {
        gcmASSERT(program->masterPgInstance != gcvNULL);
    }

    /* Create an instance, and add it into hash table. */
    pgInstance = (__GLchipSLProgramInstance*)(*gc->imports.calloc)(gc, 1, sizeof(__GLchipSLProgramInstance));
    gcmASSERT(pgInstance);

    gcChipPgInstanceInitialize(gc, pgInstance, key);
    pgInstanceObj = gcChipUtilsHashAddObject(gc, program->pgInstaceCache, (GLvoid*)pgInstance, key, master);
    gcmASSERT(pgInstanceObj);

    pgInstance->ownerCacheObj = pgInstanceObj;

    gcmFOOTER_ARG("return=0x%x", pgInstanceObj);
    return pgInstanceObj;
}

__GL_INLINE  gctUINT
gcChipDetermineDual16PrecisionRule(
    __GLchipContext     *chipCtx,
    __GLchipSLProgram   *program
    )
{
    gctUINT dual16PrecisionRule = Dual16_PrecisionRule_DEFAULT;

    switch (chipCtx->patchId)
    {
        case gcvPATCH_GLBM21:
        case gcvPATCH_GLBM25:
        case gcvPATCH_GLBM27:
        case gcvPATCH_GFXBENCH:
        case gcvPATCH_GFXBENCH4:
        case gcvPATCH_MM07:
        case gcvPATCH_NENAMARK2:
        case gcvPATCH_LEANBACK:
        case gcvPATCH_ANGRYBIRDS:
        case gcvPATCH_BM21:
            dual16PrecisionRule = Dual16_PrecisionRule_RCP_HP | Dual16_PrecisionRule_FRAC_HP | Dual16_PrecisionRule_IMMED_MP;
            break;
        default:
            break;
    }

   /* enable and disable should not be on at the same time */
    gcmASSERT(!(program->progFlags.disableHP_RCP && program->progFlags.enableHP_RCP));
    if (program->progFlags.disableHP_RCP)
    {
        dual16PrecisionRule &= ~Dual16_PrecisionRule_RCP_HP;
    }
    else if (program->progFlags.enableHP_RCP)
    {
        dual16PrecisionRule |= Dual16_PrecisionRule_RCP_HP;
    }

    gcmASSERT(!(program->progFlags.disableHP_FRAC && program->progFlags.enableHP_FRAC));
    if (program->progFlags.disableHP_FRAC)
    {
        dual16PrecisionRule &= ~Dual16_PrecisionRule_FRAC_HP;
    }
    else if (program->progFlags.enableHP_FRAC)
    {
        dual16PrecisionRule |= Dual16_PrecisionRule_FRAC_HP;
    }

    gcmASSERT(!(program->progFlags.disableHP_TEXLD_COORD && program->progFlags.enableHP_TEXLD_COORD));
    if (program->progFlags.disableHP_TEXLD_COORD)
    {
        dual16PrecisionRule &= ~Dual16_PrecisionRule_TEXLD_COORD_HP;
    }
    else if (program->progFlags.enableHP_TEXLD_COORD)
    {
        dual16PrecisionRule |= Dual16_PrecisionRule_TEXLD_COORD_HP;
    }

    gcmASSERT(!(program->progFlags.disableHP_IMMED && program->progFlags.enableHP_IMMED));
    if (program->progFlags.disableHP_IMMED)
    {
        dual16PrecisionRule &= ~Dual16_PrecisionRule_IMMED_HP;
    }
    else if (program->progFlags.enableHP_IMMED)
    {
        dual16PrecisionRule |= Dual16_PrecisionRule_IMMED_HP;
    }

    gcmASSERT(!(program->progFlags.disableMP_IMMED && program->progFlags.enableMP_IMMED));
    if (program->progFlags.disableMP_IMMED)
    {
        dual16PrecisionRule &= ~Dual16_PrecisionRule_IMMED_MP;
    }
    else if (program->progFlags.enableMP_IMMED)
    {
        dual16PrecisionRule |= Dual16_PrecisionRule_IMMED_MP;
    }

    return dual16PrecisionRule;
}

GLboolean
__glChipLinkProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLProgramInstance* masterPgInstance = gcvNULL;
    __GLchipUtilsObject* pgInstanceObj = gcvNULL;
    __GLchipProgramStateKey *pgStateKey;
    __GLSLStage stage, transformFeedbackStage = __GLSL_STAGE_VS;
    gctUINT key;
    gceSTATUS  status;
    gcsPROGRAM_STATE programState = {0};
    gceSHADER_FLAGS flags;
    gceSHADER_SUB_FLAGS subFlags;
    gcSHADER vsBinary = gcvNULL;
    gcSHADER_KIND shaderTypes[] =
    {
        gcSHADER_TYPE_VERTEX,
        gcSHADER_TYPE_TCS,
        gcSHADER_TYPE_TES,
        gcSHADER_TYPE_GEOMETRY,
        gcSHADER_TYPE_FRAGMENT,
        gcSHADER_TYPE_COMPUTE,
    };

#if __GL_CHIP_PATCH_ENABLED
    gctINT replaceIndices[__GLSL_STAGE_LAST];
    const gctCHAR *patchedSrcs[__GLSL_STAGE_LAST] = {gcvNULL};
#endif

    gcmHEADER_ARG("gc=0x%x programObject=0x%x", gc, programObject);

#if __GL_CHIP_PATCH_ENABLED
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++ stage)
    {
        replaceIndices[stage] = gcvMACHINECODE_COUNT;
    }
    /* Detect App by Shader Source: Shader Patches Here. */
    gcChipPatchCleanUpProgram(gc, program);
    gcChipPatchLink(gc, programObject, patchedSrcs, replaceIndices);

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        gcSHADER_KIND shaderTypes[] =
        {
            gcSHADER_TYPE_VERTEX_DEFAULT_UBO,
            gcSHADER_TYPE_TCS,
            gcSHADER_TYPE_TES,
            gcSHADER_TYPE_GEOMETRY,
            gcSHADER_TYPE_FRAGMENT_DEFAULT_UBO,
            gcSHADER_TYPE_COMPUTE,
        };

        if (program->progFlags.disableLoopUnrolling)
        {
            gcOPT_SetFeature(FB_DISABLE_GL_LOOP_UNROLLING);
        }

        if (patchedSrcs[stage])
        {
            __GLshaderObject *shaderObj = programObject->programInfo.attachedShader[stage];
            gcSHADER preBinary = (gcSHADER)shaderObj->shaderInfo.hBinary;
            gctSTRING preLog = shaderObj->shaderInfo.compiledLog;

            shaderObj->shaderInfo.hBinary = gcvNULL;
            shaderObj->shaderInfo.compiledLog = gcvNULL;

            /* Recompile the patched shaders. */
            status = (*chipCtx->pfCompile)(shaderTypes[stage],
                                           (gctUINT)gcoOS_StrLen(patchedSrcs[stage], gcvNULL),
                                           patchedSrcs[stage],
                                           (gcSHADER*)&shaderObj->shaderInfo.hBinary,
                                           (gctSTRING*)&shaderObj->shaderInfo.compiledLog);

            /* If patch compile failed, roll back.*/
            if (gcmIS_ERROR(status))
            {
                /* Reset pre value.*/
                /* If failed, hBinary already destory.*/
                shaderObj->shaderInfo.hBinary = preBinary;

                if (shaderObj->shaderInfo.compiledLog)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, shaderObj->shaderInfo.compiledLog));
                }
                shaderObj->shaderInfo.compiledLog = preLog;
            }
            else
            {
                /* destroy preBinary.*/
                if (preBinary)
                {
                    gcmVERIFY_OK(gcSHADER_Destroy(preBinary));
                    preBinary = gcvNULL;
                }

                if (preLog)
                {
                    gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, preLog));
                }
            }
        }

        if (program->progFlags.disableLoopUnrolling)
        {
            gcOPT_ResetFeature(FB_DISABLE_GL_LOOP_UNROLLING);
        }
    }
#endif

    program->codeSeq = programObject->programInfo.codeSeq;
#if gcdUSE_WCLIP_PATCH
    if (programObject->programInfo.attachedShader[__GLSL_STAGE_VS] != gcvNULL &&
        programObject->programInfo.attachedShader[__GLSL_STAGE_FS] != gcvNULL)
    {
        gcSHADER_CheckClipW(programObject->programInfo.attachedShader[__GLSL_STAGE_VS]->shaderInfo.source,
                            programObject->programInfo.attachedShader[__GLSL_STAGE_FS]->shaderInfo.source,
                            &chipCtx->clipW);
    }
#endif

    gcmONERROR(gcChipPgStateKeyAlloc(gc, &pgStateKey));

    key = gcChipUtilsEvaluateCRC32(pgStateKey->data, pgStateKey->size);

    /* Try to create master program instance and put it to cache */
    if ((pgInstanceObj = gcChipAddPgInstanceToCache(gc, program, key, GL_TRUE)) == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_REQUEST);
    }

    masterPgInstance = (__GLchipSLProgramInstance*)pgInstanceObj->pUserData;
    masterPgInstance->master = GL_TRUE;
    program->masterPgInstance = masterPgInstance;
    program->curPgInstance = masterPgInstance;

    /* Increase ref count of master instance. */
    gcChipUtilsObjectAddRef(pgInstanceObj);

    /* Test if any of the shaders is dirty. */
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        __GLshaderObject *shaderObj = programObject->programInfo.attachedShader[stage];

        if (shaderObj && shaderObj->shaderInfo.hBinary)
        {
            if (masterPgInstance->binaries[stage] == gcvNULL)
            {
                gcmONERROR(gcSHADER_Construct(shaderTypes[stage], &masterPgInstance->binaries[stage]));
            }
            gcmONERROR(gcSHADER_Copy(masterPgInstance->binaries[stage], (gcSHADER)shaderObj->shaderInfo.hBinary));
            if (masterPgInstance->savedBinaries[stage] == gcvNULL)
            {
                gcmONERROR(gcSHADER_Construct(shaderTypes[stage], &masterPgInstance->savedBinaries[stage]));
            }
            gcmONERROR(gcSHADER_Copy(masterPgInstance->savedBinaries[stage], (gcSHADER)shaderObj->shaderInfo.hBinary));
        }
        else
        {
            if (masterPgInstance->binaries[stage])
            {
                gcmONERROR(gcSHADER_Destroy(masterPgInstance->binaries[stage]));
                masterPgInstance->binaries[stage] = gcvNULL;
            }
            if (masterPgInstance->savedBinaries[stage])
            {
                gcmONERROR(gcSHADER_Destroy(masterPgInstance->savedBinaries[stage]));
                masterPgInstance->savedBinaries[stage] = gcvNULL;
            }
        }
    }

    if (programObject->xfbVaryingNum)
    {
        gceFEEDBACK_BUFFER_MODE xfbMode = (programObject->xfbMode == GL_SEPARATE_ATTRIBS)
                                        ? gcvFEEDBACK_SEPARATE
                                        : gcvFEEDBACK_INTERLEAVED;

        transformFeedbackStage = gcChipGetLastNonFragStage(program);

        /* If this program object has no vertex, tessellation evaluation, or geometry shader, it is a link error. */
        if (transformFeedbackStage != __GLSL_STAGE_VS &&
            transformFeedbackStage != __GLSL_STAGE_TES &&
            transformFeedbackStage != __GLSL_STAGE_GS)
        {
            status = gcvSTATUS_LINK_INVALID_SHADERS;
            gcmONERROR(status);
        }

        gcmONERROR(gcSHADER_SetTransformFeedbackVarying(masterPgInstance->binaries[transformFeedbackStage],
                                                        (gctSIZE_T)programObject->xfbVaryingNum,
                                                        (gctCONST_STRING*)programObject->ppXfbVaryingNames,
                                                        xfbMode));

        gcmONERROR(gcSHADER_SetTransformFeedbackVarying(masterPgInstance->savedBinaries[transformFeedbackStage],
                                                        (gctSIZE_T)programObject->xfbVaryingNum,
                                                        (gctCONST_STRING*)programObject->ppXfbVaryingNames,
                                                        xfbMode));
    }

    flags = (gcvSHADER_DEAD_CODE              |
             gcvSHADER_RESOURCE_USAGE         |
             gcvSHADER_OPTIMIZER              |
             gcvSHADER_USE_GL_Z               |
             gcvSHADER_USE_GL_POINT_COORD     |
             gcvSHADER_USE_GL_POSITION        |
             gcvSHADER_REMOVE_UNUSED_UNIFORMS |
             gcvSHADER_FLUSH_DENORM_TO_ZERO);

    if (
#if __GL_CHIP_PATCH_ENABLED
        (!program->progFlags.noLTC) &&
#endif
        gcChipIsLTCEnabled(program)
       )
    {
        flags |= gcvSHADER_LOADTIME_OPTIMIZER;
    }

#if gcdALPHA_KILL_IN_SHADER && __GL_CHIP_PATCH_ENABLED
    if (program->progFlags.alphaKill)
    {
        flags |= gcvSHADER_USE_ALPHA_KILL;
    }
#endif

    if (programObject->programInfo.separable)
    {
        flags |= gcvSHADER_SEPERATED_PROGRAM;
    }

    if (program->progFlags.disableDual16)
    {
        flags |= gcvSHADER_DISABLE_DUAL16;
    }

    if (program->progFlags.VIRCGNone)
    {
        flags |= gcvSHADER_VIRCG_NONE;
    }

    if (program->progFlags.VIRCGOne)
    {
        flags |= gcvSHADER_VIRCG_ONE;
    }

    if (program->progFlags.disableInline)
    {
        flags |= gcvSHADER_SET_INLINE_LEVEL_0;
    }

    if (program->progFlags.deqpMinCompTime)
    {
        flags |= gcvSHADER_MIN_COMP_TIME;
    }

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (masterPgInstance->binaries[stage])
        {
#if __GL_CHIP_PATCH_ENABLED && gcdSHADER_SRC_BY_MACHINECODE
            if (replaceIndices[stage] != gcvMACHINECODE_COUNT)
            {
                masterPgInstance->binaries[stage]->replaceIndex = replaceIndices[stage];
            }
#endif
        }
    }

    /* determine the dual16 hp rule based on benchmark and shader detection */
    subFlags.dual16PrecisionRule = gcChipDetermineDual16PrecisionRule(chipCtx, program);

    if (chipCtx->robust)
    {
        flags |= gcvSHADER_NEED_ROBUSTNESS_CHECK;
        program->progFlags.robustEnabled = gcvTRUE;
    }

    /* Call the HAL backend linker. */
    gcSetGLSLCompiler(chipCtx->pfCompile);

    /* set attribute location */
    vsBinary = masterPgInstance->binaries[__GLSL_STAGE_VS];

    if (vsBinary && gcSHADER_IsES11Compiler(vsBinary) && program->mayHasAliasedAttrib)
    {
        __GLchipSLBinding *binding;
        for (binding = program->attribBinding; binding != gcvNULL;  binding = binding->next)
        {
            gcmONERROR(gcSHADER_SetAttrLocationByDriver(vsBinary, binding->name, binding->index));
        }
    }

    status = gcLinkProgram(__GLSL_STAGE_LAST,
                             masterPgInstance->binaries,
                             flags,
                             &subFlags,
                             &programState);

    if (gcmIS_ERROR(status))
    {
        gcmONERROR(gcChipMapLinkError(gc, programObject, status));
    }

    programObject->programInfo.invalidFlags &= ~__GL_INVALID_LINK_BIT;

    /* Successful link must guarantee there is state buffer generated. */
    GL_ASSERT(programState.stateBuffer && programState.stateBufferSize > 0);

    /* Only successfully link can replace the previous shader states buffer */
    if (masterPgInstance->programState.stateBuffer != gcvNULL)
    {
        gcmVERIFY_OK(gcFreeProgramState(masterPgInstance->programState));
    }
    masterPgInstance->programState = programState;

    program->stageBits = masterPgInstance->programState.hints->stageBits;

    gcChipProgramCleanBindingInfo(gc, programObject);

    gcmONERROR(gcChipProgramBuildBindingInfo(gc, programObject));

    gcChipPgStateKeyFree(gc, &pgStateKey);

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    gcSHADER_AlignId();
    return GL_TRUE;

OnError:
    if (pgStateKey)
    {
        gcChipPgStateKeyFree(gc, &pgStateKey);
    }

    gcChipProgramCleanBindingInfo(gc, programObject);
    programObject->programInfo.invalidFlags |= __GL_INVALID_LINK_BIT;
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
gcChipCheckTextureConflict(
    __GLcontext *gc,
    __GLprogramObject *programObject
    )
{
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    GLuint enableDims[__GL_MAX_TEXTURE_UNITS];
    GLuint enableDim = 0;
    GLuint sampler;
    GLuint unit;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x", gc, programObject);

    if (__GL_API_VERSION_ES20 == gc->apiVersion)
    {
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

     /* mark enable dimensions of all texture unit to be disabled */
    for (unit = 0; unit < programObject->maxUnit; ++unit)
    {
        enableDims[unit] = __GL_MAX_TEXTURE_BINDINGS;
    }

    for (sampler = 0; sampler < programObject->maxSampler; ++sampler)
    {
        /* Skip the non-used sampler */
        if (program->samplerMap[sampler].stage == __GLSL_STAGE_LAST)
        {
            continue;
        }

        unit = program->samplerMap[sampler].unit;
        enableDim = program->samplerMap[sampler].texDim;
        GL_ASSERT(unit < gc->constants.shaderCaps.maxCombinedTextureImageUnits);
        if (enableDims[unit] == __GL_MAX_TEXTURE_BINDINGS)
        {
            enableDims[unit] = enableDim;
        }
        else if (enableDims[unit] != enableDim)
        {
            gcmFOOTER_ARG("return=%d", GL_TRUE);
            return GL_TRUE;
        }
    }

    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipUseProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLboolean *valid
    )
{
    gcmHEADER_ARG("gc=0x%x programObject=0x%x valid=0x%x", gc, programObject, valid);

    if (programObject)
    {
        gcoHAL_FrameInfoOps(gcvNULL, gcvFRAMEINFO_PROGRAM_ID, gcvFRAMEINFO_OP_SET, &programObject->objectInfo.id);
    }

#if VIVANTE_PROFILER
    if (gc->profiler.enable &&
        programObject &&
        programObject->programInfo.attachedShader[__GLSL_STAGE_VS] &&
        programObject->programInfo.attachedShader[__GLSL_STAGE_FS])
    {
        gcSHADER vertexShader   = (gcSHADER)programObject->programInfo.attachedShader[__GLSL_STAGE_VS]->shaderInfo.hBinary;
        gcSHADER fragmentShader = (gcSHADER)programObject->programInfo.attachedShader[__GLSL_STAGE_FS]->shaderInfo.hBinary;
        __glChipProfilerSet(gc, GL3_PROGRAM_IN_USE_BEGIN, programObject);
        __glChipProfilerSet(gc, GL3_PROGRAM_VERTEX_SHADER, vertexShader);
        __glChipProfilerSet(gc, GL3_PROGRAM_FRAGMENT_SHADER, fragmentShader);
        __glChipProfilerSet(gc, GL3_PROGRAM_IN_USE_END, (gctHANDLE)(gctUINTPTR_T)1);
    }
#endif
    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}

GLboolean
__glChipValidateProgram(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLboolean callFromDraw
    )
{
    /*
    ** 2 places need to validate the program: drawAPIS and glValidateProgram.
    ** These dirty bits used here will only be cleared at drawAPIs time, no other places
    ** can clear it, otherwise doesn't work.
    */
    GLboolean ret;
    gcmHEADER_ARG("gc=0x%x programObject=0x%x callFromDraw=%d", gc, programObject, callFromDraw);

    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] &
        (__GL_DIRTY_GLSL_PROGRAM_SWITCH | __GL_DIRTY_GLSL_SAMPLER))
    {
        if (gcChipCheckTextureConflict(gc, programObject))
        {
            programObject->programInfo.invalidFlags |= __GL_INVALID_TEX_BIT;
        }
        else
        {
            programObject->programInfo.invalidFlags &= ~__GL_INVALID_TEX_BIT;
        }
    }

    /* Can add more types of validate check here if there is any */
    ret = programObject->programInfo.invalidFlags ? GL_FALSE : GL_TRUE;

    gcmFOOTER_ARG("return=%d", ret);
    return ret;
}


static gceSTATUS
gcChipGetProgramBinary_V0(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei bufSize,
    GLsizei *length,
    GLenum *binaryFormat,
    GLvoid *binary
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    GLsizei   size   = 0;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLProgramInstance* masterPgInstance = program->masterPgInstance;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x bufSize=%d length=0x%x binaryFormat=0x%x binary=0x%x",
                   gc, programObject, bufSize, length, binaryFormat, binary);

    GL_ASSERT(masterPgInstance);

    /* Get size of program binary. */
    if(masterPgInstance->savedBinaries[__GLSL_STAGE_CS])
    {
        gcmONERROR(gcSaveComputeProgram(masterPgInstance->savedBinaries[__GLSL_STAGE_CS],
                                        masterPgInstance->programState, gcvNULL, (gctUINT32 *)&size));
    }
    else
    {
        gcmONERROR(gcSaveGraphicsProgram(masterPgInstance->savedBinaries,
                                     masterPgInstance->programState, gcvNULL, (gctUINT32 *)&size));
    }

    if (binary)
    {
        if (size > bufSize)
        {
            gcmONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        /* Save program binary. */
        if(masterPgInstance->savedBinaries[__GLSL_STAGE_CS])
        {
            gcmONERROR(gcSaveComputeProgram(masterPgInstance->savedBinaries[__GLSL_STAGE_CS],
                                            masterPgInstance->programState, gcvNULL, (gctUINT32 *)&size));
        }
        else
        {
            /* Save program binary. */
            gcmONERROR(gcSaveGraphicsProgram(masterPgInstance->savedBinaries,
                                             masterPgInstance->programState, &binary, (gctUINT32 *)&size));
        }

    }

    if (length != gcvNULL)
    {
        *length = size;
    }

    if (binaryFormat != gcvNULL)
    {
        *binaryFormat = GL_PROGRAM_BINARY_VIV;
    }

OnError:

    gcmFOOTER();
    return status;
}

static gceSTATUS
gcChipProgramBinary_V0(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLvoid *binary,
    GLsizei length
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLProgramInstance* masterPgInstance = gcvNULL;
    __GLchipUtilsObject* pgInstanceObj = gcvNULL;
    __GLchipProgramStateKey *pgStateKey = gcvNULL;
    gceSHADER_FLAGS flags;
    gceSHADER_SUB_FLAGS subFlags;
    __GLSLStage stage;
    gctUINT key;
    gctUINT pipelineStageMask;
    gcSHADER_KIND shaderTypes[] =
    {
        gcSHADER_TYPE_VERTEX,
        gcSHADER_TYPE_TCS,
        gcSHADER_TYPE_TES,
        gcSHADER_TYPE_GEOMETRY,
        gcSHADER_TYPE_FRAGMENT,
        gcSHADER_TYPE_COMPUTE,
    };
    gctUINT32 i;
    gctUINT32 j;
    gctUINT32 nameCount = 0;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x binary=0x%x length=%d", gc, programObject, binary, length);

    if (program == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_OBJECT);
    }

    gcChipPgStateKeyAlloc(gc, &pgStateKey);

    key = gcChipUtilsEvaluateCRC32(pgStateKey->data, pgStateKey->size);

    /* Try to create master program instance and put it to cache */
    if ((pgInstanceObj = gcChipAddPgInstanceToCache(gc, program, key, GL_TRUE)) == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_NOT_FOUND);
    }

    masterPgInstance = (__GLchipSLProgramInstance*)pgInstanceObj->pUserData;
    masterPgInstance->master = GL_TRUE;
    program->masterPgInstance = masterPgInstance;
    program->curPgInstance = masterPgInstance;

    /* Increase ref count of master instance. Note that for master instance, we don't need
       to increase historic ref count since it has been set to be perpetual */
    gcChipUtilsObjectAddRef(pgInstanceObj);

    pipelineStageMask = *(gctUINT32 *)((gctUINT8 *)binary + _gcdProgramBinaryHeaderSize);
    for (stage = __GLSL_STAGE_VS; stage <= __GLSL_STAGE_CS; ++stage)
    {
        /* First we need to destroy all previous shader. */
        if (masterPgInstance->binaries[stage])
        {
            gcmVERIFY_OK(gcSHADER_Destroy(masterPgInstance->binaries[stage]));
            masterPgInstance->binaries[stage] = gcvNULL;
        }

        if (masterPgInstance->savedBinaries[stage])
        {
            gcmVERIFY_OK(gcSHADER_Destroy(masterPgInstance->savedBinaries[stage]));
            masterPgInstance->savedBinaries[stage] = gcvNULL;
        }

        /* Skip the inactive stage. */
        if (!(pipelineStageMask & (1 << stage)))
        {
            continue;
        }

        gcmONERROR(gcSHADER_Construct(shaderTypes[stage], &masterPgInstance->binaries[stage]));
        gcmONERROR(gcSHADER_Construct(shaderTypes[stage], &masterPgInstance->savedBinaries[stage]));
    }

    if(masterPgInstance->binaries[__GLSL_STAGE_CS])
    {
        gcmONERROR(gcLoadComputeProgram((gctPOINTER)binary,
                                        (gctUINT32)length,
                                         masterPgInstance->binaries[__GLSL_STAGE_CS],
                                         gcvNULL));
    }
    else
    {
        gcmONERROR(gcLoadGraphicsProgram((gctPOINTER)binary,
                                         (gctUINT32)length,
                                         masterPgInstance->binaries,
                                         gcvNULL));
    }

    /* Loop up shaders until the last one non-fragment,and restore program XFB state if need. */
    for (i = 0; i < gcvPROGRAM_STAGE_FRAGMENT; i++)
    {
        if (masterPgInstance->binaries[i] != gcvNULL)
        {
            nameCount += GetFeedbackVaryingCount(&(masterPgInstance->binaries[i]->transformFeedback));
        }
    }

    /* If some XFB state need to restore. */
    if (nameCount > 0)
    {
        programObject->ppXfbVaryingNames = (GLchar**)(*gc->imports.malloc)(gc, nameCount*sizeof(GLchar*));
        programObject->xfbVaryingNum = nameCount;

        nameCount = 0;

        for (i = 0; i < gcvPROGRAM_STAGE_FRAGMENT; i++)
        {
            if (masterPgInstance->binaries[i] != gcvNULL)
            {
                /* Restore XFB state of program,according to binaries sharder info. */
                if(GetFeedbackVaryingCount(&(masterPgInstance->binaries[i]->transformFeedback)))
                    programObject->xfbMode = (GetFeedbackBufferMode(&(masterPgInstance->binaries[i]->transformFeedback)) == gcvFEEDBACK_SEPARATE)
                                                                                                                            ? GL_SEPARATE_ATTRIBS
                                                                                                                            : GL_INTERLEAVED_ATTRIBS;
                for (j = 0; j < GetFeedbackVaryingCount(&(masterPgInstance->binaries[i]->transformFeedback)); j++)
                {
                    gctUINT32 nameLen = (gctUINT32)strlen(&GetFeedbackVaryings(&(masterPgInstance->binaries[i]->transformFeedback))->name[j]) + 1;
                    programObject->ppXfbVaryingNames[nameCount] = (GLchar*)(*gc->imports.malloc)(gc, nameLen);
                    strcpy(programObject->ppXfbVaryingNames[nameCount],&GetFeedbackVaryings(&(masterPgInstance->binaries[i]->transformFeedback))->name[j]);
                    nameCount += 1;
                }
            }
        }
    }

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        if (masterPgInstance->savedBinaries[stage] && masterPgInstance->binaries[stage])
        {
            gcmONERROR(gcSHADER_Copy(masterPgInstance->savedBinaries[stage], masterPgInstance->binaries[stage]));
        }
    }

#if __GL_CHIP_PATCH_ENABLED && gcdUSE_WCLIP_PATCH
    gcChipPatchBinary(gc, masterPgInstance->binaries[__GLSL_STAGE_VS]);
#endif

    /* Set link flags for current program. */
    flags = (gcvSHADER_DEAD_CODE            |
             gcvSHADER_RESOURCE_USAGE       |
             gcvSHADER_OPTIMIZER            |
             gcvSHADER_USE_GL_Z             |
             gcvSHADER_USE_GL_POINT_COORD   |
             gcvSHADER_USE_GL_POSITION      |
             gcvSHADER_REMOVE_UNUSED_UNIFORMS |
             gcvSHADER_FLUSH_DENORM_TO_ZERO);

    if (
#if __GL_CHIP_PATCH_ENABLED
        (!program->progFlags.noLTC) &&
#endif
        gcChipIsLTCEnabled(program)
       )
    {
        flags |= gcvSHADER_LOADTIME_OPTIMIZER;
    }

#if gcdALPHA_KILL_IN_SHADER && __GL_CHIP_PATCH_ENABLED
    if (program->progFlags.alphaKill)
    {
        flags |= gcvSHADER_USE_ALPHA_KILL;
    }
#endif

    if (program->progFlags.disableDual16)
    {
        flags |= gcvSHADER_DISABLE_DUAL16;
    }

    if (program->progFlags.VIRCGNone)
    {
        flags |= gcvSHADER_VIRCG_NONE;
    }

    if (program->progFlags.VIRCGOne)
    {
        flags |= gcvSHADER_VIRCG_ONE;
    }

    if (program->progFlags.disableInline)
    {
        flags |= gcvSHADER_SET_INLINE_LEVEL_0;
    }

    if (program->progFlags.deqpMinCompTime)
    {
        flags |= gcvSHADER_MIN_COMP_TIME;
    }

    /* determine the dual16 hp rule based on benchmark and shader detection */
    subFlags.dual16PrecisionRule = gcChipDetermineDual16PrecisionRule(chipCtx, program);

    if (chipCtx->robust)
    {
        flags |= gcvSHADER_NEED_ROBUSTNESS_CHECK;
        program->progFlags.robustEnabled = gcvTRUE;
    }

    gcSetGLSLCompiler(chipCtx->pfCompile);
    status = gcLinkProgram(__GLSL_STAGE_LAST,
                           masterPgInstance->binaries,
                           flags,
                           &subFlags,
                           &masterPgInstance->programState);
    if (gcmIS_ERROR(status))
    {
        programObject->programInfo.invalidFlags |= __GL_INVALID_LINK_BIT;
        gcmONERROR(status);
    }
    else
    {
        __GLSLStage stage;

        programObject->bindingInfo.isSeparable = GL_FALSE;
        programObject->bindingInfo.isRetrievable = GL_FALSE;

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            /* Mark related shaderID to -1, so __glGetCurrentStageProgram can get work. */
            programObject->bindingInfo.activeShaderID[stage] = masterPgInstance->binaries[stage] ? (GLuint)-1 : 0;
        }

        program->stageBits = masterPgInstance->programState.hints->stageBits;
        programObject->programInfo.invalidFlags &= ~__GL_INVALID_LINK_BIT;
    }

    gcChipProgramCleanBindingInfo(gc, programObject);
    gcmONERROR(gcChipProgramBuildBindingInfo(gc, programObject));

    gcChipPgStateKeyFree(gc, &pgStateKey);

    gcmFOOTER();
    return gcvSTATUS_OK;


OnError:
    if (pgStateKey)
    {
        gcChipPgStateKeyFree(gc, &pgStateKey);
    }
    gcChipProgramCleanBindingInfo(gc, programObject);
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipProgramBinarySaveAttribBindingInfo(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLvoid *buf,
    GLuint bufSize,
    GLuint *length,
    GLuint *bindingCount
    )
{
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    GLuint attribBindingCount = 0;
    GLuint size = 0;
    __GLchipSLBinding *attribBinding = program->attribBinding;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x buf=0x%x bufSize=%d length=0x%x bindingCount=0x%x",
                   gc, programObject, buf, bufSize, length, bindingCount);

    while(attribBinding)
    {
        gctUINT strLen = 0;
        attribBindingCount++;
        strLen = (gctUINT) gcoOS_StrLen(attribBinding->name, gcvNULL);

        /* size for string length */
        size += gcmSIZEOF(GLuint);
        /* size for binding string, zero terminated */
        size += gcmALIGN(strLen + 1, 2);
        /* size for binding index */
        size += gcmSIZEOF(GLuint);

        attribBinding = attribBinding->next;
    }

    if (buf)
    {
        GLubyte *bufPtr = (GLubyte*)buf;
        if (size > bufSize)
        {
            gcmONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        attribBinding = program->attribBinding;

        while(attribBinding)
        {
            gctSIZE_T strLen = gcoOS_StrLen(attribBinding->name, gcvNULL);

            *(GLuint *)bufPtr = (GLuint)strLen;
            bufPtr += gcmSIZEOF(GLuint);
            gcoOS_MemCopy(bufPtr, attribBinding->name, gcmALIGN(strLen+1,2));
            bufPtr += gcmALIGN(strLen + 1, 2);
            *(GLuint *)bufPtr = attribBinding->index;
            bufPtr += gcmSIZEOF(GLuint);

            attribBinding = attribBinding->next;
        }
    }

    if (length)
    {
        *length = gcmALIGN(size, 4);
    }

    if (bindingCount)
    {
        *bindingCount = attribBindingCount;
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipProgramBinaryLoadAttribBindingInfo(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint bindingCount,
    GLvoid *buf,
    GLuint *attribBindingInfoSize
    )
{
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    GLubyte *bufPtr = (GLubyte*)buf;
    GLuint bindingInfoSize = 0;
    GLuint strLen;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x bindingCount=%d buf=0x%x attribBindingInfoSize=0x%x",
                   gc, programObject, bindingCount, buf, attribBindingInfoSize);

    while(bindingCount)
    {
        gctCONST_STRING name;
        __GLchipSLBinding *binding;
        gceSTATUS status;
        gctPOINTER pointer = gcvNULL;

        gcmERR_BREAK(gcoOS_Allocate(gcvNULL,
                                    gcmSIZEOF(__GLchipSLBinding),
                                    &pointer));

        binding = pointer;

        /* 1. retrieve string length */
        strLen = *(GLuint*)bufPtr;
        bufPtr += gcmSIZEOF(GLuint);

        /* 2. retrieve string */
        name = (gctCONST_STRING)bufPtr;

        gcmVERIFY_OK(gcoOS_StrDup(gcvNULL, name, &binding->name));

        bufPtr += gcmALIGN(strLen + 1, 2);
        /* 3. retrieve index value */
        binding->index         = *(GLuint*)bufPtr;
        binding->next          = program->attribBinding;
        program->attribBinding = binding;

        bufPtr += gcmSIZEOF(GLuint);

        bindingInfoSize += ((gcmSIZEOF(GLuint))        +   /* strlen */
                            gcmALIGN(strLen + 1, 2)    +   /* string */
                            (gcmSIZEOF(GLuint)));          /* index value */

        bindingCount--;
    }

    if (attribBindingInfoSize)
    {
        *attribBindingInfoSize = gcmALIGN(bindingInfoSize, 4);
    }

    gcmFOOTER();
    return status;
}

GLboolean
__glChipGetProgramBinary_V1(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei bufSize,
    GLsizei *length,
    GLenum *binaryFormat,
    GLvoid *binary
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    GLsizei   v0size = 0;
    GLsizei   totalSize = 0;
    GLuint    attribBindingInfoSize = 0;
    GLuint    attribBindingCount = 0;
    GLuint    v1HeaderSize = gcmSIZEOF(__GLchipProgramBinaryV1Header);
    GLuint    remainBufSize = bufSize;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x bufSize=%d length=0x%x binaryFormat=0x%x binary=0x%x",
                   gc, programObject, bufSize, length, binaryFormat, binary);

    /* Get size of HAL program binary(v0) size */
    gcmONERROR(gcChipGetProgramBinary_V0(gc, programObject, 0, &v0size, binaryFormat, gcvNULL));

    /* Get size of attrib binding info for version 1*/
    gcmONERROR(gcChipProgramBinarySaveAttribBindingInfo(gc, programObject, gcvNULL, 0,
                                               &attribBindingInfoSize, &attribBindingCount));

    totalSize = v1HeaderSize            +  /* v1 header */
                gcmSIZEOF(GLuint)       +  /* attribBind Count */
                attribBindingInfoSize   +  /* attribBindingInfo size */
                v0size;                    /* HAL program(v0) binary size */

    if (binary)
    {
        __GLchipProgramBinaryV1Header  *v1Header;
        GLubyte *bufPtr;
        GL_ASSERT((binaryFormat != gcvNULL) && (*binaryFormat == GL_PROGRAM_BINARY_VIV));

        if (totalSize > bufSize)
        {
            gcmONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }
        bufPtr = (GLubyte *)binary;

        v1Header = (__GLchipProgramBinaryV1Header*)bufPtr;
        /* 1. save v1 header */
        v1Header->signature1 = CHIP_PROG_V1_SIG1;
        v1Header->signature2 = CHIP_PROG_V1_SIG2;
        v1Header->remainSize = totalSize - v1HeaderSize;

        bufPtr += v1HeaderSize;
        remainBufSize -= v1HeaderSize;
        /* 2. save attrib binding count */
        *(GLuint*)bufPtr = attribBindingCount;
        bufPtr += gcmSIZEOF(GLuint);
        remainBufSize -= gcmSIZEOF(GLuint);
        /* 3. save attrib binding information */
        gcmONERROR(gcChipProgramBinarySaveAttribBindingInfo(gc,
                                                              programObject,
                                                              bufPtr,
                                                              remainBufSize,
                                                              gcvNULL,
                                                              gcvNULL));

        bufPtr += attribBindingInfoSize;
        remainBufSize -= attribBindingInfoSize;
        /* 4. save v0 program binary */
        gcmONERROR(gcChipGetProgramBinary_V0(gc, programObject, remainBufSize, gcvNULL, gcvNULL, bufPtr));

    }

    if (length != gcvNULL)
    {
        *length = totalSize;
    }

    if (binaryFormat != gcvNULL)
    {
        *binaryFormat = GL_PROGRAM_BINARY_VIV;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}


GLboolean
__glChipProgramBinary_V1(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLvoid *binary,
    GLsizei length
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipProgramBinaryV1Header *v1Header;
    GLubyte *bufPtr = (GLubyte *)binary;
    GLuint    attribBindingInfoSize = 0;
    GLuint    attribBindingCount = 0;
    GLuint    v1HeaderSize = gcmSIZEOF(__GLchipProgramBinaryV1Header);
    GLuint    remainBufSize = length;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x binary=0x%x length=%d", gc, programObject, binary, length);

    if (program == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* 1. retrieve v1 header */
    v1Header = (__GLchipProgramBinaryV1Header *)bufPtr;
    bufPtr += v1HeaderSize;
    remainBufSize -= v1HeaderSize;

    /* try v0 version for forward compatible*/
    if ((v1Header->signature1 != CHIP_PROG_V1_SIG1) ||
        (v1Header->signature2 != CHIP_PROG_V1_SIG2) ||
        (remainBufSize < v1Header->remainSize))
    {
        gcmONERROR(gcChipProgramBinary_V0(gc, programObject, binary, length));
        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    }

    /* 2. retrieve attrib binding count */
    attribBindingCount = *(GLuint*)bufPtr;
    bufPtr += gcmSIZEOF(GLuint);
    remainBufSize -= gcmSIZEOF(GLuint);

    /* 3. retrieve attrib binding information */
    gcmONERROR(gcChipProgramBinaryLoadAttribBindingInfo(gc, programObject, attribBindingCount, bufPtr, &attribBindingInfoSize));
    bufPtr += attribBindingInfoSize;
    remainBufSize -= attribBindingInfoSize;

    /* 4, retrieve v0 program binary */
    gcmONERROR(gcChipProgramBinary_V0(gc, programObject, bufPtr, remainBufSize));

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);

    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLboolean
__glChipShaderBinary(
    __GLcontext *gc,
    GLsizei n,
    __GLshaderObject **shaderObjects,
    GLenum binaryformat,
    const GLvoid *binary,
    GLsizei length
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    GLsizei     i;
    gcSHADER    shader         = gcvNULL;
    gcSHADER    shaders[__GLSL_STAGE_FS + 1] = { gcvNULL, gcvNULL, gcvNULL, gcvNULL, gcvNULL };
    gcSHADER    vertexShader   = gcvNULL;
    gcSHADER    fragmentShader = gcvNULL;
    gcSHADER    computeShader = gcvNULL;
    gcSHADER    tcsShader   = gcvNULL;
    gcSHADER    tesShader = gcvNULL;
    gcSHADER    gsShader = gcvNULL;
    gceSTATUS   status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x shaderObjects=0x%x binaryformat=0x%04x binary=0x%x length=%d",
                   gc, shaderObjects, binaryformat, binary, length);

    /* Iterate all shader objects. */
    for (i = 0; i < n; i++)
    {
        gcSHADER found = gcvNULL;

        switch (shaderObjects[i]->shaderInfo.shaderType)
        {
        case GL_VERTEX_SHADER:
            if (vertexShader != gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
            if (shaderObjects[i]->shaderInfo.hBinary == gcvNULL)
            {
                gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_VERTEX, (gcSHADER*)&shaderObjects[i]->shaderInfo.hBinary));
            }
            vertexShader = (gcSHADER)shaderObjects[i]->shaderInfo.hBinary;
            shaderObjects[i]->shaderInfo.compiledStatus = gcvTRUE;
            break;

        case GL_FRAGMENT_SHADER:
            if (fragmentShader != gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
            if (shaderObjects[i]->shaderInfo.hBinary == gcvNULL)
            {
                gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_FRAGMENT, (gcSHADER*)&shaderObjects[i]->shaderInfo.hBinary));
            }
            fragmentShader = (gcSHADER)shaderObjects[i]->shaderInfo.hBinary;
            shaderObjects[i]->shaderInfo.compiledStatus = gcvTRUE;
            break;

        case GL_COMPUTE_SHADER:
            if (computeShader != gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
            if (shaderObjects[i]->shaderInfo.hBinary == gcvNULL)
            {
                gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_COMPUTE, (gcSHADER*)&shaderObjects[i]->shaderInfo.hBinary));
            }
            computeShader = (gcSHADER)shaderObjects[i]->shaderInfo.hBinary;
            shaderObjects[i]->shaderInfo.compiledStatus = gcvTRUE;
            break;

        case GL_TESS_CONTROL_SHADER_EXT:
            if (tcsShader != gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
            if (shaderObjects[i]->shaderInfo.hBinary == gcvNULL)
            {
                gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_TCS, (gcSHADER*)&shaderObjects[i]->shaderInfo.hBinary));
            }
            tcsShader = (gcSHADER)shaderObjects[i]->shaderInfo.hBinary;
            shaderObjects[i]->shaderInfo.compiledStatus = gcvTRUE;
            break;

        case GL_TESS_EVALUATION_SHADER_EXT:
            if (tesShader != gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
            if (shaderObjects[i]->shaderInfo.hBinary == gcvNULL)
            {
                gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_TES, (gcSHADER*)&shaderObjects[i]->shaderInfo.hBinary));
            }
            tesShader = (gcSHADER)shaderObjects[i]->shaderInfo.hBinary;
            shaderObjects[i]->shaderInfo.compiledStatus = gcvTRUE;
            break;

        case GL_GEOMETRY_SHADER_EXT:
            if (gsShader != gcvNULL)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }
            if (shaderObjects[i]->shaderInfo.hBinary == gcvNULL)
            {
                gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_GEOMETRY, (gcSHADER*)&shaderObjects[i]->shaderInfo.hBinary));
            }
            gsShader = (gcSHADER)shaderObjects[i]->shaderInfo.hBinary;
            shaderObjects[i]->shaderInfo.compiledStatus = gcvTRUE;
            break;

        default:
            break;
        }

        if ((vertexShader == gcvNULL) && (fragmentShader == gcvNULL) && (computeShader == gcvNULL))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        /* Set shader binary according to the format. */
        if (binaryformat == GL_SHADER_BINARY_VIV)
        {
            gctUINT32_PTR   compilerVersion = gcvNULL;
            gcSHADER_KIND   shaderType      = gcSHADER_TYPE_UNKNOWN;
            gctUINT32       shaderVersion   = 0;

            gcmONERROR(gcSHADER_Construct(gcSHADER_TYPE_PRECOMPILED, &shader));

            gcSHADER_GetCompilerVersion((vertexShader ? vertexShader : fragmentShader ? fragmentShader : computeShader), &compilerVersion);
            gcSHADER_SetCompilerVersion(shader, compilerVersion);
            gcmONERROR(gcSHADER_LoadHeader(shader, (gctPOINTER) binary, length, &shaderVersion));

            gcSHADER_GetType(shader, &shaderType);
            if (shaderType == gcSHADER_TYPE_VERTEX && vertexShader)
            {
                found = vertexShader;
            }
            else if (shaderType == gcSHADER_TYPE_FRAGMENT && fragmentShader)
            {
                found = fragmentShader;
            }
            else if(shaderType == gcSHADER_TYPE_COMPUTE && computeShader)
            {
                found = computeShader;
            }
            else if (shaderType == gcSHADER_TYPE_TCS && tcsShader)
            {
                found = tcsShader;
            }
            else if (shaderType == gcSHADER_TYPE_TES && tesShader)
            {
                found = tesShader;
            }
            else if(shaderType == gcSHADER_TYPE_GEOMETRY && gsShader)
            {
                found = gsShader;
            }

            gcmONERROR(gcSHADER_Destroy(shader));
            shader = gcvNULL;

            if (!found)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            gcmONERROR(gcSHADER_Load(found, (gctPOINTER) binary, length));
        }
        else
        {
            if (binaryformat != GL_PROGRAM_BINARY_VIV)
            {
                gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            if (computeShader)
            {
                gcmONERROR(gcLoadComputeProgram((gctPOINTER) binary,
                                                 length,
                                                 computeShader,
                                                 gcvNULL));
            }
        }
    }

    if((binaryformat == GL_PROGRAM_BINARY_VIV) && vertexShader)
    {
        if ((vertexShader == gcvNULL) || (fragmentShader == gcvNULL))
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        shaders[__GLSL_STAGE_VS] = vertexShader;
        shaders[__GLSL_STAGE_FS] = fragmentShader;
        shaders[__GLSL_STAGE_TCS] = tcsShader;
        shaders[__GLSL_STAGE_TES] = tesShader;
        shaders[__GLSL_STAGE_GS] = gsShader;

        gcmONERROR(gcLoadGraphicsProgram((gctPOINTER) binary,
                                            length,
                                            shaders,
                                            gcvNULL));
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    /* free memory.*/
    if (shader)
    {
        gcmVERIFY_OK(gcSHADER_Destroy(shader));
    }
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLboolean
__glChipBindAttributeLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    const GLchar *name
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLBinding *binding = gcvNULL;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x index=%u name=%s", gc, programObject, index, name);

    if (index >= gc->constants.shaderCaps.maxUserVertAttributes)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    for (binding = program->attribBinding; binding != gcvNULL; binding = binding->next)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(binding->name, name)))
        {
            binding->index = index;
            gcmFOOTER_ARG("return=%d", GL_TRUE);
            return GL_TRUE;
        }
        /* Check aliased attrib */
        else if (binding->index == (GLint)index)
        {
            program->mayHasAliasedAttrib = gcvTRUE;
        }
    }

    do
    {
        gceSTATUS status;
        gctPOINTER pointer = gcvNULL;

        gcmERR_BREAK(gcoOS_Allocate(gcvNULL,
                                    gcmSIZEOF(__GLchipSLBinding),
                                    &pointer));

        binding = pointer;

        gcmERR_BREAK(gcoOS_StrDup(gcvNULL, name, &binding->name));

        binding->index         = index;
        binding->next          = program->attribBinding;
        program->attribBinding = binding;

        gcmFOOTER_ARG("return=%d", GL_TRUE);
        return GL_TRUE;
    } while (gcvFALSE);

OnError:
    if (binding != gcvNULL)
    {
        gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, binding));
    }
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;
}

GLint
__glChipGetAttributeLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *name
    )
{
    GLuint i;
    gctSIZE_T nameLen = 0;
    gctSIZE_T arrayIdx = 0;
    gctUINT aliasedCount = 0;
    gctUINT attribIndex = 0;
    GLboolean isArray = GL_FALSE;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x name=%s", gc, programObject, name);

    /* Program input can't be arrays of arrays, so we just need to check parsed name. */
    gcChipUtilParseUniformName(name, &nameLen, &arrayIdx, &isArray);

    /* Walk all attributes. */
    for (i = 0; i < gc->constants.shaderCaps.maxUserVertAttributes; ++i)
    {
        __GLchipSLLinkage* attribLinkage = gcvNULL;

        if (!program->attribLinkage[i])
        {
            continue;
        }

        for(attribLinkage = program->attribLinkage[i]; attribLinkage != gcvNULL; attribLinkage = attribLinkage->next)
        {
            for (aliasedCount = 0; aliasedCount < MAX_ALIASED_ATTRIB_COUNT; ++aliasedCount)
            {
                attribIndex = attribLinkage->attribLocation + aliasedCount * gc->constants.shaderCaps.maxVertAttributes;

                if (!program->attribLocation[attribIndex].pInput)
                {
                    continue;
                }

                /* See if the attribute matches the requested name. */
                if (nameLen == (gctSIZE_T)program->attribLocation[attribIndex].pInput->nameLen &&
                    (!isArray || program->attribLocation[attribIndex].pInput->isArray) &&
                    gcmIS_SUCCESS(gcoOS_StrNCmp(name, program->attribLocation[attribIndex].pInput->name, nameLen))
                    )
                {
                    GLuint comNum = 1;
                    switch (program->attribLocation[attribIndex].pInput->type)
                    {
                    case gcSHADER_FLOAT_2X2:
                    case gcSHADER_FLOAT_2X3:
                    case gcSHADER_FLOAT_2X4:
                        comNum = 2;
                        break;
                    case gcSHADER_FLOAT_3X2:
                    case gcSHADER_FLOAT_3X3:
                    case gcSHADER_FLOAT_3X4:
                        comNum = 3;
                        break;
                    case gcSHADER_FLOAT_4X2:
                    case gcSHADER_FLOAT_4X3:
                    case gcSHADER_FLOAT_4X4:
                        comNum = 4;
                        break;
                    default:
                        break;
                    }
                    gcmFOOTER_ARG("return=%d", i);
                    return i + (GLuint)(arrayIdx * comNum);
                }
            }
        }
    }

    gcmFOOTER_ARG("return=%d", -1);
    return -1;
}

GLboolean
__glChipGetActiveAttribute(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    GLsizei bufsize,
    GLsizei *length,
    GLint *size,
    GLenum *type,
    char *name
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLInput *input;
    gctSIZE_T nameLen = 0;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x index=%d bufsize=%d length=0x%x size=0x%x type=0x%x name=0x%x",
                   gc, programObject, index, bufsize, length, size, type, name);

    /* Get gcATTRIBUTE object pointer. */
    input = &program->inputs[index];

    if (input == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* According to spec, the returned string should be null-terminated,
    ** but "length" returned should exclude null-terminator.
    */
    if (name && bufsize > 0)
    {
        nameLen = gcoOS_StrLen(input->name, gcvNULL);
        nameLen = __GL_MIN(nameLen, (gctSIZE_T)bufsize - 1);
        if (nameLen > 0)
        {
            /* Copy the name to the buffer. */
            gcoOS_MemCopy(name, input->name, nameLen);
        }
        name[nameLen] = '\0';
    }

    if (length)
    {
        /* Return number of characters copied. */
        *length = (GLsizei)nameLen;
    }

    if (size)
    {
        *size = 1;
    }

    /* Convert type to GL enumeration. */
    if (type)
    {
        /* make sure the table was not broken due to definition change */
        GL_ASSERT(g_typeInfos[input->type].halType == input->type);
        *type = g_typeInfos[input->type].glType;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", GL_FALSE);
    return GL_FALSE;

}

GLint
__glChipGetFragDataLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *name
    )
{
    GLuint i;
    GLint location = -1;
    gctSIZE_T nameLen = 0;
    gctSIZE_T arrayIdx = 0;
    GLboolean isArray = GL_FALSE;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x name=%s", gc, programObject, name);

    if (gcChipUtilParseUniformName(name, &nameLen, &arrayIdx, &isArray))
    {
        __GLchipSLOutput *output = gcvNULL;

        for (i = 0; i < program->outCount; ++i)
        {
            output = &program->outputs[i];

            if (nameLen == output->nameLen &&
                (!isArray || output->isArray) &&
                gcmIS_SUCCESS(gcoOS_MemCmp(name, output->name, nameLen)))
            {
                break;
            }
        }

        /* If found the output and arrayIdx is within range */
        if (i < program->outCount && arrayIdx < (gctSIZE_T)output->arraySize)
        {
            location = output->location + (gctINT)arrayIdx - output->startIndex;
        }
    }

    gcmFOOTER_ARG("return=%d", location);
    return location;
}

GLint
__glChipGetUniformLocation(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *name
    )
{
    GLint location = -1;
    gctSIZE_T nameLen = 0;
    gctSIZE_T arrayIdx = 0;
    GLboolean isArray = GL_FALSE;
    __GLchipSLUniform* uniform;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x name=%s", gc, programObject, name);

    if (gcChipUtilParseUniformName(name, &nameLen, &arrayIdx, &isArray))
    {
        uniform = gcChipGetUniformByName(gc, program, name, nameLen);
        if (uniform &&
            (!isArray || uniform->isArray) &&
            arrayIdx < uniform->arraySize)
        {
            location = uniform->location + (gctINT)arrayIdx;
        }

        if (isArray && !uniform)
        {
            nameLen = gcoOS_StrLen(name, gcvNULL);
            uniform = gcChipGetUniformByName(gc, program, name, nameLen);
            if (uniform)
            {
                location = uniform->location;
            }
        }
    }

    gcmFOOTER_ARG("return=%d", location);
    return location;
}

GLvoid
__glChipGetActiveUniform(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint index,
    GLsizei bufsize,
    GLsizei *length,
    GLint *size,
    GLenum *type,
    GLchar *name
    )
{
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLUniform *uniform = &program->uniforms[index];
    gctSIZE_T nameLen = 0;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x index=%d bufsize=%d length=0x%x size=0x%x type=0x%x name=0x%x",
                   gc, programObject, index, bufsize, length, size, type, name);

    /* According to spec, the returned string should be null-terminated,
    ** but "length" returned should exclude null-terminator.
    */
    if (name && bufsize > 0)
    {
        nameLen = __GL_MIN(uniform->nameLen, (gctSIZE_T)bufsize - 1);
        if (nameLen > 0)
        {
            gcoOS_MemCopy(name, uniform->name, nameLen);
        }
        name[nameLen] = '\0';

        if (uniform->isArray)
        {
            gcoOS_StrCatSafe(name, bufsize, "[0]");
            nameLen = __GL_MIN(nameLen + 3, (gctSIZE_T)bufsize - 1);
        }
    }

    if (length != gcvNULL)
    {
        /* Return number of characters copied. */
        *length = (GLsizei)nameLen;
    }

    if (size)
    {
        *size = (GLint)uniform->arraySize;
    }

    if (type)
    {
        /* make sure the table was not broken due to definition change */
        GL_ASSERT(g_typeInfos[uniform->dataType].halType == uniform->dataType);
        *type = g_typeInfos[uniform->dataType].glType;
    }
    gcmFOOTER_NO();
}

GLvoid
__glChipGetActiveUniformsiv(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei uniformCount,
    const GLuint *uniformIndices,
    GLenum pname,
    GLint *params
    )
{
    GLsizei i;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x uniformCount=%d uniformIndices=0x%x pname=0x%04x params=0x%x",
                   gc, programObject, uniformCount, uniformIndices, pname, params);

    for (i = 0; i < uniformCount; ++i)
    {
        __GLchipSLUniform *uniform = &program->uniforms[uniformIndices[i]];

        switch (pname)
        {
        case GL_UNIFORM_TYPE:
            /* make sure the table was not broken due to definition change */
            GL_ASSERT(g_typeInfos[uniform->dataType].halType == uniform->dataType);
            params[i] = g_typeInfos[uniform->dataType].glType;
            break;
        case GL_UNIFORM_SIZE:
            params[i] = (GLint)uniform->arraySize;
            break;
        case GL_UNIFORM_NAME_LENGTH:
            /* If it an array, the return name length should include "[0]" */
            params[i] = (GLint)uniform->nameLen + (uniform->isArray ? 4 : 1);
            break;
        case GL_UNIFORM_BLOCK_INDEX:
            params[i] = (uniform->ubIndex < program->userDefUbCount) ? uniform->ubIndex : -1;
            break;
        case GL_UNIFORM_OFFSET:
            params[i] = (uniform->ubIndex < program->userDefUbCount) ? uniform->offset : -1;
            break;
        case GL_UNIFORM_ARRAY_STRIDE:
            params[i] = uniform->arrayStride;
            break;
        case GL_UNIFORM_MATRIX_STRIDE:
            params[i] = uniform->matrixStride;
            break;
        case GL_UNIFORM_IS_ROW_MAJOR:
            params[i] = (GLint)uniform->isRowMajor;
            break;

        default:
            GL_ASSERT(0);
        }
    }
    gcmFOOTER_NO();
}

GLvoid
__glChipGetUniformIndices(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLsizei uniformCount,
    const GLchar* const * uniformNames,
    GLuint *uniformIndices
    )
{
    GLsizei i;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x uniformCount=%d uniformNames=0x%x uniformIndices=0x%x",
                   gc, programObject, uniformCount, uniformNames, uniformIndices);

    /*
    ** Uniforms are organized in the order:
    ** user uniforms (including sampler) -> builtIn uniforms
    */
    for (i = 0; i < uniformCount; ++i)
    {
        GLint j;
        GLuint index = GL_INVALID_INDEX;
        gctCONST_STRING uniformName = uniformNames[i];
        gctSIZE_T nameLen = 0;
        gctSIZE_T arrayIdx = 0;
        GLboolean isArray = GL_FALSE;

        /* Indices can be queried by either "arrayName" or "arrayname[0]" for array types */
        if (gcChipUtilParseUniformName(uniformName, &nameLen, &arrayIdx, &isArray) || arrayIdx != 0)
        {
            for (j = 0; j < program->activeUniformCount; ++j)
            {
                if (nameLen == program->uniforms[j].nameLen &&
                    (!isArray || program->uniforms[j].isArray) &&
                    gcmIS_SUCCESS(gcoOS_MemCmp(program->uniforms[j].name, uniformName, nameLen)))
                {
                    index = j;
                    break;
                }
            }

            /* If array name cannot find, try it as not index array name */
            if (isArray && index == GL_INVALID_INDEX)
            {
                for (j = 0; j < program->activeUniformCount; ++j)
                {
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(program->uniforms[j].name, uniformName)))
                    {
                        index = j;
                        break;
                    }
                }
            }

            uniformIndices[i] = index;
        }
        else
        {
            uniformIndices[i] = GL_INVALID_INDEX;
        }
    }
    gcmFOOTER_NO();
}

GLuint
__glChipGetUniformBlockIndex(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    const GLchar *uniformBlockName
    )
{
    GLuint i, ubIdx = GL_INVALID_INDEX;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x uniformBlockName=%s",
                   gc, programObject, uniformBlockName);

    for (i = 0; i < (GLuint)program->userDefUbCount; ++i)
    {
        if (gcmIS_SUCCESS(gcoOS_StrCmp(uniformBlockName, program->uniformBlocks[i].name)))
        {
            ubIdx = i;
            break;
        }
    }

    /* If failed to find and name didn't include array index, append "[0]" and retry */
    if (ubIdx == GL_INVALID_INDEX)
    {
        gctSTRING arrayName;
        gctSIZE_T nameLen = gcoOS_StrLen(uniformBlockName, gcvNULL);

        if (uniformBlockName[nameLen - 1] != ']')
        {
            gcoOS_Allocate(gcvNULL, nameLen + 4, (gctPOINTER*)&arrayName);
            gcoOS_StrCopySafe(arrayName, nameLen + 4, uniformBlockName);
            gcoOS_StrCatSafe(arrayName, nameLen + 4, "[0]");

            for (i = 0; i < (GLuint)program->userDefUbCount; ++i)
            {
                if (gcmIS_SUCCESS(gcoOS_StrCmp(arrayName, program->uniformBlocks[i].name)))
                {
                    ubIdx = i;
                    break;
                }
            }

            gcmOS_SAFE_FREE(gcvNULL, arrayName);
        }
    }

    gcmFOOTER_ARG("return=%u", ubIdx);
    return ubIdx;
}

GLvoid
__glChipGetActiveUniformBlockiv(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint uniformBlockIndex,
    GLenum pname,
    GLint *params
    )
{

    gcmHEADER_ARG("gc=0x%x programObject=0x%x uniformBlockIndex=%d pname=0x%04x params=0x%x",
                   gc, programObject, uniformBlockIndex, pname, params);

    do {
        __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
        __GLchipSLUniformBlock *ub;
        gctSIZE_T i;

        if (uniformBlockIndex >= (GLuint)program->userDefUbCount)
        {
            break;
        }

        ub = &program->uniformBlocks[uniformBlockIndex];

        switch (pname)
        {
        case GL_UNIFORM_BLOCK_BINDING:
            *params = (GLint)ub->binding;
            break;
        case GL_UNIFORM_BLOCK_DATA_SIZE:
            *params = (GLint)ub->dataSize;
            break;
        case GL_UNIFORM_BLOCK_NAME_LENGTH:
            *params = (GLint)ub->nameLen + 1;
            break;
        case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
            *params = (GLint)ub->activeUniforms;
            break;
        case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
            for (i = 0; i < ub->activeUniforms; ++i)
            {
                params[i] = (GLint)ub->uniformIndices[i];
            }
            break;
        case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
            *params = ub->halUB[__GLSL_STAGE_VS] ? 1 : 0;
            break;
        case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
            *params = ub->halUB[__GLSL_STAGE_FS] ? 1 : 0;
            break;

        default:
            GL_ASSERT(0);
        }
    } while (GL_FALSE);

    gcmFOOTER_NO();
}

GLvoid
__glChipActiveUniformBlockName(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint uniformBlockIndex,
    GLsizei bufSize,
    GLsizei *length,
    GLchar *uniformBlockName
    )
{
    gcmHEADER_ARG("gc=0x%x programObject=0x%x uniformBlockIndex=%d bufSize=%d length=0x%x uniformBlockName=0x%x",
                   gc, programObject, uniformBlockIndex, bufSize, length, uniformBlockName);

    do {
        __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
        __GLchipSLUniformBlock *ub;
        gctSIZE_T nameLen = 0;

        if (uniformBlockIndex >= (GLuint)program->userDefUbCount)
        {
            break;
        }

        ub = &program->uniformBlocks[uniformBlockIndex];

        /* According to spec, the returned string should be null-terminated,
        ** but "length" returned should exclude null-terminator.
        */
        if (uniformBlockName && bufSize > 0)
        {
            nameLen = __GL_MIN(ub->nameLen, (gctSIZE_T)bufSize - 1);
            if (nameLen > 0)
            {
                gcoOS_MemCopy(uniformBlockName, ub->name, nameLen);
            }
            uniformBlockName[nameLen] = '\0';
        }

        if (length != gcvNULL)
        {
            /* Return number of characters copied. */
            *length = (GLsizei)nameLen;
        }
    } while (GL_FALSE);

    gcmFOOTER_NO();
}

GLvoid
__glChipUniformBlockBinding(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLuint uniformBlockIndex,
    GLuint uniformBlockBinding
    )
{
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x uniformBlockIndex=%d uniformBlockBinding=%d",
                   gc, programObject, uniformBlockIndex, uniformBlockBinding);

    GL_ASSERT(uniformBlockIndex < (GLuint)program->userDefUbCount);

    if (program->uniformBlocks[uniformBlockIndex].binding != uniformBlockBinding)
    {
        program->uniformBlocks[uniformBlockIndex].binding = uniformBlockBinding;
        __glBitmaskSet(&gc->bufferObject.bindingDirties[__GL_UNIFORM_BUFFER_INDEX], uniformBlockBinding);
    }

    gcmFOOTER_NO();
}

GLboolean
__glChipSetUniformData(
    __GLcontext       *gc,
    __GLprogramObject *programObject,
    GLint              location,
    GLint              type,
    GLsizei            count,
    const GLvoid      *values,
    GLboolean          transpose
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    GLint arrayIdx;
    __GLchipSLProgram *program;
    __GLchipSLUniform *uniform;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x location=%d type=%d count=%d values=0x%x transpose=%d",
                   gc, programObject, location, type, count, values, transpose);

    if (values == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    program = (__GLchipSLProgram *)programObject->privateData;
    if (program == gcvNULL)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (location < 0 || location >= program->maxLocation || !program->loc2Uniform[location])
    {
        __GL_ERROR(GL_INVALID_OPERATION);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    uniform = program->loc2Uniform[location];

    /* The found uniform is not locatable */
    if (uniform->location == -1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    arrayIdx = location - uniform->location;
    GL_ASSERT(arrayIdx >= 0 && arrayIdx < (GLint)uniform->arraySize);

    gcmONERROR(gcChipSetUniformData(gc, programObject, program, uniform, type, (gctSIZE_T)count,
                                    (GLuint)arrayIdx, values, transpose));

 OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}

GLboolean
__glChipGetUniformData(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLint location,
    GLint type,
    GLvoid *values
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLUniform *uniform;
    GLuint arrayIdx;
    gctSIZE_T bytes;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x location=%d type=%d values=0x%x",
                   gc, programObject, location, type, values);

    if (location < 0 || location >= program->maxLocation || !program->loc2Uniform[location])
    {
        __GL_ERROR(GL_INVALID_OPERATION);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    uniform = program->loc2Uniform[location];

    /* The found uniform is not locatable */
    if (uniform->location == -1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    arrayIdx = location - uniform->location;
    GL_ASSERT(location >= uniform->location && arrayIdx < (GLuint)uniform->arraySize);

    /* make sure the table was not broken due to definition change */
    GL_ASSERT(g_typeInfos[uniform->dataType].halType == uniform->dataType);
    bytes = g_typeInfos[uniform->dataType].size;

    if ((uniform->dataType >= gcSHADER_IMAGE_2D && uniform->dataType <= gcSHADER_IMAGE_3D) ||
        (uniform->dataType >= gcSHADER_IIMAGE_2D && uniform->dataType <= gcSHADER_UIMAGE_2D_ARRAY))
    {
        GLuint i, j;
        for (i = 0; i < gc->constants.shaderCaps.maxImageUnit; i++)
        {
            __GLchipImageUnit2Uniform *pImageUnit2Uniform = &program->imageUnit2Uniform[i];
            for (j = 0; j < pImageUnit2Uniform->numUniform; j++)
            {
                if (pImageUnit2Uniform->uniforms[j].slUniform == uniform &&
                    pImageUnit2Uniform->uniforms[j].arrayIndex == arrayIdx)
                {
                    *(GLuint*)values = i;
                    goto OnError;
                }
            }
        }
    }
    /* Boolean was converted to int {0, 1} when set to GL, it only need to be converted back to float
    ** if app wants to retrieve using glGetUniformfv
    */
    else if (uniform->dataType >= gcSHADER_BOOLEAN_X1 && uniform->dataType <= gcSHADER_BOOLEAN_X4 &&
        type == GL_FLOAT)
    {
        gcChipUtilInt2Float(values, (GLint*)((GLbyte*)uniform->data + arrayIdx * bytes), bytes/sizeof(int));
    }
    else
    {
        gcoOS_MemCopy(values, (GLbyte*)(uniform->data) + arrayIdx * bytes, bytes);
    }

OnError:
    if (gcmIS_ERROR(status))
    {
        gcChipSetError(chipCtx, status);
        gcmFOOTER_ARG("return=%d", GL_FALSE);
        return GL_FALSE;
    }

    gcmFOOTER_ARG("return=%d", GL_TRUE);
    return GL_TRUE;
}

GLsizei
__glChipGetUniformSize(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLint location
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLUniform *uniform;
    GLsizei bytes = 0;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x location=%d",
                   gc, programObject, location);

    if (location < 0 || location >= program->maxLocation || !program->loc2Uniform[location])
    {
        __GL_ERROR(GL_INVALID_OPERATION);
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    uniform = program->loc2Uniform[location];

    /* The found uniform is not locatable */
    if (uniform->location == -1)
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* make sure the table was not broken due to definition change */
    GL_ASSERT(g_typeInfos[uniform->dataType].halType == uniform->dataType);

    bytes = (GLsizei)g_typeInfos[uniform->dataType].size;

OnError:
    gcChipSetError(chipCtx, status);
    gcmFOOTER_ARG("return=%d", bytes);
    return bytes;
}

GLvoid
__glChipBuildTexEnableDim(
    __GLcontext *gc
    )
{
    GLuint unit;
    GLuint sampler;
    GLuint enableDim;
    __GLSLStage stage;
    GLuint *sampler2TexUnit = gc->state.program.sampler2TexUnit;
    __GLtexUnit2Sampler *texUnit2Sampler = gc->shaderProgram.texUnit2Sampler;
    __GLtextureUnitState *texUnitState = gc->state.texture.texUnits;
    __GLchipSLProgram *programs[__GLSL_STAGE_LAST] = { gcvNULL };
    __GLchipSLProgramInstance *pgInstance[__GLSL_STAGE_LAST] = { gcvNULL };

    gcmHEADER_ARG("gc=0x%x", gc);

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        __GLprogramObject *progObj = gc->shaderProgram.activeProgObjs[stage];
        programs[stage] = progObj ? (__GLchipSLProgram*)progObj->privateData : gcvNULL;
        pgInstance[stage] = programs[stage] ? programs[stage]->masterPgInstance : gcvNULL;
    }

    for (unit = 0; unit < gc->shaderProgram.maxUnit; ++unit)
    {
        /* mark enable dimensions of all texture unit to be disabled */
        texUnitState[unit].enableDim = __GL_MAX_TEXTURE_BINDINGS;
        /* Init all texture unit map to none sampler */
        texUnit2Sampler[unit].numSamplers = 0;
    }

    for (sampler = 0; sampler < gc->shaderProgram.maxSampler; ++sampler)
    {
        unit = __GL_MAX_TEXTURE_UNITS;
        enableDim = __GL_MAX_TEXTURE_BINDINGS;

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (programs[stage] && programs[stage]->samplerMap[sampler].stage == stage)
            {
                unit = programs[stage]->samplerMap[sampler].unit;
                enableDim = programs[stage]->samplerMap[sampler].texDim;
                break;
            }

            if (pgInstance[stage] &&
                pgInstance[stage]->extraSamplerMap[sampler].stage == stage &&
                pgInstance[stage]->extraSamplerMap[sampler].auxiliary == GL_FALSE)
            {
                unit = pgInstance[stage]->extraSamplerMap[sampler].unit;
                enableDim = pgInstance[stage]->extraSamplerMap[sampler].texDim;

                /* For advanced blend, need to set sampler state dirty for programimg rt texture later */
                if (pgInstance[stage]->extraSamplerMap[sampler].subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_SAMPLER)
                {
                    __glBitmaskSet(&gc->shaderProgram.samplerStateDirty, sampler);
                }
                break;
            }
        }

        sampler2TexUnit[sampler] = unit;

        if (enableDim < __GL_MAX_TEXTURE_BINDINGS)
        {
            /* If other samplers require the unit to be enabled, but conflict with current sampler */
            if (texUnitState[unit].enableDim == __GL_MAX_TEXTURE_BINDINGS)
            {
                texUnitState[unit].enableDim = enableDim;
            }
            else if (texUnitState[unit].enableDim != enableDim)
            {
                __glBitmaskSet(&gc->texture.texConflict, unit);
            }

            /* Record the all the samplers bound to this unit */
            texUnit2Sampler[unit].samplers[texUnit2Sampler[unit].numSamplers++] = sampler;
        }
    }

    gcmFOOTER_NO();
}

gceSTATUS
gcChipLoadCompiler(
    __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;
    VSC_HW_CONFIG hwCfg;

    gcmHEADER_ARG("gc=0x%x", gc);

    gcQueryShaderCompilerHwCfg(gcvNULL, &hwCfg);

#if gcdSTATIC_LINK || defined(__CHROME_OS__)

    do
    {
        static gcsVSC_APIS vscAPIs = {
            gcCompileShader,
            gcLinkShaders,
            gcSHADER_Construct,
            gcSHADER_AddAttribute,
            gcSHADER_AddUniform,
            gcSHADER_AddOpcode,
            gcSHADER_AddOpcodeConditional,
            gcSHADER_AddSourceUniformIndexedFormattedWithPrecision,
            gcSHADER_AddSourceAttribute,
            gcSHADER_AddSourceConstant,
            gcSHADER_AddOutput,
            gcSHADER_SetCompilerVersion,
            gcSHADER_Pack,
            gcSHADER_Destroy,
            gcSHADER_Copy,
            gcSHADER_DynamicPatch,
            gcCreateOutputConversionDirective,
            gcCreateInputConversionDirective,
            gcFreeProgramState,
            gcSetGLSLCompiler,
            gcDestroyPatchDirective
        };

        chipCtx->pfCompile = gcCompileShader;
        chipCtx->pfInitCompiler = gcInitializeCompiler;
        chipCtx->pfInitCompilerCaps = gcInitializeCompilerCaps;
        chipCtx->pfFinalizeCompiler = gcFinalizeCompiler;

        gcmERR_BREAK(gcInitializeCompiler(chipCtx->patchId, &hwCfg, &gc->constants.shaderCaps));
        gcmERR_BREAK(gcoHAL_SetCompilerFuncTable(chipCtx->hal, &vscAPIs));
    }
    while (gcvFALSE);

#else

    do
    {
        union __GLinitializerUnion
        {
            gctGLSLInitCompiler initGLSL;
            gctPOINTER          ptr;
        } intializer;

        union __GLinitializerCapsUnion
        {
            gctGLSLInitCompilerCaps initCaps;
            gctPOINTER          ptr;
        } intializerCaps;

        union __GLfinalizerUnion
        {
            gctGLSLFinalizeCompiler finalizeGLSL;
            gctPOINTER ptr;
        } finalizer;

        union __GLcompilerUnion
        {
            gctGLSLCompiler     compile;
            gctPOINTER          ptr;
        } compiler;

        status = gcoOS_LoadLibrary(gcvNULL,
#if defined(__APPLE__)
                                   "libGLSLC.dylib",
#elif defined(LINUXEMULATOR)
                                   "libGLSLC.so",
#elif defined(__QNXNTO__)
                                   "glesv2-sc-dlls",
#else
                                   "libGLSLC",
#endif
                                   &chipCtx->dll);

        if (gcmIS_ERROR(status))
        {
            break;
        }

        gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL, chipCtx->dll, "gcCompileShader", &compiler.ptr));
        gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL, chipCtx->dll, "gcInitializeCompiler", &intializer.ptr));
        gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL, chipCtx->dll, "gcInitializeCompilerCaps", &intializerCaps.ptr));
        gcmERR_BREAK(gcoOS_GetProcAddress(gcvNULL, chipCtx->dll, "gcFinalizeCompiler", &finalizer.ptr));

        chipCtx->pfCompile = compiler.compile;
        chipCtx->pfInitCompiler = intializer.initGLSL;
        chipCtx->pfInitCompilerCaps = intializerCaps.initCaps;
        chipCtx->pfFinalizeCompiler = finalizer.finalizeGLSL;

        gcmERR_BREAK(chipCtx->pfInitCompiler(chipCtx->patchId, &hwCfg, &gc->constants.shaderCaps));
    } while (gcvFALSE);

#endif

    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipReleaseCompiler(
    IN __GLcontext *gc
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x", gc);

    do
    {
        gcmERR_BREAK(chipCtx->pfFinalizeCompiler());
        chipCtx->pfCompile = gcvNULL;
    } while (gcvFALSE);


    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipMarkUniformDirtyCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    )
{
    GLint i;
    gceSTATUS status = gcvSTATUS_OK;
    GLbitfield stageDirties[] =
    {
        __GL_DIRTY_GLSL_VS_SWITCH,
        __GL_DIRTY_GLSL_TCS_SWITCH,
        __GL_DIRTY_GLSL_TES_SWITCH,
        __GL_DIRTY_GLSL_GS_SWITCH,
        __GL_DIRTY_GLSL_FS_SWITCH,
        __GL_DIRTY_GLSL_CS_SWITCH
    };

    gcmHEADER_ARG("gc=0x%x progObj=0x%x program=0x%x stage=%d",
                   gc, progObj, program, stage);

    /* Mark all uniforms dirty if program switched */
    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & stageDirties[stage])
    {
        for (i = 0; i < program->activeUniformCount; ++i)
        {
            program->uniforms[i].dirty = GL_TRUE;
        }

        for (i = 0; i < program->curPgInstance->privateUniformCount; ++i)
        {
            program->curPgInstance->privateUniforms[i].dirty = GL_TRUE;
        }
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipClearUniformDirtyCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    )
{
    GLint i;
    __GLchipSLProgramInstance* pgInstance = program->curPgInstance;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x progObj=0x%x program=0x%x stage=%d",
                   gc, progObj, program, stage);

    /* Clean up uniform dirty */
    for (i = 0; i < program->activeUniformCount; ++i)
    {
        program->uniforms[i].dirty = GL_FALSE;
    }
    for (i = 0; i < pgInstance->privateUniformCount; ++i)
    {
        pgInstance->privateUniforms[i].dirty = GL_FALSE;
    }

    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipFlushUniformBlock(
    __GLcontext *gc,
    __GLchipSLProgram *program,
    __GLchipSLUniformBlock *ub,
    gcoBUFOBJ bufObj,
    gctUINT8_PTR logical,
    gctSIZE_T offset,
    gctSIZE_T size
    )
{
    __GLSLStage stageIdx;
    gctUINT32 physical = 0;
    gceSTATUS status = gcvSTATUS_OK;
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);

    gcmHEADER_ARG("gc=0x%x program=0x%x ub=0x%x bufObj=0x%x logical=%u offset=%lu size=%lu",
                   gc, program, ub, bufObj, logical, offset, size);

    /* Verify the arguments. */
    gcmDEBUG_VERIFY_ARGUMENT(size > 0);

    if (bufObj)
    {
        /* No need to map logic if driver already allocate a copy */
        gcmONERROR(gcoBUFOBJ_Lock(bufObj, &physical, logical ? gcvNULL : (gctPOINTER*)&logical));
    }

    for (stageIdx = __GLSL_STAGE_VS; stageIdx < __GLSL_STAGE_LAST; ++stageIdx)
    {
        gcUNIFORM ubUniform = ub->halUniform[stageIdx];

        if (!(ubUniform && isUniformUsedInShader(ubUniform)))
        {
            /* Skip if not exist or used */
            continue;
        }

        /* If any part of the UB was mapped to video memory */
        if (ub->mapFlags[stageIdx] & gcdUB_MAPPED_TO_MEM)
        {
            gctUINT32 physicalAddress = 0;
            gctINT32 index;
            gctUINT32 data;
            gctUINT32 baseOffset;

            if (gc->shaderProgram.boundPPO || chipCtx->chipDirty.uDefer.sDefer.pgInsChanged)
            {
                gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(chipCtx->activeProgState->hints->hwConstRegBases,
                                                                  ubUniform,
                                                                  &ub->stateAddress[stageIdx]));
            }

            physicalAddress = ub->stateAddress[stageIdx];
            index = GetUniformPhysical(ubUniform);

            if (-1 != GetUBArrayIndex(ub->halUB[stageIdx]))
            {
                physicalAddress += GetUBArrayIndex(ub->halUB[stageIdx]) * 16;
                index += GetUBArrayIndex(ub->halUB[stageIdx]);
            }

            gcmSAFECASTSIZET(baseOffset, offset);

            data = physical + baseOffset;

            gcmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddress, index,
                                             1, 1, 1, gcvFALSE, 1, 4,
                                             &data, gcvFALSE, GetUniformShaderKind(ubUniform)));

            if (program->progFlags.robustEnabled)
            {
                gctUINT32 addressLimit[2];
                gctSIZE_T size;
                gctUINT32 bufSize;

                gcoBUFOBJ_GetSize(bufObj, &size);
                gcmSAFECASTSIZET(bufSize, size);

                addressLimit[0] = physical;
                addressLimit[1] = physical + bufSize - 1;

                /*the base channel of the ubo must be x or y*/
                gcmASSERT((physicalAddress & 0xF) == 0x0 || (physicalAddress & 0xF) == 0x4);

                gcmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddress + 4, index,
                                                 2, 1, 1, gcvFALSE, 0, 0,
                                                 addressLimit, gcvFALSE, GetUniformShaderKind(ubUniform)));

            }
        }

        /* If any part of the UB was mapped to state buffer */
        if (ub->mapFlags[stageIdx] & gcdUB_MAPPED_TO_REG)
        {
            gctUINT32 i;
            gctUINT32 numUniforms;
            gcsUNIFORM_BLOCK uniformBlock = gcvNULL;
            gcSHADER shader = program->curPgInstance->binaries[stageIdx];

            GL_ASSERT(shader);
            uniformBlock = ub->halUB[stageIdx];
            gcmONERROR(gcSHADER_GetUniformBlockUniformCount(shader, uniformBlock, &numUniforms));

            for (i = 0; i < numUniforms; ++i)
            {
                GLboolean dirtied;
                gcUNIFORM uniform;
                gcmONERROR(gcSHADER_GetUniformBlockUniform(shader, uniformBlock, i, &uniform));

                if (ub->usage == __GL_CHIP_UB_USAGE_DEFAULT)
                {
                    gctINT16 glUniformIndex = GetUniformGlUniformIndex(uniform);
                    if (glUniformIndex >= 0 && glUniformIndex < program->activeUniformCount)
                    {
                        dirtied = program->uniforms[glUniformIndex].dirty;
                    }
                    else
                    {
                        dirtied = GL_TRUE;
                    }
                }
                else
                {
                    dirtied = GL_TRUE;
                }

                if (dirtied && GetUniformPhysical(uniform) != -1 && isUniformUsedInShader(uniform))
                {
                    gctUINT      entries, arraySize;
                    gctUINT32    numCols = 0, numRows = 0;
                    gctBOOL      isRowMajor = (GetUniformMatrixStride(uniform) != 0 && GetUniformIsRowMajor(uniform));
                    gctUINT8_PTR pData = logical + offset + GetUniformOffset(uniform);
                    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
                    __GLchipFeature *chipFeature = &chipCtx->chipFeature;
                    gceUNIFORMCVT convert = gcChipQueryUniformConvert(program, GetUniformType(uniform),
                                                                           ub->usage == __GL_CHIP_UB_USAGE_USER_DEFINED,
                                                                           chipFeature->haltiLevel > __GL_CHIP_HALTI_LEVEL_0);
                    gctUINT32 physicalAddress = 0;

                    gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(chipCtx->activeProgState->hints->hwConstRegBases,
                                                                      uniform,
                                                                      &physicalAddress));

                    gcTYPE_GetTypeInfo(GetUniformType(uniform), &numCols, &numRows, gcvNULL);

                    entries = gcChipGetUniformArrayInfo(uniform, gcvNULL, gcvNULL, gcvNULL, &arraySize);
                    arraySize *= entries;   /* Expand array size for array of arrays to one dimension */

                    gcmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddress, GetUniformPhysical(uniform),
                                                     numCols, numRows, arraySize, isRowMajor,
                                                     (gctSIZE_T)GetUniformMatrixStride(uniform),
                                                     (gctSIZE_T)GetUniformArrayStride(uniform),
                                                     pData, convert, GetUniformShaderKind(uniform)));
                }
            }
        }
    }

OnError:
    if (bufObj && physical)
    {
        gcoBUFOBJ_Unlock(bufObj);
    }
    gcmFOOTER();

    return status;
}

/*
** Get the mapped texture unit of auxiliary texture uniform.
*/
__GL_INLINE GLuint
gcChipGetTexAuxUniformUnit(
    __GLcontext *gc,
    __GLchipSLProgram *program,
    __GLchipSLUniform *uniform
    )
{
    GLuint unit = 0xFFFFFFFF;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x program=0x%x uniform=0x%x", gc, program, uniform);

    do
    {
        gctUINT32 sampler = 0, samplerBase = 0;
        gcUNIFORM samplerUniform = gcvNULL;
        __GLSLStage stage;

        GL_ASSERT(uniform->subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_BASE_LEVEL_SIZE ||
                  uniform->subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_LOD_MIN_MAX);

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (uniform->halUniform[stage])
            {
                gcmERR_BREAK(gcSHADER_GetUniform(program->curPgInstance->binaries[stage],
                                                 GetUniformParent(uniform->halUniform[stage]),
                                                 &samplerUniform));
                samplerBase = gcHINTS_GetSamplerBaseOffset(program->curPgInstance->programState.hints,
                                                           program->curPgInstance->binaries[stage]);
                break;
            }
        }

        GL_ASSERT(samplerUniform);

        gcmERR_BREAK(gcUNIFORM_GetSampler(samplerUniform, &sampler));
        sampler += samplerBase;
        GL_ASSERT(!isUniformArray(samplerUniform));
        unit = program->samplerMap[sampler].unit;
    } while (gcvFALSE);

    GL_ASSERT(unit < gc->constants.shaderCaps.maxCombinedTextureImageUnits);

    gcmFOOTER_ARG("return=%u", unit);
    return unit;
}

/*
** Get the mapped texture unit of auxiliary texture uniform.
*/
__GL_INLINE GLuint
gcChipGetImageAuxUniformUnit(
    __GLcontext *gc,
    __GLchipSLProgram *program,
    __GLchipSLUniform *uniform
    )
{
    GLuint unit = 0xFFFFFFFF;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x program=0x%x uniform=0x%x", gc, program, uniform);

    do
    {
        gcUNIFORM imageUniform = gcvNULL;
        __GLSLStage stage;

        GL_ASSERT(uniform->subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_IMAGE_SIZE);

        for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
        {
            if (uniform->halUniform[stage])
            {
                gcmERR_BREAK(gcSHADER_GetUniform(program->curPgInstance->binaries[stage],
                                                 GetUniformParent(uniform->halUniform[stage]),
                                                 &imageUniform));
                break;
            }
        }

        GL_ASSERT(imageUniform);
        GL_ASSERT(!isUniformArray(imageUniform));
        unit = GetUniformBinding(imageUniform);

    } while (gcvFALSE);

    GL_ASSERT(unit < gc->constants.shaderCaps.maxImageUnit);

    gcmFOOTER_ARG("return=%u", unit);
    return unit;
}


/*
** Try to upload compiler generated uniform excluding LTC uniform
*/
__GL_INLINE GLvoid
gcChipPreparePrivateUniform(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram * program,
    __GLchipSLUniform *uniform
    )
{
    GLfloat       fValues[4];
    GLint         iValues[4];
    gctPOINTER    pointer = gcvNULL;
    gctSIZE_T     size;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x uniform=0x%x", gc, chipCtx, uniform);

    switch (uniform->subUsage)
    {
    case __GL_CHIP_UNIFORM_SUB_USAGE_XFB_ENABLE:
    case __GL_CHIP_UNIFORM_SUB_USAGE_XFB_BUFFER:
    case __GL_CHIP_UNIFORM_SUB_USAGE_VERTEXCOUNT_PER_INSTANCE:
        /* XFB uniform will be validated in gcChipValidateXFB, clear dirty to skip update here */
        uniform->dirty = gcvFALSE;
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_STARTVERTEX:
        if (*((GLint*)uniform->data) != gc->vertexArray.start)
        {
            *((GLint*)uniform->data) = gc->vertexArray.start;
            uniform->dirty = gcvTRUE;
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_RTHEIGHT:
        if (chipCtx->chipDirty.uDefer.sDefer.viewportScissor)
        {
            uniform->dirty = GL_TRUE;
        }
        if (uniform->dirty)
        {
            *(GLfloat*)uniform->data = (GLfloat)chipCtx->drawRTHeight;
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_RTWIDTH:
        if (chipCtx->chipDirty.uDefer.sDefer.viewportScissor)
        {
            uniform->dirty = GL_TRUE;
        }
        if (uniform->dirty)
        {
            *(GLfloat*)uniform->data = (GLfloat)chipCtx->drawRTWidth;
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_SAMPLEMASK_VALUE:
        if ((gc->globalDirtyState[__GL_DIRTY_ATTRS_2] & __GL_SAMPLE_MASK_BIT) != 0)
        {
            uniform->dirty = GL_TRUE;
        }
        if (uniform->dirty)
        {
            *(GLuint*)uniform->data = (GLuint)gc->state.multisample.sampleMaskValue;
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_COVERAGE_INV:
        if ((gc->globalDirtyState[__GL_DIRTY_ATTRS_2] & __GL_SAMPLECOVERAGE_BIT) != 0)
        {
            uniform->dirty = GL_TRUE;
        }
        if (uniform->dirty)
        {
            fValues[0] = gc->state.multisample.coverageValue;
            fValues[1] = gc->state.multisample.coverageInvert;
            gcoOS_MemCopy(uniform->data, fValues, 2 * gcmSIZEOF(GLfloat));
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_BASE_LEVEL_SIZE:
        /* LevelBaseSize contain x,y,z integer channels, x,y take width/height of base level
        ** z take depth for 3D texture, while layers for array texture
        */
        {
            GLuint unit = gcChipGetTexAuxUniformUnit(gc, program, uniform);
            __GLtextureObject *tex = gc->texture.units[unit].currentTexture;

            if (tex == gcvNULL)
            {
                fValues[0] = fValues[1] = fValues[2] = 1.0;
                iValues[0] = iValues[1] = iValues[2] = 1;
            }
            else
            {
                GLint baseLevel = tex->params.baseLevel;
                GLint layers = (tex->targetIndex == __GL_TEXTURE_3D_INDEX)
                           ? (tex->faceMipmap[0][baseLevel].depth)
                           : (tex->targetIndex == __GL_TEXTURE_CUBEMAP_ARRAY_INDEX)
                                ? ((tex->faceMipmap[0][baseLevel].arrays) / 6)
                                : (tex->faceMipmap[0][baseLevel].arrays);

                fValues[0] = *(GLfloat*)&(tex->faceMipmap[0][baseLevel].width);  /* width */
                fValues[1] = *(GLfloat*)&(tex->faceMipmap[0][baseLevel].height); /* height */
                fValues[2] = *(GLfloat*)&layers; /* depth */
                iValues[0] = *(GLint*)(&fValues[0]);
                iValues[1] = *(GLint*)(&fValues[1]);
                iValues[2] = *(GLint*)(&fValues[2]);
            }

            if (uniform->dataType == gcSHADER_INTEGER_X3)
            {
                pointer = iValues;
                size = gcmSIZEOF(GLfloat);
            }
            else
            {
                pointer = fValues;
                size = gcmSIZEOF(GLint);
            }

            if (!gcmIS_SUCCESS(gcoOS_MemCmp(uniform->data, pointer, 3 * size)))
            {
                gcoOS_MemCopy(uniform->data, pointer, 3 * size);
                uniform->dirty = gcvTRUE;
            }
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_LOD_MIN_MAX:
        /* MinMaxLod contain x,y,z floating channels, x, y take min/max lod for LOD clamping
        ** z is a flag to switch implicit LOD calculation in fragment shader.
        */
        {
            GLuint unit = gcChipGetTexAuxUniformUnit(gc, program, uniform);
            __GLtextureObject *tex = gc->texture.units[unit].currentTexture;
            __GLsamplerObject *samplerObj = gc->texture.units[unit].boundSampler;

            if (tex == gcvNULL)
            {
                fValues[0] = 0.0;
                fValues[1] = 1.0;
                fValues[2] = 0.0;
            }
            else
            {
                __GLsamplerParamState *samplerStates = samplerObj ? &samplerObj->params : &tex->params.sampler;

                fValues[0] = __GL_MAX(0, samplerStates->minLod); /* min lod */
                if (gc->texture.units[unit].boundSampler == gcvNULL)
                {
                    fValues[1] = samplerStates->maxLod;
                }
                else
                {
                    fValues[1] = __GL_MIN(samplerStates->maxLod,
                                          gc->texture.units[unit].maxLevelUsed); /* max lod */
                }
                fValues[2] = (samplerStates->minFilter == GL_NEAREST ||  samplerStates->minFilter == GL_LINEAR) /* mipmap */
                           ? 0.0f : 1.0f;
            }

            if (uniform->dataType == gcSHADER_INTEGER_X3)
            {
                iValues[0] = (GLint)fValues[0];
                iValues[1] = (GLint)fValues[1];
                iValues[2] = (GLint)fValues[2];
                pointer = iValues;
                size = gcmSIZEOF(GLfloat);
            }
            else
            {
                pointer = fValues;
                size = gcmSIZEOF(GLint);
            }

            if (!gcmIS_SUCCESS(gcoOS_MemCmp(uniform->data, pointer, 3 * size)))
            {
                gcoOS_MemCopy(uniform->data, pointer, 3 * size);
                uniform->dirty = gcvTRUE;
            }
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_IMAGE_SIZE:
        {
            GLuint unit = gcChipGetImageAuxUniformUnit(gc, program, uniform);
            __GLimageUnitState *imageState = &gc->state.image.imageUnit[unit];
            GLint *iValues = (GLint*)fValues;

            if (imageState->invalid)
            {
                gcoOS_ZeroMemory(iValues, 4 * gcmSIZEOF(GLint));
            }
            else
            {
                iValues[0] = (GLint)imageState->width;
                iValues[1] = (GLint)imageState->height;
                iValues[2] = (GLint)imageState->depth;

                if (imageState->type != __GL_IMAGE_2D && !imageState->singleLayered)
                {
                    gctINT32 sliceSlize;
                    __GLtextureObject *texObj = imageState->texObj;
                    gcsSURF_VIEW texView = gcChipGetTextureSurface(chipCtx, texObj, gcvTRUE, imageState->level, imageState->actualLayer);
                    gcmVERIFY_OK(gcoSURF_GetInfo(texView.surf, gcvSURF_INFO_SLICESIZE, &sliceSlize));
                    iValues[3] = sliceSlize;
                }
                else
                {
                    iValues[3] = 0;
                }
            }

            if (!gcmIS_SUCCESS(gcoOS_MemCmp(uniform->data, iValues, 4 * gcmSIZEOF(GLint))))
            {
                gcoOS_MemCopy(uniform->data, iValues, 4 * gcmSIZEOF(GLint));
                uniform->dirty = gcvTRUE;
            }
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_GROUPNUM:
        {
            if (gc->compute.indirect &&
                !chipCtx->chipFeature.hwFeature.hasComputeIndirect)
            {
                gcChipLockOutComputeIndirectBuf(gc);
            }

            fValues[0] = *(GLfloat*)&gc->compute.num_groups_x;
            fValues[1] = *(GLfloat*)&gc->compute.num_groups_y;
            fValues[2] = *(GLfloat*)&gc->compute.num_groups_z;

            if (!gcmIS_SUCCESS(gcoOS_MemCmp(uniform->data, fValues, 3 * gcmSIZEOF(GLfloat))))
            {
                gcoOS_MemCopy(uniform->data, fValues, 3 * gcmSIZEOF(GLfloat));
                uniform->dirty = gcvTRUE;
            }
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_MULTILAYER_SAMPLER:
    case __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_SAMPLER:
        uniform->dirty = GL_FALSE;
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_ADVANCED_BLEND_STATE:
        if (uniform->dirty)
        {
            GLuint *pData = (GLuint *)uniform->data;

            *pData++ = chipCtx->advBlendInShader;
            *pData++ = chipCtx->advBlendMode;
        }
        break;
    case __GL_CHIP_UNIFORM_SUB_USAGE_LTC:
    case __GL_CHIP_UNIFORM_SUB_USAGE_MULTILAYER_IMAGE:
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_SAMPLE_NUM:
        if (chipCtx->chipDirty.uBuffer.sBuffer.rtSurfDirty)
        {
            uniform->dirty = GL_TRUE;
        }
        if (uniform->dirty)
        {
            *(GLuint*)uniform->data = (chipCtx->drawRTSamples == 0) ? 1 : chipCtx->drawRTSamples;
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_TCS_PATCH_VERTICES_IN:
        if ((gc->globalDirtyState[__GL_PROGRAM_ATTRS] & __GL_DIRTY_GLSL_PATCH_VERTICES) != 0)
        {
            uniform->dirty = GL_TRUE;
        }
        if (uniform->dirty)
        {
            *(GLuint *)uniform->data = gc->shaderProgram.patchVertices;
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_TES_PATCH_VERTICES_IN:
        if (uniform->dirty)
        {
            *(GLuint *)uniform->data = progObj->bindingInfo.tessOutPatchSize;
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_SAMPLE_LOCATIONS:
        if (uniform->dirty)
        {
            GLuint i = (chipCtx->drawYInverted) ? 1: 0;
            __GL_MEMCOPY(uniform->data, &chipCtx->sampleLocations[i][0][0], 4 * 4 * sizeof(GLfloat));
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_MULTISAMPLE_BUFFERS:
        if (uniform->dirty)
        {
            GLuint i = (chipCtx->drawRTSamples > 1) ? 1 : 0;
            __GL_MEMCOPY(uniform->data, &i, sizeof(GLboolean));
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_SAMPLER:
        uniform->dirty = gcvFALSE;
        break;
    case __GL_CHIP_UNIFORM_SUB_USAGE_YINVERT:
        {

            GLfloat * pData = (GLfloat *)uniform->data;
            *pData = chipCtx->drawYInverted ? 1.0f : 0.0f;
            uniform->dirty = gcvTRUE;
        }break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_CONSTANT_COLOR:
        {
            GLfloat * pData = (GLfloat *)uniform->data;
            *pData++ = (GLfloat)gc->state.raster.blendColor.r;
            *pData++ = (GLfloat)gc->state.raster.blendColor.g;
            *pData++ = (GLfloat)gc->state.raster.blendColor.b;
            *pData++ = (GLfloat)gc->state.raster.blendColor.a;
            uniform->dirty = gcvTRUE;
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_EQUATION:
        {
            GLfloat * pData = (GLfloat *)uniform->data;
            *pData++ = (GLfloat)gc->state.raster.blendEquationRGB[0];
            *pData++ = (GLfloat)gc->state.raster.blendEquationAlpha[0];
            uniform->dirty = gcvTRUE;
        }
    break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_FUNCTION:
        {
            GLfloat * pData = (GLfloat *)uniform->data;
            *pData++ = (GLfloat)gc->state.raster.blendSrcRGB[0];
            *pData++ = (GLfloat)gc->state.raster.blendDstRGB[0];
            *pData++ = (GLfloat)gc->state.raster.blendSrcAlpha[0];
            *pData++ = (GLfloat)gc->state.raster.blendDstAlpha[0];
            uniform->dirty = gcvTRUE;
        }
    break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_BLEND_RT_WH:
        {
            GLfloat * pData = (GLfloat *)uniform->data;
            *pData++ = (GLfloat)chipCtx->drawRTWidth;
            *pData++ = (GLfloat)chipCtx->drawRTHeight;
        }
        break;

    case __GL_CHIP_UNIFORM_SUB_USAGE_DEPTH_BIAS:
        {
            GLfloat * pData = (GLfloat *)uniform->data;
            *pData++ = gc->state.polygon.factor;
            *pData++ = gc->state.polygon.units * 2.0f / 65535.0f;
            uniform->dirty = gcvTRUE;
        }
        break;
    case __GL_CHIP_UNIFORM_SUB_USAGE_RT_IMAGE:
        break;
    default:
        GL_ASSERT(0);
    }

    gcmFOOTER_NO();
}

__GL_INLINE gceSTATUS
gcChipFlushUserDefUniforms(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x progObj=0x%x program=0x%x", gc, chipCtx, progObj, program);

    if (chipCtx->chipDirty.uDefer.sDefer.activeUniform)
    {
        GLint i;

#if __GL_CHIP_PATCH_ENABLED
        gcChipPatchUpdateUniformData(gc, progObj, program);
#endif

        for (i = 0; i < program->userDefUniformCount; ++i)
        {
            __GLchipSLUniform *uniform = &program->uniforms[i];

            if (uniform->dirty)
            {
                gcmONERROR(gcChipFlushSingleUniform(gc, program, uniform, uniform->data));
            }
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipFlushBuiltinUniforms(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program
    )
{
    GLint i;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x progObj=0x%x program=0x%x", gc, chipCtx, progObj, program);

    /* Flush built-in uniforms */
    for (i = program->userDefUniformCount; i < program->activeUniformCount; ++i)
    {
        __GLchipSLUniform *uniform = &program->uniforms[i];

        switch (uniform->usage)
        {
        case __GL_CHIP_UNIFORM_USAGE_DEPTHRANGE_NEAR:
            if (chipCtx->chipDirty.uDefer.sDefer.depthRange)
            {
                uniform->dirty = GL_TRUE;
            }
            if (uniform->dirty)
            {
                *(GLfloat*)uniform->data = gc->state.depth.zNear;
            }
            break;
        case __GL_CHIP_UNIFORM_USAGE_DEPTHRANGE_FAR:
            if (chipCtx->chipDirty.uDefer.sDefer.depthRange)
            {
                uniform->dirty = GL_TRUE;
            }
            if (uniform->dirty)
            {
                *(GLfloat*)uniform->data = gc->state.depth.zFar;
            }
            break;
        case __GL_CHIP_UNIFORM_USAGE_DEPTHRANGE_DIFF:
            if (chipCtx->chipDirty.uDefer.sDefer.depthRange)
            {
                uniform->dirty = GL_TRUE;
            }
            if (uniform->dirty)
            {
                *(GLfloat*)uniform->data = gc->state.depth.zFar - gc->state.depth.zNear;
            }
            break;
        default:
            GL_ASSERT(0);
            break;
        }

        if (uniform->dirty)
        {
            gcmONERROR(gcChipFlushSingleUniform(gc, program, uniform, uniform->data));
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipFlushPrivateUniforms(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLchipSLProgramInstance* pgInstance
    )
{
    GLint i;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x progObj=0x%x program=0x%x pgInstance=0x%x",
                   gc, chipCtx, progObj, program, pgInstance);

#if TEMP_SHADER_PATCH
    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & __GL_DIRTY_GLSL_PROGRAM_SWITCH)
    {
        gcChipTmpPatchFlushUniforms(gc, program);
    }
#endif

    for (i = 0; i < pgInstance->privateUniformCount; i++)
    {
        __GLchipSLUniform *uniform = &pgInstance->privateUniforms[i];

        if (uniform->subUsage  != __GL_CHIP_UNIFORM_SUB_USAGE_LTC)
        {
            gcChipPreparePrivateUniform(gc, chipCtx, progObj, program, uniform);

            if (uniform->dirty)
            {
                gcmONERROR(gcChipFlushSingleUniform(gc, program, uniform, uniform->data));
            }
        }
    }

    /* Evaluation and flush for LTC uniforms must be the end, they depend on other uniform's data. */
    if (pgInstance->hasLTC)
    {
        gcmVERIFY_OK(gcChipLTCComputeLoadtimeConstant(gc, chipCtx, program));
    }

    for (i = 0; i < pgInstance->privateUniformCount; i++)
    {
        __GLchipSLUniform *uniform = &pgInstance->privateUniforms[i];

        if (uniform->subUsage == __GL_CHIP_UNIFORM_SUB_USAGE_LTC)
        {
            if (uniform->dirty)
            {
                gcmONERROR(gcChipFlushSingleUniform(gc, program, uniform, uniform->data));
            }
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipFlushUserDefUBs(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    __GLbitmask ubBindingDirty = gc->bufferObject.bindingDirties[__GL_UNIFORM_BUFFER_INDEX];
    GLint i;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x progObj=0x%x program=0x%x", gc, chipCtx, progObj, program);


    /* Flush user defined UBOs */
    for (i = 0; i < program->userDefUbCount; ++i)
    {
        gctSIZE_T requiredSize;
        __GLBufBindPoint *pBindingPoint;
        __GLchipVertexBufferInfo *bufInfo = gcvNULL;
        __GLchipSLUniformBlock *ub = &program->uniformBlocks[i];

        GL_ASSERT(ub->binding < gc->constants.shaderCaps.maxUniformBufferBindings);

        pBindingPoint = &gc->bufferObject.bindingPoints[__GL_UNIFORM_BUFFER_INDEX][ub->binding];

        /* No buffer object was not bound to it */
        if (!pBindingPoint->boundBufObj)
        {
            gcmTRACE(gcvLEVEL_ERROR,
                    "%s(%d): no buffer object was bound for UBO on #%d binding point",
                     __FUNCTION__, __LINE__, ub->binding);
            continue;
        }

        bufInfo = (__GLchipVertexBufferInfo*)(pBindingPoint->boundBufObj->privateData);

        /* If the buffer cannot intersect with the request binding range */
        if (!bufInfo->bufObj || bufInfo->size < (GLuint)pBindingPoint->bufOffset)
        {
            gcmTRACE(gcvLEVEL_ERROR,
                    "%s(%d): bound buffer object does not have storage.",
                     __FUNCTION__, __LINE__);
            continue;
        }

        requiredSize = (gctSIZE_T)pBindingPoint->bufSize;
        /* 0 indicates entire buffer */
        if (0 == requiredSize)
        {
            GL_ASSERT(0 == pBindingPoint->bufOffset);
            requiredSize = (gctSIZE_T)bufInfo->size;
        }

        if ((gctSIZE_T)bufInfo->size < (gctSIZE_T)pBindingPoint->bufOffset + requiredSize)
        {
            gcmTRACE(gcvLEVEL_ERROR,
                    "%s(%d): buffer object is not sufficiently large to back up the defined uniform block.",
                     __FUNCTION__, __LINE__);

            /* ES30 SPEC says: "If any active uniform block is not backed by a sufficiently
            ** large buffer object, the results of shader execution are undefined, and may
            ** result in GL interruption or termination."
            */
            gcmONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

#if gcdSYNC
        /* Mark defaultUBO will be used by this draw */
        gcmONERROR(gcoBUFOBJ_GetFence(bufInfo->bufObj, gcvFENCE_TYPE_READ));
#endif
        /* No need to flush for unchanged bindings if the ub wasn't mapped to reg */
        if (!(__glBitmaskTest(&ubBindingDirty, ub->binding)) && !(ub->mapFlag & gcdUB_MAPPED_TO_REG))
        {
            continue;
        }

#if __GL_CHIP_PATCH_ENABLED
        if (program->progFlags.CTSMaxUBOSize &&
            (ub->mapFlags[__GLSL_STAGE_FS] & gcdUB_MAPPED_TO_REG))
        {
            GLfloat *bufBase = gcvNULL;

            gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, gcvNULL, (gctPOINTER*)&bufBase));
            __GL_MEMCOPY(bufBase, bufBase + 4 * program->uboIndex, 4 * sizeof(GLfloat));
            gcmONERROR(gcoBUFOBJ_Unlock(bufInfo->bufObj));
        }
#endif

        gcmONERROR(gcChipFlushUniformBlock(gc, program, ub, bufInfo->bufObj, gcvNULL,
                                           pBindingPoint->bufOffset, requiredSize));

    }
OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipFlushDefaultUBs(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x progObj=0x%x program=0x%x", gc, chipCtx, progObj, program);

    if (chipCtx->chipDirty.uDefer.sDefer.activeUniform)
    {
        GLint i;

        /* Flush default UBOs */
        for (i = program->userDefUbCount; i < program->totalUbCount; ++i)
        {
            __GLchipSLUniformBlock *ub = &program->uniformBlocks[i];

            if (ub->halBufObj)
            {
                gcmONERROR(gcoBUFOBJ_Upload(ub->halBufObj, ub->bufBase, 0, ub->dataSize, gcvBUFOBJ_USAGE_STATIC_READ));
#if gcdSYNC
                /* Mark defaultUBO will be used by this draw */
                gcmONERROR(gcoBUFOBJ_GetFence(ub->halBufObj, gcvFENCE_TYPE_READ));
#endif
            }

            gcmONERROR(gcChipFlushUniformBlock(gc, program, ub, ub->halBufObj, gcvNULL, 0, ub->dataSize));
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

__GL_INLINE gceSTATUS
gcChipFlushPrivateUBs(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLchipSLProgramInstance* pgInstance
    )
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x progObj=0x%x program=0x%x pgInstance=0x%x",
                   gc, chipCtx, progObj, program, pgInstance);

    if (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & __GL_DIRTY_GLSL_PROGRAM_SWITCH)
    {
        GLint i;

        /* Flush private UBOs */
        for (i = 0; i < pgInstance->privateUbCount; ++i)
        {
            __GLchipSLUniformBlock *ub = &pgInstance->privateUBs[i];
            gcmONERROR(gcChipFlushUniformBlock(gc, program, ub, ub->halBufObj, gcvNULL, 0, ub->dataSize));
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipFlushAtomicCounterBuffers(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program
    )
{
    __GLbitmask acbBindingDirty = gc->bufferObject.bindingDirties[__GL_ATOMIC_COUNTER_BUFFER_INDEX];
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x progObj=0x%x program=0x%x", gc, chipCtx, progObj, program);

    if (!__glBitmaskIsAllZero(&acbBindingDirty))
    {
        GLuint i;

        /* Flush ACBO */
        for (i = 0; i < program->acbCount; ++i)
        {
            gctSIZE_T requiredSize;
            __GLBufBindPoint *pBindingPoint;
            __GLchipVertexBufferInfo *bufInfo = gcvNULL;
            __GLchipSLAtomCntBuf *acb = &program->acbs[i];
            gctUINT32 physical = 0;

            GL_ASSERT(acb->binding < (GLint)gc->constants.shaderCaps.maxAtomicCounterBufferBindings);

            /* No need to flush for unchanged bindings */
            if (!(__glBitmaskTest(&acbBindingDirty, acb->binding)))
            {
                continue;
            }

            pBindingPoint = &gc->bufferObject.bindingPoints[__GL_ATOMIC_COUNTER_BUFFER_INDEX][acb->binding];

            /* No buffer object was not bound to it */
            if (!pBindingPoint->boundBufObj)
            {
                gcmTRACE(gcvLEVEL_ERROR,
                        "%s(%d): no buffer object was bound for SSBO on #%d binding point",
                         __FUNCTION__, __LINE__, acb->binding);
                continue;
            }

            bufInfo = (__GLchipVertexBufferInfo*)(pBindingPoint->boundBufObj->privateData);

            /* If the buffer cannot intersect with the request binding range */
            if (!bufInfo->bufObj || bufInfo->size < (GLuint)pBindingPoint->bufOffset)
            {
                gcmTRACE(gcvLEVEL_ERROR,
                        "%s(%d): bound buffer object does not have storage.",
                         __FUNCTION__, __LINE__);
                continue;
            }

            requiredSize = (gctSIZE_T)pBindingPoint->bufSize;
            /* 0 indicates entire buffer */
            if (0 == requiredSize)
            {
                GL_ASSERT(0 == pBindingPoint->bufOffset);
                requiredSize = (gctSIZE_T)bufInfo->size;
            }

            if ((gctSIZE_T)bufInfo->size < (gctSIZE_T)pBindingPoint->bufOffset + requiredSize)
            {
                gcmTRACE(gcvLEVEL_ERROR,
                        "%s(%d): buffer object is not sufficiently large to back up SSBO.",
                         __FUNCTION__, __LINE__);
                gcmONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
            }

            gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, &physical, gcvNULL));

            do
            {
                __GLSLStage stageIdx;

                for (stageIdx = __GLSL_STAGE_VS; stageIdx < __GLSL_STAGE_LAST; ++stageIdx)
                {
                    if (acb->halUniform[stageIdx])
                    {
                        gctUINT32 physicalAddr;
                        gctINT32 index;
                        gctUINT32 data;

                        gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(chipCtx->activeProgState->hints->hwConstRegBases,
                                                                          acb->halUniform[stageIdx],
                                                                          &physicalAddr));
                        data = physical + (gctUINT32)pBindingPoint->bufOffset;
                        index = GetUniformPhysical(acb->halUniform[stageIdx]);

                        gcmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddr, index,
                                                         1, 1, 1, gcvFALSE, 1, 4,
                                                         &data, gcvFALSE, __glChipGLShaderStageToShaderKind[stageIdx]));

                        if (program->progFlags.robustEnabled)
                        {
                            gctUINT32 addressLimit[2];
                            gctSIZE_T size;
                            gctUINT32 bufSize;

                            gcoBUFOBJ_GetSize(bufInfo->bufObj, &size);
                            gcmSAFECASTSIZET(bufSize, size);

                            addressLimit[0] = physical;
                            addressLimit[1] = physical + bufSize - 1;

                            /*the base channel of the ubo must be x or y*/
                            gcmASSERT((physicalAddr & 0xF) == 0x0 || (physicalAddr & 0xF) == 0x4);

                            gcmONERROR(gcoSHADER_BindUniform(gcvNULL, physicalAddr + 4, index,
                                                             2, 1, 1, gcvFALSE, 0, 0,
                                                             addressLimit, gcvFALSE, __glChipGLShaderStageToShaderKind[stageIdx]));
                        }
                    }
                }
            } while (gcvFALSE);

            gcoBUFOBJ_Unlock(bufInfo->bufObj);
        }
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipFlushUserDefSSBs(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program
    )
{
    __GLbitmask ssbBindingDirty = gc->bufferObject.bindingDirties[__GL_SHADER_STORAGE_BUFFER_INDEX];
    gceSTATUS status = gcvSTATUS_OK;
    GLuint i;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x progObj=0x%x program=0x%x", gc, chipCtx, progObj, program);

    /* Flush SSBO */
    for (i = 0; i < program->userDefSsbCount; ++i)
    {
        gctSIZE_T requiredSize;
        __GLBufBindPoint *pBindingPoint;
        __GLchipVertexBufferInfo *bufInfo = gcvNULL;
        __GLchipSLStorageBlock *sb = &program->ssbs[i];
        gctUINT32 physical = 0;

        GL_ASSERT(sb->binding < gc->constants.shaderCaps.maxShaderStorageBufferBindings);

        pBindingPoint = &gc->bufferObject.bindingPoints[__GL_SHADER_STORAGE_BUFFER_INDEX][sb->binding];

        /* No buffer object was not bound to it */
        if (!pBindingPoint->boundBufObj)
        {
            gcmTRACE(gcvLEVEL_ERROR,
                    "%s(%d): no buffer object was bound for SSBO on #%d binding point",
                     __FUNCTION__, __LINE__, sb->binding);
            continue;
        }

        bufInfo = (__GLchipVertexBufferInfo*)(pBindingPoint->boundBufObj->privateData);

        /* If the buffer cannot intersect with the request binding range */
        if (!bufInfo->bufObj || bufInfo->size < (GLuint)pBindingPoint->bufOffset)
        {
            gcmTRACE(gcvLEVEL_ERROR,
                    "%s(%d): bound buffer object does not have storage.",
                     __FUNCTION__, __LINE__);
            continue;
        }

        requiredSize = (gctSIZE_T)pBindingPoint->bufSize;
        /* 0 indicates entire buffer */
        if (0 == requiredSize)
        {
            GL_ASSERT(0 == pBindingPoint->bufOffset);
            requiredSize = (gctSIZE_T)bufInfo->size;
        }

        if ((gctSIZE_T)bufInfo->size < (gctSIZE_T)pBindingPoint->bufOffset + requiredSize)
        {
            gcmTRACE(gcvLEVEL_ERROR,
                    "%s(%d): buffer object is not sufficiently large to back up SSBO.",
                     __FUNCTION__, __LINE__);
            gcmONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

#if gcdSYNC
        /* Mark SSBO will be used by this draw */
        gcmONERROR(gcoBUFOBJ_GetFence(bufInfo->bufObj, gcvFENCE_TYPE_ALL));
#endif

         /* No need to flush for unchanged bindings */
        if (!(__glBitmaskTest(&ssbBindingDirty, sb->binding)))
        {
            continue;
        }

        gcmONERROR(gcoBUFOBJ_Lock(bufInfo->bufObj, &physical, gcvNULL));

        do
        {
            __GLSLStage stageIdx;
            __GLchipSLProgramInstance* masterPgInstance = program->masterPgInstance;
            gcSHADER *pBinaries = masterPgInstance->binaries;

            for (stageIdx = __GLSL_STAGE_VS; stageIdx < __GLSL_STAGE_LAST; ++stageIdx)
            {
                gcUNIFORM sbUniform = sb->halUniform[stageIdx];
                gctUINT32 unsizedArrayLength = 0;
                gctINT32 data, index;

                if (!(sbUniform && isUniformUsedInShader(sbUniform)))
                {
                    /* Skip if not exist or used */
                    continue;
                }

                /* return the unsized array variable for StorageBlock
                 * if it's last block member the unsized array variable is returned
                 * otherwise NULL
                 */
                if (gcIsSBUnsized(sb->halSB[stageIdx]))
                {
                    gcmONERROR(gcGetSBUnsizedArrayLength(pBinaries[stageIdx],
                                                         sb->halSB[stageIdx],
                                                         (gctINT)requiredSize,
                                                         (gctINT *)&unsizedArrayLength));
                }

                if (gc->shaderProgram.boundPPO || chipCtx->chipDirty.uDefer.sDefer.pgInsChanged)
                {
                    gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(chipCtx->activeProgState->hints->hwConstRegBases,
                                                                      sbUniform,
                                                                      &sb->stateAddress[stageIdx]));
                }

                data = physical + (gctUINT32)pBindingPoint->bufOffset;
                index = GetUniformPhysical(sbUniform);

                gcmONERROR(gcoSHADER_BindUniform(gcvNULL, sb->stateAddress[stageIdx], index,
                                                 1, 1, 1, gcvFALSE, 1, 4,
                                                 &data, gcvFALSE, GetUniformShaderKind(sbUniform)));

                if (program->progFlags.robustEnabled)
                {
                    gctUINT32 addressLimit[3];
                    gctSIZE_T size;
                    gctUINT32 bufSize;

                    gcoBUFOBJ_GetSize(bufInfo->bufObj, &size);
                    gcmSAFECASTSIZET(bufSize, size);

                    addressLimit[0] = physical;               /* low limit */
                    addressLimit[1] = physical + bufSize - 1; /* upper limit */
                    addressLimit[2] = unsizedArrayLength;     /* size */

                    /*the base channel of the ubo must be x*/
                    gcmASSERT((sb->stateAddress[stageIdx] & 0xF) == 0x0);

                    gcmONERROR(gcoSHADER_BindUniform(gcvNULL, sb->stateAddress[stageIdx] + 4, index,
                                                     3, 1, 1, gcvFALSE, 0, 0,
                                                     addressLimit, gcvFALSE, GetUniformShaderKind(sbUniform)));
                }
                else
                {
                    if ((sb->stateAddress[stageIdx] & 0xF) == 0xC)
                    {
                        index += 1;
                    }

                    gcmONERROR(gcoSHADER_BindUniform(gcvNULL, sb->stateAddress[stageIdx] + 4, index,
                                                     1, 1, 1, gcvFALSE, 1, 4,
                                                     &unsizedArrayLength, gcvFALSE, GetUniformShaderKind(sbUniform)));
                }
            }

        } while (gcvFALSE);

        gcoBUFOBJ_Unlock(bufInfo->bufObj);
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipFlushPrivateSSBs(
    __GLcontext *gc,
    __GLchipContext *chipCtx,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLchipSLProgramInstance* pgInstance
    )
{
    GLuint i;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x chipCtx=0x%x progObj=0x%x program=0x%x pgInstance=0x%x",
                   gc, chipCtx, progObj, program, pgInstance);

    for (i = program->userDefSsbCount; i < program->totalSsbCount; ++i)
    {
        GLuint groups;
        gctSIZE_T dataSize = 0;
        gctUINT32 physical = 0;
        __GLchipSLStorageBlock *sb = &program->ssbs[i];
        GLboolean dirty = (gc->globalDirtyState[__GL_PROGRAM_ATTRS] & __GL_DIRTY_GLSL_PROGRAM_SWITCH) ? GL_TRUE: GL_FALSE;

        switch (sb->usage)
        {
        case __GL_CHIP_SSB_USAGE_SHAREDVAR:
            if (gc->compute.indirect)
            {
                gcmONERROR(gcChipLockOutComputeIndirectBuf(gc));
            }

            groups = gc->compute.num_groups_x * gc->compute.num_groups_y * gc->compute.num_groups_z;

            dataSize = groups * sb->dataSize;

            /* Resize sharedVar storage if group size changed */
            if (sb->groups != groups)
            {
                sb->groups = groups;
                gcmONERROR(gcoBUFOBJ_Upload(sb->halBufObj, gcvNULL, 0, dataSize, gcvBUFOBJ_USAGE_STATIC_READ));
                dirty = GL_TRUE;
            }
            break;

        case __GL_CHIP_SSB_USAGE_EXTRAREG:
            dataSize = sb->dataSize;
            break;

        default:
            GL_ASSERT(0);
            break;
        }

        if (!dirty)
        {
            continue;
        }

        GL_ASSERT(sb->halBufObj);
        gcmONERROR(gcoBUFOBJ_Lock(sb->halBufObj, &physical, gcvNULL));

        do
        {
            __GLSLStage stageIdx;

            for (stageIdx = __GLSL_STAGE_VS; stageIdx < __GLSL_STAGE_LAST; ++stageIdx)
            {
                gcUNIFORM sbUniform = sb->halUniform[stageIdx];
                gctINT32 index;

                if (!(sbUniform && isUniformUsedInShader(sbUniform)))
                {
                    /* Skip if not exist or used */
                    continue;
                }

                if (gc->shaderProgram.boundPPO || chipCtx->chipDirty.uDefer.sDefer.pgInsChanged)
                {
                    gcmONERROR(gcSHADER_ComputeUniformPhysicalAddress(chipCtx->activeProgState->hints->hwConstRegBases,
                                                                      sbUniform,
                                                                      &sb->stateAddress[stageIdx]));
                }

                index = GetUniformPhysical(sbUniform);

                gcmONERROR(gcoSHADER_BindUniform(gcvNULL, sb->stateAddress[stageIdx], index,
                                                 1, 1, 1, gcvFALSE, 1, 4,
                                                 &physical, gcvFALSE, GetUniformShaderKind(sbUniform)));

                if (program->progFlags.robustEnabled)
                {
                    gctUINT32 addressLimit[2];
                    gctSIZE_T size;
                    gctUINT32 bufSize;

                    gcoBUFOBJ_GetSize(sb->halBufObj, &size);
                    gcmSAFECASTSIZET(bufSize, size);

                    addressLimit[0] = physical;               /* low limit */
                    addressLimit[1] = physical + bufSize - 1; /* upper limit */

                    /*the base channel of the ubo must be x or y*/
                    gcmASSERT((sb->stateAddress[stageIdx] & 0xF) == 0x0 || (sb->stateAddress[stageIdx] & 0xF) == 0x4);

                    gcmONERROR(gcoSHADER_BindUniform(gcvNULL, sb->stateAddress[stageIdx] + 4, index,
                                                     2, 1, 1, gcvFALSE, 0, 0,
                                                     addressLimit, gcvFALSE, GetUniformShaderKind(sbUniform)));
                }


            }
        } while (gcvFALSE);

        gcoBUFOBJ_Unlock(sb->halBufObj);
    }

OnError:
    gcmFOOTER();
    return status;
}

gceSTATUS
gcChipFlushGLSLResourcesCB(
    __GLcontext *gc,
    __GLprogramObject *progObj,
    __GLchipSLProgram *program,
    __GLSLStage stage
    )
{
    __GLchipContext *chipCtx = CHIP_CTXINFO(gc);
    __GLchipSLProgramInstance* pgInstance;
    gceSTATUS status = gcvSTATUS_OK;

    gcmHEADER_ARG("gc=0x%x progObj=0x%x program=0x%x stage=%d",
                   gc, progObj, program, stage);

    GL_ASSERT(program);
    pgInstance = program->curPgInstance;

    /* Uniforms */
    gcmONERROR(gcChipFlushUserDefUniforms(gc, chipCtx, progObj, program));
    gcmONERROR(gcChipFlushBuiltinUniforms(gc, chipCtx, progObj, program));
    gcmONERROR(gcChipFlushPrivateUniforms(gc, chipCtx, progObj, program, pgInstance));

    /* Uniform Blocks */
    gcmONERROR(gcChipFlushUserDefUBs(gc, chipCtx, progObj, program));
    gcmONERROR(gcChipFlushDefaultUBs(gc, chipCtx, progObj, program));
    gcmONERROR(gcChipFlushPrivateUBs(gc, chipCtx, progObj, program, pgInstance));

    /* Atomic Counter Buffers */
    gcmONERROR(gcChipFlushAtomicCounterBuffers(gc, chipCtx, progObj, program));

    /* Shader Storage Blocks */
    gcmONERROR(gcChipFlushUserDefSSBs(gc, chipCtx, progObj, program));
    gcmONERROR(gcChipFlushPrivateSSBs(gc, chipCtx, progObj, program, pgInstance));

OnError:
    gcmFOOTER();
    return status;
}

/* Patch the program object with patch directive DynamicPatchInfo
 * if the DynamicPatchInfo is 0, it will reset the program to
 * original shader program before any patch.
 */
gceSTATUS
gcChipDynamicPatchProgram(
    __GLcontext * gc,
    __GLprogramObject* programObject,
    gcPatchDirective* recompilePatchDirectivePtr,
    gctUINT32 Options
    )
{
    __GLchipContext            *chipCtx          = CHIP_CTXINFO(gc);
    __GLchipSLProgram          *program          = (__GLchipSLProgram *)programObject->privateData;
    __GLchipSLProgramInstance  *masterPgInstance = program->masterPgInstance;
    __GLchipSLProgramInstance  *pgInstance       = program->curPgInstance;
    gcsPROGRAM_STATE            programState     = {0};
    gcPatchDirective           *dynamicPatchInfo = recompilePatchDirectivePtr;
    gceSHADER_FLAGS            flags;
    gceSHADER_SUB_FLAGS        subFlags;
    gceSTATUS                  status            = gcvSTATUS_OK;
    __GLSLStage                transformFeedbackStage = __GLSL_STAGE_VS;

    /* backup pre-link unpatched shaders*/
    __GLSLStage stage;
    gcSHADER_KIND shaderTypes[] =
    {
        gcSHADER_TYPE_VERTEX,
        gcSHADER_TYPE_TCS,
        gcSHADER_TYPE_TES,
        gcSHADER_TYPE_GEOMETRY,
        gcSHADER_TYPE_FRAGMENT,
        gcSHADER_TYPE_COMPUTE,
    };

    gcmHEADER_ARG("gc=0x%x programObject=0x%x recompilePatchDirectivePtr=0x%x kind=%d",
                   gc, programObject, recompilePatchDirectivePtr,
                   (recompilePatchDirectivePtr ? recompilePatchDirectivePtr->kind : 0));

    gcSetGLSLCompiler(chipCtx->pfCompile);
    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        GL_ASSERT(pgInstance->binaries[stage] == gcvNULL);
        if (masterPgInstance->binaries[stage])
        {
            gcmONERROR(gcSHADER_Construct(shaderTypes[stage], &pgInstance->binaries[stage]));
            gcmONERROR(gcSHADER_Copy(pgInstance->binaries[stage], masterPgInstance->binaries[stage]));
            if (stage == __GLSL_STAGE_FS &&
                masterPgInstance->binaries[stage]->isDual16Shader)
            {
                pgInstance->binaries[stage]->isMasterDual16Shader = gcvTRUE;
            }
            if (dynamicPatchInfo)
            {
                gcmONERROR(gcSHADER_DynamicPatch(pgInstance->binaries[stage], dynamicPatchInfo, 0));
            }
        }
    }

    if (programObject->xfbVaryingNum)
    {
        gceFEEDBACK_BUFFER_MODE xfbMode = (programObject->xfbMode == GL_SEPARATE_ATTRIBS)
                                        ? gcvFEEDBACK_SEPARATE
                                        : gcvFEEDBACK_INTERLEAVED;

        transformFeedbackStage = gcChipGetLastNonFragStage(program);

        /* Right now we only support VS and TES. */
        gcmASSERT(transformFeedbackStage == __GLSL_STAGE_VS ||
                  transformFeedbackStage == __GLSL_STAGE_TES ||
                  transformFeedbackStage == __GLSL_STAGE_GS);

        gcmONERROR(gcSHADER_SetTransformFeedbackVarying(pgInstance->binaries[transformFeedbackStage],
                                                        (gctSIZE_T)programObject->xfbVaryingNum,
                                                        (gctCONST_STRING*)programObject->ppXfbVaryingNames,
                                                        xfbMode));
    }

    /* link the shader and generate states for the patched shaders */
    flags = (gcvSHADER_DEAD_CODE            |
             gcvSHADER_RESOURCE_USAGE       |
             gcvSHADER_OPTIMIZER            |
             gcvSHADER_USE_GL_Z             |
             gcvSHADER_USE_GL_POINT_COORD   |
             gcvSHADER_USE_GL_POSITION      |
             gcvSHADER_REMOVE_UNUSED_UNIFORMS |
             gcvSHADER_FLUSH_DENORM_TO_ZERO);


#if gcdALPHA_KILL_IN_SHADER && __GL_CHIP_PATCH_ENABLED
    if (program->progFlags.alphaKill)
    {
        flags |= gcvSHADER_USE_ALPHA_KILL;
    }
#endif

    flags |= gcvSHADER_RECOMPILER;

    if (programObject->bindingInfo.isSeparable)
    {
        flags |= gcvSHADER_SEPERATED_PROGRAM;
    }

    if (program->progFlags.disableDual16)
    {
        flags |= gcvSHADER_DISABLE_DUAL16;
    }

    if (program->progFlags.VIRCGNone)
    {
        flags |= gcvSHADER_VIRCG_NONE;
    }

    if (program->progFlags.VIRCGOne)
    {
        flags |= gcvSHADER_VIRCG_ONE;
    }

    if (program->progFlags.disableInline)
    {
        flags |= gcvSHADER_SET_INLINE_LEVEL_0;
    }

    if (program->progFlags.deqpMinCompTime)
    {
        flags |= gcvSHADER_MIN_COMP_TIME;
    }

    /* determine the dual16 hp rule based on benchmark and shader detection */
    subFlags.dual16PrecisionRule = gcChipDetermineDual16PrecisionRule(chipCtx, program);

    if (Options & __GL_CHIP_RO_OUTPUT_HIGHP_CONVERSION)
    {
        subFlags.dual16PrecisionRule |= Dual16_PrecisionRule_OUTPUT_HP;
    }

    if (program->progFlags.robustEnabled)
    {
        flags |= gcvSHADER_NEED_ROBUSTNESS_CHECK;
    }

    /* Call the HAL backend linker. */
    status = gcLinkProgram(__GLSL_STAGE_LAST,
                           pgInstance->binaries,
                           flags,
                           &subFlags,
                           &programState);

    if (gcmIS_ERROR(status))
    {
        if (programState.stateBuffer != gcvNULL)
        {
            gcmVERIFY_OK(gcmOS_SAFE_FREE(gcvNULL, programState.stateBuffer));
        }
        programObject->programInfo.invalidFlags |= __GL_INVALID_LINK_BIT;
        gcmFATAL("%s(%d): gcLinkProgram failed status=%d", __FUNCTION__, __LINE__, status);
    }
    else
    {
        programObject->programInfo.invalidFlags &= ~__GL_INVALID_LINK_BIT;

        /* Successful link must guarantee there is state buffer generated. */
        GL_ASSERT(programState.stateBuffer && programState.stateBufferSize > 0);

        pgInstance->programState = programState;

        gcChipPgInstanceCleanBindingInfo(gc, pgInstance);

        /* Rebuilding uniform tables for the newly created uniform and samplers */
        gcmONERROR(gcChipProgramBindingRecompiledInfo(gc, programObject));

        if (masterPgInstance->programState.hints->unifiedStatus.samplerCount > pgInstance->programState.hints->unifiedStatus.samplerCount)
        {
            pgInstance->programState.hints->unifiedStatus.samplerCount =
            gcmMAX(pgInstance->programState.hints->unifiedStatus.samplerCount, masterPgInstance->programState.hints->unifiedStatus.samplerCount);

            __GLES_PRINT("ES30:The sampelrCount of current program should be maximun between current program and master program after recompile");
        }


    }

OnError:
    gcmFOOTER();
    return status;
}

GLuint
__glChipGetProgramResourceIndex(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLenum progInterface,
    const GLchar *name
    )
{
    GLuint i;
    gctSIZE_T nameLen = 0;
    gctSIZE_T arrayIdx = 0;
    GLboolean isArray = GL_FALSE;
    GLuint index = GL_INVALID_INDEX;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x progInterface=%d name=%s",
                   gc, programObject, progInterface, name);

    switch (progInterface)
    {
    case GL_PROGRAM_INPUT:
        /* Index can be queried by either "arrayName" or "arrayName[0]" for array types */
        if (gcChipUtilParseUniformName(name, &nameLen, &arrayIdx, &isArray) && arrayIdx == 0)
        {
            for (i = 0; i < program->inCount; ++i)
            {
                if (nameLen == (gctSIZE_T)program->inputs[i].nameLen &&
                    (!isArray || program->inputs[i].isArray) &&
                    gcmIS_SUCCESS(gcoOS_MemCmp(program->inputs[i].name, name, nameLen))
                   )
                {
                    index = i;
                    break;
                }
            }
        }
        break;

    case GL_PROGRAM_OUTPUT:
        /* Index can be queried by either "arrayName" or "arrayName[0]" for array types */
        if (gcChipUtilParseUniformName(name, &nameLen, &arrayIdx, &isArray) && arrayIdx == 0)
        {
            for (i = 0; i < program->outCount; ++i)
            {
                if (nameLen == (gctSIZE_T)program->outputs[i].nameLen &&
                    (!isArray || program->outputs[i].isArray) &&
                    gcmIS_SUCCESS(gcoOS_MemCmp(program->outputs[i].name, name, nameLen))
                   )
                {
                    index = i;
                    break;
                }
            }
        }
        break;

    case GL_TRANSFORM_FEEDBACK_VARYING:
        if (gcChipUtilParseUniformName(name, &nameLen, &arrayIdx, &isArray))
        {
            gctSIZE_T fullNameLen = gcoOS_StrLen(name, gcvNULL);

            for (i = 0; i < program->xfbCount; ++i)
            {
                __GLchipSLXfbVarying *xfbVarying = &program->xfbVaryings[i];

                /*
                ** If this xfbVarying is an array, then the name of this xfbVarying is "arrayName[0]",
                ** and both "arrayName" and "arrayName[0]" should match this xfbVarying.
                */
                if (xfbVarying->isArray)
                {
                    if (fullNameLen == (gctSIZE_T)xfbVarying->nameLen &&
                        gcmIS_SUCCESS(gcoOS_StrNCmp(xfbVarying->name, name, fullNameLen)))
                    {
                        index = i;
                        break;
                    }

                    else if ((xfbVarying->nameLen == fullNameLen + 3) &&
                             gcmIS_SUCCESS(gcoOS_StrNCmp(xfbVarying->name, name, fullNameLen)))
                    {
                        index = i;
                        break;
                    }
                }
                else
                {
                    /* Driver kept XFB varying name might have array index appended */
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(xfbVarying->name, name)))
                    {
                        index = i;
                        break;
                    }

                    /* Index can be queried by either "arrayName" or "arrayName[0]" for array types */
                    if (xfbVarying->nameLen == fullNameLen + 3 &&
                        gcmIS_SUCCESS(gcoOS_MemCmp(xfbVarying->name, name, fullNameLen)) &&
                        gcmIS_SUCCESS(gcoOS_StrNCmp(xfbVarying->name + fullNameLen, "[0]", 3)))
                    {
                        index = i;
                        break;
                    }
                }
            }
        }
        break;

    case GL_BUFFER_VARIABLE:
        /* Index can be queried by either "arrayName" or "arrayName[0]" for array types */
        if (gcChipUtilParseUniformName(name, &nameLen, &arrayIdx, &isArray))
        {
            for (i = 0; i < program->bufVariableCount; ++i)
            {
                if (nameLen == (gctSIZE_T)program->bufVariables[i].nameLen &&
                    (!isArray || program->bufVariables[i].isArray) &&
                    gcmIS_SUCCESS(gcoOS_MemCmp(program->bufVariables[i].name, name, nameLen))
                   )
                {
                    index = i;
                    break;
                }
            }

            /* If array name cannot find, try it as not index array name */
            if (isArray && index == GL_INVALID_INDEX)
            {
                for (i = 0; i < program->bufVariableCount; ++i)
                {
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(program->bufVariables[i].name, name)))
                    {
                        index = i;
                        break;
                    }
                }
            }
        }

        break;

    case GL_SHADER_STORAGE_BLOCK:
        for (i = 0; i < program->userDefSsbCount; ++i)
        {
            if (gcmIS_SUCCESS(gcoOS_StrCmp(program->ssbs[i].name, name)))
            {
                index = i;
                break;
            }
        }

        /* If failed to find and name didn't include array index, append "[0]" and retry */
        if (index == GL_INVALID_INDEX)
        {
            gctSTRING arrayName;
            gctSIZE_T nameLen = gcoOS_StrLen(name, gcvNULL);

            if (name[nameLen - 1] != ']')
            {
                gcoOS_Allocate(gcvNULL, nameLen + 4, (gctPOINTER*)&arrayName);
                gcoOS_StrCopySafe(arrayName, nameLen + 4, name);
                gcoOS_StrCatSafe(arrayName, nameLen + 4, "[0]");

                for (i = 0; i < program->userDefSsbCount; ++i)
                {
                    if (gcmIS_SUCCESS(gcoOS_StrCmp(program->ssbs[i].name, arrayName)))
                    {
                        index = i;
                        break;
                    }
                }

                gcmOS_SAFE_FREE(gcvNULL, arrayName);
            }
        }
        break;

    default:
        GL_ASSERT(0);
        break;
    }

    gcmFOOTER_ARG("return=%u", index);
    return index;
}

GLvoid
__glChipGetProgramResourceName(
    __GLcontext *gc,
    __GLprogramObject *programObject,
    GLenum progInterface,
    GLuint index,
    GLsizei bufSize,
    GLsizei *length,
    GLchar *name
    )
{
    GLsizei nameLen = 0;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x progInterface=%d index=%d "
                  "length=0x%x params=0x%x name=0x%x",
                   gc, programObject, progInterface, index, bufSize, length, name);

    /* According to spec, the returned string should be null-terminated,
    ** but "length" returned should exclude null-terminator.
    */

    switch (progInterface)
    {
    case GL_PROGRAM_INPUT:
        if (index < program->inCount)
        {
            if (name && bufSize > 0)
            {
                __GLchipSLInput *input = &program->inputs[index];

                nameLen = __GL_MIN((GLsizei)input->nameLen, bufSize - 1);
                if (nameLen > 0)
                {
                    gcoOS_MemCopy(name, input->name, nameLen);
                }
                name[nameLen] = '\0';

                if (input->isArray)
                {
                    gcoOS_StrCatSafe(name, bufSize, "[0]");
                    nameLen = __GL_MIN(nameLen + 3, bufSize - 1);
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_PROGRAM_OUTPUT:
        if (index < program->outCount)
        {
            if (name && bufSize > 0)
            {
                __GLchipSLOutput *output = &program->outputs[index];

                nameLen = __GL_MIN((GLsizei)output->nameLen, bufSize - 1);
                if (nameLen > 0)
                {
                    gcoOS_MemCopy(name, output->name, nameLen);
                }
                name[nameLen] = '\0';

                if (output->isArray)
                {
                    gcoOS_StrCatSafe(name, bufSize, "[0]");
                    nameLen = __GL_MIN(nameLen + 3, bufSize - 1);
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_BUFFER_VARIABLE:
        if (index < program->bufVariableCount)
        {
            if (name && bufSize > 0)
            {
                __GLchipSLBufVariable *bufVariable = &program->bufVariables[index];

                nameLen = __GL_MIN((GLsizei)bufVariable->nameLen, bufSize - 1);
                if (nameLen > 0)
                {
                    gcoOS_MemCopy(name, bufVariable->name, nameLen);
                }
                name[nameLen] = '\0';

                if (bufVariable->isArray)
                {
                    gcoOS_StrCatSafe(name, bufSize, "[0]");
                    nameLen = __GL_MIN(nameLen + 3, bufSize - 1);
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_SHADER_STORAGE_BLOCK:
        if (index < program->userDefSsbCount)
        {
            if (name && bufSize > 0)
            {
                __GLchipSLStorageBlock *ssb = &program->ssbs[index];

                nameLen = __GL_MIN((GLsizei)ssb->nameLen, bufSize - 1);
                if (nameLen > 0)
                {
                    gcoOS_MemCopy(name, ssb->name, nameLen);
                }
                name[nameLen] = '\0';
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    default:
        GL_ASSERT(0);
        break;
    }

    if (length)
    {
        *length = nameLen;
    }

OnError:
    gcmFOOTER_NO();
}

#define __GL_SAFE_ASSIGN(addr, boundary, value) \
    if ((addr) < (boundary)) *(addr)++ = (value);

GLvoid
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
    )
{
    GLsizei i;
    GLint *baseAddr = params;
    GLint *maxAddr = params + bufSize;
    __GLchipSLProgram *program = (__GLchipSLProgram *)programObject->privateData;

    gcmHEADER_ARG("gc=0x%x programObject=0x%x progInterface=%d index=%d "
                  "propCount=%d props=0x%x bufSize=%d length=0x%x params=0x%x",
                   gc, programObject, progInterface, index, propCount, props,
                   bufSize, length, params);

    switch (progInterface)
    {
    case GL_UNIFORM:
        if (index < (GLuint)program->activeUniformCount)
        {
            __GLchipSLUniform *uniform = &program->uniforms[index];

            for (i = 0; i < propCount; ++i)
            {
                switch (props[i])
                {
                case GL_LOCATION:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->location);
                    break;
                case GL_TYPE:
                    /* make sure the table was not broken due to definition change */
                    GL_ASSERT(g_typeInfos[uniform->dataType].halType == uniform->dataType);
                    __GL_SAFE_ASSIGN(params, maxAddr, g_typeInfos[uniform->dataType].glType);
                    break;
                case GL_ARRAY_SIZE:
                    __GL_SAFE_ASSIGN(params, maxAddr, (GLint)uniform->arraySize);
                    break;
                case GL_NAME_LENGTH:
                    __GL_SAFE_ASSIGN(params, maxAddr, (GLint)uniform->nameLen + (uniform->isArray ? 4 : 1));
                    break;
                case GL_BLOCK_INDEX:
                    __GL_SAFE_ASSIGN(params, maxAddr, (uniform->ubIndex < program->userDefUbCount) ? uniform->ubIndex : -1);
                    break;
                case GL_OFFSET:
                    __GL_SAFE_ASSIGN(params, maxAddr, (uniform->ubIndex < program->userDefUbCount) ? uniform->offset : -1);
                    break;
                case GL_ARRAY_STRIDE:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->arrayStride);
                    break;
                case GL_MATRIX_STRIDE:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->matrixStride);
                    break;
                case GL_IS_ROW_MAJOR:
                    __GL_SAFE_ASSIGN(params, maxAddr, (GLint)uniform->isRowMajor);
                    break;
                case GL_REFERENCED_BY_VERTEX_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->halUniform[__GLSL_STAGE_VS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_FRAGMENT_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->halUniform[__GLSL_STAGE_FS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_COMPUTE_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->halUniform[__GLSL_STAGE_CS] ? 1 : 0);
                    break;
                case GL_ATOMIC_COUNTER_BUFFER_INDEX:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->category == gceTK_ATOMIC ? uniform->acbIndex: -1);
                    break;
                case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->halUniform[__GLSL_STAGE_TCS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->halUniform[__GLSL_STAGE_TES] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, uniform->halUniform[__GLSL_STAGE_GS] ? 1 : 0);
                    break;
                default:
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_UNIFORM_BLOCK:
        if (index < (GLuint)program->userDefUbCount)
        {
            gctSIZE_T j;
            __GLchipSLUniformBlock *ub = &program->uniformBlocks[index];

            for (i = 0; i < propCount; ++i)
            {
                switch (props[i])
                {
                case GL_NAME_LENGTH:
                    __GL_SAFE_ASSIGN(params, maxAddr, (GLint)ub->nameLen + 1);
                    break;
                case GL_ACTIVE_VARIABLES:
                    for (j = 0; j < ub->activeUniforms; ++j)
                    {
                        __GL_SAFE_ASSIGN(params, maxAddr, ub->uniformIndices[j]);
                    }
                    break;
                case GL_BUFFER_BINDING:
                    __GL_SAFE_ASSIGN(params, maxAddr, ub->binding);
                    break;
                case GL_NUM_ACTIVE_VARIABLES:
                    __GL_SAFE_ASSIGN(params, maxAddr, (GLint)ub->activeUniforms);
                    break;
                case GL_BUFFER_DATA_SIZE:
                    __GL_SAFE_ASSIGN(params, maxAddr, (GLint)ub->dataSize);
                    break;
                case GL_REFERENCED_BY_VERTEX_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, ub->refByStage[__GLSL_STAGE_VS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_FRAGMENT_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, ub->refByStage[__GLSL_STAGE_FS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_COMPUTE_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, ub->refByStage[__GLSL_STAGE_CS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, ub->refByStage[__GLSL_STAGE_TCS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, ub->refByStage[__GLSL_STAGE_TES] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, ub->refByStage[__GLSL_STAGE_GS] ? 1 : 0);
                    break;
                default:
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_PROGRAM_INPUT:
        if (index < program->inCount)
        {
            __GLchipSLInput *input = &program->inputs[index];

            for (i = 0; i < propCount; ++i)
            {
                switch (props[i])
                {
                case GL_LOCATION:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->location);
                    break;
                case GL_TYPE:
                    __GL_SAFE_ASSIGN(params, maxAddr, g_typeInfos[input->type].glType);
                    break;
                case GL_ARRAY_SIZE:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->arraySize);
                    break;
                case GL_NAME_LENGTH:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->nameLen + (input->isArray ? 4 : 1));
                    break;
                case GL_REFERENCED_BY_VERTEX_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->refByStage[__GLSL_STAGE_VS]);
                    break;
                case GL_REFERENCED_BY_FRAGMENT_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->refByStage[__GLSL_STAGE_FS]);
                    break;
                case GL_REFERENCED_BY_COMPUTE_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->refByStage[__GLSL_STAGE_CS]);
                    break;
                case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->refByStage[__GLSL_STAGE_TCS]);
                    break;
                case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->refByStage[__GLSL_STAGE_TES]);
                    break;
                case GL_IS_PER_PATCH_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->isPerPatch);
                    break;
                case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, input->refByStage[__GLSL_STAGE_GS]);
                    break;
                default:
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                    break;
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;
        break;

    case GL_PROGRAM_OUTPUT:
        if (index < program->outCount)
        {
            __GLchipSLOutput *output = &program->outputs[index];

            for (i = 0; i < propCount; ++i)
            {
                switch (props[i])
                {
                case GL_LOCATION:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->location - output->startIndex);
                    break;
                case GL_TYPE:
                    __GL_SAFE_ASSIGN(params, maxAddr, g_typeInfos[output->type].glType);
                    break;
                case GL_ARRAY_SIZE:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->arraySize);
                    break;
                case GL_NAME_LENGTH:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->nameLen + (output->isArray ? 4 : 1));
                    break;
                case GL_REFERENCED_BY_VERTEX_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->refByStage[__GLSL_STAGE_VS]);
                    break;
                case GL_REFERENCED_BY_FRAGMENT_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->refByStage[__GLSL_STAGE_FS]);
                    break;
                case GL_REFERENCED_BY_COMPUTE_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->refByStage[__GLSL_STAGE_CS]);
                    break;
                case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->refByStage[__GLSL_STAGE_TCS]);
                    break;
                case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->refByStage[__GLSL_STAGE_TES]);
                    break;
                case GL_IS_PER_PATCH_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->isPerPatch);
                    break;
                case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, output->refByStage[__GLSL_STAGE_GS]);
                    break;

                default:
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                    break;
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_TRANSFORM_FEEDBACK_VARYING:
        if (index < program->xfbCount)
        {
            __GLchipSLXfbVarying *xfb = &program->xfbVaryings[index];

            for (i = 0; i < propCount; ++i)
            {
                switch (props[i])
                {
                case GL_TYPE:
                    __GL_SAFE_ASSIGN(params, maxAddr, g_typeInfos[xfb->type].glType);
                    break;
                case GL_ARRAY_SIZE:
                    __GL_SAFE_ASSIGN(params, maxAddr, xfb->arraySize);
                    break;
                case GL_NAME_LENGTH:
                    __GL_SAFE_ASSIGN(params, maxAddr, xfb->nameLen + 1);
                    break;
                default:
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                    break;
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_ATOMIC_COUNTER_BUFFER:
        if (index < program->acbCount)
        {
            GLint j;
            __GLchipSLAtomCntBuf* acb = &program->acbs[index];

            for (i = 0; i < propCount; ++i)
            {
                switch (props[i])
                {
                case GL_ACTIVE_VARIABLES:
                    for (j = 0; j < acb->activeACs; ++j)
                    {
                        __GL_SAFE_ASSIGN(params, maxAddr, acb->uniformIndices[j]);
                    }
                    break;
                case GL_BUFFER_BINDING:
                    __GL_SAFE_ASSIGN(params, maxAddr, acb->binding);
                    break;
                case GL_NUM_ACTIVE_VARIABLES:
                    __GL_SAFE_ASSIGN(params, maxAddr, acb->activeACs);
                    break;
                case GL_BUFFER_DATA_SIZE:
                    __GL_SAFE_ASSIGN(params, maxAddr, (GLint)acb->dataSize);
                    break;
                case GL_REFERENCED_BY_VERTEX_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, acb->halUniform[__GLSL_STAGE_VS] ? GL_TRUE : GL_FALSE);
                    break;
                case GL_REFERENCED_BY_FRAGMENT_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, acb->halUniform[__GLSL_STAGE_FS] ? GL_TRUE : GL_FALSE);
                    break;
                case GL_REFERENCED_BY_COMPUTE_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, acb->halUniform[__GLSL_STAGE_CS] ? GL_TRUE : GL_FALSE);
                    break;
                case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, acb->halUniform[__GLSL_STAGE_TCS] ? GL_TRUE : GL_FALSE);
                    break;
                case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, acb->halUniform[__GLSL_STAGE_TES] ? GL_TRUE : GL_FALSE);
                    break;
                case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, acb->halUniform[__GLSL_STAGE_GS] ? GL_TRUE : GL_FALSE);
                    break;
                default:
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                    break;
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_BUFFER_VARIABLE:
        if (index < program->bufVariableCount)
        {
            __GLchipSLBufVariable* bufVariable = &program->bufVariables[index];

            for (i = 0; i < propCount; ++i)
            {
                switch (props[i])
                {
                case GL_TYPE:
                    /* make sure the table was not broken due to definition change */
                    GL_ASSERT(g_typeInfos[bufVariable->dataType].halType == bufVariable->dataType);
                    __GL_SAFE_ASSIGN(params, maxAddr, g_typeInfos[bufVariable->dataType].glType);
                    break;
                case GL_ARRAY_SIZE:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->arraySize);
                    break;
                case GL_NAME_LENGTH:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->nameLen + (bufVariable->isArray ? 4 : 1));
                    break;
                case GL_BLOCK_INDEX:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->ssbIndex);
                    break;
                case GL_OFFSET:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->offset);
                    break;
                case GL_ARRAY_STRIDE:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->arrayStride);
                    break;
                case GL_MATRIX_STRIDE:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->matrixStride);
                    break;
                case GL_IS_ROW_MAJOR:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->isRowMajor);
                    break;
                case GL_REFERENCED_BY_VERTEX_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->refByStage[__GLSL_STAGE_VS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_FRAGMENT_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->refByStage[__GLSL_STAGE_FS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_COMPUTE_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->refByStage[__GLSL_STAGE_CS] ? 1 : 0);
                    break;
                case GL_TOP_LEVEL_ARRAY_SIZE:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->topLevelArraySize);
                    break;
                case GL_TOP_LEVEL_ARRAY_STRIDE:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->topLevelArrayStride);
                    break;
                case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->refByStage[__GLSL_STAGE_TCS] ? GL_TRUE : GL_FALSE);
                    break;
                case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->refByStage[__GLSL_STAGE_TES] ? GL_TRUE : GL_FALSE);
                    break;
                case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, bufVariable->refByStage[__GLSL_STAGE_GS] ? GL_TRUE : GL_FALSE);
                    break;
                default:
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    case GL_SHADER_STORAGE_BLOCK:
        if (index < program->userDefSsbCount)
        {
            gctSIZE_T j;
            __GLchipSLStorageBlock *ssb = &program->ssbs[index];

            for (i = 0; i < propCount; ++i)
            {
                switch (props[i])
                {
                case GL_NAME_LENGTH:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->nameLen + 1);
                    break;
                case GL_ACTIVE_VARIABLES:
                    for (j = 0; j < ssb->activeBVs; ++j)
                    {
                        __GL_SAFE_ASSIGN(params, maxAddr, ssb->bvInfos[j].index);
                    }
                    break;
                case GL_BUFFER_BINDING:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->binding);
                    break;
                case GL_NUM_ACTIVE_VARIABLES:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->activeBVs);
                    break;
                case GL_BUFFER_DATA_SIZE:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->dataSize);
                    break;
                case GL_REFERENCED_BY_VERTEX_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->refByStage[__GLSL_STAGE_VS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_FRAGMENT_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->refByStage[__GLSL_STAGE_FS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_COMPUTE_SHADER:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->refByStage[__GLSL_STAGE_CS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_TESS_CONTROL_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->refByStage[__GLSL_STAGE_TCS] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_TESS_EVALUATION_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->refByStage[__GLSL_STAGE_TES] ? 1 : 0);
                    break;
                case GL_REFERENCED_BY_GEOMETRY_SHADER_EXT:
                    __GL_SAFE_ASSIGN(params, maxAddr, ssb->refByStage[__GLSL_STAGE_GS] ? 1 : 0);
                    break;
                default:
                    __GL_ERROR_EXIT(GL_INVALID_OPERATION);
                }
            }
        }
        else
        {
            __GL_ERROR_EXIT(GL_INVALID_VALUE);
        }
        break;

    default:
        __GL_ERROR_EXIT(GL_INVALID_ENUM);
    }

    if (length)
    {
        *length = (GLsizei)(params - baseAddr);
    }

OnError:
    gcmFOOTER_NO();
}

GLboolean
__glChipValidateProgramPipeline(
    __GLcontext *gc,
    __GLprogramPipelineObject *ppObj,
    GLboolean callFromDraw
    )
{
    __GLSLStage stage;
    GLboolean valid = GL_FALSE;
    GLboolean skipped = GL_FALSE;
    GLboolean progInstalled = GL_FALSE;
    __GLprogramObject *progObj;
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL gpipeInstalled = gcvFALSE, psInstalled = gcvFALSE;

    gcmHEADER_ARG("gc=0x%x ppObj=0x%x callFromDraw=%d", gc, ppObj, callFromDraw);

    for (stage = __GLSL_STAGE_VS; stage < __GLSL_STAGE_LAST; ++stage)
    {
        progObj = ppObj->stageProgs[stage];
        if (progObj)
        {
            __GLchipSLProgram *program;
            gcePROGRAM_STAGE_BIT progStageBit;
            gcePROGRAM_STAGE progStage = gcvPROGRAM_STAGE_VERTEX;

            /* The installed program object on pipeline must be separable */
            if (!progObj->bindingInfo.isSeparable)
            {
                goto OnError;
            }

            program = (__GLchipSLProgram*)progObj->privateData;
            progStageBit = program->stageBits;

            /* The installed program object must be fully installed */
            while (progStageBit)
            {
                if (progStageBit & (1 << progStage))
                {
                    if (ppObj->stageProgs[__glChipHALShaderStageToGL[progStage]] != progObj)
                    {
                        goto OnError;
                    }
                    progStageBit &= ~(1 << progStage);
                }
                progStage++;
            }

            progInstalled = GL_TRUE;

            if (stage < __GLSL_STAGE_FS)
            {
                gpipeInstalled = gcvTRUE;
            }
            else
            {
                psInstalled = gcvTRUE;
            }
        }
    }

    /* If all stages do not have program installed. */
    if (!progInstalled)
    {
        goto OnError;
    }

    /* TCS and TES must be paired */
    if ((ppObj->stageProgs[__GLSL_STAGE_TCS] && !ppObj->stageProgs[__GLSL_STAGE_TES]) ||
        (ppObj->stageProgs[__GLSL_STAGE_TES] && !ppObj->stageProgs[__GLSL_STAGE_TCS]))
    {
        goto OnError;
    }

    /* GS must be paired with VS */
    if (ppObj->stageProgs[__GLSL_STAGE_GS] && !ppObj->stageProgs[__GLSL_STAGE_VS])
    {
        goto OnError;
    }

    if (gpipeInstalled && psInstalled)
    {
        gctINT stage, shaderCount = 0;
        gcSHADER shaderArray[__GLSL_STAGE_LAST] = {0};

        for (stage = 0; stage < __GLSL_STAGE_LAST; ++stage)
        {
            /* We don't need to check CS.*/
            if (ppObj->stageProgs[stage] && stage != __GLSL_STAGE_CS)
            {
                __GLchipSLProgram *program = (__GLchipSLProgram*)ppObj->stageProgs[stage]->privateData;

                if (program)
                {
                    shaderArray[shaderCount++] = program->curPgInstance->binaries[stage];
                }
            }
        }

        gcmONERROR(gcValidateProgramPipeline(shaderCount,
                                             shaderArray));
    }

    valid = GL_TRUE;

    /* If there is no VS and CS, pipeline is valid, but draw will be skipped. */
    if (callFromDraw &&
        (!gpipeInstalled && !psInstalled))
    {
        skipped = GL_TRUE;
        goto OnError;
    }

OnError:
    if (!valid)
    {
        skipped = GL_TRUE;  /* Skip draw with invalid ppo */
        if (callFromDraw)
        {
            __GL_ERROR(GL_INVALID_OPERATION);
        }
    }
    ppObj->validateStatus = valid;
    gcmFOOTER_ARG("return=%d", !skipped);
    return !skipped;
}

gceSTATUS
gcChipPgStateKeyAlloc(
    __GLcontext *gc,
    __GLchipProgramStateKey **ppPgStateKey
    )
{
    GLuint size;
    GLuint staticDataSize;
    GLuint numOfTexSamplers = gc->constants.shaderCaps.maxTextureSamplers;
    __GLchipProgramStateKey *pgStateKey;
    GLuint memOffset = 0;
    GLubyte *curMemory;

    gcmHEADER_ARG("gc=0x%x ppPgStateKey=0x%x", gc, ppPgStateKey);

    gcmASSERT(ppPgStateKey);

    staticDataSize = gcmSIZEOF(__GLchipProgramStateStaticKey) +
                    numOfTexSamplers * gcmSIZEOF(__GLchipPgStateKeyNP2AddrMode) +
                    numOfTexSamplers * gcmSIZEOF(__GLchipPgStateKeyShadowMapCmpInfo) +
                    numOfTexSamplers * gcmSIZEOF(__GLchipPgStateKeyTexPatchInfo);

    size = gcmSIZEOF(__GLchipProgramStateKey) + staticDataSize;

    pgStateKey = (*gc->imports.calloc)(gc, 1, size);

    /* static Data size.*/
    pgStateKey->size = staticDataSize;
    curMemory = (GLubyte *)pgStateKey;
    memOffset += gcmSIZEOF(__GLchipProgramStateKey);
    pgStateKey->staticKey = (__GLchipProgramStateStaticKey*)(curMemory + memOffset);

    pgStateKey->data = (GLvoid*)pgStateKey->staticKey;

    memOffset += gcmSIZEOF(__GLchipProgramStateStaticKey);
    pgStateKey->NP2AddrMode = (__GLchipPgStateKeyNP2AddrMode*)(curMemory + memOffset);
    memOffset += numOfTexSamplers * gcmSIZEOF(__GLchipPgStateKeyNP2AddrMode);
    pgStateKey->shadowMapCmpInfo = (__GLchipPgStateKeyShadowMapCmpInfo*)(curMemory + memOffset);
    memOffset += numOfTexSamplers * gcmSIZEOF(__GLchipPgStateKeyShadowMapCmpInfo);
    pgStateKey->texPatchInfo = (__GLchipPgStateKeyTexPatchInfo*)(curMemory + memOffset);


    *ppPgStateKey = pgStateKey;

    gcmFOOTER_NO();

    return gcvSTATUS_OK;
}

gceSTATUS
gcChipPgStateKeyFree(
    __GLcontext *gc,
    __GLchipProgramStateKey **ppPgStateKey
    )
{
    gcmHEADER_ARG("gc=0x%x ppPgStateKey=0x%x", gc, ppPgStateKey);

    (*gc->imports.free)(gc, *ppPgStateKey);

    *ppPgStateKey = gcvNULL;

    gcmFOOTER_NO();

    return gcvSTATUS_OK;
}

gceSTATUS
gcChipPgStateKeyCopy(
    __GLcontext *gc,
    __GLchipProgramStateKey *pDst,
    __GLchipProgramStateKey *pSrc
    )
{
    GLubyte *curMemory;
    GLuint memOffset = 0;
    GLuint numOfTexSamplers = gc->constants.shaderCaps.maxTextureSamplers;

    gcmHEADER_ARG("pDst=0x%x pSrc=0x%x", pDst, pSrc);

    curMemory = (GLubyte *)pDst + gcmSIZEOF(__GLchipProgramStateKey);
    /* copy static data first.*/
    __GL_MEMCOPY(curMemory, pSrc->data, pSrc->size);

    /* correct the pointer of pDst.*/
    pDst->staticKey = (__GLchipProgramStateStaticKey*)(curMemory + memOffset);
    pDst->data = (GLvoid *) pDst->staticKey;
    memOffset += gcmSIZEOF(__GLchipProgramStateStaticKey);

    pDst->NP2AddrMode = (__GLchipPgStateKeyNP2AddrMode*)(curMemory + memOffset);
    memOffset += numOfTexSamplers * gcmSIZEOF(__GLchipPgStateKeyNP2AddrMode);
    pDst->shadowMapCmpInfo = (__GLchipPgStateKeyShadowMapCmpInfo*)(curMemory + memOffset);
    memOffset += numOfTexSamplers * gcmSIZEOF(__GLchipPgStateKeyShadowMapCmpInfo);
    pDst->texPatchInfo = (__GLchipPgStateKeyTexPatchInfo*)(curMemory + memOffset);

    gcmFOOTER_NO();

    return gcvSTATUS_OK;
}



