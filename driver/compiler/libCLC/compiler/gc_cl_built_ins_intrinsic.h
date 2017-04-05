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


#ifndef __gc_cl_built_ins_intrinsic_h_
#define __gc_cl_built_ins_intrinsic_h_

/******************************************
struct _clsINTRINCSIC_BUILTIN_FUNCTION
{
    cleEXTENSION    extension;
    gctCONST_STRING symbol;
    gceINTRINSICS_KIND intrinsicKind;
    gctCONST_STRING    nameInLibrary;
    gctINT          returnType;
    gctUINT         paramCount;
    gctINT          paramTypes[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        ptrLevels[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctUINT8        typeConvertible[clmMAX_BUILT_IN_PARAMETER_COUNT];
    gctBOOL         isInline;
    gctBOOL         hasWriteArg;
    gctBOOL         passArgByRef;
    gctBOOL         hasVarArg;
}
*************************************************/

static clsINTRINSIC_BUILTIN_FUNCTION IntrinsicBuiltinFunctions[] =
{
    /* Intrinsic builtin functions */

    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long",   T_LONG,  3, {T_LONG,  T_LONG,  T_LONG},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong",  T_ULONG, 3, {T_ULONG, T_ULONG, T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long2",   T_LONG2,  3, {T_LONG2,  T_LONG2,  T_LONG2},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong2",  T_ULONG2, 3, {T_ULONG2, T_ULONG2, T_ULONG2}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long3",   T_LONG3,  3, {T_LONG3,  T_LONG3,  T_LONG3},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong3",  T_ULONG3, 3, {T_ULONG3, T_ULONG3, T_ULONG3}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long4",   T_LONG4,  3, {T_LONG4,  T_LONG4,  T_LONG4},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong4",  T_ULONG4, 3, {T_ULONG4, T_ULONG4, T_ULONG4}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long8",   T_LONG8,  3, {T_LONG8,  T_LONG8,  T_LONG8},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong8",  T_ULONG8, 3, {T_ULONG8, T_ULONG8, T_ULONG8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_long16",   T_LONG16,  3, {T_LONG16,  T_LONG16,  T_LONG16},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "mad_sat",           gceINTRIN_source, "_viv_madsat_ulong16",  T_ULONG16, 3, {T_ULONG16, T_ULONG16, T_ULONG16}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mul_long",     gceINTRIN_source, "_viv_mul_long",      T_LONG,  2, {T_LONG,  T_LONG},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mul_ulong",    gceINTRIN_source, "_viv_mul_ulong",     T_ULONG, 2, {T_ULONG, T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mulhi_long",   gceINTRIN_source, "_viv_mulhi_long",    T_LONG,  2, {T_LONG,  T_LONG},  {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_mulhi_ulong",  gceINTRIN_source, "_viv_mulhi_ulong",   T_ULONG, 2, {T_ULONG, T_ULONG}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "nextafter",         gceINTRIN_source, "_viv_nextafter",     T_FLOAT, 2, {T_FLOAT, T_FLOAT}, {0}, {0}, 1},

    {clvEXTENSION_NONE,  "_viv_get_image_width_image2d_t",   gceINTRIN_source,   "_viv_get_image_width_image2d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_width_image3d_t",   gceINTRIN_source,   "_viv_get_image_width_image3d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_width_image1d_t",   gceINTRIN_source,   "_viv_get_image_width_image1d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_width_image2d_array_t",   gceINTRIN_source,   "_viv_get_image_width_image2d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_width_image1d_array_t",   gceINTRIN_source,   "_viv_get_image_width_image1d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_width_imageld_buffer_t",  gceINTRIN_source,   "_viv_get_image_width_image1d_buufer_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_height_image2d_t",  gceINTRIN_source,   "_viv_get_image_height_image2d_t",  T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_height_image3d_t",  gceINTRIN_source,   "_viv_get_image_height_image3d_t",  T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_height_image2d_array_t",  gceINTRIN_source,   "_viv_get_image_height_image2d_array_t",  T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_depth_image3d_t",   gceINTRIN_source,   "_viv_get_image_depth_image3d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_data_type_image2d_t",   gceINTRIN_source,   "_viv_get_image_channel_data_type_image2d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_data_type_image3d_t",   gceINTRIN_source,   "_viv_get_image_channel_data_type_image3d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_data_type_image1d_t",   gceINTRIN_source,   "_viv_get_image_channel_data_type_image1d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_data_type_image2d_array_t",   gceINTRIN_source,   "_viv_get_image_channel_data_type_image2d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_data_type_image1d_array_t",   gceINTRIN_source,   "_viv_get_image_channel_data_type_image1d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_data_type_image1d_buffer_t",  gceINTRIN_source,   "_viv_get_image_channel_data_type_image1d_buffer_t",  T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_order_image2d_t",   gceINTRIN_source,   "_viv_get_image_channel_order_image2d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_order_image3d_t",   gceINTRIN_source,   "_viv_get_image_channel_order_image3d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_order_image1d_t",   gceINTRIN_source,   "_viv_get_image_channel_order_image1d_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_order_image2d_array_t",   gceINTRIN_source,   "_viv_get_image_channel_order_image2d_arrary_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_order_image1d_array_t",   gceINTRIN_source,   "_viv_get_image_channel_order_image1d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_channel_order_image1d_buffer_t",  gceINTRIN_source,   "_viv_get_image_channel_order_image1d_buffer_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_dim_image2d_t",   gceINTRIN_source,   "_viv_get_image_dim_image2d_t",   T_INT2, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_dim_image3d_t",   gceINTRIN_source,   "_viv_get_image_dim_image3d_t",   T_INT4, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_dim_image1d_t",   gceINTRIN_source,   "_viv_get_image_dim_image1d_t",   T_INT2, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_array_size_image1d_array_t",   gceINTRIN_source,   "_viv_get_image_array_size_image1d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
    {clvEXTENSION_NONE,  "_viv_get_image_array_size_image2d_array_t",   gceINTRIN_source,   "_viv_get_image_array_size_image2d_array_t",   T_INT, 1, {T_UINT8}, {0}, {0}, 1},
};

#define _cldIntrinsicBuiltinFunctionCount (sizeof(IntrinsicBuiltinFunctions) / sizeof(clsINTRINSIC_BUILTIN_FUNCTION))

#endif /* __gc_cl_built_ins_intrinsics_h_ */
