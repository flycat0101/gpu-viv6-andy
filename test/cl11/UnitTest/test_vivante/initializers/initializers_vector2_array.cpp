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

const char *kernel_code_initializers_vector2_array = "__kernel void initializers_vector2_array(__global uchar2 *out)\n"
"{\n"
"int  tid = get_global_id(0);\n"
"int k;\n"
"%s2 var[4] = {(%s2) (0,1), (%s2) (0,1),(%s2) (0,1),(%s2) (0,1)};\n"
"for(k=0; k<4 ;k++) \n"
"	out[tid*4+k] = convert_uchar2(var[k]);\n"
"}\n";

int
verify_initializers_vector2_array(cl_uchar2 *outptr, int n, const char *type)
{
	int i,j;
	for(i=0 ; i<n; i++){
		cl_uchar var0 = 0;
		cl_uchar var1 = 1;
		for(j=0; j<4;j++){
			if( outptr[i*4 + j].s[0] != var0 || outptr[i*4 + j].s[1] != var1 )
				return -1;
		}

	}
	printf("initializers test - array %s2 passed\n", type);
	return 0;
}

int initializers_vector2_array(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total)
{
	cl_mem streams;
	cl_uchar2 *output_h;
	cl_program program;
	cl_kernel kernel;
	size_t threads[1];
	char kernel_code_int[512];
	const char *constkernelint;

	int err, i;

	const char    *types[] = {
		"int", "uint", "char", "uchar", "short", "ushort", "float"
	};



	size_t length = sizeof(cl_uchar2) * num_elements * 4;
	output_h = (cl_uchar2*)malloc(length);

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


	for(int t=0; t<7; t++)
	{

		sprintf(kernel_code_int, kernel_code_initializers_vector2_array, types[t], types[t], types[t], types[t], types[t] );
		constkernelint = kernel_code_int;
		err = create_kernel(context, &program, &kernel, 1, &constkernelint, "initializers_vector2_array" );
		if (err){
			clReleaseMemObject(streams);
			clReleaseKernel(kernel);
			clReleaseProgram(program);
			free(output_h);
		fail++;
			return -1;
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
		err = verify_initializers_vector2_array(output_h, num_elements, types[t]);
		if(!err)
			pass++;
		else fail++;
	}



	clReleaseMemObject(streams);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	free(output_h);

	memset((void*)&kernel_code_int, 0, sizeof(char)*512);
	return err;
}


