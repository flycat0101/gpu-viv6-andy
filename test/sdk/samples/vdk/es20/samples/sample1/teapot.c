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
*	History : 2009.02.02, Created by qizhuang.liu.
*/

#include "teapot.h"

/*
*	Make program & set attributes' locations.
*/
GLuint TeapotMakeProgramPresetLocation(
	const char* VSFile,
	const char* FGFile,
	VDKS_Struct_AttributeInformation* AttributesInformation,
	int AttributesInformationCount
	)
{
	GLuint Program = glCreateProgram();
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
	*	Set assumed attribute location before glLinkProgram.
	*/

	VDKS_Func_Program_PresetAttributesLocations(
		Program,
		AttributesInformation,
		AttributesInformationCount);

	Program = VDKS_Func_MakeShaderProgram2(szVSFile, szFGFile, Program);
	if (Program == 0)
	{
		printf("Failed to create a new program.");
	}

	return Program;
}

void TeapotInitTextureUnit(GLuint Tex)
{
	glActiveTexture(GL_TEXTURE0 + Tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glActiveTexture(GL_TEXTURE0);
}

/*
*	Buffer Attributes Data in to buffer object, and alloc locations & set usage of the bufobj.
*/
VDKS_BOOL TeapotInit()
{
	char szCurDir[MAX_PATH + 1];
	char szTempFile[MAX_PATH + 1];
#ifndef ANDROID
	VDKS_Func_GetCurrentDir(szCurDir);
#else
	strcpy(szCurDir, "/sdcard/sample/sample1/");
#endif
	/*
	* Positon
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
	*	TexCoords.
	*/
	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, TextureCoord_0_File);
	if (VDKS_TRUE != VDKS_ReadFloats (szTempFile, &TextureCoord_0_Floats, (SIZE_T *)&TextureCoord_0_FloatsCount))
	{
		printf("Init : Failed to read position data file.");
		return VDKS_FALSE;
	}

	glGenBuffers(1, &TextureCoord_0_GLBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, TextureCoord_0_GLBuffer);
	glBufferData(GL_ARRAY_BUFFER, TextureCoord_0_FloatsCount * sizeof(float), TextureCoord_0_Floats, GL_STATIC_DRAW);

	/*
	 * Triangles
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
	 * Debug, Model Center & Radius
	*/

	VDKS_Func_ModelCenterRadius(PositionFloats, PositionFloatsCount, &CenterX, &CenterY, &CenterZ, &Radius);

	VDKS_Macro_AlertUser(0, "CenterX : %f, CenterY : %f, CenterZ : %f, Radius : %f\n",CenterX, CenterY, CenterZ, Radius);

	/*
	 * Matrix
	*/
	do
	{
		float view [3] = {0.0, 0.0, 200};
		float focus [3] = {0.0, 0.0, 0.0};
		float up [3] = {0.0, 1.0, 0.0};

		VDKS_Func_LoadIdentity(ModelView);
		VDKS_Func_LoadIdentity(Projection);
		VDKS_Func_LoadIdentity(MVP);

		VDKS_Func_Matrix_LookAt(view, focus, up, ModelView);

		VDKS_Func_Matrix_Ortho(
			-1.0f * Radius, Radius, /*left, right*/
			-1.0f * Radius, Radius, /*bottom, up*/
			1.0, 400.0, /*near, far*/
			Projection);

		VDKS_Func_Matrix_Mul4by4(MVP, Projection, ModelView);

		//VDKS_Func_Translate(ModelView, -1.0f * CenterX, -1.0f * CenterY, -1.0f * CenterZ);
	}
	while(0);

	/*
	*	Program(s)
	*/

	VDKS_Func_LocationManagerInit();

	PositionVertexAttributeArrayIndex = VDKS_Func_LocationAcquire();

	assert (PositionVertexAttributeArrayIndex);

	TextureCoord_0_VertexAttributeArrayIndex = VDKS_Func_LocationAcquire();

	assert (TextureCoord_0_VertexAttributeArrayIndex);

	/*
	*	As we have set the location name, so we can set the approprate index and usage of the buffer object in the pre-final
	*	vertex attribute array(s?) set.
	*/

	VDKS_Func_BufferObject_SetUsage(AttributesInformation, sizeof(AttributesInformation) / sizeof(AttributesInformation[0]));

	TeapotProgramFrontBack = TeapotMakeProgramPresetLocation(
		FrontBackVSFile,
		FrontBackFGFile,
		AttributesInformation,
		AttributesInformationCount
		);

	TeapotProgramBackFront = TeapotMakeProgramPresetLocation(
		BackFrontVSFile,
		BackFrontFGFile,
		AttributesInformation,
		AttributesInformationCount
		);

	TeapotProgramFrontFront = TeapotMakeProgramPresetLocation(
		FrontFrontVSFile,
		FrontFrontFGFile,
		AttributesInformation,
		AttributesInformationCount
		);

	TeapotProgramBackBack = TeapotMakeProgramPresetLocation(
		BackBackVSFile,
		BackBackFGFile,
		AttributesInformation,
		AttributesInformationCount
		);

	/*
	*	Texture Unit
	*/
	TeapotInitTextureUnit(TeapotTexUnitBackground);
	TeapotInitTextureUnit(TeapotTexUnitFrontFront);
	TeapotInitTextureUnit(TeapotTexUnitFrontBack);
	TeapotInitTextureUnit(TeapotTexUnitBackFront);
	TeapotInitTextureUnit(TeapotTexUnitBackBack);
	TeapotInitTextureUnit(TexUnitBase);

	/*
	*	Texture Object
	*/

	glGenTextures(1, &TeapotTexObjBackground);
	assert (TeapotTexObjBackground != 0);

	glGenTextures(1, &TeapotTexObjFrontFront);
	assert (TeapotTexObjFrontFront != 0);

	glGenTextures(1, &TeapotTexObjFrontBack);
	assert (TeapotTexObjFrontBack != 0);

	glGenTextures(1, &TeapotTexObjBackFront);
	assert (TeapotTexObjBackFront != 0);

	glGenTextures(1, &TeapotTexObjBackBack);
	assert (TeapotTexObjBackBack != 0);

	/*
	*	Texture "base"
	*/
	strcpy(szTempFile, szCurDir);
	strcat(szTempFile, TexImgBaseFile);
	do
	{
		int w, h;

		//int wi, hi;

		unsigned char * img = NULL;

		img = VDKS_Func_ReadBmp((char *)szTempFile/*TexImgBaseFile*/, &w, &h);
		if (NULL == img)
		{
			return VDKS_FALSE;
		}

		//for (hi = 0; hi < h; hi++)
		//{
		//	for (wi = 0; wi < w; wi++)
		//	{
		//		img[hi * w * 4 + wi * 4 + 0] = img[hi * w * 4 + wi * 4 + 3];
		//		img[hi * w * 4 + wi * 4 + 1] = img[hi * w * 4 + wi * 4 + 3];
		//		img[hi * w * 4 + wi * 4 + 2] = img[hi * w * 4 + wi * 4 + 3];
		//		img[hi * w * 4 + wi * 4 + 3] = img[hi * w * 4 + wi * 4 + 3];
		//		//img[hi * w * 4 + wi * 4 + 3] = 255;
		//	}
		//}

		//VDKS_Func_SaveBmp("test.bmp",w, h, img);

		glGenTextures(1, &TexObjBase);
		assert(TexObjBase);

		glActiveTexture(GL_TEXTURE0 + TexUnitBase);

		glBindTexture(GL_TEXTURE_2D, TexObjBase);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

		//glGenerateMipmap(GL_TEXTURE_2D);

		glActiveTexture(GL_TEXTURE0);
	}
	while(0);

	return VDKS_TRUE;
}

void TeapotPassProgram ()
{
	/*
	*	Get Active Attributes
	*/

	VDKS_Func_Program_QueryActiveAttributesCheckConsistent(
		TeapotProgram,
		AttributesInformation,
		AttributesInformationCount
		);

	/*
	*	Enable the vertex attribute array locations.
	*/

	VDKS_Func_ActiveAttribute_LocationEnable(
		AttributesInformation,
		AttributesInformationCount
		);

	/*
	*	Get Active Uniforms.
	*/

	VDKS_Func_Program_ValidateUniformsGetLocations(
		TeapotProgram,
		UnifomsInfomation,
		UnifomsInfomationCount
		);

	VDKS_Func_Program_SettingUniforms(
		TeapotProgram,
		UnifomsInfomation,
		UnifomsInfomationCount
		);
}

void TeapotPass ()
{
	GLint program_status = 0;

	TeapotPassProgram();

	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	assert(TeapotProgram);

	glValidateProgram(TeapotProgram);

	glGetProgramiv(TeapotProgram, GL_VALIDATE_STATUS, &program_status);

	if (program_status != GL_TRUE)
	{
		printf("The program is not valid now.");
		return;
	}

	glUseProgram(TeapotProgram);

	/*
	*	Other init routine may edit our GL_ELEMENT_ARRAY_BUFFER binding.
	*/
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleGLBuffer);

	glDrawElements(GL_TRIANGLES, TriangleVertexCount, GL_UNSIGNED_SHORT, 0);

	glFlush();

	glUseProgram(0);
}

