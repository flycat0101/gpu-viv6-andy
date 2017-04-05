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


#ifndef _VG_FRAME_H
#define _VG_FRAME_H

#include "PreComp.h"
#include <EGL/egl.h>
#if USE_VDK
#include <gc_vdk.h>
#else
    #ifdef LINUX
        #ifndef EGL_API_FB
            #include <X11/Xlib.h>
            #include <X11/X.h>
        #endif
    #else
    #endif
#endif

struct appAttribs
{
    int winWidth;
    int winHeight;
    int redSize;
    int greenSize;
    int blueSize;
    int alphaSize;
    int depthSize;
    int stencilSize;
    int samples;
    int openvgbit;
    int frameCount;
    int save;
    int fps;
};

#if (defined(LINUX) || defined(__QNXNTO__))
    #define BI_RGB        0L
    #define BI_RLE8       1L
    #define BI_RLE4       2L
    #define BI_BITFIELDS  3L

    typedef unsigned long    DWORD;
    typedef unsigned short    WORD;
    typedef long            LONG;

    typedef struct tagBITMAPINFOHEADER{
            DWORD      biSize;
            LONG       biWidth;
            LONG       biHeight;
            WORD       biPlanes;
            WORD       biBitCount;
            DWORD      biCompression;
            DWORD      biSizeImage;
            LONG       biXPelsPerMeter;
            LONG       biYPelsPerMeter;
            DWORD      biClrUsed;
            DWORD      biClrImportant;
    } __attribute((packed)) BITMAPINFOHEADER;

    typedef struct tagBITMAPFILEHEADER {
            WORD    bfType;
            DWORD   bfSize;
            WORD    bfReserved1;
            WORD    bfReserved2;
            DWORD   bfOffBits;
    } __attribute((packed)) BITMAPFILEHEADER;
#endif

bool InitEGL();
void Release();

void DrawString(char* str);
void testOverlayString(const char *format, ...);
//bool CDECL LoadTGA(LPCTSTR FileName, SIZE& Size, INT& BitsPerPixel, LPBYTE& Bits);

void vfFRAME_AppInit(int *width, int *height, char *title);
VGboolean vfFRAME_InitVG();
void vfFRAME_Render();
void vfFRAME_Clean();
bool keyprocessor(unsigned char key);
#if USE_VDK
    void mouseKeyProcessor(int mouseX, int mouseY, vdkEvent msg);
    void mouseExtKeyProcessor(int mouseX, int mouseY, bool bKeyDown);
    //void mouseMoveProcessor(int mouseX, int mouseY, unsigned int keyState);
    void mouseMoveProcessor(int mouseX, int mouseY, vdkEvent msg);
#endif

void getArgument(const char *arguments, char *dest, int *i, int len);

#endif
