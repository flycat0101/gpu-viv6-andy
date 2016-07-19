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
 * Model
*/
static const char *        PositionFile = "snake_position.txt";
static float *            PositionFloats = NULL;
static size_t            PositionFloatsCount = 0;

static GLuint            PositionGLBuffer = 0;

static const char *        TriangleFile = "snake_triangles.txt";
static unsigned short *    TriangleData = NULL;
static size_t            TriangleVertexCount = 0;

static GLuint            TriangleGLBuffer = 0;

/*
*    Shader
*/
static const char *    VSFile = "snake.vert";
static const char *    FGFile = "snake.frag";
static GLuint    Program = 0;

/*
 * Matrix
*/

/*    We use this two matrix to fix the view */

static float    UniformViewMatrix [] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, -80.0f,
    0.0f, 0.0f, 0.0f, 1.0f,
};

static float    UniformViewProjectionMatrix [] =
{
    0.943787f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, -1.0f, 80.0f,
    0.0f, 0.0f, -1.0f, 80.0f,
};

/*
*    Texture
*/
#ifdef ANDROID
static char* TextureFile = "/sdcard/sample/sample3/Flame.bmp";
#else
static char* TextureFile = "Flame.bmp";
#endif
static GLuint TextureObject = 0;

VDKS_BOOL SnakeInitTexture()
{
    int width = 0;
    int height = 0;
    int bpp = 0;

    unsigned char * rgba = NULL;
    char szCurDir[MAX_PATH + 1];
    char szTempFile[MAX_PATH + 1];
    VDKS_Func_GetCurrentDir(szCurDir);

    glGenTextures(1, &TextureObject);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, TextureObject);

    strcpy(szTempFile, szCurDir);
    strcat(szTempFile, TextureFile);
    rgba = VDKS_Func_ReadBmp_Bpp(szTempFile, &width, &height, &bpp);

    assert (bpp == 3);

    assert (rgba);

    if (bpp == 3)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgba);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
    }

    /* For Flame.bmp Only */
    assert (height == 1);

    free(rgba);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return VDKS_TRUE;
}

VDKS_BOOL SnakeInit()
{
    char szCurDir[MAX_PATH + 1];
    char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1], szTempFile[MAX_PATH + 1];
#ifdef ANDROID
    strcpy(szCurDir, "/sdcard/sample/sample3/");
#else
    VDKS_Func_GetCurrentDir(szCurDir);
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

    SnakeInitTexture();

    /*
    * Shaders
    */

    Program = VDKS_Func_MakeShaderProgram(szVSFile, szFGFile);

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
        GLint    position_location = 0;

        /*
        * Manage Location
        */

        VDKS_Func_LocationManagerInit();

        position_location = VDKS_Func_LocationAcquire();

        if (position_location == 0)
        {
            printf("Failed to alloc location.");
            return VDKS_FALSE;
        }

        glBindAttribLocation(Program, position_location, "position");

        glLinkProgram(Program);

        /*
        * Post re-link
        */

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

                assert( location == position_location);

                glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
                glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
                glEnableVertexAttribArray(location);
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

            if(!strcmp(name, "mvp"))
            {

                /* Make sure the matrix is transposed just once. */
                static int transposed = 0;

                assert(transposed == 0);

                transposed = 1;

                assert(size == 1);
                assert(type == GL_FLOAT_MAT4);

                location = glGetUniformLocation(Program, name);


                VDKS_Func_Matrix_Transpose(UniformViewProjectionMatrix);

                glUniformMatrix4fv(location, 1, GL_FALSE, UniformViewProjectionMatrix);

                VDKS_Macro_CheckGLError();
            }
            else if(!strcmp(name, "mv"))
            {

                /* Make sure the matrix is transposed just once. */
                static int transposed = 0;

                assert(transposed == 0);

                transposed = 1;


                assert(size == 1);
                assert(type == GL_FLOAT_MAT4);

                location = glGetUniformLocation(Program, name);


                VDKS_Func_Matrix_Transpose(UniformViewMatrix);

                glUniformMatrix4fv(location, 1, GL_FALSE, UniformViewMatrix);


                VDKS_Macro_CheckGLError();
            }

            else if(!strcmp(name, "palette"))
            {
                assert(size == 1);
                assert(type == GL_SAMPLER_2D);

                location  = glGetUniformLocation(Program, name);

                glUniform1i(location, 0);
            }
            else if(!strcmp(name, "particle_exp"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT);

                location  = glGetUniformLocation(Program, name);

                glUniform1f(location, 0.07f);
            }
            else if(!strcmp(name, "time"))
            {
                assert(size == 1);
                assert(type == GL_FLOAT);

                location = glGetUniformLocation(Program, "time");

                /*assert(location);*/

                glUniform1f(location, 0.0f);
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

    return VDKS_TRUE;
}

void SnakePass ()
{
    static float second = 1.0f;

    GLuint location_time = 0;

    /*
    *    Ok, you should known, we should be 24 frames per second, so we should move 1/24 second per frame.
    *    Here, one frame is one pass.
    */
    second += 0.001f;

    if (second > 120.0f)
    {
        second -= 120.0f;
    }

    glUseProgram(Program);

    location_time = glGetUniformLocation(Program, "time");

    /*assert(location_time);*/

    glUniform1fv(location_time, 1,  &second);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glEnable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);

    glBlendFunc(GL_ONE, GL_ONE);

    glDisable(GL_CULL_FACE);

    glDepthMask(GL_FALSE);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);

    glDrawElements(GL_TRIANGLES, TriangleVertexCount, GL_UNSIGNED_SHORT, 0);

    VDKS_Macro_CheckGLError();

    glUseProgram(0);
}

