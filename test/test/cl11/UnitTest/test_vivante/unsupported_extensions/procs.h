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


#ifndef PROCS_H
#define PROCS_H

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define print_error(errCode,msg)    printf( "ERROR: %s! (%s from %s:%d)\n", msg, get_error_string( errCode ), __FILE__, __LINE__ );

//extern int      create_program_and_kernel(const char *source, const char *kernel_name, cl_program *program_ret, cl_kernel *kernel_ret);

const char    *get_error_string( int clErrorCode );
int create_kernel( cl_context context, cl_program *outProgram, cl_kernel *outKernel, unsigned int numKernelLines, const char **kernelProgram, const char *kernelName );
int is_extension_available( cl_device_id device, const char *extensionName );


int test_int64(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp64(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp64_func_math_1arg(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp64_func_math_2arg(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp64_func_math_3arg(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp64_func_math4(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp16(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp16_func_math_1arg(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp16_func_math_2arg(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp16_func_math_3arg(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_fp16_func_math4(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_write_image3d(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_gl_sharing(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_d3d10(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);
int test_gl_event(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail);

#endif
