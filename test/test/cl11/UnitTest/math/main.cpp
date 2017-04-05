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


#include "../common.h"
#include <math.h>

const char * testName = "Math";
bool         gHasLong = false;    /* OpenCL proflile. */

/******************************************************************************\
|* CL programs                                                                *|
\******************************************************************************/
char * programSources[] =
{
/* Case 0. */
"\
__kernel void math( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    *pdst = *psrc + offset; \
}",

/* Case 1. */
"\
#define WR_STRIDE 6 \n\
__kernel void math( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    float histogram[WR_STRIDE]; \
    int i; \
    for(i = 0; i < WR_STRIDE; i++) { \
        histogram[i] = psrc[i] + offset; \
    } \
    for(i = 0; i < WR_STRIDE; i++) { \
        pdst[i] = histogram[i]; \
    } \
}",

/* Case 2. */
"\
__kernel void math( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    float2 absVec; \
    absVec = (float2)(offset, offset); \
    *((float2 *)pdst) = *((float2 *)psrc) + fabs(absVec); \
}",

/* Case 3. */
"\
__kernel void math( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    float8 absVec; \
    absVec = (float8)(offset, offset, offset, offset, offset, offset, offset, offset); \
    *((float8 *)pdst) = *((float8 *)psrc) + fabs(absVec); \
}",

/* case 4 */
    "\
__kernel void math( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    float8 absVec; \
    absVec = (float8)(offset, offset, offset, *((float4 *) psrc), offset); \
    *((float8 *)pdst) = *((float8 *)psrc) + fabs(absVec); \
}",

/* case 5 */
"\
__kernel void math( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    uint offset \
    ) \
{ \
    *pdst = *psrc << offset; \
}",

/* case 6 */
"\
#define WR_STRIDE 6 \n\
__kernel void math( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    uint offset \
    ) \
{ \
    unsigned int  histogram[WR_STRIDE]; \
    int i; \
    for(i = 0; i < WR_STRIDE; i++) { \
        histogram[i] = psrc[i] << offset; \
    } \
    for(i = 0; i < WR_STRIDE; i++) { \
        pdst[i] = histogram[i]; \
    } \
}",

/* case 7 */
"\
__kernel void math( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    unsigned int offset \
    ) \
{ \
    uint2 absVec; \
    uchar tmp; \
    char tmpchar; \
    int tid; \
    tid = get_global_id(0); \
    tmpchar = (char) offset; \
    tmp = abs(((char *)psrc)[tid] ); \
    absVec = (uint2)(offset, offset); \
    *((uint2 *)pdst) = *((uint2 *)psrc) + abs(absVec); \
}",

/* case 8 */
    "\
__kernel void math( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    unsigned int offset \
    ) \
{ \
    int8 absVec; \
    absVec = (int8)(-2); \
    *((uint8 *)pdst) = *((uint8 *)psrc) + abs(absVec); \
}",

/* case 9 */
    "\
__kernel void math( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    unsigned int offset \
    ) \
{ \
    int8 absVec; \
    absVec = (int8)(offset); \
    *((uint8 *)pdst) = *((uint8 *)psrc) + abs(absVec); \
}",

/* case 10 */
"\
__kernel void math( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    uint offset \
    ) \
{ \
    uint16 shift = (uint16)(offset); \
    *((uint16 *)pdst) = *((uint16 *)psrc) << shift; \
}",

/* case 11 */
"\
__kernel void math( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    uint offset \
    ) \
{ \
    uint andbits = offset -1; \
    uint8 bitvec = (uint8)(andbits); \
    *((uint8 *)pdst) = *((uint8 *)psrc) & bitvec; \
}",

/* case 12 */
"\
__kernel void math( \
    __global int *pdst, \
    __global int *psrc, \
    uint offset \
    ) \
{ \
    int8 intvec = (int8)(offset); \
    *((int8 *)pdst) -= *((int8 *)psrc) + intvec; \
}",

/* case 13 */
"\
__kernel void math( \
    __global int *pdst, \
    __global int *psrc, \
    uint offset \
    ) \
{ \
    int8 intvec = (int8)(offset, offset + 1, offset + 2, offset + 3, offset + 4, offset + 5, offset + 6, offset + 7); \
    *((int8 *)pdst) = *((int8 *)psrc) * intvec; \
}",

/* case 14 */
"\
__kernel void math( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    float8 fvec = (float8)(offset); \
    *((float8 *)pdst) -= *((float8 *)psrc) + fvec; \
}",

/* case 15 */
"\
__kernel void math( \
    __global int *pdst, \
    __global int *psrc, \
    uint offset \
    ) \
{ \
    char c = 2; \
    char d = 3; \
    char e = c * d; \
    *pdst = *psrc + e; \
}",

/* Case 16. */
"\
__kernel void math( \
    __global float *pdst, \
    __global int *psrc, \
    float offset \
    ) \
{ \
    pdst[0] = convert_float_rte(psrc[0]); \
}",

/* case 17 */
"\
#define WR_STRIDE 6 \n\
#define STRIDE_X 2 \n\
#define STRIDE_Y 3 \n\
__kernel void math( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    uint offset \
    ) \
{ \
    unsigned int  histogram[STRIDE_X][STRIDE_Y]; \
    int i; \
        int j; \
    for(i = 0; i < STRIDE_X; i++) { \
        for(j = 0; j < STRIDE_Y; j++) { \
        histogram[i][j] = psrc[i * STRIDE_Y + j] << offset; \
        } \
    } \
    for(i = 0; i < STRIDE_X; i++) { \
        for(j = 0; j < STRIDE_Y; j++) { \
        pdst[i * STRIDE_Y + j] = histogram[i][j]; \
        } \
    } \
}",

/* case 18 */
"\
#define WR_STRIDE 6 \n\
#define STRIDE_X 2 \n\
#define STRIDE_Y 3 \n\
__kernel void math( \
    __global unsigned int *pdst, \
    __global unsigned int *psrc, \
    uint offset \
    ) \
{ \
    unsigned int  histogram[STRIDE_X][STRIDE_Y]; \
    int i; \
        int j; \
    for(i = 0; i < STRIDE_X; i++) { \
        for(j = 0; j < STRIDE_Y; j++) { \
        *&histogram[i][j] = psrc[i * STRIDE_Y + j] << offset; \
        } \
    } \
    for(i = 0; i < STRIDE_X; i++) { \
        for(j = 0; j < STRIDE_Y; j++) { \
        pdst[i * STRIDE_Y + j] = *&histogram[i][j]; \
        } \
    } \
}",

/* case 19 */
"\
__kernel void math( \
    __global int *pdst, \
    __global int *psrc, \
    uint offset \
    ) \
{ \
    int valA = psrc[0]; \
    int valB = psrc[1]; \
    int valC = offset; \
    int destVal = valC ? valA : valB; \
    *pdst = destVal; \
}",

/* Case 20. */
"\
__kernel void math( \
    volatile __global int *pdst, \
    int offset \
    ) \
{ \
    int oldVal = atomic_add( &pdst[0], offset ); \
    pdst[1] = oldVal; \
}",

/* case 21 */
"\
__kernel void math( \n\
    __global int *pdst, \n\
    __global int *psrc, \n\
    uint offset \n\
    ) \n\
{ \n\
    int *srcptr; \n\
    __global int *srcptr1; \n\
    __global int *srcptr2; \n\
    int ptrdiff1; \n\
    int ptrdiff2; \n\
    srcptr = psrc + 5; \n\
    srcptr1 = &psrc[10]; \n\
    srcptr2 = psrc + 12; \n\
    ptrdiff1 = srcptr1 - srcptr2; \n\
    ptrdiff2 = srcptr2 - srcptr1; \n\
    pdst[0] = srcptr[ptrdiff1]; \n\
    pdst[1] = srcptr[ptrdiff2]; \n\
}",

/* case 22 */
"\
typedef struct _my_struct { \
int a; \
char b; \
int c; \
} my_struct; \
__kernel void math( \n\
    __global int *pdst, \n\
    __global my_struct *psrc, \n\
    uint offset \n\
    ) \n\
{ \n\
    __global my_struct *srcptr; \n\
    __global my_struct *srcptr1; \n\
    __global my_struct *srcptr2; \n\
    int ptrdiff1; \n\
    int ptrdiff2; \n\
    srcptr = psrc + 5; \n\
    srcptr1 = &psrc[10]; \n\
    srcptr2 = psrc + 12; \n\
    ptrdiff1 = srcptr1 - srcptr2; \n\
    ptrdiff2 = srcptr2 - srcptr1; \n\
    pdst[0] = srcptr[ptrdiff1].a; \n\
    pdst[1] = srcptr[ptrdiff2].c; \n\
}",

/* Case 23. */
"\
__kernel void math( \
    __global char *pdst, \
    __global char *psrc, \
    uint offset \
    ) \
{ \
    pdst[1] = psrc[1] / (char) offset; \
}",

/* Case 24. */
"\
__kernel void math( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    *pdst = *psrc / offset; \
}",

/* Case 25. */
"\
__kernel void math( \
    __global float *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    *pdst = pow(*psrc,  offset); \
}",

/* Case 26. long add */
"\
long viv_Add64(long x, long y) \
{ \
    unsigned int lox, loy, hix, hiy; \
    long a; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    hiy = viv_getlonghi(y); \
    if(lox > ~loy) \
        hix++; \
    hix += hiy; \
    lox += loy; \
    viv_setlong(a, lox, hix); \
    return a; \
} \
\
__kernel void math( \
    __global long *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_Add64(psrc[0], psrc[1]); \
}",

/* Case 27. ulong Rshift */
"\
long viv_Rshift64(long x, long y) \
{ \
    unsigned int lox, loy, hix; \
    long a; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    loy &= 0x3f; \
    if(loy >= 32){ \
        lox = hix >> (loy-32); \
        hix = 0; \
    } \
    else if(loy){ \
        lox = lox>>loy; \
        lox |= hix <<(32 - loy); \
        hix >>= loy; \
    } \
    viv_setlong(a, lox, hix); \
    return  a; \
} \
__kernel void math( \
    __global long *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_Rshift64(psrc[0], psrc[1]); \
}",
/* Case 28. long Rshift, signed long */
"\
long viv_Rshift64Signed(long x, long y) \
{ \
    int lox, loy, hix; \
    long a; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    loy &= 0x3f; \
    if(loy >= 32){ \
        lox = hix >> (loy-32); \
        hix >>= 31; \
    } \
    else if(loy){ \
        lox = (unsigned int)lox>>loy; \
        lox |= hix <<(32 - loy); \
        hix >>= loy; \
    } \
    viv_setlong(a, (unsigned int)lox, (unsigned int)hix); \
    return  a; \
} \
__kernel void math( \
    __global long *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_Rshift64Signed(psrc[0], psrc[1]); \
}",
/* Case 29. ulong divide */
"\
 long  viv_Mul64_32RShift(  long  x1, int n32, int rshift) \
{ \
    int lox1, hix1, mulhi, mulme, mulme2, mullo; \
    long   a1; \
    lox1 = viv_getlonglo(x1); \
    hix1 = viv_getlonghi(x1); \
    mulhi = mul_hi((unsigned int)hix1, (unsigned int)n32); \
    mulme = mul_hi((unsigned int)lox1, (unsigned int)n32); \
    mulme2 = (unsigned int)hix1*(unsigned int)n32; \
    if((unsigned int)mulme > (unsigned int)~mulme2) \
        mulhi += 1; \
    mulme += mulme2; \
    mullo = lox1 * n32; \
 \
    if(rshift >= 64){ \
        viv_setlong(a1, (unsigned int)mulhi >> rshift, (unsigned int)0 ); \
    } \
    else if(rshift >= 32){ \
        mulme =(unsigned int)mulme >> (rshift - 32); \
        if(rshift > 32) \
            mulme |= (unsigned int)mulhi << ( 64 - rshift); \
        mulhi =(unsigned int)mulhi >> (rshift - 32); \
        viv_setlong(a1, (unsigned int)mulme, (unsigned int)mulhi); \
    } \
    else{ \
        mullo =(unsigned int)mullo >> (rshift); \
        if(rshift){ \
            mullo |= mulme << (32 - rshift); \
        } \
        mulme =(unsigned int)mulme >> (rshift); \
        if(rshift) \
            mulme |= (unsigned int)mulhi <<(32 - rshift);\
            viv_setlong(a1, (unsigned int)mullo, (unsigned int)mulme); \
    } \
    return a1; \
} \
\
 long   viv_Mul64HiLo_32RShift( int lox1, int hix1, int n32, int rshift) \
{ \
    int  mulhi, mulme, mulme2, mullo; \
     long   a1; \
    mulhi = mul_hi((unsigned int)hix1, (unsigned int)n32); \
    mulme = mul_hi((unsigned int)lox1, (unsigned int)n32); \
    mulme2 = (unsigned int)hix1*(unsigned int)n32; \
    if((unsigned int)mulme > (unsigned int)~mulme2) \
        mulhi += 1; \
    mulme += mulme2; \
    mullo = lox1 * n32; \
 \
    if(rshift >= 64){ \
        viv_setlong(a1, (unsigned int)mulhi >> rshift, (unsigned int)0 ); \
    } \
    else if(rshift >= 32){ \
        mulme =(unsigned int)mulme >> (rshift - 32); \
        if(rshift > 32) \
            mulme |= (unsigned int)mulhi << ( 64 - rshift); \
        mulhi =(unsigned int)mulhi >> (rshift - 32); \
        viv_setlong(a1, (unsigned int)mulme, (unsigned int)mulhi); \
    } \
    else{ \
        mullo =(unsigned int)mullo >> (rshift); \
        if(mullo){ \
            mullo |= mulme << (32 - rshift); \
        } \
        mulme =(unsigned int)mulme >> (rshift); \
            viv_setlong(a1, (unsigned int)mullo, (unsigned int)mulme); \
    } \
    return a1; \
} \
\
 long   viv_Mul64ThenNeg(unsigned int lox2, unsigned int hix2,  long   y2) \
{ \
    unsigned int loy2, hiy2,hiz2,loz2; \
        long   a2; \
    loy2 = viv_getlonglo(y2); \
    hiy2 = viv_getlonghi(y2); \
    hiz2 = mul_hi(lox2, loy2); \
    hiz2 += lox2 * hiy2; \
    hiz2 += loy2 * hix2; \
    loz2 = loy2*lox2; \
    loz2 = -loz2; \
    hiz2 = ~hiz2; \
    if(loz2 == 0) \
        hiz2 += 1; \
    viv_setlong(a2, loz2, hiz2); \
    return a2; \
} \
\
\
 long   viv_Add64( long  x3,  long   y3) \
{ \
    unsigned int lox3, loy3, hix3, hiy3; \
    long   a3; \
    lox3 = viv_getlonglo(x3); \
    loy3 = viv_getlonglo(y3); \
    hix3 = viv_getlonghi(x3); \
    hiy3 = viv_getlonghi(y3); \
    if(lox3 > ~loy3) \
        hix3++; \
    hix3 += hiy3; \
    lox3 += loy3; \
    viv_setlong(a3, lox3, hix3); \
    return a3; \
} \
\
 \
  long  viv_Unsigned64Div( long  x,  long  y) \
 { \
    unsigned int lox, loy, hix, hiy,lor, hir, loq, hiq; \
    int z, exp, mantissa; \
    long  a, zz, res, q, dq; \
    float fValue1, fValue0; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    hiy = viv_getlonghi(y); \
    fValue1 = (float) loy + (float)hiy*4294967296.f; /*to float*/ \
    exp = as_int(fValue1) >> 23; \
    exp -= 0x7f; \
    mantissa = as_int(fValue1) & 0x007fffff; /*Clean the exp*/ \
    mantissa |= 0x30800000; /*2^(-30)*mantissaY*/ \
    fValue0 = as_float(mantissa); \
    fValue1 = 1.0f/fValue0;             /*Actually, should use our InstRcp*/ \
 \
    mantissa = as_int(fValue1) - 3;  /*make should less than 1/y */  \
    fValue0 = as_float(mantissa); \
    z = (int)fValue0;          /*2^(log2(y)+30)/y, back to integer*/ \
 \
    q =  viv_Mul64_32RShift(x, z, exp+30 ); /*Get estimation of x/y, may get 21 bit precision*/ \
    zz = viv_Mul64ThenNeg(loy, hiy , q); \
    res =  viv_Add64(x, zz); \
 \
    dq = viv_Mul64_32RShift( res, z, exp+30 );  \
    q = viv_Add64(dq, q); \
    zz = viv_Mul64ThenNeg(loy, hiy, q); \
    res =  viv_Add64(x, zz); \
 \
    dq = viv_Mul64_32RShift( res, z, exp+30 );  \
    q = viv_Add64(dq, q); \
    zz = viv_Mul64ThenNeg(loy, hiy, q); \
    res =  viv_Add64(x, zz); \
 \
    lor =  viv_getlonglo(res); \
    hir =  viv_getlonghi(res); \
\
    if( (hir > hiy) || ( (hir == hiy) && (lor >= loy) )   ){ /*q = x/y - 1*/ \
        /*res -= y; we don't need return res, if do %, we should have this step and return to res*/ \
        loq =  viv_getlonglo(q); \
        hiq =  viv_getlonghi(q); \
        loq ++; \
        if(loq == 0) \
            hiq++; \
        viv_setlong(q, loq, hiq); \
    } \
    return q; \
} \
 \
  long  viv_Signed64Div( long  x,  long  y) \
 { \
    unsigned int lox, loy, hix, hiy,lor, hir, loq, hiq,signedXY = 0; \
    int z, exp, mantissa; \
    long  a, zz, res, q, dq; \
    float fValue1, fValue0; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    hiy = viv_getlonghi(y); \
    if(hix>>31){\
        lox = -lox;\
        hix = (~hix);\
        if(lox == 0)\
            hix++;\
        signedXY ^= 0xffffffff;\
        viv_setlong(x, lox, hix);\
    }\
    if(hiy>>31){\
        loy = -loy;\
        hiy = (~hiy);\
        if(loy == 0)\
            hiy++;\
        signedXY ^= 0xffffffff;\
    }\
    fValue1 = (float) loy + (float)hiy*4294967296.f; /*to float*/ \
    exp = as_int(fValue1) >> 23; \
    exp -= 0x7f; \
    mantissa = as_int(fValue1) & 0x007fffff; /*Clean the exp*/ \
    mantissa |= 0x30800000; /*2^(-30)*mantissaY*/ \
    fValue0 = as_float(mantissa); \
    fValue1 = 1.0f/fValue0;             /*Actually, should use our InstRcp*/ \
 \
    mantissa = as_int(fValue1) - 3;  /*make should less than 1/y */  \
    fValue0 = as_float(mantissa); \
    z = (int)fValue0;          /*2^(log2(y)+30)/y, back to integer*/ \
 \
    q = viv_Mul64_32RShift(x, z, exp+30 ); /*Get estimation of x/y, may get 21 bit precision*/ \
    zz = viv_Mul64ThenNeg(loy, hiy , q); \
    res =  viv_Add64(x, zz); \
 \
    dq = viv_Mul64_32RShift( res, z, exp+30 );  \
    q = viv_Add64(dq, q); \
    zz = viv_Mul64ThenNeg(loy, hiy, q); \
    res =  viv_Add64(x, zz); \
 \
    dq = viv_Mul64_32RShift( res, z, exp+30 );  \
    q = viv_Add64(dq, q); \
    zz = viv_Mul64ThenNeg(loy, hiy, q); \
    res =  viv_Add64(x, zz); \
 \
    loq =  viv_getlonglo(res); \
    hiq =  viv_getlonghi(res); \
\
    if( (hiq > hiy) || ( (hiq == hiy) && (loq >= loy) )   ){ /*q = x/y - 1*/ \
        /*res -= y; we don't need return res, if do %, we should have this step and return to res*/ \
        loq =  viv_getlonglo(q); \
        hiq =  viv_getlonghi(q); \
        loq ++; \
        if(loq == 0) \
            hiq++; \
    } \
    else{\
        loq =  viv_getlonglo(q); \
        hiq =  viv_getlonghi(q); \
    }\
    if(signedXY){\
        loq = -loq;\
        hiq = (~hiq) + (loq == 0);\
    }\
    viv_setlong(q, loq, hiq); \
    return q; \
} \
__kernel void math( \
    __global long *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_Signed64Div(psrc[0], psrc[1]); \
}",

/* Case 30. ulong 2 float convert */
"\
 float viv_ulong2float(long x) \
 { \
    float fValue1; \
    int leadOne, leadOne31, leadOneP1;\
    unsigned int lox, hix; \
    lox = viv_getlonglo(x); \
    hix = viv_getlonghi(x); \
    fValue1 = (float)hix; \
    if(fValue1 == 0){ \
        fValue1 = (float) lox; \
    }\
    else if(fValue1 == 4294967296.f){ /*Full precision, may from 0xffffff80*/ \
        fValue1 = 4294967296.f*4294967296.f; \
    } \
    else{ \
        leadOne = as_int(fValue1); \
        leadOne = ( leadOne >> 23 ) - 127; /*leading one position on hix*/ \
        leadOne31 = (31 - leadOne); \
        leadOneP1 = leadOne + 1; \
        hix <<= leadOne31; \
        if(leadOne31){ \
            hix |= lox >> (leadOneP1) ; \
            lox <<= leadOne31; \
        } \
        if( lox ) /*sticky bit*/ \
            hix |= 1; \
        fValue1 = (float) hix; \
        leadOne = as_int(fValue1); \
        leadOne += (leadOneP1)<<23; \
        fValue1 = as_float(leadOne); \
    } \
    return fValue1; \
} \
__kernel void math( \
    __global float *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_ulong2float(psrc[0]); \
}",

/* Case 31. Signed long 2 float convert */
"\
 float viv_signed_long2float(long x) \
 { \
    float fValue1; \
    int leadOne, leadOne31, leadOneP1;\
    unsigned int lox, hix, sign; \
    lox = viv_getlonglo(x); \
    hix = viv_getlonghi(x); \
    sign = hix & 0x80000000; \
    if(sign){ \
        lox = -lox; \
        hix = ~hix; \
        if(lox == 0) \
            hix++; \
    } \
    fValue1 = (float)hix; \
    if(fValue1 == 0){ \
        fValue1 = (float) lox; \
    }\
    else{ \
        leadOne = as_int(fValue1); \
        leadOne = ( leadOne >> 23 ) - 127; /*leading one position on hix*/ \
        leadOne31 = (31 - leadOne); \
        leadOneP1 = leadOne + 1; \
        hix <<= leadOne31; \
        if(leadOne31){ \
            hix |= lox >> (leadOneP1) ; \
            lox <<= leadOne31; \
        } \
        if( lox ) /*sticky bit*/ \
            hix |= 1; \
        fValue1 = (float) hix; \
        leadOne = as_int(fValue1); \
        leadOne += (leadOneP1)<<23; \
        fValue1 = as_float(leadOne); \
    } \
    if(sign) \
        fValue1 = -fValue1; \
    return fValue1; \
} \
__kernel void math( \
    __global float *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_signed_long2float(psrc[0]); \
}",
/* Case 32. float to ulong convert */
"\
 long viv_float2ULong(float fValue1) \
 { \
    unsigned int lox, hix; \
    int  hexf, exp; \
    long a; \
    float absf = fabs(fValue1); \
    if( fValue1 >=  4294967296.f*4294967296.f){ /*Overflow*/ \
        lox = 0x0;       /*0x7ffff...; if _sat_ on*/ \
        hix = 0x0; \
    }\
    else if(fValue1 < -4294967296.f*4294967296.f/2.0f){ \
        lox = 0x0; \
        hix = 0x80000000; \
    }\
    else if( absf < 65536.*65536. ){ /*fValue < 2^32*/ \
        hix = 0; \
        lox = (unsigned int)absf; \
    } \
    else{ \
        hexf = as_int(absf); \
        exp = (hexf>> 23 ) - 127 - 23  ;\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \
        if(exp >= 32){ \
            hix = lox << (exp - 32); \
            lox = 0; \
        } \
        else{ \
            hix = lox >> (32 - exp); \
            lox <<= exp; \
        } \
    } \
    if(fValue1 < 0.0f){ \
        lox = -lox; \
        hix = ~hix; \
        if(lox == 0) \
            hix++; \
    } \
    viv_setlong(a, lox, hix); \
    return a; \
} \
\
__kernel void math( \
    __global long *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_float2ULong(psrc[0]); \
}",
/* Case 33. float to signed long convert */
"\
 long viv_float2SignedLong(float fValue1) \
 { \
    unsigned int lox, hix; \
    int  hexf, exp; \
    long a; \
    float absf = fabs(fValue1); \
    if( fValue1 >=  4294967296.f*4294967296.f/2.0f || fValue1 < -4294967296.f*4294967296.f/2.0f){ /*Overflow*/ \
        lox = 0;       /*0x7ffff...; if _sat_ on*/ \
        hix = 0x80000000; \
        fValue1 = 0; \
    }\
    else if( absf < 65536.*65536. ){ /*fValue < 2^32*/ \
        hix = 0; \
        lox = (unsigned int)absf; \
    } \
    else{ \
        hexf = as_int(absf); \
        exp = (hexf>> 23 ) - 127 - 23  ;\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \
        if(exp >= 32){ \
            hix = lox << (exp - 32); \
            lox = 0; \
        } \
        else{ \
            hix = lox >> (32 - exp); \
            lox <<= exp; \
        } \
    } \
    if(fValue1 < 0.0f){ \
        lox = -lox; \
        hix = ~hix; \
        if(lox == 0) \
            hix++; \
    } \
    viv_setlong(a, lox, hix); \
    return a; \
} \
__kernel void math( \
    __global long *pdst, \
    __global float *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_float2SignedLong(psrc[0]); \
}",
/* Case 34. mul_hi for ulong */
"\
 long viv_mulHi_uLong(long x, long y) \
 { \
    unsigned int lox, loy, hix, hiy,loz, hiz; \
    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV;\
    long a; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    hiy = viv_getlonghi(y); \
    hiz = mul_hi(hix, hiy); \
    loz = hix*hiy; \
    mulH0L1L = hix*loy; \
    mulH1L0L = lox*hiy; \
    mulHighL0L1 = mul_hi(lox, loy); \
    mulH0L1 = mul_hi(hix, loy); \
    notV = ~mulH0L1L; \
    if(mulHighL0L1 >= notV){ \
        mulH0L1++; \
    } \
    mulHighL0L1 += mulH0L1L; \
    \
    mulH1L0 = mul_hi(lox, hiy); \
    notV = ~mulH1L0L; \
    if(mulHighL0L1 >= notV){ \
        mulH1L0++; \
    } \
\
    notV = ~mulH0L1; \
    if(loz >= notV){ \
        hiz++; \
    } \
    loz += mulH0L1; \
\
    notV = ~mulH1L0; \
    if(loz >= notV){ \
        hiz++; \
    } \
    loz += mulH1L0; \
    viv_setlong(a, loz, hiz); \
    return a; \
} \
__kernel void math( \
    __global long *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_mulHi_uLong(psrc[0], psrc[1]); \
}",
/* Case 35. mul_hi for signed long */
"\
 long viv_mulHi_Long(long x, long y) \
 { \
    unsigned int lox, loy, hix, hiy,loz, hiz; \
    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV;\
    long a; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    hiy = viv_getlonghi(y); \
    hiz = mul_hi(hix, hiy); \
    loz = hix*hiy; \
    mulH0L1L = hix*loy; \
    mulH1L0L = lox*hiy; \
    mulHighL0L1 = mul_hi(lox, loy); \
    mulH0L1 = mul_hi(hix, loy); \
    notV = ~mulH0L1L; \
    if(mulHighL0L1 >= notV){ \
        mulH0L1++; \
    } \
    mulHighL0L1 += mulH0L1L; \
    \
    mulH1L0 = mul_hi(lox, hiy); \
    notV = ~mulH1L0L; \
    if(mulHighL0L1 >= notV){ \
        mulH1L0++; \
    } \
\
    notV = ~mulH0L1; \
    if(loz >= notV){ \
        hiz++; \
    } \
    loz += mulH0L1; \
\
    notV = ~mulH1L0; \
    if(loz >= notV){ \
        hiz++; \
    } \
    loz += mulH1L0; \
/* For negative input, it looks like extended -1 to bit 64~127, then we do substraction */ \
    if(hix >= 0x80000000){\
        if(loz < loy) /*borrow happened*/ \
            hiz--; \
        loz -= loy; \
        hiz -= hiy; \
    }\
    if(hiy >= 0x80000000){\
        if(loz < lox) /*borrow happened*/ \
            hiz--; \
        loz -= lox; \
        hiz -= hix; \
    }\
    viv_setlong(a, loz, hiz); \
    return a; \
} \
__kernel void math( \
    __global long *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_mulHi_Long(psrc[0], psrc[1]); \
}",
/* Case 36. mulSat for ulong */
"\
 long viv_mulSat_uLong(long x, long y) \
 { \
    unsigned int lox, loy, hix, hiy,loz, hiz; \
    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV, overflow = 0;\
    long a; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    hiy = viv_getlonghi(y); \
    hiz = mul_hi(hix, hiy); \
    loz = hix*hiy; \
    if(hiz || loz || mul_hi(lox, hiy) || mul_hi(hix, loy)){ \
        overflow = 0xffffffff; \
    }\
    else{ \
        mulH0L1L = hix*loy; \
        mulH1L0L = lox*hiy; \
        mulHighL0L1 = mul_hi(lox, loy); \
        notV = ~mulH0L1L; \
        if(mulHighL0L1 >= notV){ \
           overflow = 0xffffffff; \
        } \
        mulHighL0L1 += mulH0L1L; \
        \
        notV = ~mulH1L0L; \
        if(mulHighL0L1 >= notV){ \
            overflow = 0xffffffff; \
        }\
        hiz = mulHighL0L1 + mulH1L0L; \
        loz = lox * loy;\
    }\
    if(overflow){\
        loz = overflow;\
        hiz = overflow;\
    }\
    viv_setlong(a, loz, hiz); \
    return a; \
} \
__kernel void math( \
    __global long *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_mulSat_uLong(psrc[0], psrc[1]); \
}",
/* Case 37. mulSat for signed long */
"\
 long viv_mulSat_Long(long x, long y) \
 { \
    unsigned int lox, loy, hix, hiy,loz, hiz; \
    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV, resultSign = 0, overflow = 0, maxhi;\
    long a; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    hiy = viv_getlonghi(y); \
    if(hix>>31){ \
        hix = ~hix;\
        lox = -lox;\
        if(lox == 0) \
            hix++;\
        resultSign ^= 0xffffffff; \
    }\
    if(hiy>>31){ \
        hiy = ~hiy;\
        loy = -loy;\
        if(loy == 0) \
            hiy++;\
        resultSign ^= 0xffffffff; \
    }\
    hiz = mul_hi(hix, hiy); \
    loz = hix*hiy; \
    if(hiz || loz || mul_hi(lox, hiy) || mul_hi(hix, loy)){ \
        overflow = 0xffffffff; \
    }\
    else{ \
        mulH0L1L = hix*loy; \
        mulH1L0L = lox*hiy; \
        mulHighL0L1 = mul_hi(lox, loy); \
        notV = ~mulH0L1L; \
        if(mulHighL0L1 >= notV){ \
           overflow = 0xffffffff; \
        } \
        mulHighL0L1 += mulH0L1L; \
        \
        notV = ~mulH1L0L; \
        if(mulHighL0L1 >= notV){ \
            overflow = 0xffffffff; \
        }\
        hiz = mulHighL0L1 + mulH1L0L; \
        loz = lox * loy;\
        maxhi = (0x7fffffff - resultSign); \
        if(hiz > maxhi || (hiz == 0x80000000 && loz) ) \
            overflow = 0xffffffff;\
    }\
    notV = ~overflow;\
    if(resultSign){ \
        loz = (-loz) & notV; \
        hiz = ( (~hiz) + (loz == 0) )& notV; \
        hiz += overflow & 0x80000000;\
    }\
    else if(overflow){\
        loz = 0xffffffff;\
        hiz = 0x7fffffff;\
    }\
    viv_setlong(a, loz, hiz); \
    return a; \
} \
__kernel void math( \
    __global long *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_mulSat_Long(psrc[0], psrc[1]); \
}",
/* Case 38. mul for unsigned long */
"\
 long  viv_mul_Long( long  x, long  y) \
 { \
    unsigned int lox, loy, hix, hiy,loz, hiz; \
    long a; \
    lox = viv_getlonglo(x); \
    loy = viv_getlonglo(y); \
    hix = viv_getlonghi(x); \
    hiy = viv_getlonghi(y); \
    loz = lox*loy; \
    hiz = mul_hi(lox, loy); \
    hiz += hix*loy; \
    hiz += lox*hiy; \
    viv_setlong(a, loz, hiz); \
    return a; \
} \
__kernel void math( \
    __global long *pdst, \
    __global long *psrc, \
    float offset \
    ) \
{ \
    *pdst = viv_mul_Long(psrc[0], psrc[1]); \
}",
};

cl_int numPrograms = sizeof(programSources) / sizeof(programSources[0]);

/******************************************************************************\
|* Test function                                                              *|
\******************************************************************************/
cl_int
test(
    cl_context          context,
    cl_device_id        device,
    cl_command_queue    commandQueue,
    int                 testCase
    )
{
    cl_program          program;            /* OpenCL program. */
    cl_kernel           kernel;             /* OpenCL kernel. */
    cl_mem              srcBuf, dstBuf;     /* OpenCL memory buffer objects. */
    cl_mem              fsrcBuf, fdstBuf;   /* OpenCL memory buffer objects. */
    cl_bool             useSrc  = CL_FALSE;
    cl_bool             useFSrc = CL_FALSE;
    cl_bool             useSrcLong  = CL_FALSE;
    cl_bool             useDst  = CL_FALSE;
    cl_bool             useFDst = CL_FALSE;
    cl_bool             useDstLong = CL_FALSE;

    char        *pgmBuildLog;
    size_t      pgmBuildLogSize;

    cl_int status = CL_SUCCESS;
    cl_int errNum;

    cl_int src[16] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17};
    cl_int predst[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    cl_int dst[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    cl_float fsrc[8] = { 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
    cl_float fdst[8] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    cl_float prefdst[8] = { 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    cl_long srcLong[2] = {1, 0x4};
    cl_long dstLong[2] = {static_cast<cl_long>(0xffffffffffffff00), 0x3fffffff00ff0fff};
    const cl_uint  offset = 16;
    cl_float foffset = 1.0f;
    const cl_float fabsolute = 1.5f;
    cl_float fref;
    cl_long lref;

    typedef struct _my_struct {
    int a;
    char b;
        int c;
    } my_struct;

    my_struct ssrc[8] = {{1, '1', 1}, {2, '2', 2}, {3, '3', 3}, {4, '4', 4},
                         {5, '5', 5}, {6, '6', 6}, {7, '7', 7}, {8, '8', 8}};
    my_struct sdst[8] = {{0, '0', 0}, };

    size_t localWorkSize[1], globalWorkSize[1];

    clmVERBOSE("Running %s testcase %d...\n", testName, testCase);

    switch (testCase)
    {
    case 20:
        {
            bool supported = false;
            /* Check atomic extention. */
            errNum = checkExtension(device, "cl_khr_global_int32_extended_atomics", &supported);
            if (! supported)
                return errNum;
        }
        break;
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
        if (!gHasLong) {
            clmINFO("Device not support FULL_PROFILE.\n");
            return 0;
        }
        break;
    default:
        break;
    }

    clmVERBOSE("Creating program...\n");
    size_t sourceLength = strlen(programSources[testCase]);
    program = clCreateProgramWithSource(context,
                                        1,
                                        (const char **)&programSources[testCase],
                                        &sourceLength,
                                        &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    clmVERBOSE("Building program...\n");
    errNum = clBuildProgram(program, 0, NULL, "-cl-fast-relaxed-math", NULL, NULL);

    if (errNum) {
        clmVERBOSE("Build Error: %i\n", errNum);

        errNum = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, NULL, NULL, &pgmBuildLogSize);
        clmCHECKERROR(errNum, CL_SUCCESS);

        if (pgmBuildLogSize > 1) {
            pgmBuildLog = (char *) malloc(sizeof(char) * pgmBuildLogSize);
            errNum = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, pgmBuildLogSize, pgmBuildLog, NULL);
            clmCHECKERROR(errNum, CL_SUCCESS);
            clmVERBOSE("%s", pgmBuildLog);
        }
        return CL_BUILD_PROGRAM_FAILURE;
    }

    clmVERBOSE("Creating kernel...\n");
    kernel = clCreateKernel(program, "math", &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    clmVERBOSE("Creating buffers...\n");
    switch (testCase)
    {
    case 25:
        fsrc[0] = (float)4.294967e+09;
        foffset = -3.5f;

    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 14:
    case 24:
        fsrcBuf = clCreateBuffer(context,
                                 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(fsrc),
                                 fsrc,
                                 &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        fdstBuf = clCreateBuffer(context,
                                 CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(fdst),
                                 fdst,
                                 &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        useFSrc = CL_TRUE;
        useFDst = CL_TRUE;
        break;

    case 16:
        src[0] = -2147483647; /* 0x80000001*/
        srcBuf = clCreateBuffer(context,
                                 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(src),
                                 src,
                                 &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        fdstBuf = clCreateBuffer(context,
                                 CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(fdst),
                                 fdst,
                                 &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        useSrc  = CL_TRUE;
        useFDst = CL_TRUE;
        break;

    case 20:
        dstBuf = clCreateBuffer(context,
                                CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR,
                                sizeof(dst),
                                dst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        useDst = CL_TRUE;
        break;

    case 22:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(ssrc),
                                ssrc,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(dst),
                                dst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        useSrc = CL_TRUE;
        useDst = CL_TRUE;
        break;


case 26:
case 27:
case 28:
case 29:
case 30:
case 31:
case 34:
case 35:
case 36:
case 37:
case 38:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(srcLong),
                                srcLong,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        useSrcLong = CL_TRUE;
        if(testCase < 30 || testCase >= 34){
            dstBuf = clCreateBuffer(context,
                                    CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                    sizeof(dstLong),
                                    dstLong,
                                    &errNum);
            clmCHECKERROR(errNum, CL_SUCCESS);
            useDstLong = CL_TRUE;
        }
        else{
            fdstBuf = clCreateBuffer(context,
                                     CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                     sizeof(fdst),
                                     fdst,
                                     &errNum);
            clmCHECKERROR(errNum, CL_SUCCESS);
            useFDst = CL_TRUE;
        }
        break;
case 32:
case 33:
        fsrc[0] = testCase == 32? 9223372036854775800.000000f/655431.f: -9223372036854775800.000000f/655431.f;
        fsrcBuf = clCreateBuffer(context,
                                 CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                 sizeof(fsrc),
                                 fsrc,
                                 &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        useFSrc = CL_TRUE;

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(dstLong),
                                dstLong,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);
        useDstLong = CL_TRUE;
        break;
    case 23:
        src[0] <<= 8;
        dst[0] <<= 8;

    default:
        srcBuf = clCreateBuffer(context,
                                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(src),
                                src,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        dstBuf = clCreateBuffer(context,
                                CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
                                sizeof(dst),
                                dst,
                                &errNum);
        clmCHECKERROR(errNum, CL_SUCCESS);

        useSrc = CL_TRUE;
        useDst = CL_TRUE;
        break;
    }

    clmVERBOSE("Performing Math kernel...\n\n");

    switch (testCase)
    {
    case 0:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &fsrcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 1:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &fsrcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 2:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &fsrcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &fabsolute);
        break;

    case 3:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &fsrcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &fabsolute);
        break;

    case 4:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &fsrcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &fabsolute);
        break;

    case 5:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 6:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 7:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 8:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 9:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 10:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 11:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 12:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 13:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 14:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &fsrcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 15:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 16:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 17:
    case 18:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 19:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 20:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_uint), (void *) &offset);
        break;

    case 21:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 22:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 23:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    case 24:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &fsrcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &fabsolute);
        break;

    case 25:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &fsrcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_float), (void *) &foffset);
        break;

    case 26:
    case 27:
    case 28:
    case 29:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;
   case 30:
   case 31:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &fdstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &srcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;
   case 32:
   case 33:
        errNum  = clSetKernelArg(kernel, 0, sizeof(cl_mem),  (void *) &dstBuf);
        errNum |= clSetKernelArg(kernel, 1, sizeof(cl_mem),  (void *) &fsrcBuf);
        errNum |= clSetKernelArg(kernel, 2, sizeof(cl_uint), (void *) &offset);
        break;

    }
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Just a single iteration. */
    localWorkSize[0]  = 1;
    globalWorkSize[0] = 1;

    errNum = clEnqueueNDRangeKernel(commandQueue,
                                    kernel,
                                    1,
                                    NULL,
                                    globalWorkSize,
                                    localWorkSize,
                                    0,
                                    NULL,
                                    NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    clmVERBOSE("Reading back OpenCL results...\n");
    if (useDst)
    {
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(dst),
                                     dst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
    }
   if (useDstLong)
    {
        errNum = clEnqueueReadBuffer(commandQueue,
                                     dstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(dstLong),
                                     dstLong,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
    }
    if (useFDst)
    {
        errNum = clEnqueueReadBuffer(commandQueue,
                                     fdstBuf,
                                     CL_TRUE,
                                     0,
                                     sizeof(fdst),
                                     fdst,
                                     0,
                                     NULL,
                                     NULL);
        clmCHECKERROR(errNum, CL_SUCCESS);
    }

    switch (testCase)
    {
    case 0:
        if (fdst[0] == fsrc[0] + foffset)
        {
            clmVERBOSE("test 0 passes.\n");
        }
        else
        {
            clmINFO("test 0 fails.\n");
            clmINFO("src=%f\n", fsrc[0]);
            clmINFO("dst=%f\n", fdst[0]);
            clmINFO("offset=%f\n", foffset);
            status = CL_INVALID_VALUE;
        }
        break;

    case 1:
        {
#define WR_STRIDE 6
            int i;
            bool passed = true;

            for (i =0 ; i< WR_STRIDE; i++)
            {
                if (fdst[i] == (fsrc[i] + foffset))
                {
                    continue;
                }
                else if (passed)
                {
                    clmINFO("test 1 fails.\n");
                    clmINFO("offset=%f\n", foffset);
                    passed = false;
                }
                clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
                clmINFO("fdst[%d]=%f\n", i, fdst[i]);
                continue;
            }
            if (passed)
            {
                clmVERBOSE("test 1 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 2:
        {
            int i;
            bool passed = true;

            for (i=0; i<2; i++)
            {
                if (fdst[i] == fsrc[i] + fabs(fabsolute))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 2 fails.\n");
                        clmINFO("fabsolute=%f\n", fabsolute);
                        passed = false;
                    }
                    clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
                    clmINFO("fdst[%d]=%f\n", i, fdst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 2 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 3:
        {
            int i;
            bool passed = true;

            for (i=0; i < 8; i++)
            {
                if (fdst[i] == fsrc[i] + fabs(fabsolute))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 3 fails.\n");
                        clmINFO("fabsolute=%f\n", fabsolute);
                        passed = false;
                    }
                    clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
                    clmINFO("fdst[%d]=%f\n", i, fdst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 3 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 4:
        {
            int i;
            bool passed = true;

            for (i=0; i < 3; i++)
            {
                if (fdst[i] == fsrc[i] + fabs(fabsolute))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 4 fails.\n");
                        clmINFO("fabsolute=%f\n", fabsolute);
                        passed = false;
                    }
                    clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
                    clmINFO("fdst[%d]=%f\n", i, fdst[i]);
                }
            }
            for (i=3; i < 7; i++)
            {
                if (fdst[i] == fsrc[i] + fabs(fsrc[i - 3]))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 4 fails.\n");
                        passed = false;
                    }
                    clmINFO("fabsolute=%f\n", fsrc[i - 3]);
                    clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
                    clmINFO("fdst[%d]=%f\n", i, fdst[i]);
                }
            }
            if (fdst[7] != fsrc[7] + fabs(fabsolute))
            {
                if (passed)
                {
                    clmINFO("test 4 fails.\n");
                    clmINFO("fabsolute=%f\n", fabsolute);
                    passed = false;
                }
                clmINFO("fsrc[7]=%f\n", fsrc[i]);
                clmINFO("fdst[7]=%f\n", fdst[i]);
            }

            if (passed)
            {
                clmVERBOSE("test 4 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 5:
        if (dst[0] == src[0] << offset)
        {
            clmVERBOSE("test 5 passes.\n");
        }
        else
        {
            clmINFO("test 5 fails.\n");
            clmINFO("src=%08x\n", src[0]);
            clmINFO("dst=%08x\n", dst[0]);
            clmINFO("offset=%08x\n", offset);
            status = CL_INVALID_VALUE;
        }
        break;

    case 6:
        {
#define WR_STRIDE 6
            int i;
            bool passed = true;

            for (i =0 ; i< WR_STRIDE; i++)
            {
                if (dst[i] == (src[i] << offset))
                {
                    continue;
                }
                else if (passed)
                {
                    clmINFO("test 6 fails.\n");
                    clmINFO("offset=%08x\n", offset);
                    passed = false;
                }
                clmINFO("src[%d]=%08x\n", i, src[i]);
                clmINFO("dst[%d]=%08x\n", i, dst[i]);
                continue;
            }
            if (passed)
            {
                clmVERBOSE("test 6 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 7:
        {
            int i;
            bool passed = true;

            for (i=0; i<2; i++)
            {
                if (dst[i] == src[i] + (cl_uint) abs((int)offset))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 7 fails.\n");
                        clmINFO("absolute=%d\n", offset);
                        passed = false;
                    }
                    clmINFO("src[%d]=%d\n", i, src[i]);
                    clmINFO("dst[%d]=%d\n", i, dst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 7 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 8:
        {
            int i;
            bool passed = true;

            for (i=0; i<8; i++)
            {
                if (dst[i] == src[i] + abs(-2))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 8 fails.\n");
                        clmINFO("absolute=%d\n", -2);
                        passed = false;
                    }
                    clmINFO("src[%d]=%d\n", i, src[i]);
                    clmINFO("dst[%d]=%d\n", i, dst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 8 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 9:
        {
            int i;
            bool passed = true;

            for (i=0; i<8; i++)
            {
                if (dst[i] == (src[i] + abs((int) offset)))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 9 fails.\n");
                        clmINFO("absolute=%d\n", abs((int)offset));
                        passed = false;
                    }
                    clmINFO("src[%d]=%d\n", i, src[i]);
                    clmINFO("dst[%d]=%d\n", i, dst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 9 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 10:
        {
            int i;
            bool passed = true;

            for (i=0; i< 16; i++)
            {
                if (dst[i] == (src[i] << offset))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 10 fails.\n");
                        clmINFO("shift=%u\n", offset);
                        passed = false;
                    }
                    clmINFO("src[%d]=%d\n", i, src[i]);
                    clmINFO("dst[%d]=%d\n", i, dst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 10 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 11:
        {
            int i;
            bool passed = true;

            for (i=0; i< 8; i++)
            {
                if (dst[i] == (src[i] & (offset - 1)))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 11 fails.\n");
                        clmINFO("bits=%u\n", offset - 1);
                        passed = false;
                    }
                    clmINFO("src[%d]=%d\n", i, src[i]);
                    clmINFO("dst[%d]=%d\n", i, dst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 11 passes.\n");
            }
            else {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 12:
        {
            int i;
            bool passed = true;

            for (i=0; i< 8; i++)
            {
                if (dst[i] == (predst[i] - (src[i] + offset)))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 12 fails.\n");
                        clmINFO("offset=%u\n", offset);
                        passed = false;
                    }
                    clmINFO("src[%d]=%d\n", i, src[i]);
                    clmINFO("dst[%d]=%d\n", i, dst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 12 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 13:
        {
            int i;
            bool passed = true;

            for (i=0; i< 8; i++)
            {
                if (dst[i] == (src[i] * (offset + i)))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 13 fails.\n");
                        passed = false;
                    }
                    clmINFO("multiplier=%u\n", offset + i);
                    clmINFO("src[%d]=%d\n", i, src[i]);
                    clmINFO("dst[%d]=%d\n", i, dst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 13 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 14:
        {
            int i;
            bool passed = true;

            for (i=0; i< 8; i++)
            {
                if (fdst[i] == (prefdst[i] - (fsrc[i] + foffset)))
                {
                    continue;
                }
                else
                {
                    if (passed)
                    {
                        clmINFO("test 14 fails.\n");
                        clmINFO("foffset=%f\n", foffset);
                        passed = false;
                    }
                    clmINFO("fsrc[%d]=%f\n", i, fsrc[i]);
                    clmINFO("fdst[%d]=%f\n", i, fdst[i]);
                }
            }
            if (passed)
            {
                clmVERBOSE("test 14 passes.\n");
            }
            else
            {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 15:
        if (dst[0] == src[0] + 6)
        {
            clmVERBOSE("test 15 passes.\n");
        }
        else
        {
            clmINFO("test 15 fails.\n");
            clmINFO("src=%d\n", src[0]);
            clmINFO("dst=%d\n", dst[0]);
            clmINFO("expected=%d\n", src[0] + 6);
            status = CL_INVALID_VALUE;
        }
        break;

    case 16:
        {
            float expected = (float) src[0];

            if (fdst[0] == expected)
            {
                clmVERBOSE("test 16 passes.\n");
            }
            else
            {
                clmINFO("test 16 fails.\n");
                clmINFO("src=%d (0x%x)\n", src[0], src[0]);
                clmINFO("dst=%f (0x%x)\n", fdst[0], *(unsigned int *) &fdst[0]);
                clmINFO("expected=%f(0x%x)\n", expected, *(unsigned int *) &expected);
                clmINFO("\n");
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 17:
    case 18:
        {
#define WR_STRIDE 6
            int i;
            bool passed = true;

            for (i =0 ; i< WR_STRIDE; i++)
            {
                if (dst[i] == (src[i] << offset))
                {
                    continue;
                }
                else if (passed)
                {
                    clmINFO("test %d fails.\n", testCase);
                    clmINFO("offset=%08x\n", offset);
                    passed = false;
                }
                clmINFO("src[%d]=%08x\n", i, src[i]);
                clmINFO("dst[%d]=%08x\n", i, dst[i]);
                continue;
            }
            if (passed) {
                 clmVERBOSE("test %d passes.\n", testCase);
            }
            else {
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 19:
        if (offset ? (dst[0] == src[0]) :
                     (dst[0] == src[1]))
        {
            clmVERBOSE("test 19 passes.\n");
        }
        else
        {
            clmINFO("test 19 fails.\n");
            clmINFO("src0=%08x\n", src[0]);
            clmINFO("src1=%08x\n", src[1]);
            clmINFO("dst=%08x\n", dst[0]);
            clmINFO("offset=%08x\n", offset);
            status = CL_INVALID_VALUE;
        }
        break;

    case 20:
        if (dst[0] == dst[1] + offset)
        {
            clmVERBOSE("test 20 passes.\n");
        }
        else
        {
            clmINFO("test 20 fails.\n");
            clmINFO("dst0=%08x\n", dst[0]);
            clmINFO("dst1=%08x\n", dst[1]);
            clmINFO("offset=%08x\n", offset);
            status = CL_INVALID_VALUE;
        }
        break;

    case 21:
        if (dst[0] == src[3] &&
            dst[1] == src[7])
        {
            clmVERBOSE("test 21 passes.\n");
        }
        else
        {
            clmINFO("test 21 fails.\n");
            clmINFO("dst[0]=%d\n", dst[0]);
            clmINFO("dst[1]=%d\n", dst[1]);
            clmINFO("src[3]=%d\n", src[3]);
            clmINFO("src[7]=%d\n", src[7]);
            status = CL_INVALID_VALUE;
        }
        break;

    case 22:
        if (dst[0] == ssrc[3].a &&
            dst[1] == ssrc[7].c)
        {
            clmVERBOSE("test 22 passes.\n");
        }
        else
        {
            clmINFO("test 22 fails.\n");
            clmINFO("dst[0]=%d\n", dst[0]);
            clmINFO("dst[1]=%d\n", dst[1]);
            clmINFO("ssrc[3].a=%d\n", ssrc[3].a);
            clmINFO("ssrc[7].c=%d\n", ssrc[7].c);
            status = CL_INVALID_VALUE;
        }
        break;

    case 23:
        if (((char *)dst)[1] == ((char *)src)[1] / offset)
        {
            clmVERBOSE("test 23 passes.\n");
        }
        else
        {
            clmINFO("test 23 fails.\n");
            clmINFO("dst0=%08x\n", dst[0]);
            clmINFO("src0=%08x\n", src[0]);
            clmINFO("offset=%08x\n", offset);
            status = CL_INVALID_VALUE;
        }
        break;

    case 24:
        fref = fsrc[0] / fabsolute;
        {
            int diff = (*(int*)(&fref)) - (*(int*)(&fdst[0]));
            if ( !(diff > 3 || diff < -3))
            {
                clmVERBOSE("test 24 passes.\n");
            }
            else
            {
                clmINFO("test 24 fails.\n");
                clmINFO("ref=%f\n", fref);
                clmINFO("dst=%f\n", fdst[0]);
                clmINFO("src=%f\n", fsrc[0]);
                clmINFO("offset=%f\n", foffset);
                status = CL_INVALID_VALUE;
            }
        }
        break;

    case 25:
        fref = pow(fsrc[0],  foffset);
        {
            int diff = (*(int*)(&fref)) - (*(int*)(&fdst[0]));
            if ( !(diff > 16 || diff < -16))
            {
                clmVERBOSE("test 25 passes.\n");
            }
            else
            {
                clmINFO("test 25 fails.\n");
                clmINFO("ref=0x%x\n", *(unsigned int*)(&fref));
                clmINFO("dst=0x%x\n", *(unsigned int*)(&fdst[0]));
                clmINFO("src=%f\n", fsrc[0]);
                clmINFO("offset=%f\n", foffset);
                status = CL_INVALID_VALUE;
            }
        }
        break;
    case 26:
        lref = srcLong[0]+  srcLong[1];
        {
            if ( dstLong[0] == lref)
            {
                clmVERBOSE("test 26 passes.\n");
            }
            else
            {
                clmINFO("test 26 fails.\n");
                clmINFO("ref=0x%8.8x%8.8x\n", (lref>>(long) 32), (int)lref );
                clmINFO("dst=0x%8.8x%8.8x\n", (dstLong[0]>>(long) 32), (int)dstLong[0] );
                clmINFO("src1=0x%8.8x%8.8x\n", (srcLong[0]>>(long) 32), (int)srcLong[0] );
                clmINFO("src2=0x%8.8x%8.8x\n", (srcLong[1]>>(long) 32), (int)srcLong[1] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
    case 27:
        lref = ((cl_ulong)srcLong[0]>>((cl_ulong)srcLong[1]&0x3f));//OLC only support maximum 63 shifting
        {
            if ( dstLong[0] == lref)
            {
                clmVERBOSE("test 27 passes.\n");
            }
            else
            {
                clmINFO("test 27 fails.\n");
                clmINFO("ref=0x%8.8x%8.8x\n", (lref>>(long) 32), (int)lref );
                clmINFO("dst=0x%8.8x%8.8x\n", (dstLong[0]>>(long) 32), (int)dstLong[0] );
                clmINFO("src1=0x%8.8x%8.8x\n", (srcLong[0]>>(long) 32), (int)srcLong[0] );
                clmINFO("src2=0x%8.8x%8.8x\n", (srcLong[1]>>(long) 32), (int)srcLong[1] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
    case 28:
        lref = ((cl_long)srcLong[0]>>((cl_long)srcLong[1]&0x3f));//OLC only support maximum 63 shifting
        {
            if ( dstLong[0] == lref)
            {
                clmVERBOSE("test 28 passes.\n");
            }
            else
            {
                clmINFO("test 28 fails.\n");
                clmINFO("ref=0x%8.8x%8.8x\n", (lref>>(long) 32), (int)lref );
                clmINFO("dst=0x%8.8x%8.8x\n", (dstLong[0]>>(long) 32), (int)dstLong[0] );
                clmINFO("src1=0x%8.8x%8.8x\n", (srcLong[0]>>(long) 32), (int)srcLong[0] );
                clmINFO("src2=0x%8.8x%8.8x\n", (srcLong[1]>>(long) 32), (int)srcLong[1] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
    case 29:
        lref = ((cl_long)srcLong[0]/((cl_long)srcLong[1]));//OLC
        {
            if ( dstLong[0] == lref)
            {
                clmVERBOSE("test 29 passes.\n");
            }
            else
            {
                clmINFO("test 29 fails.\n");
                clmINFO("ref=0x%16.16x\n", lref );
                clmINFO("dst=0x%16.16x\n", dstLong[0] );
                clmINFO("src1=0x%16.16x\n", srcLong[0] );
                clmINFO("src2=0x%16.16x\n", srcLong[1] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
    case 30:
        fref = (float)(* (cl_ulong *) &srcLong[0]);//OLC
        {
            if ( fdst[0] == fref)
            {
                clmVERBOSE("test 30 passes.\n");
            }
            else
            {
                clmINFO("test 30 fails.\n");
                clmINFO("ref=%f\n", fref );
                clmINFO("dst==%f\n", fdst[0] );
                clmINFO("src1=0x%16.16x\n", srcLong[0] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
    case 31:
        fref = (float)(* (cl_long *) &srcLong[0]);//OLC
        {
            if ( fdst[0] == fref)
            {
                clmVERBOSE("test 31 passes.\n");
            }
            else
            {
                clmINFO("test 31 fails.\n");
                clmINFO("ref=%f\n", fref );
                clmINFO("dst==%f\n", fdst[0] );
                clmINFO("src1=0x%16.16x\n", srcLong[0] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
case 32:
        lref = (cl_ulong)fsrc[0];//OLC
        {
            if ( dstLong[0] == lref)
            {
                clmVERBOSE("test 32 passes.\n");
            }
            else
            {
                clmINFO("test 32 fails.\n");
                clmINFO("ref=0x%16.16x\n", lref );
                clmINFO("dst=0x%16.16x\n", dstLong[0] );
                clmINFO("src1=0x%16.16x\n", srcLong[0] );
                clmINFO("src2=0x%16.16x\n", srcLong[1] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
case 33:
        lref = (cl_long)fsrc[0];//OLC
        {
            if ( dstLong[0] == lref)
            {
                clmVERBOSE("test 33 passes.\n");
            }
            else
            {
                clmINFO("test 33 fails.\n");
                clmINFO("ref=0x%16.16x\n", lref );
                clmINFO("dst=0x%16.16x\n", dstLong[0] );
                clmINFO("src1=0x%16.16x\n", srcLong[0] );
                clmINFO("src2=0x%16.16x\n", srcLong[1] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
    case 34:
    case 35:
        cl_ulong MultHigh64_cpu(cl_ulong u, cl_ulong v, int Flag);
        lref =  MultHigh64_cpu((cl_ulong)srcLong[0], ((cl_ulong)srcLong[1]), testCase%2);//OLC
        {
            if ( dstLong[0] == lref)
            {
                clmVERBOSE("test %d passes.\n", testCase);
            }
            else
            {
                clmINFO("test %d fails.\n", testCase);
                clmINFO("ref=0x%16.16x\n", lref );
                clmINFO("dst=0x%16.16x\n", dstLong[0] );
                clmINFO("src1=0x%16.16x\n", srcLong[0] );
                clmINFO("src2=0x%16.16x\n", srcLong[1] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
    case 36:
    case 37:
        cl_ulong MultHigh64_cpu(cl_ulong u, cl_ulong v, int Flag);
        lref =  MultHigh64_cpu((cl_ulong)srcLong[0], ((cl_ulong)srcLong[1]), testCase%2);//OLC
        if(lref == 0 || lref == (unsigned long) (-1) ){
            lref = srcLong[0] * srcLong[1];
            if( ((srcLong[0] ^ srcLong[1]) >> (unsigned long)63) == 0 && testCase %2 ){
                if(lref >> (unsigned long)63 )
                    lref = 0x7fffffffffffffff;
            }
        }
        else if(testCase == 36){ //ulong sat
             lref = 0xffffffffffffffff;
        }
        else{
            lref =  ((srcLong[0] ^ srcLong[1]) >> (unsigned long)63) == 0 ? 0x7fffffffffffffff : (~0x7fffffffffffffff);

        }
        {
            if ( dstLong[0] == lref)
            {
                clmVERBOSE("test %d passes.\n", testCase);
            }
            else
            {
                clmINFO("test %d fails.\n", testCase);
                clmINFO("ref=0x%16.16x\n", lref );
                clmINFO("dst=0x%16.16x\n", dstLong[0] );
                clmINFO("src1=0x%16.16x\n", srcLong[0] );
                clmINFO("src2=0x%16.16x\n", srcLong[1] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
    case 38: //test mul64
        lref =  srcLong[0] * srcLong[1];//OLC

        {
            if ( dstLong[0] == lref)
            {
                clmVERBOSE("test %d passes.\n", testCase);
            }
            else
            {
                clmINFO("test %d fails.\n", testCase);
                clmINFO("ref=0x%16.16x\n", lref );
                clmINFO("dst=0x%16.16x\n", dstLong[0] );
                clmINFO("src1=0x%16.16x\n", srcLong[0] );
                clmINFO("src2=0x%16.16x\n", srcLong[1] );
                status = CL_INVALID_VALUE;
            }
        }
        break;
    }

    /* Release kernels and program. */
    errNum  = clReleaseKernel(kernel);
    errNum |= clReleaseProgram(program);
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Release other OpenCL objects. */
    if (useDst || useDstLong)
    {
        errNum  = clReleaseMemObject(dstBuf);
    }
    if (useFDst)
    {
        errNum  = clReleaseMemObject(fdstBuf);
    }
    if (useSrc||useSrcLong)
    {
        errNum |= clReleaseMemObject(srcBuf);
    }
    if (useFSrc)
    {
        errNum |= clReleaseMemObject(fsrcBuf);
    }

    return status;
}

/******************************************************************************\
|* Main program                                                               *|
\******************************************************************************/
int
main(
    int argc,
    const char **argv
    )
{
    cl_platform_id      platform;           /* OpenCL platform. */
    cl_device_id        device;             /* OpenCL device. */
    cl_context          context;            /* OpenCL context. */
    cl_command_queue    commandQueue;       /* OpenCL command queue. */

    cl_int errNum;

    cl_int testCase;

    parseArgs(argc, argv, numPrograms, &testCase);

    clmVERBOSE("Unit Test %s Starting...\n", testName);
    clmVERBOSE("Initializing OpenCL...\n");

    /* Get the available platform. */
    errNum = clGetPlatformIDs(1, &platform, NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Get a GPU device. */
    errNum = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    clmCHECKERROR(errNum, CL_SUCCESS);

    char profile[1024] = "";
    errNum = clGetDeviceInfo( device, CL_DEVICE_PROFILE, sizeof(profile), profile, NULL );
    clmCHECKERROR(errNum, CL_SUCCESS);
    if( strstr(profile, "FULL_PROFILE" ))
        gHasLong = true;

    /* Create the context. */
    context = clCreateContext(0, 1, &device, NULL, NULL, &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    /* Create a command-queue. */
    commandQueue = clCreateCommandQueue(context, device, 0, &errNum);
    clmCHECKERROR(errNum, CL_SUCCESS);

    if (testCase >= 0)
    {
        test(context, device, commandQueue, testCase);
    }
    else
    {
        int failure = 0;

        for (testCase = 0; testCase < numPrograms; testCase++)
        {
            if (test(context, device, commandQueue, testCase) != CL_SUCCESS)
            {
                failure++;
            }
        }

        clmINFO("%s: total = %d, failure = %d\n",
               testName, testCase, failure);
    }

    errNum |= clReleaseCommandQueue(commandQueue);
    errNum |= clReleaseContext(context);
    clmCHECKERROR(errNum, CL_SUCCESS);
}

union Six4ToInt
{
    cl_ulong u64;
    unsigned int n32[2];
};
cl_ulong MultHigh64_cpu(cl_ulong u, cl_ulong v, int flagSigned)
{
    union Six4ToInt un[4];
    cl_ulong temp;

    un[0].u64 = u;
    un[1].u64 = v;

    un[2].u64 = (cl_ulong)un[0].n32[1] * (cl_ulong)un[1].n32[1];

    un[2].u64 += ((cl_ulong)un[0].n32[0] * (cl_ulong)un[1].n32[1]) >> (cl_ulong)32;
    un[2].u64 += ((cl_ulong)un[1].n32[0] * (cl_ulong)un[0].n32[1]) >> (cl_ulong)32;

    un[3].u64 = ((cl_ulong)un[0].n32[0] * (cl_ulong)un[1].n32[0]);

    temp = ((cl_ulong)un[0].n32[0] * (cl_ulong)un[1].n32[1]) << (cl_ulong)32;
    if(un[3].u64 > ~temp) //Carry bit happen in Low part
        un[2].u64++;
    un[3].u64 += temp;

    temp = ((cl_ulong)un[1].n32[0] * (cl_ulong)un[0].n32[1]) << (cl_ulong)32;
    if(un[3].u64 > ~temp) //Carry bit happen in Low part
        un[2].u64++;

    if(flagSigned){
        if(un[0].n32[1] >>31){
            un[2].u64 -= un[1].u64;
        }
       if(un[1].n32[1] >>31){
            un[2].u64 -= un[0].u64;
        }
    }
    return un[2].u64;
}
