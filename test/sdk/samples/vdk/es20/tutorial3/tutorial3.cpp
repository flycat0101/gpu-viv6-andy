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


/*
 * OpenGL ES 2.0 Tutorial 3
 *
 * Draws a glass sphere inside a big sphere (enviroment mapping).
 * The glass sphere shows reflection effects.
 */

#include <GLES2/gl2.h>
#ifndef ANDROID_JNI
#include <gc_vdk.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "commonMath.h"

#define TUTORIAL_NAME "OpenGL ES 2.0 Tutorial 3"
// to hold vdk information.
#ifndef ANDROID_JNI
vdkEGL egl;
#endif
int width   =  0;
int height  =  0;
#ifndef ANDROID_JNI
int posX    = -1;
int posY    = -1;
int samples =  0;
int frames  =  0;
#endif
int fast    =  0;

int argCount = 6;
char argSpec = '-';
char argNames[] = {'x', 'y', 'w', 'h', 's', 'f', 'F'};
char argValues[][255] = {
    "x_coord",
    "y_coord",
    "width",
    "height",
    "samples",
    "frames",
    "fast",
};
char argDescs[][255] = {
    "x coordinate of the window, default is -1(screen center)",
    "y coordinate of the window, default is -1(screen center)",
    "width  of the window in pixels, default is 0(fullscreen)",
    "height of the window in pixels, default is 0(fullscreen)",
    "sample count for MSAA, 0/2/4, default is 0 (no MSAA)",
    "frames to run, default is 0 (no frame limit, escape to exit)",
    "fast mode",
};
int noteCount = 1;
char argNotes[][255] = {
    "Exit: [ESC] or the frame count reached."
};
// Global Variables
GLuint programHandleGlass;
GLuint programHandleBgrnd;

// Generic used buffer
GLuint bufs[5];

// hold sphere data
int numVertices, numFaces, numIndices;
GLfloat* sphereVertices = NULL;
GLushort* indices = NULL;

// attribute and uniform handle.
GLint locVertices[2];
GLint locTransformMat[2];
GLint locRadius[2];
GLint locEyePos[2];
GLint locSamplerCb[2];

// render state
State renderState;

D3DXMATRIX transformMatrix;

/***************************************************************************************
***************************************************************************************/

void RenderInit()
{
    Sphere(&sphereVertices, 40, 60, 0, NULL, 0, NULL,
           &indices, &numFaces, &numIndices, &numVertices);

    // Grab location of shader attributes.
    locVertices[0] = glGetAttribLocation(programHandleGlass, "my_Vertex");
    locVertices[1] = glGetAttribLocation(programHandleBgrnd, "my_Vertex");

    locRadius[0]   = glGetUniformLocation(programHandleGlass, "my_Radius");
    locRadius[1]   = glGetUniformLocation(programHandleBgrnd, "my_Radius");

    locEyePos[0]   = glGetUniformLocation(programHandleGlass, "my_EyePos");
    locEyePos[1]   = glGetUniformLocation(programHandleBgrnd, "my_EyePos");

    // Transform Matrix is uniform for all vertices here.
    locTransformMat[0] = glGetUniformLocation(programHandleGlass, "my_TransformMatrix");
    locTransformMat[1] = glGetUniformLocation(programHandleBgrnd, "my_TransformMatrix");

    glEnableVertexAttribArray(locVertices[0]);
    glEnableVertexAttribArray(locVertices[1]);

    glGenBuffers(2, bufs);
    if (glGetError() != GL_NO_ERROR)
    {
        // can not handle this error
        fprintf(stderr, "GL Error.\n");
        return;
    }

    // Vertices
    glBindBuffer(GL_ARRAY_BUFFER, bufs[0]);
    glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(GLfloat), sphereVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(locVertices[0], 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(locVertices[1], 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(GLushort), indices, GL_STATIC_DRAW);

    if (glGetError() != GL_NO_ERROR)
    {
        // can not handle this error
        fprintf(stderr, "GL Error.\n");
        return;
    }

    // Textures
    // Set s0 for samplerCube location.
    locSamplerCb[0] = glGetUniformLocation(programHandleGlass, "samplerCb");
    locSamplerCb[1] = glGetUniformLocation(programHandleBgrnd, "samplerCb");

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
#ifndef ANDROID_JNI
    CubeTexture* cubeTexData = LoadCubeDDS("stpeters_cross_mipmap_256.dds");
#else
    CubeTexture* cubeTexData = LoadCubeDDS("/sdcard/tutorial/tutorial3/stpeters_cross_mipmap_256.dds");
#endif
    if (glGetError() != GL_NO_ERROR || cubeTexData == NULL)
    {
        // can not handle this error
        fprintf(stderr, "GL Error.\n");
        return;
    }

    int texSize = cubeTexData->img_size;

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureHandle);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->posx);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->negx);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->posy);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->negy);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->posz);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, texSize, texSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, cubeTexData->negz);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    if (glGetError() != GL_NO_ERROR)
    {
        // can not handle this error
        fprintf(stderr, "GL Error.\n");
        return;
    }

    // Deletes the texture data, it's now in OpenGL memory.
    DeleteCubeTexture(cubeTexData);

    InitializeRenderState(&renderState, width, height);

    // Set eye position.
    SetEye(&renderState, 0, 0, -3.8f);

    // Enable needed states.
    glEnable(GL_CULL_FACE);

    if (fast)
    {
        glDisable(GL_DEPTH_TEST);
        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_FALSE);
    }
    else
    {
        glEnable(GL_DEPTH_TEST);
    }
}

// Actual rendering here.
void Render()
{
    SetupTransform(&renderState, &transformMatrix);

    if (!fast)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if (fast)
    {
        // Render background sphere.
        glCullFace(GL_BACK);
        glUseProgram(programHandleBgrnd);
        glUniform1i(locSamplerCb[1], 0);
        glUniform1f(locRadius[1],   10.0f);
        glUniform3fv(locEyePos[1], 1, (GLfloat *) &renderState.m_EyeVector);
        glUniformMatrix4fv(locTransformMat[1], 1, GL_FALSE, (GLfloat *) &transformMatrix);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    }

    // Render mirror ball.
    glCullFace(GL_FRONT);
    glUseProgram(programHandleGlass);
    glUniform1i(locSamplerCb[0], 0);
    glUniform1f(locRadius[0], 1.0f);
    glUniform3fv(locEyePos[0], 1, (GLfloat *) &renderState.m_EyeVector);
    glUniformMatrix4fv(locTransformMat[0], 1, GL_FALSE, (GLfloat *) &transformMatrix);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);

    if (!fast)
    {
        // Render background sphere.
        glCullFace(GL_BACK);
        glUseProgram(programHandleBgrnd);
        glUniform1i(locSamplerCb[1], 0);
        glUniform1f(locRadius[1], 10.0f);
        glUniform3fv(locEyePos[1], 1, (GLfloat *) &renderState.m_EyeVector);
        glUniformMatrix4fv(locTransformMat[1], 1, GL_FALSE, (GLfloat *) &transformMatrix);
        glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    }

#ifndef ANDROID_JNI
    // swap display with drawn surface.
    vdkSwapEGL(&egl);
#endif
}

void RenderCleanup()
{
    // cleanup
    glDisableVertexAttribArray(locVertices[0]);
    glDisableVertexAttribArray(locVertices[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(2, bufs);

    if (sphereVertices != NULL)
    {
        free(sphereVertices);
    }

    if (indices != NULL)
    {
        free(indices);
    }
}

/***************************************************************************************
***************************************************************************************/
#ifndef ANDROID_JNI
int ParseCommandLine(int argc, char * argv[])
{
    int result = 1;
    // Walk all command line arguments.
    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            switch (argv[i][1])
            {
            case 'x':
                // x<x> for x position (defaults to -1).
                if (++i < argc)
                    posX = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'y':
                // y<y> for y position (defaults to -1).
                if (++i < argc)
                    posY = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'w':
                // w<width> for width (defaults to 0).
                if (++i < argc)
                    width = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'h':
                // h<height> for height (defaults to 0).
                if (++i < argc)
                    height = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 's':
                // s<samples> for samples (defaults to 0).
                if (++i < argc)
                    samples = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'f':
                // f<count> for number of frames (defaults to 0).
                if (++i < argc)
                    frames = atoi(&argv[i][0]);
                else
                    result = 0;
                break;

            case 'F':
                // Turn on Fast mode.
                fast = 1;
                break;

            default:
                result = 0;
                break;
            }

            if (result == 0)
            {
                break;
            }
        }
        else
        {
            result = 0;
            break;
        }
    }

    return result;
}

void PrintHelp()
{
    int i;
    printf("Usage: ");
    printf("command + [arguments]\n");
    printf("Argument List:\n");

    for (i = 0; i < argCount; i++)
    {
        printf("\t");
        printf("%c%c %s\t%s\n", argSpec, argNames[i], argValues[i], argDescs[i]);
    }

    for (i = 0; i < noteCount; i++)
    {
        printf("%s\n", argNotes[i]);
    }
}

/***************************************************************************************
***************************************************************************************/

// Program entry.
int main(int argc, char** argv)
{
    bool pauseFlag = false;
    // EGL configuration - we use 24-bpp render target and a 16-bit Z buffer.
    EGLint configAttribs[] =
    {
        EGL_SAMPLES,      0,
        EGL_RED_SIZE,     8,
        EGL_GREEN_SIZE,   8,
        EGL_BLUE_SIZE,    8,
        EGL_ALPHA_SIZE,   EGL_DONT_CARE,
        EGL_DEPTH_SIZE,   16,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE,
    };

    EGLint attribListContext[] =
    {
        // Needs to be set for es2.0 as default client version is es1.1 .
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    // Parse the command line.
    if (!ParseCommandLine(argc, argv))
    {
        PrintHelp();
        return 0;
    }

    // Set multi-sampling.
    configAttribs[1] = samples;

    // Initialize VDK, EGL, and GLES.
    if (!vdkSetupEGL(posX, posY, width, height, configAttribs, NULL, attribListContext, &egl))
    {
        return 1;
    }

    vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);

    // Set window title and show the window.
    vdkSetWindowTitle(egl.window, TUTORIAL_NAME);
    vdkShowWindow(egl.window);

    // load and compile vertex/fragment shaders.
    GLuint vertShaderNum, pixelShaderNum, pixelShaderNum2;

    vertShaderNum  = glCreateShader(GL_VERTEX_SHADER);
    pixelShaderNum = glCreateShader(GL_FRAGMENT_SHADER);
    pixelShaderNum2 = glCreateShader(GL_FRAGMENT_SHADER);

    do {
        if (CompileShader("vs_es20t3.vert", vertShaderNum) == 0)
        {
            break;
        }

        if (CompileShader("ps_es20t3_glass.frag", pixelShaderNum) == 0)
        {
            break;
        }

        if (CompileShader("ps_es20t3_bgrnd.frag", pixelShaderNum2) == 0)
        {
            break;
        }

        GLint linkStatus;

        programHandleGlass = glCreateProgram();
        glAttachShader(programHandleGlass, vertShaderNum);
        glAttachShader(programHandleGlass, pixelShaderNum);
        glLinkProgram(programHandleGlass);
        glGetProgramiv(programHandleGlass, GL_LINK_STATUS, &linkStatus);
        if (!linkStatus)
        {
            fprintf(stderr, "Glass program failed to link.\n");
            break;
        }

        programHandleBgrnd = glCreateProgram();
        glAttachShader(programHandleBgrnd, vertShaderNum);
        glAttachShader(programHandleBgrnd, pixelShaderNum2);
        glLinkProgram(programHandleBgrnd);
        glGetProgramiv(programHandleBgrnd, GL_LINK_STATUS, &linkStatus);
        if (!linkStatus)
        {
            fprintf(stderr, "Background program failed to link.\n");
            break;
        }

        if (programHandleGlass == 0 || programHandleBgrnd == 0)
        {
            break;
        }

        RenderInit();
        int frameCount = 0;
        unsigned long start = GetTimeMillis();

        // Main loop.
        for (bool done = false; !done;)
        {
            // Get an event.
            vdkEvent event;
            if (vdkGetEvent(egl.window, &event))
            {
                // Test for Keyboard event.
                if ((event.type == VDK_KEYBOARD)
                && event.data.keyboard.pressed
                )
                {
                    // Test for key.
                    switch (event.data.keyboard.scancode)
                    {
                    case VDK_SPACE:
                        // Use SPACE to pauseFlag
                        pauseFlag = !pauseFlag;
                        break;

                    case VDK_ESCAPE:
                        // Use ESCAPE to quit.
                        done = true;
                        break;
                    default:
                        break;
                    }
                }

                // Test for Close event.
                else if (event.type == VDK_CLOSE)
                {
                    done = true;
                }
            }
            else if (!pauseFlag)
            {
                // Render one frame if there is no event.
                Render();
                frameCount ++;

                if ((frames > 0) && (frameCount >= frames))
                {
                    done = true;
                }
            }
        }

        glFinish();

        unsigned long duration = GetTimeMillis() - start;
        float fps = 1000.0f * float(frameCount) / float(duration);

        printf("Rendered %d frames in %lu milliseconds: %.2f fps\n",
                frameCount, duration, fps);

        RenderCleanup();

        // cleanup
        glDeleteProgram(programHandleGlass);
        glDeleteProgram(programHandleBgrnd);
        glUseProgram(0);

        glDeleteShader(vertShaderNum);
        glDeleteShader(pixelShaderNum);
        glDeleteShader(pixelShaderNum2);
    }
    while (false);

    vdkFinishEGL(&egl);
    return 0;
}
#endif
