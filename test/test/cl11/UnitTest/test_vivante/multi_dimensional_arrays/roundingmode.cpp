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


#include "roundingmode.h"

#if (defined(_WIN32) && defined (_MSC_VER))
#include <float.h>
#else
#include <fenv.h>
#endif

#if defined( __i386__ ) || defined( __x86_64__ ) || defined( __e2k__ ) || defined (_WIN32)
    #include <xmmintrin.h>
#elif defined( __PPC__ )
    #include <fpu_control.h>
#endif

#if !(defined(_WIN32) && defined(_MSC_VER))
int setRoundToZero() {
    fesetround(FE_DOWNWARD);
    return 1;
}
#else
int setRoundToZero() {
    unsigned int oldRound;
    int err = _controlfp_s(&oldRound, 0, 0);
    if (err)
        return 0;
    oldRound &= _MCW_RC;
    _controlfp_s(&oldRound, _RC_DOWN, _MCW_RC);
    return 1;
}
#endif

int flushToZero() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( __e2k__ ) || defined(_MSC_VER)
    _mm_setcsr(_mm_getcsr() | 0x8040);
    return 1;
#elif defined( __arm__ )
    return 1;
#elif defined( __PPC__ )
    fpu_control_t flags = 0;
    _FPU_GETCW(flags);
    flags |= _FPU_MASK_NI;
    _FPU_SETCW(flags);
    return 1;
#endif
}

