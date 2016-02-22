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


#include <iostream>
#include <CL/cl.h>
#include "tests.h"

#include "rounding_mode.h"
#pragma warning( disable : 4290 )

int main(int argc, const char *argv[]) {

    cl_platform_id platform = 0;
    cl_device_id device = 0;
	cl_int error;
	error = clGetPlatformIDs(1, &platform, NULL);
	if (error != CL_SUCCESS) {
		return 0;
	}

	error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (error != CL_SUCCESS) {
		return 0;
	}

    cl_device_fp_config fpconfig = 0;
    clGetDeviceInfo( device, CL_DEVICE_SINGLE_FP_CONFIG, sizeof( fpconfig ), &fpconfig, NULL );

    if (!(fpconfig & CL_FP_ROUND_TO_NEAREST) )
    {
	    set_round(kRoundTowardZero, kfloat);
    }

    if (!(fpconfig & CL_FP_DENORM))
    {
	    FlushToZero();
    }


	int cnt = 0;
	cnt = cnt + const_global();
	cnt = cnt + volatile_global();
	cnt = cnt + restrict_global();

	cnt = cnt + const_constant();
	cnt = cnt + volatile_constant();
	cnt = cnt + restrict_constant();

	cnt = cnt + const_private();
	cnt = cnt + restrict_private();
	cnt = cnt + volatile_private();

	cnt = cnt + const_local();
	cnt = cnt + volatile_local();
	cnt = cnt + restrict_local();

	cnt = cnt + const_global_enum();
	cnt = cnt + restrict_global_enum();
	cnt = cnt + volatile_global_enum();

	cnt = cnt + const_private_enum();
	cnt = cnt + restrict_private_enum();
	cnt = cnt + volatile_private_enum();

	cnt = cnt + const_constant_enum();
	cnt = cnt + restrict_constant_enum();
	cnt = cnt + volatile_constant_enum();

	cnt = cnt + const_local_enum();
	cnt = cnt + restrict_local_enum();
	cnt = cnt + volatile_local_enum();

	int passed_cnt = cnt;
	int failed_cnt = 24-cnt;
	std::cout << "TOTAL : " << passed_cnt+failed_cnt << std::endl;
	std::cout << "PASSED: " << passed_cnt << std::endl;
	std::cout << "FAILED: " << failed_cnt<< std::endl;
}
