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

const char *kernel_test_fp16 =  "#pragma OPENCL EXTENSION cl_khr_fp16 : enable\n"
"__kernel void test_fp16(__global uchar%s *out_val)\n"
"{\n"
"int  gid = get_global_id(0);\n"
"half%s var = (half%s)(5);\n"
"out_val[gid] = convert_uchar%s(var);\n"
"}\n";


int test_fp16(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail)
{
   cl_mem streams;
   cl_program program;
   cl_kernel kernel;
   size_t threads[1];
   char kernel_code_int[512];
   const char *constkernelint;
   cl_uchar *output_h;

   int err;
   int passCount = 0;
   int typeIndex = 0;

   const char    *types[] = {
       "", "2", "4", "8", "16"
   };


   for(int t=1; t<17; t*=2)
   {
       printf("\n\nTESTING HALF FLOAT%s: \n", types[typeIndex]);

       size_t length = sizeof(cl_uchar) * num_elements * t;
       output_h = (cl_uchar*)malloc(length);

       streams = clCreateBuffer(context, CL_MEM_READ_WRITE, length, NULL, NULL);
       if (!streams)
       {
           printf("clCreateBuffer failed\n");
           free(output_h);
           return -1;
       }

       sprintf(kernel_code_int, kernel_test_fp16, types[typeIndex], types[typeIndex], types[typeIndex], types[typeIndex]);
       constkernelint = kernel_code_int;
       if(is_extension_available(device, "cl_khr_fp16"))
       {
           err = create_kernel(context, &program, &kernel, 1, &constkernelint, "test_fp16" );
           if (err)
           {
               printf("Kernel compilation error using half float as a type.\n");
               printf("\n!!Kernel build not successful.\n");
               if(streams) clReleaseMemObject(streams);
               if(kernel) clReleaseKernel(kernel);
               if(program) clReleaseProgram(program);
               if(output_h) free(output_h);
               return -1;
           }
       }
       else
       {
           if(streams) clReleaseMemObject(streams);
           if(kernel) clReleaseKernel(kernel);
           if(program) clReleaseProgram(program);
           if(output_h) free(output_h);

           passCount++;

           printf("--------------------------------------------------------\n");
           printf("cl_khr_fp16 extension is not supported.!!\n");
           printf("half%s passed\n", types[typeIndex]);
           return 0;
       }
       err = clSetKernelArg(kernel, 0, sizeof streams, &streams);
       if (err != CL_SUCCESS)
       {
           printf("clSetKernelArgs failed\n");
           if(streams) clReleaseMemObject(streams);
           if(kernel) clReleaseKernel(kernel);
           if(program) clReleaseProgram(program);
           if(output_h) free(output_h);
           return -1;
       }
       threads[0] = (unsigned int)num_elements;
       err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, threads, NULL, 0, NULL, NULL);
       if (err != CL_SUCCESS)
       {
           printf("clEnqueueNDRangeKernel failed\n");
           if(streams) clReleaseMemObject(streams);
           if(kernel) clReleaseKernel(kernel);
           if(program) clReleaseProgram(program);
           if(output_h) free(output_h);
           return -1;
       }
       err = clEnqueueReadBuffer(queue, streams, CL_TRUE, 0, length, output_h, 0, NULL, NULL);
       if (err != CL_SUCCESS)
       {
           printf("clReadArray failed\n");
           if(streams) clReleaseMemObject(streams);
           if(kernel) clReleaseKernel(kernel);
           if(program) clReleaseProgram(program);
           if(output_h) free(output_h);
           return -1;
       }

       if(streams) clReleaseMemObject(streams);
       if(kernel) clReleaseKernel(kernel);
       if(program) clReleaseProgram(program);
       if(output_h) free(output_h);

       typeIndex++;
       passCount++;

   }

   printf("--------------------------------------------------------\n");
   printf("cl_khr_fp16 tests:\n");
   //printf("%d / %d internal tests passed.\n",passCount, typeIndex);
   total += typeIndex;
   passed += passCount;
   fail += typeIndex-passCount;

   return 0;
}
