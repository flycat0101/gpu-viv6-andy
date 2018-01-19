/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_es_utils_h__
#define __gc_es_utils_h__

#include "gc_es_types.h"
#include "gc_es_consts.h"

#define __GL_CEILF(f)           ((GLfloat)ceil((double) (f)))
#define __GL_SQRTF(f)           ((GLfloat)sqrt((double) (f)))
#define __GL_POWF(a,b)          ((GLfloat)pow((double) (a),(double) (b)))
#define __GL_ABSF(f)            ((GLfloat)fabs((double) (f)))
#define __GL_FLOORF(f)          ((GLfloat)floor((double) (f)))
#define __GL_FLOORD(f)          floor(f)
#define __GL_SINF(f)            ((GLfloat)sin((double) (f)))
#define __GL_COSF(f)            ((GLfloat)cos((double) (f)))
#define __GL_ATANF(f)           ((GLfloat)atan((double) (f)))
#define __GL_LOGF(f)            ((GLfloat)log((double) (f)))

#define __GL_MIN(a,b)           (((a)<(b))?(a):(b))
#define __GL_MAX(a,b)           (((a)>(b))?(a):(b))



#define __GL_B_TO_FLOAT(b)          (GLfloat)((GLfloat)(b) * __glInvMaxByte)
#define __GL_UB_TO_FLOAT(ub)        (GLfloat)((GLfloat)(ub) * __glInvMaxUbyte)
#define __GL_S_TO_FLOAT(s)          (GLfloat)((GLfloat)(s) * __glInvMaxShort)
#define __GL_US_TO_FLOAT(us)        (GLfloat)((GLfloat)(us) * __glInvMaxUshort)
#define __GL_I_TO_FLOAT(i)          (GLfloat)((GLfloat)(i) * __glInvMaxInt)
#define __GL_UI_TO_FLOAT(ui)        (GLfloat)((GLfloat)(ui) * __glInvMaxUint)
#define __GL_UI24_TO_FLOAT(ui)      (GLfloat)((GLfloat)(ui) * __glInvMaxUint24)
#define __GL_B_TO_UBYTE(b)          ((GLubyte)(((b) << 1) + 1))


#define __GL_FLOAT_TO_UB(x)         ((GLubyte) ((x) * __glMaxUbyte + __glHalf))
#define __GL_FLOAT_TO_B(x)          ((GLbyte)  (((x) >= 0.0f) ? ((x) * __glMaxByte + __glHalf) : ((x) * __glMaxByte - __glHalf)))
#define __GL_FLOAT_TO_US(x)         ((GLushort)((x) * __glMaxUshort + __glHalf))
#define __GL_FLOAT_TO_S(x)          ((GLshort) (((x) >= 0.0f) ? ((x) * __glMaxShort + __glHalf) : ((x) * __glMaxShort - __glHalf)))
#define __GL_FLOAT_TO_UI(x)         ((GLuint)  ((x) * __glMaxUint + __glHalf))
#define __GL_FLOAT_TO_I(x)          ((GLint)   (((x) >= 0.0f) ? ((x) * __glMaxInt + __glHalf) : ((x) * __glMaxInt - __glHalf)))

#define __GL_POINTER_TO_OFFSET(x)   ((GLintptr)((GLubyte *)(x) - (GLubyte *)NULL))
#define __GL_OFFSET_TO_POINTER(x)   ((GLvoid *)((GLubyte *)NULL + (x)))

#define __GL_SWAP(type, x, y)       {type temp = x; x = y; y = temp;}


#define __GL_REDUNDANT_ATTR(dst, src)    \
    if (dst == src) {                    \
        return;                            \
    }


#define __GL_REDUNDANT_ATTR2(dst0, src0, dst1, src1)    \
    if ((dst0 == src0) && (dst1 == src1)) {                \
        return;                                            \
    }

/*
* Compute the floor of the log base 2 of a unsigned integer (used mostly
*  for computing log2(2^n)).
*/
__GL_INLINE GLuint __glFloorLog2(GLuint n)
{
    GLint i = 1;

    while ((n >> i) > 0)
    {
        i++;
    }

    return (i-1);
}

/*
** Clamp an incoming color from the user.
*/
__GL_INLINE GLfloat __glClampf(GLfloat val, GLfloat min, GLfloat max)
{
    if (val < min)
    {
        val = min;
    }
    else if (val > max)
    {
        val = max;
    }

    return val;
}

__GL_INLINE GLint __glClampi(GLint val, GLint min, GLint max)
{
    if (val < min)
    {
        val = min;
    }
    else if (val > max)
    {
        val = max;
    }

    return val;
}

__GL_INLINE GLint __glFloat2NearestInt(const GLfloat val)
{
    GLfloat tmp;

    if (val >= 0.0f)
    {
        tmp = val + 0.5f;
    }
    else
    {
        tmp = val - 0.5f;
    }

    if (tmp > (GLfloat)__glMaxInt)
    {
        return (GLint)((1U << 31) - 1);
    }
    if (tmp < (GLfloat)__glMinInt)
    {
        return (1U << 31);
    }

    return (GLint)tmp;
}

__GL_INLINE GLsizei __glSizeOfType(GLenum type)
{
    switch (type )
    {
        case GL_BYTE:
            return sizeof(GLbyte);

        case GL_UNSIGNED_BYTE:
            return sizeof(GLubyte);

        case GL_SHORT:
            return sizeof(GLshort);

        case GL_UNSIGNED_SHORT:
            return sizeof(GLushort);

        case GL_INT:
            return sizeof(GLint);

        case GL_UNSIGNED_INT:
            return sizeof(GLuint);

        case GL_FLOAT:
            return sizeof(GLfloat);

        case GL_DOUBLE:
            return sizeof(GLdouble);

        default:
//            GL_ASSERT(0);/*error check should be done in the caller*/
            return 0;
    }
}

/* Memory operation */
#define __GL_MEMCOPY(to,from,count)     memcpy((GLvoid *)(to),(GLvoid *)(from),(size_t)(count))
#define __GL_MEMZERO(to,count)          memset(to,0,(size_t)(count))
#define __GL_MEMCMP(buf1, buf2, count)  memcmp(buf1, buf2, (size_t)(count))
#define __GL_MEMSET(to,value,count)     memset((to),(value),(count))


#define __GL_ALIGN(n, align)        (((n) + ((align) - 1)) & ~((align) - 1))
#define __GL_ISALIGNED(n, align)    (((n) & ((align) - 1)) == 0)

#define __GL_ISDIGIT(c)             ((c) >= '0' && (c) <= '9')

__GL_INLINE GLvoid __glClampColor(__GLcolor *d, __GLcolor* s)
{
    GLfloat zero = __glZero;
    GLfloat one = __glOne;

    if (s->r < zero) d->r = zero;
    else if (s->r > one) d->r = one;
    else d->r = s->r;

    if (s->g < zero) d->g = zero;
    else if (s->g > one) d->g = one;
    else d->g = s->g;

    if (s->b < zero) d->b = zero;
    else if (s->b > one) d->b = one;
    else d->b = s->b;

    if (s->a < zero) d->a = zero;
    else if (s->a > one) d->a = one;
    else d->a = s->a;
}

#if (defined(_DEBUG) || defined(DEBUG))
#define GL_ASSERT(a) gcmASSERT(a);
#else
#define GL_ASSERT(a)
#endif

#endif /* __gc_es_utils_h__ */
