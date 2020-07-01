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


/*
**/

#if !REMOVE_CL_LIBS
#define read_image_ARGS_INT_COORD_1d \
"    (\n" \
"    uint image, \n" \
"    int imageSize, \n" \
"    int coord \n" \
"    ) \n" \
"{ \n"

#define read_image_ARGS_INT_COORD_2d \
"    (\n" \
"    uint2 image, \n" \
"    int2 imageSize, \n" \
"    int2 coord \n" \
"    ) \n" \
"{ \n"

#define read_image_ARGS_INT_COORD_1dbuffer read_image_ARGS_INT_COORD_1d

#define read_image_ARGS_INT_COORD_1darray read_image_ARGS_INT_COORD_2d

#define read_image_ARGS_INT_COORD_3d \
"    (\n" \
"    uint3 image, \n" \
"    int3 imageSize, \n" \
"    int3 coord \n" \
"    ) \n" \
"{ \n"

#define read_image_ARGS_FLOAT_COORD_1d \
"    (\n" \
"    uint image, \n" \
"    int imageSize, \n" \
"    float fcoord \n" \
"    ) \n" \
"{ \n"

#define read_image_ARGS_FLOAT_COORD_1dbuffer read_image_ARGS_FLOAT_COORD_1d

#define read_image_ARGS_FLOAT_COORD_2d \
"    (\n" \
"    uint2 image, \n" \
"    int2 imageSize, \n" \
"    float2 fcoord \n" \
"    ) \n" \
"{ \n"

#define read_image_ARGS_FLOAT_COORD_1darray read_image_ARGS_FLOAT_COORD_2d

#define read_image_ARGS_FLOAT_COORD_3d \
"    (\n" \
"    uint3 image, \n" \
"    int3 imageSize, \n" \
"    float3 fcoord \n" \
"    ) \n" \
"{ \n"

/* 2D array has the same coord type as 3D except that 'z' is used as array index which is not a real coord but an integer index. */
#define read_image_ARGS_INT_COORD_2DARRAY       read_image_ARGS_INT_COORD_3d
#define read_image_ARGS_FLOAT_COORD_2DARRAY     read_image_ARGS_FLOAT_COORD_3d

/* This Marco is not used. */

#define WRAP_COORD \
"    fcoord = fcoord - floor(fcoord); \n"

#define WRAP_COORD_1d WRAP_COORD
#define WRAP_COORD_2d WRAP_COORD
#define WRAP_COORD_3d WRAP_COORD
#define WRAP_COORD_1dbuffer WRAP_COORD

#define WRAP_COORD_1darray \
"    fcoord.x = fcoord.x - floor(fcoord.x); \n"

#define WRAP_COORD_2DARRAY \
"    fcoord.xy = fcoord.xy - floor(fcoord.xy); \n"

/* Since simple round can pass cts, not need to use rint. */
#define MIRROR_COORD_1d \
"    float fcoord1 = 2.0 * floor(0.5 * fcoord + 0.5); \n" \
"    fcoord = viv_fabs(fcoord - fcoord1); \n"

#define MIRROR_COORD_1dbuffer MIRROR_COORD_1d

#define MIRROR_COORD_2d \
"    float2 fcoord1 = 2.0 * floor(0.5 * fcoord + 0.5); \n" \
"    fcoord = viv_fabs(fcoord - fcoord1); \n"

#define MIRROR_COORD_1darray \
"    float fcoord1 = 2.0 * floor(0.5 * fcoord.x + 0.5); \n" \
"    fcoord.x = viv_fabs(fcoord.x - fcoord1); \n" \
"    fcoord.y = clamp(fcoord.y, 0.0, imageSize.y - 1.0); \n"

#define MIRROR_COORD_2DARRAY \
"    float2 fcoord1 = 2.0 * floor(0.5 * fcoord.xy + 0.5); \n" \
"    fcoord.xy = viv_fabs(fcoord.xy - fcoord1); \n" \
"    fcoord.z = clamp(fcoord.z, 0.0, imageSize.z - 1.0); \n"

#define MIRROR_COORD_3d \
"    float3 fcoord1 = 2.0 * floor(0.5 * fcoord + 0.5); \n" \
"    fcoord = viv_fabs(fcoord - fcoord1); \n"

#define CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_1d \
"    fcoord = fcoord * convert_float(imageSize); \n"

#define CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_1dbuffer CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_1d

#define CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_2d \
"    fcoord = fcoord * convert_float2(imageSize); \n"

#define CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_1darray \
"    fcoord = fcoord * convert_float2(imageSize); \n" \
"    fcoord.y = clamp(fcoord.y, 0.0, imageSize.y - 1.0); \n"

/* Note that for 2D array, imageSize.z is the array size, the real size is stored in xy. */
#define CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_2DARRAY \
"    fcoord = fcoord * convert_float3(imageSize); \n" \
"    fcoord.z = clamp(fcoord.z, 0.0, imageSize.z - 1.0); \n"

#define CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_3d \
"    fcoord = fcoord * convert_float3(imageSize); \n"

#define CONVERT_UNNORMALIZED_COORD_TO_NORMALIZED_COORD_1d \
"    fcoord = fcoord / convert_float(imageSize); \n"

#define CONVERT_UNNORMALIZED_COORD_TO_NORMALIZED_COORD_1dbuffer CONVERT_UNNORMALIZED_COORD_TO_NORMALIZED_COORD_1d

#define CONVERT_UNNORMALIZED_COORD_TO_NORMALIZED_COORD_2d \
"    fcoord = fcoord / convert_float2(imageSize); \n"

#define CONVERT_UNNORMALIZED_COORD_TO_NORMALIZED_COORD_1darray \
"    fcoord = fcoord / convert_float2(imageSize); \n" \
"    fcoord.y = clamp(fcoord.y, 0.0, imageSize.y - 1.0); \n"

#define CONVERT_UNNORMALIZED_COORD_TO_NORMALIZED_COORD_2DARRAY \
"    fcoord = fcoord / convert_float3(imageSize); \n" \
"    fcoord.z = clamp(fcoord.z, 0.0, imageSize.z - 1.0); \n"

#define CONVERT_UNNORMALIZED_COORD_TO_NORMALIZED_COORD_3d \
"    fcoord = fcoord / convert_float3(imageSize); \n"

#define CONVERT_FLOAT_COORD_TO_INT_COORD_1d \
"    int coord = convert_int_rtn(fcoord); \n"

#define CONVERT_FLOAT_COORD_TO_INT_COORD_1dbuffer CONVERT_FLOAT_COORD_TO_INT_COORD_1d

#define CONVERT_FLOAT_COORD_TO_INT_COORD_2d \
"    int2 coord = convert_int2_rtn(fcoord); \n"

#define CONVERT_FLOAT_COORD_TO_INT_COORD_1darray \
"    int2 coord; \n" \
"    coord.x = convert_int_rtn(fcoord.x); \n" \
"    coord.y = rint(fcoord.y); \n"

#define CONVERT_FLOAT_COORD_TO_INT_COORD_2DARRAY \
"    int3 coord; \n" \
"    coord.xy = convert_int2_rtn(fcoord.xy); \n" \
"    coord.z = rint(fcoord.z); \n"

#define CONVERT_FLOAT_COORD_TO_INT_COORD_3d \
"    int3 coord = convert_int3_rtn(fcoord); \n"

#define CLAMP_COORD_UPPER \
"    coord = viv_min(coord, imageSize - 1); \n"
#define CLAMP_COORD_UPPER_1d CLAMP_COORD_UPPER
#define CLAMP_COORD_UPPER_1dbuffer CLAMP_COORD_UPPER
#define CLAMP_COORD_UPPER_2d CLAMP_COORD_UPPER
#define CLAMP_COORD_UPPER_3d CLAMP_COORD_UPPER

#define CLAMP_COORD_UPPER_1darray \
"    coord.x = viv_min(coord.x, imageSize.x - 1); \n"

#define CLAMP_COORD_UPPER_2DARRAY \
"    coord.xy = viv_min(coord.xy, imageSize.xy - 1); \n" \

#define CLAMP_NONE_1d
#define CLAMP_NONE_1dbuffer
#define CLAMP_NONE_2d
#define CLAMP_NONE_1darray \
"    coord.y = clamp(coord.y, 0, imageSize.y - 1); \n"
#define CLAMP_NONE_2DARRAY \
"    coord.z = clamp(coord.z, 0, imageSize.z - 1); \n"
#define CLAMP_NONE_3d

#define CLAMP_COORD_1d \
"    coord = clamp(coord, 0, imageSize - 1); \n"

#define CLAMP_COORD_1dbuffer CLAMP_COORD_1d

#define CLAMP_COORD_2d \
"    coord = clamp(coord, (int2)(0), imageSize - 1); \n"

#define CLAMP_COORD_1darray CLAMP_COORD_2d

#define CLAMP_COORD_2DARRAY CLAMP_COORD_3d

#define CLAMP_COORD_3d \
"    coord = clamp(coord, (int3)(0), imageSize - 1); \n"

#define CHECK_BORDER_1d(_TYPE_)\
"    if (((uint)coord >= (uint)imageSize) || (coord < 0)) \n" \
"    { \n" \
"        return (" #_TYPE_ ")0; \n" \
"    } \n"

#define CHECK_BORDER_1dbuffer CHECK_BORDER_1d
#define CHECK_BORDER_1d_BGRA CHECK_BORDER_1d
#define CHECK_BORDER_1dbuffer_BGRA CHECK_BORDER_1d
#define CHECK_BORDER_1dbuffer_R(_TYPE_)\
"    if (((uint)coord >= (uint)imageSize) || (coord < 0)) \n" \
"    { \n" \
"        return (" #_TYPE_ ")0; \n" \
"    } \n"

#define CHECK_BORDER_1d_R(_TYPE_) \
"    if (((uint)coord >= (uint)imageSize) || (coord < 0)) \n" \
"    { \n" \
"        return (" #_TYPE_ ")(0, 0, 0, 1); \n" \
"    } \n"

#define CHECK_BORDER_2d(_TYPE_) \
"    if (((uint)coord.x >= (uint)imageSize.x) ||\n" \
"        (coord.x < 0) ||\n" \
"        ((uint)coord.y >= (uint)imageSize.y) ||\n" \
"        (coord.y < 0))\n" \
"    { \n" \
"        return (" #_TYPE_ ")0; \n" \
"    } \n"

#define CHECK_BORDER_2d_BGRA CHECK_BORDER_2d

#define CHECK_BORDER_2d_R(_TYPE_) \
"    if (((uint)coord.x >= (uint)imageSize.x) ||\n" \
"        (coord.x < 0) ||\n" \
"        ((uint)coord.y >= (uint)imageSize.y) ||\n" \
"        (coord.y < 0))\n" \
"    { \n" \
"        return (" #_TYPE_ ")(0,0,0,1); \n" \
"    } \n"


#define CHECK_BORDER_1darray(_TYPE_) \
"    if (((uint)coord.x >= (uint)imageSize.x) || (coord.x < 0)) \n" \
"    { \n" \
"        return (" #_TYPE_ ")0; \n" \
"    } \n" \
"    coord.y = clamp(coord.y, 0, imageSize.y - 1); \n"

#define CHECK_BORDER_1darray_BGRA CHECK_BORDER_1darray
#define CHECK_BORDER_1darray_R(_TYPE_) \
"    if (((uint)coord.x >= (uint)imageSize.x) || (coord.x < 0)) \n" \
"    { \n" \
"        return (" #_TYPE_ ")(0,0,0,1); \n" \
"    } \n" \
"    coord.y = clamp(coord.y, 0, imageSize.y - 1); \n"

#define CHECK_BORDER_2DARRAY(_TYPE_) \
"    if (((uint)coord.x >= (uint)imageSize.x) ||\n" \
"        (coord.x < 0) ||\n" \
"        ((uint)coord.y >= (uint)imageSize.y) ||\n" \
"        (coord.y < 0))\n" \
"    { \n" \
"        return (" #_TYPE_ ")0; \n" \
"    } \n" \
"    coord.z = clamp(coord.z, 0, imageSize.z - 1); \n"

#define CHECK_BORDER_2DARRAY_BGRA CHECK_BORDER_2DARRAY

#define CHECK_BORDER_2DARRAY_R(_TYPE_) \
"    if (((uint)coord.x >= (uint)imageSize.x) ||\n" \
"        (coord.x < 0) ||\n" \
"        ((uint)coord.y >= (uint)imageSize.y) ||\n" \
"        (coord.y < 0))\n" \
"    { \n" \
"        return (" #_TYPE_ ")(0, 0, 0, 1); \n" \
"    } \n" \
"    coord.z = clamp(coord.z, 0, imageSize.z - 1); \n"


#define CHECK_BORDER_3d(_TYPE_) \
"    if (((uint)coord.x >= (uint)imageSize.x) ||\n" \
"        (coord.x < 0) ||\n" \
"        ((uint)coord.y >= (uint)imageSize.y) ||\n" \
"        (coord.y < 0) ||\n" \
"        ((uint)coord.z >= (uint)imageSize.z) ||\n" \
"        (coord.z < 0))\n" \
"    { \n" \
"        return (" #_TYPE_ ")0; \n" \
"    } \n"
#define CHECK_BORDER_3d_BGRA  CHECK_BORDER_3d
#define CHECK_BORDER_3d_R(_TYPE_) \
"    if (((uint)coord.x >= (uint)imageSize.x) ||\n" \
"        (coord.x < 0) ||\n" \
"        ((uint)coord.y >= (uint)imageSize.y) ||\n" \
"        (coord.y < 0) ||\n" \
"        ((uint)coord.z >= (uint)imageSize.z) ||\n" \
"        (coord.z < 0))\n" \
"    { \n" \
"        return (" #_TYPE_ ")(0, 0, 0, 1); \n" \
"    } \n"

#define GET_I0J0I1J1_SIMPLE_1d \
"    fcoord = fcoord - 0.5; \n" \
"    float fractUV = fcoord; \n" \
"    fcoord = floor(fcoord); \n" \
"    fractUV -= fcoord; \n" \
"    int2 coordQ; \n" \
"    coordQ.x = convert_int(fcoord); \n" \
"    coordQ.y = coordQ.x + 1; \n"

#define GET_I0J0I1J1_SIMPLE_1dbuffer GET_I0J0I1J1_SIMPLE_1d

#define GET_I0J0I1J1_SIMPLE_2d \
"    fcoord = fcoord - 0.5; \n" \
"    float2 fractUV = fcoord; \n" \
"    fcoord = floor(fcoord); \n" \
"    fractUV -= fcoord; \n" \
"    int4 coordQ; \n" \
"    coordQ.xy = convert_int2(fcoord); \n" \
"    coordQ.zw = coordQ.xy + 1; \n"

#define GET_I0J0I1J1_SIMPLE_1darray \
"    fcoord.x = fcoord.x - 0.5; \n" \
"    float fractUV = fcoord.x; \n" \
"    fcoord.x = floor(fcoord.x); \n" \
"    fractUV -= fcoord.x; \n" \
"    int2 coordQ; \n" \
"    coordQ.x = convert_int(fcoord.x); \n" \
"    coordQ.y = coordQ.x + 1; \n"

#define GET_I0J0I1J1_SIMPLE_2DARRAY \
"    fcoord.xy = fcoord.xy - 0.5; \n" \
"    float2 fractUV = fcoord.xy; \n" \
"    fcoord.xy = floor(fcoord.xy); \n" \
"    fractUV -= fcoord.xy; \n" \
"    int4 coordQ; \n" \
"    coordQ.xy = convert_int2(fcoord.xy); \n" \
"    coordQ.zw = coordQ.xy + 1; \n"

#define GET_I0J0I1J1_SIMPLE_3d \
"    fcoord = fcoord - 0.5; \n" \
"    float3 fractUVW = fcoord; \n" \
"    fcoord = floor(fcoord); \n" \
"    fractUVW -= fcoord; \n" \
"    int3 coordQ, coordR; \n" \
"    coordQ = convert_int3(fcoord); \n" \
"    coordR = coordQ + 1; \n"

#define CLAMP_I0J0I1J1_1d \
"    int imageSizeM1 = imageSize - 1; \n" \
"    coordQ.x = clamp(coordQ.x, (int)(0), imageSizeM1); \n" \
"    coordQ.y = clamp(coordQ.y, (int)(0), imageSizeM1); \n"

#define CLAMP_I0J0I1J1_1dbuffer CLAMP_I0J0I1J1_1d

#define CLAMP_I0J0I1J1_2d \
"    int2 imageSizeM1 = imageSize - 1; \n" \
"    coordQ.xy = clamp(coordQ.xy, (int2)(0), imageSizeM1); \n" \
"    coordQ.zw = clamp(coordQ.zw, (int2)(0), imageSizeM1); \n"

#define CLAMP_I0J0I1J1_1darray \
"    int imageSizeM1 = imageSize.x - 1; \n" \
"    coordQ.x = clamp(coordQ.x, (int)(0), imageSizeM1); \n" \
"    coordQ.y = clamp(coordQ.y, (int)(0), imageSizeM1); \n"

#define CLAMP_I0J0I1J1_2DARRAY \
"    int2 imageSizeM1 = imageSize.xy - 1; \n" \
"    coordQ.xy = clamp(coordQ.xy, (int2)(0), imageSizeM1); \n" \
"    coordQ.zw = clamp(coordQ.zw, (int2)(0), imageSizeM1); \n"

#define CLAMP_I0J0I1J1_3d \
"    int3 imageSizeM1 = imageSize - 1; \n" \
"    coordQ = clamp(coordQ, (int3)(0), imageSizeM1); \n" \
"    coordR = clamp(coordR, (int3)(0), imageSizeM1); \n"

#define GET_I0J0I1J1_WRAP_1d \
"    fcoord = fcoord - floor(fcoord); \n" \
"    fcoord = fcoord * convert_float(imageSize) - 0.5; \n" \
"    float fractUV = fcoord; \n" \
"    fcoord = floor(fcoord); \n" \
"    fractUV -= fcoord; \n" \
"    int2 coordQ; \n" \
"    coordQ.x = convert_int(fcoord); \n" \
"    coordQ.y = coordQ.x + 1; \n" \
"    if (coordQ.x < 0) coordQ.x += imageSize; \n" \
"    if (coordQ.y >= imageSize) coordQ.y -= imageSize; \n" \

#define GET_I0J0I1J1_WRAP_1dbuffer GET_I0J0I1J1_WRAP_1d

#define GET_I0J0I1J1_WRAP_2d \
"    fcoord = fcoord - floor(fcoord); \n" \
"    fcoord = fcoord * convert_float2(imageSize) - 0.5; \n" \
"    float2 fractUV = fcoord; \n" \
"    fcoord = floor(fcoord); \n" \
"    fractUV -= fcoord; \n" \
"    int4 coordQ; \n" \
"    coordQ.xy = convert_int2(fcoord); \n" \
"    coordQ.zw = coordQ.xy + 1; \n" \
"    if (coordQ.x < 0) coordQ.x += imageSize.x; \n" \
"    if (coordQ.y < 0) coordQ.y += imageSize.y; \n" \
"    if (coordQ.z >= imageSize.x) coordQ.z -= imageSize.x; \n" \
"    if (coordQ.w >= imageSize.y) coordQ.w -= imageSize.y; \n"

#define GET_I0J0I1J1_WRAP_1darray \
"    fcoord.x = fcoord.x - floor(fcoord.x); \n" \
"    fcoord.x = fcoord.x * convert_float(imageSize.x) - 0.5; \n" \
"    fcoord.y = fcoord.y * convert_float(imageSize.y); \n" \
"    float fractUV = fcoord.x; \n" \
"    fcoord.x = floor(fcoord.x); \n" \
"    fractUV -= fcoord.x; \n" \
"    int2 coordQ; \n" \
"    coordQ.x = convert_int(fcoord.x); \n" \
"    coordQ.y = coordQ.x + 1; \n" \
"    if (coordQ.x < 0) coordQ.x += imageSize.x; \n" \
"    if (coordQ.y >= imageSize.x) coordQ.y -= imageSize.x; \n" \

#define GET_I0J0I1J1_WRAP_2DARRAY \
"    fcoord.xy = fcoord.xy - floor(fcoord.xy); \n" \
"    fcoord.xy = fcoord.xy * convert_float2(imageSize.xy) - 0.5; \n" \
"    fcoord.z = fcoord.z * convert_float(imageSize.z); \n" \
"    float2 fractUV = fcoord.xy; \n" \
"    fcoord.xy = floor(fcoord.xy); \n" \
"    fractUV -= fcoord.xy; \n" \
"    int4 coordQ; \n" \
"    coordQ.xy = convert_int2(fcoord.xy); \n" \
"    coordQ.zw = coordQ.xy + 1; \n" \
"    if (coordQ.x < 0) coordQ.x += imageSize.x; \n" \
"    if (coordQ.y < 0) coordQ.y += imageSize.y; \n" \
"    if (coordQ.z >= imageSize.x) coordQ.z -= imageSize.x; \n" \
"    if (coordQ.w >= imageSize.y) coordQ.w -= imageSize.y; \n"

#define GET_I0J0I1J1_WRAP_3d \
"    fcoord = fcoord - floor(fcoord); \n" \
"    fcoord = fcoord * convert_float3(imageSize) - 0.5; \n" \
"    float3 fractUVW = fcoord; \n" \
"    fcoord = floor(fcoord); \n" \
"    fractUVW -= fcoord; \n" \
"    int3 coordQ, coordR; \n" \
"    coordQ = convert_int3(fcoord); \n" \
"    coordR = coordQ + 1; \n" \
"    if (coordQ.x < 0) coordQ.x += imageSize.x; \n" \
"    if (coordQ.y < 0) coordQ.y += imageSize.y; \n" \
"    if (coordQ.z < 0) coordQ.z += imageSize.z; \n" \
"    if (coordR.x >= imageSize.x) coordR.x -= imageSize.x; \n" \
"    if (coordR.y >= imageSize.y) coordR.y -= imageSize.y; \n" \
"    if (coordR.z >= imageSize.z) coordR.z -= imageSize.z; \n"

#define GET_I0J0I1J1_MIRROR_1d \
"    float fcoord1 = 2.0 * floor(0.5 * fcoord + 0.5); \n" \
"    fcoord = viv_fabs(fcoord - fcoord1); \n" \
"    fcoord = fcoord * convert_float(imageSize) - 0.5; \n" \
"    float fractUV = fcoord; \n" \
"    fcoord = floor(fcoord); \n" \
"    fractUV -= fcoord; \n" \
"    int2 coordQ; \n" \
"    coordQ.x = convert_int(fcoord); \n" \
"    coordQ.y = coordQ.x + 1; \n" \
"    int imageSizeM1 = imageSize - 1; \n" \
"    coordQ.x = clamp(coordQ.x, (int)(0), imageSizeM1); \n" \
"    coordQ.y = clamp(coordQ.y, (int)(0), imageSizeM1); \n"

#define GET_I0J0I1J1_MIRROR_1dbuffer GET_I0J0I1J1_MIRROR_1d

#define GET_I0J0I1J1_MIRROR_2d \
"    float2 fcoord1 = 2.0 * floor(0.5 * fcoord + 0.5); \n" \
"    fcoord = viv_fabs(fcoord - fcoord1); \n" \
"    fcoord = fcoord * convert_float2(imageSize) - 0.5; \n" \
"    float2 fractUV = fcoord; \n" \
"    fcoord = floor(fcoord); \n" \
"    fractUV -= fcoord; \n" \
"    int4 coordQ; \n" \
"    coordQ.xy = convert_int2(fcoord); \n" \
"    coordQ.zw = coordQ.xy + 1; \n" \
"    int2 imageSizeM1 = imageSize - 1; \n" \
"    coordQ.xy = clamp(coordQ.xy, (int2)(0), imageSizeM1); \n" \
"    coordQ.zw = clamp(coordQ.zw, (int2)(0), imageSizeM1); \n"

#define GET_I0J0I1J1_MIRROR_1darray \
"    float fcoord1 = 2.0 * floor(0.5 * fcoord.x + 0.5); \n" \
"    fcoord.x = viv_fabs(fcoord.x - fcoord1); \n" \
"    fcoord.x = fcoord.x * convert_float(imageSize.x) - 0.5; \n" \
"    fcoord.y = fcoord.y * convert_float(imageSize.y); \n" \
"    float fractUV = fcoord.x; \n" \
"    fcoord.x = floor(fcoord.x); \n" \
"    fractUV -= fcoord.x; \n" \
"    int2 coordQ; \n" \
"    coordQ.x = convert_int(fcoord.x); \n" \
"    coordQ.y = coordQ.x + 1; \n" \
"    int imageSizeM1 = imageSize.x - 1; \n" \
"    coordQ.x = clamp(coordQ.x, (int)(0), imageSizeM1); \n" \
"    coordQ.y = clamp(coordQ.y, (int)(0), imageSizeM1); \n"

#define GET_I0J0I1J1_MIRROR_2DARRAY \
"    float2 fcoord1 = 2.0 * floor(0.5 * fcoord.xy + 0.5); \n" \
"    fcoord.xy = viv_fabs(fcoord.xy - fcoord1); \n" \
"    fcoord.xy = fcoord.xy * convert_float2(imageSize.xy) - 0.5; \n" \
"    fcoord.z = fcoord.z * convert_float(imageSize.z); \n" \
"    float2 fractUV = fcoord.xy; \n" \
"    fcoord.xy = floor(fcoord.xy); \n" \
"    fractUV -= fcoord.xy; \n" \
"    int4 coordQ; \n" \
"    coordQ.xy = convert_int2(fcoord.xy); \n" \
"    coordQ.zw = coordQ.xy + 1; \n" \
"    int2 imageSizeM1 = imageSize.xy - 1; \n" \
"    coordQ.xy = clamp(coordQ.xy, (int2)(0), imageSizeM1); \n" \
"    coordQ.zw = clamp(coordQ.zw, (int2)(0), imageSizeM1); \n"

#define GET_I0J0I1J1_MIRROR_3d \
"    float3 fcoord1 = 2.0 * floor(0.5 * fcoord + 0.5); \n" \
"    fcoord = viv_fabs(fcoord - fcoord1); \n" \
"    fcoord = fcoord * convert_float3(imageSize) - 0.5; \n" \
"    float3 fractUVW = fcoord; \n" \
"    fcoord = floor(fcoord); \n" \
"    fractUVW -= fcoord; \n" \
"    int3 coordQ, coordR; \n" \
"    coordQ = convert_int3(fcoord); \n" \
"    coordR = coordQ + 1; \n" \
"    int3 imageSizeM1 = imageSize - 1; \n" \
"    coordQ = clamp(coordQ, (int3)(0), imageSizeM1); \n" \
"    coordR = clamp(coordR, (int3)(0), imageSizeM1); \n"

#define LOAD_TEXEL_1d(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image); \n" \
"    return " #_CONVERT_ "(base[coord]); \n" \
"} \n" \
"\n"
#define LOAD_TEXEL_1d_BGRA(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image); \n" \
"    " #_TYPE_ " pixel = base[coord];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return " #_CONVERT_ "(value); \n" \
"} \n" \
"\n"

#define GET_BASE_1d(_CONVERT_TYPE_) \
"    " #_CONVERT_TYPE_ " * base = (" #_CONVERT_TYPE_ " *) ((uchar *)image); \n"
#define CONSTRUCT_1d_1CHANNEL_LOAD(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord], 0, 0, 1);\n" \
"    return " #_CONVERT_ "(pixel); \n" \
"} \n" \
"\n"

#define LOAD_TEXEL_1d_R_uchar4(_TYPE_, _CONVERT_) \
    GET_BASE_1d(uchar) \
    CONSTRUCT_1d_1CHANNEL_LOAD(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1d_R_char4(_TYPE_, _CONVERT_) \
    GET_BASE_1d(char) \
    CONSTRUCT_1d_1CHANNEL_LOAD(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1d_R_ushort4(_TYPE_, _CONVERT_) \
    GET_BASE_1d(ushort) \
    CONSTRUCT_1d_1CHANNEL_LOAD(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1d_R_short4(_TYPE_, _CONVERT_) \
    GET_BASE_1d(short) \
    CONSTRUCT_1d_1CHANNEL_LOAD(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1d_R_uint4(_TYPE_, _CONVERT_) \
    GET_BASE_1d(uint) \
    CONSTRUCT_1d_1CHANNEL_LOAD(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1d_R_int4(_TYPE_, _CONVERT_) \
    GET_BASE_1d(int) \
    CONSTRUCT_1d_1CHANNEL_LOAD(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1d_R_float4(_TYPE_, _CONVERT_) \
    GET_BASE_1d(float) \
    CONSTRUCT_1d_1CHANNEL_LOAD(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1d_R_half(_TYPE_, _CONVERT_) \
    GET_BASE_1d(half) \
    CONSTRUCT_1d_1CHANNEL_LOAD(_TYPE_, _CONVERT_)

#define LOAD_TEXEL_1d_R(_TYPE_, _CONVERT_) \
    LOAD_TEXEL_1d_R_##_TYPE_(_TYPE_, _CONVERT_)

#define LOAD_TEXEL_1dbuffer(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image); \n" \
"    return " #_CONVERT_ "(base[coord]); \n" \
"} \n" \
"\n"
#define LOAD_TEXEL_1dbuffer_BGRA(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image); \n" \
"    " #_TYPE_ " pixel = base[coord];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return " #_CONVERT_ "(value); \n" \
"} \n" \
"\n"
#define LOAD_TEXEL_1dbuffer_R(_TYPE_, _CONVERT_) \
    LOAD_TEXEL_1d_R_##_TYPE_(_TYPE_, _CONVERT_)

#define LOAD_TEXEL_2d(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    return " #_CONVERT_ "(base[coord.x]); \n" \
"} \n" \
"\n"
#define LOAD_TEXEL_2d_BGRA(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    " #_TYPE_ " pixel = base[coord.x];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return " #_CONVERT_ "(value); \n" \
"} \n" \
"\n"

#define GET_BASE_2d(_CONVERT_TYPE_) \
"    " #_CONVERT_TYPE_ " * base = (" #_CONVERT_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n"
#define CONSTRUCT_2d_1CHANNEL(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, 1);\n" \
"    return " #_CONVERT_ "(pixel); \n" \
"} \n" \
"\n"

#define LOAD_TEXEL_2d_R_uchar4(_TYPE_, _CONVERT_) \
    GET_BASE_2d(uchar) \
    CONSTRUCT_2d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2d_R_char4(_TYPE_, _CONVERT_) \
    GET_BASE_2d(char) \
    CONSTRUCT_2d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2d_R_ushort4(_TYPE_, _CONVERT_) \
    GET_BASE_2d(ushort) \
    CONSTRUCT_2d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2d_R_short4(_TYPE_, _CONVERT_) \
    GET_BASE_2d(short) \
    CONSTRUCT_2d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2d_R_uint4(_TYPE_, _CONVERT_) \
    GET_BASE_2d(uint) \
    CONSTRUCT_2d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2d_R_int4(_TYPE_, _CONVERT_) \
    GET_BASE_2d(int) \
    CONSTRUCT_2d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2d_R_float4(_TYPE_, _CONVERT_) \
    GET_BASE_2d(float) \
    CONSTRUCT_2d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2d_R_half(_TYPE_, _CONVERT_) \
    GET_BASE_2d(half) \
    CONSTRUCT_2d_1CHANNEL(_TYPE_, _CONVERT_)


#define LOAD_TEXEL_2d_R(_TYPE_, _CONVERT_) \
    LOAD_TEXEL_2d_R_##_TYPE_(_TYPE_, _CONVERT_) \

#define LOAD_TEXEL_1darray(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    return " #_CONVERT_ "(base[coord.x]); \n" \
"} \n" \
"\n"
#define LOAD_TEXEL_1darray_BGRA(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    " #_TYPE_ " pixel = base[coord.x];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return " #_CONVERT_ "(value); \n" \
"} \n" \
"\n"

#define GET_BASE_1darray(_CONVERT_TYPE_) \
"    " #_CONVERT_TYPE_ " * base = (" #_CONVERT_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n"

#define CONSTRUCT_1darray_1CHANNEL(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, 1);\n" \
"    return " #_CONVERT_ "(pixel); \n" \
"} \n" \
"\n"

#define LOAD_TEXEL_1darray_R_uchar4(_TYPE_, _CONVERT_) \
    GET_BASE_1darray(uchar) \
    CONSTRUCT_1darray_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1darray_R_char4(_TYPE_, _CONVERT_) \
    GET_BASE_1darray(char) \
    CONSTRUCT_1darray_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1darray_R_ushort4(_TYPE_, _CONVERT_) \
    GET_BASE_1darray(ushort) \
    CONSTRUCT_1darray_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1darray_R_short4(_TYPE_, _CONVERT_) \
    GET_BASE_1darray(short) \
    CONSTRUCT_1darray_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1darray_R_uint4(_TYPE_, _CONVERT_) \
    GET_BASE_1darray(uint) \
    CONSTRUCT_1darray_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1darray_R_int4(_TYPE_, _CONVERT_) \
    GET_BASE_1darray(int) \
    CONSTRUCT_1darray_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1darray_R_float4(_TYPE_, _CONVERT_) \
    GET_BASE_1darray(float) \
    CONSTRUCT_1darray_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_1darray_R_half(_TYPE_, _CONVERT_) \
    GET_BASE_1darray(half) \
    CONSTRUCT_1darray_1CHANNEL(_TYPE_, _CONVERT_)

#define LOAD_TEXEL_1darray_R(_TYPE_, _CONVERT_) \
    LOAD_TEXEL_1darray_R_##_TYPE_(_TYPE_, _CONVERT_)



#define LOAD_TEXEL_3d(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    return " #_CONVERT_ "(base[coord.x]); \n" \
"} \n" \
"\n"
#define LOAD_TEXEL_3d_BGRA(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    " #_TYPE_ " pixel = base[coord.x];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return " #_CONVERT_ "(value); \n" \
"} \n" \
"\n"

#define LOAD_TEXEL_2DARRAY  LOAD_TEXEL_3d
#define LOAD_TEXEL_2DARRAY_BGRA  LOAD_TEXEL_3d_BGRA

#define GET_BASE_2DARRAY(_CONVERT_TYPE_) \
"     " #_CONVERT_TYPE_ " * base = (" #_CONVERT_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \


#define CONSTRUCT_2DARRAY_1CHANNEL(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, 1);\n" \
"    return " #_CONVERT_ "(pixel); \n" \
"} \n" \
"\n"

#define LOAD_TEXEL_2DARRAY_R_uchar4(_TYPE_, _CONVERT_) \
    GET_BASE_2DARRAY(uchar) \
    CONSTRUCT_2DARRAY_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2DARRAY_R_char4(_TYPE_, _CONVERT_) \
    GET_BASE_2DARRAY(char) \
    CONSTRUCT_2DARRAY_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2DARRAY_R_ushort4(_TYPE_, _CONVERT_) \
    GET_BASE_2DARRAY(ushort) \
    CONSTRUCT_2DARRAY_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2DARRAY_R_short4(_TYPE_, _CONVERT_) \
    GET_BASE_2DARRAY(short) \
    CONSTRUCT_2DARRAY_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2DARRAY_R_uint4(_TYPE_, _CONVERT_) \
    GET_BASE_2DARRAY(uint) \
    CONSTRUCT_2DARRAY_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2DARRAY_R_int4(_TYPE_, _CONVERT_) \
    GET_BASE_2DARRAY(int) \
    CONSTRUCT_2DARRAY_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2DARRAY_R_float4(_TYPE_, _CONVERT_) \
    GET_BASE_2DARRAY(float) \
    CONSTRUCT_2DARRAY_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_2DARRAY_R_half(_TYPE_, _CONVERT_) \
    GET_BASE_2DARRAY(half) \
    CONSTRUCT_2DARRAY_1CHANNEL(_TYPE_, _CONVERT_)

#define LOAD_TEXEL_2DARRAY_R(_TYPE_, _CONVERT_) \
    LOAD_TEXEL_2DARRAY_R_##_TYPE_(_TYPE_, _CONVERT_)

#define GET_BASE_3d(_CONVERT_TYPE_) \
"     " #_CONVERT_TYPE_ " * base = (" #_CONVERT_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \


#define CONSTRUCT_3d_1CHANNEL(_TYPE_, _CONVERT_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, 1);\n" \
"    return " #_CONVERT_ "(pixel); \n" \
"} \n" \
"\n"

#define LOAD_TEXEL_3d_R_uchar4(_TYPE_, _CONVERT_) \
    GET_BASE_3d(uchar) \
    CONSTRUCT_3d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_3d_R_char4(_TYPE_, _CONVERT_) \
    GET_BASE_3d(char) \
    CONSTRUCT_3d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_3d_R_ushort4(_TYPE_, _CONVERT_) \
    GET_BASE_3d(ushort) \
    CONSTRUCT_3d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_3d_R_short4(_TYPE_, _CONVERT_) \
    GET_BASE_3d(short) \
    CONSTRUCT_3d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_3d_R_uint4(_TYPE_, _CONVERT_) \
    GET_BASE_3d(uint) \
    CONSTRUCT_3d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_3d_R_int4(_TYPE_, _CONVERT_) \
    GET_BASE_3d(int) \
    CONSTRUCT_3d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_3d_R_float4(_TYPE_, _CONVERT_) \
    GET_BASE_3d(float) \
    CONSTRUCT_3d_1CHANNEL(_TYPE_, _CONVERT_)
#define LOAD_TEXEL_3d_R_half(_TYPE_, _CONVERT_) \
    GET_BASE_3d(half) \
    CONSTRUCT_3d_1CHANNEL(_TYPE_, _CONVERT_)

#define LOAD_TEXEL_3d_R(_TYPE_, _CONVERT_) \
    LOAD_TEXEL_3d_R_##_TYPE_(_TYPE_, _CONVERT_)



/* Use LOAD_TEXEL(uchar4, convert_uint4) for UINT8 */
/* Use LOAD_TEXEL(ushort4, convert_uint4) for UINT16 */
/* Use LOAD_TEXEL(uint4, ) for UINT32 */
/* Use LOAD_TEXEL(char4, convert_int4)  for INT8 */
/* Use LOAD_TEXEL(short4, convert_int4)  for INT16 */
/* Use LOAD_TEXEL(int4, )  for INT32 */

#define LOAD_UNORM_TEXEL_1d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image ); \n" \
    "    return convert_float4(base[coord]) / " #_BASE_ "; \n" \
"} \n" \
"\n"
#define LOAD_UNORM_TEXEL_1d_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    " #_TYPE_ " pixel = base[coord];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return convert_float4(value) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define CONSTRUCT_UNORM_1d_1CHANNEL(_TYPE_, _BASE_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord], 0, 0, " #_BASE_ ");\n" \
"    return convert_float4(pixel) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define LOAD_UNORM_TEXEL_1d_R_uchar4(_TYPE_, _BASE_) \
    GET_BASE_1d(uchar) \
    CONSTRUCT_UNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1d_R_char4(_TYPE_, _BASE_) \
    GET_BASE_1d(char) \
    CONSTRUCT_UNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1d_R_ushort4(_TYPE_, _BASE_) \
    GET_BASE_1d(ushort) \
    CONSTRUCT_UNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1d_R_short4(_TYPE_, _BASE_) \
    GET_BASE_1d(short) \
    CONSTRUCT_UNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1d_R_uint4(_TYPE_, _BASE_) \
    GET_BASE_1d(uint) \
    CONSTRUCT_UNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1d_R_int4(_TYPE_, _BASE_) \
    GET_BASE_1d(int) \
    CONSTRUCT_UNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1d_R_float4(_TYPE_, _BASE_) \
    GET_BASE_1d(float) \
    CONSTRUCT_UNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1d_R_half(_TYPE_, _BASE_) \
    GET_BASE_1d(half) \
    CONSTRUCT_UNORM_1d_1CHANNEL(_TYPE_, _BASE_)

#define LOAD_UNORM_TEXEL_1d_R(_TYPE_, _BASE_) \
    LOAD_UNORM_TEXEL_1d_R_##_TYPE_(_TYPE_, _BASE_)

#define LOAD_UNORM_TEXEL_1dbuffer(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    return convert_float4(base[coord]) / " #_BASE_ "; \n" \
"} \n" \
"\n"
#define LOAD_UNORM_TEXEL_1dbuffer_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    " #_TYPE_ " pixel = base[coord];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return convert_float4(value) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define LOAD_UNORM_TEXEL_1dbuffer_R(_TYPE_, _BASE_) \
    LOAD_UNORM_TEXEL_1d_R_##_TYPE_(_TYPE_, _BASE_)

#define LOAD_UNORM_TEXEL_1darray(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    return convert_float4(base[coord.x]) / " #_BASE_ "; \n" \
"} \n" \
"\n"
#define LOAD_UNORM_TEXEL_1darray_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    " #_TYPE_ " pixel = base[coord.x];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return convert_float4(value) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define LOAD_UNORM_TEXEL_2d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    return convert_float4(base[coord.x]) / " #_BASE_ "; \n" \
"} \n" \
"\n"
#define LOAD_UNORM_TEXEL_2d_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    " #_TYPE_ " pixel = base[coord.x];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return convert_float4(value) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define CONSTRUCT_UNORM_2d_1CHANNEL(_TYPE_, _BASE_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, " #_BASE_ ");\n" \
"     return convert_float4(pixel) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define LOAD_UNORM_TEXEL_2d_R_uchar4(_TYPE_, _BASE_) \
    GET_BASE_2d(uchar) \
    CONSTRUCT_UNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2d_R_char4(_TYPE_, _BASE_) \
    GET_BASE_2d(char) \
    CONSTRUCT_UNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2d_R_ushort4(_TYPE_, _BASE_) \
    GET_BASE_2d(ushort) \
    CONSTRUCT_UNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2d_R_short4(_TYPE_, _BASE_) \
    GET_BASE_2d(short) \
    CONSTRUCT_UNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2d_R_uint4(_TYPE_, _BASE_) \
    GET_BASE_2d(uint) \
    CONSTRUCT_UNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2d_R_int4(_TYPE_, _BASE_) \
    GET_BASE_2d(int) \
    CONSTRUCT_UNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2d_R_float4(_TYPE_, _BASE_) \
    GET_BASE_2d(float) \
    CONSTRUCT_UNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2d_R_half(_TYPE_, _BASE_) \
    GET_BASE_2d(half) \
    CONSTRUCT_UNORM_2d_1CHANNEL(_TYPE_, _BASE_)

#define LOAD_UNORM_TEXEL_2d_R(_TYPE_, _BASE_) \
    LOAD_UNORM_TEXEL_2d_R_##_TYPE_(_TYPE_, _BASE_) \

#define CONSTRUCT_UNORM_1darray_1CHANNEL(_TYPE_, _BASE_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, " #_BASE_ ");\n" \
"     return convert_float4(pixel) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define LOAD_UNORM_TEXEL_1darray_R_uchar4(_TYPE_, _BASE_) \
    GET_BASE_2d(uchar) \
    CONSTRUCT_UNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1darray_R_char4(_TYPE_, _BASE_) \
    GET_BASE_2d(char) \
    CONSTRUCT_UNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1darray_R_ushort4(_TYPE_, _BASE_) \
    GET_BASE_2d(ushort) \
    CONSTRUCT_UNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1darray_R_short4(_TYPE_, _BASE_) \
    GET_BASE_2d(short) \
    CONSTRUCT_UNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1darray_R_uint4(_TYPE_, _BASE_) \
    GET_BASE_2d(uint) \
    CONSTRUCT_UNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1darray_R_int4(_TYPE_, _BASE_) \
    GET_BASE_2d(int) \
    CONSTRUCT_UNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1darray_R_float4(_TYPE_, _BASE_) \
    GET_BASE_2d(float) \
    CONSTRUCT_UNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_1darray_R_half(_TYPE_, _BASE_) \
    GET_BASE_2d(half) \
    CONSTRUCT_UNORM_1darray_1CHANNEL(_TYPE_, _BASE_)

#define LOAD_UNORM_TEXEL_1darray_R(_TYPE_, _BASE_) \
    LOAD_UNORM_TEXEL_1darray_R_##_TYPE_(_TYPE_, _BASE_)



#define LOAD_UNORM_TEXEL_3d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    return convert_float4(base[coord.x]) / " #_BASE_ "; \n" \
"} \n" \
"\n"
#define LOAD_UNORM_TEXEL_3d_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    " #_TYPE_ " pixel = base[coord.x];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return convert_float4(value) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define LOAD_UNORM_TEXEL_2DARRAY    LOAD_UNORM_TEXEL_3d
#define LOAD_UNORM_TEXEL_2DARRAY_BGRA    LOAD_UNORM_TEXEL_3d_BGRA

#define CONSTRUCT_UNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, " #_BASE_ ");\n" \
"     return convert_float4(pixel) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define LOAD_UNORM_TEXEL_2DARRAY_R_uchar4(_TYPE_, _BASE_) \
    GET_BASE_2DARRAY(uchar) \
    CONSTRUCT_UNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2DARRAY_R_char4(_TYPE_, _BASE_) \
    GET_BASE_2DARRAY(char) \
    CONSTRUCT_UNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2DARRAY_R_ushort4(_TYPE_, _BASE_) \
    GET_BASE_2DARRAY(ushort) \
    CONSTRUCT_UNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2DARRAY_R_short4(_TYPE_, _BASE_) \
    GET_BASE_2DARRAY(short) \
    CONSTRUCT_UNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2DARRAY_R_uint4(_TYPE_, _BASE_) \
    GET_BASE_2DARRAY(uint) \
    CONSTRUCT_UNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2DARRAY_R_int4(_TYPE_, _BASE_) \
    GET_BASE_2DARRAY(int) \
    CONSTRUCT_UNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2DARRAY_R_float4(_TYPE_, _BASE_) \
    GET_BASE_2DARRAY(float) \
    CONSTRUCT_UNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_2DARRAY_R_half(_TYPE_, _BASE_) \
    GET_BASE_2DARRAY(half) \
    CONSTRUCT_UNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)

#define LOAD_UNORM_TEXEL_2DARRAY_R(_TYPE_, _BASE_) \
    LOAD_UNORM_TEXEL_2DARRAY_R_##_TYPE_(_TYPE_, _BASE_)

#define CONSTRUCT_UNORM_3d_1CHANNEL(_TYPE_, _BASE_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, " #_BASE_ ");\n" \
"     return convert_float4(pixel) / " #_BASE_ "; \n" \
"} \n" \
"\n"

#define LOAD_UNORM_TEXEL_3d_R_uchar4(_TYPE_, _BASE_) \
    GET_BASE_3d(uchar) \
    CONSTRUCT_UNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_3d_R_char4(_TYPE_, _BASE_) \
    GET_BASE_3d(char) \
    CONSTRUCT_UNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_3d_R_ushort4(_TYPE_, _BASE_) \
    GET_BASE_3d(ushort) \
    CONSTRUCT_UNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_3d_R_short4(_TYPE_, _BASE_) \
    GET_BASE_3d(short) \
    CONSTRUCT_UNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_3d_R_uint4(_TYPE_, _BASE_) \
    GET_BASE_3d(uint) \
    CONSTRUCT_UNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_3d_R_int4(_TYPE_, _BASE_) \
    GET_BASE_3d(int) \
    CONSTRUCT_UNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_3d_R_float4(_TYPE_, _BASE_) \
    GET_BASE_3d(float) \
    CONSTRUCT_UNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_UNORM_TEXEL_3d_R_half(_TYPE_, _BASE_) \
    GET_BASE_3d(half) \
    CONSTRUCT_UNORM_3d_1CHANNEL(_TYPE_, _BASE_)

#define LOAD_UNORM_TEXEL_3d_R(_TYPE_, _BASE_) \
    LOAD_UNORM_TEXEL_3d_R_##_TYPE_(_TYPE_, _BASE_)


/* Use LOAD_SNORM_TEXEL(char4, 127.0) for SNORM8 */
/* Use LOAD_SNORM_TEXEL(short4, 32767.0) for SNORM16 */

#define LOAD_SNORM_TEXEL_1d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image ); \n" \
    "    return max((convert_float4(base[coord]) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"
#define LOAD_SNORM_TEXEL_1d_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    " #_TYPE_ " pixel = base[coord];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return max((convert_float4(value) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"

#define CONSTRUCT_SNORM_1d_1CHANNEL(_TYPE_, _BASE_) \
"    " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord], 0, 0, " #_BASE_ ");\n" \
"    return max((convert_float4(pixel) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"

#define LOAD_SNORM_TEXEL_1d_R_uchar4(_TYPE_, _BASE_) \
    GET_BASE_1d(uchar) \
    CONSTRUCT_SNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1d_R_char4(_TYPE_, _BASE_) \
    GET_BASE_1d(char) \
    CONSTRUCT_SNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1d_R_ushort4(_TYPE_, _BASE_) \
    GET_BASE_1d(ushort) \
    CONSTRUCT_SNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1d_R_short4(_TYPE_, _BASE_) \
    GET_BASE_1d(short) \
    CONSTRUCT_SNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1d_R_uint4(_TYPE_, _BASE_) \
    GET_BASE_1d(uint) \
    CONSTRUCT_SNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1d_R_int4(_TYPE_, _BASE_) \
    GET_BASE_1d(int) \
    CONSTRUCT_SNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1d_R_float4(_TYPE_, _BASE_) \
    GET_BASE_1d(float) \
    CONSTRUCT_SNORM_1d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1d_R_half(_TYPE_, _BASE_) \
    GET_BASE_1d(half) \
    CONSTRUCT_SNORM_1d_1CHANNEL(_TYPE_, _BASE_)

#define LOAD_SNORM_TEXEL_1d_R(_TYPE_, _BASE_) \
    LOAD_SNORM_TEXEL_1d_R_##_TYPE_(_TYPE_, _BASE_)


#define LOAD_SNORM_TEXEL_1dbuffer(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    return max((convert_float4(base[coord]) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"
#define LOAD_SNORM_TEXEL_1dbuffer_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    " #_TYPE_ " pixel = base[coord];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return max((convert_float4(value) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"
#define LOAD_SNORM_TEXEL_1dbuffer_R(_TYPE_, _BASE_) \
    LOAD_SNORM_TEXEL_1d_R_##_TYPE_(_TYPE_, _BASE_)

#define LOAD_SNORM_TEXEL_1darray(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    return max((convert_float4(base[coord.x]) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"
#define LOAD_SNORM_TEXEL_1darray_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    " #_TYPE_ " pixel = base[coord.x];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return max((convert_float4(value) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"

#define LOAD_SNORM_TEXEL_2d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    return max((convert_float4(base[coord.x]) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"
#define LOAD_SNORM_TEXEL_2d_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    " #_TYPE_ " pixel = base[coord.x];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return max((convert_float4(value) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"
#define CONSTRUCT_SNORM_2d_1CHANNEL(_TYPE_, _BASE_) \
"     " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, " #_BASE_ ");\n" \
"     return max((convert_float4(pixel) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"


#define LOAD_SNORM_TEXEL_2d_R_uchar4(_TYPE_, _BASE_) \
        GET_BASE_2d(uchar) \
        CONSTRUCT_SNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2d_R_char4(_TYPE_, _BASE_) \
        GET_BASE_2d(char) \
        CONSTRUCT_SNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2d_R_ushort4(_TYPE_, _BASE_) \
        GET_BASE_2d(ushort) \
        CONSTRUCT_SNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2d_R_short4(_TYPE_, _BASE_) \
        GET_BASE_2d(short) \
        CONSTRUCT_SNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2d_R_uint4(_TYPE_, _BASE_) \
        GET_BASE_2d(uint) \
        CONSTRUCT_SNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2d_R_int4(_TYPE_, _BASE_) \
        GET_BASE_2d(int) \
        CONSTRUCT_SNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2d_R_float4(_TYPE_, _BASE_) \
        GET_BASE_2d(float) \
        CONSTRUCT_SNORM_2d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2d_R_half(_TYPE_, _BASE_) \
        GET_BASE_2d(half) \
        CONSTRUCT_SNORM_2d_1CHANNEL(_TYPE_, _BASE_)


#define LOAD_SNORM_TEXEL_2d_R(_TYPE_, _BASE_) \
        LOAD_SNORM_TEXEL_2d_R_##_TYPE_(_TYPE_, _BASE_) \

#define CONSTRUCT_SNORM_1darray_1CHANNEL(_TYPE_, _BASE_) \
"     " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, " #_BASE_ ");\n" \
"     return max((convert_float4(pixel) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"


#define LOAD_SNORM_TEXEL_1darray_R_uchar4(_TYPE_, _BASE_) \
        GET_BASE_2d(uchar) \
        CONSTRUCT_SNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1darray_R_char4(_TYPE_, _BASE_) \
        GET_BASE_2d(char) \
        CONSTRUCT_SNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1darray_R_ushort4(_TYPE_, _BASE_) \
        GET_BASE_2d(ushort) \
        CONSTRUCT_SNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1darray_R_short4(_TYPE_, _BASE_) \
        GET_BASE_2d(short) \
        CONSTRUCT_SNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1darray_R_uint4(_TYPE_, _BASE_) \
        GET_BASE_2d(uint) \
        CONSTRUCT_SNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1darray_R_int4(_TYPE_, _BASE_) \
        GET_BASE_2d(int) \
        CONSTRUCT_SNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1darray_R_float4(_TYPE_, _BASE_) \
        GET_BASE_2d(float) \
        CONSTRUCT_SNORM_1darray_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_1darray_R_half(_TYPE_, _BASE_) \
        GET_BASE_2d(half) \
        CONSTRUCT_SNORM_1darray_1CHANNEL(_TYPE_, _BASE_)


#define LOAD_SNORM_TEXEL_1darray_R(_TYPE_, _BASE_) \
        LOAD_SNORM_TEXEL_1darray_R_##_TYPE_(_TYPE_, _BASE_) \



#define LOAD_SNORM_TEXEL_3d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
    "    return max((convert_float4(base[coord.x]) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"
#define LOAD_SNORM_TEXEL_3d_BGRA(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    " #_TYPE_ " pixel = base[coord.x];\n" \
"    " #_TYPE_ " value = pixel;\n" \
"    value.x = pixel.z;\n" \
"    value.z = pixel.x;\n" \
"    return max((convert_float4(value) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"

#define LOAD_SNORM_TEXEL_2DARRAY    LOAD_SNORM_TEXEL_3d
#define LOAD_SNORM_TEXEL_2DARRAY_BGRA    LOAD_SNORM_TEXEL_3d_BGRA

#define CONSTRUCT_SNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_) \
"     " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, " #_BASE_ ");\n" \
"     return max((convert_float4(pixel) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"


#define LOAD_SNORM_TEXEL_2DARRAY_R_uchar4(_TYPE_, _BASE_) \
        GET_BASE_2DARRAY(uchar) \
        CONSTRUCT_SNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2DARRAY_R_char4(_TYPE_, _BASE_) \
        GET_BASE_2DARRAY(char) \
        CONSTRUCT_SNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2DARRAY_R_ushort4(_TYPE_, _BASE_) \
        GET_BASE_2DARRAY(ushort) \
        CONSTRUCT_SNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2DARRAY_R_short4(_TYPE_, _BASE_) \
        GET_BASE_2DARRAY(short) \
        CONSTRUCT_SNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2DARRAY_R_uint4(_TYPE_, _BASE_) \
        GET_BASE_2DARRAY(uint) \
        CONSTRUCT_SNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2DARRAY_R_int4(_TYPE_, _BASE_) \
        GET_BASE_2DARRAY(int) \
        CONSTRUCT_SNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2DARRAY_R_float4(_TYPE_, _BASE_) \
        GET_BASE_2DARRAY(float) \
        CONSTRUCT_SNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_2DARRAY_R_half(_TYPE_, _BASE_) \
        GET_BASE_2DARRAY(half) \
        CONSTRUCT_SNORM_2DARRAY_1CHANNEL(_TYPE_, _BASE_)


#define LOAD_SNORM_TEXEL_2DARRAY_R(_TYPE_, _BASE_) \
        LOAD_SNORM_TEXEL_2DARRAY_R_##_TYPE_(_TYPE_, _BASE_) \

#define CONSTRUCT_SNORM_3d_1CHANNEL(_TYPE_, _BASE_) \
"     " #_TYPE_ " pixel = (" #_TYPE_ ")(base[coord.x], 0, 0, " #_BASE_ ");\n" \
"     return max((convert_float4(pixel) / " #_BASE_ "), -1.0); \n" \
"} \n" \
"\n"


#define LOAD_SNORM_TEXEL_3d_R_uchar4(_TYPE_, _BASE_) \
        GET_BASE_3d(uchar) \
        CONSTRUCT_SNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_3d_R_char4(_TYPE_, _BASE_) \
        GET_BASE_3d(char) \
        CONSTRUCT_SNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_3d_R_ushort4(_TYPE_, _BASE_) \
        GET_BASE_3d(ushort) \
        CONSTRUCT_SNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_3d_R_short4(_TYPE_, _BASE_) \
        GET_BASE_3d(short) \
        CONSTRUCT_SNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_3d_R_uint4(_TYPE_, _BASE_) \
        GET_BASE_3d(uint) \
        CONSTRUCT_SNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_3d_R_int4(_TYPE_, _BASE_) \
        GET_BASE_3d(int) \
        CONSTRUCT_SNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_3d_R_float4(_TYPE_, _BASE_) \
        GET_BASE_3d(float) \
        CONSTRUCT_SNORM_3d_1CHANNEL(_TYPE_, _BASE_)
#define LOAD_SNORM_TEXEL_3d_R_half(_TYPE_, _BASE_) \
        GET_BASE_3d(half) \
        CONSTRUCT_SNORM_3d_1CHANNEL(_TYPE_, _BASE_)


#define LOAD_SNORM_TEXEL_3d_R(_TYPE_, _BASE_) \
        LOAD_SNORM_TEXEL_3d_R_##_TYPE_(_TYPE_, _BASE_) \



/* Use LOAD_SNORM_TEXEL(uchar4, 127.0) for SNORM8 */
/* Use LOAD_SNORM_TEXEL(ushort4, 32767.0) for SNORM16 */

/* Need a special way to load half. */
/* Need a special way to load half. */
#define LOAD_Half_TEXEL_1d(_TYPE_, _BASE_) LOAD_HALF4_1d
#define LOAD_HALF4_1d \
"    half * base = (half *) ((uchar *)image ); \n" \
"    return vload_half4((uint)coord, base); \n" \
"} \n" \
"\n"
#define LOAD_Half_TEXEL_1d_BGRA(_TYPE_, _BASE_) LOAD_HALF4_1d_BGRA
#define LOAD_HALF4_1d_BGRA \
"    half * base = (half *) ((uchar *)image ); \n"      \
"    float4 pixel = vload_half4((uint)coord, base); \n"  \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value; \n" \
"} \n" \
"\n"
#define LOAD_Half_TEXEL_1d_R(_TYPE_, _BASE_) LOAD_HALF4_1d_R
#define LOAD_HALF4_1d_R \
"    half * base = (half *) ((uchar *)image ); \n" \
"    return (float4)(vload_half((uint)coord, base), 0.0f, 0.0f, 1.0f); \n" \
"} \n" \
"\n"

#define LOAD_Half_TEXEL_1dbuffer(_TYPE_, _BASE_) LOAD_HALF4_1d
#define LOAD_Half_TEXEL_1dbuffer_BGRA(_TYPE_, _BASE_) LOAD_HALF4_1d_BGRA
#define LOAD_Half_TEXEL_1dbuffer_R(_TYPE_, _BASE_) LOAD_HALF4_1d_R

#define LOAD_Half_TEXEL_2d(_TYPE_, _BASE_) LOAD_HALF4_2d
#define LOAD_HALF4_2d \
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    return vload_half4((uint)coord.x, base); \n" \
"} \n" \
"\n"
#define LOAD_Half_TEXEL_2d_BGRA(_TYPE_, _BASE_) LOAD_HALF4_2d_BGRA
#define LOAD_HALF4_2d_BGRA \
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    float4 pixel = vload_half4((uint)coord.x, base); \n"  \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value; \n" \
"} \n" \
"\n"
#define LOAD_Half_TEXEL_2d_R(_TYPE_, _BASE_) LOAD_HALF4_2d_R
#define LOAD_HALF4_2d_R \
"     half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"     return (float4)(vload_half((uint)coord.x, base), 0.0f, 0.0f, 1.0f); \n" \
"} \n" \
"\n"
#define LOAD_Half_TEXEL_1darray_R(_TYPE_, _BASE_) LOAD_HALF4_1darry_R
#define LOAD_HALF4_1darry_R \
"     half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"     return (float4)(vload_half((uint)coord.x, base), 0.0f, 0.0f, 1.0f); \n" \
"} \n" \
"\n"
#define LOAD_Half_TEXEL_2DARRAY_R(_TYPE_, _BASE_) LOAD_HALF4_3d_R

#define LOAD_Half_TEXEL_3d_R(_TYPE_, _BASE_) LOAD_HALF4_3d_R
#define LOAD_HALF4_3d_R \
"     half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"     return (float4)(vload_half((uint)coord.x, base), 0.0f, 0.0f, 1.0f); \n" \
"} \n" \
"\n"

#define LOAD_Half_TEXEL_1darray(_TYPE_, _BASE_) LOAD_HALF4_2d
#define LOAD_Half_TEXEL_1darray_BGRA(_TYPE_, _BASE_) LOAD_HALF4_2d_BGRA

#define LOAD_Half_TEXEL_3d(_TYPE_, _BASE_) LOAD_HALF4_3d
#define LOAD_HALF4_3d \
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    return vload_half4((uint)coord.x, base); \n" \
"} \n" \
"\n"
#define LOAD_Half_TEXEL_3d_BGRA(_TYPE_, _BASE_) LOAD_HALF4_3d_BGRA
#define LOAD_HALF4_3d_BGRA \
"    half * base = (half *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    float4 pixel = vload_half4((uint)coord.x, base); \n"  \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value; \n" \
"} \n" \
"\n"

#define LOAD_Float_TEXEL_1d(_TYPE_, _BASE_) LOAD_FLOAT4_1d
#define LOAD_FLOAT4_1d \
"    float4 * base = (float4 *) ((uchar *)image ); \n" \
"    return base[coord]; \n" \
"} \n" \
"\n"
#define LOAD_Float_TEXEL_1d_BGRA(_TYPE_, _BASE_) LOAD_FLOAT4_1d_BGRA
#define LOAD_FLOAT4_1d_BGRA \
"    float4 * base = (float4 *) ((uchar *)image ); \n" \
"    float4 pixel = base[coord]; \n"  \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value; \n" \
"} \n" \
"\n"
#define LOAD_Float_TEXEL_1d_R(_TYPE_, _BASE_) LOAD_FLOAT4_1d_R
#define LOAD_FLOAT4_1d_R \
"    float * base = (float *) ((uchar *)image ); \n" \
"    return (float4)(base[coord],0.0f, 0.0f, 1.0f); \n" \
"} \n" \
"\n"

#define LOAD_Float_TEXEL_1dbuffer(_TYPE_, _BASE_) LOAD_FLOAT4_1d
#define LOAD_Float_TEXEL_1dbuffer_BGRA(_TYPE_, _BASE_) LOAD_FLOAT4_1d_BGRA
#define LOAD_Float_TEXEL_1dbuffer_R(_TYPE_, _BASE_) LOAD_FLOAT4_1d_R

#define LOAD_Half_TEXEL_2DARRAY  LOAD_Half_TEXEL_3d
#define LOAD_Half_TEXEL_2DARRAY_BGRA  LOAD_Half_TEXEL_3d_BGRA

#define LOAD_Float_TEXEL_2d(_TYPE_, _BASE_) LOAD_FLOAT4_2d
#define LOAD_FLOAT4_2d \
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    return base[coord.x]; \n" \
"} \n" \
"\n"
#define LOAD_Float_TEXEL_2d_BGRA(_TYPE_, _BASE_) LOAD_FLOAT4_2d_BGRA
#define LOAD_FLOAT4_2d_BGRA \
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    float4 pixel = base[coord.x]; \n"  \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value; \n" \
"} \n" \
"\n"
#define LOAD_Float_TEXEL_2d_R(_TYPE_, _BASE_) LOAD_FLOAT4_2d_R
#define LOAD_FLOAT4_2d_R \
"     float * base = (float *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"     return (float4)(base[coord.x],0.0f, 0.0f, 1.0f); \n" \
"} \n" \
"\n"

#define LOAD_Float_TEXEL_1darray(_TYPE_, _BASE_) LOAD_FLOAT4_2d
#define LOAD_Float_TEXEL_1darray_BGRA(_TYPE_, _BASE_) LOAD_FLOAT4_2d_BGRA
#define LOAD_Float_TEXEL_1darray_R(_TYPE_, _BASE_) LOAD_FLOAT4_1darray_R
#define LOAD_FLOAT4_1darray_R \
"     float * base = (float *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"     return (float4)(base[coord.x],0.0f, 0.0f, 1.0f); \n" \
"} \n" \
"\n"


#define LOAD_Float_TEXEL_3d(_TYPE_, _BASE_) LOAD_FLOAT4_3d
#define LOAD_FLOAT4_3d \
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    return base[coord.x]; \n" \
"} \n" \
"\n"
#define LOAD_Float_TEXEL_3d_BGRA(_TYPE_, _BASE_) LOAD_FLOAT4_3d_BGRA
#define LOAD_FLOAT4_3d_BGRA \
"    float4 * base = (float4 *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    float4 pixel = base[coord.x]; \n"  \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value; \n" \
"} \n" \
"\n"

#define LOAD_Float_TEXEL_2DARRAY_R(_TYPE_, _BASE_) LOAD_FLOAT4_2DARRAY_R
#define LOAD_FLOAT4_2DARRAY_R \
"     float * base = (float *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"     return (float4)(base[coord.x],0.0f, 0.0f, 1.0f); \n" \
"} \n" \
"\n"


#define LOAD_Float_TEXEL_2DARRAY    LOAD_Float_TEXEL_3d
#define LOAD_Float_TEXEL_2DARRAY_BGRA    LOAD_Float_TEXEL_3d_BGRA
#define LOAD_Float_TEXEL_3d_R LOAD_Float_TEXEL_2DARRAY_R

#define TEXLD \
"    return viv_texld(txSampler, fcoord); \n" \
"} \n" \
"\n"

#define LOAD_4_UNORM_TEXELS_1d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image); \n" \
"    float4 T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T11 = convert_float4(base0[coordQ.y]) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_1d_CONV(_CONV_TYPE_,_TYPE_, _BASE_) \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image); \n" \
"    float4 T00 = convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T11 = convert_float4((" #_TYPE_ ")(base0[coordQ.y], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \

#define LOAD_4_UNORM_TEXELS_1d_BGRA    LOAD_4_UNORM_TEXELS_1d

#define LOAD_4_UNORM_TEXELS_1d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1d_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1d_R(_TYPE_, _BASE_) \
       LOAD_4_UNORM_TEXELS_1d_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_UNORM_TEXELS_1dbuffer(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image); \n" \
"    float4 T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T11 = convert_float4(base0[coordQ.y]) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_1dbuffer_BGRA LOAD_4_UNORM_TEXELS_1dbuffer

#define LOAD_4_UNORM_TEXELS_1dbuffer_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1dbuffer_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1dbuffer_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1dbuffer_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1dbuffer_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1dbuffer_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1dbuffer_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1dbuffer_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1d_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1dbuffer_R(_TYPE_, _BASE_) \
       LOAD_4_UNORM_TEXELS_1dbuffer_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_UNORM_TEXELS_2d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"    float4 T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T10 = convert_float4(base0[coordQ.z]) / " #_BASE_ "; \n" \
"    " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"    float4 T01 = convert_float4(base1[coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T11 = convert_float4(base1[coordQ.z]) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_2d_BGRA    LOAD_4_UNORM_TEXELS_2d

#define LOAD_4_UNORM_TEXELS_2d_CONV(_CONV_TYPE_,_TYPE_, _BASE_) \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"    float4 T00 = convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T10 = convert_float4((" #_TYPE_ ")(base0[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"    float4 T01 = convert_float4((" #_TYPE_ ")(base1[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T11 = convert_float4((" #_TYPE_ ")(base1[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_2d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2d_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2d_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2d_R(_TYPE_, _BASE_) \
       LOAD_4_UNORM_TEXELS_2d_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_UNORM_TEXELS_1darray(_TYPE_, _BASE_) \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    float4 T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T11 = convert_float4(base0[coordQ.y]) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_1darray_BGRA LOAD_4_UNORM_TEXELS_1darray

#define LOAD_4_UNORM_TEXELS_1darray_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    float4 T00 = convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T11 = convert_float4((" #_TYPE_ ")(base0[coordQ.y], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_1darray_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1darray_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1darray_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1darray_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1darray_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1darray_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1darray_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1darray_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1darray_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1darray_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1darray_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1darray_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1darray_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1darray_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1darray_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_1darray_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_1darray_R(_TYPE_, _BASE_) \
       LOAD_4_UNORM_TEXELS_1darray_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_UNORM_TEXELS_2DARRAY(_TYPE_, _BASE_) \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"    float4 T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T10 = convert_float4(base0[coordQ.z]) / " #_BASE_ "; \n" \
"    " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"    float4 T01 = convert_float4(base1[coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T11 = convert_float4(base1[coordQ.z]) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_2DARRAY_BGRA LOAD_4_UNORM_TEXELS_2DARRAY
#define LOAD_4_UNORM_TEXELS_2DARRAY_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"    float4 T00 = convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T10 = convert_float4((" #_TYPE_ ")(base0[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"    float4 T01 = convert_float4((" #_TYPE_ ")(base1[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T11 = convert_float4((" #_TYPE_ ")(base1[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_2DARRAY_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2DARRAY_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2DARRAY_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2DARRAY_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2DARRAY_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2DARRAY_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2DARRAY_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2DARRAY_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2DARRAY_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2DARRAY_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2DARRAY_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2DARRAY_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2DARRAY_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2DARRAY_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2DARRAY_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_2DARRAY_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_2DARRAY_R(_TYPE_, _BASE_) \
       LOAD_4_UNORM_TEXELS_2DARRAY_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_UNORM_TEXELS_3d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"    float4 T000 = convert_float4(base0[(uint)coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T100 = convert_float4(base0[(uint)coordR.x]) / " #_BASE_ "; \n" \
"    " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"    float4 T010 = convert_float4(base1[(uint)coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T110 = convert_float4(base1[(uint)coordR.x]) / " #_BASE_ "; \n" \
"    " #_TYPE_ " * base2 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"    float4 T001 = convert_float4(base2[(uint)coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T101 = convert_float4(base2[(uint)coordR.x]) / " #_BASE_ "; \n" \
"    " #_TYPE_ " * base3 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"    float4 T011 = convert_float4(base3[(uint)coordQ.x]) / " #_BASE_ "; \n" \
"    float4 T111 = convert_float4(base3[(uint)coordR.x]) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_3d_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"    float4 T000 = convert_float4((" #_TYPE_ ")(base0[(uint)coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T100 = convert_float4((" #_TYPE_ ")(base0[(uint)coordR.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"    float4 T010 = convert_float4((" #_TYPE_ ")(base1[(uint)coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T110 = convert_float4((" #_TYPE_ ")(base1[(uint)coordR.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    " #_CONV_TYPE_ " * base2 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"    float4 T001 = convert_float4((" #_TYPE_ ")(base2[(uint)coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T101 = convert_float4((" #_TYPE_ ")(base2[(uint)coordR.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    " #_CONV_TYPE_ " * base3 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"    float4 T011 = convert_float4((" #_TYPE_ ")(base3[(uint)coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    float4 T111 = convert_float4((" #_TYPE_ ")(base3[(uint)coordR.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n"

#define LOAD_4_UNORM_TEXELS_3d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_3d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_3d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_3d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_3d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_3d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_3d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_3d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_3d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_3d_CONV(uint, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_3d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_3d_CONV(int, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_3d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_3d_CONV(float, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_3d_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_3d_CONV(half, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_3d_R(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_3d_R_##_TYPE_(_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_3d_BGRA LOAD_4_UNORM_TEXELS_3d

#define LOAD_4_SNORM_TEXELS_1d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image); \n" \
"    float4 T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T11 = max(convert_float4(base0[coordQ.y]) / " #_BASE_ ", -1.0); \n" \

#define LOAD_4_SNORM_TEXELS_1d_BGRA LOAD_4_SNORM_TEXELS_1d

#define LOAD_4_SNORM_TEXELS_1d_CONV(_CONV_TYPE_,_TYPE_, _BASE_) \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image); \n" \
"    float4 T00 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0f); \n" \
"    float4 T11 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.y], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0f); \n"

#define LOAD_4_SNORM_TEXELS_1d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1d_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1d_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1d_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_1d_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_SNORM_TEXELS_1dbuffer(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image); \n" \
"    float4 T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T11 = max(convert_float4(base0[coordQ.y]) / " #_BASE_ ", -1.0); \n" \

#define LOAD_4_SNORM_TEXELS_1dbuffer_BGRA LOAD_4_SNORM_TEXELS_1dbuffer
#define LOAD_4_SNORM_TEXELS_1dbuffer_R LOAD_4_SNORM_TEXELS_1dbuffer

#define LOAD_4_SNORM_TEXELS_2d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"    float4 T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T10 = max(convert_float4(base0[coordQ.z]) / " #_BASE_ ", -1.0); \n" \
"    " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"    float4 T01 = max(convert_float4(base1[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T11 = max(convert_float4(base1[coordQ.z]) / " #_BASE_ ", -1.0); \n"

#define LOAD_4_SNORM_TEXELS_2d_CONV(_CONV_TYPE_,_TYPE_, _BASE_) \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"    float4 T00 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0f); \n" \
"    float4 T10 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0f); \n" \
"    " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"    float4 T01 = max(convert_float4((" #_TYPE_ ")(base1[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0f); \n" \
"    float4 T11 = max(convert_float4((" #_TYPE_ ")(base1[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0f); \n"

#define LOAD_4_SNORM_TEXELS_2d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2d_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2d_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2d_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_2d_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_SNORM_TEXELS_2d_BGRA LOAD_4_SNORM_TEXELS_2d

#define LOAD_4_SNORM_TEXELS_1darray(_TYPE_, _BASE_) \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    float4 T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T11 = max(convert_float4(base0[coordQ.y]) / " #_BASE_ ", -1.0); \n" \

#define LOAD_4_SNORM_TEXELS_1darray_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    float4 T00 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    float4 T11 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.y], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \

#define LOAD_4_SNORM_TEXELS_1darray_BGRA LOAD_4_SNORM_TEXELS_1darray

#define LOAD_4_SNORM_TEXELS_1darray_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1darray_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1darray_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1darray_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1darray_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1darray_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1darray_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1darray_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1darray_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1darray_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1darray_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1darray_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1darray_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1darray_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1darray_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_1darray_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_1darray_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_1darray_R_##_TYPE_(_TYPE_,_BASE_)


#define LOAD_4_SNORM_TEXELS_2DARRAY(_TYPE_, _BASE_) \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"    float4 T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T10 = max(convert_float4(base0[coordQ.z]) / " #_BASE_ ", -1.0); \n" \
"    " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"    float4 T01 = max(convert_float4(base1[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T11 = max(convert_float4(base1[coordQ.z]) / " #_BASE_ ", -1.0); \n"

#define LOAD_4_SNORM_TEXELS_2DARRAY_BGRA LOAD_4_SNORM_TEXELS_2DARRAY
#define LOAD_4_SNORM_TEXELS_2DARRAY_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"    float4 T00 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    float4 T10 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"    float4 T01 = max(convert_float4((" #_TYPE_ ")(base1[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    float4 T11 = max(convert_float4((" #_TYPE_ ")(base1[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n"

#define LOAD_4_SNORM_TEXELS_2DARRAY_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2DARRAY_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2DARRAY_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2DARRAY_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2DARRAY_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2DARRAY_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2DARRAY_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2DARRAY_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2DARRAY_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2DARRAY_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2DARRAY_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2DARRAY_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2DARRAY_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2DARRAY_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2DARRAY_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_2DARRAY_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_2DARRAY_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_2DARRAY_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_SNORM_TEXELS_3d(_TYPE_, _BASE_) \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"    float4 T000 = max(convert_float4(base0[(uint)coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T100 = max(convert_float4(base0[(uint)coordR.x]) / " #_BASE_ ", -1.0); \n" \
"    " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"    float4 T010 = max(convert_float4(base1[(uint)coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T110 = max(convert_float4(base1[(uint)coordR.x]) / " #_BASE_ ", -1.0); \n" \
"    " #_TYPE_ " * base2 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"    float4 T001 = max(convert_float4(base2[(uint)coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T101 = max(convert_float4(base2[(uint)coordR.x]) / " #_BASE_ ", -1.0); \n" \
"    " #_TYPE_ " * base3 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"    float4 T011 = max(convert_float4(base3[(uint)coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    float4 T111 = max(convert_float4(base3[(uint)coordR.x]) / " #_BASE_ ", -1.0); \n"

#define LOAD_4_SNORM_TEXELS_3d_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"    float4 T000 = max(convert_float4((" #_TYPE_ " )(base0[(uint)coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    float4 T100 = max(convert_float4((" #_TYPE_ " )(base0[(uint)coordR.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"    float4 T010 = max(convert_float4((" #_TYPE_ " )(base1[(uint)coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    float4 T110 = max(convert_float4((" #_TYPE_ " )(base1[(uint)coordR.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    " #_CONV_TYPE_ " * base2 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"    float4 T001 = max(convert_float4((" #_TYPE_ " )(base2[(uint)coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    float4 T101 = max(convert_float4((" #_TYPE_ " )(base2[(uint)coordR.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    " #_CONV_TYPE_ " * base3 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"    float4 T011 = max(convert_float4((" #_TYPE_ " )(base3[(uint)coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    float4 T111 = max(convert_float4((" #_TYPE_ " )(base3[(uint)coordR.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n"

#define LOAD_4_SNORM_TEXELS_3d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_3d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_3d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_3d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_3d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_3d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_3d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_3d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_3d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_3d_CONV(uint, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_3d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_3d_CONV(int, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_3d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_3d_CONV(float, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_3d_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_3d_CONV(half, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_3d_R(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_3d_R_##_TYPE_(_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_3d_BGRA LOAD_4_SNORM_TEXELS_3d

/* Use LOAD_4_UNORM_TEXELS(uchar4, 255.0) for UNORM8 */
/* Use LOAD_4_UNORM_TEXELS(ushort4, 65535.0) for UNORM16 */
#define LOAD_4_Half_TEXELS_1d(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_1d
#define LOAD_4_HALF_TEXELS_1d \
"    half * base0 = (half *) ((uchar *)image); \n" \
"    float4 T00 = vload_half4((uint)coordQ.x, base0); \n" \
"    float4 T11 = vload_half4((uint)coordQ.y, base0); \n" \

#define LOAD_4_Half_TEXELS_1d_BGRA LOAD_4_Half_TEXELS_1d
#define LOAD_4_Half_TEXELS_1d_R(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_1d_R
#define LOAD_4_HALF_TEXELS_1d_R \
"    half * base0 = (half *) ((uchar *)image); \n" \
"    float4 T00 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"    float4 T11 = (float4)(vload_half((uint)coordQ.y, base0), .0f, .0f, 1.0f); \n" \

#define LOAD_4_Half_TEXELS_1dbuffer(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_1d

#define LOAD_4_Half_TEXELS_1dbuffer_BGRA LOAD_4_Half_TEXELS_1dbuffer
#define LOAD_4_Half_TEXELS_1dbuffer_R LOAD_4_Half_TEXELS_1dbuffer

#define LOAD_4_Half_TEXELS_2d(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_2d
#define LOAD_4_HALF_TEXELS_2d \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"    float4 T00 = vload_half4((uint)coordQ.x, base0); \n" \
"    float4 T10 = vload_half4((uint)coordQ.z, base0); \n" \
"    half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"    float4 T01 = vload_half4((uint)coordQ.x, base1); \n" \
"    float4 T11 = vload_half4((uint)coordQ.z, base1); \n"

#define LOAD_4_Half_TEXELS_2d_R(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_2d_R
#define LOAD_4_HALF_TEXELS_2d_R \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"    float4 T00 = (float4)(vload_half((uint)coordQ.x, base0), 0.0, 0.0, 1.0); \n" \
"    float4 T10 = (float4)(vload_half((uint)coordQ.z, base0), 0.0, 0.0, 1.0); \n" \
"    half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"    float4 T01 = (float4)(vload_half((uint)coordQ.x, base1), 0.0, 0.0, 1.0); \n" \
"    float4 T11 = (float4)(vload_half((uint)coordQ.z, base1), 0.0, 0.0, 1.0); \n"

#define LOAD_4_Half_TEXELS_2d_BGRA LOAD_4_Half_TEXELS_2d

#define LOAD_4_Half_TEXELS_1darray(_TYPE_, _BASE_) \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    float4 T00 = vload_half4((uint)coordQ.x, base0); \n" \
"    float4 T11 = vload_half4((uint)coordQ.y, base0); \n" \

#define LOAD_4_Half_TEXELS_1darray_BGRA LOAD_4_Half_TEXELS_1darray
#define LOAD_4_Half_TEXELS_1darray_R(_TYPE_, _BASE_) \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    float4 T00 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"    float4 T11 = (float4)(vload_half((uint)coordQ.y, base0), .0f, .0f, 1.0f); \n" \

#define LOAD_4_Half_TEXELS_2DARRAY(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_2DARRAY
#define LOAD_4_HALF_TEXELS_2DARRAY \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"    float4 T00 = vload_half4((uint)coordQ.x, base0); \n" \
"    float4 T10 = vload_half4((uint)coordQ.z, base0); \n" \
"    half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"    float4 T01 = vload_half4((uint)coordQ.x, base1); \n" \
"    float4 T11 = vload_half4((uint)coordQ.z, base1); \n"

#define LOAD_4_Half_TEXELS_2DARRAY_BGRA LOAD_4_Half_TEXELS_2DARRAY
#define LOAD_4_Half_TEXELS_2DARRAY_R(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_2DARRAY_R
#define LOAD_4_HALF_TEXELS_2DARRAY_R \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"    float4 T00 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"    float4 T10 = (float4)(vload_half((uint)coordQ.z, base0), .0f, .0f, 1.0f); \n" \
"    half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"    float4 T01 = (float4)(vload_half((uint)coordQ.x, base1), .0f, .0f, 1.0f); \n" \
"    float4 T11 = (float4)(vload_half((uint)coordQ.z, base1), .0f, .0f, 1.0f); \n"

#define LOAD_4_Half_TEXELS_3d(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_3d
#define LOAD_4_HALF_TEXELS_3d \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"    float4 T000 = vload_half4((uint)coordQ.x, base0); \n" \
"    float4 T100 = vload_half4((uint)coordR.x, base0); \n" \
"    half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"    float4 T010 = vload_half4((uint)coordQ.x, base1); \n" \
"    float4 T110 = vload_half4((uint)coordR.x, base1); \n" \
"    half * base2 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"    float4 T001 = vload_half4((uint)coordQ.x, base2); \n" \
"    float4 T101 = vload_half4((uint)coordR.x, base2); \n" \
"    half * base3 = (half *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"    float4 T011 = vload_half4((uint)coordQ.x, base3); \n" \
"    float4 T111 = vload_half4((uint)coordR.x, base3); \n"

#define LOAD_4_Half_TEXELS_3d_BGRA LOAD_4_Half_TEXELS_3d
#define LOAD_4_Half_TEXELS_3d_R(_TYPE_, _BASE_) LOAD_4_HALF_3d_R
#define LOAD_4_HALF_3d_R \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"    float4 T000 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"    float4 T100 = (float4)(vload_half((uint)coordR.x, base0), .0f, .0f, 1.0f); \n" \
"    half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"    float4 T010 = (float4)(vload_half((uint)coordQ.x, base1), .0f, .0f, 1.0f); \n" \
"    float4 T110 = (float4)(vload_half((uint)coordR.x, base1), .0f, .0f, 1.0f); \n" \
"    half * base2 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"    float4 T001 = (float4)(vload_half((uint)coordQ.x, base2), .0f, .0f, 1.0f); \n" \
"    float4 T101 = (float4)(vload_half((uint)coordR.x, base2), .0f, .0f, 1.0f); \n" \
"    half * base3 = (half *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"    float4 T011 = (float4)(vload_half((uint)coordQ.x, base3), .0f, .0f, 1.0f); \n" \
"    float4 T111 = (float4)(vload_half((uint)coordR.x, base3), .0f, .0f, 1.0f); \n"

#define LOAD_4_Float_TEXELS_1d(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_1d
#define LOAD_4_FLOAT_TEXELS_1d \
"    float4 * base0 = (float4 *) ((uchar *)image); \n" \
"    float4 T00 = base0[coordQ.x]; \n" \
"    float4 T11 = base0[coordQ.y]; \n"

#define LOAD_4_Float_TEXELS_1d_BGRA LOAD_4_Float_TEXELS_1d
#define LOAD_4_Float_TEXELS_1d_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_1d_R
#define LOAD_4_FLOAT_TEXELS_1d_R \
"    float * base0 = (float *) ((uchar *)image); \n" \
"    float4 T00 = (float4)(base0[coordQ.x], .0f, .0f, 1.0f); \n" \
"    float4 T11 = (float4)(base0[coordQ.y], .0f, .0f, 1.0f); \n"

#define LOAD_4_Float_TEXELS_1dbuffer(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_1d

#define LOAD_4_Float_TEXELS_1dbuffer_BGRA LOAD_4_Float_TEXELS_1dbuffer
#define LOAD_4_Float_TEXELS_1dbuffer_R LOAD_4_Float_TEXELS_1dbuffer

#define LOAD_4_Float_TEXELS_2d(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_2d
#define LOAD_4_FLOAT_TEXELS_2d \
"    float4 * base0 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"    float4 T00 = base0[coordQ.x]; \n" \
"    float4 T10 = base0[coordQ.z]; \n" \
"    float4 * base1 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"    float4 T01 = base1[coordQ.x]; \n" \
"    float4 T11 = base1[coordQ.z]; \n"

#define LOAD_4_Float_TEXELS_2d_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_2d_R
#define LOAD_4_FLOAT_TEXELS_2d_R \
"    float * base0 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"    float4 T00 = (float4)(base0[coordQ.x], 0.0, 0.0, 1.0); \n" \
"    float4 T10 = (float4)(base0[coordQ.z], 0.0, 0.0, 1.0); \n" \
"    float * base1 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"    float4 T01 = (float4)(base1[coordQ.x], 0.0, 0.0, 1.0); \n" \
"    float4 T11 = (float4)(base1[coordQ.z], 0.0, 0.0, 1.0); \n"

#define LOAD_4_Float_TEXELS_2d_BGRA LOAD_4_Float_TEXELS_2d

#define LOAD_4_Float_TEXELS_1darray(_TYPE_, _BASE_) \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    float4 * base0 = (float4 *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    float4 T00 = base0[coordQ.x]; \n" \
"    float4 T11 = base0[coordQ.y]; \n"

#define LOAD_4_Float_TEXELS_1darray_BGRA LOAD_4_Float_TEXELS_1darray
#define LOAD_4_Float_TEXELS_1darray_R(_TYPE_, _BASE_) \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    float * base0 = (float *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    float4 T00 = (float4)(base0[coordQ.x], .0f, .0f, 1.0f); \n" \
"    float4 T11 = (float4)(base0[coordQ.y], .0f, .0f, 1.0f); \n"

#define LOAD_4_Float_TEXELS_2DARRAY(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_2DARRAY
#define LOAD_4_FLOAT_TEXELS_2DARRAY \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    float4 * base0 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"    float4 T00 = base0[coordQ.x]; \n" \
"    float4 T10 = base0[coordQ.z]; \n" \
"    float4 * base1 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"    float4 T01 = base1[coordQ.x]; \n" \
"    float4 T11 = base1[coordQ.z]; \n"

#define LOAD_4_Float_TEXELS_2DARRAY_BGRA LOAD_4_Float_TEXELS_2DARRAY
#define LOAD_4_Float_TEXELS_2DARRAY_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_2DARRAY_R
#define LOAD_4_FLOAT_TEXELS_2DARRAY_R \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    float * base0 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"    float4 T00 = (float4)(base0[coordQ.x], .0f, .0f, 1.0f); \n" \
"    float4 T10 = (float4)(base0[coordQ.z], .0f, .0f, 1.0f); \n" \
"    float * base1 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"    float4 T01 = (float4)(base1[coordQ.x], .0f, .0f, 1.0f); \n" \
"    float4 T11 = (float4)(base1[coordQ.z], .0f, .0f, 1.0f); \n"

#define LOAD_4_Float_TEXELS_3d(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_3d
#define LOAD_4_FLOAT_TEXELS_3d \
"    float4 * base0 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"    float4 T000 = base0[(uint)coordQ.x]; \n" \
"    float4 T100 = base0[(uint)coordR.x]; \n" \
"    float4 * base1 = (float4 *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"    float4 T010 = base1[(uint)coordQ.x]; \n" \
"    float4 T110 = base1[(uint)coordR.x]; \n" \
"    float4 * base2 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"    float4 T001 = base2[(uint)coordQ.x]; \n" \
"    float4 T101 = base2[(uint)coordR.x]; \n" \
"    float4 * base3 = (float4 *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"    float4 T011 = base3[(uint)coordQ.x]; \n" \
"    float4 T111 = base3[(uint)coordR.x]; \n"

#define LOAD_4_Float_TEXELS_3d_BGRA LOAD_4_Float_TEXELS_3d
#define LOAD_4_Float_TEXELS_3d_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_3d_R
#define LOAD_4_FLOAT_TEXELS_3d_R \
"    float * base0 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"    float4 T000 = (float4)(base0[(uint)coordQ.x], .0f, .0f, 1.0f); \n" \
"    float4 T100 = (float4)(base0[(uint)coordR.x], .0f, .0f, 1.0f); \n" \
"    float * base1 = (float *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"    float4 T010 = (float4)(base1[(uint)coordQ.x], .0f, .0f, 1.0f); \n" \
"    float4 T110 = (float4)(base1[(uint)coordR.x], .0f, .0f, 1.0f); \n" \
"    float * base2 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"    float4 T001 = (float4)(base2[(uint)coordQ.x], .0f, .0f, 1.0f); \n" \
"    float4 T101 = (float4)(base2[(uint)coordR.x], .0f, .0f, 1.0f); \n" \
"    float * base3 = (float *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"    float4 T011 = (float4)(base3[(uint)coordQ.x], .0f, .0f, 1.0f); \n" \
"    float4 T111 = (float4)(base3[(uint)coordR.x], .0f, .0f, 1.0f); \n"

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d(_TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = convert_float4(base0[coordQ.y]) / " #_BASE_ "; \n" \
"    } \n" \


#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_BGRA LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)(0.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
    "        T00 = convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)(0.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = convert_float4((" #_TYPE_ ")(base0[coordQ.y], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    } \n" \

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(half,_TYPE_, _BASE_)


#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R(_TYPE_, _BASE_) LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer(_TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = convert_float4(base0[coordQ.y]) / " #_BASE_ "; \n" \
"    } \n" \


#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_BGRA LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1d_CONV(half,_TYPE_, _BASE_)


#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R(_TYPE_, _BASE_) LOAD_4_UNORM_TEXELS_BORDER_CHECK_1dbuffer_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d(_TYPE_, _BASE_) \
"    float4 T00, T01, T10, T11; \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.y) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"        T10 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T00 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T10 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = convert_float4(base0[coordQ.z]) / " #_BASE_ "; \n" \
"        } \n" \
"    } \n" \
"    if ((uint)coordQ.w >= (uint)imageSize.y) \n" \
"    { \n" \
"        T01 = (float4)0.0; \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + (uint)image.y * (uint)coordQ.w); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T01 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = convert_float4(base1[coordQ.x]) / " #_BASE_ "; \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T11 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = convert_float4(base1[coordQ.z]) / " #_BASE_ "; \n" \
"        } \n" \
"    } \n"



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_BGRA LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d


#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T00, T01, T10, T11; \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.y) \n" \
"    { \n" \
"        T00 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        T10 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T00 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T10 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = convert_float4((" #_TYPE_ ")(base0[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"        } \n" \
"    } \n" \
"    if ((uint)coordQ.w >= (uint)imageSize.y) \n" \
"    { \n" \
"        T01 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        T11 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + (uint)image.y * (uint)coordQ.w); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T01 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = convert_float4((" #_TYPE_ ")(base1[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T11 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = convert_float4((" #_TYPE_ ")(base1[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"        } \n" \
"    } \n"


#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_CONV(uchar, _TYPE_, _BASE_)



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_CONV(ushort, _TYPE_, _BASE_)



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_CONV(short, _TYPE_, _BASE_)



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_CONV(uint,_TYPE_, _BASE_)



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_CONV(int,_TYPE_, _BASE_)



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_CONV(float,_TYPE_, _BASE_)



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_CONV(half,_TYPE_, _BASE_)



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R(_TYPE_, _BASE_) LOAD_4_UNORM_TEXELS_BORDER_CHECK_2d_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray(_TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.x) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = convert_float4(base0[coordQ.y]) / " #_BASE_ "; \n" \
"    } \n" \

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"    { \n" \
"        T00 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = convert_float4((" #_TYPE_ ")(base0[coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.x) \n" \
"    { \n" \
"        T11 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = convert_float4((" #_TYPE_ ")(base0[coordQ.y], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"    } \n" \

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_BGRA LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_CONV(half,_TYPE_, _BASE_)


#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R(_TYPE_, _BASE_) LOAD_4_UNORM_TEXELS_BORDER_CHECK_1darray_R_##_TYPE_(_TYPE_,_BASE_)



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY(_TYPE_, _BASE_) \
"    float4 T00, T01, T10, T11; \n" \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    if (((uint)coordQ.y >= (uint)imageSize.y) || (coordQ.y < 0)) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"        T10 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T00 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = convert_float4(base0[coordQ.x]) / " #_BASE_ "; \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T10 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = convert_float4(base0[coordQ.z]) / " #_BASE_ "; \n" \
"        } \n" \
"    } \n" \
"    if (((uint)coordQ.w >= (uint)imageSize.y) || (coordQ.w < 0)) \n" \
"    { \n" \
"        T01 = (float4)0.0; \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T01 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = convert_float4(base1[coordQ.x]) / " #_BASE_ "; \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T11 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = convert_float4(base1[coordQ.z]) / " #_BASE_ "; \n" \
"        } \n" \
"    } \n"



#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_BGRA LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T00, T01, T10, T11; \n" \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    if (((uint)coordQ.y >= (uint)imageSize.y) || (coordQ.y < 0)) \n" \
"    { \n" \
"        T00 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T10 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T00 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T10 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = convert_float4((" #_TYPE_ ")(base0[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"        } \n" \
"    } \n" \
"    if (((uint)coordQ.w >= (uint)imageSize.y) || (coordQ.w < 0)) \n" \
"    { \n" \
"        T01 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T11 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T01 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = convert_float4((" #_TYPE_ ")(base1[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T11 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = convert_float4((" #_TYPE_ ")(base1[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ "; \n" \
"        } \n" \
"    } \n"

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R(_TYPE_, _BASE_) LOAD_4_UNORM_TEXELS_BORDER_CHECK_2DARRAY_R_##_TYPE_(_TYPE_,_BASE_)


#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d(_TYPE_, _BASE_) \
"    float4 T000, T010, T100, T110, T001, T011, T101, T111; \n" \
"    if (((int)coordQ.z >= (int)imageSize.z) || ((int)coordQ.z < 0)) \n" \
"    { \n" \
"        T000 = (float4)0.0; \n" \
"        T100 = (float4)0.0; \n" \
"        T010 = (float4)0.0; \n" \
"        T110 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T000 = (float4)0.0; \n" \
"            T100 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T000 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T000 = convert_float4(base0[(int)coordQ.x]) / " #_BASE_ "; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T100 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T100 = convert_float4(base0[(int)coordR.x]) / " #_BASE_ "; \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T010 = (float4)0.0; \n" \
"            T110 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T010 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T010 = convert_float4(base1[(int)coordQ.x]) / " #_BASE_ "; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T110 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T110 = convert_float4(base1[(int)coordR.x]) / " #_BASE_ "; \n" \
"            } \n" \
"        } \n" \
"    } \n" \
"    if (((int)coordR.z >= (int)imageSize.z) || ((int)coordR.z < 0)) \n" \
"    { \n" \
"        T001 = (float4)0.0; \n" \
"        T101 = (float4)0.0; \n" \
"        T011 = (float4)0.0; \n" \
"        T111 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T001 = (float4)0.0; \n" \
"            T101 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T001 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T001 = convert_float4(base0[(int)coordQ.x]) / " #_BASE_ "; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T101 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T101 = convert_float4(base0[(int)coordR.x]) / " #_BASE_ "; \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T011 = (float4)0.0; \n" \
"            T111 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T011 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T011 = convert_float4(base1[(int)coordQ.x]) / " #_BASE_ "; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T111 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T111 = convert_float4(base1[(int)coordR.x]) / " #_BASE_ "; \n" \
"            } \n" \
"        } \n" \
"    } \n"


#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_BGRA LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T000, T010, T100, T110, T001, T011, T101, T111; \n" \
"    if (((int)coordQ.z >= (int)imageSize.z) || ((int)coordQ.z < 0)) \n" \
"    { \n" \
"        T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T000 = convert_float4((" #_TYPE_ ")(base0[(int)coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T100 = convert_float4((" #_TYPE_ ")(base0[(int)coordR.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T010 = convert_float4((" #_TYPE_ ")(base1[(int)coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T110 = convert_float4((" #_TYPE_ ")(base1[(int)coordR.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"            } \n" \
"        } \n" \
"    } \n" \
"    if (((int)coordR.z >= (int)imageSize.z) || ((int)coordR.z < 0)) \n" \
"    { \n" \
"        T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T001 = convert_float4((" #_TYPE_ ")(base0[(int)coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T101 = convert_float4((" #_TYPE_ ")(base0[(int)coordR.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T011 = convert_float4((" #_TYPE_ ")(base1[(int)coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T111 = convert_float4((" #_TYPE_ ")(base1[(int)coordR.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ "; \n" \
"            } \n" \
"        } \n" \
"    } \n"

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R_half(_TYPE_, _BASE_) \
    LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R(_TYPE_, _BASE_) LOAD_4_UNORM_TEXELS_BORDER_CHECK_3d_R_##_TYPE_(_TYPE_,_BASE_)



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d(_TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = max(convert_float4(base0[coordQ.y]) / " #_BASE_ ", -1.0); \n" \
"    } \n" \

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.y], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    } \n" \

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d_BGRA LOAD_4_SNORM_TEXELS_BORDER_CHECK_1d

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer(_TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = max(convert_float4(base0[coordQ.y]) / " #_BASE_ ", -1.0); \n" \
"    } \n" \

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    } \n" \

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_BGRA LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_BORDER_CHECK_1dbuffer_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d(_TYPE_, _BASE_) \
"    float4 T00, T01, T10, T11; \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.y) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"        T10 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T00 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T10 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = max(convert_float4(base0[coordQ.z]) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"    } \n" \
"    if ((uint)coordQ.w >= (uint)imageSize.y) \n" \
"    { \n" \
"        T01 = (float4)0.0; \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T01 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = max(convert_float4(base1[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T11 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = max(convert_float4(base1[coordQ.z]) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"    } \n"



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_BGRA LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d


#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T00, T01, T10, T11; \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.y) \n" \
"    { \n" \
"        T00 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        T10 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T00 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T10 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"    } \n" \
"    if ((uint)coordQ.w >= (uint)imageSize.y) \n" \
"    { \n" \
"        T01 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        T11 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T01 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = max(convert_float4((" #_TYPE_ ")(base1[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T11 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = max(convert_float4((" #_TYPE_ ")(base1[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"    } \n"


#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_CONV(uchar, _TYPE_, _BASE_)



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_CONV(ushort, _TYPE_, _BASE_)



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_CONV(short, _TYPE_, _BASE_)



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_CONV(uint,_TYPE_, _BASE_)



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_CONV(int,_TYPE_, _BASE_)



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_CONV(float,_TYPE_, _BASE_)



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_CONV(half,_TYPE_, _BASE_)




#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_BORDER_CHECK_2d_R_##_TYPE_(_TYPE_,_BASE_)


#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray(_TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.x) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = max(convert_float4(base0[coordQ.y]) / " #_BASE_ ", -1.0); \n" \
"    } \n" \

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"    { \n" \
"        T00 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.x) \n" \
"    { \n" \
"        T11 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.y], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"    } \n" \

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_BGRA LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_R_##_TYPE_(_TYPE_,_BASE_)


#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY(_TYPE_, _BASE_) \
"    float4 T00, T01, T10, T11; \n" \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    if (((uint)coordQ.y >= (uint)imageSize.y) || (coordQ.y < 0)) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"        T10 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T00 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = max(convert_float4(base0[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T10 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = max(convert_float4(base0[coordQ.z]) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"    } \n" \
"    if (((uint)coordQ.w >= (uint)imageSize.y) || (coordQ.w < 0)) \n" \
"    { \n" \
"        T01 = (float4)0.0; \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T01 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = max(convert_float4(base1[coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T11 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = max(convert_float4(base1[coordQ.z]) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"    } \n"



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_BGRA LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T00, T01, T10, T11; \n" \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    if (((uint)coordQ.y >= (uint)imageSize.y) || (coordQ.y < 0)) \n" \
"    { \n" \
"        T00 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"        T10 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T00 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T10 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = max(convert_float4((" #_TYPE_ ")(base0[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"    } \n" \
"    if (((uint)coordQ.w >= (uint)imageSize.y) || (coordQ.w < 0)) \n" \
"    { \n" \
"        T01 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"        T11 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T01 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = max(convert_float4((" #_TYPE_ ")(base1[coordQ.x], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T11 = (float4)(.0f, .0f, .0f, 1.f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = max(convert_float4((" #_TYPE_ ")(base1[coordQ.z], 0, 0, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"        } \n" \
"    } \n"

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_1darray_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_BORDER_CHECK_2DARRAY_R_##_TYPE_(_TYPE_,_BASE_)



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d(_TYPE_, _BASE_) \
"    float4 T000, T010, T100, T110, T001, T011, T101, T111; \n" \
"    if (((int)coordQ.z >= (int)imageSize.z) || ((int)coordQ.z < 0)) \n" \
"    {\n" \
"        T000 = (float4)0.0; \n" \
"        T100 = (float4)0.0; \n" \
"        T010 = (float4)0.0; \n" \
"        T110 = (float4)0.0; \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T000 = (float4)0.0; \n" \
"            T100 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T000 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T000 = max(convert_float4(base0[(int)coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T100 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T100 = max(convert_float4(base0[(int)coordR.x]) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T010 = (float4)0.0; \n" \
"            T110 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T010 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T010 = max(convert_float4(base1[(int)coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T110 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T110 = max(convert_float4(base1[(int)coordR.x]) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"        } \n" \
"    }\n" \
"    if (((int)coordR.z >= (int)imageSize.z) || ((int)coordR.z < 0)) \n" \
"    {\n" \
"        T001 = (float4)0.0; \n" \
"        T101 = (float4)0.0; \n" \
"        T011 = (float4)0.0; \n" \
"        T111 = (float4)0.0; \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T001 = (float4)0.0; \n" \
"            T101 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_TYPE_ " * base0 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T001 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T001 = max(convert_float4(base0[(int)coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T101 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T101 = max(convert_float4(base0[(int)coordR.x]) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T011 = (float4)0.0; \n" \
"            T111 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_TYPE_ " * base1 = (" #_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T011 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T011 = max(convert_float4(base1[(int)coordQ.x]) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T111 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T111 = max(convert_float4(base1[(int)coordR.x]) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"        } \n" \
"    }\n"



#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_BGRA LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_CONV(_CONV_TYPE_, _TYPE_, _BASE_) \
"    float4 T000, T010, T100, T110, T001, T011, T101, T111; \n" \
"    if (((int)coordQ.z >= (int)imageSize.z) || ((int)coordQ.z < 0)) \n" \
"    {\n" \
"        T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T000 = max(convert_float4((" #_TYPE_ ")(base0[(int)coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T100 = max(convert_float4((" #_TYPE_ ")(base0[(int)coordR.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T010 = max(convert_float4((" #_TYPE_ ")(base1[(int)coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T110 = max(convert_float4((" #_TYPE_ ")(base1[(int)coordR.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"        } \n" \
"    }\n" \
"    if (((int)coordR.z >= (int)imageSize.z) || ((int)coordR.z < 0)) \n" \
"    {\n" \
"        T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_CONV_TYPE_ " * base0 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T001 = max(convert_float4((" #_TYPE_ ")(base0[(int)coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T101 = max(convert_float4((" #_TYPE_ ")(base0[(int)coordR.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            " #_CONV_TYPE_ " * base1 = (" #_CONV_TYPE_ " *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T011 = max(convert_float4((" #_TYPE_ ")(base1[(int)coordQ.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T111 = max(convert_float4((" #_TYPE_ ")(base1[(int)coordR.x], .0f, .0f, " #_BASE_ ")) / " #_BASE_ ", -1.0); \n" \
"            } \n" \
"        } \n" \
"    }\n"

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R_char4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_CONV(char, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R_uchar4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_CONV(uchar, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R_ushort4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_CONV(ushort, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R_short4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_CONV(short, _TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R_uint4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_CONV(uint,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R_int4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_CONV(int,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R_float4(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_CONV(float,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R_half(_TYPE_, _BASE_) \
    LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_CONV(half,_TYPE_, _BASE_)

#define LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R(_TYPE_, _BASE_) \
       LOAD_4_SNORM_TEXELS_BORDER_CHECK_3d_R_##_TYPE_(_TYPE_,_BASE_)

#define LOAD_4_Half_TEXELS_BORDER_CHECK_1d(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_1d
#define LOAD_4_Half_TEXELS_BORDER_CHECK_1dbuffer(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_1d
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_1d \
"    float4 T00, T11; \n" \
"    half * base0 = (half *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = vload_half4((uint)coordQ.x, base0); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = vload_half4((uint)coordQ.y, base0); \n" \
"    } \n"



#define LOAD_4_Half_TEXELS_BORDER_CHECK_1d_BGRA LOAD_4_Half_TEXELS_BORDER_CHECK_1d

#define LOAD_4_Half_TEXELS_BORDER_CHECK_1d_R(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_1d_R
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_1d_R \
"    float4 T00, T11; \n" \
"    half * base0 = (half *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = (float4)(vload_half((uint)coordQ.y, base0), .0f, .0f, 1.0f); \n" \
"    } \n"



#define LOAD_4_Half_TEXELS_BORDER_CHECK_1dbuffer_BGRA LOAD_4_Half_TEXELS_BORDER_CHECK_1dbuffer

#define LOAD_4_Half_TEXELS_BORDER_CHECK_1dbuffer_R(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_1dbuffer_R
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_1dbuffer_R \
"    float4 T00, T11; \n" \
"    half * base0 = (half *) ((uchar *)image ); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = (float4)(vload_half((uint)coordQ.y, base0), .0f, .0f, 1.0f); \n" \
"    } \n"

#define LOAD_4_Half_TEXELS_BORDER_CHECK_2d(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_2d
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_2d \
"    float4 T00, T01, T10, T11; \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.y) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"        T10 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T00 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = vload_half4((uint)coordQ.x, base0); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T10 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = vload_half4((uint)coordQ.z, base0); \n" \
"        } \n" \
"    } \n" \
"    if ((uint)coordQ.w >= (uint)imageSize.y) \n" \
"    { \n" \
"        T01 = (float4)0.0; \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T01 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = vload_half4((uint)coordQ.x, base1); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T11 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = vload_half4((uint)coordQ.z, base1); \n" \
"        } \n" \
"    } \n"

#define LOAD_4_Half_TEXELS_BORDER_CHECK_2d_R(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_2d_R
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_2d_R \
"    float4 T00, T01, T10, T11; \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.y) \n" \
"    { \n" \
"        T00 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        T10 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T00 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = (float4)(vload_half((uint)coordQ.x, base0), 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T10 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = (float4)(vload_half((uint)coordQ.z, base0), 0.0, 0.0, 1.0); \n" \
"        } \n" \
"    } \n" \
"    if ((uint)coordQ.w >= (uint)imageSize.y) \n" \
"    { \n" \
"        T01 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        T11 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T01 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = (float4)(vload_half((uint)coordQ.x, base1), 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T11 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = (float4)(vload_half((uint)coordQ.z, base1), 0.0, 0.0, 1.0); \n" \
"        } \n" \
"    } \n"



#define LOAD_4_Half_TEXELS_BORDER_CHECK_2d_BGRA LOAD_4_Half_TEXELS_BORDER_CHECK_2d

#define LOAD_4_Half_TEXELS_BORDER_CHECK_1darray(_TYPE_, _BASE_) \
"    float4 T00, T11; \n" \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = vload_half4((uint)coordQ.x, base0); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.x) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = vload_half4((uint)coordQ.y, base0); \n" \
"    } \n"



#define LOAD_4_Half_TEXELS_BORDER_CHECK_1darray_BGRA LOAD_4_Half_TEXELS_BORDER_CHECK_1darray

#define LOAD_4_Half_TEXELS_BORDER_CHECK_1darray_R(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_1darray_R
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_1darray_R \
    "    float4 T00, T11; \n" \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    half * base0 = (half *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"    { \n" \
"        T00 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.x) \n" \
"    { \n" \
"        T11 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = (float4)(vload_half((uint)coordQ.y, base0), .0f, .0f, 1.0f); \n" \
"    } \n"


#define LOAD_4_Half_TEXELS_BORDER_CHECK_2DARRAY(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_2DARRAY
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_2DARRAY \
"    float4 T00, T01, T10, T11; \n" \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    if (((uint)coordQ.y >= (uint)imageSize.y) || (coordQ.y < 0)) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"        T10 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T00 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = vload_half4((uint)coordQ.x, base0); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T10 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = vload_half4((uint)coordQ.z, base0); \n" \
"        } \n" \
"    } \n" \
"    if (((uint)coordQ.w >= (uint)imageSize.y) || (coordQ.w < 0)) \n" \
"    { \n" \
"        T01 = (float4)0.0; \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T01 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = vload_half4((uint)coordQ.x, base1); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T11 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = vload_half4((uint)coordQ.z, base1); \n" \
"        } \n" \
"    } \n"



#define LOAD_4_Half_TEXELS_BORDER_CHECK_2DARRAY_BGRA LOAD_4_Half_TEXELS_BORDER_CHECK_2DARRAY

#define LOAD_4_Half_TEXELS_BORDER_CHECK_2DARRAY_R(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_2DARRAY_R
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_2DARRAY_R \
"    float4 T00, T01, T10, T11; \n" \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    if (((uint)coordQ.y >= (uint)imageSize.y) || (coordQ.y < 0)) \n" \
"    { \n" \
"        T00 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T10 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T00 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T10 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = (float4)(vload_half((uint)coordQ.z, base0), .0f, .0f, 1.0f); \n" \
"        } \n" \
"    } \n" \
"    if (((uint)coordQ.w >= (uint)imageSize.y) || (coordQ.w < 0)) \n" \
"    { \n" \
"        T01 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T11 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T01 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = (float4)(vload_half((uint)coordQ.x, base1), .0f, .0f, 1.0f); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T11 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = (float4)(vload_half((uint)coordQ.z, base1), .0f, .0f, 1.0f); \n" \
"        } \n" \
"    } \n"


#define LOAD_4_Half_TEXELS_BORDER_CHECK_3d(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_3d
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_3d \
"    float4 T000, T010, T100, T110, T001, T011, T101, T111; \n" \
"    if (((int)coordQ.z >= (int)imageSize.z) || ((int)coordQ.z < 0)) \n" \
"    {\n" \
"        T000 = (float4)0.0; \n" \
"        T100 = (float4)0.0; \n" \
"        T010 = (float4)0.0; \n" \
"        T110 = (float4)0.0; \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T000 = (float4)0.0; \n" \
"            T100 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T000 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T000 = vload_half4((uint)coordQ.x, base0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T100 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T100 = vload_half4((uint)coordR.x, base0); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T010 = (float4)0.0; \n" \
"            T110 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T010 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T010 = vload_half4((uint)coordQ.x, base1); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T110 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T110 = vload_half4((uint)coordR.x, base1); \n" \
"            } \n" \
"        } \n" \
"    } \n"\
"    if (((int)coordR.z >= (int)imageSize.z) || ((int)coordR.z < 0)) \n" \
"    {\n" \
"        T001 = (float4)0.0; \n" \
"        T101 = (float4)0.0; \n" \
"        T011 = (float4)0.0; \n" \
"        T111 = (float4)0.0; \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T001 = (float4)0.0; \n" \
"            T101 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T001 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T001 = vload_half4((uint)coordQ.x, base0); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T101 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T101 = vload_half4((uint)coordR.x, base0); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T011 = (float4)0.0; \n" \
"            T111 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T011 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T011 = vload_half4((uint)coordQ.x, base1); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T111 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T111 = vload_half4((uint)coordR.x, base1); \n" \
"            } \n" \
"        } \n" \
"    } \n"\



#define LOAD_4_Half_TEXELS_BORDER_CHECK_3d_BGRA LOAD_4_Half_TEXELS_BORDER_CHECK_3d

#define LOAD_4_Half_TEXELS_BORDER_CHECK_3d_R(_TYPE_, _BASE_) LOAD_4_HALF_TEXELS_BORDER_CHECK_3d_R
#define LOAD_4_HALF_TEXELS_BORDER_CHECK_3d_R \
"    float4 T000, T010, T100, T110, T001, T011, T101, T111; \n" \
"    if (((int)coordQ.z >= (int)imageSize.z) || ((int)coordQ.z < 0)) \n" \
"    {\n" \
"        T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T000 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T100 = (float4)(vload_half((uint)coordR.x, base0), .0f, .0f, 1.0f); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T010 = (float4)(vload_half((uint)coordQ.x, base1), .0f, .0f, 1.0f); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T110 = (float4)(vload_half((uint)coordR.x, base1), .0f, .0f, 1.0f); \n" \
"            } \n" \
"        } \n" \
"    } \n"\
"    if (((int)coordR.z >= (int)imageSize.z) || ((int)coordR.z < 0)) \n" \
"    {\n" \
"        T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            half * base0 = (half *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T001 = (float4)(vload_half((uint)coordQ.x, base0), .0f, .0f, 1.0f); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T101 = (float4)(vload_half((uint)coordR.x, base0), .0f, .0f, 1.0f); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            half * base1 = (half *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T011 = (float4)(vload_half((uint)coordQ.x, base1), .0f, .0f, 1.0f); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T111 = (float4)(vload_half((uint)coordR.x, base1), .0f, .0f, 1.0f); \n" \
"            } \n" \
"        } \n" \
"    } \n"\

#define LOAD_4_Float_TEXELS_BORDER_CHECK_1d(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1d
#define LOAD_4_Float_TEXELS_BORDER_CHECK_1dbuffer(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1d
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1d \
"    float4 T00, T11; \n" \
"    float4 * base0 = (float4 *) ((uchar *)image); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = base0[coordQ.x]; \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = base0[coordQ.y]; \n" \
"    } \n" \



#define LOAD_4_Float_TEXELS_BORDER_CHECK_1d_BGRA LOAD_4_Float_TEXELS_BORDER_CHECK_1d

#define LOAD_4_Float_TEXELS_BORDER_CHECK_1d_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1d_R
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1d_R \
"    float4 T00, T11; \n" \
"    float * base0 = (float *) ((uchar *)image); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = (float4)(base0[coordQ.x], .0f, .0f, 1.0f); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = (float4)(base0[coordQ.y], .0f, .0f, 1.0f); \n" \
"    } \n" \



#define LOAD_4_Float_TEXELS_BORDER_CHECK_1dbuffer_BGRA LOAD_4_Float_TEXELS_BORDER_CHECK_1dbuffer


#define LOAD_4_Float_TEXELS_BORDER_CHECK_1dbuffer_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1dbuffer_R
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1dbuffer_R \
"    float4 T00, T11; \n" \
"    float * base0 = (float *) ((uchar *)image); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize) \n" \
"    { \n" \
"        T00 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = (float4)(base0[coordQ.x], .0f, .0f, 1.0f); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize) \n" \
"    { \n" \
"        T11 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = (float4)(base0[coordQ.y], .0f, .0f, 1.0f); \n" \
"    } \n" \

#define LOAD_4_Float_TEXELS_BORDER_CHECK_2d(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_2d
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_2d \
"    float4 T00, T01, T10, T11; \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.y) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"        T10 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        float4 * base0 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T00 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = base0[coordQ.x]; \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T10 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = base0[coordQ.z]; \n" \
"        } \n" \
"    } \n" \
"    if ((uint)coordQ.w >= (uint)imageSize.y) \n" \
"    { \n" \
"        T01 = (float4)0.0; \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        float4 * base1 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T01 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = base1[coordQ.x]; \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T11 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = base1[coordQ.z];\n" \
"        } \n" \
"    } \n"



#define LOAD_4_Float_TEXELS_BORDER_CHECK_2d_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_2d_R
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_2d_R \
"    float4 T00, T01, T10, T11; \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.y) \n" \
"    { \n" \
"        T00 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        T10 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        float * base0 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.y); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T00 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = (float4)(base0[coordQ.x], 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T10 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = (float4)(base0[coordQ.z], 0.0, 0.0, 1.0); \n" \
"        } \n" \
"    } \n" \
"    if ((uint)coordQ.w >= (uint)imageSize.y) \n" \
"    { \n" \
"        T01 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        T11 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        float * base1 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.w); \n" \
"        if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"        { \n" \
"            T01 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = (float4)(base1[coordQ.x], 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        if ((uint)coordQ.z >= (uint)imageSize.x) \n" \
"        { \n" \
"            T11 = (float4)(0.0, 0.0, 0.0, 1.0); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = (float4)(base1[coordQ.z], 0.0, 0.0, 1.0);\n" \
"        } \n" \
"    } \n"



#define LOAD_4_Float_TEXELS_BORDER_CHECK_2d_BGRA LOAD_4_Float_TEXELS_BORDER_CHECK_2d

#define LOAD_4_Float_TEXELS_BORDER_CHECK_1darray(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1darray
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1darray \
"    float4 T00, T11; \n" \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    float4 * base0 = (float4 *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = base0[coordQ.x]; \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.x) \n" \
"    { \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = base0[coordQ.y]; \n" \
"    } \n" \



#define LOAD_4_Float_TEXELS_BORDER_CHECK_1darray_BGRA LOAD_4_Float_TEXELS_BORDER_CHECK_1darray


#define LOAD_4_Float_TEXELS_BORDER_CHECK_1darray_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1darray_R
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_1darray_R \
"    float4 T00, T11; \n" \
"    int index = rint(fcoord.y); \n" \
"    index = clamp(index, 0, imageSize.y - 1); \n" \
"    float * base0 = (float *) ((uchar *)image.x + image.y * (uint)index); \n" \
"    if ((uint)coordQ.x >= (uint)imageSize.x) \n" \
"    { \n" \
"        T00 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T00 = (float4)(base0[coordQ.x], .0f, .0f, 1.0f); \n" \
"    } \n" \
"    if ((uint)coordQ.y >= (uint)imageSize.x) \n" \
"    { \n" \
"        T11 = (float4)(0.0, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        T11 = (float4)(base0[coordQ.y], .0f, .0f, 1.0f); \n" \
"    } \n" \


#define LOAD_4_Float_TEXELS_BORDER_CHECK_2DARRAY(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_2DARRAY
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_2DARRAY \
"    float4 T00, T01, T10, T11; \n" \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    if (((uint)coordQ.y >= (uint)imageSize.y) || (coordQ.y < 0)) \n" \
"    { \n" \
"        T00 = (float4)0.0; \n" \
"        T10 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        float4 * base0 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T00 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = base0[coordQ.x]; \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T10 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = base0[coordQ.z]; \n" \
"        } \n" \
"    } \n" \
"    if (((uint)coordQ.w >= (uint)imageSize.y) || (coordQ.w < 0)) \n" \
"    { \n" \
"        T01 = (float4)0.0; \n" \
"        T11 = (float4)0.0; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        float4 * base1 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T01 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = base1[coordQ.x]; \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T11 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = base1[coordQ.z];\n" \
"        } \n" \
"    } \n"



#define LOAD_4_Float_TEXELS_BORDER_CHECK_2DARRAY_BGRA LOAD_4_Float_TEXELS_BORDER_CHECK_2DARRAY

#define LOAD_4_Float_TEXELS_BORDER_CHECK_2DARRAY_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_2DARRAY_R
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_2DARRAY_R \
"    float4 T00, T01, T10, T11; \n" \
"    int index = rint(fcoord.z); \n" \
"    index = clamp(index, 0, imageSize.z - 1); \n" \
"    if (((uint)coordQ.y >= (uint)imageSize.y) || (coordQ.y < 0)) \n" \
"    { \n" \
"        T00 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T10 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        float * base0 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T00 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T00 = (float4)(base0[coordQ.x], .0f, .0f, 1.0f); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T10 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T10 = (float4)(base0[coordQ.z], .0f, .0f, 1.0f); \n" \
"        } \n" \
"    } \n" \
"    if (((uint)coordQ.w >= (uint)imageSize.y) || (coordQ.w < 0)) \n" \
"    { \n" \
"        T01 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T11 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        float * base1 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.w + image.z * (uint)index); \n" \
"        if (((uint)coordQ.x >= (uint)imageSize.x) || (coordQ.x < 0)) \n" \
"        { \n" \
"            T01 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T01 = (float4)(base1[coordQ.x], .0f, .0f, 1.0f); \n" \
"        } \n" \
"        if (((uint)coordQ.z >= (uint)imageSize.x) || (coordQ.z < 0)) \n" \
"        { \n" \
"            T11 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            T11 = (float4)(base1[coordQ.z], .0f, .0f, 1.0f);\n" \
"        } \n" \
"    } \n"


#define LOAD_4_Float_TEXELS_BORDER_CHECK_3d(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_3d
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_3d \
"    float4 T000, T010, T100, T110, T001, T011, T101, T111; \n" \
"    if (((int)coordQ.z >= (int)imageSize.z) || ((int)coordQ.z < 0)) \n" \
"    {\n" \
"        T000 = (float4)0.0; \n" \
"        T100 = (float4)0.0; \n" \
"        T010 = (float4)0.0; \n" \
"        T110 = (float4)0.0; \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T000 = (float4)0.0; \n" \
"            T100 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            float4 * base0 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T000 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T000 = base0[(int)coordQ.x]; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T100 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T100 = base0[(int)coordR.x]; \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T010 = (float4)0.0; \n" \
"            T110 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            float4 * base1 = (float4 *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T010 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T010 = base1[(int)coordQ.x]; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T110 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T110 = base1[(int)coordR.x];\n" \
"            } \n" \
"        } \n" \
"    } \n" \
"    if (((int)coordR.z >= (int)imageSize.z) || ((int)coordR.z < 0)) \n" \
"    {\n" \
"        T001 = (float4)0.0; \n" \
"        T101 = (float4)0.0; \n" \
"        T011 = (float4)0.0; \n" \
"        T111 = (float4)0.0; \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T001 = (float4)0.0; \n" \
"            T101 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            float4 * base0 = (float4 *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T001 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T001 = base0[(int)coordQ.x]; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T101 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T101 = base0[(int)coordR.x]; \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T011 = (float4)0.0; \n" \
"            T111 = (float4)0.0; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            float4 * base1 = (float4 *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T011 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T011 = base1[(int)coordQ.x]; \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T111 = (float4)0.0; \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T111 = base1[(int)coordR.x];\n" \
"            } \n" \
"        } \n" \
"    } \n"



#define LOAD_4_Float_TEXELS_BORDER_CHECK_3d_BGRA LOAD_4_Float_TEXELS_BORDER_CHECK_3d

#define LOAD_4_Float_TEXELS_BORDER_CHECK_3d_R(_TYPE_, _BASE_) LOAD_4_FLOAT_TEXELS_BORDER_CHECK_3d_R
#define LOAD_4_FLOAT_TEXELS_BORDER_CHECK_3d_R \
"    float4 T000, T010, T100, T110, T001, T011, T101, T111; \n" \
"    if (((int)coordQ.z >= (int)imageSize.z) || ((int)coordQ.z < 0)) \n" \
"    {\n" \
"        T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            float * base0 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T000 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T000 = (float4)(base0[(int)coordQ.x], .0f, .0f, 1.0f); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T100 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T100 = (float4)(base0[(int)coordR.x], .0f, .0f, 1.0f); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            float * base1 = (float *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordQ.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T010 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T010 = (float4)(base1[(int)coordQ.x], .0f, .0f, 1.0f); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T110 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T110 = (float4)(base1[(int)coordR.x], .0f, .0f, 1.0f);\n" \
"            } \n" \
"        } \n" \
"    } \n" \
"    if (((int)coordR.z >= (int)imageSize.z) || ((int)coordR.z < 0)) \n" \
"    {\n" \
"        T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"    }\n" \
"    else\n" \
"    {\n" \
"        if (((int)coordQ.y >= (int)imageSize.y) || ((int)coordQ.y < 0)) \n" \
"        { \n" \
"            T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            float * base0 = (float *) ((uchar *)image.x + image.y * (uint)coordQ.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T001 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T001 = (float4)(base0[(int)coordQ.x], .0f, .0f, 1.0f); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T101 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T101 = (float4)(base0[(int)coordR.x], .0f, .0f, 1.0f); \n" \
"            } \n" \
"        } \n" \
"        if (((int)coordR.y >= (int)imageSize.y) || ((int)coordR.y < 0)) \n" \
"        { \n" \
"            T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            float * base1 = (float *) ((uchar *)image.x + image.y * (uint)coordR.y + image.z * (uint)coordR.z); \n" \
"            if (((int)coordQ.x >= (int)imageSize.x) || ((int)coordQ.x < 0)) \n" \
"            { \n" \
"                T011 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T011 = (float4)(base1[(int)coordQ.x], .0f, .0f, 1.0f); \n" \
"            } \n" \
"            if (((int)coordR.x >= (int)imageSize.x) || ((int)coordR.x < 0)) \n" \
"            { \n" \
"                T111 = (float4)(.0f, .0f, .0f, 1.0f); \n" \
"            } \n" \
"            else \n" \
"            { \n" \
"                T111 = (float4)(base1[(int)coordR.x], .0f, .0f, 1.0f);\n" \
"            } \n" \
"        } \n" \
"    } \n"

#define LINEAR_FILTER_1d \
"    float oneMinusFractUV = 1 - fractUV; \n" \
"    return oneMinusFractUV * T00 + fractUV * T11;\n" \
"} \n" \
"\n"
#define LINEAR_FILTER_1d_BGRA \
"    float oneMinusFractUV = 1 - fractUV; \n" \
"    float4 pixel = oneMinusFractUV * T00 + fractUV * T11; \n" \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value;\n" \
"} \n" \
"\n"

#define LINEAR_FILTER_1d_R LINEAR_FILTER_1d

#define LINEAR_FILTER_1dbuffer LINEAR_FILTER_1d
#define LINEAR_FILTER_1dbuffer_BGRA LINEAR_FILTER_1d_BGRA

#define LINEAR_FILTER_2d \
"    float2 oneMinusFractUV = 1 - fractUV; \n" \
"    return oneMinusFractUV.x * oneMinusFractUV.y * T00 + \n" \
"                   fractUV.x * oneMinusFractUV.y * T10 + \n" \
"           oneMinusFractUV.x *         fractUV.y * T01 + \n" \
"                   fractUV.x *         fractUV.y * T11; \n" \
"} \n" \
"\n"

#define LINEAR_FILTER_2d_BGRA \
"    float2 oneMinusFractUV = 1 - fractUV; \n" \
"    float4 pixel = oneMinusFractUV.y * oneMinusFractUV.x * T00 + oneMinusFractUV.y * fractUV.x * T10 + \n" \
"                   fractUV.y * oneMinusFractUV.x * T01 + fractUV.y * fractUV.x * T11; \n" \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value;\n" \
"} \n" \
"\n"

#define LINEAR_FILTER_2d_R LINEAR_FILTER_2d


#define LINEAR_FILTER_1darray \
"    float oneMinusFractUV = 1 - fractUV; \n" \
"    return oneMinusFractUV * T00 + fractUV * T11;\n" \
"} \n" \
"\n"

#define LINEAR_FILTER_1darray_BGRA \
"    float oneMinusFractUV = 1 - fractUV; \n" \
"    float4 pixel = oneMinusFractUV * T00 + fractUV * T11; \n" \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value;\n" \
"} \n" \
"\n"

#define LINEAR_FILTER_1darray_R LINEAR_FILTER_1darray
#define LINEAR_FILTER_1dbuffer_R LINEAR_FILTER_1darray

#define LINEAR_FILTER_2DARRAY LINEAR_FILTER_2d
#define LINEAR_FILTER_2DARRAY_BGRA LINEAR_FILTER_2d_BGRA
#define LINEAR_FILTER_2DARRAY_R LINEAR_FILTER_2d_R


#define LINEAR_FILTER_3d \
"    float3 oneMinusFractUVW = 1 - fractUVW; \n" \
"    float4 t0, t1, t2, t3, t4, t5, t6;\n" \
"    t0 = oneMinusFractUVW.x * T000 + fractUVW.x * T100; \n" \
"    t1 = oneMinusFractUVW.x * T010 + fractUVW.x * T110; \n" \
"    t2 = oneMinusFractUVW.x * T001 + fractUVW.x * T101; \n" \
"    t3 = oneMinusFractUVW.x * T011 + fractUVW.x * T111; \n" \
"    t4 = oneMinusFractUVW.y * t0 + fractUVW.y * t1; \n" \
"    t5 = oneMinusFractUVW.y * t2 + fractUVW.y * t3; \n" \
"    t6 = oneMinusFractUVW.z * t4 + fractUVW.z * t5; \n" \
"    return t6; \n" \
"} \n" \
"\n"
#define LINEAR_FILTER_3d_BGRA \
"    float3 oneMinusFractUVW = 1 - fractUVW; \n" \
"    float4 t0, t1, t2, t3, t4, t5, t6;\n" \
"    t0 = oneMinusFractUVW.x * T000 + fractUVW.x * T100; \n" \
"    t1 = oneMinusFractUVW.x * T010 + fractUVW.x * T110; \n" \
"    t2 = oneMinusFractUVW.x * T001 + fractUVW.x * T101; \n" \
"    t3 = oneMinusFractUVW.x * T011 + fractUVW.x * T111; \n" \
"    t4 = oneMinusFractUVW.y * t0 + fractUVW.y * t1; \n" \
"    t5 = oneMinusFractUVW.y * t2 + fractUVW.y * t3; \n" \
"    t6 = oneMinusFractUVW.z * t4 + fractUVW.z * t5; \n" \
"    float4 pixel = t6; \n" \
"    float4 value = pixel; \n" \
"    value.x = pixel.z; \n" \
"    value.z = pixel.x; \n" \
"    return value;\n" \
"} \n" \
"\n"
#define LINEAR_FILTER_3d_R LINEAR_FILTER_3d

#define READFUNC(IMAGETYPE, ORDER) \
"uint4 \n" \
"_read_image_nearest_unnorm_none_intcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_clamp_intcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_border_intcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(uint4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_none_intcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_clamp_intcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_border_intcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(uint4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_none_intcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_unnorm_clamp_intcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_unnorm_border_intcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(uint4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_unnorm_none_floatcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_clamp_floatcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_border_floatcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(uint4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_none_floatcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_clamp_floatcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_border_floatcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(uint4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_unnorm_none_floatcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_unnorm_clamp_floatcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_unnorm_border_floatcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(uint4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_norm_none_floatcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_clamp_floatcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_border_floatcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(uint4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_wrap_floatcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    WRAP_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_mirror_floatcoord_ui_uint8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    MIRROR_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uchar4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_none_floatcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_clamp_floatcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_border_floatcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(uint4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_wrap_floatcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    WRAP_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_mirror_floatcoord_ui_uint16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    MIRROR_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(ushort4, convert_uint4) \
"uint4 \n" \
"_read_image_nearest_norm_none_floatcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_norm_clamp_floatcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_norm_border_floatcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(uint4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_norm_wrap_floatcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    WRAP_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"uint4 \n" \
"_read_image_nearest_norm_mirror_floatcoord_ui_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    MIRROR_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(uint4, ) \
"int4 \n" \
"_read_image_nearest_unnorm_none_intcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_clamp_intcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_border_intcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(int4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_none_intcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_clamp_intcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_border_intcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(int4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_none_intcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_unnorm_clamp_intcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_unnorm_border_intcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(int4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_unnorm_none_floatcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_clamp_floatcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_border_floatcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(int4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_none_floatcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_clamp_floatcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_border_floatcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(int4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_unnorm_none_floatcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_unnorm_clamp_floatcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_unnorm_border_floatcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(int4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_norm_none_floatcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_clamp_floatcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_border_floatcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(int4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_wrap_floatcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    WRAP_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_mirror_floatcoord_i_int8_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    MIRROR_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(char4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_none_floatcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_clamp_floatcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_border_floatcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(int4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_wrap_floatcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    WRAP_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_mirror_floatcoord_i_int16_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    MIRROR_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(short4, convert_int4) \
"int4 \n" \
"_read_image_nearest_norm_none_floatcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_norm_clamp_floatcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_norm_border_floatcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(int4) \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_norm_wrap_floatcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    WRAP_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_norm_mirror_floatcoord_i_int32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    MIRROR_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, ) \
"int4 \n" \
"_read_image_nearest_unnorm_none_intcoord_i_uint32_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_TEXEL_##IMAGETYPE##ORDER(int4, )

extern gctSTRING gcLibCLImage_ReadFunc_1D;
extern gctSTRING gcLibCLImage_ReadFunc_1D_BGRA;
extern gctSTRING gcLibCLImage_ReadFunc_1D_R;
extern gctSTRING gcLibCLImage_ReadFunc_1DBUFFER ;
extern gctSTRING gcLibCLImage_ReadFunc_1DBUFFER_BGRA;
extern gctSTRING gcLibCLImage_ReadFunc_1DBUFFER_R;
extern gctSTRING gcLibCLImage_ReadFunc_2D;
extern gctSTRING gcLibCLImage_ReadFunc_2D_BGRA ;
extern gctSTRING gcLibCLImage_ReadFunc_2D_R;
extern gctSTRING gcLibCLImage_ReadFunc_1DARRAY;
extern gctSTRING gcLibCLImage_ReadFunc_1DARRAY_BGRA;
extern gctSTRING gcLibCLImage_ReadFunc_1DARRAY_R;
extern gctSTRING gcLibCLImage_ReadFunc_2DARRAY;
extern gctSTRING gcLibCLImage_ReadFunc_2DARRAY_BGRA;
extern gctSTRING gcLibCLImage_ReadFunc_2DARRAY_R;
extern gctSTRING gcLibCLImage_ReadFunc_3D;
extern gctSTRING gcLibCLImage_ReadFunc_3D_BGRA;
extern gctSTRING gcLibCLImage_ReadFunc_3D_R;

#define IMGLOAD_FLOAT_TEXEL_2d \
"    float4 result;\n" \
"    _viv_asm(IMAGE_READ, result, image, coord);\n"\
"    return result;\n"\
"}\n"

#define IMGLOAD_4_FLOAT_TEXELS_2d \
"    float4 T00;\n" \
"    float4 T10;\n" \
"    float4 T01;\n" \
"    float4 T11;\n" \
"    _viv_asm(IMAGE_READ, T00, image, coordQ.xy);\n"\
"    _viv_asm(IMAGE_READ, T10, image, coordQ.zy);\n"\
"    _viv_asm(IMAGE_READ, T01, image, coordQ.xw);\n"\
"    _viv_asm(IMAGE_READ, T11, image, coordQ.zw);\n"\

#define read_image_IMG_ARGS_FLOAT_COORD_2d \
"    (\n" \
"    uint4 image, \n" \
"    int2 imageSize, \n" \
"    float2 fcoord \n" \
"    ) \n" \
"{ \n"

#define READ_IMAGEF_CHANNEL_TYPE_IMGLD(SUFFIX, IMAGETYPE, ORDER) \
"float4 \n" \
"_read_image_nearest_norm_none_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER"_imageLd \n" \
    read_image_IMG_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    IMGLOAD_FLOAT_TEXEL_##IMAGETYPE \
"float4 \n" \
"_read_image_nearest_norm_clamp_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER"_imageLd \n" \
    read_image_IMG_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    IMGLOAD_FLOAT_TEXEL_##IMAGETYPE \
"float4 \n" \
"_read_image_nearest_norm_border_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER"_imageLd \n" \
    read_image_IMG_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    IMGLOAD_FLOAT_TEXEL_##IMAGETYPE \
"float4 \n" \
"_read_image_linear_norm_none_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER"_imageLd \n" \
    read_image_IMG_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    GET_I0J0I1J1_SIMPLE_##IMAGETYPE \
    IMGLOAD_4_FLOAT_TEXELS_##IMAGETYPE \
    LINEAR_FILTER_##IMAGETYPE \
"float4 \n" \
"_read_image_linear_norm_clamp_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER"_imageLd \n" \
    read_image_IMG_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    GET_I0J0I1J1_SIMPLE_##IMAGETYPE \
    IMGLOAD_4_FLOAT_TEXELS_##IMAGETYPE \
    LINEAR_FILTER_##IMAGETYPE \
"float4 \n" \
"_read_image_linear_norm_border_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER"_imageLd \n" \
    read_image_IMG_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    GET_I0J0I1J1_SIMPLE_##IMAGETYPE \
    IMGLOAD_4_FLOAT_TEXELS_##IMAGETYPE \
    LINEAR_FILTER_##IMAGETYPE

#define READ_IMAGEF_CHANNEL_TYPE(SUFFIX, LOAD_TYPE, CHANNEL_TYPE, BASE, IMAGETYPE, ORDER) \
"float4 \n" \
"_read_image_nearest_unnorm_none_intcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_unnorm_clamp_intcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_unnorm_border_intcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(float4) \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_unnorm_none_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_unnorm_clamp_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_unnorm_border_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(float4) \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_norm_none_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_NONE_##IMAGETYPE \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_norm_clamp_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_##IMAGETYPE \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_norm_border_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CHECK_BORDER_##IMAGETYPE##ORDER(float4) \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_norm_wrap_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    WRAP_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_nearest_norm_mirror_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    MIRROR_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    CONVERT_FLOAT_COORD_TO_INT_COORD_##IMAGETYPE \
    CLAMP_COORD_UPPER_##IMAGETYPE \
    LOAD_##LOAD_TYPE##_TEXEL_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
"float4 \n" \
"_read_image_linear_unnorm_none_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    GET_I0J0I1J1_SIMPLE_##IMAGETYPE \
    LOAD_4_##LOAD_TYPE##_TEXELS_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
    LINEAR_FILTER_##IMAGETYPE##ORDER \
"float4 \n" \
"_read_image_linear_unnorm_clamp_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    GET_I0J0I1J1_SIMPLE_##IMAGETYPE \
    CLAMP_I0J0I1J1_##IMAGETYPE \
    LOAD_4_##LOAD_TYPE##_TEXELS_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
    LINEAR_FILTER_##IMAGETYPE##ORDER \
"float4 \n" \
"_read_image_linear_unnorm_border_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    GET_I0J0I1J1_SIMPLE_##IMAGETYPE \
    LOAD_4_##LOAD_TYPE##_TEXELS_BORDER_CHECK_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
    LINEAR_FILTER_##IMAGETYPE##ORDER \
"float4 \n" \
"_read_image_linear_norm_none_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    GET_I0J0I1J1_SIMPLE_##IMAGETYPE \
    LOAD_4_##LOAD_TYPE##_TEXELS_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
    LINEAR_FILTER_##IMAGETYPE##ORDER \
"float4 \n" \
"_read_image_linear_norm_clamp_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    GET_I0J0I1J1_SIMPLE_##IMAGETYPE \
    CLAMP_I0J0I1J1_##IMAGETYPE \
    LOAD_4_##LOAD_TYPE##_TEXELS_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
    LINEAR_FILTER_##IMAGETYPE##ORDER \
"float4 \n" \
"_read_image_linear_norm_border_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    CONVERT_NORMALIZED_COORD_TO_UNNORMALIZED_COORD_##IMAGETYPE \
    GET_I0J0I1J1_SIMPLE_##IMAGETYPE \
    LOAD_4_##LOAD_TYPE##_TEXELS_BORDER_CHECK_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
    LINEAR_FILTER_##IMAGETYPE##ORDER \
"float4 \n" \
"_read_image_linear_norm_wrap_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    GET_I0J0I1J1_WRAP_##IMAGETYPE \
    LOAD_4_##LOAD_TYPE##_TEXELS_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
    LINEAR_FILTER_##IMAGETYPE##ORDER \
"float4 \n" \
"_read_image_linear_norm_mirror_floatcoord_f_"#SUFFIX"_"#IMAGETYPE#ORDER" \n" \
    read_image_ARGS_FLOAT_COORD_##IMAGETYPE \
    GET_I0J0I1J1_MIRROR_##IMAGETYPE \
    LOAD_4_##LOAD_TYPE##_TEXELS_##IMAGETYPE##ORDER(CHANNEL_TYPE, BASE) \
    LINEAR_FILTER_##IMAGETYPE##ORDER


extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1D;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1D_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1D_R;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1DBUFFER;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1DBUFFER_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1DBUFFER_R;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2D;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2D_BGRA1;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2D_BGRA2;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2D_R1;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2D_R2;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY1;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY2;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY1_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY2_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY1_R;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_1DARRAY2_R;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY1;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY2;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY1_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY2_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY1_R;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_2DARRAY2_R;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_3D0;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_3D1;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_3D0_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_3D1_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_3D0_R;
extern gctSTRING gcLibCLImage_ReadFuncF_NORM_3D1_R;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1D;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1D_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1D_R;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DBUFFER;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DBUFFER_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DBUFFER_R;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2D;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2D_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2D_R;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DARRAY;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DARRAY_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_1DARRAY_R;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2DARRAY;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2DARRAY_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_2DARRAY_R;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_3D;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_3D_BGRA;
extern gctSTRING gcLibCLImage_ReadFuncF_UNNORM_3D_R;

#define STORE_TEXEL_1d(CHANNEL_TYPE) \
"    "#CHANNEL_TYPE" * base = ("#CHANNEL_TYPE" *) ((uchar *)image); \n" \
"    base[coord] = convert_"#CHANNEL_TYPE"_sat(color); \n"

#define STORE_TEXEL_1darray(CHANNEL_TYPE) \
"    "#CHANNEL_TYPE" * base = ("#CHANNEL_TYPE" *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color); \n"

#define STORE_TEXEL_2d(CHANNEL_TYPE) \
"    "#CHANNEL_TYPE" * base = ("#CHANNEL_TYPE" *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color); \n"

#define STORE_TEXEL_2DARRAY STORE_TEXEL_3d

#define STORE_TEXEL_3d(CHANNEL_TYPE) \
"    "#CHANNEL_TYPE" * base = ("#CHANNEL_TYPE" *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color); \n"

#define STORE_TEXELF_1d(CHANNEL_TYPE, BASE) \
"    "#CHANNEL_TYPE" * base = ("#CHANNEL_TYPE" *) ((uchar *)image); \n" \
"    base[coord] = convert_"#CHANNEL_TYPE"_sat(color * "#BASE"f + 0.5f); \n"

#define STORE_TEXELF_1darray(CHANNEL_TYPE, BASE) \
"    "#CHANNEL_TYPE" * base = ("#CHANNEL_TYPE" *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color * "#BASE"f + 0.5f); \n"

#define STORE_TEXELF_2d(CHANNEL_TYPE, BASE) \
"    "#CHANNEL_TYPE" * base = ("#CHANNEL_TYPE" *) ((uchar *)image.x + image.y * (uint)coord.y); \n" \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color * "#BASE"f + 0.5f); \n"

#define STORE_TEXELF_2DARRAY STORE_TEXELF_3d

#define STORE_TEXELF_3d(CHANNEL_TYPE, BASE) \
"    "#CHANNEL_TYPE" * base = ("#CHANNEL_TYPE" *) ((uchar *)image.x + image.y * (uint)coord.y + image.z * (uint)coord.z); \n" \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color * "#BASE"f + 0.5f); \n"

#define CONSTRUCT_TEXEL_1d_1CHANNEL_STORE(CHANNEL_TYPE) \
"    base[coord] = convert_"#CHANNEL_TYPE"_sat(color.x); \n"
#define STORE_TEXEL_1d_R_uchar4(CHANNEL_TYPE) \
    GET_BASE_1d(uchar) \
    CONSTRUCT_TEXEL_1d_1CHANNEL_STORE(uchar)
#define STORE_TEXEL_1d_R_char4(CHANNEL_TYPE) \
    GET_BASE_1d(char) \
    CONSTRUCT_TEXEL_1d_1CHANNEL_STORE(char)
#define STORE_TEXEL_1d_R_ushort4(CHANNEL_TYPE) \
    GET_BASE_1d(ushort) \
    CONSTRUCT_TEXEL_1d_1CHANNEL_STORE(ushort)
#define STORE_TEXEL_1d_R_short4(CHANNEL_TYPE) \
    GET_BASE_1d(short) \
    CONSTRUCT_TEXEL_1d_1CHANNEL_STORE(short)
#define STORE_TEXEL_1d_R_uint4(CHANNEL_TYPE) \
    GET_BASE_1d(uint) \
    CONSTRUCT_TEXEL_1d_1CHANNEL_STORE(uint)
#define STORE_TEXEL_1d_R_int4(CHANNEL_TYPE) \
    GET_BASE_1d(int) \
    CONSTRUCT_TEXEL_1d_1CHANNEL_STORE(int)
#define STORE_TEXEL_1d_R_float4(CHANNEL_TYPE) \
    GET_BASE_1d(float) \
    CONSTRUCT_TEXEL_1d_1CHANNEL_STORE(float)
#define STORE_TEXEL_1d_R_half(CHANNEL_TYPE) \
    GET_BASE_1d(half) \
"    vstore_half((float)(color.x), coord,base); \n"

#define STORE_TEXEL_1CHANNEL_1d(CHANNEL_TYPE) \
    STORE_TEXEL_1d_R_##CHANNEL_TYPE(CHANNEL_TYPE)

#define CONSTRUCT_TEXEL_1darray_1CHANNEL_STORE(CHANNEL_TYPE) \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color.x); \n"
#define STORE_TEXEL_1darray_R_uchar4(CHANNEL_TYPE) \
    GET_BASE_1darray(uchar) \
    CONSTRUCT_TEXEL_1darray_1CHANNEL_STORE(uchar)
#define STORE_TEXEL_1darray_R_char4(CHANNEL_TYPE) \
    GET_BASE_1darray(char) \
    CONSTRUCT_TEXEL_1darray_1CHANNEL_STORE(char)
#define STORE_TEXEL_1darray_R_ushort4(CHANNEL_TYPE) \
    GET_BASE_1darray(ushort) \
    CONSTRUCT_TEXEL_1darray_1CHANNEL_STORE(ushort)
#define STORE_TEXEL_1darray_R_short4(CHANNEL_TYPE) \
    GET_BASE_1darray(short) \
    CONSTRUCT_TEXEL_1darray_1CHANNEL_STORE(short)
#define STORE_TEXEL_1darray_R_uint4(CHANNEL_TYPE) \
    GET_BASE_1darray(uint) \
    CONSTRUCT_TEXEL_1darray_1CHANNEL_STORE(uint)
#define STORE_TEXEL_1darray_R_int4(CHANNEL_TYPE) \
    GET_BASE_1darray(int) \
    CONSTRUCT_TEXEL_1darray_1CHANNEL_STORE(int)
#define STORE_TEXEL_1darray_R_float4(CHANNEL_TYPE) \
    GET_BASE_1darray(float) \
    CONSTRUCT_TEXEL_1darray_1CHANNEL_STORE(float)
#define STORE_TEXEL_1darray_R_half(CHANNEL_TYPE) \
    GET_BASE_1darray(half) \
"    vstore_half((float)(color.x), coord.x,base); \n"

#define STORE_TEXEL_1CHANNEL_1darray(CHANNEL_TYPE) \
    STORE_TEXEL_1darray_R_##CHANNEL_TYPE(CHANNEL_TYPE)

#define CONSTRUCT_TEXEL_2d_1CHANNEL_STORE(CHANNEL_TYPE) \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color.x); \n"
#define STORE_TEXEL_2d_R_uchar4(CHANNEL_TYPE) \
    GET_BASE_2d(uchar) \
    CONSTRUCT_TEXEL_2d_1CHANNEL_STORE(uchar)
#define STORE_TEXEL_2d_R_char4(CHANNEL_TYPE) \
    GET_BASE_2d(char) \
    CONSTRUCT_TEXEL_2d_1CHANNEL_STORE(char)
#define STORE_TEXEL_2d_R_ushort4(CHANNEL_TYPE) \
    GET_BASE_2d(ushort) \
    CONSTRUCT_TEXEL_2d_1CHANNEL_STORE(ushort)
#define STORE_TEXEL_2d_R_short4(CHANNEL_TYPE) \
    GET_BASE_2d(short) \
    CONSTRUCT_TEXEL_2d_1CHANNEL_STORE(short)
#define STORE_TEXEL_2d_R_uint4(CHANNEL_TYPE) \
    GET_BASE_2d(uint) \
    CONSTRUCT_TEXEL_2d_1CHANNEL_STORE(uint)
#define STORE_TEXEL_2d_R_int4(CHANNEL_TYPE) \
    GET_BASE_2d(int) \
    CONSTRUCT_TEXEL_2d_1CHANNEL_STORE(int)
#define STORE_TEXEL_2d_R_float4(CHANNEL_TYPE) \
    GET_BASE_2d(float) \
    CONSTRUCT_TEXEL_2d_1CHANNEL_STORE(float)
#define STORE_TEXEL_2d_R_half(CHANNEL_TYPE) \
    GET_BASE_2d(half) \
"    vstore_half((float)(color.x), coord.x,base); \n"

#define STORE_TEXEL_1CHANNEL_2d(CHANNEL_TYPE) \
    STORE_TEXEL_2d_R_##CHANNEL_TYPE(CHANNEL_TYPE)

#define CONSTRUCT_TEXEL_2DARRAY_1CHANNEL_STORE(CHANNEL_TYPE) \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color.x); \n"
#define STORE_TEXEL_2DARRAY_R_uchar4(CHANNEL_TYPE) \
    GET_BASE_2DARRAY(uchar) \
    CONSTRUCT_TEXEL_2DARRAY_1CHANNEL_STORE(uchar)
#define STORE_TEXEL_2DARRAY_R_char4(CHANNEL_TYPE) \
    GET_BASE_2DARRAY(char) \
    CONSTRUCT_TEXEL_2DARRAY_1CHANNEL_STORE(char)
#define STORE_TEXEL_2DARRAY_R_ushort4(CHANNEL_TYPE) \
    GET_BASE_2DARRAY(ushort) \
    CONSTRUCT_TEXEL_2DARRAY_1CHANNEL_STORE(ushort)
#define STORE_TEXEL_2DARRAY_R_short4(CHANNEL_TYPE) \
    GET_BASE_2DARRAY(short) \
    CONSTRUCT_TEXEL_2DARRAY_1CHANNEL_STORE(short)
#define STORE_TEXEL_2DARRAY_R_uint4(CHANNEL_TYPE) \
    GET_BASE_2DARRAY(uint) \
    CONSTRUCT_TEXEL_2DARRAY_1CHANNEL_STORE(uint)
#define STORE_TEXEL_2DARRAY_R_int4(CHANNEL_TYPE) \
    GET_BASE_2DARRAY(int) \
    CONSTRUCT_TEXEL_2DARRAY_1CHANNEL_STORE(int)
#define STORE_TEXEL_2DARRAY_R_float4(CHANNEL_TYPE) \
    GET_BASE_2DARRAY(float) \
    CONSTRUCT_TEXEL_2DARRAY_1CHANNEL_STORE(float)
#define STORE_TEXEL_2DARRAY_R_half(CHANNEL_TYPE) \
    GET_BASE_2DARRAY(half) \
"    vstore_half((float)(color.x), coord.x,base); \n"

#define STORE_TEXEL_1CHANNEL_2DARRAY(CHANNEL_TYPE) \
    STORE_TEXEL_2DARRAY_R_##CHANNEL_TYPE(CHANNEL_TYPE)

#define CONSTRUCT_TEXEL_3d_1CHANNEL_STORE(CHANNEL_TYPE) \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color.x); \n"
#define STORE_TEXEL_3d_R_uchar4(CHANNEL_TYPE) \
    GET_BASE_3d(uchar) \
    CONSTRUCT_TEXEL_3d_1CHANNEL_STORE(uchar)
#define STORE_TEXEL_3d_R_char4(CHANNEL_TYPE) \
    GET_BASE_3d(char) \
    CONSTRUCT_TEXEL_3d_1CHANNEL_STORE(char)
#define STORE_TEXEL_3d_R_ushort4(CHANNEL_TYPE) \
    GET_BASE_3d(ushort) \
    CONSTRUCT_TEXEL_3d_1CHANNEL_STORE(ushort)
#define STORE_TEXEL_3d_R_short4(CHANNEL_TYPE) \
    GET_BASE_3d(short) \
    CONSTRUCT_TEXEL_3d_1CHANNEL_STORE(short)
#define STORE_TEXEL_3d_R_uint4(CHANNEL_TYPE) \
    GET_BASE_3d(uint) \
    CONSTRUCT_TEXEL_3d_1CHANNEL_STORE(uint)
#define STORE_TEXEL_3d_R_int4(CHANNEL_TYPE) \
    GET_BASE_3d(int) \
    CONSTRUCT_TEXEL_3d_1CHANNEL_STORE(int)
#define STORE_TEXEL_3d_R_float4(CHANNEL_TYPE) \
    GET_BASE_3d(float) \
    CONSTRUCT_TEXEL_3d_1CHANNEL_STORE(float)
#define STORE_TEXEL_3d_R_half(CHANNEL_TYPE) \
    GET_BASE_3d(half) \
"    vstore_half((float)(color.x), coord.x,base); \n"

#define STORE_TEXEL_1CHANNEL_3d(CHANNEL_TYPE) \
    STORE_TEXEL_3d_R_##CHANNEL_TYPE(CHANNEL_TYPE)

#define CONSTRUCT_TEXELF_1d_1CHANNEL_STORE(CHANNEL_TYPE, BASE) \
"    base[coord] = convert_"#CHANNEL_TYPE"_sat(color.x * "#BASE"f + 0.5f); \n"
#define STORE_TEXELF_1d_R_uchar4(CHANNEL_TYPE, BASE) \
    GET_BASE_1d(uchar) \
    CONSTRUCT_TEXELF_1d_1CHANNEL_STORE(uchar, BASE)
#define STORE_TEXELF_1d_R_char4(CHANNEL_TYPE, BASE) \
    GET_BASE_1d(char) \
    CONSTRUCT_TEXELF_1d_1CHANNEL_STORE(char, BASE)
#define STORE_TEXELF_1d_R_ushort4(CHANNEL_TYPE, BASE) \
    GET_BASE_1d(ushort) \
    CONSTRUCT_TEXELF_1d_1CHANNEL_STORE(ushort, BASE)
#define STORE_TEXELF_1d_R_short4(CHANNEL_TYPE, BASE) \
    GET_BASE_1d(short) \
    CONSTRUCT_TEXELF_1d_1CHANNEL_STORE(short, BASE)
#define STORE_TEXELF_1d_R_uint4(CHANNEL_TYPE, BASE) \
    GET_BASE_1d(uint) \
    CONSTRUCT_TEXELF_1d_1CHANNEL_STORE(uint, BASE)
#define STORE_TEXELF_1d_R_int4(CHANNEL_TYPE, BASE) \
    GET_BASE_1d(int) \
    CONSTRUCT_TEXELF_1d_1CHANNEL_STORE(int, BASE)
#define STORE_TEXELF_1d_R_float4(CHANNEL_TYPE, BASE) \
    GET_BASE_1d(float) \
    CONSTRUCT_TEXELF_1d_1CHANNEL_STORE(float, BASE)
#define STORE_TEXELF_1d_R_half(CHANNEL_TYPE, BASE) \
    GET_BASE_1d(half) \
"    vstore_half((color.x * "#BASE"f + 0.5f), coord,base); \n"

#define STORE_TEXELF_1CHANNEL_1d(CHANNEL_TYPE, BASE) \
    STORE_TEXELF_1d_R_##CHANNEL_TYPE(CHANNEL_TYPE, BASE)

#define CONSTRUCT_TEXELF_1darray_1CHANNEL_STORE(CHANNEL_TYPE, BASE) \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color.x * "#BASE"f + 0.5f); \n"
#define STORE_TEXELF_1darray_R_uchar4(CHANNEL_TYPE, BASE) \
    GET_BASE_1darray(uchar) \
    CONSTRUCT_TEXELF_1darray_1CHANNEL_STORE(uchar, BASE)
#define STORE_TEXELF_1darray_R_char4(CHANNEL_TYPE, BASE) \
    GET_BASE_1darray(char) \
    CONSTRUCT_TEXELF_1darray_1CHANNEL_STORE(char, BASE)
#define STORE_TEXELF_1darray_R_ushort4(CHANNEL_TYPE, BASE) \
    GET_BASE_1darray(ushort) \
    CONSTRUCT_TEXELF_1darray_1CHANNEL_STORE(ushort, BASE)
#define STORE_TEXELF_1darray_R_short4(CHANNEL_TYPE, BASE) \
    GET_BASE_1darray(short) \
    CONSTRUCT_TEXELF_1darray_1CHANNEL_STORE(short, BASE)
#define STORE_TEXELF_1darray_R_uint4(CHANNEL_TYPE, BASE) \
    GET_BASE_1darray(uint) \
    CONSTRUCT_TEXELF_1darray_1CHANNEL_STORE(uint, BASE)
#define STORE_TEXELF_1darray_R_int4(CHANNEL_TYPE, BASE) \
    GET_BASE_1darray(int) \
    CONSTRUCT_TEXELF_1darray_1CHANNEL_STORE(int, BASE)
#define STORE_TEXELF_1darray_R_float4(CHANNEL_TYPE, BASE) \
    GET_BASE_1darray(float) \
    CONSTRUCT_TEXELF_1darray_1CHANNEL_STORE(float, BASE)
#define STORE_TEXELF_1darray_R_half(CHANNEL_TYPE, BASE) \
    GET_BASE_1darray(half) \
"    vstore_half((color.x * "#BASE"f + 0.5f), coord.x,base); \n"

#define STORE_TEXELF_1CHANNEL_1darray(CHANNEL_TYPE, BASE) \
    STORE_TEXELF_1darray_R_##CHANNEL_TYPE(CHANNEL_TYPE, BASE)

#define CONSTRUCT_TEXELF_2d_1CHANNEL_STORE(CHANNEL_TYPE, BASE) \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color.x * "#BASE"f + 0.5f); \n"
#define STORE_TEXELF_2d_R_uchar4(CHANNEL_TYPE, BASE) \
    GET_BASE_2d(uchar) \
    CONSTRUCT_TEXELF_2d_1CHANNEL_STORE(uchar, BASE)
#define STORE_TEXELF_2d_R_char4(CHANNEL_TYPE, BASE) \
    GET_BASE_2d(char) \
    CONSTRUCT_TEXELF_2d_1CHANNEL_STORE(char, BASE)
#define STORE_TEXELF_2d_R_ushort4(CHANNEL_TYPE, BASE) \
    GET_BASE_2d(ushort) \
    CONSTRUCT_TEXELF_2d_1CHANNEL_STORE(ushort, BASE)
#define STORE_TEXELF_2d_R_short4(CHANNEL_TYPE, BASE) \
    GET_BASE_2d(short) \
    CONSTRUCT_TEXELF_2d_1CHANNEL_STORE(short, BASE)
#define STORE_TEXELF_2d_R_uint4(CHANNEL_TYPE, BASE) \
    GET_BASE_2d(uint) \
    CONSTRUCT_TEXELF_2d_1CHANNEL_STORE(uint, BASE)
#define STORE_TEXELF_2d_R_int4(CHANNEL_TYPE, BASE) \
    GET_BASE_2d(int) \
    CONSTRUCT_TEXELF_2d_1CHANNEL_STORE(int, BASE)
#define STORE_TEXELF_2d_R_float4(CHANNEL_TYPE, BASE) \
    GET_BASE_2d(float) \
    CONSTRUCT_TEXELF_2d_1CHANNEL_STORE(float, BASE)
#define STORE_TEXELF_2d_R_half(CHANNEL_TYPE, BASE) \
    GET_BASE_2d(half) \
"    vstore_half((color.x * "#BASE"f + 0.5f), coord.x,base); \n"

#define STORE_TEXELF_1CHANNEL_2d(CHANNEL_TYPE, BASE) \
    STORE_TEXELF_2d_R_##CHANNEL_TYPE(CHANNEL_TYPE, BASE)

#define CONSTRUCT_TEXELF_2DARRAY_1CHANNEL_STORE(CHANNEL_TYPE, BASE) \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color.x * "#BASE"f + 0.5f); \n"
#define STORE_TEXELF_2DARRAY_R_uchar4(CHANNEL_TYPE, BASE) \
    GET_BASE_2DARRAY(uchar) \
    CONSTRUCT_TEXELF_2DARRAY_1CHANNEL_STORE(uchar, BASE)
#define STORE_TEXELF_2DARRAY_R_char4(CHANNEL_TYPE, BASE) \
    GET_BASE_2DARRAY(char) \
    CONSTRUCT_TEXELF_2DARRAY_1CHANNEL_STORE(char, BASE)
#define STORE_TEXELF_2DARRAY_R_ushort4(CHANNEL_TYPE, BASE) \
    GET_BASE_2DARRAY(ushort) \
    CONSTRUCT_TEXELF_2DARRAY_1CHANNEL_STORE(ushort, BASE)
#define STORE_TEXELF_2DARRAY_R_short4(CHANNEL_TYPE, BASE) \
    GET_BASE_2DARRAY(short) \
    CONSTRUCT_TEXELF_2DARRAY_1CHANNEL_STORE(short, BASE)
#define STORE_TEXELF_2DARRAY_R_uint4(CHANNEL_TYPE, BASE) \
    GET_BASE_2DARRAY(uint) \
    CONSTRUCT_TEXELF_2DARRAY_1CHANNEL_STORE(uint, BASE)
#define STORE_TEXELF_2DARRAY_R_int4(CHANNEL_TYPE, BASE) \
    GET_BASE_2DARRAY(int) \
    CONSTRUCT_TEXELF_2DARRAY_1CHANNEL_STORE(int, BASE)
#define STORE_TEXELF_2DARRAY_R_float4(CHANNEL_TYPE, BASE) \
    GET_BASE_2DARRAY(float) \
    CONSTRUCT_TEXELF_2DARRAY_1CHANNEL_STORE(float, BASE)
#define STORE_TEXELF_2DARRAY_R_half(CHANNEL_TYPE, BASE) \
    GET_BASE_2DARRAY(half) \
"    vstore_half((color.x * "#BASE"f + 0.5f), coord.x,base); \n"

#define STORE_TEXELF_1CHANNEL_2DARRAY(CHANNEL_TYPE, BASE) \
    STORE_TEXELF_2DARRAY_R_##CHANNEL_TYPE(CHANNEL_TYPE, BASE)


#define CONSTRUCT_TEXELF_3d_1CHANNEL_STORE(CHANNEL_TYPE, BASE) \
"    base[coord.x] = convert_"#CHANNEL_TYPE"_sat(color.x * "#BASE"f + 0.5f); \n"
#define STORE_TEXELF_3d_R_uchar4(CHANNEL_TYPE, BASE) \
    GET_BASE_3d(uchar) \
    CONSTRUCT_TEXELF_3d_1CHANNEL_STORE(uchar, BASE)
#define STORE_TEXELF_3d_R_char4(CHANNEL_TYPE, BASE) \
    GET_BASE_3d(char) \
    CONSTRUCT_TEXELF_3d_1CHANNEL_STORE(char, BASE)
#define STORE_TEXELF_3d_R_ushort4(CHANNEL_TYPE, BASE) \
    GET_BASE_3d(ushort) \
    CONSTRUCT_TEXELF_3d_1CHANNEL_STORE(ushort, BASE)
#define STORE_TEXELF_3d_R_short4(CHANNEL_TYPE, BASE) \
    GET_BASE_3d(short) \
    CONSTRUCT_TEXELF_3d_1CHANNEL_STORE(short, BASE)
#define STORE_TEXELF_3d_R_uint4(CHANNEL_TYPE, BASE) \
    GET_BASE_3d(uint) \
    CONSTRUCT_TEXELF_3d_1CHANNEL_STORE(uint, BASE)
#define STORE_TEXELF_3d_R_int4(CHANNEL_TYPE, BASE) \
    GET_BASE_3d(int) \
    CONSTRUCT_TEXELF_3d_1CHANNEL_STORE(int, BASE)
#define STORE_TEXELF_3d_R_float4(CHANNEL_TYPE, BASE) \
    GET_BASE_3d(float) \
    CONSTRUCT_TEXELF_3d_1CHANNEL_STORE(float, BASE)
#define STORE_TEXELF_3d_R_half(CHANNEL_TYPE, BASE) \
    GET_BASE_3d(half) \
"    vstore_half((color.x * "#BASE"f + 0.5f), coord.x,base); \n"

#define STORE_TEXELF_1CHANNEL_3d(CHANNEL_TYPE, BASE) \
    STORE_TEXELF_3d_R_##CHANNEL_TYPE(CHANNEL_TYPE, BASE)

#define WRITE_IMAGE_PROTOTYPE_1d(TYPE, CHANNEL_TYPE, SWAP_RB) \
"void\n" \
    "_write_image_"#TYPE"_"#CHANNEL_TYPE#SWAP_RB"_1d (\n" \
"    uint image, \n" \
"    uint imageSize, \n" \
"    int coord, \n" \
"    "#TYPE" color \n"\
"    ) \n"

#define WRITE_IMAGE_PROTOTYPE_1darray(TYPE, CHANNEL_TYPE, SWAP_RB) \
"void\n" \
"_write_image_"#TYPE"_"#CHANNEL_TYPE#SWAP_RB"_1darray (\n" \
"    uint2 image, \n" \
"    uint2 imageSize, \n" \
"    int2 coord, \n" \
"    "#TYPE" color \n"\
"    ) \n"

#define WRITE_IMAGE_PROTOTYPE_2d(TYPE, CHANNEL_TYPE, SWAP_RB) \
"void\n" \
"_write_image_"#TYPE"_"#CHANNEL_TYPE#SWAP_RB"_2d (\n" \
"    uint2 image, \n" \
"    uint2 imageSize, \n" \
"    int2 coord, \n" \
"    "#TYPE" color \n"\
"    ) \n"

#define WRITE_IMAGE_PROTOTYPE_2DARRAY(TYPE, CHANNEL_TYPE, SWAP_RB) \
"void\n" \
"_write_image_"#TYPE"_"#CHANNEL_TYPE#SWAP_RB"_2DARRAY (\n" \
"    uint3 image, \n" \
"    uint3 imageSize, \n" \
"    int3 coord, \n" \
"    "#TYPE" color \n"\
"    ) \n"

#define WRITE_IMAGE_PROTOTYPE_3d(TYPE, CHANNEL_TYPE, SWAP_RB) \
"void\n" \
"_write_image_"#TYPE"_"#CHANNEL_TYPE#SWAP_RB"_3d (\n" \
"    uint3 image, \n" \
"    uint3 imageSize, \n" \
"    int3 coord, \n" \
"    "#TYPE" color \n"\
"    ) \n"

#define STORE_SWAP_RB_1d(CHANNEL_TYPE) \
"    "#CHANNEL_TYPE" swap = base[coord];\n" \
"    base[coord].r = swap.b;\n" \
"    base[coord].b = swap.r;\n"
#define STORE_SWAP_RB(CHANNEL_TYPE) \
"    "#CHANNEL_TYPE" swap = base[coord.x];\n" \
"    base[coord.x].r = swap.b;\n" \
"    base[coord.x].b = swap.r;\n"
#define STORE_SWAP_RB_1darray   STORE_SWAP_RB
#define STORE_SWAP_RB_2DARRAY   STORE_SWAP_RB
#define STORE_SWAP_RB_2d   STORE_SWAP_RB
#define STORE_SWAP_RB_3d   STORE_SWAP_RB

#define WRITE_IMAGE(TYPE, CHANNEL_TYPE, IMAGETYPE) \
WRITE_IMAGE_PROTOTYPE_##IMAGETYPE(TYPE, CHANNEL_TYPE,) \
"{ \n" \
    STORE_TEXEL_##IMAGETYPE(CHANNEL_TYPE) \
"} \n"

#define WRITE_IMAGEF_NORM(CHANNEL_TYPE, BASE, IMAGETYPE) \
WRITE_IMAGE_PROTOTYPE_##IMAGETYPE(float4, CHANNEL_TYPE,) \
"{ \n" \
    STORE_TEXELF_##IMAGETYPE(CHANNEL_TYPE, BASE) \
"} \n"

#define WRITE_IMAGE_BGRA(TYPE, CHANNEL_TYPE, IMAGETYPE) \
WRITE_IMAGE_PROTOTYPE_##IMAGETYPE(TYPE, CHANNEL_TYPE, _BGRA) \
"{ \n" \
    STORE_TEXEL_##IMAGETYPE(CHANNEL_TYPE) \
    STORE_SWAP_RB_##IMAGETYPE(CHANNEL_TYPE) \
"} \n"

#define WRITE_IMAGEF_NORM_BGRA(CHANNEL_TYPE, BASE, IMAGETYPE) \
WRITE_IMAGE_PROTOTYPE_##IMAGETYPE(float4, CHANNEL_TYPE, _BGRA) \
"{ \n" \
    STORE_TEXELF_##IMAGETYPE(CHANNEL_TYPE, BASE) \
    STORE_SWAP_RB_##IMAGETYPE(CHANNEL_TYPE) \
"} \n"

#define WRITE_IMAGE_R(TYPE, CHANNEL_TYPE, IMAGETYPE) \
WRITE_IMAGE_PROTOTYPE_##IMAGETYPE(TYPE, CHANNEL_TYPE, _R) \
"{ \n" \
    STORE_TEXEL_1CHANNEL_##IMAGETYPE(CHANNEL_TYPE) \
"} \n"

#define WRITE_IMAGEF_NORM_R(CHANNEL_TYPE, BASE, IMAGETYPE) \
WRITE_IMAGE_PROTOTYPE_##IMAGETYPE(float4, CHANNEL_TYPE, _R) \
"{ \n" \
    STORE_TEXELF_1CHANNEL_##IMAGETYPE(CHANNEL_TYPE, BASE) \
"} \n"

#define WRITE_IMAGE_IMGST(TYPE, IMAGETYPE) \
"void\n" \
    "_write_image_"#TYPE"_"#IMAGETYPE" (\n" \
"    uint4 image, \n" \
"    uint2 imageSize, \n" \
"    int2 coord, \n" \
"    "#TYPE" color \n"\
"    ) \n" \
"{ \n" \
"    _viv_asm(IMAGE_WRITE, color, image, coord);\n"\
"}\n"

extern gctSTRING gcLibCLImage_WriteFunc;
extern gctSTRING gcLibCLImage_WriteFunc_BGRA;
extern gctSTRING gcLibCLImage_WriteFunc_R;
extern gctSTRING gcLibCLPatch_MainFunc;

#if _SUPPORT_LONG_ULONG_DATA_TYPE
/************************************* 64-bit integer SHIFT. ***********************************/
#define _longulong_left_shift_long_scalar    \
"long long_LeftShift_scalar(uint count, long src0, long src1)   \n" \
"{ \n"                                      \
"    uint lowV, highV; \n"  \
"    uint lowR, highR; \n" \
"    uint mask; \n" \
"    uint n;\n" \
"    long result = 0L; \n" \
"\n"\
"    n = viv_getlonglo(src1); \n" \
"    n = n & 63; \n" \
"    lowV = viv_getlonglo(src0); \n" \
"    highV = viv_getlonghi(src0); \n" \
" \n" \
"    if (n == 0) \n" \
"    { \n" \
"        lowR = lowV;\n"\
"        highR = highV;\n"\
"    } \n" \
"    else \n" \
"    if (n >= 32) \n" \
"    { \n" \
"        n -= 32; \n" \
"        lowR = 0; \n" \
"        highR = lowV << n; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        lowR = lowV << n; \n" \
"        highR = highV << n; \n" \
"        lowV = lowV >> (32 - n); \n" \
"        mask = (~0) >> (32 - n); \n" \
"        lowV = lowV & mask; \n" \
"        highR = highR | lowV; \n" \
"    } \n" \
"    viv_setlong(result, as_uint(lowR), as_uint(highR)); \n" \
"    return result; \n" \
"} \n"

#define _longulong_left_shift_long    \
"long4 long_LeftShift(uint count, long4 src0, long4 src1)   \n" \
"{ \n"                                      \
"    uint lowV, highV; \n"  \
"    uint lowR, highR; \n" \
"    uint mask; \n" \
"    uint n, i;\n" \
"    long data, shift, tl;\n"\
"    long4 result = 0L; \n" \
"\n"\
"    for (i = 0; i < count; i++) \n"\
"    {\n"\
"        if (i == 0)      {data = src0.x; shift = src1.x;}\n"\
"        else if (i == 1) {data = src0.y; shift = src1.y;}\n"\
"        else if (i == 2) {data = src0.z; shift = src1.z;}\n"\
"        else             {data = src0.w; shift = src1.w;}\n"\
"        n = viv_getlonglo(shift); \n" \
"        n = n & 63; \n" \
"        lowV = viv_getlonglo(data); \n" \
"        highV = viv_getlonghi(data); \n" \
" \n" \
"        if (n == 0) \n" \
"        { \n" \
"            lowR = lowV;\n"\
"            highR = highV;\n"\
"        } \n" \
"        else \n" \
"        if (n >= 32) \n" \
"        { \n" \
"            n -= 32; \n" \
"            lowR = 0; \n" \
"            highR = lowV << n; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            lowR = lowV << n; \n" \
"            highR = highV << n; \n" \
"            lowV = lowV >> (32 - n); \n" \
"            mask = (~0) >> (32 - n); \n" \
"            lowV = lowV & mask; \n" \
"            highR = highR | lowV; \n" \
"        } \n" \
"        viv_setlong(tl, as_uint(lowR), as_uint(highR)); \n" \
"        if (i == 0)      {result.x = tl;}\n"\
"        else if (i == 1) {result.y = tl;}\n"\
"        else if (i == 2) {result.z = tl;}\n"\
"        else             {result.w = tl;}\n"\
"    }\n" \
"    return result; \n" \
"} \n"

#define _longulong_left_shift_ulong_scalar    \
"ulong ulong_LeftShift_scalar(uint count, ulong src0, ulong src1)   \n" \
"{ \n"                                      \
"    uint lowV, highV; \n"  \
"    uint lowR, highR; \n" \
"    uint mask; \n" \
"    ulong result = 0L; \n" \
"    uint n;\n" \
"\n"\
"    n = viv_getlonglo(src1); \n" \
"    n = n & 63; \n" \
"    lowV = viv_getlonglo(src0); \n" \
"    highV = viv_getlonghi(src0); \n" \
" \n" \
"    if (n == 0) \n" \
"    { \n" \
"        lowR = lowV;\n"\
"        highR = highV;\n"\
"    } \n" \
"    else \n" \
"    if (n >= 32) \n" \
"    { \n" \
"        n -= 32; \n" \
"        lowR = 0; \n" \
"        highR = lowV << n; \n" \
"    } \n" \
"    else \n" \
"    { \n" \
"        lowR = lowV << n; \n" \
"        highR = highV << n; \n" \
"        lowV = lowV >> (32 - n); \n" \
"        mask = (~0) >> (32 - n); \n" \
"        lowV = lowV & mask; \n" \
"        highR = highR | lowV; \n" \
"    } \n" \
" \n" \
"    viv_setlong(result, as_uint(lowR), as_uint(highR)); \n" \
"    return result; \n" \
"} \n"

#define _longulong_left_shift_ulong    \
"ulong4 ulong_LeftShift(uint count, ulong4 src0, ulong4 src1)   \n" \
"{ \n"                                      \
"    uint lowV, highV; \n"  \
"    uint lowR, highR; \n" \
"    uint mask; \n" \
"    ulong4 result = 0L; \n" \
"    uint n, i;\n" \
"    ulong data, shift, tl;\n"\
"\n"\
"    for (i = 0; i < count; i++)\n"\
"    {\n"\
"        if (i == 0)      {data = src0.x; shift = src1.x;}\n"\
"        else if (i == 1) {data = src0.y; shift = src1.y;}\n"\
"        else if (i == 2) {data = src0.z; shift = src1.z;}\n"\
"        else             {data = src0.w; shift = src1.w;}\n"\
"        n = viv_getlonglo(shift); \n" \
"        n = n & 63; \n" \
"        lowV = viv_getlonglo(data); \n" \
"        highV = viv_getlonghi(data); \n" \
" \n" \
"        if (n == 0) \n" \
"        { \n" \
"            lowR = lowV;\n"\
"            highR = highV;\n"\
"        } \n" \
"        else \n" \
"        if (n >= 32) \n" \
"        { \n" \
"            n -= 32; \n" \
"            lowR = 0; \n" \
"            highR = lowV << n; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            lowR = lowV << n; \n" \
"            highR = highV << n; \n" \
"            lowV = lowV >> (32 - n); \n" \
"            mask = (~0) >> (32 - n); \n" \
"            lowV = lowV & mask; \n" \
"            highR = highR | lowV; \n" \
"        } \n" \
" \n" \
"        viv_setlong(tl, as_uint(lowR), as_uint(highR)); \n" \
"        if (i == 0)      {result.x = tl;}\n"\
"        else if (i == 1) {result.y = tl;}\n"\
"        else if (i == 2) {result.z = tl;}\n"\
"        else             {result.w = tl;}\n"\
"    } \n"\
"    return result; \n" \
"} \n"

#define _longulong_right_shift_long    \
"long4 long_RightShift(uint count, long4 src0, long4 src1)   \n" \
"{ \n"                                      \
"    uint lowV; \n"  \
"    int  highV, highR, lowR; \n" \
"    uint n, i;\n" \
"    long data, shift, tl;\n"\
"    long4 result = 0L; \n" \
"\n" \
"    for (i = 0; i < count; i++)\n" \
"    {\n"\
"        if (i == 0)      {data = src0.x; shift = src1.x;}\n"\
"        else if (i == 1) {data = src0.y; shift = src1.y;}\n"\
"        else if (i == 2) {data = src0.z; shift = src1.z;}\n"\
"        else             {data = src0.w; shift = src1.w;}\n"\
"        n = viv_getlonglo(shift); \n" \
"        n = n & 63; \n" \
"        lowV = viv_getlonglo(data); \n" \
"        highV = as_int(viv_getlonghi(data)); \n" \
" \n" \
"        if (n == 0) \n" \
"        { \n" \
"            lowR = lowV;\n"\
"            highR = highV;\n"\
"        } \n" \
"        else \n" \
"        if (n >= 32) \n" \
"        { \n" \
"            n -= 32; \n" \
"            lowR = highV >> n; \n" \
"            highR = highV >= 0 ? 0 : -1; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            highR = highV >> n; \n" \
"            lowR = lowV >> n; \n" \
"            highV = highV << (32 - n); \n" \
"            lowR |= highV; \n" \
"        } \n" \
" \n" \
"        viv_setlong(tl, as_uint(lowR), as_uint(highR)); \n" \
"        if (i == 0)      {result.x = tl;}\n"\
"        else if (i == 1) {result.y = tl;}\n"\
"        else if (i == 2) {result.z = tl;}\n"\
"        else             {result.w = tl;}\n"\
"    }\n"\
"    return result; \n" \
"} \n"

#define _longulong_right_shift_ulong   \
"ulong4 ulong_RightShift(uint count, ulong4 src0, ulong4 src1)   \n" \
"{ \n"                                      \
"    uint lowV, lowR; \n"  \
"    uint highV, highR; \n" \
"    ulong4 result; \n" \
"    uint n, i;\n" \
"    ulong data, shift, tl;\n"\
"\n"\
"    for (i = 0; i < count; i++)\n" \
"    {\n"\
"        if (i == 0)      {data = src0.x; shift = src1.x;}\n"\
"        else if (i == 1) {data = src0.y; shift = src1.y;}\n"\
"        else if (i == 2) {data = src0.z; shift = src1.z;}\n"\
"        else             {data = src0.w; shift = src1.w;}\n"\
"        n = viv_getlonglo(shift); \n" \
"        n = n & 63; \n" \
"        lowV = viv_getlonglo(data); \n" \
"        highV = viv_getlonghi(data); \n" \
" \n" \
"        if (n == 0) \n" \
"        { \n" \
"            lowR = lowV;\n"\
"            highR = highV;\n"\
"        } \n" \
"        else \n" \
"        if (n >= 32) \n" \
"        { \n" \
"            n -= 32; \n" \
"            lowR = highV >> n; \n" \
"            highR = highV >= 0 ? 0 : -1; \n" \
"        } \n" \
"        else \n" \
"        { \n" \
"            highR = highV >> n; \n" \
"            lowR = lowV >> n; \n" \
"            highV = highV << (32 - n); \n" \
"            lowR |= highV; \n" \
"        } \n" \
"\n" \
"        viv_setlong(tl, as_uint(lowR), as_uint(highR)); \n" \
"        if (i == 0)      {result.x = tl;}\n"\
"        else if (i == 1) {result.y = tl;}\n"\
"        else if (i == 2) {result.z = tl;}\n"\
"        else             {result.w = tl;}\n"\
"    } \n"\
"    return result;\n" \
"}\n"

/************************************* 64-bit integer ADD. ***********************************/
#define _longulong_add_long \
"long4 long_Add(uint count, long4 x, long4 y)\n\
{\n\
    uint lox, loy, hix, hiy;\n\
    long a;\n\
    long4 r;\n\
    uint i;\n\
    for (i = 0; i < count; i++)\n\
    {\n\
        if (i == 0)     {lox = viv_getlonglo(x.x); loy = viv_getlonglo(y.x); hix = viv_getlonghi(x.x); hiy = viv_getlonghi(y.x);}\n\
        else if (i == 1){lox = viv_getlonglo(x.y); loy = viv_getlonglo(y.y); hix = viv_getlonghi(x.y); hiy = viv_getlonghi(y.y);}\n\
        else if (i == 2){lox = viv_getlonglo(x.z); loy = viv_getlonglo(y.z); hix = viv_getlonghi(x.z); hiy = viv_getlonghi(y.z);}\n\
        else            {lox = viv_getlonglo(x.w); loy = viv_getlonglo(y.w); hix = viv_getlonghi(x.w); hiy = viv_getlonghi(y.w);};\n\
        if(lox > ~loy)\n\
            hix++;\n\
        hix += hiy;\n\
        lox += loy;\n\
        viv_setlong(a, lox, hix);\n\
        if (i == 0)      r.x = a;\n\
        else if (i == 1) r.y = a;\n\
        else if (i == 2) r.z = a;\n\
        else             r.w = a;\n\
    }\n\
    return r;\n\
}\n"

#define _longulong_add_ulong \
"ulong4 ulong_Add(uint count, ulong4 x, ulong4 y)\n\
{\n\
    uint lox, loy, hix, hiy;\n\
    ulong a;\n\
    ulong4 r;\n\
    uint i;\n\
    for (i = 0; i < count; i++)\n\
    {\n\
        if (i == 0)     {lox = viv_getlonglo(x.x); loy = viv_getlonglo(y.x); hix = viv_getlonghi(x.x); hiy = viv_getlonghi(y.x);}\n\
        else if (i == 1){lox = viv_getlonglo(x.y); loy = viv_getlonglo(y.y); hix = viv_getlonghi(x.y); hiy = viv_getlonghi(y.y);}\n\
        else if (i == 2){lox = viv_getlonglo(x.z); loy = viv_getlonglo(y.z); hix = viv_getlonghi(x.z); hiy = viv_getlonghi(y.z);}\n\
        else            {lox = viv_getlonglo(x.w); loy = viv_getlonglo(y.w); hix = viv_getlonghi(x.w); hiy = viv_getlonghi(y.w);}\n\
        if(lox > ~loy)\n\
            hix++;\n\
        hix += hiy;\n\
        lox += loy;\n\
        viv_setlong(a, lox, hix);\n\
        if (i == 0)      r.x = a;\n\
        else if (i == 1) r.y = a;\n\
        else if (i == 2) r.z = a;\n\
        else             r.w = a;\n\
    }\n\
    return r;\n\
}\n"

#define _longulong_greater_long \
"\
int long_Greater(long src0, long src1)\n\
{\n\
    int  hi0, hi1;\n\
    uint lo0, lo1;\n\
    int result = -1;\n\
    lo0 = viv_getlonglo(src0);\n\
    hi0 = viv_getlonghi(src0);\n\
    lo1 = viv_getlonglo(src1);\n\
    hi1 = viv_getlonghi(src1);\n\
    if (hi0 < hi1)\n\
    {\n\
        result = 0;\n\
        return result;\n\
    }\n\
    else if (hi0 == hi1)\n\
    {\n\
        if (lo0 <= lo1)\n\
        {\n\
            result = 0;\n\
            return result;\n\
        }\n\
        else\n \
            return result;\n\
    }\n\
    else\n \
    {\n \
        result = 1;\n \
        return result;\n\
    }\n \
\
}\n\
"

#define _longulong_addsat_long \
"long4 long_AddSat(uint count, long4 x, long4 y)\n\
{\n\
    uint lox, loy, hix, hiy;\n\
    long a;\n\
    long4 r;\n\
    uint i;\n\
    for (i = 0; i < count; i++)\n\
    {\n\
        long tmpx = 0;\n \
        if (i == 0)     {lox = viv_getlonglo(x.x); loy = viv_getlonglo(y.x); hix = viv_getlonghi(x.x); hiy = viv_getlonghi(y.x);}\n\
        else if (i == 1){lox = viv_getlonglo(x.y); loy = viv_getlonglo(y.y); hix = viv_getlonghi(x.y); hiy = viv_getlonghi(y.y);}\n\
        else if (i == 2){lox = viv_getlonglo(x.z); loy = viv_getlonglo(y.z); hix = viv_getlonghi(x.z); hiy = viv_getlonghi(y.z);}\n\
        else            {lox = viv_getlonglo(x.w); loy = viv_getlonglo(y.w); hix = viv_getlonghi(x.w); hiy = viv_getlonghi(y.w);};\n\
        viv_setlong(tmpx, lox, hix);\n\
        if(lox > ~loy)\n\
            hix++;\n\
        hix += hiy;\n\
        lox += loy;\n\
        viv_setlong(a, lox, hix);\n\
        if(as_int(hiy) > 0)\n \
        {\n \
            if(long_Greater(tmpx, a) == 1)\n \
            {\n \
                viv_setlong(a, as_uint(0xffffffff), as_uint(0x7fffffff));\n \
            }\n \
        }\n \
        else \n \
        {\n \
            if(long_Greater(a, tmpx) == 1)\n \
                viv_setlong(a, as_uint(0), as_uint(0x80000000));\n \
        }\n \
        if (i == 0)      r.x = a;\n\
        else if (i == 1) r.y = a;\n\
        else if (i == 2) r.z = a;\n\
        else             r.w = a;\n\
    }\n\
    return r;\n\
}\n"

#define _longulong_subsat_long \
"long4 long_SubSat(uint count, long4 x, long4 y)\n\
{\n\
    uint lox, loy, hix, hiy;\n\
    long a;\n\
    long4 r;\n\
    uint i;\n\
    for (i = 0; i < count; i++)\n\
    {\n\
        long tmpx = 0;\n \
        if (i == 0)     {lox = viv_getlonglo(x.x); loy = viv_getlonglo(y.x); hix = viv_getlonghi(x.x); hiy = viv_getlonghi(y.x);}\n\
        else if (i == 1){lox = viv_getlonglo(x.y); loy = viv_getlonglo(y.y); hix = viv_getlonghi(x.y); hiy = viv_getlonghi(y.y);}\n\
        else if (i == 2){lox = viv_getlonglo(x.z); loy = viv_getlonglo(y.z); hix = viv_getlonghi(x.z); hiy = viv_getlonghi(y.z);}\n\
        else            {lox = viv_getlonglo(x.w); loy = viv_getlonglo(y.w); hix = viv_getlonghi(x.w); hiy = viv_getlonghi(y.w);};\n\
        viv_setlong(tmpx, lox, hix);\n\
        if(lox < loy)\n\
            hix--;\n\
        hix -= hiy;\n\
        lox -= loy;\n\
        viv_setlong(a, lox, hix);\n\
        if(as_int(hiy) < 0)\n \
        {\n \
            if(long_Greater(tmpx, a) == 1)\n \
            {\n \
                viv_setlong(a, as_uint(0xffffffff), as_uint(0x7fffffff));\n \
            }\n \
        }\n \
        else \n \
        {\n \
            if(long_Greater(a, tmpx) == 1)\n \
                viv_setlong(a, as_uint(0), as_uint(0x80000000));\n \
        }\n \
        if (i == 0)      r.x = a;\n\
        else if (i == 1) r.y = a;\n\
        else if (i == 2) r.z = a;\n\
        else             r.w = a;\n\
    }\n\
    return r;\n\
}\n"

#define _longulong_addsat_ulong \
"ulong4 ulong_AddSat(uint count, ulong4 x, ulong4 y)\n\
{\n\
    uint lox, loy, hix, hiy;\n\
    ulong a;\n\
    ulong4 r;\n\
    uint i;\n\
    for (i = 0; i < count; i++)\n\
    {\n\
        ulong tmpx;\n \
        if (i == 0)     {lox = viv_getlonglo(x.x); loy = viv_getlonglo(y.x); hix = viv_getlonghi(x.x); hiy = viv_getlonghi(y.x);}\n\
        else if (i == 1){lox = viv_getlonglo(x.y); loy = viv_getlonglo(y.y); hix = viv_getlonghi(x.y); hiy = viv_getlonghi(y.y);}\n\
        else if (i == 2){lox = viv_getlonglo(x.z); loy = viv_getlonglo(y.z); hix = viv_getlonghi(x.z); hiy = viv_getlonghi(y.z);}\n\
        else            {lox = viv_getlonglo(x.w); loy = viv_getlonglo(y.w); hix = viv_getlonghi(x.w); hiy = viv_getlonghi(y.w);}\n\
        viv_setlong(tmpx, lox, hix);\n\
        if(lox > ~loy)\n\
            hix++;\n\
        hix += hiy;\n\
        lox += loy;\n\
        viv_setlong(a, lox, hix);\n\
        if(ulong_Greater(tmpx, a) == 1)\n \
        {\n \
            viv_setlong(a, as_uint(0xffffffff), as_uint(0xffffffff));\n\
        }\n \
        if (i == 0)      r.x = a;\n\
        else if (i == 1) r.y = a;\n\
        else if (i == 2) r.z = a;\n\
        else             r.w = a;\n\
    }\n\
    return r;\n\
}\n"

#define _longulong_subsat_ulong \
"ulong4 ulong_SubSat(uint count, ulong4 x, ulong4 y)\n\
{\n\
    uint lox, loy, hix, hiy;\n\
    ulong a;\n\
    ulong4 r;\n\
    uint i;\n\
    for (i = 0; i < count; i++)\n\
    {\n\
        ulong tmpx, tmpy;\n \
        if (i == 0)     {lox = viv_getlonglo(x.x); loy = viv_getlonglo(y.x); hix = viv_getlonghi(x.x); hiy = viv_getlonghi(y.x);}\n\
        else if (i == 1){lox = viv_getlonglo(x.y); loy = viv_getlonglo(y.y); hix = viv_getlonghi(x.y); hiy = viv_getlonghi(y.y);}\n\
        else if (i == 2){lox = viv_getlonglo(x.z); loy = viv_getlonglo(y.z); hix = viv_getlonghi(x.z); hiy = viv_getlonghi(y.z);}\n\
        else            {lox = viv_getlonglo(x.w); loy = viv_getlonglo(y.w); hix = viv_getlonghi(x.w); hiy = viv_getlonghi(y.w);}\n\
        viv_setlong(tmpx, lox, hix);\n\
        viv_setlong(tmpy, loy, hiy);\n\
        if(lox < loy)\n\
            hix--;\n\
        hix -= hiy;\n\
        lox -= loy;\n\
        viv_setlong(a, lox, hix);\n\
        if(ulong_Greater(tmpy, tmpx) == 1)\n \
        {\n \
            viv_setlong(a, as_uint(0), as_uint(0));\n\
        }\n \
        if (i == 0)      r.x = a;\n\
        else if (i == 1) r.y = a;\n\
        else if (i == 2) r.z = a;\n\
        else             r.w = a;\n\
    }\n\
    return r;\n\
}\n"

#define longulong_sub \
"\
ulong4 ulong_Sub(uint count, ulong4 x, ulong4 y)\n\
{\n\
    uint lox, loy, hix, hiy;\n\
    ulong a;\n\
    ulong4 r;\n\
    uint i;\n\
    for (i = 0; i < count; i++)\n\
    {\n\
        if (i == 0)     {lox = viv_getlonglo(x.x); loy = viv_getlonglo(y.x); hix = viv_getlonghi(x.x); hiy = viv_getlonghi(y.x);}\n\
        else if (i == 1){lox = viv_getlonglo(x.y); loy = viv_getlonglo(y.y); hix = viv_getlonghi(x.y); hiy = viv_getlonghi(y.y);}\n\
        else if (i == 2){lox = viv_getlonglo(x.z); loy = viv_getlonglo(y.z); hix = viv_getlonghi(x.z); hiy = viv_getlonghi(y.z);}\n\
        else            {lox = viv_getlonglo(x.w); loy = viv_getlonglo(y.w); hix = viv_getlonghi(x.w); hiy = viv_getlonghi(y.w);}\n\
        if (lox < loy)\n\
        {\n\
            hix--;\n\
        }\n\
        hix -= hiy;\n\
        lox -= loy;\n\
        viv_setlong(a, lox, hix);\n\
        if (i == 0)      r.x = a;\n\
        else if (i == 1) r.y = a;\n\
        else if (i == 2) r.z = a;\n\
        else             r.w = a;\n\
    }\n\
    return r;\n\
}\n\
long4 long_Sub(uint count, long4 x, long4 y)\n\
{\n\
    uint lox, loy, hix, hiy;\n\
    long a;\n\
    long4 r;\n\
    uint i;\n\
    for (i = 0; i < count; i++)\n\
    {\n\
        if (i == 0)     {lox = viv_getlonglo(x.x); loy = viv_getlonglo(y.x); hix = viv_getlonghi(x.x); hiy = viv_getlonghi(y.x);}\n\
        else if (i == 1){lox = viv_getlonglo(x.y); loy = viv_getlonglo(y.y); hix = viv_getlonghi(x.y); hiy = viv_getlonghi(y.y);}\n\
        else if (i == 2){lox = viv_getlonglo(x.z); loy = viv_getlonglo(y.z); hix = viv_getlonghi(x.z); hiy = viv_getlonghi(y.z);}\n\
        else            {lox = viv_getlonglo(x.w); loy = viv_getlonglo(y.w); hix = viv_getlonghi(x.w); hiy = viv_getlonghi(y.w);}\n\
        if (lox < loy)\n\
        {\n\
            hix--;\n\
        }\n\
        hix -= hiy;\n\
        lox -= loy;\n\
        viv_setlong(a, lox, hix);\n\
        if (i == 0)      r.x = a;\n\
        else if (i == 1) r.y = a;\n\
        else if (i == 2) r.z = a;\n\
        else             r.w = a;\n\
    }\n\
    return r;\n\
}\n\
"
/************************************* 64-bit integer ADD. ***********************************/
#define _longulong_div_ulong \
"\
long  viv_Mul64_32RShift(long  x1, int n32, int rshift) \n\
{ \n\
    int lox1, hix1, mulhi, mulme, mulme2, mullo; \n\
     long   a1; \n\
    lox1 = viv_getlonglo(x1); \n\
    hix1 = viv_getlonghi(x1); \n\
    mulhi = mul_hi((unsigned int)hix1, (unsigned int)n32); \n\
    mulme = mul_hi((unsigned int)lox1, (unsigned int)n32); \n\
    mulme2 = (unsigned int)hix1*(unsigned int)n32; \n\
    if((unsigned int)mulme > (unsigned int)~mulme2) \n\
        mulhi += 1; \n\
    mulme += mulme2; \n\
    mullo = lox1 * n32; \n\
 \
    if(rshift >= 64){ \n\
        viv_setlong(a1, (unsigned int)mulhi >> rshift, (unsigned int)0 ); \n\
    } \n\
    else if(rshift >= 32){ \n\
        mulme =(unsigned int)mulme >> (rshift - 32); \n\
        if(rshift > 32) \n\
            mulme |= (unsigned int)mulhi << (64 - rshift); \n\
        mulhi =(unsigned int)mulhi >> (rshift - 32); \n\
        viv_setlong(a1, (unsigned int)mulme, (unsigned int)mulhi); \n\
    } \n\
    else{ \n\
        mullo =(unsigned int)mullo >> (rshift); \n\
        if(rshift){ \n\
            mullo |= mulme << (32 - rshift); \n\
        } \n\
        mulme =(unsigned int)mulme >> (rshift); \n\
        if(rshift) \n\
            mulme |= (unsigned int)mulhi <<(32 - rshift);\n\
         viv_setlong(a1, (unsigned int)mullo, (unsigned int)mulme); \n\
    } \n\
    return a1; \n\
} \n\
\
long   viv_Mul64HiLo_32RShift(int lox1, int hix1, int n32, int rshift) \n\
{ \n\
    int  mulhi, mulme, mulme2, mullo; \n\
     long   a1; \n\
    mulhi = mul_hi((unsigned int)hix1, (unsigned int)n32); \n\
    mulme = mul_hi((unsigned int)lox1, (unsigned int)n32); \n\
    mulme2 = (unsigned int)hix1*(unsigned int)n32; \n\
    if((unsigned int)mulme > (unsigned int)~mulme2) \n\
        mulhi += 1; \n\
    mulme += mulme2; \n\
    mullo = lox1 * n32; \n\
 \n\
    if(rshift >= 64){ \n\
        viv_setlong(a1, (unsigned int)mulhi >> rshift, (unsigned int)0 ); \n\
    } \n\
    else if(rshift >= 32){ \n\
        mulme =(unsigned int)mulme >> (rshift - 32); \n\
        if(rshift > 32) \n\
            mulme |= (unsigned int)mulhi << (64 - rshift); \n\
        mulhi =(unsigned int)mulhi >> (rshift - 32); \n\
        viv_setlong(a1, (unsigned int)mulme, (unsigned int)mulhi); \n\
    } \n\
    else{ \n\
        mullo =(unsigned int)mullo >> (rshift); \n\
        if(mullo){ \n\
            mullo |= mulme << (32 - rshift); \n\
        } \n\
        mulme =(unsigned int)mulme >> (rshift); \n\
         viv_setlong(a1, (unsigned int)mullo, (unsigned int)mulme); \n\
    } \n\
    return a1; \n\
} \n\
\
long   viv_Mul64ThenNeg(unsigned int lox2, unsigned int hix2, long   y2) \n\
{ \n\
    unsigned int loy2, hiy2,hiz2,loz2; \n\
     long   a2; \n\
    loy2 = viv_getlonglo(y2); \n\
    hiy2 = viv_getlonghi(y2); \n\
    hiz2 = mul_hi(lox2, loy2); \n\
    hiz2 += lox2 * hiy2; \n\
    hiz2 += loy2 * hix2; \n\
    loz2 = loy2*lox2; \n\
    loz2 = 0-loz2; \n\
    hiz2 = ~hiz2; \n\
    if(loz2 == 0) \n\
        hiz2 += 1; \n\
    viv_setlong(a2, loz2, hiz2); \n\
    return a2; \n\
} \n\
\
long   viv_Add64(long  x3, long   y3) \n\
{ \n\
    unsigned int lox3, loy3, hix3, hiy3; \n\
     long   a3; \n\
    lox3 = viv_getlonglo(x3); \n\
    loy3 = viv_getlonglo(y3); \n\
    hix3 = viv_getlonghi(x3); \n\
    hiy3 = viv_getlonghi(y3); \n\
    if(lox3 > ~loy3) \n\
        hix3++; \n\
    hix3 += hiy3; \n\
    lox3 += loy3; \n\
    viv_setlong(a3, lox3, hix3); \n\
    return a3; \n\
} \n\
\
long  viv_Div_ulong(long  x, long  y) \n\
 { \n\
    unsigned int lox, loy, hix, hiy,lor, hir, loq, hiq; \n\
    int z, exp, mantissa; \n\
     long  a, zz, res, q, dq; \n\
    float fValue1, fValue0; \n\
    lox = viv_getlonglo(x); \n\
    loy = viv_getlonglo(y); \n\
    hix = viv_getlonghi(x); \n\
    hiy = viv_getlonghi(y); \n\
    fValue1 = (float) loy + (float)hiy*4294967296.f; /*to float*/ \n\
    exp = as_int(fValue1) >> 23; \n\
    exp -= 0x7f; \n\
    mantissa = as_int(fValue1) & 0x007fffff; /*Clean the exp*/ \n\
    mantissa |= 0x30800000; /*2^(-30)*mantissaY*/ \n\
    fValue0 = as_float(mantissa); \n\
    fValue1 = 1.0f/fValue0;                /*Actually, should use our InstRcp*/ \n\
 \
    mantissa = as_int(fValue1) - 3;  /*make should less than 1/y */  \n\
    fValue0 = as_float(mantissa); \n\
    z = (int)fValue0;           /*2^(log2(y)+30)/y, back to integer*/ \n\
 \
    q =  viv_Mul64_32RShift(x, z, exp+30 ); /*Get estimation of x/y, may get 21 bit precision*/ \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
 \
    dq = viv_Mul64_32RShift(res, z, exp+30 );  \n\
    q = viv_Add64(dq, q); \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
 \
    dq = viv_Mul64_32RShift(res, z, exp+30 );  \n\
    q = viv_Add64(dq, q); \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
 \
    lor =  viv_getlonglo(res); \n\
    hir =  viv_getlonghi(res); \n\
\
    if((hir > hiy) || ((hir == hiy) && (lor >= loy) )   ){ /*q = x/y - 1*/ \n\
        /*res -= y; we don't need return res, if do %, we should have this step and return to res*/ \n\
        loq =  viv_getlonglo(q); \n\
        hiq =  viv_getlonghi(q); \n\
        loq ++; \n\
        if(loq == 0) \n\
            hiq++; \n\
        viv_setlong(q, loq, hiq); \n\
    } \n\
    return q; \n\
} \n\
 \
long  viv_Div_long(long  x, long  y)\n\
{\n\
    uint lox, loy, hix, hiy,lor, hir, loq, hiq,signedXY = 0; \n\
    int z, exp, mantissa; \n\
     long  a, zz, res, q, dq; \n\
    float fValue1, fValue0; \n\
    lox = viv_getlonglo(x); \n\
    loy = viv_getlonglo(y); \n\
    hix = viv_getlonghi(x); \n\
    hiy = viv_getlonghi(y); \n\
    if(hix>>31){\n\
        lox = 0-lox;\n\
        hix = (~hix);\n\
        if(lox == 0)\n\
            hix++;\n\
        signedXY ^= 0xffffffff;\n\
        viv_setlong(x, lox, hix);\n\
    }\n\
    if(hiy>>31){\n\
        loy = 0-loy;\n\
        hiy = (~hiy);\n\
        if(loy == 0)\n\
            hiy++;\n\
        signedXY ^= 0xffffffff;\n\
    }\n\
    fValue1 = (float) loy + (float)hiy*4294967296.f; /*to float*/ \n\
    exp = as_int(fValue1) >> 23; \n\
    exp -= 0x7f; \n\
    mantissa = as_int(fValue1) & 0x007fffff; /*Clean the exp*/ \n\
    mantissa |= 0x30800000; /*2^(-30)*mantissaY*/ \n\
    fValue0 = as_float(mantissa); \n\
    fValue1 = 1.0f/fValue0;                /*Actually, should use our InstRcp*/ \n\
 \
    mantissa = as_int(fValue1) - 3;  /*make should less than 1/y */  \n\
    fValue0 = as_float(mantissa); \n\
    z = (int)fValue0;           /*2^(log2(y)+30)/y, back to integer*/ \n\
 \
    q = viv_Mul64_32RShift(x, z, exp+30 ); /*Get estimation of x/y, may get 21 bit precision*/ \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
 \
    dq = viv_Mul64_32RShift(res, z, exp+30 );  \n\
    q = viv_Add64(dq, q); \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
 \
    dq = viv_Mul64_32RShift(res, z, exp+30 );  \n\
    q = viv_Add64(dq, q); \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
 \
    loq =  viv_getlonglo(res); \n\
    hiq =  viv_getlonghi(res); \n\
\
    if((hiq > hiy) || ((hiq == hiy) && (loq >= loy) )   ){ /*q = x/y - 1*/ \n\
        /*res -= y; we don't need return res, if do %, we should have this step and return to res*/ \n\
        loq =  viv_getlonglo(q); \n\
        hiq =  viv_getlonghi(q); \n\
        loq ++; \n\
        if(loq == 0) \n\
            hiq++; \n\
    } \n\
    else{\n\
        loq =  viv_getlonglo(q); \n\
        hiq =  viv_getlonghi(q); \n\
    }\n\
    if(signedXY){\n\
        loq = 0-loq;\n\
        hiq = (~hiq) + (loq == 0);\n\
    }\n\
    viv_setlong(q, loq, hiq); \n\
    return q; \n\
}\n\
\
long4  long_Div(uint count, long4  src0, long4  src1) \n\
{ \n\
    long4 r = 0L;\n\
\
    switch (count){\n\
    case 4:\n\
        r.w = viv_Div_long(src0.w, src1.w);\n\
    case 3:\n\
        r.z = viv_Div_long(src0.z, src1.z);\n\
    case 2:\n\
        r.y = viv_Div_long(src0.y, src1.y);\n\
    case 1:\n\
    default:\n\
        r.x = viv_Div_long(src0.x, src1.x);\n\
    break;\n\
    }\n\
    return r;\n\
} \n\
\
long4  ulong_Div(uint count, long4  src0, long4  src1) \n\
{ \n\
    long4 r = 0L;\n\
\
    switch (count){\n\
    case 4:\n\
        r.w = viv_Div_ulong(src0.w, src1.w);\n\
    case 3:\n\
        r.z = viv_Div_ulong(src0.z, src1.z);\n\
    case 2:\n\
        r.y = viv_Div_ulong(src0.y, src1.y);\n\
    case 1:\n\
    default:\n\
        r.x = viv_Div_ulong(src0.x, src1.x);\n\
    break;\n\
    }\n\
    return r;\n\
} \n\
\
long  viv_Mod_ulong(long  x, long  y) \n\
 { \n\
    uint lox, loy, hix, hiy,lor, hir, loq, hiq; \n\
    int z, exp, mantissa; \n\
    long  a, zz, res, q, dq; \n\
    float fValue1, fValue0; \n\
    lox = viv_getlonglo(x); \n\
    loy = viv_getlonglo(y); \n\
    hix = viv_getlonghi(x); \n\
    hiy = viv_getlonghi(y); \n\
    fValue1 = (float) loy + (float)hiy*4294967296.f; /*to float*/ \n\
    exp = as_int(fValue1) >> 23; \n\
    exp -= 0x7f; \n\
    mantissa = as_int(fValue1) & 0x007fffff; /*Clean the exp*/ \n\
    mantissa |= 0x30800000; /*2^(-30)*mantissaY*/ \n\
    fValue0 = as_float(mantissa); \n\
    fValue1 = 1.0f/fValue0;                /*Actually, should use our InstRcp*/ \n\
 \
    mantissa = as_int(fValue1) - 3;  /*make should less than 1/y */  \n\
    fValue0 = as_float(mantissa); \n\
    z = (int)fValue0;           /*2^(log2(y)+30)/y, back to integer*/ \n\
 \
    q =  viv_Mul64_32RShift(x, z, exp+30 ); /*Get estimation of x/y, may get 21 bit precision*/ \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
 \
    dq = viv_Mul64_32RShift(res, z, exp+30 );  \n\
    q = viv_Add64(dq, q); \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
 \
    dq = viv_Mul64_32RShift(res, z, exp+30 );  \n\
    q = viv_Add64(dq, q); \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
 \
    lor =  viv_getlonglo(res); \n\
    hir =  viv_getlonghi(res); \n\
\
    if((hir > hiy) || ((hir == hiy) && (lor >= loy) )   ){ /*q = x/y - 1*/ \n\
        /*res -= y; we don't need return res, if do %, we should have this step and return to res*/ \n\
      if(lor < loy)\n\
            hir--;\n\
      lor -= loy;\n\
      hir -= hiy;\n\
      viv_setlong(res, lor, hir); \n\
    } \n\
    return res; \n\
} \n\
\
long  viv_Mod_long(long  x, long  y) \n\
{ \n\
      uint lox, loy, hix, hiy,lor, hir, loq, hiq,signedXY = 0; \n\
    int z, exp, mantissa; \n\
      long  a, zz, res, q, dq; \n\
    float fValue1, fValue0; \n\
      lox = viv_getlonglo(x); \n\
      loy = viv_getlonglo(y); \n\
      hix = viv_getlonghi(x); \n\
      hiy = viv_getlonghi(y); \n\
    if(hix>>31){\n\
        lox = 0-lox;\n\
        hix = (~hix);\n\
        if(lox == 0)\n\
            hix++;\n\
            signedXY ^= 0xffffffff;\n\
            viv_setlong(x, lox, hix);\n\
    }\n\
    if(hiy>>31){\n\
        loy = 0-loy;\n\
        hiy = (~hiy);\n\
        if(loy == 0)\n\
            hiy++;\n\
    }\n\
      fValue1 = (float) loy + (float)hiy*4294967296.f; /*to float*/ \n\
      exp = as_int(fValue1) >> 23; \n\
      exp -= 0x7f; \n\
      mantissa = as_int(fValue1) & 0x007fffff; /*Clean the exp*/ \n\
      mantissa |= 0x30800000; /*2^(-30)*mantissaY*/ \n\
      fValue0 = as_float(mantissa); \n\
    fValue1 = 1.0f/fValue0;                     /*Actually, should use our InstRcp*/ \n\
\
      mantissa = as_int(fValue1) - 3;  /*make should less than 1/y */  \n\
    fValue0 = as_float(mantissa); \n\
      z = (int)fValue0;          /*2^(log2(y)+30)/y, back to integer*/ \n\
\
      q = viv_Mul64_32RShift(x, z, exp+30 ); /*Get estimation of x/y, may get 21 bit precision*/ \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
\
      dq = viv_Mul64_32RShift(res, z, exp+30 );  \n\
      q = viv_Add64(dq, q); \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
\
      dq = viv_Mul64_32RShift(res, z, exp+30 );  \n\
      q = viv_Add64(dq, q); \n\
    zz = viv_Mul64ThenNeg(loy, hiy, q); \n\
    res =  viv_Add64(x, zz); \n\
\
    loq =  viv_getlonglo(res); \n\
    hiq =  viv_getlonghi(res); \n\
\
    if((hiq > hiy) || ((hiq == hiy) && (loq >= loy) )   ){ \n\
        /*res -= y; if do %*/ \n\
      if(loq < loy)\n\
                  hiq--;\n\
      loq -= loy;\n\
      hiq -= hiy;\n\
    }\n\
\
    if(signedXY){\n\
        loq = 0-loq;\n\
        hiq = (~hiq) + (loq == 0);\n\
    }\n\
    viv_setlong(res, loq, hiq); \n\
    return res; \n\
} \n\
\
long4  long_Mod(uint count, long4  src0, long4  src1) \n\
{ \n\
    long4 r = 0L;\n\
\
    switch (count){\n\
    case 4:\n\
        r.w = viv_Mod_long(src0.w, src1.w);\n\
    case 3:\n\
        r.z = viv_Mod_long(src0.z, src1.z);\n\
    case 2:\n\
        r.y = viv_Mod_long(src0.y, src1.y);\n\
    case 1:\n\
    default:\n\
        r.x = viv_Mod_long(src0.x, src1.x);\n\
    break;\n\
    }\n\
    return r;\n\
} \n\
\
long4  ulong_Mod(uint count, long4  src0, long4  src1) \n\
{ \n\
    long4 r = 0L;\n\
\
    switch (count){\n\
    case 4:\n\
        r.w = viv_Mod_ulong(src0.w, src1.w);\n\
    case 3:\n\
        r.z = viv_Mod_ulong(src0.z, src1.z);\n\
    case 2:\n\
        r.y = viv_Mod_ulong(src0.y, src1.y);\n\
    case 1:\n\
    default:\n\
        r.x = viv_Mod_ulong(src0.x, src1.x);\n\
    break;\n\
    }\n\
    return r;\n\
} \n\
"

#define _longulong_mulhi_ulong \
"\n\
 ulong viv_MulHi_ulong(ulong x, ulong y) \n\
 { \n\
    unsigned int lox, loy, hix, hiy,loz, hiz; \n\
    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV;\n\
    long a; \n\
    lox = viv_getlonglo(x); \n\
    loy = viv_getlonglo(y); \n\
    hix = viv_getlonghi(x); \n\
    hiy = viv_getlonghi(y); \n\
    hiz = mul_hi(hix, hiy); \n\
    loz = hix*hiy; \n\
    mulH0L1L = hix*loy; \n\
    mulH1L0L = lox*hiy; \n\
    mulHighL0L1 = mul_hi(lox, loy); \n\
    mulH0L1 = mul_hi(hix, loy); \n\
    notV = ~mulH0L1L; \n\
    if(mulHighL0L1 >= notV){ \n\
        mulH0L1++; \n\
    } \n\
    mulHighL0L1 += mulH0L1L; \n\
    \n\
    mulH1L0 = mul_hi(lox, hiy); \n\
    notV = ~mulH1L0L; \n\
    if(mulHighL0L1 >= notV){ \n\
        mulH1L0++; \n\
    } \n\
\
    notV = ~mulH0L1; \n\
    if(loz >= notV){ \n\
        hiz++; \n\
    } \n\
    loz += mulH0L1; \n\
\
    notV = ~mulH1L0; \n\
    if(loz >= notV){ \n\
        hiz++; \n\
    } \n\
    loz += mulH1L0; \n\
    viv_setlong(a, loz, hiz); \n\
    return (ulong)a; \n\
} \n\
ulong4  ulong_MulHi(uint count, ulong4  src0, ulong4  src1) \n\
{ \n\
    ulong4 r = 0L;\n\
\
    switch (count){\n\
    case 4:\n\
        r.w = viv_MulHi_ulong(src0.w, src1.w);\n\
    case 3:\n\
        r.z = viv_MulHi_ulong(src0.z, src1.z);\n\
    case 2:\n\
        r.y = viv_MulHi_ulong(src0.y, src1.y);\n\
    case 1:\n\
    default:\n\
        r.x = viv_MulHi_ulong(src0.x, src1.x);\n\
    break;\n\
    }\n\
    return r;\n\
} \n"

#define _longulong_mad \
"\
/*\n\
Just to avoid build error, after compiler fixed, it should be:\n\
#define LONG_MIN         ((long) -0x7FFFFFFFFFFFFFFFLL - 1LL) \n\
#define LONG_MAX         ((cl_long) 0x7FFFFFFFFFFFFFFFLL)  \n\
#define ULONG_MAX        ((cl_ulong) 0xFFFFFFFFFFFFFFFFULL) \n\
*/\n\
#define LONG_MIN         0 \n\
#define LONG_MAX         0  \n\
#define ULONG_MAX        0 \n\
long viv_MadSat_long(long x, long y, long z)\n\
{\n\
    long  mulhi;\n\
    ulong mullo, sum;\n\
    mullo = viv_Mul_long(x, y);\n\
    mulhi = viv_MulHi_long(x, y);\n\
    sum = mullo + z;\n\
    if (z >= 0)\n\
    {\n\
        if (mullo > sum)\n\
        {\n\
            mulhi++;\n\
            if (mulhi == LONG_MIN)\n\
            {\n\
                mulhi = LONG_MAX;\n\
                sum = ULONG_MAX;\n\
            }\n\
        }\n\
    }\n\
    else\n\
    {\n\
        if (mullo < sum)\n\
        {\n\
            mulhi--;\n\
            if (LONG_MAX == mulhi)\n\
            {\n\
                mulhi = LONG_MIN;\n\
                sum = 0;\n\
            }\n\
        }\n\
    }\n\
\
    if (mulhi > 0)\n\
        sum = LONG_MAX;\n\
    else if (mulhi < -1)\n\
        sum = LONG_MIN;\n\
\
    return (long)sum;\n\
}\n\
\
ulong viv_MadSat_ulong(ulong x, ulong y, ulong z)\n\
{\n\
    ulong mulhi, mullo, sum;\n\
    mullo = viv_Mul_ulong(x, y);\n\
    mulhi = viv_MulHi_ulong(x, y);\n\
    mullo += z;\n\
    if (mullo < z) mulhi++;\n\
    if (mulhi != 0) mullo = 0xFFFFFFFFFFFFFFFFULL;\n\
    return mullo;\n\
}\n\
\
long4 long_MadSat(uint count, long4 x, long4 y, long4 z)\n\
{\n\
    long4 r;\n\
    switch (count)\n\
    {\n\
        case 4: r.w = viv_MadSat_long(x.w, y.w, z.w);\n\
        case 3: r.z = viv_MadSat_long(x.z, y.z, z.z);\n\
        case 2: r.y = viv_MadSat_long(x.y, y.y, z.y);\n\
        case 1:\n\
        default: r.x = viv_MadSat_long(x.x, y.x, z.x);\n\
        break;\n\
    }\n\
\
    return r;\n\
}\n\
\
ulong4 ulong_MadSat(uint count, ulong4 x, ulong4 y, ulong4 z)\n\
{\n\
    ulong4 r;\n\
    switch (count)\n\
    {\n\
        case 4: r.w = viv_MadSat_ulong(x.w, y.w, z.w);\n\
        case 3: r.z = viv_MadSat_ulong(x.z, y.z, z.z);\n\
        case 2: r.y = viv_MadSat_ulong(x.y, y.y, z.y);\n\
        case 1:\n\
        default: r.x = viv_MadSat_ulong(x.x, y.x, z.x);\n\
        break;\n\
    }\n\
\
    return r;\n\
}\n\
"

#define _longulong_mul_long \
"\
 long viv_MulHi_long(long x, long y) \n\
 { \n\
    unsigned int lox, loy, hix, hiy,loz, hiz; \n\
    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV;\n\
    long a; \n\
    lox = viv_getlonglo(x); \n\
    loy = viv_getlonglo(y); \n\
    hix = viv_getlonghi(x); \n\
    hiy = viv_getlonghi(y); \n\
    hiz = mul_hi(hix, hiy); \n\
    loz = hix*hiy; \n\
    mulH0L1L = hix*loy; \n\
    mulH1L0L = lox*hiy; \n\
    mulHighL0L1 = mul_hi(lox, loy); \n\
    mulH0L1 = mul_hi(hix, loy); \n\
    notV = ~mulH0L1L; \n\
    if(mulHighL0L1 >= notV){ \n\
        mulH0L1++; \n\
    } \n\
    mulHighL0L1 += mulH0L1L; \n\
    \
    mulH1L0 = mul_hi(lox, hiy); \n\
    notV = ~mulH1L0L; \n\
    if(mulHighL0L1 >= notV){ \n\
        mulH1L0++; \n\
    } \n\
\
    notV = ~mulH0L1; \n\
    if(loz >= notV){ \n\
        hiz++; \n\
    } \n\
    loz += mulH0L1; \n\
\
    notV = ~mulH1L0; \n\
    if(loz >= notV){ \n\
        hiz++; \n\
    } \n\
    loz += mulH1L0; \n\
/* For negative input, it looks like extended -1 to bit 64~127, then we do substraction */ \
    if(hix >= 0x80000000){\n\
        if(loz < loy) /*borrow happened*/ \n\
            hiz--; \n\
        loz -= loy; \n\
        hiz -= hiy; \n\
    }\n\
    if(hiy >= 0x80000000){\n\
        if(loz < lox) /*borrow happened*/ \n\
            hiz--; \n\
        loz -= lox; \n\
        hiz -= hix; \n\
    }\n\
    viv_setlong(a, loz, hiz); \n\
    return a; \n\
} \n\
long4  long_MulHi(uint count, long4  src0, long4  src1) \n\
{ \n\
    long4 r = 0L;\n\
\
    switch (count){\n\
    case 4:\n\
        r.w = viv_MulHi_long(src0.w, src1.w);\n\
    case 3:\n\
        r.z = viv_MulHi_long(src0.z, src1.z);\n\
    case 2:\n\
        r.y = viv_MulHi_long(src0.y, src1.y);\n\
    case 1:\n\
    default:\n\
        r.x = viv_MulHi_long(src0.x, src1.x);\n\
    break;\n\
    }\n\
    return r;\n\
} \n\
\
 long long_MulSat(uint count, long x, long y) \n\
 { \n\
    unsigned int lox, loy, hix, hiy,loz, hiz; \n\
    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV, resultSign = 0, overflow = 0, maxhi;\n\
    long a; \n\
    lox = viv_getlonglo(x); \n\
    loy = viv_getlonglo(y); \n\
    hix = viv_getlonghi(x); \n\
    hiy = viv_getlonghi(y); \n\
    if(hix>>31){ \n\
        hix = ~hix;\n\
        lox = 0-lox;\n\
        if(lox == 0) \n\
            hix++;\n\
        resultSign ^= 0xffffffff; \n\
    }\n\
    if(hiy>>31){ \n\
        hiy = ~hiy;\n\
        loy = 0-loy;\n\
        if(loy == 0) \n\
            hiy++;\n\
        resultSign ^= 0xffffffff; \n\
    }\n\
    hiz = mul_hi(hix, hiy); \n\
    loz = hix*hiy; \n\
    if(hiz || loz || mul_hi(lox, hiy) || mul_hi(hix, loy)){ \n\
        overflow = 0xffffffff; \n\
    }\n\
    else{ \n\
        mulH0L1L = hix*loy; \n\
        mulH1L0L = lox*hiy; \n\
        mulHighL0L1 = mul_hi(lox, loy); \n\
        notV = ~mulH0L1L; \n\
        if(mulHighL0L1 >= notV){ \n\
           overflow = 0xffffffff; \n\
        } \n\
        mulHighL0L1 += mulH0L1L; \n\
        \
        notV = ~mulH1L0L; \n\
        if(mulHighL0L1 >= notV){ \n\
            overflow = 0xffffffff; \n\
        }\n\
        hiz = mulHighL0L1 + mulH1L0L; \n\
        loz = lox * loy;\n\
        maxhi = (0x7fffffff - resultSign); \n\
        if(hiz > maxhi || (hiz == 0x80000000 && loz) ) \n\
            overflow = 0xffffffff;\n\
    }\n\
    notV = ~overflow;\n\
    if(resultSign){ \n\
        loz = (0-loz) & notV; \n\
        hiz = ((~hiz) + (loz == 0) )& notV; \n\
        hiz += overflow & 0x80000000;\n\
    }\n\
    else if(overflow){\n\
        loz = 0xffffffff;\n\
        hiz = 0x7fffffff;\n\
    }\n\
    viv_setlong(a, loz, hiz); \n\
    return a; \n\
} \n\
\
long viv_Mul_long(long x, long y)\n\
{\n\
    long r;\n\
    uint lox, loy, hix, hiy,loz, hiz;\n\
    lox = viv_getlonglo(x);\n\
    hix = viv_getlonghi(x);\n\
    loy = viv_getlonglo(y);\n\
    hiy = viv_getlonghi(y);\n\
\
    loz = lox*loy; \n\
    hiz = mul_hi(lox, loy); \n\
    hiz += hix*loy; \n\
    hiz += lox*hiy; \n\
    viv_setlong(r, loz, hiz); \n\
\
    return r;\n\
}\n\
\
long4  long_Mul(uint count, long4  x, long4  y) \n\
 { \n\
    long4 r;\n\
    switch(count)\n\
    {\n\
    case 4:\n\
        r.w = viv_Mul_long(x.w, y.w);\n\
    case 3:\n\
        r.z = viv_Mul_long(x.z, y.z);\n\
    case 2:\n\
        r.y = viv_Mul_long(x.y, y.y);\n\
    case 1:\n\
    default:\n\
        r.x = viv_Mul_long(x.x, y.x);\n\
        break;\n\
    }\n\
\
    return r; \n\
} \n"

#define _longulong_mul_ulong \
"\
 long ulong_MulSat(uint count, long x, long y) \n\
 { \n\
    unsigned int lox, loy, hix, hiy,loz, hiz; \n\
    unsigned int mulH1L0, mulH0L1, mulHighL0L1, mulH0L1L, mulH1L0L, notV, overflow = 0;\n\
    long a; \n\
    lox = viv_getlonglo(x); \n\
    loy = viv_getlonglo(y); \n\
    hix = viv_getlonghi(x); \n\
    hiy = viv_getlonghi(y); \n\
    hiz = mul_hi(hix, hiy); \n\
    loz = hix*hiy; \n\
    if(hiz || loz || mul_hi(lox, hiy) || mul_hi(hix, loy)){ \n\
        overflow = 0xffffffff; \n\
    }\n\
    else{ \n\
        mulH0L1L = hix*loy; \n\
        mulH1L0L = lox*hiy; \n\
        mulHighL0L1 = mul_hi(lox, loy); \n\
        notV = ~mulH0L1L; \n\
        if(mulHighL0L1 >= notV){ \n\
           overflow = 0xffffffff; \n\
        } \n\
        mulHighL0L1 += mulH0L1L; \n\
        \
        notV = ~mulH1L0L; \n\
        if(mulHighL0L1 >= notV){ \n\
            overflow = 0xffffffff; \n\
        }\n\
        hiz = mulHighL0L1 + mulH1L0L; \n\
        loz = lox * loy;\n\
    }\n\
    if(overflow){\n\
        loz = overflow;\n\
        hiz = overflow;\n\
    }\n\
    viv_setlong(a, loz, hiz); \n\
    return a; \n\
} \n\
\
ulong viv_Mul_ulong(ulong x, ulong y)\n\
{\n\
    ulong r;\n\
    uint lox, loy, hix, hiy,loz, hiz;\n\
    lox = viv_getlonglo(x);\n\
    hix = viv_getlonghi(x);\n\
    loy = viv_getlonglo(y);\n\
    hiy = viv_getlonghi(y);\n\
\
    loz = lox*loy; \n\
    hiz = mul_hi(lox, loy); \n\
    hiz += hix*loy; \n\
    hiz += lox*hiy; \n\
    viv_setlong(r, loz, hiz); \n\
\
    return r;\n\
}\n\
\
ulong4  ulong_Mul(uint count, ulong4  x, ulong4  y) \
 { \
    ulong4 r;\n\
    switch(count)\n\
    {\n\
    case 4:\n\
        r.w = viv_Mul_ulong(x.w, y.w);\n\
    case 3:\n\
        r.z = viv_Mul_ulong(x.z, y.z);\n\
    case 2:\n\
        r.y = viv_Mul_ulong(x.y, y.y);\n\
    case 1:\n\
    default:\n\
        r.x = viv_Mul_ulong(x.x, y.x);\n\
        break;\n\
    }\n\
\
    return r; \n\
} \n"

#define _longulong_f2i_ulong \
"\
ulong viv_F2I_ulong(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f){ /*Overflow*/ \n\
        lox = 0x0; \n\
        hix = 0x0; \n\
    }\n\
    else if(fValue1 < -4294967296.f*4294967296.f/2.0f){ \n\
        lox = 0x0; \n\
        hix = 0x80000000; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = (unsigned int)absf; \n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(fValue1 < 0.0f){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
ulong4 ulong_F2I(uint count, float4 x) \n\
{ \n\
    ulong4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_ulong(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_ulong(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_ulong(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_ulong(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n\
\
ulong viv_F2I_ulong_sat(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f){ /*Overflow*/ \n\
        lox = 0xffffffff; \n\
        hix = 0xffffffff; \n\
    }\n\
    else if(fValue1 < 0.0f){ \n\
        lox = 0; \n\
        hix = 0; \n\
    }\n\
    else if(fValue1 < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_sat(fValue1); \n\
        fValue1 = 0.0f;\n\
    } \n\
    else{ \n\
        hexf = as_int(fValue1); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
ulong4 ulong_F2I_sat(uint count, float4 x) \n\
{ \n\
    ulong4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_ulong_sat(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_ulong_sat(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_ulong_sat(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_ulong_sat(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n\
\
ulong viv_F2I_ulong_sat_rte(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f){ /*Overflow*/ \n\
        lox = 0xffffffff; \n\
        hix = 0xffffffff; \n\
    }\n\
    else if(fValue1 < 0.0f){ \n\
        lox = 0; \n\
        hix = 0; \n\
    }\n\
    else if(fValue1 < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_sat_rte(fValue1); \n\
        fValue1 = 0.0f;\n\
    } \n\
    else{ \n\
        hexf = as_int(fValue1); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
ulong4 ulong_F2I_sat_rte(uint count, float4 x) \n\
{ \n\
    ulong4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_ulong_sat_rte(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_ulong_sat_rte(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_ulong_sat_rte(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_ulong_sat_rte(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n\
\
ulong viv_F2I_ulong_sat_rtp(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    if(fValue1 >=  4294967296.f*4294967296.f){ /*Overflow*/ \n\
        lox = 0xffffffff; \n\
        hix = 0xffffffff; \n\
    }\n\
    else if(fValue1 < 0.0f){ \n\
        lox = 0; \n\
        hix = 0; \n\
    }\n\
    else if(fValue1 < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_sat_rtp(fValue1); \n\
        fValue1 = 0.0f;\n\
    } \n\
    else{ \n\
        hexf = as_int(fValue1); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
ulong4 ulong_F2I_sat_rtp(uint count, float4 x) \n\
{ \n\
    ulong4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_ulong_sat_rtp(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_ulong_sat_rtp(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_ulong_sat_rtp(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_ulong_sat_rtp(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n\
\
ulong viv_F2I_ulong_sat_rtn(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    if(fValue1 >=  4294967296.f*4294967296.f){ /*Overflow*/ \n\
        lox = 0xffffffff; \n\
        hix = 0xffffffff; \n\
    }\n\
    else if(fValue1 < 0.0f){ \n\
        lox = 0; \n\
        hix = 0; \n\
    }\n\
    else if(fValue1 < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_sat_rtn(fValue1); \n\
        fValue1 = 0.0f;\n\
    } \n\
    else{ \n\
        hexf = as_int(fValue1); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
ulong4 ulong_F2I_sat_rtn(uint count, float4 x) \n\
{ \n\
    ulong4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_ulong_sat_rtn(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_ulong_sat_rtn(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_ulong_sat_rtn(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_ulong_sat_rtn(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n\
\
ulong viv_F2I_ulong_rte(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f){ /*Overflow*/ \n\
        lox = 0x0; \n\
        hix = 0x0; \n\
    }\n\
    else if(fValue1 < -4294967296.f*4294967296.f/2.0f){ \n\
        lox = 0x0; \n\
        hix = 0x80000000; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_rte(absf); \n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(fValue1 < 0.0f){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
ulong4 ulong_F2I_rte(uint count, float4 x) \n\
{ \n\
    ulong4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_ulong_rte(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_ulong_rte(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_ulong_rte(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_ulong_rte(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n\
\
ulong viv_F2I_ulong_rtp(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f){ /*Overflow*/ \n\
        lox = 0x0; \n\
        hix = 0x0; \n\
    }\n\
    else if(fValue1 < -4294967296.f*4294967296.f/2.0f){ \n\
        lox = 0x0; \n\
        hix = 0x80000000; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_rtp(fValue1); \n\
        fValue1 = 0.0f;\n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(fValue1 < 0.0f){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
ulong4 ulong_F2I_rtp(uint count, float4 x) \n\
{ \n\
    ulong4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_ulong_rtp(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_ulong_rtp(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_ulong_rtp(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_ulong_rtp(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n\
\
ulong viv_F2I_ulong_rtn(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f){ /*Overflow*/ \n\
        lox = 0x0; \n\
        hix = 0x0; \n\
    }\n\
    else if(fValue1 < -4294967296.f*4294967296.f/2.0f){ \n\
        lox = 0x0; \n\
        hix = 0x80000000; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_rtn(absf); \n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(fValue1 < 0.0f){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
ulong4 ulong_F2I_rtn(uint count, float4 x) \n\
{ \n\
    ulong4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_ulong_rtn(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_ulong_rtn(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_ulong_rtn(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_ulong_rtn(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
}\n"

#define _longulong_f2i_long \
"\
long viv_F2I_long(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f/2.0f || fValue1 < -4294967296.f*4294967296.f/2.0f){ /*Overflow*/ \n\
        lox = 0; \n\
        hix = 0x80000000; \n\
        fValue1 = 0; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = (unsigned int)absf; \n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(fValue1 < 0.0f){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
long4 long_F2I(uint count, float4 x) \n\
{ \n\
    long4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_long(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_long(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_long(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_long(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n \
\
long viv_F2I_long_sat(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f/2.0f ){ /*Overflow*/ \n\
        lox = 0xffffffff; \n\
        hix = 0x7fffffff; \n\
        fValue1 = 0; \n\
    }\n\
    else if(fValue1 < -4294967296.f*4294967296.f/2.0f){ /*Overflow*/ \n\
        lox = 0; \n\
        hix = 0x80000000; \n\
        fValue1 = 0; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_sat(absf); \n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(fValue1 < 0.0f){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
long4 long_F2I_sat(uint count, float4 x) \n\
{ \n\
    long4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_long_sat(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_long_sat(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_long_sat(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_long_sat(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n \
\
long viv_F2I_long_sat_rte(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f/2.0f ){ /*Overflow*/ \n\
        lox = 0xffffffff; \n\
        hix = 0x7fffffff; \n\
        fValue1 = 0; \n\
    }\n\
    else if(fValue1 < -4294967296.f*4294967296.f/2.0f){ /*Overflow*/ \n\
        lox = 0; \n\
        hix = 0x80000000; \n\
        fValue1 = 0; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_sat_rte(absf); \n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(fValue1 < 0.0f){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
long4 long_F2I_sat_rte(uint count, float4 x) \n\
{ \n\
    long4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_long_sat_rte(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_long_sat_rte(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_long_sat_rte(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_long_sat_rte(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n\
\
long viv_F2I_long_sat_rtp(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    int isNeg = fValue1<0.0f ? 1:0;\n\
    if(fValue1 >=  4294967296.f*4294967296.f/2.0f ){ /*Overflow*/ \n\
        lox = 0xffffffff; \n\
        hix = 0x7fffffff; \n\
        fValue1 = 0; \n\
    }\n\
    else if(fValue1 < -4294967296.f*4294967296.f/2.0f){ /*Overflow*/ \n\
        lox = 0; \n\
        hix = 0x80000000; \n\
        fValue1 = 0; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        switch(isNeg)\n\
        {\n\
            case 1:\n\
                hix = 0; \n\
                lox = convert_uint_sat_rtn(absf); \n\
                break;\n\
            case 0:\n\
            default:\n\
                hix = 0; \n\
                lox = convert_uint_sat_rtp(absf); \n\
                break;\n\
        }\n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(isNeg){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
long4 long_F2I_sat_rtp(uint count, float4 x) \n\
{ \n\
    long4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_long_sat_rtp(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_long_sat_rtp(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_long_sat_rtp(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_long_sat_rtp(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n \
long viv_F2I_long_sat_rtn(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    int isNeg = fValue1<0.0f ? 1:0;\n\
    if(fValue1 >=  4294967296.f*4294967296.f/2.0f ){ /*Overflow*/ \n\
        lox = 0xffffffff; \n\
        hix = 0x7fffffff; \n\
        fValue1 = 0; \n\
    }\n\
    else if(fValue1 < -4294967296.f*4294967296.f/2.0f){ /*Overflow*/ \n\
        lox = 0; \n\
        hix = 0x80000000; \n\
        fValue1 = 0; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        switch(isNeg)\n\
        {\n\
            case 1:\n\
                hix = 0; \n\
                lox = convert_uint_sat_rtp(absf); \n\
                break;\n\
            case 0:\n\
            default:\n\
                hix = 0; \n\
                lox = convert_uint_sat_rtn(absf); \n\
                break;\n\
        }\n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(isNeg){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
long4 long_F2I_sat_rtn(uint count, float4 x) \n\
{ \n\
    long4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_long_sat_rtn(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_long_sat_rtn(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_long_sat_rtn(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_long_sat_rtn(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n \
long viv_F2I_long_rte(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    if(fValue1 >=  4294967296.f*4294967296.f/2.0f || fValue1 < -4294967296.f*4294967296.f/2.0f){ /*Overflow*/ \n\
        lox = 0; \n\
        hix = 0x80000000; \n\
        fValue1 = 0; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        hix = 0; \n\
        lox = convert_uint_rte(absf); \n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(fValue1 < 0.0f){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
long4 long_F2I_rte(uint count, float4 x) \n\
{ \n\
    long4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_long_rte(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_long_rte(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_long_rte(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_long_rte(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n \
long viv_F2I_long_rtp(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    int isNeg = fValue1<0.0f ? 1:0;\n\
    if(fValue1 >=  4294967296.f*4294967296.f/2.0f || fValue1 < -4294967296.f*4294967296.f/2.0f){ /*Overflow*/ \n\
        lox = 0; \n\
        hix = 0x80000000; \n\
        fValue1 = 0; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        switch(isNeg)\n\
        {\n\
            case 1:\n\
                hix = 0; \n\
                lox = convert_uint_rtn(absf); \n\
                break;\n\
            case 0:\n\
            default:\n\
                hix = 0; \n\
                lox = convert_uint_rtp(absf); \n\
                break;\n\
        }\n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(isNeg){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
long4 long_F2I_rtp(uint count, float4 x) \n\
{ \n\
    long4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_long_rtp(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_long_rtp(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_long_rtp(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_long_rtp(x.x);\n\
        break;\n\
    default: break;\n\
    }\n\
    return r;\n\
} \n \
long viv_F2I_long_rtn(float fValue1)\n\
{\n\
    uint lox, hix; \n\
    int  hexf, exp; \n\
    long a; \n\
    float absf = fabs(fValue1); \n\
    int isNeg = fValue1<0.0f ? 1:0;\n\
    if(fValue1 >=  4294967296.f*4294967296.f/2.0f || fValue1 < -4294967296.f*4294967296.f/2.0f){ /*Overflow*/ \n\
        lox = 0; \n\
        hix = 0x80000000; \n\
        fValue1 = 0; \n\
    }\n\
    else if(absf < 65536.*65536. ){ /*fValue < 2^32*/ \n\
        switch(isNeg)\n\
        {\n\
            case 1:\n\
                hix = 0; \n\
                lox = convert_uint_rtp(absf); \n\
                break;\n\
            case 0:\n\
            default:\n\
                hix = 0; \n\
                lox = convert_uint_rtn(absf); \n\
                break;\n\
        }\n\
    } \n\
    else{ \n\
        hexf = as_int(absf); \n\
        exp = (hexf>> 23 ) - 127 - 23  ;\n\
        lox = (hexf & 0x7fffff) | 0x800000; /*mantissa*/ \n\
        if(exp >= 32){ \n\
            hix = lox << (exp - 32); \n\
            lox = 0; \n\
        } \n\
        else{ \n\
            hix = lox >> (32 - exp); \n\
            lox <<= exp; \n\
        } \n\
    } \n\
    if(isNeg){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    viv_setlong(a, lox, hix); \n\
    return a; \n\
}\n\
\
long4 long_F2I_rtn(uint count, float4 x) \n\
{ \n\
    long4 r = 0L;\n\
    switch (count)\n\
    {\n\
    case 4:\n\
        r.w = viv_F2I_long_rtn(x.w);\n\
    case 3:\n\
        r.z = viv_F2I_long_rtn(x.z);\n\
    case 2:\n\
        r.y = viv_F2I_long_rtn(x.y);\n\
    case 1:\n\
        r.x = viv_F2I_long_rtn(x.x);\n\
    default:\n\
        break;\n\
    }\n\
    return r;\n\
} \n"

#define _longulong_i2f \
"\
float viv_I2F_long(long x)\n\
{\n\
    float fValue1; \n\
    int leadOne, leadOne31, leadOneP1;\n\
    unsigned int lox, hix, sign; \n\
    lox = viv_getlonglo(x); \n\
    hix = viv_getlonghi(x); \n\
    sign = hix & 0x80000000; \n\
    if(sign){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    fValue1 = (float)hix; \n\
    if(fValue1 == 0){ \n\
        fValue1 = (float) lox; \n\
    }\n\
    else{ \n\
        leadOne = as_int(fValue1); \n\
        leadOne = (leadOne >> 23 ) - 127; /*leading one position on hix*/ \n\
        leadOne31 = (31 - leadOne); \n\
        leadOneP1 = leadOne + 1; \n\
        hix <<= leadOne31; \n\
        if(leadOne31){ \n\
            hix |= lox >> (leadOneP1) ; \n\
            lox <<= leadOne31; \n\
        } \n\
        if(lox ) /*sticky bit*/ \n\
            hix |= 1; \n\
        fValue1 = (float) hix; \n\
        leadOne = as_int(fValue1); \n\
        leadOne += (leadOneP1)<<23; \n\
        fValue1 = as_float(leadOne); \n\
    } \n\
    if(sign) \n\
        fValue1 = -fValue1; \n\
    return fValue1; \n\
}\n\
\
float4 long_I2F(uint count, long4 x) \n\
{ \n\
    float4 r = 0.0f;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            r.w = viv_I2F_long(x.w);\n\
        case 3:\n\
            r.z = viv_I2F_long(x.z);\n\
        case 2:\n\
            r.y = viv_I2F_long(x.y);\n\
        case 1:\n\
            r.x = viv_I2F_long(x.x);\n\
        default: break;\n\
    }\n\
    return r;\n\
} \n\
\
float viv_I2F_long_rtz(long x)\n\
{\n\
    float fValue1; \n\
    int leadOne, leadOne31, leadOneP1;\n\
    unsigned int lox, hix, sign; \n\
    lox = viv_getlonglo(x); \n\
    hix = viv_getlonghi(x); \n\
    sign = hix & 0x80000000; \n\
    if(sign){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    fValue1 = convert_float_rtz(hix); \n\
    if(fValue1 == 0){ \n\
        fValue1 = convert_float_rtz(lox); \n\
    }\n\
    else{ \n\
        leadOne = as_int(fValue1); \n\
        leadOne = (leadOne >> 23 ) - 127; /*leading one position on hix*/ \n\
        leadOne31 = (31 - leadOne); \n\
        leadOneP1 = leadOne + 1; \n\
        hix <<= leadOne31; \n\
        if(leadOne31){ \n\
            hix |= lox >> (leadOneP1) ; \n\
        } \n\
        fValue1 = convert_float_rtz(hix); \n\
        leadOne = as_int(fValue1); \n\
        leadOne += (leadOneP1)<<23; \n\
        fValue1 = as_float(leadOne); \n\
    } \n\
    if(sign) \n\
        fValue1 = -fValue1; \n\
    return fValue1; \n\
}\n\
\
float4 long_I2F_rtz(uint count, long4 x) \n\
{ \n\
    float4 r = 0.0f;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            r.w = viv_I2F_long_rtz(x.w);\n\
        case 3:\n\
            r.z = viv_I2F_long_rtz(x.z);\n\
        case 2:\n\
            r.y = viv_I2F_long_rtz(x.y);\n\
        case 1:\n\
            r.x = viv_I2F_long_rtz(x.x);\n\
        default: break;\n\
    }\n\
    return r;\n\
} \n\
\
float viv_I2F_long_rtp(long x) /*Round up, always up, negative value, round to zero mode, positive value, check and round up*/\n\
{\n\
    float fValue1; \n\
    int leadOne, leadOne31, leadOneP1;\n\
    unsigned int lox, hix, sign; \n\
    lox = viv_getlonglo(x); \n\
    hix = viv_getlonghi(x); \n\
    sign = hix & 0x80000000; \n\
    if(sign){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    fValue1 = convert_float_rtz(hix); /*use RTZ rounding mode*/ \n\
    if(fValue1 == 0){ \n\
        fValue1 = convert_float_rtz(lox); /*use RTZ rounding mode*/\n\
        if((unsigned int)fValue1 < lox && sign == 0){ /*need round up*/\n\
            leadOne = as_int(fValue1); \n\
            leadOne ++; \n\
            fValue1 = as_float(leadOne); \n\
        } \n\
    }\n\
    else{ \n\
        leadOne = as_int(fValue1); \n\
        leadOne = (leadOne >> 23 ) - 127; /*leading one position on hix*/ \n\
        leadOne31 = (31 - leadOne); \n\
        leadOneP1 = leadOne + 1; \n\
        hix <<= leadOne31; \n\
        if(leadOne31){ \n\
            hix |= lox >> (leadOneP1) ; \n\
            lox <<= leadOne31; \n\
        } \n\
        if(lox ) /*sticky bit*/ \n\
            hix |= 1; \n\
        fValue1 = convert_float_rtz(hix); \n\
        if((unsigned int)fValue1 < hix && sign == 0){ /*need round up*/\n\
            leadOne = as_int(fValue1); \n\
            leadOne ++; \n\
            fValue1 = as_float(leadOne); \n\
        } \n\
        leadOne = as_int(fValue1); \n\
        leadOne += (leadOneP1)<<23; \n\
        fValue1 = as_float(leadOne); \n\
    } \n\
    if(sign) \n\
        fValue1 = -fValue1; \n\
    return fValue1; \n\
}\n\
float4 long_I2F_rtp(uint count, long4 x) \n\
{ \n\
    float4 r = 0.0f;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            r.w = viv_I2F_long_rtp(x.w);\n\
        case 3:\n\
            r.z = viv_I2F_long_rtp(x.z);\n\
        case 2:\n\
            r.y = viv_I2F_long_rtp(x.y);\n\
        case 1:\n\
            r.x = viv_I2F_long_rtp(x.x);\n\
        default: break;\n\
    }\n\
    return r;\n\
} \n\
\
float viv_I2F_long_rtn(long x) /*Round down, always down, positive same as rtz, negative as absolute value round up*/\n\
{\n\
    float fValue1; \n\
    int leadOne, leadOne31, leadOneP1;\n\
    unsigned int lox, hix, sign; \n\
    lox = viv_getlonglo(x); \n\
    hix = viv_getlonghi(x); \n\
    sign = hix & 0x80000000; \n\
    if(sign){ \n\
        lox = 0 - lox; \n\
        hix = ~hix; \n\
        if(lox == 0) \n\
            hix++; \n\
    } \n\
    fValue1 = convert_float_rtz(hix); /*use RTZ rounding mode*/ \n\
    if(fValue1 == 0){ \n\
        fValue1 = convert_float_rtz(lox); /*use RTZ rounding mode*/\n\
        if((unsigned int)fValue1 < lox && sign){ /*need round up*/\n\
            leadOne = as_int(fValue1); \n\
            leadOne ++; \n\
            fValue1 = as_float(leadOne); \n\
        } \n\
    }\n\
    else{ \n\
        leadOne = as_int(fValue1); \n\
        leadOne = (leadOne >> 23 ) - 127; /*leading one position on hix*/ \n\
        leadOne31 = (31 - leadOne); \n\
        leadOneP1 = leadOne + 1; \n\
        hix <<= leadOne31; \n\
        if(leadOne31){ \n\
            hix |= lox >> (leadOneP1) ; \n\
            lox <<= leadOne31; \n\
        } \n\
        if(lox ) /*sticky bit*/ \n\
            hix |= 1; \n\
        fValue1 = convert_float_rtz(hix); \n\
        if((unsigned int)fValue1 < hix && sign){ /*need round up*/\n\
            leadOne = as_int(fValue1); \n\
            leadOne ++; \n\
            fValue1 = as_float(leadOne); \n\
        } \n\
        leadOne = as_int(fValue1); \n\
        leadOne += (leadOneP1)<<23; \n\
        fValue1 = as_float(leadOne); \n\
    } \n\
    if(sign) \n\
        fValue1 = -fValue1; \n\
    return fValue1; \n\
}\n\
\
float4 long_I2F_rtn(uint count, long4 x) \n\
{ \n\
    float4 r = 0.0f;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            r.w = viv_I2F_long_rtn(x.w);\n\
        case 3:\n\
            r.z = viv_I2F_long_rtn(x.z);\n\
        case 2:\n\
            r.y = viv_I2F_long_rtn(x.y);\n\
        case 1:\n\
            r.x = viv_I2F_long_rtn(x.x);\n\
        default: break;\n\
    }\n\
    return r;\n\
} \n\
\
float viv_I2F_ulong(ulong x)\n\
{\n\
    float fValue1; \n\
    int leadOne, leadOne31, leadOneP1;\n\
    uint lox, hix; \n\
    lox = viv_getlonglo(x); \n\
    hix = viv_getlonghi(x); \n\
    fValue1 = (float)hix; \n\
    if(fValue1 == 0){ \n\
        fValue1 = (float) lox; \n\
    }\n\
    else if(fValue1 == 4294967296.f){ /*Full precision, may from 0xffffff80*/ \n\
        fValue1 = 4294967296.f*4294967296.f; \n\
    } \n\
    else{ \n\
        leadOne = as_int(fValue1); \n\
        leadOne = (leadOne >> 23 ) - 127; /*leading one position on hix*/ \n\
        leadOne31 = (31 - leadOne); \n\
        leadOneP1 = leadOne + 1; \n\
        hix <<= leadOne31; \n\
        if(leadOne31){ \n\
            hix |= lox >> (leadOneP1) ; \n\
            lox <<= leadOne31; \n\
        } \n\
        if(lox ) /*sticky bit*/ \n\
            hix |= 1; \n\
        fValue1 = (float) hix; \n\
        leadOne = as_int(fValue1); \n\
        leadOne += (leadOneP1)<<23; \n\
        fValue1 = as_float(leadOne); \n\
    } \n\
    return fValue1; \n\
}\n\
\
float4 ulong_I2F(uint count, ulong4 x) \n\
{ \n\
    float4 r = 0.0f;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            r.w = viv_I2F_ulong(x.w);\n\
        case 3:\n\
            r.z = viv_I2F_ulong(x.z);\n\
        case 2:\n\
            r.y = viv_I2F_ulong(x.y);\n\
        case 1:\n\
            r.x = viv_I2F_ulong(x.x);\n\
        break;\n\
        default:   break;\n\
    }\n\
    return r;\n\
} \n\
\
float viv_I2F_ulong_rtz(ulong x)\n\
{\n\
    float fValue1; \n\
    int leadOne, leadOne31, leadOneP1;\n\
    uint lox, hix; \n\
    lox = viv_getlonglo(x); \n\
    hix = viv_getlonghi(x); \n\
    fValue1 = convert_float_rtz(hix); \n\
    if(fValue1 == 0){ \n\
        fValue1 = convert_float_rtz(lox); \n\
    }\n\
    else if(fValue1 == 4294967296.f){ /*Full precision, may from 0xffffff80*/ \n\
        fValue1 = 4294967296.f*4294967296.f; \n\
    } \n\
    else{ \n\
        leadOne = as_int(fValue1); \n\
        leadOne = (leadOne >> 23 ) - 127; /*leading one position on hix*/ \n\
        leadOne31 = (31 - leadOne); \n\
        leadOneP1 = leadOne + 1; \n\
        hix <<= leadOne31; \n\
        if(leadOne31){ \n\
            hix |= lox >> (leadOneP1) ; \n\
        } \n\
        fValue1 = convert_float_rtz(hix); \n\
        leadOne = as_int(fValue1); \n\
        leadOne += (leadOneP1)<<23; \n\
        fValue1 = as_float(leadOne); \n\
    } \n\
    return fValue1; \n\
}\n\
\
float4 ulong_I2F_rtz(uint count, ulong4 x) \n\
{ \n\
    float4 r = 0.0f;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            r.w = viv_I2F_ulong_rtz(x.w);\n\
        case 3:\n\
            r.z = viv_I2F_ulong_rtz(x.z);\n\
        case 2:\n\
            r.y = viv_I2F_ulong_rtz(x.y);\n\
        case 1:\n\
            r.x = viv_I2F_ulong_rtz(x.x);\n\
        break;\n\
        default:   break;\n\
    }\n\
    return r;\n\
} \n\
float4 ulong_I2F_rtn(uint count, ulong4 x) \n\
{ \n\
    float4 r = 0.0f;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            r.w = viv_I2F_ulong_rtz(x.w);\n\
        case 3:\n\
            r.z = viv_I2F_ulong_rtz(x.z);\n\
        case 2:\n\
            r.y = viv_I2F_ulong_rtz(x.y);\n\
        case 1:\n\
            r.x = viv_I2F_ulong_rtz(x.x);\n\
        break;\n\
        default:   break;\n\
    }\n\
    return r;\n\
} \n\
float viv_I2F_ulong_rtp(ulong x)/*need round up, always*/\n\
{\n\
    float fValue1; \n\
    int leadOne, leadOne31, leadOneP1;\n\
    uint lox, hix; \n\
    lox = viv_getlonglo(x); \n\
    hix = viv_getlonghi(x); \n\
    fValue1 = (float)hix; \n\
    if(fValue1 == 0){ \n\
        fValue1 = (float) lox; \n\
        if((unsigned int)fValue1 < lox){ /*need round up*/\n\
            leadOne = as_int(fValue1); \n\
            leadOne ++; \n\
            fValue1 = as_float(leadOne); \n\
        } \n\
    }\n\
    else if(fValue1 == 4294967296.f){ /*Full precision, may from 0xffffff80*/ \n\
        fValue1 = 4294967296.f*4294967296.f; \n\
    } \n\
    else{ \n\
        leadOne = as_int(fValue1); \n\
        leadOne = (leadOne >> 23 ) - 127; /*leading one position on hix*/ \n\
        leadOne31 = (31 - leadOne); \n\
        leadOneP1 = leadOne + 1; \n\
        hix <<= leadOne31; \n\
        if(leadOne31){ \n\
            hix |= lox >> (leadOneP1) ; \n\
            lox <<= leadOne31; \n\
        } \n\
        if(lox ) /*sticky bit*/ \n\
            hix |= 1; \n\
        fValue1 = (float) hix; \n\
        if((unsigned int)fValue1 < hix){ /*need round up*/\n\
            leadOne = as_int(fValue1); \n\
            leadOne ++; \n\
            fValue1 = as_float(leadOne); \n\
        } \n\
        leadOne = as_int(fValue1); \n\
        leadOne += (leadOneP1)<<23; \n\
        fValue1 = as_float(leadOne); \n\
    } \n\
    return fValue1; \n\
}\n\
float4 ulong_I2F_rtp(uint count, ulong4 x) \n\
{ \n\
    float4 r = 0.0f;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            r.w = viv_I2F_ulong_rtp(x.w);\n\
        case 3:\n\
            r.z = viv_I2F_ulong_rtp(x.z);\n\
        case 2:\n\
            r.y = viv_I2F_ulong_rtp(x.y);\n\
        case 1:\n\
            r.x = viv_I2F_ulong_rtp(x.x);\n\
        break;\n\
        default:   break;\n\
    }\n\
    return r;\n\
} \n"

#define _longulong_not_equal \
"\
int long_NotEqual(uint count, long src0, long src1)\n\
{\n\
    uint lo0, lo1, hi0, hi1;\n\
    lo0 = viv_getlonglo(src0);\n\
    hi0 = viv_getlonghi(src0);\n\
    lo1 = viv_getlonglo(src1);\n\
    hi1 = viv_getlonghi(src1);\n\
    return ((lo0 != lo1) && (hi0 != hi1));\n\
}\n\
"
#define _longulong_greater_ulong \
"\
int ulong_Greater(ulong src0, ulong src1)\n\
{\n\
    uint lo0, lo1, hi0, hi1;\n\
    int result = 1;\n\
    lo0 = viv_getlonglo(src0);\n\
    hi0 = viv_getlonghi(src0);\n\
    lo1 = viv_getlonglo(src1);\n\
    hi1 = viv_getlonghi(src1);\n\
    if (hi0 < hi1)\n\
    {\n\
        result = 0;\n\
    }\n\
    else if (hi0 == hi1)\n\
    {\n\
        if (lo0 <= lo1)\n\
        {\n\
            result = 0;\n\
        }\n\
    }\n\
\
    return result;\n\
}\n\
"

/* longulong to char uchar convert */
#define _long2char_convert_sat \
"\
char4 long_2charConvert_sat(uint count, long4 src)\n \
{\n \
    int templo = 0, temphi = 0;\n \
    char4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 && (templo < 0 || templo > CHAR_MAX)) || (temphi > 0))\n \
            {\n \
                result.w = CHAR_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && (templo < CHAR_MIN || templo >= 0)) || (temphi < -1))\n \
            {\n \
                result.w = CHAR_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.w = (char)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 && (templo < 0 || templo > CHAR_MAX)) || (temphi > 0))\n \
            {\n \
                result.z = CHAR_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && (templo < CHAR_MIN || templo >= 0)) || (temphi < -1))\n \
            {\n \
                result.z = CHAR_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.z = (char)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 && (templo < 0 || templo > CHAR_MAX)) || (temphi > 0))\n \
            {\n \
                result.y = CHAR_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && (templo < CHAR_MIN || templo >= 0)) || (temphi < -1))\n \
            {\n \
                result.y = CHAR_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.y = (char)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 && (templo < 0 || templo > CHAR_MAX)) || (temphi > 0))\n \
            {\n \
                result.x = CHAR_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && (templo < CHAR_MIN || templo >= 0)) || (temphi < -1))\n \
            {\n \
                result.x = CHAR_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.x = (char)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _ulong2char_convert_sat \
"\
char4 ulong_2charConvert_sat(uint count, ulong4 src)\n \
{\n \
    uint templo = 0, temphi = 0;\n \
    char4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 &&  templo > CHAR_MAX) || (temphi > 0))\n \
            {\n \
                result.w = CHAR_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.w = (char)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 &&  templo > CHAR_MAX) || (temphi > 0))\n \
            {\n \
                result.z = CHAR_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.z = (char)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 &&  templo > CHAR_MAX) || (temphi > 0))\n \
            {\n \
                result.y = CHAR_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.y = (char)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 &&  templo > CHAR_MAX) || (temphi > 0))\n \
            {\n \
                result.x = CHAR_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.x = (char)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"
#define _long2uchar_convert_sat \
"\
uchar4 long_2ucharConvert_sat(uint count, long4 src)\n \
{\n \
    int templo = 0, temphi = 0;\n \
    uchar4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 && (templo < 0 || templo > UCHAR_MAX)) || (temphi > 0))\n \
            {\n \
                result.w = UCHAR_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.w = 0;\n \
            }\n \
            else\n \
            {\n \
                result.w = (uchar)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 && (templo < 0 || templo > UCHAR_MAX)) || (temphi > 0))\n \
            {\n \
                result.z = UCHAR_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.z = 0;\n \
            }\n \
            else\n \
            {\n \
                result.z = (uchar)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 && (templo < 0 || templo > UCHAR_MAX)) || (temphi > 0))\n \
            {\n \
                result.y = UCHAR_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.y = 0;\n \
            }\n \
            else\n \
            {\n \
                result.y = (uchar)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 && (templo < 0 || templo > UCHAR_MAX)) || (temphi > 0))\n \
            {\n \
                result.x = UCHAR_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.x = 0;\n \
            }\n \
            else\n \
            {\n \
                result.x = (uchar)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _ulong2uchar_convert_sat \
"\
uchar4 ulong_2ucharConvert_sat(uint count, ulong4 src)\n \
{\n \
    uint templo = 0, temphi = 0;\n \
    uchar4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 &&  templo > UCHAR_MAX) || (temphi > 0))\n \
            {\n \
                result.w = UCHAR_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.w = (uchar)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 &&  templo > UCHAR_MAX) || (temphi > 0))\n \
            {\n \
                result.z = UCHAR_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.z = (uchar)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 &&  templo > UCHAR_MAX) || (temphi > 0))\n \
            {\n \
                result.y = UCHAR_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.y = (uchar)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 &&  templo > UCHAR_MAX) || (temphi > 0))\n \
            {\n \
                result.x = UCHAR_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.x = (uchar)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

/* long ulong to short unsigned short convert */
#define _long2short_convert_sat \
"\
short4 long_2shortConvert_sat(uint count, long4 src)\n \
{\n \
    int templo = 0, temphi = 0;\n \
    short4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 && (templo < 0 || templo > SHRT_MAX)) || (temphi > 0))\n \
            {\n \
                result.w = SHRT_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && (templo < SHRT_MIN || templo >= 0)) || (temphi < -1))\n \
            {\n \
                result.w = SHRT_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.w = (short)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 && (templo < 0 || templo > SHRT_MAX)) || (temphi > 0))\n \
            {\n \
                result.z = SHRT_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && (templo < SHRT_MIN || templo >= 0)) || (temphi < -1))\n \
            {\n \
                result.z = SHRT_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.z = (short)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 && (templo < 0 || templo > SHRT_MAX)) || (temphi > 0))\n \
            {\n \
                result.y = SHRT_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && (templo < SHRT_MIN || templo >= 0)) || (temphi < -1))\n \
            {\n \
                result.y = SHRT_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.y = (short)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 && (templo < 0 || templo > SHRT_MAX)) || (temphi > 0))\n \
            {\n \
                result.x = SHRT_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && (templo < SHRT_MIN || templo >= 0)) || (temphi < -1))\n \
            {\n \
                result.x = SHRT_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.x = (short)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _ulong2short_convert_sat \
"\
short4 ulong_2shortConvert_sat(uint count, ulong4 src)\n \
{\n \
    uint templo = 0, temphi = 0;\n \
    short4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 &&  templo > SHRT_MAX) || (temphi > 0))\n \
            {\n \
                result.w = SHRT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.w = (short)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 &&  templo > SHRT_MAX) || (temphi > 0))\n \
            {\n \
                result.z = SHRT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.z = (short)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 &&  templo > SHRT_MAX) || (temphi > 0))\n \
            {\n \
                result.y = SHRT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.y = (short)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 &&  templo > SHRT_MAX) || (temphi > 0))\n \
            {\n \
                result.x = SHRT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.x = (short)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"
#define _long2ushort_convert_sat \
"\
ushort4 long_2ushortConvert_sat(uint count, long4 src)\n \
{\n \
    int templo = 0, temphi = 0;\n \
    ushort4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 && (templo < 0 || templo > USHRT_MAX)) || (temphi > 0))\n \
            {\n \
                result.w = USHRT_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.w = 0;\n \
            }\n \
            else\n \
            {\n \
                result.w = (ushort)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 && (templo < 0 || templo > USHRT_MAX)) || (temphi > 0))\n \
            {\n \
                result.z = USHRT_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.z = 0;\n \
            }\n \
            else\n \
            {\n \
                result.z = (ushort)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 && (templo < 0 || templo > USHRT_MAX)) || (temphi > 0))\n \
            {\n \
                result.y = USHRT_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.y = 0;\n \
            }\n \
            else\n \
            {\n \
                result.y = (ushort)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 && (templo < 0 || templo > USHRT_MAX)) || (temphi > 0))\n \
            {\n \
                result.x = USHRT_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.x = 0;\n \
            }\n \
            else\n \
            {\n \
                result.x = (ushort)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _ulong2ushort_convert_sat \
"\
ushort4 ulong_2ushortConvert_sat(uint count, ulong4 src)\n \
{\n \
    uint templo = 0, temphi = 0;\n \
    ushort4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 &&  templo > USHRT_MAX) || (temphi > 0))\n \
            {\n \
                result.w = USHRT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.w = (ushort)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 &&  templo > USHRT_MAX) || (temphi > 0))\n \
            {\n \
                result.z = USHRT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.z = (ushort)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 &&  templo > USHRT_MAX) || (temphi > 0))\n \
            {\n \
                result.y = USHRT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.y = (ushort)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 &&  templo > USHRT_MAX) || (temphi > 0))\n \
            {\n \
                result.x = USHRT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.x = (ushort)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

/* long ulong to int uint convert */
#define _long2int_convert_sat \
"\
int4 long_2intConvert_sat(uint count, long4 src)\n \
{\n \
    int templo = 0, temphi = 0;\n \
    int4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 && templo < 0) || (temphi > 0))\n \
            {\n \
                result.w = INT_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && templo >= 0) || (temphi < -1))\n \
            {\n \
                result.w = INT_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.w = (int)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 && templo < 0) || (temphi > 0))\n \
            {\n \
                result.z = INT_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && templo >= 0) || (temphi < -1))\n \
            {\n \
                result.z = INT_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.z = (int)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 && templo < 0) || (temphi > 0))\n \
            {\n \
                result.y = INT_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && templo >= 0) || (temphi < -1))\n \
            {\n \
                result.y = INT_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.y = (int)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 && templo < 0) || (temphi > 0))\n \
            {\n \
                result.x = INT_MAX;\n \
            }\n \
            else if((temphi == 0xffffffff && templo >= 0) || (temphi < -1))\n \
            {\n \
                result.x = INT_MIN;\n \
            }\n \
            else\n \
            {\n \
                result.x = (int)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _ulong2int_convert_sat \
"\
int4 ulong_2intConvert_sat(uint count, ulong4 src)\n \
{\n \
    uint templo = 0, temphi = 0;\n \
    int4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if((temphi == 0 &&  templo > INT_MAX) || (temphi > 0))\n \
            {\n \
                result.w = INT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.w = (int)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if((temphi == 0 &&  templo > INT_MAX) || (temphi > 0))\n \
            {\n \
                result.z = INT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.z = (int)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if((temphi == 0 &&  templo > INT_MAX) || (temphi > 0))\n \
            {\n \
                result.y = INT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.y = (int)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if((temphi == 0 &&  templo > INT_MAX) || (temphi > 0))\n \
            {\n \
                result.x = INT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.x = (int)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"
#define _long2uint_convert_sat \
"\
uint4 long_2uintConvert_sat(uint count, long4 src)\n \
{\n \
    int templo = 0, temphi = 0;\n \
    uint4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if(temphi > 0)\n \
            {\n \
                result.w = UINT_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.w = 0;\n \
            }\n \
            else\n \
            {\n \
                result.w = (uint)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if(temphi > 0)\n \
            {\n \
                result.z = UINT_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.z = 0;\n \
            }\n \
            else\n \
            {\n \
                result.z = (uint)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if(temphi > 0)\n \
            {\n \
                result.y = UINT_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.y = 0;\n \
            }\n \
            else\n \
            {\n \
                result.y = (uint)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if(temphi > 0)\n \
            {\n \
                result.x = UINT_MAX;\n \
            }\n \
            else if(temphi < 0)\n \
            {\n \
                result.x = 0;\n \
            }\n \
            else\n \
            {\n \
                result.x = (uint)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _ulong2uint_convert_sat \
"\
uint4 ulong_2uintConvert_sat(uint count, ulong4 src)\n \
{\n \
    uint templo = 0, temphi = 0;\n \
    uint4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if(temphi != 0)\n \
            {\n \
                result.w = UINT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.w = (uint)templo;\n \
            }\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if(temphi != 0)\n \
            {\n \
                result.z = UINT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.z = (uint)templo;\n \
            }\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if(temphi != 0)\n \
            {\n \
                result.y = UINT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.y = (uint)templo;\n \
            }\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if(temphi != 0)\n \
            {\n \
                result.x = UINT_MAX;\n \
            }\n \
            else\n \
            {\n \
                result.x = (uint)templo;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

/* long to ulong convert while _sat mode enable */
#define _long2ulong_convert_sat \
"\
ulong4 long_2ulongConvert_sat(uint count, long4 src)\n \
{\n \
    uint templo = 0, temphi = 0;\n \
    ulong4 result = 0L;\n \
    ulong r = 0L;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if(temphi > 0x7fffffff)\n \
            {\n \
                viv_setlong(r, as_uint(0), as_uint(0));\n \
            }\n \
            else\n \
            {\n \
                viv_setlong(r, templo, temphi);\n \
            }\n \
            result.w = r;\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if(temphi > 0x7fffffff)\n \
            {\n \
                viv_setlong(r, as_uint(0), as_uint(0));\n \
            }\n \
            else\n \
            {\n \
                viv_setlong(r, templo, temphi);\n \
            }\n \
            result.z = r;\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if(temphi > 0x7fffffff)\n \
            {\n \
                viv_setlong(r, as_uint(0), as_uint(0));\n \
            }\n \
            else\n \
            {\n \
                viv_setlong(r, templo, temphi);\n \
            }\n \
            result.y = r;\n \
       default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if(temphi > 0x7fffffff)\n \
            {\n \
                viv_setlong(r, as_uint(0), as_uint(0));\n \
            }\n \
            else\n \
            {\n \
                viv_setlong(r, templo, temphi);\n \
            }\n \
            result.x = r;\n \
           break;\n \
    }\n \
    return result;\n \
}\n \
"

/* ulong to long convert while _sat mode enable */
#define _ulong2long_convert_sat \
"\
long4 ulong_2longConvert_sat(uint count, ulong4 src)\n \
{\n \
    uint templo = 0, temphi = 0;\n \
    long4 result = 0L;\n \
    long r = 0L;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if(temphi > INT_MAX)\n \
            {\n \
                viv_setlong(r, as_uint(UINT_MAX), as_uint(INT_MAX));\n \
            }\n \
            else\n \
            {\n \
                viv_setlong(r, templo, temphi);\n \
            }\n \
            result.w = r;\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if(temphi > INT_MAX)\n \
            {\n \
                viv_setlong(r, as_uint(UINT_MAX), as_uint(INT_MAX));\n \
            }\n \
            else\n \
            {\n \
                viv_setlong(r, templo, temphi);\n \
            }\n \
            result.z = r;\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if(temphi > INT_MAX)\n \
            {\n \
                viv_setlong(r, as_uint(UINT_MAX), as_uint(INT_MAX));\n \
            }\n \
            else\n \
            {\n \
                viv_setlong(r, templo, temphi);\n \
            }\n \
            result.y = r;\n \
        default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if(temphi > INT_MAX)\n \
            {\n \
                viv_setlong(r, as_uint(UINT_MAX), as_uint(INT_MAX));\n \
            }\n \
            else\n \
            {\n \
                viv_setlong(r, templo, temphi);\n \
            }\n \
            result.x = r;\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"
/* 64 bit logic AND\OR\NOT - scalar */
#define _longulong_jmp_long \
"int long_jmp(long src) \n" \
"{ \n" \
"   int lowSrc, hiSrc, resultSrc; \n" \
"   lowSrc = viv_getlonglo(src); \n" \
"   hiSrc = viv_getlonghi(src); \n" \
"   resultSrc = lowSrc || hiSrc; \n" \
"   return resultSrc; \n" \
"} \n"

#define _longulong_jmp_ulong \
"int ulong_jmp(ulong src) \n" \
"{ \n" \
"   uint lowSrc, hiSrc; \n" \
"   int resultSrc; \n" \
"   lowSrc = viv_getlonglo(src); \n" \
"   hiSrc = viv_getlonghi(src); \n" \
"   resultSrc = lowSrc || hiSrc; \n" \
"   return resultSrc; \n" \
"} \n"

/* 64 bit logic AND\OR - vector */
#define _longulong_cmp_nz_long \
"long4 long_cmp_nz(uint count, long4 src) \n" \
"{\n" \
"   uint hiSrc, lowSrc, i; \n" \
"   long4 result = 0L; \n" \
"   long r; \n" \
"   uint zeroConst = 0; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(i=0; i<count; i++) \n" \
"   {\n" \
"       long data; \n" \
"       int temp; \n" \
"       if(i == 0) data = src.x; \n" \
"       else if(i == 1) data = src.y; \n" \
"       else if(i == 2) data = src.z; \n" \
"       else if(i == 3) data = src.w; \n" \
"       lowSrc = viv_getlonglo(data); \n" \
"       hiSrc = viv_getlonghi(data); \n" \
"       temp = hiSrc || lowSrc; \n" \
"       if(temp == 0) viv_setlong(r, zeroConst, zeroConst); \n" \
"       else viv_setlong(r, oneConst, oneConst); \n" \
"       if(i == 0) result.x = r; \n" \
"       else if(i == 1) result.y = r; \n" \
"       else if(i == 2) result.z = r; \n" \
"       else if(i == 3) result.w = r; \n" \
"   }\n" \
"   return result;\n" \
"}\n"

#define _longulong_cmp_nz_ulong \
"long4 ulong_cmp_nz(uint count, ulong4 src) \n" \
"{\n" \
"   uint hiSrc, lowSrc, i; \n" \
"   long4 result = 0L; \n" \
"   long r; \n" \
"   uint zeroConst = 0; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(i=0; i<count; i++) \n" \
"   {\n" \
"       long data; \n" \
"       int temp; \n" \
"       if(i == 0) data = src.x; \n" \
"       else if(i == 1) data = src.y; \n" \
"       else if(i == 2) data = src.z; \n" \
"       else if(i == 3) data = src.w; \n" \
"       lowSrc = viv_getlonglo(data); \n" \
"       hiSrc = viv_getlonghi(data); \n" \
"       temp = hiSrc || lowSrc; \n" \
"       if(temp == 0) viv_setlong(r, zeroConst, zeroConst); \n" \
"       else viv_setlong(r, oneConst, oneConst); \n" \
"       if(i == 0) result.x = r; \n" \
"       else if(i == 1) result.y = r; \n" \
"       else if(i == 2) result.z = r; \n" \
"       else if(i == 3) result.w = r; \n" \
"   }\n" \
"   return result;\n" \
"}\n"

/* 64 bit logic NOT - vector */
#define _longulong_cmp_z_long \
"long4 long_cmp_z(uint count, long4 src) \n" \
"{\n" \
"   uint hiSrc, lowSrc, i; \n" \
"   long4 result = 0L; \n" \
"   long r; \n" \
"   uint zeroConst = 0; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(i=0; i<count; i++) \n" \
"   {\n" \
"       long data; \n" \
"       int temp; \n" \
"       if(i == 0) data = src.x; \n" \
"       else if(i == 1) data = src.y; \n" \
"       else if(i == 2) data = src.z; \n" \
"       else if(i == 3) data = src.w; \n" \
"       lowSrc = viv_getlonglo(data); \n" \
"       hiSrc = viv_getlonghi(data); \n" \
"       temp = hiSrc || lowSrc; \n" \
"       if(temp == 0) viv_setlong(r, oneConst, oneConst); \n" \
"       else viv_setlong(r, zeroConst, zeroConst); \n" \
"       if(i == 0) result.x = r; \n" \
"       else if(i == 1) result.y = r; \n" \
"       else if(i == 2) result.z = r; \n" \
"       else if(i == 3) result.w = r; \n" \
"   }\n" \
"   return result;\n" \
"}\n"

#define _longulong_cmp_z_ulong \
"long4 ulong_cmp_z(uint count, ulong4 src) \n" \
"{\n" \
"   uint hiSrc, lowSrc, i; \n" \
"   long4 result = 0L; \n" \
"   long r; \n" \
"   uint zeroConst = 0; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(i=0; i<count; i++) \n" \
"   {\n" \
"       long data; \n" \
"       int temp; \n" \
"       if(i == 0) data = src.x; \n" \
"       else if(i == 1) data = src.y; \n" \
"       else if(i == 2) data = src.z; \n" \
"       else if(i == 3) data = src.w; \n" \
"       lowSrc = viv_getlonglo(data); \n" \
"       hiSrc = viv_getlonghi(data); \n" \
"       temp = hiSrc || lowSrc; \n" \
"       if(temp == 0) viv_setlong(r, oneConst, oneConst); \n" \
"       else viv_setlong(r, zeroConst, zeroConst); \n" \
"       if(i == 0) result.x = r; \n" \
"       else if(i == 1) result.y = r; \n" \
"       else if(i == 2) result.z = r; \n" \
"       else if(i == 3) result.w = r; \n" \
"   }\n" \
"   return result;\n" \
"}\n"

/* 64 bit ABS */
#define _longulong_abs \
"\
ulong4 long_abs(uint count, long4 src)\n \
{\n \
    uint templo = 0, temphi = 0, zeroConstantLow = 0;\n \
    ulong4 result = 0L;\n \
    ulong r = 0L;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if(temphi > 0x7fffffff)\n \
            {\n \
                temphi = 0xffffffff - temphi;\n \
                templo = zeroConstantLow - templo;\n \
            }\n \
            viv_setlong(r, templo, temphi);\n \
            result.w = r;\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if(temphi > 0x7fffffff)\n \
            {\n \
                temphi = 0xffffffff - temphi;\n \
                templo = zeroConstantLow - templo;\n \
            }\n \
            viv_setlong(r, templo, temphi);\n \
            result.z = r;\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if(temphi > 0x7fffffff)\n \
            {\n \
                temphi = 0xffffffff - temphi;\n \
                templo = zeroConstantLow - templo;\n \
            }\n \
            viv_setlong(r, templo, temphi);\n \
            result.y = r;\n \
       default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if(temphi > 0x7fffffff)\n \
            {\n \
                temphi = 0xffffffff - temphi;\n \
                templo = zeroConstantLow - templo;\n \
            }\n \
            viv_setlong(r, templo, temphi);\n \
            result.x = r;\n \
           break;\n \
    }\n \
    return result;\n \
}\n \
"
/* 64 bit clz */
#define _longulong_clz_long \
"\
long4 long_clz(uint count, long4 src)\n \
{\n \
    int templo = 0, temphi = 0, r =0;\n \
    long4 result = 0L;\n \
    long tmpr = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if(temphi == 0)\n \
            {\n \
                r = 8 + clz(templo);\n \
            }\n \
            else\n \
            {\n \
                r = clz(temphi);\n \
            }\n \
            viv_setlong(tmpr, as_uint(r), as_uint(0));\n \
            result.w = r;\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if(temphi == 0)\n \
            {\n \
                r = 8 + clz(templo);\n \
            }\n \
            else\n \
            {\n \
                r = clz(temphi);\n \
            }\n \
            viv_setlong(tmpr, as_uint(r), as_uint(0));\n \
            result.z = r;\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if(temphi == 0)\n \
            {\n \
                r = 8 + clz(templo);\n \
            }\n \
            else\n \
            {\n \
                r = clz(temphi);\n \
            }\n \
            viv_setlong(tmpr, as_uint(r), as_uint(0));\n \
            result.y = r;\n \
       default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if(temphi == 0)\n \
            {\n \
                r = 8 + clz(templo);\n \
            }\n \
            else\n \
            {\n \
                r = clz(temphi);\n \
            }\n \
            viv_setlong(tmpr, as_uint(r), as_uint(0));\n \
            result.x = r;\n \
           break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _longulong_clz_ulong \
"\
ulong4 ulong_clz(uint count, ulong4 src)\n \
{\n \
    uint templo = 0, temphi = 0, r =0;\n \
    ulong4 result = 0L;\n \
    ulong tmpr = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            templo = viv_getlonglo(src.w);\n \
            temphi = viv_getlonghi(src.w);\n \
            if(temphi == 0)\n \
            {\n \
                r = 8 + clz(templo);\n \
            }\n \
            else\n \
            {\n \
                r = clz(temphi);\n \
            }\n \
            viv_setlong(tmpr, as_uint(r), as_uint(0));\n \
            result.w = r;\n \
        case 3:\n \
            templo = viv_getlonglo(src.z);\n \
            temphi = viv_getlonghi(src.z);\n \
            if(temphi == 0)\n \
            {\n \
                r = 8 + clz(templo);\n \
            }\n \
            else\n \
            {\n \
                r = clz(temphi);\n \
            }\n \
            viv_setlong(tmpr, as_uint(r), as_uint(0));\n \
            result.z = r;\n \
        case 2:\n \
            templo = viv_getlonglo(src.y);\n \
            temphi = viv_getlonghi(src.y);\n \
            if(temphi == 0)\n \
            {\n \
                r = 8 + clz(templo);\n \
            }\n \
            else\n \
            {\n \
                r = clz(temphi);\n \
            }\n \
            viv_setlong(tmpr, as_uint(r), as_uint(0));\n \
            result.y = r;\n \
       default:\n \
            templo = viv_getlonglo(src.x);\n \
            temphi = viv_getlonghi(src.x);\n \
            if(temphi == 0)\n \
            {\n \
                r = 8 + clz(templo);\n \
            }\n \
            else\n \
            {\n \
                r = clz(temphi);\n \
            }\n \
            viv_setlong(tmpr, as_uint(r), as_uint(0));\n \
            result.x = r;\n \
           break;\n \
    }\n \
    return result;\n \
}\n \
"

/* 64 bit compare LESS_OR_EQUAL - scalar */
#define _longulong_jmp_lessEqual_long \
"long long_jmp_lessEqual(long srcX, long srcY) \n" \
"{ \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   int resLessH, resLessL, resEqualH, resEqualL; \n" \
"   long result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resLessH = hiSrcX < hiSrcY; \n" \
"   resEqualH = resLessH ? 0 : hiSrcX == hiSrcY; \n" \
"   resLessL = lowSrcX < lowSrcY; \n" \
"   resEqualL = resLessL ? 0 : lowSrcX == lowSrcY; \n" \
"   result = resLessH ? 1 : (resEqualH ? (resLessL? 1: (resEqualL?1:0)):0) ; \n" \
"   return result; \n" \
"} \n"

#define _longulong_jmp_lessEqual_ulong \
"ulong ulong_jmp_lessEqual(ulong srcX, ulong srcY) \n" \
"{ \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   uint resLessH, resLessL, resEqualH, resEqualL; \n" \
"   ulong result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resLessH = hiSrcX < hiSrcY; \n" \
"   resEqualH = resLessH ? 0 : hiSrcX == hiSrcY; \n" \
"   resLessL = lowSrcX < lowSrcY; \n" \
"   resEqualL = resLessL ? 0 : lowSrcX == lowSrcY; \n" \
"   result = resLessH ? 1 : (resEqualH ? (resLessL? 1: (resEqualL?1:0)):0) ; \n" \
"   return result; \n" \
"} \n"

/* 64 bit logic Less or Equal - vector */
#define _longulong_cmp_lessEqual_long \
"long4 long_cmp_lessEqual(uint count, long4 s0, long4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   long4 result = 0L; \n" \
"   int  resInt; \n" \
"   long resLong; \n" \
"   int resLessH, resLessL, resEqualH, resEqualL; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resLessH = hiSrcX < hiSrcY; \n" \
"       resEqualH = resLessH ? 0 : (hiSrcX == hiSrcY); \n" \
"       resLessL = lowSrcX < lowSrcY; \n" \
"       resEqualL = resLessL ? 0 : (lowSrcX == lowSrcY); \n" \
"       resInt = resLessH ? 1 : (resEqualH ? (resLessL? 1: (resEqualL?1:0)):0) ; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

#define _longulong_cmp_lessEqual_ulong \
"ulong4 ulong_cmp_lessEqual(uint count, ulong4 s0, ulong4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   ulong4 result = 0L; \n" \
"   uint  resInt; \n" \
"   ulong resLong; \n" \
"   uint resLessH, resLessL, resEqualH, resEqualL; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resLessH = hiSrcX < hiSrcY; \n" \
"       resEqualH = resLessH ? 0 : (hiSrcX == hiSrcY); \n" \
"       resLessL = lowSrcX < lowSrcY; \n" \
"       resEqualL = resLessL ? 0 : (lowSrcX == lowSrcY); \n" \
"       resInt = resLessH ? 1 : (resEqualH ? (resLessL? 1: (resEqualL?1:0)):0) ; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"


/* 64 bit compare GREATER_OR_EQUAL - scalar */
#define _longulong_jmp_greaterEqual_long \
"long long_jmp_greaterEqual(long srcX, long srcY) \n" \
"{ \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   int resGreaterH, resGreaterL, resEqualH, resEqualL; \n" \
"   long result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resGreaterH = hiSrcX > hiSrcY; \n" \
"   resEqualH = resGreaterH ? 0 : hiSrcX == hiSrcY; \n" \
"   resGreaterL = lowSrcX > lowSrcY; \n" \
"   resEqualL = resGreaterL ? 0 : lowSrcX == lowSrcY; \n" \
"   result = resGreaterH ? 1 : (resEqualH ? (resGreaterL? 1: (resEqualL?1:0)):0) ; \n" \
"   return result; \n" \
"} \n"

#define _longulong_jmp_greaterEqual_ulong \
"ulong ulong_jmp_greaterEqual(ulong srcX, ulong srcY) \n" \
"{ \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   uint resGreaterH, resGreaterL, resEqualH, resEqualL; \n" \
"   ulong result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resGreaterH = hiSrcX > hiSrcY; \n" \
"   resEqualH = resGreaterH ? 0 : hiSrcX == hiSrcY; \n" \
"   resGreaterL = lowSrcX > lowSrcY; \n" \
"   resEqualL = resGreaterL ? 0 : lowSrcX == lowSrcY; \n" \
"   result = resGreaterH ? 1 : (resEqualH ? (resGreaterL? 1: (resEqualL?1:0)):0) ; \n" \
"   return result; \n" \
"} \n"

/* 64 bit logic Greater or Equal - vector */
#define _longulong_cmp_greaterEqual_long \
"long4 long_cmp_greaterEqual(uint count, long4 s0, long4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   long4 result = 0L; \n" \
"   int  resInt; \n" \
"   long resLong; \n" \
"   int resGreaterH, resGreaterL, resEqualH, resEqualL; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resGreaterH = hiSrcX > hiSrcY; \n" \
"       resEqualH = resGreaterH ? 0 : (hiSrcX == hiSrcY); \n" \
"       resGreaterL = lowSrcX > lowSrcY; \n" \
"       resEqualL = resGreaterL ? 0 : (lowSrcX == lowSrcY); \n" \
"       resInt = resGreaterH ? 1 : (resEqualH ? (resGreaterL? 1: (resEqualL?1:0)):0) ; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

#define _longulong_cmp_greaterEqual_ulong \
"ulong4 ulong_cmp_greaterEqual(uint count, ulong4 s0, ulong4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   ulong4 result = 0L; \n" \
"   uint  resInt; \n" \
"   ulong resLong; \n" \
"   uint resGreaterH, resGreaterL, resEqualH, resEqualL; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resGreaterH = hiSrcX > hiSrcY; \n" \
"       resEqualH = resGreaterH ? 0 : (hiSrcX == hiSrcY); \n" \
"       resGreaterL = lowSrcX > lowSrcY; \n" \
"       resEqualL = resGreaterL ? 0 : (lowSrcX == lowSrcY); \n" \
"       resInt = resGreaterH ? 1 : (resEqualH ? (resGreaterL? 1: (resEqualL?1:0)):0) ; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

/* 64 bit compare Less - scalar */
#define _longulong_jmp_less_long \
"long long_jmp_less(long srcX, long srcY) \n" \
"{ \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   int reslessH, reslessL, resEqualH; \n" \
"   long result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   reslessH = hiSrcX < hiSrcY; \n" \
"   resEqualH = reslessH ? 0 : hiSrcX == hiSrcY; \n" \
"   reslessL = lowSrcX < lowSrcY; \n" \
"   result = reslessH ? 1 : (resEqualH ? (reslessL? 1:0):0) ; \n" \
"   return result; \n" \
"} \n"

#define _longulong_jmp_less_ulong \
"ulong ulong_jmp_less(ulong srcX, ulong srcY) \n" \
"{ \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   uint reslessH, reslessL, resEqualH; \n" \
"   ulong result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   reslessH = hiSrcX < hiSrcY; \n" \
"   resEqualH = reslessH ? 0 : hiSrcX == hiSrcY; \n" \
"   reslessL = lowSrcX < lowSrcY; \n" \
"   result = reslessH ? 1 : (resEqualH ? (reslessL? 1:0):0) ; \n" \
"   return result; \n" \
"} \n"

/* 64 bit logic Less - vector */
#define _longulong_cmp_less_long \
"long4 long_cmp_less(uint count, long4 s0, long4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   long4 result = 0L; \n" \
"   int  resInt; \n" \
"   long resLong; \n" \
"   int reslessH, reslessL, resEqualH; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       reslessH = hiSrcX < hiSrcY; \n" \
"       resEqualH = reslessH ? 0 : (hiSrcX == hiSrcY); \n" \
"       reslessL = lowSrcX < lowSrcY; \n" \
"       resInt = reslessH ? 1 : (resEqualH ? (reslessL? 1:0):0) ; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

#define _longulong_cmp_less_ulong \
"ulong4 ulong_cmp_less(uint count, ulong4 s0, ulong4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   ulong4 result = 0L; \n" \
"   uint  resInt; \n" \
"   ulong resLong; \n" \
"   uint reslessH, reslessL, resEqualH; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       reslessH = hiSrcX < hiSrcY; \n" \
"       resEqualH = reslessH ? 0 : (hiSrcX == hiSrcY); \n" \
"       reslessL = lowSrcX < lowSrcY; \n" \
"       resInt = reslessH ? 1 : (resEqualH ? (reslessL? 1:0):0) ; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

/* 64 bit compare Greater - scalar */
#define _longulong_jmp_greater_long \
"long long_jmp_greater(long srcX, long srcY) \n" \
"{ \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   int resGreaterH, resGreaterL, resEqualH; \n" \
"   long result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resGreaterH = hiSrcX > hiSrcY; \n" \
"   resEqualH = resGreaterH ? 0 : hiSrcX == hiSrcY; \n" \
"   resGreaterL = lowSrcX > lowSrcY; \n" \
"   result = resGreaterH ? 1 : (resEqualH ? (resGreaterL? 1:0):0) ; \n" \
"   return result; \n" \
"} \n"

#define _longulong_jmp_greater_ulong \
"ulong ulong_jmp_greater(ulong srcX, ulong srcY) \n" \
"{ \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   uint resGreaterH, resGreaterL, resEqualH; \n" \
"   ulong result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resGreaterH = hiSrcX > hiSrcY; \n" \
"   resEqualH = resGreaterH ? 0 : hiSrcX == hiSrcY; \n" \
"   resGreaterL = lowSrcX > lowSrcY; \n" \
"   result = resGreaterH ? 1 : (resEqualH ? (resGreaterL? 1:0):0) ; \n" \
"   return result; \n" \
"} \n"

/* 64 bit logic Greater - vector */
#define _longulong_cmp_greater_long \
"long4 long_cmp_greater(uint count, long4 s0, long4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   long4 result = 0L; \n" \
"   int  resInt; \n" \
"   long resLong; \n" \
"   int resGreaterH, resGreaterL, resEqualH; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resGreaterH = hiSrcX > hiSrcY; \n" \
"       resEqualH = resGreaterH ? 0 : (hiSrcX == hiSrcY); \n" \
"       resGreaterL = lowSrcX > lowSrcY; \n" \
"       resInt = resGreaterH ? 1 : (resEqualH ? (resGreaterL? 1:0):0) ; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

#define _longulong_cmp_greater_ulong \
"ulong4 ulong_cmp_greater(uint count, ulong4 s0, ulong4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   ulong4 result = 0L; \n" \
"   uint  resInt; \n" \
"   ulong resLong; \n" \
"   uint resGreaterH, resGreaterL, resEqualH; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resGreaterH = hiSrcX > hiSrcY; \n" \
"       resEqualH = resGreaterH ? 0 : (hiSrcX == hiSrcY); \n" \
"       resGreaterL = lowSrcX > lowSrcY; \n" \
"       resInt = resGreaterH ? 1 : (resEqualH ? (resGreaterL? 1:0):0) ; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

/* 64 bit compare EQUAL - scalar */
#define _longulong_jmp_equal_long \
"long long_jmp_equal(long srcX, long srcY) \n" \
"{ \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   int resEqualH, resEqualL; \n" \
"   long result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resEqualH = hiSrcX == hiSrcY; \n" \
"   resEqualL = lowSrcX == lowSrcY; \n" \
"   result = resEqualH && resEqualL; \n" \
"   return result; \n" \
"} \n"

#define _longulong_jmp_equal_ulong \
"ulong ulong_jmp_equal(ulong srcX, ulong srcY) \n" \
"{ \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   uint resEqualH, resEqualL; \n" \
"   ulong result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resEqualH = hiSrcX == hiSrcY; \n" \
"   resEqualL = lowSrcX == lowSrcY; \n" \
"   result = resEqualH && resEqualL; \n" \
"   return result; \n" \
"} \n"

/* 64 bit logic Greater or EQUAL - vector */
#define _longulong_cmp_equal_long \
"long4 long_cmp_equal(uint count, long4 s0, long4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   long4 result = 0L; \n" \
"   int  resInt; \n" \
"   long resLong; \n" \
"   int resEqualH, resEqualL; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resEqualH = hiSrcX == hiSrcY; \n" \
"       resEqualL = lowSrcX == lowSrcY; \n" \
"       resInt = resEqualH && resEqualL; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

#define _longulong_cmp_equal_ulong \
"ulong4 ulong_cmp_equal(uint count, ulong4 s0, ulong4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   ulong4 result = 0L; \n" \
"   uint  resInt; \n" \
"   ulong resLong; \n" \
"   uint resEqualH, resEqualL; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resEqualH = hiSrcX == hiSrcY; \n" \
"       resEqualL = lowSrcX == lowSrcY; \n" \
"       resInt = resEqualH && resEqualL; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"


/* 64 bit compare NOT_EQUAL - scalar */
#define _longulong_jmp_notequal_long \
"long long_jmp_notEqual(long srcX, long srcY) \n" \
"{ \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   int resNotEqualH, resNotEqualL; \n" \
"   long result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resNotEqualH = hiSrcX != hiSrcY; \n" \
"   resNotEqualL = lowSrcX != lowSrcY; \n" \
"   result = resNotEqualH || resNotEqualL; \n" \
"   return result; \n" \
"} \n"

#define _longulong_jmp_notequal_ulong \
"ulong ulong_jmp_notEqual(ulong srcX, ulong srcY) \n" \
"{ \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   uint resNotEqualH, resNotEqualL; \n" \
"   ulong result; \n" \
"   lowSrcX = viv_getlonglo(srcX); \n" \
"   hiSrcX = viv_getlonghi(srcX); \n" \
"   lowSrcY = viv_getlonglo(srcY); \n" \
"   hiSrcY = viv_getlonghi(srcY); \n" \
"   resNotEqualH = hiSrcX != hiSrcY; \n" \
"   resNotEqualL = lowSrcX != lowSrcY; \n" \
"   result = resNotEqualH || resNotEqualL; \n" \
"   return result; \n" \
"} \n"

/* 64 bit logic Greater or NOT_EQUAL - vector */
#define _longulong_cmp_notequal_long \
"long4 long_cmp_notEqual(uint count, long4 s0, long4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   int hiSrcX, hiSrcY; \n" \
"   uint lowSrcX, lowSrcY; \n" \
"   long4 result = 0L; \n" \
"   int  resInt; \n" \
"   long resLong; \n" \
"   int resNotEqualH, resNotEqualL; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resNotEqualH = hiSrcX != hiSrcY; \n" \
"       resNotEqualL = lowSrcX != lowSrcY; \n" \
"       resInt = resNotEqualH || resNotEqualL; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

#define _longulong_cmp_notequal_ulong \
"ulong4 ulong_cmp_notEqual(uint count, ulong4 s0, ulong4 s1) \n" \
"{\n" \
"   uint ii; \n" \
"   uint lowSrcX, hiSrcX; \n" \
"   uint lowSrcY, hiSrcY; \n" \
"   ulong4 result = 0L; \n" \
"   uint  resInt; \n" \
"   ulong resLong; \n" \
"   uint resNotEqualH, resNotEqualL; \n" \
"   uint zeroConst = 0L; \n" \
"   uint oneConst = 0xffffffff; \n" \
"   for(ii=0; ii<count; ii++) \n" \
"   {\n" \
"       if(ii == 0) { lowSrcX = viv_getlonglo(s0.x); hiSrcX = viv_getlonghi(s0.x); lowSrcY = viv_getlonglo(s1.x); hiSrcY = viv_getlonghi(s1.x); } \n"  \
"       else if(ii == 1) { lowSrcX = viv_getlonglo(s0.y); hiSrcX = viv_getlonghi(s0.y); lowSrcY = viv_getlonglo(s1.y); hiSrcY = viv_getlonghi(s1.y); } \n"  \
"       else if(ii == 2) { lowSrcX = viv_getlonglo(s0.z); hiSrcX = viv_getlonghi(s0.z); lowSrcY = viv_getlonglo(s1.z); hiSrcY = viv_getlonghi(s1.z); } \n"  \
"       else if(ii == 3) { lowSrcX = viv_getlonglo(s0.w); hiSrcX = viv_getlonghi(s0.w); lowSrcY = viv_getlonglo(s1.w); hiSrcY = viv_getlonghi(s1.w); } \n"  \
"       resNotEqualH = hiSrcX != hiSrcY; \n" \
"       resNotEqualL = lowSrcX != lowSrcY; \n" \
"       resInt = resNotEqualH || resNotEqualL; \n" \
"       if(resInt == 1) viv_setlong(resLong, oneConst, oneConst); \n" \
"       else viv_setlong(resLong, zeroConst, zeroConst); \n" \
"       if(ii == 0) result.x = resLong; \n" \
"       else if(ii == 1) result.y = resLong; \n" \
"       else if(ii == 2) result.z = resLong; \n" \
"       else if(ii == 3) result.w = resLong; \n" \
"   }\n" \
"   return result;\n" \
"} \n"

#define _longulong_rotate \
"\
ulong viv_Rotate64(ulong src, uint rot)\n\
{\n\
    uint hi, lo;\n\
    uint hi_shift, hi_rot, lo_shift, lo_rot;\n\
    uint hi_r, lo_r;\n\
    uint nbits, mask;\n\
    ulong result;\n\
\
    hi = viv_getlonghi(src);\n\
    lo = viv_getlonglo(src);\n\
    if (rot == 32)\n\
    {\n\
        hi_r = lo;\n\
        lo_r = hi;\n\
    }\n\
    else\n\
    if (rot < 32)\n\
    {\n\
        mask = (0x1 << rot) - 1;\n\
        nbits = 32 - rot;\n\
        lo_shift = lo << rot;\n\
        lo_rot = (lo >> nbits) & mask;\n\
        hi_shift = hi << rot;\n\
        hi_rot = (hi >> nbits) & mask;\n\
        hi_r = hi_shift | lo_rot;\n\
        lo_r = lo_shift | hi_rot;\n\
    }\n\
    else\n\
    {\n\
        rot -= 32;\n\
        nbits = 32 - rot;\n\
        mask = (0x1 << rot) - 1;\n\
        lo_shift = lo << rot;\n\
        lo_rot = (lo >> nbits) & mask;\n\
        hi_shift = hi << rot;\n\
        hi_rot = (hi >> nbits) & mask;\n\
        lo_r = hi_shift | lo_rot;\n\
        hi_r = lo_shift | hi_rot;\n\
    }\n\
\
    viv_setlong(result, lo_r, hi_r);\n\
    return result;\n\
}\n\
\
ulong4 ulong_Rotate(uint count, ulong4 x, ulong4 n)\n\
{\n\
    ulong4 result;\n\
    uint4 bits;\n\
    bits = viv_getlonglo4(n);\n\
    bits = bits & 63;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            result.w = viv_Rotate64(x.w, bits.w);\n\
        case 3:\n\
            result.z = viv_Rotate64(x.z, bits.z);\n\
        case 2:\n\
            result.y = viv_Rotate64(x.y, bits.y);\n\
        case 1:\n\
        default:\n\
            result.x = viv_Rotate64(x.x, bits.x);\n\
            break;\n\
    }\n\
    return result;\n\
}\n\
\
long4 long_Rotate(uint count, long4 x, long4 n)\n\
{\n\
    long4 result;\n\
    uint4 bits;\n\
    bits = viv_getlonglo4(n);\n\
    bits = bits & 63;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            result.w = viv_Rotate64(x.w, bits.w);\n\
        case 3:\n\
            result.z = viv_Rotate64(x.z, bits.z);\n\
        case 2:\n\
            result.y = viv_Rotate64(x.y, bits.y);\n\
        case 1:\n\
        default:\n\
            result.x = viv_Rotate64(x.x, bits.x);\n\
            break;\n\
    }\n\
    return result;\n\
}\n\
"

#define _longulong_popcount \
"\
ulong viv_Popcount(ulong x)\n\
{\n\
    uint hi, lo;\n\
    uint hi_r, lo_r;\n\
    ulong result;\n\
\
    hi = viv_getlonghi(x);\n\
    lo = viv_getlonglo(x);\n\
    hi_r = 0;\n\
    lo_r = popcount(hi) + popcount(lo);\n\
    viv_setlong(result, lo_r, hi_r);\n\
    return result;\n\
}\n\
\
ulong4 ulong_Popcount(uint count, ulong4 x)\n\
{\n\
    ulong4 result;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            result.w = viv_Popcount(x.w);\n\
        case 3:\n\
            result.z = viv_Popcount(x.z);\n\
        case 2:\n\
            result.y = viv_Popcount(x.y);\n\
        case 1:\n\
        default:\n\
            result.x = viv_Popcount(x.x);\n\
            break;\n\
    }\n\
    return result;\n\
}\n\
\
long4 long_Popcount(uint count, long4 x)\n\
{\n\
    long4 result;\n\
    switch (count)\n\
    {\n\
        case 4:\n\
            result.w = viv_Popcount(x.w);\n\
        case 3:\n\
            result.z = viv_Popcount(x.z);\n\
        case 2:\n\
            result.y = viv_Popcount(x.y);\n\
        case 1:\n\
        default:\n\
            result.x = viv_Popcount(x.x);\n\
            break;\n\
    }\n\
    return result;\n\
}\n\
"

#define _long_max \
"\
long4 long_max(uint count, long4 x, long4 y)\n \
{\n \
    long4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            if(long_Greater(x.w, y.w) == 1)\n \
            {\n \
                result.w = x.w;\n \
            }\n \
            else\n \
            {\n \
                result.w = y.w;\n \
            }\n \
        case 3:\n \
            if(long_Greater(x.z, y.z) == 1)\n \
            {\n \
                result.z = x.z;\n \
            }\n \
            else\n \
            {\n \
                result.z = y.z;\n \
            }\n \
        case 2:\n \
            if(long_Greater(x.y, y.y) == 1)\n \
            {\n \
                result.y = x.y;\n \
            }\n \
            else\n \
            {\n \
                result.y = y.y;\n \
            }\n \
        default:\n \
            if(long_Greater(x.x, y.x) == 1)\n \
            {\n \
                result.x = x.x;\n \
            }\n \
            else\n \
            {\n \
                result.x = y.x;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _ulong_max \
"\
ulong4 ulong_max(uint count, ulong4 x, ulong4 y)\n \
{\n \
    ulong4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            if(ulong_Greater(x.w, y.w) == 1)\n \
            {\n \
                result.w = x.w;\n \
            }\n \
            else\n \
            {\n \
                result.w = y.w;\n \
            }\n \
        case 3:\n \
            if(ulong_Greater(x.z, y.z) == 1)\n \
            {\n \
                result.z = x.z;\n \
            }\n \
            else\n \
            {\n \
                result.z = y.z;\n \
            }\n \
        case 2:\n \
            if(ulong_Greater(x.y, y.y) == 1)\n \
            {\n \
                result.y = x.y;\n \
            }\n \
            else\n \
            {\n \
                result.y = y.y;\n \
            }\n \
        default:\n \
            if(ulong_Greater(x.x, y.x) == 1)\n \
            {\n \
                result.x = x.x;\n \
            }\n \
            else\n \
            {\n \
                result.x = y.x;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _long_min \
"\
long4 long_min(uint count, long4 x, long4 y)\n \
{\n \
    long4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            if(long_Greater(x.w, y.w) == 1)\n \
            {\n \
                result.w = y.w;\n \
            }\n \
            else\n \
            {\n \
                result.w = x.w;\n \
            }\n \
        case 3:\n \
            if(long_Greater(x.z, y.z) == 1)\n \
            {\n \
                result.z = y.z;\n \
            }\n \
            else\n \
            {\n \
                result.z = x.z;\n \
            }\n \
        case 2:\n \
            if(long_Greater(x.y, y.y) == 1)\n \
            {\n \
                result.y = y.y;\n \
            }\n \
            else\n \
            {\n \
                result.y = x.y;\n \
            }\n \
        default:\n \
            if(long_Greater(x.x, y.x) == 1)\n \
            {\n \
                result.x = y.x;\n \
            }\n \
            else\n \
            {\n \
                result.x = x.x;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

#define _ulong_min \
"\
ulong4 ulong_min(uint count, ulong4 x, ulong4 y)\n \
{\n \
    ulong4 result = 0;\n \
    switch(count)\n \
    {\n \
        case 4:\n \
            if(ulong_Greater(x.w, y.w) == 1)\n \
            {\n \
                result.w = y.w;\n \
            }\n \
            else\n \
            {\n \
                result.w = x.w;\n \
            }\n \
        case 3:\n \
            if(ulong_Greater(x.z, y.z) == 1)\n \
            {\n \
                result.z = y.z;\n \
            }\n \
            else\n \
            {\n \
                result.z = x.z;\n \
            }\n \
        case 2:\n \
            if(ulong_Greater(x.y, y.y) == 1)\n \
            {\n \
                result.y = y.y;\n \
            }\n \
            else\n \
            {\n \
                result.y = x.y;\n \
            }\n \
        default:\n \
            if(ulong_Greater(x.x, y.x) == 1)\n \
            {\n \
                result.x = y.x;\n \
            }\n \
            else\n \
            {\n \
                result.x = x.x;\n \
            }\n \
            break;\n \
    }\n \
    return result;\n \
}\n \
"

extern gctSTRING    gcLibCLLong_Func;
extern gctSTRING    gcLibCLLong_Func1;
extern gctSTRING    gcLibCLLong_Func2;
#endif

/****************************************************************************************
new read_image & write_image
****************************************************************************************/

/* common code */
extern gctSTRING gcLibCL_ReadImage_Header_Str;
extern gctSTRING gcLibCL_ReadImage_Common_Func_Str;

/* common macros */
#define DECLARE_RESULT(RET_TYPE) \
"    "#RET_TYPE" result;\n"

#define RETURN_RESULT \
"    return result;\n"

/* read image with imgld macros */
#define READ_IMAGE_WITH_IMGLD_FUNC_NAME(RET_TYPE, DIM, COORD_TYPE) \
#RET_TYPE" _read_image_with_imgld_"#RET_TYPE"_"#DIM"_"#COORD_TYPE"(viv_generic_image_t image, sampler_t sampler, "#COORD_TYPE" coord)\n"

#define DECLARE_AND_GET_GENERIC_IMAGE_DSP(SAMPLER) \
"    uint8 imageDSP;\n" \
"    _viv_asm(GET_IMAGE_T_IMAGE_DSP, imageDSP, image, "#SAMPLER");\n"

#define DECLARE_AND_GET_ONE_INT_COORD_FROM_INTEGER_COORD \
"    int3 intCoord0;\n" \
"    intCoord0 = _viv_getNearestCoordFromIntegerCoord(imageDSP, sampler, coord);\n"

#define DECLARE_AND_GET_ONE_INT_COORD_FROM_FLOAT_COORD \
"    int3 intCoord0;\n" \
"    intCoord0 = _viv_getNearestCoordFromFloatCoord(imageDSP, sampler, coord);\n"

#define READ_IMAGE_WITH_IMGLD_1D_COORD_TYPE_int3(RET_TYPE) \
    READ_IMAGE_WITH_IMGLD_FUNC_NAME(RET_TYPE, 1d, int3) \
    "{\n" \
    DECLARE_RESULT(RET_TYPE) \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(sampler) \
    DECLARE_AND_GET_ONE_INT_COORD_FROM_INTEGER_COORD \
    "    uint imageType;\n" \
    "    _viv_asm(GET_IMAGE_T_TYPE, imageType, imageDSP.s0123);\n" \
    "    if(imageType != 2)                                     /* 1d_array */\n" \
    "    {\n" \
    "        intCoord0.s1 = 0;\n" \
    "    }\n" \
    "    _viv_asm(IMAGE_READ, result, imageDSP, intCoord0.s01);\n" \
    RETURN_RESULT \
    "}\n"

#define READ_IMAGE_WITH_IMGLD_2D_COORD_TYPE_int3(RET_TYPE) \
    READ_IMAGE_WITH_IMGLD_FUNC_NAME(RET_TYPE, 2d, int3) \
    "{\n" \
    DECLARE_RESULT(RET_TYPE) \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(sampler) \
    DECLARE_AND_GET_ONE_INT_COORD_FROM_INTEGER_COORD \
    "    uint imageType;\n" \
    "    _viv_asm(GET_IMAGE_T_TYPE, imageType, imageDSP.s0123);\n" \
    "    if(imageType == 4)                                     /* 2d_array */\n" \
    "    {\n" \
    "        intCoord0.s2 = _viv_get3DImageNewBaseAddr(imageDSP, intCoord0);\n" \
    "        _viv_asm(IMAGE_READ_3D, result, imageDSP, intCoord0);\n" \
    "    }\n" \
    "    else\n" \
    "    {\n" \
    "        _viv_asm(IMAGE_READ, result, imageDSP, intCoord0.s01);\n" \
    "    }\n" \
    RETURN_RESULT \
    "}\n"

#define READ_IMAGE_WITH_IMGLD_3D_COORD_TYPE_int3(RET_TYPE) \
    READ_IMAGE_WITH_IMGLD_FUNC_NAME(RET_TYPE, 3d, int3) \
    "{\n" \
    DECLARE_RESULT(RET_TYPE) \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(sampler) \
    DECLARE_AND_GET_ONE_INT_COORD_FROM_INTEGER_COORD \
    "    intCoord0.s2 = _viv_get3DImageNewBaseAddr(imageDSP, intCoord0);\n" \
    "    _viv_asm(IMAGE_READ_3D, result, imageDSP, intCoord0);\n" \
    RETURN_RESULT \
    "}\n"

#define READ_IMAGEIUI_WITH_IMGLD_1D_COORD_TYPE_float3(RET_TYPE) \
    READ_IMAGE_WITH_IMGLD_FUNC_NAME(RET_TYPE, 1d, float3) \
    "{\n" \
    DECLARE_RESULT(RET_TYPE) \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(sampler) \
    DECLARE_AND_GET_ONE_INT_COORD_FROM_FLOAT_COORD \
    "    uint imageType;\n" \
    "    _viv_asm(GET_IMAGE_T_TYPE, imageType, imageDSP.s0123);\n" \
    "    if(imageType != 2)                                     /* 1d_array */\n" \
    "    {\n" \
    "        intCoord0.s1 = 0;\n" \
    "    }\n" \
    "    _viv_asm(IMAGE_READ, result, imageDSP, intCoord0.s01);\n" \
    RETURN_RESULT \
    "}\n"

#define READ_IMAGEIUI_WITH_IMGLD_2D_COORD_TYPE_float3(RET_TYPE) \
    READ_IMAGE_WITH_IMGLD_FUNC_NAME(RET_TYPE, 2d, float3) \
    "{\n" \
    DECLARE_RESULT(RET_TYPE) \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(sampler) \
    DECLARE_AND_GET_ONE_INT_COORD_FROM_FLOAT_COORD \
    "    uint imageType;\n" \
    "    _viv_asm(GET_IMAGE_T_TYPE, imageType, imageDSP.s0123);\n" \
    "    if(imageType == 4)                                     /* 2d_array */\n" \
    "    {\n" \
    "        intCoord0.s2 = _viv_get3DImageNewBaseAddr(imageDSP, intCoord0);\n" \
    "        _viv_asm(IMAGE_READ_3D, result, imageDSP, intCoord0);\n" \
    "    }\n" \
    "    else\n" \
    "    {\n" \
    "        _viv_asm(IMAGE_READ, result, imageDSP, intCoord0.s01);\n" \
    "    }\n" \
    RETURN_RESULT \
    "}\n"

#define READ_IMAGEIUI_WITH_IMGLD_3D_COORD_TYPE_float3(RET_TYPE) \
    READ_IMAGE_WITH_IMGLD_FUNC_NAME(RET_TYPE, 3d, float3) \
    "{\n" \
    DECLARE_RESULT(RET_TYPE) \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(sampler) \
    DECLARE_AND_GET_ONE_INT_COORD_FROM_FLOAT_COORD \
    "    intCoord0.s2 = _viv_get3DImageNewBaseAddr(imageDSP, intCoord0);\n" \
    "    _viv_asm(IMAGE_READ_3D, result, imageDSP, intCoord0);\n" \
    RETURN_RESULT \
    "}\n"

#define READ_IMAGEF_WITH_IMGLD_1D_COORD_TYPE_float3 \
    READ_IMAGE_WITH_IMGLD_FUNC_NAME(float4, 1d, float3) \
    "{\n" \
    DECLARE_RESULT(float4) \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(sampler) \
    "    result =  _viv_readImage1dCoordF(imageDSP, sampler, coord);\n" \
    RETURN_RESULT \
    "}\n"

#define READ_IMAGEF_WITH_IMGLD_2D_COORD_TYPE_float3 \
    READ_IMAGE_WITH_IMGLD_FUNC_NAME(float4, 2d, float3) \
    "{\n" \
    DECLARE_RESULT(float4) \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(sampler) \
    "    result =  _viv_readImage2dCoordF(imageDSP, sampler, coord);\n" \
    RETURN_RESULT \
    "}\n"

#define READ_IMAGEF_WITH_IMGLD_3D_COORD_TYPE_float3 \
    READ_IMAGE_WITH_IMGLD_FUNC_NAME(float4, 3d, float3) \
    "{\n" \
    DECLARE_RESULT(float4) \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(sampler) \
    "    result = _viv_readImage3dCoordF(imageDSP, sampler, coord);\n" \
    RETURN_RESULT \
    "}\n"

extern gctSTRING gcLibCL_ReadImage_With_IMGLD_Funcs;

/* read image with texldu macros */
#define READ_IMAGE_WITH_TEXLDU_FUNC_NAME(RET_TYPE, COORD_TYPE) \
#RET_TYPE" _read_image_with_texldu_"#RET_TYPE"_"#COORD_TYPE"(viv_generic_image_t image, sampler_t sampler, "#COORD_TYPE" coord)\n"

#define DECLARE_GENERIC_SAMPLER_IDX \
"    uint glSamplerIndex;\n"

#define GET_IMAGE_T_SAMPLER_IDX_STMT \
"    _viv_asm(GET_IMAGE_T_SAMPLER_IDX, glSamplerIndex, image);\n"

#define SINGLE_TEXLDU_TO_RESULT \
"    _viv_asm(TEXU, result, sampler);\n" \
"    _viv_asm(TEXTURE_LOAD, result, glSamplerIndex, coord);\n"

#define READ_IMAGE_WITH_TEXLDU(RET_TYPE, COORD_TYPE) \
    READ_IMAGE_WITH_TEXLDU_FUNC_NAME(RET_TYPE, COORD_TYPE) \
    "{\n" \
    DECLARE_RESULT(RET_TYPE) \
    DECLARE_GENERIC_SAMPLER_IDX \
    GET_IMAGE_T_SAMPLER_IDX_STMT \
    SINGLE_TEXLDU_TO_RESULT \
    RETURN_RESULT \
    "}\n"

extern gctSTRING gcLibCL_ReadImage_With_TEXLDU_Funcs;

extern gctSTRING gcLibCL_ReadImage_With_V55_TEXLDU_Funcs;

extern gctSTRING gcLibCL_ReadImage_With_TEXLD_Funcs;

#define WRITE_IMAGE_WITH_IMGST_FUNC_NAME(DIMENSION, DATA_TYPE) \
    "void _write_image_with_imgst_"#DIMENSION"_"#DATA_TYPE"(viv_generic_image_t image, int3 coord, "#DATA_TYPE" data)\n"

#define SINGLE_IMAGE_WR_1d \
    "    coord.s12 = 0;\n" \
    "    _viv_asm(IMAGE_WRITE, data, imageDSP, coord.s01);\n"

#define SINGLE_IMAGE_WR_2d \
    "    _viv_asm(IMAGE_WRITE, data, imageDSP, coord.s01);\n"

#define SINGLE_IMAGE_WR_3d \
    "    coord.s2 = _viv_get3DImageNewBaseAddr(imageDSP, coord);\n" \
    "    _viv_asm(IMAGE_WRITE_3D, data, imageDSP, coord);\n" \

#define WRITE_IMAGE_WITH_IMGST(DIMENSION, DATA_TYPE) \
    WRITE_IMAGE_WITH_IMGST_FUNC_NAME(DIMENSION, DATA_TYPE) \
    "{\n" \
    DECLARE_AND_GET_GENERIC_IMAGE_DSP(0) \
    SINGLE_IMAGE_WR_##DIMENSION \
    "}\n"

#endif

extern gctSTRING gcLibCL_WriteImage_With_IMGST_Funcs;
extern const gctCONST_STRING gcLibCL_ReadImage_VIR_Common_Func_Str;
extern gctSTRING gcLib_AtomicPatch_Common_Func_core1_Str;
extern gctSTRING gcLib_AtomicPatch_Common_Func_core2_Str;
extern gctSTRING gcLib_AtomicPatch_Common_Func_core4_Str;
extern gctSTRING gcLib_AtomicPatch_Common_Func_core8_Str;
extern gctSTRING gcCLLib_AtomcmpxchgPatch_Func_core1_Str;
extern gctSTRING gcCLLib_AtomcmpxchgPatch_Func_core2_Str;
extern gctSTRING gcCLLib_AtomcmpxchgPatch_Func_core4_Str;
extern gctSTRING gcCLLib_AtomcmpxchgPatch_Func_core8_Str;
extern gctSTRING gcCLLibGetLocalID;

#if !REMOVE_CL_LIBS
/* macro switch */
#define FIX_LEN_COORD   0
#define PARAM_FIX       1
#define HAS_SAMPLER     1
/* end of macro switch */
#if HAS_SAMPLER
#define SAMPLER_PARAM   ", sampler_t sampler"
#else
#define SAMPLER_PARAM   ""
#endif

#if FIX_LEN_COORD
#undef  PARAM_FIX
#define PARAM_FIX       1
#endif

#define _ToStr(x)       #x
#define ToStr(x)        _ToStr(x)

#define NEWLINE         "\n"
#define SPLITLINE       "\n"

#define retfloat        "retf"
#define retint          "reti"
#define retuint         "retui"

#define half_RET        float
#define float_RET       float
#define snorm8_RET      float
#define snorm16_RET     float
#define unorm8_RET      float
#define unorm16_RET     float
#define int8_RET        int
#define int16_RET       int
#define int32_RET       int
#define uint8_RET       uint
#define uint16_RET      uint
#define uint32_RET      uint
#define unorm555_RET    float
#define unorm565_RET    float
#define unorm101010_RET float

#define __RETURN_TYPE(type)     type
#define _RETURN_TYPE(type)      __RETURN_TYPE(type)
#define RETURN_TYPE(ch_data)    _RETURN_TYPE(ch_data##_RET)

#define __retRETURN_TYPE(type)  ret##type
#define _retRETURN_TYPE(type)   __retRETURN_TYPE(type)
#define retRETURN_TYPE(ch_data) _retRETURN_TYPE(ch_data##_RET)

/* default border color */
#define BORDER_CLR_RGBA         "(float4)(.0f)"
#define BORDER_CLR_LUMINANCE    BORDER_CLR_RGBA
#define BORDER_CLR_BGRA         BORDER_CLR_RGBA
#define BORDER_CLR_ARGB         BORDER_CLR_RGBA
#define BORDER_CLR_A            BORDER_CLR_RGBA
#define BORDER_CLR_RA           BORDER_CLR_RGBA
#define BORDER_CLR_Rx           BORDER_CLR_RGBA
#define BORDER_CLR_RGx          BORDER_CLR_RGBA
#define BORDER_CLR_RGBx         BORDER_CLR_RGBA
#define BORDER_CLR_R            "(float4)(.0f, .0f, .0f, 1.f)"
#define BORDER_CLR_RG           BORDER_CLR_R
#define BORDER_CLR_RGB          BORDER_CLR_R
#define BORDER_CLR_INTENSITY    BORDER_CLR_R

#define DIM_1d_NUM          1
#define DIM_1darray_NUM     1
#define DIM_1dbuffer_NUM    1
#define DIM_2d_NUM          2
#define DIM_2darray_NUM     2
#define DIM_3d_NUM          3

#define DIM_1d_NUM_VEC          ""
#define DIM_1darray_NUM_VEC     ""
#define DIM_1dbuffer_NUM_VEC    ""
#define DIM_2d_NUM_VEC          "2"
#define DIM_2darray_NUM_VEC     "2"
#define DIM_3d_NUM_VEC          "3"

#define __DIM_STR(num)      num
#define _DIM_STR(num)        __DIM_STR(num)
#define DIM_STR(dim)        _DIM_STR(DIM_##dim##_NUM_VEC)
#define DIM_NUM_FLOAT(dim)  "float" DIM_STR(dim)
#define DIM_NUM_INT(dim)    "int" DIM_STR(dim)
#define DIM_NUM_UINT(dim)   "uint" DIM_STR(dim)

#define COORD_1d_NUM        1
#define COORD_1darray_NUM   2
#define COORD_1dbuffer_NUM  1
#define COORD_2d_NUM        2
#define COORD_2darray_NUM   3
#define COORD_3d_NUM        3

#define COORD_1d_NUM_VEC        ""
#define COORD_1darray_NUM_VEC   "2"
#define COORD_1dbuffer_NUM_VEC  ""
#define COORD_2d_NUM_VEC        "2"
#define COORD_2darray_NUM_VEC   "3"
#define COORD_3d_NUM_VEC        "3"

#define __COORD_STR(num)    num
#define _COORD_STR(num)     __COORD_STR(num)
#define COORD_STR(dim)      _COORD_STR(COORD_##dim##_NUM_VEC)

#if FIX_LEN_COORD
#define COORD_NUM_INT(dim)      "int4"
#define COORD_NUM_FLOAT(dim)    "float4"
#define COORD_X(dim)            ".x"
#define COORD_ALIGN_1           ", 0, 0, 0"
#define COORD_ALIGN_2           ", 0, 0"
#define COORD_ALIGN_3           ", 0"
#else

#define COORD_NUM_INT(dim)      "int" COORD_STR(dim)
#define COORD_NUM_UINT(dim)     "uint" COORD_STR(dim)
#define COORD_NUM_FLOAT(dim)    "float" COORD_STR(dim)

#define COORD_X_1d()        ""
#define COORD_X_2d()        ".x"
#define COORD_X_3d()        ".x"
#define COORD_X_1darray()   ".x"
#define COORD_X_2darray()   ".x"
#define COORD_X_1dbuffer()  ""
#define COORD_X(dim)        COORD_X_##dim()

#if PARAM_FIX
#define COORD_1d_CVT        x
#define COORD_1dbuffer_CVT  x
#define COORD_1darray_CVT   xy
#define COORD_2d_CVT        xy
#define COORD_2darray_CVT   xyz
#define COORD_3d_CVT        xyz
#else
#define COORD_1d_CVT        none
#define COORD_1darray_CVT   none
#define COORD_1dbuffer_CVT  none
#define COORD_2d_CVT        none
#define COORD_2darray_CVT   none
#define COORD_3d_CVT        none
#endif

#define COORD_ALIGN_1       ""
#define COORD_ALIGN_2       ""
#define COORD_ALIGN_3       ""
#endif

#define __COORD_ALIGN(num)  COORD_ALIGN_##num
#define _COORD_ALIGN(num)   __COORD_ALIGN(num)
#define COORD_ALIGN(dim)    _COORD_ALIGN(COORD_##dim##_NUM)

#define DIM_1d_SUFFIX       ""
#define DIM_1darray_SUFFIX  "array_"
#define DIM_1dbuffer_SUFFIX "buffer_"
#define DIM_2d_SUFFIX       ""
#define DIM_2darray_SUFFIX  "array_"
#define DIM_3d_SUFFIX       ""

/* channel data */
#define CH_TYPE_unorm8          uchar
#define CH_TYPE_snorm8          char
#define CH_TYPE_unorm16         ushort
#define CH_TYPE_snorm16         short
#define CH_TYPE_uint8           uchar
#define CH_TYPE_int8            char
#define CH_TYPE_uint16          ushort
#define CH_TYPE_int16           short
#define CH_TYPE_uint32          uint
#define CH_TYPE_int32           int
#define CH_TYPE_float           float
#define CH_TYPE_half            half
#define CH_TYPE_unorm555        ushort
#define CH_TYPE_unorm565        ushort
#define CH_TYPE_unorm101010     uint

#define CH_TYPE_R_unorm8        uchar
#define CH_TYPE_R_snorm8        char
#define CH_TYPE_R_unorm16       ushort
#define CH_TYPE_R_snorm16       short
#define CH_TYPE_R_uint8         uchar
#define CH_TYPE_R_int8          char
#define CH_TYPE_R_uint16        ushort
#define CH_TYPE_R_int16         short
#define CH_TYPE_R_uint32        uint
#define CH_TYPE_R_int32         int
#define CH_TYPE_R_float         float
#define CH_TYPE_R_half          float
#define CH_TYPE_R_unorm555      ushort
#define CH_TYPE_R_unorm565      ushort
#define CH_TYPE_R_unorm101010   uint

/* channel order */
#define CH_ORDERNONE        ""
#define CH_ORDER_RGBA       "4"
#define CH_ORDER_BGRA       "4"
#define CH_ORDER_R          ""
#define CH_ORDER_Rx         ""
#define CH_ORDER_A          ""
#define CH_ORDER_INTENSITY  ""
#define CH_ORDER_LUMINANCE  ""
#define CH_ORDER_RG         "2"
#define CH_ORDER_RGx        "2"
#define CH_ORDER_RA         "2"
#define CH_ORDER_RGB        ""
#define CH_ORDER_RGBx       ""
#define CH_ORDER_ARGB       "4"

#define _LoadType(ch_data, ch_order)    ToStr(CH_TYPE_##ch_data) CH_ORDER##ch_order
#define LoadType(ch_data, ch_order)     _LoadType(ch_data, ch_order)
#define _Load_R_Type(ch_data, ch_order) ToStr(CH_TYPE_R_##ch_data) CH_ORDER##ch_order
#define Load_R_Type(ch_data, ch_order)  _Load_R_Type(ch_data, ch_order)

/* get image size */
#define imagesize_1d(dim, type)         COORD_NUM_INT(dim)" imageSize = ("COORD_NUM_INT(dim)")(get_image_width(image)"COORD_ALIGN(dim)");" NEWLINE
#define imagesize_2d(dim, type)         COORD_NUM_INT(dim)" imageSize = ("COORD_NUM_INT(dim)")(get_image_width(image), get_image_height(image)"COORD_ALIGN(dim)");" NEWLINE
#define imagesize_3d(dim, type)         COORD_NUM_INT(dim)" imageSize = ("COORD_NUM_INT(dim)")(get_image_width(image), get_image_height(image), get_image_depth(image)"COORD_ALIGN(dim)");" NEWLINE
#define imagesize_1dbuffer(dim, type)   imagesize_1d(dim, type)
#define imagesize_1darray(dim, type)    COORD_NUM_INT(dim)" imageSize = ("COORD_NUM_INT(dim)")(get_image_width(image), get_image_array_size(image)"COORD_ALIGN(dim)");" NEWLINE
#define imagesize_2darray(dim, type)    COORD_NUM_INT(dim)" imageSize = ("COORD_NUM_INT(dim)")(get_image_width(image), get_image_height(image), get_image_array_size(image)"COORD_ALIGN(dim)");" NEWLINE
#define ImageSize(dim)                  imagesize_##dim(dim, "viv_generic_image_t")

/* coord convert */
#define norm_COORD(dim, type, coord, cdim)    type"fcoord = convert_"COORD_NUM_FLOAT(dim)"("coord cdim") * convert_"COORD_NUM_FLOAT(dim)"(imageSize);" NEWLINE
#define unorm_COORD(dim, type, coord, cdim)   type"fcoord = convert_"COORD_NUM_FLOAT(dim)"("coord cdim");" NEWLINE
#define unnorm_COORD(dim, type, coord, cdim)  unorm_COORD(dim, type, coord, cdim)

/* calculate base address */
#define BASE_X                          "_viv_getImageAddr(image)"
#define ROW_STRIDE                      "_viv_getImageRowStride(image)"
#define SLICE_STRIDE                    "_viv_getImage3DSliceStride(image)"

#define BASE_uchar_1(type, y, z)        type "base =  (uchar *)"BASE_X";" NEWLINE
#define BASE_uchar_2(type, y, z)        type "base = ((uchar *)"BASE_X") + "ROW_STRIDE" * (uint)"y".y;" NEWLINE
#define BASE_uchar_3(type, y, z)        type "base = ((uchar *)"BASE_X") + "ROW_STRIDE" * (uint)"y".y + "SLICE_STRIDE" * (uint)"z".z;" NEWLINE
#define __BASE_uchar(num, type, y, z)   BASE_uchar_##num(type, y, z)
#define _BASE_uchar(num, type, y, z)    __BASE_uchar(num, type, y, z)
#define BASE_uchar(dim, type, y, z)     _BASE_uchar(COORD_##dim##_NUM, type, y, z)

/* load value */
#define vload_uchar(type, num, x)         "(("ToStr(type) num"*)base)["x"]"
#define vload_char(type, num, x)          "(("ToStr(type) num"*)base)["x"]"
#define vload_short(type, num, x)         "(("ToStr(type) num"*)base)["x"]"
#define vload_ushort(type, num, x)        "(("ToStr(type) num"*)base)["x"]"
#define vload_int(type, num, x)           "(("ToStr(type) num"*)base)["x"]"
#define vload_uint(type, num, x)          "(("ToStr(type) num"*)base)["x"]"
#define vload_float(type, num, x)         "(("ToStr(type) num"*)base)["x"]"
#define vload_half(type, num, x)          "vload_half"num"("x", (half*)base)"
#define __vload_TYPE(type, num, x)        vload_##type(type, num, x)
#define _vload_TYPE(type, num, x)         __vload_TYPE(type, num, x)
#define vload_TYPE(ch_data, ch_order, x)  _vload_TYPE(CH_TYPE_##ch_data, CH_ORDER##ch_order, x)

#define LoadVal(dim, ch_data, ch_order, x, type)    type"val = "vload_TYPE(ch_data, ch_order, x COORD_X(dim))";" NEWLINE

/* store value */
#define vstore_uchar(type, num, x)          "(("ToStr(type) num"*)base)["x".x] = val"
#define vstore_char(type, num, x)           "(("ToStr(type) num"*)base)["x".x] = val"
#define vstore_short(type, num, x)          "(("ToStr(type) num"*)base)["x".x] = val"
#define vstore_ushort(type, num, x)         "(("ToStr(type) num"*)base)["x".x] = val"
#define vstore_int(type, num, x)            "(("ToStr(type) num"*)base)["x".x] = val"
#define vstore_uint(type, num, x)           "(("ToStr(type) num"*)base)["x".x] = val"
#define vstore_float(type, num, x)          "(("ToStr(type) num"*)base)["x".x] = val"
#define vstore_half(type, num, x)           "vstore_half"num"(val, "x".x, (half*)base)"
#define __vstore_TYPE(type, num, x)         vstore_##type(type, num, x)
#define _vstore_TYPE(type, num, x)          __vstore_TYPE(type, num, x)
#define vstore_TYPE(ch_data, ch_order, x)   _vstore_TYPE(CH_TYPE_##ch_data, CH_ORDER##ch_order, x)
#define StoreVal(dim, ch_data, ch_order, x) vstore_TYPE(ch_data, ch_order, x /*COORD_X(dim)*/ )";" NEWLINE

/* convert value to rgba */
#define snorm8_RGBA_CvtRGBA(type)           type"rgba = max(convert_float4(val) / 127.f, -1.f);" NEWLINE
#define snorm16_RGBA_CvtRGBA(type)          type"rgba = max(convert_float4(val) / 32767.f, -1.f);" NEWLINE
#define unorm8_RGBA_CvtRGBA(type)           type"rgba = convert_float4(val) / 255.f;" NEWLINE
#define unorm16_RGBA_CvtRGBA(type)          type"rgba = convert_float4(val) / 65535.f;" NEWLINE
#define int8_RGBA_CvtRGBA(type)             type"rgba = convert_int4(val);" NEWLINE
#define int16_RGBA_CvtRGBA(type)            type"rgba = convert_int4(val);" NEWLINE
#define int32_RGBA_CvtRGBA(type)            type"rgba = convert_int4(val);" NEWLINE
#define uint8_RGBA_CvtRGBA(type)            type"rgba = convert_uint4(val);" NEWLINE
#define uint16_RGBA_CvtRGBA(type)           type"rgba = convert_uint4(val);" NEWLINE
#define uint32_RGBA_CvtRGBA(type)           type"rgba = convert_uint4(val);" NEWLINE
#define float_RGBA_CvtRGBA(type)            type"rgba = val;" NEWLINE
#define half_RGBA_CvtRGBA(type)             type"rgba = val;" NEWLINE

#define snorm8_BGRA_CvtRGBA(type)           type"rgba = max(convert_float4(val.zyxw) / 127.f, -1.f);" NEWLINE
#define snorm16_BGRA_CvtRGBA(type)          type"rgba = max(convert_float4(val.zyxw) / 32767.f, -1.f);" NEWLINE
#define unorm8_BGRA_CvtRGBA(type)           type"rgba = convert_float4(val.zyxw) / 255.f;" NEWLINE
#define unorm16_BGRA_CvtRGBA(type)          type"rgba = convert_float4(val.zyxw) / 65535.f;" NEWLINE
#define int8_BGRA_CvtRGBA(type)             type"rgba = convert_int4(val.zyxw);" NEWLINE
#define int16_BGRA_CvtRGBA(type)            type"rgba = convert_int4(val.zyxw);" NEWLINE
#define int32_BGRA_CvtRGBA(type)            type"rgba = convert_int4(val.zyxw);" NEWLINE
#define uint8_BGRA_CvtRGBA(type)            type"rgba = convert_uint4(val.zyxw);" NEWLINE
#define uint16_BGRA_CvtRGBA(type)           type"rgba = convert_uint4(val.zyxw);" NEWLINE
#define uint32_BGRA_CvtRGBA(type)           type"rgba = convert_uint4(val.zyxw);" NEWLINE
#define float_BGRA_CvtRGBA(type)            type"rgba = val.zyxw;" NEWLINE
#define half_BGRA_CvtRGBA(type)             type"rgba = val.zyxw;" NEWLINE

#define snorm8_ARGB_CvtRGBA(type)           type"rgba = max(convert_float4(val.wxyz) / 127.f, -1.f);" NEWLINE
#define snorm16_ARGB_CvtRGBA(type)          type"rgba = max(convert_float4(val.wxyz) / 32767.f, -1.f);" NEWLINE
#define unorm8_ARGB_CvtRGBA(type)           type"rgba = convert_float4(val.wxyz) / 255.f;" NEWLINE
#define unorm16_ARGB_CvtRGBA(type)          type"rgba = convert_float4(val.wxyz) / 65535.f;" NEWLINE
#define int8_ARGB_CvtRGBA(type)             type"rgba = convert_int4(val.wxyz);" NEWLINE
#define int16_ARGB_CvtRGBA(type)            type"rgba = convert_int4(val.wxyz);" NEWLINE
#define int32_ARGB_CvtRGBA(type)            type"rgba = convert_int4(val.wxyz);" NEWLINE
#define uint8_ARGB_CvtRGBA(type)            type"rgba = convert_uint4(val.wxyz);" NEWLINE
#define uint16_ARGB_CvtRGBA(type)           type"rgba = convert_uint4(val.wxyz);" NEWLINE
#define uint32_ARGB_CvtRGBA(type)           type"rgba = convert_uint4(val.wxyz;" NEWLINE
#define float_ARGB_CvtRGBA(type)            type"rgba = val.wxyz;" NEWLINE
#define half_ARGB_CvtRGBA(type)             type"rgba = val.wxyz;" NEWLINE

#define snorm8_R_CvtRGBA(type)              type"rgba = max((float4)((float)val / 127.f, .0f, .0f, 1.f), -1.f);" NEWLINE
#define snorm16_R_CvtRGBA(type)             type"rgba = max((float4)((float)val / 32767.f, .0f, .0f, 1.f), -1.f);" NEWLINE
#define unorm8_R_CvtRGBA(type)              type"rgba = (float4)((float)val / 255.f, .0f, .0f, 1.f);" NEWLINE
#define unorm16_R_CvtRGBA(type)             type"rgba = (float4)((float)val / 65535.f, .0f, .0f, 1.f);" NEWLINE
#define int8_R_CvtRGBA(type)                type"rgba = (int4)(val, 0, 0, 1);" NEWLINE
#define int16_R_CvtRGBA(type)               type"rgba = (int4)(val, 0, 0, 1);" NEWLINE
#define int32_R_CvtRGBA(type)               type"rgba = (int4)(val, 0, 0, 1);" NEWLINE
#define uint8_R_CvtRGBA(type)               type"rgba = (uint4)(val, 0, 0, 1);" NEWLINE
#define uint16_R_CvtRGBA(type)              type"rgba = (uint4)(val, 0, 0, 1);" NEWLINE
#define uint32_R_CvtRGBA(type)              type"rgba = (uint4)(val, 0, 0, 1);" NEWLINE
#define float_R_CvtRGBA(type)               type"rgba = (float4)(val, .0f, .0f, 1.f);" NEWLINE
#define half_R_CvtRGBA(type)                type"rgba = (float4)(val, .0f, .0f, 1.f);" NEWLINE
#define snorm8_Rx_CvtRGBA(type)             snorm8_R_CvtRGBA(type)  NEWLINE
#define snorm16_Rx_CvtRGBA(type)            snorm16_R_CvtRGBA(type) NEWLINE
#define unorm8_Rx_CvtRGBA(type)             unorm8_R_CvtRGBA(type)  NEWLINE
#define unorm16_Rx_CvtRGBA(type)            unorm16_R_CvtRGBA(type) NEWLINE
#define int8_Rx_CvtRGBA(type)               int8_R_CvtRGBA(type)    NEWLINE
#define int16_Rx_CvtRGBA(type)              int16_R_CvtRGBA(type)   NEWLINE
#define int32_Rx_CvtRGBA(type)              int32_R_CvtRGBA(type)   NEWLINE
#define uint8_Rx_CvtRGBA(type)              uint8_R_CvtRGBA(type)   NEWLINE
#define uint16_Rx_CvtRGBA(type)             uint16_R_CvtRGBA(type)  NEWLINE
#define uint32_Rx_CvtRGBA(type)             uint32_R_CvtRGBA(type)  NEWLINE
#define float_Rx_CvtRGBA(type)              float_R_CvtRGBA(type)   NEWLINE
#define half_Rx_CvtRGBA(type)               half_R_CvtRGBA(type)    NEWLINE

#define snorm8_RG_CvtRGBA(type)             type"rgba = max((float4)(convert_float2(val) / 127.f, .0f, 1.f), -1.f);" NEWLINE
#define snorm16_RG_CvtRGBA(type)            type"rgba = max((float4)(convert_float2(val) / 32767.f, .0f, 1.f), -1.f);" NEWLINE
#define unorm8_RG_CvtRGBA(type)             type"rgba = (float4)(convert_float2(val) / 255.f, .0f, 1.f);" NEWLINE
#define unorm16_RG_CvtRGBA(type)            type"rgba = (float4)(convert_float2(val) / 65535.f, .0f, 1.f);" NEWLINE
#define int8_RG_CvtRGBA(type)               type"rgba = (int4)(val, 0, 1);" NEWLINE
#define int16_RG_CvtRGBA(type)              type"rgba = (int4)(val, 0, 1);" NEWLINE
#define int32_RG_CvtRGBA(type)              type"rgba = (int4)(val, 0, 1);" NEWLINE
#define uint8_RG_CvtRGBA(type)              type"rgba = (uint4)(val, 0, 1);" NEWLINE
#define uint16_RG_CvtRGBA(type)             type"rgba = (uint4)(val, 0, 1);" NEWLINE
#define uint32_RG_CvtRGBA(type)             type"rgba = (uint4)(val, 0, 1);" NEWLINE
#define float_RG_CvtRGBA(type)              type"rgba = (float4)(val, .0f, 1.f);" NEWLINE
#define half_RG_CvtRGBA(type)               type"rgba = (float4)(val, .0f, 1.f);" NEWLINE
#define snorm8_RGx_CvtRGBA(type)            snorm8_RG_CvtRGBA(type)  NEWLINE
#define snorm16_RGx_CvtRGBA(type)           snorm16_RG_CvtRGBA(type) NEWLINE
#define unorm8_RGx_CvtRGBA(type)            unorm8_RG_CvtRGBA(type)  NEWLINE
#define unorm16_RGx_CvtRGBA(type)           unorm16_RG_CvtRGBA(type) NEWLINE
#define int8_RGx_CvtRGBA(type)              int8_RG_CvtRGBA(type)    NEWLINE
#define int16_RGx_CvtRGBA(type)             int16_RG_CvtRGBA(type)   NEWLINE
#define int32_RGx_CvtRGBA(type)             int32_RG_CvtRGBA(type)   NEWLINE
#define uint8_RGx_CvtRGBA(type)             uint8_RG_CvtRGBA(type)   NEWLINE
#define uint16_RGx_CvtRGBA(type)            uint16_RG_CvtRGBA(type)  NEWLINE
#define uint32_RGx_CvtRGBA(type)            uint32_RG_CvtRGBA(type)  NEWLINE
#define float_RGx_CvtRGBA(type)             float_RG_CvtRGBA(type)   NEWLINE
#define half_RGx_CvtRGBA(type)              half_RG_CvtRGBA(type)    NEWLINE

#define snorm8_A_CvtRGBA(type)              type"rgba = max((float4)(.0f, .0f, .0f, (float)val / 127.f), -1.f);" NEWLINE
#define snorm16_A_CvtRGBA(type)             type"rgba = max((float4)(.0f, .0f, .0f, (float)val / 32767.f), -1.f);" NEWLINE
#define unorm8_A_CvtRGBA(type)              type"rgba = (float4)(.0f, .0f, .0f, (float)val / 255.f);" NEWLINE
#define unorm16_A_CvtRGBA(type)             type"rgba = (float4)(.0f, .0f, .0f, (float)val / 65535.f);" NEWLINE
#define int8_A_CvtRGBA(type)                type"rgba = (int4)(0, 0, 0, val);" NEWLINE
#define int16_A_CvtRGBA(type)               type"rgba = (int4)(0, 0, 0, val);" NEWLINE
#define int32_A_CvtRGBA(type)               type"rgba = (int4)(0, 0, 0, val);" NEWLINE
#define uint8_A_CvtRGBA(type)               type"rgba = (uint4)(0, 0, 0, val);" NEWLINE
#define uint16_A_CvtRGBA(type)              type"rgba = (uint4)(0, 0, 0, val);" NEWLINE
#define uint32_A_CvtRGBA(type)              type"rgba = (uint4)(0, 0, 0, val);" NEWLINE
#define float_A_CvtRGBA(type)               type"rgba = (float4)(.0f, .0f, .0f, val);" NEWLINE
#define half_A_CvtRGBA(type)                type"rgba = (float4)(.0f, .0f, .0f, val);" NEWLINE

#define snorm8_RA_CvtRGBA(type)             type"rgba = max((float4)((float)val.x / 127.f, .0f, .0f, (float)val.y / 127.f), -1.f);" NEWLINE
#define snorm16_RA_CvtRGBA(type)            type"rgba = max((float4)((float)val.x / 32767.f, .0f, .0f, (float)val.y / 32767.f), -1.f);" NEWLINE
#define unorm8_RA_CvtRGBA(type)             type"rgba = (float4)((float)val.x / 255.f, .0f, .0f, (float)val.y / 255.f);" NEWLINE
#define unorm16_RA_CvtRGBA(type)            type"rgba = (float4)((float)val.x / 65535.f, .0f, .0f, (float)val.y / 65535.f);" NEWLINE
#define int8_RA_CvtRGBA(type)               type"rgba = (int4)(val.x, 0, 0 val.y);" NEWLINE
#define int16_RA_CvtRGBA(type)              type"rgba = (int4)(val.x, 0, 0 val.y);" NEWLINE
#define int32_RA_CvtRGBA(type)              type"rgba = (int4)(val.x, 0, 0 val.y);" NEWLINE
#define uint8_RA_CvtRGBA(type)              type"rgba = (uint4)(val.x, 0, 0 val.y);" NEWLINE
#define uint16_RA_CvtRGBA(type)             type"rgba = (uint4)(val.x, 0, 0 val.y);" NEWLINE
#define uint32_RA_CvtRGBA(type)             type"rgba = (uint4)(val.x, 0, 0 val.y);" NEWLINE
#define float_RA_CvtRGBA(type)              type"rgba = (float4)(val.x, .0f, .0f, val.y);" NEWLINE
#define half_RA_CvtRGBA(type)               type"rgba = (float4)(val.x, .0f, .0f, val.y);" NEWLINE

#define snorm8_INTENSITY_CvtRGBA(type)      type"rgba = max((float4)((float3)((float)val / 127.f ), 1.f), -1.f);" NEWLINE
#define snorm16_INTENSITY_CvtRGBA(type)     type"rgba = max((float4)((float3)((float)val / 32767.f), 1.f), -1.f);" NEWLINE
#define unorm8_INTENSITY_CvtRGBA(type)      type"rgba = (float4)((float3)((float)val / 255.f), 1.f);" NEWLINE
#define unorm16_INTENSITY_CvtRGBA(type)     type"rgba = (float4)((float3)((float)val / 65535.f), 1.f);" NEWLINE
#define float_INTENSITY_CvtRGBA(type)       type"rgba = (float4)((float3)(val), 1.f);" NEWLINE
#define half_INTENSITY_CvtRGBA(type)        type"rgba = (float4)((float3)(val), 1.f);" NEWLINE

#define snorm8_LUMINANCE_CvtRGBA(type)      type"rgba = max((float4)((float)val / 127.f), -1.f);" NEWLINE
#define snorm16_LUMINANCE_CvtRGBA(type)     type"rgba = max((float4)((float)val / 32767.f), -1.f);" NEWLINE
#define unorm8_LUMINANCE_CvtRGBA(type)      type"rgba = (float4)((float)val / 255.f);" NEWLINE
#define unorm16_LUMINANCE_CvtRGBA(type)     type"rgba = (float4)((float)val / 65535.f );" NEWLINE
#define float_LUMINANCE_CvtRGBA(type)       type"rgba = (float4)(val);" NEWLINE
#define half_LUMINANCE_CvtRGBA(type)        type"rgba = (float4)(val);" NEWLINE

#define unorm555_RGB_CvtRGBA(type)          type"rgba = (float4)((float)((val >> 10)&0x1f)   (float)((val >> 5 )&0x1f), (float)(val&0x1f), 32.f) / 32.f;" NEWLINE
#define unorm565_RGB_CvtRGBA(type)          type"rgba = (float4)((float)((val >> 10)&0x1f)   (float)((val >> 5 )&0x1f), (float)(val&0x1f), 32.f) / 32.f;" NEWLINE
#define unorm101010_RGB_CvtRGBA(type)       type"rgba = (float4)((float)((val >> 20)&0x3ff), (float)((val >> 10)&0x3ff), (float)(val&0x3ff), 1024.f) / 1024.f;" NEWLINE
#define unorm555_RGBx_CvtRGBA(type)         unorm555_RGB_CvtRGBA(type)      NEWLINE
#define unorm565_RGBx_CvtRGBA(type)         unorm565_RGB_CvtRGBA(type)      NEWLINE
#define unorm101010_RGBx_CvtRGBA(type)      unorm101010_RGB_CvtRGBA(type)   NEWLINE

/* convert rgba to value */
#define snorm8_RGBA_CvtVALUE(type)           type"val = convert_char4_sat  (.5f + rgba * 127.f);" NEWLINE
#define snorm16_RGBA_CvtVALUE(type)          type"val = convert_short4_sat (.5f + rgba * 32767.f);" NEWLINE
#define unorm8_RGBA_CvtVALUE(type)           type"val = convert_uchar4_sat (.5f + rgba * 255.f);" NEWLINE
#define unorm16_RGBA_CvtVALUE(type)          type"val = convert_ushort4_sat(.5f + rgba * 65535.f);" NEWLINE
#define int8_RGBA_CvtVALUE(type)             type"val = convert_char4_sat  (rgba);" NEWLINE
#define int16_RGBA_CvtVALUE(type)            type"val = convert_short4_sat (rgba);" NEWLINE
#define int32_RGBA_CvtVALUE(type)            type"val = convert_int4       (rgba);" NEWLINE
#define uint8_RGBA_CvtVALUE(type)            type"val = convert_uchar4_sat (rgba);" NEWLINE
#define uint16_RGBA_CvtVALUE(type)           type"val = convert_ushort4_sat(rgba);" NEWLINE
#define uint32_RGBA_CvtVALUE(type)           type"val = convert_uint4      (rgba);" NEWLINE
#define float_RGBA_CvtVALUE(type)            type"val = rgba;" NEWLINE
#define half_RGBA_CvtVALUE(type)             type"val = rgba;" NEWLINE

#define snorm8_BGRA_CvtVALUE(type)           type"val = convert_char4_sat  (.5f + rgba.zyxw * 127.f);" NEWLINE
#define snorm16_BGRA_CvtVALUE(type)          type"val = convert_short4_sat (.5f + rgba.zyxw * 32767.f);" NEWLINE
#define unorm8_BGRA_CvtVALUE(type)           type"val = convert_uchar4_sat (.5f + rgba.zyxw * 255.f);" NEWLINE
#define unorm16_BGRA_CvtVALUE(type)          type"val = convert_ushort4_sat(.5f + rgba.zyxw * 65535.f);" NEWLINE
#define int8_BGRA_CvtVALUE(type)             type"val = convert_char4_sat  (rgba.zyxw);" NEWLINE
#define int16_BGRA_CvtVALUE(type)            type"val = convert_short4_sat (rgba.zyxw);" NEWLINE
#define int32_BGRA_CvtVALUE(type)            type"val = convert_int4       (rgba.zyxw);" NEWLINE
#define uint8_BGRA_CvtVALUE(type)            type"val = convert_uchar4_sat (rgba.zyxw);" NEWLINE
#define uint16_BGRA_CvtVALUE(type)           type"val = convert_ushort4_sat(rgba.zyxw);" NEWLINE
#define uint32_BGRA_CvtVALUE(type)           type"val = convert_uint4      (rgba.zyxw);" NEWLINE
#define float_BGRA_CvtVALUE(type)            type"val = rgba.zyxw;" NEWLINE
#define half_BGRA_CvtVALUE(type)             type"val = rgba.zyxw;" NEWLINE

#define snorm8_ARGB_CvtVALUE(type)           type"val = convert_char4_sat  (.5f + rgba.wxyz * 127.f);" NEWLINE
#define snorm16_ARGB_CvtVALUE(type)          type"val = convert_short4_sat (.5f + rgba.wxyz * 32767.f);" NEWLINE
#define unorm8_ARGB_CvtVALUE(type)           type"val = convert_uchar4_sat (.5f + rgba.wxyz * 255.f);" NEWLINE
#define unorm16_ARGB_CvtVALUE(type)          type"val = convert_ushort4_sat(.5f + rgba.wxyz * 65535.f);" NEWLINE
#define int8_ARGB_CvtVALUE(type)             type"val = convert_char4_sat  (rgba.wxyz);" NEWLINE
#define int16_ARGB_CvtVALUE(type)            type"val = convert_short4_sat (rgba.wxyz);" NEWLINE
#define int32_ARGB_CvtVALUE(type)            type"val = convert_int4       (rgba.wxyz);" NEWLINE
#define uint8_ARGB_CvtVALUE(type)            type"val = convert_uchar4_sat (rgba.wxyz);" NEWLINE
#define uint16_ARGB_CvtVALUE(type)           type"val = convert_ushort4_sat(rgba.wxyz);" NEWLINE
#define uint32_ARGB_CvtVALUE(type)           type"val = convert_uint4      (rgba.wxyz;" NEWLINE
#define float_ARGB_CvtVALUE(type)            type"val = rgba.wxyz;" NEWLINE
#define half_ARGB_CvtVALUE(type)             type"val = rgba.wxyz;" NEWLINE

#define snorm8_R_CvtVALUE(type)              type"val = convert_char_sat  (.5f + rgba.x * 127.f);" NEWLINE
#define snorm16_R_CvtVALUE(type)             type"val = convert_short_sat (.5f + rgba.x * 32767.f);" NEWLINE
#define unorm8_R_CvtVALUE(type)              type"val = convert_uchar_sat (.5f + rgba.x * 255.f);" NEWLINE
#define unorm16_R_CvtVALUE(type)             type"val = convert_ushort_sat(.5f + rgba.x * 65535.f);" NEWLINE
#define int8_R_CvtVALUE(type)                type"val = convert_char_sat  (rgba.x);" NEWLINE
#define int16_R_CvtVALUE(type)               type"val = convert_short_sat (rgba.x);" NEWLINE
#define int32_R_CvtVALUE(type)               type"val = convert_int       (rgba.x);" NEWLINE
#define uint8_R_CvtVALUE(type)               type"val = convert_uchar_sat (rgba.x);" NEWLINE
#define uint16_R_CvtVALUE(type)              type"val = convert_ushort_sat(rgba.x);" NEWLINE
#define uint32_R_CvtVALUE(type)              type"val = convert_uint      (rgba.x);" NEWLINE
#define float_R_CvtVALUE(type)               type"val = (float) (rgba.x);" NEWLINE
#define half_R_CvtVALUE(type)                type"val = (float) (rgba.x);" NEWLINE
#define snorm8_Rx_CvtVALUE(type)             snorm8_R_CvtVALUE(type)  NEWLINE
#define snorm16_Rx_CvtVALUE(type)            snorm16_R_CvtVALUE(type) NEWLINE
#define unorm8_Rx_CvtVALUE(type)             unorm8_R_CvtVALUE(type)  NEWLINE
#define unorm16_Rx_CvtVALUE(type)            unorm16_R_CvtVALUE(type) NEWLINE
#define int8_Rx_CvtVALUE(type)               int8_R_CvtVALUE(type)    NEWLINE
#define int16_Rx_CvtVALUE(type)              int16_R_CvtVALUE(type)   NEWLINE
#define int32_Rx_CvtVALUE(type)              int32_R_CvtVALUE(type)   NEWLINE
#define uint8_Rx_CvtVALUE(type)              uint8_R_CvtVALUE(type)   NEWLINE
#define uint16_Rx_CvtVALUE(type)             uint16_R_CvtVALUE(type)  NEWLINE
#define uint32_Rx_CvtVALUE(type)             uint32_R_CvtVALUE(type)  NEWLINE
#define float_Rx_CvtVALUE(type)              float_R_CvtVALUE(type)   NEWLINE
#define half_Rx_CvtVALUE(type)               half_R_CvtVALUE(type)    NEWLINE

#define snorm8_RG_CvtVALUE(type)             type"val = convert_char2_sat  (.5f + rgba.xy * 127.f);" NEWLINE
#define snorm16_RG_CvtVALUE(type)            type"val = convert_short2_sat (.5f + rgba.xy * 32767.f);" NEWLINE
#define unorm8_RG_CvtVALUE(type)             type"val = convert_uchar2_sat (.5f + rgba.xy * 255.f);" NEWLINE
#define unorm16_RG_CvtVALUE(type)            type"val = convert_ushort2_sat(.5f + rgba.xy * 65535.f);" NEWLINE
#define int8_RG_CvtVALUE(type)               type"val = convert_char2_sat  (rgba.xy);" NEWLINE
#define int16_RG_CvtVALUE(type)              type"val = convert_short2_sat (rgba.xy);" NEWLINE
#define int32_RG_CvtVALUE(type)              type"val = convert_int2       (rgba.xy);" NEWLINE
#define uint8_RG_CvtVALUE(type)              type"val = convert_uchar2_sat (rgba.xy);" NEWLINE
#define uint16_RG_CvtVALUE(type)             type"val = convert_ushort2_sat(rgba.xy);" NEWLINE
#define uint32_RG_CvtVALUE(type)             type"val = convert_uint2      (rgba.xy);" NEWLINE
#define float_RG_CvtVALUE(type)              type"val = (float2) (rgba.xy);" NEWLINE
#define half_RG_CvtVALUE(type)               type"val = (float2) (rgba.xy);" NEWLINE
#define snorm8_RGx_CvtVALUE(type)            snorm8_RG_CvtVALUE(type)  NEWLINE
#define snorm16_RGx_CvtVALUE(type)           snorm16_RG_CvtVALUE(type) NEWLINE
#define unorm8_RGx_CvtVALUE(type)            unorm8_RG_CvtVALUE(type)  NEWLINE
#define unorm16_RGx_CvtVALUE(type)           unorm16_RG_CvtVALUE(type) NEWLINE
#define int8_RGx_CvtVALUE(type)              int8_RG_CvtVALUE(type)    NEWLINE
#define int16_RGx_CvtVALUE(type)             int16_RG_CvtVALUE(type)   NEWLINE
#define int32_RGx_CvtVALUE(type)             int32_RG_CvtVALUE(type)   NEWLINE
#define uint8_RGx_CvtVALUE(type)             uint8_RG_CvtVALUE(type)   NEWLINE
#define uint16_RGx_CvtVALUE(type)            uint16_RG_CvtVALUE(type)  NEWLINE
#define uint32_RGx_CvtVALUE(type)            uint32_RG_CvtVALUE(type)  NEWLINE
#define float_RGx_CvtVALUE(type)             float_RG_CvtVALUE(type)   NEWLINE
#define half_RGx_CvtVALUE(type)              half_RG_CvtVALUE(type)    NEWLINE

#define snorm8_A_CvtVALUE(type)              type"val = convert_char_sat  (.5f + rgba.w * 127.f);" NEWLINE
#define snorm16_A_CvtVALUE(type)             type"val = convert_short_sat (.5f + rgba.w * 32767.f);" NEWLINE
#define unorm8_A_CvtVALUE(type)              type"val = convert_uchar_sat (.5f + rgba.w * 255.f);" NEWLINE
#define unorm16_A_CvtVALUE(type)             type"val = convert_ushort_sat(.5f + rgba.w * 65535.f);" NEWLINE
#define int8_A_CvtVALUE(type)                type"val = convert_char_sat  (rgba.w);" NEWLINE
#define int16_A_CvtVALUE(type)               type"val = convert_short_sat (rgba.w);" NEWLINE
#define int32_A_CvtVALUE(type)               type"val = convert_int       (rgba.w);" NEWLINE
#define uint8_A_CvtVALUE(type)               type"val = convert_uchar_sat (rgba.w);" NEWLINE
#define uint16_A_CvtVALUE(type)              type"val = convert_ushort_sat(rgba.w);" NEWLINE
#define uint32_A_CvtVALUE(type)              type"val = convert_uint      (rgba.w);" NEWLINE
#define float_A_CvtVALUE(type)               type"val = (float) (rgba.w);" NEWLINE
#define half_A_CvtVALUE(type)                type"val = (float) (rgba.w);" NEWLINE

#define snorm8_RA_CvtVALUE(type)             type"val = convert_char2_sat  (.5f + rgba.xw * 127.f);" NEWLINE
#define snorm16_RA_CvtVALUE(type)            type"val = convert_short2_sat (.5f + rgba.xw * 32767.f);" NEWLINE
#define unorm8_RA_CvtVALUE(type)             type"val = convert_uchar2_sat (.5f + rgba.xw * 255.f);" NEWLINE
#define unorm16_RA_CvtVALUE(type)            type"val = convert_ushort2_sat(.5f + rgba.xw * 65535.f);" NEWLINE
#define int8_RA_CvtVALUE(type)               type"val = convert_char2_sat  (rgba.xw);" NEWLINE
#define int16_RA_CvtVALUE(type)              type"val = convert_short2_sat (rgba.xw);" NEWLINE
#define int32_RA_CvtVALUE(type)              type"val = convert_int2       (rgba.xw);" NEWLINE
#define uint8_RA_CvtVALUE(type)              type"val = convert_uchar2_sat (rgba.xw);" NEWLINE
#define uint16_RA_CvtVALUE(type)             type"val = convert_ushort2_sat(rgba.xw);" NEWLINE
#define uint32_RA_CvtVALUE(type)             type"val = convert_uint2      (rgba.xw);" NEWLINE
#define float_RA_CvtVALUE(type)              type"val = (float2) (rgba.xw);" NEWLINE
#define half_RA_CvtVALUE(type)               type"val = (float2) (rgba.xw);" NEWLINE

#define snorm8_INTENSITY_CvtVALUE(type)      type"val = convert_char_sat  (.5f + rgba.x * 127.f);" NEWLINE
#define snorm16_INTENSITY_CvtVALUE(type)     type"val = convert_short_sat (.5f + rgba.x * 12768.f);" NEWLINE
#define unorm8_INTENSITY_CvtVALUE(type)      type"val = convert_uchar_sat (.5f + rgba.x * 255.f);" NEWLINE
#define unorm16_INTENSITY_CvtVALUE(type)     type"val = convert_ushort_sat(.5f + rgba.x * 65535.f);" NEWLINE
#define float_INTENSITY_CvtVALUE(type)       type"val = (float)(rgba.x);" NEWLINE
#define half_INTENSITY_CvtVALUE(type)        type"val = (float)(rgba.x);" NEWLINE

#define snorm8_LUMINANCE_CvtVALUE(type)      type"val = convert_char_sat  (.5f + rgba.x * 127.f);" NEWLINE
#define snorm16_LUMINANCE_CvtVALUE(type)     type"val = convert_short_sat (.5f + rgba.x * 12768.f);" NEWLINE
#define unorm8_LUMINANCE_CvtVALUE(type)      type"val = convert_uchar_sat (.5f + rgba.x * 255.f);" NEWLINE
#define unorm16_LUMINANCE_CvtVALUE(type)     type"val = convert_ushor_sat (.5f + rgba.x * 65535.f);" NEWLINE
#define float_LUMINANCE_CvtVALUE(type)       type"val = (float)(rgba.x);" NEWLINE
#define half_LUMINANCE_CvtVALUE(type)        type"val = (float)(rgba.x);" NEWLINE

/* linear support*/
#define LINEAR_snorm8(x)        x
#define LINEAR_snorm16(x)       x
#define LINEAR_unorm8(x)        x
#define LINEAR_unorm16(x)       x
#define LINEAR_unorm555(x)      x
#define LINEAR_unorm565(x)      x
#define LINEAR_unorm101010(x)   x
#define LINEAR_float(x)         x
#define LINEAR_half(x)          x
#define LINEAR_int8(x)          ""
#define LINEAR_int16(x)         ""
#define LINEAR_int32(x)         ""
#define LINEAR_uint8(x)         ""
#define LINEAR_uint16(x)        ""
#define LINEAR_uint32(x)        ""
#define LINEAR_int(x)           ""
#define LINEAR_uint(x)          ""

#define LINEAR_SUPPORT(ch_data, x)  LINEAR_##ch_data(x)

#define _LINEAR_1D(dim)    \
"    "DIM_NUM_FLOAT(dim)" oneMinusFract = 1.f - fract;" NEWLINE \
"    rgba = oneMinusFract"COORD_X(1d)" * T0 + fract"COORD_X(1d)" * T1;" NEWLINE

#define _LINEAR_2D(dim)    \
"    "DIM_NUM_FLOAT(dim)" oneMinusFract = 1.f - fract;" NEWLINE \
"    rgba = oneMinusFract.x * oneMinusFract.y * T00 + " NEWLINE \
"                   fract.x * oneMinusFract.y * T10 + " NEWLINE \
"           oneMinusFract.x *         fract.y * T01 + " NEWLINE \
"                   fract.x *         fract.y * T11;" NEWLINE
#define _LINEAR_3D(dim)    \
"    "DIM_NUM_FLOAT(dim)" oneMinusFract = 1.f - fract; " NEWLINE \
"    float4 t0, t1, t2, t3, t4, t5;" NEWLINE \
"    t0 = oneMinusFract.x * T000 + fract.x * T100; " NEWLINE \
"    t1 = oneMinusFract.x * T010 + fract.x * T110; " NEWLINE \
"    t2 = oneMinusFract.x * T001 + fract.x * T101; " NEWLINE \
"    t3 = oneMinusFract.x * T011 + fract.x * T111; " NEWLINE \
"    t4 = oneMinusFract.y * t0   + fract.y * t1; " NEWLINE \
"    t5 = oneMinusFract.y * t2   + fract.y * t3; " NEWLINE \
"    rgba = oneMinusFract.z * t4 + fract.z * t5; " NEWLINE

#define __CALC_LINEAR(num, dim) _LINEAR_##num##D(dim)
#define _CALC_LINEAR(num, dim)  __CALC_LINEAR(num, dim)
#define CALC_LINEAR(dim)        _CALC_LINEAR(DIM_##dim##_NUM, dim)

/* address */
#define _none_none          ""
#define _none_x             ".x"
#define _none_xy            ".xy"
#define _none_xyz           ".xyz"
#define _x_none             ".x"
#define _xy_none            ".xy"
#define _xyz_none           ".xyz"
#define _x_x                ".x"
#define _xy_x               ".x"
#define _xyz_x              ".x"
#define _xy_xy              ".xy"
#define _xyz_xy             ".xy"
#define _xyz_xyz            ".xyz"

#define _COORD_ADDRESS_1d           none
#define _COORD_ADDRESS_1dbuffer     none
#define _COORD_ADDRESS_2d           xy
#define _COORD_ADDRESS_3d           xyz
#define _COORD_ADDRESS_1darray      x
#define _COORD_ADDRESS_2darray      xy
#define __COORD_ADDRESS(addr, dim)  _##dim##_##addr
#define _COORD_ADDRESS(x, y)        __COORD_ADDRESS(x, y)
#define COORD_ADDRESS(dim)          _COORD_ADDRESS(_COORD_ADDRESS_##dim, _COORD_ADDRESS_1d)
#define COORD_ADDRESS2(dim)         _COORD_ADDRESS(_COORD_ADDRESS_##dim, COORD_##dim##_CVT)

#define _DIM_ADDRESS_1d             none
#define _DIM_ADDRESS_1dbuffer       none
#define _DIM_ADDRESS_2d             xy
#define _DIM_ADDRESS_3d             xyz
#define _DIM_ADDRESS_1darray        xy
#define _DIM_ADDRESS_2darray        xyz
#define __DIM_ADDRESS(addr, dim)    _##dim##_##addr
#define _DIM_ADDRESS(x, y)          __DIM_ADDRESS(x, y)
#define DIM_ADDRESS(dim)            _DIM_ADDRESS(_DIM_ADDRESS_##dim, _DIM_ADDRESS_1d)
#define DIM_ADDRESS2(dim)           _DIM_ADDRESS(_DIM_ADDRESS_##dim, COORD_##dim##_CVT)

#define _INDEX_1d           ""
#define _INDEX_1dbuffer     ""
#define _INDEX_2d           ""
#define _INDEX_3d           ""
#define _INDEX_1darray      ".y"
#define _INDEX_2darray      ".z"
#define __INDEX_DIM(dmap)   dmap
#define _INDEX_DIM(dmap)    __INDEX_DIM(dmap)
#define INDEX_ADDRESS(dim)  _INDEX_DIM(_INDEX_##dim)

#define _SELECT_IMAGE_array(dim, dst, src)      "    "dst INDEX_ADDRESS(dim) " = clamp((int)rint("src INDEX_ADDRESS(dim)"), 0, imageSize" INDEX_ADDRESS(dim)" - 1);" NEWLINE
#define _SELECT_IMAGE_1darray(dim, dst, src)    _SELECT_IMAGE_array(dim, dst, src)
#define _SELECT_IMAGE_2darray(dim, dst, src)    _SELECT_IMAGE_array(dim, dst, src)
#define _SELECT_IMAGE_1dbuffer(dim, dst, src)   ""
#define _SELECT_IMAGE_1d(dim, dst, src)         ""
#define _SELECT_IMAGE_2d(dim, dst, src)         ""
#define _SELECT_IMAGE_3d(dim, dst, src)         ""
#define SELECT_IMAGE(dim, dst, src)             _SELECT_IMAGE_##dim(dim, dst, src)

#define _SELECT_COORD_array(dim)      "    fcoord"INDEX_ADDRESS(dim)" = coord"INDEX_ADDRESS(dim)";" NEWLINE
#define _SELECT_COORD_1darray(dim)    _SELECT_COORD_array(dim)
#define _SELECT_COORD_2darray(dim)    _SELECT_COORD_array(dim)
#define _SELECT_COORD_1dbuffer(dim)   ""
#define _SELECT_COORD_1d(dim)         ""
#define _SELECT_COORD_2d(dim)         ""
#define _SELECT_COORD_3d(dim)         ""
#define SELECT_COORD(dim)             _SELECT_COORD_##dim(dim)

/* nearest none */
#define VIR_READ_IMAGE_LOAD_STORE_none_nearest(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_nearest_" ToStr(normCoord) "Coord_none_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"_rte(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    " BASE_uchar(dim, "uchar *", "icoord", "icoord") \
    "    " LoadVal(dim, ch_data, ch_order, "icoord", Load_R_Type(ch_data, ch_order)" ") \
    "    " ch_data##ch_order##_CvtRGBA(ToStr(retType)"4 ") \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* nearest clamp */
#define VIR_READ_IMAGE_LOAD_STORE_clamp_nearest(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_nearest_" ToStr(normCoord) "Coord_clamp_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    fcoord = clamp(fcoord, ("COORD_NUM_FLOAT(dim)")(0.0), convert_"COORD_NUM_FLOAT(dim)"(imageSize) - 1.f);" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    " BASE_uchar(dim, "uchar *", "icoord", "icoord") \
    "    " LoadVal(dim, ch_data, ch_order, "icoord", Load_R_Type(ch_data, ch_order)" ") \
    "    " ch_data##ch_order##_CvtRGBA(ToStr(retType)"4 ") \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* nearest border */
/* border judge */
#define BORDER_JUDGE_OK_1(dim)      "(icoord"COORD_X(dim)" >= 0 && icoord"COORD_X(dim)" < imageSize"COORD_X(dim)")"
#define BORDER_JUDGE_OK_2(dim)      "(icoord.x >= 0 && icoord.x < imageSize.x && icoord.y >= 0 && icoord.y < imageSize.y)"
#define BORDER_JUDGE_OK_3(dim)      "(icoord.x >= 0 && icoord.x < imageSize.x && icoord.y >= 0 && icoord.y < imageSize.y && icoord.z >= 0 && icoord.z < imageSize.z)"
#define __BORDER_JUDGE_OK(num, dim) BORDER_JUDGE_OK_##num(dim)
#define _BORDER_JUDGE_OK(num, dim)  __BORDER_JUDGE_OK(num, dim)
#define BORDER_JUDGE_OK(dim)        _BORDER_JUDGE_OK(DIM_##dim##_NUM, dim)
#define VIR_READ_IMAGE_LOAD_STORE_border_nearest(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_nearest_" ToStr(normCoord) "Coord_border_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"_rtn(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    if (!" BORDER_JUDGE_OK(dim) ") {" NEWLINE \
    "        float4 rgba = "BORDER_CLR##ch_order";" NEWLINE \
    "        return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "    }" NEWLINE \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    " BASE_uchar(dim, "uchar *", "icoord", "icoord") \
    "    " LoadVal(dim, ch_data, ch_order, "icoord", Load_R_Type(ch_data, ch_order)" ") \
    "    " ch_data##ch_order##_CvtRGBA(ToStr(retType)"4 ") \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* nearest wrap */
#define VIR_READ_IMAGE_LOAD_STORE_wrap_nearest(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_nearest_" ToStr(normCoord) "Coord_wrap_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    "COORD_NUM_FLOAT(dim)" fcoord;" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = coord" COORD_ADDRESS2(dim)" - floor(coord" COORD_ADDRESS2(dim)");" NEWLINE \
    SELECT_COORD(dim) \
    "    " normCoord##_COORD(dim, "", "fcoord", DIM_ADDRESS(dim)) \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"_rtn(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = viv_min(icoord"COORD_ADDRESS(dim)", imageSize"COORD_ADDRESS(dim)" - 1);" NEWLINE \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    " BASE_uchar(dim, "uchar *", "icoord", "icoord") \
    "    " LoadVal(dim, ch_data, ch_order, "icoord", Load_R_Type(ch_data, ch_order)" ") \
    "    " ch_data##ch_order##_CvtRGBA(ToStr(retType)"4 ") \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* nearest mirror */
#define VIR_READ_IMAGE_LOAD_STORE_mirror_nearest(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_nearest_" ToStr(normCoord) "Coord_mirror_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    "DIM_NUM_FLOAT(dim)" fcoord0 = 2.f * floor(.5f * coord" COORD_ADDRESS2(dim)" + .5f);" NEWLINE \
    "    "COORD_NUM_FLOAT(dim)" fcoord;" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = viv_fabs(coord" COORD_ADDRESS2(dim)" - fcoord0);" NEWLINE \
    SELECT_COORD(dim) \
    "    " normCoord##_COORD(dim, "", "fcoord", DIM_ADDRESS(dim)) \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord "COORD_ADDRESS(dim)"= convert_"DIM_NUM_INT(dim)"_rtn(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = viv_min(icoord"COORD_ADDRESS(dim)", imageSize"COORD_ADDRESS(dim)" - 1);" NEWLINE \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    " BASE_uchar(dim, "uchar *", "icoord", "icoord") \
    "    " LoadVal(dim, ch_data, ch_order, "icoord", Load_R_Type(ch_data, ch_order)" ") \
    "    " ch_data##ch_order##_CvtRGBA(ToStr(retType)"4 ") \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

#define VIR_READ_IMAGE_LOAD_STORE_nearest_all(normCoord, coordType, ch_data, dim, ch_order)    \
    VIR_READ_IMAGE_LOAD_STORE_none_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)   \
    VIR_READ_IMAGE_LOAD_STORE_clamp_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)  \
    VIR_READ_IMAGE_LOAD_STORE_border_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order) \
    VIR_READ_IMAGE_LOAD_STORE_wrap_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)   \
    VIR_READ_IMAGE_LOAD_STORE_mirror_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)
#define VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(normCoord, coordType, ch_data, dim, ch_order)    \
    VIR_READ_IMAGE_LOAD_STORE_none_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)   \
    VIR_READ_IMAGE_LOAD_STORE_clamp_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)  \
    VIR_READ_IMAGE_LOAD_STORE_border_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)
#define VIR_READ_IMAGE_LOAD_STORE_nearest_adv(normCoord, coordType, ch_data, dim, ch_order)    \
    VIR_READ_IMAGE_LOAD_STORE_wrap_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)   \
    VIR_READ_IMAGE_LOAD_STORE_mirror_nearest(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)

/* wrap,mirror: normcoord only */
/* linear none */
#define none_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, T0, y, z, T1, base_type, rgba_type, load_type) \
"    float4 "T0" = " BORDER_CLR##ch_order ";" NEWLINE \
"    float4 "T1" = " BORDER_CLR##ch_order ";" NEWLINE \
"    " BASE_uchar(dim, base_type, y, z) \
"    " LoadVal(dim, ch_data, ch_order, "icoordQ", load_type) \
"    " ch_data##ch_order##_CvtRGBA(rgba_type) \
"    "T0" = rgba;" NEWLINE \
"    " LoadVal(dim, ch_data, ch_order, "icoordR", "") \
"    " ch_data##ch_order##_CvtRGBA("") \
"    "T1" = rgba;" NEWLINE \
"" NEWLINE
#define none_LINEAR_LOAD_JUDGE_CONV_1(dim, ch_data, ch_order) \
    none_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T0", "icoordQ", "", "T1", "uchar *", "float4 ", Load_R_Type(ch_data, ch_order)" ")
#define none_LINEAR_LOAD_JUDGE_CONV_2(dim, ch_data, ch_order) \
    none_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T00", "icoordQ", "icoordQ", "T10", "uchar *", "float4 ", Load_R_Type(ch_data, ch_order)" ") \
    none_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T01", "icoordR", "icoordQ", "T11", "", "", "")
#define none_LINEAR_LOAD_JUDGE_CONV_3(dim, ch_data, ch_order) \
    none_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T000", "icoordQ", "icoordQ", "T100", "uchar *", "float4 ", Load_R_Type(ch_data, ch_order)" ") \
    none_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T010", "icoordR", "icoordQ", "T110", "", "", "") \
    none_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T001", "icoordQ", "icoordR", "T101", "", "", "") \
    none_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T011", "icoordR", "icoordR", "T111", "", "", "")
#define __none_LINEAR_LOAD_JUDGE_CONV(num, dim, ch_data, ch_order)  none_LINEAR_LOAD_JUDGE_CONV_##num(dim, ch_data, ch_order)
#define _none_LINEAR_LOAD_JUDGE_CONV(num, dim, ch_data, ch_order)   __none_LINEAR_LOAD_JUDGE_CONV(num, dim, ch_data, ch_order)
#define none_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order)         _none_LINEAR_LOAD_JUDGE_CONV(DIM_##dim##_NUM, dim, ch_data, ch_order)

#define VIR_READ_IMAGE_LOAD_STORE_none_linear(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_linear_" ToStr(normCoord) "Coord_none_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" = fcoord"COORD_ADDRESS(dim)" - .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    fract -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = icoordQ"COORD_ADDRESS(dim)" + 1;" NEWLINE \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    none_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order) \
    CALC_LINEAR(dim) \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* linear clamp */
#define clamp_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, T0, y, z, T1, base_type, rgba_type, load_type) \
"    float4 "T0" = " BORDER_CLR##ch_order ";" NEWLINE \
"    float4 "T1" = " BORDER_CLR##ch_order ";" NEWLINE \
"    " BASE_uchar(dim, base_type, y, z) \
"    " LoadVal(dim, ch_data, ch_order, "icoordQ", load_type) \
"    " ch_data##ch_order##_CvtRGBA(rgba_type) \
"    "T0" = rgba;" NEWLINE \
"    " LoadVal(dim, ch_data, ch_order, "icoordR", "") \
"    " ch_data##ch_order##_CvtRGBA("") \
"    "T1" = rgba;" NEWLINE \
"" NEWLINE
#define clamp_LINEAR_LOAD_JUDGE_CONV_1(dim, ch_data, ch_order) \
    clamp_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T0", "icoordQ", "", "T1", "uchar *", "float4 ", Load_R_Type(ch_data, ch_order)" ")
#define clamp_LINEAR_LOAD_JUDGE_CONV_2(dim, ch_data, ch_order) \
    clamp_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T00", "icoordQ", "icoordQ", "T10", "uchar *", "float4 ", Load_R_Type(ch_data, ch_order)" ") \
    clamp_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T01", "icoordR", "icoordQ", "T11", "", "", "")
#define clamp_LINEAR_LOAD_JUDGE_CONV_3(dim, ch_data, ch_order) \
    clamp_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T000", "icoordQ", "icoordQ", "T100", "uchar *", "float4 ", Load_R_Type(ch_data, ch_order)" ") \
    clamp_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T010", "icoordR", "icoordQ", "T110", "", "", "") \
    clamp_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T001", "icoordQ", "icoordR", "T101", "", "", "") \
    clamp_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T011", "icoordR", "icoordR", "T111", "", "", "")
#define __clamp_LINEAR_LOAD_JUDGE_CONV(num, dim, ch_data, ch_order)  clamp_LINEAR_LOAD_JUDGE_CONV_##num(dim, ch_data, ch_order)
#define _clamp_LINEAR_LOAD_JUDGE_CONV(num, dim, ch_data, ch_order)   __clamp_LINEAR_LOAD_JUDGE_CONV(num, dim, ch_data, ch_order)
#define clamp_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order)         _clamp_LINEAR_LOAD_JUDGE_CONV(DIM_##dim##_NUM, dim, ch_data, ch_order)

#define VIR_READ_IMAGE_LOAD_STORE_clamp_linear(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_linear_" ToStr(normCoord) "Coord_clamp_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" = fcoord"COORD_ADDRESS(dim)" - .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    fract -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = icoordQ"COORD_ADDRESS(dim)" + 1;" NEWLINE \
    "    "DIM_NUM_INT(dim)" imgsizeM1 = imageSize"COORD_ADDRESS(dim)" - 1;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = clamp(icoordQ"COORD_ADDRESS(dim)", 0, imgsizeM1);" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = clamp(icoordR"COORD_ADDRESS(dim)", 0, imgsizeM1);" NEWLINE \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    clamp_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order) \
    CALC_LINEAR(dim) \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* linear border */
#define border_LINEAR_JUDGE_1(dim, y, z) "1"
#define border_LINEAR_JUDGE_2(dim, y, z) y".y >= 0 && "y".y < imageSize.y"
#define border_LINEAR_JUDGE_3(dim, y, z) z".z >= 0 && "z".z < imageSize.z && "y".y >= 0 && "y".y < imageSize.y"
#define __border_LINEAR_JUDGE(num, dim, y, z)  border_LINEAR_JUDGE_##num(dim, y, z)
#define _border_LINEAR_JUDGE(num, dim, y, z)   __border_LINEAR_JUDGE(num, dim, y, z)
#define border_LINEAR_JUDGE(dim, y, z)         _border_LINEAR_JUDGE(DIM_##dim##_NUM, dim, y, z)

#define border_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, T0, y, z, T1) \
"    float4 "T0" = " BORDER_CLR##ch_order ";" NEWLINE \
"    float4 "T1" = " BORDER_CLR##ch_order ";" NEWLINE \
"    if ("border_LINEAR_JUDGE(dim, y, z)")" NEWLINE \
"    {" NEWLINE \
"        " BASE_uchar(dim, "uchar *", y, z) \
"        if (icoordQ"COORD_X(dim)" >= 0 && icoordQ"COORD_X(dim)" < imageSize"COORD_X(dim)")" NEWLINE \
"        {" NEWLINE \
"           " LoadVal(dim, ch_data, ch_order, "icoordQ", Load_R_Type(ch_data, ch_order)" ") \
"           " ch_data##ch_order##_CvtRGBA("") \
"           "T0" = rgba;" NEWLINE \
"        }" NEWLINE \
"        if (icoordR"COORD_X(dim)" >= 0 && icoordR"COORD_X(dim)" < imageSize"COORD_X(dim)")" NEWLINE \
"        {" NEWLINE \
"            " LoadVal(dim, ch_data, ch_order, "icoordR", Load_R_Type(ch_data, ch_order)" ") \
"            " ch_data##ch_order##_CvtRGBA("") \
"            "T1" = rgba;" NEWLINE \
"        }" NEWLINE \
"    }" NEWLINE \
"" NEWLINE
#define border_LINEAR_LOAD_JUDGE_CONV_1(dim, ch_data, ch_order) \
    border_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T0", "icoordQ", "", "T1")
#define border_LINEAR_LOAD_JUDGE_CONV_2(dim, ch_data, ch_order) \
    border_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T00", "icoordQ", "icoordQ", "T10") \
    border_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T01", "icoordR", "icoordQ", "T11")
#define border_LINEAR_LOAD_JUDGE_CONV_3(dim, ch_data, ch_order) \
    border_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T000", "icoordQ", "icoordQ", "T100") \
    border_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T010", "icoordR", "icoordQ", "T110") \
    border_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T001", "icoordQ", "icoordR", "T101") \
    border_LINEAR_LOAD_JUDGE_CONV_TEMPLETE(dim, ch_data, ch_order, "T011", "icoordR", "icoordR", "T111")
#define __border_LINEAR_LOAD_JUDGE_CONV(num, dim, ch_data, ch_order)  border_LINEAR_LOAD_JUDGE_CONV_##num(dim, ch_data, ch_order)
#define _border_LINEAR_LOAD_JUDGE_CONV(num, dim, ch_data, ch_order)   __border_LINEAR_LOAD_JUDGE_CONV(num, dim, ch_data, ch_order)
#define border_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order)         _border_LINEAR_LOAD_JUDGE_CONV(DIM_##dim##_NUM, dim, ch_data, ch_order)

#define VIR_READ_IMAGE_LOAD_STORE_border_linear(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_linear_" ToStr(normCoord) "Coord_border_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" = fcoord"COORD_ADDRESS(dim)" - .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    fract -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = icoordQ"COORD_ADDRESS(dim)" + 1;" NEWLINE \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    "    float4 rgba;" NEWLINE \
    border_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order) \
    CALC_LINEAR(dim) \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* linear wrap */
#define wrap_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order) clamp_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order)
#define _wrap_LINEAR_COORD_1(dim) \
    "    if (icoordQ"COORD_X(dim)" < 0) icoordQ"COORD_X(dim)" += imageSize"COORD_X(dim)"; " NEWLINE \
    "    if (icoordR"COORD_X(dim)" >= imageSize"COORD_X(dim)") icoordR"COORD_X(dim)" -= imageSize"COORD_X(dim)"; " NEWLINE
#define _wrap_LINEAR_COORD_2(dim) \
    "    if (icoordQ.x < 0) icoordQ.x += imageSize.x; " NEWLINE \
    "    if (icoordQ.y < 0) icoordQ.y += imageSize.y; " NEWLINE \
    "    if (icoordR.x >= imageSize.x) icoordR.x -= imageSize.x; " NEWLINE \
    "    if (icoordR.y >= imageSize.y) icoordR.y -= imageSize.y; " NEWLINE
#define _wrap_LINEAR_COORD_3(dim) \
    "    if (icoordQ.x < 0) icoordQ.x += imageSize.x; " NEWLINE \
    "    if (icoordQ.y < 0) icoordQ.y += imageSize.y; " NEWLINE \
    "    if (icoordQ.z < 0) icoordQ.z += imageSize.z; " NEWLINE \
    "    if (icoordR.x >= imageSize.x) icoordR.x -= imageSize.x; " NEWLINE \
    "    if (icoordR.y >= imageSize.y) icoordR.y -= imageSize.y; " NEWLINE \
    "    if (icoordR.z >= imageSize.z) icoordR.z -= imageSize.z; " NEWLINE
#define __wrap_LINEAR_COORD(num, dim)   _wrap_LINEAR_COORD_##num(dim)
#define _wrap_LINEAR_COORD(num, dim)    __wrap_LINEAR_COORD(num, dim)
#define wrap_LINEAR_COORD(dim)          _wrap_LINEAR_COORD(DIM_##dim##_NUM, dim)

#define VIR_READ_IMAGE_LOAD_STORE_wrap_linear(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_linear_" ToStr(normCoord) "Coord_wrap_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    "COORD_NUM_FLOAT(dim)" fcoord;" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = coord" COORD_ADDRESS2(dim)" - floor(coord" COORD_ADDRESS2(dim)");" NEWLINE \
    SELECT_COORD(dim) \
    "    " normCoord##_COORD(dim, "", "fcoord", DIM_ADDRESS(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" -= .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    fract -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = icoordQ"COORD_ADDRESS(dim)" + 1;" NEWLINE \
    wrap_LINEAR_COORD(dim) \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    wrap_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order) \
    CALC_LINEAR(dim) \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* linear mirror */
#define mirror_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order) clamp_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order)
#define VIR_READ_IMAGE_LOAD_STORE_mirror_linear(retType, normCoord, coordType, ch_data, dim, ch_order) \
ToStr(retType) "4 _read_image_with_load_linear_" ToStr(normCoord) "Coord_mirror_" ToStr(coordType) "coord_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    "COORD_NUM_FLOAT(dim)" fcoord;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fcoord0 = 2. * floor(.5f * coord" COORD_ADDRESS2(dim)" + .5);" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = viv_fabs(coord"COORD_ADDRESS2(dim)" - fcoord0); " NEWLINE \
    SELECT_COORD(dim) \
    "    " normCoord##_COORD(dim, "", "fcoord", DIM_ADDRESS(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" -= .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)"); "NEWLINE \
    "    fract  -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)"); " NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = icoordQ"COORD_ADDRESS(dim)" + 1; " NEWLINE \
    "    "DIM_NUM_INT(dim)" imageSizeM1 = imageSize"COORD_ADDRESS(dim)" - 1; " NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = clamp(icoordQ"COORD_ADDRESS(dim)", 0, imageSizeM1);" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = clamp(icoordR"COORD_ADDRESS(dim)", 0, imageSizeM1);" NEWLINE \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    mirror_LINEAR_LOAD_JUDGE_CONV(dim, ch_data, ch_order) \
    CALC_LINEAR(dim) \
    "    return convert_"ToStr(retType)"4(rgba);" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

#define VIR_READ_IMAGE_LOAD_STORE_linear_all(normCoord, coordType, ch_data, dim, ch_order)    \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_none_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order))   \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_clamp_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order))  \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_border_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order)) \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_wrap_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order))   \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_mirror_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order))
#define VIR_READ_IMAGE_LOAD_STORE_linear_ncb(normCoord, coordType, ch_data, dim, ch_order)    \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_none_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order))   \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_clamp_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order))  \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_border_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order))
#define VIR_READ_IMAGE_LOAD_STORE_linear_adv(normCoord, coordType, ch_data, dim, ch_order)    \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_wrap_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order))   \
    LINEAR_SUPPORT(ch_data, VIR_READ_IMAGE_LOAD_STORE_mirror_linear(RETURN_TYPE(ch_data), normCoord, coordType, ch_data, dim, ch_order))

/* write image */
#define _WRITE_snorm8(x)                x
#define _WRITE_snorm16(x)               x
#define _WRITE_unorm8(x)                x
#define _WRITE_unorm16(x)               x
#define _WRITE_int8(x)                  x
#define _WRITE_int16(x)                 x
#define _WRITE_int32(x)                 x
#define _WRITE_uint8(x)                 x
#define _WRITE_uint16(x)                x
#define _WRITE_uint32(x)                x
#define _WRITE_float(x)                 x
#define _WRITE_half(x)                  x
#define _WRITE_unorm555(x)              ""
#define _WRITE_unorm565(x)              ""
#define _WRITE_unorm101010(x)           ""
#define _WRITE_1d(x)                    x
#define _WRITE_2d(x)                    x
#define _WRITE_3d(x)                    ""
#define _WRITE_1dbuffer(x)              ""
#define _WRITE_1darray(x)               x
#define _WRITE_2darray(x)               x
#define WRITE_SUPPORT(dim, ch_data, x)  _WRITE_##dim(_WRITE_##ch_data(x))

#define VIR_WRITE_IMAGE_LOAD_STORE(retType, ch_data, dim, ch_order) \
"void _write_image_with_store_" ret##retType "_" ToStr(ch_data) "_" ToStr(dim) ToStr(ch_order) \
    NEWLINE "    (viv_generic_image_t image, "/*COORD_NUM_INT(dim)*/"int4 coord, " ToStr(retType) "4 rgba)" NEWLINE \
    "{" NEWLINE \
    "    " BASE_uchar(dim, "uchar *", "coord", "coord") \
    "    " ch_data##ch_order##_CvtVALUE(Load_R_Type(ch_data, ch_order)" ")\
    "    " StoreVal(dim, ch_data, ch_order, "coord") \
    "}" NEWLINE \
    "" SPLITLINE

/* group: nearest linear, 1d 2d 3d, 1dbuffer 1darray 2darray */
/* compiler limit: max string length: 65535         */
#define VIR_IMAGE_LOAD_STORE_group(ch_data, ch_order) \
{ NEWLINE \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, int, ch_data, 1d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, float, ch_data, 1d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_all(norm, float, ch_data, 1d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_ncb (unnorm, float, ch_data, 1d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_all (norm, float, ch_data, 1d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, int, ch_data, 1darray, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, float, ch_data, 1darray, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_all(norm, float, ch_data, 1darray, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_ncb (unnorm, float, ch_data, 1darray, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_all (norm, float, ch_data, 1darray, ch_order) \
WRITE_SUPPORT(1d, ch_data, VIR_WRITE_IMAGE_LOAD_STORE(RETURN_TYPE(ch_data), ch_data, 1d, ch_order)) \
WRITE_SUPPORT(1darray, ch_data, VIR_WRITE_IMAGE_LOAD_STORE(RETURN_TYPE(ch_data), ch_data, 1darray, ch_order)) \
, \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, int, ch_data, 2d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, float, ch_data, 2d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_all(norm, float, ch_data, 2d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_ncb (unnorm, float, ch_data, 2d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_all (norm, float, ch_data, 2d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, int, ch_data, 2darray, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, float, ch_data, 2darray, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_all(norm, float, ch_data, 2darray, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_ncb (unnorm, float, ch_data, 2darray, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_all (norm, float, ch_data, 2darray, ch_order) \
WRITE_SUPPORT(2d, ch_data, VIR_WRITE_IMAGE_LOAD_STORE(RETURN_TYPE(ch_data), ch_data, 2d, ch_order)) \
WRITE_SUPPORT(2darray, ch_data, VIR_WRITE_IMAGE_LOAD_STORE(RETURN_TYPE(ch_data), ch_data, 2darray, ch_order)) \
,\
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, int, ch_data, 3d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, float, ch_data, 3d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_all(norm, float, ch_data, 3d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_ncb (unnorm, float, ch_data, 3d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_all (norm, float, ch_data, 3d, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, int, ch_data, 1dbuffer, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_ncb(unnorm, float, ch_data, 1dbuffer, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_nearest_all(norm, float, ch_data, 1dbuffer, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_ncb (unnorm, float, ch_data, 1dbuffer, ch_order) \
VIR_READ_IMAGE_LOAD_STORE_linear_all (norm, float, ch_data, 1dbuffer, ch_order) \
WRITE_SUPPORT(3d, ch_data, VIR_WRITE_IMAGE_LOAD_STORE(RETURN_TYPE(ch_data), ch_data, 3d, ch_order)) \
WRITE_SUPPORT(1dbuffer, ch_data, VIR_WRITE_IMAGE_LOAD_STORE(RETURN_TYPE(ch_data), ch_data, 1dbuffer, ch_order)) \
};

/* spilt line ******/
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_RGBA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_RGBA[3];

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_BGRA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_BGRA[3];

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_R[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_R[3];

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_A[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_A[3] ;

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_INTENSITY[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_INTENSITY[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_INTENSITY[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_INTENSITY[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_INTENSITY[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_INTENSITY[3];

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_LUMINANCE[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_LUMINANCE[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_LUMINANCE[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_LUMINANCE[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_LUMINANCE[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_LUMINANCE[3];

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_RG[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_RG[3];

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_RA[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_RA[3];

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm555_RGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm565_RGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm101010_RGB[3];

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_float_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_half_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm8_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_unorm16_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm8_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_snorm16_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int8_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int16_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_int32_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint8_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint16_ARGB[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_LOAD_uint32_ARGB[3];

/* imgld ****************************************************************/
/***********************************/
#define IMGLD_3D_BORDER_PATCH     1
/***********************************/
#define IMGLD_SUFFIX_1d           ""
#define IMGLD_SUFFIX_1dbuffer     ""
#define IMGLD_SUFFIX_1darray      ""
#define IMGLD_SUFFIX_2d           ""
#define IMGLD_SUFFIX_2darray      "_3D"
#define IMGLD_SUFFIX_3d           "_3D"

#define _MAKE_IMGLD_COORD_1(x, y, z)            "(int2)("x", 0)"
#define _MAKE_IMGLD_COORD_2(x, y, z)            "(int2)("x", "y")"
#define _MAKE_IMGLD_COORD_3(x, y, z)            "(int3)("x", "y", "z")"
#define __MAKE_IMGLD_COORD(address, x, y, z)    _MAKE_IMGLD_COORD_##address(x, y, z)
#define _MAKE_IMGLD_COORD(address, x, y, z)     __MAKE_IMGLD_COORD(address, x, y, z)
#define MAKE_IMGLD_COORD(dim, x, y, z)          _MAKE_IMGLD_COORD(IMGLD_ADDRESS_##dim, x, y, z)

#define _VIR_IMAGE_IMGLD_1(type, dim, x, y, z)        "_viv_asm("type IMGLD_SUFFIX_##dim", rgba, image, (int2)("x", 0) );" NEWLINE
#define _VIR_IMAGE_IMGLD_2(type, dim, x, y, z)        "_viv_asm("type IMGLD_SUFFIX_##dim", rgba, image, (int2)("x", "y") );" NEWLINE
#define _VIR_IMAGE_IMGLD_3(type, dim, x, y, z)        "_viv_asm("type IMGLD_SUFFIX_##dim", rgba, image, (int3)("x", "y", _viv_get3DImageNewBaseAddr(image, (int3)("x", "y", "z")) ));" NEWLINE

#define __VIR_IMAGE_IMGLD(address, type, dim, x, y, z)  _VIR_IMAGE_IMGLD_##address(type, dim, x, y, z)
#define _VIR_IMAGE_IMGLD(address, type, dim, x, y, z)   __VIR_IMAGE_IMGLD(address, type, dim, x, y, z)
#define VIR_IMAGE_IMGLD(type, dim, x, y, z)             _VIR_IMAGE_IMGLD(COORD_##dim##_NUM, type, dim, x, y, z)

#define VIR_IMAGE_IMGLD_R(dim, x, y, z)             VIR_IMAGE_IMGLD("IMAGE_READ", dim, x, y, z)
#define VIR_IMAGE_IMGLD_W(dim, x, y, z)             VIR_IMAGE_IMGLD("IMAGE_WRITE", dim, x, y, z)

#if (IMGLD_3D_BORDER_PATCH == 1)
#define _VIR_IMAGE_IMGLD_1_border(type, dim, x, y, z)        _VIR_IMAGE_IMGLD_1(type, dim, x, y, z)
#define _VIR_IMAGE_IMGLD_2_border(type, dim, x, y, z)        _VIR_IMAGE_IMGLD_2(type, dim, x, y, z)
#define _VIR_IMAGE_IMGLD_3_border(type, dim, x, y, z)        "_viv_asm("type IMGLD_SUFFIX_##dim", rgba, image, (int3)("x", ("z" < 0 || "z" > imageSize.z - 1) ? -1 : "y", _viv_get3DImageNewBaseAddr(image, (int3)("x", "y", "z")) ));" NEWLINE

#define __VIR_IMAGE_IMGLD_border(address, type, dim, x, y, z)   _VIR_IMAGE_IMGLD_##address##_border(type, dim, x, y, z)
#define _VIR_IMAGE_IMGLD_border(address, type, dim, x, y, z)    __VIR_IMAGE_IMGLD_border(address, type, dim, x, y, z)
#define VIR_IMAGE_IMGLD_border(type, dim, x, y, z)              _VIR_IMAGE_IMGLD_border(COORD_##dim##_NUM, type, dim, x, y, z)
#define VIR_IMAGE_IMGLD_R_border(dim, x, y, z)                  VIR_IMAGE_IMGLD_border("IMAGE_READ", dim, x, y, z)
#else
#define VIR_IMAGE_IMGLD_R_border(dim, x, y, z)                  VIR_IMAGE_IMGLD_R(dim, x, y, z)
#endif

#define IMGLD_LINEAR_SUPPORT_int(x)         ""
#define IMGLD_LINEAR_SUPPORT_uint(x)        ""
#define IMGLD_LINEAR_SUPPORT_float(x)       x

/* nearest none */
#define VIR_READ_IMAGE_IMGLD_none_nearest(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_nearest_" ToStr(normCoord) "Coord_none_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"_rte(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    "    "VIR_IMAGE_IMGLD_R(dim, "icoord"COORD_X(dim), "icoord.y", "icoord.z") \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* nearest clamp */
#define VIR_READ_IMAGE_IMGLD_clamp_nearest(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_nearest_" ToStr(normCoord) "Coord_clamp_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    fcoord = clamp(fcoord, ("COORD_NUM_FLOAT(dim)")(0.0), convert_"COORD_NUM_FLOAT(dim)"(imageSize) - 1.f);" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    "    "VIR_IMAGE_IMGLD_R(dim, "icoord"COORD_X(dim), "icoord.y", "icoord.z") \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* nearest border */
#define _VIR_IMGLD_border_nearest_1()   ""
#define _VIR_IMGLD_border_nearest_2()   ""
#define _VIR_IMGLD_border_nearest_3()   \
    "    if (!" BORDER_JUDGE_OK(3d) ") {" NEWLINE \
    "         icoord.x = -1;" NEWLINE \
    "    }" NEWLINE
#define __VIR_IMGLD_border_nearest(x)   _VIR_IMGLD_border_nearest_##x()
#define _VIR_IMGLD_border_nearest(x)    __VIR_IMGLD_border_nearest(x)
#define VIR_IMGLD_border_nearest(dim)   _VIR_IMGLD_border_nearest(COORD_##dim##_NUM)
#define VIR_READ_IMAGE_IMGLD_border_nearest(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_nearest_" ToStr(normCoord) "Coord_border_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"_rtn(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    /*"    icoord"COORD_ADDRESS(dim)" = clamp(icoord"COORD_ADDRESS(dim)", -1, imageSize"COORD_ADDRESS(dim)");" NEWLINE*/ \
    /*VIR_IMGLD_border_nearest(dim)*/ \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    "    "VIR_IMAGE_IMGLD_R_border(dim, "icoord"COORD_X(dim), "icoord.y", "icoord.z") \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* nearest wrap */
#define VIR_READ_IMAGE_IMGLD_wrap_nearest(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_nearest_" ToStr(normCoord) "Coord_wrap_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    "COORD_NUM_FLOAT(dim)" fcoord;" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = coord" COORD_ADDRESS2(dim)" - floor(coord" COORD_ADDRESS2(dim)");" NEWLINE \
    SELECT_COORD(dim) \
    "    " normCoord##_COORD(dim, "", "fcoord", DIM_ADDRESS(dim)) \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"_rtn(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = viv_min(icoord"COORD_ADDRESS(dim)", imageSize"COORD_ADDRESS(dim)" - 1);" NEWLINE \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    "    "VIR_IMAGE_IMGLD_R(dim, "icoord"COORD_X(dim), "icoord.y", "icoord.z") \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* nearest mirror */
#define VIR_READ_IMAGE_IMGLD_mirror_nearest(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_nearest_" ToStr(normCoord) "Coord_mirror_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    "DIM_NUM_FLOAT(dim)" fcoord0 = 2.f * floor(.5f * coord" COORD_ADDRESS2(dim)" + .5f);" NEWLINE \
    "    "COORD_NUM_FLOAT(dim)" fcoord;" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = viv_fabs(coord" COORD_ADDRESS2(dim)" - fcoord0);" NEWLINE \
    SELECT_COORD(dim) \
    "    " normCoord##_COORD(dim, "", "fcoord", DIM_ADDRESS(dim)) \
    "    "COORD_NUM_INT(dim)" icoord;" NEWLINE \
    "    icoord "COORD_ADDRESS(dim)"= convert_"DIM_NUM_INT(dim)"_rtn(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoord"COORD_ADDRESS(dim)" = viv_min(icoord"COORD_ADDRESS(dim)", imageSize"COORD_ADDRESS(dim)" - 1);" NEWLINE \
    SELECT_IMAGE(dim, "icoord", "fcoord") \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    "    "VIR_IMAGE_IMGLD_R(dim, "icoord"COORD_X(dim), "icoord.y", "icoord.z") \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

#define VIR_READ_IMAGE_IMGLD_nearest_all(retType, normCoord, coordType, dim)    \
    VIR_READ_IMAGE_IMGLD_none_nearest(retType, normCoord, coordType, dim)   \
    VIR_READ_IMAGE_IMGLD_clamp_nearest(retType, normCoord, coordType, dim)  \
    VIR_READ_IMAGE_IMGLD_border_nearest(retType, normCoord, coordType, dim) \
    VIR_READ_IMAGE_IMGLD_wrap_nearest(retType, normCoord, coordType, dim)   \
    VIR_READ_IMAGE_IMGLD_mirror_nearest(retType, normCoord, coordType, dim)
#define VIR_READ_IMAGE_IMGLD_nearest_ncb(retType, normCoord, coordType, dim)    \
    VIR_READ_IMAGE_IMGLD_none_nearest(retType, normCoord, coordType, dim)   \
    VIR_READ_IMAGE_IMGLD_clamp_nearest(retType, normCoord, coordType, dim)  \
    VIR_READ_IMAGE_IMGLD_border_nearest(retType, normCoord, coordType, dim)
#define VIR_READ_IMAGE_IMGLD_nearest_adv(retType, normCoord, coordType, dim)    \
    VIR_READ_IMAGE_IMGLD_wrap_nearest(retType, normCoord, coordType, dim)   \
    VIR_READ_IMAGE_IMGLD_mirror_nearest(retType, normCoord, coordType, dim)

/* wrap,mirror: normcoord only */
/* linear none */
#define none_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, T0, y, z, T1) \
"    float4 "T0" = (float4)(.0f);" NEWLINE \
"    float4 "T1" = (float4)(.0f);" NEWLINE \
"    "VIR_IMAGE_IMGLD_R(dim, "icoordQ"COORD_X(dim), y".y", z".z") \
"    "T0" = rgba;" NEWLINE \
"    "VIR_IMAGE_IMGLD_R(dim, "icoordR"COORD_X(dim), y".y", z".z") \
"    "T1" = rgba;" NEWLINE \
"" NEWLINE
#define none_LINEAR_IMGLD_JUDGE_CONV_1(dim) \
    none_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T0", "icoordQ", "", "T1")
#define none_LINEAR_IMGLD_JUDGE_CONV_2(dim) \
    none_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T00", "icoordQ", "icoordQ", "T10") \
    none_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T01", "icoordR", "icoordQ", "T11")
#define none_LINEAR_IMGLD_JUDGE_CONV_3(dim) \
    none_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T000", "icoordQ", "icoordQ", "T100") \
    none_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T010", "icoordR", "icoordQ", "T110") \
    none_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T001", "icoordQ", "icoordR", "T101") \
    none_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T011", "icoordR", "icoordR", "T111")
#define __none_LINEAR_IMGLD_JUDGE_CONV(num, dim)  none_LINEAR_IMGLD_JUDGE_CONV_##num(dim)
#define _none_LINEAR_IMGLD_JUDGE_CONV(num, dim)   __none_LINEAR_IMGLD_JUDGE_CONV(num, dim)
#define none_LINEAR_IMGLD_JUDGE_CONV(dim)         _none_LINEAR_IMGLD_JUDGE_CONV(DIM_##dim##_NUM, dim)

#define VIR_READ_IMAGE_IMGLD_none_linear(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_linear_" ToStr(normCoord) "Coord_none_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" = fcoord"COORD_ADDRESS(dim)" - .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    fract -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = icoordQ"COORD_ADDRESS(dim)" + 1;" NEWLINE \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    none_LINEAR_IMGLD_JUDGE_CONV(dim) \
    CALC_LINEAR(dim) \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* linear clamp */
#define clamp_LINEAR_IMGLD_JUDGE_CONV(dim)         none_LINEAR_IMGLD_JUDGE_CONV(dim)
#define VIR_READ_IMAGE_IMGLD_clamp_linear(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_linear_" ToStr(normCoord) "Coord_clamp_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" = fcoord"COORD_ADDRESS(dim)" - .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    fract -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = icoordQ"COORD_ADDRESS(dim)" + 1;" NEWLINE \
    "    "DIM_NUM_INT(dim)" imgsizeM1 = imageSize"COORD_ADDRESS(dim)" - 1;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = clamp(icoordQ"COORD_ADDRESS(dim)", 0, imgsizeM1);" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = clamp(icoordR"COORD_ADDRESS(dim)", 0, imgsizeM1);" NEWLINE \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    clamp_LINEAR_IMGLD_JUDGE_CONV(dim) \
    CALC_LINEAR(dim) \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* linear border */
#if (IMGLD_3D_BORDER_PATCH == 1)
#define border_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, T0, y, z, T1) \
"    float4 "T0" = (float4)(.0f);" NEWLINE \
"    float4 "T1" = (float4)(.0f);" NEWLINE \
"    "VIR_IMAGE_IMGLD_R_border(dim, "icoordQ"COORD_X(dim), y".y", z".z") \
"    "T0" = rgba;" NEWLINE \
"    "VIR_IMAGE_IMGLD_R_border(dim, "icoordR"COORD_X(dim), y".y", z".z") \
"    "T1" = rgba;" NEWLINE \
"" NEWLINE
#define border_LINEAR_IMGLD_JUDGE_CONV_1(dim) \
    border_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T0", "icoordQ", "", "T1")
#define border_LINEAR_IMGLD_JUDGE_CONV_2(dim) \
    border_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T00", "icoordQ", "icoordQ", "T10") \
    border_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T01", "icoordR", "icoordQ", "T11")
#define border_LINEAR_IMGLD_JUDGE_CONV_3(dim) \
    border_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T000", "icoordQ", "icoordQ", "T100") \
    border_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T010", "icoordR", "icoordQ", "T110") \
    border_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T001", "icoordQ", "icoordR", "T101") \
    border_LINEAR_IMGLD_JUDGE_CONV_TEMPLETE(dim, "T011", "icoordR", "icoordR", "T111")
#define __border_LINEAR_IMGLD_JUDGE_CONV(num, dim)  border_LINEAR_IMGLD_JUDGE_CONV_##num(dim)
#define _border_LINEAR_IMGLD_JUDGE_CONV(num, dim)   __border_LINEAR_IMGLD_JUDGE_CONV(num, dim)
#define border_LINEAR_IMGLD_JUDGE_CONV(dim)         _border_LINEAR_IMGLD_JUDGE_CONV(DIM_##dim##_NUM, dim)
#else
#define border_LINEAR_IMGLD_JUDGE_CONV(dim)         clamp_LINEAR_IMGLD_JUDGE_CONV(dim)
#endif
#define VIR_READ_IMAGE_IMGLD_border_linear(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_linear_" ToStr(normCoord) "Coord_border_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    " normCoord##_COORD(dim, COORD_NUM_FLOAT(dim)" ", "coord", DIM_ADDRESS2(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" = fcoord"COORD_ADDRESS(dim)" - .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    fract -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = clamp(convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)"), ("DIM_NUM_INT(dim)")(-1), imageSize"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = clamp(convert_"DIM_NUM_INT(dim)"(1+fcoord"COORD_ADDRESS(dim)"), ("DIM_NUM_INT(dim)")(-1), imageSize"COORD_ADDRESS(dim)");" NEWLINE \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    border_LINEAR_IMGLD_JUDGE_CONV(dim) \
    CALC_LINEAR(dim) \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* linear wrap */
#define wrap_LINEAR_IMGLD_JUDGE_CONV(dim) clamp_LINEAR_IMGLD_JUDGE_CONV(dim)
#define _wrap_LINEAR_IMGLD_COORD_1(dim) \
    "    if (icoordQ"COORD_X(dim)" < 0) icoordQ"COORD_X(dim)" += imageSize"COORD_X(dim)"; " NEWLINE \
    "    if (icoordR"COORD_X(dim)" >= imageSize"COORD_X(dim)") icoordR"COORD_X(dim)" -= imageSize"COORD_X(dim)"; " NEWLINE
#define _wrap_LINEAR_IMGLD_COORD_2(dim) \
    "    if (icoordQ.x < 0) icoordQ.x += imageSize.x; " NEWLINE \
    "    if (icoordQ.y < 0) icoordQ.y += imageSize.y; " NEWLINE \
    "    if (icoordR.x >= imageSize.x) icoordR.x -= imageSize.x; " NEWLINE \
    "    if (icoordR.y >= imageSize.y) icoordR.y -= imageSize.y; " NEWLINE
#define _wrap_LINEAR_IMGLD_COORD_3(dim) \
    "    if (icoordQ.x < 0) icoordQ.x += imageSize.x; " NEWLINE \
    "    if (icoordQ.y < 0) icoordQ.y += imageSize.y; " NEWLINE \
    "    if (icoordQ.z < 0) icoordQ.z += imageSize.z; " NEWLINE \
    "    if (icoordR.x >= imageSize.x) icoordR.x -= imageSize.x; " NEWLINE \
    "    if (icoordR.y >= imageSize.y) icoordR.y -= imageSize.y; " NEWLINE \
    "    if (icoordR.z >= imageSize.z) icoordR.z -= imageSize.z; " NEWLINE
#define __wrap_LINEAR_IMGLD_COORD(num, dim)   _wrap_LINEAR_IMGLD_COORD_##num(dim)
#define _wrap_LINEAR_IMGLD_COORD(num, dim)    __wrap_LINEAR_IMGLD_COORD(num, dim)
#define wrap_LINEAR_IMGLD_COORD(dim)          _wrap_LINEAR_IMGLD_COORD(DIM_##dim##_NUM, dim)

#define VIR_READ_IMAGE_IMGLD_wrap_linear(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_linear_" ToStr(normCoord) "Coord_wrap_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    "COORD_NUM_FLOAT(dim)" fcoord;" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = coord" COORD_ADDRESS2(dim)" - floor(coord" COORD_ADDRESS2(dim)");" NEWLINE \
    SELECT_COORD(dim) \
    "    " normCoord##_COORD(dim, "", "fcoord", DIM_ADDRESS(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" -= .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    fract -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)");" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = icoordQ"COORD_ADDRESS(dim)" + 1;" NEWLINE \
    wrap_LINEAR_COORD(dim) \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    wrap_LINEAR_IMGLD_JUDGE_CONV(dim) \
    CALC_LINEAR(dim) \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

/* linear mirror */
#define mirror_LINEAR_IMGLD_JUDGE_CONV(dim) clamp_LINEAR_IMGLD_JUDGE_CONV(dim)
#define VIR_READ_IMAGE_IMGLD_mirror_linear(retType, normCoord, coordType, dim) \
ToStr(retType) "4 _read_image_width_imgread_linear_" ToStr(normCoord) "Coord_mirror_" ToStr(coordType) "coord_" ToStr(retType)  "_" ToStr(dim)  \
    NEWLINE "    (viv_generic_image_t image, " ToStr(coordType) "4 coord" SAMPLER_PARAM ")" NEWLINE \
    "{" NEWLINE \
    "    "ImageSize(dim) \
    "    "COORD_NUM_FLOAT(dim)" fcoord;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fcoord0 = 2. * floor(.5f * coord" COORD_ADDRESS2(dim)" + .5);" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = viv_fabs(coord"COORD_ADDRESS2(dim)" - fcoord0); " NEWLINE \
    SELECT_COORD(dim) \
    "    " normCoord##_COORD(dim, "", "fcoord", DIM_ADDRESS(dim)) \
    "    fcoord"COORD_ADDRESS(dim)" -= .5f;" NEWLINE \
    "    "DIM_NUM_FLOAT(dim)" fract = fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    fcoord"COORD_ADDRESS(dim)" = floor(fcoord"COORD_ADDRESS(dim)"); "NEWLINE \
    "    fract  -= fcoord"COORD_ADDRESS(dim)";" NEWLINE \
    "    "COORD_NUM_INT(dim)" icoordQ, icoordR;" NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = convert_"DIM_NUM_INT(dim)"(fcoord"COORD_ADDRESS(dim)"); " NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = icoordQ"COORD_ADDRESS(dim)" + 1; " NEWLINE \
    "    "DIM_NUM_INT(dim)" imageSizeM1 = imageSize"COORD_ADDRESS(dim)" - 1; " NEWLINE \
    "    icoordQ"COORD_ADDRESS(dim)" = clamp(icoordQ"COORD_ADDRESS(dim)", 0, imageSizeM1);" NEWLINE \
    "    icoordR"COORD_ADDRESS(dim)" = clamp(icoordR"COORD_ADDRESS(dim)", 0, imageSizeM1);" NEWLINE \
    SELECT_IMAGE(dim, "icoordQ", "fcoord") \
    "" NEWLINE \
    "    "ToStr(retType)"4 rgba;" NEWLINE \
    mirror_LINEAR_IMGLD_JUDGE_CONV(dim) \
    CALC_LINEAR(dim) \
    "    return rgba;" NEWLINE \
    "}" NEWLINE \
    "" SPLITLINE

#define VIR_READ_IMAGE_IMGLD_linear_all(retType, normCoord, coordType, dim)    \
    VIR_READ_IMAGE_IMGLD_none_linear(retType, normCoord, coordType, dim)   \
    VIR_READ_IMAGE_IMGLD_clamp_linear(retType, normCoord, coordType, dim)  \
    VIR_READ_IMAGE_IMGLD_border_linear(retType, normCoord, coordType, dim) \
    VIR_READ_IMAGE_IMGLD_wrap_linear(retType, normCoord, coordType, dim)   \
    VIR_READ_IMAGE_IMGLD_mirror_linear(retType, normCoord, coordType, dim)
#define VIR_READ_IMAGE_IMGLD_linear_ncb(retType, normCoord, coordType, dim)    \
    VIR_READ_IMAGE_IMGLD_none_linear(retType, normCoord, coordType, dim)   \
    VIR_READ_IMAGE_IMGLD_clamp_linear(retType, normCoord, coordType, dim)  \
    VIR_READ_IMAGE_IMGLD_border_linear(retType, normCoord, coordType, dim)
#define VIR_READ_IMAGE_IMGLD_linear_adv(retType, normCoord, coordType, dim)    \
    VIR_READ_IMAGE_IMGLD_wrap_linear(retType, normCoord, coordType, dim)   \
    VIR_READ_IMAGE_IMGLD_mirror_linear(retType, normCoord, coordType, dim)

/* write image */
#define VIR_WRITE_IMAGE_IMGLD(dataType, dim) \
"void _write_image_with_imgwrite_" ToStr(dataType) "_"ToStr(dim)    \
    NEWLINE "    (viv_generic_image_t image, "/*COORD_NUM_INT(dim)*/"int4 coord, "ToStr(dataType)"4 rgba)" NEWLINE \
    "{" NEWLINE \
    "    "VIR_IMAGE_IMGLD_W(dim, "coord.x"/*COORD_X(dim)*/, "coord.y", "coord.z") \
    "}" NEWLINE \
    "" SPLITLINE

#define VIR_IMAGE_IMGLD_group(type) \
{ NEWLINE \
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, int, 1d) \
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, float, 1d) \
VIR_READ_IMAGE_IMGLD_nearest_all(type, norm, float, 1d) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_ncb (type, unnorm, float, 1d)) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_all (type, norm, float, 1d)) \
VIR_WRITE_IMAGE_IMGLD(type, 1d) \
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, int, 1darray) \
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, float, 1darray) \
VIR_READ_IMAGE_IMGLD_nearest_all(type, norm, float, 1darray) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_ncb (type, unnorm, float, 1darray)) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_all (type, norm, float, 1darray)) \
VIR_WRITE_IMAGE_IMGLD(type, 1darray) \
, \
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, int, 2d) \
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, float, 2d) \
VIR_READ_IMAGE_IMGLD_nearest_all(type, norm, float, 2d) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_ncb (type, unnorm, float, 2d)) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_all (type, norm, float, 2d)) \
VIR_WRITE_IMAGE_IMGLD(type, 2d) \
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, int, 2darray) \
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, float, 2darray) \
VIR_READ_IMAGE_IMGLD_nearest_all(type, norm, float, 2darray) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_ncb (type, unnorm, float, 2darray)) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_all (type, norm, float, 2darray)) \
VIR_WRITE_IMAGE_IMGLD(type, 2darray) \
,\
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, int, 3d) \
VIR_READ_IMAGE_IMGLD_nearest_ncb(type, unnorm, float, 3d) \
VIR_READ_IMAGE_IMGLD_nearest_all(type, norm, float, 3d) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_ncb (type, unnorm, float, 3d)) \
LINEAR_SUPPORT(type, VIR_READ_IMAGE_IMGLD_linear_all (type, norm, float, 3d)) \
VIR_WRITE_IMAGE_IMGLD(type, 3d) \
};

extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_IMGLD_int[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_IMGLD_uint[3];
extern const gctCONST_STRING gcLibCL_VIR_ReadImage_WITH_IMGLD_float[3];
#endif

