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


#ifndef math_Mat_INCLUDED
#define math_Mat_INCLUDED

#include <math/misc.h>
#include <math/Vec.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    float m[4][4];
}
Matf;

void MatTranslatef(Matf* M, float x, float y, float z);

void MatScalef(Matf* M, float x, float y, float z);

void MatFrustumf(Matf* M, float l, float r, float b, float t, float n, float f);

void MatMulf(Matf* M1, Matf* M2);

void RotXDegf(Matf* M, float degree);

void RotYDegf(Matf* M, float degree);

void RotZDegf(Matf* M, float degree);

#ifdef __cplusplus
}
#endif

#endif

