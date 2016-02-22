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


#include "vdk_sample_common.h"

/*
 * VDKS
*/
#ifndef ANDROID
vdkEGL VdkEgl;

int VDKS_Val_WindowsWidth = 512;
int VDKS_Val_WindowsHeight = 512;

char* VDKS_ARG0 = NULL;
#endif
/*
 * Test Data
*/

#define TEST_SIZE (0.3f)
static const float PositionData [] =
{
	TEST_SIZE, TEST_SIZE, 0.0,
	TEST_SIZE, -TEST_SIZE, 0.0,
	-TEST_SIZE, -TEST_SIZE, 0.0,
	-TEST_SIZE, TEST_SIZE, 0.0
};

static GLuint	PositionGLBuffer = 0;

static const unsigned short TriangleData [] =
{
	0, 1, 2, 3, 0, 2
};

static GLuint	TriangleGLBuffer = 0;

static float	CenterX = 0.0f;
static float	CenterY = 0.0f;
static float	CenterZ = 0.0f;

static float	Radius	= 0.0f;

/*
*	Shader
*/
const char *	VSFile = "test.vert";
const char *	FGFile = "test.frag";
static GLuint	Program = 0;

char szCurDir[MAX_PATH + 1];

/*
 * Matrix
*/

float	ModelView	[16];
float	Projection	[16];
float	MVP			[16];

VDKS_BOOL VDKS_TestInit()
{
	char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1];
	strcpy(szVSFile, szCurDir);
	strcpy(szFGFile, szCurDir);
	strcat(szVSFile, VSFile);
	strcat(szFGFile, FGFile);

	glGenBuffers(1, &PositionGLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PositionData), PositionData, GL_STATIC_DRAW);

	glGenBuffers(1, &TriangleGLBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TriangleData), TriangleData, GL_STATIC_DRAW);

	/*
	* Matrix
	*/
	do
	{
		float view	[3] = {0.0, 0.0, 200};
		float focus [3] = {0.0, 0.0, 0.0};
		float up	[3] = {0.0, 1.0, 0.0};

		/*
		* Assume
		*/
		assert (CenterX == 0.0f);
		assert (CenterY == 0.0f);
		assert (CenterZ == 0.0f);
		assert (Radius < 200.0f);

		VDKS_Func_LoadIdentity(ModelView);
		VDKS_Func_LoadIdentity(Projection);
		VDKS_Func_LoadIdentity(MVP);

		//VDKS_Func_Matrix_Rotate(ModelView, 88.0f, 0.0f, 1.0f, 1.0f);

		VDKS_Func_Matrix_LookAt(view, focus, up, ModelView);

		Radius = 1.0f;

		VDKS_Func_Matrix_Ortho(
			-1.0f * Radius, Radius,	/*left, right*/
			-1.0f * Radius, Radius,	/*bottom, up*/
			1.0, 400.0,				/*near, far*/
			Projection);

		VDKS_Func_Matrix_Mul4by4(MVP, Projection, ModelView);

		//VDKS_Func_Translate(ModelView, -1.0 * CenterX, -1.0 * CenterY, -1.0 * CenterZ);
	}
	while(0);

	/*
	* Shaders
	*/
	Program = VDKS_Func_MakeShaderProgram(szVSFile, szFGFile);

	if (Program == 0)
	{
		printf("Failed to make test program.");
		return VDKS_FALSE;
	}

	/*
	*	Attributes
	*/
	do
	{
		int		index = 0;
		GLint	nr_act_att = 0;
		char *	name  = NULL;
		GLsizei bufsize = 128;
		GLint	location = 0;
		GLint	position_location = 0;

		/*
		* Manage Location
		*/

		VDKS_Func_LocationManagerInit();

		position_location = VDKS_Func_LocationAcquire();

		if (position_location == 0)
		{
			printf("Failed to alloc location.");

			return VDKS_FALSE;
		}

		glBindAttribLocation(Program, position_location, "position");

		glLinkProgram(Program);

		/*
		* Post re-link
		*/

		glGetProgramiv(Program, GL_ACTIVE_ATTRIBUTES, &nr_act_att);

		name = malloc(bufsize);
		if (NULL == name) {
			printf("out-of-memory\n");
			return VDKS_FALSE;
		}

		for (index = 0; index < nr_act_att; index++)
		{
			GLsizei length;
			GLenum	type;
			GLint	size = 0;
			glGetActiveAttrib(Program, index, bufsize, &length, &size, &type, name);

			if (!strcmp(name, "position"))
			{
				assert(size == 1);
				assert(type == GL_FLOAT_VEC3);

				location = glGetAttribLocation(Program, name);

				assert( location == position_location);

				glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
				glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, 0, 0);
				glEnableVertexAttribArray(location);
			}
			else
			{
				printf("The uniform name is not in handling.");
				free(name);
				return VDKS_FALSE;
			}
		}

		free(name);
	}
	while(0);

	/*
	* Uniforms
	*/
	do
	{
		GLint nr_act_uni = 0;

		GLint i = 0;

		char *	name  = NULL;
		GLsizei bufsize = 128;
		GLint location = 0;

		name = malloc(bufsize);
		if (NULL == name) {
			printf("out-of-memory\n");
			return VDKS_FALSE;
		}

		glGetProgramiv(Program, GL_ACTIVE_UNIFORMS, &nr_act_uni);

		glUseProgram(Program);

		for(i = 0; i < nr_act_uni; i++)
		{
			GLsizei length;
			GLenum	type;
			GLint	size = 0;
			glGetActiveUniform(Program, i, bufsize, &length, &size, &type, name);

			if(!strcmp(name, "mvp"))
			{

				assert(size == 1);
				assert(type == GL_FLOAT_MAT4);

				location = glGetUniformLocation(Program, name);

				glUniformMatrix4fv(location, 1, GL_FALSE, MVP);
			}
			else
			{
				printf("The uniform name is not in handling.");
				free(name);
				return VDKS_FALSE;
			}
		}

		free(name);

		glUseProgram(0);
	}
	while(0);

	/*
	* Miscellaneous GL States
	*/

	return VDKS_TRUE;
}

void TestRun ()
{
	char szBMPFile[MAX_PATH + 1];
	strcpy(szBMPFile, szCurDir);
	strcat(szBMPFile, "vdksample10_es20_result.bmp");

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glUseProgram(Program);

	glDrawElements(GL_TRIANGLES, sizeof(TriangleData) / sizeof(unsigned short), GL_UNSIGNED_SHORT, 0);

	VDKS_Func_SaveScreen(szBMPFile);
#ifndef ANDROID
	vdkSwapEGL(&VdkEgl);
#endif
}

extern VDKS_BOOL	SphereInit();
extern void			SphereRun ();

VDKS_BOOL Init()
{
#ifdef ANDROID
	strcpy(szCurDir,"/sdcard/sample/sample10/");
#endif
	return VDKS_TestInit();
}

void Run ()
{
	TestRun();
}
#ifndef ANDROID
int main(int argc, char * argv[])
{
	int i = 0;
	int count = 0;

	vdkEvent vdk_event;
	VDKS_Func_GetCurrentDir(szCurDir);

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

	count = 1;
	if (argc > 1) {
		count = atoi(argv[1]);
	}

	if(count)
	{
		for (i = 0; i < count; i++)
		{
			Run();
		}
	}
	else
	{
		int done = 0;
		while(!done)
		{
			if (0 != vdkGetEvent(VdkEgl.window, &vdk_event))
			{
				if (vdk_event.type == VDK_CLOSE)
				{
					done = 1;
				}
				else
				{
					Run();
				}
			}
			else
			{
				Run();
			}
		}
	}


	vdkFinishEGL(&VdkEgl);

	return 0;
}
#endif
