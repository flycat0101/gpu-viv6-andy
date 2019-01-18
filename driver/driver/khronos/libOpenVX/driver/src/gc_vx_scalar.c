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
#include <gc_hal_user_precomp.h>
#include <gc_hal_vx.h>

VX_INTERNAL_API vx_enum vxoScalar_GetDataType(vx_scalar scalar)
{
    return scalar->dataType;
}

VX_INTERNAL_API vx_uint32 vxoScalar_GetTypeSize(vx_scalar scalar)
{
    vx_uint32 size = 0;
    switch(scalar->dataType)
    {
    case VX_TYPE_INT8:
    case VX_TYPE_UINT8:
        size = sizeof(vx_int8);
        break;
    case VX_TYPE_INT16:
    case VX_TYPE_FLOAT16:
    case VX_TYPE_UINT16:
        size = sizeof(vx_int16);
        break;
    case VX_TYPE_FLOAT32:
        size = sizeof(vx_float32);
        break;
    case VX_TYPE_INT32:
    case VX_TYPE_UINT32:
        size = sizeof(vx_int32);
        break;
    case VX_TYPE_CHAR:
        size = sizeof(vx_char);
        break;
    case VX_TYPE_INT64:
    case VX_TYPE_UINT64:
        size = sizeof(vx_uint64);
        break;
    case VX_TYPE_FLOAT64:
        size = sizeof(vx_float64);
        break;
    case VX_TYPE_DF_IMAGE:
        size = sizeof(vx_df_image);
        break;
    case VX_TYPE_ENUM:
        size = sizeof(vx_enum);
        break;
    case VX_TYPE_SIZE:
        size = sizeof(vx_size);
        break;
    case VX_TYPE_BOOL:
        size = sizeof(vx_bool);
        break;
    default:
        vxError("The value type of the scalar, %p->%d, is not supported", scalar, scalar->dataType);
        vxmASSERT(0);
        break;
    }

    return size;
}


#define vxmDUMP_SCALAR_VALUE(format, value) \
        vxTrace(VX_TRACE_SCALAR, "   <value>"format"</value>", (value))

VX_INTERNAL_API void vxoScalar_Dump(vx_scalar scalar)
{
    if (scalar == VX_NULL)
    {
        vxTrace(VX_TRACE_SCALAR, "<scalar>null</scalar>\n");
    }
    else
    {
        vxoReference_Dump((vx_reference)scalar);

        vxTrace(VX_TRACE_SCALAR,
                "<scalar>\n"
                "   <address>"VX_FORMAT_HEX"</address>\n"
                "   <dataType>"VX_FORMAT_HEX"</dataType>\n",
                scalar, scalar->dataType);

        switch (scalar->dataType)
        {
            case VX_TYPE_CHAR:
                vxmDUMP_SCALAR_VALUE("%c", scalar->value->c);
                break;

            case VX_TYPE_INT8:
                vxmDUMP_SCALAR_VALUE("%d", scalar->value->n8);
                break;

            case VX_TYPE_UINT8:
                vxmDUMP_SCALAR_VALUE("%u", scalar->value->u8);
                break;

            case VX_TYPE_INT16:
                vxmDUMP_SCALAR_VALUE("%d", scalar->value->n16);
                break;

            case VX_TYPE_UINT16:
                vxmDUMP_SCALAR_VALUE("%u", scalar->value->u16);
                break;

            case VX_TYPE_INT32:
                vxmDUMP_SCALAR_VALUE("%d", scalar->value->n32);
                break;

            case VX_TYPE_UINT32:
                vxmDUMP_SCALAR_VALUE("%u", scalar->value->u32);
                break;

            case VX_TYPE_INT64:
                vxmDUMP_SCALAR_VALUE("%ld", scalar->value->n64);
                break;

            case VX_TYPE_UINT64:
                vxmDUMP_SCALAR_VALUE("%lu", scalar->value->u64);
                break;

#if defined(OPENVX_PLATFORM_SUPPORTS_16_FLOAT)
            case VX_TYPE_FLOAT16:
                vxmDUMP_SCALAR_VALUE("%f", scalar->value->f16);
                break;
#endif

            case VX_TYPE_FLOAT32:
                vxmDUMP_SCALAR_VALUE("%f", scalar->value->f32);
                break;

            case VX_TYPE_FLOAT64:
                vxmDUMP_SCALAR_VALUE("%lf", scalar->value->f64);
                break;

            case VX_TYPE_DF_IMAGE:
                vxmDUMP_SCALAR_VALUE(VX_FORMAT_HEX, scalar->value->imageFormat);
                break;

            case VX_TYPE_ENUM:
                vxmDUMP_SCALAR_VALUE("%d", scalar->value->e);
                break;

            case VX_TYPE_SIZE:
                vxmDUMP_SCALAR_VALUE("%lu", scalar->value->s);
                break;

            case VX_TYPE_BOOL:
                vxmDUMP_SCALAR_VALUE("%s", vxmBOOL_TO_STRING(scalar->value->b));
                break;

            default:
                vxError("The value type of the scalar, %p->%d, is not supported", scalar, scalar->dataType);
                 vxmASSERT(0);
                break;
        }

        vxTrace(VX_TRACE_SCALAR, "</scalar>");
    }
}

VX_PRIVATE_API vx_status vxoScalar_CommitValue(vx_scalar scalar, const void *ptr)
{
    if (!vxoReference_IsValidAndSpecific(&scalar->base,VX_TYPE_SCALAR)) return VX_ERROR_INVALID_REFERENCE;

    if (ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    vxAcquireMutex(scalar->base.lock);

    if(vxmIS_SCALAR(scalar->dataType))
    {
        switch (scalar->dataType)
        {
            case VX_TYPE_CHAR:
                scalar->value->c = *(vx_char *)ptr;
                break;

            case VX_TYPE_INT8:
                scalar->value->n8 = *(vx_int8 *)ptr;
                break;

            case VX_TYPE_UINT8:
                scalar->value->u8 = *(vx_uint8 *)ptr;
                break;

            case VX_TYPE_INT16:
                scalar->value->n16 = *(vx_int16 *)ptr;
                break;

            case VX_TYPE_UINT16:
                scalar->value->u16 = *(vx_uint16 *)ptr;
                break;

            case VX_TYPE_INT32:
                scalar->value->n32 = *(vx_int32 *)ptr;
                break;

            case VX_TYPE_UINT32:
                scalar->value->u32 = *(vx_uint32 *)ptr;
                break;

            case VX_TYPE_INT64:
                scalar->value->n64 = *(vx_int64 *)ptr;
                break;

            case VX_TYPE_UINT64:
                scalar->value->u64 = *(vx_uint64 *)ptr;
                break;

    #if defined(OPENVX_PLATFORM_SUPPORTS_16_FLOAT)
            case VX_TYPE_FLOAT16:
                scalar->value->f16 = *(vx_float16 *)ptr;
                break;
    #endif

            case VX_TYPE_FLOAT32:
                scalar->value->f32 = *(vx_float32 *)ptr;
                break;

            case VX_TYPE_FLOAT64:
                scalar->value->f64 = *(vx_float64 *)ptr;
                break;

            case VX_TYPE_DF_IMAGE:
                scalar->value->imageFormat = *(vx_df_image *)ptr;
                break;

            case VX_TYPE_ENUM:
                scalar->value->e = *(vx_enum *)ptr;
                break;

            case VX_TYPE_SIZE:
                scalar->value->s = *(vx_size *)ptr;
                break;

            case VX_TYPE_BOOL:
                scalar->value->b = *(vx_bool *)ptr;
                break;

            default:
                vxReleaseMutex(scalar->base.lock);

                vxError("The value type of the scalar, %p->%d, is not supported", scalar, scalar->dataType);
                vxmASSERT(0);

                return VX_ERROR_NOT_SUPPORTED;
        }

        vxoScalar_Dump(scalar);

#if gcdDUMP
    {
        gctUINT32 physical = scalar->physical;
        gctPOINTER logical = scalar->value;
        gctUINT32 size = vxoScalar_GetTypeSize(scalar);

        gcmDUMP_BUFFER(gcvNULL,
                       gcvDUMP_BUFFER_MEMORY,
                       physical,
                       logical,
                       0,
                       size);
    }
#endif
    }
    else
    {
        vx_context context = vxoContext_GetFromReference((vx_reference)scalar);
        vx_int32 index = vxoContext_GetUserStructIndex(context, scalar->dataType);

        gcoOS_MemCopy(scalar->userValue, ptr, context->userStructTable[index].size);
    }

    vxReleaseMutex(scalar->base.lock);

    vxoReference_IncrementWriteCount(&scalar->base);

    return VX_SUCCESS;
}

VX_INTERNAL_API vx_scalar vxoScalar_Create(vx_context context, vx_enum dataType, const void * ptr, vx_uint32 size)
{
    vx_scalar scalar;
    vx_uint32 dataSize;
    vx_int32 index = vxoContext_GetUserStructIndex(context, dataType);

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (!vxmIS_SCALAR(dataType) && index == -1)
    {
        vxError("The value type, %d, is not a scalar type", dataType);
        return (vx_scalar)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }

    scalar = (vx_scalar)vxoReference_Create(context, VX_TYPE_SCALAR, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS) return scalar;

    scalar->dataType = dataType;

    if(vxmIS_SCALAR(dataType))
    {
        gcoVX_AllocateMemory(sizeof(vx_scalar_data), (gctPOINTER*)&scalar->value, &scalar->physical, (gcsSURF_NODE_PTR*)&scalar->node);

        gcoOS_MemFill(scalar->value, 0, sizeof(vx_scalar_data));
    }
    else
    {
        vxmASSERT(index != -1);

        dataSize = (vx_uint32)(size ? size : context->userStructTable[index].size);

        gcoVX_AllocateMemory(dataSize, (gctPOINTER*)&scalar->userValue, &scalar->physical, (gcsSURF_NODE_PTR*)&scalar->node);

        gcoOS_MemFill(scalar->userValue, 0, dataSize);
    }

    vxoScalar_CommitValue(scalar, (vx_ptr)ptr);

    return scalar;
}

void vxoReference_Destroy(vx_reference ref);

VX_INTERNAL_CALLBACK_API void vxoScalar_Destructor(vx_reference ref)
{
    vx_scalar scalar = (vx_scalar)ref;

    vxmASSERT(scalar);

    if (scalar->node != VX_NULL)
    {
        vx_context context = vxGetContext(ref);

        gcoVX_FreeMemory((gcsSURF_NODE_PTR)scalar->node);

        context->memoryCount --;

        scalar->value = VX_NULL;
        scalar->node = VX_NULL;
    }
}

VX_API_ENTRY vx_scalar VX_API_CALL vxCreateScalar(vx_context context, vx_enum dataType, const void *ptr)
{
    vx_scalar scalar;

    gcmDUMP_API("$VX vxCreateScalar: context=%p, dataType=0x%x, ptr=%p", context, dataType, ptr);

    scalar = vxoScalar_Create(context, dataType, ptr, 0);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalar reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&context->base, VX_ERROR_INVALID_REFERENCE, "%s[%d]: Get scalar reference failed!\n", __FUNCTION__, __LINE__);
        return scalar;
    }

    context->memoryCount ++;

    return scalar;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseScalar(vx_scalar *scalar)
{
    gcmDUMP_API("$VX vxReleaseScalar: scalar=%p", scalar);

    return vxoReference_Release((vx_reference *)scalar, VX_TYPE_SCALAR, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryScalar(vx_scalar scalar, vx_enum attribute, void *ptr, vx_size size)
{
    gcmDUMP_API("$VX vxQueryScalar: scalar=%p, attribute=0x%x, ptr=%p, size=0x%lx", scalar, attribute, ptr, size);

    if (!vxoReference_IsValidAndSpecific(&scalar->base,VX_TYPE_SCALAR)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_SCALAR_TYPE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = vxoScalar_GetDataType(scalar);
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

static vx_status gcoVX_ScalarToHostMem(vx_scalar scalar, void* user_ptr, vx_uint32 size)
{
    vx_status status = VX_SUCCESS;

    if (vx_false_e == vxAcquireMutex(scalar->base.lock))
        return VX_ERROR_NO_RESOURCES;

    //vxPrintScalarValue(scalar);
    if(vxmIS_SCALAR(scalar->dataType))
    {
        switch (scalar->dataType)
        {
        case VX_TYPE_CHAR:     *(vx_char*)user_ptr   = scalar->value->u8; break;
        case VX_TYPE_INT8:     *(vx_int8*)user_ptr   = scalar->value->n8; break;
        case VX_TYPE_UINT8:    *(vx_uint8*)user_ptr  = scalar->value->u8; break;
        case VX_TYPE_INT16:    *(vx_int16*)user_ptr  = scalar->value->n16; break;
        case VX_TYPE_UINT16:   *(vx_uint16*)user_ptr = scalar->value->u16; break;
        case VX_TYPE_INT32:    *(vx_int32*)user_ptr  = scalar->value->n32; break;
        case VX_TYPE_UINT32:   *(vx_uint32*)user_ptr = scalar->value->u32; break;
        case VX_TYPE_INT64:    *(vx_int64*)user_ptr  = scalar->value->n64; break;
        case VX_TYPE_UINT64:   *(vx_uint64*)user_ptr = scalar->value->u64; break;
    #if OVX_SUPPORT_HALF_FLOAT
        case VX_TYPE_FLOAT16:  *(vx_float16*)user_ptr = scalar->value->f16; break;
    #endif
        case VX_TYPE_FLOAT32:  *(vx_float32*)user_ptr  = scalar->value->f32; break;
        case VX_TYPE_FLOAT64:  *(vx_float64*)user_ptr  = scalar->value->f64; break;
        case VX_TYPE_DF_IMAGE: *(vx_df_image*)user_ptr = scalar->value->imageFormat; break;
        case VX_TYPE_ENUM:     *(vx_enum*)user_ptr     = scalar->value->e; break;
        case VX_TYPE_SIZE:     *(vx_size*)user_ptr     = scalar->value->s; break;
        case VX_TYPE_BOOL:     *(vx_bool*)user_ptr     = scalar->value->b; break;

        default:
            vxError("some case is not covered in %s\n", __FUNCTION__);
            status = VX_ERROR_NOT_SUPPORTED;
            break;
        }
    }
    else
    {
        vx_context context = vxoContext_GetFromReference((vx_reference)scalar);
        vx_int32 index = vxoContext_GetUserStructIndex(context, scalar->dataType);
        vx_uint32 dataSize = (vx_uint32)(size ? size : context->userStructTable[index].size);

        gcoOS_MemCopy(user_ptr, scalar->userValue, dataSize);
    }


    if (vx_false_e == vxReleaseMutex(scalar->base.lock))
        return VX_ERROR_NO_RESOURCES;

    vxoReference_IncrementReadCount(&scalar->base);

    return status;
} /* gcoVX_ScalarToHostMem() */

static vx_status gcoVX_HostMemToScalar(vx_scalar scalar, void* user_ptr, vx_uint32 size)
{
    vx_status status = VX_SUCCESS;

    if (vx_false_e == vxAcquireMutex(scalar->base.lock))
        return VX_ERROR_NO_RESOURCES;

    if(vxmIS_SCALAR(scalar->dataType))
    {
        switch (scalar->dataType)
        {
        case VX_TYPE_CHAR:     scalar->value->u8 = *(vx_char*)user_ptr; break;
        case VX_TYPE_INT8:     scalar->value->n8 = *(vx_int8*)user_ptr; break;
        case VX_TYPE_UINT8:    scalar->value->u8 = *(vx_uint8*)user_ptr; break;
        case VX_TYPE_INT16:    scalar->value->n16 = *(vx_int16*)user_ptr; break;
        case VX_TYPE_UINT16:   scalar->value->u16 = *(vx_uint16*)user_ptr; break;
        case VX_TYPE_INT32:    scalar->value->n32 = *(vx_int32*)user_ptr; break;
        case VX_TYPE_UINT32:   scalar->value->u32 = *(vx_uint32*)user_ptr; break;
        case VX_TYPE_INT64:    scalar->value->n64 = *(vx_int64*)user_ptr; break;
        case VX_TYPE_UINT64:   scalar->value->u64 = *(vx_uint64*)user_ptr; break;
    #if OVX_SUPPORT_HALF_FLOAT
        case VX_TYPE_FLOAT16:  scalar->value->f16 = *(vx_float16*)user_ptr; break;
    #endif
        case VX_TYPE_FLOAT32:  scalar->value->f32 = *(vx_float32*)user_ptr; break;
        case VX_TYPE_FLOAT64:  scalar->value->f64 = *(vx_float64*)user_ptr; break;
        case VX_TYPE_DF_IMAGE: scalar->value->imageFormat = *(vx_df_image*)user_ptr; break;
        case VX_TYPE_ENUM:     scalar->value->e = *(vx_enum*)user_ptr; break;
        case VX_TYPE_SIZE:     scalar->value->s = *(vx_size*)user_ptr; break;
        case VX_TYPE_BOOL:     scalar->value->b = *(vx_bool*)user_ptr; break;

        default:
            vxError("some case is not covered in %s\n", __FUNCTION__);
            status = VX_ERROR_NOT_SUPPORTED;
            break;
        }
    }
    else
    {
        vx_context context = vxoContext_GetFromReference((vx_reference)scalar);
        vx_int32 index = vxoContext_GetUserStructIndex(context, scalar->dataType);
        vx_uint32 dataSize = (vx_uint32)(size ? size : context->userStructTable[index].size);

        gcoOS_MemCopy(scalar->userValue, user_ptr, dataSize);
    }

    //vxPrintScalarValue(scalar);

    if (vx_false_e == vxReleaseMutex(scalar->base.lock))
        return VX_ERROR_NO_RESOURCES;

    vxoReference_IncrementWriteCount(&scalar->base);

    return status;
} /* gcoVX_HostMemToScalar() */

VX_API_ENTRY vx_status VX_API_CALL vxCopyScalar(vx_scalar scalar, void* user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;

    gcmDUMP_API("$VX vxReleaseScalar: scalar=%p, user_ptr=%p, usage=0x%x, user_mem_type=0x%x", scalar, user_ptr, usage, user_mem_type);

    if (!vxoReference_IsValidAndSpecific(&scalar->base,VX_TYPE_SCALAR)) return VX_ERROR_INVALID_REFERENCE;

    if (NULL == user_ptr || VX_MEMORY_TYPE_HOST != user_mem_type)
        return VX_ERROR_INVALID_PARAMETERS;

    switch (usage)
    {
    case VX_READ_ONLY:  status = gcoVX_ScalarToHostMem(scalar, user_ptr, 0);  break;
    case VX_WRITE_ONLY: status = gcoVX_HostMemToScalar(scalar, user_ptr, 0); break;

    default:
        status = VX_ERROR_INVALID_PARAMETERS;
        break;
    }

    return status;
} /* vxCopyScalar() */

VX_API_ENTRY vx_status VX_API_CALL vxReadScalarValue(vx_scalar scalar, void *ptr)
{
    gcmDUMP_API("$VX vxReadScalarValue: scalar=%p, user_ptr=%p", scalar, ptr);

    if (!vxoReference_IsValidAndSpecific(&scalar->base,VX_TYPE_SCALAR)) return VX_ERROR_INVALID_REFERENCE;

    if (ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    vxAcquireMutex(scalar->base.lock);

    if(vxmIS_SCALAR(scalar->dataType))
    {
        switch (scalar->dataType)
        {
            case VX_TYPE_CHAR:
                *(vx_char *)ptr = scalar->value->c;
                break;

            case VX_TYPE_INT8:
                *(vx_int8 *)ptr = scalar->value->n8;
                break;

            case VX_TYPE_UINT8:
                *(vx_uint8 *)ptr = scalar->value->u8;
                break;

            case VX_TYPE_INT16:
                *(vx_int16 *)ptr = scalar->value->n16;
                break;

            case VX_TYPE_UINT16:
                *(vx_uint16 *)ptr = scalar->value->u16;
                break;

            case VX_TYPE_INT32:
                *(vx_int32 *)ptr = scalar->value->n32;
                break;

            case VX_TYPE_UINT32:
                *(vx_uint32 *)ptr = scalar->value->u32;
                break;

            case VX_TYPE_INT64:
                *(vx_int64 *)ptr = scalar->value->n64;
                break;

            case VX_TYPE_UINT64:
                *(vx_uint64 *)ptr = scalar->value->u64;
                break;

    #if defined(OPENVX_PLATFORM_SUPPORTS_16_FLOAT)
            case VX_TYPE_FLOAT16:
                *(vx_float16 *)ptr = scalar->value->f16;
                break;
    #endif

            case VX_TYPE_FLOAT32:
                *(vx_float32 *)ptr = scalar->value->f32;
                break;

            case VX_TYPE_FLOAT64:
                *(vx_float64 *)ptr = scalar->value->f64;
                break;

            case VX_TYPE_DF_IMAGE:
                *(vx_df_image *)ptr = scalar->value->imageFormat;
                break;

            case VX_TYPE_ENUM:
                *(vx_enum *)ptr = scalar->value->e;
                break;

            case VX_TYPE_SIZE:
                *(vx_size *)ptr = scalar->value->s;
                break;

            case VX_TYPE_BOOL:
                *(vx_bool *)ptr = scalar->value->b;
                break;

            default:
                vxReleaseMutex(scalar->base.lock);

                vxError("The value type of the scalar, %p->%d, is not supported", scalar, scalar->dataType);
                vxmASSERT(0);

                return VX_ERROR_NOT_SUPPORTED;
        }
    }
     else
    {
        vx_context context = vxoContext_GetFromReference((vx_reference)scalar);
        vx_int32 index = vxoContext_GetUserStructIndex(context, scalar->dataType);

        gcoOS_MemCopy(ptr, scalar->userValue, context->userStructTable[index].size);
    }

    vxReleaseMutex(scalar->base.lock);

    vxoReference_IncrementReadCount(&scalar->base);

    vxoScalar_Dump(scalar);

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxWriteScalarValue(vx_scalar scalar, const void *ptr)
{
    gcmDUMP_API("$VX vxWriteScalarValue: scalar=%p, ptr=%p", scalar, ptr);

    return vxoScalar_CommitValue(scalar, ptr);
}

VX_API_ENTRY vx_scalar VX_API_CALL vxCreateScalarWithSize(vx_context context, vx_enum data_type, const void *ptr, vx_size size)
{
    vx_scalar scalar;

    gcmDUMP_API("$VX vxCreateScalarWithSize: context=%p, data_type=0x%x, ptr=%p, size=0x%lx", context, data_type, ptr, size);

    scalar = vxoScalar_Create(context, data_type, ptr, (vx_uint32)size);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS) return scalar;

    context->memoryCount ++;

    return scalar;
}

VX_API_ENTRY vx_scalar VX_API_CALL vxCreateVirtualScalar(vx_graph graph, vx_enum data_type)
{
    vx_scalar scalar;
    vx_context context = vxoContext_GetFromReference((vx_reference)graph);

    gcmDUMP_API("$VX vxCreateVirtualScalar: graph=%p, data_type=0x%x", graph, data_type);

    scalar = vxoScalar_Create(context, data_type, VX_NULL, 0);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS) return scalar;

    scalar->base.isVirtual = vx_true_e;

    scalar->base.scope = (vx_reference)graph;

    return scalar;
}

VX_API_ENTRY vx_status VX_API_CALL vxCopyScalarWithSize(vx_scalar scalar, vx_size size, void *user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;

    gcmDUMP_API("$VX vxCopyScalarWithSize: scalar=%p, size=0x%lx, usage=0x%x, user_mem_type=0x%x", scalar, size, usage, user_mem_type);

    if (!vxoReference_IsValidAndSpecific(&scalar->base,VX_TYPE_SCALAR)) return VX_ERROR_INVALID_REFERENCE;

    if (NULL == user_ptr || VX_MEMORY_TYPE_HOST != user_mem_type)
        return VX_ERROR_INVALID_PARAMETERS;

    switch (usage)
    {
    case VX_READ_ONLY:  status = gcoVX_ScalarToHostMem(scalar, user_ptr, (vx_uint32)size);  break;
    case VX_WRITE_ONLY: status = gcoVX_HostMemToScalar(scalar, user_ptr, (vx_uint32)size); break;

    default:
        status = VX_ERROR_INVALID_PARAMETERS;
        break;
    }

    return status;
}

