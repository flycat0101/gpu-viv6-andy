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


#include "kernelgenerator.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"
#include "testcasesinit.h"
#include "vectorgenerator.h"
#include "testcases.h"


int currentTest = 0;

typedef int (*basefn)(cl_device_id, cl_context, cl_command_queue, int);

size_t memSizes[MEM_TYPE_COUNT] = {
	sizeof(cl_int8),
	sizeof(cl_int16),
	sizeof(cl_uint8),
	sizeof(cl_uint16),
	sizeof(cl_short8),
	sizeof(cl_short16),
	sizeof(cl_ushort8),
	sizeof(cl_ushort16),
	sizeof(cl_char8),
	sizeof(cl_char16),
	sizeof(cl_uchar8),
	sizeof(cl_uchar16),
	sizeof(cl_float8),
	sizeof(cl_float16)
};

char typeNames[TYPE_COUNT][10] = {
	"int",
	"uint",
	"short",
	"ushort",
	"char",
	"uchar",
	"float"
};

unsigned int vectorSizes[VECTOR_SIZE_COUNT] = {
	8,
	16
};

unsigned int illegalSizes[ILLEGAL_SIZE_COUNT] = {
	5,
	6,
	7,
	9,
	10,
	11,
	12,
	13,
	14,
	15
};

unsigned int vectorCaseSizes[VECTOR_SIZE_COUNT];

typedef struct TestCase {
	size_t memSize;
	char typeName[10];
	unsigned int vectorSize;
	unsigned int illegalVectorSize;
} TestCase;

TestCase *testCases;

unsigned int *vecTypeIndex;
unsigned int *vecSizeIndex;

cl_platform_id platform = 0;
cl_context context = 0;
cl_device_id device = 0;
cl_command_queue queue = 0;

char *errorMessage = "";

int initializeCL() {
	cl_int error;
	error = clGetPlatformIDs(1, &platform, NULL);
	if (error != CL_SUCCESS) {
		errorMessage = "Error getting platform id!";
		return 0;
	}

	error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (error != CL_SUCCESS) {
		errorMessage = "Error getting device ids";
		return 0;
	}

	context = clCreateContext(0, 1, &device, NULL, NULL, &error);
	if (error != CL_SUCCESS) {
		errorMessage = "Error creating context";
		return 0;
	}

	queue = clCreateCommandQueue(context, device, 0, &error);
	if (error != CL_SUCCESS) {
		errorMessage = "Error creating command queue";
		clReleaseContext(context);
		return 0;
	}

	return 1;
}

int releaseCL() {
	cl_int error;
	int returnVal = 1;

	error = clReleaseCommandQueue(queue);
	if (error != CL_SUCCESS) {
		errorMessage = "Error releasing command queue";
		returnVal = 0;
	}

	error = clReleaseContext(context);
	if (error != CL_SUCCESS) {
		errorMessage = "Error releasing context";
		returnVal = 0;
	}

	return returnVal;
}

int testFunc(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements) {
	cl_int error = 0;
	cl_int error2 = 0;
	size_t logSize = 0;
	const size_t globalWS = 1;
	const size_t localWS = 1;
	cl_program program;
	cl_mem input = NULL;
	cl_mem output = NULL;
	cl_kernel kernel = NULL;
	char kernelName[100];
	void *inputHost = NULL;
	void *outputHost = NULL;
	char *buildLog;
	char source[5000];
	unsigned int srcSize1;
	size_t srcSize2;
	unsigned int *key;
	unsigned int i;
	const unsigned int kernelSize = 2;
	int kernelTypes[2] = {1,2};
	const char *src;

	outputHost = (void*)malloc(testCases[currentTest].memSize);
	key = (unsigned int*)malloc(sizeof(unsigned int)*testCases[currentTest].vectorSize);

	for (i=0; i<kernelSize; i++) {
		generateKernel(source, &srcSize1, testCases[currentTest].typeName, testCases[currentTest].vectorSize, testCases[currentTest].illegalVectorSize, kernelTypes[i], key);
		src = source;
		srcSize2 = srcSize1;
		program = clCreateProgramWithSource(context, 1, &src, &srcSize2, &error);
		if (error != CL_SUCCESS) {
			errorMessage = "Program cannot be created!";
			printf("%s\n", errorMessage);
			return -1;
		}
		error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
		if (error == CL_SUCCESS) {
			error = -1;
			printf("The code below compiled:\n\n%s",source);
			inputHost = generateVector();
			input = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				testCases[currentTest].memSize, inputHost, &error2);
			output = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
				testCases[currentTest].memSize, NULL, &error2);
			sprintf(kernelName,"illegal_vector_size_k%d_%s%u",kernelTypes[i],testCases[currentTest].typeName,testCases[currentTest].vectorSize);
			kernel = clCreateKernel(program, kernelName, &error2);
			error2 = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
			error2 = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
			error2 = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &globalWS, &localWS, 0, NULL, NULL);
			error2 = clEnqueueReadBuffer(queue, output, CL_TRUE, 0, testCases[currentTest].memSize, outputHost, 0, NULL, NULL);
			if (compare(inputHost, outputHost, key))
				printf("Values copied sucessfully.\n");
			else
				printf("Values cannot copied as expected.\n");
		} else if (error == CL_BUILD_PROGRAM_FAILURE) {
			error = 0;
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
			buildLog = (char*)malloc(sizeof(char)*(logSize+1));
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, buildLog, NULL);
			buildLog[logSize] = '\0';
			puts(buildLog);
			free(buildLog);
		}
		clReleaseKernel(kernel);
		clReleaseMemObject(input);
		clReleaseMemObject(output);
		clReleaseProgram(program);
		free(inputHost);
	}

	free(key);
	free(outputHost);
	currentTest++;
	return error;
}

unsigned int totalCaseSize() {
	unsigned int i;
	unsigned int j;
	unsigned int illegalSizeCount;
	unsigned int totalSize = 0;
	for (i=0; i<VECTOR_SIZE_COUNT; i++) {
		illegalSizeCount = 0;
		for (j=0; j<ILLEGAL_SIZE_COUNT; j++) {
			if (vectorSizes[i] > illegalSizes[j])
				illegalSizeCount++;
		}
		vectorCaseSizes[i] = illegalSizeCount;
		totalSize += illegalSizeCount;
	}
	countOfTestsPerType += totalSize;
	totalSize *= TYPE_COUNT;
	return totalSize;
}


int runTests(int argc, const char *argv[]) {
	unsigned int i,j,m,n,typeNameIndex,vectorSizeIndex;
	int error = 0;
	char testName[20];
	unsigned int totalSize;
	basefn *basefn_list;
	char **basefn_names;
	unsigned int passedCount = 0;
	unsigned int failedCount = 0;

	totalSize = totalCaseSize();
	vecTypeIndex = (unsigned int*)malloc(sizeof(unsigned int)*totalSize+1);
	vecSizeIndex = (unsigned int*)malloc(sizeof(unsigned int)*totalSize+1);

	testCases = (TestCase*)malloc(sizeof(TestCase)*totalSize+1);
	basefn_list = (basefn*)malloc(sizeof(basefn)*totalSize+1);
	basefn_names = (char**)malloc(sizeof(char*)*(totalSize+1));
	n = 0;

	srand(time(NULL));

	for (i=0; i<VECTOR_SIZE_COUNT; i++)
		n += vectorCaseSizes[i];

	for (i=0; i<totalSize; i++) {
		basefn_list[i] = testFunc;
		basefn_names[i] = (char*)malloc(sizeof(char)*100);
		m = i%n;
		for (j=0; j<VECTOR_SIZE_COUNT; j++) {
			if (m < vectorCaseSizes[j]) {
				vectorSizeIndex = j;
				testCases[i].vectorSize = vectorSizes[j];
				testCases[i].illegalVectorSize = illegalSizes[m];
				break;
			} else
				m -= vectorCaseSizes[j];
		}
		typeNameIndex = i/(totalSize/TYPE_COUNT);
		vecTypeIndex[i] = typeNameIndex;
		vecSizeIndex[i] = vectorSizeIndex;
		strcpy(testCases[i].typeName,typeNames[typeNameIndex]);
		testCases[i].memSize = memSizes[typeNameIndex*VECTOR_SIZE_COUNT+vectorSizeIndex];
		sprintf(basefn_names[i],"Base Vector:%s%u, Illegal Vector Size:%u",testCases[i].typeName,testCases[i].vectorSize,testCases[i].illegalVectorSize);
	}

	basefn_names[totalSize] = (char*)malloc(sizeof(char)*100);
	sprintf(basefn_names[totalSize], "all");

	for (i=0; i<totalSize; i++) {
		printf("%s\n", basefn_names[i]);
		if (basefn_list[i](device, context, queue, 0) == 0) {
			printf("PASSED\n");
			passedCount++;
		} else {
			printf("FAILED\n");
			failedCount++;
		}
	}
	printf("\nDONE!\n\nPASSED: %d\nFAILED: %d\nTOTAL: %d\n", passedCount, failedCount, totalSize);

	//error = runTestHarness(argc, argv, totalSize+1, basefn_list, (const char**)basefn_names, false, false, 0);

	free(testCases);
	free(basefn_list);
	//for (i=0; i<totalSize; i++)
		//free(basefn_names[i]);
	free(basefn_names);
	free(vecTypeIndex);
	free(vecSizeIndex);

	return 0;
}
