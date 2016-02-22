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
 * 2D Data
*/

#define SIDE_SIZE (0.7f)

static const float PositionData [] =
{
	SIDE_SIZE, SIDE_SIZE, 0.0f,
	SIDE_SIZE, -SIDE_SIZE, 0.0f,
	-SIDE_SIZE, -SIDE_SIZE, 0.0f,
	-SIDE_SIZE, SIDE_SIZE, 0.0f,
};

static GLuint	PositionGLBuffer = 0;

static GLuint	PositionVertexAttributeArrayIndex = 0;

static const unsigned short TriangleData [] =
{
	0, 1, 2, 3, 0, 2
};

static GLuint	TriangleGLBuffer = 0;

/*
*	Shader
*/
static const char *	VSFile = "Blend.vert";
static const char *	FGFile = "Blend.frag";

static GLuint	Program = 0;


/*
 * Matrix
*/

static float	ModelView	[16];
static float	Projection	[16];
static float	MVP			[16];

/*
*	Texture
*/

extern GLuint	TeapotTexUnitBackground;
extern GLuint	TeapotTexUnitFrontFront;
extern GLuint	TeapotTexUnitFrontBack;
extern GLuint	TeapotTexUnitBackBack;
extern GLuint	TeapotTexUnitBackFront;

/*
*	Attributes Infomation
*/

//typedef struct
//{
//	char *			name;
//	int				in_use;
//	GLenum			glessl_type;
//	GLint			glessl_size;
//	GLuint *		gl_bufobj_name;
//	GLuint *		general_index;
//	GLint			size_of_type;
//	GLenum			type;
//	GLboolean		normalized;
//	GLsizei			stride;
//	const void *	ptr;
//}
//VDKS_Struct_AttributeInformation;

static VDKS_Struct_AttributeInformation AttributesInformation [] =
{
	{
		"position",
		0, /* in_use */
		GL_FLOAT_VEC3,
		1, /* glessl_size*/
		&PositionGLBuffer,
		&PositionVertexAttributeArrayIndex,
		3, /* size_of_type */
		GL_FLOAT,
		GL_FALSE, /* normalized */
		0, /* stride */
		0
	}
};

static int AttributesInformationCount = sizeof(AttributesInformation) / sizeof(AttributesInformation[0]);

//typedef struct
//{
//	GLuint *		program;
//	char *			name;
//	int				in_use;
//	GLenum			glessl_type;
//	GLint			glessl_size;
//	GLuint			location;
//	GLboolean		transpose;
//	const void *	ptr;
//}
//VDKS_Struct_UnifomInfomation;

static VDKS_Struct_UnifomInfomation UniformsInformation [] =
{
	{
		&Program,
		"background",
		0, /* in_use */
		GL_SAMPLER_2D,
		1,
		0, /* location */
		GL_FALSE, /* transpose */
		&TeapotTexUnitBackground
	},
	{
		&Program,
		"front_front",
		0, /* in_use */
		GL_SAMPLER_2D,
		1,
		0, /* location */
		GL_FALSE, /* transpose */
		&TeapotTexUnitFrontFront
	},
	{
		&Program,
		"front_back",
		0, /* in_use */
		GL_SAMPLER_2D,
		1,
		0, /* location */
		GL_FALSE, /* transpose */
		&TeapotTexUnitFrontBack
	},
	{
		&Program,
		"back_front",
		0, /* in_use */
		GL_SAMPLER_2D,
		1,
		0, /* location */
		GL_FALSE, /* transpose */
		&TeapotTexUnitBackFront
	},
	{
		&Program,
		"back_back",
		0, /* in_use */
		GL_SAMPLER_2D,
		1,
		0, /* location */
		GL_FALSE, /* transpose */
		&TeapotTexUnitBackBack
	},
};

static int UniformsInformationCount = sizeof(UniformsInformation) / sizeof(UniformsInformation[0]);

VDKS_BOOL BlendInit()
{
	char szCurDir[MAX_PATH + 1];
	char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1];
#ifndef ANDROID
	VDKS_Func_GetCurrentDir(szCurDir);
#else
	strcpy(szCurDir, "/sdcard/sample/sample1/");
#endif
	strcpy(szVSFile, szCurDir);
	strcpy(szFGFile, szCurDir);
	strcat(szVSFile, VSFile);
	strcat(szFGFile, FGFile);

	/*
	*	Make buffer object.
	*/
	glGenBuffers(1, &PositionGLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PositionData), PositionData, GL_STATIC_DRAW);

	glGenBuffers(1, &TriangleGLBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(TriangleData), TriangleData, GL_STATIC_DRAW);

	/*
	*	Alloc location.
	*/

	VDKS_Func_LocationManagerInit();

	PositionVertexAttributeArrayIndex = VDKS_Func_LocationAcquire();

	assert (PositionVertexAttributeArrayIndex);

	/*
	*	Set the location binding & usage of the buffer object.
	*/

	VDKS_Func_BufferObject_SetUsage(AttributesInformation, AttributesInformationCount);

	/*
	*	Uniform(s)
	*/

	/*
	* Matrix
	*/
	do
	{
		float view	[3] = {0.0, 0.0, 200};
		float focus [3] = {0.0, 0.0, 0.0};
		float up	[3] = {0.0, 1.0, 0.0};

		float Radius = 1.0f;

		VDKS_Func_LoadIdentity(ModelView);
		VDKS_Func_LoadIdentity(Projection);
		VDKS_Func_LoadIdentity(MVP);

		VDKS_Func_Matrix_LookAt(view, focus, up, ModelView);

		VDKS_Func_Matrix_Ortho(
			-1.0f * Radius, Radius,	/*left, right*/
			-1.0f * Radius, Radius,	/*bottom, up*/
			1.0f, 400.0f,				/*near, far*/
			Projection);

		VDKS_Func_Matrix_Mul4by4(MVP, Projection, ModelView);
	}
	while(0);

	/*
	*	Program & Shader
	*/
	Program = glCreateProgram();

	assert (Program);

	VDKS_Func_Program_PresetAttributesLocations(Program, AttributesInformation, AttributesInformationCount);

	Program = VDKS_Func_MakeShaderProgram2(szVSFile, szFGFile, Program);

	assert(Program);

	VDKS_Func_Program_ValidateUniformsGetLocations(
		Program,
		UniformsInformation,
		UniformsInformationCount);

	return VDKS_TRUE;
}


void BlendPass ()
{
	GLint program_status = 0;

	VDKS_Func_DisableAllVertexAttributeArray();

	/*
	*	Determin which attributes is active in this program.
	*/
	VDKS_Func_Program_QueryActiveAttributesCheckConsistent(
		Program,
		AttributesInformation,
		AttributesInformationCount);

	/*
	*	Enable the location(s) we need.
	*/
	VDKS_Func_ActiveAttribute_LocationEnable(
		AttributesInformation,
		AttributesInformationCount);

	/*
	*	Download uniforms here for we may change some uniform data outsides.
	*/

	VDKS_Func_Program_SettingUniforms(
		Program,
		UniformsInformation,
		UniformsInformationCount);

	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glValidateProgram(Program);

	glGetProgramiv(Program, GL_VALIDATE_STATUS, &program_status);

	if (program_status != GL_TRUE)
	{
		printf("The program is not valid now.");
		return;
	}

	glUseProgram(Program);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);

	glDrawElements(GL_TRIANGLES, sizeof(TriangleData) / sizeof(unsigned short), GL_UNSIGNED_SHORT, 0);

	glUseProgram(0);
}

