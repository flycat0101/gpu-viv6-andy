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
#include "gc_es_core.h"
#include "gc_es_thread.h"
#include "gc_es_utils.h"


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

#endif

#else
#define GL_ASSERT(a)
#endif

typedef GLvoid *DHGLRC;

#if defined(_WIN32)
#define __gl_context __glGetGLcontext();
#define __GL_SETUP() __GLcontext *gc = __gl_context
#endif
#if defined(_LINUX_)
#define __gl_context ((__GLcontext *)_glapi_get_context())
#define __GL_SETUP() __GLcontext *gc = __gl_context
#endif

#endif /* __gl_os_h_ */
