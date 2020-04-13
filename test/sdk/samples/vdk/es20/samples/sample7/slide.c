/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
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

#define MAX_DESCRIPTOR_COUNT 2048

/*
*    Type Definition
*/
typedef struct _Segment
{
    float    angle;
    float    length;
}
Segment;

typedef struct
{
    struct
    {
        float x;
        float y;
    }
    acc_position;

    float acc_length;
}
MeshAccState;

/*
*    Texture
*/
GLuint TextureObject = 0;

/*
 * Data
*/

static GLuint PositionGLBuffer = 0;
static GLuint TexcoordGLBuffer = 0;
#ifndef ANDROID
const static GLuint    PositionVertexAttributeArrayIndex = 0;
const static GLuint    TexcoordVertexAttributeArrayIndex = 1;
#else
static const GLuint PositionVertexAttributeArrayIndex = 0;
static const GLuint TexcoordVertexAttributeArrayIndex = 1;
#endif
static GLuint    TriangleGLBuffer = 0;
static GLuint    TrianglesVertexCount = 0;

/*
*    Shader
*/

const char *    VSFile = "slide.vert";
const char *    FGFile = "slide.frag";

static GLuint    Program = 0;

/*
 * Matrix
*/

float    ModelView    [16];
float    Projection    [16];
float    MVP            [16];

/*
*    Slide Control
*/

float        passed_time = 0.6f;
GLuint        TimeLocation = 0;
float        LengthInXY = 0.0f;
float        LengthInZ = 0.0f;
unsigned    VertexCount = 0;
GLuint        OffsetInTexcoordLocation = 0;

#define SIN_A (500.0f)

void MakeMesh (
    float **    VertexBuffer,
    unsigned *    VertexCount,
    float **    TexcoordBuffer,
    Segment *    Descriptor,
    unsigned    DescriptorCount,
    MeshAccState * InitState,
    float *        LengthInXYFace,
    float *        MaxX,
    float *        MinX,
    float *        MaxY,
    float *        MinY,
    float        ZStep,
    unsigned    ZSegmentCount
    );

void MakeSinDescriptors(
    float A,
    float T,
    float InitPhase,
    float ShadowOnX,
    float Step,
    Segment ** DescriptorSet,
    unsigned * DescriptorCount
    );

float GetYSin(
    float A,
    float T,
    float InitPhase,
    float X
)
{
    return A * (float)sin( X * VDKS_PI * 2.0f / T + InitPhase);
}

float GetDyDxSin(
    float A,
    float T,
    float InitPhase,
    float X
)
{
    return A * (VDKS_PI * 2.0f / T) * (float)cos( X * VDKS_PI * 2.0f / T + InitPhase);
}

VDKS_BOOL GeneratePosition()
{
    unsigned i = 0;

    Segment * segs = NULL;
    unsigned descriptor_count = 0;

    float * vertex_buffer = NULL;
    float * texcoord_buffer = NULL;

    unsigned short * data_s = NULL;

    MeshAccState * init_state = malloc(sizeof(MeshAccState));
    if (NULL == init_state) {
        printf("out-of-memory\n");
        return VDKS_FALSE;
    }

    init_state->acc_position.x = 0.0f;
    init_state->acc_position.y = 0.0f;

    init_state->acc_length = 0.0f;

    MakeSinDescriptors(
        SIN_A,//float A,
        640.0f,//float T,
        0.0f,//float InitPhase,
        640.0f,//float ShadowOnX,
        10.0f,//float Step,
        &segs,//Segment ** DescriptorSet,
        &descriptor_count//unsigned * DescriptorCount
        );

    if (NULL == segs) return VDKS_FALSE;

    MakeMesh (
        &vertex_buffer, //float **    VertexBuffer,
        &VertexCount, //unsigned *    VertexCount,
        &texcoord_buffer, //float **    TexcoordBuffer,
        segs, //Segment *    Descriptor,
        descriptor_count, //unsigned    DescriptorCount,
        init_state, //MeshAccState * InitState,
        &LengthInXY, //float *        LengthInXYFace,
        NULL, //float *        MaxX,
        NULL, //float *        MinX,
        NULL, //float *        MaxY,
        NULL, //float *        MinY,
        10.0f, //float        ZStep,
        48//unsigned    ZSegmentCount
        );

    if ((NULL != vertex_buffer) && (NULL != texcoord_buffer)) {
        LengthInZ = 480;

        glGenBuffers(1, &PositionGLBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
        glBufferData(GL_ARRAY_BUFFER, VertexCount * 3 * sizeof(float), vertex_buffer, GL_STATIC_DRAW);

        glGenBuffers(1, &TexcoordGLBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, TexcoordGLBuffer);
        glBufferData(GL_ARRAY_BUFFER, VertexCount * 2 * sizeof(float), texcoord_buffer, GL_STATIC_DRAW);
    }

    if (NULL != vertex_buffer) free(vertex_buffer);
    if (NULL != texcoord_buffer) free(texcoord_buffer);

    data_s = malloc(sizeof(unsigned short) * VertexCount);
    if (NULL == data_s) {
        printf("out-of-memory\n");
        return VDKS_FALSE;
    }

    TrianglesVertexCount = VertexCount;

    for(i = 0; i < VertexCount; i++)
    {
        data_s[i] = i;
    }

    glGenBuffers(1, &TriangleGLBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * TrianglesVertexCount, data_s, GL_STATIC_DRAW);

    free(init_state);
    free(data_s);
    free(segs);

    return VDKS_TRUE;
}

void GenerateTexture()
{
    char szBMPFile[MAX_PATH + 1];
#ifndef ANDROID
    VDKS_Func_GetCurrentDir(szBMPFile);
#else
    strcpy(szBMPFile, "/sdcard/sample/sample7/");
#endif
    strcat(szBMPFile, "Mars_POSITIVE_Z_MIPMAP_LEVEL_0.bmp");
    TextureObject = VDKS_Func_Make2DTexture(szBMPFile, NULL, NULL);

    assert(TextureObject);
}


VDKS_BOOL SlideInit()
{
    char szCurDir[MAX_PATH + 1];
    char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1];
#ifndef ANDROID
    VDKS_Func_GetCurrentDir(szCurDir);
#else
    strcpy(szCurDir, "/sdcard/sample/sample7/");
#endif
    strcpy(szVSFile, szCurDir);
    strcpy(szFGFile, szCurDir);
    strcat(szVSFile, VSFile);
    strcat(szFGFile, FGFile);

    GenerateTexture();

    if (!GeneratePosition()) return VDKS_FALSE;

    /*
    * Matrix
    */
    do
    {
        float view    [3] = {320.0, 0.0, 2000};
        float focus [3] = {320.0, 0.0, 0.0};

        float up    [3] = {0.0, 1.0, 0.0};

        /*
        * Assume
        */
        //assert (CenterX == 0.0f);
        //assert (CenterY == 0.0f);
        //assert (CenterZ == 0.0f);
        //assert (Radius < 200.0f);

        VDKS_Func_LoadIdentity(ModelView);
        VDKS_Func_LoadIdentity(Projection);
        VDKS_Func_LoadIdentity(MVP);

        VDKS_Func_Matrix_Translate(ModelView, 0.0f, 0.0f, -240.0f);

        VDKS_Func_Matrix_Rotate(ModelView, 90.0f, -1.0f, 0.0f, 0.0f);

        VDKS_Func_Matrix_LookAt(view, focus, up, ModelView);


        //VDKS_Func_Matrix_Ortho(
        //    0.0f, 640.0f, /*left, right*/
        //    -240.0f, 240.0f, /*bottom, up*/
        //    1.0, 4000.0,      /*near, far*/
        //    Projection);

        VDKS_Func_Matrix_Frustum(
            Projection,
            -1.0 * 320.0f/* left */,
            320.0f/* right */,
            -1.0 * 240.0f/* bottom */,
            240.0f/* top */,
            2000.0f - SIN_A/* nearval */,
            5000.0f/* farval */);


        VDKS_Func_Matrix_Mul4by4(MVP, Projection, ModelView);

        //VDKS_Func_Translate(ModelView, -1.0 * CenterX, -1.0 * CenterY, -1.0 * CenterZ);
    }
    while(0);

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

        glBindAttribLocation(Program, PositionVertexAttributeArrayIndex, "position");
        glBindAttribLocation(Program, TexcoordVertexAttributeArrayIndex, "texcoord");

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
                //assert(type == GL_FLOAT);
                assert(type == GL_FLOAT_VEC3);

                location = glGetAttribLocation(Program, name);

                assert(location == PositionVertexAttributeArrayIndex);
            }
            else if (!strcmp(name, "texcoord"))
            {
                assert(size == 1);
                //assert(type == GL_FLOAT);
                assert(type == GL_FLOAT_VEC2);

                location = glGetAttribLocation(Program, name);

                assert(location ==  TexcoordVertexAttributeArrayIndex);
            }
            else
            {
                printf("The uniform name is not in handling.");
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

                location = glGetUniformLocation(Program, name);

                glUniform1i(location, 0);
            }

            else if(!strcmp(name, "time"))
            {

                assert(size == 1);
                assert(type == GL_FLOAT);

                TimeLocation = glGetUniformLocation(Program, name);
            }
            else if(!strcmp(name, "scale"))
            {

                assert(size == 1);
                assert(type == GL_FLOAT);

                location = glGetUniformLocation(Program, name);

                glUniform1f(location, 5.0f);
            }
            else if(!strcmp(name, "length_in_xy"))
            {

                assert(size == 1);
                assert(type == GL_FLOAT);

                location = glGetUniformLocation(Program, name);

                glUniform1f(location, LengthInXY);
            }
            else if(!strcmp(name, "length_in_z"))
            {

                assert(size == 1);
                assert(type == GL_FLOAT);

                location = glGetUniformLocation(Program, name);

                glUniform1f(location, LengthInZ);
            }
            //offset_in_texcoord
            else if(!strcmp(name, "offset_in_texcoord"))
            {

                assert(size == 1);
                assert(type == GL_FLOAT);

                OffsetInTexcoordLocation = glGetUniformLocation(Program, name);
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

void SlideRun ()
{
    static float offset_in_texcoord = 0.0f;

    passed_time += 0.05f;

    offset_in_texcoord += LengthInXY / VertexCount;

    if (offset_in_texcoord > LengthInXY)
    {
        offset_in_texcoord = 0.0f;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
    glVertexAttribPointer(PositionVertexAttributeArrayIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(PositionVertexAttributeArrayIndex);


    glBindBuffer(GL_ARRAY_BUFFER, TexcoordGLBuffer);
    glVertexAttribPointer(TexcoordVertexAttributeArrayIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(TexcoordVertexAttributeArrayIndex);

    glActiveTexture(GL_TEXTURE0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, TextureObject);

    glUseProgram(Program);

    glUniform1f(TimeLocation, passed_time);

    glUniform1f(OffsetInTexcoordLocation, offset_in_texcoord);

    glDrawElements(GL_TRIANGLES, TrianglesVertexCount, GL_UNSIGNED_SHORT, 0);
}

/*
*    Make mesh
*/

void MakeMesh (
    float **    VertexBuffer,
    unsigned *    VertexCount,
    float **    TexcoordBuffer,
    Segment *    Descriptor,
    unsigned    DescriptorCount,
    MeshAccState * InitState,
    float *        LengthInXYFace,
    float *        MaxX,
    float *        MinX,
    float *        MaxY,
    float *        MinY,
    float        ZStep,
    unsigned    ZSegmentCount
    )
{
    unsigned i;

    MeshAccState state;

    /* Miscelleneous Information */

    float max_x = InitState->acc_position.x;
    float max_y = InitState->acc_position.y;
    float min_x = InitState->acc_position.x;
    float min_y = InitState->acc_position.y;

    float length_in_xy = 0.0f;

    /* data of x,y face */
    float *   vertex_buffer = NULL;
    unsigned  vertex_count = 0;

    float *   texcoord_buffer = NULL;

    /* mesh acc state */
    memcpy(&state, InitState, sizeof(MeshAccState));
    state.acc_length = 0.0f;

    /* vertex buffer */
    vertex_count = DescriptorCount + 1;

    vertex_buffer = malloc(sizeof(float) * vertex_count * 2);
    if (NULL == vertex_buffer) return;

    texcoord_buffer = malloc(sizeof(float) * vertex_count);
    if (NULL == texcoord_buffer) {
        free(vertex_buffer);
        return;
    }

    vertex_buffer [0] = InitState->acc_position.x;
    vertex_buffer [1] = InitState->acc_position.y;

    texcoord_buffer[0] = 0.0f;

    for(i = 0; i < DescriptorCount; i++)
    {
        Segment *next_seg = &(Descriptor[i]);

        float x_offset = (float)cos(next_seg->angle) * next_seg->length;

        float y_offset = (float)sin(next_seg->angle) * next_seg->length;

        vertex_buffer[(i + 1) * 2 + 0] = state.acc_position.x + x_offset;

        state.acc_position.x += x_offset;

        vertex_buffer[(i + 1) * 2 + 1] = state.acc_position.y + y_offset;

        state.acc_position.y += y_offset;

        texcoord_buffer[i + 1] = state.acc_length + next_seg->length;

        state.acc_length += next_seg->length;

        max_x = max(max_x, state.acc_position.x);
        max_y = max(max_y, state.acc_position.y);

        min_x = min(min_x, state.acc_position.x);
        min_y = min(min_y, state.acc_position.y);
    }

    length_in_xy = state.acc_length;


    *VertexBuffer = vertex_buffer;
    *TexcoordBuffer = texcoord_buffer;
    *VertexCount = vertex_count;

    //for(i = 0; i < vertex_count; i++)
    //{
    //    printf("vertex_buffer : %d:\t%f, %f\n",i, vertex_buffer[i * 2 + 0], vertex_buffer[i * 2 + 1]);
    //}

    //for(i = 0; i < vertex_count; i++)
    //{
    //    printf("texcoord_buffer : %d:\t%f\n",i, texcoord_buffer[i]);
    //}

    /* Make Z */

    if (ZSegmentCount != 0)
    {
        unsigned i = 0;
        unsigned j = 0;

        unsigned vertex_pointer = 0;
        unsigned vertex_count = ZSegmentCount * DescriptorCount * 2 * 3;

        float * vertex_buffer_3d = NULL;
        float * texcoord_buffer_3d = NULL;

        vertex_buffer_3d = malloc(sizeof(float) * vertex_count * 3);
        if (NULL == vertex_buffer_3d) {
            free(*VertexBuffer);
            *VertexBuffer = NULL;
            free(*TexcoordBuffer);
            *TexcoordBuffer = NULL;
            return;
        }

        texcoord_buffer_3d = malloc(sizeof(float) * vertex_count * 2);
        if (NULL == texcoord_buffer_3d) {
            free(*VertexBuffer);
            *VertexBuffer = NULL;
            free(*TexcoordBuffer);
            *TexcoordBuffer = NULL;
            free(vertex_buffer_3d);
            return;
        }

        for (i = 0; i < ZSegmentCount ; i++)
        {
            float z = i * ZStep;
            float next_z = (i + 1) * ZStep;

            for (j = 0; j < DescriptorCount; j++)
            {
                /* triangle 1 */
                vertex_buffer_3d[vertex_pointer * 3 + 0] = vertex_buffer[j * 2 + 0];
                vertex_buffer_3d[vertex_pointer * 3 + 1] = vertex_buffer[j * 2 + 1];
                vertex_buffer_3d[vertex_pointer * 3 + 2] = z;

                texcoord_buffer_3d[vertex_pointer * 2 + 0] = texcoord_buffer[j];
                texcoord_buffer_3d[vertex_pointer * 2 + 1] = z;

                vertex_pointer ++;

                vertex_buffer_3d[vertex_pointer * 3 + 0] = vertex_buffer[(j + 1) * 2 + 0];
                vertex_buffer_3d[vertex_pointer * 3 + 1] = vertex_buffer[(j + 1) * 2 + 1];
                vertex_buffer_3d[vertex_pointer * 3 + 2] = z;

                texcoord_buffer_3d[vertex_pointer * 2 + 0] = texcoord_buffer[j + 1];
                texcoord_buffer_3d[vertex_pointer * 2 + 1] = z;

                vertex_pointer ++;

                vertex_buffer_3d[vertex_pointer * 3 + 0] = vertex_buffer[(j + 1) * 2 + 0];
                vertex_buffer_3d[vertex_pointer * 3 + 1] = vertex_buffer[(j + 1) * 2 + 1];
                vertex_buffer_3d[vertex_pointer * 3 + 2] = next_z;

                texcoord_buffer_3d[vertex_pointer * 2 + 0] = texcoord_buffer[j + 1];
                texcoord_buffer_3d[vertex_pointer * 2 + 1] = next_z;

                vertex_pointer ++;

                /* triangle 2 */

                vertex_buffer_3d[vertex_pointer * 3 + 0] = vertex_buffer[(j + 1) * 2 + 0];
                vertex_buffer_3d[vertex_pointer * 3 + 1] = vertex_buffer[(j + 1) * 2 + 1];
                vertex_buffer_3d[vertex_pointer * 3 + 2] = next_z;

                texcoord_buffer_3d[vertex_pointer * 2 + 0] = texcoord_buffer[j + 1];
                texcoord_buffer_3d[vertex_pointer * 2 + 1] = next_z;

                vertex_pointer ++;

                vertex_buffer_3d[vertex_pointer * 3 + 0] = vertex_buffer[j * 2 + 0];
                vertex_buffer_3d[vertex_pointer * 3 + 1] = vertex_buffer[j * 2 + 1];
                vertex_buffer_3d[vertex_pointer * 3 + 2] = next_z;

                texcoord_buffer_3d[vertex_pointer * 2 + 0] = texcoord_buffer[j];
                texcoord_buffer_3d[vertex_pointer * 2 + 1] = next_z;

                vertex_pointer ++;

                vertex_buffer_3d[vertex_pointer * 3 + 0] = vertex_buffer[j * 2 + 0];
                vertex_buffer_3d[vertex_pointer * 3 + 1] = vertex_buffer[j * 2 + 1];
                vertex_buffer_3d[vertex_pointer * 3 + 2] = z;

                texcoord_buffer_3d[vertex_pointer * 2 + 0] = texcoord_buffer[j];
                texcoord_buffer_3d[vertex_pointer * 2 + 1] = z;

                vertex_pointer ++;

            }
        }

        assert(vertex_pointer == vertex_count);

        *VertexBuffer = vertex_buffer_3d;
        *TexcoordBuffer = texcoord_buffer_3d;
        *VertexCount = vertex_count;


        //for(i = 0; i < vertex_count; i++)
        //{
        //    printf("vertex_buffer_3d : %d:\t%f, %f, %f\n", i, vertex_buffer_3d[i * 3 + 0], vertex_buffer_3d[i * 3 + 1], vertex_buffer_3d[i * 3 + 2]);
        //}

        //for(i = 0; i < vertex_count; i++)
        //{
        //    printf("texcoord_buffer_3d : %d:\t%f, %f\n",i, texcoord_buffer_3d[i * 2 + 0], texcoord_buffer_3d[i * 2 + 1]);
        //}

        free(vertex_buffer);
        free(texcoord_buffer);


    } /* if (ZSegmentCount != 0) */

    if(MaxX)
    {
        *MaxX = max_x;
    }

    if(MinX)
    {
        *MinX = min_x;
    }

    if(MaxY)
    {
        *MaxY = max_y;
    }

    if(MinY)
    {
        *MinY = min_y;
    }

    if(LengthInXYFace)
    {
        *LengthInXYFace = length_in_xy;
    }

}

void MakeSinDescriptors(
    float A,
    float T,
    float InitPhase,
    float ShadowOnX,
    float Step,
    Segment ** DescriptorSet,
    unsigned * DescriptorCount
    )
{
    float current_x = 0.0f;
    unsigned counter = 0;

    Segment * rt = malloc(sizeof(Segment) * MAX_DESCRIPTOR_COUNT);
    if (NULL == rt) {
        printf("out-of-memory\n");
        return;
    }

    while (current_x < ShadowOnX && counter < MAX_DESCRIPTOR_COUNT)
    {
        Segment * new_seg = &(rt[counter]);

        float differential_coefficient = 0.0f;

        float angle = 0.0f;

        differential_coefficient = GetDyDxSin(A, T, InitPhase, current_x);
        angle = (float)atan(differential_coefficient);

        new_seg->angle = angle;
        new_seg->length = Step;

        current_x += (float)cos(angle) * Step;

        counter++;
    }

    *DescriptorSet = rt;
    *DescriptorCount = counter;
}
