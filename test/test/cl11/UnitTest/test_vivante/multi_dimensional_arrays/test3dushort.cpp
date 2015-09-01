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


#include "test3dushort.h"
#include "kernelgenerator.h"
#include "randnumgenerator.h"
#include <iostream>

Test3DUShort::Test3DUShort(const size_t sizeX, const size_t sizeY, const size_t sizeZ):
MultiDimensionalArraysTest3D("ushort", sizeX, sizeY, sizeZ) {
	inputAHost = new cl_ushort[size];
	inputBHost = new cl_ushort[size];
	resultHost = new cl_ushort[size];
	inputA = inputAHost;
	inputB = inputBHost;
	result = resultHost;
}

Test3DUShort::~Test3DUShort() {
	delete[] inputAHost;
	delete[] inputBHost;
	delete[] result;
}

const int Test3DUShort::getMemSize() const {
	return sizeof(cl_ushort)*size;
}

void Test3DUShort::generateInput(const unsigned int &kernelIndex) {
	for (unsigned int k=0; k<size; k++) {
		inputAHost[k] = randUShort();
		if (getKernelInputArgNum(kernelIndex) == 2)
			inputBHost[k] = randUShort();
	}
}

void* Test3DUShort::getInputAElement(const unsigned int &index) const {
	return &inputAHost[index];
}

void* Test3DUShort::getInputBElement(const unsigned int &index) const {
	return &inputBHost[index];
}

void* Test3DUShort::getResultElement(const unsigned int &index) const {
	return &resultHost[index];
}