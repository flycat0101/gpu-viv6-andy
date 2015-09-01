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


#include <math.h>
#include <stdlib.h>
#include "render.h"
#include "font.h"
#include "fixed_point.h"
#ifdef ANDROID_JNI
#include <stdio.h>
#include <math.h>
#include <time.h>
extern int width;
extern int height;
extern void *LoadTGA(FILE * File,GLenum * Format,GLsizei * Width,GLsizei * Height);
static struct timeval tm;
static long currenttime = 0;
#endif
// Texture handle.
GLuint texture = 0;

// Bitmapped font.
BitmappedFont * font = NULL;

static struct
{
    char letter;
    int x, y;
    int final_x;
    int final_y;
}
letters[]=
{
    { 6 , 0, 0, 0, 0 },
    { 'H', 0, 0, 0, 0 },
    { 'e', 0, 0, 0, 0 },
    { 'l', 0, 0, 0, 0 },
    { 'l', 0, 0, 0, 0 },
    { 'o', 0, 0, 0, 0 },
    { '!', 0, 0, 0, 0 },
    { 12 , 0, 0, 0, 0 },
    { 'C', 0, 0, 0, 0 },
    { 'a', 0, 0, 0, 0 },
    { 'n', 0, 0, 0, 0 },
    { ' ', 0, 0, 0, 0 },
    { 'y', 0, 0, 0, 0 },
    { 'o', 0, 0, 0, 0 },
    { 'u', 0, 0, 0, 0 },
    { ' ', 0, 0, 0, 0 },
    { 'r', 0, 0, 0, 0 },
    { 'e', 0, 0, 0, 0 },
    { 'a', 0, 0, 0, 0 },
    { 'd', 0, 0, 0, 0 },
    { 11 , 0, 0, 0, 0 },
    { 't', 0, 0, 0, 0 },
    { 'h', 0, 0, 0, 0 },
    { 'i', 0, 0, 0, 0 },
    { 's', 0, 0, 0, 0 },
    { ' ', 0, 0, 0, 0 },
    { 'n', 0, 0, 0, 0 },
    { 'o', 0, 0, 0, 0 },
    { 'w', 0, 0, 0, 0 },
    { ' ', 0, 0, 0, 0 },
    { 'o', 0, 0, 0, 0 },
    { 'n', 0, 0, 0, 0 },
    { 11 , 0, 0, 0, 0 },
    { 't', 0, 0, 0, 0 },
    { 'h', 0, 0, 0, 0 },
    { 'e', 0, 0, 0, 0 },
    { ' ', 0, 0, 0, 0 },
    { 's', 0, 0, 0, 0 },
    { 'c', 0, 0, 0, 0 },
    { 'r', 0, 0, 0, 0 },
    { 'e', 0, 0, 0, 0 },
    { 'e', 0, 0, 0, 0 },
    { 'n', 0, 0, 0, 0 },
    { '?', 0, 0, 0, 0 },
};

int random(int max)
{
    return rand() % max;
}

bool InitGLES()
{
    glClearColorx(FixedFromFloat(0.5f), FixedFromFloat(0.5f), FixedFromFloat(0.5f), ONE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    SetOrtho2D();
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

    file = fopen("/sdcard/es11/tutorial5/font.tga", "rb");
    bits = LoadTGA(file, &format, &width, &height);
    glTexParameteri( GL_TEXTURE_2D,GL_GENERATE_MIPMAP,GL_TRUE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (bits != NULL)
	    glTexImage2D(GL_TEXTURE_2D,0,format,width,height,0,format,GL_UNSIGNED_BYTE,bits);
    else
        return false;

	fclose(file);
#endif
    if (texture == 0)
    {
        return false;
    }

    font = new BitmappedFont(texture);
#ifndef ANDROID_JNI
    int width, height;
    vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);

    srand(vdkGetTicks());
#else
    gettimeofday(&tm, NULL);
	currenttime = tm.tv_sec * 1000 + tm.tv_usec / 1000;
	srand(currenttime);
#endif
    int n = 0;
    int final_x = 0, final_y = (height - 4 * 15) / 2 - 15;
    for (unsigned int i = 0; i < sizeof(letters) / sizeof(letters[0]); ++i)
    {
        if (n-- == 0)
        {
            n = letters[i].letter;
            final_x = (width - n * 15) / 2;
            final_y += 15;
        }
        else
        {
            letters[i].final_x = final_x;
            letters[i].final_y = final_y;
            letters[i].x       = random(width);
            letters[i].y       = random(height);

            final_x += 15;
        }
    }

    return true;
}

//----------------------------------------------------------------------------

void Render()
{
#ifndef ANDROID_JNI
    int width, height;
    vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);
#endif
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    glColor4x(ZERO, ONE, ZERO, ONE);
    BitmappedFont::EnableStates();

    int n = 0;
    for (unsigned int i = 0; i < sizeof(letters) / sizeof(letters[0]); ++i)
    {
        if (n-- == 0)
        {
            n = letters[i].letter;
        }
        else
        {
            font->Print(letters[i].x,
                        height - letters[i].y,
                        "%c", letters[i].letter);

            if (letters[i].x > letters[i].final_x)
            {
                --letters[i].x;
            }
            else if (letters[i].x < letters[i].final_x)
            {
                ++letters[i].x;
            }

            if (letters[i].y > letters[i].final_y)
            {
                --letters[i].y;
            }
            else if (letters[i].y < letters[i].final_y)
            {
                ++letters[i].y;
            }
        }
    }

    BitmappedFont::DisableStates();
#ifndef ANDROID_JNI
    vdkSwapEGL(&egl);
#endif
}

//----------------------------------------------------------------------------

void CleanGLES()
{
    if (font != NULL) delete font;
    glDeleteTextures(1, &texture);
}

//----------------------------------------------------------------------------

void SetOrtho2D()
{
#ifndef ANDROID_JNI
    int width, height;
    vdkGetWindowInfo(egl.window, NULL, NULL, &width, &height, NULL, NULL);
#endif
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrthox(FixedFromInt(0), FixedFromInt(width),
             FixedFromInt(0), FixedFromInt(height),
             FixedFromInt(-1) , FixedFromInt(1));
    glMatrixMode(GL_MODELVIEW);
}
