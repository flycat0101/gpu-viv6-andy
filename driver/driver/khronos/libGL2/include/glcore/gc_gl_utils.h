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


#ifndef __gc_gl_utils_h_
#define __gc_gl_utils_h_

#ifdef __sgi
#define __GL_CEILF(f)           ceilf(f)
#define __GL_SQRTF(f)           sqrtf(f)
#define __GL_POWF(a,b)          powf(a,b)
#define __GL_ABSF(f)            fabsf(f)
#define __GL_FLOORF(f)          floorf(f)
#define __GL_FLOORD(f)          floor(f)
#define __GL_SINF(f)            sinf(f)
#define __GL_COSF(f)            cosf(f)
#define __GL_ATANF(f)           atanf(f)
#define __GL_LOGF(f)            logf(f)
#else
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
#endif

#define __GL_MIN(a,b)    (((a)<(b))?(a):(b))
#define __GL_MAX(a,b)    (((a)>(b))?(a):(b))

#define XY_FRAC_BITS            11
#define Z_FRAC_BITS             0
#define COLOR_FRAC_BITS         16
#define STRQ_FRAC_BITS          16

#define FLOATTMP    GLdouble dtmp;

#define FloatBits(a)                (ftmp = (float) (a), *((GLint *)&ftmp))
#define FloatBias(bits)             ((float) (3 << (22-(bits))))
#define BiasedFloatToFixed(a)       ((FloatBits(a) & 0x7FFFFFL) - 0x400000L)
#define BiasedFloatToInt(a,bits)    TruncFixed(BiasedFloatToFixed(a),bits)

#define DoubleBits(a)               (dtmp = (GLdouble) (a), *((GLint *)&dtmp))
#define DoubleBias(bits)            ((GLdouble) (3 << 20) * (1UL << (31-(bits))))
#define BiasedDoubleToFixed(a)      DoubleBits(a)
#define BiasedDoubleToInt(a,bits)   TruncFixed(BiasedDoubleToFixed(a),bits)

#define FloatToFixedI(a,bits)       ((GLint) ((a) * (float) (1UL << (bits))))
#define FloatToFixedF(a,bits)       BiasedFloatToFixed((a)+FloatBias(bits))
#define FloatToFixedD(a,bits)       BiasedDoubleToFixed((a)+DoubleBias(bits))

#define FloatToInt(a,bits)          ((GLint) (a))
#define FloatToFrac(a)              ((FloatBits((a)+3.0F) << 9) ^ 0x80000000UL)

#ifdef FORCE_SINGLE_PRECISION
#define FloatToFixed(a,bits)        FloatToFixedF(a,bits)
#else
#define FloatToFixed(a,bits)        FloatToFixedD(a,bits)
#endif

#define TruncFixed(a,bits)          ((GLint) (a) >> (bits))

#define CoordToInt(a)               TruncFixed((a), COLOR_FRAC_BITS)

#define DITHER_BITS                 4
#define DITHER_BITS_LOG2            2
#define DITHER_MASK                 ((1 << DITHER_BITS_LOG2) - 1)
#define DITHER_SHIFT                (COLOR_FRAC_BITS-DITHER_BITS)

#define COLOR_ROUND_NO_DITHER       0.5F
#define COLOR_ROUND_DITHER          (0.5F / (1 << DITHER_BITS))

#define XY_BIAS                     FloatBias(XY_FRAC_BITS)

#define XY_SUBPIX_FRAC_BITS         4
#define XY_QUANTIZE_MASK            (~0 << (XY_FRAC_BITS - XY_SUBPIX_FRAC_BITS))


#ifdef __cplusplus
extern "C" {
#endif
extern GLfloat g_uByteToFloat[];
#ifdef __cplusplus
}
#endif

/*table 2.9*/
#define __GL_B_TO_FLOAT(b)      (GLfloat)((2 * ((GLfloat)(b)) + 1) * __glInvMaxUbyte)
#define __GL_UB_TO_FLOAT(ub)    (GLfloat)(g_uByteToFloat[ub])
#define __GL_S_TO_FLOAT(s)      (GLfloat)((2 * ((GLfloat)(s)) + 1) * __glInvMaxUshort)
#define __GL_US_TO_FLOAT(us)    (GLfloat)((us) * __glInvMaxUshort)
#define __GL_I_TO_FLOAT(i)        (GLfloat)((2 * ((GLfloat)(i)) + 1) * __glInvMaxUint)
#define __GL_UI_TO_FLOAT(ui)    (GLfloat)((GLfloat)(ui) * __glInvMaxUint)
#define __GL_UI24_TO_FLOAT(ui)    (GLfloat)((GLfloat)(ui) * __glInvMaxUint24)
#define __GL_B_TO_UBYTE(b)        ((GLubyte)(((b) << 1) + 1))


/* table 4.7 */
#define __GL_FLOAT_TO_UB(x)        ((GLubyte) ((x) * __glMaxUbyte  + __glHalf))
#define __GL_FLOAT_TO_B(x)        ((GLbyte)  (((x) * __glMaxUbyte - __glOne)/2))
#define __GL_FLOAT_TO_US(x)        ((GLushort)((x) * __glMaxUshort + __glHalf))
#define __GL_FLOAT_TO_S(x)        ((GLshort) (((x) * __glMaxUshort - __glOne)/2))
#define __GL_FLOAT_TO_UI(x)        ((GLuint)  ((x) * __glMaxUint + __glHalf))
#define __GL_FLOAT_TO_I(x)        ((GLint)   (((x) * __glMaxUint - __glOne)/2))

#define __GL_POINTER_TO_OFFSET(x) ((GLintptr)((GLubyte *)(x) - (GLubyte *)NULL))
#define __GL_OFFSET_TO_POINTER(x) ((GLvoid *)((GLubyte *)NULL + (x)))


#define __GL_REDUNDANT_ATTR(dst, src)    \
    if (dst == src) {                    \
        return;                            \
    }

#define __GL_REDUNDANT_ATTR2(dst0, src0, dst1, src1)    \
    if ((dst0 == src0) && (dst1 == src1)) {                \
        return;                                            \
    }

#define __GL_REDUNDANT_ATTR3(dst0, src0, dst1, src1, dst2, src2)    \
    if ((dst0 == src0) && (dst1 == src1) && (dst2 == src2)) {        \
        return;                                                        \
    }

#define __GL_REDUNDANT_ATTR4(dst0, src0, dst1, src1, dst2, src2, dst3, src3)    \
    if ((dst0 == src0) && (dst1 == src1) && (dst2 == src2) && (dst3 == src3)) {    \
        return;                                                                    \
    }

#define __CHECK_COLOR_CLAMP(clamp, dstColor, srcColor) \
    if(clamp) \
    { \
        __glClampColor(dstColor, srcColor); \
    }else \
    { \
        *dstColor = *srcColor; \
    }
/*
** Extrace from RGB565 color group to get R8/G8/B8
*/
 #define __glGetR8FromRGB565(color) (((color >> 11) & 0x1F) * (255.0F / 31.0F))
 #define __glGetG8FromRGB565(color) (((color>>5) & 0x3F)*(255.0F/63.0F))
 #define __glGetB8FromRGB565(color) ((color & 0x1F) * (255.0F / 31.0F))

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

__GL_INLINE GLsizei __glGetPow2(GLsizei value)
{
    GLint i;
    if (value == 0)
    {
        return 1;
    }
    else
    {
        i = 1;
    }
    while ((0xFFFFFFFF << i) & value)
    {
        i++;
    }
    return (1 << i);
}

/*
** Clamp an incoming color from the user.
*/

__GL_INLINE GLfloat __glClampf(GLfloat val, GLfloat min, GLfloat max)
{
    if (val < min)
        val = min;
    else if (val > max)
        val = max;

    return val;
}

__GL_INLINE GLvoid __glClampColorf(__GLcolor *d, const GLfloat s[4])
{
    GLfloat zero = __glZero;
    GLfloat one = __glOne;
    GLfloat r,g,b,a;

    r = s[0];
    g = s[1];
    b = s[2];
    a = s[3];

    if (r < zero) d->r = zero;
    else if (r > one) d->r = one;
    else d->r = r;

    if (g < zero) d->g = zero;
    else if (g > one) d->g = one;
    else d->g = g;

    if (b < zero) d->b = zero;
    else if (b > one) d->b = one;
    else d->b = b;

    if (a < zero) d->a = zero;
    else if (a > one) d->a = one;
    else d->a = a;
}

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
            GL_ASSERT(0);/*error check should be done in the caller*/
            return 0;
    }
}

__GL_INLINE GLuint __glCalMaskBitNum( GLuint uMask )
{
    GLuint uRet = 0;
    while ( uMask )
    {
        if ( uMask & 0x01 )
        {
            uRet++;
        }
        uMask = uMask>>1;
    }
    return uRet;

}

/************************************************************************/
/* Implementation for helper functions                                  */
/************************************************************************/
typedef GLushort __GLfloat16;

#define  F16_cFracBits       (10)                       // Number of fraction bits
#define  F16_cExpBits        (5)                        // Number of exponent bits
#define  F16_cSignBit        (15)                       // Index of the sign bit
#define  F16_SignMask        (1 << F16_cSignBit)      // Sign mask
#define  F16_FracMask        ((1 << F16_cFracBits) - 1) // Fraction mask
#define  F16_ExpMask         (((1 << F16_cExpBits) - 1) << F16_cFracBits)   // Exponent mask
#define  F16_cExpBias        ((1 << (F16_cExpBits - 1)) - 1)                // Exponent bias
#define  F16_eMax            F16_cExpBias                                 // Max exponent
#define  F16_eMin            (-F16_cExpBias + 1)                            // Min exponent
#define  F16_wMaxNormal      (((F16_eMax + 127) << 23) | 0x7FE000)
#define  F16_wMinNormal      ((F16_eMin + 127) << 23)
#define  F16_BiasDiffo       ((F16_cExpBias - 127) << 23)
#define  F16_cFracBitsDiff   (23 - F16_cFracBits)

/* pass value is not as good as pass pointer, for SNaN may be changned to QNaN,
   but I think it's ok for current usage. */
__GL_INLINE __GLfloat16 Float32ToFloat16(GLfloat value)
{
    GLushort ret;
    GLuint u = *(GLuint*)(&value);
    GLuint sign = (u & 0x80000000) >> 16;
    GLuint mag = u & 0x7FFFFFFF;

    if( (u & (0xff<<23)) == (0xff<<23) )
    {
        // INF or NaN
        ret = (GLushort)(sign | (((1 << F16_cExpBits) - 1))<<F16_cFracBits);
        if( (u & (~0xff800000)) != 0 )
        {
            // NaN - smash together the fraction bits to
            //       keep the random 1's (in a way that keeps float16->float->float16
            //       conversion invertible down to the bit level, even with NaN).
            ret = (GLushort)(ret| (((u>>13)|(u>>3)|(u))&0x000003ff));
        }
    }
    else if (mag > F16_wMaxNormal)
    {
        // Not representable by 16 bit float -> use flt16_max (due to round to zero)
        ret = (GLushort)(sign | ((((1 << F16_cExpBits) - 2))<<F16_cFracBits) | F16_FracMask);
    }
    else if (mag < F16_wMinNormal)
    {
        // Denormalized value

        // Make implicit 1 explicit
        UINT Frac = (mag & ((1<<23)-1)) | (1<<23);
        GLint nshift = (F16_eMin + 127 - (mag >> 23));

        if (nshift < 24)
        {
            mag = Frac >> nshift;
        }
        else
        {
            mag = 0;
        }

        // Round to zero
        ret = (GLushort)(sign | (mag>>F16_cFracBitsDiff));
    }
    else
    {
        // Normalize value with Round to zero
        ret = (GLushort)(sign | ((mag + F16_BiasDiffo)>>F16_cFracBitsDiff));
    }

    return ret;
}

__GL_INLINE GLfloat Float16ToFloat32(__GLfloat16 value)
{
    GLfloat ret;
    GLushort u = *(GLushort *)(&value);
    GLuint mantissa = u & F16_FracMask;
    GLuint exp = (u & F16_ExpMask) >> F16_cFracBits;
    GLuint sign = (u & 0x8000) << 16;
    GLint pow2value;
    GLuint uRet;
    GLfloat temp;

    if (exp == 0)
    {
        if (mantissa == 0)//(-1)^S * 0.0
        {
            ret = sign? -0.0f: 0.0f;
        }
        else//(-1)^S * 2^-14 * (M/2^10)
        {
            temp = 0.00006103515625f * (mantissa / 1024.f);
            ret = sign? (-1.0f * temp) : temp;
        }
    }
    else if (exp == 31)
    {
        if (mantissa == 0) //(-1)^S*INF
        {
            uRet = sign|(255 << 23);
        }
        else // NaN
        {
            uRet = (255<<23)|1;
        }

        ret = *(GLfloat *)(&uRet);
    }
    else//(-1)^S * 2^(E-15)*(1+M/2^10)
    {
        pow2value = __glGetPow2(exp-15);
        temp = pow2value * (1.0f + mantissa/1024.f);
        ret = sign? (-1.0f *temp) : temp;
    }

    return ret;
}


typedef GLushort __GLfloat10;

#define  F10_cFracBits       (5)                       // Number of fraction bits
#define  F10_cExpBits        (5)                        // Number of exponent bits
#define  F10_FracMask        ((1 << F10_cFracBits) - 1) // Fraction mask
#define  F10_ExpMask         (((1 << F10_cExpBits) - 1) << F10_cFracBits)   // Exponent mask
#define  F10_cExpBias        ((1 << (F10_cExpBits - 1)) - 1)                // Exponent bias
#define  F10_eMax            F10_cExpBias                                 // Max exponent
#define  F10_eMin            (-F10_cExpBias + 1)                            // Min exponent
#define  F10_wMaxNormal      (((F10_eMax + 127) << 23) | 0x7FE000)
#define  F10_wMinNormal      ((F10_eMin + 127) << 23)
#define  F10_BiasDiffo       ((F10_cExpBias - 127) << 23)
#define  F10_cFracBitsDiff   (23 - F10_cFracBits)

/* pass value is not as good as pass pointer, for SNaN may be changned to QNaN,
   but I think it's ok for current usage. */
__GL_INLINE __GLfloat10 Float32ToFloat10(GLfloat value)
{
    GLushort ret;
    GLuint u = *(GLuint*)(&value);
    GLuint sign = (u & 0x80000000);
    GLuint mag = u & 0x7FFFFFFF;


    if( (u & (0xff<<23)) == (0xff<<23) )
    {
        // INF or NaN
        if( (u & (~0xff800000)) != 0 )
        {
            // NaN - smash together the fraction bits to
            //       keep the random 1's (in a way that keeps float16->float->float16
            //       conversion invertible down to the bit level, even with NaN).
            ret = (GLushort)F10_ExpMask|1;
        }
        else
        {
            if (sign)//-INF
            {
                ret = (GLushort)0;
            }
            else//+INF
            {
                ret = (GLushort)F10_ExpMask;
            }
        }
    }
    else if (mag > F10_wMaxNormal)
    {
        if (sign)
        {
            ret = 0;
        }
        else
        {
            // Not representable by 16 bit float -> use flt16_max (due to round to zero)
            ret = (GLushort)(((((1 << F10_cExpBits) - 2))<<F10_cFracBits) | F10_FracMask);
        }
    }
    else if (mag < F10_wMinNormal)
    {
        // Denormalized value

        // Make implicit 1 explicit
        UINT Frac = (mag & ((1<<23)-1)) | (1<<23);
        GLint nshift = (F10_eMin + 127 - (mag >> 23));

        if (nshift < 24)
        {
            mag = Frac >> nshift;
        }
        else
        {
            mag = 0;
        }

        if (sign)
            ret = 0;
        else
            // Round to zero
            ret = (GLushort)(mag>>F10_cFracBitsDiff);
    }
    else
    {
        if (sign)
            ret = 0;
        else
            // Normalize value with Round to zero
            ret = (GLushort)((mag + F10_BiasDiffo)>>F10_cFracBitsDiff);
    }

    return ret;
}

__GL_INLINE GLfloat Float10ToFloat32(__GLfloat10 value)
{
    GLfloat ret;
    GLushort u = *(GLushort *)(&value);
    GLuint mantissa = u & F10_FracMask;
    GLuint exp = (u & F10_ExpMask) >> F10_cFracBits;
    GLint pow2value;
    GLuint uRet;

    if (exp == 0)
    {
        if (mantissa == 0)//0.0
        {
            ret = 0.0f;
        }
        else// 2^-14 * (M/2^5)
        {
            ret = 0.00006103515625f * (mantissa / 32.f);
        }
    }
    else if (exp == 31)
    {
        if (mantissa == 0) //INF
        {
            uRet = (255 << 23);
        }
        else // NaN
        {
            uRet = (255<<23)|1;
        }

        ret = *(GLfloat *)(&uRet);
    }
    else//2^(E-15)*(1+M/2^5)
    {
        pow2value = __glGetPow2(exp-15);
        ret = pow2value * (1.0f + mantissa/32.f);
    }
    return ret;
}

typedef GLushort __GLfloat11;

#define  F11_cFracBits       (6)                       // Number of fraction bits
#define  F11_cExpBits        (5)                        // Number of exponent bits
#define  F11_FracMask        ((1 << F11_cFracBits) - 1) // Fraction mask
#define  F11_ExpMask         (((1 << F11_cExpBits) - 1) << F11_cFracBits)   // Exponent mask
#define  F11_cExpBias        ((1 << (F11_cExpBits - 1)) - 1)                // Exponent bias
#define  F11_eMax            F11_cExpBias                                 // Max exponent
#define  F11_eMin            (-F11_cExpBias + 1)                            // Min exponent
#define  F11_wMaxNormal      (((F11_eMax + 127) << 23) | 0x7FE000)
#define  F11_wMinNormal      ((F11_eMin + 127) << 23)
#define  F11_BiasDiffo       ((F11_cExpBias - 127) << 23)
#define  F11_cFracBitsDiff   (23 - F11_cFracBits)

/* pass value is not as good as pass pointer, for SNaN may be changned to QNaN,
   but I think it's ok for current usage. */
__GL_INLINE __GLfloat11 Float32ToFloat11(GLfloat value)
{
    GLushort ret;
    GLuint u = *(GLuint*)(&value);
    GLuint sign = (u & 0x80000000);
    GLuint mag = u & 0x7FFFFFFF;


    if( (u & (0xff<<23)) == (0xff<<23) )
    {
        // INF or NaN
        if( (u & (~0xff800000)) != 0 )
        {
            // NaN - smash together the fraction bits to
            //       keep the random 1's (in a way that keeps float16->float->float16
            //       conversion invertible down to the bit level, even with NaN).
            ret = (GLushort)F11_ExpMask|1;
        }
        else
        {
            if (sign)//-INF
            {
                ret = (GLushort)0;
            }
            else//+INF
            {
                ret = (GLushort)F11_ExpMask;
            }
        }
    }
    else if (mag > F11_wMaxNormal)
    {
        if (sign)
        {
            ret = 0;
        }
        else
        {
            // Not representable by 16 bit float -> use flt16_max (due to round to zero)
            ret = (GLushort)(((((1 << F11_cExpBits) - 2))<<F11_cFracBits) | F11_FracMask);
        }
    }
    else if (mag < F11_wMinNormal)
    {
        // Denormalized value

        // Make implicit 1 explicit
        UINT Frac = (mag & ((1<<23)-1)) | (1<<23);
        GLint nshift = (F11_eMin + 127 - (mag >> 23));

        if (nshift < 24)
        {
            mag = Frac >> nshift;
        }
        else
        {
            mag = 0;
        }

        if (sign)
            ret = 0;
        else
            // Round to zero
            ret = (GLushort)(mag>>F11_cFracBitsDiff);
    }
    else
    {
        if (sign)
            ret = 0;
        else
            // Normalize value with Round to zero
            ret = (GLushort)((mag + F11_BiasDiffo)>>F11_cFracBitsDiff);
    }

    return ret;
}

__GL_INLINE GLfloat Float11ToFloat32(__GLfloat11 value)
{
    GLfloat ret;
    GLushort u = *(GLushort *)(&value);
    GLuint mantissa = u & F11_FracMask;
    GLuint exp = (u & F11_ExpMask) >> F11_cFracBits;
    GLint pow2value;
    GLuint uRet;

    if (exp == 0)
    {
        if (mantissa == 0)//0.0
        {
            ret = 0.0f;
        }
        else// 2^-14 * (M/2^6)
        {
            ret = 0.00006103515625f * (mantissa / 64.f);
        }
    }
    else if (exp == 31)
    {
        if (mantissa == 0) //INF
        {
            uRet = (255 << 23);
        }
        else // NaN
        {
            uRet = (255<<23)|1;
        }

        ret = *(GLfloat *)(&uRet);
    }
    else//2^(E-15)*(1+M/2^6)
    {
        pow2value = __glGetPow2(exp-15);
        ret = pow2value * (1.0f + mantissa/64.f);
    }
    return ret;
}


__GL_INLINE GLuint PackFloat32ToR9B9G9E5UINT(GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat red_c, green_c, blue_c, max_c;
    GLint pow2value;
    GLuint outValue;
    GLfloat exp_shared;
    /*
    ** sharedexp_max = (2^N-1)/2^N * 2^(Emax-B)
    ** for RBG9_E5 N=9, Emax=30, B=15
    */
    GLfloat sharedexp_max = 32704.0;

    red_c = __GL_MAX(0, __GL_MIN(sharedexp_max, r));
    green_c = __GL_MAX(0, __GL_MIN(sharedexp_max, g));
    blue_c = __GL_MAX(0, __GL_MIN(sharedexp_max, b));
    max_c = __GL_MAX(red_c, __GL_MAX(green_c, blue_c));

    max_c = (GLfloat)log((GLdouble)max_c);
    pow2value = (GLint)__GL_FLOORF(max_c);
    exp_shared = (GLfloat)(__GL_MAX(-16, pow2value) + 16);

    pow2value = __glGetPow2((GLsizei)(exp_shared - 6));
    r = __GL_FLOORF(red_c / pow2value + 0.5);
    g = __GL_FLOORF(green_c / pow2value + 0.5);
    b = __GL_FLOORF(blue_c / pow2value + 0.5);

    outValue = (GLuint)r;
    outValue |= ((GLuint)g << 9);
    outValue |= ((GLuint)b << 18);
    outValue |= ((GLuint)exp_shared << 27);
    return outValue;

}

#endif /* __gc_gl_utils_h_ */
