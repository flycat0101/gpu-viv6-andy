/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
*    All Rights Reserved.
*
*    Permission is hereby granted, free of charge, to any person obtaining
*    a copy of this software and associated documentation files (the
*    'Software'), to deal in the Software without restriction, including
*    without limitation the rights to use, copy, modify, merge, publish,
*    distribute, sub license, and/or sell copies of the Software, and to
*    permit persons to whom the Software is furnished to do so, subject
*    to the following conditions:
*
*    The above copyright notice and this permission notice (including the
*    next paragraph) shall be included in all copies or substantial
*    portions of the Software.
*
*    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
*    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
*    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
*    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
*    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
*    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/


/*
*    History : 2009.02.02, Created by qizhuang.liu.
*/

#ifndef _TEAPOT_H_
#define _TEAPOT_H_

#include "vdk_sample_common.h"

/*
 * Model
*/
static const char *        PositionFile = "teapot_position.txt";
static float *            PositionFloats = NULL;
static size_t            PositionFloatsCount = 0;
static GLuint            PositionGLBuffer = 0;

static GLuint            PositionVertexAttributeArrayIndex = 0;

static const char *        TextureCoord_0_File = "teapot_texcoord_0.txt";
static float *            TextureCoord_0_Floats = NULL;
static size_t            TextureCoord_0_FloatsCount = 0;
static GLuint            TextureCoord_0_GLBuffer = 0;

static GLuint            TextureCoord_0_VertexAttributeArrayIndex = 0;

static const char *        TriangleFile = "teapot_triangles.txt";
static unsigned short *    TriangleData = NULL;
static size_t            TriangleVertexCount = 0;
static GLuint            TriangleGLBuffer = 0;

static float    CenterX = 0.0f;
static float    CenterY = 0.0f;
static float    CenterZ = 0.0f;

static float    Radius    = 0.0f;

/*
 * Program & Shader
*/

static const char *    FrontBackVSFile = "FrontBack.vert";
static const char *    FrontBackFGFile = "FrontBack.frag";
GLuint TeapotProgramFrontBack = 0;

static const char *    BackFrontVSFile = "BackFront.vert";
static const char *    BackFrontFGFile = "BackFront.frag";
GLuint TeapotProgramBackFront = 0;

static const char *    FrontFrontVSFile = "FrontFront.vert";
static const char *    FrontFrontFGFile = "FrontFront.frag";
GLuint TeapotProgramFrontFront = 0;

static const char *    BackBackVSFile = "BackBack.vert";
static const char *    BackBackFGFile = "BackBack.frag";
GLuint TeapotProgramBackBack = 0;

GLuint TeapotProgram = 0;

static GLfloat TransparencyBias = 0.72f;
static GLfloat TransparencyScale = -1.0f;

/*
 * Matrix
*/

static float    ModelView    [16];
static float    Projection    [16];
static float    MVP            [16];

/*
*    Texture unit & object of render targets.
*/
GLuint    TeapotTexUnitBackground    = 0;
GLuint    TeapotTexUnitFrontFront    = 1;
GLuint    TeapotTexUnitFrontBack    = 2;
GLuint    TeapotTexUnitBackBack    = 3;
GLuint    TeapotTexUnitBackFront    = 4;

GLuint    TeapotTexObjBackground    = 0;
GLuint    TeapotTexObjFrontFront    = 0;
GLuint    TeapotTexObjFrontBack    = 0;
GLuint    TeapotTexObjBackFront    = 0;
GLuint    TeapotTexObjBackBack    = 0;


/*
*    Texture Image
*/

static char *    TexImgBaseFile    = "Hex_MIPMAP_LEVEL_0.bmp";
static GLuint    TexUnitBase        = 5;
static GLuint    TexObjBase        = 0;

/*
*    Passes Data
*/

/*
typedef struct
{
    char *            name;
    int                in_use;
    GLenum            glessl_type;
    GLint            glessl_size;
    GLuint *        gl_bufobj_name;
    GLuint *        general_index;
    GLint            size_of_type;
    GLenum            type;
    GLboolean        normalized;
    GLsizei            stride;
    const void *    ptr;
}
VDKS_Struct_AttributeInformation;
*/

static VDKS_Struct_AttributeInformation AttributesInformation [] =
{
    {
        "position",
        0, /*need to set.*/
        GL_FLOAT_VEC3,
        1,
        &PositionGLBuffer,
        &PositionVertexAttributeArrayIndex, /*general_index, need to set.*/
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        0 /*ptr, we use buffer object.*/
    },
    {
        "tex_coord0",
        0, /*need to set.*/
        GL_FLOAT_VEC2,
        1,
        &TextureCoord_0_GLBuffer,
        &TextureCoord_0_VertexAttributeArrayIndex,
        2,
        GL_FLOAT,
        GL_FALSE,
        0,
        0 /*ptr, we use buffer object.*/
    }
};

static int AttributesInformationCount = sizeof(AttributesInformation) / sizeof(AttributesInformation[0]);

/*
typedef struct
{
    GLuint *        program;
    char *            name;
    int                in_use;
    GLenum            glessl_type;
    GLint            glessl_size;
    GLuint            location;
    GLboolean        transpose;
    const void *    ptr;
}
VDKS_Struct_UnifomInfomation;
*/
static VDKS_Struct_UnifomInfomation UnifomsInfomation [] =
{
    /* FrontFront */
    {
        &TeapotProgramFrontFront,
        "transparency_bias",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TransparencyBias
    },
    {
        &TeapotProgramFrontFront,
        "transparency_scale",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TransparencyScale
    },
    {
        &TeapotProgramFrontFront,
        "mvp",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT_MAT4,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        MVP
    },
    {
        &TeapotProgramFrontFront,
        "base",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_SAMPLER_2D,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TexUnitBase
    },
    /* FrontBack */
    {
        &TeapotProgramFrontBack,
        "transparency_bias",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TransparencyBias
    },
    {
        &TeapotProgramFrontBack,
        "transparency_scale",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TransparencyScale
    },
    {
        &TeapotProgramFrontBack,
        "mvp",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT_MAT4,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        MVP
    },
    {
        &TeapotProgramFrontBack,
        "base",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_SAMPLER_2D,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TexUnitBase
    },
    /* BackBack */
    {
        &TeapotProgramBackBack,
        "transparency_bias",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TransparencyBias
    },
    {
        &TeapotProgramBackBack,
        "transparency_scale",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TransparencyScale
    },
    {
        &TeapotProgramBackBack,
        "mvp",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT_MAT4,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        MVP
    },
    {
        &TeapotProgramBackBack,
        "base",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_SAMPLER_2D,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TexUnitBase
    },
    /* BackFront */
    {
        &TeapotProgramBackFront,
        "transparency_bias",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TransparencyBias
    },
    {
        &TeapotProgramBackFront,
        "transparency_scale",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TransparencyScale
    },
    {
        &TeapotProgramBackFront,
        "mvp",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FLOAT_MAT4,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        MVP
    },
    {
        &TeapotProgramBackFront,
        "base",
        0, /*in_use, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_SAMPLER_2D,
        1,
        0, /*location, Will be set in VDKS_Func_Program_ValidateUniformsGetLocations*/
        GL_FALSE,
        &TexUnitBase
    },
};

static int UnifomsInfomationCount = sizeof(UnifomsInfomation) / sizeof(UnifomsInfomation[0]);

#endif
