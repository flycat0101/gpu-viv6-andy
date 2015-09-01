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


#include "render.h"
#include "fixedpoint.h"
#include <math.h>
#include <stdlib.h>

/*******************************************************************************
*******************************************************************************/
#ifdef ANDROID_JNI
extern int width;
extern int height;
#endif
bool InitGLES()
{
	// Set clear colors.
	glClearColorx(ZERO, ZERO, ZERO, ZERO);

	// Do not want to see smoothed colors, only a plain color for face.
	glShadeModel(GL_FLAT);

	// Enable the depth test in order to see the cube correctly.
	glEnable(GL_DEPTH_TEST);

	// Taking care of specifying correctly the winding order of the vertices
	// (counter clock wise order), we can cull up all back faces.  This is
	// probably one of the better optimizations we could work, because we are
	// avoiding a lot of computations that wont be reflected in the screen.
	glEnable(GL_CULL_FACE);

	// Start with orthographic matrix.
	SetOrtho();
	return true;
}

/*******************************************************************************
*******************************************************************************/

void Render()
{
	static int rotation = 0;

	// We are going to draw a cube centered at origin, and with an edge of 10
	// units.

	//			   7                      6
	//		        +--------------------+
	//	          / |                   /|
	//		    /   |                 /  |
	//	   3  /     |             2 /    |
	//	    +---------------------+      |
	//	    |       |             |      |
	//	    |       |             |      |
	//	    |       |             |      |
	//	    |       |             |      |
	//	    |       |             |      |
	//	    |       |             |      |
	//	    |       |             |      |
	//	    |       |4            |      |
	//	    |       +-------------|------+ 5
	//	    |     /               |     /
	//	    |   /                 |   /
	//	    | /                   | /
	//	    +---------------------+
	//	   0                      1

	// Clear the buffers.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Rotation cube.
	glLoadIdentity();
	if (!drawInOrtho)
	{
		// Move cube closer when rendering in perspective.
		glTranslatex(0, 0, FixedFromInt(-30));
	}
	glRotatex(FixedFromInt(45), ONE, ZERO, ZERO);
	glRotatex(FixedFromInt(rotation++), ZERO, ONE, ZERO);

	// Enable vertex array.
	glEnableClientState(GL_VERTEX_ARRAY);
	static GLshort vertices[] = { -5, -5, -5,
								   5, -5, -5,
								   5,  5, -5,
								  -5,  5, -5,
								  -5, -5,  5,
								   5, -5,  5,
								   5,  5,  5,
								  -5,  5,  5 };
	glVertexPointer(3, GL_SHORT, 0, vertices);

	// Render front face (red).
	glColor4x(ONE, ZERO, ZERO, ONE);
	static GLubyte front[] = { 2, 1, 3, 0 };
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, front);

	// Render back face (green).
	glColor4x(ZERO, ONE, ZERO, ONE);
	static GLubyte back[] = { 5, 6, 4, 7 };
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, back);

	// Render top face (blue).
	glColor4x(ZERO, ZERO, ONE, ONE);
	static GLubyte top[] = { 6, 2, 7, 3 };
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, top);

	// Render bottom face (yellow).
	glColor4x(ONE, ONE, ZERO, ONE);
	static GLubyte bottom[] = { 1, 5, 0, 4 };
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, bottom);

	// Render left face (cyan).
	glColor4x(ZERO, ONE, ONE, ONE);
	static GLubyte left[] = { 3, 0, 7, 4 };
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, left);

	// Render right face (magenta).
	glColor4x(ONE, ZERO, ONE, ONE);
	static GLubyte right[] = { 6, 5, 2, 1 };
	glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, right);

	// Disable vertex array (we are good citizens).
	glDisableClientState(GL_VERTEX_ARRAY);
#ifndef ANDROID_JNI
	// Display the frame.
	vdkSwapEGL(&egl);
#endif
}

/*******************************************************************************
*******************************************************************************/

void Perspective(GLfloat fovy, GLfloat aspect, GLfixed zNear, GLfixed zFar)
{
	// Compute field of view.
	GLfixed ymax = MultiplyFixed(zNear,
								 FixedFromFloat((GLfloat)tan(fovy * 3.1415962f / 360.0f)));
	GLfixed ymin = -ymax;

	if (aspect > 1.0f)
	{
		// Correct aspect ratio for landscape mode.
		aspect = 1.0f / aspect;
	}

	// Compute aspect ratio.
	GLfixed xmin = MultiplyFixed(ymin, FixedFromFloat(aspect));
	GLfixed xmax = MultiplyFixed(ymax, FixedFromFloat(aspect));

	// Set frustum.
	glFrustumx(xmin, xmax, ymin, ymax, zNear, zFar);
}

/*******************************************************************************
*******************************************************************************/

void SetOrtho()
{
	// Setup of the orthographic matrix.  We use an ortho cube centered at the
	// origin with 40 units of edge.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrthox(FixedFromInt(-20), FixedFromInt(20),
			 FixedFromInt(-20), FixedFromInt(20),
			 FixedFromInt(-20), FixedFromInt(20));
	glMatrixMode(GL_MODELVIEW);
#ifndef ANDROID_JNI
	// Change the window title.
	vdkSetWindowTitle(egl.window, "OpenGL ES 1.1 Tutorial 2 - Ortho");
#endif
}

/*******************************************************************************
*******************************************************************************/

void SetPerspective()
{
#ifndef ANDROID_JNI
	// Query the window dimensions.
	int width, height;

	vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);
#endif
	// Compute the ratio.
	float ratio = static_cast<float>(width) / static_cast<float>(height);

	// Set the frustum matrix width a 45 degree field of view and a depth of 40.
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	Perspective(45.0f, ratio, FixedFromInt(1), FixedFromInt(40));
	glMatrixMode(GL_MODELVIEW);
#ifndef ANDROID_JNI
	// Change the window title.
	vdkSetWindowTitle(egl.window, "OpenGL ES 1.1 Tutorial 2 - Perspective");
#endif
}

/*******************************************************************************
*******************************************************************************/

void CleanGLES()
{
}
