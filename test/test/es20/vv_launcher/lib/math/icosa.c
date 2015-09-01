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


#include "icosa.h"

#include <math.h>

/*
A = sqrt(2.0/(5.0 + sqrt(5.0)));
B = sqrt(2.0/(5.0 - sqrt(5.0)));
*/
#define A 0.525731112f
#define B 0.850650808f

icosa_t icosa = {

/* vertices */
{
	{-A, 0.0f, B}, {A, 0.0f, B}, {-A, 0.0f, -B}, {A, 0.0f, -B},
	{0.0f, B, A}, {0.0f, B, -A}, {0.0f, -B, A}, {0.0f, -B, -A},
	{B, A, 0.0f}, {-B, A, 0.0f}, {B, -A, 0.0f}, {-B, -A, 0.0f}
},

/* indices */
{
	{1, 4, 0}, {4, 9, 0}, {4, 5, 9}, {8, 5, 4}, {1, 8, 4},
	{1, 10, 8}, {10, 3, 8}, {8, 3, 5}, {3, 2, 5}, {3, 7, 2},
	{3, 10, 7}, {10, 6, 7}, {6, 11, 7}, {6, 0, 11}, {6, 1, 0},
	{10, 1, 6}, {11, 0, 9}, {2, 11, 9}, {5, 2, 9}, {11, 2, 7}
}
};

