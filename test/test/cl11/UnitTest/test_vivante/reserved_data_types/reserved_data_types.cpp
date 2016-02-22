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


/******************************************************************
//
//  OpenCL Conformance Tests
//
//  Copyright:	(c) 2008-2009 by Apple Inc. All Rights Reserved.
//
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#if !defined(_WIN32)
#include <stdbool.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>


#include "procs.h"

const char *kernel_code_reserved_data_types = "__kernel void reserved_data_types(__global uchar *out)\n"
"{\n"
"typedef int %s%s;\n"
"}\n";


int reserved_data_types(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass){
	cl_mem streams;
	unsigned char *output_h;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	size_t threads[1];
	char kernel_code_int[512];
	const char *constkernelint;
	const char *floatnxm="float%sx%s";
	const char *doublenxm="double%sx%s";
	char charf_nxm[10];
	char chard_nxm[10];

	int err;

	int randomN1 = rand()%20;
	char R1[2];
	//itoa(randomN1, R1, 10);
	sprintf(R1, "%d", randomN1);

	int randomN2 = rand()%20;
	char R2[2];
	//itoa(randomN2, R2, 10);
	sprintf(R2, "%d", randomN2);

	sprintf(charf_nxm, floatnxm,R1,R2);
	sprintf(chard_nxm, doublenxm,R1,R2);

	const char    *types[] = {
		"quad", "half", "complex half", "imaginary half", "complex float", "imaginary float", "complex double",
		"imaginary double","complex quad", "imaginary quad", "long double", "long long","unsigned long long","ulong long",
		"char", "uchar", "short", "ushort", "int", "uint", "long", "ulong", "float", charf_nxm, chard_nxm
	};

	const char    *Ntypes[] = {
		"", "2", "3", "4", "8", "16"
	};


	size_t length = sizeof(unsigned char) * num_elements;
	output_h = (unsigned char*)malloc(length);

	streams = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
	if (!streams)
	{
		printf("clCreateBuffer failed\n");
		clReleaseMemObject(streams);
		clReleaseKernel(kernel);
		clReleaseProgram(program);
		free(output_h);
		fail++;
		return -1;
	}


	for(int t=0; t<25; t++)
	{
		int s=0;
		int n=6;
		if((strcmp(types[t], "quad")==0) ||
			(strcmp(types[t], "half")==0) ||
			(strcmp(types[t], "unsigned long long")==0) ||
			(strcmp(types[t], "char")==0) ||
			(strcmp(types[t], "uchar")==0) ||
			(strcmp(types[t], "short")==0) ||
			(strcmp(types[t], "ushort")==0) ||
			(strcmp(types[t], "int")==0) ||
			(strcmp(types[t], "uint")==0) ||
			(strcmp(types[t], "long")==0) ||
			(strcmp(types[t], "ulong")==0) ||
			(strcmp(types[t], "float")==0))
			s=1;
		if((strcmp(types[t], charf_nxm)==0) || (strcmp(types[t], chard_nxm)==0))
			n=1;

		for( ;s<n;s++)
		{
			sprintf(kernel_code_int, kernel_code_reserved_data_types,types[t],Ntypes[s]);
			constkernelint = kernel_code_int;

			err = create_kernel(context, &program, &kernel, 1, &constkernelint, "reserved_data_types" );

			if (err != CL_SUCCESS) {
				printf("Kernel compilation error using %s%s as a type name.\n", types[t],Ntypes[s]);
				err = CL_SUCCESS;
			        pass++;
				continue;
			}

			err  = clSetKernelArg(kernel, 0, sizeof streams, &streams);
			if (err != CL_SUCCESS)
			{
				printf("clSetKernelArgs failed\n");
				clReleaseMemObject(streams);
				clReleaseKernel(kernel);
				clReleaseProgram(program);
				free(output_h);
				fail++;
				return -1;
			}
			threads[0] = (unsigned int)num_elements;
			err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, threads, NULL, 0, NULL, NULL);
			if (err != CL_SUCCESS)
			{
				printf("clEnqueueNDRangeKernel failed\n");
				clReleaseMemObject(streams);
				clReleaseKernel(kernel);
				clReleaseProgram(program);
				free(output_h);
				fail++;
				return -1;
			}
			err = clEnqueueReadBuffer(queue, streams, CL_TRUE, 0, length, output_h, 0, NULL, NULL);
			if (err != CL_SUCCESS)
			{
				printf("clReadArray failed\n");
				clReleaseMemObject(streams);
				clReleaseKernel(kernel);
				clReleaseProgram(program);
				free(output_h);
				fail++;
				return -1;
			}
			pass++;
		}
	}

	clReleaseMemObject(streams);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	free(output_h);

	return err;
}


