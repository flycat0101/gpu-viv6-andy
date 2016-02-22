/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gl_os_h_
#define __gl_os_h_

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#if defined(_WIN32)
#include <windows.h>
#include <wtypes.h>
#endif /* defined(_WIN32) */


#include <math.h>
#include "GL/gl.h"
#include "GL/glext.h"
#include "gl/gl_core.h"
#include "gl/gl_thread.h"
#include "g_disp.h"

#define __GL_CEILF(f)           ((GLfloat)ceil((GLdouble) (f)))
#define __GL_SQRTF(f)           ((GLfloat)sqrt((GLdouble) (f)))
#define __GL_POWF(a,b)          ((GLfloat)pow((GLdouble) (a),(GLdouble) (b)))
#define __GL_ABSF(f)            ((GLfloat)fabs((GLdouble) (f)))
#define __GL_FLOORF(f)          ((GLfloat)floor((GLdouble) (f)))
#define __GL_FLOORD(f)          floor(f)
#define __GL_SINF(f)            ((GLfloat)sin((GLdouble) (f)))
#define __GL_COSF(f)            ((GLfloat)cos((GLdouble) (f)))
#define __GL_ATANF(f)           ((GLfloat)atan((GLdouble) (f)))
#define __GL_LOGF(f)            ((GLfloat)log((GLdouble) (f)))


/* Memory operation */
#define __GL_MEMCOPY(to,from,count)     memcpy((GLvoid *)(to),(GLvoid *)(from),(size_t)(count))
#define __GL_MEMZERO(to,count)          memset(to,0,(size_t)(count))
#define __GL_MEMCMP(buf1, buf2, count)  memcmp(buf1, buf2, (size_t)(count))
#define __GL_MEMSET(to,value,count)     memset((to),(value),(count))

/* Use this for overlapping copies!! Often slower than memcpy */
#define __GL_MEMMOVE(to,from,count) memmove(to,from,(size_t)(count))

#if defined(_WIN32)
#define __GL_INLINE static __forceinline
#else
#define __GL_INLINE static __inline
#endif

#ifdef _LINUX_
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif

#if defined (_DEBUG) || defined (DEBUG)

#if defined (AMD64) || defined (__e2k__)
#define GL_ASSERT(a) assert(a)
#elif defined (_LINUX_)
#define GL_ASSERT(a) assert(a)
#else
#define GL_ASSERT(a) \
    do \
    { \
        if (!(a)) {_asm int 3};\
    } while(0)
#endif

#else
#define GL_ASSERT(a)
#endif

typedef GLvoid *DHGLRC;

#if defined(_WIN32)
#define __gl_context ((__GLcontext *)__glGetTLSCXValue())
#define __GL_SETUP() __GLcontext *gc = __gl_context
#endif
#if defined(_LINUX_)
#define __gl_context ((__GLcontext *)_glapi_get_context())
#define __GL_SETUP() __GLcontext *gc = __gl_context
#endif

#endif /* __gl_os_h_ */
