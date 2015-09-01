/****************************************************************************
*
*    Copyright 2012 - 2015 Vivante Corporation, Santa Clara, California.
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


#if defined(ANDROID_JNI)
#if ANDROID_SDK_VERSION >= 16
#include <gui/Surface.h>
#else
#include <surfaceflinger/Surface.h>
#endif
using namespace android;
#include <utils/Log.h>
#if ANDROID_SDK_VERSION >= 16
#   include <ui/ANativeObjectBase.h>

#   undef LOGI
#   undef LOGD
#   undef LOGW
#   undef LOGE
#   define LOGI(...) ALOGI(__VA_ARGS__)
#   define LOGD(...) ALOGD(__VA_ARGS__)
#   define LOGW(...) ALOGW(__VA_ARGS__)
#   define LOGE(...) ALOGE(__VA_ARGS__)
#else
#   include <ui/android_native_buffer.h>
#endif
#   include <gc_vdk.h>
#   include <pthread.h>
#define DEBUG_PRINT         LOGE
#elif defined(LINUX)
#   define DEBUG_PRINT         printf
#   include <gc_vdk.h>
#   include <pthread.h>
#elif defined(UNDER_CE)
#    define DEBUG_PRINT        printf
#    include <gc_vdk.h>
#    include <windows.h>
#else
#include <windows.h>
#define DEBUG_PRINT         printf
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef UNDER_CE
#include <sys/stat.h>
#endif
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#if defined(ANDROID_JNI) || defined(LINUX)
pthread_t   g_thread;
#else
HDC         g_hDC   = NULL;
HWND        g_hWnd  = NULL;
#endif

GLchar      fragSrc[] = "\
uniform vec4 uFragColor;\n\
uniform sampler2D sTex2D;\n\
varying vec2 vTexCoord;\n\
\n\
void main(void)\n\
{\n\
    gl_FragColor = texture2D(sTex2D, vTexCoord);\n\
}";

GLchar      vertSrc[] = "\
attribute vec4 aVertexCoord;\n\
attribute vec2 aTexCoord;\n\
\n\
varying vec2 vTexCoord;\n\
\n\
void main(void)\n\
{\n\
    vTexCoord   = aTexCoord;\n\
    gl_Position = aVertexCoord;\n\
}";

int         width;
int         height;
int         frames;


EGLDisplay  g_eglDisplay  = EGL_NO_DISPLAY;
EGLSurface  g_eglSurface1 = EGL_NO_SURFACE;
EGLSurface  g_eglSurface2 = EGL_NO_SURFACE;
EGLContext  g_eglContext1 = EGL_NO_CONTEXT;
EGLContext  g_eglContext2 = EGL_NO_CONTEXT;
EGLint      g_eglError    = EGL_SUCCESS;

GLint       g_glError  = GL_NO_ERROR;
GLuint      g_vbo;
GLuint      g_texture;
GLuint      g_program;
GLuint      g_vertShader;
GLuint      g_fragShader;
GLint       g_aLoc_vertexCoord;
GLint       g_aLoc_texCoord;
GLint       g_uLoc_useTex;
GLint       g_uLoc_fragColor;
GLint       g_sLoc_tex2D;
GLsizei     g_attribStride;

#define WINDOW_WIDTH                    320
#define WINDOW_HEIGHT                   240
#define BUFFER_OFFSET(i)                ((char *)NULL + (i))

#define MAKE_CURRENT_CTX1               eglMakeCurrent(g_eglDisplay, g_eglSurface1, g_eglSurface1, g_eglContext1)
#define MAKE_CURRENT_CTX2 \
    do\
    {\
        eglBindAPI(EGL_OPENGL_ES_API);\
        eglMakeCurrent(g_eglDisplay, g_eglSurface1, g_eglSurface1, g_eglContext2);\
    }\
    while (0)
#define LOSE_CURRENT                    eglMakeCurrent(g_eglDisplay, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_NO_SURFACE)
#define SWAP_BUFFER_CTX1                eglSwapBuffers(g_eglDisplay, g_eglSurface1)
#define SWAP_BUFFER_CTX2                eglSwapBuffers(g_eglDisplay, g_eglSurface1)

GLboolean LoadTexture()
{
    char        *pixels = (char*)malloc(sizeof(int) * 256);
    char        *dst    = pixels;

    width   = 16;
    height  = 16;

    if (pixels)
    {
        for (int i=0; i<height; i++)
        {
            for (int j=0; j<width; j++)
            {
                if (j < 8)
                {
                    *dst ++ = 0x00;
                    *dst ++ = 0xFF;
                    *dst ++ = 0x00;
                    *dst ++ = 0x00;
                }
                else
                {
                    *dst ++ = 0x00;
                    *dst ++ = 0x00;
                    *dst ++ = 0xFF;
                    *dst ++ = 0x00;
                }
            }
        }

        glGenTextures(1, &g_texture);
        glBindTexture(GL_TEXTURE_2D, g_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        free(pixels);
    }
    else
    {
        return GL_FALSE;
    }

    /* Success. */
    return GL_TRUE;
}

GLchar *readShaderFile(const char *fileName)
{
    FILE    *file = fopen(fileName, "r");
    GLint   len;

    if (file == NULL)
    {
        DEBUG_PRINT("Cannot open shader file!\n");
        return 0;
    }

    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);

    GLchar *buffer = (GLchar*)malloc(len);

    int bytes = fread(buffer, 1, len, file);

    buffer[bytes] = 0;

    fclose(file);

    return buffer;
}

void initEGL(int nativeWindow)
{
#if defined(ANDROID_JNI)
    int iConfigs;
    EGLConfig eglConfig = 0;
    EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    const EGLint pi32ConfigAttribs[] =
    {
        /*EGL_LEVEL,              0,*/
        EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
        /*EGL_NATIVE_RENDERABLE,  EGL_FALSE,*/
        EGL_RED_SIZE,           8,
        EGL_GREEN_SIZE,         8,
        EGL_BLUE_SIZE,          8,
        EGL_ALPHA_SIZE,         8,
        EGL_DEPTH_SIZE,         0,
        EGL_NONE
    };

    g_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(g_eglDisplay, NULL, NULL);
    eglBindAPI(EGL_OPENGL_ES_API);
    eglChooseConfig(g_eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs);
    if (iConfigs != 1)
    {
        DEBUG_PRINT("Cannot get required config\n");
        return;
    }

    g_eglSurface1 = eglCreateWindowSurface(g_eglDisplay, eglConfig,  (Surface *)nativeWindow, NULL);
    g_eglSurface2 = eglCreateWindowSurface(g_eglDisplay, eglConfig, (Surface *)nativeWindow, NULL);
    g_eglContext1 = eglCreateContext(g_eglDisplay, eglConfig, NULL, ai32ContextAttribs);
    g_eglContext2 = eglCreateContext(g_eglDisplay, eglConfig, g_eglContext1, ai32ContextAttribs);

    DEBUG_PRINT("*************************SURF1(0x%08X), CONTEXT(0x%08X)", (GLuint)g_eglSurface1, (GLuint)g_eglContext1);
    DEBUG_PRINT("*************************SURF2(0x%08X), CONTEXT(0x%08X)", (GLuint)g_eglSurface2, (GLuint)g_eglContext2);

    eglMakeCurrent(g_eglDisplay, g_eglSurface1, g_eglSurface1, g_eglContext1);
#elif defined(LINUX)
    int iConfigs;
    EGLConfig eglConfig = 0;
    EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    const EGLint pi32ConfigAttribs[] =
    {
        /*EGL_LEVEL,              0,*/
        EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
        /*EGL_NATIVE_RENDERABLE,  EGL_FALSE,*/
        EGL_RED_SIZE,           8,
        EGL_GREEN_SIZE,         8,
        EGL_BLUE_SIZE,          8,
        EGL_ALPHA_SIZE,         8,
        EGL_DEPTH_SIZE,         0,
        EGL_NONE
    };

    g_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(g_eglDisplay, NULL, NULL);
    eglBindAPI(EGL_OPENGL_ES_API);
    eglChooseConfig(g_eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs);
    if (iConfigs != 1)
    {
        DEBUG_PRINT("Cannot get required config\n");
        return;
    }

    g_eglSurface1 = eglCreateWindowSurface(g_eglDisplay, eglConfig, (_FBWindow*)nativeWindow, NULL);
    g_eglSurface2 = eglCreateWindowSurface(g_eglDisplay, eglConfig, (_FBWindow*)nativeWindow, NULL);
    g_eglContext1 = eglCreateContext(g_eglDisplay, eglConfig, NULL, ai32ContextAttribs);
    g_eglContext2 = eglCreateContext(g_eglDisplay, eglConfig, g_eglContext1, ai32ContextAttribs);

    DEBUG_PRINT("*************************SURF1(0x%08X), CONTEXT(0x%08X)", (GLuint)g_eglSurface1, (GLuint)g_eglContext1);
    DEBUG_PRINT("*************************SURF2(0x%08X), CONTEXT(0x%08X)", (GLuint)g_eglSurface2, (GLuint)g_eglContext2);

    eglMakeCurrent(g_eglDisplay, g_eglSurface1, g_eglSurface1, g_eglContext1);
#elif defined(UNDER_CE)
    int iConfigs;
    EGLConfig eglConfig = 0;
    EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    const EGLint pi32ConfigAttribs[] =
    {
        /*EGL_LEVEL,              0,*/
        EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
        /*EGL_NATIVE_RENDERABLE,  EGL_FALSE,*/
        EGL_RED_SIZE,           8,
        EGL_GREEN_SIZE,         8,
        EGL_BLUE_SIZE,          8,
        EGL_ALPHA_SIZE,         8,
        EGL_DEPTH_SIZE,         0,
        EGL_NONE
    };

    g_eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(g_eglDisplay, NULL, NULL);
    eglBindAPI(EGL_OPENGL_ES_API);
    eglChooseConfig(g_eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs);
    if (iConfigs != 1)
    {
        DEBUG_PRINT("Cannot get required config\n");
        return;
    }

    g_eglSurface1 = eglCreateWindowSurface(g_eglDisplay, eglConfig, (HWND)nativeWindow, NULL);
    g_eglSurface2 = eglCreateWindowSurface(g_eglDisplay, eglConfig, (HWND)nativeWindow, NULL);
    g_eglContext1 = eglCreateContext(g_eglDisplay, eglConfig, NULL, ai32ContextAttribs);
    g_eglContext2 = eglCreateContext(g_eglDisplay, eglConfig, g_eglContext1, ai32ContextAttribs);

    DEBUG_PRINT("*************************SURF1(0x%08X), CONTEXT(0x%08X)", (GLuint)g_eglSurface1, (GLuint)g_eglContext1);
    DEBUG_PRINT("*************************SURF2(0x%08X), CONTEXT(0x%08X)", (GLuint)g_eglSurface2, (GLuint)g_eglContext2);

    eglMakeCurrent(g_eglDisplay, g_eglSurface1, g_eglSurface1, g_eglContext1);
#else
    int iConfigs;
    EGLConfig eglConfig = 0;
    EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    const EGLint pi32ConfigAttribs[] =
    {
        EGL_LEVEL,              0,
        EGL_SURFACE_TYPE,       EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
        EGL_NATIVE_RENDERABLE,  EGL_FALSE,
        EGL_RED_SIZE,           8,
        EGL_GREEN_SIZE,         8,
        EGL_BLUE_SIZE,          8,
        EGL_ALPHA_SIZE,         8,
        EGL_DEPTH_SIZE,         24,
        EGL_NONE
    };

    g_eglDisplay = eglGetDisplay(g_hDC);
    eglInitialize(g_eglDisplay, NULL, NULL);
    eglBindAPI(EGL_OPENGL_ES_API);
    eglChooseConfig(g_eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs);
    if (iConfigs != 1)
    {
        DEBUG_PRINT("Cannot get required config\n");
    }

    g_eglSurface1 = eglCreateWindowSurface(g_eglDisplay, eglConfig, g_hWnd, NULL);
    g_eglSurface2 = eglCreateWindowSurface(g_eglDisplay, eglConfig, g_hWnd, NULL);
    g_eglContext1 = eglCreateContext(g_eglDisplay, eglConfig, NULL, ai32ContextAttribs);
    g_eglContext2 = eglCreateContext(g_eglDisplay, eglConfig, g_eglContext1, ai32ContextAttribs);

    eglMakeCurrent(g_eglDisplay, g_eglSurface1, g_eglSurface1, g_eglContext1);
#endif

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void initGL(void)
{
    char    infoLog[4096];
    GLint   bVertCompiled;
    GLint   bFragCompiled;
    GLint   bLinked;
    GLchar  *pszVertShader;
    GLchar  *pszFragShader;

    GLfloat pfVertices[] =
    {
        /* Vertex3f       ,     TexCoord2f */
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
        -1.0f, +1.0f, 0.0f,     0.0f, 1.0f,
        +0.0f, -1.0f, 0.0f,     0.5f, 0.0f,
        +0.0f, +1.0f, 0.0f,     0.5f, 1.0f,
        +1.0f, -1.0f, 0.0f,     1.0f, 0.0f,
        +1.0f, +1.0f, 0.0f,     1.0f, 1.0f,
    };

    g_attribStride = 5 * sizeof(GLfloat);

    glDisable(GL_BLEND);

    glGenBuffers(1, &g_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pfVertices), pfVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    LoadTexture();

    g_vertShader    = glCreateShader(GL_VERTEX_SHADER);
    pszVertShader   = vertSrc;
    glShaderSource(g_vertShader, 1, (const GLchar**)&pszVertShader, NULL);
    glCompileShader(g_vertShader);

    glGetShaderiv(g_vertShader, GL_COMPILE_STATUS, &bVertCompiled);
    if (bVertCompiled  == false)
    {
        glGetShaderInfoLog(g_vertShader, sizeof(infoLog), NULL, infoLog);
        DEBUG_PRINT("Vertex Shader Compile Error\n");
    }

    g_fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    pszFragShader = fragSrc;
    glShaderSource(g_fragShader, 1, (const GLchar**)&pszFragShader, NULL);
    glCompileShader(g_fragShader);
    glGetShaderiv(g_fragShader, GL_COMPILE_STATUS, &bFragCompiled);
    if (bFragCompiled == false)
    {
        glGetShaderInfoLog(g_fragShader, sizeof(infoLog), NULL, infoLog);
        DEBUG_PRINT("Fragment Shader Compile Error\n");
    }

    g_program = glCreateProgram();
    glAttachShader(g_program, g_vertShader);
    glAttachShader(g_program, g_fragShader);
    glLinkProgram(g_program);
    glGetProgramiv(g_program, GL_LINK_STATUS, &bLinked);
    if (bLinked == false)
    {
        glGetProgramInfoLog(g_program, sizeof(infoLog), NULL, infoLog);
        DEBUG_PRINT("Linking Error\n");
    }

    g_aLoc_vertexCoord  = glGetAttribLocation(g_program, "aVertexCoord");
    g_aLoc_texCoord     = glGetAttribLocation(g_program, "aTexCoord");
    g_uLoc_fragColor    = glGetUniformLocation(g_program, "uFragColor");
    g_sLoc_tex2D        = glGetUniformLocation(g_program, "sTex2D");
}

void shutDown(void)
{
    glDeleteBuffers(1, &g_vbo);
    glDeleteTextures(1, &g_texture);
    glDeleteShader(g_vertShader);
    glDeleteShader(g_fragShader);
    glDeleteProgram(g_program);

#ifndef ANDROID_JNI
    if (EGL_NO_DISPLAY != g_eglDisplay)
    {
        if (EGL_NO_CONTEXT != g_eglContext1)
        {
            eglDestroyContext(g_eglDisplay, g_eglContext1);
            g_eglContext1 = EGL_NO_CONTEXT;
        }

        if (EGL_NO_CONTEXT != g_eglContext2)
        {
            eglDestroyContext(g_eglDisplay, g_eglContext2);
            g_eglContext2 = EGL_NO_CONTEXT;
        }

        if (EGL_NO_SURFACE != g_eglSurface1)
        {
            eglDestroySurface(g_eglDisplay, g_eglSurface1);
            g_eglSurface1 = EGL_NO_SURFACE;
        }

        if (EGL_NO_SURFACE != g_eglSurface2)
        {
            eglDestroySurface(g_eglDisplay, g_eglSurface2);
            g_eglSurface2 = EGL_NO_SURFACE;
        }

        eglTerminate(g_eglDisplay);
    }

#if defined(WIN32) || defined(UNDER_CE)
    if (g_hDC != NULL)
    {
        ReleaseDC(g_hWnd, g_hDC);
        g_hDC = NULL;
    }
#endif
#endif
}

GLboolean CheckResult()
{
    GLuint  *pixels = new GLuint[WINDOW_WIDTH * WINDOW_HEIGHT];
    GLuint  *dst    = pixels;

    memset(pixels, 0, WINDOW_WIDTH*WINDOW_HEIGHT*sizeof(GLuint));
    glReadPixels(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    GLboolean passed = GL_TRUE;
    for (int i=0; i<WINDOW_HEIGHT; ++i)
    {
        for (int j=0; j<WINDOW_WIDTH; j++)
        {
            if (j < WINDOW_WIDTH/2)
            {
                if (dst[j] != 0x0000FF00)
                {
                    passed = GL_FALSE;
                    break;
                }
            }
            else
            {
                if (dst[j] != 0x00FF0000)
                {
                    passed = GL_FALSE;
                    break;
                }
            }
        }

        dst += WINDOW_WIDTH;
    }

    delete [] pixels;

    return passed;
}

void renderThread1()
{
    MAKE_CURRENT_CTX1;

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    glBindTexture(GL_TEXTURE_2D, g_texture);
    glUseProgram(g_program);
    glUniform4f(g_uLoc_fragColor, 1.0f, 1.0f, 0.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glEnableVertexAttribArray(g_aLoc_vertexCoord);
    glEnableVertexAttribArray(g_aLoc_texCoord);
    glVertexAttribPointer(g_aLoc_vertexCoord, 3, GL_FLOAT, GL_FALSE, g_attribStride, BUFFER_OFFSET(0));
    glVertexAttribPointer(g_aLoc_texCoord, 2, GL_FLOAT, GL_FALSE, g_attribStride, BUFFER_OFFSET(3*sizeof(GLfloat)));

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glFlush();

    glDisableVertexAttribArray(g_aLoc_vertexCoord);
    glDisableVertexAttribArray(g_aLoc_texCoord);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (CheckResult())
    {
        DEBUG_PRINT("Share context test thread1 PASSED\n");
    }
    else
    {
        DEBUG_PRINT("Share context test thread1 FAILED\n");
    }

    SWAP_BUFFER_CTX1;
    LOSE_CURRENT;
}

#if defined(ANDROID_JNI) || defined(LINUX)
void* renderThread2(void* param)
#else
DWORD WINAPI renderThread2(LPVOID lpParam)
#endif
{
    GLint   bVertCompiled;
    GLint   bFragCompiled;
    GLint   bLinked;
    GLchar  *pszVertShader;
    GLchar  *pszFragShader;
    GLuint  vertShader;
    GLuint  fragShader;
    GLuint  program;
    char    infoLog[4096];
    GLuint  i = 0;

    MAKE_CURRENT_CTX2;

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    vertShader      = glCreateShader(GL_VERTEX_SHADER);
    pszVertShader   = vertSrc;
    glShaderSource(vertShader, 1, (const GLchar**)&pszVertShader, NULL);
    glCompileShader(vertShader);
    if (pszVertShader)
    {
        /*free(pszVertShader);*/
        pszVertShader = NULL;
    }
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &bVertCompiled);
    if (bVertCompiled  == false)
    {
        glGetShaderInfoLog(vertShader, sizeof(infoLog), NULL, infoLog);
        DEBUG_PRINT("Vertex Shader Compile Error\n");
    }

    fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    pszFragShader = fragSrc;
    glShaderSource(fragShader, 1, (const GLchar**)&pszFragShader, NULL);
    glCompileShader(fragShader);
    if (pszFragShader)
    {
        /*free(pszFragShader);   */
        pszFragShader = NULL;
    }
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &bFragCompiled);
    if (bFragCompiled == false)
    {
        glGetShaderInfoLog(fragShader, sizeof(infoLog), NULL, infoLog);
        DEBUG_PRINT("Fragment Shader Compile Error\n");
    }

    program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &bLinked);
    if (bLinked == false)
    {
        glGetProgramInfoLog(program, sizeof(infoLog), NULL, infoLog);
        DEBUG_PRINT("Linking Error\n");
    }

    for (i=0; i<1; i++)
    {
        MAKE_CURRENT_CTX2;

        glUseProgram(program);

        glBindTexture(GL_TEXTURE_2D, g_texture);
        /*glUniform1i(g_uLoc_useTex, 0);*/
        glUniform4f(g_uLoc_fragColor, 1.0f, 0.0f, 0.0f, 1.0f);

        glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
        glEnableVertexAttribArray(g_aLoc_vertexCoord);
        glEnableVertexAttribArray(g_aLoc_texCoord);
        glVertexAttribPointer(g_aLoc_vertexCoord, 3, GL_FLOAT, GL_FALSE, g_attribStride, BUFFER_OFFSET(2*g_attribStride));
        glVertexAttribPointer(g_aLoc_texCoord, 2, GL_FLOAT, GL_FALSE, g_attribStride, BUFFER_OFFSET(2*g_attribStride + 3*sizeof(GLfloat)));


        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glFlush();

        glDisableVertexAttribArray(g_aLoc_vertexCoord);
        glDisableVertexAttribArray(g_aLoc_texCoord);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (CheckResult())
        {
            DEBUG_PRINT("Share context test thread2 PASSED\n");
        }
        else
        {
            DEBUG_PRINT("Share context test thread2 FAILED\n");
        }

        SWAP_BUFFER_CTX2;
        LOSE_CURRENT;
    }

    glDeleteProgram(program);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    SWAP_BUFFER_CTX2;
    LOSE_CURRENT;

    return 0;
}

void RenderInit(int nativeWindow)
{
    initEGL(nativeWindow);

    DEBUG_PRINT("Begin GL init");
    initGL();

#if defined(ANDROID_JNI) || defined(LINUX)
    pthread_create(&g_thread, NULL, renderThread2, NULL);
#endif
}

#if defined(ANDROID_JNI)
void Render()
{
    renderThread1();
}
#elif defined(LINUX)
int main(int argc, char** argv)
{
    bool        pause = false;
    vdkEGL      egl;

    memset(&egl, 0, sizeof(vdkEGL));
    egl.vdk = vdkInitialize();
    if (!egl.vdk)
    {
        goto _Error;
    }

    egl.display = vdkGetDisplay(egl.vdk);
    if (!egl.display)
    {
        goto _Error;
    }

    egl.window = vdkCreateWindow(egl.display, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!egl.window)
    {
        goto _Error;
    }

    RenderInit((int)egl.window);

    // Main loop
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
                        // Use SPACE to pause.
                        pause = !pause;
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
        else if (!pause)
        {
            // Render one frame if there is no event.
            renderThread1();
        }
    }

_Error:

    shutDown();
    LOSE_CURRENT;

    if (egl.window)
    {
        vdkDestroyWindow(egl.window);
    }

    if (egl.display)
    {
        vdkDestroyDisplay(egl.display);
    }

    if (egl.vdk)
    {
        vdkExit(egl.vdk);
    }

    return 0;
}
#elif defined(UNDER_CE)
int main(int argc, char **argv)
{
    bool        pause = false;
    vdkEGL      egl;

    memset(&egl, 0, sizeof(vdkEGL));
    egl.vdk = vdkInitialize();
    if (!egl.vdk)
    {
        goto _Error;
    }

    egl.display = vdkGetDisplay(egl.vdk);
    if (!egl.display)
    {
        goto _Error;
    }

    egl.window = vdkCreateWindow(egl.display, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!egl.window)
    {
        goto _Error;
    }

    RenderInit((int)egl.window);

    HANDLE hThread = CreateThread(NULL, 0, renderThread2, NULL, 0, NULL);
    if (hThread == NULL)
    {
        MessageBox(0, TEXT("Cannot create another render thread"), TEXT("Error"), MB_OK|MB_ICONEXCLAMATION);
        shutDown();
    }

    // Main loop
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
                        // Use SPACE to pause.
                        pause = !pause;
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
        else if (!pause)
        {
            // Render one frame if there is no event.
            renderThread1();
        }
    }

_Error:

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    shutDown();
    LOSE_CURRENT;

    if (egl.window)
    {
        vdkDestroyWindow(egl.window);
    }

    if (egl.display)
    {
        vdkDestroyDisplay(egl.display);
    }

    if (egl.vdk)
    {
        vdkExit(egl.vdk);
    }

    return 0;
}
#else
int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX winClass;
    DWORD style = WS_POPUPWINDOW | WS_CAPTION;

    winClass.lpszClassName = "MY_WINDOWS_CLASS";
    winClass.cbSize        = sizeof(WNDCLASSEX);
    winClass.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    winClass.lpfnWndProc   = DefWindowProc;
    winClass.hInstance     = hInstance;
    winClass.hIcon         = LoadIcon(hInstance, NULL);
    winClass.hIconSm       = LoadIcon(hInstance, NULL);
    winClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    winClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    winClass.lpszMenuName  = NULL;
    winClass.cbClsExtra    = 0;
    winClass.cbWndExtra    = 0;

    if (!RegisterClassEx(&winClass))
    {
        return E_FAIL;
    }

    RECT sRect;
    SetRect(&sRect, 200, 200, 200+WINDOW_WIDTH, 200+WINDOW_HEIGHT);
    AdjustWindowRectEx(&sRect, style, false, 0);
    g_hWnd = CreateWindowEx(NULL,
                            "MY_WINDOWS_CLASS",
                            "OpenGL - Share Context",
                            style,
                            sRect.left > 0 ? sRect.left : 0,
                            sRect.top > 0 ? sRect.top : 0,
                            sRect.right - sRect.left,
                            sRect.bottom - sRect.top,
                            NULL, NULL, hInstance, NULL);

    if (g_hWnd == NULL)
    {
        return E_FAIL;
    }

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    g_hDC = GetDC(g_hWnd);

    RenderInit(NULL);

    HANDLE hThread = CreateThread(NULL, 0, renderThread2, NULL, 0, NULL);
    if (hThread == NULL)
    {
        MessageBox(0, TEXT("Cannot create another render thread"), TEXT("Error"), MB_OK|MB_ICONEXCLAMATION);
        shutDown();
    }

    renderThread1();

    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);

    shutDown();
    LOSE_CURRENT;
    UnregisterClass("MY_WINDOWS_CLASS", hInstance);

    return 0;
}
#endif

