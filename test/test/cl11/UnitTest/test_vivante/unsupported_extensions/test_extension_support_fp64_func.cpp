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


#include "testBase.h"
#ifndef _WIN32
#include <unistd.h>
#endif


const char *kernel_test_fp64_func_math_1arg =  "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"__kernel void test_fp64_func_math1(__global %s *out_val, __global %s *input_val)\n"
"{\n"
"int  gid = get_global_id(0);\n"
"out_val[gid] = %s(input_val);\n"
"}\n";



const char *kernel_test_fp64_func_math_2arg =  "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"__kernel void test_fp64_func_math2(__global %s *out_val, __global %s *input_A, __global %s *input_B)\n"
"{\n"
"int  gid = get_global_id(0);\n"
"out_val[gid] = %s(input_A, input_B);\n"
"}\n";

const char *kernel_test_fp64_func_math_3arg =  "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"__kernel void test_fp64_func_math3(__global %s *out_val, __global %s *input_A, __global %s *input_B, __global %s *input_C)\n"
"{\n"
"int  gid = get_global_id(0);\n"
"out_val[gid] = %s(input_A, input_B, input_C);\n"
"}\n";

const char *kernel_test_fp64_func_math4 = "#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"__kernel void test_fp64_func_math4(__global %s *out_val, __global %s *input_A, %s %s *input_B)\n"
"{\n"
"int  gid = get_global_id(0);\n"
"out_val[gid] = %s(input_A, &input_B);\n"
"}\n";




int test_fp64_func_math_1arg(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail)
{
    cl_mem streams[2];
    cl_program program;
    cl_kernel kernel;
    size_t threads[1];
    cl_double *output_h, *input_h;
    char kernel_code_int[512];
    const char *constkernelint;

    int err, passCount = 0;
    int typeIndex = 0;

    const char    *types[] = {
        "double", "double2", "double4", "double8", "double16"
    };

    const char *func_name[] = {
        /* math functions */
        "acos",
        "acosh",
        "acospi",
        "asin",
        "asinh",
        "asinpi",
        "atan",
        "atanh",
        "atanpi",
        "cbrt",
        "ceil",
        "cos",
        "cosh",
        "cospi",
        "erfc",
        "erf",
        "exp",
        "exp2",
        "exp10",
        "expm1",
        "fabs",
        "floor",
        "ilogb",
        "log",
        "log2",
        "log10",
        "log1p",
        "logb",
        "rint",
        "round",
        "rsqrt",
        "sin",
        "sinh",
        "sinpi",
        "sqrt",
        "tan",
        "tanh",
        "tanpi",
        "tgamma",
        "trunc",
        /* common functions */
        "degrees",
        "radians",
        "sign",
        /* geometric functions */
        "length",
        "normalize",
        /* relational functions */
        "isfinite",
        "isinf",
        "isnan",
        "isnormal",
        "signbit"
    };

    for(int t=1; t<17; t*=2)
    {

        size_t length = sizeof(cl_double) * num_elements * t;
        input_h = (cl_double*)malloc(length);
        output_h = (cl_double*)malloc(length);

        for(int index = 0; index < 50; index++)
        {

            printf("%s with gentype %s TESTING STARTED:\n", func_name[index], types[typeIndex]);
            printf("--------------------------------------------------");

            streams[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
            if (!streams[0])
            {
                printf("clCreateBuffer #1 failed\n");
                if(input_h) free(input_h);
                if(output_h) free(output_h);
                return -1;
            }

            streams[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
            if (!streams[1])
            {
                printf("clCreateBuffer #2 failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(input_h) free(input_h);
                if(output_h) free(output_h);
                return -1;
            }

            sprintf(kernel_code_int, kernel_test_fp64_func_math_1arg, types[typeIndex], types[typeIndex], func_name[index], types[typeIndex]);
            constkernelint = kernel_code_int;

            if(is_extension_available(device, "cl_khr_fp64"))
            {
                err = create_kernel(context, &program, &kernel, 1, &constkernelint, "test_fp64_func_math1" );
                if (err)
                {
                    printf("Kernel compilation error using double as a type.\n");
                    printf("\n!!Kernel build not successful.\n");
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(kernel) clReleaseKernel(kernel);
                    if(program) clReleaseProgram(program);
                    if(input_h) free(input_h);
                    if(output_h) free(output_h);
                    return -1;
                }
            }
            else
            {
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_h) free(input_h);
                if(output_h) free(output_h);

                passCount++;

                printf("--------------------------------------------------------\n");
                printf("cl_khr_fp64 extension is not supported.!!\n");
                printf("%s function with %s argument passed.\n\n", func_name[index], types[typeIndex]);
                return 0;
            }

            err  = clSetKernelArg(kernel, 0, sizeof streams, &streams);
            if (err != CL_SUCCESS)
            {
                printf("clSetKernelArgs failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_h) free(input_h);
                if(output_h) free(output_h);
                return -1;
            }

            threads[0] = (unsigned int)num_elements;
            err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, threads, NULL, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clEnqueueNDRangeKernel failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_h) free(input_h);
                if(output_h) free(output_h);
                return -1;
            }

            err = clEnqueueReadBuffer(queue, streams[0], CL_TRUE, 0, length, output_h, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clReadArray failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_h) free(input_h);
                if(output_h) free(output_h);
                return -1;
            }

            *input_h = (cl_double)23.0;
            err = clEnqueueReadBuffer(queue, streams[1], CL_TRUE, 0, length, input_h, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clReadArray failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_h) free(input_h);
                if(output_h) free(output_h);
                return -1;
            }

            if(streams[0]) clReleaseMemObject(streams[0]);
            if(streams[1]) clReleaseMemObject(streams[1]);
            if(kernel) clReleaseKernel(kernel);
            if(program) clReleaseProgram(program);
        }

        if(input_h) free(input_h);
        if(output_h) free(output_h);

        printf("%s TESTING FOR ALL FUNCTIONS FINISHED.\n", types[typeIndex]);
        printf("-*****************************************************-\n\n\n");

        typeIndex++;

    }


    //printf("%d / 250 internal tests passed.\n\n", passCount);
    total += 250;
    passed += passCount;
    fail += 250-passCount;


    return 0;
}


int test_fp64_func_math_2arg(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail)
{
    cl_mem streams[3];
    cl_program program;
    cl_kernel kernel;
    size_t threads[1];
    cl_double *output_h, *input_A, *input_B;
    char kernel_code_int[512];
    const char *constkernelint;

    int err, passCount = 0;
    int typeIndex = 0;

    const char    *types[] = {
        "double", "double2", "double4", "double8", "double16"
    };

    const char *func_name[] = {
        /* math functions */
        "atan2",
        "copysign",
        "fdim",
        "fmax",
        "fmin",
        "fmod",
        "hypot",
        "nextafter",
        "pow",
        "remainder",
        /* common functions */
        "max",
        "min",
        "step",
        /* geometric functions */
        "dot",
        "distance",
        /* relational functions */
        "isequal",
        "isnotequal",
        "isgreater",
        "isgreaterequal",
        "isless",
        "islessequal",
        "islessgreater",
        "isordered",
        "isunordered"
    };

    for(int t=1; t<17; t*=2)
    {

        size_t length = sizeof(cl_double) * num_elements * t;
        input_A = (cl_double*)malloc(length);
        input_B = (cl_double*)malloc(length);
        output_h = (cl_double*)malloc(length);

        for(int index = 0; index < 24; index++)
        {

            printf("%s with gentype %s TESTING STARTED:\n", func_name[index], types[typeIndex]);
            printf("--------------------------------------------------");

            streams[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
            if (!streams[0])
            {
                printf("clCreateBuffer #1 failed\n");
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(output_h) free(output_h);
                return -1;
            }

            streams[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
            if (!streams[1])
            {
                printf("clCreateBuffer #2 failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(output_h) free(output_h);
                return -1;
            }

            streams[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
            if (!streams[2])
            {
                printf("clCreateBuffer #3 failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(output_h) free(output_h);
                return -1;
            }

            sprintf(kernel_code_int, kernel_test_fp64_func_math_2arg, types[typeIndex], types[typeIndex], types[typeIndex], func_name[index]);
            constkernelint = kernel_code_int;

            if(is_extension_available(device, "cl_khr_fp64"))
            {
                err = create_kernel(context, &program, &kernel, 1, &constkernelint, "test_fp64_func_math2" );
                if(err)
                {
                    printf("Kernel compilation error using double as a type.\n");
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);
                    if(kernel) clReleaseKernel(kernel);
                    if(program) clReleaseProgram(program);
                    return -1;
                }
            }
            else
            {
                printf("cl_khr_fp64 extension is not supported.!!\n");
                printf("%s function with %s argument passed.\n\n", func_name[index], types[typeIndex]);

                passCount++;

                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(output_h) free(output_h);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);

                return 0;
            }

            err  = clSetKernelArg(kernel, 0, sizeof streams, &streams);
            if (err != CL_SUCCESS)
            {
                printf("clSetKernelArgs failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(output_h) free(output_h);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                return -1;
            }

            threads[0] = (unsigned int)num_elements;
            err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, threads, NULL, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clEnqueueNDRangeKernel failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(output_h) free(output_h);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                return -1;
            }

            err = clEnqueueReadBuffer(queue, streams[0], CL_TRUE, 0, length, output_h, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clReadArray failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(output_h) free(output_h);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                return -1;
            }

            *input_A = (cl_double)23.0;
            err = clEnqueueReadBuffer(queue, streams[1], CL_TRUE, 0, length, input_A, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clReadArray failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(output_h) free(output_h);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                return -1;
            }

            *input_B = (cl_double)2.3;
            err = clEnqueueReadBuffer(queue, streams[2], CL_TRUE, 0, length, input_B, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clReadArray failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(output_h) free(output_h);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                return -1;
            }

            if(streams[0]) clReleaseMemObject(streams[0]);
            if(streams[1]) clReleaseMemObject(streams[1]);
            if(kernel) clReleaseKernel(kernel);
            if(program) clReleaseProgram(program);

        }

        if(input_A) free(input_A);
        if(input_B) free(input_B);
        if(output_h) free(output_h);

        printf("%s TESTING FOR ALL FUNCTIONS FINISHED.\n", types[typeIndex]);
        printf("-*****************************************************-\n\n\n");

        typeIndex++;

    }

    //printf("%d / 120 internal tests passed.\n\n", passCount);
    total += 120;
    passed += passCount;
    fail += 120-passCount;

    return 0;
}


int test_fp64_func_math_3arg(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail)
{
    cl_mem streams[4];
    cl_program program;
    cl_kernel kernel;
    size_t threads[1];
    cl_double *output_h, *input_A, *input_B, *input_C;
    char kernel_code_int[512];
    const char *constkernelint;

    int err, passCount = 0;
    int typeIndex = 0;

    const char    *types[] = {
        "double", "double2", "double4", "double8", "double16"
    };

    const char *func_name[] = {
        /* math functions */
        "fma",
        "mad",
        /* common functions */
        "clamp",
        "mix",
        "smoothstep",
        /* geometric functions */
        /* ------------------- */
        /* relational functions */
        "bitselect"
    };

    for(int t=1; t<17; t*=2)
    {

        size_t length = sizeof(cl_double) * num_elements * t;
        input_A = (cl_double*)malloc(length);
        input_B = (cl_double*)malloc(length);
        input_C = (cl_double*)malloc(length);
        output_h = (cl_double*)malloc(length);

        for(int index = 0; index < 6; index++)
        {

            printf("%s with gentype %s TESTING STARTED:\n", func_name[index], types[typeIndex]);
            printf("--------------------------------------------------");

            streams[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
            if (!streams[0])
            {
                printf("clCreateBuffer #1 failed\n");
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }

            streams[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
            if (!streams[1])
            {
                printf("clCreateBuffer #2 failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }

            streams[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
            if (!streams[2])
            {
                printf("clCreateBuffer #3 failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }

            streams[3] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
            if (!streams[1])
            {
                printf("clCreateBuffer #4 failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(streams[2]) clReleaseMemObject(streams[2]);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }


            sprintf(kernel_code_int, kernel_test_fp64_func_math_3arg, types[typeIndex], types[typeIndex], types[typeIndex], types[typeIndex], func_name[index]);
            constkernelint = kernel_code_int;


            if(is_extension_available(device, "cl_khr_fp64"))
            {
                err = create_kernel(context, &program, &kernel, 1, &constkernelint, "test_fp64_func_math3" );
                if(err)
                {
                    printf("Kernel compilation error using double as a type.\n");

                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(streams[2]) clReleaseMemObject(streams[2]);
                    if(streams[3]) clReleaseMemObject(streams[3]);
                    if(kernel) clReleaseKernel(kernel);
                    if(program) clReleaseProgram(program);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(input_C) free(input_C);
                    if(output_h) free(output_h);

                    return -1;
                }
            }
            else
            {
                printf("\n!!Kernel build not successful.\n");
                printf("cl_khr_fp64 extension is not supported.!!\n");
                printf("%s function with %s argument passed.\n\n", func_name[index], types[typeIndex]);

                passCount++;

                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(streams[2]) clReleaseMemObject(streams[2]);
                if(streams[3]) clReleaseMemObject(streams[3]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);

                return 0;
            }

            err  = clSetKernelArg(kernel, 0, sizeof streams, &streams);
            if (err != CL_SUCCESS)
            {
                printf("clSetKernelArgs failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(streams[2]) clReleaseMemObject(streams[2]);
                if(streams[3]) clReleaseMemObject(streams[3]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }

            threads[0] = (unsigned int)num_elements;
            err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, threads, NULL, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clEnqueueNDRangeKernel failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(streams[2]) clReleaseMemObject(streams[2]);
                if(streams[3]) clReleaseMemObject(streams[3]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }

            err = clEnqueueReadBuffer(queue, streams[0], CL_TRUE, 0, length, output_h, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clReadArray failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(streams[2]) clReleaseMemObject(streams[2]);
                if(streams[3]) clReleaseMemObject(streams[3]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }

            *input_A = (cl_double)23.0;
            err = clEnqueueReadBuffer(queue, streams[1], CL_TRUE, 0, length, input_A, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clReadArray failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(streams[2]) clReleaseMemObject(streams[2]);
                if(streams[3]) clReleaseMemObject(streams[3]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }

            *input_B = (cl_double)2.3;
            err = clEnqueueReadBuffer(queue, streams[2], CL_TRUE, 0, length, input_B, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clReadArray failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(streams[2]) clReleaseMemObject(streams[2]);
                if(streams[3]) clReleaseMemObject(streams[3]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }

            *input_C = (cl_double)4.7;
            err = clEnqueueReadBuffer(queue, streams[3], CL_TRUE, 0, length, input_C, 0, NULL, NULL);
            if (err != CL_SUCCESS)
            {
                printf("clReadArray failed\n");
                if(streams[0]) clReleaseMemObject(streams[0]);
                if(streams[1]) clReleaseMemObject(streams[1]);
                if(streams[2]) clReleaseMemObject(streams[2]);
                if(streams[3]) clReleaseMemObject(streams[3]);
                if(kernel) clReleaseKernel(kernel);
                if(program) clReleaseProgram(program);
                if(input_A) free(input_A);
                if(input_B) free(input_B);
                if(input_C) free(input_C);
                if(output_h) free(output_h);
                return -1;
            }

            if(streams[0]) clReleaseMemObject(streams[0]);
            if(streams[1]) clReleaseMemObject(streams[1]);
            if(streams[2]) clReleaseMemObject(streams[2]);
            if(streams[3]) clReleaseMemObject(streams[3]);
            if(kernel) clReleaseKernel(kernel);
            if(program) clReleaseProgram(program);

        }

        printf("%s TESTING FOR ALL FUNCTIONS FINISHED.\n", types[typeIndex]);
        printf("-*****************************************************-\n\n\n");

        typeIndex++;
        if(input_A) free(input_A);
        if(input_B) free(input_B);
        if(input_C) free(input_C);
        if(output_h) free(output_h);

    }

    //printf("%d / 30 internal tests passed.\n\n", passCount);
    total += 30;
    passed += passCount;
    fail += 30-passCount;

    return 0;
}

int test_fp64_func_math4(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail)
{
    cl_mem streams[3];
    cl_program program;
    cl_kernel kernel;
    size_t threads[1];
    cl_double *output_h, *input_A, *input_B;
    char kernel_code_int[512];
    const char *constkernelint;

    int err, passCount = 0;
    int typeIndex = 0;

    const char    *types[] = {
        "double", "double2", "double4", "double8", "double16"
    };
    const char *intTypes[] = {
        "int", "int2", "int4", "int8", "int16"
    };

    const char *func_name[] = {
        "frexp",
        "lgamma_r",
        "mad",
        "remquo",
        "modf",
        "fract",
        "sincos"
    };

    const char *qualifier_name[] = {
        "__global",
        "__local",
        "__private"
    };

    for(int t=1; t<17; t*=2)
    {

        size_t length = sizeof(cl_double) * num_elements * t;
        input_A = (cl_double*)malloc(length);
        input_B = (cl_double*)malloc(length);
        output_h = (cl_double*)malloc(length);

        for(int index = 0; index < 7; index++)
        {

            for(int qualIndex = 0; qualIndex < 3; qualIndex++)
            {
                if(index < 4)
                {
                    printf("%s with gentype %s TESTING STARTED:\n", func_name[index], intTypes[typeIndex]);
                }
                else
                {
                    printf("%s with gentype %s TESTING STARTED:\n", func_name[index], types[typeIndex]);
                }

                printf("--------------------------------------------------");

                streams[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
                if (!streams[0])
                {
                    printf("clCreateBuffer #1 failed\n");
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);
                    return -1;
                }

                streams[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
                if (!streams[1])
                {
                    printf("clCreateBuffer #2 failed\n");
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);
                    return -1;
                }

                streams[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
                if (!streams[2])
                {
                    printf("clCreateBuffer #3 failed\n");
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);
                    return -1;
                }

                if(index < 4)
                {
                    sprintf(kernel_code_int, kernel_test_fp64_func_math4, types[typeIndex], types[typeIndex], qualifier_name[qualIndex], intTypes[typeIndex], func_name[index]);
                }
                else
                {
                    sprintf(kernel_code_int, kernel_test_fp64_func_math4, types[typeIndex], types[typeIndex], qualifier_name[qualIndex], types[typeIndex], func_name[index]);
                }

                constkernelint = kernel_code_int;

                if(is_extension_available(device, "cl_khr_fp64"))
                {
                    err = create_kernel(context, &program, &kernel, 1, &constkernelint, "test_fp64_func_math4" );
                    if(err)
                    {
                        if(streams[0]) clReleaseMemObject(streams[0]);
                        if(streams[1]) clReleaseMemObject(streams[1]);
                        if(streams[2]) clReleaseMemObject(streams[2]);
                        if(kernel) clReleaseKernel(kernel);
                        if(program) clReleaseProgram(program);
                        if(input_A) free(input_A);
                        if(input_B) free(input_B);
                        if(output_h) free(output_h);
                        printf("\n!!Kernel build not successful.\n");
                        printf("Kernel compilation error using double as a type.\n");
                        return -1;
                    }
                }
                else
                {
                    printf("cl_khr_fp64 extension is not supported.!!\n");
                    printf("%s function with %s %s argument passed.\n\n", func_name[index], qualifier_name[qualIndex], intTypes[typeIndex]);
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(streams[2]) clReleaseMemObject(streams[2]);
                    if(kernel) clReleaseKernel(kernel);
                    if(program) clReleaseProgram(program);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);

                    passCount++;
                    continue;
                }
                err  = clSetKernelArg(kernel, 0, sizeof streams, &streams);
                if (err != CL_SUCCESS)
                {
                    printf("clSetKernelArgs failed\n");
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(streams[2]) clReleaseMemObject(streams[2]);
                    if(kernel) clReleaseKernel(kernel);
                    if(program) clReleaseProgram(program);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);
                    return -1;
                }

                threads[0] = (unsigned int)num_elements;
                err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, threads, NULL, 0, NULL, NULL);
                if (err != CL_SUCCESS)
                {
                    printf("clEnqueueNDRangeKernel failed\n");
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(streams[2]) clReleaseMemObject(streams[2]);
                    if(kernel) clReleaseKernel(kernel);
                    if(program) clReleaseProgram(program);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);
                    return -1;
                }

                err = clEnqueueReadBuffer(queue, streams[0], CL_TRUE, 0, length, output_h, 0, NULL, NULL);
                if (err != CL_SUCCESS)
                {
                    printf("clReadArray failed\n");
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(streams[2]) clReleaseMemObject(streams[2]);
                    if(kernel) clReleaseKernel(kernel);
                    if(program) clReleaseProgram(program);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);
                    return -1;
                }

                *input_A = (cl_double)23.0;
                err = clEnqueueReadBuffer(queue, streams[1], CL_TRUE, 0, length, input_A, 0, NULL, NULL);
                if (err != CL_SUCCESS)
                {
                    printf("clReadArray failed\n");
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(streams[2]) clReleaseMemObject(streams[2]);
                    if(kernel) clReleaseKernel(kernel);
                    if(program) clReleaseProgram(program);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);
                    return -1;
                }

                *input_B = (cl_double)2.3;
                err = clEnqueueReadBuffer(queue, streams[2], CL_TRUE, 0, length, input_B, 0, NULL, NULL);
                if (err != CL_SUCCESS)
                {
                    printf("clReadArray failed\n");
                    if(streams[0]) clReleaseMemObject(streams[0]);
                    if(streams[1]) clReleaseMemObject(streams[1]);
                    if(streams[2]) clReleaseMemObject(streams[2]);
                    if(kernel) clReleaseKernel(kernel);
                    if(program) clReleaseProgram(program);
                    if(input_A) free(input_A);
                    if(input_B) free(input_B);
                    if(output_h) free(output_h);
                    return -1;
                }
            }

            if(streams[0]) clReleaseMemObject(streams[0]);
            if(streams[1]) clReleaseMemObject(streams[1]);
            if(streams[2]) clReleaseMemObject(streams[2]);
            if(kernel) clReleaseKernel(kernel);
            if(program) clReleaseProgram(program);

        }

        printf("%s TESTING FOR ALL FUNCTIONS FINISHED.\n", types[typeIndex]);
        printf("-*****************************************************-\n\n\n");

        typeIndex++;

        if(input_A) free(input_A);
        if(input_B) free(input_B);
        if(output_h) free(output_h);

    }
    //printf("%d / 105 internal tests passed.\n\n", passCount);
    total += 105;
    passed += passCount;
    fail += 105-passCount;

    return 0;
}


