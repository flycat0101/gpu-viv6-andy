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
 * Mesa 3-D graphics library
 * Version:  3.5
 *
 * Copyright (C) 1999-2001  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _VDK_SAMPLES_COMMON_H_
#define _VDK_SAMPLES_COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <assert.h>
#include <stdarg.h>
#include <EGL/egl.h>
#ifndef ANDROID
#include <gc_vdk.h>
#endif
#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX_PATH
#define MAX_PATH 255
#endif

#ifndef SIZE_T
#define SIZE_T unsigned long
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define VDKS_DEFAULT_WIN_W	512
#define VDKS_DEFAULT_WIN_H	512

#define VDKS_PI 3.1415926f
/*
 * Types
*/
typedef int VDKS_BOOL;

#define VDKS_TRUE	1
#define VDKS_FALSE	0

/*
 * Debug
*/
extern char* VDKS_Arg0;

#define VDKS_DEBUG_STOP_RUNNING		1

int VDKS_Func_AlertUser(int StopRunning, char* FormatString, ...);

#define VDKS_Macro_AlertUser	\
	VDKS_Func_AlertUser(!VDKS_DEBUG_STOP_RUNNING, "%s, %d : \n\t", __FILE__, __LINE__);\
	VDKS_Func_AlertUser

#define VDKS_Macro_CheckGLError()	\
	VDKS_Func_CheckGLError(__FILE__, __LINE__)

VDKS_BOOL VDKS_Func_SaveScreen(char* Name);
void VDKS_Func_GetCurrentDir(char path[MAX_PATH + 1]);
/*
 * Matrix Operations
*/
void VDKS_Func_Matrix_LookAt	( float view[3], float focus[3], float up[3], float matrix[16] );
void VDKS_Func_Matrix_Mul4by4	( float *product, const float *a, const float *b );
void VDKS_Func_Matrix_Ortho		( float left, float right, float bottom, float top, float nearval, float farval, float *m );
void VDKS_Func_Matrix_Rotate	( float *mat, float angle, float x, float y, float z );
void VDKS_Func_Matrix_Translate	(float *mat, float x, float y, float z );
void VDKS_Func_Matrix_Transpose	(float *mat);
int  VDKS_Func_Matrix_Inverse	(const float src[16], float inverse[16]);
void VDKS_Func_Matrix_Frustum	(float * left_mat, float left, float right, float bottom, float top, float nearval, float farval);

void VDKS_Func_Frustum				(float left, float right, float bottom, float top, float nearval, float farval, float* m);
void VDKS_Func_Translate		(float *mat, float x, float y, float z);
void VDKS_Func_Rotate			(float *mat, float angle, float x, float y, float z);
void VDKS_Func_Matrix_Mul4by4	(float *product, const float *a, const float *b);
void VDKS_Func_Ortho			(float left, float right, float bottom, float top, float nearval, float farval, float *m);
void VDKS_Func_LookAt			(float view[3], float focus[3], float up[3], float matrix[16]);
void VDKS_Func_LoadIdentity		(float *mat);

/*
 * Shader & Program
*/

GLuint		VDKS_Func_MakeShaderProgram		(const char* vs_file, const char* fs_file);
GLuint		VDKS_Func_MakeShaderProgram2	(const char* vs_file, const char* fs_file, GLuint pro);
VDKS_BOOL	VDKS_Func_CompileShaderFile		(const char* file,	GLuint shader);
VDKS_BOOL	VDKS_Func_BufferFile			(const char* file,	char** buffer, int * size);

/*
 * EGL
*/
extern int VDKS_Val_WindowsWidth;
extern int VDKS_Val_WindowsHeight;
#ifndef ANDROID
VDKS_BOOL	VDKS_Init(vdkEGL * vdk_egl);
#endif
/*
 * Data
*/
VDKS_BOOL VDKS_ReadFloats		(const char* Path, float **Buffer, SIZE_T *Size);
VDKS_BOOL VDKS_ReadTriangle		(const char* Path, unsigned short **Buffer, SIZE_T *Size);

/*
 * Miscellaneous
*/

void VDKS_Func_ModelCenterRadius(float* Position, int PositionFloatCount, float* X, float* Y, float* Z, float * R);

void VDKS_Func_Model_LeftNearBottomRightFarTop(
	float* Position, int PositionFloatCount,
	float* MAX_X, float* MAX_Y, float* MAX_Z,
	float* MIN_X, float* MIN_Y, float* MIN_Z);

void VDKS_Func_CheckGLError(char* file, int line);

/*
 * BMP Default : RGBA
*/

void			VDKS_Func_SaveBmp(const char* fileName, int width, int height, void* pixels);
unsigned char * VDKS_Func_ReadBmp_Bpp(const char* filename, int * width, int * height, int *  bytepp);
unsigned char * VDKS_Func_ReadBmp(char* filename, int * width, int * height);
unsigned char * VDKS_Func_TransformBmp(unsigned char * Pixles, int width, int height, VDKS_BOOL X, VDKS_BOOL Y);

/*
 * Location Manager
*/

GLint	VDKS_Func_LocationManagerInit();
void	VDKS_Func_LocationManagerDestroy();

GLuint	VDKS_Func_LocationAcquire();
void	VDKS_Func_LocationRelease(GLuint Location);

void	VDKS_Func_DisableAllVertexAttributeArray();

/*
*	Vertex Attribute Array Manager
*/
typedef struct
{
	char *			name;
	int				in_use;
	GLenum			glessl_type;
	GLint			glessl_size;
	GLuint *		gl_bufobj_name;
	GLuint *		general_index;
	GLint			size_of_type;
	GLenum			type;
	GLboolean		normalized;
	GLsizei			stride;
	const void *	ptr;
}
VDKS_Struct_AttributeInformation;

void VDKS_Func_Program_PresetAttributesLocations(
	GLuint Program,
	VDKS_Struct_AttributeInformation* IndexBindings,
	int IndexBindingCount
	);

VDKS_BOOL VDKS_Func_Program_QueryActiveAttributesCheckConsistent(
	GLuint Program,
	VDKS_Struct_AttributeInformation* IndexBindings,
	int IndexBindingCount);

VDKS_BOOL VDKS_Func_BufferObject_SetUsage(
	VDKS_Struct_AttributeInformation* IndexBindings,
	int IndexBindingCount);

VDKS_BOOL VDKS_Func_ActiveAttribute_LocationEnable(
	VDKS_Struct_AttributeInformation* IndexBindings,
	int IndexBindingCount);

/*
*	Uniform Manager
*/
typedef struct
{
	GLuint *		program;
	char *			name;
	int				in_use;
	GLenum			glessl_type;
	GLint			glessl_size;
	GLuint			location;
	GLboolean		transpose;
	const void *	ptr;
}
VDKS_Struct_UnifomInfomation;

VDKS_BOOL VDKS_Func_Program_SettingUniforms(
	GLuint Program,
	VDKS_Struct_UnifomInfomation * Uniforms,
	int UniformCount
	);

VDKS_BOOL VDKS_Func_Program_ValidateUniformsGetLocations(
	GLuint Program,
	VDKS_Struct_UnifomInfomation * Uniforms,
	int UniformCount
	);

/*
*	Texture.
*/

GLuint	VDKS_Func_MakeRandAlpha2DTexture	(unsigned RandSeed, unsigned Size, unsigned WhiteDensity, unsigned WhiteRadius);
GLuint	VDKS_Func_Make2DTexture				(const char * BMPFile, int * Width, int * Height);

/*
*	TODO : Texture Unit Manager.
*/

#ifdef __cplusplus
} //extern "C" {
#endif

#endif
