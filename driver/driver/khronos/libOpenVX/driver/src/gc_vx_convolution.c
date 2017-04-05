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


#include <gc_vx_common.h>

VX_INTERNAL_CALLBACK_API void vxoConvolution_Destructor(vx_reference ref)
{
    vx_convolution convolution = (vx_convolution)ref;

    vxoMatrix_Destructor((vx_reference)&convolution->matrix);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseConvolution(vx_convolution *convolution)
{
    return vxoReference_Release((vx_reference_ptr)convolution, VX_TYPE_CONVOLUTION, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_convolution VX_API_CALL vxCreateConvolution(vx_context context, vx_size columns, vx_size rows)
{
    vx_convolution convolution;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if ((!vxIsOdd(columns) || columns < 3) && (!vxIsOdd(rows) || rows < 3))
    {
        return (vx_convolution )vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    convolution = (vx_convolution)vxoReference_Create(context, VX_TYPE_CONVOLUTION, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)convolution) != VX_SUCCESS) return convolution;

    convolution->matrix.dataType            = VX_TYPE_INT16;
    convolution->matrix.columns             = columns;
    convolution->matrix.rows                = rows;

    convolution->matrix.memory.planeCount   = 1;
    convolution->matrix.memory.dimCount     = 2;
    convolution->matrix.memory.dims[0][0]   = sizeof(vx_int16);
    convolution->matrix.memory.dims[0][1]   = (vx_int32)(columns*rows);

    convolution->scale = 1;

    return convolution;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryConvolution(vx_convolution convolution, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&convolution->matrix.base, VX_TYPE_CONVOLUTION))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    switch (attribute)
    {
        case VX_CONVOLUTION_ATTRIBUTE_ROWS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = convolution->matrix.rows;
            break;

        case VX_CONVOLUTION_ATTRIBUTE_COLUMNS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = convolution->matrix.columns;
            break;

        case VX_CONVOLUTION_ATTRIBUTE_SCALE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = convolution->scale;
            break;

        case VX_CONVOLUTION_ATTRIBUTE_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = convolution->matrix.columns * convolution->matrix.rows * sizeof(vx_int16);
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetConvolutionAttribute(vx_convolution convolution, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_uint32 scale;

    if (!vxoReference_IsValidAndSpecific(&convolution->matrix.base, VX_TYPE_CONVOLUTION))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    switch (attribute)
    {
        case VX_CONVOLUTION_ATTRIBUTE_SCALE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            scale = *(vx_uint32_ptr)ptr;

            if (!vxIsPowerOfTwo(scale)) return VX_ERROR_INVALID_VALUE;

            convolution->scale = scale;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxReadConvolutionCoefficients(vx_convolution convolution, vx_int16 *array)
{
    if (!vxoReference_IsValidAndSpecific(&convolution->matrix.base, VX_TYPE_CONVOLUTION))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!vxoMemory_Allocate(convolution->matrix.base.context, &convolution->matrix.memory))
    {
        return VX_ERROR_NO_MEMORY;
    }

    vxAcquireMutex(convolution->matrix.base.lock);

    if (array != VX_NULL)
    {
        vx_size size = convolution->matrix.memory.strides[0][1] * convolution->matrix.memory.dims[0][1];

        vxMemCopy(array, convolution->matrix.memory.logicals[0], size);
    }

    vxReleaseMutex(convolution->matrix.base.lock);

    vxoReference_IncrementReadCount(&convolution->matrix.base);

    vxoReference_Increment(&convolution->matrix.base, VX_REF_EXTERNAL);

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxWriteConvolutionCoefficients(vx_convolution convolution, const vx_int16 *array)
{
    if (!vxoReference_IsValidAndSpecific(&convolution->matrix.base, VX_TYPE_CONVOLUTION))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!vxoMemory_Allocate(convolution->matrix.base.context, &convolution->matrix.memory))
    {
        return VX_ERROR_NO_MEMORY;
    }

    vxAcquireMutex(convolution->matrix.base.lock);

    if (array != VX_NULL)
    {
        vx_size size = convolution->matrix.memory.strides[0][1] * convolution->matrix.memory.dims[0][1];

        vxMemCopy(convolution->matrix.memory.logicals[0], array, size);
    }

    vxReleaseMutex(convolution->matrix.base.lock);

    vxoReference_IncrementWriteCount(&convolution->matrix.base);

    vxoReference_Decrement(&convolution->matrix.base, VX_REF_EXTERNAL);

    return VX_SUCCESS;
}

