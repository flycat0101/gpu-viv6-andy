/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include "lib/gc_vsc_lib_common.h"
/* library for gl built-in functions that are written in high level shader */
#include "lib/gc_vsc_lib_gl_builtin.h"

/* library for cl built-in functions that are written in high level shader */
#include "lib/gc_vsc_lib_cl_builtin.h"
#include "lib/gc_vsc_lib_cl_patch.h"

/* libCL strings */
#if !REMOVE_CL_LIBS
gctSTRING gcLibCLImage_ReadFunc_1D =
    READFUNC(1d, )
;
gctSTRING gcLibCLImage_ReadFunc_1D_BGRA =
    READFUNC(1d, _BGRA)
;
gctSTRING gcLibCLImage_ReadFunc_1D_R =
    READFUNC(1d, _R)
;

gctSTRING gcLibCLImage_ReadFunc_1DBUFFER =
    READFUNC(1dbuffer,)
;
gctSTRING gcLibCLImage_ReadFunc_1DBUFFER_BGRA =
    READFUNC(1dbuffer, _BGRA)
;
gctSTRING gcLibCLImage_ReadFunc_1DBUFFER_R =
    READFUNC(1dbuffer, _R)
;

gctSTRING gcLibCLImage_ReadFunc_2D =
    READFUNC(2d,)
;
gctSTRING gcLibCLImage_ReadFunc_2D_BGRA =
    READFUNC(2d, _BGRA)
;
gctSTRING gcLibCLImage_ReadFunc_2D_R =
    READFUNC(2d, _R)
;

gctSTRING gcLibCLImage_ReadFunc_1DARRAY =
    READFUNC(1darray,)
;
gctSTRING gcLibCLImage_ReadFunc_1DARRAY_BGRA =
    READFUNC(1darray, _BGRA)
;
gctSTRING gcLibCLImage_ReadFunc_1DARRAY_R =
    READFUNC(1darray, _R)
;

gctSTRING gcLibCLImage_ReadFunc_2DARRAY =
    READFUNC(2DARRAY,)
;
gctSTRING gcLibCLImage_ReadFunc_2DARRAY_BGRA =
    READFUNC(2DARRAY, _BGRA)
;
gctSTRING gcLibCLImage_ReadFunc_2DARRAY_R =
    READFUNC(2DARRAY, _R)
;

gctSTRING gcLibCLImage_ReadFunc_3D =
    READFUNC(3d,)
;
gctSTRING gcLibCLImage_ReadFunc_3D_BGRA =
    READFUNC(3d, _BGRA)
;
gctSTRING gcLibCLImage_ReadFunc_3D_R =
    READFUNC(3d, _R)
;


gctSTRING gcLibCLImage_ReadFuncF_NORM_1D =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 1d,)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 1d,)
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 1d,)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 1d,)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_1D_BGRA =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 1d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 1d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 1d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 1d, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_1D_R =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 1d, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 1d, _R)
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 1d, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 1d, _R)
;

gctSTRING gcLibCLImage_ReadFuncF_NORM_1DBUFFER =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 1dbuffer,)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 1dbuffer,)
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 1dbuffer,)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 1dbuffer,)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_1DBUFFER_BGRA =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 1dbuffer, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 1dbuffer, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 1dbuffer, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 1dbuffer, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_1DBUFFER_R =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 1dbuffer, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 1dbuffer, _R)
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 1dbuffer, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 1dbuffer, _R)
;

gctSTRING gcLibCLImage_ReadFuncF_NORM_2D =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 2d,)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 2d,)
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 2d,)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 2d,)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_2D_BGRA1 =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 2d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 2d, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_2D_BGRA2 =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 2d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 2d, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_2D_R1 =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 2d, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 2d, _R)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_2D_R2 =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 2d, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 2d, _R)
;

gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY1 =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 1darray,)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 1darray,)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY2 =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 1darray,)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 1darray,)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY1_BGRA =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 1darray, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 1darray, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY2_BGRA =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 1darray, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 1darray, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY1_R =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 1darray, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 1darray, _R)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY2_R =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 1darray, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 1darray, _R)
;


gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY1 =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 2DARRAY,)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 2DARRAY,)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY2 =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 2DARRAY,)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 2DARRAY,)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY1_BGRA =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 2DARRAY, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 2DARRAY, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY2_BGRA =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 2DARRAY, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 2DARRAY, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY1_R =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 2DARRAY, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 2DARRAY, _R)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY2_R =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 2DARRAY, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 2DARRAY, _R)
;


gctSTRING gcLibCLImage_ReadFuncF_NORM_3D0 =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 3d,)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 3d,);
gctSTRING gcLibCLImage_ReadFuncF_NORM_3D1 =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 3d,)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 3d,)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_3D0_BGRA =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 3d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 3d, _BGRA);
gctSTRING gcLibCLImage_ReadFuncF_NORM_3D1_BGRA =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 3d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 3d, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_NORM_3D0_R =
READ_IMAGEF_CHANNEL_TYPE(unorm8, UNORM, uchar4, 255.0, 3d, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm8, SNORM, char4, 127.0, 3d, _R);
gctSTRING gcLibCLImage_ReadFuncF_NORM_3D1_R =
READ_IMAGEF_CHANNEL_TYPE(unorm16, UNORM, ushort4, 65535.0, 3d, _R)
READ_IMAGEF_CHANNEL_TYPE(snorm16, SNORM, short4, 32767.0, 3d, _R)
;


gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1D =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 1d,)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 1d,)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1D_BGRA =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 1d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 1d, _BGRA)
;

gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1D_R =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 1d, _R)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 1d, _R)
;


gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DBUFFER =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 1dbuffer,)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 1dbuffer,)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DBUFFER_BGRA =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 1dbuffer, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 1dbuffer, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DBUFFER_R =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 1dbuffer, _R)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 1dbuffer, _R)
;

gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2D =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 2d,)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 2d,)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2D_BGRA =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 2d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 2d, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2D_R =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 2d, _R)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 2d, _R)
;

gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DARRAY =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 1darray,)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 1darray,)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DARRAY_BGRA =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 1darray, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 1darray, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DARRAY_R =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 1darray, _R)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 1darray, _R)
;

gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2DARRAY =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 2DARRAY,)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 2DARRAY,)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2DARRAY_BGRA =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 2DARRAY, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 2DARRAY, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2DARRAY_R =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 2DARRAY, _R)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 2DARRAY, _R)
;


gctSTRING gcLibCLImage_ReadFuncF_UNNORM_3D =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 3d,)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 3d,)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_3D_BGRA =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 3d, _BGRA)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 3d, _BGRA)
;
gctSTRING gcLibCLImage_ReadFuncF_UNNORM_3D_R =
READ_IMAGEF_CHANNEL_TYPE(float, Float, ignore, ignore, 3d, _R)
READ_IMAGEF_CHANNEL_TYPE(half, Half, ignore, ignore, 3d, _R)
;

gctSTRING gcLibCLImage_WriteFunc =
/* write_imageui */
WRITE_IMAGE(uint4, uchar4, 1d)
WRITE_IMAGE(uint4, ushort4, 1d)
WRITE_IMAGE(uint4, uint4, 1d)
WRITE_IMAGE(uint4, uint4, 1darray)
WRITE_IMAGE(uint4, uchar4, 1darray)
WRITE_IMAGE(uint4, ushort4, 1darray)
WRITE_IMAGE(uint4, uchar4, 2d)
WRITE_IMAGE(uint4, ushort4, 2d)
WRITE_IMAGE(uint4, uint4, 2d)
WRITE_IMAGE(uint4, uchar4, 2DARRAY)
WRITE_IMAGE(uint4, ushort4, 2DARRAY)
WRITE_IMAGE(uint4, uint4, 2DARRAY)
/*WRITE_IMAGE(uint4, uchar4, 3d)
WRITE_IMAGE(uint4, ushort4, 3d)
WRITE_IMAGE(uint4, uint4, 3d)*/

/* write_imagei */
WRITE_IMAGE(int4, char4, 1d)
WRITE_IMAGE(int4, short4, 1d)
WRITE_IMAGE(int4, int4, 1d)
WRITE_IMAGE(int4, char4, 1darray)
WRITE_IMAGE(int4, short4, 1darray)
WRITE_IMAGE(int4, int4, 1darray)
WRITE_IMAGE(int4, char4, 2d)
WRITE_IMAGE(int4, short4, 2d)
WRITE_IMAGE(int4, int4, 2d)
WRITE_IMAGE(int4, char4, 2DARRAY)
WRITE_IMAGE(int4, short4, 2DARRAY)
WRITE_IMAGE(int4, int4, 2DARRAY)
/*WRITE_IMAGE(int4, char4, 3d)
WRITE_IMAGE(int4, short4, 3d)
WRITE_IMAGE(int4, int4, 3d)*/

/* write_imagef */
WRITE_IMAGEF_NORM(uchar4, 255.0, 1d)
WRITE_IMAGEF_NORM(char4, 127.0, 1d)
WRITE_IMAGEF_NORM(ushort4, 65535.0, 1d)
WRITE_IMAGEF_NORM(short4, 32767.0, 1d)
WRITE_IMAGEF_NORM(uchar4, 255.0, 1darray)
WRITE_IMAGEF_NORM(char4, 127.0, 1darray)
WRITE_IMAGEF_NORM(ushort4, 65535.0, 1darray)
WRITE_IMAGEF_NORM(short4, 32767.0, 1darray)
WRITE_IMAGEF_NORM(uchar4, 255.0, 2d)
WRITE_IMAGEF_NORM(char4, 127.0, 2d)
WRITE_IMAGEF_NORM(ushort4, 65535.0, 2d)
WRITE_IMAGEF_NORM(short4, 32767.0, 2d)
WRITE_IMAGEF_NORM(uchar4, 255.0, 2DARRAY)
WRITE_IMAGEF_NORM(char4, 127.0, 2DARRAY)
WRITE_IMAGEF_NORM(ushort4, 65535.0, 2DARRAY)
WRITE_IMAGEF_NORM(short4, 32767.0, 2DARRAY)
/*WRITE_IMAGEF_NORM(uchar4, 255.0, 3d)
WRITE_IMAGEF_NORM(char4, 127.0, 3d)
WRITE_IMAGEF_NORM(ushort4, 65535.0, 3d)
WRITE_IMAGEF_NORM(short4, 32767.0, 3d)*/

"void\n"
"_write_image_float4_float4_1d (\n"
"    uint image, \n"
"    uint imageSize, \n"
"    int  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image ); \n"
"    base[coord] = color; \n"
"} \n"
"void\n"
"_write_image_float4_float4_2d (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    base[coord.x] = color; \n"
"} \n"
"void\n"
"_write_image_float4_float4_1darray (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    base[coord.x] = color; \n"
"} \n"
"void\n"
"_write_image_float4_float4_2DARRAY (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    base[coord.x] = color; \n"
"} \n"
/*"void\n"
"_write_image_float4_float4_3d (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    base[coord.x] = color; \n"
"} \n"*/
"void\n"
"_write_image_float4_half4_1d (\n"
"    uint image, \n"
"    uint imageSize, \n"
"    int  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image); \n"
"    vstore_half4(color, coord, base); \n"
"} \n"
"void\n"
"_write_image_float4_half4_2d (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    vstore_half4(color, coord.x, base); \n"
"} \n"
"void\n"
"_write_image_float4_half4_1darray (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    vstore_half4(color, coord.x, base); \n"
"} \n"
"void\n"
"_write_image_float4_half4_2DARRAY (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    vstore_half4(color, coord.x, base); \n"
"} \n"
/*"void\n"
"_write_image_float4_half4_3d (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    vstore_half4(color, coord.x, base); \n"
"} \n"*/
"void\n"
"_write_image_null_1d (\n"
"    uint image, \n"
"    uint imageSize, \n"
"    int  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"} \n"
"void\n"
"_write_image_null_2d (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"} \n"
"void\n"
"_write_image_null_1darray (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"} \n"
"void\n"
"_write_image_null_2DARRAY (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"} \n";
/*"void\n"
"_write_image_null_3d (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"} \n"*/

gctSTRING gcLibCLImage_WriteFunc_BGRA =
/* write_imageui */
WRITE_IMAGE_BGRA(uint4, uchar4, 1d)
WRITE_IMAGE_BGRA(uint4, ushort4, 1d)
WRITE_IMAGE_BGRA(uint4, uint4, 1d)
WRITE_IMAGE_BGRA(uint4, uint4, 1darray)
WRITE_IMAGE_BGRA(uint4, uchar4, 1darray)
WRITE_IMAGE_BGRA(uint4, ushort4, 1darray)
WRITE_IMAGE_BGRA(uint4, uchar4, 2d)
WRITE_IMAGE_BGRA(uint4, ushort4, 2d)
WRITE_IMAGE_BGRA(uint4, uint4, 2d)
WRITE_IMAGE_BGRA(uint4, uchar4, 2DARRAY)
WRITE_IMAGE_BGRA(uint4, ushort4, 2DARRAY)
WRITE_IMAGE_BGRA(uint4, uint4, 2DARRAY)
/*WRITE_IMAGE_BGRA(uint4, uchar4, 3d)
WRITE_IMAGE_BGRA(uint4, ushort4, 3d)
WRITE_IMAGE_BGRA(uint4, uint4, 3d)*/

/* write_imagei */
WRITE_IMAGE_BGRA(int4, char4, 1d)
WRITE_IMAGE_BGRA(int4, short4, 1d)
WRITE_IMAGE_BGRA(int4, int4, 1d)
WRITE_IMAGE_BGRA(int4, char4, 1darray)
WRITE_IMAGE_BGRA(int4, short4, 1darray)
WRITE_IMAGE_BGRA(int4, int4, 1darray)
WRITE_IMAGE_BGRA(int4, char4, 2d)
WRITE_IMAGE_BGRA(int4, short4, 2d)
WRITE_IMAGE_BGRA(int4, int4, 2d)
WRITE_IMAGE_BGRA(int4, char4, 2DARRAY)
WRITE_IMAGE_BGRA(int4, short4, 2DARRAY)
WRITE_IMAGE_BGRA(int4, int4, 2DARRAY)
/*WRITE_IMAGE_BGRA(int4, char4, 3d)
WRITE_IMAGE_BGRA(int4, short4, 3d)
WRITE_IMAGE_BGRA(int4, int4, 3d)*/

/* write_imagef */
WRITE_IMAGEF_NORM_BGRA(uchar4, 255.0, 1d)
WRITE_IMAGEF_NORM_BGRA(char4, 127.0, 1d)
WRITE_IMAGEF_NORM_BGRA(ushort4, 65535.0, 1d)
WRITE_IMAGEF_NORM_BGRA(short4, 32767.0, 1d)
WRITE_IMAGEF_NORM_BGRA(uchar4, 255.0, 1darray)
WRITE_IMAGEF_NORM_BGRA(char4, 127.0, 1darray)
WRITE_IMAGEF_NORM_BGRA(ushort4, 65535.0, 1darray)
WRITE_IMAGEF_NORM_BGRA(short4, 32767.0, 1darray)
WRITE_IMAGEF_NORM_BGRA(uchar4, 255.0, 2d)
WRITE_IMAGEF_NORM_BGRA(char4, 127.0, 2d)
WRITE_IMAGEF_NORM_BGRA(ushort4, 65535.0, 2d)
WRITE_IMAGEF_NORM_BGRA(short4, 32767.0, 2d)
WRITE_IMAGEF_NORM_BGRA(uchar4, 255.0, 2DARRAY)
WRITE_IMAGEF_NORM_BGRA(char4, 127.0, 2DARRAY)
WRITE_IMAGEF_NORM_BGRA(ushort4, 65535.0, 2DARRAY)
WRITE_IMAGEF_NORM_BGRA(short4, 32767.0, 2DARRAY)
/*WRITE_IMAGEF_NORM_BGRA(uchar4, 255.0, 3d)
WRITE_IMAGEF_NORM_BGRA(char4, 127.0, 3d)
WRITE_IMAGEF_NORM_BGRA(ushort4, 65535.0, 3d)
WRITE_IMAGEF_NORM_BGRA(short4, 32767.0, 3d)*/

"void\n"
"_write_image_float4_float4_BGRA_1d (\n"
"    uint image, \n"
"    uint imageSize, \n"
"    int  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image ); \n"
"    base[coord] = color; \n"
"    base[coord].r = color.b; \n"
"    base[coord].b = color.r; \n"
"} \n"
"void\n"
"_write_image_float4_float4_BGRA_2d (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    base[coord.x] = color; \n"
"    base[coord.x].r = color.b; \n"
"    base[coord.x].b = color.r; \n"
"} \n"
"void\n"
"_write_image_float4_float4_BGRA_1darray (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    base[coord.x] = color; \n"
"    base[coord.x].r = color.b; \n"
"    base[coord.x].b = color.r; \n"
"} \n"
"void\n"
"_write_image_float4_float4_BGRA_2DARRAY (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    base[coord.x] = color; \n"
"    base[coord.x].r = color.b; \n"
"    base[coord.x].b = color.r; \n"
"} \n"
/*"void\n"
"_write_image_float4_float4_BGRA_3d (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    base[coord.x] = color; \n"
"    base[coord.x].r = color.b; \n"
"    base[coord.x].b = color.r; \n"
"} \n"*/
"void\n"
"_write_image_float4_half4_BGRA_1d (\n"
"    uint image, \n"
"    uint imageSize, \n"
"    int  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image); \n"
"    float4 t = color; \n"
"    t.r = color.b; \n"
"    t.b = color.r; \n"
"    vstore_half4(t, coord, base); \n"
"} \n"
"void\n"
"_write_image_float4_half4_BGRA_2d (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    float4 t = color; \n"
"    t.r = color.b; \n"
"    t.b = color.r; \n"
"    vstore_half4(t, coord.x, base); \n"
"} \n"
"void\n"
"_write_image_float4_half4_BGRA_1darray (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    float4 t = color; \n"
"    t.r = color.b; \n"
"    t.b = color.r; \n"
"    vstore_half4(t, coord.x, base); \n"
"} \n"
"void\n"
"_write_image_float4_half4_BGRA_2DARRAY (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    float4 t = color; \n"
"    t.r = color.b; \n"
"    t.b = color.r; \n"
"    vstore_half4(t, coord.x, base); \n"
"} \n";
/*"void\n"
"_write_image_float4_half4_BGRA_3d (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    float4 t = color; \n"
"    t.r = color.b; \n"
"    t.b = color.r; \n"
"    vstore_half4(t, coord.x, base); \n"
"} \n";*/

gctSTRING gcLibCLImage_WriteFunc_R =
/* write_imageui */
WRITE_IMAGE_R(uint4, uchar4, 1d)
WRITE_IMAGE_R(uint4, ushort4, 1d)
WRITE_IMAGE_R(uint4, uint4, 1d)
WRITE_IMAGE_R(uint4, uint4, 1darray)
WRITE_IMAGE_R(uint4, uchar4, 1darray)
WRITE_IMAGE_R(uint4, ushort4, 1darray)
WRITE_IMAGE_R(uint4, uchar4, 2d)
WRITE_IMAGE_R(uint4, ushort4, 2d)
WRITE_IMAGE_R(uint4, uint4, 2d)
WRITE_IMAGE_R(uint4, uchar4, 2DARRAY)
WRITE_IMAGE_R(uint4, ushort4, 2DARRAY)
WRITE_IMAGE_R(uint4, uint4, 2DARRAY)
/*WRITE_IMAGE_R(uint4, uchar4, 3d)
WRITE_IMAGE_R(uint4, ushort4, 3d)
WRITE_IMAGE_R(uint4, uint4, 3d)*/

/* write_imagei */
WRITE_IMAGE_R(int4, char4, 1d)
WRITE_IMAGE_R(int4, short4, 1d)
WRITE_IMAGE_R(int4, int4, 1d)
WRITE_IMAGE_R(int4, char4, 1darray)
WRITE_IMAGE_R(int4, short4, 1darray)
WRITE_IMAGE_R(int4, int4, 1darray)
WRITE_IMAGE_R(int4, char4, 2d)
WRITE_IMAGE_R(int4, short4, 2d)
WRITE_IMAGE_R(int4, int4, 2d)
WRITE_IMAGE_R(int4, char4, 2DARRAY)
WRITE_IMAGE_R(int4, short4, 2DARRAY)
WRITE_IMAGE_R(int4, int4, 2DARRAY)
/*WRITE_IMAGE_R(int4, char4, 3d)
WRITE_IMAGE_R(int4, short4, 3d)
WRITE_IMAGE_R(int4, int4, 3d)*/

/* write_imagef */
WRITE_IMAGEF_NORM_R(uchar4, 255.0, 1d)
WRITE_IMAGEF_NORM_R(char4, 127.0, 1d)
WRITE_IMAGEF_NORM_R(ushort4, 65535.0, 1d)
WRITE_IMAGEF_NORM_R(short4, 32767.0, 1d)
WRITE_IMAGEF_NORM_R(uchar4, 255.0, 1darray)
WRITE_IMAGEF_NORM_R(char4, 127.0, 1darray)
WRITE_IMAGEF_NORM_R(ushort4, 65535.0, 1darray)
WRITE_IMAGEF_NORM_R(short4, 32767.0, 1darray)
WRITE_IMAGEF_NORM_R(uchar4, 255.0, 2d)
WRITE_IMAGEF_NORM_R(char4, 127.0, 2d)
WRITE_IMAGEF_NORM_R(ushort4, 65535.0, 2d)
WRITE_IMAGEF_NORM_R(short4, 32767.0, 2d)
WRITE_IMAGEF_NORM_R(uchar4, 255.0, 2DARRAY)
WRITE_IMAGEF_NORM_R(char4, 127.0, 2DARRAY)
WRITE_IMAGEF_NORM_R(ushort4, 65535.0, 2DARRAY)
WRITE_IMAGEF_NORM_R(short4, 32767.0, 2DARRAY)
/*WRITE_IMAGEF_NORM_R(uchar4, 255.0, 3d)
WRITE_IMAGEF_NORM_R(char4, 127.0, 3d)
WRITE_IMAGEF_NORM_R(ushort4, 65535.0, 3d)
WRITE_IMAGEF_NORM_R(short4, 32767.0, 3d)*/

"void\n"
"_write_image_float4_float4_R_1d (\n"
"    uint image, \n"
"    uint imageSize, \n"
"    int  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float * base = (float *) ((uchar *)image ); \n"
"    base[coord] = color.r; \n"
"} \n"
"void\n"
"_write_image_float4_float4_R_2d (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float * base = (float *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    base[coord.x] = color.r; \n"
"} \n"
"void\n"
"_write_image_float4_float4_R_1darray (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float * base = (float *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    base[coord.x] = color.r; \n"
"} \n"
"void\n"
"_write_image_float4_float4_R_2DARRAY (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float * base = (float *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    base[coord.x] = color.r; \n"
"} \n"
/*"void\n"
"_write_image_float4_float4_R_3d (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    float * base = (float *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    base[coord.x] = color.r; \n"
"} \n"*/
"void\n"
"_write_image_float4_half4_R_1d (\n"
"    uint image, \n"
"    uint imageSize, \n"
"    int  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image); \n"
"    vstore_half(color.x, coord, base); \n"
"} \n"
"void\n"
"_write_image_float4_half4_R_2d (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    vstore_half(color.r, coord.x, base); \n"
"} \n"
"void\n"
"_write_image_float4_half4_R_1darray (\n"
"    uint2 image, \n"
"    uint2 imageSize, \n"
"    int2  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
"    vstore_half(color.r, coord.x, base); \n"
"} \n"
"void\n"
"_write_image_float4_half4_R_2DARRAY (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    vstore_half(color.r, coord.x, base); \n"
"} \n";
/*"void\n"
"_write_image_float4_half4_R_3d (\n"
"    uint3 image, \n"
"    uint3 imageSize, \n"
"    int3  coord, \n"
"    float4 color \n"
"    ) \n"
"{ \n"
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n"
"    vstore_half(color.r, coord.x, base); \n"
"} \n";*/

gctSTRING gcLibCLPatch_MainFunc =
"/* Testing function, not real. */ \n"
"__kernel void sampleKernel(\n"
"    read_only image2d_t input, \n"
"    sampler_t imageSampler, \n"
"    __global uint4 *results) \n"
"{ \n"
"    uint2 imageSize = {64, 64}; \n"
"    int2 coords = {0.0, 0.0}; \n"
"\n"
"    results[0] = _read_image_nearest_unnorm_none_intcoord_ui_uint32_2d(imageSize, as_int2(imageSize), coords); \n"
"} \n";


#if _SUPPORT_LONG_ULONG_DATA_TYPE
gctSTRING    gcLibCLLong_Func =
    _longulong_left_shift_long
    _longulong_left_shift_ulong
    _longulong_right_shift_ulong
    _longulong_right_shift_long
    _longulong_add_long
    _longulong_add_ulong
    longulong_sub
    _longulong_div_ulong
    _longulong_mulhi_ulong
    _longulong_mul_long
    _longulong_mul_ulong
    _longulong_jmp_long
    _longulong_jmp_ulong
    _longulong_cmp_nz_long
    _longulong_cmp_nz_ulong
    _longulong_cmp_z_long
    _longulong_cmp_z_ulong
    _longulong_abs
    _longulong_greater_long
    _longulong_addsat_long
    _longulong_greater_ulong
    _longulong_addsat_ulong
    _longulong_clz_long
    _longulong_clz_ulong
    _longulong_jmp_lessEqual_long
    _longulong_jmp_lessEqual_ulong
    _longulong_cmp_lessEqual_long
    _longulong_cmp_lessEqual_ulong
    _longulong_jmp_greaterEqual_long
    _longulong_jmp_greaterEqual_ulong
    _longulong_cmp_greaterEqual_long
    _longulong_cmp_greaterEqual_ulong
    _longulong_jmp_less_long
    _longulong_jmp_less_ulong
    _longulong_cmp_less_long
    _longulong_cmp_less_ulong
    _longulong_jmp_greater_long
    _longulong_jmp_greater_ulong
    _longulong_cmp_greater_long
    _longulong_cmp_greater_ulong
    _longulong_jmp_equal_long
    _longulong_jmp_equal_ulong
    _longulong_cmp_equal_long
    _longulong_cmp_equal_ulong
    _longulong_jmp_notequal_long
    _longulong_jmp_notequal_ulong
    _longulong_cmp_notequal_long
    _longulong_cmp_notequal_ulong;

gctSTRING    gcLibCLLong_Func1 =
    _longulong_mad
    _longulong_f2i_ulong
    _longulong_i2f
    _long2char_convert_sat
    _ulong2char_convert_sat
    _long2uchar_convert_sat
    _ulong2uchar_convert_sat
    _long2short_convert_sat
    _ulong2short_convert_sat
    _long2ushort_convert_sat
    _ulong2ushort_convert_sat
    _long2int_convert_sat
    _ulong2int_convert_sat
    _long2uint_convert_sat
    _ulong2uint_convert_sat
    _long2ulong_convert_sat
    _ulong2long_convert_sat
    _longulong_rotate
    _longulong_popcount
    _long_max
    _ulong_max
    _long_min
    _ulong_min;

gctSTRING    gcLibCLLong_Func2 =
    _longulong_subsat_long
    _longulong_subsat_ulong
    _longulong_f2i_long
    _longulong_left_shift_long_scalar
    _longulong_left_shift_ulong_scalar;
#endif

/****************************************************************************************
new read_image & write_image
****************************************************************************************/

/* common code */
gctSTRING gcLibCL_ReadImage_Header_Str =
"#pragma OPENCL EXTENSION  CL_VIV_asm : enable\n";

gctSTRING gcLibCL_ReadImage_Common_Func_Str =
"uint _viv_getImageAddr(uint8 imageDSP)\n"
"{\n"
"    return imageDSP.s0;\n"
"}\n"
"uint _viv_getImage3DSliceStride(uint8 imageDSP)\n"
"{\n"
"    return imageDSP.s4;\n"
"}\n"
"int3 _viv_getImageDimensions(uint8 imageDSP)\n"
"{\n"
"    int3 result;\n"
"    result.x = imageDSP.s2 & 0xffff;\n"
"    result.y = (imageDSP.s2 >> 16) & 0xffff;\n"
"    result.z = imageDSP.s5;\n"
"    return result;\n"
"}\n"
"uint _viv_get3DImageNewBaseAddr(uint8 imageDSP, int3 coord)\n"
"{\n"
"    return _viv_getImageAddr(imageDSP) + _viv_getImage3DSliceStride(imageDSP) * coord.s2;\n"
"}\n"
"bool _viv_isSamplerNormalized(sampler_t sampler)\n"
"{\n"
"    uint temp;\n"
"    _viv_asm(ASSIGN, temp, sampler);\n"
"    return temp & 0x10000;\n"
"}\n"
"bool _viv_isSamplerFilterModeLinear(sampler_t sampler)\n"
"{\n"
"    uint temp;\n"
"    _viv_asm(ASSIGN, temp, sampler);\n"
"    return temp & 0x100;\n"
"}\n"
"uint _viv_getSamplerAddressingMode(sampler_t sampler)\n"
"{\n"
"    uint temp;\n"
"    _viv_asm(ASSIGN, temp, sampler);\n"
"    return temp & 0x7;\n"
"}\n"
"float3 _viv_getUnnormalizedCoord(uint8 imageDSP, sampler_t sampler, float3 coord)\n"
"{\n"
"    if(_viv_isSamplerNormalized(sampler))\n"
"    {\n"
"        return coord * convert_float3(_viv_getImageDimensions(imageDSP));\n"
"    }\n"
"    return coord;\n"
"}\n"
"int3 _viv_getNearestCoordFromIntegerCoord(uint8 imageDSP, sampler_t sampler, int3 coord)\n"
"{\n"
"    int3 dimentions = _viv_getImageDimensions(imageDSP);\n"
"    if(_viv_getSamplerAddressingMode(sampler) == 0x1)                /* edge */\n"
"    {\n"
"        return clamp(coord, 0, dimentions - 1);\n"
"    }\n"
"    uint imageType;\n"
"    _viv_asm(GET_IMAGE_T_TYPE, imageType, imageDSP.s0123);\n"
"    if(imageType == 2)                                     /* 1d_array */\n"
"    {\n"
"        coord.s1 = clamp(coord.s1, 0, dimentions.s1 - 1);\n"
"    }\n"
"    if(imageType == 4)                                     /* 2d_array */\n"
"    {\n"
"        coord.s2 = clamp(coord.s2, 0, dimentions.s2 - 1);\n"
"    }\n"
"    return coord;\n"
"}\n"
"int3 _viv_getNearestCoordFromFloatCoord(uint8 imageDSP, sampler_t sampler, float3 coord)\n"
"{\n"
"    int3 result;\n"
"    float3 unnormalizedCoord = _viv_getUnnormalizedCoord(imageDSP, sampler, coord);\n"
"    int3 dimentions = _viv_getImageDimensions(imageDSP);\n"
"    switch(_viv_getSamplerAddressingMode(sampler))\n"
"    {\n"
"        case 0x3:                                                /* repeat */\n"
"        {\n"
"            result.s0 = convert_int(floor((coord.s0 - floor(coord.s0)) * dimentions.s0));\n"
"            if(result.s0 > dimentions.s0 - 1)\n"
"            {\n"
"                result.s0 = result.s0 - dimentions.s0;\n"
"            }\n"
"            result.s1 = convert_int(floor((coord.s1 - floor(coord.s1)) * dimentions.s1));\n"
"            if(result.s1 > dimentions.s1 - 1)\n"
"            {\n"
"                result.s1 = result.s1 - dimentions.s1;\n"
"            }\n"
"            result.s2 = convert_int(floor((coord.s2 - floor(coord.s2)) * dimentions.s2));\n"
"            if(result.s2 > dimentions.s2 - 1)\n"
"            {\n"
"                result.s2 = result.s2 - dimentions.s2;\n"
"            }\n"
"            break;\n"
"        }\n"
"        case 0x4:                                                /* mirrored repeat */\n"
"        {\n"
"            float3 temp, temp2;\n"
"            temp.s0 = 2.0 * rint(0.5 * coord.s0);\n"
"            temp.s0 = fabs(coord.s0 - temp.s0);\n"
"            temp2.s0 = temp.s0 * (float)dimentions.s0;\n"
"            result.s0 = min((int)floor(temp2.s0), dimentions.s0 - 1);\n"
"            temp.s1 = 2.0 * rint(0.5 * coord.s1);\n"
"            temp.s1 = fabs(coord.s1 - temp.s1);\n"
"            temp2.s1 = temp.s1 * (float)dimentions.s1;\n"
"            result.s1 = min((int)floor(temp2.s1), dimentions.s1 - 1);\n"
"            temp.s2 = 2.0 * rint(0.5 * coord.s2);\n"
"            temp.s2 = fabs(coord.s2 - temp.s2);\n"
"            temp2.s2 = temp.s2 * (float)dimentions.s2;\n"
"            result.s2 = min((int)floor(temp2.s2), dimentions.s2 - 1);\n"
"            break;\n"
"        }\n"
"        case 0x2:                                                /* boarder */\n"
"        {\n"
"            result = convert_int3(floor(unnormalizedCoord));\n"
"            break;\n"
"        }\n"
"        case 0x1:                                                /* edge */\n"
"        {\n"
"            result = clamp(convert_int3(floor(unnormalizedCoord)), 0, dimentions - 1);\n"
"            break;\n"
"        }\n"
"    }\n"
"    uint imageType;\n"
"    _viv_asm(GET_IMAGE_T_TYPE, imageType, imageDSP.s0123);\n"
"    if(imageType == 2)                                     /* 1d_array */\n"
"    {\n"
"        result.s1 = clamp(convert_int(rint(unnormalizedCoord.s1)), 0, dimentions.s1 - 1);\n"
"    }\n"
"    if(imageType == 4)                                     /* 2d_array */\n"
"    {\n"
"        result.s2 = clamp(convert_int(rint(unnormalizedCoord.s2)), 0, dimentions.s2 - 1);\n"
"    }\n"
"    return result;\n"
"}\n"
"typedef struct VIV_INTCOORDANDOFFSET {\n"
"    int3 intCoord0;\n"
"    int3 intCoord1;\n"
"    float3 offset;\n"
"} VIV_IntCoordAndOffset;\n"
"VIV_IntCoordAndOffset _viv_getLinearCoordsAndOffset(uint8 imageDSP, sampler_t sampler, float3 coord)\n"
"{\n"
"    VIV_IntCoordAndOffset result;\n"
"    float3 unnormalizedCoord = _viv_getUnnormalizedCoord(imageDSP, sampler, coord);\n"
"    int3 dimentions = _viv_getImageDimensions(imageDSP);\n"
"    switch(_viv_getSamplerAddressingMode(sampler))\n"
"    {\n"
"        case 0x3:                                                /* repeat */\n"
"        {\n"
"            float3 tempCoord;\n"
"            tempCoord.s0 = (coord.s0 - floor(coord.s0)) * (float)dimentions.s0;\n"
"            result.intCoord0.s0 = (int)floor(tempCoord.s0 - 0.5);\n"
"            result.intCoord1.s0 = result.intCoord0.s0 + 1;\n"
"            if(result.intCoord0.s0 < 0)\n"
"            {\n"
"                result.intCoord0.s0 += dimentions.s0;\n"
"            }\n"
"            if(result.intCoord1.s0 > dimentions.s0 - 1)\n"
"            {\n"
"                result.intCoord1.s0 -= dimentions.s0;\n"
"            }\n"
"            tempCoord.s1 = (coord.s1 - floor(coord.s1)) * (float)dimentions.s1;\n"
"            result.intCoord0.s1 = (int)floor(tempCoord.s1 - 0.5);\n"
"            result.intCoord1.s1 = result.intCoord0.s1 + 1;\n"
"            if(result.intCoord0.s1 < 0)\n"
"            {\n"
"                result.intCoord0.s1 += dimentions.s1;\n"
"            }\n"
"            if(result.intCoord1.s1 > dimentions.s1 - 1)\n"
"            {\n"
"                result.intCoord1.s1 -= dimentions.s1;\n"
"            }\n"
"            tempCoord.s2 = (coord.s2 - floor(coord.s2)) * (float)dimentions.s2;\n"
"            result.intCoord0.s2 = (int)floor(tempCoord.s2 - 0.5);\n"
"            result.intCoord1.s2 = result.intCoord0.s2 + 1;\n"
"            if(result.intCoord0.s2 < 0)\n"
"            {\n"
"                result.intCoord0.s2 += dimentions.s2;\n"
"            }\n"
"            if(result.intCoord1.s2 > dimentions.s2 - 1)\n"
"            {\n"
"                result.intCoord1.s2 -= dimentions.s2;\n"
"            }\n"
"            _viv_asm(FRACT!<rnd:RTNE>, result.offset, tempCoord - (float3)0.5);\n"
"            break;\n"
"        }\n"
"        case 0x4:                                                /* mirrored repeat */\n"
"        {\n"
"            float3 tempCoord;\n"
"            tempCoord.s0 = fabs(coord.s0 - 2.0 * rint(0.5 * coord.s0)) * (float)dimentions.s0;\n"
"            result.intCoord0.s0 = (int)floor(tempCoord.s0 - 0.5);\n"
"            result.intCoord1.s0 = result.intCoord0.s0 + 1;\n"
"            result.intCoord0.s0 = max(result.intCoord0.s0, 0);\n"
"            result.intCoord1.s0 = min(result.intCoord1.s0, dimentions.s0 - 1);\n"
"            tempCoord.s1 = fabs(coord.s1 - 2.0 * rint(0.5 * coord.s1)) * (float)dimentions.s1;\n"
"            result.intCoord0.s1 = (int)floor(tempCoord.s1 - 0.5);\n"
"            result.intCoord1.s1 = result.intCoord0.s1 + 1;\n"
"            result.intCoord0.s1 = max(result.intCoord0.s1, 0);\n"
"            result.intCoord1.s1 = min(result.intCoord1.s1, dimentions.s1 - 1);\n"
"            tempCoord.s2 = fabs(coord.s2 - 2.0 * rint(0.5 * coord.s2)) * (float)dimentions.s2;\n"
"            result.intCoord0.s2 = (int)floor(tempCoord.s2 - 0.5);\n"
"            result.intCoord1.s2 = result.intCoord0.s2 + 1;\n"
"            result.intCoord0.s2 = max(result.intCoord0.s2, 0);\n"
"            result.intCoord1.s2 = min(result.intCoord1.s2, dimentions.s2 - 1);\n"
"            _viv_asm(FRACT!<rnd:RTNE>, result.offset, tempCoord - (float3)0.5);\n"
"            break;\n"
"        }\n"
"        case 0x2:                                                /* border */\n"
"        {\n"
"            result.intCoord0.s0 = clamp(convert_int(floor(unnormalizedCoord.s0 - 0.5)), -1, dimentions.s0);\n"
"            result.intCoord0.s1 = clamp(convert_int(floor(unnormalizedCoord.s1 - 0.5)), -1, dimentions.s1);\n"
"            result.intCoord0.s2 = clamp(convert_int(floor(unnormalizedCoord.s2 - 0.5)), -1, dimentions.s2);\n"
"            result.intCoord1.s0 = clamp(convert_int(floor(unnormalizedCoord.s0 - 0.5)) + 1, -1, dimentions.s0);\n"
"            result.intCoord1.s1 = clamp(convert_int(floor(unnormalizedCoord.s1 - 0.5)) + 1, -1, dimentions.s1);\n"
"            result.intCoord1.s2 = clamp(convert_int(floor(unnormalizedCoord.s2 - 0.5)) + 1, -1, dimentions.s2);\n"
"            _viv_asm(FRACT!<rnd:RTNE>, result.offset, unnormalizedCoord - (float3)0.5);\n"
"            break;\n"
"        }\n"
"        case 0x1:                                                /* edge */\n"
"        {\n"
"            result.intCoord0.s0 = clamp(convert_int(floor(unnormalizedCoord.s0 - 0.5)), 0, dimentions.s0 - 1);\n"
"            result.intCoord0.s1 = clamp(convert_int(floor(unnormalizedCoord.s1 - 0.5)), 0, dimentions.s1 - 1);\n"
"            result.intCoord0.s2 = clamp(convert_int(floor(unnormalizedCoord.s2 - 0.5)), 0, dimentions.s2 - 1);\n"
"            result.intCoord1.s0 = clamp(convert_int(floor(unnormalizedCoord.s0 - 0.5)) + 1, 0, dimentions.s0 - 1);\n"
"            result.intCoord1.s1 = clamp(convert_int(floor(unnormalizedCoord.s1 - 0.5)) + 1, 0, dimentions.s1 - 1);\n"
"            result.intCoord1.s2 = clamp(convert_int(floor(unnormalizedCoord.s2 - 0.5)) + 1, 0, dimentions.s2 - 1);\n"
"            _viv_asm(FRACT!<rnd:RTNE>, result.offset, unnormalizedCoord - (float3)0.5);\n"
"            break;\n"
"        }\n"
"    }\n"
"    uint imageType;\n"
"    _viv_asm(GET_IMAGE_T_TYPE, imageType, imageDSP.s0123);\n"
"    if(imageType == 2)                                     /* 1d_array */\n"
"    {\n"
"        result.intCoord0.s1 = clamp(convert_int(rint(unnormalizedCoord.s1)), 0, dimentions.s1 - 1);\n"
"        result.intCoord1.s1 = clamp(convert_int(rint(unnormalizedCoord.s1)), 0, dimentions.s1 - 1);\n"
"    }\n"
"    if(imageType == 4)                                     /* 2d_array */\n"
"    {\n"
"        result.intCoord0.s2 = clamp(convert_int(rint(unnormalizedCoord.s2)), 0, dimentions.s2 - 1);\n"
"        result.intCoord1.s2 = clamp(convert_int(rint(unnormalizedCoord.s2)), 0, dimentions.s2 - 1);\n"
"    }\n"
"    return result;\n"
"}\n"
"float4 _viv_readImage1dCoordF(uint8 imageDSP, sampler_t sampler, float3 coord)\n"
"{\n"
"    uint imageType;\n"
"    _viv_asm(GET_IMAGE_T_TYPE, imageType, imageDSP.s0123);\n"
"    float4 result;\n"
"    if(!_viv_isSamplerFilterModeLinear(sampler))                /* nearest */\n"
"    {\n"
"        int3 intCoord0 = _viv_getNearestCoordFromFloatCoord(imageDSP, sampler, coord);\n"
"        if(imageType != 2)                                     /* 1d_array */\n"
"        {\n"
"            intCoord0.s1 = 0;\n"
"        }\n"
"        _viv_asm(IMAGE_READ, result, imageDSP, intCoord0.s01);\n"
"    }\n"
"    else                                                       /* linear */\n"
"    {\n"

"        VIV_IntCoordAndOffset intCoordAndOffset;\n"
"        intCoordAndOffset = _viv_getLinearCoordsAndOffset(imageDSP, sampler, coord);\n"
"        float4 result0, result1;\n"
"        if(imageType != 2)                                     /* 1d_array */\n"
"        {\n"
"            intCoordAndOffset.intCoord0.s1 = 0;\n"
"            intCoordAndOffset.intCoord1.s1 = 0;\n"
"        }\n"
"        _viv_asm(IMAGE_READ, result0, imageDSP, intCoordAndOffset.intCoord0.s01);\n"
"        _viv_asm(IMAGE_READ, result1, imageDSP, intCoordAndOffset.intCoord1.s01);\n"
"        result = (1 - intCoordAndOffset.offset.s0) * result0 +\n"
"                 intCoordAndOffset.offset.s0 * result1;\n"
"    }\n"
"    return result;\n"
"}\n"
"float4 _viv_readImage2dCoordF(uint8 imageDSP, sampler_t sampler, float3 coord)\n"
"{\n"
"    float4 result;\n"
"    uint imageType;\n"
"    _viv_asm(GET_IMAGE_T_TYPE, imageType, imageDSP.s0123);\n"
"    if(!_viv_isSamplerFilterModeLinear(sampler))                /* nearest */\n"
"    {\n"
"        int3 intCoord0 = _viv_getNearestCoordFromFloatCoord(imageDSP, sampler, coord);\n"
"        if(imageType == 4)                                     /* 2d_array */\n"
"        {\n"
"            intCoord0.s2 = _viv_get3DImageNewBaseAddr(imageDSP, intCoord0);\n"
"            _viv_asm(IMAGE_READ_3D, result, imageDSP, intCoord0);\n"
"        }\n"
"        else\n"
"        {\n"
"            _viv_asm(IMAGE_READ, result, imageDSP, intCoord0.s01);\n"
"        }\n"
"    }\n"
"    else                                                       /* linear */\n"
"    {\n"
"        VIV_IntCoordAndOffset intCoordAndOffset;\n"
"        intCoordAndOffset = _viv_getLinearCoordsAndOffset(imageDSP, sampler, coord);\n"
"        float4 result0, result1, result2, result3;\n"
"        if(imageType == 4)                                     /* 2d_array */\n"
"        {\n"
"            int3 tempCoord;\n"
"            uint newBase = _viv_get3DImageNewBaseAddr(imageDSP, intCoordAndOffset.intCoord0);\n"
"            tempCoord = (int3)(intCoordAndOffset.intCoord0.s0, intCoordAndOffset.intCoord0.s1, newBase);\n"
"            _viv_asm(IMAGE_READ_3D, result0, imageDSP, tempCoord);\n"
"            tempCoord = (int3)(intCoordAndOffset.intCoord1.s0, intCoordAndOffset.intCoord0.s1, newBase);\n"
"            _viv_asm(IMAGE_READ_3D, result1, imageDSP, tempCoord);\n"
"            tempCoord = (int3)(intCoordAndOffset.intCoord0.s0, intCoordAndOffset.intCoord1.s1, newBase);\n"
"            _viv_asm(IMAGE_READ_3D, result2, imageDSP, tempCoord);\n"
"            tempCoord = (int3)(intCoordAndOffset.intCoord1.s0, intCoordAndOffset.intCoord1.s1, newBase);\n"
"            _viv_asm(IMAGE_READ_3D, result3, imageDSP, tempCoord);\n"
"        }\n"
"        else\n"
"        {\n"
"            _viv_asm(IMAGE_READ, result0, imageDSP, intCoordAndOffset.intCoord0.s01);\n"
"            _viv_asm(IMAGE_READ, result1, imageDSP, (int2)(intCoordAndOffset.intCoord1.s0, intCoordAndOffset.intCoord0.s1));\n"
"            _viv_asm(IMAGE_READ, result2, imageDSP, (int2)(intCoordAndOffset.intCoord0.s0, intCoordAndOffset.intCoord1.s1));\n"
"            _viv_asm(IMAGE_READ, result3, imageDSP, intCoordAndOffset.intCoord1.s01);\n"
"        }\n"
"        result = (1 - intCoordAndOffset.offset.s0) * (1 - intCoordAndOffset.offset.s1) * result0 +\n"
"                 intCoordAndOffset.offset.s0 * (1 - intCoordAndOffset.offset.s1) * result1 +\n"
"                 (1 - intCoordAndOffset.offset.s0) * intCoordAndOffset.offset.s1 * result2 +\n"
"                 intCoordAndOffset.offset.s0 * intCoordAndOffset.offset.s1 * result3;\n"
"    }\n"
"    return result;\n"
"}\n"
"float4 _viv_readImage3dCoordF(uint8 imageDSP, sampler_t sampler, float3 coord)\n"
"{\n"
"    float4 result;\n"
"    if(!_viv_isSamplerFilterModeLinear(sampler))                /* nearest */\n"
"    {\n"
"        int3 intCoord0 = _viv_getNearestCoordFromFloatCoord(imageDSP, sampler, coord);\n"
"        intCoord0.s2 = _viv_get3DImageNewBaseAddr(imageDSP, intCoord0);\n"
"        _viv_asm(IMAGE_READ_3D, result, imageDSP, intCoord0);\n"
"    }\n"
"    else                                                       /* linear */\n"
"    {\n"
"        float4 result0, result1, result2, result3, result4, result5, result6, result7;\n"
"        VIV_IntCoordAndOffset intCoordAndOffset = _viv_getLinearCoordsAndOffset(imageDSP, sampler, coord);\n"
"        int3 tempCoord;\n"
"        tempCoord = intCoordAndOffset.intCoord0;\n"
"        tempCoord.s2 = _viv_get3DImageNewBaseAddr(imageDSP, tempCoord);\n"
"        _viv_asm(IMAGE_READ_3D, result0, imageDSP, tempCoord);\n"
"        tempCoord = (int3)(intCoordAndOffset.intCoord1.s0, intCoordAndOffset.intCoord0.s1, intCoordAndOffset.intCoord0.s2);\n"
"        tempCoord.s2 = _viv_get3DImageNewBaseAddr(imageDSP, tempCoord);\n"
"        _viv_asm(IMAGE_READ_3D, result1, imageDSP, tempCoord);\n"
"        tempCoord = (int3)(intCoordAndOffset.intCoord0.s0, intCoordAndOffset.intCoord1.s1, intCoordAndOffset.intCoord0.s2);\n"
"        tempCoord.s2 = _viv_get3DImageNewBaseAddr(imageDSP, tempCoord);\n"
"        _viv_asm(IMAGE_READ_3D, result2, imageDSP, tempCoord);\n"
"        tempCoord = (int3)(intCoordAndOffset.intCoord1.s0, intCoordAndOffset.intCoord1.s1, intCoordAndOffset.intCoord0.s2);\n"
"        tempCoord.s2 = _viv_get3DImageNewBaseAddr(imageDSP, tempCoord);\n"
"        _viv_asm(IMAGE_READ_3D, result3, imageDSP, tempCoord);\n"
"        tempCoord = (int3)(intCoordAndOffset.intCoord0.s0, intCoordAndOffset.intCoord0.s1, intCoordAndOffset.intCoord1.s2);\n"
"        tempCoord.s2 = _viv_get3DImageNewBaseAddr(imageDSP, tempCoord);\n"
"        _viv_asm(IMAGE_READ_3D, result4, imageDSP, tempCoord);\n"
"        tempCoord = (int3)(intCoordAndOffset.intCoord1.s0, intCoordAndOffset.intCoord0.s1, intCoordAndOffset.intCoord1.s2);\n"
"        tempCoord.s2 = _viv_get3DImageNewBaseAddr(imageDSP, tempCoord);\n"
"        _viv_asm(IMAGE_READ_3D, result5, imageDSP, tempCoord);\n"
"        tempCoord = (int3)(intCoordAndOffset.intCoord0.s0, intCoordAndOffset.intCoord1.s1, intCoordAndOffset.intCoord1.s2);\n"
"        tempCoord.s2 = _viv_get3DImageNewBaseAddr(imageDSP, tempCoord);\n"
"        _viv_asm(IMAGE_READ_3D, result6, imageDSP, tempCoord);\n"
"        tempCoord = intCoordAndOffset.intCoord1;\n"
"        tempCoord.s2 = _viv_get3DImageNewBaseAddr(imageDSP, tempCoord);\n"
"        _viv_asm(IMAGE_READ_3D, result7, imageDSP, tempCoord);\n"
"        result = (1 - intCoordAndOffset.offset.s0) * (1 - intCoordAndOffset.offset.s1) * (1 - intCoordAndOffset.offset.s2) * result0 +\n"
"                 intCoordAndOffset.offset.s0 * (1 - intCoordAndOffset.offset.s1) * (1 - intCoordAndOffset.offset.s2) * result1 +\n"
"                 (1 - intCoordAndOffset.offset.s0) * intCoordAndOffset.offset.s1 * (1 - intCoordAndOffset.offset.s2) * result2 +\n"
"                 intCoordAndOffset.offset.s0 * intCoordAndOffset.offset.s1 * (1 - intCoordAndOffset.offset.s2) * result3 +\n"
"                 (1 - intCoordAndOffset.offset.s0) * (1 - intCoordAndOffset.offset.s1) * intCoordAndOffset.offset.s2 * result4 +\n"
"                 intCoordAndOffset.offset.s0 * (1 - intCoordAndOffset.offset.s1) * intCoordAndOffset.offset.s2 * result5 +\n"
"                 (1 - intCoordAndOffset.offset.s0) * intCoordAndOffset.offset.s1 * intCoordAndOffset.offset.s2 * result6 +\n"
"                 intCoordAndOffset.offset.s0 * intCoordAndOffset.offset.s1 * intCoordAndOffset.offset.s2 * result7;\n"
"    }\n"
"    return result;\n"
"}\n"
;


gctSTRING gcLibCL_ReadImage_With_IMGLD_Funcs =
    READ_IMAGE_WITH_IMGLD_1D_COORD_TYPE_int3(float4)
    READ_IMAGE_WITH_IMGLD_1D_COORD_TYPE_int3(int4)
    READ_IMAGE_WITH_IMGLD_1D_COORD_TYPE_int3(uint4)
    READ_IMAGE_WITH_IMGLD_2D_COORD_TYPE_int3(float4)
    READ_IMAGE_WITH_IMGLD_2D_COORD_TYPE_int3(int4)
    READ_IMAGE_WITH_IMGLD_2D_COORD_TYPE_int3(uint4)
    READ_IMAGE_WITH_IMGLD_3D_COORD_TYPE_int3(float4)
    READ_IMAGE_WITH_IMGLD_3D_COORD_TYPE_int3(int4)
    READ_IMAGE_WITH_IMGLD_3D_COORD_TYPE_int3(uint4)
    READ_IMAGEF_WITH_IMGLD_1D_COORD_TYPE_float3
    READ_IMAGEIUI_WITH_IMGLD_1D_COORD_TYPE_float3(int4)
    READ_IMAGEIUI_WITH_IMGLD_1D_COORD_TYPE_float3(uint4)
    READ_IMAGEF_WITH_IMGLD_2D_COORD_TYPE_float3
    READ_IMAGEIUI_WITH_IMGLD_2D_COORD_TYPE_float3(int4)
    READ_IMAGEIUI_WITH_IMGLD_2D_COORD_TYPE_float3(uint4)
    READ_IMAGEF_WITH_IMGLD_3D_COORD_TYPE_float3
    READ_IMAGEIUI_WITH_IMGLD_3D_COORD_TYPE_float3(int4)
    READ_IMAGEIUI_WITH_IMGLD_3D_COORD_TYPE_float3(uint4)
    ;

gctSTRING gcLibCL_ReadImage_With_TEXLDU_Funcs =
    READ_IMAGE_WITH_TEXLDU(float4, float3)
    READ_IMAGE_WITH_TEXLDU(float4, int3)
    READ_IMAGE_WITH_TEXLDU(int4, float3)
    READ_IMAGE_WITH_TEXLDU(int4, int3)
    READ_IMAGE_WITH_TEXLDU(uint4, float3)
    READ_IMAGE_WITH_TEXLDU(uint4, int3);

gctSTRING gcLibCL_ReadImage_With_V55_TEXLDU_Funcs =
"";

gctSTRING gcLibCL_ReadImage_With_TEXLD_Funcs =
"";

gctSTRING gcLibCL_WriteImage_With_IMGST_Funcs =
    WRITE_IMAGE_WITH_IMGST(1d, int4)
    WRITE_IMAGE_WITH_IMGST(1d, uint4)
    WRITE_IMAGE_WITH_IMGST(1d, float4)
    WRITE_IMAGE_WITH_IMGST(2d, int4)
    WRITE_IMAGE_WITH_IMGST(2d, uint4)
    WRITE_IMAGE_WITH_IMGST(2d, float4)
    WRITE_IMAGE_WITH_IMGST(3d, int4)
    WRITE_IMAGE_WITH_IMGST(3d, uint4)
    WRITE_IMAGE_WITH_IMGST(3d, float4)
    ;

/* split line ***************/
const gctCONST_STRING gcLibCL_ReadImage_VIR_Common_Func_Str =
"uint _viv_getImageAddr(viv_generic_image_t image)\n"
"{\n"
"    uint8 addr;\n"
"    _viv_asm(ASSIGN, addr, image);\n"
"    return addr.x;\n"
"}\n"
"uint _viv_getImageRowStride(viv_generic_image_t image)\n"
"{\n"
"    uint8 sliceStride;\n"
"    _viv_asm(ASSIGN, sliceStride, image);\n"
"    return sliceStride.s1;\n"
"}\n"
"uint _viv_getImage3DSliceStride(viv_generic_image_t image)\n"
"{\n"
"    uint8 sliceStride;\n"
"    _viv_asm(ASSIGN, sliceStride, image);\n"
"    return sliceStride.s4;\n"
"}\n"
"int3 _viv_getImageDimensions(viv_generic_image_t image)\n"
"{\n"
"    int4 result;\n"
"    result = get_image_dim(image);\n"
"    return result.xyz;\n"
"}\n"
"uint _viv_get3DImageNewBaseAddr(viv_generic_image_t image, int3 coord)\n"
"{\n"
"    return _viv_getImageAddr(image) + _viv_getImage3DSliceStride(image) * coord.s2;\n"
"}\n"
;

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(float, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(half, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm8, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm16, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm8, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm16, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(int8, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(int16, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(int32, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint8, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint16, _RGBA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_RGBA[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint32, _RGBA);

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(float, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(half, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm8, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm16, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm8, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm16, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(int8, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(int16, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(int32, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint8, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint16, _BGRA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_BGRA[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint32, _BGRA);

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(float, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(half, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm8, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm16, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm8, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm16, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(int8, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(int16, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(int32, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint8, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint16, _R);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_R[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint32, _R);

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(float, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(half, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm8, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm16, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm8, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm16, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(int8, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(int16, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(int32, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint8, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint16, _A);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_A[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint32, _A);

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_INTENSITY[3] = \
    VIR_IMAGE_LOAD_STORE_group(float, _INTENSITY);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_INTENSITY[3] = \
    VIR_IMAGE_LOAD_STORE_group(half, _INTENSITY);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_INTENSITY[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm8, _INTENSITY);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_INTENSITY[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm16, _INTENSITY);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_INTENSITY[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm8, _INTENSITY);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_INTENSITY[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm16, _INTENSITY);

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_LUMINANCE[3] = \
    VIR_IMAGE_LOAD_STORE_group(float, _LUMINANCE);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_LUMINANCE[3] = \
    VIR_IMAGE_LOAD_STORE_group(half, _LUMINANCE);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_LUMINANCE[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm8, _LUMINANCE);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_LUMINANCE[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm16, _LUMINANCE);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_LUMINANCE[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm8, _LUMINANCE);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_LUMINANCE[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm16, _LUMINANCE);

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(float, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(half, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm8, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm16, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm8, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm16, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(int8, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(int16, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(int32, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint8, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint16, _RG);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_RG[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint32, _RG);

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(float, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(half, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm8, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm16, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm8, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm16, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(int8, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(int16, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(int32, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint8, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint16, _RA);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_RA[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint32, _RA);

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm555_RGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm555, _RGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm565_RGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm565, _RGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm101010_RGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm101010, _RGB);


const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(float, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(half, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm8, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(unorm16, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm8, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(snorm16, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(int8, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(int16, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(int32, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint8, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint16, _ARGB);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_ARGB[3] = \
    VIR_IMAGE_LOAD_STORE_group(uint32, _ARGB);

const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_IMGLD_int[3] = \
    VIR_IMAGE_IMGLD_group(int);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_IMGLD_uint[3] = \
    VIR_IMAGE_IMGLD_group(uint);
const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_IMGLD_float[3] = \
    VIR_IMAGE_IMGLD_group(float);

#endif

static gctBOOL isAppConformance(gcePATCH_ID patchId)
{
    if (patchId == gcvPATCH_GTFES30 || patchId == gcvPATCH_DEQP)
        return gcvTRUE;

    return gcvFALSE;
}

gceSTATUS
gcSHADER_InitClBuiltinLibrary(
    IN gcSHADER     Shader,
    IN gctINT       ShaderType,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary,
    OUT gctSTRING   *builtinSource)
{
    gceSTATUS   status        = gcvSTATUS_OK;
    gctINT      i, stringNum = 0;
    gctSIZE_T   length;
    gctPOINTER  pointer = gcvNULL;
    gctBOOL     fmaSupported = gcHWCaps.hwFeatureFlags.supportAdvancedInsts;
    gctBOOL     isHalti5 = gcHWCaps.hwFeatureFlags.hasHalti5;
    gctBOOL     isHalti2 = gcHWCaps.hwFeatureFlags.hasHalti2;
    gctBOOL     useImgInst = gcHWCaps.hwFeatureFlags.supportImgAddr&&
        !gcdHasOptimization(gcGetOptimizerOptionVariable()->optFlags, gcvOPTIMIZATION_IMAGE_PATCHING) &&
        gcSHADER_GoVIRPass(Shader);
    gcePATCH_ID patchId = *gcGetPatchId();
    /* built-in function library */

    gctSTRING   builtinLib[] =
    {
        gcCLLibLongMADSAT_Funcs,
        gcCLLibLongNEXTAFTER_Funcs,
    };

    gctSTRING   builtinLib_Math_hasFMASupport[] =
    {
        gcCLLibSIN_Funcs,
        gcCLLibCOS_Funcs,
        gcCLLibTAN_Funcs,
        gcCLLibPOW_Funcs,
    };

    gctSTRING   builtinLib_Math_noFMASupport[] =
    {
        gcCLLibSIN_noFMA_Funcs,
        gcCLLibCOS_noFMA_Funcs,
        gcCLLibTAN_noFMA_Funcs,
        gcCLLibPOW_noFMA_Funcs,
    };

    gctSTRING   builtinLib_Triangle[] =
    {
        gcCLLibASIN_ACOS_Funcs_Common,
        gcCLLibASIN_Funcs,
        gcCLLibACOS_Funcs,
        gcCLLibATAN_Funcs,
        gcCLLibATAN2_Funcs,
    };

    gctSTRING   builtinLib_Triangle_halti2[] =
    {
        gcCLLibASIN_Funcs_halti2,
        gcCLLibACOS_Funcs_halti2,
        gcCLLibATAN_Funcs_halti2,
        gcCLLibATAN2_Funcs_halti2,
    };

    gctSTRING   builtinLib_Triangle_halti5[] =
    {
        gcCLLibASIN_Funcs_halti5,
        gcCLLibACOS_Funcs_halti5,
        gcCLLibATAN_Funcs_halti5,
        gcCLLibATAN2_Funcs_halti5,
    };

    gctSTRING   builtinLib_Triangle_halti5_fmasupported[] =
    {
        gcCLLibASIN_Funcs_halti5_fmaSupported,
        gcCLLibACOS_Funcs_halti5_fmaSupported,
        gcCLLibATAN_Funcs_halti5_fmaSupported,
        gcCLLibATAN2_Funcs_halti5_fmaSupported,
    };

    gcmASSERT((LibType == gcLIB_CL_BUILTIN && GetShaderHasIntrinsicBuiltin(Shader)));

    gcmONERROR(gcoOS_Allocate(gcvNULL, __BUILTIN_SHADER_LENGTH__, &pointer));
    *builtinSource = pointer;

    /* add the header source */
    length = gcoOS_StrLen(gcCLLibHeader, gcvNULL);
    gcoOS_StrCopySafe(*builtinSource, length + 1, gcCLLibHeader);

    /* add the extension pragma */
    length = gcoOS_StrLen(gcCLLibFunc_Extension, gcvNULL);
    gcoOS_StrCatSafe(*builtinSource, __BUILTIN_SHADER_LENGTH__, gcCLLibFunc_Extension);

    /* add the extension pragma */
    length = gcoOS_StrLen(gcCLLibFunc_Defines, gcvNULL);
    gcoOS_StrCatSafe(*builtinSource, __BUILTIN_SHADER_LENGTH__, gcCLLibFunc_Defines);

    if (LibType == gcLIB_CL_BUILTIN)
    {
        stringNum = sizeof(builtinLib) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(*builtinSource,
                __BUILTIN_SHADER_LENGTH__, builtinLib[i]);
        }

        if(fmaSupported)
        {
            stringNum = sizeof(builtinLib_Math_hasFMASupport) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*builtinSource,
                    __BUILTIN_SHADER_LENGTH__, builtinLib_Math_hasFMASupport[i]);
            }
        }
        else
        {
            stringNum = sizeof(builtinLib_Math_noFMASupport) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*builtinSource,
                    __BUILTIN_SHADER_LENGTH__, builtinLib_Math_noFMASupport[i]);
            }
        }

        if (!_OCL_USE_INTRINSIC_FOR_IMAGE || gcShaderHasVivGcslDriverImage(Shader))
        {
            if (useImgInst)
            {
                /* add the intrinsic builtin function source in gc7000*/
                gcoOS_StrCatSafe(*builtinSource,
                    __BUILTIN_SHADER_LENGTH__, gcCLLibImageQuery_Funcs_UseImgInst);
            }
            else
            {
                /* add the intrinsic builtin function source in gc3000/5000*/
                gcoOS_StrCatSafe(*builtinSource,
                    __BUILTIN_SHADER_LENGTH__, gcCLLibImageQuery_Funcs);
            }
        }

        if(fmaSupported && isHalti5)
        {
            gcoOS_StrCatSafe(*builtinSource,
                __BUILTIN_SHADER_LENGTH__, gcCLLibFMA_Func_fmaSupported);

            stringNum = sizeof(builtinLib_Triangle_halti5_fmasupported) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*builtinSource,
                    __BUILTIN_SHADER_LENGTH__, builtinLib_Triangle_halti5_fmasupported[i]);
            }
        }
        else if(isHalti5)
        {
            stringNum = sizeof(builtinLib_Triangle_halti5) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*builtinSource,
                    __BUILTIN_SHADER_LENGTH__, builtinLib_Triangle_halti5[i]);
            }
        }
        else if(isHalti2 || patchId == gcvPATCH_OCLCTS)
        {
            stringNum = sizeof(builtinLib_Triangle_halti2) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*builtinSource,
                    __BUILTIN_SHADER_LENGTH__, builtinLib_Triangle_halti2[i]);
            }
        }
        else
        {
            stringNum = sizeof(builtinLib_Triangle) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*builtinSource,
                    __BUILTIN_SHADER_LENGTH__, builtinLib_Triangle[i]);
            }
        }
    }

OnError:
    return status;

}


gceSTATUS
gcSHADER_InitBuiltinLibrary(
    IN gcSHADER     Shader,
    IN gctINT       ShaderType,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary,
    OUT gctSTRING   *sloBuiltinSource
    )
{

    gceSTATUS   status          = gcvSTATUS_OK;

    gcePATCH_ID patchId = gcPatchId;

    gctSIZE_T   length;
    gctPOINTER  pointer = gcvNULL;
    gctINT      i, stringNum = 0;


    gctBOOL     fmaSupported = gcHWCaps.hwFeatureFlags.supportAdvancedInsts;
    gctBOOL     isHalti5 = gcHWCaps.hwFeatureFlags.hasHalti5;
    gctBOOL     isHalti4 = gcHWCaps.hwFeatureFlags.hasHalti4;
    gctBOOL     isHalti2 = gcHWCaps.hwFeatureFlags.hasHalti2;
    gctBOOL     isHalti0 = gcHWCaps.hwFeatureFlags.hasHalti0;
    gctBOOL     isSupportTextureGather = gcHWCaps.hwFeatureFlags.supportTxGather;
    gctBOOL     isSupportImgAddr = gcHWCaps.hwFeatureFlags.supportImgAddr;
    gctBOOL     isSupportImgInst = gcHWCaps.hwFeatureFlags.hasHalti5 ?
        gcHWCaps.hwFeatureFlags.hasUscGosAddrFix :
    isSupportImgAddr;
    gctBOOL     isSupportTexelFetchForMSAA = gcHWCaps.hwFeatureFlags.supportMSAATexture;
    /* Use extension string to check extension feature. */
    gctBOOL     isSupportTexMSAA2DArray = gcoOS_StrStr(GetGLExtensionString(), "GL_OES_texture_storage_multisample_2d_array", gcvNULL);
    gctBOOL     isSupportCubeMapArray = gcoOS_StrStr(GetGLExtensionString(), "GL_EXT_texture_cube_map_array", gcvNULL);
    gctBOOL     isSupportTextureBuffer = gcoOS_StrStr(GetGLExtensionString(), "GL_EXT_texture_buffer", gcvNULL);
    gctBOOL     isSupportMSShading = gcoOS_StrStr(GetGLExtensionString(), "GL_OES_shader_multisample_interpolation", gcvNULL);



    /* built-in function library */

    /* the following builtin functions have different implementation in gc3000/5000 and gc7000 */

    /* gc3000/5000 implementation */
    gctSTRING   BuiltinLib[] =
    {
        gcLibFindLSB_Func_1,
        gcLibFindLSB_Func_2,
        gcLibFindLSB_Func_3,
        gcLibFindLSB_Func_4,
        gcLibFindLSB_Func_5,
        gcLibFindLSB_Func_6,
        gcLibFindLSB_Func_7,
        gcLibFindLSB_Func_8,

        gcLibFindMSB_Func_1,
        gcLibFindMSB_Func_2,
        gcLibFindMSB_Func_3,
        gcLibFindMSB_Func_4,
        gcLibFindMSB_Func_5,
        gcLibFindMSB_Func_6,
        gcLibFindMSB_Func_7,
        gcLibFindMSB_Func_8,

        gcLibBitfieldReverse_Func_1,
        gcLibBitfieldReverse_Func_2,
        gcLibBitfieldReverse_Func_3,
        gcLibBitfieldReverse_Func_4,
        gcLibBitfieldReverse_Func_5,
        gcLibBitfieldReverse_Func_6,
        gcLibBitfieldReverse_Func_7,
        gcLibBitfieldReverse_Func_8,

        gcLibBitfieldExtract_Func,

        gcLibBitfieldInsert_Func,

        gcLibUaddCarry_Func,
    };

    /* gc7000 implementation */
    gctSTRING   BuiltinLib_hati4[] =
    {
        gcLibFindLSB_Func_1_hati4,
        gcLibFindLSB_Func_2_hati4,
        gcLibFindLSB_Func_3_hati4,
        gcLibFindLSB_Func_4_hati4,
        gcLibFindLSB_Func_5_hati4,
        gcLibFindLSB_Func_6_hati4,
        gcLibFindLSB_Func_7_hati4,
        gcLibFindLSB_Func_8_hati4,

        gcLibFindMSB_Func_1_hati4,
        gcLibFindMSB_Func_2_hati4,
        gcLibFindMSB_Func_3_hati4,
        gcLibFindMSB_Func_4_hati4,
        gcLibFindMSB_Func_5_hati4,
        gcLibFindMSB_Func_6_hati4,
        gcLibFindMSB_Func_7_hati4,
        gcLibFindMSB_Func_8_hati4,

        gcLibBitfieldReverse_Func_1_hati4,
        gcLibBitfieldReverse_Func_2_hati4,
        gcLibBitfieldReverse_Func_3_hati4,
        gcLibBitfieldReverse_Func_4_hati4,
        gcLibBitfieldReverse_Func_5_hati4,
        gcLibBitfieldReverse_Func_6_hati4,
        gcLibBitfieldReverse_Func_7_hati4,
        gcLibBitfieldReverse_Func_8_hati4,

        gcLibBitfieldExtract_Func_halti4,

        gcLibBitfieldInsert_Func_halti4,

        gcLibUaddCarry_Func_hati4,

    };

#define BUILTINLIB_MIX_IDX    0
    /* the following builtin functions have same implementation in gc3000/5000 and gc7000 */
    gctSTRING   BuiltinLib_Common[] =
    {
        gcLib_2instMixFunc, /* it can be replaced with 3 inst version for comformance */

        gcLibMODF_Func,

        /* common functions */
        gcLibCommon_Func,
        gcLibACOSH_Funcs,

        gcLibLDEXP_Func,

        gcLibFREXP_Func,

        gcLibUsubBorrow_Func,

        gcLibPack_Func,
        gcLibUnpack_Func,

        gcLibUmulExtended_Func,
        gcLibImulExtended_Func,

        gcLibFMA_Func_fmaSupported,

        /* textureSize functions. */
        gcLibTextureSize_Func_1,
        gcLibTextureSize_Func_2,
        gcLibTextureSize_Func_3,
        gcLibTextureSize_Func_4,
        gcLibTextureSize_Func_5,
        gcLibTextureSize_Func_6,
        gcLibTextureSize_Func_7,
        gcLibTextureSize_Func_8,
        gcLibTextureSize_Func_9,
        gcLibTextureSize_Func_10,
        gcLibTextureSize_Func_11,
        gcLibTextureSize_Func_12,
        gcLibTextureSize_Func_13,
        gcLibTextureSize_Func_14,
        gcLibTextureSize_Func_15,
        gcLibTextureSize_Func_16,
        gcLibTextureSize_Func_17,
        gcLibTextureSize_Func_18,
        gcLibTextureSize_Func_33,
        gcLibTextureSize_Func_34,
        gcLibTextureSize_Func_35,
        gcLibTextureSize_Func_36,
        gcLibTextureSize_Func_37,
        gcLibTextureSize_Func_38,

        gcLibTextureCommon_Func,
        gcLibTextureGatherCommon_Func_1,
    };

    gctSTRING   BuiltinLib_Reflect[] =
    {
        gcLibREFLECT_Func_float,
        gcLibREFLECT_Func_vec2,
        gcLibREFLECT_Func_vec3,
        gcLibREFLECT_Func_vec4,
    };
    gctSTRING   BuiltinLib_Reflect_fmaSupported[] =
    {
        gcLibREFLECT_Func_float_fmaSupported,
        gcLibREFLECT_Func_vec2_fmaSupported,
        gcLibREFLECT_Func_vec3_fmaSupported,
        gcLibREFLECT_Func_vec4_fmaSupported,
    };

    /* advanced blend equation library */
    /* blend equations have different implementation on gc3000/5000 and gc7000,
    since on gc7000, hardware has some advanced blend equation support and
    texld_u instruction. While, on gc3000/5000, there are no such support. */

    /* gc3000/5000 implementation */
    gctSTRING   BlendEquationLib_all[] =
    {
        gcLibBlendEquation_Multiply,
        gcLibBlendEquation_Screen,
        gcLibBlendEquation_Overlay,
        gcLibBlendEquation_Darken,
        gcLibBlendEquation_Lighten,
        gcLibBlendEquation_Hardlight,
        gcLibBlendEquation_Difference,
        gcLibBlendEquation_Exclusion,

        gcLibBlendEquation_Colordodge,
        gcLibBlendEquation_Colorburn,
        gcLibBlendEquation_Softlight,
        gcLibBlendEquation_HSL_HUE,
        gcLibBlendEquation_HSL_SATURATION,
        gcLibBlendEquation_HSL_COLOR,
        gcLibBlendEquation_HSL_LUMINOSITY,
        gcLibBlendEquation_ALL,

    };

    /* gc7000 implementation */
    gctSTRING   BlendEquationLib_part0[] =
    {
        gcLibBlendEquation_Colordodge_hati4,
        gcLibBlendEquation_Colorburn_hati4,
        gcLibBlendEquation_Softlight_hati4,
        gcLibBlendEquation_HSL_HUE_hati4,
        gcLibBlendEquation_HSL_SATURATION_hati4,
        gcLibBlendEquation_HSL_COLOR_hati4,
        gcLibBlendEquation_HSL_LUMINOSITY_hati4,
        gcLibBlendEquation_ALL_hati4,

    };

    /* HW that can directly support textureGather. */
    gctSTRING TextureGatherLib[] =
    {
        gcLibTextureGather_Func_1,
        gcLibTextureGather_Func_2,
        gcLibTextureGather_Func_3,
        gcLibTextureGather_Func_4,
        gcLibTextureGather_Func_5,
        gcLibTextureGather_Func_6,
        gcLibTextureGather_Func_7,
        gcLibTextureGather_Func_8,
        gcLibTextureGather_Func_9,
        gcLibTextureGather_Func_10,
        gcLibTextureGather_Func_11,
        gcLibTextureGather_Func_12,
        gcLibTextureGather_Func_13,
        gcLibTextureGather_Func_14,
        gcLibTextureGather_Func_15,
        gcLibTextureGather_Func_16,
        gcLibTextureGather_Func_17,
        gcLibTextureGather_Func_18,
        gcLibTextureGather_Func_19,
        gcLibTextureGather_Func_20,
        gcLibTextureGather_Func_21,
        gcLibTextureGather_Func_29,
        gcLibTextureGather_Func_30,
        gcLibTextureGather_Func_31,
        gcLibTextureGather_Func_32,
        gcLibTextureGather_Func_33,
        gcLibTextureGather_Func_34,
        gcLibTextureGather_Func_35
    };

    /* HW that can't directly support textureGather. */
    gctSTRING TextureGatherLib_2[] =
    {
        gcLibTextureGather_Func_2_0,
        gcLibTextureGather_Func_2_1,
        gcLibTextureGather_Func_2_2,
        gcLibTextureGather_Func_2_3,
        gcLibTextureGather_Func_2_4,
        gcLibTextureGather_Func_2_5,
        gcLibTextureGather_Func_2_6,
        gcLibTextureGather_Func_2_7,
        gcLibTextureGather_Func_2_8,
        gcLibTextureGather_Func_2_9,
        gcLibTextureGather_Func_2_10,
        gcLibTextureGather_Func_2_11,
        gcLibTextureGather_Func_2_12,
        gcLibTextureGather_Func_2_13,
        gcLibTextureGather_Func_2_14,
        gcLibTextureGather_Func_2_15,
        gcLibTextureGather_Func_2_16,
        gcLibTextureGather_Func_2_17,
        gcLibTextureGather_Func_2_18,
        gcLibTextureGather_Func_2_19,
        gcLibTextureGather_Func_2_20,
        gcLibTextureGather_Func_2_21
    };

    gctSTRING TextureGatherOffsetLib[] =
    {
        gcLibTextureGatherOffset_Func_1,
        gcLibTextureGatherOffset_Func_2,
        gcLibTextureGatherOffset_Func_3,
        gcLibTextureGatherOffset_Func_4,
        gcLibTextureGatherOffset_Func_5,
        gcLibTextureGatherOffset_Func_6,
        gcLibTextureGatherOffset_Func_7,
        gcLibTextureGatherOffset_Func_8,
        gcLibTextureGatherOffset_Func_9,
        gcLibTextureGatherOffset_Func_10,
        gcLibTextureGatherOffset_Func_11,
        gcLibTextureGatherOffset_Func_12,
        gcLibTextureGatherOffset_Func_13,
        gcLibTextureGatherOffset_Func_14,
        gcLibTextureGatherOffset_Func_15,
        gcLibTextureGatherOffset_Func_16,
        gcLibTextureGatherOffset_Func_17,
        gcLibTextureGatherOffset_Func_18,
        gcLibTextureGatherOffset_Func_19,
        gcLibTextureGatherOffset_Func_20,
        gcLibTextureGatherOffset_Func_21
    };

    gctSTRING TextureGatherOffsetsLib[] =
    {
        gcLibTextureGatherOffsets_Func_1,
        gcLibTextureGatherOffsets_Func_2,
        gcLibTextureGatherOffsets_Func_3,
        gcLibTextureGatherOffsets_Func_4,
        gcLibTextureGatherOffsets_Func_5,
        gcLibTextureGatherOffsets_Func_6,
        gcLibTextureGatherOffsets_Func_7,
        gcLibTextureGatherOffsets_Func_8,
        gcLibTextureGatherOffsets_Func_9,
        gcLibTextureGatherOffsets_Func_10,
        gcLibTextureGatherOffsets_Func_11,
        gcLibTextureGatherOffsets_Func_12,
        gcLibTextureGatherOffsets_Func_13,
        gcLibTextureGatherOffsets_Func_14,
        gcLibTextureGatherOffsets_Func_15,
        gcLibTextureGatherOffsets_Func_16,
        gcLibTextureGatherOffsets_Func_17,
        gcLibTextureGatherOffsets_Func_18,
        gcLibTextureGatherOffsets_Func_19,
        gcLibTextureGatherOffsets_Func_20,
        gcLibTextureGatherOffsets_Func_21
    };

    gctSTRING TexelFetchForMSAALib[] =
    {
        gcLibTexelFetchForMSAA_Func_1,
        gcLibTexelFetchForMSAA_Func_2,
        gcLibTexelFetchForMSAA_Func_3,
    };

    gctSTRING TexelFetchForMSAALib_2[] =
    {
        gcLibTexelFetchForMSAA_Func_2_1,
        gcLibTexelFetchForMSAA_Func_2_2,
        gcLibTexelFetchForMSAA_Func_2_3,
    };

    gctSTRING TexMS2DArrayLib[] =
    {
        gcLibTextureSize_Func_19,
        gcLibTextureSize_Func_20,
        gcLibTextureSize_Func_21,
        gcLibTexelFetchForMSAA_Func_4,
        gcLibTexelFetchForMSAA_Func_5,
        gcLibTexelFetchForMSAA_Func_6
    };

    gctSTRING TexMS2DArrayLib_2[] =
    {
        gcLibTextureSize_Func_19,
        gcLibTextureSize_Func_20,
        gcLibTextureSize_Func_21,
        gcLibTexelFetchForMSAA_Func_2_4,
        gcLibTexelFetchForMSAA_Func_2_5,
        gcLibTexelFetchForMSAA_Func_2_6
    };

    gctSTRING ImageLib_common[] =
    {
        gcLibImageSize,
        gcLibImageSize_2D_float,
        gcLibImageSize_3D_float,
        gcLibImageSize_CUBE_float,
        gcLibImageSize_2DArray_float,
        gcLibImageSize_2D_int,
        gcLibImageSize_3D_int,
        gcLibImageSize_CUBE_int,
        gcLibImageSize_2DArray_int,
        gcLibImageSize_2D_uint,
        gcLibImageSize_3D_uint,
        gcLibImageSize_CUBE_uint,
        gcLibImageSize_2DArray_uint,
    };

    /* image_load, image_store gc3000/5000 implementation */
    gctSTRING ImageLib[] =
    {
        /* gcLibImageAddr must be the first element. */
        gcLibImageAddr,
        gcLibImageSwizzle,
        gcLibImageStoreSwizzle,

        gcLibImageLoad_2D_int, /* 16i */
        gcLibImageLoad_2D_int_rgba32i,
        gcLibImageLoad_2D_int_rgba8i,
        gcLibImageLoad_2D_int_r32i,
        gcLibImageLoad_2D_uint, /* 16ui */
        gcLibImageLoad_2D_uint_rgba32ui,
        gcLibImageLoad_2D_uint_rgba8ui,
        gcLibImageLoad_2D_uint_r32ui,
        gcLibImageLoad_2D_float, /* 16f */
        gcLibImageLoad_2D_float_rgba8,
        gcLibImageLoad_2D_float_rgba8_snorm,
        gcLibImageLoad_2D_float_rgba32f,
        gcLibImageLoad_2D_float_r32f,

        gcLibImageLoad_3Dcommon,
        gcLibImageLoad_3D,
        gcLibImageLoad_cube,
        gcLibImageLoad_2DArray,

        gcLibImageStore_2D_float, /* 16f */
        gcLibImageStore_2D_float_rgba32f,
        gcLibImageStore_2D_float_r32f,
        gcLibImageStore_2D_float_rgba8,
        gcLibImageStore_2D_float_rgba8_snorm,
        gcLibImageStore_2D_int, /* 16i */
        gcLibImageStore_2D_int_rgba32i,
        gcLibImageStore_2D_int_r32i,
        gcLibImageStore_2D_int_rgba8i,
        gcLibImageStore_2D_uint, /* 16ui */
        gcLibImageStore_2D_uint_rgba32ui,
        gcLibImageStore_2D_uint_r32ui,
        gcLibImageStore_2D_uint_rgba8ui,

        gcLibImageStore_3Dcommon,
        gcLibImageStore_3D,
        gcLibImageStore_cube,
        gcLibImageStore_2DArray,

        gcLibImageAtomicAdd_2D_int,
        gcLibImageAtomicAdd_2D_uint,
        gcLibImageAtomicAdd_3D_int,
        gcLibImageAtomicAdd_3D_uint,
        gcLibImageAtomicAdd_CUBE_int,
        gcLibImageAtomicAdd_CUBE_uint,
        gcLibImageAtomicAdd_2DARRAY_int,
        gcLibImageAtomicAdd_2DARRAY_uint,
        gcLibImageAtomicMin_2D_int,
        gcLibImageAtomicMin_2D_uint,
        gcLibImageAtomicMin_3D_int,
        gcLibImageAtomicMin_3D_uint,
        gcLibImageAtomicMin_CUBE_int,
        gcLibImageAtomicMin_CUBE_uint,
        gcLibImageAtomicMin_2DARRAY_int,
        gcLibImageAtomicMin_2DARRAY_uint,
        gcLibImageAtomicMax_2D_int,
        gcLibImageAtomicMax_2D_uint,
        gcLibImageAtomicMax_3D_int,
        gcLibImageAtomicMax_3D_uint,
        gcLibImageAtomicMax_CUBE_int,
        gcLibImageAtomicMax_CUBE_uint,
        gcLibImageAtomicMax_2DARRAY_int,
        gcLibImageAtomicMax_2DARRAY_uint,
        gcLibImageAtomicAnd_2D_int,
        gcLibImageAtomicAnd_2D_uint,
        gcLibImageAtomicAnd_3D_int,
        gcLibImageAtomicAnd_3D_uint,
        gcLibImageAtomicAnd_CUBE_int,
        gcLibImageAtomicAnd_CUBE_uint,
        gcLibImageAtomicAnd_2DARRAY_int,
        gcLibImageAtomicAnd_2DARRAY_uint,
        gcLibImageAtomicOr_2D_int,
        gcLibImageAtomicOr_2D_uint,
        gcLibImageAtomicOr_3D_int,
        gcLibImageAtomicOr_3D_uint,
        gcLibImageAtomicOr_CUBE_int,
        gcLibImageAtomicOr_CUBE_uint,
        gcLibImageAtomicOr_2DARRAY_int,
        gcLibImageAtomicOr_2DARRAY_uint,
        gcLibImageAtomicXor_2D_int,
        gcLibImageAtomicXor_2D_uint,
        gcLibImageAtomicXor_3D_int,
        gcLibImageAtomicXor_3D_uint,
        gcLibImageAtomicXor_CUBE_int,
        gcLibImageAtomicXor_CUBE_uint,
        gcLibImageAtomicXor_2DARRAY_int,
        gcLibImageAtomicXor_2DARRAY_uint,
        gcLibImageAtomicXchg_2D_int,
        gcLibImageAtomicXchg_2D_uint,
        gcLibImageAtomicXchg_2D_float,
        gcLibImageAtomicXchg_3D_int,
        gcLibImageAtomicXchg_3D_uint,
        gcLibImageAtomicXchg_3D_float,
        gcLibImageAtomicXchg_CUBE_int,
        gcLibImageAtomicXchg_CUBE_uint,
        gcLibImageAtomicXchg_CUBE_float,
        gcLibImageAtomicXchg_2DARRAY_int,
        gcLibImageAtomicXchg_2DARRAY_uint,
        gcLibImageAtomicXchg_2DARRAY_float,
        gcLibImageAtomicCmpXchg_2D_int,
        gcLibImageAtomicCmpXchg_2D_uint,
        gcLibImageAtomicCmpXchg_3D_int,
        gcLibImageAtomicCmpXchg_3D_uint,
        gcLibImageAtomicCmpXchg_CUBE_int,
        gcLibImageAtomicCmpXchg_CUBE_uint,
        gcLibImageAtomicCmpXchg_2DARRAY_int,
        gcLibImageAtomicCmpXchg_2DARRAY_uint,
    };

    /* image_load, image_store gc7000 implementation */
    gctSTRING ImageLib_hati4[] =
    {
        gcLibImageLoad_2D_float_hati4,
        gcLibImageLoad_2D_float_1_hati4,
        gcLibImageLoad_2D_int_hati4,
        gcLibImageLoad_2D_int_1_hati4,
        gcLibImageLoad_2D_uint_hati4,
        gcLibImageLoad_2D_uint_1_hati4,

        gcLibImageLoad_3D_float_hati4,
        gcLibImageLoad_3D_float_1_hati4,
        gcLibImageLoad_3D_int_hati4,
        gcLibImageLoad_3D_int_1_hati4,
        gcLibImageLoad_3D_uint_hati4,
        gcLibImageLoad_3D_uint_1_hati4,

        gcLibImageLoad_cube_float_hati4,
        gcLibImageLoad_cube_float_1_hati4,
        gcLibImageLoad_cube_int_hati4,
        gcLibImageLoad_cube_int_1_hati4,
        gcLibImageLoad_cube_uint_hati4,
        gcLibImageLoad_cube_uint_1_hati4,

        gcLibImageLoad_2DArray_float_hati4,
        gcLibImageLoad_2DArray_float_1_hati4,
        gcLibImageLoad_2DArray_int_hati4,
        gcLibImageLoad_2DArray_int_1_hati4,
        gcLibImageLoad_2DArray_uint_hati4,
        gcLibImageLoad_2DArray_uint_1_hati4,

        gcLibImageStore_2D_float_hati4,
        gcLibImageStore_2D_float_1_hati4,
        gcLibImageStore_2D_int_hati4,
        gcLibImageStore_2D_int_1_hati4,
        gcLibImageStore_2D_uint_hati4,
        gcLibImageStore_2D_uint_1_hati4,

        gcLibImageStore_3D_float_hati4,
        gcLibImageStore_3D_float_1_hati4,
        gcLibImageStore_3D_int_hati4,
        gcLibImageStore_3D_int_1_hati4,
        gcLibImageStore_3D_uint_hati4,
        gcLibImageStore_3D_uint_1_hati4,

        gcLibImageStore_cube_float_hati4,
        gcLibImageStore_cube_float_1_hati4,
        gcLibImageStore_cube_int_hati4,
        gcLibImageStore_cube_int_1_hati4,
        gcLibImageStore_cube_uint_hati4,
        gcLibImageStore_cube_uint_1_hati4,

        gcLibImageStore_2DArray_float_hati4,
        gcLibImageStore_2DArray_float_1_hati4,
        gcLibImageStore_2DArray_int_hati4,
        gcLibImageStore_2DArray_int_1_hati4,
        gcLibImageStore_2DArray_uint_hati4,
        gcLibImageStore_2DArray_uint_1_hati4,

        gcLibImageAtomicAdd_2D_int_hati4,
        gcLibImageAtomicAdd_2D_uint_hati4,
        gcLibImageAtomicAdd_3D_int_hati4,
        gcLibImageAtomicAdd_3D_uint_hati4,
        gcLibImageAtomicAdd_CUBE_int_hati4,
        gcLibImageAtomicAdd_CUBE_uint_hati4,
        gcLibImageAtomicAdd_2DARRAY_int_hati4,
        gcLibImageAtomicAdd_2DARRAY_uint_hati4,
        gcLibImageAtomicMin_2D_int_hati4,
        gcLibImageAtomicMin_2D_uint_hati4,
        gcLibImageAtomicMin_3D_int_hati4,
        gcLibImageAtomicMin_3D_uint_hati4,
        gcLibImageAtomicMin_CUBE_int_hati4,
        gcLibImageAtomicMin_CUBE_uint_hati4,
        gcLibImageAtomicMin_2DARRAY_int_hati4,
        gcLibImageAtomicMin_2DARRAY_uint_hati4,
        gcLibImageAtomicMax_2D_int_hati4,
        gcLibImageAtomicMax_2D_uint_hati4,
        gcLibImageAtomicMax_3D_int_hati4,
        gcLibImageAtomicMax_3D_uint_hati4,
        gcLibImageAtomicMax_CUBE_int_hati4,
        gcLibImageAtomicMax_CUBE_uint_hati4,
        gcLibImageAtomicMax_2DARRAY_int_hati4,
        gcLibImageAtomicMax_2DARRAY_uint_hati4,
        gcLibImageAtomicAnd_2D_int_hati4,
        gcLibImageAtomicAnd_2D_uint_hati4,
        gcLibImageAtomicAnd_3D_int_hati4,
        gcLibImageAtomicAnd_3D_uint_hati4,
        gcLibImageAtomicAnd_CUBE_int_hati4,
        gcLibImageAtomicAnd_CUBE_uint_hati4,
        gcLibImageAtomicAnd_2DARRAY_int_hati4,
        gcLibImageAtomicAnd_2DARRAY_uint_hati4,
        gcLibImageAtomicOr_2D_int_hati4,
        gcLibImageAtomicOr_2D_uint_hati4,
        gcLibImageAtomicOr_3D_int_hati4,
        gcLibImageAtomicOr_3D_uint_hati4,
        gcLibImageAtomicOr_CUBE_int_hati4,
        gcLibImageAtomicOr_CUBE_uint_hati4,
        gcLibImageAtomicOr_2DARRAY_int_hati4,
        gcLibImageAtomicOr_2DARRAY_uint_hati4,
        gcLibImageAtomicXor_2D_int_hati4,
        gcLibImageAtomicXor_2D_uint_hati4,
        gcLibImageAtomicXor_3D_int_hati4,
        gcLibImageAtomicXor_3D_uint_hati4,
        gcLibImageAtomicXor_CUBE_int_hati4,
        gcLibImageAtomicXor_CUBE_uint_hati4,
        gcLibImageAtomicXor_2DARRAY_int_hati4,
        gcLibImageAtomicXor_2DARRAY_uint_hati4,
        gcLibImageAtomicXchg_2D_int_hati4,
        gcLibImageAtomicXchg_2D_uint_hati4,
        gcLibImageAtomicXchg_2D_float_hati4,
        gcLibImageAtomicXchg_3D_int_hati4,
        gcLibImageAtomicXchg_3D_uint_hati4,
        gcLibImageAtomicXchg_3D_float_hati4,
        gcLibImageAtomicXchg_CUBE_int_hati4,
        gcLibImageAtomicXchg_CUBE_uint_hati4,
        gcLibImageAtomicXchg_CUBE_float_hati4,
        gcLibImageAtomicXchg_2DARRAY_int_hati4,
        gcLibImageAtomicXchg_2DARRAY_uint_hati4,
        gcLibImageAtomicXchg_2DARRAY_float_hati4,
        gcLibImageAtomicCmpXchg_2D_int_hati4,
        gcLibImageAtomicCmpXchg_2D_uint_hati4,
        gcLibImageAtomicCmpXchg_3D_int_hati4,
        gcLibImageAtomicCmpXchg_3D_uint_hati4,
        gcLibImageAtomicCmpXchg_CUBE_int_hati4,
        gcLibImageAtomicCmpXchg_CUBE_uint_hati4,
        gcLibImageAtomicCmpXchg_2DARRAY_int_hati4,
        gcLibImageAtomicCmpXchg_2DARRAY_uint_hati4,
    };

    /*--------------------extension built-in support--------------------*/
    /* texture buffer related built-in functions. */
    gctSTRING TextureBuffer_general[] =
    {
        gcLibTextureSize_Func_26,
        gcLibTextureSize_Func_27,
        gcLibTextureSize_Func_28,
        gcLibImageSize_Buffer_float,
        gcLibImageSize_Buffer_int,
        gcLibImageSize_Buffer_uint,
    };

    gctSTRING TextureBuffer[] =
    {
        /* imageLoad/imageStore.*/
        gcLibImageLoad_Buffer_int, /* 16i */
        gcLibImageLoad_Buffer_int_rgba32i,
        gcLibImageLoad_Buffer_int_rgba8i,
        gcLibImageLoad_Buffer_int_r32i,
        gcLibImageLoad_Buffer_uint, /* 16ui */
        gcLibImageLoad_Buffer_uint_rgba32ui,
        gcLibImageLoad_Buffer_uint_rgba8ui,
        gcLibImageLoad_Buffer_uint_r32ui,
        gcLibImageLoad_Buffer_float, /* 16f */
        gcLibImageLoad_Buffer_float_rgba8,
        gcLibImageLoad_Buffer_float_rgba8_snorm,
        gcLibImageLoad_Buffer_float_rgba32f,
        gcLibImageLoad_Buffer_float_r32f,
        gcLibImageStore_Buffer_float, /* 16f */
        gcLibImageStore_Buffer_float_rgba32f,
        gcLibImageStore_Buffer_float_r32f,
        gcLibImageStore_Buffer_float_rgba8,
        gcLibImageStore_Buffer_float_rgba8_snorm,
        gcLibImageStore_Buffer_int, /* 16i */
        gcLibImageStore_Buffer_int_rgba32i,
        gcLibImageStore_Buffer_int_r32i,
        gcLibImageStore_Buffer_int_rgba8i,
        gcLibImageStore_Buffer_uint, /* 16ui */
        gcLibImageStore_Buffer_uint_rgba32ui,
        gcLibImageStore_Buffer_uint_r32ui,
        gcLibImageStore_Buffer_uint_rgba8ui,

        /* imageAtomicXXX. */
        gcLibImageAtomicAdd_buffer_int,
        gcLibImageAtomicAdd_buffer_uint,

        gcLibImageAtomicMin_buffer_int,
        gcLibImageAtomicMin_buffer_uint,

        gcLibImageAtomicMax_buffer_int,
        gcLibImageAtomicMax_buffer_uint,

        gcLibImageAtomicAnd_buffer_int,
        gcLibImageAtomicAnd_buffer_uint,

        gcLibImageAtomicOr_buffer_int,
        gcLibImageAtomicOr_buffer_uint,

        gcLibImageAtomicXor_buffer_int,
        gcLibImageAtomicXor_buffer_uint,

        gcLibImageAtomicXchg_buffer_int,
        gcLibImageAtomicXchg_buffer_uint,
        gcLibImageAtomicXchg_buffer_float,

        gcLibImageAtomicCmpXchg_buffer_int,
        gcLibImageAtomicCmpXchg_buffer_uint,
    };

    gctSTRING TextureBuffer_support_img_access[] =
    {
        /* imageLoad/imageStore.*/
        gcLibImageLoad_Buffer_float_img_access,
        gcLibImageLoad_Buffer_int_img_access,
        gcLibImageLoad_Buffer_uint_img_access,
        gcLibImageStore_Buffer_float_img_access,
        gcLibImageStore_Buffer_int_img_access,
        gcLibImageStore_Buffer_uint_img_access,

        /* imageAtomicXXX. */
        gcLibImageAtomicAdd_buffer_int_img_access,
        gcLibImageAtomicAdd_buffer_uint_img_access,

        gcLibImageAtomicMin_buffer_int_img_access,
        gcLibImageAtomicMin_buffer_uint_img_access,

        gcLibImageAtomicMax_buffer_int_img_access,
        gcLibImageAtomicMax_buffer_uint_img_access,

        gcLibImageAtomicAnd_buffer_int_img_access,
        gcLibImageAtomicAnd_buffer_uint_img_access,

        gcLibImageAtomicOr_buffer_int_img_access,
        gcLibImageAtomicOr_buffer_uint_img_access,

        gcLibImageAtomicXor_buffer_int_img_access,
        gcLibImageAtomicXor_buffer_uint_img_access,

        gcLibImageAtomicXchg_buffer_int_img_access,
        gcLibImageAtomicXchg_buffer_uint_img_access,
        gcLibImageAtomicXchg_buffer_float_img_access,

        gcLibImageAtomicCmpXchg_buffer_int_img_access,
        gcLibImageAtomicCmpXchg_buffer_uint_img_access,
    };

    /* cubeMap related built-in functions. */
    gctSTRING ImageLib_cubeMapArray_general[] =
    {
        gcLibTextureSize_Func_22,
        gcLibTextureSize_Func_23,
        gcLibTextureSize_Func_24,
        gcLibTextureSize_Func_25,

        gcLibImageSize_CubeArray_float,
        gcLibImageSize_CubeArray_int,
        gcLibImageSize_CubeArray_uint,
    };

    gctSTRING ImageLib_cubeMapArray[] =
    {
        gcLibImageLoad_CubeArray,
        gcLibImageStore_CubeArray,
    };

    gctSTRING ImageLib_cubeMapArray_img_access[] =
    {
        gcLibImageLoad_CubeArray_float_img_access,
        gcLibImageLoad_CubeArray_float_1_img_access,
        gcLibImageLoad_CubeArray_int_img_access,
        gcLibImageLoad_CubeArray_int_1_img_access,
        gcLibImageLoad_CubeArray_uint_img_access,
        gcLibImageLoad_CubeArray_uint_1_img_access,

        gcLibImageStore_CubeArray_float_img_access,
        gcLibImageStore_CubeArray_float_1_img_access,
        gcLibImageStore_CubeArray_int_img_access,
        gcLibImageStore_CubeArray_int_1_img_access,
        gcLibImageStore_CubeArray_uint_img_access,
        gcLibImageStore_CubeArray_uint_1_img_access,
    };

    gctSTRING TextureGatherLib_cubeMapArray_halti4[] =
    {
        gcLibTextureGather_Func_22,
        gcLibTextureGather_Func_23,
        gcLibTextureGather_Func_24,
        gcLibTextureGather_Func_25,
        gcLibTextureGather_Func_26,
        gcLibTextureGather_Func_27,
        gcLibTextureGather_Func_28,
    };

    /* MS shading related built-in functions. */
    gctSTRING MSShadingLib[] =
    {
        gcLibInterpolateCommon,
        gcLibInterpolateAtCentroid_float,
        gcLibInterpolateAtCentroid_vec2,
        gcLibInterpolateAtCentroid_vec3,
        gcLibInterpolateAtCentroid_vec4,

        gcLibInterpolateAtSample_float,
        gcLibInterpolateAtSample_vec2,
        gcLibInterpolateAtSample_vec3,
        gcLibInterpolateAtSample_vec4,

        gcLibInterpolateAtOffset_float,
        gcLibInterpolateAtOffset_vec2,
        gcLibInterpolateAtOffset_vec3,
        gcLibInterpolateAtOffset_vec4,
    };

    gctSTRING TextureSize_gl[] =
    {
        gcLibTextureSize_Func_29,
        gcLibTextureSize_Func_30,
        gcLibTextureSize_Func_31,
        gcLibTextureSize_Func_32,
    };

    if (isSupportImgAddr && !isSupportImgInst &&
        (GetShaderType(Shader) == gcSHADER_TYPE_COMPUTE || GetShaderType(Shader) == gcSHADER_TYPE_CL))
    {
        isSupportImgInst = gcvTRUE;
    }

    if (isSupportImgAddr && !isSupportImgInst)
    {
        ImageLib[0] = gcLibImageAddr_halti4;
    }

    gcmASSERT((LibType == gcLIB_BUILTIN && GetShaderHasIntrinsicBuiltin(Shader)) ||
        (LibType == gcLIB_BLEND_EQUATION &&
        gceLAYOUT_QUALIFIER_HasHWNotSupportingBlendMode(GetShaderOutputBlends(Shader))));

    if (gcGLSLCompiler == gcvNULL)
    {
        return gcvSTATUS_LINK_LIB_ERROR;
    }
    /*
    if (LibType == gcLIB_BUILTIN)
    {
    if (isSupportImgInst)
    {
    if (gcBuiltinLibrary1 != gcvNULL)
    {
    *Binary = gcBuiltinLibrary1;
    return gcvSTATUS_OK;
    }
    }
    else
    {
    if (gcBuiltinLibrary0 != gcvNULL)
    {
    *Binary = gcBuiltinLibrary0;
    return gcvSTATUS_OK;
    }
    }
    }
    else if (LibType == gcLIB_BLEND_EQUATION)
    {
    if (gcBlendEquationLibrary != gcvNULL)
    {
    * Binary = gcBlendEquationLibrary;
    return gcvSTATUS_OK;
    }
    }*/

    gcmONERROR(gcoOS_Allocate(gcvNULL, __BUILTIN_SHADER_LENGTH__, &pointer));
    *sloBuiltinSource = pointer;

    /* add the extension source */
    if (VIR_Shader_IsDesktopGL(Shader))
    {
        length = gcoOS_StrLen(gcLibFunc_Extension_For_GL, gcvNULL);
        gcoOS_StrCopySafe(*sloBuiltinSource, length + 1, gcLibFunc_Extension_For_GL);
    }
    else
    {
        length = gcoOS_StrLen(gcLibFunc_Extension, gcvNULL);
        gcoOS_StrCopySafe(*sloBuiltinSource, length + 1, gcLibFunc_Extension);
    }

    /* add the extension source */
    if (isSupportTexMSAA2DArray)
    {
        gcoOS_StrCatSafe(*sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_Extension_For_TexMS2DArray);
    }

    if (isSupportCubeMapArray)
    {
        gcoOS_StrCatSafe(*sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_Extension_For_CubeMapArray);
    }

    if (isSupportTextureBuffer)
    {
        gcoOS_StrCatSafe(*sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_Extension_For_TextureBuffer);
    }

    if (isSupportMSShading)
    {
        gcoOS_StrCatSafe(*sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_Extension_For_MSShading);
    }

    if (LibType == gcLIB_BUILTIN)
    {
        /* add the header source */
        gcoOS_StrCatSafe(*sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_TextureBufferSize_For_OES);
        gcoOS_StrCatSafe(*sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_BuiltinHeader);

        if (isHalti4)
        {
            /* add the intrinsic builtin function source in gc7000*/
            stringNum = sizeof(BuiltinLib_hati4) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BuiltinLib_hati4[i]);
            }
        }
        else
        {
            /* add the intrinsic builtin function source in gc3000/5000*/
            stringNum = sizeof(BuiltinLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BuiltinLib[i]);
            }
        }

        if (fmaSupported && isHalti5)
        {
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibASIN_ACOS_Funcs_halti5_fmaSupported);
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibATAN_Funcs_halti5_fmaSupported);
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibATAN2_Funcs_halti5_fmaSupported);
        }
        else if (isHalti5)
        {
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibASIN_ACOS_Funcs_halti5);
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibATAN_Funcs_halti5);
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibATAN2_Funcs_halti5);
        }
        else if (isHalti2 && isAppConformance(patchId))
        {
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibASIN_ACOS_Funcs_halti2);
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibATAN_Funcs_halti2);
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibATAN2_Funcs_halti2);
        }
        else
        {
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibASIN_ACOS_Funcs_Common);
            if (isHalti0)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibASIN_Funcs_halti0);
            }
            else
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, gcLibASIN_Funcs);
            }
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibACOS_Funcs);
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibATAN_Funcs);
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, gcLibATAN2_Funcs);
        }

        /* Add tan functions. */
        gcoOS_StrCatSafe(*sloBuiltinSource,
            __BUILTIN_SHADER_LENGTH__, gcLibTAN_Funcs_Halti);

        /* add common intrinsic builtin function source */
        gcmASSERT(BuiltinLib_Common[BUILTINLIB_MIX_IDX] == gcLib_2instMixFunc);
        if (isAppConformance(patchId))
        {
            BuiltinLib_Common[BUILTINLIB_MIX_IDX] = gcLib_3instMixFunc;
        }
        stringNum = sizeof(BuiltinLib_Common) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, BuiltinLib_Common[i]);
        }

        if (fmaSupported)
        {
            stringNum = sizeof(BuiltinLib_Reflect_fmaSupported) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BuiltinLib_Reflect_fmaSupported[i]);
            }
        }
        else
        {
            stringNum = sizeof(BuiltinLib_Reflect) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BuiltinLib_Reflect[i]);
            }
        }

        if (isSupportTextureGather)
        {
            stringNum = sizeof(TextureGatherLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TextureGatherLib[i]);
            }
        }
        else
        {
            stringNum = sizeof(TextureGatherLib_2) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TextureGatherLib_2[i]);
            }
        }

        /* add textureGatherOffset functions. */
        stringNum = sizeof(TextureGatherOffsetLib) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, TextureGatherOffsetLib[i]);
        }

        /* add textureGatherOffsets functions. */
        stringNum = sizeof(TextureGatherOffsetsLib) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, TextureGatherOffsetsLib[i]);
        }

        stringNum = sizeof(ImageLib_common) / sizeof(gctSTRING);
        for (i = 0; i < stringNum; i++)
        {
            gcoOS_StrCatSafe(*sloBuiltinSource,
                __BUILTIN_SHADER_LENGTH__, ImageLib_common[i]);
        }
        /* hati4 support image instruction */
        if (isSupportImgInst)
        {
            stringNum = sizeof(ImageLib_hati4) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, ImageLib_hati4[i]);
            }
        }
        else
        {
            stringNum = sizeof(ImageLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, ImageLib[i]);
            }
        }

        /* texelFetch for MSAA. */
        if (isSupportTexelFetchForMSAA)
        {
            stringNum = sizeof(TexelFetchForMSAALib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TexelFetchForMSAALib[i]);
            }
        }
        else
        {
            stringNum = sizeof(TexelFetchForMSAALib_2) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TexelFetchForMSAALib_2[i]);
            }
        }

        /* MSAA Tex 2D Array */
        if (isSupportTexMSAA2DArray)
        {
            if (isSupportTexelFetchForMSAA)
            {
                stringNum = sizeof(TexMS2DArrayLib) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(*sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, TexMS2DArrayLib[i]);
                }
            }
            else
            {
                stringNum = sizeof(TexMS2DArrayLib_2) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(*sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, TexMS2DArrayLib_2[i]);
                }
            }
        }

        if (isSupportCubeMapArray)
        {
            stringNum = sizeof(ImageLib_cubeMapArray_general) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, ImageLib_cubeMapArray_general[i]);
            }

            stringNum = sizeof(TextureGatherLib_cubeMapArray_halti4) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TextureGatherLib_cubeMapArray_halti4[i]);
            }

            if (isSupportImgInst)
            {
                stringNum = sizeof(ImageLib_cubeMapArray_img_access) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(*sloBuiltinSource,
                        __BUILTIN_SHADER_LENGTH__, ImageLib_cubeMapArray_img_access[i]);
                }
            }
            else
            {
                stringNum = sizeof(ImageLib_cubeMapArray) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(*sloBuiltinSource,
                        __BUILTIN_SHADER_LENGTH__, ImageLib_cubeMapArray[i]);
                }
            }
        }

        if (isSupportTextureBuffer)
        {
            stringNum = sizeof(TextureBuffer_general) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TextureBuffer_general[i]);
            }

            if (isSupportImgInst)
            {
                stringNum = sizeof(TextureBuffer_support_img_access) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(*sloBuiltinSource,
                        __BUILTIN_SHADER_LENGTH__, TextureBuffer_support_img_access[i]);
                }
            }
            else
            {
                stringNum = sizeof(TextureBuffer) / sizeof(gctSTRING);
                for (i = 0; i < stringNum; i++)
                {
                    gcoOS_StrCatSafe(*sloBuiltinSource,
                        __BUILTIN_SHADER_LENGTH__, TextureBuffer[i]);
                }
            }
        }

        if (isSupportMSShading)
        {
            stringNum = sizeof(MSShadingLib) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, MSShadingLib[i]);
            }
        }

        if (VIR_Shader_IsDesktopGL(Shader))
        {
            stringNum = sizeof(TextureSize_gl) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, TextureSize_gl[i]);
            }
        }
    }
    else if (LibType == gcLIB_BLEND_EQUATION)
    {
        /* add the header source */
        gcoOS_StrCatSafe(*sloBuiltinSource, __BUILTIN_SHADER_LENGTH__, gcLibFunc_BlendEquationHeader);

        if (gcHWCaps.hwFeatureFlags.supportAdvBlendPart0)
        {
            /* add the blend equation source that are not supported by HW in gc7000 */
            stringNum = sizeof(BlendEquationLib_part0) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BlendEquationLib_part0[i]);
            }
        }
        else
        {
            /* add the blend equation source that are not supported by HW in gc3000/5000*/
            stringNum = sizeof(BlendEquationLib_all) / sizeof(gctSTRING);
            for (i = 0; i < stringNum; i++)
            {
                gcoOS_StrCatSafe(*sloBuiltinSource,
                    __BUILTIN_SHADER_LENGTH__, BlendEquationLib_all[i]);
            }
        }
    }

OnError:
    return status;
}

static gceSTATUS _ProcessExLockLibFile(gctFILE filp, gctUINT32 bufferSize)
{
    gceSTATUS status;

    gcmVERIFY_ARGUMENT(filp != gcvNULL);

    status = gcoOS_LockFile(gcvNULL, filp, gcvFALSE /* exclusive lock */, gcvFALSE /* non blocking */);
    if (!gcmIS_SUCCESS(status))
    {
        gcoOS_Print("_ProcessExLockLibFile: Failed to exlock libfile ");
    }
    return status;
}

static gceSTATUS _ProcessShLockLibFile(gctFILE filp, gctUINT32 bufferSize)
{
    gceSTATUS status;

    gcmVERIFY_ARGUMENT(filp != gcvNULL);

    status = gcoOS_LockFile(gcvNULL, filp, gcvTRUE /* shared lock */, gcvTRUE /* blocking */);
    if (!gcmIS_SUCCESS(status))
    {
        gcoOS_Print("_ProcessShLockLibFile: Failed to lock libfile ");
    }

    return status;
}

static gceSTATUS _ProcessUnLockLibFile(gctFILE filp, gctUINT32 bufferSize)
{
    gceSTATUS status;

    gcmVERIFY_ARGUMENT(filp != gcvNULL);

    status  =gcoOS_UnlockFile(gcvNULL, filp);
    if (!gcmIS_SUCCESS(status))
    {
        gcoOS_Print("_ProcessUnLockLibFile:Failed to unlock libfile ");
    }

    return status;
}

static gcsATOM_PTR  _LibFileLockRef = gcvNULL;
static gctPOINTER   _LibFileLock = gcvNULL;

static gceSTATUS _ThreadLockLibFile(void)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (_LibFileLock == gcvNULL)
    {
        if (_LibFileLockRef != gcvNULL)
        {
            status = gcvSTATUS_INVALID_OBJECT;
        }
        else
        {
            status = gcvSTATUS_OK;
        }
    }
    else
    {
        status = gcoOS_AcquireMutex(gcvNULL, _LibFileLock, gcvINFINITE);
    }

    return status;
}

static gceSTATUS _ThreadUnLockLibFile(void)
{
    gceSTATUS status = gcvSTATUS_OK;

    if (_LibFileLock == gcvNULL)
    {
        if (_LibFileLockRef != gcvNULL)
        {
            status = gcvSTATUS_INVALID_OBJECT;
        }
        else
        {
            status = gcvSTATUS_OK;
        }
    }
    else
    {
        status = gcoOS_ReleaseMutex(gcvNULL, _LibFileLock);
    }

    return status;
}

/*******************************************************************************************
**  Initialize LibFile
*/
gceSTATUS
gcInitializeLibFile(void)
{
    gctINT32 reference;
    gceSTATUS status;

    if (_LibFileLockRef == gcvNULL)
    {
        /* Create a new reference counter. */
        gcmONERROR(gcoOS_AtomConstruct(gcvNULL, &_LibFileLockRef));
    }

    /* Increment the reference counter */
    gcmONERROR(gcoOS_AtomIncrement(gcvNULL, _LibFileLockRef, &reference));

    if (reference == 0)
    {
        /* Create a global lock. */
        status = gcoOS_CreateMutex(gcvNULL, &_LibFileLock);

        if (gcmIS_ERROR(status))
        {
            _LibFileLock = gcvNULL;
        }
    }

OnError:
    return status;

}

/*******************************************************************************************
**  Finalize LibFile.
*/
gceSTATUS
gcFinalizeLibFile(void)
{
    gctINT32 reference = 0;
    gceSTATUS status = gcvSTATUS_OK;

    /* _LibFileLockRef could be NULL when Construction failed. */
    if(_LibFileLockRef != gcvNULL)
    {
        /* Decrement the reference counter */
        gcmVERIFY_OK(gcoOS_AtomDecrement(gcvNULL, _LibFileLockRef, &reference));
    }

    if (reference == 1)
    {
        /* Delete the global lock */
        gcmVERIFY_OK(gcoOS_DeleteMutex(gcvNULL, _LibFileLock));

        /* Destroy the reference counter */
        gcmVERIFY_OK(gcoOS_AtomDestroy(gcvNULL, _LibFileLockRef));

        _LibFileLockRef = gcvNULL;
    }

    return status;
}

#define _cldFILENAME_MAX 1024
static gceSTATUS gcSHADER_GetTempFileName(IN gctBOOL     isPatch,
                                          IN gctBOOL     isSupportImgInst,
                                          IN gcLibType    LibType,
                                          IN gctINT nameBufferSize,
                                          OUT gctSTRING nameBuffer)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR gcTmpFileName[_cldFILENAME_MAX+1];
    gcePATCH_ID patchId = gcPatchId;
    gctSTRING env = gcvNULL;

    gcoOS_GetEnv(gcvNULL, "VIV_LIB_SHADER_DIR", &env);
    if (env)
    {
        gcoOS_StrCopySafe(gcTmpFileName, _cldFILENAME_MAX, env);
    }
    else
    {
        gcmONERROR(vscGetTemporaryDir(gcTmpFileName));
    }
#if _WIN32
    gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName,
        _cldFILENAME_MAX,
        "\\"));
#else
    gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName,
        _cldFILENAME_MAX,
        "/"));
#endif
    if (isAppConformance(patchId))
    {
        gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "cts_"));
    }

    if (isPatch)
    {
        switch(LibType)
        {
        case gcLIB_BUILTIN:
            if(isSupportImgInst)
                gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_gc_img_patch.lib"));
            else
                gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_gc_noimg_patch.lib"));
            break;
        case gcLIB_CL_BUILTIN:
            gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_cl_patch.lib"));
            break;
        case gcLIB_CL_LONG_ULONG_FUNCS:
            gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_cl_long_ulong.lib"));
            break;
        default:
            gcoOS_Print("gcSHADER_GetTemporaryName:Failed to get the Patch BUILTIN LIBTYPE");
            break;
        }
    }
    else
    {
        switch(LibType)
        {
        case gcLIB_BUILTIN:
            if(isSupportImgInst)
                gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_gc_img_builtin.lib"));
            else
                gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_gc_noimg_builtin.lib"));
            break;
        case gcLIB_BLEND_EQUATION:
            gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_blend_equation.lib"));
            break;
        case gcLIB_DX_BUILTIN:
            gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_dx_builtin.lib"));
            break;
        case gcLIB_CL_BUILTIN:
            if(isSupportImgInst)
                gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_cl_img_builtin.lib"));
            else
                gcmONERROR(gcoOS_StrCatSafe(gcTmpFileName, _cldFILENAME_MAX, "viv_cl_noimg_builtin.lib"));
            break;
        default:
            gcoOS_Print("gcSHADER_GetTemporaryName:Failed to get the BUILTIN LIBTYPE");
            break;
        }
    }

    gcmONERROR(gcoOS_StrCopySafe(nameBuffer, nameBufferSize,gcTmpFileName));

OnError:
    return status;

}

gceSTATUS
gcSHADER_WriteBufferToFile(
    IN gctSTRING buffer,
    IN gctUINT32 bufferSize,
    IN gctSTRING ShaderName
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctFILE filp = gcvNULL;
    gcoOS os = gcvNULL;

    gcmONERROR(_ThreadLockLibFile());
    status = gcoOS_Open(gcvNULL, ShaderName, gcvFILE_CREATE, &filp);
    if (gcmIS_SUCCESS(status))
    {
        gcmONERROR(_ProcessExLockLibFile(filp, bufferSize));

        status = gcoOS_Write(os, filp, bufferSize, buffer);
        gcmASSERT(status == gcvSTATUS_OK);
        if (!gcmIS_SUCCESS(status))
        {
            gcoOS_Print("gcSHADER_WriteBufferToFile: Failed to write the buffer to file %s", ShaderName);
        }
        gcmONERROR(_ProcessUnLockLibFile(filp, bufferSize));
    }
    else
    {
        gcoOS_Print("gcSHADER_WriteBufferToFile: Failed to open the file %s for writing", ShaderName);
    }

OnError:
    if (filp != gcvNULL)
    {
        gcoOS_Close(os, filp);
    }
    gcmVERIFY_OK(_ThreadUnLockLibFile());
    return status;
}

gceSTATUS
gcSHADER_WriteShaderToFile(
    IN gcSHADER    Binary,
    IN gctSTRING   ShaderName
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gcSHADER    binary = Binary;
    gctUINT32 bufferSize = 0;
    gctSTRING buffer = gcvNULL;
    gcoOS os = gcvNULL;

    gcmVERIFY_ARGUMENT(ShaderName != gcvNULL);

    if(binary->type == gcSHADER_TYPE_CL)
        status = gcSHADER_SaveEx(binary,gcvNULL,&bufferSize);
    else
        status = gcSHADER_Save(binary,gcvNULL,&bufferSize);

    if (gcmIS_ERROR(status))
    {
        gcoOS_Print("gcSHADER_WriteShaderToFile: Failed to get the buffer size of Shader");
    }

    status = gcoOS_Allocate(os, bufferSize, (gctPOINTER *)&buffer);
    if (!gcmIS_SUCCESS(status))
    {
        gcoOS_Print("gcSHADER_WriteShaderToFile: Failed to allocate memory for buffer");
        return status;
    }

    if(binary->type == gcSHADER_TYPE_CL)
        status = gcSHADER_SaveEx(binary,buffer,&bufferSize);
    else
        status = gcSHADER_Save(binary,buffer,&bufferSize);

    if (!gcmIS_SUCCESS(status))
    {
        gcoOS_Print("gcSHADER_WriteShaderToFile: Failed to save the shader to buffer status=%d", status);
    }
    else
    {
        status = gcSHADER_WriteBufferToFile(buffer, bufferSize, ShaderName);
        if (gcmIS_SUCCESS(status) && gcmOPT_DUMP_CODEGEN_VERBOSE())
        {
            gcoOS_Print("INFO:  Successfully write the library shader file %s\n", ShaderName);
        }
    }

    if (buffer != gcvNULL)
    {
        gcoOS_Free(os, buffer);
    }

    return status;
}

gceSTATUS
gcSHADER_ReadBufferFromFile(
    IN gctSTRING    ShaderName,
    OUT gctSTRING    *buf,
    OUT gctUINT *bufSize
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctFILE filp = gcvNULL;
    gctUINT32 fileSize = 0;
    gctSIZE_T bufferSize = 0;
    gcoOS os = gcvNULL;
    gctSTRING buffer = gcvNULL;
    gctBOOL locked = gcvFALSE;

    gcmONERROR(_ThreadLockLibFile());

    gcmVERIFY_ARGUMENT(ShaderName != gcvNULL);

    status = gcoOS_Open(os, ShaderName, gcvFILE_READ, &filp);
    if (gcmIS_ERROR(status))
    {
        if (gcmOPT_DUMP_CODEGEN())
        {
            gcoOS_Print("gcSHADER_ReadBufferFromFile: Cannot open the library file: %s\n", ShaderName);
        }
    }
    else
    {
        status = _ProcessShLockLibFile(filp, fileSize);
        if (gcmIS_SUCCESS(status))
        {
            locked = gcvTRUE;
        }
        else
        {
            gcoOS_Close(os, filp);
            gcmVERIFY_OK(_ThreadUnLockLibFile());
            return status;
        }

        gcmONERROR(gcoOS_Seek(os, filp, 0,gcvFILE_SEEK_END));
        gcmONERROR(gcoOS_GetPos(os, filp, &fileSize));

        /*when other process write file,the filesize is 0*/
        if(fileSize == 0)
        {
            status = gcvSTATUS_INVALID_DATA;
        }
        else
        {
            status = gcoOS_Allocate(os,fileSize + 1,(gctPOINTER*)&buffer); /* one extra byte for '\0' end of string */
            if (!gcmIS_SUCCESS(status))
            {
                gcoOS_Print("gcSHADER_ReadBufferFromFile:Failed to allocate the mem to buffer ");
            }
            else
            {
                *buf = buffer;
                gcmONERROR(gcoOS_Seek(gcvNULL, filp, 0, gcvFILE_SEEK_SET));/*file pos to 0;*/
                status = gcoOS_Read(os, filp ,fileSize, buffer, &bufferSize);
                *bufSize = bufferSize;
                if (gcmIS_SUCCESS(status) && (fileSize == bufferSize))
                {
                    if (gcmOPT_DUMP_CODEGEN_VERBOSE())
                    {
                        gcoOS_Print("INFO: Successfully read library shader file %s",ShaderName);
                    }
                }
                else
                {
                    gcoOS_Print("ERROR: Failed to read library shader file %s",ShaderName);
                    status = gcvSTATUS_INVALID_DATA;
                }
            }
        }
    }
OnError:
    if (locked)
        gcmONERROR(_ProcessUnLockLibFile(filp, fileSize));
    if (filp != gcvNULL)
    {
        gcoOS_Close(os, filp);
    }
    gcmVERIFY_OK(_ThreadUnLockLibFile());
    return status;
}

gceSTATUS
gcSHADER_ReadShaderFromFile(
    IN gctSTRING    ShaderName,
    OUT gcSHADER    *Binary
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT bufferSize = 0;
    gcoOS os = gcvNULL;
    gctSTRING buffer = gcvNULL;
    gcSHADER_KIND ShaderType;
    /*struct stat fileinfo;*/

    /*multi thread variable binary share the same global memory*/
    if(*Binary!=gcvNULL)
        return status;

    status = gcSHADER_ReadBufferFromFile(ShaderName ,&buffer,&bufferSize);
    if (gcmIS_SUCCESS(status))
    {
        ShaderType = (gcSHADER_KIND)((*(gctUINT32 *) ((gceOBJECT_TYPE *) buffer + 4))>>16);/*get the shadertype from word4*/
        if (ShaderType >= gcSHADER_TYPE_UNKNOWN &&  ShaderType <= gcSHADER_KIND_COUNT)
        {
            gctUINT32           shaderVersion;
            gcmONERROR(gcSHADER_Construct(ShaderType, Binary));

            status = gcSHADER_LoadHeader(*Binary,buffer,bufferSize,&shaderVersion);
            if (gcmIS_SUCCESS(status))
            {
                if (ShaderType == gcSHADER_TYPE_CL)
                    status = gcSHADER_LoadEx(*Binary, buffer, bufferSize);
                else
                    status = gcSHADER_Load(*Binary, buffer, bufferSize);

                if (!gcmIS_SUCCESS(status))
                {
                    gcoOS_Print("gcSHADER_ReadShaderFromFile:Failed to extract the buffer to shader status=%d ",status);
                }

                if (gcSHADER_DumpCodeGenVerbose(*Binary))
                {
                    gcoOS_Print("gcSHADER_ReadShaderFromFile:  %s,status=%d\n", ShaderName,status);
                }
            }
            else
            {
                gcoOS_Print("gcSHADER_ReadShaderFromFile:Failed to extract the buffer to shader status=%d ",status);
                status = gcvSTATUS_VERSION_MISMATCH;
            }
        }
        else
        {
            gcoOS_Print("gcSHADER_ReadShaderFromFile:Failed to get the shadre type=%d ",ShaderType);
            status = gcvSTATUS_VERSION_MISMATCH;
        }
    }

OnError:
    if (buffer != gcvNULL)
    {
        gcoOS_Free(os, buffer);
    }
    if (!gcmIS_SUCCESS(status))
    {
        if(*Binary!=gcvNULL)
        {
            gcSHADER_Destroy(*Binary);
            *Binary = gcvNULL;
            Binary = gcvNULL;
        }
    }

    return status;
}

gceSTATUS
gcSHADER_WriteBuiltinLibToFile(
    IN gcSHADER    Binary,
    IN gctBOOL     isSupportImgInst,
    IN gcLibType    LibType
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR gcTmpFileName[_cldFILENAME_MAX+1];

    gcmONERROR(gcSHADER_GetTempFileName(gcvFALSE,isSupportImgInst,LibType,gcmSIZEOF(gcTmpFileName),gcTmpFileName));

    gcmONERROR(gcSHADER_WriteShaderToFile(Binary, gcTmpFileName));

OnError:

    return status;
}


gceSTATUS
gcSHADER_ReadBuiltinLibFromFile(
    IN  gctBOOL     isSupportImgInst,
    IN  gcLibType   LibType,
    OUT gcSHADER    *Binary
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR gcTmpFileName[_cldFILENAME_MAX+1];

    gcmONERROR(gcSHADER_GetTempFileName(gcvFALSE,isSupportImgInst,LibType,gcmSIZEOF(gcTmpFileName),gcTmpFileName));

    status = gcSHADER_ReadShaderFromFile(gcTmpFileName, Binary);

OnError:

    return status;
}

gceSTATUS
gcSHADER_WritePatchLibToFile(
    IN gcSHADER    Binary,
    IN gctBOOL     isSupportImgInst,
    IN gcLibType    LibType
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR gcTmpFileName[_cldFILENAME_MAX+1];

    gcmONERROR(gcSHADER_GetTempFileName(gcvTRUE,isSupportImgInst,LibType,gcmSIZEOF(gcTmpFileName),gcTmpFileName));

    gcmONERROR(gcSHADER_WriteShaderToFile(Binary, gcTmpFileName));

OnError:

    return status;
}


gceSTATUS
gcSHADER_ReadPatchLibFromFile(
    IN gctBOOL     isSupportImgInst,
    IN gcLibType    LibType,
    OUT gcSHADER    *Binary
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR gcTmpFileName[_cldFILENAME_MAX+1];

    gcmONERROR(gcSHADER_GetTempFileName(gcvTRUE,isSupportImgInst,LibType,gcmSIZEOF(gcTmpFileName),gcTmpFileName));

    status = gcSHADER_ReadShaderFromFile(gcTmpFileName, Binary);

OnError:

    return status;
}


static gceSTATUS
gcSHADER_GetTempVirFileName(
    IN gctSTRING virLibName,
    IN gctINT nameBufferSize,
    OUT gctSTRING nameBuffer)
{

    gceSTATUS status = gcvSTATUS_OK;

    gcmONERROR(vscGetTemporaryDir(nameBuffer));
#if _WIN32
    gcmONERROR(gcoOS_StrCatSafe(nameBuffer,
        nameBufferSize,
        "\\"));
#else
    gcmONERROR(gcoOS_StrCatSafe(nameBuffer,
        nameBufferSize,
        "/"));
#endif

    gcmVERIFY_ARGUMENT(virLibName != gcvNULL);

    gcmONERROR(gcoOS_StrCatSafe(nameBuffer, nameBufferSize, virLibName));

OnError:
    return status;

}

gceSTATUS
gcSHADER_WriteVirLibToFile(
    IN gctSTRING virLibName,
    IN VIR_Shader    *VirShader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR gcTmpFileName[_cldFILENAME_MAX+1];
    gctUINT   bufferSize = 0;
    gctSTRING   buffer = gcvNULL;

    gcmVERIFY_ARGUMENT(virLibName != gcvNULL);

    gcmONERROR(gcSHADER_GetTempVirFileName(virLibName, gcmSIZEOF(gcTmpFileName),gcTmpFileName));

    gcmONERROR(vscSaveShaderToBinary(VirShader, (gctPOINTER *)&buffer, &bufferSize));

    status = gcSHADER_WriteBufferToFile(buffer, bufferSize, gcTmpFileName);

OnError:

    if(buffer != gcvNULL)
    {
        gcoOS_Free(gcvNULL, buffer);
    }

    return status;
}

gceSTATUS
gcSHADER_ReadVirLibFromFile(
    IN gctSTRING virLibName,
    OUT VIR_Shader    **VirShader
    )
{
    gceSTATUS status = gcvSTATUS_OK;
    gctCHAR gcTmpFileName[_cldFILENAME_MAX+1];
    gctSTRING buf = gcvNULL;
    gctUINT bufSize = 0;

    gcmVERIFY_ARGUMENT(virLibName != gcvNULL);

    gcmONERROR(gcSHADER_GetTempVirFileName(virLibName,gcmSIZEOF(gcTmpFileName),gcTmpFileName));

    status = gcSHADER_ReadBufferFromFile(gcTmpFileName, &buf,&bufSize);

    if (gcmIS_SUCCESS(status))
    {
        gcmONERROR(vscLoadShaderFromBinary(buf, bufSize, (SHADER_HANDLE*)VirShader, gcvFALSE));
    }

OnError:
    if (buf != gcvNULL)
    {
        gcoOS_Free(gcvNULL, buf);
    }
    if (!gcmIS_SUCCESS(status))
    {
        if (*VirShader != gcvNULL)
        {
            VIR_Shader_Destroy(*VirShader);
            *VirShader = gcvNULL;
        }
    }

    return status;
}

#ifndef NL
#define NL "\n"
#endif

gctSTRING gcCLLibHeader =
NL "/* Vivante OpenCL builtin library */"
NL;

gctSTRING gcCLLibFunc_Extension =
NL "#pragma OPENCL EXTENSION  CL_VIV_asm : enable"
NL;

gctSTRING gcCLLibFunc_Defines =
NL "//Sin(x)/x approximation with x^(2k), for |x| <= 1.6 > Pi/2"
NL "#define gcdSinCoeffXLess1dot6_0   1.0\n"
NL "#define gcdSinCoeffXLess1dot6_1   -0.166666597127914430000000f"
NL "#define gcdSinCoeffXLess1dot6_2   0.008333060890436172500000f"
NL "#define gcdSinCoeffXLess1dot6_3   -0.000198088469915091990000f"
NL "#define gcdSinCoeffXLess1dot6_4   0.000002603574557724641600f"
NL "#define gcdTanCoeffXLess1_0       1.000000000000000000000000f"
NL "#define gcdTanCoeffXLess1_1       0.333330631256103520000000f"
NL "#define gcdTanCoeffXLess1_2       0.133404493331909180000000f"
NL "#define gcdTanCoeffXLess1_3       0.053259164094924927000000f"
NL "#define gcdTanCoeffXLess1_4       0.025360789149999619000000f"
NL "#define gcdTanCoeffXLess1_5       -0.000597973237745463850000f"
NL "#define gcdTanCoeffXLess1_6       0.018156010657548904000000f"
NL "#define gcdTanCoeffXLess1_7       -0.010787991806864738000000f"
NL "#define gcdTanCoeffXLess1_8       0.005282592494040727600000f"
NL;

#if !CompileIntrinsicLibfromSrc

gctSTRING gcCLLibFMA_Func_fmaSupported =
"float _viv_fma_float(float a, float b, float c)\n"
"{\n"
"   float temp, result;\n"
"   _viv_asm(FMA_MUL, temp, a, b);\n"
"   _viv_asm(FMA_ADD, result, temp, c);\n"
"   return result;\n"
"}\n"
"float2 _viv_fma_float2(float2 a, float2 b, float2 c)\n"
"{\n"
"   float2 temp, result;\n"
"   _viv_asm(FMA_MUL, temp, a, b);\n"
"   _viv_asm(FMA_ADD, result, temp, c);\n"
"   return result;\n"
"}\n"
"float3 _viv_fma_float3(float3 a, float3 b, float3 c)\n"
"{\n"
"   float3 temp, result;\n"
"   _viv_asm(FMA_MUL, temp, a, b);\n"
"   _viv_asm(FMA_ADD, result, temp, c);\n"
"   return result;\n"
"}\n"
"float4 _viv_fma_float4(float4 a, float4 b, float4 c)\n"
"{\n"
"   float4 temp, result;\n"
"   _viv_asm(FMA_MUL, temp, a, b);\n"
"   _viv_asm(FMA_ADD, result, temp, c);\n"
"   return result;\n"
"}\n";

gctSTRING gcCLLibASIN_ACOS_Funcs_Common =
"float _viv_asin_base_1(float a)\n"
"{\n"
"   float temp  = a * a;\n"
"   float result = a * (1.0004488056969831f+ temp*(0.15295127520390661f + temp* 0.13409603620943705f));\n"
"   return result;\n"
"}\n"
"float2 _viv_asin_base_2(float2 a)\n"
"{\n"
"   float2 temp  = a * a;\n"
"   float2 result = a * ((float2)(1.0004488056969831f) + temp*((float2)(0.15295127520390661f) + temp* (float2)(0.13409603620943705f)));\n"
"   return result;\n"
"}\n"
"float3 _viv_asin_base_3(float3 a)\n"
"{\n"
"   float3 temp  = a * a;\n"
"   float3 result = a * ((float3)(1.0004488056969831f)+ temp*((float3)(0.15295127520390661f) + temp* (float3)(0.13409603620943705f)));\n"
"   return result;\n"
"}\n"
"float4 _viv_asin_base_4(float4 a)\n"
"{\n"
"   float4 temp  = a * a;\n"
"   float4 result = a * ((float4)(1.0004488056969831f) + temp*((float4)(0.15295127520390661f) + temp* (float4)(0.13409603620943705f)));\n"
"   return result;\n"
"}\n";

gctSTRING gcCLLibPOW_noFMA_Funcs =
"float _viv_pow_float(float x, float y)\n"
"{\n"
"    return viv_pow_noFMA(x, y);\n"
"}\n"
"float2 _viv_pow_float2(float2 x, float2 y)\n"
"{\n"
"    return viv_pow_noFMA(x, y);\n"
"}\n"
"float3 _viv_pow_float3(float3 x, float3 y)\n"
"{\n"
"    return viv_pow_noFMA(x, y);\n"
"}\n"
"float4 _viv_pow_float4(float4 x, float4 y)\n"
"{\n"
"    return viv_pow_noFMA(x, y);\n"
"}\n"
"float8 _viv_pow_float8(float8 x, float8 y)\n"
"{\n"
"     return (float8)(viv_pow_noFMA(x.s0123, y.s0123), viv_pow_noFMA(x.s4567, y.s4567)); \n"
"}\n"
"float16 _viv_pow_float16(float16 x, float16 y)\n"
"{\n"
"     return (float16)(viv_pow_noFMA(x.s0123, y.s0123), viv_pow_noFMA(x.s4567, y.s4567), \n"
"                      viv_pow_noFMA(x.s89AB, y.s89AB), viv_pow_noFMA(x.sCDEF, y.sCDEF)); \n"
"}\n";

gctSTRING gcCLLibPOW_Funcs =
"float _viv_pow_float(float x, float y)\n"
"{\n"
"    if((x > 0) & (as_uint(x) < 0x7f800000) & ((as_uint(y) & 0x7fffffff) < (0x7f800000 - (8 << 23)))){  //Only handle the positive x^y, only real value\n"
"        int expPart = as_int(x) >> 23, manPart = (as_int(x) & 0x7fffff); //Get the exponent of value\n"
"        float expPartF = (float) expPart -127.; //exponent bias 127 remove\n"
"        float manPartF = manPart/(float)(1<<23) + 1.0f; //The mantissa part\n"
"        float expPartY0, yTimes0, yTimes, error;\n"
"        if(expPartF < 0.){ //ExpPartF < 0., means x < 1.0\n"
"            expPartF += 1.;\n"
"            manPartF /= 2.;  //mantissa part is 0.5~1, actually, the maximum error isn't 2.88/2^24, it is 3.xx/2^24\n"
"        }\n"
"        manPartF = log2(manPartF);  //The log2 part,\n"
"        expPartY0 = floor(expPartF * y); //exponent*Y\n"
"        yTimes = fast_fma(expPartF, y, -expPartY0);//fractional part, use FMA more accurate\n"
"        yTimes0 = fast_fma(manPartF, y, yTimes);    //fma, fractional part + mantissa*y\n"
"        yTimes = exp2(yTimes0);  //exp2 of small part, the rest are the integer power\n"
"        error = (fabs(manPartF * y) * ((1 + 2.88) * 0.70) + 3.76f) ; //maximum relative error*(1<<24)\n"
"        //1-- the trancation of yTimes fma part, 2.88 the log2 relative error, 3.76, the exp2 error\n"
"        //Theoretically, 2.88 should be 3.2066, if expPartF < 0., but 2.88 works well.\n"
"        manPart = as_int(yTimes) & 0x7fffff;\n"
"        manPartF = manPart/(float)(1<<23) + 1.0f;\n"
"        if(manPart < 0x10) //Sometimes the manPart < 16, the real value is different exponent, so the ULP times 2.\n"
"             manPartF = 2.0f;\n"
"        error *= manPartF; //After multiply the mantis part, it is the ULP/2.\n"
"        if(error < 31.f && fabs(expPartY0 + yTimes0) < 125.75f){ //2nd condition avoid over/underflow, but cover most cases\n"
"            expPart = ((int)expPartY0)*(1<<23) + as_int(yTimes); //Integer fma, 2 instructions\n"
"            return as_float(expPart);\n"
"        }\n"
"    }\n"
"  return viv_pow(x, y); //Use many instructions\n"
"}\n"
"float2 _viv_pow_float2(float2 x, float2 y)\n"
"{\n"
"    if(all((x > 0) & (as_uint2(x) < 0x7f800000) & ((as_uint2(y) & 0x7fffffff) < (0x7f800000 - (8 << 23))))){  //Only handle the positive x^y, only real value\n"
"        int2 expPart = as_int2(x) >> 23, manPart = (as_int2(x) & 0x7fffff); //Get the exponent of value\n"
"        float2 expPartF = convert_float2(expPart) - 127.; //exponent bias 127 remove\n"
"        float2 manPartF = convert_float2(manPart)/(float)(1<<23) + 1.0f; //The mantissa part\n"
"        float2 expPartY0, yTimes0, yTimes, error;\n"
"        int2 expLessThan = expPartF < 0.; //ExpPartF < 0., means x < 1.0\n"
"        int2 manLessThan;\n"
"        float2 expModifier;\n"
"        float2 manModifier;\n"
"        expModifier = select((float2)(0.0, 0.0), (float2)(1.0, 1.0), expLessThan);\n"
"        manModifier = select((float2)(1.0, 1.0), (float2)(0.5, 0.5), expLessThan);\n"
"        expPartF += expModifier;\n"
"        manPartF *= manModifier;\n"
"        manPartF = log2(manPartF);  //The log2 part,\n"
"        expPartY0 = floor(expPartF * y); //exponent*Y\n"
"        yTimes = fast_fma(expPartF, y, -expPartY0);//fractional part, use FMA more accurate\n"
"        yTimes0 = fast_fma(manPartF, y, yTimes);    //fma, fractional part + mantissa*y\n"
"        yTimes = exp2(yTimes0);  //exp2 of small part, the rest are the integer power\n"
"        error = (fabs(manPartF * y) * ((1 + 2.88) * 0.70) + 3.76f) ; //maximum relative error*(1<<24)\n"
"        //1-- the trancation of yTimes fma part, 2.88 the log2 relative error, 3.76, the exp2 error\n"
"        //Theoretically, 2.88 should be 3.2066, if expPartF < 0., but 2.88 works well.\n"
"        manPart = as_int2(yTimes) & 0x7fffff;\n"
"        manPartF = convert_float2(manPart)*(1.0f/(float)(1<<23)) + 1.0f;\n"
"        manLessThan = (manPart < 0x10); //Sometimes the manPart < 16, the real value is different exponent, so the ULP times 2.\n"
"        manPartF = select(manPartF, (float2)(2.0f, 2.0f), manLessThan);\n"
"        error *= manPartF; //After multiply the mantis part, it is the ULP/2.\n"
"        if(all(error < 31.f && fabs(expPartY0 + yTimes0) < 125.75f)){ //2nd condition avoid over/underflow, but cover most cases\n"
"            expPart = convert_int2(expPartY0)*(1<<23) + as_int2(yTimes); //Integer fma, 2 instructions\n"
"            return as_float2(expPart);\n"
"        }\n"
"    }\n"
"    return (float2)(_viv_pow_float(x.x, y.x), _viv_pow_float(x.y, y.y)); \n"
"}\n"
"float3 _viv_pow_float3(float3 x, float3 y)\n"
"{\n"
"    if(all((x > 0) & (as_uint3(x) < 0x7f800000) & ((as_uint3(y) & 0x7fffffff) < (0x7f800000 - (8 << 23))))){  //Only handle the positive x^y, only real value\n"
"        int3 expPart = as_int3(x) >> 23, manPart = (as_int3(x) & 0x7fffff); //Get the exponent of value\n"
"        float3 expPartF = convert_float3(expPart) - 127.; //exponent bias 127 remove\n"
"        float3 manPartF = convert_float3(manPart)/(float)(1<<23) + 1.0f; //The mantissa part\n"
"        float3 expPartY0, yTimes0, yTimes, error;\n"
"        int3 expLessThan = expPartF < 0.; //ExpPartF < 0., means x < 1.0\n"
"        int3 manLessThan;\n"
"        float3 expModifier;\n"
"        float3 manModifier;\n"
"        expModifier = select((float3)(0.0, 0.0, 0.0), (float3)(1.0, 1.0, 1.0), expLessThan);\n"
"        manModifier = select((float3)(1.0, 1.0, 1.0), (float3)(0.5, 0.5, 0.5), expLessThan);\n"
"        expPartF += expModifier;\n"
"        manPartF *= manModifier;\n"
"        manPartF = log2(manPartF);  //The log2 part,\n"
"        expPartY0 = floor(expPartF * y); //exponent*Y\n"
"        yTimes = fast_fma(expPartF, y, -expPartY0);//fractional part, use FMA more accurate\n"
"        yTimes0 = fast_fma(manPartF, y, yTimes);    //fma, fractional part + mantissa*y\n"
"        yTimes = exp2(yTimes0);  //exp2 of small part, the rest are the integer power\n"
"        error = (fabs(manPartF * y) * ((1 + 2.88) * 0.70) + 3.76f) ; //maximum relative error*(1<<24)\n"
"        //1-- the trancation of yTimes fma part, 2.88 the log2 relative error, 3.76, the exp2 error\n"
"        //Theoretically, 2.88 should be 3.2066, if expPartF < 0., but 2.88 works well.\n"
"        manPart = as_int3(yTimes) & 0x7fffff;\n"
"        manPartF = convert_float3(manPart)*(1.0/(float)(1<<23)) + 1.0f;\n"
"        manLessThan = (manPart < 0x10); //Sometimes the manPart < 16, the real value is different exponent, so the ULP times 2.\n"
"        manPartF = select(manPartF, (float3)(2.0f, 2.0f, 2.0f), manLessThan);\n"
"        error *= manPartF; //After multiply the mantis part, it is the ULP/2.\n"
"        if(all(error < 31.f && fabs(expPartY0 + yTimes0) < 125.75f)){ //2nd condition avoid over/underflow, but cover most cases\n"
"            expPart = convert_int3(expPartY0)*(1<<23) + as_int3(yTimes); //Integer fma, 2 instructions\n"
"            return as_float3(expPart);\n"
"        }\n"
"    }\n"
"    return (float3)(_viv_pow_float(x.x, y.x), _viv_pow_float(x.y, y.y), \n"
"                    _viv_pow_float(x.z, y.z)); \n"
"}\n"
"float4 _viv_pow_float4(float4 x, float4 y)\n"
"{\n"
"    if(all((x > 0) & (as_uint4(x) < 0x7f800000) & ((as_uint4(y) & 0x7fffffff) < (0x7f800000 - (8 << 23))))){  //Only handle the positive x^y, only real value\n"
"        int4 expPart = as_int4(x) >> 23, manPart = (as_int4(x) & 0x7fffff); //Get the exponent of value\n"
"        float4 expPartF = convert_float4(expPart) - 127.; //exponent bias 127 remove\n"
"        float4 manPartF = convert_float4(manPart)/(float)(1<<23) + 1.0f; //The mantissa part\n"
"        float4 expPartY0, yTimes0, yTimes, error;\n"
"        int4 expLessThan = expPartF < 0.; //ExpPartF < 0., means x < 1.0\n"
"        int4 manLessThan;\n"
"        float4 expModifier;\n"
"        float4 manModifier;\n"
"        expModifier = select((float4)(0.0, 0.0, 0.0, 0.0), (float4)(1.0, 1.0, 1.0, 1.0), expLessThan);\n"
"        manModifier = select((float4)(1.0, 1.0, 1.0, 1.0), (float4)(0.5, 0.5, 0.5, 0.5), expLessThan);\n"
"        expPartF += expModifier;\n"
"        manPartF *= manModifier;\n"
"        manPartF = log2(manPartF);  //The log2 part,\n"
"        expPartY0 = floor(expPartF * y); //exponent*Y\n"
"        yTimes = fast_fma(expPartF, y, -expPartY0);//fractional part, use FMA more accurate\n"
"        yTimes0 = fast_fma(manPartF, y, yTimes);    //fma, fractional part + mantissa*y\n"
"        yTimes = exp2(yTimes0);  //exp2 of small part, the rest are the integer power\n"
"        error = (fabs(manPartF * y) * ((1 + 2.88) * 0.70) + 3.76f) ; //maximum relative error*(1<<24)\n"
"        //1-- the trancation of yTimes fma part, 2.88 the log2 relative error, 3.76, the exp2 error\n"
"        //Theoretically, 2.88 should be 3.2066, if expPartF < 0., but 2.88 works well.\n"
"        manPart = as_int4(yTimes) & 0x7fffff;\n"
"        manPartF = convert_float4(manPart)*(1.0/(float)(1<<23)) + 1.0f;\n"
"        manLessThan = (manPart < 0x10); //Sometimes the manPart < 16, the real value is different exponent, so the ULP times 2.\n"
"        manPartF = select(manPartF, (float4)(2.0f, 2.0f, 2.0f, 2.0f), manLessThan);\n"
"        error *= manPartF; //After multiply the mantis part, it is the ULP/2.\n"
"        if(all(error < 31.f && fabs(expPartY0 + yTimes0) < 125.75f)){ //2nd condition avoid over/underflow, but cover most cases\n"
"            expPart = convert_int4(expPartY0)*(1<<23) + as_int4(yTimes); //Integer fma, 2 instructions\n"
"            return as_float4(expPart);\n"
"        }\n"
"    }\n"
"    return (float4)(_viv_pow_float(x.x, y.x), _viv_pow_float(x.y, y.y), \n"
"                    _viv_pow_float(x.z, y.z), _viv_pow_float(x.w, y.w)); \n"
"}\n"
"float8 _viv_pow_float8(float8 x, float8 y)\n"
"{\n"
"     return (float8)(_viv_pow_float4(x.s0123, y.s0123), _viv_pow_float4(x.s4567, y.s4567)); \n"
"}\n"
"float16 _viv_pow_float16(float16 x, float16 y)\n"
"{\n"
"     return (float16)(_viv_pow_float4(x.s0123, y.s0123), _viv_pow_float4(x.s4567, y.s4567), \n"
"                      _viv_pow_float4(x.s89AB, y.s89AB), _viv_pow_float4(x.sCDEF, y.sCDEF)); \n"
"}\n";

gctSTRING gcCLLibSIN_noFMA_Funcs =
"float _viv_sin_float(float x)\n"
"{\n"
"    return viv_sin_noFMA(x);\n"
"}\n"
"float2 _viv_sin_float2(float2 x)\n"
"{\n"
"    return viv_sin_noFMA(x);\n"
"}\n"
"float3 _viv_sin_float3(float3 x)\n"
"{\n"
"    return viv_sin_noFMA(x);\n"
"}\n"
"float4 _viv_sin_float4(float4 x)\n"
"{\n"
"    return viv_sin_noFMA(x);\n"
"}\n"
"float8 _viv_sin_float8(float8 x)\n"
"{\n"
"     return (float8)(viv_sin_noFMA(x.s0123), viv_sin_noFMA(x.s4567)); \n"
"}\n"
"float16 _viv_sin_float16(float16 x)\n"
"{\n"
"     return (float16)(viv_sin_noFMA(x.s0123), viv_sin_noFMA(x.s4567), \n"
"                      viv_sin_noFMA(x.s89AB), viv_sin_noFMA(x.sCDEF)); \n"
"}\n";

gctSTRING gcCLLibSIN_Funcs =
"float _viv_sin_float(float x)\n"
"{\n"
"    if(fabs(x) < 6615140.00) \n"
"    { //All instruction can be implement to Vector4 \n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int signFlag = as_int(x) & 0x80000000, IntPart; //Get the sign of input\n"
"        float a, xSubN_Pi, x_2;\n"
"        a = fast_fma(fabs(x), (1.f/3.1415926535897932384626433832795f), 0.5f) ; //Use Fma;\n"
"        IntPart = (int) a;\n"
"        signFlag ^= (IntPart)<<31; //Sin(x - n*Pi) = (-1)^nSin(x)\n"
"        a = (float)IntPart;\n"
"        //Now we calculate the x - IntPart*Pi\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        x_2 = xSubN_Pi*xSubN_Pi;\n"
"        a = fast_fma(x_2, gcdSinCoeffXLess1dot6_4, gcdSinCoeffXLess1dot6_3);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_2); \n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_1);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_0);\n"
"        a *= xSubN_Pi;\n"
"        signFlag ^= as_int(a); //Put the sign back\n"
"        return  as_float(signFlag);       \n"
"    }\n"
"    return viv_sin(x);\n"
"}\n"
"float2 _viv_sin_float2(float2 x)\n"
"{\n"
"    if(all(fabs(x) < 6615140.00)) \n"
"    { //All instruction can be implement to Vector4 \n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int2 signFlag = as_int(x) & 0x80000000, IntPart;\n"
"        float2 a, xSubN_Pi, x_2;\n"
"        a = fast_fma(fabs(x), (1.f/3.1415926535897932384626433832795f), 0.5f) ;\n"
"        IntPart = convert_int2(a);\n"
"        signFlag ^= (IntPart)<<31; //Sin(x - n*Pi) = (-1)^nSin(x)\n"
"        a = convert_float2(IntPart);\n"
"        //Now we calculate the x - IntPart*Pi\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        x_2 = xSubN_Pi * xSubN_Pi;\n"
"        a = fast_fma(x_2, gcdSinCoeffXLess1dot6_4, gcdSinCoeffXLess1dot6_3);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_2); \n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_1);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_0);\n"
"        a *= xSubN_Pi;\n"
"        signFlag ^= as_int2(a); //Put the sign back\n"
"        return  as_float2(signFlag);       \n"
"    }\n"
"    return (float2) (_viv_sin_float(x.x), _viv_sin_float(x.y));\n"
"}\n"
"float3 _viv_sin_float3(float3 x)\n"
"{\n"
"    if(all(fabs(x) < 6615140.00)) \n"
"    { //All instruction can be implement to Vector4 \n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int3 signFlag = as_int(x) & 0x80000000, IntPart;\n"
"        float3 a, xSubN_Pi, x_2;\n"
"        a = fast_fma(fabs(x), (1.f/3.1415926535897932384626433832795f), 0.5f) ;\n"
"        IntPart = convert_int3(a);\n"
"        signFlag ^= (IntPart)<<31; //Sin(x - n*Pi) = (-1)^nSin(x)\n"
"        a = convert_float3(IntPart);\n"
"        //Now we calculate the x - IntPart*Pi\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        x_2 = xSubN_Pi * xSubN_Pi;\n"
"        a = fast_fma(x_2, gcdSinCoeffXLess1dot6_4, gcdSinCoeffXLess1dot6_3);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_2);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_1);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_0);\n"
"        a *= xSubN_Pi;\n"
"        signFlag ^= as_int3(a); //Put the sign back\n"
"        return  as_float3(signFlag);       \n"
"    }\n"
"    return (float3) (_viv_sin_float(x.x), _viv_sin_float(x.y), _viv_sin_float(x.z));\n"
"}\n"
"float4 _viv_sin_float4(float4 x)\n"
"{\n"
"    if(all(fabs(x) < 6615140.00)) \n"
"    { //All instruction can be implement to Vector4 \n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int4 signFlag = as_int(x) & 0x80000000, IntPart;\n"
"        float4 a, xSubN_Pi, x_2;\n"
"        a = fast_fma(fabs(x), (1.f/3.1415926535897932384626433832795f), 0.5f) ;\n"
"        IntPart = convert_int4(a);\n"
"        signFlag ^= (IntPart)<<31; //Sin(x - n*Pi) = (-1)^nSin(x)\n"
"        a = convert_float4(IntPart);\n"
"        //Now we calculate the x - IntPart*Pi\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        x_2 = xSubN_Pi * xSubN_Pi;\n"
"        a = fast_fma(x_2, gcdSinCoeffXLess1dot6_4, gcdSinCoeffXLess1dot6_3);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_2); \n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_1);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_0);\n"
"        a *= xSubN_Pi;\n"
"        signFlag ^= as_int4(a); //Put the sign back\n"
"        return  as_float4(signFlag);       \n"
"    }\n"
"    return (float4) (_viv_sin_float(x.x), _viv_sin_float(x.y), _viv_sin_float(x.z), _viv_sin_float(x.w));\n"
"}\n"
"float8 _viv_sin_float8(float8 x)\n"
"{\n"
"     return (float8)(_viv_sin_float4(x.s0123), _viv_sin_float4(x.s4567)); \n"
"}\n"
"float16 _viv_sin_float16(float16 x)\n"
"{\n"
"     return (float16)(_viv_sin_float4(x.s0123), _viv_sin_float4(x.s4567), \n"
"                      _viv_sin_float4(x.s89AB), _viv_sin_float4(x.sCDEF)); \n"
"}\n";

gctSTRING gcCLLibCOS_noFMA_Funcs =
"float _viv_cos_float(float x)\n"
"{\n"
"    return viv_cos_noFMA(x);\n"
"}\n"
"float2 _viv_cos_float2(float2 x)\n"
"{\n"
"    return viv_cos_noFMA(x);\n"
"}\n"
"float3 _viv_cos_float3(float3 x)\n"
"{\n"
"    return viv_cos_noFMA(x);\n"
"}\n"
"float4 _viv_cos_float4(float4 x)\n"
"{\n"
"    return viv_cos_noFMA(x);\n"
"}\n"
"float8 _viv_cos_float8(float8 x)\n"
"{\n"
"     return (float8)(viv_cos_noFMA(x.s0123), viv_cos_noFMA(x.s4567)); \n"
"}\n"
"float16 _viv_cos_float16(float16 x)\n"
"{\n"
"     return (float16)(viv_cos_noFMA(x.s0123), viv_cos_noFMA(x.s4567), \n"
"                      viv_cos_noFMA(x.s89AB), viv_cos_noFMA(x.sCDEF)); \n"
"}\n";

gctSTRING gcCLLibCOS_Funcs =
"float _viv_cos_float(float x)\n"
"{\n"
"    if(fabs(x) < 3294198.50)\n"
"    { //All instruction can be implement to Vector4\n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int signFlag, IntPart; //Get the sign of input\n"
"        float a, xSubN_Pi, x_2;\n"
"        a = fast_fma(fabs(x), (1.f/3.1415926535897932384626433832795f), 1.0f);\n"
"        IntPart = (int) a;\n"
"        a = (float)IntPart - 0.5f;\n"
"        signFlag = (IntPart) << 31; //sin(x - n*Pi + 0.5Pi) = (-1)^n Sin(x + Pi/2) = (-1)^n Cos(x)\n"
"        //Now we calculate the x - (IntPart - 0.5)*Pi\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        //Now we calculate Sin(x - (IntPart - 0.5)*Pi) \n"
"        x_2 = xSubN_Pi * xSubN_Pi;\n"
"        a = fast_fma(x_2, gcdSinCoeffXLess1dot6_4, gcdSinCoeffXLess1dot6_3);  //Polynormial approximation\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_2); \n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_1);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_0);\n"
"        a *= xSubN_Pi;\n"
"        signFlag ^= as_int(a); //Put the sign back\n"
"        return as_float(signFlag);  \n"
"    }\n"
"    return viv_cos(x);\n"
"}\n"
"float2 _viv_cos_float2(float2 x)\n"
"{\n"
"    if(all(fabs(x) < 3294198.50)) \n"
"    { //All instruction can be implement to Vector4\n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int2 signFlag, IntPart; //Get the sign of input\n"
"        float2 a, xSubN_Pi, x_2;\n"
"        a = fast_fma(fabs(x), (1.f/3.1415926535897932384626433832795f), 1.0f) ; //Use Fma;\n"
"        IntPart = convert_int2(a);\n"
"        a = convert_float2(IntPart) - 0.5f;\n"
"        signFlag = (IntPart) << 31; //sin(x - n*Pi + 0.5Pi) = (-1)^n Sin(x + Pi/2) = (-1)^n Cos(x)\n"
"        //Now we calculate the x - (IntPart - 0.5)*Pi\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        //Now we calculate Sin(x - (IntPart - 0.5)*Pi) \n"
"        x_2 = xSubN_Pi * xSubN_Pi;\n"
"        a = fast_fma(x_2, gcdSinCoeffXLess1dot6_4, gcdSinCoeffXLess1dot6_3);  //Polynormial approximation\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_2); \n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_1);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_0);\n"
"        a *= xSubN_Pi;\n"
"        signFlag ^= as_int2(a); //Put the sign back\n"
"        return as_float2(signFlag);  \n"
"    }\n"
"    return viv_cos(x);\n"
"}\n"
"float3 _viv_cos_float3(float3 x)\n"
"{\n"
"    if(all(fabs(x) < 3294198.50)) \n"
"    { //All instruction can be implement to Vector4\n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int3 signFlag, IntPart; //Get the sign of input\n"
"        float3 a, xSubN_Pi, x_2;\n"
"        a = fast_fma(fabs(x), (1.f/3.1415926535897932384626433832795f), 1.0f) ; //Use Fma;\n"
"        IntPart = convert_int3(a);\n"
"        a = convert_float3(IntPart) - 0.5f;\n"
"        signFlag = (IntPart) << 31; //sin(x - n*Pi + 0.5Pi) = (-1)^n Sin(x + Pi/2) = (-1)^n Cos(x)\n"
"        //Now we calculate the x - (IntPart - 0.5)*Pi\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        //Now we calculate Sin(x - (IntPart - 0.5)*Pi) \n"
"        x_2 = xSubN_Pi * xSubN_Pi;\n"
"        a = fast_fma(x_2, gcdSinCoeffXLess1dot6_4, gcdSinCoeffXLess1dot6_3);  //Polynormial approximation\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_2); \n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_1);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_0);\n"
"        a *= xSubN_Pi;\n"
"        signFlag ^= as_int3(a); //Put the sign back\n"
"        return as_float3(signFlag);  \n"
"    }\n"
"    return viv_cos(x);\n"
"}\n"
"float4 _viv_cos_float4(float4 x)\n"
"{\n"
"    if(all(fabs(x) < 3294198.50)) \n"
"    { //All instruction can be implement to Vector4\n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int4 signFlag, IntPart; //Get the sign of input\n"
"        float4 a, xSubN_Pi, x_2;\n"
"        a = fast_fma(fabs(x), (1.f/3.1415926535897932384626433832795f), 1.0f) ; //Use Fma;\n"
"        IntPart = convert_int4(a);\n"
"        a = convert_float4(IntPart) - 0.5f;\n"
"        signFlag = (IntPart) << 31; //sin(x - n*Pi + 0.5Pi) = (-1)^n Sin(x + Pi/2) = (-1)^n Cos(x)\n"
"        //Now we calculate the x - (IntPart - 0.5)*Pi\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        //Now we calculate Sin(x - (IntPart - 0.5)*Pi) \n"
"        x_2 = xSubN_Pi * xSubN_Pi;\n"
"        a = fast_fma(x_2, gcdSinCoeffXLess1dot6_4, gcdSinCoeffXLess1dot6_3);  //Polynormial approximation\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_2); \n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_1);\n"
"        a = fast_fma(x_2, a, gcdSinCoeffXLess1dot6_0);\n"
"        a *= xSubN_Pi;\n"
"        signFlag ^= as_int4(a); //Put the sign back\n"
"        return as_float4(signFlag);  \n"
"    }\n"
"    return viv_cos(x);\n"
"}\n"
"float8 _viv_cos_float8(float8 x)\n"
"{\n"
"     return (float8)(_viv_cos_float4(x.s0123), _viv_cos_float4(x.s4567)); \n"
"}\n"
"float16 _viv_cos_float16(float16 x)\n"
"{\n"
"     return (float16)(_viv_cos_float4(x.s0123), _viv_cos_float4(x.s4567), \n"
"                      _viv_cos_float4(x.s89AB), _viv_cos_float4(x.sCDEF)); \n"
"}\n";

gctSTRING gcCLLibTAN_noFMA_Funcs =
"float _viv_tan_float(float x)\n"
"{\n"
"    return viv_tan_noFMA(x);\n"
"}\n"
"float2 _viv_tan_float2(float2 x)\n"
"{\n"
"    return viv_tan_noFMA(x);\n"
"}\n"
"float3 _viv_tan_float3(float3 x)\n"
"{\n"
"    return viv_tan_noFMA(x);\n"
"}\n"
"float4 _viv_tan_float4(float4 x)\n"
"{\n"
"    return viv_tan_noFMA(x);\n"
"}\n"
"float8 _viv_tan_float8(float8 x)\n"
"{\n"
"     return (float8)(viv_tan_noFMA(x.s0123), viv_tan_noFMA(x.s4567)); \n"
"}\n"
"float16 _viv_tan_float16(float16 x)\n"
"{\n"
"     return (float16)(viv_tan_noFMA(x.s0123), viv_tan_noFMA(x.s4567), \n"
"                      viv_tan_noFMA(x.s89AB), viv_tan_noFMA(x.sCDEF)); \n"
"}\n";

gctSTRING gcCLLibTAN_Funcs =
"float _viv_tan_float(float x)\n"
"{\n"
"    if(fabs(x) < 2206348.50)\n"
"    {\n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int signFlag = as_int(x) & 0x80000000, IntPart; //Get the sign of input\n"
"        float a, xSubN_Pi, x_2, u,v;\n"
"        IntPart = (int) (fast_fma(fabs(x), (2.f/3.1415926535897932384626433832795f), 0.5f));\n"
"        a = ((float)IntPart) * 0.5;\n"
"\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        x_2 = xSubN_Pi*xSubN_Pi;\n"
"\n"
"        a = fast_fma(x_2, gcdTanCoeffXLess1_8, gcdTanCoeffXLess1_7);  //Polynormial approximation\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_6); \n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_5);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_4);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_3); \n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_2);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_1);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_0); \n"
"        \n"
"        a *= xSubN_Pi;\n"
"        if(IntPart & 1)  //This step cannot be vectorized,if for V4 input, we should make it checking for every component .x, .y, .z, .w\n"
"          a = 1.0f/(-a);\n"
"        signFlag ^= as_int(a); //Put the sign back\n"
"        return  as_float(signFlag); \n"
"    }\n"
"    return viv_tan(x); //Conventional Tan(x)\n"
"}\n"
"float2 _viv_tan_float2(float2 x)\n"
"{\n"
"    if(all(fabs(x) < 2206348.50)) \n"
"    {\n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int2 signFlag = as_int2(x) & 0x80000000, IntPart; //Get the sign of input\n"
"        float2 a, xSubN_Pi, x_2, u,v;\n"
"        IntPart = convert_int2(fast_fma(fabs(x), (2.f/3.1415926535897932384626433832795f), 0.5f));\n"
"        a = convert_float2(IntPart) * 0.5;\n"
"\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        x_2 = xSubN_Pi*xSubN_Pi;\n"
"\n"
"        a = fast_fma(x_2, gcdTanCoeffXLess1_8, gcdTanCoeffXLess1_7);  //Polynormial approximation\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_6); \n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_5);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_4);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_3); \n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_2);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_1);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_0); \n"
"        \n"
"        a *= xSubN_Pi;\n"
"        if(IntPart.x & 1) a.x = 1.0f/(-a.x);\n"
"        if(IntPart.y & 1) a.y = 1.0f/(-a.y);\n"
"        signFlag ^= as_int2(a); //Put the sign back\n"
"        return  as_float2(signFlag); \n"
"    }\n"
"    return viv_tan(x); //Conventional Tan(x)\n"
"}\n"
"float3 _viv_tan_float3(float3 x)\n"
"{\n"
"    if(all(fabs(x) < 2206348.50)) \n"
"    {\n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int3 signFlag = as_int3(x) & 0x80000000, IntPart; //Get the sign of input\n"
"        float3 a, xSubN_Pi, x_2, u,v;\n"
"        IntPart = convert_int3(fast_fma(fabs(x), (2.f/3.1415926535897932384626433832795f), 0.5f));\n"
"        a = convert_float3(IntPart) * 0.5;\n"
"\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        x_2 = xSubN_Pi*xSubN_Pi;\n"
"\n"
"        a = fast_fma(x_2, gcdTanCoeffXLess1_8, gcdTanCoeffXLess1_7);  //Polynormial approximation\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_6); \n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_5);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_4);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_3); \n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_2);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_1);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_0); \n"
"        \n"
"        a *= xSubN_Pi;\n"
"        if(IntPart.x & 1) a.x = 1.0f/(-a.x);\n"
"        if(IntPart.y & 1) a.y = 1.0f/(-a.y);\n"
"        if(IntPart.z & 1) a.z = 1.0f/(-a.z);\n"
"        signFlag ^= as_int3(a); //Put the sign back\n"
"        return  as_float3(signFlag); \n"
"    }\n"
"    return viv_tan(x); //Conventional Tan(x)\n"
"}\n"
"float4 _viv_tan_float4(float4 x)\n"
"{\n"
"    if(all(fabs(x) < 2206348.50)) \n"
"    {\n"
"        const float Pi0 = 0x3243F6/(float)(1<<20), Pi1 = 0xa8885a/(float)(1<<20)/(1<<24), Pi2 = 0x308d31/(float)(1<<20)/(1<<24)/(1<<24);\n"
"        int4 signFlag = as_int4(x) & 0x80000000, IntPart; //Get the sign of input\n"
"        float4 a, xSubN_Pi, x_2, u,v;\n"
"        IntPart = convert_int4(fast_fma(fabs(x), (2.f/3.1415926535897932384626433832795f), 0.5f));\n"
"        a = convert_float4(IntPart) * 0.5;\n"
"\n"
"        xSubN_Pi = fast_fma(-a, Pi0, fabs(x)); //Actually, we can use a*(-Pi0)\n"
"        xSubN_Pi = fast_fma(-a, Pi1, xSubN_Pi);\n"
"        xSubN_Pi = fast_fma(-a, Pi2, xSubN_Pi);\n"
"        x_2 = xSubN_Pi*xSubN_Pi;\n"
"\n"
"        a = fast_fma(x_2, gcdTanCoeffXLess1_8, gcdTanCoeffXLess1_7);  //Polynormial approximation\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_6); \n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_5);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_4);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_3); \n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_2);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_1);\n"
"        a = fast_fma(x_2, a, gcdTanCoeffXLess1_0); \n"
"        \n"
"        a *= xSubN_Pi;\n"
"        if(IntPart.x & 1) a.x = 1.0f/(-a.x);\n"
"        if(IntPart.y & 1) a.y = 1.0f/(-a.y);\n"
"        if(IntPart.z & 1) a.z = 1.0f/(-a.z);\n"
"        if(IntPart.w & 1) a.w = 1.0f/(-a.w);\n"
"        signFlag ^= as_int4(a); //Put the sign back\n"
"        return  as_float4(signFlag); \n"
"    }\n"
"    return viv_tan(x); //Conventional Tan(x)\n"
"}\n"
"float8 _viv_tan_float8(float8 x)\n"
"{\n"
"     return (float8)(_viv_tan_float4(x.s0123), _viv_tan_float4(x.s4567)); \n"
"}\n"
"float16 _viv_tan_float16(float16 x)\n"
"{\n"
"     return (float16)(_viv_tan_float4(x.s0123), _viv_tan_float4(x.s4567), \n"
"                      _viv_tan_float4(x.s89AB), _viv_tan_float4(x.sCDEF)); \n"
"}\n";

gctSTRING gcCLLibASIN_Funcs =
"float _viv_asin_float(float a)\n"
"{\n"
"   float result;\n"
"   if (fabs(a) < 0.7072f)\n"
"   {\n"
"       result = _viv_asin_base_1(a);\n"
"   } else {\n"
"       result = (3.14159265358979323846f/2.0f - _viv_asin_base_1(sqrt(1.0f - a * a))) * (signbit(a) ? -1.0 : 1.0);\n"
"   }\n"
"   return result;\n"
"}\n"
"float2 _viv_asin_float2(float2 a)\n"
"{\n"
"   float2 result, result1, result2;\n"
"   int2 sel = isless(fabs(a), (float2)(0.7072f));\n"
"   result1 = _viv_asin_base_2(a);\n"
"   result2 = (float2)(3.14159265358979323846f/2.0f) - _viv_asin_base_2(sqrt((float2)(1.0f) - a * a));\n"
"   result2 = as_float2(as_int2(result2) | (signbit(a) << 31));\n"
"   if(sel.s0)\n"
"   {\n"
"       result.s0 = result1.s0;\n"
"   }\n"
"   else\n"
"   {\n"
"       result.s0 = result2.s0;\n"
"   }\n"
"   if(sel.s1)\n"
"   {\n"
"       result.s1 = result1.s1;\n"
"   }\n"
"   else\n"
"   {\n"
"       result.s1 = result2.s1;\n"
"   }\n"
"   return result;\n"
"}\n"
"float3 _viv_asin_float3(float3 a)\n"
"{\n"
"   float3 result, result1, result2;\n"
"   int3 sel = isless(fabs(a), (float3)(0.7072f));\n"
"   result1 = _viv_asin_base_3(a);\n"
"   result2 = (float3)(3.14159265358979323846f/2.0f) - _viv_asin_base_3(sqrt((float3)(1.0f) - a * a));\n"
"   result2 = as_float3(as_int3(result2) | (signbit(a) << 31));\n"
"   if(sel.s0)\n"
"   {\n"
"       result.s0 = result1.s0;\n"
"   }\n"
"   else\n"
"   {\n"
"       result.s0 = result2.s0;\n"
"   }\n"
"   if(sel.s1)\n"
"   {\n"
"       result.s1 = result1.s1;\n"
"   }\n"
"   else\n"
"   {\n"
"       result.s1 = result2.s1;\n"
"   }\n"
"   if(sel.s2)\n"
"   {\n"
"       result.s2 = result1.s2;\n"
"   }\n"
"   else\n"
"   {\n"
"       result.s2 = result2.s2;\n"
"   }\n"
"   return result;\n"
"}\n"
"float4 _viv_asin_float4(float4 a)\n"
"{\n"
"   float4 result, result1, result2;\n"
"   int4 sel = isless(fabs(a), (float4)(0.7072f));\n"
"   result1 = _viv_asin_base_4(a);\n"
"   result2 = (float4)(3.14159265358979323846f/2.0f) - _viv_asin_base_4(sqrt((float4)(1.0f) - a * a));\n"
"   result2 = as_float4(as_int4(result2) | (signbit(a) << 31));\n"
"   if(sel.s0)\n"
"   {\n"
"       result.s0 = result1.s0;\n"
"   }\n"
"   else\n"
"   {\n"
"       result.s0 = result2.s0;\n"
"   }\n"
"   if(sel.s1)\n"
"   {\n"
"       result.s1 = result1.s1;\n"
"   }\n"
"   else\n"
"   {\n"
"       result.s1 = result2.s1;\n"
"   }\n"
"   if(sel.s2)\n"
"   {\n"
"       result.s2 = result1.s2;\n"
"   }\n"
"   else\n"
"   {\n"
"       result.s2 = result2.s2;\n"
"   }\n"
"   if(sel.s3)\n"
"   {\n"
"       result.s3 = result1.s3;\n"
"   }\n"
"   else\n"
"   {\n"
"       result.s3 = result2.s3;\n"
"   }\n"
"   return result;\n"
"}\n"
"float8 _viv_asin_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_asin_float4(a.s0123);\n"
"   result.s4567 = _viv_asin_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_asin_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_asin_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_asin_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n";

gctSTRING gcCLLibASIN_Funcs_halti2 =
"float _viv_asin_float(float a)\n"
"{\n"
"   float result;\n"
"   float absolute = fabs(a);\n"
"   if(absolute > 1.0)\n"
"   {\n"
"       result = as_float(0x7fffffff);\n"
"   }\n"
"   else if(absolute == 1.0)\n"
"   {\n"
"       result = as_float(0x3fc90fdb | (as_uint(a) & 0x80000000));\n"
"   }\n"
"   else\n"
"   {\n"
"       if(absolute <= as_float(0x3f350b0f))\n"
"       {\n"
"           float square;\n"
"           _viv_asm(MUL!<rnd:RTZ>, square, a, a);\n"
"           _viv_asm(MUL!<rnd:RTZ>, result, square, as_float(0x3de3f893));\n"
"           _viv_asm(ADD!<rnd:RTZ>, result, result, as_float(0xbdbdac66));\n"
"           _viv_asm(MUL!<rnd:RTZ>, result, result, square);\n"
"           _viv_asm(ADD!<rnd:RTZ>, result, result, as_float(0x3d9da182));\n"
"           _viv_asm(MUL!<rnd:RTZ>, result, result, square);\n"
"           _viv_asm(ADD!<rnd:RTZ>, result, result, as_float(0x3c8645fd));\n"
"           _viv_asm(MUL!<rnd:RTZ>, result, result, square);\n"
"           _viv_asm(ADD!<rnd:RTZ>, result, result, as_float(0x3d3e78d9));\n"
"           _viv_asm(MUL!<rnd:RTZ>, result, result, square);\n"
"           _viv_asm(ADD!<rnd:RTZ>, result, result, as_float(0x3d995d3d));\n"
"           _viv_asm(MUL!<rnd:RTZ>, result, result, square);\n"
"           _viv_asm(ADD!<rnd:RTZ>, result, result, as_float(0x3e2aab4b));\n"
"           _viv_asm(MUL!<rnd:RTZ>, result, result, square);\n"
"           _viv_asm(ADD!<rnd:RTZ>, result, result, 1.0);\n"
"           _viv_asm(MUL!<rnd:RTZ>, result, a, result);\n"
"       }\n"
"       else\n"
"       {\n"
"           float oneMinorAbs;\n"
"           _viv_asm(ADD!<rnd:RTZ>, oneMinorAbs, 1.0, -absolute);\n"
"           float temp0;\n"
"           _viv_asm(MUL!<rnd:RTZ>, temp0, as_float(0x3b6a2228), oneMinorAbs);\n"
"           _viv_asm(ADD!<rnd:RTZ>, temp0, temp0, as_float(0x3bfaa371));\n"
"           _viv_asm(MUL!<rnd:RTZ>, temp0, temp0, oneMinorAbs);\n"
"           _viv_asm(ADD!<rnd:RTZ>, temp0, temp0, as_float(0x3cd97055));\n"
"           _viv_asm(MUL!<rnd:RTZ>, temp0, temp0, oneMinorAbs);\n"
"           _viv_asm(ADD!<rnd:RTZ>, temp0, temp0, as_float(0x3df15b6b));\n"
"           _viv_asm(MUL!<rnd:RTZ>, temp0, temp0, oneMinorAbs);\n"
"           _viv_asm(ADD!<rnd:RTZ>, temp0, temp0, as_float(0x3fb504f4));\n"
"           float temp1 = sqrt(oneMinorAbs);\n"
"           float temp2;\n"
"           _viv_asm(MUL!<rnd:RTZ>, temp2, temp1, temp1);\n"
"           _viv_asm(ADD!<rnd:RTZ>, temp2, temp2, -oneMinorAbs);\n"
"           float temp3;\n"
"           _viv_asm(MUL!<rnd:RTZ>, temp3, temp1, 2.0);\n"
"           _viv_asm(DIV, temp2, temp2, temp3);\n"
"           _viv_asm(ADD!<rnd:RTZ>, temp2, temp1, -temp2);\n"
"           _viv_asm(MULLO!<rnd:RTZ>, temp1, temp0, temp2);\n"
"           _viv_asm(MUL!<rnd:RTZ>, temp3, temp0, temp2);\n"
"           _viv_asm(MUL!<rnd:RTZ>, temp2, temp1, 2.0);\n"
"           _viv_asm(ADD!<rnd:RTZ>, temp2, temp3, temp2);\n"
"           _viv_asm(ADD!<rnd:RTZ>, temp2, as_float(0x3fc90fdb), -temp2);\n"
"           result = as_float((as_uint(temp2) & 0x7fffffff) | (as_uint(a) & 0x80000000));\n"
"       }\n"
"   }\n"
"   return result;\n"
"}\n"
"float2 _viv_asin_float2(float2 a)\n"
"{\n"
"   float2 result;\n"
"   result.s0 = _viv_asin_float(a.s0);\n"
"   result.s1 = _viv_asin_float(a.s1);\n"
"   return result;\n"
"}\n"
"float3 _viv_asin_float3(float3 a)\n"
"{\n"
"   float3 result;\n"
"   result.s0 = _viv_asin_float(a.s0);\n"
"   result.s1 = _viv_asin_float(a.s1);\n"
"   result.s2 = _viv_asin_float(a.s2);\n"
"   return result;\n"
"}\n"
"float4 _viv_asin_float4(float4 a)\n"
"{\n"
"   float4 result;\n"
"   result.s0 = _viv_asin_float(a.s0);\n"
"   result.s1 = _viv_asin_float(a.s1);\n"
"   result.s2 = _viv_asin_float(a.s2);\n"
"   result.s3 = _viv_asin_float(a.s3);\n"
"   return result;\n"
"}\n"
"float8 _viv_asin_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_asin_float4(a.s0123);\n"
"   result.s4567 = _viv_asin_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_asin_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_asin_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_asin_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n";

gctSTRING gcCLLibASIN_Funcs_halti5 =
"float _viv_asin_float(float a)\n"
"{\n"
"    float2 vec2Temp0;\n"
"    float floatTemp0, result;\n"
"    floatTemp0 = (1.0 - a) * (1.0 + a);\n"
"    floatTemp0 = sqrt(floatTemp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, floatTemp0, floatTemp0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x1);\n"
"    result = a * vec2Temp0.s0;\n"
"    return result;\n"
"}\n"
"float2 _viv_asin_float2(float2 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1;\n"
"    float2 vec2Temp2, result;\n"
"    vec2Temp2 = (1.0 - a) * (1.0 + a);\n"
"    vec2Temp2 = sqrt(vec2Temp2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec2Temp2.s0, vec2Temp2.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec2Temp2.s1, vec2Temp2.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x1);\n"
"    result = a * (float2)(vec2Temp0.s0, vec2Temp1.s0);\n"
"    return result;\n"
"}\n"
"float3 _viv_asin_float3(float3 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2;\n"
"    float3 vec3Temp0, result;\n"
"    vec3Temp0 = (1.0 - a) * (1.0 + a);\n"
"    vec3Temp0 = sqrt(vec3Temp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec3Temp0.s0, vec3Temp0.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec3Temp0.s1, vec3Temp0.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, vec3Temp0.s2, vec3Temp0.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x1);\n"
"    result = a * (float3)(vec2Temp0.s0, vec2Temp1.s0, vec2Temp2.s0);\n"
"    return result;\n"
"}\n"
"float4 _viv_asin_float4(float4 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2, vec2Temp3;\n"
"    float4 vec4Temp0, result;\n"
"    vec4Temp0 = (1.0 - a) * (1.0 + a);\n"
"    vec4Temp0 = sqrt(vec4Temp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec4Temp0.s0, vec4Temp0.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec4Temp0.s1, vec4Temp0.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, vec4Temp0.s2, vec4Temp0.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp3, vec4Temp0.s3, vec4Temp0.s3);\n"
"    _viv_asm(ARCTRIG1, vec2Temp3, vec2Temp3, 0x1);\n"
"    result = a * (float4)(vec2Temp0.s0, vec2Temp1.s0, vec2Temp2.s0, vec2Temp3.s0);\n"
"    return result;\n"
"}\n"
"float8 _viv_asin_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_asin_float4(a.s0123);\n"
"   result.s4567 = _viv_asin_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_asin_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_asin_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_asin_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n";

    gctSTRING gcCLLibASIN_Funcs_halti5_fmaSupported =
"float _viv_fma_float(float a, float b, float c);\n"
"float _viv_asin_float(float a)\n"
"{\n"
"    float2 vec2Temp0;\n"
"    float floatTemp0, result;\n"
"    floatTemp0 = _viv_fma_float(a, -a, 1.0);\n"
"    floatTemp0 = sqrt(floatTemp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, floatTemp0, floatTemp0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x1);\n"
"    result = a * vec2Temp0.s0;\n"
"    return result;\n"
"}\n"
"float2 _viv_fma_float2(float2 a, float2 b, float2 c);\n"
"float2 _viv_asin_float2(float2 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1;\n"
"    float2 vec2Temp2, result;\n"
"    vec2Temp2 = _viv_fma_float2(a, -a, (float2)(1.0));\n"
"    vec2Temp2 = sqrt(vec2Temp2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec2Temp2.s0, vec2Temp2.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec2Temp2.s1, vec2Temp2.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x1);\n"
"    result = a * (float2)(vec2Temp0.s0, vec2Temp1.s0);\n"
"    return result;\n"
"}\n"
"float3 _viv_fma_float3(float3 a, float3 b, float3 c);\n"
"float3 _viv_asin_float3(float3 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2;\n"
"    float3 vec3Temp0, result;\n"
"    vec3Temp0 = _viv_fma_float3(a, -a, (float3)(1.0));\n"
"    vec3Temp0 = sqrt(vec3Temp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec3Temp0.s0, vec3Temp0.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec3Temp0.s1, vec3Temp0.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, vec3Temp0.s2, vec3Temp0.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x1);\n"
"    result = a * (float3)(vec2Temp0.s0, vec2Temp1.s0, vec2Temp2.s0);\n"
"    return result;\n"
"}\n"
"float4 _viv_fma_float4(float4 a, float4 b, float4 c);\n"
"float4 _viv_asin_float4(float4 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2, vec2Temp3;\n"
"    float4 vec4Temp0, result;\n"
"    vec4Temp0 = _viv_fma_float4(a, -a, (float4)(1.0));\n"
"    vec4Temp0 = sqrt(vec4Temp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec4Temp0.s0, vec4Temp0.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec4Temp0.s1, vec4Temp0.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, vec4Temp0.s2, vec4Temp0.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x1);\n"
"    _viv_asm(ARCTRIG0, vec2Temp3, vec4Temp0.s3, vec4Temp0.s3);\n"
"    _viv_asm(ARCTRIG1, vec2Temp3, vec2Temp3, 0x1);\n"
"    result = a * (float4)(vec2Temp0.s0, vec2Temp1.s0, vec2Temp2.s0, vec2Temp3.s0);\n"
"    return result;\n"
"}\n"
"float8 _viv_asin_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_asin_float4(a.s0123);\n"
"   result.s4567 = _viv_asin_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_asin_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_asin_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_asin_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n";

gctSTRING gcCLLibACOS_Funcs =
"float _viv_acos_float(float a)\n"
"{\n"
"   float result = 3.14159265358979323846f/2.0f - _viv_asin_float(a);\n"
"   return result;\n"
"}\n"
"float2 _viv_acos_float2(float2 a)\n"
"{\n"
"   float2 result = (float2)(3.14159265358979323846f/2.0f) -  _viv_asin_float2(a);\n"
"   return result;\n"
"}\n"
"float3 _viv_acos_float3(float3 a)\n"
"{\n"
"   float3 result = (float3)(3.14159265358979323846f/2.0f) -  _viv_asin_float3(a);\n"
"   return result;\n"
"}\n"
"float4 _viv_acos_float4(float4 a)\n"
"{\n"
"   float4 result = (float4)(3.14159265358979323846f/2.0f) -  _viv_asin_float4(a);\n"
"   return result;\n"
"}\n"
"float8 _viv_acos_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_acos_float4(a.s0123);\n"
"   result.s4567 = _viv_acos_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_acos_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_acos_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_acos_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n";

gctSTRING gcCLLibACOS_Funcs_halti2 =
"float _viv_acos_float(float a)\n"
"{\n"
"   float result;\n"
"   float absolute = fabs(a);\n"
"   if(absolute > 1.0)\n"
"   {\n"
"       result = as_float(0x7fffffff);\n"
"   }\n"
"   else if(a == 1.0)\n"
"   {\n"
"       result = 0.0;\n"
"   }\n"
"   else if(a == -1.0)\n"
"   {\n"
"       result = as_float(0x40490fdb);\n"
"   }\n"
"   else if(absolute <= as_float(0x3f350b0f))\n"
"   {\n"
"       float temp0, temp1;\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp0, a, a);\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp0, as_float(0x3de3f893));\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0xbdbdac66));\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp1, temp0);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3d9da182));\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp1, temp0);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3c8645fd));\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp1, temp0);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3d3e78d9));\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp1, temp0);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3d995d3d));\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp1, temp0);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3e2aab4b));\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp1, temp0);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp0, temp1, 1.0);\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, a, temp0);\n"
"       _viv_asm(ADD!<rnd:RTZ>, result, as_float(0x3fc90fdb), -temp1);\n"
"   }\n"
"   else\n"
"   {\n"
"       float temp0, temp1, temp2, temp3;\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp0, 1.0, -absolute);\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp0, as_float(0x3b6a2228));\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3bfaa371));\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp0, temp1);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3cd97055));\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp0, temp1);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3df15b6b));\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp0, temp1);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3fb504f4));\n"
"       temp2 = sqrt(temp0);\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp3, temp2, temp2);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp0, temp3, -temp0);\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp3, temp2, 2.0);\n"
"       _viv_asm(DIV, temp0, temp0, temp3);\n"
"       _viv_asm(ADD!<rnd:RTZ>, temp0, temp2, -temp0);\n"
"       _viv_asm(MULLO!<rnd:RTZ>, temp2, temp1, temp0);\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp2, temp2, 2.0);\n"
"       _viv_asm(MUL!<rnd:RTZ>, temp1, temp0, temp1);\n"
"       _viv_asm(ADD!<rnd:RTZ>, result, temp1, temp2);\n"
"       if(a < 0.0)\n"
"       {\n"
"           _viv_asm(MUL!<rnd:RTZ>, result, result, -1.0);\n"
"           _viv_asm(ADD!<rnd:RTZ>, result, result, as_float(0x40490fdb));\n"
"       }\n"
"   }\n"
"   return result;\n"
"}\n"
"float2 _viv_acos_float2(float2 a)\n"
"{\n"
"   float2 result;\n"
"   result.s0 = _viv_acos_float(a.s0);\n"
"   result.s1 = _viv_acos_float(a.s1);\n"
"   return result;\n"
"}\n"
"float3 _viv_acos_float3(float3 a)\n"
"{\n"
"   float3 result;\n"
"   result.s01 = _viv_acos_float2(a.s01);\n"
"   result.s2 = _viv_acos_float(a.s2);\n"
"   return result;\n"
"}\n"
"float4 _viv_acos_float4(float4 a)\n"
"{\n"
"   float4 result;\n"
"   result.s01 = _viv_acos_float2(a.s01);\n"
"   result.s23 = _viv_acos_float2(a.s23);\n"
"   return result;\n"
"}\n"
"float8 _viv_acos_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_acos_float4(a.s0123);\n"
"   result.s4567 = _viv_acos_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_acos_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_acos_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_acos_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n"
;

gctSTRING gcCLLibACOS_Funcs_halti5 =
"float _viv_acos_float(float a)\n"
"{\n"
"    float2 vec2Temp0;\n"
"    float floatTemp0, floatTemp1, result;\n"
"    floatTemp0 = 1.0 + a;\n"
"    floatTemp1 = 1.0 - a;\n"
"    floatTemp0 = floatTemp0 * floatTemp1;\n"
"    floatTemp0 = sqrt(floatTemp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, a, floatTemp0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x0);\n"
"    result = floatTemp0 * vec2Temp0.s0 + vec2Temp0.s1;\n"
"    return result;\n"
"}\n"
"float2 _viv_acos_float2(float2 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1;\n"
"    float2 vec2Temp2, result;\n"
"    vec2Temp0 = (float2)(1.0) + a;\n"
"    vec2Temp1 = (float2)(1.0) - a;\n"
"    vec2Temp2 = vec2Temp0 * vec2Temp1;\n"
"    vec2Temp2 = sqrt(vec2Temp2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, a.s0, vec2Temp2.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, a.s1, vec2Temp2.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x0);\n"
"    result.s0 = vec2Temp2.s0 * vec2Temp0.s0 + vec2Temp0.s1;\n"
"    result.s1 = vec2Temp2.s1 * vec2Temp1.s0 + vec2Temp1.s1;\n"
"    return result;\n"
"}\n"
"float3 _viv_acos_float3(float3 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2;\n"
"    float3 vec3Temp0, vec3Temp1, result;\n"
"    vec3Temp0 = (float3)(1.0) + a;\n"
"    vec3Temp1 = (float3)(1.0) - a;\n"
"    vec3Temp0 = vec3Temp0 * vec3Temp1;\n"
"    vec3Temp0 = sqrt(vec3Temp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, a.s0, vec3Temp0.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, a.s1, vec3Temp0.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, a.s2, vec3Temp0.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x0);\n"
"    result.s0 = vec3Temp0.s0 * vec2Temp0.s0 + vec2Temp0.s1;\n"
"    result.s1 = vec3Temp0.s1 * vec2Temp1.s0 + vec2Temp1.s1;\n"
"    result.s2 = vec3Temp0.s2 * vec2Temp2.s0 + vec2Temp2.s1;\n"
"    return result;\n"
"}\n"
"float4 _viv_acos_float4(float4 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2, vec2Temp3;\n"
"    float4 vec4Temp0, vec4Temp1, result;\n"
"    vec4Temp0 = (float4)(1.0) + a;\n"
"    vec4Temp1 = (float4)(1.0) - a;\n"
"    vec4Temp0 = vec4Temp0 * vec4Temp1;\n"
"    vec4Temp0 = sqrt(vec4Temp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, a.s0, vec4Temp0.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, a.s1, vec4Temp0.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, a.s2, vec4Temp0.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp3, a.s3, vec4Temp0.s3);\n"
"    _viv_asm(ARCTRIG1, vec2Temp3, vec2Temp3, 0x0);\n"
"    result.s0 = vec4Temp0.s0 * vec2Temp0.s0 + vec2Temp0.s1;\n"
"    result.s1 = vec4Temp0.s1 * vec2Temp1.s0 + vec2Temp1.s1;\n"
"    result.s2 = vec4Temp0.s2 * vec2Temp2.s0 + vec2Temp2.s1;\n"
"    result.s3 = vec4Temp0.s3 * vec2Temp3.s0 + vec2Temp3.s1;\n"
"    return result;\n"
"}\n"
"float8 _viv_acos_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_acos_float4(a.s0123);\n"
"   result.s4567 = _viv_acos_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_acos_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_acos_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_acos_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n"
;

gctSTRING gcCLLibACOS_Funcs_halti5_fmaSupported =
"float _viv_fma_float(float a, float b, float c);\n"
"float _viv_acos_float(float a)\n"
"{\n"
"    float2 vec2Temp0;\n"
"    float floatTemp0, result;\n"
"    floatTemp0 = _viv_fma_float(a, -a, 1.0);\n"
"    floatTemp0 = sqrt(floatTemp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, a, floatTemp0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x0);\n"
"    result = _viv_fma_float(floatTemp0, vec2Temp0.s0, vec2Temp0.s1);\n"
"    return result;\n"
"}\n"
"float2 _viv_fma_float2(float2 a, float2 b, float2 c);\n"
"float2 _viv_acos_float2(float2 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1;\n"
"    float2 vec2Temp2, result;\n"
"    vec2Temp2 = _viv_fma_float2(a, -a, (float2)(1.0));\n"
"    vec2Temp2 = sqrt(vec2Temp2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, a.s0, vec2Temp2.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, a.s1, vec2Temp2.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x0);\n"
"    result.s0 = _viv_fma_float(vec2Temp2.s0, vec2Temp0.s0, vec2Temp0.s1);\n"
"    result.s1 = _viv_fma_float(vec2Temp2.s1, vec2Temp1.s0, vec2Temp1.s1);\n"
"    return result;\n"
"}\n"
"float3 _viv_fma_float3(float3 a, float3 b, float3 c);\n"
"float3 _viv_acos_float3(float3 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2;\n"
"    float3 vec3Temp0, result;\n"
"    vec3Temp0 = _viv_fma_float3(a, -a, (float3)(1.0));\n"
"    vec3Temp0 = sqrt(vec3Temp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, a.s0, vec3Temp0.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, a.s1, vec3Temp0.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, a.s2, vec3Temp0.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x0);\n"
"    result.s0 = _viv_fma_float(vec3Temp0.s0, vec2Temp0.s0, vec2Temp0.s1);\n"
"    result.s1 = _viv_fma_float(vec3Temp0.s1, vec2Temp1.s0, vec2Temp1.s1);\n"
"    result.s2 = _viv_fma_float(vec3Temp0.s2, vec2Temp2.s0, vec2Temp2.s1);\n"
"    return result;\n"
"}\n"
"float4 _viv_fma_float4(float4 a, float4 b, float4 c);\n"
"float4 _viv_acos_float4(float4 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2, vec2Temp3;\n"
"    float4 vec4Temp0, result;\n"
"    vec4Temp0 = _viv_fma_float4(a, -a, (float4)(1.0));\n"
"    vec4Temp0 = sqrt(vec4Temp0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, a.s0, vec4Temp0.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, a.s1, vec4Temp0.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, a.s2, vec4Temp0.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x0);\n"
"    _viv_asm(ARCTRIG0, vec2Temp3, a.s3, vec4Temp0.s3);\n"
"    _viv_asm(ARCTRIG1, vec2Temp3, vec2Temp3, 0x0);\n"
"    result.s0 = _viv_fma_float(vec4Temp0.s0, vec2Temp0.s0, vec2Temp0.s1);\n"
"    result.s1 = _viv_fma_float(vec4Temp0.s1, vec2Temp1.s0, vec2Temp1.s1);\n"
"    result.s2 = _viv_fma_float(vec4Temp0.s2, vec2Temp2.s0, vec2Temp2.s1);\n"
"    result.s3 = _viv_fma_float(vec4Temp0.s3, vec2Temp3.s0, vec2Temp3.s1);\n"
"    return result;\n"
"}\n"
"float8 _viv_acos_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_acos_float4(a.s0123);\n"
"   result.s4567 = _viv_acos_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_acos_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_acos_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_acos_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n"
;

gctSTRING gcCLLibATAN_Funcs =
"float _viv_atan_float(float a)\n"
"{\n"
"   float result;\n"
"   _viv_asm(ATAN, result, a);\n"
"   return result;\n"
"}\n"
"float2 _viv_atan_float2(float2 a)\n"
"{\n"
"   float2 result;\n"
"   _viv_asm(ATAN, result, a);\n"
"   return result;\n"
"}\n"
"float3 _viv_atan_float3(float3 a)\n"
"{\n"
"   float3 result;\n"
"   _viv_asm(ATAN, result, a);\n"
"   return result;\n"
"}\n"
"float4 _viv_atan_float4(float4 a)\n"
"{\n"
"   float4 result;\n"
"   _viv_asm(ATAN, result, a);\n"
"   return result;\n"
"}\n"
"float8 _viv_atan_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_atan_float4(a.s0123);\n"
"   result.s4567 = _viv_atan_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_atan_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_atan_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_atan_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n"
;

    gctSTRING gcCLLibATAN_Funcs_halti2 =
"float _viv_atan_float(float a)\n"
"{\n"
"   float result;\n"
"   float signOfA = as_float((as_uint(a) & 0x80000000) | as_uint(1.0));\n"
"   float temp0, temp1, temp2, temp3;\n"
"   if(fabs(a) > 1.0)\n"
"   {\n"
"       _viv_asm(INVERSE, temp0, a);\n"
"       temp0 = temp0 * signOfA;\n"
"       temp1 = as_float(0x3fc90fdb);\n"
"       temp2 = -1.0;\n"
"   }\n"
"   else\n"
"   {\n"
"       temp0 = fabs(a);\n"
"       temp1 = 0.0;\n"
"       temp2 = 1.0;\n"
"   }\n"
"   _viv_asm(MUL!<rnd:RTZ>, result, temp0, temp0);\n"
"   _viv_asm(MUL!<rnd:RTZ>, temp3, result, as_float(0x3b42f491));\n"
"   _viv_asm(ADD!<rnd:RTZ>, temp3, temp3, as_float(0xbc87d6c3));\n"
"   _viv_asm(MUL!<rnd:RTZ>, temp3, temp3, result);\n"
"   _viv_asm(ADD!<rnd:RTZ>, temp3, temp3, as_float(0x3d326651));\n"
"   _viv_asm(MUL!<rnd:RTZ>, temp3, temp3, result);\n"
"   _viv_asm(ADD!<rnd:RTZ>, temp3, temp3, as_float(0xbd9b407e));\n"
"   _viv_asm(MUL!<rnd:RTZ>, temp3, temp3, result);\n"
"   _viv_asm(ADD!<rnd:RTZ>, temp3, temp3, as_float(0x3ddab495));\n"
"   _viv_asm(MUL!<rnd:RTZ>, temp3, temp3, result);\n"
"   _viv_asm(ADD!<rnd:RTZ>, temp3, temp3, as_float(0xbe118db7));\n"
"   _viv_asm(MUL!<rnd:RTZ>, temp3, temp3, result);\n"
"   _viv_asm(ADD!<rnd:RTZ>, temp3, temp3, as_float(0x3e4cbd6f));\n"
"   _viv_asm(MUL!<rnd:RTZ>, temp3, temp3, result);\n"
"   _viv_asm(ADD!<rnd:RTZ>, temp3, temp3, as_float(0xbeaaaa73));\n"
"   _viv_asm(MUL!<rnd:RTZ>, result, temp3, result);\n"
"   _viv_asm(ADD!<rnd:RTZ>, result, result, 1.0);\n"
"   _viv_asm(MUL!<rnd:RTZ>, result, temp0, result);\n"
"   _viv_asm(MUL!<rnd:RTZ>, result, result, temp2);\n"
"   _viv_asm(ADD!<rnd:RTZ>, result, result, temp1);\n"
"   _viv_asm(MUL!<rnd:RTZ>, result, result, signOfA);\n"
"   return result;\n"
"}\n"
"float2 _viv_atan_float2(float2 a)\n"
"{\n"
"   float2 result;\n"
"   result.s0 = _viv_atan_float(a.s0);\n"
"   result.s1 = _viv_atan_float(a.s1);\n"
"   return result;\n"
"}\n"
"float3 _viv_atan_float3(float3 a)\n"
"{\n"
"   float3 result;\n"
"   result.s01 = _viv_atan_float2(a.s01);\n"
"   result.s2 = _viv_atan_float(a.s2);\n"
"   return result;\n"
"}\n"
"float4 _viv_atan_float4(float4 a)\n"
"{\n"
"   float4 result;\n"
"   result.s01 = _viv_atan_float2(a.s01);\n"
"   result.s23 = _viv_atan_float2(a.s23);\n"
"   return result;\n"
"}\n"
"float8 _viv_atan_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_atan_float4(a.s0123);\n"
"   result.s4567 = _viv_atan_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_atan_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_atan_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_atan_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n"
;

gctSTRING gcCLLibATAN_Funcs_halti5 =
"float _viv_atan_float(float a)\n"
"{\n"
"    float2 vec2Temp0;\n"
"    float floatTemp0, floatTemp1, result;\n"
"    floatTemp0 = a * a + 1.0;\n"
"    floatTemp0 = rsqrt(floatTemp0);\n"
"    _viv_asm(MUL_Z, floatTemp1, floatTemp0, a);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, floatTemp0, floatTemp1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x2);\n"
"    result = floatTemp1 * vec2Temp0.s0 + vec2Temp0.s1;\n"
"    return result;\n"
"}\n"
"float2 _viv_atan_float2(float2 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1;\n"
"    float2 vec2Temp2, vec2Temp3, result;\n"
"    vec2Temp2 = a * a + 1.0;\n"
"    vec2Temp2 = rsqrt(vec2Temp2);\n"
"    _viv_asm(MUL_Z, vec2Temp3, vec2Temp2, a);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec2Temp2.s0, vec2Temp3.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec2Temp2.s1, vec2Temp3.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x2);\n"
"    result.s0 = vec2Temp3.x * vec2Temp0.s0 + vec2Temp0.s1;\n"
"    result.s1 = vec2Temp3.y * vec2Temp1.s0 + vec2Temp1.s1;\n"
"    return result;\n"
"}\n"
"float3 _viv_atan_float3(float3 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2;\n"
"    float3 vec3Temp0, vec3Temp1, result;\n"
"    vec3Temp0 = a * a + 1.0;\n"
"    vec3Temp0 = rsqrt(vec3Temp0);\n"
"    _viv_asm(MUL_Z, vec3Temp1, vec3Temp0, a);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec3Temp0.s0, vec3Temp1.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec3Temp0.s1, vec3Temp1.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, vec3Temp0.s2, vec3Temp1.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x2);\n"
"    result.s0 = vec3Temp1.s0 * vec2Temp0.s0 + vec2Temp0.s1;\n"
"    result.s1 = vec3Temp1.s1 * vec2Temp1.s0 + vec2Temp1.s1;\n"
"    result.s2 = vec3Temp1.s2 * vec2Temp2.s0 + vec2Temp2.s1;\n"
"    return result;\n"
"}\n"
"float4 _viv_atan_float4(float4 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2, vec2Temp3;\n"
"    float4 vec4Temp0, vec4Temp1, result;\n"
"    vec4Temp0 = a * a + 1.0;\n"
"    vec4Temp0 = rsqrt(vec4Temp0);\n"
"    _viv_asm(MUL_Z, vec4Temp1, vec4Temp0, a);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec4Temp0.s0, vec4Temp1.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec4Temp0.s1, vec4Temp1.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, vec4Temp0.s2, vec4Temp1.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp3, vec4Temp0.s3, vec4Temp1.s3);\n"
"    _viv_asm(ARCTRIG1, vec2Temp3, vec2Temp3, 0x2);\n"
"    result.s0 = vec4Temp1.s0 * vec2Temp0.s0 + vec2Temp0.s1;\n"
"    result.s1 = vec4Temp1.s1 * vec2Temp1.s0 + vec2Temp1.s1;\n"
"    result.s2 = vec4Temp1.s2 * vec2Temp2.s0 + vec2Temp2.s1;\n"
"    result.s3 = vec4Temp1.s3 * vec2Temp3.s0 + vec2Temp3.s1;\n"
"    return result;\n"
"}\n"
"float8 _viv_atan_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_atan_float4(a.s0123);\n"
"   result.s4567 = _viv_atan_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_atan_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_atan_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_atan_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n"
;

gctSTRING gcCLLibATAN_Funcs_halti5_fmaSupported =
"float _viv_fma_float(float a, float b, float c);\n"
"float _viv_atan_float(float a)\n"
"{\n"
"    float2 vec2Temp0;\n"
"    float floatTemp0, floatTemp1, result;\n"
"    floatTemp0 = _viv_fma_float(a, a, 1.0);\n"
"    floatTemp0 = rsqrt(floatTemp0);\n"
"    _viv_asm(MUL_Z, floatTemp1, floatTemp0, a);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, floatTemp0, floatTemp1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x2);\n"
"    result = _viv_fma_float(floatTemp1, vec2Temp0.s0, vec2Temp0.s1);\n"
"    return result;\n"
"}\n"
"float2 _viv_fma_float2(float2 a, float2 b, float2 c);\n"
"float2 _viv_atan_float2(float2 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1;\n"
"    float2 vec2Temp2, vec2Temp3, result;\n"
"    vec2Temp2 = _viv_fma_float2(a, a, 1.0);\n"
"    vec2Temp2 = rsqrt(vec2Temp2);\n"
"    _viv_asm(MUL_Z, vec2Temp3, vec2Temp2, a);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec2Temp2.s0, vec2Temp3.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec2Temp2.s1, vec2Temp3.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x2);\n"
"    result.s0 = _viv_fma_float(vec2Temp3.x, vec2Temp0.s0, vec2Temp0.s1);\n"
"    result.s1 = _viv_fma_float(vec2Temp3.y, vec2Temp1.s0, vec2Temp1.s1);\n"
"    return result;\n"
"}\n"
"float3 _viv_fma_float3(float3 a, float3 b, float3 c);\n"
"float3 _viv_atan_float3(float3 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2;\n"
"    float3 vec3Temp0, vec3Temp1, result;\n"
"    vec3Temp0 = _viv_fma_float3(a, a, 1.0);\n"
"    vec3Temp0 = rsqrt(vec3Temp0);\n"
"    _viv_asm(MUL_Z, vec3Temp1, vec3Temp0, a);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec3Temp0.s0, vec3Temp1.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec3Temp0.s1, vec3Temp1.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, vec3Temp0.s2, vec3Temp1.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x2);\n"
"    result.s0 = _viv_fma_float(vec3Temp1.s0, vec2Temp0.s0, vec2Temp0.s1);\n"
"    result.s1 = _viv_fma_float(vec3Temp1.s1, vec2Temp1.s0, vec2Temp1.s1);\n"
"    result.s2 = _viv_fma_float(vec3Temp1.s2, vec2Temp2.s0, vec2Temp2.s1);\n"
"    return result;\n"
"}\n"
"float4 _viv_fma_float4(float4 a, float4 b, float4 c);\n"
"float4 _viv_atan_float4(float4 a)\n"
"{\n"
"    float2 vec2Temp0, vec2Temp1, vec2Temp2, vec2Temp3;\n"
"    float4 vec4Temp0, vec4Temp1, result;\n"
"    vec4Temp0 = _viv_fma_float4(a, a, 1.0);\n"
"    vec4Temp0 = rsqrt(vec4Temp0);\n"
"    _viv_asm(MUL_Z, vec4Temp1, vec4Temp0, a);\n"
"    _viv_asm(ARCTRIG0, vec2Temp0, vec4Temp0.s0, vec4Temp1.s0);\n"
"    _viv_asm(ARCTRIG1, vec2Temp0, vec2Temp0, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp1, vec4Temp0.s1, vec4Temp1.s1);\n"
"    _viv_asm(ARCTRIG1, vec2Temp1, vec2Temp1, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp2, vec4Temp0.s2, vec4Temp1.s2);\n"
"    _viv_asm(ARCTRIG1, vec2Temp2, vec2Temp2, 0x2);\n"
"    _viv_asm(ARCTRIG0, vec2Temp3, vec4Temp0.s3, vec4Temp1.s3);\n"
"    _viv_asm(ARCTRIG1, vec2Temp3, vec2Temp3, 0x2);\n"
"    result.s0 = _viv_fma_float(vec4Temp1.s0, vec2Temp0.s0, vec2Temp0.s1);\n"
"    result.s1 = _viv_fma_float(vec4Temp1.s1, vec2Temp1.s0, vec2Temp1.s1);\n"
"    result.s2 = _viv_fma_float(vec4Temp1.s2, vec2Temp2.s0, vec2Temp2.s1);\n"
"    result.s3 = _viv_fma_float(vec4Temp1.s3, vec2Temp3.s0, vec2Temp3.s1);\n"
"    return result;\n"
"}\n"
"float8 _viv_atan_float8(float8 a)\n"
"{\n"
"   float8 result;\n"
"   result.s0123 = _viv_atan_float4(a.s0123);\n"
"   result.s4567 = _viv_atan_float4(a.s4567);\n"
"   return result;\n"
"}\n"
"float16 _viv_atan_float16(float16 a)\n"
"{\n"
"   float16 result;\n"
"   result.s01234567 = _viv_atan_float8(a.s01234567);\n"
"   result.s89abcdef = _viv_atan_float8(a.s89abcdef);\n"
"   return result;\n"
"}\n"
;

gctSTRING gcCLLibATAN2_Funcs =
"float _viv_atan2_float(float y, float x)\n"
"{\n"
"    float result;\n"
"    if (x == -0.0)\n"
"    {\n"
"         if (y == 0.0)  return  3.14159265358979323846f;\n"
"         if (y == -0.0) return -3.14159265358979323846f;\n"
"         if (y < 0.0)   return -1.5707963267948966f;\n"
"         return 1.5707963267948966f;\n"
"    }\n"
"    else if (x == 0.0)\n"
"    {\n"
"         if (y == 0.0)  return  0.0;\n"
"         if (y == -0.0) return -0.0;\n"
"         if (y < 0.0)   return -1.5707963267948966f;\n"
"         return 1.5707963267948966f;\n"
"    }\n"
"    _viv_asm(ATAN, result, y / x);\n"
"    if(signbit(x) == 1)\n"
"    {\n"
"        if(result <= 0.0)\n"
"        {\n"
"            result = result + 3.14159265358979323846f;\n"
"        }\n"
"        else if(result >= -0.0)\n"
"        {\n"
"            result = result - 3.14159265358979323846f;\n"
"        }\n"
"    }\n"
"    return result;\n"
"}\n"
"float2 _viv_atan2_float2(float2 y, float2 x)\n"
"{\n"
"    float2 result;\n"
"    result.x = _viv_atan2_float(y.x, x.x);\n"
"    result.y = _viv_atan2_float(y.y, x.y);\n"
"    return result;\n"
"}\n"
"float3 _viv_atan2_float3(float3 y, float3 x)\n"
"{\n"
"    float3 result;\n"
"    result.x = _viv_atan2_float(y.x, x.x);\n"
"    result.y = _viv_atan2_float(y.y, x.y);\n"
"    result.z = _viv_atan2_float(y.z, x.z);\n"
"    return result;\n"
"}\n"
"float4 _viv_atan2_float4(float4 y, float4 x)\n"
"{\n"
"    float4 result;\n"
"    result.x = _viv_atan2_float(y.x, x.x);\n"
"    result.y = _viv_atan2_float(y.y, x.y);\n"
"    result.z = _viv_atan2_float(y.z, x.z);\n"
"    result.w = _viv_atan2_float(y.w, x.w);\n"
"    return result;\n"
"}\n"
"float8 _viv_atan2_float8(float8 y, float8 x)\n"
"{\n"
"    float8 result;\n"
"    result.s0123 = _viv_atan2_float4(y.s0123, x.s0123);\n"
"    result.s4567 = _viv_atan2_float4(y.s4567, x.s4567);\n"
"    return result;\n"
"}\n"
"float16 _viv_atan2_float16(float16 y, float16 x)\n"
"{\n"
"    float16 result;\n"
"    result.s0123 = _viv_atan2_float4(y.s0123, x.s0123);\n"
"    result.s4567 = _viv_atan2_float4(y.s4567, x.s4567);\n"
"    result.s89ab = _viv_atan2_float4(y.s89ab, x.s89ab);\n"
"    result.scdef = _viv_atan2_float4(y.scdef, x.scdef);\n"
"    return result;\n"
"}\n";

gctSTRING gcCLLibATAN2_Funcs_halti2 =
"float _viv_atan2_float(float y, float x)\n"
"{\n"
"    float r1z, r1w, r2x, r2y, r2z, r2w, r3y, r3w, temp0, result;\n"
"    uint r1x, r3x, r3z, r4x, r4y;\n"
"    r3x = 0;\n"
"    r3y = 0.0 + fabs(y);\n"
"    r3z = 0x80000000 & as_uint(y);\n"
"    r3w = 0.0 + fabs(x);\n"
"    r4x = 0x80000000 & as_uint(x);\n"
"    _viv_asm(ADD!<rnd:RTZ>, temp0, r3y, r3w);\n"
"    r4y = 0x7fffffff & as_uint(temp0);\n"
"    if(0x7f800000 < r4y)\n"
"    {\n"
"        r3x = 0x7f800000;\n"
"        result = as_float(r4y);\n"
"    }\n"
"    else\n"
"    {\n"
"        if(0x80000000 != r4x)\n"
"        {\n"
"            r1x = 0;\n"
"        }\n"
"        else\n"
"        {\n"
"            r1x = 0x40490fdb;\n"
"        }\n"
"        if(r3y == 0.0)\n"
"        {\n"
"            r3x = 0x7f800000;\n"
"            result = as_float(r3z | r1x);\n"
"        }\n"
"        else\n"
"        {\n"
"            if(r3w == as_float(0x7f800000))\n"
"            {\n"
"                if(0x7f800000 == as_uint(r3y))\n"
"                {\n"
"                    float temp1;\n"
"                    _viv_asm(MUL!<rnd:RTZ>, temp1, 0.5, as_float(r1x));\n"
"                    _viv_asm(ADD!<rnd:RTZ>, temp1, temp1, as_float(0x3f490fdb));\n"
"                    r3x = 0x7f800000;\n"
"                    result = as_float(r3z | as_uint(temp1));\n"
"                }\n"
"                else\n"
"                {\n"
"                    r3x = 0x7f800000;\n"
"                    result = as_float(r3z | r1x);\n"
"                }\n"
"            }\n"
"            else\n"
"            {\n"
"                if(0x7f800000 == as_uint(r3y))\n"
"                {\n"
"                    r3x = 0x7f800000;\n"
"                    result = as_float(r3z | 0x3fc90fdb);\n"
"                }\n"
"            }\n"
"        }\n"
"    }\n"
"    if(0 == r3x)\n"
"    {\n"
"        r3x = 0x7fffffff & as_uint(x);\n"
"        if(r3x > 0x7e800000)\n"
"        {\n"
"            r3x = as_uint(0.25 * x);\n"
"            r3y = 0.25;\n"
"        }\n"
"        else\n"
"        {\n"
"            r3x = as_uint(x);\n"
"            r3y = 1.0;\n"
"        }\n"
"        _viv_asm(INVERSE, r2x, as_float(r3x));\n"
"        if(fabs(r2x) < as_float(0x33800000))\n"
"        {\n"
"            r2y = 16777216.0;\n"
"            r2z = as_float(0x33800000);\n"
"        }\n"
"        else\n"
"        {\n"
"            r2y = 1.0;\n"
"            r2z = 1.0;\n"
"        }\n"
"        r2w = (-r2x) * as_float(r3x) + 1.0;\n"
"        r2x = r2y * r2x;\n"
"        r2y = r2w * r2x;\n"
"        r2w = as_float(0x007fffff & as_uint(r2x));\n"
"        if(0 != as_uint(r2w))\n"
"        {\n"
"            r2w = as_float(1);\n"
"        }\n"
"        r2y = as_float(as_uint(r2w) * as_uint(r2y));\n"
"        r2x = r2y + r2x;\n"
"        r2x = r2z * r2x;\n"
"        r1z = r2x * y;\n"
"        r1z = r1z * r3y;\n"
"        if(fabs(r1z) > 1.0)\n"
"        {\n"
"            if(r1z >= 0.0)\n"
"            {\n"
"                _viv_asm(INVERSE, r1w, r1z);\n"
"                r2x = as_float(0x3fc90fdb);\n"
"                r2y = -1.0;\n"
"                r2z = 1.0;\n"
"            }\n"
"            else\n"
"            {\n"
"                _viv_asm(INVERSE, r1w, r1z);\n"
"                r1w = -r1w;\n"
"                r2x = as_float(0x3fc90fdb);\n"
"                r2y = -1.0;\n"
"                r2z = -1.0;\n"
"            }\n"
"        }\n"
"        else\n"
"        {\n"
"            if(r1z >= 0.0)\n"
"            {\n"
"                r1w = r1z;\n"
"                r2x = 0.0;\n"
"                r2y = 1.0;\n"
"                r2z = 1.0;\n"
"            }\n"
"            else\n"
"            {\n"
"                r1w = -r1z;\n"
"                r2x = 0.0;\n"
"                r2y = 1.0;\n"
"                r2z = -1.0;\n"
"            }\n"
"        }\n"
"       _viv_asm(MUL!<rnd:RTZ>, r1z, r1w, r1w);\n"
"       _viv_asm(MUL!<rnd:RTZ>, r2w, r1z, as_float(0x3b42f491));\n"
"       _viv_asm(ADD!<rnd:RTZ>, r2w, r2w, as_float(0xbc87d6c3));\n"
"       _viv_asm(MUL!<rnd:RTZ>, r2w, r2w, r1z);\n"
"       _viv_asm(ADD!<rnd:RTZ>, r2w, r2w, as_float(0x3d326651));\n"
"       _viv_asm(MUL!<rnd:RTZ>, r2w, r2w, r1z);\n"
"       _viv_asm(ADD!<rnd:RTZ>, r2w, r2w, as_float(0xbd9b407e));\n"
"       _viv_asm(MUL!<rnd:RTZ>, r2w, r2w, r1z);\n"
"       _viv_asm(ADD!<rnd:RTZ>, r2w, r2w, as_float(0x3ddab495));\n"
"       _viv_asm(MUL!<rnd:RTZ>, r2w, r2w, r1z);\n"
"       _viv_asm(ADD!<rnd:RTZ>, r2w, r2w, as_float(0xbe118db7));\n"
"       _viv_asm(MUL!<rnd:RTZ>, r2w, r2w, r1z);\n"
"       _viv_asm(ADD!<rnd:RTZ>, r2w, r2w, as_float(0x3e4cbd6f));\n"
"       _viv_asm(MUL!<rnd:RTZ>, r2w, r2w, r1z);\n"
"       _viv_asm(ADD!<rnd:RTZ>, r2w, r2w, as_float(0xbeaaaa73));\n"
"       _viv_asm(MUL!<rnd:RTZ>, r1z, r2w, r1z);\n"
"       _viv_asm(ADD!<rnd:RTZ>, r1z, r1z, 1.0);\n"
"       _viv_asm(MUL!<rnd:RTZ>, r1z, r1w, r1z);\n"
"       _viv_asm(MUL!<rnd:RTZ>, r1z, r1z, r2y);\n"
"       _viv_asm(ADD!<rnd:RTZ>, r1z, r1z, r2x);\n"
"        r1x = r3z | r1x;\n"
"       _viv_asm(MUL!<rnd:RTZ>, r1z, r1z, r2z);\n"
"       _viv_asm(ADD!<rnd:RTZ>, result, r1z, as_float(r1x));\n"
"    }\n"
"    return result;\n"
"}\n"
"float2 _viv_atan2_float2(float2 y, float2 x)\n"
"{\n"
"    return (float2)(_viv_atan2_float(y.s0, x.s0), \n"
"                    _viv_atan2_float(y.s1, x.s1));\n"
"}\n"
"float3 _viv_atan2_float3(float3 y, float3 x)\n"
"{\n"
"    return (float3)(_viv_atan2_float(y.s0, x.s0), \n"
"                    _viv_atan2_float(y.s1, x.s1), \n"
"                    _viv_atan2_float(y.s2, x.s2));\n"
"}\n"
"float4 _viv_atan2_float4(float4 y, float4 x)\n"
"{\n"
"    return (float4)(_viv_atan2_float(y.s0, x.s0), \n"
"                    _viv_atan2_float(y.s1, x.s1), \n"
"                    _viv_atan2_float(y.s2, x.s2), \n"
"                    _viv_atan2_float(y.s3, x.s3));\n"
"}\n"
"float8 _viv_atan2_float8(float8 y, float8 x)\n"
"{\n"
"    float8 result;\n"
"    result.s0123 = _viv_atan2_float4(y.s0123, x.s0123);\n"
"    result.s4567 = _viv_atan2_float4(y.s4567, x.s4567);\n"
"    return result;\n"
"}\n"
"float16 _viv_atan2_float16(float16 y, float16 x)\n"
"{\n"
"    float16 result;\n"
"    result.s0123 = _viv_atan2_float4(y.s0123, x.s0123);\n"
"    result.s4567 = _viv_atan2_float4(y.s4567, x.s4567);\n"
"    result.s89ab = _viv_atan2_float4(y.s89ab, x.s89ab);\n"
"    result.scdef = _viv_atan2_float4(y.scdef, x.scdef);\n"
"    return result;\n"
"}\n"
;

gctSTRING gcCLLibATAN2_Funcs_halti5 =
"float _viv_atan2_float(float y, float x)\n"
"{\n"
"    float2 arctrig0, vec2Temp0;\n"
"    float dot0, rsq0, result;\n"
"    _viv_asm(ARCTRIG0, arctrig0, y, x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x83);\n"
"    dot0 = arctrig0.x * arctrig0.x + arctrig0.y * arctrig0.y;\n"
"    _viv_asm(RSQ, rsq0, dot0);\n"
"    vec2Temp0 = arctrig0 * rsq0;\n"
"    _viv_asm(ARCTRIG0, arctrig0, vec2Temp0.y, vec2Temp0.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x3);\n"
"    result = arctrig0.x * vec2Temp0.x + arctrig0.y;\n"
"    return result;\n"
"}\n"
"float2 _viv_atan2_float2(float2 y, float2 x)\n"
"{\n"
"    float2 arctrig0, arctrig1, vec2Temp0, vec2Temp1;\n"
"    float2 dot0, rsq0, result;\n"
"    _viv_asm(ARCTRIG0, arctrig0, y.x, x.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig1, y.y, x.y);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x83);\n"
"    dot0.x = arctrig0.x * arctrig0.x + arctrig0.y * arctrig0.y;\n"
"    dot0.y = arctrig1.x * arctrig1.x + arctrig1.y * arctrig1.y;\n"
"    _viv_asm(RSQ, rsq0, dot0);\n"
"    vec2Temp0 = arctrig0 * rsq0.x;\n"
"    vec2Temp1 = arctrig1 * rsq0.y;\n"
"    _viv_asm(ARCTRIG0, arctrig0, vec2Temp0.y, vec2Temp0.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig1, vec2Temp1.y, vec2Temp1.x);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x3);\n"
"    result.x = arctrig0.x * vec2Temp0.x + arctrig0.y;\n"
"    result.y = arctrig1.x * vec2Temp1.x + arctrig1.y;\n"
"    return result;\n"
"}\n"
"float3 _viv_atan2_float3(float3 y, float3 x)\n"
"{\n"
"    float2 arctrig0, arctrig1, arctrig2, vec3Temp0, vec3Temp1, vec3Temp2;\n"
"    float3 dot0, rsq0, result;\n"
"    _viv_asm(ARCTRIG0, arctrig0, y.x, x.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig1, y.y, x.y);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig2, y.z, x.z);\n"
"    _viv_asm(ARCTRIG1, arctrig2, arctrig2, 0x83);\n"
"    dot0.x = arctrig0.x * arctrig0.x + arctrig0.y * arctrig0.y;\n"
"    dot0.y = arctrig1.x * arctrig1.x + arctrig1.y * arctrig1.y;\n"
"    dot0.z = arctrig2.x * arctrig2.x + arctrig2.y * arctrig2.y;\n"
"    _viv_asm(RSQ, rsq0, dot0);\n"
"    vec3Temp0 = arctrig0 * rsq0.x;\n"
"    vec3Temp1 = arctrig1 * rsq0.y;\n"
"    vec3Temp2 = arctrig2 * rsq0.z;\n"
"    _viv_asm(ARCTRIG0, arctrig0, vec3Temp0.y, vec3Temp0.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig1, vec3Temp1.y, vec3Temp1.x);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig2, vec3Temp2.y, vec3Temp2.x);\n"
"    _viv_asm(ARCTRIG1, arctrig2, arctrig2, 0x3);\n"
"    result.x = arctrig0.x * vec3Temp0.x + arctrig0.y;\n"
"    result.y = arctrig1.x * vec3Temp1.x + arctrig1.y;\n"
"    result.z = arctrig2.x * vec3Temp2.x + arctrig2.y;\n"
"    return result;\n"
"}\n"
"float4 _viv_atan2_float4(float4 y, float4 x)\n"
"{\n"
"    float2 arctrig0, arctrig1, arctrig2, arctrig3, vec4Temp0, vec4Temp1, vec4Temp2, vec4Temp3;\n"
"    float4 dot0, rsq0, result;\n"
"    _viv_asm(ARCTRIG0, arctrig0, y.x, x.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig1, y.y, x.y);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig2, y.z, x.z);\n"
"    _viv_asm(ARCTRIG1, arctrig2, arctrig2, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig3, y.w, x.w);\n"
"    _viv_asm(ARCTRIG1, arctrig3, arctrig3, 0x83);\n"
"    dot0.x = arctrig0.x * arctrig0.x + arctrig0.y * arctrig0.y;\n"
"    dot0.y = arctrig1.x * arctrig1.x + arctrig1.y * arctrig1.y;\n"
"    dot0.z = arctrig2.x * arctrig2.x + arctrig2.y * arctrig2.y;\n"
"    dot0.w = arctrig3.x * arctrig3.x + arctrig3.y * arctrig3.y;\n"
"    _viv_asm(RSQ, rsq0, dot0);\n"
"    vec4Temp0 = arctrig0 * rsq0.x;\n"
"    vec4Temp1 = arctrig1 * rsq0.y;\n"
"    vec4Temp2 = arctrig2 * rsq0.z;\n"
"    vec4Temp3 = arctrig3 * rsq0.w;\n"
"    _viv_asm(ARCTRIG0, arctrig0, vec4Temp0.y, vec4Temp0.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig1, vec4Temp1.y, vec4Temp1.x);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig2, vec4Temp2.y, vec4Temp2.x);\n"
"    _viv_asm(ARCTRIG1, arctrig2, arctrig2, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig3, vec4Temp3.y, vec4Temp3.x);\n"
"    _viv_asm(ARCTRIG1, arctrig3, arctrig3, 0x3);\n"
"    result.x = arctrig0.x * vec4Temp0.x + arctrig0.y;\n"
"    result.y = arctrig1.x * vec4Temp1.x + arctrig1.y;\n"
"    result.z = arctrig2.x * vec4Temp2.x + arctrig2.y;\n"
"    result.w = arctrig3.x * vec4Temp3.x + arctrig3.y;\n"
"    return result;\n"
"}\n"
"float8 _viv_atan2_float8(float8 y, float8 x)\n"
"{\n"
"    float8 result;\n"
"    result.s0123 = _viv_atan2_float4(y.s0123, x.s0123);\n"
"    result.s4567 = _viv_atan2_float4(y.s4567, x.s4567);\n"
"    return result;\n"
"}\n"
"float16 _viv_atan2_float16(float16 y, float16 x)\n"
"{\n"
"    float16 result;\n"
"    result.s0123 = _viv_atan2_float4(y.s0123, x.s0123);\n"
"    result.s4567 = _viv_atan2_float4(y.s4567, x.s4567);\n"
"    result.s89ab = _viv_atan2_float4(y.s89ab, x.s89ab);\n"
"    result.scdef = _viv_atan2_float4(y.scdef, x.scdef);\n"
"    return result;\n"
"}\n"
;
gctSTRING gcCLLibATAN2_Funcs_halti5_fmaSupported =
"float _viv_fma_float(float a, float b, float c);\n"
"float _viv_atan2_float(float y, float x)\n"
"{\n"
"    float2 arctrig0, vec2Temp0;\n"
"    float dot0, rsq0, result;\n"
"    _viv_asm(ARCTRIG0, arctrig0, y, x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x83);\n"
"    dot0 = dot(arctrig0, arctrig0);\n"
"    _viv_asm(RSQ, rsq0, dot0);\n"
"    vec2Temp0 = arctrig0 * rsq0;\n"
"    _viv_asm(ARCTRIG0, arctrig0, vec2Temp0.y, vec2Temp0.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x3);\n"
"    result = _viv_fma_float(arctrig0.x, vec2Temp0.x, arctrig0.y);\n"
"    return result;\n"
"}\n"
"float2 _viv_fma_float2(float2 a, float2 b, float2 c);\n"
"float2 _viv_atan2_float2(float2 y, float2 x)\n"
"{\n"
"    float2 arctrig0, arctrig1, vec2Temp0, vec2Temp1;\n"
"    float2 dot0, rsq0, result;\n"
"    _viv_asm(ARCTRIG0, arctrig0, y.x, x.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig1, y.y, x.y);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x83);\n"
"    dot0.x = dot(arctrig0, arctrig0);\n"
"    dot0.y = dot(arctrig1, arctrig1);\n"
"    _viv_asm(RSQ, rsq0, dot0);\n"
"    vec2Temp0 = arctrig0 * rsq0.x;\n"
"    vec2Temp1 = arctrig1 * rsq0.y;\n"
"    _viv_asm(ARCTRIG0, arctrig0, vec2Temp0.y, vec2Temp0.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig1, vec2Temp1.y, vec2Temp1.x);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x3);\n"
"    result.x = _viv_fma_float(arctrig0.x, vec2Temp0.x, arctrig0.y);\n"
"    result.y = _viv_fma_float(arctrig1.x, vec2Temp1.x, arctrig1.y);\n"
"    return result;\n"
"}\n"
"float3 _viv_fma_float3(float3 a, float3 b, float3 c);\n"
"float3 _viv_atan2_float3(float3 y, float3 x)\n"
"{\n"
"    float2 arctrig0, arctrig1, arctrig2, vec3Temp0, vec3Temp1, vec3Temp2;\n"
"    float3 dot0, rsq0, result;\n"
"    _viv_asm(ARCTRIG0, arctrig0, y.x, x.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig1, y.y, x.y);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig2, y.z, x.z);\n"
"    _viv_asm(ARCTRIG1, arctrig2, arctrig2, 0x83);\n"
"    dot0.x = dot(arctrig0, arctrig0);\n"
"    dot0.y = dot(arctrig1, arctrig1);\n"
"    dot0.z = dot(arctrig2, arctrig2);\n"
"    _viv_asm(RSQ, rsq0, dot0);\n"
"    vec3Temp0 = arctrig0 * rsq0.x;\n"
"    vec3Temp1 = arctrig1 * rsq0.y;\n"
"    vec3Temp2 = arctrig2 * rsq0.z;\n"
"    _viv_asm(ARCTRIG0, arctrig0, vec3Temp0.y, vec3Temp0.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig1, vec3Temp1.y, vec3Temp1.x);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig2, vec3Temp2.y, vec3Temp2.x);\n"
"    _viv_asm(ARCTRIG1, arctrig2, arctrig2, 0x3);\n"
"    result.x = _viv_fma_float(arctrig0.x, vec3Temp0.x, arctrig0.y);\n"
"    result.y = _viv_fma_float(arctrig1.x, vec3Temp1.x, arctrig1.y);\n"
"    result.z = _viv_fma_float(arctrig2.x, vec3Temp2.x, arctrig2.y);\n"
"    return result;\n"
"}\n"
"float4 _viv_fma_float4(float4 a, float4 b, float4 c);\n"
"float4 _viv_atan2_float4(float4 y, float4 x)\n"
"{\n"
"    float2 arctrig0, arctrig1, arctrig2, arctrig3, vec4Temp0, vec4Temp1, vec4Temp2, vec4Temp3;\n"
"    float4 dot0, rsq0, result;\n"
"    _viv_asm(ARCTRIG0, arctrig0, y.x, x.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig1, y.y, x.y);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig2, y.z, x.z);\n"
"    _viv_asm(ARCTRIG1, arctrig2, arctrig2, 0x83);\n"
"    _viv_asm(ARCTRIG0, arctrig3, y.w, x.w);\n"
"    _viv_asm(ARCTRIG1, arctrig3, arctrig3, 0x83);\n"
"    dot0.x = dot(arctrig0, arctrig0);\n"
"    dot0.y = dot(arctrig1, arctrig1);\n"
"    dot0.z = dot(arctrig2, arctrig2);\n"
"    dot0.w = dot(arctrig3, arctrig3);\n"
"    _viv_asm(RSQ, rsq0, dot0);\n"
"    vec4Temp0 = arctrig0 * rsq0.x;\n"
"    vec4Temp1 = arctrig1 * rsq0.y;\n"
"    vec4Temp2 = arctrig2 * rsq0.z;\n"
"    vec4Temp3 = arctrig3 * rsq0.w;\n"
"    _viv_asm(ARCTRIG0, arctrig0, vec4Temp0.y, vec4Temp0.x);\n"
"    _viv_asm(ARCTRIG1, arctrig0, arctrig0, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig1, vec4Temp1.y, vec4Temp1.x);\n"
"    _viv_asm(ARCTRIG1, arctrig1, arctrig1, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig2, vec4Temp2.y, vec4Temp2.x);\n"
"    _viv_asm(ARCTRIG1, arctrig2, arctrig2, 0x3);\n"
"    _viv_asm(ARCTRIG0, arctrig3, vec4Temp3.y, vec4Temp3.x);\n"
"    _viv_asm(ARCTRIG1, arctrig3, arctrig3, 0x3);\n"
"    result.x = _viv_fma_float(arctrig0.x, vec4Temp0.x, arctrig0.y);\n"
"    result.y = _viv_fma_float(arctrig1.x, vec4Temp1.x, arctrig1.y);\n"
"    result.z = _viv_fma_float(arctrig2.x, vec4Temp2.x, arctrig2.y);\n"
"    result.w = _viv_fma_float(arctrig3.x, vec4Temp3.x, arctrig3.y);\n"
"    return result;\n"
"}\n"
"float8 _viv_atan2_float8(float8 y, float8 x)\n"
"{\n"
"    float8 result;\n"
"    result.s0123 = _viv_atan2_float4(y.s0123, x.s0123);\n"
"    result.s4567 = _viv_atan2_float4(y.s4567, x.s4567);\n"
"    return result;\n"
"}\n"
"float16 _viv_atan2_float16(float16 y, float16 x)\n"
"{\n"
"    float16 result;\n"
"    result.s0123 = _viv_atan2_float4(y.s0123, x.s0123);\n"
"    result.s4567 = _viv_atan2_float4(y.s4567, x.s4567);\n"
"    result.s89ab = _viv_atan2_float4(y.s89ab, x.s89ab);\n"
"    result.scdef = _viv_atan2_float4(y.scdef, x.scdef);\n"
"    return result;\n"
"}\n"
;

gctSTRING gcCLLibLongMADSAT_Funcs =
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

gctSTRING gcCLLibLongNEXTAFTER_Funcs =
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
#endif

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

gctSTRING gcCLLibImageQuery_Funcs_UseImgInst =
NL "int _viv_image_query_width_image1d_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_width_image2d_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_width_image3d_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_width_image1d_array_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_width_image2d_array_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_width_image1d_buffer_t(uint8 image)"
NL "{"
NL "    return image.s2 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_height_image2d_t(uint8 image)"
NL "{"
NL "    return (image.s2 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_height_image2d_array_t(uint8 image)"
NL "{"
NL "    return (image.s2 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_height_image3d_t(uint8 image)"
NL "{"
NL "    return (image.s2 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_depth_image3d_t(uint8 image)"
NL "{"
NL "    return image.s5;"
NL "}"
NL
NL "int2 _viv_image_query_size_image2d_t(uint8 image)"
NL "{"
NL "    return (int2) (image.s2 & 0xFFFF, (image.s2 >> 16) & 0xFFFF);"
NL "}"
NL
NL "int4 _viv_image_query_size_image3d_t(uint8 image)"
NL "{"
NL "    return (int4) (image.s2 & 0xFFFF, (image.s2 >> 16) & 0xFFFF, image.s5, 0);"
NL "}"
NL
NL "int2 _viv_image_query_size_image2d_array_t(uint8 image)"
NL "{"
NL "    return (int2) (image.s2 & 0xFFFF, (image.s2 >> 16) & 0xFFFF);"
NL "}"
NL
NL "int _viv_image_query_order_image1d_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_order_image2d_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_order_image3d_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_order_image1d_array_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_order_image2d_array_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_order_image1d_buffer_t(uint8 image)"
NL "{"
NL "    return image.s6 & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_format_image1d_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_format_image2d_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_format_image3d_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_format_image1d_array_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_format_image2d_array_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_format_image1d_buffer_t(uint8 image)"
NL "{"
NL "    return (image.s6 >> 16) & 0xFFFF;"
NL "}"
NL
NL "int _viv_image_query_array_size_image1d_array_t(uint8 image)"
NL "{"
NL "    return image.s5;"
NL "}"
NL
NL "int _viv_image_query_array_size_image2d_array_t(uint8 image)"
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

gctSTRING gcCLLibImageQuery_Funcs =
NL "int _viv_image_query_width_image1d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_image_query_width_image2d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_image_query_width_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_image_query_width_image1d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_image_query_width_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_image_query_width_image1d_buffer_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return *imghdr;"
NL "}"
NL
NL "int _viv_image_query_height_image2d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[1];"
NL "}"
NL
NL "int _viv_image_query_height_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[1];"
NL "}"
NL
NL "int _viv_image_query_height_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[1];"
NL "}"
NL
NL "int _viv_image_query_depth_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[2];"
NL "}"
NL
NL "int2 _viv_image_query_size_image2d_t(uint8 image)"
NL "{"
NL "    uint2 *imghdr = (uint2 *) image.s0;"
NL "    return (int2)((*imghdr).x, (*imghdr).y);"
NL "}"
NL
NL "int4 _viv_image_query_size_image3d_t(uint8 image)"
NL "{"
NL "    uint4 *imghdr = (uint4 *) image.s0;"
NL "    return (int4)((*imghdr).x, (*imghdr).y, (*imghdr).z, 0);"
NL "}"
NL
NL "int2 _viv_image_query_size_image2d_array_t(uint8 image)"
NL "{"
NL "    uint2 *imghdr = (uint2 *) image.s0;"
NL "    return (int2)((*imghdr).x, (*imghdr).y);"
NL "}"
NL
NL "int _viv_image_query_order_image1d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_image_query_order_image2d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_image_query_order_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_image_query_order_image1d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_image_query_order_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_image_query_order_image1d_buffer_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[4];"
NL "}"
NL
NL "int _viv_image_query_format_image1d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_image_query_format_image2d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_image_query_format_image3d_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_image_query_format_image1d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_image_query_format_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_image_query_format_image1d_buffer_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[3];"
NL "}"
NL
NL "int _viv_image_query_array_size_image1d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[8];"
NL "}"
NL
NL "int _viv_image_query_array_size_image2d_array_t(uint8 image)"
NL "{"
NL "    uint *imghdr = (uint *) image.s0;"
NL "    return imghdr[8];"
NL "}"
NL;

/* opencl version to getLocalID */
gctSTRING gcCLLibGetLocalID =
NL "int _viv_getLocalID()"
NL "{"
NL "    return get_local_id(2) * get_local_size(1)* get_local_size(0) + get_local_id(1) * get_local_size(0) + get_local_id(0);"
NL "}"
NL;

/* atomicpatch lib function, Atomcmpxchg patch functions are put in another string
 * because the second argument is different between opencl and opengl versions
 */
gctSTRING gcCLLib_AtomcmpxchgPatch_Func_core1_Str =
"int _atomcmpxchg_int_uint_int2_core1(uint a, int2 val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomcmpxchg_uint_uint_uint2_core1(uint a, uint2 val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "float _atomcmpxchg_float_uint_float2_core1(uint a, float2 val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    float result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL;

gctSTRING gcCLLib_AtomcmpxchgPatch_Func_core2_Str =
"int _atomcmpxchg_int_uint_int2_core2(uint a, int2 val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomcmpxchg_uint_uint_uint2_core2(uint a, uint2 val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "float _atomcmpxchg_float_uint_float2_core2(uint a, float2 val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    float result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL;

gctSTRING gcCLLib_AtomcmpxchgPatch_Func_core4_Str =
"int _atomcmpxchg_int_uint_int2_core4(uint a, int2 val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomcmpxchg_uint_uint_uint2_core4(uint a, uint2 val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "float _atomcmpxchg_float_uint_float2_core4(uint a, float2 val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    float result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL;

gctSTRING gcCLLib_AtomcmpxchgPatch_Func_core8_Str =
"int _atomcmpxchg_int_uint_int2_core8(uint a, int2 val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomcmpxchg_uint_uint_uint2_core8(uint a, uint2 val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "float _atomcmpxchg_float_uint_float2_core8(uint a, float2 val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    float result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMCMPXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL;

gctSTRING gcLib_AtomicPatch_Common_Func_core1_Str =
"int _atomadd_int_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomadd_uint_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomadd_uint_uint_uint_core1(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomsub_int_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomsub_uint_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomsub_uint_uint_uint_core1(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomxchg_int_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomxchg_uint_uint_uint_core1(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "float _atomxchg_float_uint_float_core1(uint a, float val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    float result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atommin_int_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atommin_uint_uint_uint_core1(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atommax_int_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atommax_uint_uint_uint_core1(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomor_int_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomor_uint_uint_uint_core1(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomand_int_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomand_uint_uint_uint_core1(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomxor_int_uint_int_core1(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomxor_uint_uint_uint_core1(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x3;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL;

gctSTRING gcLib_AtomicPatch_Common_Func_core2_Str =
"int _atomadd_int_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomadd_uint_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomadd_uint_uint_uint_core2(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomsub_int_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomsub_uint_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomsub_uint_uint_uint_core2(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomxchg_int_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomxchg_uint_uint_uint_core2(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "float _atomxchg_float_uint_float_core2(uint a, float val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    float result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL"int _atommin_int_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atommin_uint_uint_uint_core2(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL"int _atommax_int_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atommax_uint_uint_uint_core2(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL"int _atomor_int_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomor_uint_uint_uint_core2(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL"int _atomand_int_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL
NL "uint _atomand_uint_uint_uint_core2(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL"int _atomxor_int_uint_int_core2(uint a, int val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomxor_uint_uint_uint_core2(uint a, uint val)"
NL "{"
NL "    int tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x7;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL;

gctSTRING gcLib_AtomicPatch_Common_Func_core4_Str =
 "int _atomadd_int_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomadd_uint_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomadd_uint_uint_uint_core4(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomsub_int_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomsub_uint_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomsub_uint_uint_uint_core4(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomxchg_int_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomxchg_uint_uint_uint_core4(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "float _atomxchg_float_uint_float_core4(uint a, float val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    float result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atommin_int_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atommin_uint_uint_uint_core4(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atommax_int_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atommax_uint_uint_uint_core4(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomor_int_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomor_uint_uint_uint_core4(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomand_int_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomand_uint_uint_uint_core4(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomxor_int_uint_int_core4(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomxor_uint_uint_uint_core4(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0xf;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL;

gctSTRING gcLib_AtomicPatch_Common_Func_core8_Str =
 "int _atomadd_int_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomadd_uint_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomadd_uint_uint_uint_core8(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMADD, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomsub_int_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomsub_uint_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomsub_uint_uint_uint_core8(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMSUB, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomxchg_int_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomxchg_uint_uint_uint_core8(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "float _atomxchg_float_uint_float_core8(uint a, float val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    float result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMXCHG, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atommin_int_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atommin_uint_uint_uint_core8(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMMIN, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atommax_int_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atommax_uint_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMMAX, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomor_int_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomor_uint_uint_uint_core8(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomand_int_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomand_uint_uint_uint_core8(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMAND, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "int _atomxor_int_uint_int_core8(uint a, int val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    int result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL "uint _atomxor_uint_uint_uint_core8(uint a, uint val)"
NL "{"
NL "    int  tid = _viv_getLocalID();"
NL "    int id_in_a_sh_group = tid & 0x1f;"
NL "    uint result;"
NL "    switch(id_in_a_sh_group)"
NL "    {"
NL "        case 0:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 1:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 2:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 3:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 4:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 5:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 6:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 7:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 8:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 9:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 10:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 11:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 12:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 13:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 14:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 15:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 16:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 17:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 18:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 19:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 20:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 21:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 22:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 23:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 24:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 25:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 26:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 27:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 28:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 29:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 30:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "        case 31:"
NL "             _viv_asm(ATOMXOR, result, a, val);"
NL "             break;"
NL "    }"
NL "    return result;"
NL "}"
NL;
#undef NL

