/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
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


#include "test3duint4.h"
#include "kernelgenerator.h"
#include "randnumgenerator.h"
#include <iostream>

#define VECTOR_SIZE 4

Test3DUInt4::Test3DUInt4(const size_t sizeX, const size_t sizeY, const size_t sizeZ):
MultiDimensionalArraysTest3D("uint4", sizeX, sizeY, sizeZ) {
    inputAHost = new cl_uint4[size];
    inputBHost = new cl_uint4[size];
    resultHost = new cl_uint4[size];
    inputA = inputAHost;
    inputB = inputBHost;
    result = resultHost;
}

Test3DUInt4::~Test3DUInt4() {
    delete[] inputAHost;
    delete[] inputBHost;
    delete[] resultHost;
}

const int Test3DUInt4::getMemSize() const {
    return sizeof(cl_uint4)*size;
}

void Test3DUInt4::generateInput(const unsigned int &kernelIndex) {
    for (unsigned int i=0; i<VECTOR_SIZE; i++) {
        for (unsigned int k=0; k<size; k++) {
            inputAHost[k].s[i] = randUInt();
            if (getKernelInputArgNum(kernelIndex) == 2)
                inputBHost[k].s[i] = randUInt();
        }
    }
}

void* Test3DUInt4::getInputAElement(const unsigned int &index) const {
    return inputAHost[index].s;
}

void* Test3DUInt4::getInputBElement(const unsigned int &index) const {
    return &inputBHost[index].s;
}

void* Test3DUInt4::getResultElement(const unsigned int &index) const {
    return &resultHost[index].s;
}