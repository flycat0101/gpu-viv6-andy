/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
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


#ifndef glutils_check_INCLUDED
#define glutils_check_INCLUDED

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
    If there is an error, the checks log a message and returns
    true. Otherwise it returns false.
*/

const char * GetGLMessage(GLenum);

int CheckGL(const char * Message);

const char * GetEGLMessage(EGLint);

int CheckEGL(const char * Message);

const char * GetFramebufferMessage(GLenum);

int CheckFramebufferStatus(const char * Message);

/*
    Checked versions of glGetAttribLocation and
    glGetUniformLocation. Returns the attrib/uniform value and prints
    out a message if value is less than zero.
*/

int CheckedGetAttribLoc(GLuint Prog,
                        const char * Attrib,
                        const char * Message);

int CheckedGetUniformLoc(GLuint prog,
                         const char * Uniform,
                         const char * Message);

#ifdef __cplusplus
}
#endif

#endif

