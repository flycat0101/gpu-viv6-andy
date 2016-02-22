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


#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#include "compat.h"

#define ISSUBNORM(f)  (fabs(f) < FLT_MIN)

int multiply_union_arrays_1(void);
int add_union_arrays_1(void);
int copy_union_arrays_1(void);
int power_union_arrays_1(void);
int floor_union_arrays_1(void);
int multiply_union_scalar_1(void);
int multiply_union_scalar_2(void);
int add_union_scalar_1(void);
int copy_union_scalar_1(void);
int copy_union_scalar_2(void);
int power_union_scalar_1(void);
int power_union_scalar_2(void);
int floor_union_scalar_1(void);
int floor_union_scalar_2(void);
int add_union_vector2_1(void);
int multiply_union_vector2_1(void);
int copy_union_vector2_1(void);
int add_union_scalar_2(void);
int floor_union_vector2_1(void);
int power_union_vector2_1(void);
int multiply_union_vector3_1(void);
int add_union_vector3_1(void);
int copy_union_vector3_1(void);
int floor_union_vector3_1(void);
int power_union_vector3_1(void);
int multiply_union_vector4_1(void);
int add_union_vector4_1(void);
int copy_union_vector4_1(void);
int floor_union_vector4_1(void);
int power_union_vector4_1(void);
int multiply_union_vector8_1(void);
int add_union_vector8_1(void);
int copy_union_vector8_1(void);
int floor_union_vector8_1(void);
int power_union_vector8_1(void);
int multiply_union_vector16_1(void);
int add_union_vector16_1(void);
int copy_union_vector16_1(void);
int floor_union_vector16_1(void);
int power_union_vector16_1(void);
///////////////////////////////////
int multiply_union_arrays_2(void);
int add_union_arrays_2(void);
int copy_union_arrays_2(void);
int power_union_arrays_2(void);
int floor_union_arrays_2(void);
//int multiply_union_scalar_1(void);
//int multiply_union_scalar_2(void);
//int add_union_scalar_1(void);
//int copy_union_scalar_1(void);
//int copy_union_scalar_2(void);
//int power_union_scalar_1(void);
//int power_union_scalar_2(void);
//int floor_union_scalar_1(void);
//int floor_union_scalar_2(void);
int add_union_vector2_2(void);
int multiply_union_vector2_2(void);
int copy_union_vector2_2(void);
//int add_union_scalar_2(void);
int floor_union_vector2_2(void);
int power_union_vector2_2(void);

int multiply_union_vector3_2(void);
int add_union_vector3_2(void);
int copy_union_vector3_2(void);
int floor_union_vector3_2(void);
int power_union_vector3_2(void);

int multiply_union_vector4_2(void);
int add_union_vector4_2(void);
int copy_union_vector4_2(void);
int floor_union_vector4_2(void);
int power_union_vector4_2(void);

int multiply_union_vector8_2(void);
int add_union_vector8_2(void);
int copy_union_vector8_2(void);
int floor_union_vector8_2(void);
int power_union_vector8_2(void);

int multiply_union_vector16_2(void);
int add_union_vector16_2(void);
int copy_union_vector16_2(void);
int floor_union_vector16_2(void);
int power_union_vector16_2(void);

/////////////////////////////////////

int multiply_union_arrays_3(void);
int add_union_arrays_3(void);
int copy_union_arrays_3(void);
int power_union_arrays_3(void);
int floor_union_arrays_3(void);
int multiply_union_scalar_3(void);
int add_union_scalar_3(void);
int copy_union_scalar_3(void);
int power_union_scalar_3(void);
int floor_union_scalar_3(void);
int add_union_vector2_3(void);
int multiply_union_vector2_3(void);
int copy_union_vector2_3(void);
//int add_union_scalar_2(void);
int floor_union_vector2_3(void);
int power_union_vector2_3(void);

int multiply_union_vector3_3(void);
int add_union_vector3_3(void);
int copy_union_vector3_3(void);
int floor_union_vector3_3(void);
int power_union_vector3_3(void);

int multiply_union_vector4_3(void);
int add_union_vector4_3(void);
int copy_union_vector4_3(void);
int floor_union_vector4_3(void);
int power_union_vector4_3(void);

int multiply_union_vector8_3(void);
int add_union_vector8_3(void);
int copy_union_vector8_3(void);
int floor_union_vector8_3(void);
int power_union_vector8_3(void);

int multiply_union_vector16_3(void);
int add_union_vector16_3(void);
int copy_union_vector16_3(void);
int floor_union_vector16_3(void);
int power_union_vector16_3(void);

/////////////////////////////////////

int multiply_union_arrays_4(void);
int add_union_arrays_4(void);
int copy_union_arrays_4(void);
int power_union_arrays_4(void);
int floor_union_arrays_4(void);
int multiply_union_scalar_4(void);
int add_union_scalar_4(void);
int copy_union_scalar_4(void);
int power_union_scalar_4(void);
int floor_union_scalar_4(void);
int add_union_vector2_4(void);
int multiply_union_vector2_4(void);
int copy_union_vector2_4(void);
//int add_union_scalar_2(void);
int floor_union_vector2_4(void);
int power_union_vector2_4(void);

int multiply_union_vector3_4(void);
int add_union_vector3_4(void);
int copy_union_vector3_4(void);
int floor_union_vector3_4(void);
int power_union_vector3_4(void);

int multiply_union_vector4_4(void);
int add_union_vector4_4(void);
int copy_union_vector4_4(void);
int floor_union_vector4_4(void);
int power_union_vector4_4(void);

int multiply_union_vector8_4(void);
int add_union_vector8_4(void);
int copy_union_vector8_4(void);
int floor_union_vector8_4(void);
int power_union_vector8_4(void);

int multiply_union_vector16_4(void);
int add_union_vector16_4(void);
int copy_union_vector16_4(void);
int floor_union_vector16_4(void);
int power_union_vector16_4(void);
/////////////////////////////////////

int multiply_union_arrays_5(void);
int add_union_arrays_5(void);
int copy_union_arrays_5(void);
int power_union_arrays_5(void);
int floor_union_arrays_5(void);
int multiply_union_scalar_5(void);
int add_union_scalar_5(void);
int copy_union_scalar_5(void);
int power_union_scalar_5(void);
int floor_union_scalar_5(void);
int add_union_vector2_5(void);
int multiply_union_vector2_5(void);
int copy_union_vector2_5(void);
//int add_union_scalar_2(void);
int floor_union_vector2_5(void);
int power_union_vector2_5(void);

int multiply_union_vector3_5(void);
int add_union_vector3_5(void);
int copy_union_vector3_5(void);
int floor_union_vector3_5(void);
int power_union_vector3_5(void);

int multiply_union_vector4_5(void);
int add_union_vector4_5(void);
int copy_union_vector4_5(void);
int floor_union_vector4_5(void);
int power_union_vector4_5(void);

int multiply_union_vector8_5(void);
int add_union_vector8_5(void);
int copy_union_vector8_5(void);
int floor_union_vector8_5(void);
int power_union_vector8_5(void);

int multiply_union_vector16_5(void);
int add_union_vector16_5(void);
int copy_union_vector16_5(void);
int floor_union_vector16_5(void);
int power_union_vector16_5(void);

/////////////////////////////////////

int multiply_union_arrays_6(void);
int add_union_arrays_6(void);
int copy_union_arrays_6(void);
int power_union_arrays_6(void);
int floor_union_arrays_6(void);
int multiply_union_scalar_6(void);
int add_union_scalar_6(void);
int copy_union_scalar_6(void);
int power_union_scalar_6(void);
int floor_union_scalar_6(void);
int add_union_vector2_6(void);
int multiply_union_vector2_6(void);
int copy_union_vector2_6(void);
//int add_union_scalar_2(void);
int floor_union_vector2_6(void);
int power_union_vector2_6(void);

int multiply_union_vector3_6(void);
int add_union_vector3_6(void);
int copy_union_vector3_6(void);
int floor_union_vector3_6(void);
int power_union_vector3_6(void);

int multiply_union_vector4_6(void);
int add_union_vector4_6(void);
int copy_union_vector4_6(void);
int floor_union_vector4_6(void);
int power_union_vector4_6(void);

int multiply_union_vector8_6(void);
int add_union_vector8_6(void);
int copy_union_vector8_6(void);
int floor_union_vector8_6(void);
int power_union_vector8_6(void);

int multiply_union_vector16_6(void);
int add_union_vector16_6(void);
int copy_union_vector16_6(void);
int floor_union_vector16_6(void);
int power_union_vector16_6(void);
