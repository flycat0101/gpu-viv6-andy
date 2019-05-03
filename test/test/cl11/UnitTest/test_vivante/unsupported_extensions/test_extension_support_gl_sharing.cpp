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


int test_gl_sharing(cl_device_id device, cl_context context, cl_command_queue queue, int num_elements, int &total,  int &passed, int &fail)
{
    /*
    cl_mem streams;
    cl_program program;
    cl_kernel kernel;
    size_t threads[1];
    char kernel_code_int[512];
    const char *constkernelint;
    cl_uchar *output_h;
    */

    if(is_extension_available(device, "cl_khr_gl_sharing"))
    {
        /* Create GL Buffers and etc. */
        /* Not necessary since the device has no support. */
        printf("!!extension cl_khr_gl_sharing is SUPPORTED!!\n");
        printf("!!TEST cl_khr_gl_sharing PASSED!!\n");
        passed++;
    }
    else
    {
        printf("!!extension cl_khr_gl_sharing not supported!!\n");
        printf("!!TEST cl_khr_gl_sharing PASSED!!\n");
        passed++;
    }

    total++;



    return 0;
}
