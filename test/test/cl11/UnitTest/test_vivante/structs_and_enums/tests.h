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


int const_global(void);
int volatile_global(void);
int restrict_global(void);

int const_constant(void);
int volatile_constant(void);
int restrict_constant(void);

int const_private(void);
int restrict_private(void);
int volatile_private(void);

int const_local(void);
int restrict_local(void);
int volatile_local(void);

int const_global_enum(void);
int restrict_global_enum(void);
int volatile_global_enum(void);

int const_private_enum(void);
int restrict_private_enum(void);
int volatile_private_enum(void);

int const_constant_enum(void);
int restrict_constant_enum(void);
int volatile_constant_enum(void);

int const_local_enum(void);
int restrict_local_enum(void);
int volatile_local_enum(void);