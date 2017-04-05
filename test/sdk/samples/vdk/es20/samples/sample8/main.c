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

Some code of matrix operations is from Mesa3D, please pay attention.

History        :    2008-09-19,    Written by qizhuang.liu.
                2008-09-18, Porting to WinCE, qizhuang.liu.
                2008-09-22,    Rename the project and solution,qizhuang.liu.
                            Add CopyRight infomation,qizhuang.liu.
                2008-09-24, Porting to Linux and add support of EGL_API_FB,
                            qizhuang.liu.
                2009-01-21, Port to VDK, qizhuang.liu.
*/

//#define _CRT_SECURE_NO_DEPRECATE

#include "vdk_sample_common.h"

char*    PositionFile = "position.txt";
char*    BinormalFile = "binormal.txt";
char*    TriangleFile = "triangles.txt";
char*    VertexShaderFile = "VertexShader.vert";
char*    FragmentShaderFile = "FragmentShader.frag";
#ifndef ANDROID
/*
 * VDKS
*/
char *    VDKS_ARG0 = NULL;
int        VDKS_Val_WindowsWidth = 0;
int        VDKS_Val_WindowsHeight = 0;
vdkEGL    VDK_EGL;
#endif
/*
 * APP Data
*/
float    *PositionBuffer;
size_t  PositionCount;

float    *BinormalBuffer;
size_t    BinomalCount;

unsigned short *TriangleBuffer;
size_t    TriangleVertexCount;

/*
 * Matrix
*/
float    ModelView    [16];
float    Projection    [16];
float    MVP            [16];

/*
 * Program
*/
GLuint    ShaderProgram        = 0;
GLuint    Location            = 0;
GLint    NrActiveAttributes    = 0;

VDKS_BOOL    Init();
void    Run();
void    CheckError (const char* Comment);

static  float pass = 0.0f;

int SaveResult = 0;



void Run()
{
    float rot_matrix        [16];
    float trans_matrix        [16];
    float rot_trans_matrix    [16];
    float bufModelView        [16];
    char szBMPFile[MAX_PATH + 1];

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    //Rotate
    memcpy(bufModelView, ModelView, sizeof(float) * 16);

    VDKS_Func_LoadIdentity(rot_matrix);
    VDKS_Func_LoadIdentity(trans_matrix);

    VDKS_Func_Translate(trans_matrix, 0, 0, 10);
    VDKS_Func_Rotate(rot_matrix, pass, 0, 1, 0);

    VDKS_Func_Matrix_Mul4by4(rot_trans_matrix, rot_matrix, trans_matrix);

    VDKS_Func_Matrix_Mul4by4(ModelView, bufModelView, rot_trans_matrix);

    VDKS_Func_Matrix_Mul4by4(
            (float *)MVP,
            (float *)Projection,
            (float *)ModelView);

    glUniformMatrix4fv( Location, 1, GL_FALSE, MVP);

    memcpy(ModelView, bufModelView,  sizeof(float) * 16);

    /*Marvell's depth report.*/

    glDrawElements( GL_TRIANGLES, TriangleVertexCount, GL_UNSIGNED_SHORT, TriangleBuffer);

    pass += 5.0f;

    if (SaveResult)
    {
#ifndef ANDROID
        VDKS_Func_GetCurrentDir(szBMPFile);
#else
    strcpy(szBMPFile, "/sdcard/sample/sample8/");
#endif

        strcat(szBMPFile, "vdksample8_es20_result.bmp");
        VDKS_Func_SaveScreen(szBMPFile);
    }
#ifndef ANDROID
        vdkSwapEGL(&VDK_EGL);
#endif
}



VDKS_BOOL Init()
{
    int index = 0;

    const int    bufSize = 1024;

    char        name [1024];

    GLsizei        length = 0;
    GLenum        type = 0;
    GLint        size = 0;

    char szCurDir[MAX_PATH + 1];
    char szVertexShaderFile[MAX_PATH + 1], szFragmentShaderFile[MAX_PATH + 1], szTempFile[MAX_PATH + 1];
#ifndef ANDROID
    VDKS_Func_GetCurrentDir(szCurDir);
#else
    strcpy(szCurDir, "/sdcard/sample/sample8/");
#endif
    strcpy(szVertexShaderFile, szCurDir);
    strcpy(szFragmentShaderFile, szCurDir);
    strcat(szVertexShaderFile, VertexShaderFile);
    strcat(szFragmentShaderFile, FragmentShaderFile);
#ifndef ANDROID
    if (VDKS_TRUE != VDKS_Init( &VDK_EGL ))
    {
        return VDKS_FALSE;
    }
#endif
    //Stream Data
    strcpy(szTempFile, szCurDir);
    strcat(szTempFile, PositionFile);
    if(!VDKS_ReadFloats(szTempFile , &PositionBuffer, (SIZE_T *)&PositionCount))
        return VDKS_FALSE;

    strcpy(szTempFile, szCurDir);
    strcat(szTempFile, BinormalFile);
    if(!VDKS_ReadFloats(szTempFile, &BinormalBuffer, (SIZE_T *)&BinomalCount))
        return VDKS_FALSE;

    strcpy(szTempFile, szCurDir);
    strcat(szTempFile, TriangleFile);
    if(!VDKS_ReadTriangle(szTempFile, &TriangleBuffer, (SIZE_T *)&TriangleVertexCount))
        return VDKS_FALSE;

    //Matrix
    VDKS_Func_LoadIdentity(ModelView);
    VDKS_Func_LoadIdentity(Projection);
    VDKS_Func_LoadIdentity(MVP);

    do
    {
        float view_position    []    = {-0.5, -0.5,    150.0};
        float view_focus    []    = {0.5,    0.5,    0.0};
        float view_up        []    = {0.0,    1.0,    0.0};

        float left        = -    70 ;
        float right        =    -left ;

        float bottom    =    left * ((float)VDKS_Val_WindowsHeight / (float)VDKS_Val_WindowsWidth);
        float top        =    - bottom ;

        float nearval    =    1.0    ;
        float farval    =    300.0 ;

        VDKS_Func_LookAt(view_position, view_focus, view_up, ModelView);

        VDKS_Func_Ortho(left, right, bottom, top, nearval, farval, Projection);

        VDKS_Func_Matrix_Mul4by4(
            (float *)MVP,
            (float *)Projection,
            (float *)ModelView);

    }
    while(0);

    ShaderProgram = VDKS_Func_MakeShaderProgram(szVertexShaderFile, szFragmentShaderFile);
    if (ShaderProgram == 0)
    {
        return VDKS_FALSE;
    }

    glUseProgram( ShaderProgram);

    Location = glGetUniformLocation( ShaderProgram, "matViewProjection");
    glUniformMatrix4fv( Location, 1, GL_FALSE, MVP);

    Location = glGetUniformLocation( ShaderProgram, "gamma" );
    glUniform1f( Location, 2.2f);

    glGetProgramiv(ShaderProgram, GL_ACTIVE_ATTRIBUTES, &NrActiveAttributes);

    for(index = 0; index < NrActiveAttributes; index++)
    {

        glGetActiveAttrib(
            ShaderProgram,
            index,
            bufSize,
            &length,
            &size,
            &type,
            name);

        Location = glGetAttribLocation(ShaderProgram, name);

        if(!strcmp(name, "rm_Vertex"))
        {
            int        group  = 3;

            VDKS_BOOL    normalized =GL_FALSE;

            glVertexAttribPointer( Location, group, GL_FLOAT, normalized, 0, PositionBuffer);

            glEnableVertexAttribArray( Location);

        }
        else if(!strcmp(name, "rm_Binormal"))
        {
            int group  = 3;

            VDKS_BOOL normalized =GL_FALSE;

            glVertexAttribPointer( Location, group, GL_FLOAT, normalized, 0, BinormalBuffer);

            glEnableVertexAttribArray( Location);
        }
        else
        {
            printf("Error : the name query from GL is unknown : %s\n", name);
            assert(0);
        }
    }

    Location = glGetUniformLocation( ShaderProgram, "matViewProjection");

    glViewport(0, 0, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight);

    glUseProgram( ShaderProgram );

    glEnable(GL_DEPTH_TEST);

    return VDKS_TRUE;
}
#ifndef ANDROID
int main(int argc, char* argv[])
{

    int i = 0;

    int count = 1;

    int tex_w = 0;

    /*
     * Miscellaneous
    */
    if (argc == 2)
    {
        count = atoi(argv[1]);
    }
    else if(argc == 4)
    {
        count = atoi(argv[1]);
        VDKS_Val_WindowsWidth = atoi(argv[2]);
        VDKS_Val_WindowsHeight = atoi(argv[3]);
    }
    else if(argc == 5)
    {
        count = atoi(argv[1]);
        VDKS_Val_WindowsWidth = atoi(argv[2]);
        VDKS_Val_WindowsHeight = atoi(argv[3]);
        SaveResult = atoi(argv[4]);;
    }



    VDKS_ARG0 = argv[0];

    if ( !Init() )
        return 1;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &tex_w );


    vdkShowWindow(VDK_EGL.window);

    for(i = 0; i < count; i++)
    {
        Run();
    }

    vdkFinishEGL(&VDK_EGL);

    VDKS_Macro_AlertUser(0, "Program exits");

    return 0;

}
#endif
