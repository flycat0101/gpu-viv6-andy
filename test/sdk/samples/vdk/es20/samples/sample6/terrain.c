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
 * VDK
*/
#ifndef ANDROID
extern vdkEGL VdkEgl;
#endif
/*
 * Model
*/
static const char *		PositionFile = "terrain_position.txt";
static float *			PositionFloats = NULL;
static size_t			PositionFloatsCount = 0;
static GLuint			PositionGLBuffer = 0;
static GLuint			PositionVertexAttributeArrayIndex = 0;

static const char *		NormalFile = "terrain_normal.txt";
static float *			NormalFloats = NULL;
static size_t			NormalFloatsCount = 0;
static GLuint			NormalGLBuffer = 0;
static GLuint			NormalVertexAttributeArrayIndex  = 1;

static const char *		TextureCoord0File = "terrain_texcoord0.txt";
static float *			TextureCoord0Floats = NULL;
static size_t			TextureCoord0FloatsCount = 0;
static GLuint			TextureCoord0GLBuffer = 0;
static GLuint			TextureCoord0VertexAttributeArrayIndex  = 2;

static const char *		TriangleFile = "terrain_triangles.txt";
static unsigned short *	TriangleData = NULL;
static size_t			TriangleVertexCount = 0;

static GLuint			TriangleGLBuffer = 0;

static float	CenterX = 0.0f;
static float	CenterY = 0.0f;
static float	CenterZ = 0.0f;

static float	MaxX = 0.0f;
static float	MaxY = 0.0f;
static float	MaxZ = 0.0f;

static float	MinX = 0.0f;
static float	MinY = 0.0f;
static float	MinZ = 0.0f;

static float	Radius	= 0.0f;

/*
 * Shaders
*/

static const char *	VSFile = "terrain.vert";
static const char *	FGFile = "terrain.frag";

static GLuint		Program = 0;

/*
 * Matrix
*/

extern const float	ModelView	[16];
extern const float	Projection	[16];
extern const float	MVP			[16];

/*
*	Texture
*/
static char*	 BaseTextureFile = "base.bmp";
static GLuint	 BaseTextureObject = 0;
#ifndef ANDROID
const static int BaseTextureUnit = 0;
#else
static const int BaseTextureUnit = 0;
#endif
/*
*	Miscellaneous Uniforms
*/

float UniformColor [] = {0.0f, 0.0f, 1.0f, 1.0f};

float UniformEyePosition [] = {0.0f, 0.0f, 400.0f};

GLuint	MVPLocation = 0;

VDKS_BOOL TerrainInitTexture()
{
	char szBaseTextureFile[MAX_PATH + 1];
#ifdef ANDROID
	strcpy(szBaseTextureFile, "/sdcard/sample/sample6/");
#else
	VDKS_Func_GetCurrentDir(szBaseTextureFile);
#endif
	strcat(szBaseTextureFile, BaseTextureFile);

	BaseTextureObject = VDKS_Func_Make2DTexture(szBaseTextureFile, NULL, NULL);
	assert(BaseTextureObject);
	return VDKS_TRUE;
}

VDKS_BOOL TerrainInit()
{
	char szCurDir[MAX_PATH + 1];
	char szVSFile[MAX_PATH + 1], szFGFile[MAX_PATH + 1], szTempFile[MAX_PATH + 1];
#ifdef ANDROID
    strcpy(szCurDir, "/sdcard/sample/sample6/");
#else
	VDKS_Func_GetCurrentDir(szCurDir);
#endif
	strcpy(szVSFile, szCurDir);
	strcpy(szFGFile, szCurDir);
	strcat(szVSFile, VSFile);
	strcat(szFGFile, FGFile);

	/*
	*	Texture
	*/

	TerrainInitTexture();

	/*
	 * Model Positon
	*/
	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, PositionFile);
	if (VDKS_TRUE !=  VDKS_ReadFloats (szTempFile, &PositionFloats, (SIZE_T *)&PositionFloatsCount))
	{
		printf("Init : Failed to read position data file.");
		return VDKS_FALSE;
	}

	glGenBuffers(1, &PositionGLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
	glBufferData(GL_ARRAY_BUFFER, PositionFloatsCount * sizeof(float), PositionFloats, GL_STATIC_DRAW);

	/*
	 * Model Postion Center & Radius
	*/

	VDKS_Func_ModelCenterRadius(PositionFloats, PositionFloatsCount, &CenterX, &CenterY, &CenterZ, &Radius);

	VDKS_Macro_AlertUser(0, "CenterX : %f, CenterY : %f, CenterZ : %f, Radius : %f\n",CenterX, CenterY, CenterZ, Radius);

	VDKS_Func_Model_LeftNearBottomRightFarTop(PositionFloats, PositionFloatsCount, &MaxX, &MaxY, &MaxZ, &MinX, &MinY, &MinZ);

	VDKS_Macro_AlertUser(0, "MaxX : %f, MaxY: %f, MaxZ: %f\nMinX : %f, MinY : %f, MinZ : %f\n", MaxX, MaxY, MaxZ, MinX, MinY, MinZ);

	/*
	*	Model Normal
	*/
    strcpy(szTempFile, szCurDir);
	strcat(szTempFile, NormalFile);
	if (VDKS_TRUE !=  VDKS_ReadFloats (szTempFile, &NormalFloats, (SIZE_T *)&NormalFloatsCount))
	{
		printf("Init : Failed to read position data file.");
		return VDKS_FALSE;
	}

	glGenBuffers(1, &NormalGLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, NormalGLBuffer);
	glBufferData(GL_ARRAY_BUFFER, NormalFloatsCount * sizeof(float), NormalFloats, GL_STATIC_DRAW);

	/*
	*	Model Texcoord
	*/
    strcpy(szTempFile, szCurDir);
	strcat(szTempFile, TextureCoord0File);
	if (VDKS_TRUE !=  VDKS_ReadFloats (szTempFile, &TextureCoord0Floats, (SIZE_T *)&TextureCoord0FloatsCount))
	{
		printf("Init : Failed to read position data file.");
		return VDKS_FALSE;
	}

	glGenBuffers(1, &TextureCoord0GLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, TextureCoord0GLBuffer);
	glBufferData(GL_ARRAY_BUFFER, TextureCoord0FloatsCount * sizeof(float), TextureCoord0Floats, GL_STATIC_DRAW);


	/*
	 * Model Triangle
	*/
    strcpy(szTempFile, szCurDir);
	strcat(szTempFile, TriangleFile);
	if (VDKS_TRUE != VDKS_ReadTriangle(szTempFile, &TriangleData, (SIZE_T *)&TriangleVertexCount))
	{
		printf("Init : Failed to read triangle data file.");
		return VDKS_FALSE;
	}

	glGenBuffers(1, &TriangleGLBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TriangleVertexCount * sizeof(unsigned short), TriangleData, GL_STATIC_DRAW);

	/*
	* Shaders
	*/
	Program = glCreateProgram();

	glBindAttribLocation(Program, PositionVertexAttributeArrayIndex, "position");

	glBindAttribLocation(Program, NormalVertexAttributeArrayIndex, "normal");

	glBindAttribLocation(Program, TextureCoord0VertexAttributeArrayIndex, "texcoord0");

	Program = VDKS_Func_MakeShaderProgram2(szVSFile, szFGFile, Program);

	if (Program == 0)
	{
		printf("Failed to create a new program.");
	}

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

				assert(location == PositionVertexAttributeArrayIndex);
			}
			else if (!strcmp(name, "texcoord0"))
			{
				assert(size == 1);
				assert(type == GL_FLOAT_VEC2);

				location = glGetAttribLocation(Program, name);

				assert(location == TextureCoord0VertexAttributeArrayIndex);
			}
			else
			{
				printf("The attribute name is not in handling.");
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

			if (!strcmp(name, "mvp"))
			{
				assert(size == 1);
				assert(type == GL_FLOAT_MAT4);

				MVPLocation = glGetUniformLocation(Program, name);
				glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, MVP);
			}
			else if(!strcmp(name, "base"))
			{
				assert(size == 1);
				assert(type == GL_SAMPLER_2D);

				location  = glGetUniformLocation(Program, name);

				glUniform1i(location, BaseTextureUnit);
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

	VDKS_Macro_CheckGLError();


	return VDKS_TRUE;
}

void TerrainPass()
{
	VDKS_Func_DisableAllVertexAttributeArray();

	glBindBuffer(GL_ARRAY_BUFFER, PositionGLBuffer);
	glVertexAttribPointer(PositionVertexAttributeArrayIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(PositionVertexAttributeArrayIndex);

	glBindBuffer(GL_ARRAY_BUFFER, TextureCoord0GLBuffer);
	glVertexAttribPointer(TextureCoord0VertexAttributeArrayIndex, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(TextureCoord0VertexAttributeArrayIndex);

	//glEnableVertexAttribArray(NormalVertexAttributeArrayIndex);

	/*
	*	Texture
	*/

	glActiveTexture(GL_TEXTURE0 + BaseTextureUnit);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, BaseTextureObject);

	/*
	*	Matrix
	*/
	do
	{
		float new_mvp [16];
		float rotate  [16];

		VDKS_Func_LoadIdentity(rotate);
		VDKS_Func_LoadIdentity(new_mvp);

		VDKS_Func_Matrix_Rotate(rotate, 90.0f, -1.0, 0.0, 0.0);

		VDKS_Func_Matrix_Mul4by4(new_mvp, MVP, rotate);

		glUseProgram(Program);
		glUniformMatrix4fv(MVPLocation, 1, GL_FALSE, new_mvp);
	}
	while(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);

	glDrawElements(GL_TRIANGLES, TriangleVertexCount, GL_UNSIGNED_SHORT, 0);

	VDKS_Macro_CheckGLError();

	glUseProgram(0);

	glFinish();
}

