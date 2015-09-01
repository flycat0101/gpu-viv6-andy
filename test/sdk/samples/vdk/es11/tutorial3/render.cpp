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
#include <string.h>

#ifdef ANDROID_JNI
#include <utils/Log.h>
extern int width;
extern int height;
#endif

GLuint texture1;
GLuint texture2;
GLint maxAniso;
bool multiDrawElementsAvailable = false;
PFNGLMULTIDRAWELEMENTSEXTPROC _glMultiDrawElementsEXT;

/*******************************************************************************
*******************************************************************************/
#ifndef ANDROID_JNI
bool InitGLES()
{
    // Set clear colors.
    glClearColorx(FixedFromFloat(0.5f),
                  FixedFromFloat(0.5f),
                  FixedFromFloat(0.5f),
                  ONE);

    // Do not want to see smoothed colors, only a plain color for face.
    glShadeModel(GL_FLAT);

    // Enable the depth test in order to see the cube correctly.
    glEnable(GL_DEPTH_TEST);

    // Taking care of specifying correctly the winding order of the vertices
    // (counter clock wise order), we can cull up all back faces.  This is
    // probably one of the better optimizations we could work, because we are
    // avoiding a lot of computations that wont be reflected in the screen.
    glEnable(GL_CULL_FACE);

    // Set perspective matrix.
    SetPerspective();

    // Load door texture.
    texture1 = vdkLoadTexture(&egl, "door.tga", VDK_TGA, VDK_2D);
    if (texture1 == 0)
    {
        return false;
    }

    // Load fire texture.
    texture2 = vdkLoadTexture(&egl, "fire.tga", VDK_TGA, VDK_2D);
    if (texture2 == 0)
    {
        return false;
    }

    {
        char* extensions = (char *)glGetString(GL_EXTENSIONS);

        if (strstr(extensions, "GL_EXT_multi_draw_arrays") != 0)
        {
            _glMultiDrawElementsEXT = (PFNGLMULTIDRAWELEMENTSEXTPROC)eglGetProcAddress("glMultiDrawElementsEXT");
            if (_glMultiDrawElementsEXT != NULL)
            {
                multiDrawElementsAvailable = true;
            }
        }

#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
        if (strstr(extensions, "GL_EXT_texture_filter_anisotropic") != 0)
        {
            glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
        }
        else
#endif
        {
            maxAniso = 1;
        }
    }

	return true;
}
#endif
/*******************************************************************************
*******************************************************************************/

void Render()
{
    static int rotation = 0;

    // We are going to draw a cube centered at origin, and with an edge of 10
    // units.

    //             7                      6
    //              +--------------------+
    //            / |                   /|
    //          /   |                 /  |
    //     3  /     |             2 /    |
    //      +---------------------+      |
    //      |       |             |      |
    //      |       |             |      |
    //      |       |             |      |
    //      |       |             |      |
    //      |       |             |      |
    //      |       |             |      |
    //      |       |             |      |
    //      |       |4            |      |
    //      |       +-------------|------+ 5
    //      |     /               |     /
    //      |   /                 |   /
    //      | /                   | /
    //      +---------------------+
    //     0                      1

    // Clear the buffers.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Rotate cube.
    glLoadIdentity();
    glTranslatex(ZERO, ZERO, FixedFromInt(-20));
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

    // Enable texture 0.
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture1);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso);
#endif
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    static GLfixed texCoords[] = { ZERO, ZERO,
                                    ONE, ZERO,
                                    ONE,  ONE,
                                   ZERO,  ONE,
                                    ONE,  ONE,
                                   ZERO,  ONE,
                                   ZERO, ZERO,
                                    ONE, ZERO };
    glTexCoordPointer(2, GL_FIXED, 0, texCoords);

    // Enable texture 1.
    glActiveTexture(GL_TEXTURE1);
    glClientActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#ifdef GL_TEXTURE_MAX_ANISOTROPY_EXT
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
#endif
    glTexCoordPointer(2, GL_FIXED, 0, texCoords);

    // Add the textures.
    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);

    // Translate fire texture horizontally.
    static float offset = 0.0f;
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatex(FixedFromFloat(offset += 0.01f), ZERO, ZERO);
    glMatrixMode(GL_MODELVIEW);

	static GLubyte front[]  = { 2, 1, 3, 0 };
    static GLubyte back[]   = { 5, 6, 4, 7 };
    static GLubyte top[]    = { 6, 2, 7, 3 };
    static GLubyte bottom[] = { 1, 5, 0, 4 };
    static GLubyte left[]   = { 3, 0, 7, 4 };
    static GLubyte right[]  = { 6, 5, 2, 1 };

    if (multiDrawElementsAvailable)
    {
        static GLsizei count[6] = {4, 4, 4, 4, 4, 4};
        GLvoid** indices = (GLvoid**)malloc(6*sizeof(GLvoid*));
        indices[0] = front;
        indices[1] = back;
        indices[2] = top;
        indices[3] = bottom;
        indices[4] = left;
        indices[5] = right;

        // Render all the 6 faces.
        _glMultiDrawElementsEXT(GL_TRIANGLE_STRIP, count, GL_UNSIGNED_BYTE, (const GLvoid**)indices, 6);
        free(indices);
    }
    else
    {
        // Render front face.
	    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, front);

	    // Render back face.
	    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, back);

	    // Render top face.
	    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, top);

	    // Render bottom face.
	    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, bottom);

	    // Render left face.
	    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, left);

	    // Render right face.
	    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, right);
    }

    // Disable both texture arrays and vertex array (we are good citizens).
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
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
}

/*******************************************************************************
*******************************************************************************/

void CleanGLES()
{
    // Delete the textures.
    glDeleteTextures(1, &texture1);
    glDeleteTextures(1, &texture2);
}
