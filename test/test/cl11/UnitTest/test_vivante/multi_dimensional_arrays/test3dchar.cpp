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


#include "test3dchar.h"
#include "kernelgenerator.h"
#include "randnumgenerator.h"
#include <iostream>

Test3DChar::Test3DChar(const size_t sizeX, const size_t sizeY, const size_t sizeZ):
MultiDimensionalArraysTest3D("char", sizeX, sizeY, sizeZ) {
    inputAHost = new cl_char[size];
    inputBHost = new cl_char[size];
    resultHost = new cl_char[size];
    inputA = inputAHost;
    inputB = inputBHost;
    result = resultHost;
}

Test3DChar::~Test3DChar() {
    delete[] inputAHost;
    delete[] inputBHost;
    delete[] resultHost;
}

const int Test3DChar::getMemSize() const {
    return sizeof(cl_char)*size;
}

void Test3DChar::generateInput(const unsigned int &kernelIndex) {
    for (unsigned int k=0; k<size; k++) {
        inputAHost[k] = randChar();
        if (getKernelInputArgNum(kernelIndex) == 2)
            inputBHost[k] = randChar();
    }
}

void* Test3DChar::getInputAElement(const unsigned int &index) const {
    return &inputAHost[index];
}

void* Test3DChar::getInputBElement(const unsigned int &index) const {
    return &inputBHost[index];
}

void* Test3DChar::getResultElement(const unsigned int &index) const {
    return &resultHost[index];
}