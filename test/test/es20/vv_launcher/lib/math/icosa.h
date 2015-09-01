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


#ifndef math_icosa_INCLUDED
#define math_icosa_INCLUDED

/*
 Each edge on an icosahedron has an edge on the other side that is
 parallel to it. The pair of edges forms a rectangle. You can
 include all the vertices of an icosahedron with three such
 rectangles perpendicular to each other and sharing their center
 point. (Four corners times three rectangles equals twelve
 vertices.) Six of the edges of the icosahedron correspond to the
 smaller sides of the rectangles, and the rest of the edges of the
 icosahedron correspond to connections between neighboring corners
 of different pairs of rectangles.

 The surprising thing is that the ratio of the lengths of the edges
 of the rectangle is the golden ratio. You can prove this by
 setting the length of the smaller edge of the rectangle equal to
 the length of an edge formed by a connection between different
 rectangles. What you get is a quadratic equation that when solved
 produces the golden ratio (1 + sqrt(5))/2. (Wow!)

 Setting the points of the icosahedron onto the surface of a unit
 sphere is the same thing as setting the distance of the center of
 a rectangle to any of its corners equal to one. When you solve for
 the lengths of the edges of the rectangle, you get: sqrt(2/(5 +
 sqrt(5))) and (2/(5 - sqrt(5))). This is where the raw numbers in
 the OpenGL book come from.

 -thant
*/
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _icosa
{
	float vertices[12][3];
	unsigned short indices[20][3];
}
icosa_t;

extern icosa_t icosa;

#ifdef __cplusplus
}
#endif

#endif

