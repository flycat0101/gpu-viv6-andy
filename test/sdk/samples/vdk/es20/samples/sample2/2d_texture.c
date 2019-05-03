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
 * 2D Data
*/

#define SIDE_SIZE (0.7f)

static const float PositionData [] =
{
    SIDE_SIZE, SIDE_SIZE, 0.0f,
    SIDE_SIZE, -SIDE_SIZE, 0.0f,
    -SIDE_SIZE, -SIDE_SIZE, 0.0f,
    -SIDE_SIZE, SIDE_SIZE, 0.0f
};

static GLuint    PositionGLBuffer = 0;

static const unsigned short TriangleData [] =
{
    0, 1, 2, 3, 0, 2
};

static GLuint    TriangleGLBuffer = 0;

static float    CenterX = 0.0f;
static float    CenterY = 0.0f;
static float    CenterZ = 0.0f;

static float    Radius    = 0.0f;

/*
*    Shader
*/
static const char *    VSFile = "2d_texture.vert";
static const char *    FGFile = "2d_texture.frag";
static GLuint    Program = 0;


/*
 * Matrix
*/

static float    ModelView    [16];
static float    Projection    [16];
static float    MVP            [16];

/*
*    Texture
*/

static char* TextureFile = "Mars_POSITIVE_Z_MIPMAP_LEVEL_0.bmp";

static GLuint TextureObject = 0;

VDKS_BOOL Tex2DInitTexture()
{
    int width = 0;
    int height = 0;

    unsigned char * rgba = NULL;
    char szTextureFile[MAX_PATH + 1];
#ifndef ANDROID
    VDKS_Func_GetCurrentDir(szTextureFile);
#else
    strcpy(szTextureFile, "/sdcard/sample/sample2/");
#endif
    glGenTextures(1, &TextureObject);

    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, TextureObject);

    strcat(szTextureFile, TextureFile);
    rgba = VDKS_Func_ReadBmp(szTextureFile, &width, &height);
    if (NULL == rgba) {
        return VDKS_FALSE;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

    free(rgba);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return VDKS_TRUE;
}

VDKS_BOOL Tex2DInit()
{
    char szCurDir[MAX_PATH + 1];
    char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1];
#ifndef ANDROID
    VDKS_Func_GetCurrentDir(szCurDir);
#else
    strcpy(szCurDir, "/sdcard/sample/sample2/");
#endif
    strcpy(szVSFile, szCurDir);
    strcpy(szFGFile, szCurDir);
    strcat(szVSFile, VSFile);
    strcat(szFGFile, FGFile);

    glGenBuffers(1, &PositionGLBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PositionData), PositionData, GL_STATIC_DRAW);

    glGenBuffers(1, &TriangleGLBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TriangleData), TriangleData, GL_STATIC_DRAW);

    /*
    * Matrix
    */
    do
    {
        float view    [3] = {0.0, 0.0, 200};
        float focus [3] = {0.0, 0.0, 0.0};
        float up    [3] = {0.0, 1.0, 0.0};

        /*
        * Assume
        */
        assert (CenterX == 0.0f);
        assert (CenterY == 0.0f);
        assert (CenterZ == 0.0f);
        assert (Radius < 200.0f);

        VDKS_Func_LoadIdentity(ModelView);
        VDKS_Func_LoadIdentity(Projection);
        VDKS_Func_LoadIdentity(MVP);

        VDKS_Func_Matrix_LookAt(view, focus, up, ModelView);

        Radius = 1.0f;

        VDKS_Func_Matrix_Ortho(
            -1.0f * Radius, Radius,    /*left, right*/
            -1.0f * Radius, Radius,    /*bottom, up*/
            1.0f, 400.0,                /*near, far*/
            Projection);

        VDKS_Func_Matrix_Mul4by4(MVP, Projection, ModelView);
    }
    while(0);

    Tex2DInitTexture();

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

                assert(size == 1);
                assert(type == GL_FLOAT_MAT4);

                location = glGetUniformLocation(Program, name);

                glUniformMatrix4fv(location, 1, GL_FALSE, MVP);
            }
            else if(!strcmp(name, "image"))
            {
                assert(size == 1);
                assert(type == GL_SAMPLER_2D);
                //assert(type == GL_SAMPLER_CUBE);

                location  = glGetUniformLocation(Program, name);

                glUniform1i(location, 0);
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

void Tex2DPass ()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glUseProgram(Program);

    glDrawElements(GL_TRIANGLES, sizeof(TriangleData) / sizeof(unsigned short), GL_UNSIGNED_SHORT, 0);
}

