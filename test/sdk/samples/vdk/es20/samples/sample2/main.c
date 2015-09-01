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


/*
*	History : 2009.02.01, Created by qizhuang.liu.
*			  2009.02.01, Finished by qizhuang.liu.
*/

#include "vdk_sample_common.h"

#ifndef ANDROID
/*
 * VDKS
*/

vdkEGL VdkEgl;

int VDKS_Val_WindowsWidth = 0;
int VDKS_Val_WindowsHeight = 0;

char* VDKS_ARG0 = NULL;
#endif
/*
*	Outside Routine
*/
extern VDKS_BOOL	Tex2DInit();
extern void			Tex2DPass();

/*
*	Save Screen.
*/

int SaveResult = 0;

VDKS_BOOL Init()
{
	return Tex2DInit();
}

void Run ()
{
	unsigned int size = sizeof(unsigned char) * VDKS_Val_WindowsWidth * VDKS_Val_WindowsHeight * 4;
	unsigned char * rgba = malloc(size);
	if (rgba == NULL) {
		printf("Run : Failed to malloc");
		return;
	}

	Tex2DPass();

	if (SaveResult)
	{
		char szBMPFile[MAX_PATH + 1];
		glReadPixels(
			0, 0,
			VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight,
			GL_RGBA,
			GL_UNSIGNED_BYTE,
			rgba);
#ifndef ANDROID
		VDKS_Func_GetCurrentDir(szBMPFile);
#else
		strcpy(szBMPFile, "/sdcard/sample/sample2/");
#endif
		strcat(szBMPFile, "vdksample2_es20_result.bmp");
		VDKS_Func_SaveBmp(szBMPFile, VDKS_Val_WindowsWidth, VDKS_Val_WindowsHeight, rgba);

	}

	free(rgba);
#ifndef ANDROID
	vdkSwapEGL(&VdkEgl);
#endif
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
