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
    (tensor)->dataFormat

#define TENSOR_POS(tensor) \
    (tensor)->fixedPointPos

#define TENSOR_QUANT_TYPE(tensor) \
    (tensor)->quantFormat

#define TENSOR_TF_SCALE(tensor) \
    (tensor)->scale

#define TENSOR_TF_ZEROPOINT(tensor) \
    (tensor)->zeroPoint

#define TENSOR_DATA_SIZE(tensor) \
    (tensor)->elementSize

#define TENSOR_ROUNDING_MODE(tensor) \
    (tensor)->tensorBuffer->roundingMode

#define TENSOR_PAD_ZERO_VALUE(tensor) \
    (tensor)->tensorBuffer->padZeorValue

#define TENSOR_VALUED(tensor) \
    (tensor)->tensorBuffer->valued

#define TENSOR_PRECISION(tensor) \
    (tensor)->tensorBuffer->precision

#define TENSOR_DATA_LIFETIME(tensor) \
    (tensor)->tensorBuffer->data_lifetime

#define TENSOR_RANK(tensor) \
    (tensor)->rank

#define TENSOR_SIZE_INDEX(tensor, index) \
    (TENSOR_SIZES(tensor)[index])

#define TENSOR_VIEW_STARTS(tensor) \
    (tensor)->viewRegion.viewStarts

#define TENSOR_VIEW_ENDS(tensor) \
    (tensor)->viewRegion.viewEnds

#define TENSOR_VIEW_SIZE_INDEX(tensor, index) \
    ((tensor)->viewRegion.viewEnds[index] - (tensor)->viewRegion.viewStarts[index])

#define TENSOR_VIEW_START_INDEX(tensor, index) \
    ((tensor)->viewRegion.viewStarts[index])

#define TENSOR_VIEW_END_INDEX(tensor, index) \
    ((tensor)->viewRegion.viewEnds[index])

#define TENSOR_STRIDES(tensor) \
    (tensor)->strides

#define TENSOR_ORIG_STRIDES(tensor) \
    (tensor)->tensorBuffer->memory.strides[0]

#define TENSOR_STRIDE_INDEX(tensor, index) \
    (TENSOR_STRIDES(tensor)[index])

#define TENSOR_GRAPH(tensor) \
    (tensor)->tensorBuffer->memory.graph

#define TENSOR_TF_VALUED(tensor) \
    (tensor)->tensorBuffer->valued


VX_INTERNAL_CALLBACK_API void
vxoTensor_Destructor(
    vx_reference ref
    );

VX_INTERNAL_API vx_bool
vxoTensor_IsValidTensor(
    vx_tensor tensor
    );

VX_INTERNAL_API vx_bool
vxoTensor_IsVirtualTensor(
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

VX_INTERNAL_API vx_bool
vxoTensor_IsShared(
    vx_tensor tensor1,
    vx_tensor tensor2
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorViewMemory(
    vx_tensor tensor,
    gctPOINTER * logical,
    vx_uint32 * physical
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorViewOffset(
    vx_tensor  tensor,
    vx_uint32* offset
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorBatchArrayViewMemory(
    vx_tensor tensor,
    vx_uint32 batchIndex,
    gctPOINTER * logical,
    vx_uint32 * physical
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorElementCount(
    vx_tensor tensor,
    vx_uint32 *count
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorSize(
    vx_tensor tensor,
    vx_uint32 *size
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorWholeElementCount(
    vx_tensor tensor,
    vx_uint32 *count
    );

VX_INTERNAL_API vx_status
vxoTensor_GetTensorWholeSize(
    vx_tensor tensor,
    vx_size   *size
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
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_bool is_virtual
    );

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensorEx(
    vx_context context,
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_bool is_virtual
    );

VX_INTERNAL_API vx_tensor
vxoTensor_CreateVirtualTensor(
    vx_graph graph,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_int8 fixed_point_pos
    );

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensor2(
    vx_context context,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_size size_of_create_params
    );

VX_INTERNAL_API vx_tensor
vxoTensor_CreateVirtualTensor2(
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_size size_of_create_params
    );

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensorFromView(
    vx_tensor tensor,
    vx_tensor_view view
    );

VX_INTERNAL_API vx_tensor
vxoTensor_ReshapeTensor(
    vx_tensor    tensor,
    vx_int32*    num_of_dims,
    vx_uint32    sizes
    );

VX_INTERNAL_API vx_status
vxoTensor_ReleaseTensor(
    vx_tensor *tensor
    );

VX_INTERNAL_API vx_status
vxoTensor_ReleaseTensorEx(
    vx_tensor *tensor
    );

VX_INTERNAL_API vx_status
vxoTensor_SetMemorySize(
    vx_tensor tensor,
    vx_uint32 size
    );

VX_INTERNAL_API vx_status
vxoTensor_AllocateMemory(
    vx_tensor tensor
    );

VX_INTERNAL_API vx_status
vxoTensor_ReleaseMemory(
    vx_tensor tensor
    );

VX_INTERNAL_API vx_bool
vxoTensor_MemoryIsAllocated(
    vx_tensor tensor
    );

VX_INTERNAL_API vx_status
vxoTensor_SetAddress(
    vx_tensor    tensor,
    vx_uint8_ptr logical,
    vx_uint32    physical
    );

VX_INTERNAL_API vx_status
vxoTensor_AdjustAddress(
    vx_tensor tensor,
    vx_size   all_size
    );

VX_INTERNAL_API vx_bool
vxoTensor_IsAllocated(
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

VX_INTERNAL_API vx_int32 vxoTensor_CheckTensorViewSizes(
    vx_uint32* dimensions,
    const vx_size * view_start,
    const vx_size * view_end,
    vx_size number_of_dimensions
    );

VX_INTERNAL_API vx_status
vxoCopyTensorPatch(
    vx_tensor tensor,
    vx_tensor_view view,
    vx_tensor_addressing user_addr,
    void *user_ptr,
    vx_enum usage,
    vx_enum user_mem_type
    );

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensorWithStrides(
    vx_context context,
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_uint32 * strides,
    vx_bool is_virtual
    );

VX_INTERNAL_API vx_tensor
vxoTensor_ReformatTensor(
    vx_tensor tensor,
    vx_enum format
    );

EXTERN_C_END

#endif /* __GC_VX_TENSOR_H__*/

