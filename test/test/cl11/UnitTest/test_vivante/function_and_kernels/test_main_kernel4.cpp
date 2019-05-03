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
#include <time.h>
#include <stdlib.h>


int loop_test_main_kernel4 = 0 , test_num_test_main_kernel4 = 24;



static const char format_test_main_kernel4[] = "%s void %s(__global unsigned char *src, __global unsigned char *dst %s)\n"
"{\n"
"   int  tid = get_global_id(0);\n"
"\n"
"   dst[tid] = tid %% (1 << (src[tid] %% (%s)));\n"
"\n"
"}\n"
"\n"
"%s void %s(__global unsigned char *src, __global unsigned char *dst %s)\n"
"{\n"
"\n"
"    %s(src, dst %s);\n"
"\n"
"}\n"
"\n"
"__kernel void %s(__global unsigned char *src, __global unsigned char *dst , __global uint *test1 , __global uint *test2)\n"
"{\n"
"    __private unsigned char n;\n"
"    n= %s;\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    if(test2[tid] == test1[tid]+tid)\n"
"        %s(src, dst %s);\n"
"//test1[tid] = test1[tid]+tid;// + test2[tid];\n"
"\n"
"}\n";

const char* func_types_test_main_kernel4[] = {    "__kernel"    ,    ""            ,
"__kernel"    ,    ""            ,
"__kernel"    ,    ""            ,
""            ,    ""            ,
""            ,    ""            ,
""            ,    ""            ,

""            ,    ""            ,
""            ,    ""            ,
""            ,    ""            ,
"__kernel"    ,    "__kernel"    ,
"__kernel"    ,    "__kernel"    ,
"__kernel"    ,    "__kernel"    ,

""            ,    ""            ,
""            ,    ""            ,
""            ,    ""            ,
"__kernel"    ,    "__kernel"    ,
"__kernel"    ,    "__kernel"    ,
"__kernel"    ,    "__kernel"    ,

""            ,    ""            ,
""            ,    ""            ,
""            ,    ""            ,
"__kernel"    ,    "__kernel"    ,
"__kernel"    ,    "__kernel"    ,
"__kernel"    ,    "__kernel"    };

const char* func_names_test_main_kernel4[] = {    "kernel3_4_2"    ,    "NULLkernel2_4_2"    ,    "kernel4_2"        ,
"kernel3_4_3"    ,    "NULLkernel2_4_3"    ,    "kernel4_3"        ,
"kernel3_4_4"    ,    "NULLkernel2_4_4"    ,    "kernel4_4"        ,
"func3_4_2"        ,    "NULLfunc2_4_2"        ,    "func4_2"        ,
"func3_4_3"        ,    "NULLfunc2_4_3"        ,    "func4_3"        ,
"func3_4_4"        ,    "NULLfunc2_4_4"        ,    "func4_4"        ,

"func3_4_2_2"    ,    "func2_4_2_2"        ,    "func4_2_2"        ,
"func3_4_2_3"    ,    "func2_4_2_3"        ,    "func4_2_3"        ,
"func3_4_2_4"    ,    "func2_4_2_4"        ,    "func4_2_4"        ,
"kernel3_4_2_2"    ,    "kernel2_4_2_2"        ,    "kernel4_2_2"    ,
"kernel3_4_2_3"    ,    "kernel2_4_2_3"        ,    "kernel4_2_3"    ,
"kernel3_4_2_4"    ,    "kernel2_4_2_4"        ,    "kernel4_2_4"    ,

"func3_4_3_2"    ,    "func2_4_3_2"        ,    "func4_3_2"        ,
"func3_4_3_3"    ,    "func2_4_3_3"        ,    "func4_3_3"        ,
"func3_4_3_4"    ,    "func2_4_3_4"        ,    "func4_3_4"        ,
"kernel3_4_3_2"    ,    "kernel2_4_3_2"        ,    "kernel4_3_2"    ,
"kernel3_4_3_3"    ,    "kernel2_4_3_3"        ,    "kernel4_3_3"    ,
"kernel3_4_3_4"    ,    "kernel2_4_3_4"        ,    "kernel4_3_4"    ,

"func3_4_4_2"    ,    "func2_4_4_2"        ,    "func4_4_2"        ,
"func3_4_4_3"    ,    "func2_4_4_3"        ,    "func4_4_3"        ,
"func3_4_4_4"    ,    "func2_4_4_4"        ,    "func4_4_4"        ,
"kernel3_4_4_2"    ,    "kernel2_4_4_2"        ,    "kernel4_4_2"    ,
"kernel3_4_4_3"    ,    "kernel2_4_4_3"        ,    "kernel4_4_3"    ,
"kernel3_4_4_4"    ,    "kernel2_4_4_4"        ,    "kernel4_4_4"    };

const char* func_arguments_test_main_kernel4[] = {    ""                        ,    ""                        ,
", unsigned char n"                ,    ""                        ,
", unsigned char n , unsigned char m"    ,    ""                        ,
""                        ,    ""                        ,
", unsigned char n"                ,    ""                        ,
", unsigned char n , unsigned char m"    ,    ""                        ,

""                        ,    ""                        ,
", unsigned char n"                ,    ""                        ,
", unsigned char n , unsigned char m"    ,    ""                        ,
""                        ,    ""                        ,
", unsigned char n"                ,    ""                        ,
", unsigned char n , unsigned char m"    ,    ""                        ,

""                        ,    ", unsigned char n"                ,
", unsigned char n"                ,    ", unsigned char n"                ,
", unsigned char n , unsigned char m"    ,    ", unsigned char n"                ,
""                        ,    ", unsigned char n"                ,
", unsigned char n"                ,    ", unsigned char n"                ,
", unsigned char n , unsigned char m"    ,    ", unsigned char n"                ,

""                        ,    ", unsigned char n , unsigned char m"    ,
", unsigned char n"                ,    ", unsigned char n , unsigned char m"    ,
", unsigned char n , unsigned char m"    ,    ", unsigned char n , unsigned char m"    ,
""                        ,    ", unsigned char n , unsigned char m"    ,
", unsigned char n"                ,    ", unsigned char n , unsigned char m"    ,
", unsigned char n , unsigned char m"    ,    ", unsigned char n , unsigned char m"    };
const char* work_test_main_kernel4[] = {    "8"        ,
"n"        ,
"n*m"    ,
"8"        ,
"n"        ,
"n*m"    ,

"8"        ,
"n"        ,
"n*m"    ,
"8"        ,
"n"        ,
"n*m"    ,

"8"        ,
"n"        ,
"n*m"    ,
"8"        ,
"n"        ,
"n*m"    ,

"8"        ,
"n"        ,
"n*m"    ,
"8"        ,
"n"        ,
"n*m"    };

const char* call_names_test_main_kernel4[] = {    "kernel3_4_2"                    ,    "kernel3_4_2"    ,
"kernel3_4_3"                    ,    "kernel3_4_3"    ,
"kernel3_4_4"                    ,    "kernel3_4_4"    ,
"func3_4_2"                        ,    "func3_4_2"        ,
"func3_4_3"                        ,    "func3_4_3"        ,
"func3_4_4"                        ,    "func3_4_4"        ,

"func3_4_2_2"                    ,    "func2_4_2_2"    ,
"func3_4_2_3"                    ,    "func2_4_2_3"    ,
"func3_4_2_4"                    ,    "func2_4_2_4"    ,
"kernel3_4_2_2"                    ,    "kernel2_4_2_2"    ,
"kernel3_4_2_3"                    ,    "kernel2_4_2_3"    ,
"kernel3_4_2_4"                    ,    "kernel2_4_2_4"    ,

"if(n==5)\n\t\tfunc3_4_3_2"        ,    "func2_4_3_2"    ,
"func3_4_3_3"                    ,    "func2_4_3_3"    ,
"func3_4_3_4"                    ,    "func2_4_3_4"    ,
"if(n==5)\n\t\tkernel3_4_3_2"    ,    "kernel2_4_3_2"    ,
"kernel3_4_3_3"                    ,    "kernel2_4_3_3"    ,
"kernel3_4_3_4"                    ,    "kernel2_4_3_4"    ,

"if(m*n==5)\n\t\tfunc3_4_4_2"    ,    "func2_4_4_2"    ,
"if(m==5)\n\t\tfunc3_4_4_3"        ,    "func2_4_4_3"    ,
"func3_4_4_4"                    ,    "func2_4_4_4"    ,
"if(m*n==5)\n\t\tkernel3_4_4_2"    ,    "kernel2_4_4_2"    ,
"if(m==5)\n\t\tkernel3_4_4_3"    ,    "kernel2_4_4_3"    ,
"kernel3_4_4_4"                    ,    "kernel2_4_4_4"    };

const char* definition_test_main_kernel4[] = {    "0"    ,
"8"    ,
"4"    ,
"0"    ,
"8"    ,
"4"    ,

"5"    ,
"4"    ,
"4"    ,
"5"    ,
"4"    ,
"4"    ,

"5"    ,
"4"    ,
"4"    ,
"5"    ,
"4"    ,
"4"    ,

"1"    ,
"4"    ,
"4"    ,
"1"    ,
"4"    ,
"4"    };

const char* call_arguments_test_main_kernel4[] = {    ""                        ,    ""                ,
",0"                    ,    ",n"            ,
",0,0"                    ,    ",n, (unsigned char)2"    ,
""                        ,    ""                ,
",0"                    ,    ",n"            ,
",0,0"                    ,    ",n, (unsigned char)2"    ,

""                        ,    ""                ,
",(unsigned char)8"                ,    ""                ,
",(unsigned char)4, (unsigned char)2"    ,    ""                ,
""                        ,    ""                ,
",(unsigned char)8"                ,    ""                ,
",(unsigned char)4, (unsigned char)2"    ,    ""                ,

""                        ,    ",n"            ,
",n*2"                    ,    ",n"            ,
",n, (unsigned char)2"            ,    ",n"            ,
""                        ,    ",n"            ,
",n*2"                    ,    ",n"            ,
",n, (unsigned char)2"            ,    ",n"            ,

""                        ,    ",n,5"            ,
",n*2"                    ,    ",n,5"            ,
",n, m"                    ,    ",n,(unsigned char)2"    ,
""                        ,    ",n,5"            ,
",n*2"                    ,    ",n,5"            ,
",n, m"                    ,    ",n,(unsigned char)2"    };


int
verify_test_main_kernel4(unsigned char *inptr, unsigned char *outptr, int n)
{
    int i;


    for(i=0 ; i<n; i++){

        if( outptr[i] != i % (1 << (inptr[i] % 8)))
            return -1;

    }

    return 0;
}

int test_main_kernel4(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail)
{
    total += test_num_test_main_kernel4;
    cl_mem streams[4];
    unsigned char *input_h, *output_h;
    uint *test1_h, *test2_h;
    cl_program program;
    cl_kernel kernel;
    size_t threads[1];
    int err, i;
    const char *ptr;
    size_t length = sizeof(unsigned char) * num_elements;
    int test_length = sizeof(uint) * num_elements;

    char* prg = (char*)malloc(sizeof(char)*4096);
    ptr = (char*)malloc(sizeof(char)*4096);
    srand((unsigned int)time(NULL));

    for( ; loop_test_main_kernel4<test_num_test_main_kernel4 ; loop_test_main_kernel4++){

        sprintf((char*)ptr ,format_test_main_kernel4 ,func_types_test_main_kernel4[loop_test_main_kernel4*2+0], func_names_test_main_kernel4[loop_test_main_kernel4*3+0], func_arguments_test_main_kernel4[loop_test_main_kernel4*2+0], work_test_main_kernel4[loop_test_main_kernel4], func_types_test_main_kernel4[loop_test_main_kernel4*2+1], func_names_test_main_kernel4[loop_test_main_kernel4*3+1], func_arguments_test_main_kernel4[loop_test_main_kernel4*2+1], call_names_test_main_kernel4[loop_test_main_kernel4*2+0], call_arguments_test_main_kernel4[loop_test_main_kernel4*2+0], func_names_test_main_kernel4[loop_test_main_kernel4*3+2], definition_test_main_kernel4[loop_test_main_kernel4], call_names_test_main_kernel4[loop_test_main_kernel4*2+1], call_arguments_test_main_kernel4[loop_test_main_kernel4*2+1]);

        input_h  = (unsigned char*)malloc(length);
        output_h = (unsigned char*)malloc(length);
        test1_h = (uint*)malloc(test_length);
        test2_h = (uint*)malloc(test_length);

        streams[0] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
        if (!streams[0])
        {
            printf("clCreateBuffer failed\n");
            fail++;
            return -1;
        }
        streams[1] = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
        if (!streams[1])
        {
            printf("clCreateBuffer failed\n");
            fail++;
            return -1;
        }
        streams[2] = clCreateBuffer(context, CL_MEM_READ_WRITE, test_length, NULL, NULL);
        if (!streams[2])
        {
            printf("clCreateBuffer failed\n");
            fail++;
            return -1;
        }
        streams[3] = clCreateBuffer(context, CL_MEM_READ_WRITE, test_length, NULL, NULL);
        if (!streams[3])
        {
            printf("clCreateBuffer failed\n");
            fail++;
            return -1;
        }
        for (i=0; i<num_elements; i++){

            input_h[i] = ((unsigned int)rand())%256;

        }

        for(i=0 ; i<num_elements ; i++){

            test1_h[i] = num_elements-i;
            test2_h[i] = num_elements;

        }

        err = clEnqueueWriteBuffer(queue, streams[0], CL_TRUE, 0, length, input_h, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            printf("clEnqueueWriteBuffer failed\n");
            fail++;
            return -1;
        }

        err = clEnqueueWriteBuffer(queue, streams[2], CL_TRUE, 0, test_length, test1_h, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            printf("clEnqueueWriteBuffer failed\n");
            fail++;
            return -1;
        }

        err = clEnqueueWriteBuffer(queue, streams[3], CL_TRUE, 0, test_length, test2_h, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            printf("clEnqueueWriteBuffer failed\n");
            fail++;
            return -1;
        }

        err = create_kernel(context, &program, &kernel, 1, &ptr, func_names_test_main_kernel4[loop_test_main_kernel4*3+2] );
        if (err)
            return -1;

        /*for(i=0 ; i<test_length ; i++)
        printf(" %d", test2_h[i]);*/

        err  = clSetKernelArg(kernel, 0, sizeof streams[0], &streams[0]);
        err |= clSetKernelArg(kernel, 1, sizeof streams[1], &streams[1]);
        err |= clSetKernelArg(kernel, 2, sizeof streams[2], &streams[2]);
        err |= clSetKernelArg(kernel, 3, sizeof streams[3], &streams[3]);
        if (err != CL_SUCCESS)
        {
            printf("clSetKernelArgs failed\n");
            fail++;
            return -1;
        }

        threads[0] = (unsigned int)num_elements;
        err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, threads, NULL, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            printf("clEnqueueNDRangeKernel failed\n");
            fail++;
            return -1;
        }

        err = clEnqueueReadBuffer(queue, streams[1], CL_TRUE, 0, length, output_h, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
            printf("clReadArray failed\n");
            fail++;
            return -1;
        }

        /*err = clEnqueueReadBuffer(queue, streams[2], CL_TRUE, 0, test_length, test2_h, 0, NULL, NULL);
        if (err != CL_SUCCESS)
        {
        printf("clReadArray failed\n");
        return -1;
        }
        for(i=0 ; i<num_elements ; i++)
        printf(" %d", test2_h[i]);*/

        err = verify_test_main_kernel4(input_h, output_h, num_elements);
        if(err != 0)
        {
            fail++;
        }
        passed++;

        printf("   %s case is %s.\n",func_names_test_main_kernel4[loop_test_main_kernel4*3+2], (err) ? "failed" : "successful");

        // cleanup
        clReleaseMemObject(streams[0]);
        clReleaseMemObject(streams[1]);
        clReleaseMemObject(streams[2]);
        clReleaseMemObject(streams[3]);
        clReleaseKernel(kernel);
        clReleaseProgram(program);
        free(input_h);
        free(output_h);
        free(test1_h);
        free(test2_h);
    }

    return err;
}


