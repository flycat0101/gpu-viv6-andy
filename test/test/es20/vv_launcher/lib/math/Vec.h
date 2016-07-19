/****************************************************************************
*
*    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
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


#ifndef math_Vec_INCLUDED
#define math_Vec_INCLUDED

#include <assert.h>
#include <math.h>

#include "misc.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _Vec2f
{
    float v[2];
} Vec2f;

typedef struct _Vec3f
{
    float v[3];
} Vec3f;

float Dot3f(Vec3f* V1, Vec3f* V2);

void VecNormalize3f(Vec3f* V);

void VecAdd3f(Vec3f* Res, Vec3f* V1, Vec3f* V2);

void VecSub3f(Vec3f* Res, Vec3f* V1, Vec3f* V2);

void VecScale3f(Vec3f* Res, Vec3f* V1, float Factor);

void VecCross3f(Vec3f* Res, Vec3f* V1, Vec3f* V2);

void VecCopy3f(Vec3f* V1, Vec3f* V2);

void VecSet3f(Vec3f* V, float f1, float f2, float f3);

#ifdef __cplusplus
}
#endif

#endif

