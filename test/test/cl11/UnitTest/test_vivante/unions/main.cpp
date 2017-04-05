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


#include <iostream>
#include <CL/cl.h>
#include "tests.h"

#include "rounding_mode.h"

int main(int argc, const char *argv[]) {
    int cnt = 0;

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

    cnt = cnt + add_union_arrays_1();
    cnt = cnt + add_union_arrays_2();
    cnt = cnt + add_union_arrays_3();
    cnt = cnt + add_union_arrays_4();
    cnt = cnt + add_union_arrays_5();
    cnt = cnt + add_union_arrays_6();
    cnt = cnt + add_union_scalar_1();
    cnt = cnt + add_union_scalar_2();
    cnt = cnt + add_union_scalar_3();
    cnt = cnt + add_union_scalar_4();
    cnt = cnt + add_union_scalar_5();
    cnt = cnt + add_union_scalar_6();
    cnt = cnt + add_union_vector16_1();
    cnt = cnt + add_union_vector16_2();
    cnt = cnt + add_union_vector16_3();
    cnt = cnt + add_union_vector16_4();
    cnt = cnt + add_union_vector16_5();
    cnt = cnt + add_union_vector16_6();
    cnt = cnt + add_union_vector2_1();
    cnt = cnt + add_union_vector2_2();
    cnt = cnt + add_union_vector2_3();
    cnt = cnt + add_union_vector2_4();
    cnt = cnt + add_union_vector2_5();
    cnt = cnt + add_union_vector2_6();
    cnt = cnt + add_union_vector3_1();
    cnt = cnt + add_union_vector3_2();
    cnt = cnt + add_union_vector3_3();
    cnt = cnt + add_union_vector3_4();
    cnt = cnt + add_union_vector3_5();
    cnt = cnt + add_union_vector3_6();
    cnt = cnt + add_union_vector4_1();
    cnt = cnt + add_union_vector4_2();
    cnt = cnt + add_union_vector4_3();
    cnt = cnt + add_union_vector4_4();
    cnt = cnt + add_union_vector4_5();
    cnt = cnt + add_union_vector4_6();
    cnt = cnt + add_union_vector8_1();
    cnt = cnt + add_union_vector8_2();
    cnt = cnt + add_union_vector8_3();
    cnt = cnt + add_union_vector8_4();
    cnt = cnt + add_union_vector8_5();
    cnt = cnt + add_union_vector8_6();

    cnt = cnt + copy_union_arrays_1();
    cnt = cnt + copy_union_arrays_2();
    cnt = cnt + copy_union_arrays_3();
    cnt = cnt + copy_union_arrays_4();
    cnt = cnt + copy_union_arrays_5();
    cnt = cnt + copy_union_arrays_6();
    cnt = cnt + copy_union_scalar_1();
    cnt = cnt + copy_union_scalar_2();
    cnt = cnt + copy_union_scalar_3();
    cnt = cnt + copy_union_scalar_4();
    cnt = cnt + copy_union_scalar_5();
    cnt = cnt + copy_union_scalar_6();
    cnt = cnt + copy_union_vector16_1();
    cnt = cnt + copy_union_vector16_2();
    cnt = cnt + copy_union_vector16_3();
    cnt = cnt + copy_union_vector16_4();
    cnt = cnt + copy_union_vector16_5();
    cnt = cnt + copy_union_vector16_6();
    cnt = cnt + copy_union_vector2_1();
    cnt = cnt + copy_union_vector2_2();
    cnt = cnt + copy_union_vector2_3();
    cnt = cnt + copy_union_vector2_4();
    cnt = cnt + copy_union_vector2_5();
    cnt = cnt + copy_union_vector2_6();
    cnt = cnt + copy_union_vector3_1();
    cnt = cnt + copy_union_vector3_2();
    cnt = cnt + copy_union_vector3_3();
    cnt = cnt + copy_union_vector3_4();
    cnt = cnt + copy_union_vector3_5();
    cnt = cnt + copy_union_vector3_6();
    cnt = cnt + copy_union_vector4_1();
    cnt = cnt + copy_union_vector4_2();
    cnt = cnt + copy_union_vector4_3();
    cnt = cnt + copy_union_vector4_4();
    cnt = cnt + copy_union_vector4_5();
    cnt = cnt + copy_union_vector4_6();
    cnt = cnt + copy_union_vector8_1();
    cnt = cnt + copy_union_vector8_2();
    cnt = cnt + copy_union_vector8_3();
    cnt = cnt + copy_union_vector8_4();
    cnt = cnt + copy_union_vector8_5();
    cnt = cnt + copy_union_vector8_6();

    cnt = cnt + floor_union_arrays_1();
    cnt = cnt + floor_union_arrays_2();
    cnt = cnt + floor_union_arrays_3();
    cnt = cnt + floor_union_arrays_4();
    cnt = cnt + floor_union_arrays_5();
    cnt = cnt + floor_union_arrays_6();
    cnt = cnt + floor_union_scalar_1();
    cnt = cnt + floor_union_scalar_2();
    cnt = cnt + floor_union_scalar_3();
    cnt = cnt + floor_union_scalar_4();
    cnt = cnt + floor_union_scalar_5();
    cnt = cnt + floor_union_scalar_6();
    cnt = cnt + floor_union_vector16_1();
    cnt = cnt + floor_union_vector16_2();
    cnt = cnt + floor_union_vector16_3();
    cnt = cnt + floor_union_vector16_4();
    cnt = cnt + floor_union_vector16_5();
    cnt = cnt + floor_union_vector16_6();
    cnt = cnt + floor_union_vector2_1();
    cnt = cnt + floor_union_vector2_2();
    cnt = cnt + floor_union_vector2_3();
    cnt = cnt + floor_union_vector2_4();
    cnt = cnt + floor_union_vector2_5();
    cnt = cnt + floor_union_vector2_6();
    cnt = cnt + floor_union_vector3_1();
    cnt = cnt + floor_union_vector3_2();
    cnt = cnt + floor_union_vector3_3();
    cnt = cnt + floor_union_vector3_4();
    cnt = cnt + floor_union_vector3_5();
    cnt = cnt + floor_union_vector3_6();
    cnt = cnt + floor_union_vector4_1();
    cnt = cnt + floor_union_vector4_2();
    cnt = cnt + floor_union_vector4_3();
    cnt = cnt + floor_union_vector4_4();
    cnt = cnt + floor_union_vector4_5();
    cnt = cnt + floor_union_vector4_6();
    cnt = cnt + floor_union_vector8_1();
    cnt = cnt + floor_union_vector8_2();
    cnt = cnt + floor_union_vector8_3();
    cnt = cnt + floor_union_vector8_4();
    cnt = cnt + floor_union_vector8_5();
    cnt = cnt + floor_union_vector8_6();

    cnt = cnt + multiply_union_arrays_1();
    cnt = cnt + multiply_union_arrays_2();
    cnt = cnt + multiply_union_arrays_3();
    cnt = cnt + multiply_union_arrays_4();
    cnt = cnt + multiply_union_arrays_5();
    cnt = cnt + multiply_union_arrays_6();
    cnt = cnt + multiply_union_scalar_1();
    cnt = cnt + multiply_union_scalar_2();
    cnt = cnt + multiply_union_scalar_3();
    cnt = cnt + multiply_union_scalar_4();
    cnt = cnt + multiply_union_scalar_5();
    cnt = cnt + multiply_union_scalar_6();
    cnt = cnt + multiply_union_vector16_1();
    cnt = cnt + multiply_union_vector16_2();
    cnt = cnt + multiply_union_vector16_3();
    cnt = cnt + multiply_union_vector16_4();
    cnt = cnt + multiply_union_vector16_5();
    cnt = cnt + multiply_union_vector16_6();
    cnt = cnt + multiply_union_vector2_1();
    cnt = cnt + multiply_union_vector2_2();
    cnt = cnt + multiply_union_vector2_3();
    cnt = cnt + multiply_union_vector2_4();
    cnt = cnt + multiply_union_vector2_5();
    cnt = cnt + multiply_union_vector2_6();
    cnt = cnt + multiply_union_vector3_1();
    cnt = cnt + multiply_union_vector3_2();
    cnt = cnt + multiply_union_vector3_3();
    cnt = cnt + multiply_union_vector3_4();
    cnt = cnt + multiply_union_vector3_5();
    cnt = cnt + multiply_union_vector3_6();
    cnt = cnt + multiply_union_vector4_1();
    cnt = cnt + multiply_union_vector4_2();
    cnt = cnt + multiply_union_vector4_3();
    cnt = cnt + multiply_union_vector4_4();
    cnt = cnt + multiply_union_vector4_5();
    cnt = cnt + multiply_union_vector4_6();
    cnt = cnt + multiply_union_vector8_1();
    cnt = cnt + multiply_union_vector8_2();
    cnt = cnt + multiply_union_vector8_3();
    cnt = cnt + multiply_union_vector8_4();
    cnt = cnt + multiply_union_vector8_5();
    cnt = cnt + multiply_union_vector8_6();

    cnt = cnt + power_union_arrays_1();
    cnt = cnt + power_union_arrays_2();
    cnt = cnt + power_union_arrays_3();
    cnt = cnt + power_union_arrays_4();
    cnt = cnt + power_union_arrays_5();
    cnt = cnt + power_union_arrays_6();
    cnt = cnt + power_union_scalar_1();
    cnt = cnt + power_union_scalar_2();
    cnt = cnt + power_union_scalar_3();
    cnt = cnt + power_union_scalar_4();
    cnt = cnt + power_union_scalar_5();
    cnt = cnt + power_union_scalar_6();
    cnt = cnt + power_union_vector16_1();
    cnt = cnt + power_union_vector16_2();
    cnt = cnt + power_union_vector16_3();
    cnt = cnt + power_union_vector16_4();
    cnt = cnt + power_union_vector16_5();
    cnt = cnt + power_union_vector16_6();
    cnt = cnt + power_union_vector2_1();
    cnt = cnt + power_union_vector2_2();
    cnt = cnt + power_union_vector2_3();
    cnt = cnt + power_union_vector2_4();
    cnt = cnt + power_union_vector2_5();
    cnt = cnt + power_union_vector2_6();
    cnt = cnt + power_union_vector3_1();
    cnt = cnt + power_union_vector3_2();
    cnt = cnt + power_union_vector3_3();
    cnt = cnt + power_union_vector3_4();
    cnt = cnt + power_union_vector3_5();
    cnt = cnt + power_union_vector3_6();
    cnt = cnt + power_union_vector4_1();
    cnt = cnt + power_union_vector4_2();
    cnt = cnt + power_union_vector4_3();
    cnt = cnt + power_union_vector4_4();
    cnt = cnt + power_union_vector4_5();
    cnt = cnt + power_union_vector4_6();
    cnt = cnt + power_union_vector8_1();
    cnt = cnt + power_union_vector8_2();
    cnt = cnt + power_union_vector8_3();
    cnt = cnt + power_union_vector8_4();
    cnt = cnt + power_union_vector8_5();
    cnt = cnt + power_union_vector8_6();

    int passed_cnt = cnt;
    int failed_cnt = 210-cnt;
    std::cout << "TOTAL : " << passed_cnt+failed_cnt << std::endl;
    std::cout << "PASSED: " << passed_cnt << std::endl;
    std::cout << "FAILED: " << failed_cnt << std::endl;
}
