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


#ifndef _testcasesinit_h
#define _testcasesinit_h

#include <CL/cl.h>

#define MEM_TYPE_COUNT            14
#define VECTOR_SIZE_COUNT        2
#define ILLEGAL_SIZE_COUNT        10
#define TYPE_COUNT                7

extern int currentTest;
extern size_t memSizes[MEM_TYPE_COUNT];
extern char typeNames[TYPE_COUNT][10];
extern unsigned int vectorSizes[VECTOR_SIZE_COUNT];
extern unsigned int illegalSizes[ILLEGAL_SIZE_COUNT];
extern unsigned int vectorCaseSizes[VECTOR_SIZE_COUNT];
extern unsigned int countOfTestsPerType;
extern unsigned int *vecTypeIndex;
extern unsigned int *vecSizeIndex;

#endif /*_testcasesinit_h*/