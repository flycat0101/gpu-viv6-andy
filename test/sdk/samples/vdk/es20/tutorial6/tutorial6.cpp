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


/*
 * OpenGL ES 2.0 Tutorial 5
 *
 * Draws a simple triangle with basic vertex and pixel shaders.
 */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif

#ifndef EGL_EGLEXT_PROTOTYPES
#define EGL_EGLEXT_PROTOTYPES 1
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#ifndef ANDROID_JNI
#include <gc_vdk.h>
#include <EGL/eglext.h>
#else
#include <utils/Log.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#ifndef UNDER_CE
#include <sys/timeb.h>
#include <fcntl.h>
#endif

#include <string.h>

#define TUTORIAL_NAME "OpenGL ES 2.0 Tutorial 6: EGLImage from native pixmap"




// Wrapper to load vetex and pixel shader.
void LoadShaders(const char * vShaderFName, const char * pShaderFName);
// Cleanup the shaders.
void DestroyShaders();


#ifndef ANDROID_JNI
// to hold vdk information.
vdkEGL egl;
#endif
NativePixmapType pixmap;

EGLImageKHR eglimage;

int width  = 320;
int height = 240;
int posX   = -1;
int posY   = -1;
int samples = 0;
int frames = 500;

// Global Variables, attribute and uniform
GLint locVertices     = 0;
GLint locTransformMat = 0;
GLint locTexcoord     = 0;
GLint locSampler = 0;
GLuint gTexObj = 0;

// Global Variables, shader handle and program handle
GLuint vertShaderNum  = 0;
GLuint pixelShaderNum = 0;
GLuint programHandle  = 0;

#ifndef ANDROID_JNI
int argCount = 6;
char argSpec = '-';
char argNames[] = {'x', 'y', 'w', 'h', 's', 'f'};
char argValues[][255] = {
    "x_coord",
    "y_coord",
    "width",
    "height",
    "samples",
    "frames",
};
char argDescs[][255] = {
    "x coordinate of the window, default is -1(screen center)",
    "y coordinate of the window, default is -1(screen center)",
    "width  of the window in pixels, default is 0(fullscreen)",
    "height of the window in pixels, default is 0(fullscreen)",
    "sample count for MSAA, 0/2/4, default is 0 (no MSAA)",
    "frames to run, default is 0 (no frame limit, escape to exit)",
};
int noteCount = 1;
char argNotes[][255] = {
    "Exit: [ESC] or the frame count reached."
};
#endif
// Triangle Vertex positions.
const GLfloat vertices[][2] = {
    { -0.5f, -0.5f},
    {  0.5f, -0.5f},
    { -0.5f, 0.5f},
    {  0.5f,  0.5f}
};
const GLfloat texcoords[][2] = {
    { 0.0f, 1.0f},
    { 1.0f, 1.0f},
    { 0.0f, 0.0f},
    { 1.0f, 0.0f}
};

// Start with an identity matrix.
GLfloat transformMatrix[16] =
{
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

bool switchPixmapData = true;

/***************************************************************************************
***************************************************************************************/

#define BF_TYPE 0x4D42             /* "MB" */

typedef struct RGB {                      /**** Colormap entry structure ****/
    unsigned char   rgbBlue;          /* Blue value */
    unsigned char   rgbGreen;         /* Green value */
    unsigned char   rgbRed;           /* Red value */
    unsigned char   rgbReserved;      /* Reserved */
}
RGB;

typedef struct BMPINFOHEADER {                     /**** BMP file info structure ****/
    unsigned int   biSize;           /* Size of info header */
    int            biWidth;          /* Width of image */
    int            biHeight;         /* Height of image */
    unsigned short biPlanes;         /* Number of color planes */
    unsigned short biBitCount;       /* Number of bits per pixel */
    unsigned int   biCompression;    /* Type of compression to use */
    unsigned int   biSizeImage;      /* Size of image data */
    int            biXPelsPerMeter;  /* X pixels per meter */
    int            biYPelsPerMeter;  /* Y pixels per meter */
    unsigned int   biClrUsed;        /* Number of colors used */
    unsigned int   biClrImportant;   /* Number of important colors */
}
BMPINFOHEADER;

typedef struct BMPINFO {                      /**** Bitmap information structure ****/
    BMPINFOHEADER   bmiHeader;      /* Image header */
    RGB             bmiColors[256]; /* Image colormap */
}
BMPINFO;

typedef struct BMPFILEHEADER {                      /**** BMP file header structure ****/
    unsigned short bfType;           /* Magic number for file */
    unsigned int   bfSize;           /* Size of file */
    unsigned short bfReserved1;      /* Reserved */
    unsigned short bfReserved2;      /* ... */
    unsigned int   bfOffBits;        /* Offset to bitmap data */
}
BMPFILEHEADER;

/*
 * 'read_word()' - Read a 16-bit unsigned integer.
 */
static unsigned short read_word(FILE *fp)
{
    unsigned char b0, b1; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);

    return ((b1 << 8) | b0);
}

/*
 * 'read_dword()' - Read a 32-bit unsigned integer.
 */
static unsigned int read_dword(FILE *fp)
{
    unsigned char b0, b1, b2, b3; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);

    return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

/*
 * 'read_long()' - Read a 32-bit signed integer.
 */
static int read_long(FILE *fp)
{
    unsigned char b0, b1, b2, b3; /* Bytes from file */

    b0 = getc(fp);
    b1 = getc(fp);
    b2 = getc(fp);
    b3 = getc(fp);

    return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

unsigned char * GltLoadDIBitmap(const char *filename, BMPINFO *info)
{
    FILE            *fp;          /* Open file pointer */
    unsigned char   *bits;        /* Bitmap pixel bits */
    unsigned int    bitsize;      /* Size of bitmap */
    int             infosize;     /* Size of header information */
    BMPFILEHEADER   header;       /* File header */

    /* Try opening the file; use "rb" mode to read this *binary* file. */
#ifdef UNDER_CE
    wchar_t buffer[MAX_PATH + 1];
    int i = GetModuleFileName(NULL, buffer, MAX_PATH);
    while (buffer[i - 1] != L'\\') i--;
    while (*filename != '\0') buffer[i++] = (wchar_t)(*filename++);
    buffer[i] = L'\0';
    fp = _wfopen(buffer, L"rb");
#else
    fp = fopen(filename, "rb");
#endif
    if (fp == NULL)
    {
        return (NULL);
    }

    /* Read the file header and any following bitmap information... */
    header.bfType      = read_word(fp);
    header.bfSize      = read_dword(fp);
    header.bfReserved1 = read_word(fp);
    header.bfReserved2 = read_word(fp);
    header.bfOffBits   = read_dword(fp);

    if (header.bfType != BF_TYPE){
        /* Not a bitmap file - return NULL... */
        fclose(fp);
        return (NULL);
    }

    infosize = header.bfOffBits - 18;

    info->bmiHeader.biSize          = read_dword(fp);
    info->bmiHeader.biWidth         = read_long(fp);
    info->bmiHeader.biHeight        = read_long(fp);
    info->bmiHeader.biPlanes        = read_word(fp);
    info->bmiHeader.biBitCount      = read_word(fp);
    info->bmiHeader.biCompression   = read_dword(fp);
    info->bmiHeader.biSizeImage     = read_dword(fp);
    info->bmiHeader.biXPelsPerMeter = read_long(fp);
    info->bmiHeader.biYPelsPerMeter = read_long(fp);
    info->bmiHeader.biClrUsed       = read_dword(fp);
    info->bmiHeader.biClrImportant  = read_dword(fp);

    if (infosize > 40){
        if (fread(info->bmiColors, infosize - 40, 1, fp) < 1){
            /* Couldn't read the bitmap header - return NULL... */
            fclose(fp);
            return (NULL);
        }
    }

    /* Now that we have all the header info read in, allocate memory for *
     * the bitmap and read *it* in...                                    */
    if ((bitsize = info->bmiHeader.biSizeImage) == 0)
        bitsize = (info->bmiHeader.biWidth *
                   info->bmiHeader.biBitCount + 7) / 8 *
               abs(info->bmiHeader.biHeight);

    bits = (unsigned char *)malloc( sizeof(unsigned char) * bitsize);

    if ( bits == NULL )
    {
        /* Couldn't allocate memory - return NULL! */
        fclose(fp);
        return (NULL);
    }

    if (fread(bits, 1, bitsize, fp) < bitsize)
    {
        /* Couldn't read bitmap - free memory and return NULL! */
        free(bits);
        fclose(fp);
        return (NULL);
    }

    /* OK, everything went fine - return the allocated bitmap... */
    fclose(fp);
    return (bits);
}



GLuint LoadTexture(
    const char* FileName,
    NativePixmapType pixmap
    )
{
    GLuint result = 0;
    do
    {
        BMPINFO info = { 0 };
        int width, height, Width, Height;
        void* pointer = NULL;
        unsigned char* img = GltLoadDIBitmap(FileName, &info);

        if (img != NULL)
        {
            width =  info.bmiHeader.biWidth;
            height = info.bmiHeader.biHeight;
        }
        else
            return result;

        glFinish();

        vdkGetPixmapInfo(pixmap, &Width, &Height, NULL, NULL, &pointer);
        memcpy(pointer, img, width * height * 4);
        free(img);
        /* Success. */
        result = 1;
    }
    while (0);
    /* Return result. */
    return result;
}

bool RenderInit()
{
    EGLint attrib_list [] = {
        EGL_NONE
    };
    glGenTextures(1, &gTexObj);
    // Grab location of shader attributes.
    locVertices = glGetAttribLocation(programHandle, "my_Vertex");
    locTexcoord = glGetAttribLocation(programHandle, "my_Texcoor");
    // Transform Matrix is uniform for all vertices here.
    locTransformMat = glGetUniformLocation(programHandle, "my_TransformMatrix");
    locSampler = glGetUniformLocation(programHandle, "my_Sampler");

    // enable vertex arrays to push the data.
    glEnableVertexAttribArray(locVertices);
    glEnableVertexAttribArray(locTexcoord);

    // set data in the arrays.
    glVertexAttribPointer(locVertices, 2, GL_FLOAT, GL_FALSE, 0, &vertices[0][0]);
    glVertexAttribPointer(locTexcoord, 2, GL_FLOAT, GL_FALSE, 0, &texcoords[0][0]);
    glUniformMatrix4fv(locTransformMat, 1, GL_FALSE, transformMatrix);
    glUniform1i(locSampler, 0);// gTexObj);

    pixmap = vdkCreatePixmap(egl.display, 320, 240, 32);

    if (!LoadTexture("tex1.bmp", pixmap))
    {
        printf("Could not load the tex1.bmp file.\n");
        return 0;
    }
    eglimage = (EGLImageKHR)eglCreateImageKHR(egl.eglDisplay, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR, (EGLClientBuffer)pixmap, attrib_list);
    glBindTexture(GL_TEXTURE_2D, gTexObj);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)eglimage);
    /* Explicit set min/mag filter to only use mipmap level 0. */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return (GL_NO_ERROR == glGetError());
}

// Actual rendering here.
void Render()
{



    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    if(switchPixmapData)
    {
        if (!LoadTexture("tex2.bmp", pixmap))
        {
            printf("Could not load the tex2.bmp file.\n");
            return;
        }
    }
    else
    {
        if (!LoadTexture("tex1.bmp", pixmap))
        {
            printf("Could not load the tex1.bmp file.\n");
            return;
        }
    }
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glFlush ();

    vdkSwapEGL(&egl);

}

void RenderCleanup()
{
    // cleanup
    glDisableVertexAttribArray(locVertices);
    glDisableVertexAttribArray(locTexcoord);
    DestroyShaders();
}

/***************************************************************************************
***************************************************************************************/

int CompileShader(const char * FName, GLuint ShaderNum)
{
    FILE * fptr = NULL;
#ifdef UNDER_CE
    static wchar_t buffer[MAX_PATH + 1];
    int i = GetModuleFileName(NULL, buffer, MAX_PATH);
    while (buffer[i - 1] != L'\\') i--;
    while (*FName != '\0') buffer[i++] = (wchar_t)(*FName++);
    buffer[i] = L'\0';
    fptr = _wfopen(buffer, L"rb");
#else
    fptr = fopen(FName, "rb");
#endif

    if (fptr == NULL)
    {
        return 0;
    }

    int length;
    fseek(fptr, 0, SEEK_END);
    length = ftell(fptr);
    fseek(fptr, 0 ,SEEK_SET);

    char * shaderSource = (char*)malloc(sizeof (char) * length);
    if (shaderSource == NULL)
    {
        fclose(fptr);
        fprintf(stderr, "Out of memory.\n");
        return 0;
    }

    fread(shaderSource, length, 1, fptr);

    glShaderSource(ShaderNum, 1, (const char**)&shaderSource, &length);
    glCompileShader(ShaderNum);

    free(shaderSource);
    fclose(fptr);

    GLint compiled = 0;
    glGetShaderiv(ShaderNum, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {

        GLint errorBufSize, errorLength;
        glGetShaderiv(ShaderNum, GL_INFO_LOG_LENGTH, &errorBufSize);

        char * infoLog = (char*)malloc(errorBufSize * sizeof(char) + 1);
        if (infoLog)
        {

            glGetShaderInfoLog(ShaderNum, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize] = '\0';
            fprintf(stderr, "%s\n", infoLog);

            free(infoLog);
        }
        return 0;
    }

    return 1;
}

/***************************************************************************************
***************************************************************************************/

void LoadShaders(const char * vShaderFName, const char * pShaderFName)
{
    vertShaderNum = glCreateShader(GL_VERTEX_SHADER);
    pixelShaderNum = glCreateShader(GL_FRAGMENT_SHADER);

    if (CompileShader(vShaderFName, vertShaderNum) == 0)
    {
        printf("%d: PS compile failed.\n", __LINE__);
        return;
    }

    if (CompileShader(pShaderFName, pixelShaderNum) == 0)
    {
        printf("%d: VS compile failed.\n", __LINE__);
        return;
    }

    programHandle = glCreateProgram();

    glAttachShader(programHandle, vertShaderNum);
    glAttachShader(programHandle, pixelShaderNum);

    glLinkProgram(programHandle);
    // Check if linking succeeded.
    GLint linked = false;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        printf("%d: Link failed.\n", __LINE__);
        // Retrieve error buffer size.
        GLint errorBufSize, errorLength;
        glGetShaderiv(programHandle, GL_INFO_LOG_LENGTH, &errorBufSize);

        char * infoLog = (char*)malloc(errorBufSize * sizeof (char) + 1);
        if (!infoLog)
        {
            // Retrieve error.
            glGetProgramInfoLog(programHandle, errorBufSize, &errorLength, infoLog);
            infoLog[errorBufSize + 1] = '\0';
            fprintf(stderr, "%s", infoLog);

            free(infoLog);
        }

        return;
    }
    glUseProgram(programHandle);
}

// Cleanup the shaders.
void DestroyShaders()
{
    if (programHandle) {
        glDeleteShader(vertShaderNum);
        glDeleteShader(pixelShaderNum);
        glDeleteProgram(programHandle);
        glUseProgram(0);
        programHandle = 0;
    }
}

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

// Program entry.
int main(int argc, char** argv)
{
    // EGL configuration - we use 24-bpp render target and a 16-bit Z buffer.
    EGLint configAttribs[] =
    {
        EGL_SAMPLES,      0,
        EGL_RED_SIZE,     8,
        EGL_GREEN_SIZE,   8,
        EGL_BLUE_SIZE,    8,
        EGL_ALPHA_SIZE,   EGL_DONT_CARE,
        EGL_DEPTH_SIZE,   0,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_NONE,
    };

    EGLint attribListContext[] =
    {
        // Needs to be set for es2.0 as default client version is es1.1.
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

    // Adjust the window size to make sure these size values does not go beyound the screen limits.
    vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);

    // Set window title and show the window.
    vdkSetWindowTitle(egl.window, TUTORIAL_NAME);
    vdkShowWindow(egl.window);

    // load and compiler vertex/fragment shaders.
    LoadShaders("vs_es20t6.vert", "ps_es20t6.frag");
    if (programHandle != 0)
    {
        if (!RenderInit())
        {
            goto OnError;
        }
        int frameCount = 0;
        int halfFrames = (int)(frames / 2);
        unsigned int start = vdkGetTicks();
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
            else
            {
                // Render one frame if there is no event.
                Render();
                frameCount++;
                if(frameCount > halfFrames)
                {
                    switchPixmapData = !switchPixmapData;
                }
                if ((frames > 0) && (--frames == 0)) {
                    done = true;
                }
            }
        }

        glFinish();
        unsigned int end = vdkGetTicks();
        float fps = frameCount / ((end - start) / 1000.0f);
        printf("%d frames in %d ticks -> %.3f fps\n", frameCount, end - start, fps);
        RenderCleanup();
    }

OnError:

    DestroyShaders();

    // cleanup
    vdkFinishEGL(&egl);

    return 0;
}
#endif

