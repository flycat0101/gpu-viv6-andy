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


#ifndef __GC_VX_IMAGE_H__
#define __GC_VX_IMAGE_H__

EXTERN_C_BEGIN

VX_INTERNAL_API vx_bool vxImageFormat_IsSupported(vx_df_image imageFormat);

VX_INTERNAL_API vx_size vxImageFormat_GetChannelSize(vx_df_image color);

VX_INTERNAL_API vx_uint32 vxComputePlaneOffset(vx_image image, vx_uint32 x, vx_uint32 y, vx_uint32 planeIndex);

VX_INTERNAL_API vx_image vxoImage_LocateROI(vx_image image, OUT vx_rectangle_t *rect);

VX_INTERNAL_API vx_bool vxoImage_IsValid(vx_image image);

VX_INTERNAL_API void vxoImage_Initialize(
        vx_image image, vx_uint32 width, vx_uint32 height, vx_df_image imageFormat);

VX_INTERNAL_CALLBACK_API void vxoImage_Destructor(vx_reference ref);

VX_INTERNAL_API void vxoImage_Dump(vx_image image);

VX_INTERNAL_API vx_bool vxoImage_AllocateMemory(vx_image image);

VX_INTERNAL_API void vxoImage_FreeMemory(vx_image image);

VX_INTERNAL_API vx_bool vxoImage_WrapUserMemory(vx_image image);

VX_INTERNAL_API void vxoImage_FreeWrappedMemory(vx_image image);

VX_INTERNAL_API vx_image VX_API_CALL vxoImage_CreateImageFromInternalHandle(vx_context context, vx_df_image format, vx_imagepatch_addressing_t *addr, void **ptrs, vx_uint32 *phys);

EXTERN_C_END

#endif /* __GC_VX_IMAGE_H__ */

