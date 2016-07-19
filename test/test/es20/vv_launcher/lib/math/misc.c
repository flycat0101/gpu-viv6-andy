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


#include "misc.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>

int isPowerOfTwo(unsigned int i)
{
    return (i & (i - 1)) == 0;
}


float invSqrt(float t)
{
    return 1.0f/(float)sqrt(t);
}


float wrap(float a, float min, float max)
{
    float d = max-min;
    float s = a - min;
    float q = s/d;
    float m = q - (float)floor(q);

    assert(max > min);

    return m * d + min;
}


float degToRad(float d)
{
    return d * 3.14159265358979323846f / 180.0f;
}


float ease(float t)
{
    float t_2 = t * t;
    float t_3 = t_2 * t;
    return 3.0f * t_2 - 2.0f * t_3;
}

