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


#include "tiger.h" //We need the defines and prototypes of there
#include "test_tiger_paths.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <VG/openvg.h>
VGPath *tigerPaths;
VGPaint tigerStroke;
VGPaint tigerFill;


//#define SAMPLES 4

int i32WindowWidth = 640;
int i32WindowHeight = 480;
int angle;

void animateTiger();

//----------------------------------------------------------------------------

VGboolean     zoomOut = VG_FALSE;
VGint         scaleCount = 0;

void vfFRAME_AppInit(int* width, int* height, char *title)
{
	if (title != NULL)
	{
		*width = i32WindowWidth;
		*height = i32WindowHeight;
		strcpy(title, "OpenVG tiger");
	}
	else
	{
		i32WindowWidth = *width;
		i32WindowHeight = *height;
	}
}

VGboolean vfFRAME_InitVG()
{
	int i;

	tigerPaths = (VGPath*)malloc(pathCount * sizeof(VGPath));
	if (!tigerPaths) return VG_FALSE;

	for (i=0; i<pathCount; ++i)
	{
		tigerPaths[i] = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
			1,0,0,0, VG_PATH_CAPABILITY_ALL);
		vgAppendPathData(tigerPaths[i], commandCounts[i],
			commandArrays[i], dataArrays[i]);
	}

	tigerStroke = vgCreatePaint();
	tigerFill = vgCreatePaint();
	vgSetPaint(tigerStroke, VG_STROKE_PATH);
	vgSetPaint(tigerFill, VG_FILL_PATH);

	vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
	vgTranslate((VGfloat)(i32WindowWidth/2 -50.0), (VGfloat)(i32WindowHeight/2 + 100.0));
	vgScale(1.0,-1.0);
	//vgScale(0.5,0.5);

	VGfloat clearColor[] = {1,1,1,1};
	vgSetfv(VG_CLEAR_COLOR, 4, clearColor);

	return VG_TRUE;

}

/*
 * Render the scene
 */
void vfFRAME_Render()
{
	int i;
	const VGfloat *style;

	vgClear(0,0,i32WindowWidth,i32WindowHeight);

	for (i=0; i<pathCount; ++i)
	{
		style = styleArrays[i];
		vgSetParameterfv(tigerStroke, VG_PAINT_COLOR, 4, &style[0]);
		vgSetParameterfv(tigerFill, VG_PAINT_COLOR, 4, &style[4]);
		vgSetf(VG_STROKE_LINE_WIDTH, style[8]);
		vgDrawPath((VGPath)tigerPaths[i], (VGint)style[9]);
	}

	animateTiger();
}

void animateTiger()
{
    if (zoomOut)
    {
        vgScale((VGfloat)1.25, (VGfloat)1.25);
        if (0 == --scaleCount) zoomOut = VG_FALSE;

    }
    else
    {
        vgScale((VGfloat)0.8, (VGfloat)0.8);
        if (5 == ++scaleCount) zoomOut = VG_TRUE;
    }

    vgRotate(5);

}

//----------------------------------------------------------------------------
void vfFRAME_Clean()
{

	int i;
	for (i=0; i<pathCount; ++i)
	{
		vgDestroyPath(tigerPaths[i]);
	}

	vgDestroyPaint(tigerStroke);
	vgDestroyPaint(tigerFill);

	free(tigerPaths);

}

bool keyprocessor(unsigned char key)
{
	return false;
}
#if USE_VDK
void mouseKeyProcessor(int mouseX, int mouseY, vdkEvent msg)
{
}

void mouseExtKeyProcessor(int mouseX, int mouseY, bool bKeyDown)
{
}

void mouseMoveProcessor(int mouseX, int mouseY, vdkEvent msg)
{
}
#endif


