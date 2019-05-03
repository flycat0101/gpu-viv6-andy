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


#include "Mat.h"

void MatTranslatef(Matf* M, float x, float y, float z)
{
    M->m[0][0] = 1.0f;
    M->m[0][1] = 0.0f;
    M->m[0][2] = 0.0f;
    M->m[0][3] = 0.0f;

    M->m[1][0] = 0.0f;
    M->m[1][1] = 1.0f;
    M->m[1][2] = 0.0f;
    M->m[1][3] = 0.0f;

    M->m[2][0] = 0.0f;
    M->m[2][1] = 0.0f;
    M->m[2][2] = 1.0f;
    M->m[2][3] = 0.0f;

    M->m[3][0] = x;
    M->m[3][1] = y;
    M->m[3][2] = z;
    M->m[3][3] = 1.0f;
}


void MatFrustumf(Matf* M, float l, float r, float b, float t, float n, float f)
{
    float m00 = n*2.0f/(r-l);
    float m11 = n*2.0f/(t-b);
    float m22 = -(f+n)/(f-n);

    float m20 = (r+l)/(r-l);
    float m21 = (t+b)/(t-b);

    float m32 = -(2.0f*f*n)/(f-n);
    float m23 = -1.0f;

    M->m[0][0] = m00;
    M->m[0][1] = 0.0f;
    M->m[0][2] = 0.0f;
    M->m[0][3] = 0.0f;

    M->m[1][0] = 0.0f;
    M->m[1][1] = m11;
    M->m[1][2] = 0.0f;
    M->m[1][3] = 0.0f;

    M->m[2][0] = m20;
    M->m[2][1] = m21;
    M->m[2][2] = m22;
    M->m[2][3] = m23;

    M->m[3][0] = 0.0f;
    M->m[3][1] = 0.0f;
    M->m[3][2] = m32;
    M->m[3][3] = 0.0f;
}


void MatScalef(Matf* M, float x, float y, float z)
{
    M->m[0][0] = x;
    M->m[0][1] = 0.0f;
    M->m[0][2] = 0.0f;
    M->m[0][3] = 0.0f;

    M->m[1][0] = 0.0f;
    M->m[1][1] = y;
    M->m[1][2] = 0.0f;
    M->m[1][3] = 0.0f;

    M->m[2][0] = 0.0f;
    M->m[2][1] = 0.0f;
    M->m[2][2] = z;
    M->m[2][3] = 0.0f;

    M->m[3][0] = 0.0f;
    M->m[3][1] = 0.0f;
    M->m[3][2] = 0.0f;
    M->m[3][3] = 1.0f;
}


void MatMulf(Matf* M1, Matf* M2)
{
      M1->m[0][0] = M1->m[0][0]*M2->m[0][0] + M1->m[1][0]*M2->m[0][1] + M1->m[2][0]*M2->m[0][2] + M1->m[3][0]*M2->m[0][3];
      M1->m[0][1] = M1->m[0][1]*M2->m[0][0] + M1->m[1][1]*M2->m[0][1] + M1->m[2][1]*M2->m[0][2] + M1->m[3][1]*M2->m[0][3];
      M1->m[0][2] = M1->m[0][2]*M2->m[0][0] + M1->m[1][2]*M2->m[0][1] + M1->m[2][2]*M2->m[0][2] + M1->m[3][2]*M2->m[0][3];
      M1->m[0][3] = M1->m[0][3]*M2->m[0][0] + M1->m[1][3]*M2->m[0][1] + M1->m[2][3]*M2->m[0][2] + M1->m[3][3]*M2->m[0][3];

      M1->m[1][0] = M1->m[0][0]*M2->m[1][0] + M1->m[1][0]*M2->m[1][1] + M1->m[2][0]*M2->m[1][2] + M1->m[3][0]*M2->m[1][3];
      M1->m[1][1] = M1->m[0][1]*M2->m[1][0] + M1->m[1][1]*M2->m[1][1] + M1->m[2][1]*M2->m[1][2] + M1->m[3][1]*M2->m[1][3];
      M1->m[1][2] = M1->m[0][2]*M2->m[1][0] + M1->m[1][2]*M2->m[1][1] + M1->m[2][2]*M2->m[1][2] + M1->m[3][2]*M2->m[1][3];
      M1->m[1][3] = M1->m[0][3]*M2->m[1][0] + M1->m[1][3]*M2->m[1][1] + M1->m[2][3]*M2->m[1][2] + M1->m[3][3]*M2->m[1][3];

      M1->m[2][0] = M1->m[0][0]*M2->m[2][0] + M1->m[1][0]*M2->m[2][1] + M1->m[2][0]*M2->m[2][2] + M1->m[3][0]*M2->m[2][3];
      M1->m[2][1] = M1->m[0][1]*M2->m[2][0] + M1->m[1][1]*M2->m[2][1] + M1->m[2][1]*M2->m[2][2] + M1->m[3][1]*M2->m[2][3];
      M1->m[2][2] = M1->m[0][2]*M2->m[2][0] + M1->m[1][2]*M2->m[2][1] + M1->m[2][2]*M2->m[2][2] + M1->m[3][2]*M2->m[2][3];
      M1->m[2][3] = M1->m[0][3]*M2->m[2][0] + M1->m[1][3]*M2->m[2][1] + M1->m[2][3]*M2->m[2][2] + M1->m[3][3]*M2->m[2][3];

      M1->m[3][0] = M1->m[0][0]*M2->m[3][0] + M1->m[1][0]*M2->m[3][1] + M1->m[2][0]*M2->m[3][2] + M1->m[3][0]*M2->m[3][3];
      M1->m[3][1] = M1->m[0][1]*M2->m[3][0] + M1->m[1][1]*M2->m[3][1] + M1->m[2][1]*M2->m[3][2] + M1->m[3][1]*M2->m[3][3];
      M1->m[3][2] = M1->m[0][2]*M2->m[3][0] + M1->m[1][2]*M2->m[3][1] + M1->m[2][2]*M2->m[3][2] + M1->m[3][2]*M2->m[3][3];
      M1->m[3][3] = M1->m[0][3]*M2->m[3][0] + M1->m[1][3]*M2->m[3][1] + M1->m[2][3]*M2->m[3][2] + M1->m[3][3]*M2->m[3][3];
}


void RotXDegf(Matf* M, float degree)
{
    float s = (float)sin(degToRad(degree));
    float c = (float)cos(degToRad(degree));

    M->m[0][0] = 1.0f;
    M->m[0][1] = 0.0f;
    M->m[0][2] = 0.0f;
    M->m[0][3] = 0.0f;

    M->m[1][0] = 0.0f;
    M->m[1][1] = c;
    M->m[1][2] = s;
    M->m[1][3] = 0.0f;

    M->m[2][0] = 0.0f;
    M->m[2][1] = -s;
    M->m[2][2] = c;
    M->m[2][3] = 0.0f;

    M->m[3][0] = 0.0f;
    M->m[3][1] = 0.0f;
    M->m[3][2] = 0.0f;
    M->m[3][3] = 1.0f;
}


void RotYDegf(Matf* M, float degree)
{
    float s = (float)sin(degToRad(degree));
    float c = (float)cos(degToRad(degree));

    M->m[0][0] = c;
    M->m[0][1] = 0.0f;
    M->m[0][2] = -s;
    M->m[0][3] = 0.0f;

    M->m[1][0] = 0.0f;
    M->m[1][1] = 1.0f;
    M->m[1][2] = 0.0f;
    M->m[1][3] = 0.0f;

    M->m[2][0] = s;
    M->m[2][1] = 0.0f;
    M->m[2][2] = c;
    M->m[2][3] = 0.0f;

    M->m[3][0] = 0.0f;
    M->m[3][1] = 0.0f;
    M->m[3][2] = 0.0f;
    M->m[3][3] = 1.0f;
}


void RotZDegf(Matf* M, float degree)
{
    float s = (float)sin(degToRad(degree));
    float c = (float)cos(degToRad(degree));

    M->m[0][0] = c;
    M->m[0][1] = s;
    M->m[0][2] = 0.0f;
    M->m[0][3] = 0.0f;

    M->m[1][0] = -s;
    M->m[1][1] = c;
    M->m[1][2] = 0.0f;
    M->m[1][3] = 0.0f;

    M->m[2][0] = 0.0f;
    M->m[2][1] = 0.0f;
    M->m[2][2] = 1.0f;
    M->m[2][3] = 0.0f;

    M->m[3][0] = 0.0f;
    M->m[3][1] = 0.0f;
    M->m[3][2] = 0.0f;
    M->m[3][3] = 1.0f;
}

