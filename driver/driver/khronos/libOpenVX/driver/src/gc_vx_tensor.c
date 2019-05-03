/****************************************************************************
*
*    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_TENSOR

static vx_float32 Fp16toFp32(vx_int16 in)
{
    vx_int32 t1;
    vx_int32 t2;
    vx_int32 t3;
    vx_float32 out;

    t1 = in & 0x7fff;                       // Non-sign bits
    t2 = in & 0x8000;                       // Sign bit
    t3 = in & 0x7c00;                       // Exponent

    t1 <<= 13;                              // Align mantissa on MSB
    t2 <<= 16;                              // Shift sign bit into position

    t1 += 0x38000000;                       // Adjust bias

    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

    t1 |= t2;                               // Re-insert sign bit

    *((uint32_t*)&out) = t1;

    return out;
}

static vx_uint16 Fp32toFp16(vx_float32 in)
{
    vx_uint32 fp32 = *((vx_uint32 *) &in);
    vx_uint32 t1 = (fp32 & 0x80000000u) >> 16;  /* sign bit. */
    vx_uint32 t2 = (fp32 & 0x7F800000u) >> 13;  /* Exponent bits */
    vx_uint32 t3 = (fp32 & 0x007FE000u) >> 13;  /* Mantissa bits, no rounding */
    vx_uint32 fp16 = 0u;

    if (t2 >= 0x023c00u)
    {
        fp16 = t1 | 0x7BFF;     /* Don't round to infinity. */
    }
    else if (t2 <= 0x01c000u)
    {
        fp16 = t1;
    }
    else
    {
        t2 -= 0x01c000u;
        fp16 = t1 | t2 | t3;
    }

    return (vx_uint16) fp16;
}

#define F32TOF16(in) Fp32toFp16(in)
#define F16TOF32(in) Fp16toFp32(in)


#define GET_CONTEXT(rf) \
    ((vx_reference)rf)->context

#define TENSOR_VIEW_DIM_NUM(tensor) \
    (tensor)->viewRegion.dimCount

enum vx_tensor_type_e
{
    VX_TENSOR_NORMAL  = 0x001,
    VX_TENSOR_VIRTUAL = 0x010,
    VX_TENSOR_SHARED  = 0x100,
    VX_TENSOR_NUM,
};

VX_PRIVATE_API vx_bool
vxoTensor_DataFormatIsSupported(
    vx_enum data_format
    )
{
    switch (data_format)
    {
        case VX_TYPE_INT8:
        case VX_TYPE_UINT8:
        case VX_TYPE_INT16:
        case VX_TYPE_INT32:
        case VX_TYPE_UINT16:
        case VX_TYPE_UINT32:
        case VX_TYPE_FLOAT16:
        case VX_TYPE_FLOAT32:
        case VX_TYPE_INT64:
            return vx_true_e;

        default:
            return vx_false_e;
    }
}

VX_PRIVATE_API vx_bool
vxoTensor_ImageFormatIsSupported(
    vx_enum image_format
    )
{
    switch (image_format)
    {
        case VX_DF_IMAGE_RGB:
        case VX_DF_IMAGE_RGBX:
        case VX_DF_IMAGE_UYVY:
        case VX_DF_IMAGE_YUYV:
        case VX_DF_IMAGE_U8:
        case VX_DF_IMAGE_U16:
        case VX_DF_IMAGE_S16:
        case VX_DF_IMAGE_U32:
        case VX_DF_IMAGE_S32:
        case VX_DF_IMAGE_F32:
            return vx_true_e;

        default:
            return vx_false_e;
    }
}

VX_PRIVATE_API vx_bool
vxoTensor_ViewRegionIsValid(
    vx_view_region_s * region
    )
{
    vx_uint32 i;

    if (region == VX_NULL || !region->dimCount) return vx_false_e;

    for (i = 0; i < region->dimCount; i++)
    {
        if (region->viewStarts[i] > region->viewEnds[i])
        {
            return vx_false_e;
        }
    }

    return vx_true_e;
}

VX_INTERNAL_API vx_int32 vxoTensor_CheckTensorViewSizes(
    vx_uint32* dimensions,
    const vx_size * view_start,
    const vx_size * view_end,
    vx_size number_of_dimensions
    )
{
    vx_size i;

    for (i = 0; i < number_of_dimensions; i++)
    {
        if (view_end[i] <= view_start[i] ||
                (vx_uint32)(view_end[i]) > dimensions[i])
            return -1;
    }
    return 0;

}

VX_PRIVATE_API vx_uint32
vxoTensor_GetDataSizeByFormat(
    vx_enum format
    )
{
    switch (format)
    {
        case VX_TYPE_INT8:
        case VX_TYPE_UINT8:
            return 1;

        case VX_TYPE_INT16:
        case VX_TYPE_UINT16:
        case VX_TYPE_FLOAT16:
            return 2;

        case VX_TYPE_INT32:
        case VX_TYPE_UINT32:
        case VX_TYPE_FLOAT32:
            return 4;
        case VX_TYPE_INT64:
            return 8;
        default:
            return 0;
    }
}

VX_PRIVATE_API vx_uint32
vxoTensor_CalculateDimOffsetByStarts(
    vx_tensor tensor,
    vx_uint32 starts[]
    )
{
    vx_uint32 i, offset = 0;

    for (i = 0; i < TENSOR_DIM_NUM(tensor); i++)
    {
        offset += starts[i] * TENSOR_STRIDE_INDEX(tensor, i);
    }

    return offset;
}

VX_PRIVATE_API vx_uint32
vxoTensor_CalculateMaxMemorySize(
    vx_tensor tensor
    )
{
    vx_int32 i;

    for (i = TENSOR_DIM_NUM(tensor) - 1; i >= 0; i--)
    {
        if (TENSOR_VIEW_SIZE_INDEX(tensor, i) != 1) break;
    }

    return TENSOR_STRIDE_INDEX(tensor, i) * TENSOR_VIEW_SIZE_INDEX(tensor, i);
}

VX_PRIVATE_API void
vxoTensor_CalculateSizesFromViewRegion(
    vx_view_region_s * view,
    vx_uint32 sizes[]
    )
{
    vx_uint32 i;

    for (i = 0; i < view->dimCount; i++)
    {
        sizes[i] = view->viewEnds[i] - view->viewStarts[i];
    }
}

#define VX_MAX(a, b) (a) > (b) ? (a) : (b)
#define VX_MIN(a, b) (a) < (b) ? (a) : (b)

VX_PRIVATE_API vx_bool
vxoTensor_MergeTwoViews(
    vx_view_region_s * viewA,
    vx_view_region_s * viewB,
    vx_view_region_s * viewOut
    )
{
    vx_uint32 i;

    gcmHEADER_ARG("viewA=%p, viewB=%p, viewOut=%p", viewA, viewB, viewOut);

    if (viewA->dimCount != viewB->dimCount)
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }
    viewOut->dimCount = viewA->dimCount;

    for (i = 0; i < viewA->dimCount; i++)
    {
        viewOut->viewStarts[i] = VX_MAX(viewA->viewStarts[i], viewB->viewStarts[i]);
        viewOut->viewEnds[i] = VX_MAX(1, VX_MIN(viewA->viewEnds[i], viewB->viewEnds[i]));
    }

    for (i = viewA->dimCount; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        viewOut->viewStarts[i] = 0;
        viewOut->viewEnds[i] = 1;
    }

    gcmFOOTER_NO();
    return vx_true_e;
}

/* vxoTensor_RectIsIn3DTensorRange_11 is for back compatibility with spec 1.1*/
VX_PRIVATE_API vx_bool
vxoTensor_RectIsIn3DTensorRange_11(
    vx_tensor tensor,
    vx_rectangle_t rect,
    vx_uint32 stride,
    vx_uint32 array_size
    )
{
    vx_uint32 dimx, dimy, dimz;

    dimz = TENSOR_VIEW_SIZE_INDEX(tensor, 2);

    if (array_size > dimz)
    {
        return vx_false_e;
    }

    dimx = TENSOR_VIEW_SIZE_INDEX(tensor, 0);
    dimy = TENSOR_VIEW_SIZE_INDEX(tensor, 1);

    if (rect.end_x > dimx ||
        rect.end_y > dimy)
    {
        return vx_false_e;
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_bool
vxoTensor_RectIsIn3DTensorRange(
    vx_tensor tensor,
    const vx_rectangle_t * rect,
    vx_uint32 stride,
    vx_uint32 array_size
    )
{
    vx_uint32 dimx, dimy, dimz;

    dimz = TENSOR_VIEW_SIZE_INDEX(tensor, 2);

    if (array_size > dimz)
    {
        return vx_false_e;
    }

    dimx = TENSOR_VIEW_SIZE_INDEX(tensor, 0);
    dimy = TENSOR_VIEW_SIZE_INDEX(tensor, 1);

    if (rect->end_x > dimx ||
        rect->end_y > dimy)
    {
        return vx_false_e;
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_bool
vxoTensor_IsContiguousMemory(
    vx_tensor tensor
    )
{
    vx_int32 i, j;

    for (j = TENSOR_DIM_NUM(tensor) - 1; j >= 0; j--)
    {
        if (TENSOR_VIEW_SIZE_INDEX(tensor, j) != 1) break;
    }

    for (i = 0; i < j; i++)
    {
        if (TENSOR_VIEW_SIZE_INDEX(tensor, i) != TENSOR_SIZE_INDEX(tensor, i))
        {
            return vx_false_e;
        }
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_tensor
vxoTensor_Create(
    vx_context context,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_uint32 * strides,
    vx_view_region_s * viewRegion,
    vx_tensor_buffer_s * tensorBuffer,
    vx_uint32 baseOffset,
    vx_enum tensorType,
    vx_reference_kind_e kind
    )
{
    vx_tensor tensor = VX_NULL;
    vx_uint32 i, elementSize, uSizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    gcmHEADER_ARG("context=%p, tensor_create_params=%p, strides=%p, viewRegion=%p, tensorBuffer=%p, baseOffset=0x%x, tensorType=0x%x, kind=0x%x",
        context, tensor_create_params, strides, viewRegion, tensorBuffer, baseOffset, tensorType, kind);
    if (tensor_create_params == VX_NULL) goto OnError;

    gcoOS_MemCopy(uSizes, tensor_create_params->sizes, VX_CONTEXT_TENSOR_MAX_DIMENSION * sizeof(vx_uint32));

    tensor = (vx_tensor)vxoReference_Create(context, VX_TYPE_TENSOR, kind, &context->base);
    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS) goto OnError;

    tensor->quantFormat = tensor_create_params->quant_format;
    tensor->fixedPointPos = 0;
    tensor->scale = 1.0f;
    tensor->zeroPoint = 0;

    if (tensor->quantFormat == VX_QUANT_AFFINE_SCALE)
    {
        tensor->scale = tensor_create_params->quant_data.affine.scale;
        tensor->zeroPoint = tensor_create_params->quant_data.affine.zeroPoint;
    }
    else
    {
        tensor->fixedPointPos = tensor_create_params->quant_data.dfp.fixed_point_pos;
    }

    if (!(tensorType & VX_TENSOR_SHARED))
    {
        tensor->tensorBuffer = (vx_tensor_buffer_s *)vxAllocateAndZeroMemory(sizeof(vx_tensor_buffer_s));
        if (tensor->tensorBuffer == VX_NULL) goto OnError;

        elementSize = vxoTensor_GetDataSizeByFormat(tensor_create_params->data_format);
        if (!elementSize) goto OnError;

        tensor->tensorBuffer->elementSize = elementSize;
        tensor->tensorBuffer->dataFormat = tensor_create_params->data_format;


        if (tensor->quantFormat == VX_QUANT_AFFINE_SCALE)
        {
            tensor->tensorBuffer->padZeorValue = tensor->zeroPoint;
        }

        tensor->tensorBuffer->memory.logicals[0] = VX_NULL;
        tensor->tensorBuffer->memory.physicals[0] = 0;
        tensor->tensorBuffer->memory.planeCount = 1;
        tensor->tensorBuffer->memory.dimCount = tensor->dimCount = tensor_create_params->num_of_dims;

        for (i = 0; i < tensor->tensorBuffer->memory.planeCount; i++)
        {
            tensor->tensorBuffer->memory.sizes[i] = 0;
        }

        tensor->viewRegion.dimCount = tensor_create_params->num_of_dims;
        tensor->baseAddressOffset = 0;

        /* set the left dimensions to 1 */
        for (i = tensor_create_params->num_of_dims; i< VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
        {
            uSizes[i] = 1;
        }

        for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
        {
            tensor->dims[i] = viewRegion != VX_NULL ? viewRegion->viewEnds[i] - viewRegion->viewStarts[i] : uSizes[i];
            tensor->strides[i] = strides != VX_NULL ? strides[i] : i ? uSizes[i-1] * tensor->strides[i-1] : elementSize;

            tensor->tensorBuffer->memory.dims[0][i] = tensor->dims[i] = viewRegion != VX_NULL ? viewRegion->viewEnds[i] - viewRegion->viewStarts[i] : uSizes[i];
            tensor->tensorBuffer->memory.strides[0][i] = tensor->strides[i] =
                strides != VX_NULL ? strides[i] : i ? uSizes[i-1] * tensor->tensorBuffer->memory.strides[0][i-1]: elementSize;

            tensor->viewRegion.viewStarts[i] = viewRegion != VX_NULL ? viewRegion->viewStarts[i] : 0;
            tensor->viewRegion.viewEnds[i] = viewRegion != VX_NULL ? viewRegion->viewEnds[i] : uSizes[i];
        }

        tensor->tensorBuffer->valued        = vx_false_e;
        tensor->tensorBuffer->data_lifetime = VX_TENSOR_LIFE_TIME_DYNAMIC;

        /*it is used by nnapi and graph optimization*/
        if(tensor->tensorBuffer->elementSize == 1 || tensor->tensorBuffer->elementSize == 2)
            tensor->tensorBuffer->precision     = VX_TENSOR_PRECISION_AUTO;
        else
            tensor->tensorBuffer->precision     = VX_TENSOR_PRECISION_HIGH;
    }
    else if (tensorBuffer != VX_NULL)
    {
        /* create from view or reshape */
        tensor->dimCount = tensor_create_params->num_of_dims;
        tensor->viewRegion.dimCount = tensor_create_params->num_of_dims;

        /* set the left dimensions to 1 */
        for (i = tensor_create_params->num_of_dims; i< VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
        {
            uSizes[i] = 1;
        }

        for (i = 0; i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
        {
            tensor->dims[i] = viewRegion != VX_NULL ? viewRegion->viewEnds[i] - viewRegion->viewStarts[i] : uSizes[i];
            tensor->strides[i] = strides != VX_NULL ? strides[i] : i ? uSizes[i-1] * tensor->strides[i-1] : tensorBuffer->elementSize;
            tensor->viewRegion.viewStarts[i] = 0;
            tensor->viewRegion.viewEnds[i] = uSizes[i];
        }

        tensor->tensorBuffer = tensorBuffer;
        tensor->isVirtual = vx_false_e;
        tensor->baseAddressOffset = baseOffset;

        if (viewRegion != VX_NULL)
        {
            vxMemCopy(&tensor->viewRegion, viewRegion, sizeof(vx_view_region_s));
            tensor->isViewed = vx_true_e;
        }
    }
    else
    {
        goto OnError;
    }

    tensor->viewOffset = 0;
    tensor->tensorBuffer->refNum = 0;
    tensor->tensorBuffer->memory.lastUseId = VXNNE_MEM_ID_INIT_VALUE;
    tensor->tensorBuffer->memory.firstUseId = VXNNE_MEM_ID_INIT_VALUE;

    if (tensorType & VX_TENSOR_VIRTUAL)
    {
        tensor->isVirtual = vx_true_e;
    }

    tensor->tensorBuffer->bufRefCount++;

    tensor->tensorBuffer->roundingMode = ((tensor->tensorBuffer->dataFormat == VX_TYPE_INT8) || (tensor->tensorBuffer->dataFormat == VX_TYPE_INT16))
                                         ? VX_NN_ROUNDING_MODE_RTNE : VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING;

    if (tensor->base.context->options.nnRoundingMode)
    {
        gctSTRING env = tensor->base.context->options.nnRoundingMode;
        if (env)
        {
            if (gcoOS_StrCmp(env, "SIMPLE") == 0)
            {
                tensor->tensorBuffer->roundingMode = VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING;
            }
            else if (gcoOS_StrCmp(env, "RTNE") == 0)
            {
                tensor->tensorBuffer->roundingMode = VX_NN_ROUNDING_MODE_RTNE;
            }
            else if (gcoOS_StrCmp(env, "RTZ") == 0)
            {
                tensor->tensorBuffer->roundingMode = VX_NN_ROUNDING_MODE_RTZ;
            }
            else if (gcoOS_StrCmp(env, "RTNI") == 0)
            {
                tensor->tensorBuffer->roundingMode = VX_NN_ROUNDING_MODE_RTNI;
            }
        }
    }

    context->allTensorNum++;

    tensor->brickMode = vx_false_e;

    tensor->memorySize = 0;

    tensor->rank            = VX_TENSOR_RANK_WHCN;
    tensor->reshape         = VX_NULL;

    tensor->useInternalMem  = vx_true_e;

    gcmFOOTER_NO();
    return tensor;

OnError:
    if (!(tensorType & VX_TENSOR_SHARED) &&
        tensor->tensorBuffer != VX_NULL)
    {
        vxFree(tensor->tensorBuffer);
        tensor->tensorBuffer = VX_NULL;
    }

    if (tensor != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&tensor, VX_TYPE_TENSOR, VX_REF_EXTERNAL);
    }

    gcmFOOTER_NO();
    return VX_NULL;
}

VX_PRIVATE_API vx_bool
    vxoTensor_ComputeViewOffset(
    vx_tensor tensor,
    vx_tensor_view view
    )
{
    vx_uint32 i;
    gcmHEADER_ARG("tensor=%p, view=%p", tensor, view);

    if(tensor->isViewed)
    {
        for(i = 0; i < TENSOR_DIM_NUM(tensor); i++)
        {
            if(view->viewRegion.viewStarts[i] != tensor->viewRegion.viewStarts[i] ||
                view->viewRegion.viewEnds[i] != tensor->viewRegion.viewEnds[i])
            {
                view->viewRegion.viewStarts[i] += tensor->viewRegion.viewStarts[i];
                view->viewRegion.viewEnds[i] += tensor->viewRegion.viewStarts[i];
            }
        }
    }

    for (i = 0; i < TENSOR_DIM_NUM(tensor); i++)
    {
        if (view->viewRegion.viewStarts[i] < tensor->viewRegion.viewStarts[i] ||
            view->viewRegion.viewEnds[i] > tensor->viewRegion.viewEnds[i])
        {
            vxError("The %dth view dim range [%d - %d] is beyond tensor orignal range [%d - %d]", i,
                view->viewRegion.viewStarts[i], view->viewRegion.viewEnds[i],
                tensor->viewRegion.viewStarts[i], tensor->viewRegion.viewEnds[i]);
            gcmFOOTER_NO();
            return vx_false_e;
        }
    }
    gcmFOOTER_NO();
    return vx_true_e;
}

VX_PRIVATE_API vx_bool
vxoTensor_CheckValidTensorView(
    vx_tensor tensor,
    vx_tensor_view view
    )
{
    if (!vxoTensor_IsValidTensor(tensor)) return vx_false_e;
    if (!vxoTensor_IsValidView(view)) return vx_false_e;

    if (TENSOR_DIM_NUM(tensor) != view->viewRegion.dimCount)
    {
        vxError("The tensor dim %d is not equal to view dim %d",
                 TENSOR_DIM_NUM(tensor), TENSOR_VIEW_DIM_NUM(tensor));
        return vx_false_e;
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_bool
vxoTensor_ViewRegionIsEqualAddressing(
    vx_view_region_s * viewRegion,
    vx_tensor_addressing addressing
    )
{
    vx_uint32 i;

    for (i = 0; i < viewRegion->dimCount; i++)
    {
        if (viewRegion->viewStarts[i] ||
            viewRegion->viewEnds[i] != addressing->dimSizesUser[i])
        {
            return vx_false_e;
        }
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_bool
vxoTensor_CheckValidTensorAddressing(
    vx_tensor tensor,
    vx_tensor_addressing addressing
    )
{
    vx_uint32 i;
    gcmHEADER_ARG("tensor=%p, addressing=%p", tensor, addressing);

    if (!vxoTensor_IsValidTensor(tensor))
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }
    if (!vxoTensor_IsValidAddressing(addressing))
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }
    if (TENSOR_DIM_NUM(tensor) != addressing->dimCount)
    {
        vxError("The tensor dim %d is not equal to addressing dim %d",
                 TENSOR_DIM_NUM(tensor), addressing->dimCount);
        gcmFOOTER_NO();
        return vx_false_e;
    }

    for (i = 0; i < TENSOR_DIM_NUM(tensor); i++)
    {
        if (addressing->dimSizesUser[i] > TENSOR_VIEW_SIZE_INDEX(tensor, i))
        {
            vxError("The %dth addressing dim size %d is beyond tensor orignal range %d",
                     i, addressing->dimSizesUser[i], TENSOR_VIEW_END_INDEX(tensor, i));
            gcmFOOTER_NO();
            return vx_false_e;
        }
    }

    gcmFOOTER_NO();
    return vx_true_e;
}

VX_PRIVATE_API vx_bool
vxoTensor_CheckValidViewAddressing(
    vx_tensor_view view,
    vx_tensor_addressing addressing
    )
{
    vx_uint32 i;
    gcmHEADER_ARG("view=%p, addressing=%p", view, addressing);

    if (!vxoTensor_IsValidView(view))
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }
    if (!vxoTensor_IsValidAddressing(addressing))
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }
    if (view->viewRegion.dimCount != addressing->dimCount)
    {
        vxError("The view dim %d is not equal to addressing dim %d", view->viewRegion.dimCount, addressing->dimCount);
        gcmFOOTER_NO();
        return vx_false_e;
    }

    for (i = 0; i < view->viewRegion.dimCount; i++)
    {
        if (view->viewRegion.viewEnds[i] - view->viewRegion.viewStarts[i] != addressing->dimSizesUser[i])
        {
            vxError("The %dth addressing size %d is not equel to view range [%d - %d]",
                     i, addressing->dimSizesUser[i], view->viewRegion.viewStarts[i], view->viewRegion.viewEnds[i]);
            gcmFOOTER_NO();
            return vx_false_e;
        }
    }
    gcmFOOTER_NO();
    return vx_true_e;
}

VX_PRIVATE_API vx_status
vxoTensor_GetTensorBaseMemory(
    vx_tensor tensor,
    vx_uint8_ptr * logical,
    vx_uint32 * physical
    )
{
    if (!vxoTensor_IsValidTensor(tensor))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (logical == VX_NULL && physical == VX_NULL)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (vxoMemory_GetType(&tensor->tensorBuffer->memory) == VXNNE_MEM_POOL_TYPE_ORIG_DDR &&
        !vxoTensor_MemoryIsAllocated(tensor))
    {
        return VX_ERROR_NOT_ALLOCATED;
    }

    if (logical != VX_NULL) *logical = TENSOR_LOGICAL_ADDR(tensor) + tensor->baseAddressOffset;
    if (physical != VX_NULL) *physical = TENSOR_PHYSICAL_ADDR(tensor) + tensor->baseAddressOffset;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoTensor_GetTensorBatchArrayBaseMemory(
    vx_tensor tensor,
    vx_uint32 batchIndex,
    gctPOINTER * logical,
    vx_uint32 * physical
    )
{
    vx_status status;
    vx_uint8_ptr logicalBase;
    vx_uint32  physicalBase;
    vx_uint32 strideIndex;

    status = vxoTensor_GetTensorBaseMemory(tensor, &logicalBase, &physicalBase);
    if (status != VX_SUCCESS) return status;

    strideIndex = (tensor->dimCount == 1) ? 0 : (tensor->dimCount - 1);

    if (logical != VX_NULL) *logical = logicalBase + batchIndex * tensor->strides[strideIndex];
    if (physical != VX_NULL) *physical = physicalBase + batchIndex * tensor->strides[strideIndex];

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
vxoTensor_LineCopyConvert(
    vx_uint8 * src,
    vx_uint8 * dst,
    vx_enum srcType,
    vx_enum dstType,
    vx_uint32 count
    )
{
    vx_uint32 ssize, dsize;

    gcmHEADER_ARG("src=%p, dst=%p, srcType=0x%x, dstType=0x%x, count=0x%x", src, dst, srcType, dstType, count);

    if (!src || !dst || !count)
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }
    if (srcType == dstType)
    {
        ssize = vxoTensor_GetDataSizeByFormat(srcType);
        vxMemCopy(dst, src, ssize * count);
    }
    else
    {
        ssize = vxoTensor_GetDataSizeByFormat(srcType);
        dsize = vxoTensor_GetDataSizeByFormat(dstType);

        while (count--)
        {
            if (srcType == VX_TYPE_FLOAT32 &&
                dstType == VX_TYPE_FLOAT16)
            {
                *(vx_uint16*)dst = F32TOF16(*(vx_float32*)src);
            }
            else if (srcType == VX_TYPE_FLOAT16 &&
                     dstType == VX_TYPE_FLOAT32)
            {
                *(vx_float32*)dst = F16TOF32(*(vx_uint16*)src);
            }
            else
            {
                gcmFOOTER_NO();
                return VX_ERROR_NOT_SUPPORTED;
            }
            src += ssize;
            dst += dsize;
        }
    }
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

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
    )
{
    vx_uint32 max = curDim, stack[VX_CONTEXT_TENSOR_MAX_DIMENSION+1] = {0};
    vx_uint8_ptr srcs[VX_CONTEXT_TENSOR_MAX_DIMENSION], dsts[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_bool back = vx_false_e;

    gcmHEADER_ARG("src=%p, dst=%p, curDim=0x%x, sizes=%p, srcStrides=%p, dstStrides=%p, srcType=0x%x, dstType=0x%x", src, dst, curDim, sizes, srcStrides, dstStrides, srcType, dstType);
    for (;;)
    {
        srcs[curDim] = src;
        dsts[curDim] = dst;

        if (!curDim)
        {
            vxoTensor_LineCopyConvert(
                src, dst,
                srcType, dstType,
                sizes[curDim]
                );
            stack[++curDim]++;
            if (curDim > max) break;
            back = vx_true_e;
        }
        else if (!back)
        {
            curDim--;
        }
        else
        {
            if (stack[curDim] < sizes[curDim])
            {
                src += srcStrides[curDim];
                dst += dstStrides[curDim];
                back = vx_false_e;
            }
            else
            {
                stack[curDim] = 0;
                stack[++curDim]++;
                if (curDim > max) break;
                src = srcs[curDim];
                dst = dsts[curDim];
            }
        }
    }
    gcmFOOTER_NO();
}

VX_PRIVATE_API vx_status
vxoTensor_CopyTensorPatch(
    vx_tensor tensor,
    vx_tensor_view view,
    vx_tensor_addressing user_addr,
    void *user_ptr,
    vx_enum usage
    )
{
    vx_uint32 usrElemSize, totalSize;
    vx_tensor_buffer_s * dim;
    gctPOINTER logical;
    gcmHEADER_ARG("tensor=%p, view=%p, user_addr=%p, user_ptr=%p, usage=0x%x", tensor, view, user_addr, user_ptr, usage);

    usrElemSize = user_addr->dimStridesUser[0];
    dim = tensor->tensorBuffer;

    vxoTensor_GetTensorBaseMemory(tensor, (vx_uint8_ptr*)&logical, VX_NULL);

    if (view == VX_NULL &&
        !tensor->isViewed &&
        usrElemSize == dim->elementSize &&
        vxoTensor_ViewRegionIsEqualAddressing(&tensor->viewRegion, user_addr))
    {
        /* Quick path for full tensor copy */
        vxoTensor_GetTensorSize(tensor, &totalSize);

        if (usage == VX_READ_ONLY)
        {
            vxMemCopy(user_ptr, logical, totalSize);
        }
        else if (usage == VX_WRITE_ONLY)
        {
            vxMemCopy(logical, user_ptr, totalSize);
        }
        else
        {
            gcmFOOTER_NO();
            return VX_ERROR_NOT_SUPPORTED;
        }
    }
    else
    {
        /* Complicated path for partial tensor copy */
        vx_uint32 offset;
        vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];
        vx_uint8_ptr tensorLogic;
        vx_tensor_view_s viewMerged;
        vx_enum usrDataFormat;

        /* TODO: User data format is not defined in API yet.
         *       Initialize it to tensor format if same size.
         *       Otherwise init it to float32 or float16 first.
         */
        if (usrElemSize == dim->elementSize) usrDataFormat = TENSOR_DATA_TYPE(tensor);
        else if (usrElemSize == 4) usrDataFormat = VX_TYPE_FLOAT32;
        else if (usrElemSize == 2) usrDataFormat = VX_TYPE_FLOAT16;
        else
        {
            gcmFOOTER_NO();
            return VX_ERROR_NOT_SUPPORTED;
        }
        if (view != VX_NULL)
        {
            vxoTensor_MergeTwoViews(&tensor->viewRegion, &view->viewRegion, &viewMerged.viewRegion);
            offset = vxoTensor_CalculateDimOffsetByStarts(tensor, viewMerged.viewRegion.viewStarts);
            vxoTensor_CalculateSizesFromViewRegion(&viewMerged.viewRegion, sizes);
        }
        else
        {
            offset = vxoTensor_CalculateDimOffsetByStarts(tensor, TENSOR_VIEW_STARTS(tensor));
            vxoTensor_CalculateSizesFromViewRegion(&tensor->viewRegion, sizes);
        }

        tensorLogic = (vx_uint8_ptr)logical + offset;

        if (usage == VX_READ_ONLY)
        {
            vxoTensor_CopyTensorPatchEx(
                tensorLogic,
                (vx_uint8_ptr)user_ptr,
                TENSOR_DIM_NUM(tensor) - 1,
                sizes,
                (vx_uint32*)TENSOR_STRIDES(tensor),
                user_addr->dimStridesUser,
                TENSOR_DATA_TYPE(tensor),
                usrDataFormat
                );
        }
        else if (usage == VX_WRITE_ONLY)
        {
            vxoTensor_CopyTensorPatchEx(
                (vx_uint8_ptr)user_ptr,
                tensorLogic,
                TENSOR_DIM_NUM(tensor) - 1,
                sizes,
                user_addr->dimStridesUser,
                (vx_uint32*)TENSOR_STRIDES(tensor),
                usrDataFormat,
                TENSOR_DATA_TYPE(tensor)
                );
        }
        else
        {
            gcmFOOTER_NO();
            return VX_ERROR_NOT_SUPPORTED;
        }
    }

    gcmFOOTER_NO();
    return VX_SUCCESS;
}

/* vxoTensor_CreateImageArray_11 is for back compatibility with spec 1.1*/
VX_PRIVATE_API vx_object_array
vxoTensor_CreateImageArray_11(
    vx_tensor tensor,
    vx_rectangle_t rect,
    vx_uint32 array_size,
    vx_uint32 stride,
    vx_df_image image_format
    )
{
    vx_context context;
    vx_uint32 i, width, height, offset, d2stride;
    vx_uint8 *logical;
    vx_uint32 physical;
    vx_object_array imageArray = VX_NULL;
    vx_imagepatch_addressing_t addr;
    vx_image image;
    vx_bool finished = vx_false_e;

    gcmHEADER_ARG("tensor=%p, rect=%p, array_size=0x%x, stride=0x%x, image_format=0x%x", tensor, rect, array_size, stride, image_format);

    context = GET_CONTEXT(tensor);
    if (!vxoContext_IsValid(context)) goto exit;

    if (vxoTensor_AllocateMemory(tensor) != VX_SUCCESS) goto exit;

    /* Stride must be equal to one image size. */
    d2stride = TENSOR_STRIDE_INDEX(tensor, 2);
    if (stride != d2stride) goto exit;

    width = rect.end_x - rect.start_x;
    height = rect.end_y - rect.start_y;

    imageArray = vxoOA_CreateObjectArrayEmpty((vx_reference)context, VX_TYPE_IMAGE, array_size);
    if (vxoReference_GetStatus((vx_reference) imageArray) != VX_SUCCESS) goto exit;
    imageArray->base.scope = (vx_reference)tensor;

    if (vxoTensor_GetTensorBaseMemory(tensor, &logical, &physical) != VX_SUCCESS) goto exit;
    offset = vxoTensor_CalculateDimOffsetByStarts(tensor, TENSOR_VIEW_STARTS(tensor));
    logical += offset;
    physical += offset;

    addr.dim_x = width;
    addr.dim_y = height;
    addr.stride_x = TENSOR_STRIDE_INDEX(tensor, 0);
    addr.stride_y = TENSOR_STRIDE_INDEX(tensor, 1);

    for (i=0; i < array_size; i++)
    {
        /* One plane image. */
        image = vxoImage_CreateImageFromInternalHandle(context, image_format, &addr, (void**)&logical, &physical);
        if (vxoReference_GetStatus((vx_reference) image) != VX_SUCCESS) break;
        if (!vxoOA_SetObjectArrayItem(imageArray, (vx_reference) image))
        {
            vxReleaseImage(&image);
            break;
        }

        logical += d2stride;
        physical += d2stride;
    }

    if (i < array_size)
    {
        vxReleaseObjectArray(&imageArray);
        goto exit;
    }

    finished = vx_true_e;

exit:
    if (!finished)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    gcmFOOTER_NO();
    return imageArray;
}

VX_PRIVATE_API vx_object_array
vxoTensor_CreateImageArray(
    vx_tensor tensor,
    const vx_rectangle_t * rect,
    vx_uint32 array_size,
    vx_uint32 stride,
    vx_df_image image_format
    )
{
    vx_context context;
    vx_uint32 i, width, height, offset, d2stride;
    vx_uint8 *logical;
    vx_uint32 physical;
    vx_object_array imageArray = VX_NULL;
    vx_imagepatch_addressing_t addr;
    vx_image image;
    vx_bool finished = vx_false_e;

    gcmHEADER_ARG("tensor=%p, rect=%p, array_size=0x%x, stride=0x%x, image_format=0x%x", tensor, rect, array_size, stride, image_format);

    context = GET_CONTEXT(tensor);
    if (!vxoContext_IsValid(context)) goto exit;

    if (vxoTensor_AllocateMemory(tensor) != VX_SUCCESS) goto exit;

    /* Stride must be equal to one image size. */
    d2stride = TENSOR_STRIDE_INDEX(tensor, 2);
    if (stride != d2stride) goto exit;

    width = rect->end_x - rect->start_x;
    height = rect->end_y - rect->start_y;

    imageArray = vxoOA_CreateObjectArrayEmpty((vx_reference)context, VX_TYPE_IMAGE, array_size);
    if (vxoReference_GetStatus((vx_reference) imageArray) != VX_SUCCESS) goto exit;
    imageArray->base.scope = (vx_reference)tensor;

    if (vxoTensor_GetTensorBaseMemory(tensor, &logical, &physical) != VX_SUCCESS) goto exit;
    offset = vxoTensor_CalculateDimOffsetByStarts(tensor, TENSOR_VIEW_STARTS(tensor));
    logical += offset;
    physical += offset;

    addr.dim_x = width;
    addr.dim_y = height;
    addr.stride_x = TENSOR_STRIDE_INDEX(tensor, 0);
    addr.stride_y = TENSOR_STRIDE_INDEX(tensor, 1);

    for (i=0; i < array_size; i++)
    {
        /* One plane image. */
        image = vxoImage_CreateImageFromInternalHandle(context, image_format, &addr, (void**)&logical, &physical);
        if (vxoReference_GetStatus((vx_reference) image) != VX_SUCCESS) break;
        if (!vxoOA_SetObjectArrayItem(imageArray, (vx_reference) image))
        {
            vxReleaseImage(&image);
            break;
        }

        logical += d2stride;
        physical += d2stride;
    }

    if (i < array_size)
    {
        vxReleaseObjectArray(&imageArray);
        goto exit;
    }

    finished = vx_true_e;

exit:
    if (!finished)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    gcmFOOTER_NO();
    return imageArray;
}

VX_INTERNAL_API vx_image
vxoTensor_CreateImageFromTensor(
    vx_tensor tensor,
    vx_uint32 width,
    vx_uint32 height,
    vx_df_image format
    )
{
    vx_context context;
    vx_uint32 offset;
    vx_uint8 *logical;
    vx_uint32 physical;
    vx_imagepatch_addressing_t addr;
    vx_image image;
    gcmHEADER_ARG("tensor=%p, width=0x%x, height=0x%x, format=0x%x", tensor, width, height, format);

    context = GET_CONTEXT(tensor);
    if (!vxoContext_IsValid(context)) goto exit;

    if (vxoTensor_AllocateMemory(tensor) != VX_SUCCESS) goto exit;

    if (vxoTensor_GetTensorBaseMemory(tensor, &logical, &physical) != VX_SUCCESS) goto exit;
    offset = vxoTensor_CalculateDimOffsetByStarts(tensor, TENSOR_VIEW_STARTS(tensor));
    logical += offset;
    physical += offset;

    addr.dim_x = width;
    addr.dim_y = height;
    addr.stride_x = TENSOR_STRIDE_INDEX(tensor, 0);
    /*addr.stride_y = TENSOR_STRIDE_INDEX(tensor, 1);*/

    addr.stride_y = width * TENSOR_STRIDE_INDEX(tensor, 0);

    {
        /* One plane image. */
        image = vxoImage_CreateImageFromInternalHandle(context, format, &addr, (void**)&logical, &physical);
        if (vxoReference_GetStatus((vx_reference) image) != VX_SUCCESS) goto exit;
    }
    gcmFOOTER_NO();
    return image;

exit:
    gcmFOOTER_NO();
    return VX_NULL;
}

VX_PRIVATE_API vx_bool
vxoTensor_FillDimBuffer(
    vx_tensor  tensor,
    vx_int32*  dbuff,
    vx_uint32  bufsize,
    vx_uint32* dims
    )
{
    vx_uint32 dsize, i;
    vx_uint64 omul = 1;
    vx_bool contiguous;

    gcmHEADER_ARG("tensor=%p, dbuff=%p, bufsize=0x%x, dims=%p", tensor, dbuff, bufsize, dims);

    dsize = TENSOR_DIM_NUM(tensor);

    for (i = 0; i < dsize; i++)
    {
        omul *= TENSOR_VIEW_SIZE_INDEX(tensor, i);
    }

    contiguous = vxoTensor_IsContiguousMemory(tensor);

    /* only support tensor with contiguous memory to reshape now */
    if ((vx_int32)bufsize > 0 && !contiguous && dsize != bufsize){
        gcmFOOTER_NO();
        return vx_false_e;
    }

    if ((vx_int32)bufsize == -1 || (bufsize == 1 && dbuff[0] == -1))
    {
        dims[0] = (vx_uint32)omul;
    }
    else
    {
        vx_uint64 nmul = 1;
        vx_int32 pos = -1;

        for (i = 0; i < bufsize; i++)
        {
            if (dbuff[i] == -1)
            {
                if (pos >= 0)
                {
                    gcmFOOTER_NO();
                    return vx_false_e;
                }
                pos = i;
            }
            else if (dbuff[i] == 0)
            {
                if (i >= dsize)
                {
                    gcmFOOTER_NO();
                    return vx_false_e;
                }
                dims[i] = TENSOR_VIEW_SIZE_INDEX(tensor, i);
                nmul *= dims[i];
            }
            else
            {
                dims[i] = dbuff[i];
                nmul *=  dbuff[i];
            }
        }

        if (omul % nmul)
        {
            gcmFOOTER_NO();
            return vx_false_e;
        }
        if (pos >= 0)
        {
            dims[pos] = (vx_uint32)(omul / nmul);
        }
    }
    gcmFOOTER_NO();
    return vx_true_e;
}

/*****************************/
/* vxCreateTensor_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateTensor_11(
    vx_context context,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_int8 fixed_point_pos
    )
{
    vx_tensor tensor;
    vx_tensor_create_params_t tensor_create_params;

    gcmHEADER_ARG("context=%p, num_of_dims=0x%x, sizes=%p, data_format=0x%x, fixed_point_pos=0x%x",
        context, num_of_dims, sizes, data_format, fixed_point_pos);
    gcmDUMP_API("$VX vxCreateTensor_11: context=%p, num_of_dims=0x%x, sizes=%p, data_format=0x%x, fixed_point_pos=0x%x",
        context, num_of_dims, sizes, data_format, fixed_point_pos);

    vxmASSERT(sizes);

    if (!vxoContext_IsValid(context))
    {
        vxError("%s[%d]: Context is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (num_of_dims > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("%s[%d]: The tensor view dim num %d is out of range!\n", __FUNCTION__, __LINE__, num_of_dims);
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_DIMENSION, "%s[%d]: The tensor view dim num %d is out of range!\n", __FUNCTION__, __LINE__, num_of_dims);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (!vxoTensor_DataFormatIsSupported(data_format))
    {
        vxError("%s[%d]: The tensor does not support data format %d", __FUNCTION__, __LINE__, data_format);
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_FORMAT, "%s[%d]: The tensor does not support data format %d", __FUNCTION__, __LINE__, data_format);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = num_of_dims;
    tensor_create_params.sizes = sizes;
    tensor_create_params.data_format = data_format;
    tensor_create_params.quant_format = (data_format == VX_TYPE_FLOAT16) ? 0 : VX_QUANT_DYNAMIC_FIXED_POINT;
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = fixed_point_pos;
    }

    tensor = vxoTensor_CreateTensorEx(
                context,
                VX_NULL,
                &tensor_create_params,
                vx_false_e
                );

    gcmFOOTER_NO();
    return tensor;
}

/* vxCreateTensorForNN11 is for back compatibility with spec 1.1, which is used in nn*/
VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateTensorForNN11(
    vx_context context,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_int8 fixed_point_pos
    )
{
    vx_tensor tensor;
    vx_tensor_create_params_t tensor_create_params;

    gcmHEADER_ARG("context=%p, num_of_dims=0x%x, sizes=%p, data_format=0x%x, fixed_point_pos=0x%x",
        context, num_of_dims, sizes, data_format, fixed_point_pos);
    gcmDUMP_API("$VX vxCreateTensorForNN11: context=%p, num_of_dims=0x%x, sizes=%p, data_format=0x%x, fixed_point_pos=0x%x",
        context, num_of_dims, sizes, data_format, fixed_point_pos);

    vxmASSERT(sizes);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (num_of_dims > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor view dim num %d is out of range", num_of_dims);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (!vxoTensor_DataFormatIsSupported(data_format))
    {
        vxError("The tensor does not support data format %d", data_format);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = num_of_dims;
    tensor_create_params.sizes = sizes;
    tensor_create_params.data_format = data_format;
    tensor_create_params.quant_format = (data_format == VX_TYPE_FLOAT16) ? 0 : VX_QUANT_DYNAMIC_FIXED_POINT;
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = fixed_point_pos;
    }

    tensor = vxoTensor_CreateTensorEx(
                context,
                VX_NULL,
                &tensor_create_params,
                vx_false_e
                );
    gcmFOOTER_NO();
    return tensor;
}

VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateTensor(
    vx_context context,
    vx_size number_of_dims,
    const vx_size * dims,
    vx_enum data_type,
    vx_int8 fixed_point_position)
{
    vx_tensor tensor;
    vx_uint32 num_of_dims;
    vx_uint32 *dimensions = NULL;
    vx_uint32 i;
    vx_tensor_create_params_t tensor_create_params;

    gcmHEADER_ARG("context=%p, number_of_dims=0x%lx, dims=%p, data_type=0x%x, fixed_point_position=0x%x",
        context, number_of_dims, dims, data_type, fixed_point_position);
    gcmDUMP_API("$VX vxCreateTensor: context=%p, number_of_dims=0x%lx, dims=%p, data_type=0x%x, fixed_point_position=0x%x",
        context, number_of_dims, dims, data_type, fixed_point_position);

    vxmASSERT(dims);

    if (!vxoContext_IsValid(context)) {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    gcmSAFECASTSIZET(num_of_dims, number_of_dims);
    if (num_of_dims > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor view dim num %d is out of range", num_of_dims);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (!vxoTensor_DataFormatIsSupported(data_type))
    {
        vxError("The tensor does not support data format %d", data_type);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    dimensions = (vx_uint32 *)vxAllocate(sizeof(vx_uint32) * num_of_dims);
    for (i = 0; i < num_of_dims; i++)
    {
        vx_uint32 dimVal_uint32;
        vx_size dimVal_vsize = dims[i];

        gcmSAFECASTSIZET(dimVal_uint32, dimVal_vsize);

        dimensions[i] = dimVal_uint32;
    }

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = (vx_uint32)number_of_dims;
    tensor_create_params.sizes = dimensions;
    tensor_create_params.data_format = data_type;
    tensor_create_params.quant_format = (data_type == VX_TYPE_FLOAT16) ? 0 : VX_QUANT_DYNAMIC_FIXED_POINT;
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = fixed_point_position;
    }

    tensor = vxoTensor_CreateTensorEx(
                context,
                VX_NULL,
                &tensor_create_params,
                vx_false_e
                );
    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS) {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    vxFree(dimensions);
    gcmFOOTER_NO();
    return tensor;
}

VX_PRIVATE_API vx_tensor
_CreateTensor2(
    vx_context context,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_size size_of_create_params,
    vx_reference_kind_e kind
    )
{
    vx_tensor tensor;
    gcmHEADER_ARG("context=%p, tensor_create_params=%p, size_of_create_params=0x%lx, kind=0x%x", context, tensor_create_params, size_of_create_params, kind);

    if (!vxoContext_IsValid(context)) {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (size_of_create_params != sizeof(vx_tensor_create_params_t))
    {
        vxError(" size_of_create_params doesn't match");
        gcmFOOTER_NO();
        return NULL;
    }

    vxmASSERT(tensor_create_params->sizes);

    if (tensor_create_params->num_of_dims > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor view dim num %d is out of range", tensor_create_params->num_of_dims);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (!vxoTensor_DataFormatIsSupported(tensor_create_params->data_format))
    {
        vxError("The tensor does not support data format %d", tensor_create_params->data_format);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (kind == VX_REF_EXTERNAL)
    {
        tensor = vxoTensor_CreateTensorEx(
                    context,
                    VX_NULL,
                    tensor_create_params,
                    vx_false_e
                    );
    }
    else
    {
        tensor = vxoTensor_CreateTensor(
                    context,
                    VX_NULL,
                    tensor_create_params,
                    vx_false_e
                    );
    }
    gcmFOOTER_NO();
    return tensor;
}

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensor2(
    vx_context context,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_size size_of_create_params
    )
{
    return _CreateTensor2(context, tensor_create_params, size_of_create_params, VX_REF_INTERNAL);
}

VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateTensor2(
    vx_context context,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_size size_of_create_params
    )
{
    gcmDUMP_API("$VX vxCreateTensor2: context=%p, tensor_create_params=%p, size_of_create_params=0x%lx",
        context, tensor_create_params, size_of_create_params);

    return _CreateTensor2(context, tensor_create_params, size_of_create_params, VX_REF_EXTERNAL);
}

VX_PRIVATE_API vx_tensor VX_API_CALL
_CreateVirtualTensor(
    vx_graph graph,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_int8 fixed_point_pos,
    vx_reference_kind_e kind
    )
{
    vx_context context;
    vx_tensor tensor;

    vx_tensor_create_params_t tensor_create_params;

    gcmHEADER_ARG("graph=%p, num_of_dims=0x%x, sizes=%p, data_format=0x%x, fixed_point_pos=0x%x, kind=0x%x", graph, num_of_dims, sizes, data_format, fixed_point_pos, kind);

    vxmASSERT(sizes);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    context = GET_CONTEXT(graph);

    if (!vxoContext_IsValid(context)) {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (num_of_dims > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor view dim num %d is out of range", num_of_dims);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (!vxoTensor_DataFormatIsSupported(data_format))
    {
        vxError("The tensor does not support data format %d", data_format);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = num_of_dims;
    tensor_create_params.sizes = sizes;
    tensor_create_params.data_format = data_format;
    tensor_create_params.quant_format = (data_format == VX_TYPE_FLOAT16) ? 0 : VX_QUANT_DYNAMIC_FIXED_POINT;
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = fixed_point_pos;
    }

    if (kind == VX_REF_EXTERNAL)
    {
        tensor = vxoTensor_CreateTensorEx(
                    context,
                    graph,
                    &tensor_create_params,
                    vx_true_e
                    );
    }
    else
    {
        tensor = vxoTensor_CreateTensor(
                    context,
                    graph,
                    &tensor_create_params,
                    vx_true_e
                    );
    }
    gcmFOOTER_NO();
    return tensor;
}

VX_INTERNAL_API vx_tensor
vxoTensor_CreateVirtualTensor(
    vx_graph graph,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_int8 fixed_point_pos
    )
{
    return _CreateVirtualTensor(graph, num_of_dims, sizes, data_format, fixed_point_pos, VX_REF_INTERNAL);
}

/* vxCreateVirtualTensor_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateVirtualTensor_11(
    vx_graph graph,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_int8 fixed_point_pos
    )
{
    gcmDUMP_API("$VX vxCreateVirtualTensor_11: graph=%p, num_of_dims=0x%x, sizes=%p, data_format=0x%x, fixed_point_pos=0x%x",
        graph, num_of_dims, sizes, data_format, fixed_point_pos);

    return _CreateVirtualTensor(graph, num_of_dims, sizes, data_format, fixed_point_pos, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateVirtualTensor(
    vx_graph graph,
    vx_size number_of_dims,
    const vx_size *dims,
    vx_enum data_type,
    vx_int8 fixed_point_position)
{
    vx_context context;
    vx_tensor tensor;
    vx_uint32 num_of_dims, i;
    vx_uint32 *dimSizes;

    gcmHEADER_ARG("graph=%p, num_of_dims=0x%lx, dims=%p, data_type=0x%x, fixed_point_position=0x%x",
        graph, number_of_dims, dims, data_type, fixed_point_position);

    gcmDUMP_API("$VX vxCreateVirtualTensor: graph=%p, num_of_dims=0x%lx, dims=%p, data_type=0x%x, fixed_point_position=0x%x",
        graph, number_of_dims, dims, data_type, fixed_point_position);

    vxmASSERT(dims);

    gcmSAFECASTSIZET(num_of_dims, number_of_dims);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    context = GET_CONTEXT(graph);

    if (!vxoContext_IsValid(context)) {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (num_of_dims > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor view dim num %d is out of range", num_of_dims);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (!vxoTensor_DataFormatIsSupported(data_type))
    {
        vxError("The tensor does not support data format %d", data_type);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    dimSizes = (vx_uint32 *)vxAllocate(sizeof(vx_uint32) * num_of_dims);
    for (i = 0; i < num_of_dims; i++)
    {
        vx_uint32 simSizeVal_uint32;
        vx_size dimSizeVal_vsize = dims[i];

        gcmSAFECASTSIZET(simSizeVal_uint32, dimSizeVal_vsize);

        dimSizes[i] = simSizeVal_uint32;
    }

    tensor = _CreateVirtualTensor(
                graph,
                num_of_dims,
                dimSizes,
                data_type,
                fixed_point_position,
                VX_REF_EXTERNAL
                );

    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    tensor->base.scope = (vx_reference)graph;

    vxFree(dimSizes);

    gcmFOOTER_NO();
    return tensor;
}

VX_PRIVATE_API vx_tensor VX_API_CALL
_CreateVirtualTensor2(
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_size size_of_create_params,
    vx_reference_kind_e kind
    )
{
    vx_tensor tensor;

    gcmHEADER_ARG("graph=%p, tensor_create_params=%p, size_of_create_params=0x%lx, kind=0x%x", graph, tensor_create_params, size_of_create_params, kind);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (size_of_create_params != sizeof(vx_tensor_create_params_t))
    {
        vxError(" size_of_create_params doesn't match");
        gcmFOOTER_NO();
        return NULL;
    }

    vxmASSERT(tensor_create_params->sizes);

    if (tensor_create_params->num_of_dims > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor view dim num %d is out of range", tensor_create_params->num_of_dims);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (!vxoTensor_DataFormatIsSupported(tensor_create_params->data_format))
    {
        vxError("The tensor does not support data format %d", tensor_create_params->data_format);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (kind == VX_REF_EXTERNAL)
    {
        tensor = vxoTensor_CreateTensorEx(
                    graph->base.context,
                    graph,
                    tensor_create_params,
                    vx_true_e
                    );
    }
    else
    {
        tensor = vxoTensor_CreateTensor(
                    graph->base.context,
                    graph,
                    tensor_create_params,
                    vx_true_e
                    );
    }

    gcmFOOTER_NO();
    return tensor;
}

VX_INTERNAL_API vx_tensor
vxoTensor_CreateVirtualTensor2(
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_size size_of_create_params
    )
{
    return _CreateVirtualTensor2(graph, tensor_create_params, size_of_create_params, VX_REF_INTERNAL);
}

VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateVirtualTensor2(
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_size size_of_create_params
    )
{
    gcmDUMP_API("$VX vxCreateVirtualTensor2: graph=%p, tensor_create_params=%p, size_of_create_params=0x%lx",
        graph, tensor_create_params, size_of_create_params);

    return _CreateVirtualTensor2(graph, tensor_create_params, size_of_create_params, VX_REF_EXTERNAL);
}

VX_PRIVATE_API vx_tensor VX_API_CALL
_CreateTensorFromView(
    vx_tensor tensor,
    vx_tensor_view view,
    vx_reference_kind_e kind
    )
{
    vx_tensor childTensor;
    vx_context context;
    vx_tensor_view_s viewMerged;
    vx_tensor_create_params_t tensor_create_params;

    gcmHEADER_ARG("tensor=%p, view=%p, kind=0x%x", tensor, view, kind);

    if (!vxoTensor_CheckValidTensorView(tensor, view))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (!vxoTensor_ComputeViewOffset(tensor, view))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    context = GET_CONTEXT(tensor);
    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    tensor_create_params.num_of_dims = TENSOR_DIM_NUM(tensor);
    tensor_create_params.sizes = TENSOR_SIZES(tensor);
    tensor_create_params.data_format = TENSOR_DATA_TYPE(tensor);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(tensor);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(tensor);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(tensor);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(tensor);
    }

    vxoTensor_MergeTwoViews(&tensor->viewRegion, &view->viewRegion, &viewMerged.viewRegion);

    childTensor = vxoTensor_Create(
                    context,
                    &tensor_create_params,
                    tensor->strides,
                    &viewMerged.viewRegion,
                    tensor->tensorBuffer,
                    tensor->baseAddressOffset,
                    tensor->isVirtual ? VX_TENSOR_SHARED | VX_TENSOR_VIRTUAL : VX_TENSOR_SHARED,
                    kind
                    );

    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }

    gcmFOOTER_NO();
    return childTensor;
}

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensorFromView(
    vx_tensor tensor,
    vx_tensor_view view
    )
{
    return _CreateTensorFromView(tensor, view, VX_REF_INTERNAL);
}

/* vxCreateTensorFromView_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateTensorFromView_11(
    vx_tensor tensor,
    vx_tensor_view view
    )
{
    gcmDUMP_API("$VX vxCreateTensorFromView_11: tensor=%p, view=%p", tensor, view);

    return _CreateTensorFromView(tensor, view, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateTensorFromView(
    vx_tensor tensor,
    vx_size num_of_dims,
    const vx_size * view_start,
    const vx_size * view_end)
{
    vx_tensor childTensor;
    vx_tensor_view view = NULL;
    vx_uint32 number_of_dims, i;
    vx_uint32 *view_array_start = NULL;
    vx_uint32 *view_array_end = NULL;

    gcmHEADER_ARG("tensor=%p, num_of_dims=0x%lx, view_start=%p, view_end=%p",
        tensor, num_of_dims, view_start, view_end);
    gcmDUMP_API("$VX vxCreateTensorFromView: tensor=%p, num_of_dims=0x%lx, view_start=%p, view_end=%p",
        tensor, num_of_dims, view_start, view_end);
    gcmSAFECASTSIZET(number_of_dims, num_of_dims);

    view_array_start = (vx_uint32 *)vxAllocate(sizeof(vx_uint32) * number_of_dims);
    view_array_end = (vx_uint32 *)vxAllocate(sizeof(vx_uint32) * number_of_dims);
    for (i = 0; i < number_of_dims; i++)
    {
        vx_uint32 startVal_uint32;
        vx_size startVal_vsize = view_start[i];
        vx_uint32 endVal_uint32;
        vx_size endVal_vsize = view_end[i];

        gcmSAFECASTSIZET(startVal_uint32, startVal_vsize);
        gcmSAFECASTSIZET(endVal_uint32, endVal_vsize);

        view_array_start[i] = startVal_uint32;
        view_array_end[i] = endVal_uint32;
    }
    view = vxCreateTensorView(tensor->base.context, view_array_start, view_array_end, (vx_uint8)number_of_dims);

    childTensor = _CreateTensorFromView(tensor, view, VX_REF_EXTERNAL);

    vxReleaseTensorView(&view);
    vxFree(view_array_start);
    vxFree(view_array_end);

    gcmFOOTER_NO();
    return childTensor;
}

VX_API_ENTRY vx_status VX_API_CALL
vxReleaseTensor(
    vx_tensor *tensor
    )
{
    gcmDUMP_API("$VX vxReleaseTensor: tensor=%p", tensor);
    return vxoTensor_ReleaseTensorEx(tensor);
}

VX_API_ENTRY vx_tensor_view VX_API_CALL
vxCreateTensorView(
    vx_context context,
    vx_uint32 *view_array_start,
    vx_uint32 *view_array_end,
    vx_uint8 numViewDimensions
    )
{
    vx_tensor_view view;
    gctUINT8 i;

    gcmHEADER_ARG("context=%p, view_array_start=%p, view_array_end=%p, numViewDimensions=0x%x",
        context, view_array_start, view_array_end, numViewDimensions);
    gcmDUMP_API("$VX vxCreateTensorView: context=%p, view_array_start=%p, view_array_end=%p, numViewDimensions=0x%x",
        context, view_array_start, view_array_end, numViewDimensions);

    vxmASSERT(view_array_start);
    vxmASSERT(view_array_end);

    if (!vxoContext_IsValid(context))
    {
        vxError("%s[%d]: Context is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (numViewDimensions > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("%s[%d]: The tensor view dim num %d is out of range!\n", __FUNCTION__, __LINE__, numViewDimensions);
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: The tensor view dim num %d is out of range!\n",
            __FUNCTION__, __LINE__, numViewDimensions);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    view = (vx_tensor_view)vxoReference_Create(context, VX_TYPE_TENSOR_VIEW, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)view) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get tensor_view reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&view->base, vxoReference_GetStatus((vx_reference)view), "%s[%d]: Get tensor_view reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_NULL;
    }
    for (i = 0; i < numViewDimensions; i++)
    {
        if (view_array_start[i] > view_array_end[i])
        {
            vxError("%s[%d]: The %dth of view array start %d is smaller than end %d!\n",
                     __FUNCTION__, __LINE__, i, view_array_start[i], view_array_end[i]);
            vxAddLogEntry(&view->base, VX_ERROR_INVALID_VALUE, "%s[%d]: The %dth of view array start %d is smaller than end %d!\n",
                __FUNCTION__, __LINE__, i, view_array_start[i], view_array_end[i]);
            gcmFOOTER_NO();
            return VX_NULL;
        }

        view->viewRegion.viewStarts[i] = view_array_start[i];
        view->viewRegion.viewEnds[i] = view_array_end[i];
    }

    view->viewRegion.dimCount = numViewDimensions;

    gcmFOOTER_NO();
    return view;
}

VX_API_ENTRY vx_status VX_API_CALL
vxReleaseTensorView(
    vx_tensor_view *tensor_view
    )
{
    gcmDUMP_API("$VX vxReleaseTensorView: tensor_view=%p", tensor_view);

    return vxoReference_Release((vx_reference_ptr)tensor_view, VX_TYPE_TENSOR_VIEW, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_tensor_addressing VX_API_CALL
vxCreateTensorAddressing(
    vx_context context,
    vx_uint32 *addressing_array_dimension,
    vx_uint32 *addressing_array_stride,
    vx_uint8 numViewDimensions
    )
{
    vx_tensor_addressing addressing;
    gctUINT8 i;

    gcmHEADER_ARG("context=%p, addressing_array_dimension=%p, addressing_array_stride=%p, numViewDimensions=0x%x",
        context, addressing_array_dimension, addressing_array_stride, numViewDimensions);
    gcmDUMP_API("$VX vxCreateTensorAddressing: context=%p, addressing_array_dimension=%p, addressing_array_stride=%p, numViewDimensions=0x%x",
        context, addressing_array_dimension, addressing_array_stride, numViewDimensions);

    vxmASSERT(addressing_array_dimension);
    vxmASSERT(addressing_array_stride);

    if (!vxoContext_IsValid(context))
    {
        vxError("%s[%d]: Context is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (numViewDimensions > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("%s[%d]: The tensor addressing dim num %d is out of range", __FUNCTION__, __LINE__, numViewDimensions);
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_DIMENSION, "%s[%d]: The tensor addressing dim num %d is out of range", __FUNCTION__, __LINE__, numViewDimensions);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    addressing = (vx_tensor_addressing)vxoReference_Create(context, VX_TYPE_TENSOR_ADDRESS, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)addressing) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get tensor_addressing reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&addressing->base, vxoReference_GetStatus((vx_reference)addressing), "%s[%d]: Get tensor_addressing reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    for (i = 0; i < numViewDimensions; i++)
    {
        addressing->dimSizesUser[i]   = addressing_array_dimension[i];
        addressing->dimStridesUser[i] = addressing_array_stride[i];
    }

    addressing->dimCount = numViewDimensions;

    gcmFOOTER_NO();
    return addressing;
}

VX_API_ENTRY vx_status VX_API_CALL
vxReleaseTensorAddressing(
    vx_tensor_addressing *tensor_addr
    )
{
    gcmDUMP_API("$VX vxReleaseTensorAddressing: tensor_addr=%p", tensor_addr);
    return vxoReference_Release((vx_reference_ptr)tensor_addr, VX_TYPE_TENSOR_ADDRESS, VX_REF_EXTERNAL);
}

/* vxCopyTensorPatch_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_status VX_API_CALL
vxCopyTensorPatch_11(
    vx_tensor tensor,
    vx_tensor_view view,
    vx_tensor_addressing user_addr,
    void *user_ptr,
    vx_enum usage,
    vx_enum user_mem_type
    )
{
    vx_status status;

    gcmHEADER_ARG("tensor=%p, view=%p, user_addr=%p, user_ptr=%p, usage=0x%x, user_mem_type=0x%x",
        tensor, view, user_addr, user_ptr, usage, user_mem_type);
    gcmDUMP_API("$VX vxCopyTensorPatch_11: tensor=%p, view=%p, user_addr=%p, user_ptr=%p, usage=0x%x, user_mem_type=0x%x",
        tensor, view, user_addr, user_ptr, usage, user_mem_type);

    if (view != VX_NULL)
    {
        if (!vxoTensor_CheckValidTensorView(tensor, view))
        {
            vxError("%s[%d]: TensorView is invalid!\n", __FUNCTION__, __LINE__);
            vxAddLogEntry(&tensor->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: TensorView is invalid!\n", __FUNCTION__, __LINE__);
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_REFERENCE;
        }

        if (!vxoTensor_CheckValidViewAddressing(view, user_addr))
        {
            vxError("%s[%d]: ViewAddressing is invalid!\n", __FUNCTION__, __LINE__);
            vxAddLogEntry(&view->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: ViewAddressing is invalid!\n", __FUNCTION__, __LINE__);
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        if (!vxoTensor_CheckValidTensorAddressing(tensor, user_addr))
        {
            vxError("%s[%d]: TensorAddressing is invalid!\n", __FUNCTION__, __LINE__);
            vxAddLogEntry(&tensor->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: TensorAddressing is invalid!\n", __FUNCTION__, __LINE__);
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_REFERENCE;
        }
    }

    if (tensor->isVirtual)
    {
        vxError("%s[%d]: The tensor is virtual tensor!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&tensor->base, VX_ERROR_OPTIMIZED_AWAY, "%s[%d]: The tensor is virtual tensor!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_ERROR_OPTIMIZED_AWAY;
    }

    if (user_ptr == VX_NULL)
    {
        vxError("%s[%d]: The user_ptr parameter is NULL!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&tensor->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: The user_ptr parameter is NULL!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (vxoTensor_AllocateMemory(tensor) != VX_SUCCESS)
    {
        vxError("%s[%d]: Allocate tensor memroy failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&tensor->base, VX_ERROR_NOT_ALLOCATED, "%s[%d]: Allocate tensor memroy failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_ERROR_NOT_ALLOCATED;
    }

    status = vxoTensor_CopyTensorPatch(tensor, view, user_addr, user_ptr, usage);

#if gcdDUMP
    if (usage == VX_WRITE_ONLY)
    {
        gctPOINTER logical;
        vx_uint32 size, physical;
        vxoTensor_GetTensorSize(tensor, &size);
        vxoTensor_GetTensorViewMemory(tensor, &logical, &physical);

        gcmDUMP(gcvNULL, "#[input]\n");
        gcmDUMP_BUFFER(gcvNULL,
                       gcvDUMP_BUFFER_MEMORY,
                       physical,
                       logical,
                       0,
                       size);
    }
#endif
    gcmFOOTER_ARG("%d", status);
    return status;
}

/* vxCopyTensorPatchForNN11 is for back compatibility with spec 1.1, which is used in nn*/
VX_API_ENTRY vx_status VX_API_CALL
vxCopyTensorPatchForNN11(
    vx_tensor tensor,
    vx_tensor_view view,
    vx_tensor_addressing user_addr,
    void *user_ptr,
    vx_enum usage,
    vx_enum user_mem_type
    )
{
    vx_status status;

    gcmHEADER_ARG(" tensor=%p, view=%p, user_addr=%p, user_ptr=%p, usage=0x%x, user_mem_type=0x%x",
        tensor, view, user_addr, user_ptr, usage, user_mem_type);
    gcmDUMP_API("$VX vxCopyTensorPatchForNN11: tensor=%p, view=%p, user_addr=%p, user_ptr=%p, usage=0x%x, user_mem_type=0x%x",
        tensor, view, user_addr, user_ptr, usage, user_mem_type);

    if (view != VX_NULL)
    {
        if (!vxoTensor_CheckValidTensorView(tensor, view))
        {
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_REFERENCE;
        }
        if (!vxoTensor_CheckValidViewAddressing(view, user_addr))
        {
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_REFERENCE;
        }
    }
    else
    {
        if (!vxoTensor_CheckValidTensorAddressing(tensor, user_addr))
        {
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_REFERENCE;
        }
    }

    if (tensor->isVirtual)
    {
        gcmFOOTER_NO();
        return VX_ERROR_OPTIMIZED_AWAY;
    }

    if (user_ptr == VX_NULL)
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (vxoTensor_AllocateMemory(tensor) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return VX_ERROR_NOT_ALLOCATED;
    }

    status = vxoTensor_CopyTensorPatch(tensor, view, user_addr, user_ptr, usage);

#if gcdDUMP
    if (usage == VX_WRITE_ONLY)
    {
        gctPOINTER logical;
        vx_uint32 size, physical;
        vxoTensor_GetTensorSize(tensor, &size);
        vxoTensor_GetTensorViewMemory(tensor, &logical, &physical);

        gcmDUMP(gcvNULL, "#[input]\n");
        gcmDUMP_BUFFER(gcvNULL,
                       gcvDUMP_BUFFER_MEMORY,
                       physical,
                       logical,
                       0,
                       size);
    }
#endif

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL
vxCopyTensorPatch(
    vx_tensor tensor,
    vx_size number_of_dims,
    const vx_size * view_start,
    const vx_size * view_end,
    const vx_size * user_stride,
    void * user_ptr,
    vx_enum usage,
    vx_enum user_memory_type)
{
    vx_status status = VX_FAILURE;
    vx_tensor_view view;
    vx_tensor_addressing user_addr = NULL;
    vx_uint32 *view_array_start = NULL;
    vx_uint32 *view_array_end = NULL;
    vx_uint32 *addressing_array_stride = NULL;
    vx_uint32 *addressing_array_dimension = NULL;
    vx_size i;
    vx_uint32 num_of_dims;

    gcmHEADER_ARG("tensor=%p, number_of_dims=0x%lx, view_start=%p, view_end=%p, user_stride=%p, user_ptr=%p, usage=0x%x,"\
        " user_memory_type=0x%x", tensor, number_of_dims, view_start, view_end, user_stride, user_ptr, usage, user_memory_type);

    gcmDUMP_API("$VX vxCopyTensorPatch: tensor=%p, number_of_dims=0x%lx, view_start=%p, view_end=%p, user_stride=%p, user_ptr=%p, usage=0x%x,"\
        " user_memory_type=0x%x", tensor, number_of_dims, view_start, view_end, user_stride, user_ptr, usage, user_memory_type);

    /* bad parameters */
    if ((view_start == NULL) || (view_end == NULL) || (user_stride == NULL) || (user_ptr == NULL))
    {
        gcmFOOTER_ARG("%d", status);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    /* bad references */
    if (vxoTensor_IsValidTensor(tensor) == vx_false_e)
    {
        gcmFOOTER_ARG("%d", status);
        return VX_ERROR_INVALID_REFERENCE;
    }

    /* determine if virtual before checking for memory */
    if (tensor->base.isVirtual == vx_true_e)
    {
        /* User tried to access a "virtual" tensor. */
        vxError("Can not access a virtual tensor\n");
        gcmFOOTER_ARG("%d", status);
        return VX_ERROR_OPTIMIZED_AWAY;
    }

    if (!vxoTensor_MemoryIsAllocated(tensor))
    {
        if (usage != VX_WRITE_ONLY || vxoTensor_AllocateMemory(tensor) != VX_SUCCESS)
        {
            vxError("Tensor memory was not allocated!\n");
            gcmFOOTER_ARG("%d", status);
            return VX_ERROR_NOT_ALLOCATED;
        }
    }

    gcmSAFECASTSIZET(num_of_dims, number_of_dims);
    if (tensor->dimCount < num_of_dims || num_of_dims ==0)
    {
        vxError("Invalid number of patch dimensions\n");
        gcmFOOTER_ARG("%d", status);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (vxoTensor_CheckTensorViewSizes(tensor->dims, view_start, view_end, num_of_dims) != 0)
    {
        vxError("Invalid view\n");
        gcmFOOTER_ARG("%d", status);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    view_array_start = (vx_uint32 *)vxAllocate(sizeof(vx_uint32) * num_of_dims);
    view_array_end = (vx_uint32 *)vxAllocate(sizeof(vx_uint32) * num_of_dims);
    addressing_array_dimension = (vx_uint32 *)vxAllocate(sizeof(vx_uint32) * num_of_dims);
    for (i = 0; i < num_of_dims; i++)
    {
        vx_uint32 startVal_uint32;
        vx_size startVal_vsize = view_start[i];
        vx_uint32 endVal_uint32;
        vx_size endVal_vsize = view_end[i];

        gcmSAFECASTSIZET(startVal_uint32, startVal_vsize);
        gcmSAFECASTSIZET(endVal_uint32, endVal_vsize);

        view_array_start[i] = startVal_uint32;
        view_array_end[i] = endVal_uint32;
        addressing_array_dimension[i] = endVal_uint32 - startVal_uint32;
    }

    addressing_array_stride = (vx_uint32 *)vxAllocate(sizeof(vx_uint32) * num_of_dims);
    for (i = 0; i < num_of_dims; i++)
    {
        vx_uint32 addressingStrideVal_uint32;
        vx_size addressingStrideVal_vsize = user_stride[i];

        gcmSAFECASTSIZET(addressingStrideVal_uint32, addressingStrideVal_vsize);

        addressing_array_stride[i] = addressingStrideVal_uint32;
    }
    user_addr = vxCreateTensorAddressing(tensor->base.context, addressing_array_dimension, addressing_array_stride, (vx_uint8)num_of_dims);


    view = vxCreateTensorView(tensor->base.context, view_array_start, view_array_end, (vx_uint8)num_of_dims);

    status = vxoCopyTensorPatch(
                tensor,
                view,
                user_addr,
                user_ptr,
                usage,
                user_memory_type
               );

    status |= vxReleaseTensorAddressing(&user_addr);
    status |= vxReleaseTensorView(&view);

    vxFree(view_array_start);
    vxFree(view_array_end);
    vxFree(addressing_array_stride);
    vxFree(addressing_array_dimension);

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status
vxoCopyTensorPatch(
    vx_tensor tensor,
    vx_tensor_view view,
    vx_tensor_addressing user_addr,
    void *user_ptr,
    vx_enum usage,
    vx_enum user_mem_type
    )
{
    vx_status status;

    if (view != VX_NULL)
    {
        if (!vxoTensor_CheckValidTensorView(tensor, view)) return VX_ERROR_INVALID_REFERENCE;
        if (!vxoTensor_CheckValidViewAddressing(view, user_addr)) return VX_ERROR_INVALID_REFERENCE;
    }
    else
    {
        if (!vxoTensor_CheckValidTensorAddressing(tensor, user_addr)) return VX_ERROR_INVALID_REFERENCE;
    }

    if (tensor->isVirtual) return VX_ERROR_OPTIMIZED_AWAY;

    if (user_ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    if (vxoTensor_AllocateMemory(tensor) != VX_SUCCESS) return VX_ERROR_NOT_ALLOCATED;

    status = vxoTensor_CopyTensorPatch(tensor, view, user_addr, user_ptr, usage);

#if gcdDUMP
    if (usage == VX_WRITE_ONLY)
    {
        gctPOINTER logical;
        vx_uint32 size, physical;
        vxoTensor_GetTensorSize(tensor, &size);
        vxoTensor_GetTensorViewMemory(tensor, &logical, &physical);

        gcmDUMP(gcvNULL, "#[input]\n");
        gcmDUMP_BUFFER(gcvNULL,
                       gcvDUMP_BUFFER_MEMORY,
                       physical,
                       logical,
                       0,
                       size);
    }
#endif

    return status;
}

/* vxCreateImageObjectArrayFromTensor_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_object_array VX_API_CALL
vxCreateImageObjectArrayFromTensor_11(
    vx_tensor tensor,
    vx_rectangle_t rect,
    vx_uint32 array_size,
    vx_uint32 stride,
    vx_df_image image_format
    )
{
    gcmDUMP_API("$VX vxCreateImageObjectArrayFromTensor_11: tensor=%p, rect=%p, array_size=0x%x, stride=0x%x, image_format=0x%x",
        tensor, rect, array_size, stride, image_format);

    if (!vxoTensor_IsValidTensor(tensor)) return VX_NULL;

    if (TENSOR_DIM_NUM(tensor) != 3 || !array_size) return VX_NULL;
    if (!vxoTensor_RectIsIn3DTensorRange_11(tensor, rect, stride, array_size)) return VX_NULL;
    if (!vxoTensor_ImageFormatIsSupported(image_format)) return VX_NULL;

    return vxoTensor_CreateImageArray_11(tensor, rect, array_size, stride, image_format);
}

VX_API_ENTRY vx_object_array VX_API_CALL
    vxCreateImageObjectArrayFromTensor(
    vx_tensor tensor,
    const vx_rectangle_t *rect,
    vx_size array_size,
    vx_size jump,
    vx_df_image image_format)
{
    vx_uint32 array_size_u32;
    vx_uint32 jump_u32;

    gcmDUMP_API("$VX vxCreateImageObjectArrayFromTensor: tensor=%p, rect=%p, array_size=0x%lx, jump=0x%lx, image_format=0x%x",
        tensor, rect, array_size, jump, image_format);

    gcmSAFECASTSIZET(array_size_u32, array_size);
    gcmSAFECASTSIZET(jump_u32, jump);

    if (!vxoTensor_IsValidTensor(tensor)) return VX_NULL;

    if (TENSOR_DIM_NUM(tensor) != 3 || !array_size) return VX_NULL;
    if (!vxoTensor_RectIsIn3DTensorRange(tensor, rect, jump_u32, array_size_u32)) return VX_NULL;
    if (!vxoTensor_ImageFormatIsSupported(image_format)) return VX_NULL;

    return vxoTensor_CreateImageArray(tensor, rect, array_size_u32, jump_u32, image_format);
}

VX_PRIVATE_API vx_tensor VX_API_CALL
_ReshapeTensor(
    vx_tensor    tensor,
    vx_int32*    num_of_dims,
    vx_uint32    sizes,
    vx_reference_kind_e kind
    )
{
    vx_uint32 newDims[VX_CONTEXT_TENSOR_MAX_DIMENSION], offset = 0;
    vx_context context = GET_CONTEXT(tensor);
    vx_tensor_create_params_t tensor_create_params;

    gcmHEADER_ARG("tensor=%p, num_of_dims=%p, sizes=0x%x, kind=0x%x", tensor, num_of_dims, sizes, kind);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (!vxoTensor_IsValidTensor(tensor))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (!vxoTensor_FillDimBuffer(tensor, num_of_dims, sizes, newDims))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }

    gcoOS_MemFill(&tensor_create_params, 0, sizeof(vx_tensor_create_params_t));
    tensor_create_params.num_of_dims = sizes;
    tensor_create_params.sizes = newDims;
    tensor_create_params.data_format = TENSOR_DATA_TYPE(tensor);
    tensor_create_params.quant_format = TENSOR_QUANT_TYPE(tensor);
    if (tensor_create_params.quant_format == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        tensor_create_params.quant_data.dfp.fixed_point_pos = TENSOR_POS(tensor);
    }
    else
    {
        tensor_create_params.quant_data.affine.scale = TENSOR_TF_SCALE(tensor);
        tensor_create_params.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(tensor);
    }

    if (tensor->isViewed)
    {
        offset = vxoTensor_CalculateDimOffsetByStarts(tensor, TENSOR_VIEW_STARTS(tensor));
    }

    gcmFOOTER_NO();
    return vxoTensor_Create(
                context,
                &tensor_create_params,
                VX_NULL,
                VX_NULL,
                tensor->tensorBuffer,
                tensor->baseAddressOffset + offset,
                tensor->isVirtual ? VX_TENSOR_SHARED | VX_TENSOR_VIRTUAL : VX_TENSOR_SHARED,
                kind
                );
}

VX_INTERNAL_API vx_tensor
vxoTensor_ReshapeTensor(
    vx_tensor    tensor,
    vx_int32*    num_of_dims,
    vx_uint32    sizes
    )
{
    return _ReshapeTensor(tensor, num_of_dims, sizes, VX_REF_INTERNAL);
}

VX_API_ENTRY vx_tensor VX_API_CALL
vxReshapeTensor(
    vx_tensor    tensor,
    vx_int32*    num_of_dims,
    vx_uint32    sizes
    )
{
    gcmDUMP_API("$VX vxReshapeTensor: tensor=%p, num_of_dims=%p, sizes=0x%x", tensor, num_of_dims, sizes);
    return _ReshapeTensor(tensor, num_of_dims, sizes, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL
vxQueryTensor(
    vx_tensor tensor,
    vx_enum attribute,
    void *ptr,
    vx_size size
    )
{
    gcmHEADER_ARG("tensor=%p, attribute=0x%x, ptr=%p, size=0x%lx", tensor, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryTensor: tensor=%p, attribute=0x%x, ptr=%p, size=0x%lx", tensor, attribute, ptr, size);

    if (!vxoTensor_IsValidTensor(tensor))
    {
        vxError("%s[%d]: Tensor is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    switch (attribute)
    {
        case VX_TENSOR_NUMBER_OF_DIMS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            *(vx_uint32 *)ptr = TENSOR_DIM_NUM(tensor);
            break;

        case VX_TENSOR_DIMS:
            if (size < sizeof(vx_uint32) * TENSOR_DIM_NUM(tensor) ||
                size > sizeof(vx_uint32) * VX_CONTEXT_TENSOR_MAX_DIMENSION)
            {
                vxError("%s[%d]: Tensor size is invalid!\n", __FUNCTION__, __LINE__);
                vxAddLogEntry(&tensor->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: Tensor size is invalid!\n", __FUNCTION__, __LINE__);
                gcmFOOTER_NO();
                return VX_ERROR_INVALID_PARAMETERS;
            }
            if (tensor->isViewed)
            {
                vx_uint32 i;

                for (i = 0; i < TENSOR_DIM_NUM(tensor); i++)
                {
                    *((vx_uint32*)ptr + i) = TENSOR_VIEW_SIZE_INDEX(tensor, i);
                }
            }
            else
            {
                vxMemCopy(ptr, TENSOR_SIZES(tensor), size);
            }
            break;

        case VX_TENSOR_DATA_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            *(vx_uint32 *)ptr = TENSOR_DATA_TYPE(tensor);
            break;

        case VX_TENSOR_FIXED_POINT_POSITION:
            vxmVALIDATE_PARAMETERS_EX(ptr, size, vx_uint8);
            *(vx_uint8 *)ptr = TENSOR_POS(tensor);
            break;

        case VX_TENSOR_QUANT_FORMAT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            *(vx_uint32 *)ptr = TENSOR_QUANT_TYPE(tensor);
            break;

        case VX_TENSOR_ZERO_POINT:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_int32, 0x3);
            *(vx_int32 *)ptr = TENSOR_TF_ZEROPOINT(tensor);
            break;

        case VX_TENSOR_SCALE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_float32, 0x3);
            *(vx_float32 *)ptr = TENSOR_TF_SCALE(tensor);
            break;
        case VX_TENSOR_RANK:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            *(vx_uint32 *)ptr = tensor->rank;
            break;
        case VX_TENSOR_PRECISION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            *(vx_uint32 *)ptr = tensor->tensorBuffer->precision;
            break;
        case VX_TENSOR_LIFETIME:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            *(vx_uint32 *)ptr = tensor->tensorBuffer->data_lifetime;
            break;
        case VX_TENSOR_VALUE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_bool, 0x1);
            *(vx_bool *)ptr = tensor->tensorBuffer->valued;
            break;
        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            vxAddLogEntry(&tensor->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: The attribute parameter, \
                %d, is not supported\n", __FUNCTION__, __LINE__, attribute);
            gcmFOOTER_NO();
            return VX_ERROR_INVALID_PARAMETERS;
    }
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

/*****************************/
VX_INTERNAL_CALLBACK_API void
vxoTensor_Destructor(
    vx_reference ref
    )
{
    vx_tensor tensor = (vx_tensor)ref;
    gcmHEADER_ARG("ref=%p", ref);

    if (tensor->tensorBuffer != VX_NULL &&
        tensor->tensorBuffer->bufRefCount &&
        !--tensor->tensorBuffer->bufRefCount)
    {
        vx_context context = GET_CONTEXT(tensor);
        context->allTensorNum--;

        vxoTensor_ReleaseMemory(tensor);

        vxFree(tensor->tensorBuffer);
        tensor->tensorBuffer = VX_NULL;
    }
    gcmFOOTER_NO();
}

/*****************************/
VX_INTERNAL_API vx_bool
vxoTensor_IsValidTensor(
    vx_tensor tensor
    )
{
    vx_uint32 i;

    if (tensor == VX_NULL) return vx_false_e;
    if (!vxoReference_IsValidAndSpecific(&tensor->base, VX_TYPE_TENSOR)) return vx_false_e;
    if (tensor->tensorBuffer == VX_NULL) return vx_false_e;

    if (!TENSOR_DIM_NUM(tensor)) return vx_false_e;
    if (TENSOR_DIM_NUM(tensor) != TENSOR_VIEW_DIM_NUM(tensor)) return vx_false_e;

    for (i = 0; i < TENSOR_DIM_NUM(tensor); i++)
    {
        if (!TENSOR_SIZE_INDEX(tensor, i) ||
            !TENSOR_STRIDE_INDEX(tensor, i))
            return vx_false_e;
    }

    return vxoTensor_ViewRegionIsValid(&tensor->viewRegion);
}

VX_INTERNAL_API vx_bool
vxoTensor_IsVirtualTensor(
    vx_tensor tensor
    )
{
    return tensor->isVirtual;
}

VX_INTERNAL_API vx_bool
vxoTensor_IsValidView(
    vx_tensor_view view
    )
{
    if (view == VX_NULL) return vx_false_e;
    if (!vxoReference_IsValidAndSpecific(&view->base, VX_TYPE_TENSOR_VIEW)) return vx_false_e;
    if (!view->viewRegion.dimCount) return vx_false_e;

    return vxoTensor_ViewRegionIsValid(&view->viewRegion);
}

VX_INTERNAL_API vx_bool
vxoTensor_IsValidAddressing(
    vx_tensor_addressing addressing
    )
{
    vx_uint32 i;
    gcmHEADER_ARG("addressing=%p", addressing);

    if (addressing == VX_NULL)
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }
    if (!vxoReference_IsValidAndSpecific(&addressing->base, VX_TYPE_TENSOR_ADDRESS))
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }
    if (!addressing->dimCount)
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }
    for (i = 0; i < addressing->dimCount; i++)
    {
        if (!addressing->dimSizesUser[i] ||
            !addressing->dimStridesUser[i])
        {
            gcmFOOTER_NO();
            return vx_false_e;
        }
    }
    gcmFOOTER_NO();
    return vx_true_e;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorViewMemory(
    vx_tensor tensor,
    gctPOINTER * logical,
    vx_uint32 * physical
    )
{
    vx_status status;
    gcmHEADER_ARG("tensor=%p, logical=%p, physical=%p", tensor, logical, physical);

    if (logical != VX_NULL) *logical = (vx_uint8_ptr)0;
    if (physical != VX_NULL) *physical = 0;

    status = vxoTensor_GetTensorBaseMemory(tensor, (vx_uint8_ptr*)logical, physical);
    if (status != VX_SUCCESS)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    if (tensor->isViewed)
    {
        vx_uint32 viewOffset = 0;

        if (tensor->viewOffset)
        {
            viewOffset = tensor->viewOffset;
        }
        else
        {
            vx_uint32 dimIndex = 0;

            for (dimIndex = 0; dimIndex < TENSOR_VIEW_DIM_NUM(tensor); dimIndex++)
            {
                viewOffset += TENSOR_VIEW_START_INDEX(tensor, dimIndex) * TENSOR_STRIDE_INDEX(tensor, dimIndex);
            }
            tensor->viewOffset = viewOffset;
        }

        if (logical) *logical = (vx_uint8_ptr)*logical + viewOffset;
        if (physical) *physical = (vx_uint32)*physical + viewOffset;
    }
    gcmFOOTER_ARG("%d", status);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorViewOffset(
    vx_tensor  tensor,
    vx_uint32* offset
    )
{
    vx_uint32 viewOffset = 0;
    gcmHEADER_ARG("tensor=%p, offset=%p", tensor, offset);
    vxmASSERT(offset);
    if (tensor->isViewed)
    {

        if (tensor->viewOffset)
        {
            viewOffset = tensor->viewOffset;
        }
        else
        {
            vx_uint32 dimIndex = 0;

            for (dimIndex = 0; dimIndex < TENSOR_VIEW_DIM_NUM(tensor); dimIndex++)
            {
                viewOffset += TENSOR_VIEW_START_INDEX(tensor, dimIndex) * TENSOR_STRIDE_INDEX(tensor, dimIndex);
            }
            tensor->viewOffset = viewOffset;
        }
    }

    *offset = viewOffset + tensor->baseAddressOffset;
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorBatchArrayViewMemory(
    vx_tensor tensor,
    vx_uint32 batchIndex,
    gctPOINTER * logical,
    vx_uint32 * physical
    )
{
    vx_status status;
    gcmHEADER_ARG("tensor=%p, batchIndex=0x%x, logical=%p, physical=%p", tensor, batchIndex, logical, physical);

    status = vxoTensor_GetTensorBatchArrayBaseMemory(tensor, batchIndex, logical, physical);
    if (status != VX_SUCCESS)
    {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    if (tensor->isViewed)
    {
        vx_uint32 dimIndex = 0;
        vx_uint32 viewOffset = 0;

        if (tensor->viewOffset)
        {
            viewOffset = tensor->viewOffset;
        }
        else
        {
            for (dimIndex = 0; dimIndex < TENSOR_VIEW_DIM_NUM(tensor); dimIndex++)
            {
                viewOffset += TENSOR_VIEW_START_INDEX(tensor, dimIndex) * TENSOR_STRIDE_INDEX(tensor, dimIndex);
            }
            tensor->viewOffset = viewOffset;
        }

        if (logical) *logical = (vx_uint8_ptr)*logical + viewOffset;
        if (physical) *physical = (vx_uint32)*physical + viewOffset;
    }
    gcmFOOTER_ARG("%d", status);
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorElementCount(
    vx_tensor tensor,
    vx_uint32 *count
    )
{
    gcmHEADER_ARG("tensor=%p, count=%p", tensor, count);
    if (!vxoTensor_IsValidTensor(tensor))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    if(count != VX_NULL)
    {
        vx_uint32 index;
        vx_uint32 elementCount = 1;

        for(index = 0; index < TENSOR_VIEW_DIM_NUM(tensor); index++)
        {
            elementCount *= TENSOR_VIEW_SIZE_INDEX(tensor, index);
        }

        *count = elementCount;
    }

    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorSize(
    vx_tensor tensor,
    vx_uint32 *size
    )
{
    vx_status status;
    vx_uint32 elementCount;
    gcmHEADER_ARG("tensor=%p, size=%p", tensor, size);

    status = vxoTensor_GetTensorElementCount(tensor, &elementCount);
    if (status != VX_SUCCESS) {
        gcmFOOTER_ARG("%d", status);
        return status;
    }
    if (size != VX_NULL)
    {
        *size =  elementCount * TENSOR_DATA_SIZE(tensor);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorWholeElementCount(
    vx_tensor tensor,
    vx_uint32 *count
    )
{
    gcmHEADER_ARG("tensor=%p, count=%p", tensor, count);
    if (!vxoTensor_IsValidTensor(tensor))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    if(count != VX_NULL)
    {
        vx_uint32 elementCount = 1;
        vx_uint32 dim_count = TENSOR_ORIG_DIM_NUM(tensor);

        if (tensor->tensorBuffer->memory.strides[0][dim_count] > 0)
            elementCount = tensor->tensorBuffer->memory.strides[0][dim_count];
        else
        {
            vx_uint32 index;
            for(index = 0; index < dim_count; index++)
            {
                elementCount *= tensor->tensorBuffer->memory.dims[0][index];
            }
        }

        *count = elementCount;
    }

    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorWholeSize(
    vx_tensor tensor,
    vx_size   *size
    )
{
    vx_status status;
    vx_uint32 elementCount;

    status = vxoTensor_GetTensorWholeElementCount(tensor, &elementCount);
    if (status != VX_SUCCESS) return status;

    if (size != VX_NULL)
    {
        *size = elementCount * TENSOR_DATA_SIZE(tensor);
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorDimStride(
    vx_tensor tensor,
    vx_uint32 * count,
    vx_uint32 * sizes,
    vx_uint32 * strides
    )
{
    vx_uint32 i;
    gcmHEADER_ARG("tensor=%p, count=%p, sizes=%p, strides=%p", tensor, count, sizes, strides);

    if (!vxoTensor_IsValidTensor(tensor))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!count || (sizes == VX_NULL && strides == VX_NULL))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (*count && *count > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (!*count)
    {
        *count = TENSOR_DIM_NUM(tensor);
    }

    if (sizes != VX_NULL)
    {
        if (tensor->isViewed)
        {
            for (i = 0; i < *count; i++)
            {
                sizes[i] = TENSOR_VIEW_SIZE_INDEX(tensor, i);
            }
        }
        else
        {
            vxMemCopy(sizes, TENSOR_SIZES(tensor), *count * sizeof(vx_uint32));
        }
    }

    if (strides != VX_NULL)
    {
        vxMemCopy(strides, TENSOR_STRIDES(tensor), *count * sizeof(vx_uint32));
    }

    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorViewRegion(
    vx_tensor tensor,
    vx_uint32 count,
    vx_uint32 * starts,
    vx_uint32 * ends
    )
{
    gcmHEADER_ARG("tensor=%p, count=0x%x, starts=%p, ends=%p", tensor, count, starts, ends);
    if (!vxoTensor_IsValidTensor(tensor))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!count || (starts == VX_NULL && ends == VX_NULL))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (count > TENSOR_VIEW_DIM_NUM(tensor))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (starts != VX_NULL)
    {
        vxMemCopy(starts, TENSOR_VIEW_STARTS(tensor), count * sizeof(vx_uint32));
    }
    if (ends != VX_NULL)
    {
        vxMemCopy(ends, TENSOR_VIEW_ENDS(tensor), count * sizeof(vx_uint32));
    }

    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_tensor
_CreateTensor(
    vx_context context,
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_bool is_virtual,
    vx_reference_kind_e kind
    )
{
    vx_tensor tensor;
    gcmHEADER_ARG("context=%p, graph=%p, tensor_create_params=%p, is_virtual=0x%x, kind=0x%x", context, graph, tensor_create_params, is_virtual, kind);

    tensor = vxoTensor_Create(
                context,
                tensor_create_params,
                VX_NULL,
                VX_NULL,
                VX_NULL,
                0,
                is_virtual ? VX_TENSOR_VIRTUAL : VX_TENSOR_NORMAL,
                kind
                );

    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS) {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (is_virtual)
    {
        tensor->base.scope = (vx_reference)graph;
        TENSOR_GRAPH(tensor) = graph;
        graph->virtTensorNum++;
    }
    gcmFOOTER_NO();
    return tensor;
}

vx_tensor
vxoTensor_CreateTensorExt(
    vx_context context,
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_uint32 * strides,
    vx_view_region_s * viewRegion,
    vx_tensor_buffer_s * tensorBuffer,
    vx_bool is_virtual,
    vx_reference_kind_e kind
)
{

    vx_tensor tensor;

    tensor = vxoTensor_Create(
        context,
        tensor_create_params,
        strides,
        viewRegion,
        tensorBuffer,
        0,
        is_virtual ? VX_TENSOR_VIRTUAL : VX_TENSOR_NORMAL,
        kind
    );

    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS) return VX_NULL;

    if (is_virtual)
    {
        tensor->base.scope = (vx_reference)graph;
        TENSOR_GRAPH(tensor) = graph;
        graph->virtTensorNum++;
    }

    return tensor;
}

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensor(
    vx_context context,
    vx_graph graph,
    const vx_tensor_create_params_t  *tensor_create_params,
    vx_bool is_virtual
    )
{
    return _CreateTensor(context, graph, tensor_create_params, is_virtual, VX_REF_INTERNAL);
}

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensorEx(
    vx_context context,
    vx_graph graph,
    const vx_tensor_create_params_t* tensor_create_params,
    vx_bool is_virtual
    )
{
    return _CreateTensor(context, graph, tensor_create_params, is_virtual, VX_REF_EXTERNAL);
}

VX_INTERNAL_API vx_status
vxoTensor_ReleaseTensor(
    vx_tensor *tensor
    )
{
    return vxoReference_Release((vx_reference_ptr)tensor, VX_TYPE_TENSOR, VX_REF_INTERNAL);
}

VX_INTERNAL_API vx_status
vxoTensor_ReleaseTensorEx(
    vx_tensor *tensor
    )
{
    return vxoReference_Release((vx_reference_ptr)tensor, VX_TYPE_TENSOR, VX_REF_EXTERNAL);
}

/*
 * Set tensor's memory size.
 * As tensor has only one plane, we just set sizes[0] of memory object.
 */
VX_INTERNAL_API vx_status
vxoTensor_SetMemorySize(
    vx_tensor tensor,
    vx_uint32 size
    )
{
    if (!vxoTensor_IsValidTensor(tensor))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    tensor->memorySize = size;

    tensor->tensorBuffer->memory.sizes[0] = tensor->memorySize;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status
_AllocateMemory(
    vx_tensor tensor,
    vx_bool   real
    )
{
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("tensor=%p, real=0x%x", tensor, real);

    if (!vxoTensor_IsValidTensor(tensor))
    {
        gcmFOOTER_ARG("%d", status);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (tensor->alloced)
    {
        gcmFOOTER_ARG("%d", status);
        return VX_SUCCESS;
    }
    if (vxoTensor_MemoryIsAllocated(tensor))
    {
        status = VX_SUCCESS;
    }
    else if (real)
    {
        if (!tensor->isVirtual &&
            !vxoMemory_Allocate(tensor->base.context, &tensor->tensorBuffer->memory))
        {
            status = VX_FAILURE;
        }
        else if (tensor->isVirtual &&
                 !vxoMemory_AllocateEx(tensor->base.context, &tensor->tensorBuffer->memory))
        {
            status = VX_FAILURE;
        }
    }
    else if (status == VX_SUCCESS)
    {
        tensor->tensorBuffer->memory.allocated = vx_true_e;
    }

    if (status == VX_SUCCESS) tensor->alloced = vx_true_e;

    /*tensor->tensorBuffer->valued = vx_true_e;*/
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status
vxoTensor_AllocateMemory(
    vx_tensor tensor
    )
{
    return _AllocateMemory(tensor, vx_true_e);
}

VX_PRIVATE_API vx_status
_ReleaseMemory(
    vx_tensor tensor,
    vx_bool real
    )
{
    vx_status status = VX_SUCCESS;
    gcmHEADER_ARG("tensor=%p, real=0x%x", tensor, real);

    if (!vxoTensor_IsValidTensor(tensor))
    {
        gcmFOOTER_ARG("%d", status);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!tensor->alloced)
    {
        gcmFOOTER_ARG("%d", status);
        return VX_SUCCESS;
    }
    if (real)
    {
        if (!tensor->isVirtual &&
            !vxoMemory_Free(tensor->base.context, &tensor->tensorBuffer->memory))
        {
            status = VX_FAILURE;
        }
        else if (tensor->isVirtual &&
                 !vxoMemory_FreeEx(tensor->base.context, &tensor->tensorBuffer->memory))
        {
            status = VX_FAILURE;
        }
    }
    else if (status == VX_SUCCESS)
    {
        tensor->tensorBuffer->memory.allocated = vx_false_e;
    }

    if (status == VX_SUCCESS)
    {
        tensor->alloced = vx_false_e;
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_status
vxoTensor_ReleaseMemory(
    vx_tensor tensor
    )
{
    return _ReleaseMemory(tensor, vx_true_e);
}

VX_INTERNAL_API vx_bool
vxoTensor_MemoryIsAllocated(
    vx_tensor tensor
    )
{
    return tensor->tensorBuffer->memory.allocated;
}

VX_INTERNAL_API vx_bool
vxoTensor_IsAllocated(
    vx_tensor tensor
    )
{
    return tensor->alloced;
}

VX_INTERNAL_API vx_bool
vxoTensor_IsOverlap(
    vx_tensor tensor1,
    vx_tensor tensor2
    )
{
    gcmHEADER_ARG("tensor1=%p, tensor2=%p", tensor1, tensor2);

    if (!vxoTensor_IsValidTensor(tensor1) ||
        !vxoTensor_IsValidTensor(tensor2))
    {
        gcmFOOTER_NO();
        return vx_false_e;
    }

    if (tensor1->tensorBuffer == tensor2->tensorBuffer)
    {
        vx_uint32 i, start1, end1, start2, end2;
        vx_bool contiguous1, contiguous2;

        start1 = tensor1->baseAddressOffset + vxoTensor_CalculateDimOffsetByStarts(tensor1, TENSOR_VIEW_STARTS(tensor1));
        end1 = start1 + vxoTensor_CalculateMaxMemorySize(tensor1) - 1;
        start2 = tensor2->baseAddressOffset + vxoTensor_CalculateDimOffsetByStarts(tensor2, TENSOR_VIEW_STARTS(tensor2));
        end2 = start2 + vxoTensor_CalculateMaxMemorySize(tensor2) - 1;
        if (end1 < start2 || end2 < start1)
        {
            gcmFOOTER_NO();
            return vx_false_e;
        }
        contiguous1 = vxoTensor_IsContiguousMemory(tensor1);
        contiguous2 = vxoTensor_IsContiguousMemory(tensor2);

        if (contiguous1 || contiguous2)
        {
            gcmFOOTER_NO();
            return vx_true_e;
        }
        else
        {
            vx_view_region_s view1, view2;
            vx_uint32 delta, times, dims;

            delta = gcmABS((vx_int32)tensor1->baseAddressOffset - (vx_int32)tensor2->baseAddressOffset);

            vxMemCopy(&view1, &tensor1->viewRegion, sizeof(vx_view_region_s));
            vxMemCopy(&view2, &tensor2->viewRegion, sizeof(vx_view_region_s));

            if (delta)
            {
                /* Adjust view first based on addressOffset */
                if (tensor1->baseAddressOffset < tensor2->baseAddressOffset)
                {
                    dims = TENSOR_DIM_NUM(tensor2) - 1;
                    times = delta / TENSOR_STRIDE_INDEX(tensor2, dims);
                    delta = delta % TENSOR_STRIDE_INDEX(tensor2, dims);
                    view2.viewStarts[dims] += times;
                    view2.viewEnds[dims] += times;
                    view2.viewStarts[0] += delta / TENSOR_DATA_SIZE(tensor2);
                    view2.viewEnds[0] += delta / TENSOR_DATA_SIZE(tensor2);
                }
                else
                {
                    dims = TENSOR_DIM_NUM(tensor1) - 1;
                    times = delta / TENSOR_STRIDE_INDEX(tensor1, dims);
                    delta = delta % TENSOR_STRIDE_INDEX(tensor1, dims);
                    view1.viewStarts[dims] += times;
                    view1.viewEnds[dims] += times;
                    view1.viewStarts[0] += delta / TENSOR_DATA_SIZE(tensor1);
                    view1.viewEnds[0] += delta / TENSOR_DATA_SIZE(tensor1);
                }
            }

            dims = gcmMIN(TENSOR_DIM_NUM(tensor1), TENSOR_DIM_NUM(tensor2));
            for (i = 0; i < dims; i++)
            {
                if (view1.viewEnds[i] - 1 < view2.viewStarts[i] ||
                    view2.viewEnds[i] - 1 < view1.viewStarts[i])
                {
                    gcmFOOTER_NO();
                    return vx_false_e;
                }
            }

            if (TENSOR_DIM_NUM(tensor1) == TENSOR_DIM_NUM(tensor2))
            {
                gcmFOOTER_NO();
                return vx_true_e;
            }
            else
            {
                dims = gcmMAX(TENSOR_DIM_NUM(tensor1), TENSOR_DIM_NUM(tensor2));
                for (; i < dims; i++)
                {
                    if ((TENSOR_DIM_NUM(tensor1) > TENSOR_DIM_NUM(tensor2) && view1.viewStarts[i]) ||
                        (TENSOR_DIM_NUM(tensor2) > TENSOR_DIM_NUM(tensor1) && view2.viewStarts[i]))
                    {
                        gcmFOOTER_NO();
                        return vx_false_e;
                    }
                }
                gcmFOOTER_NO();
                return vx_true_e;
            }
        }
    }
    gcmFOOTER_NO();
    return vx_false_e;
}

VX_INTERNAL_API vx_bool
vxoTensor_IsShared(
    vx_tensor tensor1,
    vx_tensor tensor2
    )
{
    return (vx_bool)(tensor1->tensorBuffer == tensor2->tensorBuffer);
}

VX_INTERNAL_API void vxoTensor_FreeWrappedMemory(vx_tensor tensor)
{
    vxmASSERT(tensor);

    vxoMemory_FreeWrappedMemory(tensor->base.context, &tensor->tensorBuffer->memory);
}

VX_INTERNAL_API vx_bool vxoTensro_WrapUserMemory(vx_tensor tensor)
{
    vx_bool status = vx_false_e;
    gcmHEADER_ARG("tensor=%p", tensor);

    status = vxoMemory_WrapUserMemory(tensor->base.context, &tensor->tensorBuffer->memory);

    if(status == vx_false_e)
    {
        gctPOINTER logical = gcvNULL;
        vx_uint32 size;

        vxmASSERT(tensor);
        vxoTensor_GetTensorSize(tensor, &size);
        tensor->tensorBuffer->memory.sizes[0] = size;

        gcoVX_AllocateMemory(size, (gctPOINTER*)&logical,
                                &tensor->tensorBuffer->memory.physicals[0],
                                &tensor->tensorBuffer->memory.nodePtrs[0]);

        gcoOS_MemCopy(tensor->tensorBuffer->memory.nodePtrs[0]->logical,
                        tensor->tensorBuffer->memory.logicals[0],
                        tensor->tensorBuffer->memory.sizes[0]);

        tensor->tensorBuffer->memory.allocated = vx_true_e;

        if (!vxCreateMutex(OUT &tensor->tensorBuffer->memory.writeLocks[0]))
        {
            goto OnError;
        }
        gcmFOOTER_ARG("%d", status);
        return vx_true_e;
    }
    else
    {
        tensor->useInternalMem = vx_false_e;
        gcoOS_CacheFlush(gcvNULL, tensor->tensorBuffer->memory.wrappedNode[0],tensor->tensorBuffer->memory.logicals[0], tensor->tensorBuffer->memory.wrappedSize[0]);
    }

    gcmFOOTER_ARG("%d", status);
    return vx_true_e;

OnError:
    if (tensor->tensorBuffer->memory.writeLocks[0] != VX_NULL)
    {
        vxDestroyMutex(tensor->tensorBuffer->memory.writeLocks[0]);
        tensor->tensorBuffer->memory.writeLocks[0]  = VX_NULL;
    }

    gcmFOOTER_ARG("%d", status);
    return vx_false_e;
}


VX_API_ENTRY vx_tensor VX_API_CALL vxCreateTensorFromHandle(
        vx_context context, const vx_tensor_create_params_t* tensor_create_params, vx_size size_of_create_params, const vx_tensor_addressing addrs,
        void * const ptr, vx_enum import_type)
{
    vx_tensor   tensor = VX_NULL;
    vx_uint32 i;

     gcmHEADER_ARG("context=%p, tensor_create_params=%p, size_of_create_params=0x%lx, addrs=%p, ptr=%p, import_type=0x%x",
         context, tensor_create_params, size_of_create_params, addrs, ptr, import_type);
     gcmDUMP_API("$VX vxCreateTensorFromHandle: context=%p, tensor_create_params=%p, size_of_create_params=0x%lx, addrs=%p, ptr=%p, import_type=0x%x",
         context, tensor_create_params, size_of_create_params, addrs, ptr, import_type);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (!vxoTensor_DataFormatIsSupported(tensor_create_params->data_format))
    {
        vxError("The tensor does not support data format %d", tensor_create_params->data_format);
        gcmFOOTER_NO();
        return VX_NULL;
    }

    if (!vxIsValidImportType(import_type))
    {
        gcmFOOTER_NO();
        return (vx_tensor)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_FORMAT);
    }

    if (ptr == VX_NULL)
    {
        gcmFOOTER_NO();
        return (vx_tensor)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }
    tensor = vxoTensor_CreateTensorEx(
                context,
                VX_NULL,
                tensor_create_params,
                vx_false_e
                );
    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return tensor;
    }
    if (import_type == VX_MEMORY_TYPE_HOST)
    {
        tensor->tensorBuffer->memory.wrapFlag  = gcvALLOC_FLAG_USERMEMORY;
    }
    else if(import_type == VX_MEMORY_TYPE_DMABUF)
    {
        tensor->tensorBuffer->memory.wrapFlag  = gcvALLOC_FLAG_DMABUF;
    }

    tensor->tensorBuffer->memory.logicals[0] = (vx_uint8_ptr)ptr;

    for (i = 0;i < VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
    {
        tensor->strides[i] = tensor->tensorBuffer->memory.strides[0][i] = addrs[0].dimStridesUser[i];
    }

    if (!vxoTensro_WrapUserMemory(tensor)) goto OnError;

    gcmFOOTER_NO();
    return tensor;

OnError:
    vxReleaseTensor(&tensor);
    gcmFOOTER_NO();
    return (vx_tensor)vxoContext_GetErrorObject(context, VX_ERROR_NO_RESOURCES);
}

VX_API_ENTRY vx_status VX_API_CALL vxSwapTensorHandle(vx_tensor tensor, void* const new_ptrs, void** prev_ptrs)
{
    vx_status status = VX_SUCCESS;

    gcmHEADER_ARG("tensor=%p, new_ptrs=%p, prev_ptrs=%p", tensor, new_ptrs, prev_ptrs);
    gcmDUMP_API("$VX vxSwapTensorHandle: tensor=%p, new_ptrs=%p, prev_ptrs=%p", tensor, new_ptrs, prev_ptrs);

    if (vxoTensor_IsValidTensor(tensor) == vx_true_e)
    {
        if (prev_ptrs != NULL)
        {
            if(tensor->useInternalMem == vx_false_e && tensor->tensorBuffer->memory.wrapFlag == gcvALLOC_FLAG_USERMEMORY)
            {
                if (tensor->tensorBuffer->memory.logicals[0] && tensor->tensorBuffer->memory.wrappedSize[0])
                {
                    gcoOS_CacheInvalidate(gcvNULL, tensor->tensorBuffer->memory.wrappedNode[0], tensor->tensorBuffer->memory.logicals[0], tensor->tensorBuffer->memory.wrappedSize[0]);
                }
            }
            *prev_ptrs = tensor->tensorBuffer->memory.logicals[0];
            vxInfo("prev_ptrs = %#llx", *prev_ptrs);
        }

        /* reclaim previous and set new handlers for this image */

        if (new_ptrs != NULL)
        {
            if(tensor->tensorBuffer->memory.wrapFlag == gcvALLOC_FLAG_USERMEMORY)
            {
                if(tensor->useInternalMem == vx_false_e)
                {
                    vx_bool sucess = vx_false_e;

                    vxoTensor_FreeWrappedMemory(tensor);
                    /* offset is non zero if this is a subimage of some image */
                    tensor->tensorBuffer->memory.logicals[0] = (vx_uint8_ptr)new_ptrs;
                    sucess = vxoTensro_WrapUserMemory(tensor);
                    if(sucess)
                    {
                        gcoOS_CacheFlush(gcvNULL, tensor->tensorBuffer->memory.wrappedNode[0], tensor->tensorBuffer->memory.logicals[0], tensor->tensorBuffer->memory.wrappedSize[0]);
                    }
                    vxInfo("memory.logicals = %#llx", tensor->tensorBuffer->memory.logicals[0]);
                }
                else
                {
                    gctUINT32_PTR logical = 0;
                    vx_uint32 size = 0;

                    if (tensor->tensorBuffer->memory.nodePtrs[0] != VX_NULL &&
                        tensor->tensorBuffer->memory.logicals[0] != tensor->tensorBuffer->memory.nodePtrs[0]->logical)
                    {
                        vxoTensor_ReleaseMemory(tensor);
                        tensor->tensorBuffer->memory.nodePtrs[0] = VX_NULL;
                        vxoTensor_GetTensorSize(tensor, &size);
                        tensor->tensorBuffer->memory.sizes[0] = size;
                    }

                    tensor->tensorBuffer->memory.logicals[0] = (vx_uint8_ptr)new_ptrs;

                    gcoVX_AllocateMemory((gctUINT32)tensor->tensorBuffer->memory.sizes[0], (gctPOINTER*)&logical,
                                &tensor->tensorBuffer->memory.physicals[0],
                                &tensor->tensorBuffer->memory.nodePtrs[0]);
                    tensor->tensorBuffer->memory.allocated = vx_true_e;
                    gcoOS_MemCopy(tensor->tensorBuffer->memory.nodePtrs[0]->logical, tensor->tensorBuffer->memory.logicals[0], tensor->tensorBuffer->memory.sizes[0]);

                }
            }
        }

    }
    else
    {
        status = VX_ERROR_INVALID_REFERENCE;
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}


VX_API_ENTRY vx_status VX_API_CALL vxSetTensorAttribute(vx_tensor tensor, vx_enum attribute, const void *ptr, vx_size size)
{
    gcmHEADER_ARG("tensor=%p, attribute=0x%x, ptr=%p, size=0x%lx", tensor, attribute, ptr, size);
    gcmDUMP_API("$VX vxSetTensorAttribute: tensor=%p, attribute=0x%x, ptr=%p, size=0x%lx", tensor, attribute, ptr, size);

    if (!vxoTensor_IsValidTensor(tensor))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_TENSOR_RANK:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            tensor->rank = *(vx_enum *)ptr;
            break;

        case VX_TENSOR_PRECISION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            tensor->tensorBuffer->precision = *(vx_enum *)ptr;
            break;

        case VX_TENSOR_LIFETIME:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            tensor->tensorBuffer->data_lifetime = *(vx_enum *)ptr;
            break;

        case VX_TENSOR_VALUE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_bool, 0x3);

            tensor->tensorBuffer->valued = *(vx_bool *)ptr;
            break;

        default:
            vxError("The attribute parameter, %d([%d]), is not supported", attribute, __LINE__);
            gcmFOOTER_NO();
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_NO();
    return VX_SUCCESS;
}


