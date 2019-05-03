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

const char *kernel_code_initializers_struct_multiple_vector16 = "typedef struct _my_struct{ \n"
"int16 i; \n"
"uint16 ui; \n"
"char16 ch; \n"
"uchar16 uc; \n"
"short16 s; \n"
"ushort16 us; \n"
"float16 f; \n"
"int16 i_ar[2]; \n"
"uint16 ui_ar[2]; \n"
"char16 ch_ar[2]; \n"
"uchar16 uc_ar[2]; \n"
"short16 s_ar[2]; \n"
"ushort16 us_ar[2]; \n"
"float16 f_ar[2]; \n"
"} my_struct; \n"
"__kernel void initializers_struct_multiple_vector16(__global uchar16 *out)\n"
"{\n"
"int i=0; \n"
"my_struct s = {(int16) (1),(uint16) (2),(char16)('3'),(uchar16) ('4'),(short16) (5),(ushort16) (6),(float16) (7.0),\n"
"{(int16)(8),(int16)(9)}, {(uint16)(10),(uint16)(11)},{(char16) ('12'), (char16) ('13')},{(uchar16)('14'), (uchar16)('15')},\n"
"{(short16)(16),(short16)(17)},{(ushort16)(18),(ushort16)(19)},{(float16)(20.0),(float16) (21.0)}};\n"
"out[0] = convert_uchar16(s.i); \n"
"out[1] = convert_uchar16(s.ui); \n"
"out[2] = convert_uchar16(s.ch); \n"
"out[3] = convert_uchar16(s.uc); \n"
"out[4] = convert_uchar16(s.s); \n"
"out[5] = convert_uchar16(s.us); \n"
"out[6] = convert_uchar16(s.f); \n"
"out[7] = convert_uchar16(s.i_ar[0]); \n"
"out[8] = convert_uchar16(s.i_ar[1]); \n"
"out[9] = convert_uchar16(s.ui_ar[0]); \n"
"out[10] = convert_uchar16(s.ui_ar[1]); \n"
"out[11] = convert_uchar16(s.ch_ar[0]); \n"
"out[12] = convert_uchar16(s.ch_ar[1]); \n"
"out[13] = convert_uchar16(s.uc_ar[0]); \n"
"out[14] = convert_uchar16(s.uc_ar[1]); \n"
"out[15] = convert_uchar16(s.s_ar[0]); \n"
"out[16] = convert_uchar16(s.s_ar[1]); \n"
"out[17] = convert_uchar16(s.us_ar[0]); \n"
"out[18] = convert_uchar16(s.us_ar[1]); \n"
"out[19] = convert_uchar16(s.f_ar[0]); \n"
"out[20] = convert_uchar16(s.f_ar[1]); \n"
"}\n";


int
verify_initializers_struct_multiple_vector16(cl_uchar16 *outptr, int n)
{
    cl_uchar u1[21] = {1,2,'3','4',5,6,7.0,8,9,10,11,'12','13','14','15',16,17,18,19,20.0,21.0};
    int i=0;

    for(i=0;i<21;i++){
        //    printf("%d: %d %d %d\n", i,  outptr[i].s[0] ,outptr[i].s[1], outptr[i].s[2] );
        if(outptr[i].s[0] != u1[i] || outptr[i].s[1] != u1[i] || outptr[i].s[2] != u1[i] || outptr[i].s[3] != u1[i] ||
            outptr[i].s[4] != u1[i] || outptr[i].s[5] != u1[i] || outptr[i].s[6] != u1[i] || outptr[i].s[7] != u1[i] ||
            outptr[i].s[8] != u1[i] || outptr[i].s[9] != u1[i] || outptr[i].s[10] != u1[i] || outptr[i].s[11] != u1[i] ||
            outptr[i].s[12] != u1[i] || outptr[i].s[13] != u1[i] || outptr[i].s[14] != u1[i] || outptr[i].s[15] != u1[i])
            return -1;
    }

    printf("initializers test - multiple struct vector16 passed\n");
    return 0;

}

int initializers_struct_multiple_vector16(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total)
{

    cl_mem streams;
    cl_uchar16 *output_h;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    size_t threads[1];
    char kernel_code_int[2048];
    const char *constkernelint;

    int err;


    size_t length = 512;
    output_h = (cl_uchar16*)malloc(length);



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



    sprintf(kernel_code_int, kernel_code_initializers_struct_multiple_vector16);
    constkernelint = kernel_code_int;
    err = create_kernel(context, &program, &kernel, 1, &constkernelint, "initializers_struct_multiple_vector16" );
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
    err = verify_initializers_struct_multiple_vector16(output_h, num_elements);
        if(!err)
            pass++;
        else fail++;




    clReleaseMemObject(streams);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    free(output_h);

    return err;
}


