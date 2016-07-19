/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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


#include "vdk_sample_common.h"

/*
 * VDK
*/
#ifndef ANDROID
extern vdkEGL VdkEgl;
#endif
/*
 * Model
*/
static const char *        PositionFile = "ball_position.txt";
static float *            PositionFloats = NULL;
static size_t            PositionFloatsCount = 0;
static GLuint            PositionGLBuffer = 0;
#ifndef ANDROID
const static GLuint        PositionVertexAttributeArrayIndex = 0;
#else
static const GLuint     PositionVertexAttributeArrayIndex = 0;
#endif

static const char *        NormalFile = "ball_normal.txt";
static float *            NormalFloats = NULL;
static size_t            NormalFloatsCount = 0;
static GLuint            NormalGLBuffer = 0;
#ifndef ANDROID
const static GLuint        NormalVertexAttributeArrayIndex = 1;
#else
static const GLuint     NormalVertexAttributeArrayIndex = 1;
#endif

static const char *        TriangleFile = "ball_triangles.txt";
static unsigned short *    TriangleData = NULL;
static size_t            TriangleVertexCount = 0;
static GLuint            TriangleGLBuffer = 0;
#ifdef ANDROID
static const GLuint     TriangleVertexAttributeArrayIndex = 2;
#else
const static GLuint        TriangleVertexAttributeArrayIndex = 2;
#endif

static float CenterX = 0.0f;
static float CenterY = 0.0f;
static float CenterZ = 0.0f;
static float Radius     = 0.0f;

/*
 * Shaders
*/

static const char *    VSFile = "ball.vert";
static const char *    FGFile = "ball.frag";

static GLuint        Program = 0;

/*
 * Matrix
*/

/*
 * Matrix
*/

extern const float    ModelView    [16];
extern const float    Projection    [16];
extern const float    MVP            [16];

static float    MV_INV_TRANS[16];

/*
*    Uniforms
*/
#ifndef ANDROID
const static float BallSize = 40;
const static float BounceHeight = 300;
const static float BounceSpeed = 1.0;

const static float LightDirection[] = {-0.43644f, 0.21822f, -0.87287f, 1.0f};
const static float BallColor     [] = {1.0, 0.3f, 0.2f, 1.0};
#else
static const float BallSize = 40;
static const float BounceHeight = 300;
static const float BounceSpeed = 1.0;

static const float LightDirection[] = {-0.43644f, 0.21822f, -0.87287f, 1.0f};
static const float BallColor     [] = {1.0, 0.3f, 0.2f, 1.0};
#endif
extern float passed_time;
static GLuint TimeLocation = 0;


VDKS_BOOL BallInit()
{
    char szCurDir[MAX_PATH + 1];
    char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1], szTempFile[MAX_PATH + 1];
#ifdef ANDROID
    strcpy(szCurDir,"/sdcard/sample/sample6/");
#else
    VDKS_Func_GetCurrentDir(szCurDir);
#endif
    strcpy(szVSFile, szCurDir);
    strcpy(szFGFile, szCurDir);
    strcat(szVSFile, VSFile);
    strcat(szFGFile, FGFile);

    /*
     * Read in Model's Positon Floats Data.
    */
    strcpy(szTempFile, szCurDir);
    strcat(szTempFile, PositionFile);
    if (VDKS_TRUE !=  VDKS_ReadFloats (szTempFile, &PositionFloats, (SIZE_T *)&PositionFloatsCount))
    {
        printf("Init : Failed to read position data file.");
        return VDKS_FALSE;
    }

    glGenBuffers(1, &PositionGLBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
    glBufferData(GL_ARRAY_BUFFER, PositionFloatsCount * sizeof(float), PositionFloats, GL_STATIC_DRAW);

    /*
     * Read in Model's Normal Floats Data.
    */
    strcpy(szTempFile, szCurDir);
    strcat(szTempFile, NormalFile);
    if (VDKS_TRUE !=  VDKS_ReadFloats (szTempFile, &NormalFloats, (SIZE_T *)&NormalFloatsCount))
    {
        printf("Init : Failed to read position data file.");
        return VDKS_FALSE;
    }

    glGenBuffers(1, &NormalGLBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, NormalGLBuffer);
    glBufferData(GL_ARRAY_BUFFER, NormalFloatsCount * sizeof(float), NormalFloats, GL_STATIC_DRAW);

    /*
     * Model Postion Center & Radius
    */

    VDKS_Func_ModelCenterRadius(PositionFloats, PositionFloatsCount, &CenterX, &CenterY, &CenterZ, &Radius);
    VDKS_Macro_AlertUser(0, "CenterX : %f, CenterY : %f, CenterZ : %f, Radius : %f\n",CenterX, CenterY, CenterZ, Radius);

    /*
     * Read in Model's Triangle Floats Data.
    */
    strcpy(szTempFile, szCurDir);
    strcat(szTempFile, TriangleFile);
    if (VDKS_TRUE != VDKS_ReadTriangle(szTempFile, &TriangleData, (SIZE_T *)&TriangleVertexCount))
    {
        printf("Init : Failed to read triangle data file.");
        return VDKS_FALSE;
    }

    glGenBuffers(1, &TriangleGLBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, TriangleVertexCount * sizeof(unsigned short), TriangleData, GL_STATIC_DRAW);

    /*
     * Matrix
    */
    do
    {
        VDKS_Func_Matrix_Inverse(ModelView, MV_INV_TRANS);
        VDKS_Func_Matrix_Transpose(MV_INV_TRANS);
    }
    while(0);

    /*
    * Shaders
    */
    Program = glCreateProgram();

    glBindAttribLocation(Program, PositionVertexAttributeArrayIndex, "position");
    glBindAttribLocation(Program, NormalVertexAttributeArrayIndex, "normal");

    Program = VDKS_Func_MakeShaderProgram2(szVSFile, szFGFile, Program);

    if (Program == 0)
    {
        printf("Failed to make test program.");
        return VDKS_FALSE;
    }

    /*
    *    Attributes
    */
    do
    {
        int        index = 0;
        GLint    nr_act_att = 0;
        char *    name  = NULL;
        GLsizei bufsize = 128;

        GLint    location = 0;


        glGetProgramiv(Program, GL_ACTIVE_ATTRIBUTES, &nr_act_att);

        name = malloc(bufsize);
        if (NULL == name) {
            printf("out-of-memory\n");
            return VDKS_FALSE;
        }

        for (index = 0; index < nr_act_att; index++)
        {
            GLsizei length;
            GLenum    type;
            GLint    size = 0;
            glGetActiveAttrib(Program, index, bufsize, &length, &size, &type, name);

            if (!strcmp(name, "position"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT_VEC3);

                location = glGetAttribLocation(Program, name);
                assert( location == PositionVertexAttributeArrayIndex);

            }
            else if (!strcmp(name, "normal"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT_VEC3);

                location = glGetAttribLocation(Program, name);
                assert( location == NormalVertexAttributeArrayIndex);
            }
            else
            {
                printf("The attribute name is not in handling.");
                free(name);
                return VDKS_FALSE;
            }
        }

        free(name);
    }
    while(0);

    /*
     * Uniforms
    */
    do
    {
        GLint nr_act_uni = 0;

        GLint i = 0;

        char *    name  = NULL;
        GLsizei bufsize = 128;
        GLint location = 0;

        name = malloc(bufsize);
        if (NULL == name) {
            printf("out-of-memory\n");
            return VDKS_FALSE;
        }

        glGetProgramiv(Program, GL_ACTIVE_UNIFORMS, &nr_act_uni);

        glUseProgram(Program);

        for(i = 0; i < nr_act_uni; i++)
        {
            GLsizei length;
            GLenum    type;
            GLint    size = 0;
            glGetActiveUniform(Program, i, bufsize, &length, &size, &type, name);

            if (!strcmp(name, "mv"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT_MAT4);

                location = glGetUniformLocation(Program, name);
                glUniformMatrix4fv(location, 1, GL_FALSE, ModelView);
            }
            else if (!strcmp(name, "mvp"))
            {

                assert(size == 1);
                assert(type == GL_FLOAT_MAT4);

                location = glGetUniformLocation(Program, name);
                glUniformMatrix4fv(location, 1, GL_FALSE, MVP);
            }
            else if (!strcmp(name, "mv_inverse_transpose"))
            {

                assert(size == 1);
                assert(type == GL_FLOAT_MAT4);

                location = glGetUniformLocation(Program, name);
                glUniformMatrix4fv(location, 1, GL_FALSE, MV_INV_TRANS);
            }
            else if (!strcmp(name, "ball_size"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT);

                location = glGetUniformLocation(Program, name);

                glUniform1f(location, BallSize);
            }
            else if (!strcmp(name, "bounce_speed"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT);

                location = glGetUniformLocation(Program, name);

                glUniform1f(location, BounceSpeed);
            }
            else if (!strcmp(name, "bounce_height"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT);

                location = glGetUniformLocation(Program, name);

                glUniform1f(location, BounceHeight);
            }
            else if (!strcmp(name, "time"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT);

                TimeLocation = glGetUniformLocation(Program, name);

                assert(TimeLocation);
            }
            else if (!strcmp(name, "light_dir"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT_VEC4);

                location = glGetUniformLocation(Program, name);

                assert(location);

                glUniform4fv(location, 1, LightDirection);
            }
            else if (!strcmp(name, "ball_color"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT_VEC4);

                location = glGetUniformLocation(Program, name);

                assert(location);

                glUniform4fv(location, 1, BallColor);
            }
            else
            {
                printf("The uniform name is not in handling.");
                return VDKS_FALSE;
            }
        }

        free(name);

        glUseProgram(0);
    }
    while(0);

    /*
     * Miscellaneous GL States
    */

    VDKS_Macro_CheckGLError();
    return VDKS_TRUE;
}

void BallPass()
{
    /*
    *    Attributes.
    */

    VDKS_Func_DisableAllVertexAttributeArray();

    glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
    glVertexAttribPointer(PositionVertexAttributeArrayIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(PositionVertexAttributeArrayIndex);

    glBindBuffer(GL_ARRAY_BUFFER, NormalGLBuffer);
    glVertexAttribPointer(NormalVertexAttributeArrayIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(NormalVertexAttributeArrayIndex);


    glUseProgram(Program);

    glUniform1f(TimeLocation, passed_time);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);

    glDrawElements(GL_TRIANGLES, TriangleVertexCount, GL_UNSIGNED_SHORT, 0);

    VDKS_Macro_CheckGLError();

    glUseProgram(0);

    glFinish();
}

