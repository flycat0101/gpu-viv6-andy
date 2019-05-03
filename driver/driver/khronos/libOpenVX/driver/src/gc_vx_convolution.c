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

#define _GC_OBJ_ZONE            gcdZONE_VX_CONV

VX_INTERNAL_CALLBACK_API void vxoConvolution_Destructor(vx_reference ref)
{
    vx_convolution convolution = (vx_convolution)ref;

    vxoMatrix_Destructor((vx_reference)&convolution->matrix);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseConvolution(vx_convolution *convolution)
{
    gcmDUMP_API("$VX vxReleaseConvolution: convolution=%p", convolution);

    return vxoReference_Release((vx_reference_ptr)convolution, VX_TYPE_CONVOLUTION, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_convolution VX_API_CALL vxCreateConvolution(vx_context context, vx_size columns, vx_size rows)
{
    vx_convolution convolution;
    gcmHEADER_ARG("context=%p, columns=0x%lx rows=0x%lx", context, columns, rows);
    gcmDUMP_API("$VX vxCreateConvolution: context=%p, columns=0x%lx rows=0x%lx", context, columns, rows);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if ((!vxIsOdd(columns) || columns < 3) && (!vxIsOdd(rows) || rows < 3))
    {
        gcmFOOTER_NO();
        return (vx_convolution )vxoContext_GetErrorObject(context, VX_ERROR_INVALID_DIMENSION);
    }

    convolution = (vx_convolution)vxoReference_Create(context, VX_TYPE_CONVOLUTION, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)convolution) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get reference status failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: Get reference status failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%p", convolution);
        return convolution;
    }

    convolution->matrix.dataType            = VX_TYPE_INT16;
    convolution->matrix.columns             = columns;
    convolution->matrix.rows                = rows;

    convolution->matrix.memory.planeCount   = 1;
    convolution->matrix.memory.dimCount     = 2;
    convolution->matrix.memory.dims[0][0]   = sizeof(vx_int16);
    convolution->matrix.memory.dims[0][1]   = (vx_int32)(columns*rows);

    convolution->scale = 1;

    gcmFOOTER_ARG("%p", convolution);
    return convolution;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryConvolution(vx_convolution convolution, vx_enum attribute, void *ptr, vx_size size)
{

    gcmHEADER_ARG("convolution=%p, attribute=0x%x, ptr=%p, size=0x%lx", convolution, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryConvolution: convolution=%p, attribute=0x%x, ptr=%p, size=0x%lx", convolution, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&convolution->matrix.base, VX_TYPE_CONVOLUTION))
    {
        vxError("%s[%d]: Convolution reference is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    switch (attribute)
    {
        case VX_CONVOLUTION_ROWS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = convolution->matrix.rows;
            break;

        case VX_CONVOLUTION_COLUMNS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = convolution->matrix.columns;
            break;

        case VX_CONVOLUTION_SCALE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = convolution->scale;
            break;

        case VX_CONVOLUTION_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = convolution->matrix.columns * convolution->matrix.rows * sizeof(vx_int16);
            break;

        default:
            vxError("%s[%d]: The attribute parameter, %d, is not supported!\n", __FUNCTION__, __LINE__, attribute);
            vxAddLogEntry(&convolution->matrix.base, VX_ERROR_NOT_SUPPORTED, "%s[%d]: The attribute parameter, %d, is not supported!\n", __FUNCTION__, __LINE__, attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetConvolutionAttribute(vx_convolution convolution, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_uint32 scale;

    gcmHEADER_ARG("convolution=%p, attribute=0x%x, ptr=%p, size=0x%lx", convolution, attribute, ptr, size);
    gcmDUMP_API("$VX vxSetConvolutionAttribute: convolution=%p, attribute=0x%x, ptr=%p, size=0x%lx", convolution, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&convolution->matrix.base, VX_TYPE_CONVOLUTION))
    {
        vxError("%s[%d]: Convolution reference is invalid!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    switch (attribute)
    {
        case VX_CONVOLUTION_SCALE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            scale = *(vx_uint32_ptr)ptr;

            if (!vxIsPowerOfTwo(scale)) return VX_ERROR_INVALID_VALUE;

            convolution->scale = scale;
            break;

        default:
            vxError("%s[%d]: The attribute parameter, %d, is not supported", __FUNCTION__, __LINE__, attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxReadConvolutionCoefficients(vx_convolution convolution, vx_int16 *array)
{
    gcmHEADER_ARG("convolution=%p, array=%p", convolution, array);
    gcmDUMP_API("$VX vxReadConvolutionCoefficients: convolution=%p, array=%p", convolution, array);

    if (!vxoReference_IsValidAndSpecific(&convolution->matrix.base, VX_TYPE_CONVOLUTION))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!vxoMemory_Allocate(convolution->matrix.base.context, &convolution->matrix.memory))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_NO_MEMORY);
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

    /*vxoReference_Increment(&convolution->matrix.base, VX_REF_EXTERNAL);*/

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxWriteConvolutionCoefficients(vx_convolution convolution, const vx_int16 *array)
{
    gcmHEADER_ARG("convolution=%p, array=%p", convolution, array);
    gcmDUMP_API("$VX vxWriteConvolutionCoefficients: convolution=%p, array=%p", convolution, array);

    if (!vxoReference_IsValidAndSpecific(&convolution->matrix.base, VX_TYPE_CONVOLUTION))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (!vxoMemory_Allocate(convolution->matrix.base.context, &convolution->matrix.memory))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_NO_MEMORY);
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

    /*vxoReference_Decrement(&convolution->matrix.base, VX_REF_EXTERNAL);*/

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyConvolutionCoefficients(vx_convolution convolution, void *ptr, vx_enum usage, vx_enum mem_type)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;

    gcmHEADER_ARG("convolution=%p, ptr=%p, usage=0x%x, mem_type=0x%x", convolution, ptr, usage, mem_type);
    gcmDUMP_API("$VX vxCopyConvolutionCoefficients: convolution=%p, ptr=%p, usage=0x%x, mem_type=0x%x", convolution, ptr, usage, mem_type);

    if (vxoReference_IsValidAndSpecific(&convolution->matrix.base, VX_TYPE_CONVOLUTION) == vx_true_e)
    {
        if (vxoMemory_Allocate(convolution->matrix.base.context, &convolution->matrix.memory) == vx_true_e)
        {
            if (usage == VX_READ_ONLY)
            {
                vxAcquireMutex(convolution->matrix.base.lock);
                if (ptr)
                {
                    vx_size size = convolution->matrix.memory.strides[0][1] *
                                   convolution->matrix.memory.dims[0][1];
                    memcpy(ptr, convolution->matrix.memory.logicals[0], size);
                }
                vxReleaseMutex(convolution->matrix.base.lock);
                vxoReference_IncrementReadCount(&convolution->matrix.base);
                status = VX_SUCCESS;
            }
            else if (usage == VX_WRITE_ONLY)
            {
                vxAcquireMutex(convolution->matrix.base.lock);
                if (ptr)
                {
                    vx_size size = convolution->matrix.memory.strides[0][1] *
                                   convolution->matrix.memory.dims[0][1];

                    memcpy(convolution->matrix.memory.logicals[0], ptr, size);
                }
                vxReleaseMutex(convolution->matrix.base.lock);
                vxoReference_IncrementWriteCount(&convolution->matrix.base);
                status = VX_SUCCESS;
            }
            else
            {
                vxError("Wrong parameters for convolution\n");
                status = VX_ERROR_INVALID_PARAMETERS;
            }
        }
        else
        {
            vxError("Failed to allocate convolution\n");
            status = VX_ERROR_NO_MEMORY;
        }
    }
    else
    {
        vxError("Invalid reference for convolution\n");
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_convolution VX_API_CALL vxCreateVirtualConvolution(vx_graph graph, vx_size columns, vx_size rows)
{
    vx_convolution convolution;
    vx_context context = vxoContext_GetFromReference((vx_reference)graph);

    gcmHEADER_ARG("graph=%p, columns=0x%lx rows=0x%lx", graph, columns, rows);
    gcmDUMP_API("$VX vxCreateVirtualConvolution: graph=%p, columns=0x%lx rows=0x%lx", graph, columns, rows);

    if (!vxoReference_IsValidAndSpecific(&graph->base, VX_TYPE_GRAPH))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    convolution = vxCreateConvolution(context, columns, rows);

    if (vxoReference_GetStatus((vx_reference)convolution) != VX_SUCCESS)
    {
        gcmFOOTER_ARG("%p", convolution);
        return convolution;
    }
    convolution->matrix.base.scope        = (vx_reference)graph;

    convolution->matrix.base.isVirtual    = vx_true_e;

    gcmFOOTER_ARG("%p", convolution);
    return convolution;
}

