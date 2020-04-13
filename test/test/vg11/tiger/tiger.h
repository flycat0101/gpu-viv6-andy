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


#if old_tiger
#ifndef _TIGER_H
#define _TIGER_H

/*EGL is the "machine" that glues OpenGL ES with the underlying
windowing system. We need it to create a suitable context and a drawable window*/
#include <EGL/egl.h>
#include "vgframe.h"

/*Because we are building software device dependent (the PDA), we have care about
its limitations. PDA's doesn't have a FPU unit, so all floating point operations
are emulated in the CPU. To have real data type, PDA's uses reals with a fixed point
format. For a fixed point number we only need an integer, with the same size (in bytes)
that a float, that is, a normal int number. The first 16 bits of the int will be the
"integer" part, and the last 16 bits will be the "real" part. This will cause a lack
of precision, but it is better than emulate all FPU in the CPU. To convert an integer
number to a fixed point number we need to displace its bits to the left, as the FixedFromInt
function does. In this chapter we only will need the conversion int->fixed point.
Other conversions will be showed when needed, in later chapters. A complete description of
the fixed point maths is beyond the purpose of this set of tutorials, but the topic will
be widely covered through the chapters.
OpenGL ES offers us a set of functions that works with fixed point (Glfixed). These
functions are available through the OpenGL ES OES_fixed_point extension.
A little word about the OpenGL ES extensions: They are divided into two categories:
those that are fully integrated into the profile definition (core additions); and those
that remain extensions (profile extensions). Core additions do not use extension suffixes
and does not requires initialization, whereas profile extensions retain their extension suffixes.
OES_fixed_point is a core addition. The other extensions are listed and explained in the
OpenGL ES 1.1 specification.*/

#define PRECISION 16
#define ONE    (1 << PRECISION)
#define ZERO 0
//inline GLfixed FixedFromInt(int value) {return value << PRECISION;};

#endif
#else
#ifndef __TIGER_H
#define __TIGER_H

/*------------------------------------------------------------------------
 *
 * OpenVG 1.0.1 Reference Implementation sample code
 * -------------------------------------------------
 *
 * Copyright (c) 2007 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and /or associated documentation files
 * (the "Materials "), to deal in the Materials without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Materials,
 * and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE MATERIALS OR
 * THE USE OR OTHER DEALINGS IN THE MATERIALS.
 *
 *//**
 * \file
 * \brief    Header for including the Tiger image data.
 * \note
 *//*-------------------------------------------------------------------*/
#include "vgframe.h"

extern const int tigerCommandCount;
extern const char tigerCommands[];
extern const float tigerMinX;
extern const float tigerMaxX;
extern const float tigerMinY;
extern const float tigerMaxY;
extern const int tigerPointCount;
extern const float tigerPoints[];

#endif /* __TIGER_H */

#endif

