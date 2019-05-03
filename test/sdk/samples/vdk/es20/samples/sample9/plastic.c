/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
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
static const char *        PositionFile = "elephant_position.txt";
static float *            PositionFloats = NULL;
static size_t            PositionFloatsCount = 0;

static GLuint            PositionGLBuffer = 0;

static GLuint            PositionVertexAttributeArrayIndex = 0;

static const char *        NormalFile = "elephant_normal.txt";
static float *            NormalFloats = NULL;
static size_t            NormalFloatsCount = 0;

static GLuint            NormalGLBuffer = 0;

static GLuint            NormalVertexAttributeArrayIndex  = 0;

static const char *        TriangleFile = "elephant_triangles.txt";
static unsigned short *    TriangleData = NULL;
static size_t            TriangleVertexCount = 0;

static GLuint            TriangleGLBuffer = 0;

static float    CenterX = 0.0f;
static float    CenterY = 0.0f;
static float    CenterZ = 0.0f;

static float    Radius    = 0.0f;

/*
 * Shaders
*/

static const char *    VSFile = "plastic.vert";
static const char *    FGFile = "plastic.frag";

static GLuint        Program = 0;

/*
 * Matrix
*/

static float    ModelView    [16];
static float    Projection    [16];
static float    MVP            [16];

/*
*    Miscellaneous Uniforms
*/

float UniformColor [] = {0.0f, 0.0f, 1.0f, 1.0f};

float UniformEyePosition [] = {0.0f, 0.0f, 200.0f};

VDKS_BOOL PlasticInit()
{
    char szCurDir[MAX_PATH + 1];
    char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1], szTempFile[MAX_PATH + 1];
#ifndef ANDROID
    VDKS_Func_GetCurrentDir(szCurDir);
#else
    strcpy(szCurDir,"/sdcard/sample/sample9/");
#endif

    strcpy(szVSFile, szCurDir);
    strcpy(szFGFile, szCurDir);
    strcat(szVSFile, VSFile);
    strcat(szFGFile, FGFile);
    /*
     * Model Positon
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
     * Model Postion Center & Radius
    */
    VDKS_Func_ModelCenterRadius(PositionFloats, PositionFloatsCount, &CenterX, &CenterY, &CenterZ, &Radius);

    VDKS_Macro_AlertUser(0, "CenterX : %f, CenterY : %f, CenterZ : %f, Radius : %f\n",CenterX, CenterY, CenterZ, Radius);

    /*
    *    Model Normal
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
     * Model Triangle
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
    * Shaders
    */
    Program = glCreateProgram();

    /*
    * Manage Location
    */

    VDKS_Func_LocationManagerInit();

    PositionVertexAttributeArrayIndex = VDKS_Func_LocationAcquire();

    NormalVertexAttributeArrayIndex = VDKS_Func_LocationAcquire();

    if ( !NormalVertexAttributeArrayIndex || ! PositionVertexAttributeArrayIndex)
    {
        printf("Failed to alloc location.");
        return VDKS_FALSE;
    }


    glBindAttribLocation(Program, PositionVertexAttributeArrayIndex, "position");

    glBindAttribLocation(Program, NormalVertexAttributeArrayIndex, "normal");

    Program = VDKS_Func_MakeShaderProgram2(szVSFile, szFGFile, Program);

    if (Program == 0)
    {
        printf("Failed to create a new program.");
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

                glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
                glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);

            }
            else if (!strcmp(name, "normal"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT_VEC3);

                location = glGetAttribLocation(Program, name);

                assert( location == NormalVertexAttributeArrayIndex);

                glBindBuffer(GL_ARRAY_BUFFER, NormalVertexAttributeArrayIndex);
                glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);

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
        GLint    size = 0;
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
            glGetActiveUniform(Program, i, bufsize, &length, &size, &type, name);

            if (!strcmp(name, "mvp"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT_MAT4);

                location = glGetUniformLocation(Program, name);
                glUniformMatrix4fv(location, 1, GL_FALSE, MVP);
            }
            else if (!strcmp(name, "color"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT_VEC4);

                location  = glGetUniformLocation(Program, name);
                glUniform4fv(location, 1, UniformColor);
            }
            else if (!strcmp(name, "eye_position"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT_VEC3);

                location = glGetUniformLocation(Program, name);

                glUniform3fv(location, 1, UniformEyePosition);
            }
            else
            {
                printf("The uniform name is not in handling.");
                free(name);
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

void PlasticPass()
{
    static float rotate = 0.0;

    rotate += 10.0;

    if(rotate > 360.0)
    {
        rotate -= 360.0;
    }

    /*
     * Matrix
    */
    do
    {
        GLuint location = 0;

        float * view =  UniformEyePosition;
        float focus [3] = {0.0, 0.0, 0.0};
        float up    [3] = {0.0, 1.0, 0.0};

        /*
         * Assume
        */

        VDKS_Func_LoadIdentity(ModelView);
        VDKS_Func_LoadIdentity(Projection);
        VDKS_Func_LoadIdentity(MVP);

        //VDKS_Func_Matrix_Rotate(ModelView, 180.0, 0.0, 1.0, 0.0);
        VDKS_Func_Matrix_Rotate(ModelView, rotate, 0.0, 1.0, 0.0);

        VDKS_Func_Matrix_LookAt(view, focus, up, ModelView);

        //VDKS_Func_Matrix_Ortho(
        //    -1.0f * Radius, Radius,    /*left, right*/
        //    -1.0f * Radius, Radius,    /*bottom, up*/
        //    1.0f, 2 * UniformEyePosition[2], /*near, far*/
        //    Projection);

        if (UniformEyePosition[2] < Radius + 10.0f)
        {
            printf("Please adjust your eye location to far away.");
            return;
        }

        VDKS_Func_Matrix_Frustum(
            Projection,
            -1.0f * Radius/* left */,
            Radius/* right */,
            -1.0f * Radius/* bottom */,
            Radius/* top */,
            UniformEyePosition[2] - Radius - 10.0f/* nearval */,
            UniformEyePosition[2] * 2.0f/* farval */);

        VDKS_Func_Matrix_Mul4by4(MVP, Projection, ModelView);

        //VDKS_Func_Translate(ModelView, -1.0 * CenterX, -1.0 * CenterY, -1.0 * CenterZ);

        glUseProgram(Program);
        location = glGetUniformLocation(Program, "mvp");
        glUniformMatrix4fv(location, 1, GL_FALSE, MVP);
    }
    while(0);

    VDKS_Func_DisableAllVertexAttributeArray();

    glEnableVertexAttribArray(PositionVertexAttributeArrayIndex);

    glEnableVertexAttribArray(NormalVertexAttributeArrayIndex);

    glEnable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(Program);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);

    glDrawElements(GL_TRIANGLES, TriangleVertexCount, GL_UNSIGNED_SHORT, 0);

    VDKS_Macro_CheckGLError();

    glUseProgram(0);
}

