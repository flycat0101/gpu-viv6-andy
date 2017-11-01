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



#ifndef __GC_VX_TENSOR_H__
#define __GC_VX_TENSOR_H__

EXTERN_C_BEGIN

#define TENSOR_DIM_NUM(tensor) \
    (tensor)->dimCount

#define TENSOR_VIEW_DIM_NUM(tensor) \
    (tensor)->viewRegion.dimCount

#define TENSOR_ORIG_DIM_NUM(tensor) \
    (tensor)->tensorBuffer->memory.dimCount

#define TENSOR_LOGICAL_ADDR(tensor) \
    (tensor)->tensorBuffer->memory.logicals[0]

#define TENSOR_PHYSICAL_ADDR(tensor) \
    (tensor)->tensorBuffer->memory.physicals[0]

#define TENSOR_SIZES(tensor) \
    (tensor)->dims

#define TENSOR_ORIG_SIZES(tensor) \
    (tensor)->tensorBuffer->memory.dims[0]

#define TENSOR_DATA_TYPE(tensor) \
    (tensor)->tensorBuffer->dataFormat

#define TENSOR_POS(tensor) \
    (tensor)->tensorBuffer->fixedPointPos

#define TENSOR_DATA_SIZE(tensor) \
    (tensor)->tensorBuffer->elementSize

#define TENSOR_ROUNDING_MODE(tensor) \
    (tensor)->tensorBuffer->roundingMode

#define TENSOR_SIZE_INDEX(tensor, index) \
    (TENSOR_SIZES(tensor)[index])

#define TENSOR_VIEW_SIZE_INDEX(tensor, index) \
    ((tensor)->viewRegion.viewEnds[index] - (tensor)->viewRegion.viewStarts[index])

#define TENSOR_STRIDES(tensor) \
    (tensor)->strides

#define TENSOR_ORIG_STRIDES(tensor) \
    (tensor)->tensorBuffer->memory.strides[0]

#define TENSOR_STRIDE_INDEX(tensor, index) \
    (TENSOR_STRIDES(tensor)[index])


VX_INTERNAL_CALLBACK_API void
vxoTensor_Destructor(
    vx_reference ref
    );

VX_INTERNAL_API vx_bool
vxoTensor_IsValidTensor(
    vx_tensor tensor
    );

VX_INTERNAL_API vx_bool
vxoTensor_IsValidView(
    vx_tensor_view view
    );

VX_INTERNAL_API vx_bool
vxoTensor_IsValidAddressing(
    vx_tensor_addressing addressing
    );

VX_INTERNAL_API vx_bool
vxoTensor_IsOverlap(
    vx_tensor tensor1,
    vx_tensor tensor2
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorBaseMemory(
    vx_tensor tensor,
    gctPOINTER * logical,
    vx_uint32 * physical
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorViewMemory(
    vx_tensor tensor,
    gctPOINTER * logical,
    vx_uint32 * physical
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorSize(
    vx_tensor tensor,
    vx_uint32 *size
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorElementCount(
    vx_tensor tensor,
    vx_uint32 *count
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorDimStride(
    vx_tensor tensor,
    vx_uint32 * count,
    vx_uint32 * sizes,
    vx_uint32 * strides
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorViewRegion(
    vx_tensor tensor,
    vx_uint32 count,
    vx_uint32 * starts,
    vx_uint32 * ends
    );

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensor(
    vx_context context,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_bool is_virtual
    );

VX_INTERNAL_API vx_status
vxoTensor_ReleaseTensor(
    vx_tensor *tensor
    );

VX_INTERNAL_API vx_status
vxoTensor_AllocateMemory(
    vx_tensor tensor
    );

VX_INTERNAL_API vx_status
vxoTensor_ReleaseMemory(
    vx_tensor tensor
    );

VX_INTERNAL_API void
vxoTensor_CopyTensorPatchEx(
    vx_uint8 * src,
    vx_uint8 * dst,
    vx_uint32 curDim,
    vx_uint32 * sizes,
    vx_uint32 * srcStrides,
    vx_uint32 * dstStrides,
    vx_enum srcType,
    vx_enum dstType
    );

VX_INTERNAL_API vx_image
vxoTensor_CreateImageFromTensor(
    vx_tensor tensor,
    vx_uint32 width,
    vx_uint32 height,
    vx_df_image format
    );

EXTERN_C_END

#endif /* __GC_VX_TENSOR_H__*/

