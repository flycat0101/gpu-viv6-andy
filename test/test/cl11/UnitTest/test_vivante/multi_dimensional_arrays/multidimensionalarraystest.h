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


#ifndef _multidimensionalarraystest_h
#define _multidimensionalarraystest_h

#include <CL/cl.h>

class MultiDimensionalArraysTest {
public:
    MultiDimensionalArraysTest(const char *typeName, const char *dimension,
        const size_t size);
    ~MultiDimensionalArraysTest();
    void cleanup();
    char* getErrorMessage() const;
    char* getBuildLog() const;
    int runHarnessMode(cl_device_id device, cl_context context, cl_command_queue queue);
    const char* getTitle() const;

protected:
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_mem inputADevice;
    cl_mem inputBDevice;
    cl_mem resultDevice;
    char *errorMessage;
    char *buildLog;
    char *controlFailMessage;
    const size_t size;
    const char *typeName;
    const char *dimension;
    char title[30];
    cl_kernel *kernels;
    void *inputA;
    void *inputB;
    void *result;

    virtual const int getMemSize() const = 0;
    virtual void generateInput(const unsigned int &kernelIndex) = 0;
    void getFullKernelName(const unsigned int &kernelIndex, char *kernelName);
    virtual cl_int enqueueKernel(cl_kernel kernel) = 0;
    virtual void* getInputAElement(const unsigned int &index) const = 0;
    virtual void* getInputBElement(const unsigned int &index) const = 0;
    virtual void* getResultElement(const unsigned int &index) const = 0;
    virtual void generateSource(char *source, unsigned int *sourceSize) = 0;

    cl_bool controlResult(const unsigned int &kernelIndex);
    cl_bool checkInt(const unsigned int &kernelIndex, const unsigned int &vectorSize);
    cl_bool checkUInt(const unsigned int &kernelIndex, const unsigned int &vectorSize);
    cl_bool checkShort(const unsigned int &kernelIndex, const unsigned int &vectorSize);
    cl_bool checkUShort(const unsigned int &kernelIndex, const unsigned int &vectorSize);
    cl_bool checkChar(const unsigned int &kernelIndex, const unsigned int &vectorSize);
    cl_bool checkUChar(const unsigned int &kernelIndex, const unsigned int &vectorSize);
    cl_bool checkFloat(const unsigned int &kernelIndex, const unsigned int &vectorSize);
};

#endif /*_multidimensionalarraystest_h*/