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


#include "multidimensionalarraystestrunner.h"
#include "multidimensionalarraystest.h"
#include <iostream>
#include "randnumgenerator.h"
#include "roundingmode.h"

typedef struct TestNode {
	MultiDimensionalArraysTest *test;
	TestNode *next;
} TestNode;

TestNode *head = NULL;
TestNode *tail = NULL;
TestNode *current = NULL;
int nodeSize = 0;

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

    cl_device_fp_config fpconfig = 0;
    clGetDeviceInfo( device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof( fpconfig ), &fpconfig, NULL );

    if (!(fpconfig & CL_FP_ROUND_TO_NEAREST) )
    {
	    setRoundToZero();
    }

    if (!(fpconfig & CL_FP_DENORM))
    {
	    flushToZero();
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

const char* getErrorMessage() {
	return errorMessage;
}

void addTest(MultiDimensionalArraysTest* test) {
	if (nodeSize == 0) {
		head = new TestNode;
		head->test = test;
		head->next = NULL;
		tail = head;
	} else {
		tail->next = new TestNode;
		tail = tail->next;
		tail->test = test;
		tail->next = NULL;
	}
	nodeSize++;
}

void cleanTests() {
	TestNode *current;
	while (nodeSize > 0) {
		current = head;
		head = head->next;
		if (current->test) {
			delete current->test;
			current->test = 0;
		}
		if (current) {
			delete current;
			current = 0;
		}
		nodeSize--;
	}
}

int runTests(int argc, const char *argv[]) {
	if (nodeSize == 0)
		return 0;

	unsigned int passedSize = 0;
	unsigned int failedSize = 0;

	setRand();
	TestNode *curr = head;
	std::cout << "Multi-Dimensional Arrays Tests:" << std::endl << std::endl;
	while (curr != NULL) {
		std::cout << curr->test->getTitle() << std::endl;
		int successCode = curr->test->runHarnessMode(device, context, queue);
		if (successCode != CL_SUCCESS) {
			errorMessage = curr->test->getErrorMessage();
			std::cout << "Failed: " << std::endl;
			failedSize++;
		} else {
			std::cout << "Passed" << std::endl;
			passedSize++;
		}
		curr->test->cleanup();
		curr = curr->next;
	}

	std::cout << std::endl << "Done!" << std::endl << std::endl;
	std::cout << "Passed: " << passedSize << std::endl;
	std::cout << "Failed: " << failedSize << std::endl;
	std::cout << "Total: " << nodeSize << std::endl;

	return 1;
}