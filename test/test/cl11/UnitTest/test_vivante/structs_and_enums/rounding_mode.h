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



#ifndef __ROUNDING_MODE_H__
#define __ROUNDING_MODE_H__

#include <stdlib.h>

#if (defined(_WIN32) && defined (_MSC_VER))
// need for _controlfp_s and rouinding modes in RoundingMode
#include <float.h>
#else
    #include <fenv.h>
#endif

typedef enum
{
    kDefaultRoundingMode = 0,
    kRoundToNearestEven,
    kRoundUp,
    kRoundDown,
    kRoundTowardZero,

    kRoundingModeCount
}RoundingMode;

typedef enum
{
    kuchar = 0,
    kchar = 1,
    kushort = 2,
    kshort = 3,
    kuint = 4,
    kint = 5,
    kfloat = 6,
    kdouble = 7,
    kulong = 8,
    klong = 9,

    //This goes last
    kTypeCount
}Type;

#ifdef __cplusplus
extern "C" {
#endif

extern RoundingMode set_round( RoundingMode r, Type outType );
extern RoundingMode get_round( void );
extern void *FlushToZero( void );
extern void UnFlushToZero( void *p);

#ifdef __cplusplus
}
#endif



#endif /* __ROUNDING_MODE_H__ */
