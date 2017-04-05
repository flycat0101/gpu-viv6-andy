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


#ifndef _DEFINES_H_
#define _DEFINES_H_

#include <VX/vx.h>

typedef struct _vx_param_description_t
{
    vx_enum     direction;
    vx_enum     dataType;
    vx_enum     state;
} vx_param_description_t;

typedef struct _vx_kernel_description_t
{
    vx_enum                       enumeration;
    vx_char                       name[VX_MAX_KERNEL_NAME];
    vx_kernel_f                   function;
    vx_param_description_t*       parameters;
    vx_uint32                     numParams;
    vx_kernel_input_validate_f    inputValidator;
    vx_kernel_output_validate_f	  outputValidator;
    vx_kernel_initialize_f        initializer;
    vx_kernel_deinitialize_f      deinitializer;
} vx_kernel_description_t;

#endif
