/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
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


#ifndef __cover_flow_h_
#define __cover_flow_h_

#define COMMON

#include <gc_vdk.h>
#include <GLES/gl.h>

#define NUM_IMAGES          9

/*******************************************************************************
** Initialization code.
*/

int
InitGLES(
    vdkEGL * Egl,
    int Width,
    int Height,
    int Samples
    );

/*******************************************************************************
** Render code.
*/

typedef struct _CoverMatrix
{
    GLfloat x, y, z;
    GLfloat sizeX, sizeY;
    GLfloat rotation;
    GLfloat color;
}
CoverMatrix;

typedef enum _MatrixType
{
    CURRENT_MATRIX,
    TARGET_MATRIX,
    STEP_MATRIX,
    MATRIX_TYPE_COUNT,
}
MatrixType;

typedef struct _CoverFlow
{
    int         animation;
    int         redraw;
    float       angle;
    int         currentImage;
    int         automatic;
    CoverMatrix matrix[NUM_IMAGES][MATRIX_TYPE_COUNT];
    GLint       textures[NUM_IMAGES];
}
CoverFlow;

void
Render(
    vdkEGL * Egl,
    CoverFlow * App
    );

void
AdjustMatrix(
    int Steps,
    CoverFlow * App
    );

void
DrawImage(
    int Image,
    GLint Texture,
    CoverMatrix * Matrix
    );

#endif /* __cover_flow_h_ */
