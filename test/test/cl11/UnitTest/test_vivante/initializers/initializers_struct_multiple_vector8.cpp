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

const char *kernel_code_initializers_struct_multiple_vector8 = "typedef struct _my_struct{ \n"
"int8 i; \n"
"uint8 ui; \n"
"char8 ch; \n"
"uchar8 uc; \n"
"short8 s; \n"
"ushort8 us; \n"
"float8 f; \n"
"int8 i_ar[2]; \n"
"uint8 ui_ar[2]; \n"
"char8 ch_ar[2]; \n"
"uchar8 uc_ar[2]; \n"
"short8 s_ar[2]; \n"
"ushort8 us_ar[2]; \n"
"float8 f_ar[2]; \n"
"} my_struct; \n"
"__kernel void initializers_struct_multiple_vector8(__global uchar8 *out)\n"
"{\n"
"int i=0; \n"
"my_struct s = {(int8) (1),(uint8) (2),(char8)('3'),(uchar8) ('4'),(short8) (5),(ushort8) (6),(float8) (7.0),{(int8)(8),(int8)(9)},\n"
"{(uint8)(10),(uint8)(11)},{(char8) ('12'), (char8) ('13')},{(uchar8)('14'), (uchar8)('15')},{(short8)(16),(short8)(17)},\n"
"{(ushort8)(18),(ushort8)(19)},{(float8)(20.0),(float8) (21.0)} };\n"
"out[0] = convert_uchar8(s.i); \n"
"out[1] = convert_uchar8(s.ui); \n"
"out[2] = convert_uchar8(s.ch); \n"
"out[3] = convert_uchar8(s.uc); \n"
"out[4] = convert_uchar8(s.s); \n"
"out[5] = convert_uchar8(s.us); \n"
"out[6] = convert_uchar8(s.f); \n"
"out[7] = convert_uchar8(s.i_ar[0]); \n"
"out[8] = convert_uchar8(s.i_ar[1]); \n"
"out[9] = convert_uchar8(s.ui_ar[0]); \n"
"out[10] = convert_uchar8(s.ui_ar[1]); \n"
"out[11] = convert_uchar8(s.ch_ar[0]); \n"
"out[12] = convert_uchar8(s.ch_ar[1]); \n"
"out[13] = convert_uchar8(s.uc_ar[0]); \n"
"out[14] = convert_uchar8(s.uc_ar[1]); \n"
"out[15] = convert_uchar8(s.s_ar[0]); \n"
"out[16] = convert_uchar8(s.s_ar[1]); \n"
"out[17] = convert_uchar8(s.us_ar[0]); \n"
"out[18] = convert_uchar8(s.us_ar[1]); \n"
"out[19] = convert_uchar8(s.f_ar[0]); \n"
"out[20] = convert_uchar8(s.f_ar[1]); \n"
"}\n";


int
verify_initializers_struct_multiple_vector8(cl_uchar8 *outptr, int n)
{
	cl_uchar u1[21] = {1,2,'3','4',5,6,7.0,8,9,10,11,'12','13','14','15',16,17,18,19,20.0,21.0};
	int i=0;

	for(i=0;i<21;i++){
		//	printf("%d: %d %d %d\n", i,  outptr[i].s[0] ,outptr[i].s[1], outptr[i].s[2] );
		if(outptr[i].s[0] != u1[i] || outptr[i].s[1] != u1[i] || outptr[i].s[2] != u1[i] || outptr[i].s[3] != u1[i] ||
			outptr[i].s[4] != u1[i] || outptr[i].s[5] != u1[i] || outptr[i].s[6] != u1[i] || outptr[i].s[7] != u1[i]  )
			return -1;
	}

	printf("initializers test - multiple struct vector8 passed\n");
	return 0;

}

int initializers_struct_multiple_vector8(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total)
{

	cl_mem streams;
	cl_uchar8 *output_h;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	size_t threads[1];
	char kernel_code_int[2048];
	const char *constkernelint;

	int err;


	size_t length = 512;
	output_h = (cl_uchar8*)malloc(length);



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



	sprintf(kernel_code_int, kernel_code_initializers_struct_multiple_vector8);
	constkernelint = kernel_code_int;
	err = create_kernel(context, &program, &kernel, 1, &constkernelint, "initializers_struct_multiple_vector8" );
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
	err = verify_initializers_struct_multiple_vector8(output_h, num_elements);
		if(!err)
			pass++;
		else fail++;




	clReleaseMemObject(streams);
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	free(output_h);

	return err;
}


