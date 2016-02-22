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


#include "Common.h"

int gWinSizeX     = 800;
int gWinSizeY     = 448;

int gRunRed       = 5;
int gRunGreen     = 6;
int gRunBlue      = 5;
int gRunAlpha     = 0;
int gRunDepth     = 16;
int gRunStencil   = 0;
int gRunSamples   = 0;
int gRunSampleBuf = 0;

EGLDisplay gDisplay = 0;
EGLSurface gSurface = 0;
EGLContext gContext = 0;
EGLConfig  gConfig = 0;
EGLNativeWindowType gNativeWindow =0;

char *gRunCaseName    = NULL;

//eglmode:
//  0, window mode (default)
//  1, pixmap mode
int gEglmode = 0;
int gVGmode = 1;
CaseRec cases[] =
{
    {tigerRun,  "tiger2DVG"},
    {NULL, ""}
};


void ConfigSetup(void)
{

}

long runCases()
{
	int i = 0;
	int specify = 0;

	for(i = 0; cases[i].casefunc != NULL; i++)
    {
		if (gRunCaseName == NULL || (specify = (strcmp(cases[i].casename, gRunCaseName) == 0)))
        {

          (	*(cases[i].casefunc))();
		}
		if(specify)
			break;
	}

	return 1;
}


