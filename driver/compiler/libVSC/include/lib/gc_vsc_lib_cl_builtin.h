/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
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


#if CompileInstrisicLibfromSrc
static gctSTRING gcCLLibFunc_Extension =
NL "#pragma OPENCL EXTENSION  CL_VIV_asm : enable"
NL;

static gctSTRING gcCLLibRelational_Funcs_packed =
/* comment out as functionality is in HW */
NL
NL;
#else
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
NL "    loz = lox*loy; "
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
NL "    loz = lox*loy; "
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
NL "    unsigned int lox, loy, hix, hiy,loz, hiz; "
NL "    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV;"
NL "    long a; "
NL "    lox = viv_getlonglo(x); "
NL "    loy = viv_getlonglo(y); "
NL "    hix = viv_getlonghi(x); "
NL "    hiy = viv_getlonghi(y); "
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
NL "    unsigned int lox, loy, hix, hiy,loz, hiz; "
NL "    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV;"
NL "    long a; "
NL "    lox = viv_getlonglo(x); "
NL "    loy = viv_getlonglo(y); "
NL "    hix = viv_getlonghi(x); "
NL "    hiy = viv_getlonghi(y); "
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
NL
NL "long2 _viv_madsat_long2(long2 x, long2 y, long2 z)"
NL "{"
NL "    long2 r;"
NL "    r.x = _viv_madsat_long(x.x, y.x, z.x);"
NL "    r.y = _viv_madsat_long(x.y, y.y, z.y);"
NL "    return r;"
NL "}"
NL
NL "ulong2 _viv_madsat_ulong2(ulong2 x, ulong2 y, ulong2 z)"
NL "{"
NL "    ulong2 r;"
NL "    r.x = _viv_madsat_ulong(x.x, y.x, z.x);"
NL "    r.y = _viv_madsat_ulong(x.y, y.y, z.y);"
NL "    return r;"
NL "}"
NL
NL "long3 _viv_madsat_long3(long3 x, long3 y, long3 z)"
NL "{"
NL "    long3 r;"
NL "    r.s0 = _viv_madsat_long(x.s0, y.s0, z.s0);"
NL "    r.s1 = _viv_madsat_long(x.s1, y.s1, z.s1);"
NL "    r.s2 = _viv_madsat_long(x.s2, y.s2, z.s2);"
NL "    return r;"
NL "}"
NL
NL "ulong3 _viv_madsat_ulong3(ulong3 x, ulong3 y, ulong3 z)"
NL "{"
NL "    ulong3 r;"
NL "    r.s0 = _viv_madsat_ulong(x.s0, y.s0, z.s0);"
NL "    r.s1 = _viv_madsat_ulong(x.s1, y.s1, z.s1);"
NL "    r.s2 = _viv_madsat_ulong(x.s2, y.s2, z.s2);"
NL "    return r;"
NL "}"
NL
NL "long4 _viv_madsat_long4(long4 x, long4 y, long4 z)"
NL "{"
NL "    long4 r;"
NL "    r.s0 = _viv_madsat_long(x.s0, y.s0, z.s0);"
NL "    r.s1 = _viv_madsat_long(x.s1, y.s1, z.s1);"
NL "    r.s2 = _viv_madsat_long(x.s2, y.s2, z.s2);"
NL "    r.s3 = _viv_madsat_long(x.s3, y.s3, z.s3);"
NL "    return r;"
NL "}"
NL
NL "ulong4 _viv_madsat_ulong4(ulong4 x, ulong4 y, ulong4 z)"
NL "{"
NL "    ulong4 r;"
NL "    r.s0 = _viv_madsat_ulong(x.s0, y.s0, z.s0);"
NL "    r.s1 = _viv_madsat_ulong(x.s1, y.s1, z.s1);"
NL "    r.s2 = _viv_madsat_ulong(x.s2, y.s2, z.s2);"
NL "    r.s3 = _viv_madsat_ulong(x.s3, y.s3, z.s3);"
NL "    return r;"
NL "}"
NL
NL "long8 _viv_madsat_long8(long8 x, long8 y, long8 z)"
NL "{"
NL "    long8 r;"
NL "    r.s0 = _viv_madsat_long(x.s0, y.s0, z.s0);"
NL "    r.s1 = _viv_madsat_long(x.s1, y.s1, z.s1);"
NL "    r.s2 = _viv_madsat_long(x.s2, y.s2, z.s2);"
NL "    r.s3 = _viv_madsat_long(x.s3, y.s3, z.s3);"
NL "    r.s4 = _viv_madsat_long(x.s4, y.s4, z.s4);"
NL "    r.s5 = _viv_madsat_long(x.s5, y.s5, z.s5);"
NL "    r.s6 = _viv_madsat_long(x.s6, y.s6, z.s6);"
NL "    r.s7 = _viv_madsat_long(x.s7, y.s7, z.s7);"
NL "    return r;"
NL "}"
NL
NL "ulong8 _viv_madsat_ulong8(ulong8 x, ulong8 y, ulong8 z)"
NL "{"
NL "    ulong8 r;"
NL "    r.s0 = _viv_madsat_ulong(x.s0, y.s0, z.s0);"
NL "    r.s1 = _viv_madsat_ulong(x.s1, y.s1, z.s1);"
NL "    r.s2 = _viv_madsat_ulong(x.s2, y.s2, z.s2);"
NL "    r.s3 = _viv_madsat_ulong(x.s3, y.s3, z.s3);"
NL "    r.s4 = _viv_madsat_ulong(x.s4, y.s4, z.s4);"
NL "    r.s5 = _viv_madsat_ulong(x.s5, y.s5, z.s5);"
NL "    r.s6 = _viv_madsat_ulong(x.s6, y.s6, z.s6);"
NL "    r.s7 = _viv_madsat_ulong(x.s7, y.s7, z.s7);"
NL "    return r;"
NL "}"
NL
NL "long16 _viv_madsat_long16(long16 x, long16 y, long16 z)"
NL "{"
NL "    long16 r;"
NL "    r.s0 = _viv_madsat_long(x.s0, y.s0, z.s0);"
NL "    r.s1 = _viv_madsat_long(x.s1, y.s1, z.s1);"
NL "    r.s2 = _viv_madsat_long(x.s2, y.s2, z.s2);"
NL "    r.s3 = _viv_madsat_long(x.s3, y.s3, z.s3);"
NL "    r.s4 = _viv_madsat_long(x.s4, y.s4, z.s4);"
NL "    r.s5 = _viv_madsat_long(x.s5, y.s5, z.s5);"
NL "    r.s6 = _viv_madsat_long(x.s6, y.s6, z.s6);"
NL "    r.s7 = _viv_madsat_long(x.s7, y.s7, z.s7);"
NL "    r.s8 = _viv_madsat_long(x.s8, y.s8, z.s8);"
NL "    r.s9 = _viv_madsat_long(x.s9, y.s9, z.s9);"
NL "    r.sa = _viv_madsat_long(x.sa, y.sa, z.sa);"
NL "    r.sb = _viv_madsat_long(x.sb, y.sb, z.sb);"
NL "    r.sc = _viv_madsat_long(x.sc, y.sc, z.sc);"
NL "    r.sd = _viv_madsat_long(x.sd, y.sd, z.sd);"
NL "    r.se = _viv_madsat_long(x.se, y.se, z.se);"
NL "    r.sf = _viv_madsat_long(x.sf, y.sf, z.sf);"
NL "    return r;"
NL "}"
NL
NL "ulong16 _viv_madsat_ulong16(ulong16 x, ulong16 y, ulong16 z)"
NL "{"
NL "    ulong16 r;"
NL "    r.s0 = _viv_madsat_ulong(x.s0, y.s0, z.s0);"
NL "    r.s1 = _viv_madsat_ulong(x.s1, y.s1, z.s1);"
NL "    r.s2 = _viv_madsat_ulong(x.s2, y.s2, z.s2);"
NL "    r.s3 = _viv_madsat_ulong(x.s3, y.s3, z.s3);"
NL "    r.s4 = _viv_madsat_ulong(x.s4, y.s4, z.s4);"
NL "    r.s5 = _viv_madsat_ulong(x.s5, y.s5, z.s5);"
NL "    r.s6 = _viv_madsat_ulong(x.s6, y.s6, z.s6);"
NL "    r.s7 = _viv_madsat_ulong(x.s7, y.s7, z.s7);"
NL "    r.s8 = _viv_madsat_ulong(x.s8, y.s8, z.s8);"
NL "    r.s9 = _viv_madsat_ulong(x.s9, y.s9, z.s9);"
NL "    r.sa = _viv_madsat_ulong(x.sa, y.sa, z.sa);"
NL "    r.sb = _viv_madsat_ulong(x.sb, y.sb, z.sb);"
NL "    r.sc = _viv_madsat_ulong(x.sc, y.sc, z.sc);"
NL "    r.sd = _viv_madsat_ulong(x.sd, y.sd, z.sd);"
NL "    r.se = _viv_madsat_ulong(x.se, y.se, z.se);"
NL "    r.sf = _viv_madsat_ulong(x.sf, y.sf, z.sf);"
NL "    return r;"
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

/*****************************
   image descriptor:  uint8
   s0: base address of image data
   s1: stride of a row of the image
   s2: image size
       [15:0]: width (U16)
       [31:16]: height (U16)
   s3: image properties
       [2:0]: shift (for calculation of bpp)
       [3:3]: multiply (for calculation of bpp)
          0: ONE
          1: THREE
       [5:4]: addressing mode
          0: NONE   // Currently aliases to BORDER0.
          1: BORDER0    (0,0,0,0)
          2: BORDER1    (0,0,0,1) or (0,0,0,1.0), depending on image format.
       [9:6]: image format
       [11:10]: tiling mode
          0: LINEAR
          1: TILED
          2: SUPER_TILED
       [12:12]: image type
       [15:14]: component count (packed formats are treated as 1 component)
                0: 4 components
                1: 1 component
                2: 2 components
                3: 3 components
       [18:16]: swizzle_r
       [22:20]: swizzle_g
       [26:24]: swizzle_b
       [30:28]: swizzle_a
   s4: size of an 2-D image in a 2-D image array or slice size of 2-D layer in a 3-D image
   s5: depth of 3-D image or array size of 1-D/2-D array.
   s6: image format
       [0:15]: channel order
       [16:31]: channel data type
****************************/

static gctSTRING gcCLLibImageQuery_Funcs_UseImgInst =
NL "int _viv_get_image_width_image1d_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_width_image2d_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_width_image3d_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_width_image1d_array_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_width_image2d_array_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_width_image1d_buffer_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_height_image2d_t(uint8 image)"
NL "{"
NL "    return (image.s2 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_height_image2d_array_t(uint8 image)"
NL "{"
NL "    return (image.s2 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_height_image3d_t(uint8 image)"
NL "{"
NL "    return (image.s2 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_depth_image3d_t(uint8 image)"
NL "{"
NL "    return image.s5;"
NL "}"
NL
NL "int2 _viv_get_image_dim_image2d_t(uint8 image)"
NL "{"
NL "    return (int2) (image.s2 & 0xFFFF, (image.s2 >> 16) & 0xFFFF);"
NL "}"
NL
NL "int4 _viv_get_image_dim_image3d_t(uint8 image)"
NL "{"
NL "    return (int4) (image.s2 & 0xFFFF, (image.s2 >> 16) & 0xFFFF, image.s5, 0);"
NL "}"
NL
NL "int2 _viv_get_image_dim_image2d_array_t(uint8 image)"
NL "{"
NL "    return (int2) (image.s2 & 0xFFFF, (image.s2 >> 16) & 0xFFFF);"
NL "}"
NL
NL "int _viv_get_image_channel_order_image1d_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_order_image2d_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_order_image3d_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_order_image1d_array_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_order_image2d_array_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_order_image1d_buffer_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image1d_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image2d_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image3d_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image1d_array_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image2d_array_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image1d_buffer_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_get_image_array_size_image1d_array_t(uint8 image)"
NL "{"
NL "    return image.s5;"
NL "}"
NL
NL "int _viv_get_image_array_size_image2d_array_t(uint8 image)"
NL "{"
NL "    return image.s5;"
NL "}"
NL;

/* Description of the image header:
    struct _cl_image_header
    {
        int                  width;
        int                  height;
        int                  depth;
        int                  channelDataType;
        int                  channelOrder;
        int                  samplerValue;
        int                  rowPitch;
        int                  slicePitch;
        int                  textureNum;
        int*                 physical;
     };
*/

static gctSTRING gcCLLibImageQuery_Funcs =
NL "int _viv_get_image_width_image1d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_get_image_width_image2d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_get_image_width_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_get_image_width_image1d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_get_image_width_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_get_image_width_image1d_buffer_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_get_image_height_image2d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[1];"
NL "}"
NL
NL "int _viv_get_image_height_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[1];"
NL "}"
NL
NL "int _viv_get_image_height_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[1];"
NL "}"
NL
NL "int _viv_get_image_depth_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[2];"
NL "}"
NL
NL "int2 _viv_get_image_dim_image2d_t(uint8 image)"
NL "{"
NL "    uint2 *imghdr = (uint2 *) image.s0;"
NL "    return (int2)((*imghdr).x, (*imghdr).y);"
NL "}"
NL
NL "int4 _viv_get_image_dim_image3d_t(uint8 image)"
NL "{"
NL "    uint4 *imghdr = (uint4 *) image.s0;"
NL "    return (int4)((*imghdr).x, (*imghdr).y, (*imghdr).z, 0);"
NL "}"
NL
NL "int2 _viv_get_image_dim_image2d_array_t(uint8 image)"
NL "{"
NL "    uint2 *imghdr = (uint2 *) image.s0;"
NL "    return (int2)((*imghdr).x, (*imghdr).y);"
NL "}"
NL
NL "int _viv_get_image_channel_order_image1d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_get_image_channel_order_image2d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_get_image_channel_order_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_get_image_channel_order_image1d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_get_image_channel_order_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_get_image_channel_order_image1d_buffer_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image1d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image2d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image1d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_get_image_channel_data_type_image1d_buffer_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_get_image_array_size_image1d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[8];"
NL "}"
NL
NL "int _viv_get_image_array_size_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[8];"
NL "}"
NL;
#endif

#undef NL
#ifdef _CL_LOCALLY_SET
#undef __BUILTIN_SHADER_LENGTH__
#endif
#endif /* __gc_vsc_cl_builtin_lib_h_ */

