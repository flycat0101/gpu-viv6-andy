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


#include "multidimensionalarraystest2D.h"
#include "kernelgenerator.h"

MultiDimensionalArraysTest2D::MultiDimensionalArraysTest2D(const char *typeName,
														   const size_t sizeX,
														   const size_t sizeY):
MultiDimensionalArraysTest(typeName, "2D", sizeX*sizeY),
sizeX(sizeX),
sizeY(sizeY) {
}

cl_int MultiDimensionalArraysTest2D::enqueueKernel(cl_kernel kernel) {
	const size_t globalWS[2] = {sizeX, sizeY};
	const size_t localWS[2] = {sizeX, sizeY};
	return clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalWS, localWS, 0, NULL, NULL);
}

void MultiDimensionalArraysTest2D::generateSource(char *source, unsigned int *sourceSize) {
	const unsigned int dimensions[2] = {sizeX, sizeY};
	generateKernels(typeName, source, sourceSize, 2, dimensions);
}