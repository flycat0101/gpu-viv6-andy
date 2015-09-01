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


#ifndef __GC_VX_PYRAMID_H__
#define __GC_VX_PYRAMID_H__

EXTERN_C_BEGIN

void vxReleasePyramidInt(vx_pyramid pyramid);

VX_INTERNAL_API vx_status vxoPyramid_Initialize(
        vx_pyramid pyramid, vx_size levels, vx_float32 scale,
        vx_uint32 width, vx_uint32 height, vx_df_image format);

VX_INTERNAL_CALLBACK_API void vxoPyramid_Destructor(vx_reference ref);

EXTERN_C_END

#endif /* __GC_VX_PYRAMID_H__*/
