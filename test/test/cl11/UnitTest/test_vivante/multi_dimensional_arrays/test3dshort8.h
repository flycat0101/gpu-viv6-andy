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


#ifndef _test3dshort8_h
#define _test3dshort8_h

#include "multidimensionalarraystest3D.h"

class Test3DShort8: public MultiDimensionalArraysTest3D {
public:
	Test3DShort8(const size_t sizeX, const size_t sizeY, const size_t sizeZ);
	~Test3DShort8();

protected:
	cl_short8* inputAHost;
	cl_short8* inputBHost;
	cl_short8* resultHost;

	const int getMemSize() const;
	void generateInput(const unsigned int &kernelIndex);
	void* getInputAElement(const unsigned int &index) const;
	void* getInputBElement(const unsigned int &index) const;
	void* getResultElement(const unsigned int &index) const;
};

#endif /*_test3dshort8_h*/