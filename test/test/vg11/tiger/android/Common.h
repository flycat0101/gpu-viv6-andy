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


#ifndef COMMON_H
#define COMMON_H

#include<EGL/egl.h>
extern int gWinSizeX;
extern int gWinSizeY;

extern int gRunRed;
extern int gRunGreen;
extern int gRunBlue;
extern int gRunAlpha;
extern int gRunDepth;
extern int gRunStencil;
extern int gRunSamples;
extern int gRunSampleBuf;

extern EGLDisplay gDisplay;
extern EGLSurface gSurface;
extern EGLContext gContext;
extern EGLConfig  gConfig;
extern EGLNativeWindowType gNativeWindow;

extern char * gRunCaseName;
extern int gEglmode;
extern int gVGmode;
typedef struct _CaseRec {
    void (*casefunc)();
    char * casename;
} CaseRec;

extern CaseRec cases[];
#ifdef __cplusplus
extern "C" {
#endif

long runCases();

////#### Test Cases ####////
void tigerRun();


#ifdef __cplusplus
}
#endif


#endif //SHELL_H


