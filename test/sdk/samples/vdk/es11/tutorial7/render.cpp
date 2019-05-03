/****************************************************************************
*
*    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
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


#include <math.h>
#include "render.h"
#include "fixed_point.h"
#include "mesh.h"
#include "font.h"
#include "texture.h"
#include "timer.h"
#include <stdlib.h>

#ifdef ANDROID_JNI
#include <stdio.h>
extern int width;
extern int height;
extern void *LoadTGA(FILE * File,GLenum * Format,GLsizei * Width,GLsizei * Height);
#endif


Mesh * mesh = NULL;
BitmappedFont * font = NULL;
Texture * meshTexture = NULL;
Timer * timer = NULL;
GLuint texture;

bool InitGLES()
{
    glClearColorx(FixedFromFloat(0.5f), FixedFromFloat(0.5f), FixedFromFloat(0.5f), ONE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    SetPerspective();
    // Load all needed resources and create needed objects.
    timer = new Timer();
#ifndef ANDROID_JNI
    texture = vdkLoadTexture(&egl, "font.tga", VDK_TGA, VDK_2D);
#else
    FILE* file;
    void * bits;
    GLenum  format;
    GLsizei width;
    GLsizei height;
    GLsizei bytes;

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    file = fopen("/sdcard/es11/tutorial7/font.tga", "rb");
    bits = LoadTGA(file, &format, &width, &height);
    glTexParameteri( GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (bits != NULL)
        glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,bits);
    else
        return false;
    fclose(file);
#endif
    if (texture == 0) return false;
    font = new BitmappedFont(texture);
#ifndef ANDROID_JNI
    mesh = new Mesh("knot.gsd", true);
    if (!mesh->GetState()) return false;
    meshTexture = new Texture("fire.tga",GL_LINEAR_MIPMAP_LINEAR);
#else
    mesh = new Mesh("/sdcard/es11/tutorial7/knot.gsd", true);
    if (!mesh->GetState()) return false;
    meshTexture = new Texture("/sdcard/es11/tutorial7/fire.tga",GL_LINEAR_MIPMAP_LINEAR);
#endif
    if (!meshTexture->GetState()) return false;
    return true;
}

//----------------------------------------------------------------------------
void Render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup a good point of view.
    SetPerspective();
    glLoadIdentity();
    glTranslatef(0, 0, -100);
    glRotatef(10, 1, 0, 0);
    static GLfloat angle = 0;
    glRotatef(angle, 0, 1, 0);
    GLfloat speed = timer->GetTimeBetweenTwoFrames() / 100.0f;
    angle += speed;

    // Draw the mesh.
    glEnable(GL_TEXTURE_2D);
    meshTexture->BindTexture();
    mesh->Draw();
    glDisable(GL_TEXTURE_2D);

    // Draw the fps line.
    SetOrtho2D();
      glLoadIdentity();
    glColor4x(ONE, ONE, ONE, ONE);
    BitmappedFont::EnableStates();
    timer->UpdateTimer();
    font->Print(0,0,"FPS: %.2f",timer->GetFPS());
    BitmappedFont::DisableStates();
#ifndef ANDROID_JNI
//  glFinish();
    vdkSwapEGL(&egl);
#endif
}

//----------------------------------------------------------------------------

void CleanGLES()
{
    if (mesh != NULL)
    {
        delete mesh;
    }

    if (font != NULL)
    {
        delete font;
    }

    if (meshTexture != NULL)
    {
        delete meshTexture;
    }

    if (timer != NULL)
    {
        delete timer;
    }

    if (texture != 0)
    {
        glDeleteTextures(1, &texture);
    }
}

//----------------------------------------------------------------------------

void SetOrtho2D()
{
#ifndef ANDROID_JNI
    // Query the window dimensions.
    int width, height;
    vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);
#endif
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthox(FixedFromInt(0), FixedFromInt(width),
             FixedFromInt(0), FixedFromInt(height),
             FixedFromInt(-1), FixedFromInt(1));
    glMatrixMode(GL_MODELVIEW);
}

//----------------------------------------------------------------------------

void Perspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
{
    // Compute field of view.
    GLfloat ymax = zNear * (GLfloat)tan(fovy * 3.1415962f / 360.0f);
    GLfloat ymin = -ymax;

    if (aspect > 1.0f)
    {
        // Correct aspect ratio for landscape mode.
        aspect = 1.0f / aspect;
    }

    // Compute aspect ratio.
    GLfloat xmin = ymin * aspect;
    GLfloat xmax = ymax * aspect;

    // Set frustum.
    glFrustumf(xmin, xmax, ymin, ymax, zNear, zFar);
}

//----------------------------------------------------------------------------

void SetPerspective()
{
#ifndef ANDROID_JNI
    // Query the window dimensions.
    int width, height;
    vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);
#endif
    // Compute the ratio.
    float ratio = static_cast<float>(width) / static_cast<float>(height);

    // Set the frustum matrix width a 45 degree field of view and a depth of
    // 200.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    Perspective(45.0f, ratio, 1.0f, 200.0f);
    glMatrixMode(GL_MODELVIEW);
}
