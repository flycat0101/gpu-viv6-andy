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


#ifndef _COMMONMATH_H_
#define _COMMONMATH_H_

#include <GLES2/gl2.h>
#ifndef ANDROID_JNI
#ifndef M_PI
#define M_PI 3.14159265f
#endif
#endif
#define D3DX_PI 3.14159265f

typedef float FLOAT;

typedef unsigned long ULONG;

struct D3DXVECTOR3
{
	float x, y, z;
};

struct D3DXVECTOR4
{
	float x, y, z, w;
};

typedef struct _D3DMATRIX {
    float        _11, _12, _13, _14;
    float        _21, _22, _23, _24;
    float        _31, _32, _33, _34;
    float        _41, _42, _43, _44;
} D3DXMATRIX;

struct State
{
	D3DXVECTOR3 m_Eye,
				m_Lookat,
				m_UpVec,
				m_EyeVector;

	float		m_Time,
				m_TimeStep,
				m_StepsPerCircle;

	D3DXMATRIX  m_ViewMatrix,
				m_ProjMatrix,
				m_ScaleMatrix,
				m_RotateMatrix,
				m_MoveMatrix,
				m_WorldMatrix;

	int			FRAME_BUFFER_WIDTH,
				FRAME_BUFFER_HEIGHT;
};

// Setup matrices.
void SetupMatrices(
	State* renderState
	);

// Setup transformation maxtrix.
void SetupTransform(
	State* renderState,
	D3DXMATRIX* matrix
	);

void GetFrustum(
	GLfloat *matrix,
	GLfloat left,
	GLfloat right,
	GLfloat bottom,
	GLfloat top,
	GLfloat zNear,
	GLfloat zFar
	);


void MatMult(
	GLfloat* Result,
	GLfloat* Mat1,
	GLfloat Mat2[16]
	);

void Sphere(
	GLfloat** VertexArray,
	int Width,
	int Height,
	int Colors,
	GLfloat** ColorArray,
    int Textures,
	GLfloat** TextureArray,
	GLushort** IndexArray,
	int *numPrimitives,
	int* numIndices,
	int* numVertices
    );

// Set eye position.
void SetEye(
	State* state,
	float X,
	float Y,
	float Z
	);

// Set object scale in each direction.
void SetScale(
	State* state,
	float X,
	float Y,
	float Z
	);

// Set object rotation in each direction.
void SetRotation(
	State* state,
	float X,
	float Y,
	float Z
	);

// Set object 3D position in each direction.
void SetMove(
	State* state,
	float X,
	float Y,
	float Z
	);

// Set time step.
void SetTimeStep(
	State* state,
	ULONG TimeStep
	);

void InitializeRenderState(
	State* state,
	int width,
	int height
	);

#endif

