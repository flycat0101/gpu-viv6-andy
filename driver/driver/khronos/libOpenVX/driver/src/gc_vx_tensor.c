/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

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
    VX_TENSOR_NORMAL,
    VX_TENSOR_VIRTUAL,
    VX_TENSOR_SHARED,
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
        case VX_TYPE_INT32:
        case VX_TYPE_FLOAT16:
        case VX_TYPE_FLOAT32:
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

VX_PRIVATE_API vx_bool
vxoTensor_MemoryIsAllocated(
    vx_tensor_buffer_s * buffer
    )
{
    return buffer->memory.allocated;
}

VX_PRIVATE_API vx_uint32
vxoTensor_GetDataSizeByFormat(
    vx_enum format
    )
{
    switch (format)
    {
        case VX_TYPE_INT8:
            return 1;

        case VX_TYPE_FLOAT16:
            return 2;

        case VX_TYPE_INT32:
        case VX_TYPE_FLOAT32:
            return 4;

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
    vx_uint32 dnum;

    dnum = TENSOR_DIM_NUM(tensor);

    return TENSOR_STRIDE_INDEX(tensor, dnum-1) * TENSOR_VIEW_SIZE_INDEX(tensor, dnum-1);
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

    if (viewA->dimCount != viewB->dimCount) return vx_false_e;

    viewOut->dimCount = viewA->dimCount;

    for (i = 0; i < viewA->dimCount; i++)
    {
        viewOut->viewStarts[i] = VX_MAX(viewA->viewStarts[i], viewB->viewStarts[i]);
        viewOut->viewEnds[i] = VX_MIN(viewA->viewEnds[i], viewB->viewEnds[i]);
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_bool
vxoTensor_RectIsIn3DTensorRange(
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

VX_PRIVATE_API vx_status
vxoTensor_CheckAllocateMemory(
    vx_tensor tensor
    )
{
    if (tensor->tensorBuffer->memRefCount > 0) return VX_SUCCESS;

    return vxoTensor_AllocateMemory(tensor);
}

VX_PRIVATE_API vx_tensor
vxoTensor_Create(
    vx_context context,
    vx_uint32 numOfDims,
    vx_uint32 * sizes,
    vx_enum dataFormat,
    vx_view_region_s * viewRegion,
    vx_tensor_buffer_s * tensorBuffer,
    vx_uint32 baseOffset,
    vx_enum tensorType
    )
{
    vx_tensor tensor;
    vx_uint32 i, elementSize;

    tensor = (vx_tensor)vxoReference_Create(context, VX_TYPE_TENSOR, VX_REF_EXTERNAL, &context->base);
    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS) goto OnError;

    elementSize = vxoTensor_GetDataSizeByFormat(dataFormat);
    if (!elementSize) goto OnError;

    if (tensorType != VX_TENSOR_SHARED)
    {
        tensor->tensorBuffer = vxAllocateAndZeroMemory(sizeof(vx_tensor_buffer_s));
        if (tensor->tensorBuffer == VX_NULL) goto OnError;

        tensor->tensorBuffer->dataFormat = dataFormat;
        tensor->tensorBuffer->elementSize = elementSize;

        tensor->tensorBuffer->memory.planeCount = 1;
        tensor->tensorBuffer->memory.dimCount = tensor->dimCount = numOfDims;

        tensor->viewRegion.dimCount = numOfDims;
        tensor->baseAddressOffset = 0;

        for (i = 0; i < numOfDims; i++)
        {
            tensor->tensorBuffer->memory.dims[0][i] = tensor->dims[i] = sizes[i];
            tensor->tensorBuffer->memory.strides[0][i] = tensor->strides[i] =
                i ? sizes[i-1] * tensor->tensorBuffer->memory.strides[0][i-1]: elementSize;

            tensor->viewRegion.viewStarts[i] = 0;
            tensor->viewRegion.viewEnds[i] = sizes[i];
        }

        /* set the left dimensions to 1 */
        for (; i< VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
        {
            tensor->tensorBuffer->memory.dims[0][i] = tensor->dims[i] = 1;
            tensor->viewRegion.viewStarts[i] = 0;
            tensor->viewRegion.viewEnds[i] = 1;
        }

        if (tensorType == VX_TENSOR_VIRTUAL)
        {
            tensor->isVirtual = vx_true_e;
        }
    }
    else if (tensorBuffer != VX_NULL)
    {
        /* create from view or reshape */
        tensor->dimCount = numOfDims;
        tensor->viewRegion.dimCount = numOfDims;

        for (i = 0; i < numOfDims; i++)
        {
            tensor->dims[i] = sizes[i];
            tensor->strides[i] = i ? sizes[i-1] * tensor->strides[i-1] : elementSize;
            tensor->viewRegion.viewStarts[i] = 0;
            tensor->viewRegion.viewEnds[i] = sizes[i];
        }

        /* set the left dimensions to 1 */
        for (; i< VX_CONTEXT_TENSOR_MAX_DIMENSION; i++)
        {
            tensor->dims[i] = 1;
            tensor->viewRegion.viewStarts[i] = 0;
            tensor->viewRegion.viewEnds[i] = 1;
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

    if (tensorType == VX_TENSOR_SHARED) tensor->tensorBuffer->memRefCount++;
    tensor->tensorBuffer->bufRefCount++;

    tensor->tensorBuffer->roundingMode = (tensor->tensorBuffer->dataFormat == VX_TYPE_INT8) ? VX_NN_ROUNDING_MODE_RTNE : VX_NN_ROUNDING_MODE_SIMPLE_ROUNDING;

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

    return tensor;

OnError:
    if (tensorType != VX_TENSOR_SHARED &&
        tensor->tensorBuffer != VX_NULL)
    {
        vxFree(tensor->tensorBuffer);
        tensor->tensorBuffer = VX_NULL;
    }

    if (tensor != VX_NULL)
    {
        vxoReference_Release((vx_reference_ptr)&tensor, VX_TYPE_TENSOR, VX_REF_EXTERNAL);
    }

    return VX_NULL;
}

VX_PRIVATE_API vx_bool
vxoTensor_CheckValidTensorView(
    vx_tensor tensor,
    vx_tensor_view view
    )
{
    vx_uint32 i;

    if (!vxoTensor_IsValidTensor(tensor)) return vx_false_e;
    if (!vxoTensor_IsValidView(view)) return vx_false_e;

    if (TENSOR_DIM_NUM(tensor) != view->viewRegion.dimCount)
    {
        vxError("The tensor dim %d is not equal to view dim %d",
                 TENSOR_DIM_NUM(tensor), TENSOR_VIEW_DIM_NUM(tensor));
        return vx_false_e;
    }

    for (i = 0; i < TENSOR_DIM_NUM(tensor); i++)
    {
        if (view->viewRegion.viewStarts[i] < tensor->viewRegion.viewStarts[i] ||
            view->viewRegion.viewEnds[i] > tensor->viewRegion.viewEnds[i])
        {
            vxError("The %dth view dim range [%d - %d] is beyond tensor orignal range [%d - %d]", i,
                    view->viewRegion.viewStarts[i], view->viewRegion.viewEnds[i],
                    tensor->viewRegion.viewStarts[i], tensor->viewRegion.viewEnds[i]);
            return vx_false_e;
        }
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

    if (!vxoTensor_IsValidTensor(tensor)) return vx_false_e;
    if (!vxoTensor_IsValidAddressing(addressing)) return vx_false_e;

    if (TENSOR_DIM_NUM(tensor) != addressing->dimCount)
    {
        vxError("The tensor dim %d is not equal to addressing dim %d",
                 TENSOR_DIM_NUM(tensor), addressing->dimCount);
        return vx_false_e;
    }

    for (i = 0; i < TENSOR_DIM_NUM(tensor); i++)
    {
        if (addressing->dimSizesUser[i] > TENSOR_VIEW_SIZE_INDEX(tensor, i))
        {
            vxError("The %dth addressing dim size %d is beyond tensor orignal range %d",
                     i, addressing->dimSizesUser[i], tensor->viewRegion.viewEnds[i]);
            return vx_false_e;
        }
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_bool
vxoTensor_CheckValidViewAddressing(
    vx_tensor_view view,
    vx_tensor_addressing addressing
    )
{
    vx_uint32 i;

    if (!vxoTensor_IsValidView(view)) return vx_false_e;
    if (!vxoTensor_IsValidAddressing(addressing)) return vx_false_e;

    if (view->viewRegion.dimCount != addressing->dimCount)
    {
        vxError("The view dim %d is not equal to addressing dim %d", view->viewRegion.dimCount, addressing->dimCount);
        return vx_false_e;
    }

    for (i = 0; i < view->viewRegion.dimCount; i++)
    {
        if (view->viewRegion.viewEnds[i] - view->viewRegion.viewStarts[i] != addressing->dimSizesUser[i])
        {
            vxError("The %dth addressing size %d is not equel to view range [%d - %d]",
                     i, addressing->dimSizesUser[i], view->viewRegion.viewStarts[i], view->viewRegion.viewEnds[i]);
            return vx_false_e;
        }
    }

    return vx_true_e;
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

    if (!src || !dst || !count) return VX_ERROR_INVALID_PARAMETERS;

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
                return VX_ERROR_NOT_SUPPORTED;
            }
            src += ssize;
            dst += dsize;
        }
    }

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
    vx_uint32 max = curDim, stack[VX_CONTEXT_TENSOR_MAX_DIMENSION] = {0};
    vx_uint8_ptr srcs[VX_CONTEXT_TENSOR_MAX_DIMENSION], dsts[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_bool back = vx_false_e;

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

    usrElemSize = user_addr->dimStridesUser[0];
    dim = tensor->tensorBuffer;

    if (view == VX_NULL &&
        !tensor->isViewed &&
        usrElemSize == dim->elementSize &&
        vxoTensor_ViewRegionIsEqualAddressing(&tensor->viewRegion, user_addr))
    {
        /* Quick path for full tensor copy */
        totalSize = (vx_uint32)vxoMemory_ComputeSize(&dim->memory, 0);
        if (usage == VX_READ_ONLY)
        {
            vxMemCopy(user_ptr, (void*)TENSOR_LOGICAL_ADDR(tensor), totalSize);
        }
        else if (usage == VX_WRITE_ONLY)
        {
            vxMemCopy((void*)TENSOR_LOGICAL_ADDR(tensor), user_ptr, totalSize);
        }
        else
        {
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
        else return VX_ERROR_NOT_SUPPORTED;

        if (view != VX_NULL)
        {
            vxoTensor_MergeTwoViews(&tensor->viewRegion, &view->viewRegion, &viewMerged.viewRegion);
            offset = vxoTensor_CalculateDimOffsetByStarts(tensor, viewMerged.viewRegion.viewStarts);
            vxoTensor_CalculateSizesFromViewRegion(&viewMerged.viewRegion, sizes);
        }
        else
        {
            offset = vxoTensor_CalculateDimOffsetByStarts(tensor, tensor->viewRegion.viewStarts);
            vxoTensor_CalculateSizesFromViewRegion(&tensor->viewRegion, sizes);
        }

        tensorLogic = TENSOR_LOGICAL_ADDR(tensor) + offset;

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
            return VX_ERROR_NOT_SUPPORTED;
        }
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_object_array
vxoTensor_CreateImageArray(
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

    context = GET_CONTEXT(tensor);
    if (!vxoContext_IsValid(context)) goto exit;

    if (vxoTensor_CheckAllocateMemory(tensor) != VX_SUCCESS) goto exit;

    /* Stride must be equal to one image size. */
    d2stride = TENSOR_STRIDE_INDEX(tensor, 2);
    if (stride != d2stride) goto exit;

    width = rect.end_x - rect.start_x;
    height = rect.end_y - rect.start_y;

    imageArray = vxoOA_CreateObjectArrayEmpty((vx_reference)context, VX_TYPE_IMAGE, array_size);
    if (vxoReference_GetStatus((vx_reference) imageArray) != VX_SUCCESS) goto exit;
    imageArray->base.scope = (vx_reference)tensor;

    if (vxoTensor_GetTensorBaseMemory(tensor, (gctPOINTER*)&logical, &physical) != VX_SUCCESS) goto exit;
    offset = vxoTensor_CalculateDimOffsetByStarts(tensor, tensor->viewRegion.viewStarts);
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
    if (!finished) return VX_NULL;
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

    context = GET_CONTEXT(tensor);
    if (!vxoContext_IsValid(context)) goto exit;

    if (vxoTensor_CheckAllocateMemory(tensor) != VX_SUCCESS) goto exit;

    if (vxoTensor_GetTensorBaseMemory(tensor, (gctPOINTER*)&logical, &physical) != VX_SUCCESS) goto exit;
    offset = vxoTensor_CalculateDimOffsetByStarts(tensor, tensor->viewRegion.viewStarts);
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

    return image;

exit:
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
    vx_bool contiguous = vx_true_e;

    dsize = TENSOR_DIM_NUM(tensor);

    for (i = 0; i < dsize; i++)
    {
        omul *= TENSOR_VIEW_SIZE_INDEX(tensor, i);
        if (i < dsize-1 && tensor->viewRegion.viewStarts[i]) contiguous = vx_false_e;
    }

    /* only support tensor with contiguous memory to reshape now */
    if (!contiguous && dsize != bufsize)
        return vx_false_e;

    if (bufsize == 1 && dbuff[0] == -1)
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
                if (pos >= 0) return vx_false_e;
                pos = i;
                continue;
            }

            dims[i] = dbuff[i];
            nmul *=  dbuff[i];
        }

        if (pos >= 0)
        {
            if (omul % nmul) return vx_false_e;
            else dims[pos] = (vx_uint32)(omul / nmul);
        }
    }

    return vx_true_e;
}

/*****************************/
VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateTensor(
    vx_context context,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_uint8 fixed_point_pos
    )
{
    vx_tensor tensor;

    vxmASSERT(sizes);

    if (!vxoContext_IsValid(context)) return VX_NULL;
    if (num_of_dims > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor view dim num %d is out of range", num_of_dims);
        return VX_NULL;
    }

    if (!vxoTensor_DataFormatIsSupported(data_format))
    {
        vxError("The tensor does not support data format %d", data_format);
        return VX_NULL;
    }

    tensor = vxoTensor_CreateTensor(
                context,
                num_of_dims,
                sizes,
                data_format,
                vx_false_e
                );

    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS) return VX_NULL;

    tensor->tensorBuffer->fixedPointPos = fixed_point_pos;

    return tensor;
}

VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateVirtualTensor(
    vx_graph graph,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_uint8 fixed_point_pos
    )
{
    vx_context context;
    vx_tensor tensor;

    vxmASSERT(sizes);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH)) return VX_NULL;

    context = GET_CONTEXT(graph);

    if (!vxoContext_IsValid(context)) return VX_NULL;
    if (num_of_dims > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor view dim num %d is out of range", num_of_dims);
        return VX_NULL;
    }

    if (!vxoTensor_DataFormatIsSupported(data_format))
    {
        vxError("The tensor does not support data format %d", data_format);
        return VX_NULL;
    }

    tensor = vxoTensor_CreateTensor(
                context,
                num_of_dims,
                sizes,
                data_format,
                vx_true_e
                );

    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS) return VX_NULL;

    tensor->tensorBuffer->fixedPointPos = fixed_point_pos;
    tensor->base.scope = (vx_reference)graph;

    return tensor;
}

VX_API_ENTRY vx_tensor VX_API_CALL
vxCreateTensorFromView(
    vx_tensor tensor,
    vx_tensor_view view
    )
{
    vx_tensor childTensor;
    vx_context context;
    vx_uint32 offset = 0;

    if (!vxoTensor_CheckValidTensorView(tensor, view)) return VX_NULL;

    context = GET_CONTEXT(tensor);
    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (tensor->isViewed)
    {
        offset = vxoTensor_CalculateDimOffsetByStarts(tensor, tensor->viewRegion.viewStarts);
    }

    childTensor = vxoTensor_Create(
                    context,
                    TENSOR_DIM_NUM(tensor),
                    TENSOR_SIZES(tensor),
                    TENSOR_DATA_TYPE(tensor),
                    &view->viewRegion,
                    tensor->tensorBuffer,
                    tensor->baseAddressOffset + offset,
                    VX_TENSOR_SHARED
                    );

    if (vxoReference_GetStatus((vx_reference)tensor) != VX_SUCCESS) return VX_NULL;

    return childTensor;
}

VX_API_ENTRY vx_status VX_API_CALL
vxReleaseTensor(
    vx_tensor *tensor
    )
{
    return vxoTensor_ReleaseTensor(tensor);
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

    vxmASSERT(view_array_start);
    vxmASSERT(view_array_end);

    if (!vxoContext_IsValid(context)) return VX_NULL;
    if (numViewDimensions > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor view dim num %d is out of range", numViewDimensions);
        return VX_NULL;
    }

    view = (vx_tensor_view)vxoReference_Create(context, VX_TYPE_TENSOR_VIEW, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)view) != VX_SUCCESS) return VX_NULL;

    for (i = 0; i < numViewDimensions; i++)
    {
        if (view_array_start[i] > view_array_end[i])
        {
            vxError("The %dth of view array start %d is smaller than end %d",
                     i, view_array_start[i], view_array_end[i]);
            return VX_NULL;
        }

        view->viewRegion.viewStarts[i] = view_array_start[i];
        view->viewRegion.viewEnds[i] = view_array_end[i];
    }

    view->viewRegion.dimCount = numViewDimensions;

    return view;
}

VX_API_ENTRY vx_status VX_API_CALL
vxReleaseTensorView(
    vx_tensor_view *tensor_view
    )
{
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

    vxmASSERT(addressing_array_dimension);
    vxmASSERT(addressing_array_stride);

    if (!vxoContext_IsValid(context)) return VX_NULL;
    if (numViewDimensions > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
        vxError("The tensor addressing dim num %d is out of range", numViewDimensions);
        return VX_NULL;
    }

    addressing = (vx_tensor_addressing)vxoReference_Create(context, VX_TYPE_TENSOR_ADDRESS, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)addressing) != VX_SUCCESS) return VX_NULL;

    for (i = 0; i < numViewDimensions; i++)
    {
        addressing->dimSizesUser[i]   = addressing_array_dimension[i];
        addressing->dimStridesUser[i] = addressing_array_stride[i];
    }

    addressing->dimCount = numViewDimensions;

    return addressing;
}

VX_API_ENTRY vx_status VX_API_CALL
vxReleaseTensorAddressing(
    vx_tensor_addressing *tensor_addr
    )
{
    return vxoReference_Release((vx_reference_ptr)tensor_addr, VX_TYPE_TENSOR_ADDRESS, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL
vxCopyTensorPatch(
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

    if (vxoTensor_CheckAllocateMemory(tensor) != VX_SUCCESS) return VX_ERROR_NOT_ALLOCATED;

    status = vxoTensor_CopyTensorPatch(tensor, view, user_addr, user_ptr, usage);

    return status;
}

VX_API_ENTRY vx_object_array VX_API_CALL
vxCreateImageObjectArrayFromTensor(
    vx_tensor tensor,
    vx_rectangle_t rect,
    vx_uint32 array_size,
    vx_uint32 stride,
    vx_df_image image_format
    )
{
    if (!vxoTensor_IsValidTensor(tensor)) return VX_NULL;

    if (TENSOR_DIM_NUM(tensor) != 3 || !array_size) return VX_NULL;
    if (!vxoTensor_RectIsIn3DTensorRange(tensor, rect, stride, array_size)) return VX_NULL;
    if (!vxoTensor_ImageFormatIsSupported(image_format)) return VX_NULL;

    return vxoTensor_CreateImageArray(tensor, rect, array_size, stride, image_format);
}

VX_API_ENTRY vx_tensor VX_API_CALL
vxReshapeTensor(
    vx_tensor    tensor,
    vx_int32*    num_of_dims,
    vx_uint32    sizes
    )
{
    vx_uint32 newDims[VX_CONTEXT_TENSOR_MAX_DIMENSION], offset = 0;

    vx_context context = GET_CONTEXT(tensor);
    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (!vxoTensor_IsValidTensor(tensor)) return VX_NULL;

    if (!vxoTensor_FillDimBuffer(tensor, num_of_dims, sizes, newDims))
        return VX_NULL;

    if (tensor->isViewed)
    {
        offset = vxoTensor_CalculateDimOffsetByStarts(tensor, tensor->viewRegion.viewStarts);
    }

    return vxoTensor_Create(
                context,
                sizes,
                newDims,
                TENSOR_DATA_TYPE(tensor),
                VX_NULL,
                tensor->tensorBuffer,
                tensor->baseAddressOffset + offset,
                VX_TENSOR_SHARED
                );
}

VX_API_ENTRY vx_status VX_API_CALL
vxQueryTensor(
    vx_tensor tensor,
    vx_enum attribute,
    void *ptr,
    vx_size size
    )
{
    if (!vxoTensor_IsValidTensor(tensor)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_TENSOR_NUM_OF_DIMS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);
            *(vx_uint32 *)ptr = TENSOR_DIM_NUM(tensor);
            break;

        case VX_TENSOR_DIMS:
            if (size < sizeof(vx_uint32) * TENSOR_DIM_NUM(tensor) ||
                size > sizeof(vx_uint32) * VX_CONTEXT_TENSOR_MAX_DIMENSION)
            {
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

        case VX_TENSOR_FIXED_POINT_POS:
            vxmVALIDATE_PARAMETERS_EX(ptr, size, vx_uint8);
            *(vx_uint8 *)ptr = TENSOR_POS(tensor);
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_INVALID_PARAMETERS;
    }

    return VX_SUCCESS;
}

/*****************************/
VX_INTERNAL_CALLBACK_API void
vxoTensor_Destructor(
    vx_reference ref
    )
{
    vx_tensor tensor = (vx_tensor)ref;

    vxoTensor_ReleaseMemory(tensor);

    if (tensor->tensorBuffer != VX_NULL &&
        !--tensor->tensorBuffer->bufRefCount)
    {
        vxFree(tensor->tensorBuffer);
        tensor->tensorBuffer = VX_NULL;
    }
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

    if (addressing == VX_NULL) return vx_false_e;
    if (!vxoReference_IsValidAndSpecific(&addressing->base, VX_TYPE_TENSOR_ADDRESS)) return vx_false_e;
    if (!addressing->dimCount) return vx_false_e;

    for (i = 0; i < addressing->dimCount; i++)
    {
        if (!addressing->dimSizesUser[i] ||
            !addressing->dimStridesUser[i])
            return vx_false_e;
    }

    return vx_true_e;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorBaseMemory(
    vx_tensor tensor,
    gctPOINTER * logical,
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

    if (!vxoTensor_MemoryIsAllocated(tensor->tensorBuffer))
    {
        return VX_ERROR_NOT_ALLOCATED;
    }

    if (logical) *logical = TENSOR_LOGICAL_ADDR(tensor) + tensor->baseAddressOffset;
    if (physical) *physical = TENSOR_PHYSICAL_ADDR(tensor) + tensor->baseAddressOffset;

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorViewMemory(
    vx_tensor tensor,
    gctPOINTER * logical,
    vx_uint32 * physical
    )
{
    vxoTensor_GetTensorBaseMemory(tensor, logical, physical);

    if (tensor->isViewed)
    {
        vx_uint32 dimIndex = 0;
        vx_uint32 viewOffset = 0;
        for (dimIndex = 0; dimIndex < TENSOR_VIEW_DIM_NUM(tensor); dimIndex++)
        {
            viewOffset += tensor->viewRegion.viewStarts[dimIndex] * TENSOR_STRIDE_INDEX(tensor, dimIndex);;
        }

        if (logical) *logical = (vx_uint8_ptr)*logical + viewOffset;
        if (physical) *physical = (vx_uint32)*physical + viewOffset;
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorSize(
    vx_tensor tensor,
    vx_uint32 *size
    )
{
    if (!vxoTensor_IsValidTensor(tensor))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (size != VX_NULL)
    {
        if (tensor->isViewed)
        {
            vx_uint32 index;
            vx_uint32 elementCount = 1;

            for (index = 0; index < TENSOR_VIEW_DIM_NUM(tensor); index++)
            {
                elementCount *= TENSOR_VIEW_SIZE_INDEX(tensor, index);
            }

            *size =  elementCount * vxoTensor_GetDataSizeByFormat(TENSOR_DATA_TYPE(tensor));
        }
        else
        {
            *size = (vx_uint32)vxoMemory_ComputeElementCount(&tensor->tensorBuffer->memory, 0) * vxoTensor_GetDataSizeByFormat(TENSOR_DATA_TYPE(tensor));
        }
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_GetTensorElementCount(
    vx_tensor tensor,
    vx_uint32 *count
    )
{
    if (!vxoTensor_IsValidTensor(tensor))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if(count != NULL)
    {
        vx_uint32 elementCount = 1;
        if(tensor->isViewed)
        {
            vx_uint32 index;

            for(index = 0; index < TENSOR_VIEW_DIM_NUM(tensor); index++)
            {
                elementCount *= TENSOR_VIEW_SIZE_INDEX(tensor, index);
            }
        }
        else
        {
            elementCount = (vx_uint32)vxoMemory_ComputeElementCount(&tensor->tensorBuffer->memory, 0);
        }
        *count = elementCount;
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

    if (!vxoTensor_IsValidTensor(tensor))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!count || (sizes == VX_NULL && strides == VX_NULL))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (*count && *count > VX_CONTEXT_TENSOR_MAX_DIMENSION)
    {
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
    if (!vxoTensor_IsValidTensor(tensor))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!count || (starts == VX_NULL && ends == VX_NULL))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (count > TENSOR_VIEW_DIM_NUM(tensor))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    if (starts != VX_NULL)
    {
        vxMemCopy(starts, tensor->viewRegion.viewStarts, count * sizeof(vx_uint32));
    }
    if (ends != VX_NULL)
    {
        vxMemCopy(ends, tensor->viewRegion.viewEnds, count * sizeof(vx_uint32));
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_tensor
vxoTensor_CreateTensor(
    vx_context context,
    vx_uint32 num_of_dims,
    vx_uint32 *sizes,
    vx_enum data_format,
    vx_bool is_virtual
    )
{
    return vxoTensor_Create(
                context,
                num_of_dims,
                sizes,
                data_format,
                VX_NULL,
                VX_NULL,
                0,
                is_virtual ? VX_TENSOR_VIRTUAL : VX_TENSOR_NORMAL
                );
}

VX_INTERNAL_API vx_status
vxoTensor_ReleaseTensor(
    vx_tensor *tensor
    )
{
    return vxoReference_Release((vx_reference_ptr)tensor, VX_TYPE_TENSOR, VX_REF_EXTERNAL);
}

VX_INTERNAL_API vx_status
vxoTensor_AllocateMemory(
    vx_tensor tensor
    )
{
    if (!vxoTensor_IsValidTensor(tensor))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (vxoTensor_MemoryIsAllocated(tensor->tensorBuffer)) return VX_SUCCESS;

    ++tensor->tensorBuffer->memRefCount;

    if (!vxoMemory_Allocate(tensor->base.context, &tensor->tensorBuffer->memory))
    {
        --tensor->tensorBuffer->memRefCount;
        return VX_FAILURE;
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status
vxoTensor_ReleaseMemory(
    vx_tensor tensor
    )
{
    if (!vxoTensor_IsValidTensor(tensor))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!vxoTensor_MemoryIsAllocated(tensor->tensorBuffer) ||
        --tensor->tensorBuffer->memRefCount > 0)
    {
        return VX_SUCCESS;
    }

    if (!vxoMemory_Free(tensor->base.context, &tensor->tensorBuffer->memory))
    {
        return VX_FAILURE;
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_bool
vxoTensor_IsOverlap(
    vx_tensor tensor1,
    vx_tensor tensor2
    )
{
    vx_uint32 start1, end1, start2, end2;

    if (!vxoTensor_IsValidTensor(tensor1) ||
        !vxoTensor_IsValidTensor(tensor2))
    {
        return vx_false_e;
    }

    if (tensor1->tensorBuffer == tensor2->tensorBuffer)
    {
        start1 = tensor1->baseAddressOffset + vxoTensor_CalculateDimOffsetByStarts(tensor1, tensor1->viewRegion.viewStarts);
        end1   = start1 + vxoTensor_CalculateMaxMemorySize(tensor1) - 1;
        start2 = tensor2->baseAddressOffset + vxoTensor_CalculateDimOffsetByStarts(tensor2, tensor2->viewRegion.viewStarts);
        end2   = start2 + vxoTensor_CalculateMaxMemorySize(tensor2) - 1;

        if (end1 < start2 || end2 < start1) return vx_false_e;
        else return vx_true_e;
    }

    return vx_false_e;
}


