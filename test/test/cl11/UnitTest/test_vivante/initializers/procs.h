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


#ifndef PROCS_H
#define PROCS_H

#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define uint unsigned int
#define print_error(errCode,msg)    printf( "ERROR: %s! (%s from %s:%d)\n", msg, get_error_string( errCode ), __FILE__, __LINE__ );

extern int      initializers_scalar(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector2(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector3(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector4(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector8(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector16(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);

extern int      initializers_scalar_array(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector2_array(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector3_array(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector4_array(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector8_array(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector16_array(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);

extern int      initializers_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector2_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector3_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector4_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector8_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector16_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);

extern int      initializers_scalar_array_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector2_array_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector3_array_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector4_array_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector8_array_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_vector16_array_union(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);

extern int      initializers_union_multiple(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_union_multiple_vector2(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_union_multiple_vector3(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_union_multiple_vector4(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_union_multiple_vector8(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_union_multiple_vector16(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);

extern int      initializers_struct_multiple(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_struct_multiple_vector2(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_struct_multiple_vector3(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_struct_multiple_vector4(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_struct_multiple_vector8(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);
extern int      initializers_struct_multiple_vector16(cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements, int& fail, int& pass, int&total);

const char    *get_error_string( int clErrorCode );
int create_kernel( cl_context context, cl_program *outProgram, cl_kernel *outKernel, unsigned int numKernelLines, const char **kernelProgram, const char *kernelName );

#endif