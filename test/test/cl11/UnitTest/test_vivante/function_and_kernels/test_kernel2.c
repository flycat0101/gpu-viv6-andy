/****************************************************************************
*
*    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
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

const char *kernel_code_test_kernel2 = "__kernel void kernel2(__global unsigned char *src, __global unsigned char *dst, unsigned char n)\n"
"{\n"
 "   int  tid = get_global_id(0);\n"
"\n"
"    dst[tid] = tid % (1 << (src[tid] % (n)));\n"
"\n"
"}\n"
"\n"
"__kernel void test_kernel2(__global unsigned char *src, __global unsigned char *dst)\n"
"{\n"
"    __private unsigned char n;\n"
"    n= 8;\n"
"\n"
 "   kernel2(src, dst, n);\n"
"\n"
"}\n";

int
verify_test_kernel2(unsigned char *inptr, unsigned char *outptr, int n)
{
    int i;

    for(i=0 ; i<n; i++){

        if( outptr[i] != i % (1 << (inptr[i] % 8)))
            return -1;

    }

    log_info("test_kernel2 test passed\n");
    return 0;
}

int test_kernel2(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements)
{

    cl_mem streams[2];
    unsigned char *input_h, *output_h;
    cl_program program;
    cl_kernel kernel;
    size_t threads[1];
    int err, i;
    MTdata d = init_genrand( gRandomSeed );

      //kernel_code_test_kernel2 = (const char*)read_source_file("test_kernel2.cl");

    size_t length = sizeof(unsigned char) * num_elements;
    input_h  = (unsigned char*)malloc(length);
    output_h = (unsigned char*)malloc(length);

    streams[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
    if (!streams[0])
    {
        log_error("clCreateBuffer failed\n");
        return -1;
    }
    streams[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
    if (!streams[1])
    {
        log_error("clCreateBuffer failed\n");
        return -1;
    }

    for (i=0; i<num_elements; i++){

        input_h[i] = ((unsigned int)genrand_int32(d))%256;

    }

    free_mtdata(d); d = NULL;

  err = clEnqueueWriteBuffer(queue, streams[0], CL_TRUE, 0, length, input_h, 0, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    log_error("clEnqueueWriteBuffer failed\n");
    return -1;
  }

  err = create_kernel(context, &program, &kernel, 1, &kernel_code_test_kernel2, "test_kernel2" );
  if (err)
    return -1;

  err  = clSetKernelArg(kernel, 0, sizeof streams[0], &streams[0]);
  err |= clSetKernelArg(kernel, 1, sizeof streams[1], &streams[1]);
    if (err != CL_SUCCESS)
    {
        log_error("clSetKernelArgs failed\n");
        return -1;
    }

    threads[0] = (unsigned int)num_elements;
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, threads, NULL, 0, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    log_error("clEnqueueNDRangeKernel failed\n");
    return -1;
  }

  err = clEnqueueReadBuffer(queue, streams[1], CL_TRUE, 0, length, output_h, 0, NULL, NULL);
  if (err != CL_SUCCESS)
  {
    log_error("clReadArray failed\n");
    return -1;
  }

  err = verify_test_kernel2(input_h, output_h, num_elements);

    // cleanup
    clReleaseMemObject(streams[0]);
    clReleaseMemObject(streams[1]);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    free(input_h);
    free(output_h);

    return err;
}


