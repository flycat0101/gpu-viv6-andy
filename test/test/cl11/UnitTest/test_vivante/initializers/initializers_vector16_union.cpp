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

const char *kernel_code_initializers_vector16_union = "__kernel void initializers_vector16_union(__global uchar16 *out)\n"
"{\n"
"int  tid = get_global_id(0);\n"
"union {%s16 var; } u;\n"
"u.var = (%s16) (1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1); \n"
"out[tid] = convert_uchar16(u.var);\n"
"}\n";

int
verify_initializers_vector16_union(cl_uchar16 *outptr, int n, const char *type)
{
    int i;
    for(i=0 ; i<n; i++){
        cl_uchar var = 1;

        if( outptr[i].s[0] != var || outptr[i].s[1] != var || outptr[i].s[2] != var || outptr[i].s[3] != var ||
            outptr[i].s[4] != var || outptr[i].s[5] != var || outptr[i].s[6] != var || outptr[i].s[7] != var ||
            outptr[i].s[8] != var || outptr[i].s[9] != var || outptr[i].s[10] != var || outptr[i].s[11] != var ||
            outptr[i].s[12] != var || outptr[i].s[13] != var || outptr[i].s[14] != var || outptr[i].s[15] != var)
            return -1;
    }
    printf("initializers test - union %s16 passed\n", type);
    return 0;
}

int initializers_vector16_union(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total)
{
    cl_mem streams;
    cl_uchar16 *output_h;
    cl_program program;
    cl_kernel kernel;
    size_t threads[1];
    char kernel_code_int[512];
    const char *constkernelint;

    int err;

    const char    *types[] = {
        "int", "uint", "char", "uchar", "short", "ushort", "float"
    };



    size_t length = sizeof(cl_uchar16) * num_elements;
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


    for(int t=0; t<7; t++)
    {

        sprintf(kernel_code_int, kernel_code_initializers_vector16_union, types[t], types[t]);
        constkernelint = kernel_code_int;
        err = create_kernel(context, &program, &kernel, 1, &constkernelint, "initializers_vector16_union" );
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
        err = verify_initializers_vector16_union(output_h, num_elements, types[t]);
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


