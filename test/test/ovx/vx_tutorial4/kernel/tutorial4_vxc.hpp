/****************************************************************************
*
*    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
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


char vxcTutorial4Source[] =
{
"#include \"cl_viv_vx_ext.h\" \n\
\n\
_viv_uniform VXC_512Bits EE_EO_AccTwoLines; \n\
_viv_uniform VXC_512Bits EO_OO_AccTwoLines; \n\
_viv_uniform VXC_512Bits EE_EO_AccOneLine; \n\
_viv_uniform VXC_512Bits EO_OO_AccOneLine; \n\
_viv_uniform VXC_512Bits EE_EO_PackBGR_0; \n\
_viv_uniform VXC_512Bits EE_EO_PackBGR_1; \n\
_viv_uniform VXC_512Bits EO_OO_PackBGR_0; \n\
_viv_uniform VXC_512Bits EO_OO_PackBGR_1; \n\
\n\
__kernel void tutorial4VXC \n\
    ( \n\
    __read_only image2d_t     in_image, \n\
    __write_only image2d_t     out_image \n\
    ) \n\
{ \n\
    int2 coord_in = (int2)(get_global_id(0), get_global_id(1)); \n\
    int2 coord_out = coord_in; \n\
    coord_out.x = coord_out.x * 3; \n\
    coord_in.xy = coord_in.xy - 2; \n\
\n\
    vxc_uchar16 bgr_register; \n\
    vxc_uchar16 lineA, lineB, lineC, lineD; \n\
    vxc_short8 acc_0, acc_1, acc_2, dst; \n\
 \n\
    //process EE EO  \n\
    VXC_ReadImage(lineA, in_image, coord_in, 0, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(lineB, in_image, coord_in, 0x20, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(lineC, in_image, coord_in, 0x40, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_DP4x8(acc_0, lineA, lineB, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), EE_EO_AccTwoLines); \n\
    VXC_ReadImage(lineA, in_image, coord_in, 0x60, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_ReadImage(lineD, in_image, coord_in, 0x80, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_DP4x8(acc_1, lineC, lineC, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), EE_EO_AccOneLine); \n\
    VXC_DP4x8(acc_2, lineD, lineA, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), EE_EO_AccTwoLines); \n\
    dst = acc_0 + acc_1 + acc_2; \n\
    VXC_DP2x8(bgr_register, lineC, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), EE_EO_PackBGR_0); \n\
    VXC_DP2x8(bgr_register, lineC, dst, VXC_MODIFIER(8, 11, 0, VXC_RM_TowardZero, 1), EE_EO_PackBGR_1); \n\
    VXC_WriteImage(out_image, coord_out, bgr_register, VXC_MODIFIER(0, 11, 0, VXC_RM_TowardZero, 0)); \n\
    coord_out.y ++; \n\
 \n\
    //process OE OO  \n\
    VXC_DP4x8(acc_0, lineB, lineC, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), EO_OO_AccTwoLines); \n\
    VXC_ReadImage(lineB, in_image, coord_in, 0xa0, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
    VXC_DP4x8(acc_2, lineA, lineA, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), EO_OO_AccOneLine); \n\
    VXC_DP4x8(acc_1, lineB, lineD, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), EO_OO_AccTwoLines); \n\
    dst = acc_0 + acc_1 + acc_2; \n\
    VXC_DP2x8(bgr_register, lineA, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), EO_OO_PackBGR_0); \n\
    VXC_DP2x8(bgr_register, lineA, dst, VXC_MODIFIER(8, 11, 0, VXC_RM_TowardZero, 1), EO_OO_PackBGR_1); \n\
    VXC_WriteImage(out_image, coord_out, bgr_register, VXC_MODIFIER(0, 11, 0, VXC_RM_TowardZero, 0)); \n\
}"
};
