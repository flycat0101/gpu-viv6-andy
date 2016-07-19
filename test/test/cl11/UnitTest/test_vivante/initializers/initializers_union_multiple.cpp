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
//  Copyright:    (c) 2008-2009 by Apple Inc. All Rights Reserved.
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

const char *kernel_code_initializers_union_multiple = "typedef union _my_union{ \n"
"int i; \n"
"uint ui; \n"
"char ch; \n"
"uchar uc; \n"
"short s; \n"
"ushort us; \n"
"float f; \n"
"int i_ar[3]; \n"
"uint ui_ar[3]; \n"
"char ch_ar[3]; \n"
"uchar uc_ar[3]; \n"
"short s_ar[3]; \n"
"ushort us_ar[3]; \n"
"float f_ar[3]; \n"
"} my_union; \n"
"__kernel void initializers_union_multiple(__global uchar *out)\n"
"{\n"
"int i=0; \n"
"my_union u1 = {.i=1};\n"
"my_union u2 = {.ui=2};\n"
"my_union u3 = {.ch='3'};\n"
"my_union u4 = {.uc='4'};\n"
"my_union u5 = {.s=5};\n"
"my_union u6 = {.us=6};\n"
"my_union u7 = {.f=7.0};\n"
"my_union u8 = {.i_ar={1,2,3}};\n"
"my_union u9 = {.ui_ar={2,3,4}};\n"
"my_union u10 = {.ch_ar={'3','4','5'}};\n"
"my_union u11 = {.uc_ar={'4','5','6'}};\n"
"my_union u12 = {.s_ar={5,6,7}};\n"
"my_union u13 = {.us_ar={6,7,8}};\n"
"my_union u14 = {.f_ar={7.0,8.0,9.0}};\n"
"out[0] = u1.i; \n"
"out[1] = u2.ui; \n"
"out[2] = u3.ch; \n"
"out[3] = u4.uc; \n"
"out[4] = u5.s; \n"
"out[5] = u6.us; \n"
"out[6] = u7.f; \n"
"out[7] = u8.i_ar[0]; \n"
"out[8] = u8.i_ar[1]; \n"
"out[9] = u8.i_ar[2]; \n"
"out[10] = u9.ui_ar[0]; \n"
"out[11] = u9.ui_ar[1]; \n"
"out[12] = u9.ui_ar[2]; \n"
"out[13] = u10.ch_ar[0]; \n"
"out[14] = u10.ch_ar[1]; \n"
"out[15] = u10.ch_ar[2]; \n"
"out[16] = u11.uc_ar[0]; \n"
"out[17] = u11.uc_ar[1]; \n"
"out[18] = u11.uc_ar[2]; \n"
"out[19] = u12.s_ar[0]; \n"
"out[20] = u12.s_ar[1]; \n"
"out[21] = u12.s_ar[2]; \n"
"out[22] = u13.us_ar[0]; \n"
"out[23] = u13.us_ar[1]; \n"
"out[24] = u13.us_ar[2]; \n"
"out[25] = u14.f_ar[0]; \n"
"out[26] = u14.f_ar[1]; \n"
"out[27] = u14.f_ar[2]; \n"
"}\n";


int
verify_initializers_union_multiple(cl_uchar *outptr, int n)
{
    cl_uchar u1[28] = {1,2,'3','4',5,6,7.0,1,2,3,2,3,4,'3','4','5','4','5','6',5,6,7,6,7,8,7.0,8.0,9.0};
    int i=0;

    for(i=0;i<28;i++){
        //    printf("%d %d\n", outptr[i], u1[i]);
        if(outptr[i] != u1[i])
            return -1;
    }

    printf("initializers test - multiple union passed\n");
    return 0;

}

int initializers_union_multiple(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total)
{

    cl_mem streams;
    cl_uchar *output_h;
    cl_program program;
    cl_kernel kernel;
    size_t threads[1];
    char kernel_code_int[2048];
    const char *constkernelint;

    int err;


    size_t length = 512;
    output_h = (cl_uchar*)malloc(length);



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



    sprintf(kernel_code_int, kernel_code_initializers_union_multiple);
    constkernelint = kernel_code_int;
    err = create_kernel(context, &program, &kernel, 1, &constkernelint, "initializers_union_multiple" );
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
    err = verify_initializers_union_multiple(output_h, num_elements);
        if(!err)
            pass++;
        else fail++;




    clReleaseMemObject(streams);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    free(output_h);

    return err;
}


