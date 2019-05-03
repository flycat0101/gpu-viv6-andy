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


#include "multidimensionalarraystest3D.h"
#include "kernelgenerator.h"

MultiDimensionalArraysTest3D::MultiDimensionalArraysTest3D(const char *typeName,
                                                           const size_t sizeX,
                                                           const size_t sizeY,
                                                           const size_t sizeZ):
MultiDimensionalArraysTest(typeName, "3D", sizeX*sizeY*sizeZ),
sizeX(sizeX),
sizeY(sizeY),
sizeZ(sizeZ) {
}

cl_int MultiDimensionalArraysTest3D::enqueueKernel(cl_kernel kernel) {
    const size_t globalWS[3] = {sizeX, sizeY, sizeZ};
    const size_t localWS[3] = {sizeX, sizeY, sizeZ};
    return clEnqueueNDRangeKernel(queue, kernel, 3, NULL, globalWS, localWS, 0, NULL, NULL);
}

void MultiDimensionalArraysTest3D::generateSource(char *source, unsigned int *sourceSize) {
    const unsigned int dimensions[3] = {sizeX, sizeY, sizeZ};
    generateKernels(typeName, source, sourceSize, 3, dimensions);
}