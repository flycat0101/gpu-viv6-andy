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


#ifndef _COMPAT_H_
#define _COMPAT_H_

#if defined(_WIN32) && defined (_MSC_VER)

#include <Windows.h>
#include <Winbase.h>
#include <CL/cl.h>
#include <float.h>
#include <xmmintrin.h>

#define isfinite(x) _finite(x)

#if !defined(__cplusplus)
typedef char bool;
#define inline

#else
extern "C" {
#endif

#define INFINITY    (FLT_MAX + FLT_MAX)
//#define NAN (INFINITY | 1)
//const static int PINFBITPATT_SP32  = INFINITY;


#define    isnan( x )       ((x) != (x))
#define     isinf( _x)      ((_x) == INFINITY || (_x) == -INFINITY)

#ifdef __cplusplus
}
#endif

#else // !((defined(_WIN32) && defined(_MSC_VER)
#if defined(__MINGW32__)
#include <windows.h>
#define sleep(X)   Sleep(1000*X)

#endif

#endif // !((defined(_WIN32) && defined(_MSC_VER)


#endif // _COMPAT_H_
