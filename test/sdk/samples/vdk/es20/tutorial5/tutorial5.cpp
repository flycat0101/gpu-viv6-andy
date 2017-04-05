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
 * OpenGL ES 2.0 Tutorial 5
 *
 * Draws a simple triangle with basic vertex and pixel shaders.
 */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES    1
#endif

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#ifndef ANDROID_JNI
#include <gc_vdk.h>
#else
#include <utils/Log.h>
#include <EGL/egl.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#ifndef UNDER_CE
#include <fcntl.h>
#endif
#include <string.h>

#define TUTORIAL_NAME "OpenGL ES 2.0 Tutorial 5"

/* GL_VIV_direct_texture */
#ifndef GL_VIV_direct_texture
#define GL_VIV_YV12                     0x8FC0
#define GL_VIV_NV12                     0x8FC1
#define GL_VIV_YUY2                     0x8FC2
#define GL_VIV_UYVY                     0x8FC3
#define GL_VIV_NV21                     0x8FC4
#endif

#if !GL_ES_VERSION_3_0
#define GL_COMPRESSED_R11_EAC                            0x9270
#define GL_COMPRESSED_SIGNED_R11_EAC                     0x9271
#define GL_COMPRESSED_RG11_EAC                           0x9272
#define GL_COMPRESSED_SIGNED_RG11_EAC                    0x9273
#define GL_COMPRESSED_RGB8_ETC2                          0x9274
#define GL_COMPRESSED_SRGB8_ETC2                         0x9275
#define GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
#define GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2     0x9277
#define GL_COMPRESSED_RGBA8_ETC2_EAC                     0x9278
#define GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC              0x9279
#endif

typedef void (GL_APIENTRY *PFNGLTEXDIRECTVIV) (GLenum Target, GLsizei Width, GLsizei Height, GLenum Format, GLvoid ** Pixels);
typedef void (GL_APIENTRY *PFNGLTEXDIRECTINVALIDATEVIV) (GLenum Target);

static PFNGLTEXDIRECTVIV            pFNglTexDirectVIV           = NULL;
static PFNGLTEXDIRECTINVALIDATEVIV  pFNglTexDirectInvalidateVIV = NULL;

// Wrapper to load vetex and pixel shader.
void LoadShaders(const char * vShaderFName, const char * pShaderFName);
// Cleanup the shaders.
void DestroyShaders();

#ifndef ANDROID_JNI
vdkEGL  egl;
#endif

int     width                   = 0;
int     height                  = 0;
int     posX                    = -1;
int     posY                    = -1;
int     samples                 = 0;
int     frames                  = 0;
int     curFrame                = 0;
int     nCounter                = 0;
bool    bUseYUVTex              = true;

bool    linearTexture           = false;
int     texType                 = GL_COMPRESSED_RGB8_ETC2;


bool    linearRGBTexture        = false;
int     rgbTexType              = GL_RGB;
bool    linearNV21Texture       = true;
int     nv21TexType             = GL_VIV_NV21;

#ifdef ANDROID_JNI
char    nv21TexFileName[256]    = "/sdcard/tutorial/tutorial5/f430_160x120xNV21.yuv";
char    rgbTexFileName[256]     = "/sdcard/tutorial/tutorial5/background.tga";
char    texFileName[256]        = "/sdcard/tutorial/tutorial5/snake-reptile.ktx";
#else
char    nv21TexFileName[256]    = "f430_160x120xNV21.yuv";
char    rgbTexFileName[256]     = "background.tga";
char    texFileName[256]        = "snake-reptile.ktx";
#endif


// Global Variables, attribute and uniform
GLint   locVertices             = 0;
GLint   locTransformMat         = 0;
GLint   locTexcoord             = 0;
GLint   locSampler              = 0;
GLuint  gNV21TexObj             = 0;
GLuint  gRGBTexObj              = 0;

// Global Variables, shader handle and program handle
GLuint  vertShaderNum           = 0;
GLuint  pixelShaderNum          = 0;
GLuint  programHandle           = 0;

#ifndef ANDROID_JNI
int     argCount = 6;
char    argSpec = '-';
char    argNames[] = {'x', 'y', 'w', 'h', 's', 'f'};
char    argValues[][255] = {
    "x_coord",
    "y_coord",
    "width",
    "height",
    "samples",
    "frames",
};
char    argDescs[][255] = {
    "x coordinate of the window, default is -1(screen center)",
    "y coordinate of the window, default is -1(screen center)",
    "width  of the window in pixels, default is 0(fullscreen)",
    "height of the window in pixels, default is 0(fullscreen)",
    "sample count for MSAA, 0/2/4, default is 0 (no MSAA)",
    "frames to run, default is 0 (no frame limit, escape to exit)",
};
int     noteCount = 1;
char    argNotes[][255] = {
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


/***************************************************************************************
***************************************************************************************/
void        *nv21Planes[3];
void        *rgbPlanes[3];
void        *planes[3];
int         vFrames             = 0;
FILE        *nv21File           = NULL;
FILE        *rgbFile            = NULL;
FILE        *file               = NULL;
long long   nv21FileSize        = 0;
int         nv21FileFrameSize   = 0;
int         ySize               = 0;
int         uSize               = 0;
int         vSize               = 0;

int LoadFrame()
{
    if (feof(nv21File))
    {
        fseek(nv21File, nv21FileSize % nv21FileFrameSize, SEEK_SET);
        return 0;
    }

    if (fread(nv21Planes[0], ySize, 1, nv21File) <= 0)
    {
        return 0;
    }

    if (vSize > 0)
    {
        if (fread(nv21Planes[1], vSize, 1, nv21File) <= 0)
        {
            return 0;
        }
    }

    if (uSize > 0)
    {
        if (fread(nv21Planes[2], uSize, 1, nv21File) <= 0)
        {
            return 0;
        }
    }

    /* Mark as dirty. */
    (*pFNglTexDirectInvalidateVIV)(GL_TEXTURE_2D);

    return 0;
}

GLuint Load420Texture(const char* FileName,
                      int Width,
                      int Height,
                      int format)
{
    GLuint result = 0;

    do
    {
        GLuint name;

#ifdef UNDER_CE
        static wchar_t buffer[MAX_PATH + 1];
        int i = GetModuleFileName(NULL, buffer, MAX_PATH);
        while (buffer[i - 1] != L'\\') i--;
        while (*FileName != '\0') buffer[i++] = (wchar_t)(*FileName++);
        buffer[i] = L'\0';
        nv21File = _wfopen(buffer, L"rb");
#else
        /* Open the texture file. */
        nv21File = fopen(FileName, "rb");
#endif

        if (nv21File == NULL)
        {
            break;
        }

        /* Determine the size of the file. */
        if (fseek(nv21File, 0, SEEK_END) == -1)
        {
            break;
        }

        switch (format)
        {
        case GL_VIV_YV12:
            ySize     = Width * Height;
            vSize     = ySize / 4;
            uSize     = vSize;
            break;

        case GL_VIV_NV12:
        case GL_VIV_NV21:
            ySize     = Width * Height;
            vSize     = ySize / 2;
            uSize     = 0;
            break;

        case GL_VIV_YUY2:
        case GL_VIV_UYVY:
            ySize     = 2 * Width * Height;
            vSize     = 0;
            uSize     = 0;
            break;
        }

        nv21FileFrameSize   = ySize + uSize + vSize;
        nv21FileSize        = ftell(nv21File);
        vFrames             = (int)nv21FileSize/nv21FileFrameSize;
        curFrame            = 0;

        /* Determine the number of frames in the file. */
        if ((nv21FileSize <= 0) || (nv21FileFrameSize <= 0))
        {
            break;
        }

        /* Create the texture. */
        glGenTextures(1, &name);
        glBindTexture(GL_TEXTURE_2D, name);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        (*pFNglTexDirectVIV)(GL_TEXTURE_2D,
                             Width,
                             Height,
                             format,
                             (GLvoid**) &nv21Planes);
        if (glGetError() != GL_NO_ERROR)
        {
            break;
        }

        fseek(nv21File, nv21FileSize % nv21FileFrameSize, SEEK_SET);

        if (fread(nv21Planes[0], ySize, 1, nv21File) <= 0)
        {
            return 0;
        }

        if (vSize > 0)
        {
            if (fread(nv21Planes[1], vSize, 1, nv21File) <= 0)
            {
                return 0;
            }
        }

        if (uSize > 0)
        {
            if (fread(nv21Planes[2], uSize, 1, nv21File) <= 0)
            {
                return 0;
            }
        }

        /* Mark as dirty. */
        (*pFNglTexDirectInvalidateVIV)(GL_TEXTURE_2D);

        /* Success. */
        result = name;
    }
    while (0);

    /* Return result. */
    return result;
}

GLuint LoadTGA( const char* strFileName)
{
    struct TARGA_HEADER
    {
        unsigned char   IDLength, ColormapType, ImageType;
        unsigned char   ColormapSpecification[5];
        unsigned short  XOrigin, YOrigin;
        unsigned short  ImageWidth, ImageHeight;
        unsigned char   PixelDepth;
        unsigned char   ImageDescriptor;
    };

    static struct TARGA_HEADER header;
    GLuint format;
    GLuint name;

    // Read the TGA rgbFile
#ifdef UNDER_CE
    wchar_t tgabuffer[MAX_PATH + 1];
    int i = GetModuleFileName(NULL, tgabuffer, MAX_PATH);
    while (tgabuffer[i - 1] != L'\\') i--;
    while (*strFileName != '\0') tgabuffer[i++] = (wchar_t)(*strFileName++);
    tgabuffer[i] = L'\0';
    rgbFile = _wfopen(tgabuffer, L"rb");
#else
    rgbFile = fopen(strFileName, "rb" );
#endif

    if( NULL == rgbFile )
    {
        return 0;
    }

    fread( &header, sizeof(header), 1, rgbFile );
    unsigned int nPixelSize = header.PixelDepth / 8;
    format = nPixelSize == 3 ? GL_RGB : GL_BGRA_EXT;

    /* Create the texture. */
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_2D, name);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    (*pFNglTexDirectVIV)(GL_TEXTURE_2D,
                         header.ImageWidth,
                         header.ImageHeight,
                         format,
                         (GLvoid**) &rgbPlanes);

    int err = glGetError();
    if (err != GL_NO_ERROR)
    {
        return 0;
    }

    if (fread(rgbPlanes[0], nPixelSize, header.ImageWidth * header.ImageHeight, rgbFile) <= 0)
    {
        return 0;
    }

    /* Mark as dirty. */
    (*pFNglTexDirectInvalidateVIV)(GL_TEXTURE_2D);

    /* Success. */
    return name;
}

void removeComments(FILE *f1)
{
    int c;

    while((c = getc(f1)) == '#')
    {
        char line[1024];
        fgets(line, 1024, f1);
    }
    ungetc(c, f1);
}

void removeSpaces(FILE *f1)
{
    int c;

    c = getc(f1);
    while(c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r')
    {
        c = getc(f1);
    }
    ungetc(c, f1);
}


GLuint LoadPPM( const char* strFileName)
{
    GLuint name;
    int width, height, maximum;
    int bitrate = 8;

// In a PKM-file, the codecs are stored using the following identifiers
//
// identifier                         value               codec
// --------------------------------------------------------------------
// ETC1_RGB_NO_MIPMAPS                  0                 GL_ETC1_RGB8_OES
// ETC2PACKAGE_RGB_NO_MIPMAPS           1                 GL_COMPRESSED_RGB8_ETC2
// ETC2PACKAGE_RGBA_NO_MIPMAPS_OLD      2, not used       -
// ETC2PACKAGE_RGBA_NO_MIPMAPS          3                 GL_COMPRESSED_RGBA8_ETC2_EAC
// ETC2PACKAGE_RGBA1_NO_MIPMAPS         4                 GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
// ETC2PACKAGE_R_NO_MIPMAPS             5                 GL_COMPRESSED_R11_EAC
// ETC2PACKAGE_RG_NO_MIPMAPS            6                 GL_COMPRESSED_RG11_EAC
// ETC2PACKAGE_R_SIGNED_NO_MIPMAPS      7                 GL_COMPRESSED_SIGNED_R11_EAC
// ETC2PACKAGE_RG_SIGNED_NO_MIPMAPS     8                 GL_COMPRESSED_SIGNED_RG11_EAC
//
// In the code, the identifiers are not always used strictly. For instance, the
// identifier ETC2PACKAGE_R_NO_MIPMAPS is sometimes used for both the unsigned
// (GL_COMPRESSED_R11_EAC) and signed (GL_COMPRESSED_SIGNED_R11_EAC) version of
// the codec.

    // Read the TGA file
#ifdef UNDER_CE
    wchar_t ppmbuffer[MAX_PATH + 1];
    int i = GetModuleFileName(NULL, ppmbuffer, MAX_PATH);
    while (ppmbuffer[i - 1] != L'\\') i--;
    while (*strFileName != '\0') ppmbuffer[i++] = (wchar_t)(*strFileName++);
    ppmbuffer[i] = L'\0';
    file = _wfopen(ppmbuffer, L"rb");
#else
    file = fopen( strFileName, "rb" );
#endif

    if(file)
    {
        char line[255];

        removeSpaces(file);
        removeComments(file);
        removeSpaces(file);

        fscanf(file, "%s", line);

        if(strcmp(line, "P6")!=0)
        {
            printf("Error: %s is not binary\n", line);
            printf("(Binary .ppm files start with P6).\n");
            fclose(file);
            return 0;
        }
        removeSpaces(file);
        removeComments(file);
        removeSpaces(file);

        fscanf(file, "%d %d", &width, &height);
        if( width<=0 || height <=0)
        {
            printf("Error: width or height negative. File: %s\n",strFileName);
            fclose(file);
            return 0;
        }

        removeSpaces(file);
        removeComments(file);
        removeSpaces(file);

        fscanf(file, "%d", &maximum);
        if( maximum!= 255&&maximum!=(1<<16)-1)
        {
            printf("Error: Color resolution must be 255. File: %s\n",strFileName);
            fclose(file);
            return 0;
        }

        //printf("maximum is %d\n",maximum);
        if(maximum!=255)
            bitrate=16;

        // We need to remove the newline.
        char c = 0;
        while(c != '\n')
        {
            fscanf(file, "%c", &c);
        }

        planes[0] = malloc(3*width*height*bitrate/8);
        if(!planes[0])
        {
            printf("Error: Could not allocate memory for image. File: %s\n", strFileName);
            fclose(file);
            return 0;
        }

        if(fread(planes[0], 3*width*height*bitrate/8, 1, file) != 1)
        {
            printf("Error: Could not read all pixels. File: %s\n", strFileName);
            free(planes[0]);
            fclose(file);
            return 0;
        }
    }
    else
    {
        return 0;
    }

    /* Create the texture. */
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_2D, name);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (glGetError() != GL_NO_ERROR)
    {
        return 0;
    }

    /* Upload texture. */
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, texType, width, height, 0,
        (width * height * (bitrate/8)) / 4, planes[0]);

    /* Success. */
    return name;
}

typedef struct KTX_header_t
{
    unsigned char identifier[12];
    unsigned int endianness;
    unsigned int glType;
    unsigned int glTypeSize;
    unsigned int glFormat;
    unsigned int glInternalFormat;
    unsigned int glBaseInternalFormat;
    unsigned int pixelWidth;
    unsigned int pixelHeight;
    unsigned int pixelDepth;
    unsigned int numberOfArrayElements;
    unsigned int numberOfFaces;
    unsigned int numberOfMipmapLevels;
    unsigned int bytesOfKeyValueData;
}
KTX_header;

GLuint LoadKTX( const char* strFileName)
{
    GLuint name;
    int width, height;
    int bitsPixelPixel;

    // Read the PKM file
#ifdef UNDER_CE
    wchar_t ktxbuffer[MAX_PATH + 1];
    int i = GetModuleFileName(NULL, ktxbuffer, MAX_PATH);
    while (ktxbuffer[i - 1] != L'\\') i--;
    while (*strFileName != '\0') ktxbuffer[i++] = (wchar_t)(*strFileName++);
    ktxbuffer[i] = L'\0';
    file = _wfopen(ktxbuffer, L"rb");
#else
    file = fopen( strFileName, "rb" );
#endif

    if(file)
    {
        //read ktx header..
        KTX_header header;
        unsigned int bitsize;

        fread(&header,sizeof(KTX_header),1,file);
        //read size parameter, which we don't actually need..
        fread(&bitsize,sizeof(unsigned int),1,file);

        if ((header.glInternalFormat == GL_COMPRESSED_RGBA8_ETC2_EAC)
         || (header.glInternalFormat == GL_COMPRESSED_RGBA8_ETC2_EAC))
        {
            bitsPixelPixel = 8;
        }
        else
        {
            bitsPixelPixel = 4;
        }

        texType = header.glInternalFormat;

        width = ((header.pixelWidth+3)/4)*4;
        height = ((header.pixelHeight+3)/4)*4;

        planes[0] = malloc((width*height*bitsPixelPixel)/8);
        if(!planes[0])
        {
            printf("Error: Could not allocate memory for image. File: %s\n", strFileName);
            fclose(file);
            return 0;
        }

        if(fread(planes[0], (width*height*bitsPixelPixel)/8, 1, file) != 1)
        {
            printf("Error: Could not read all pixels. File: %s\n", strFileName);
            free(planes[0]);
            fclose(file);
            return 0;
        }
    }
    else
    {
        return 0;
    }

    /* Create the texture. */
    glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_2D, name);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (glGetError() != GL_NO_ERROR)
    {
        return 0;
    }

    /* Upload texture. */
    glCompressedTexImage2D(GL_TEXTURE_2D, 0, texType, width, height, 0,
        (width*height*bitsPixelPixel)/8, planes[0]);

    /* Success. */
    return name;
}

bool RenderInit()
{
    int extensionPos;

    if (linearNV21Texture)
    {
        if (pFNglTexDirectVIV == NULL)
        {
            /* Get the pointer to the glTexDirectVIV function. */
            pFNglTexDirectVIV = (PFNGLTEXDIRECTVIV)
                eglGetProcAddress("glTexDirectVIV");

            if (pFNglTexDirectVIV == NULL)
            {
                printf("Required extension not supported.\n");
                /* The glTexDirectVIV function is not available. */
                return false;
            }
        }

        if (pFNglTexDirectInvalidateVIV == NULL) {
            /* Get the pointer to the glTexDirectInvalidate function. */
            pFNglTexDirectInvalidateVIV = (PFNGLTEXDIRECTINVALIDATEVIV)
                eglGetProcAddress("glTexDirectInvalidateVIV");

            if (pFNglTexDirectInvalidateVIV == NULL)
            {
                printf("Required extension not supported.\n");
                /* The glTexDirectInvalidateVIV function is not available. */
                return false;
            }
        }
    }

    // Grab location of shader attributes.
    locVertices = glGetAttribLocation(programHandle, "my_Vertex");
    locTexcoord = glGetAttribLocation(programHandle, "my_Texcoor");

    // Transform Matrix is uniform for all vertices here.
    locTransformMat = glGetUniformLocation(programHandle, "my_TransformMatrix");
    locSampler = glGetUniformLocation(programHandle, "my_Sampler");

    gNV21TexObj = Load420Texture(nv21TexFileName, 160, 120, nv21TexType);
    gRGBTexObj  = LoadTGA(rgbTexFileName);

    if (gNV21TexObj == 0)
    {
        printf("Could not load the yuv texture file: %s. Exiting.\n", nv21TexFileName);
        return false;
    }

    if (gRGBTexObj == 0)
    {
        printf("Could not load the tga texture file: %s. Exiting.\n", rgbTexFileName);
        return false;
    }

    // enable vertex arrays to push the data.
    glEnableVertexAttribArray(locVertices);
    glEnableVertexAttribArray(locTexcoord);

    // set data in the arrays.
    glVertexAttribPointer(locVertices, 2, GL_FLOAT, GL_FALSE, 0, &vertices[0][0]);
    glVertexAttribPointer(locTexcoord, 2, GL_FLOAT, GL_FALSE, 0, &texcoords[0][0]);
    glUniformMatrix4fv(locTransformMat, 1, GL_FALSE, transformMatrix);
    glUniform1i(locSampler, 0);// gTexObj);

    return (GL_NO_ERROR == glGetError());
}

// Actual rendering here.
void Render()
{
    static float angle = 0;

    if (bUseYUVTex)
    {
        glBindTexture(GL_TEXTURE_2D, gNV21TexObj);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, gRGBTexObj);
    }

    // Clear background.
    glClearColor(0.0f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Set up rotation matrix rotating by angle around y axis.
    transformMatrix[0] = transformMatrix[10] = (GLfloat)cos(angle);
    transformMatrix[2] = (GLfloat)sin(angle);
    transformMatrix[8] = -transformMatrix[2];
    angle += 0.1f;

    /*According to Linux programer's manual, need to clamp the input value of sin or cos to valid value,
        otherwise return NaN and exception may be raised.*/
    if(angle >= 6.3)
    {
        angle = fmod(angle, (float)6.3);
    }

    glUniformMatrix4fv(locTransformMat, 1, GL_FALSE, transformMatrix);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // flush all commands.
    glFlush ();

#ifndef ANDROID_JNI
    // swap display with drawn surface.
    vdkSwapEGL(&egl);
#endif

    LoadFrame();
}

void RenderCleanup()
{
    // cleanup
    glDisableVertexAttribArray(locVertices);
    glDisableVertexAttribArray(locTexcoord);
    DestroyShaders();

    if (nv21File != NULL)
    {
        fclose(nv21File);
        nv21File = NULL;
    }

    if (rgbFile != NULL)
    {
        fclose(rgbFile);
        rgbFile = NULL;
    }
}

/***************************************************************************************
***************************************************************************************/

// Compile a vertex or pixel shader.
// returns 0: fail
//         1: success
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
        // Retrieve error buffer size.
        GLint errorBufSize, errorLength;
        glGetShaderiv(ShaderNum, GL_INFO_LOG_LENGTH, &errorBufSize);

        char * infoLog = (char*)malloc(errorBufSize * sizeof(char) + 1);
        if (infoLog)
        {
            // Retrieve error.
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

// Wrapper to load vetex and pixel shader.
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
    if (programHandle)
    {
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
    LoadShaders("vs_es20t5.vert", "ps_es20t5.frag");

    if (programHandle != 0)
    {
        if (!RenderInit())
        {
            goto OnError;
        }

        int frameCount = 0;
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
                    case VDK_SPACE:
                        // Use SPACE to pauseFlag.
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
                if (!(frameCount % 60))
                {
                    bUseYUVTex  = !bUseYUVTex;
                }

                // Render one frame if there is no event.
                Render();
                ++ frameCount;

                if ((frames > 0) && (--frames == 0))
                {
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

    if (nv21File != NULL)
    {
        fclose(nv21File);
    }

    if (rgbFile != NULL)
    {
        fclose(rgbFile);
    }

    // cleanup
    vdkFinishEGL(&egl);

    return 0;
}
#endif
