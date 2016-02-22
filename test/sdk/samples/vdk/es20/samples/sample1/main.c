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
*	History : 2009.02.01, Created by qizhuang.liu.
*
*	TODO	: Change the read-pixel-copy-texture to rend into texture.
*/

#include "vdk_sample_common.h"

#ifndef ANDROID
/*
 * VDKS
*/

vdkEGL VdkEgl;

int VDKS_Val_WindowsWidth	= 320;
int VDKS_Val_WindowsHeight	= 240;

char* VDKS_ARG0 = NULL;
#endif
/*
*	Outside Routine
*/
extern VDKS_BOOL	TeapotInit();
extern void			TeapotPass();

extern VDKS_BOOL	SphereInit();
extern void			SpherePass();

extern VDKS_BOOL	BlendInit ();
extern void			BlendPass ();
#ifndef ANDROID
/*
*	Framebuffer & Render Target.
*/

GLuint FrameBuffer = 0;
GLuint RenBufColor = 0;
GLuint RenBufDepth = 0;
GLuint TexObjBackground = 0;
#endif
/*
*	Texture object to collect by main file.
*/

extern GLuint	TeapotTexUnitBackground;
extern GLuint	TeapotTexUnitFrontFront;
extern GLuint	TeapotTexUnitFrontBack;
extern GLuint	TeapotTexUnitBackBack;
extern GLuint	TeapotTexUnitBackFront;


extern GLuint	TeapotTexObjBackground;
extern GLuint	TeapotTexObjFrontFront;
extern GLuint	TeapotTexObjFrontBack;
extern GLuint	TeapotTexObjBackFront;
extern GLuint	TeapotTexObjBackBack;

/*
*	Teapot Rendering Program Control
*/

extern GLuint	TeapotProgramFrontFront;
extern GLuint	TeapotProgramFrontBack;
extern GLuint	TeapotProgramBackFront;
extern GLuint	TeapotProgramBackBack;

extern GLuint	TeapotProgram;

/*
*	Save Screen.
*/

int SaveResult = 0;

VDKS_BOOL Init()
{
	if (VDKS_TRUE != TeapotInit())
	{
		return VDKS_FALSE;
	}

	if (VDKS_TRUE != SphereInit())
	{
		return VDKS_FALSE;
	}

	if (VDKS_TRUE != BlendInit())
	{
		return VDKS_FALSE;
	}

	return VDKS_TRUE;
}

VDKS_BOOL SaveScreenMakeTexture(char* Name, GLuint TexUnit, GLuint TexObj)
{
	unsigned int size = sizeof(unsigned char) * VDKS_Val_WindowsWidth * VDKS_Val_WindowsHeight * 4;
	unsigned char * rgba = malloc(size);
	if (NULL == rgba)
	{
		printf("Error: out-of-memory.\n");
		return VDKS_FALSE;
	}

	glReadPixels(
		0, 0,
		VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		rgba);

	VDKS_Macro_CheckGLError();  /////

	if (TexObj)
	{
		/*
		*	Make texture.
		*/

		glActiveTexture(GL_TEXTURE0 + TexUnit);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glBindTexture(GL_TEXTURE_2D, TexObj);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);

		glActiveTexture(GL_TEXTURE0);

	}

	if (SaveResult)
	{
		/*
		*	This will change "rgba'.
		*/
		char szBMPFile[MAX_PATH + 1];
#ifndef ANDROID
		VDKS_Func_GetCurrentDir(szBMPFile);
#else
		strcpy(szBMPFile, "/sdcard/sample/sample1/");
#endif
		strcat(szBMPFile, "vdksample1_es20_");
		strcat(szBMPFile, Name);
		VDKS_Func_SaveBmp(szBMPFile, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight, rgba);
	}

	free(rgba);

	return VDKS_TRUE;
}

void Run ()
{


	/* Background */

	SpherePass();

	SaveScreenMakeTexture(
		"Background.bmp",
		TeapotTexUnitBackground,
		TeapotTexObjBackground);


	/* FrontFront */

	TeapotProgram = TeapotProgramFrontFront;

	TeapotPass();

	SaveScreenMakeTexture(
		"FrontFront.bmp",
		TeapotTexUnitFrontFront,
		TeapotTexObjFrontFront);

	/* FrontBack */

	TeapotProgram = TeapotProgramFrontBack;

	TeapotPass();

	SaveScreenMakeTexture(
		"FrontBack.bmp",
		TeapotTexUnitFrontBack,
		TeapotTexObjFrontBack);

	/* BackFront */

	TeapotProgram = TeapotProgramBackFront;

	TeapotPass();

	SaveScreenMakeTexture(
		"BackFront.bmp",
		TeapotTexUnitBackFront,
		TeapotTexObjBackFront);

	/* BackBack */

	TeapotProgram = TeapotProgramBackBack;

	TeapotPass();

	SaveScreenMakeTexture(
		"BackBack.bmp",
		TeapotTexUnitBackBack,
		TeapotTexObjBackBack);

	/* Blend */

	BlendPass();

	SaveScreenMakeTexture("result.bmp", 0, 0);
#ifndef ANDROID
    vdkSwapEGL(&VdkEgl);
#endif
	VDKS_Macro_CheckGLError();
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
