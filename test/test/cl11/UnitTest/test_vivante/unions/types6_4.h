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


#ifndef _TYPES6_H_
#define _TYPES6_H_

#ifdef __OPENCL_VERSION__
#define ALIGNED_STRUCT(structureType, alignBytes) structureType __attribute__ ((aligned(alignBytes)))
#else // __OPENCL_VERSION__
#ifdef _WIN32
#define ALIGNED_STRUCT(structureType, alignBytes) __declspec(align(alignBytes)) structureType
#else
#define ALIGNED_STRUCT(structureType, alignBytes) structureType __attribute__ ((aligned(alignBytes)))
#endif
#endif // __OPENCL_VERSION__

ALIGNED_STRUCT(union, 32) InputA {
    cl_ushort4 a;
    cl_float4 h;
};

ALIGNED_STRUCT(union, 32) InputB {
    cl_ushort4 k;
    cl_float4 b;
};

ALIGNED_STRUCT(union, 32) Result {
    cl_ushort4 z;
    cl_float4 r;
};

#endif // _TYPES3_H_
