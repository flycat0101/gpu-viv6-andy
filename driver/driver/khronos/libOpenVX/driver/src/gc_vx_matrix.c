/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>

VX_INTERNAL_CALLBACK_API void vxoMatrix_Destructor(vx_reference ref)
{
    vx_matrix matrix = (vx_matrix)ref;

    vxoMemory_Free(matrix->base.context, &matrix->memory);
}

VX_PUBLIC_API vx_status vxReleaseMatrix(vx_matrix *mat)
{
    return vxoReference_Release((vx_reference_ptr)mat, VX_TYPE_MATRIX, VX_REF_EXTERNAL);
}

VX_PUBLIC_API vx_matrix vxCreateMatrix(vx_context context, vx_enum data_type, vx_size columns, vx_size rows)
{
    vx_matrix   matrix;
    vx_size     dataSize;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    switch (data_type)
    {
        case VX_TYPE_INT8:
            dataSize = sizeof(vx_int8);
            break;

        case VX_TYPE_UINT8:
            dataSize = sizeof(vx_uint8);
            break;

        case VX_TYPE_INT16:
            dataSize = sizeof(vx_int16);
            break;

        case VX_TYPE_UINT16:
            dataSize = sizeof(vx_uint16);
            break;

        case VX_TYPE_INT32:
            dataSize = sizeof(vx_int32);
            break;

        case VX_TYPE_UINT32:
            dataSize = sizeof(vx_uint32);
            break;

        case VX_TYPE_FLOAT32:
            dataSize = sizeof(vx_float32);
            break;

        case VX_TYPE_INT64:
            dataSize = sizeof(vx_int64);
            break;

        case VX_TYPE_UINT64:
            dataSize = sizeof(vx_uint64);
            break;

        case VX_TYPE_FLOAT64:
            dataSize = sizeof(vx_float64);
            break;

        default:
            vxError("Invalid data type: %d", data_type);
            return (vx_matrix)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }

    vxmASSERT(dataSize != 0);

    if (columns == 0 || rows == 0)
    {
        vxError("Invalid columns x rows: %d x %d", columns, rows);
        return (vx_matrix)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    matrix = (vx_matrix)vxoReference_Create(context, VX_TYPE_MATRIX, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)matrix) != VX_SUCCESS) return matrix;

    matrix->dataType            = data_type;
    matrix->columns             = columns;
    matrix->rows                = rows;

    matrix->memory.planeCount   = 1;
    matrix->memory.dimCount     = 2;
    matrix->memory.dims[0][0]   = (vx_int32)dataSize;
    matrix->memory.dims[0][1]   = (vx_int32)(columns * rows);

    return (vx_matrix)matrix;
}

VX_PUBLIC_API vx_status vxQueryMatrix(vx_matrix matrix, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&matrix->base, VX_TYPE_MATRIX)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_MATRIX_ATTRIBUTE_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = matrix->dataType;
            break;

        case VX_MATRIX_ATTRIBUTE_ROWS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = matrix->rows;
            break;

        case VX_MATRIX_ATTRIBUTE_COLUMNS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = matrix->columns;
            break;

        case VX_MATRIX_ATTRIBUTE_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = matrix->columns * matrix->rows * matrix->memory.dims[0][0];
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxAccessMatrix(vx_matrix matrix, void *array)
{
    if (!vxoReference_IsValidAndSpecific(&matrix->base, VX_TYPE_MATRIX)) return VX_ERROR_INVALID_REFERENCE;

    if (!vxoMemory_Allocate(matrix->base.context, &matrix->memory)) return VX_ERROR_NO_MEMORY;

    vxAcquireMutex(matrix->base.lock);

    if (array != VX_NULL)
    {
        vx_size size = matrix->memory.strides[0][1] * matrix->memory.dims[0][1];

        vxMemCopy(array, matrix->memory.logicals[0], size);
    }

    vxReleaseMutex(matrix->base.lock);

    vxoReference_IncrementReadCount(&matrix->base);

    vxoReference_Increment(&matrix->base, VX_REF_EXTERNAL);

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxCommitMatrix(vx_matrix matrix, void *array)
{
    if (!vxoReference_IsValidAndSpecific(&matrix->base, VX_TYPE_MATRIX)) return VX_ERROR_INVALID_REFERENCE;

    if (!vxoMemory_Allocate(matrix->base.context, &matrix->memory)) return VX_ERROR_NO_MEMORY;

    vxAcquireMutex(matrix->base.lock);

    if (array != VX_NULL)
    {
        vx_size size = matrix->memory.strides[0][1] * matrix->memory.dims[0][1];

        vxMemCopy(matrix->memory.logicals[0], array, size);
    }

    vxReleaseMutex(matrix->base.lock);

    vxoReference_IncrementWriteCount(&matrix->base);

    vxoReference_Decrement(&matrix->base, VX_REF_EXTERNAL);

    return VX_SUCCESS;
}
