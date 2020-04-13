/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
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


#include "multidimensionalarraystest.h"
#include "kernelloader.h"
#include "kernelgenerator.h"
#include "resultchecker.h"
#include "randnumgenerator.h"
#include <string.h>
#include <iostream>

MultiDimensionalArraysTest::MultiDimensionalArraysTest(const char *typeName, const char *dimension,
                                                       const size_t size):
typeName(typeName),
dimension(dimension),
size(size) {
    this->errorMessage = "";
    this->controlFailMessage = "";
    this->buildLog = NULL;
    this->inputADevice = NULL;
    this->inputBDevice = NULL;
    this->resultDevice = NULL;
    sprintf(title, "Dimension:%s, Type:%s", dimension, typeName);
    this->kernels = new cl_kernel[KERNEL_SIZE];
    for (unsigned int i=0; i<KERNEL_SIZE; i++)
        this->kernels[i] = 0;
}

MultiDimensionalArraysTest::~MultiDimensionalArraysTest() {
    if (buildLog != NULL)
        delete[] buildLog;
    delete[] kernels;
}

void MultiDimensionalArraysTest::getFullKernelName(const unsigned int &kernelIndex, char *kernelName) {
#ifdef _WIN32
    sprintf_s(kernelName, 20, "%s%s%s", typeName, getKernelName(kernelIndex), dimension);
#else
    snprintf(kernelName, 20, "%s%s%s", typeName, getKernelName(kernelIndex), dimension);
#endif
}

int MultiDimensionalArraysTest::runHarnessMode(cl_device_id device, cl_context context, cl_command_queue queue) {
    int error = 0;
    size_t logSize = 0;
    const int memSize = getMemSize();

    this->device = device;
    this->context = context;
    this->queue = queue;

    char source[MAX_SOURCE_SIZE];
    unsigned int srcSize1;
   size_t srcSize2;
    generateSource(source, &srcSize1);

    const char *src = source;
   srcSize2 = srcSize1;
    program = clCreateProgramWithSource(context, 1, &src, &srcSize2, &error);
    if (error != CL_SUCCESS) {
        errorMessage = "Program cannot be created";
        return error;
    }

    error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (error != CL_SUCCESS) {
        errorMessage = "Program cannot be built";
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
        buildLog = new char[logSize+1];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
        buildLog[logSize] = '\0';
        std::cout << buildLog << std::endl;
        return error;
    }

    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
    buildLog = new char[logSize+1];
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
    buildLog[logSize] = '\0';

    inputADevice = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, memSize, inputA, &error);
    if (error != CL_SUCCESS) {
        errorMessage = "Memory A cannot allocated";
        return error;
    }

    inputBDevice = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, memSize, inputB, &error);
    if (error != CL_SUCCESS) {
        errorMessage = "Memory B cannot allocated";
        return error;
    }

    resultDevice = clCreateBuffer(context, CL_MEM_WRITE_ONLY, memSize, NULL, &error);
    if (error != CL_SUCCESS) {
        errorMessage = "Result memory cannot allocated";
        return error;
    }

    for (unsigned int i=0; i<KERNEL_SIZE; i++) {
        char kernelName[20];
        getFullKernelName(i, kernelName);
        unsigned int argCount = 0;
        kernels[i] = clCreateKernel(program, kernelName, &error);
        if (error != CL_SUCCESS) {
            char errMsg[40];
            sprintf(errMsg, "Kernel %s cannot be created", kernelName);
            errorMessage = errMsg;
            return error;
        }
        error = clSetKernelArg(kernels[i], argCount++, sizeof(cl_mem), &inputADevice);
        if (getKernelInputArgNum(i) == 2)
            error |= clSetKernelArg(kernels[i], argCount++, sizeof(cl_mem), &inputBDevice);
        error |= clSetKernelArg(kernels[i], argCount++, sizeof(cl_mem), &resultDevice);
        if (error != CL_SUCCESS) {
            errorMessage = "Error while setting arguments";
            return error;
        }
    }

    for (unsigned int i=0; i<KERNEL_SIZE; i++) {
        char kernelName[20];
        getFullKernelName(i, kernelName);
        generateInput(i);
        error = clEnqueueWriteBuffer(queue, inputADevice, CL_TRUE, 0, memSize, inputA, 0, NULL, NULL);
        if (getKernelInputArgNum(i) == 2)
            error |= clEnqueueWriteBuffer(queue, inputBDevice, CL_TRUE, 0, memSize, inputB, 0, NULL, NULL);
        if (error != CL_SUCCESS) {
            errorMessage = "Error while writing buffer";
            return error;
        }
        error = enqueueKernel(kernels[i]);
        if (error != CL_SUCCESS) {
            char errMsg[40];
#ifdef _WIN32
            sprintf_s(errMsg, 40, "Error while running kernel %s", kernelName);
#else
            snprintf(errMsg, 40, "Error while running kernel %s", kernelName);
#endif
            errorMessage = errMsg;
            return error;
        }
        clEnqueueReadBuffer(queue, resultDevice, CL_TRUE, 0, memSize, result, 0, NULL, NULL);

        if (!controlResult(i))
            return 1;
    }

    /*cleanup();*/

    return error;
}

cl_bool MultiDimensionalArraysTest::controlResult(const unsigned int &kernelIndex) {
    unsigned int vectorSize = 1;
    unsigned int typeIndex = 0;
    while (typeName[typeIndex] != '\0') {
        if (typeName[typeIndex] == '2') {
            vectorSize = 2;
            break;
        } else if (typeName[typeIndex] == '4') {
            vectorSize = 4;
            break;
        } else if (typeName[typeIndex] == '8') {
            vectorSize = 8;
            break;
        } else if (typeName[typeIndex] == '1' && typeName[typeIndex+1] == '6') {
            vectorSize = 16;
            break;
        }
        typeIndex++;
    }

    if (strncmp(typeName, "int", 3) == 0)
        return checkInt(kernelIndex,vectorSize);
    else if (strncmp(typeName, "uint", 4) == 0)
        return checkUInt(kernelIndex,vectorSize);
    else if (strncmp(typeName, "short", 5) == 0)
        return checkShort(kernelIndex,vectorSize);
    else if (strncmp(typeName, "ushort", 6) == 0)
        return checkUShort(kernelIndex,vectorSize);
    else if (strncmp(typeName, "char", 4) == 0)
        return checkChar(kernelIndex,vectorSize);
    else if (strncmp(typeName, "uchar", 5) == 0)
        return checkUChar(kernelIndex,vectorSize);
    else if (strncmp(typeName, "float", 5) == 0)
        return checkFloat(kernelIndex,vectorSize);

    return CL_TRUE;
}

cl_bool MultiDimensionalArraysTest::checkInt(const unsigned int &kernelIndex, const unsigned int &vectorSize) {
    cl_int *inputAElement;
    cl_int *inputBElement;
    cl_int *resultElement;

    int (*checkFunc)(int,int,int) = 0;
    cl_bool success = CL_TRUE;

    if (kernelIndex == 0)
        checkFunc = &checkAddInt;
    else if (kernelIndex == 1)
        checkFunc = &checkMultInt;
    else if (kernelIndex == 2)
        checkFunc = &checkCopyInt;
    else if (kernelIndex == 3)
        checkFunc = &checkDivideInt;
    else if (kernelIndex == 4)
        checkFunc = &checkClzInt;
    else
        return CL_FALSE;

    for (unsigned int i=0; i<size; i++) {
        inputAElement = (cl_int*)getInputAElement(i);
        inputBElement = (cl_int*)getInputBElement(i);
        resultElement = (cl_int*)getResultElement(i);
        for (unsigned int j=0; j<vectorSize; j++) {
            if (checkFunc(inputAElement[j],inputBElement[j],resultElement[j]) != 0) {
                return CL_FALSE;
                /*success = CL_FALSE;*/
            }
        }
    }

    return success;
}

cl_bool MultiDimensionalArraysTest::checkUInt(const unsigned int &kernelIndex, const unsigned int &vectorSize) {
    cl_uint *inputAElement;
    cl_uint *inputBElement;
    cl_uint *resultElement;

    int (*checkFunc)(unsigned int,unsigned int,unsigned int) = 0;
    cl_bool success = CL_TRUE;

    if (kernelIndex == 0)
        checkFunc = &checkAddUInt;
    else if (kernelIndex == 1)
        checkFunc = &checkMultUInt;
    else if (kernelIndex == 2)
        checkFunc = &checkCopyUInt;
    else if (kernelIndex == 3)
        checkFunc = &checkDivideUInt;
    else if (kernelIndex == 4)
        checkFunc = &checkClzUInt;
    else
        return CL_FALSE;

    for (unsigned int i=0; i<size; i++) {
        inputAElement = (cl_uint*)getInputAElement(i);
        inputBElement = (cl_uint*)getInputBElement(i);
        resultElement = (cl_uint*)getResultElement(i);
        for (unsigned int j=0; j<vectorSize; j++) {
            if (checkFunc(inputAElement[j],inputBElement[j],resultElement[j]) != 0) {
                return CL_FALSE;
                /*success = CL_FALSE;*/
            }
        }
    }

    return success;
}

cl_bool MultiDimensionalArraysTest::checkShort(const unsigned int &kernelIndex, const unsigned int &vectorSize) {
    cl_short *inputAElement;
    cl_short *inputBElement;
    cl_short *resultElement;

    int (*checkFunc)(short,short,short) = 0;
    cl_bool success = CL_TRUE;

    if (kernelIndex == 0)
        checkFunc = &checkAddShort;
    else if (kernelIndex == 1)
        checkFunc = &checkMultShort;
    else if (kernelIndex == 2)
        checkFunc = &checkCopyShort;
    else if (kernelIndex == 3)
        checkFunc = &checkDivideShort;
    else if (kernelIndex == 4)
        checkFunc = &checkClzShort;
    else
        return CL_FALSE;

    for (unsigned int i=0; i<size; i++) {
        inputAElement = (cl_short*)getInputAElement(i);
        inputBElement = (cl_short*)getInputBElement(i);
        resultElement = (cl_short*)getResultElement(i);
        for (unsigned int j=0; j<vectorSize; j++) {
            if (checkFunc(inputAElement[j],inputBElement[j],resultElement[j]) != 0) {
                return CL_FALSE;
                /*success = CL_FALSE;*/
            }
        }
    }

    return success;
}

cl_bool MultiDimensionalArraysTest::checkUShort(const unsigned int &kernelIndex, const unsigned int &vectorSize) {
    cl_ushort *inputAElement;
    cl_ushort *inputBElement;
    cl_ushort *resultElement;

    int (*checkFunc)(unsigned short,unsigned short,unsigned short) = 0;
    cl_bool success = CL_TRUE;

    if (kernelIndex == 0)
        checkFunc = &checkAddUShort;
    else if (kernelIndex == 1)
        checkFunc = &checkMultUShort;
    else if (kernelIndex == 2)
        checkFunc = &checkCopyUShort;
    else if (kernelIndex == 3)
        checkFunc = &checkDivideUShort;
    else if (kernelIndex == 4)
        checkFunc = &checkClzUShort;
    else
        return CL_FALSE;

    for (unsigned int i=0; i<size; i++) {
        inputAElement = (cl_ushort*)getInputAElement(i);
        inputBElement = (cl_ushort*)getInputBElement(i);
        resultElement = (cl_ushort*)getResultElement(i);
        for (unsigned int j=0; j<vectorSize; j++) {
            if (checkFunc(inputAElement[j],inputBElement[j],resultElement[j]) != 0) {
                return CL_FALSE;
                /*success = CL_FALSE;*/
            }
        }
    }

    return success;
}

cl_bool MultiDimensionalArraysTest::checkChar(const unsigned int &kernelIndex, const unsigned int &vectorSize) {
    cl_char *inputAElement;
    cl_char *inputBElement;
    cl_char *resultElement;

    int (*checkFunc)(char,char,char) = 0;
    cl_bool success = CL_TRUE;

    if (kernelIndex == 0)
        checkFunc = &checkAddChar;
    else if (kernelIndex == 1)
        checkFunc = &checkMultChar;
    else if (kernelIndex == 2)
        checkFunc = &checkCopyChar;
    else if (kernelIndex == 3)
        checkFunc = &checkDivideChar;
    else if (kernelIndex == 4)
        checkFunc = &checkClzChar;
    else
        return CL_FALSE;

    for (unsigned int i=0; i<size; i++) {
        inputAElement = (cl_char*)getInputAElement(i);
        inputBElement = (cl_char*)getInputBElement(i);
        resultElement = (cl_char*)getResultElement(i);
        for (unsigned int j=0; j<vectorSize; j++) {
            if (checkFunc(inputAElement[j],inputBElement[j],resultElement[j]) != 0) {
                return CL_FALSE;
                /*success = CL_FALSE;*/
            }
        }
    }

    return success;
}

cl_bool MultiDimensionalArraysTest::checkUChar(const unsigned int &kernelIndex, const unsigned int &vectorSize) {
    cl_uchar *inputAElement;
    cl_uchar *inputBElement;
    cl_uchar *resultElement;

    int (*checkFunc)(unsigned char,unsigned char,unsigned char) = 0;
    cl_bool success = CL_TRUE;

    if (kernelIndex == 0)
        checkFunc = &checkAddUChar;
    else if (kernelIndex == 1)
        checkFunc = &checkMultUChar;
    else if (kernelIndex == 2)
        checkFunc = &checkCopyUChar;
    else if (kernelIndex == 3)
        checkFunc = &checkDivideUChar;
    else if (kernelIndex == 4)
        checkFunc = &checkClzUChar;
    else
        return CL_FALSE;

    for (unsigned int i=0; i<size; i++) {
        inputAElement = (cl_uchar*)getInputAElement(i);
        inputBElement = (cl_uchar*)getInputBElement(i);
        resultElement = (cl_uchar*)getResultElement(i);
        for (unsigned int j=0; j<vectorSize; j++) {
            if (checkFunc(inputAElement[j],inputBElement[j],resultElement[j]) != 0) {
                return CL_FALSE;
                /*success = CL_FALSE;*/
            }
        }
    }

    return success;
}

cl_bool MultiDimensionalArraysTest::checkFloat(const unsigned int &kernelIndex, const unsigned int &vectorSize) {
    cl_float *inputAElement;
    cl_float *inputBElement;
    cl_float *resultElement;

    int (*checkFunc)(float,float,float) = 0;
    cl_bool success = CL_TRUE;

    if (kernelIndex == 0)
        checkFunc = &checkAddFloat;
    else if (kernelIndex == 1)
        checkFunc = &checkMultFloat;
    else if (kernelIndex == 2)
        checkFunc = &checkCopyFloat;
    else if (kernelIndex == 3)
        checkFunc = &checkDivideFloat;
    else if (kernelIndex == 4)
        checkFunc = &checkClzFloat;
    else
        return CL_FALSE;

    for (unsigned int i=0; i<size; i++) {
        inputAElement = (cl_float*)getInputAElement(i);
        inputBElement = (cl_float*)getInputBElement(i);
        resultElement = (cl_float*)getResultElement(i);
        for (unsigned int j=0; j<vectorSize; j++) {
            if (checkFunc(inputAElement[j],inputBElement[j],resultElement[j]) != 0) {
                return CL_FALSE;
                /*success = CL_FALSE;*/
            }
        }
    }

    return success;
}

void MultiDimensionalArraysTest::cleanup() {
    clReleaseProgram(program);
    for (unsigned int i=0; i<KERNEL_SIZE; i++) {
        clReleaseKernel(kernels[i]);
    }
    if (inputADevice) clReleaseMemObject(inputADevice);
    if (inputBDevice) clReleaseMemObject(inputBDevice);
    if (resultDevice) clReleaseMemObject(resultDevice);
}

char* MultiDimensionalArraysTest::getErrorMessage() const {
    return errorMessage;
}

char* MultiDimensionalArraysTest::getBuildLog() const {
    return buildLog;
}

const char* MultiDimensionalArraysTest::getTitle() const {
    return title;
}
