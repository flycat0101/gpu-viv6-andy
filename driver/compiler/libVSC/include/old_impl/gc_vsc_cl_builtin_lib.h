/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef __gc_vsc_cl_builtin_lib_h_
#define __gc_vsc_cl_builtin_lib_h_

#undef _CL_LOCALLY_SET
#ifndef __BUILTIN_SHADER_LENGTH__
#define __BUILTIN_SHADER_LENGTH__ (65535 * 8)
#define _CL_LOCALLY_SET
#endif
#ifndef NL
#define NL "\n"
#endif

static gctSTRING gcCLLibHeader =
NL "/* Vivante OpenCL builtin library */"
NL;

static gctSTRING gcCLLibLongMADSAT_Funcs =
NL "long _viv_mul_long(long x, long y)"
NL "{"
NL "    long r;"
NL "    uint lox, loy, hix, hiy,loz, hiz;"
NL "    lox = viv_getlonglo(x);"
NL "    hix = viv_getlonghi(x);"
NL "    loy = viv_getlonglo(y);"
NL "    hiy = viv_getlonghi(y);"
NL
NL "	loz = lox*loy; "
NL "    hiz = mul_hi(lox, loy); "
NL "    hiz += hix*loy; "
NL "    hiz += lox*hiy; "
NL "    viv_setlong(r, loz, hiz); "
NL
NL "    return r;"
NL "}"
NL "ulong _viv_mul_ulong(ulong x, ulong y)"
NL "{"
NL "    ulong r;"
NL "    uint lox, loy, hix, hiy,loz, hiz;"
NL "    lox = viv_getlonglo(x);"
NL "    hix = viv_getlonghi(x);"
NL "    loy = viv_getlonglo(y);"
NL "    hiy = viv_getlonghi(y);"
NL
NL "	loz = lox*loy; "
NL "    hiz = mul_hi(lox, loy); "
NL "    hiz += hix*loy; "
NL "    hiz += lox*hiy; "
NL "    viv_setlong(r, loz, hiz); "
NL
NL "    return r;"
NL "}"
NL
NL " long _viv_mulhi_long(long x, long y) "
NL " { "
NL "	unsigned int lox, loy, hix, hiy,loz, hiz; "
NL "    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV;"
NL "	long a; "
NL "	lox = viv_getlonglo(x); "
NL "	loy = viv_getlonglo(y); "
NL "	hix = viv_getlonghi(x); "
NL "	hiy = viv_getlonghi(y); "
NL "    hiz = mul_hi(hix, hiy); "
NL "    loz = hix*hiy; "
NL "    mulH0L1L = hix*loy; "
NL "    mulH1L0L = lox*hiy; "
NL "    mulHighL0L1 = mul_hi(lox, loy); "
NL "    mulH0L1 = mul_hi(hix, loy); "
NL "    notV = ~mulH0L1L; "
NL "    if(mulHighL0L1 >= notV){ "
NL "        mulH0L1++; "
NL "    } "
NL "    mulHighL0L1 += mulH0L1L; "
NL
NL "    mulH1L0 = mul_hi(lox, hiy); "
NL "    notV = ~mulH1L0L; "
NL "    if(mulHighL0L1 >= notV){ "
NL "        mulH1L0++; "
NL "    } "
NL
NL "    notV = ~mulH0L1; "
NL "    if(loz >= notV){ "
NL "        hiz++; "
NL "    } "
NL "    loz += mulH0L1; "
NL
NL "    notV = ~mulH1L0; "
NL "    if(loz >= notV){ "
NL "        hiz++; "
NL "    } "
NL "    loz += mulH1L0; "
NL "/* For negative input, it looks like extended -1 to bit 64~127, then we do substraction */"
NL "    if(hix >= 0x80000000){"
NL "        if(loz < loy) /*borrow happened*/ "
NL "            hiz--; "
NL "        loz -= loy; "
NL "        hiz -= hiy; "
NL "    }"
NL "    if(hiy >= 0x80000000){"
NL "        if(loz < lox) /*borrow happened*/ "
NL "            hiz--; "
NL "        loz -= lox; "
NL "        hiz -= hix; "
NL "    }"
NL "    viv_setlong(a, loz, hiz); "
NL "    return a; "
NL "} "
NL
NL " ulong _viv_mulhi_ulong(ulong x, ulong y) "
NL " { "
NL "	unsigned int lox, loy, hix, hiy,loz, hiz; "
NL "    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV;"
NL "	long a; "
NL "	lox = viv_getlonglo(x); "
NL "	loy = viv_getlonglo(y); "
NL "	hix = viv_getlonghi(x); "
NL "	hiy = viv_getlonghi(y); "
NL "    hiz = mul_hi(hix, hiy); "
NL "    loz = hix*hiy; "
NL "    mulH0L1L = hix*loy; "
NL "    mulH1L0L = lox*hiy; "
NL "    mulHighL0L1 = mul_hi(lox, loy); "
NL "    mulH0L1 = mul_hi(hix, loy); "
NL "    notV = ~mulH0L1L; "
NL "    if(mulHighL0L1 >= notV){ "
NL "        mulH0L1++; "
NL "    } "
NL "    mulHighL0L1 += mulH0L1L; "
NL "    "
NL "    mulH1L0 = mul_hi(lox, hiy); "
NL "    notV = ~mulH1L0L; "
NL "    if(mulHighL0L1 >= notV){ "
NL "        mulH1L0++; "
NL "    } "
NL
NL "    notV = ~mulH0L1; "
NL "    if(loz >= notV){ "
NL "        hiz++; "
NL "    } "
NL "    loz += mulH0L1; "
NL
NL "    notV = ~mulH1L0; "
NL "    if(loz >= notV){ "
NL "        hiz++; "
NL "    } "
NL "    loz += mulH1L0; "
NL "    viv_setlong(a, loz, hiz); "
NL "    return (ulong)a; "
NL "}"
NL
NL "long _viv_madsat_long(long x, long y, long z)"
NL "{"
NL "    long  mulhi;"
NL "    ulong mullo, sum;"
NL "    mullo = _viv_mul_long(x, y);"
NL "    mulhi = _viv_mulhi_long(x, y);"
NL "    sum = mullo + z;"
NL "    if (z >= 0)"
NL "    {"
NL "        if (mullo > sum)"
NL "        {"
NL "            mulhi++;"
NL "            if (mulhi == LONG_MIN)"
NL "            {"
NL "                mulhi = LONG_MAX;"
NL "                sum = ULONG_MAX;"
NL "            }"
NL "        }"
NL "    }"
NL "    else"
NL "    {"
NL "        if (mullo < sum)"
NL "        {"
NL "            mulhi--;"
NL "            if (LONG_MAX == mulhi)"
NL "            {"
NL "                mulhi = LONG_MIN;"
NL "                sum = 0;"
NL "            }"
NL "        }"
NL "    }"
NL
NL "    if (mulhi > 0)"
NL "        sum = LONG_MAX;"
NL "    else if (mulhi < -1)"
NL "        sum = LONG_MIN;"
NL
NL "    return (long)sum;"
NL "}"
NL
NL "ulong _viv_madsat_ulong(ulong x, ulong y, ulong z)"
NL "{"
NL "    ulong mulhi, mullo, sum;"
NL "    mullo = _viv_mul_ulong(x, y);"
NL "    mulhi = _viv_mulhi_ulong(x, y);"
NL "    mullo += z;"
NL "    if (mullo < z) mulhi++;"
NL "    if (mulhi != 0) mullo = 0xFFFFFFFFFFFFFFFFULL;"
NL "    return mullo;"
NL "}"
NL;

static gctSTRING gcCLLibLongNEXTAFTER_Funcs =
NL "float _viv_nextafter(float x, float y)"
NL "{"
NL "    int a, b;"
NL "    a = as_int(x);"
NL "    b = as_int(y);"
NL "    if((a == 0) && (b == 0x80000000)) return as_float(0x80000000);"
NL "    if((a == 0x80000000) && (b == 0)) return as_float(0x0);"
NL "    if(x != x ) return x;"
NL "    if(y != y ) return y;"
NL "    if(a == b ) return y;"
NL "    if(a & 0x80000000 ) a = 0x80000000 - a;"
NL "    if(b & 0x80000000 ) b = 0x80000000 - b;"
NL "    a += ((a < b) ? 1 : -1);"
NL "    a = ((a < 0) ? (int) 0x80000000 - a : a);"
NL "    if (((a & 0x7f800000) == 0) && (a & 0x7fffffff)) a &= ~0x7f800000;"
NL "    return as_float(a);"
NL "}"
NL;

#undef NL
#ifdef _CL_LOCALLY_SET
#undef __BUILTIN_SHADER_LENGTH__
#endif
#endif /* __gc_vsc_gl_builtin_lib_h_ */

