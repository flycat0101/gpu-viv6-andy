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


#ifndef __fixedpoint_h_
#define __fixedpoint_h_

#include "main.h"

// Convert integer to fixed point.
inline GLfixed FixedFromInt(int value)
{
	return static_cast<GLfixed>(value << 16);
}
#define ZERO	FixedFromInt(0)
#define ONE		FixedFromInt(1)

// Convert float to fixed point.
inline GLfixed FixedFromFloat(float value)
{
	return static_cast<GLfixed>(value * static_cast<float>(ONE));
}

// Multiply two fixed points.
inline GLfixed MultiplyFixed(GLfixed value1, GLfixed value2)
{
	return (value1 * value2) >> 16;
}

#endif // __fixedpoint_h_
