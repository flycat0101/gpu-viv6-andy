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


#include "Vec.h"

float Dot3f(Vec3f* V1, Vec3f* V2)
{
    return (V1->v[0] * V2->v[0] + V1->v[1] * V2->v[1] + V1->v[2] * V2->v[2]);
}


void VecNormalize3f(Vec3f* V)
{
    float inv_sqrt = invSqrt(Dot3f(V, V));
    V->v[0] *= inv_sqrt;
    V->v[1] *= inv_sqrt;
    V->v[2] *= inv_sqrt;
}


void VecAdd3f(Vec3f* Res, Vec3f* V1, Vec3f* V2)
{
    Res->v[0] = V1->v[0] + V2->v[0];
    Res->v[1] = V1->v[1] + V2->v[1];
    Res->v[2] = V1->v[2] + V2->v[2];
}


void VecSub3f(Vec3f* Res, Vec3f* V1, Vec3f* V2)
{
    Res->v[0] = V1->v[0] - V2->v[0];
    Res->v[1] = V1->v[1] - V2->v[1];
    Res->v[2] = V1->v[2] - V2->v[2];
}


void VecScale3f(Vec3f* Res, Vec3f* V1, float Factor)
{
    Res->v[0] = V1->v[0] * Factor;
    Res->v[1] = V1->v[1] * Factor;
    Res->v[2] = V1->v[2] * Factor;
}


void VecCross3f(Vec3f* Res, Vec3f* V1, Vec3f* V2)
{
    Res->v[0] = V1->v[1]*V2->v[2] - V1->v[2]*V2->v[1];
    Res->v[1] = V1->v[2]*V2->v[0] - V1->v[0]*V2->v[2];
    Res->v[2] = V1->v[0]*V2->v[1] - V1->v[1]*V2->v[0];
}


void VecCopy3f(Vec3f* V1, Vec3f* V2)
{
    V1->v[0] = V2->v[0];
    V1->v[1] = V2->v[1];
    V1->v[2] = V2->v[2];
}


void VecSet3f(Vec3f* V, float f1, float f2, float f3)
{
    V->v[0] =  f1;
#ifndef ANDROID
    V->v[2] =  f2;
    V->v[3] =  f3;
#endif
    V->v[1] =  f2;
    V->v[2] =  f3;
}

