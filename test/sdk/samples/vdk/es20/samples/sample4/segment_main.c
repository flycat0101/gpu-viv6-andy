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
 * VDKS
*/
#ifndef ANDROID
vdkEGL VdkEgl;

int VDKS_Val_WindowsWidth = 0;
int VDKS_Val_WindowsHeight = 0;

char* VDKS_ARG0 = NULL;
#endif
int SaveResult = 1;

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
 * Test Data
*/

#define TEST_SIZE (0.3f)
static const float PositionData [] =
{
    TEST_SIZE, TEST_SIZE, 0.0,
    TEST_SIZE, -TEST_SIZE, 0.0,
    -TEST_SIZE, -TEST_SIZE, 0.0,
    -TEST_SIZE, TEST_SIZE, 0.0
};

static GLuint    PositionGLBuffer = 0;
static GLuint    TexcoordGLBuffer = 0;
#ifndef ANDROID
const static GLuint    PositionVertexAttributeArrayIndex = 0;
const static GLuint    TexcoordVertexAttributeArrayIndex = 1;
#else
static const GLuint PositionVertexAttributeArrayIndex = 0;
static const GLuint TexcoordVertexAttributeArrayIndex = 1;
#endif
static const unsigned short TriangleData [] =
{
    0, 1, 2, 3, 0, 2
};

static GLuint    TriangleGLBuffer = 0;
static GLuint    TriangleCount = 0;

static float    CenterX = 0.0f;
static float    CenterY = 0.0f;
static float    CenterZ = 0.0f;

static float    Radius    = 0.0f;

/*
*    Shader
*/
//const char *    VSFile = "test.vert";
//const char *    FGFile = "test.frag";

const char *    VSFile = "segment.vert";
const char *    FGFile = "segment.frag";

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

float passed_time = 0.0f;

GLuint TimeLocation = 0;

float LengthInXY = 0.0f;

unsigned VertexCount = 0;

GLuint OffsetInTexcoordLocation = 0;

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


void GeneratePosition_2()
{
    const int desc_count = 1024;

    unsigned i = 0;

    unsigned short * data_s = NULL;

    float length_in_xy = 0.0f;


    float * vertex_buffer = NULL;
    float * texcoord_buffer = NULL;

    unsigned vertex_count = 0;

    Segment *       desc_set = malloc(sizeof(Segment) * desc_count);
    MeshAccState * init_state = malloc(sizeof(MeshAccState));
    if ((NULL == desc_set) || (NULL == init_state)) {
        printf("out-of-memory\n");
        if(desc_set) free(desc_set);
        if (init_state) free(init_state);
        return;
    }

    init_state->acc_position.x = 0.0f;
    init_state->acc_position.y = 0.0f;

    init_state->acc_length = 0.0f;

    desc_set[0].length= 1.0f;

    desc_set[0].angle = VDKS_PI * 1 / 3;

    for(i = 0; i < 200; i++)
    {
        if (i < 100)
        {
            desc_set[i].angle = (VDKS_PI * 1 / 4.f);
            desc_set[i].length = 0.05f;
        }
        else
        {
            desc_set[i].angle = -1.0f * i * (VDKS_PI * 1 / 200.f);
            desc_set[i].length = 0.05f;
        }
    }

    VertexCount = 200;

    MakeMesh (
        &vertex_buffer, //float **    VertexBuffer,
        &vertex_count, //unsigned *    VertexCount,
        &texcoord_buffer, //float **    TexcoordBuffer,
        desc_set, //Segment *    Descriptor,
        VertexCount, //unsigned    DescriptorCount,
        init_state, //MeshAccState * InitState,
        &length_in_xy, //float *        LengthInXYFace,
        NULL, //float *        MaxX,
        NULL, //float *        MinX,
        NULL, //float *        MaxY,
        NULL, //float *        MinY,
        0.0f, //float        ZStep,
        0//unsigned    ZSegmentCount
        );

    if ((NULL == vertex_buffer) || (NULL == texcoord_buffer)) {
        if (NULL != vertex_buffer) free(vertex_buffer);
        if (NULL != texcoord_buffer) free(texcoord_buffer);
        return;
    }

    LengthInXY = length_in_xy;

    VertexCount = vertex_count;

    glGenBuffers(1, &PositionGLBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 2 * sizeof(float), vertex_buffer, GL_STATIC_DRAW);

    glGenBuffers(1, &TexcoordGLBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, TexcoordGLBuffer);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * 1 * sizeof(float), texcoord_buffer, GL_STATIC_DRAW);

    free(vertex_buffer);
    free(texcoord_buffer);

    data_s = malloc(sizeof(unsigned short) * vertex_count);
    if (NULL == data_s) {
        printf("out-of-memory\n");
        return;
    }

    TriangleCount = vertex_count;

    for(i = 0; i < vertex_count; i++)
    {
        data_s[i] = i;
    }

    glGenBuffers(1, &TriangleGLBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * vertex_count, data_s, GL_STATIC_DRAW);

    free(data_s);
}
void GenerateTexture()
{
    unsigned char * texels  = (unsigned char *) malloc(sizeof(unsigned char) * 64 * 3);
    int i;
    if(texels == NULL)
    {
        printf("GenerateTexture : out-of-memory\n");
        return;
    }



    for(i = 0; i < 64; i++)
    {
        //if(i == 0 || i == 63)
        //{
        //    texels[i * 3 + 0] = 0;
        //    texels[i * 3 + 1] = 0;
        //    texels[i * 3 + 2] = 0;
        //}
        //else
        {
            texels[i * 3 + 0] = 0;
            texels[i * 3 + 1] = 255;
            texels[i * 3 + 2] = 0;
        }
    }

    glGenTextures(1, &TextureObject);
    glBindTexture(GL_TEXTURE_2D, TextureObject);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, texels);
}
void GeneratePosition()
{
    unsigned i;
    unsigned count = 100;

    unsigned short * data_s = NULL;

    float * data = malloc(count * 1 * sizeof(float));
    if (data == NULL) return;
    for(i = 0; i < count; i++)
    {
        data[i] = 10.0f * i / count;
        //data[i] -= 1.0f;
    }

    glGenBuffers(1, &PositionGLBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
    glBufferData(GL_ARRAY_BUFFER, count * 1 * sizeof(float), data, GL_STATIC_DRAW);

    free(data);

    data_s = malloc(sizeof(unsigned short) * count);
    if (data_s == NULL) return;
    for(i = 0; i < count; i++)
    {
        data_s[i] = i;
    }

    glGenBuffers(1, &TriangleGLBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * count, data_s, GL_STATIC_DRAW);

    free(data_s);
}

VDKS_BOOL VDKS_TestInit()
{
    char szCurDir[MAX_PATH + 1];
    char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1];
#ifndef ANDROID
    VDKS_Func_GetCurrentDir(szCurDir);
#else
    strcpy(szCurDir,"/sdcard/sample/sample4/");
#endif
    strcpy(szVSFile, szCurDir);
    strcpy(szFGFile, szCurDir);
    strcat(szVSFile, VSFile);
    strcat(szFGFile, FGFile);

    //GeneratePosition();
    GeneratePosition_2();
    GenerateTexture();

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

        //VDKS_Func_Matrix_Rotate(ModelView, 88.0f, 0.0f, 1.0f, 1.0f);

        VDKS_Func_Matrix_LookAt(view, focus, up, ModelView);

        Radius = 10.0f;

        //VDKS_Func_Matrix_Ortho(
        //    -1.0f * Radius, Radius,    /*left, right*/
        //    -1.0f * Radius, Radius,    /*bottom, up*/
        //    1.0, 400.0,                /*near, far*/
        //    Projection);

        VDKS_Func_Matrix_Ortho(
            0.0f, Radius, /*left, right*/
            0.0f, Radius, /*bottom, up*/
            1.0, 400.0,      /*near, far*/
            Projection);


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
        int        index;
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
                assert(type == GL_FLOAT_VEC2);

                location = glGetAttribLocation(Program, name);

                assert(location == PositionVertexAttributeArrayIndex);
            }
            //texcoord
            else if (!strcmp(name, "texcoord"))
            {
                assert(size == 1);
                //assert(type == GL_FLOAT);
                assert(type == GL_FLOAT);

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

        GLint i;
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
            else if(!strcmp(name, "length_in_xy"))
            {

                assert(size == 1);
                assert(type == GL_FLOAT);

                location = glGetUniformLocation(Program, name);

                glUniform1f(location, LengthInXY);
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

void TestRun ()
{
    static float offset_in_texcoord = 0.0f;

    passed_time += 0.1f;

    offset_in_texcoord += 3 * (LengthInXY / VertexCount);

    if (offset_in_texcoord > LengthInXY)
    {
        offset_in_texcoord = 0.0f;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
    //glVertexAttribPointer(PositionVertexAttributeArrayIndex, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(PositionVertexAttributeArrayIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(PositionVertexAttributeArrayIndex);


    glBindBuffer(GL_ARRAY_BUFFER, TexcoordGLBuffer);
    //glVertexAttribPointer(PositionVertexAttributeArrayIndex, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(TexcoordVertexAttributeArrayIndex, 1, GL_FLOAT, GL_FALSE, 0, 0);
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


    //glDrawElements(GL_TRIANGLES, sizeof(TriangleData) / sizeof(unsigned short), GL_UNSIGNED_SHORT, 0);
    glDrawElements(GL_POINTS, TriangleCount, GL_UNSIGNED_SHORT, 0);

    //VDKS_Func_SaveScreen("result.bmp");
#ifndef ANDROID
    vdkSwapEGL(&VdkEgl);
#endif
}

extern VDKS_BOOL    SphereInit();
extern void            SphereRun ();

VDKS_BOOL Init()
{
    return VDKS_TestInit();
}

void Run ()
{
    TestRun();
}
#ifndef ANDROID
int main(int argc, char * argv[])
{
    int i = 0;
    int count = 1;

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

    /*
     * VDKS
    */
    VDKS_ARG0 = argv[0];

    memset(&VdkEgl, 0, sizeof(vdkEGL));

    VDKS_Init(&VdkEgl);

    vdkShowWindow(VdkEgl.window);

    /*
     * App Data
    */

    if (VDKS_TRUE != Init())
    {
        printf("main : Failed to init case.");
        return 1;
    }

    for(i = 0; i < count; i++)
    {
        Run();
    }

    vdkFinishEGL(&VdkEgl);

    return 0;
}

#endif

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
                vertex_buffer_3d[vertex_pointer + 0] = vertex_buffer[j * 2 + 0];
                vertex_buffer_3d[vertex_pointer + 1] = vertex_buffer[j * 2 + 1];
                vertex_buffer_3d[vertex_pointer + 2] = z;

                texcoord_buffer_3d[vertex_pointer + 0] = vertex_buffer[j];
                texcoord_buffer_3d[vertex_pointer + 1] = z;

                vertex_pointer ++;

                vertex_buffer_3d[vertex_pointer + 0] = vertex_buffer[(j + 1) * 2 + 0];
                vertex_buffer_3d[vertex_pointer + 1] = vertex_buffer[(j + 1) * 2 + 1];
                vertex_buffer_3d[vertex_pointer + 2] = z;

                texcoord_buffer_3d[vertex_pointer + 0] = vertex_buffer[j];
                texcoord_buffer_3d[vertex_pointer + 1] = z;

                vertex_pointer ++;

                vertex_buffer_3d[vertex_pointer + 0] = vertex_buffer[(j + 1) * 2 + 0];
                vertex_buffer_3d[vertex_pointer + 1] = vertex_buffer[(j + 1) * 2 + 1];
                vertex_buffer_3d[vertex_pointer + 2] = next_z;

                texcoord_buffer_3d[vertex_pointer + 0] = vertex_buffer[j];
                texcoord_buffer_3d[vertex_pointer + 1] = next_z;

                vertex_pointer ++;

                /* triangle 2 */

                vertex_buffer_3d[vertex_pointer + 0] = vertex_buffer[(j + 1) * 2 + 0];
                vertex_buffer_3d[vertex_pointer + 1] = vertex_buffer[(j + 1) * 2 + 1];
                vertex_buffer_3d[vertex_pointer + 2] = next_z;

                texcoord_buffer_3d[vertex_pointer + 0] = vertex_buffer[j];
                texcoord_buffer_3d[vertex_pointer + 1] = next_z;

                vertex_pointer ++;

                vertex_buffer_3d[vertex_pointer + 0] = vertex_buffer[j * 2 + 0];
                vertex_buffer_3d[vertex_pointer + 1] = vertex_buffer[j * 2 + 1];
                vertex_buffer_3d[vertex_pointer + 2] = next_z;

                texcoord_buffer_3d[vertex_pointer + 0] = vertex_buffer[j];
                texcoord_buffer_3d[vertex_pointer + 1] = next_z;

                vertex_pointer ++;

                vertex_buffer_3d[vertex_pointer + 0] = vertex_buffer[j * 2 + 0];
                vertex_buffer_3d[vertex_pointer + 1] = vertex_buffer[j * 2 + 1];
                vertex_buffer_3d[vertex_pointer + 2] = z;

                texcoord_buffer_3d[vertex_pointer + 0] = vertex_buffer[j];
                texcoord_buffer_3d[vertex_pointer + 1] = z;

                vertex_pointer ++;

            }
        }

        assert(vertex_pointer == (vertex_count * 3));

        if (NULL != vertex_buffer) free (vertex_buffer);
        free(texcoord_buffer);

        *VertexBuffer = vertex_buffer_3d;
        *TexcoordBuffer = texcoord_buffer_3d;
        *VertexCount = vertex_count;

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

    //return VDKS_TRUE;
}

