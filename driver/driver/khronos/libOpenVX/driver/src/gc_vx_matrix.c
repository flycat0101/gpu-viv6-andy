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

VX_INTERNAL_CALLBACK_API void vxoMatrix_Destructor(vx_reference ref)
{
    vx_matrix matrix = (vx_matrix)ref;

    vxoMemory_Free(matrix->base.context, &matrix->memory);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseMatrix(vx_matrix *mat)
{
    return vxoReference_Release((vx_reference_ptr)mat, VX_TYPE_MATRIX, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_matrix VX_API_CALL vxCreateMatrix(vx_context context, vx_enum data_type, vx_size columns, vx_size rows)
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

    matrix->origin.x = (vx_uint32)columns / 2;
    matrix->origin.y = (vx_uint32)rows / 2;
    matrix->pattern = VX_PATTERN_OTHER;

    return (vx_matrix)matrix;
}

VX_API_ENTRY vx_matrix VX_API_CALL vxCreateMatrixFromPattern(vx_context context, vx_enum pattern, vx_size columns, vx_size rows)
{
    vx_matrix matrix;
    if (!vxoContext_IsValid(context))
        return 0;

    if ((columns > VX_INT_MAX_NONLINEAR_DIM) || (rows > VX_INT_MAX_NONLINEAR_DIM))
    {
        vxError("Invalid dimensions to matrix\n");
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_DIMENSION, "Invalid dimensions to matrix\n");
        return (vx_matrix)vxoError_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    matrix = vxCreateMatrix(context, VX_TYPE_UINT8, columns, rows);
    if (vxoReference_IsValidAndSpecific(&matrix->base, VX_TYPE_MATRIX) == vx_true_e)
    {
        if (vxoMemory_Allocate(matrix->base.context, &matrix->memory) == vx_true_e)
        {
            vxAcquireMutex(matrix->base.lock);
            {
            vx_uint8* ptr = matrix->memory.logicals[0];
            vx_size x, y;
            for (y = 0; y < rows; ++y)
            {
                for (x = 0; x < columns; ++x)
                {
                    vx_uint8 value = 0;
                    switch (pattern)
                    {
                    case VX_PATTERN_BOX: value = 255; break;
                    case VX_PATTERN_CROSS: value = ((y == rows / 2) || (x == columns / 2)) ? 255 : 0; break;
                    case VX_PATTERN_DISK:
                        value = (((y - rows / 2.0 + 0.5) * (y - rows / 2.0 + 0.5)) / ((rows / 2.0) * (rows / 2.0)) +
                            ((x - columns / 2.0 + 0.5) * (x - columns / 2.0 + 0.5)) / ((columns / 2.0) * (columns / 2.0)))
                            <= 1 ? 255 : 0;
                        break;
                    }
                    ptr[x + y * columns] = value;
                }
            }
            }
            vxReleaseMutex(matrix->base.lock);
            vxoReference_IncrementWriteCount(&matrix->base);
            matrix->pattern = pattern;
        }
        else
        {
            vxReleaseMatrix(&matrix);
            vxError("Failed to allocate matrix\n");
            vxAddLogEntry(&context->base, VX_ERROR_NO_MEMORY, "Failed to allocate matrix\n");
            matrix = (vx_matrix)vxoError_GetErrorObject(context, VX_ERROR_NO_MEMORY);
        }
    }

    return matrix;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryMatrix(vx_matrix matrix, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&matrix->base, VX_TYPE_MATRIX)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_MATRIX_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = matrix->dataType;
            break;

        case VX_MATRIX_ROWS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = matrix->rows;
            break;

        case VX_MATRIX_COLUMNS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = matrix->columns;
            break;

        case VX_MATRIX_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = matrix->columns * matrix->rows * matrix->memory.dims[0][0];
            break;

        case VX_MATRIX_ORIGIN:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_coordinates2d_t, 0x3);

            *(vx_coordinates2d_t *)ptr = matrix->origin;
            break;

        case VX_MATRIX_PATTERN:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = matrix->pattern;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxReadMatrix(vx_matrix matrix, void *array)
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

    /*remove reference count for v1.1(nonlinear)*/
    /*vxoReference_Increment(&matrix->base, VX_REF_EXTERNAL);*/

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxWriteMatrix(vx_matrix matrix, const void *array)
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

    /*remove reference count for v1.1(nonlinear)*/
    /*vxoReference_Decrement(&matrix->base, VX_REF_EXTERNAL);*/

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyMatrix(vx_matrix matrix, void *ptr, vx_enum usage, vx_enum mem_type)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if (vxoReference_IsValidAndSpecific(&matrix->base, VX_TYPE_MATRIX))
    {
        if (vxoMemory_Allocate(matrix->base.context, &matrix->memory) == vx_true_e)
        {
            if (usage == VX_READ_ONLY)
            {
                vxAcquireMutex(matrix->base.lock);
                if (ptr)
                {
                    vx_size size = matrix->memory.strides[0][1] *
                                   matrix->memory.dims[0][1];
                    memcpy(ptr, matrix->memory.logicals[0], size);
                }
                vxReleaseMutex(matrix->base.lock);
                vxoReference_IncrementReadCount(&matrix->base);
                status = VX_SUCCESS;
            }
            else if (usage == VX_WRITE_ONLY)
            {
                vxAcquireMutex(matrix->base.lock);
                if (ptr)
                {
                    vx_size size = matrix->memory.strides[0][1] *
                                   matrix->memory.dims[0][1];
                    memcpy(matrix->memory.logicals[0], ptr, size);
                }
                vxReleaseMutex(matrix->base.lock);
                vxoReference_IncrementWriteCount(&matrix->base);
                status = VX_SUCCESS;
            }
            else
            {
                vxError("Wrong parameters for matrix\n");
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
        else
        {
            vxError("Failed to allocate matrix\n");
            status = VX_ERROR_NO_MEMORY;
        }
    }
    else
    {
        vxError("Invalid reference for matrix\n");
    }

    return status;
}

