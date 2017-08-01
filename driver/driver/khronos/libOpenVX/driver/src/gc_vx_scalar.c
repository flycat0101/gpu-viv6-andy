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
#include <gc_hal_user_precomp.h>
#include <gc_hal_vx.h>

VX_INTERNAL_API vx_enum vxoScalar_GetDataType(vx_scalar scalar)
{
    return scalar->dataType;
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

    vxReleaseMutex(scalar->base.lock);

    vxoReference_IncrementWriteCount(&scalar->base);

    vxoScalar_Dump(scalar);

    return VX_SUCCESS;
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

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (!vxmIS_SCALAR(dataType))
    {
        vxError("The value type, %d, is not a scalar type", dataType);
        return (vx_scalar)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_TYPE);
    }

    scalar = (vx_scalar)vxoReference_Create(context, VX_TYPE_SCALAR, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS) return scalar;

    scalar->dataType = dataType;

    gcoVX_AllocateMemory(sizeof(vx_scalar_data), (gctPOINTER*)&scalar->value, (gctPHYS_ADDR*)&scalar->physical, (gcsSURF_NODE_PTR*)&scalar->node);

    context->memoryCount ++;

    memset(scalar->value, 0, sizeof(vx_scalar_data));

    vxoScalar_CommitValue(scalar, (vx_ptr)ptr);

    vxoScalar_Dump(scalar);

    return scalar;
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseScalar(vx_scalar *scalar)
{
    return vxoReference_Release((vx_reference *)scalar, VX_TYPE_SCALAR, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryScalar(vx_scalar scalar, vx_enum attribute, void *ptr, vx_size size)
{
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

static vx_status gcoVX_ScalarToHostMem(vx_scalar scalar, void* user_ptr)
{
    vx_status status = VX_SUCCESS;

    if (vx_false_e == vxAcquireMutex(scalar->base.lock))
        return VX_ERROR_NO_RESOURCES;

    //vxPrintScalarValue(scalar);

    switch (scalar->dataType)
    {
    case VX_TYPE_CHAR:     *(vx_char*)user_ptr = scalar->value->u8; break;
    case VX_TYPE_INT8:     *(vx_int8*)user_ptr = scalar->value->n8; break;
    case VX_TYPE_UINT8:    *(vx_uint8*)user_ptr = scalar->value->u8; break;
    case VX_TYPE_INT16:    *(vx_int16*)user_ptr = scalar->value->n16; break;
    case VX_TYPE_UINT16:   *(vx_uint16*)user_ptr = scalar->value->u16; break;
    case VX_TYPE_INT32:    *(vx_int32*)user_ptr = scalar->value->n32; break;
    case VX_TYPE_UINT32:   *(vx_uint32*)user_ptr = scalar->value->u32; break;
    case VX_TYPE_INT64:    *(vx_int64*)user_ptr = scalar->value->n64; break;
    case VX_TYPE_UINT64:   *(vx_uint64*)user_ptr = scalar->value->u64; break;
#if OVX_SUPPORT_HALF_FLOAT
    case VX_TYPE_FLOAT16:  *(vx_float16*)ptr = scalar->data.f16; break;
#endif
    case VX_TYPE_FLOAT32:  *(vx_float32*)user_ptr = scalar->value->f32; break;
    case VX_TYPE_FLOAT64:  *(vx_float64*)user_ptr = scalar->value->f64; break;
    case VX_TYPE_DF_IMAGE: *(vx_df_image*)user_ptr = scalar->value->imageFormat; break;
    case VX_TYPE_ENUM:     *(vx_enum*)user_ptr = scalar->value->e; break;
    case VX_TYPE_SIZE:     *(vx_size*)user_ptr = scalar->value->s; break;
    case VX_TYPE_BOOL:     *(vx_bool*)user_ptr = scalar->value->b; break;

    default:
        vxError("some case is not covered in %s\n", __FUNCTION__);
        status = VX_ERROR_NOT_SUPPORTED;
        break;
    }

    if (vx_false_e == vxReleaseMutex(scalar->base.lock))
        return VX_ERROR_NO_RESOURCES;

    vxoReference_IncrementReadCount(&scalar->base);

    return status;
} /* gcoVX_ScalarToHostMem() */

static vx_status gcoVX_HostMemToScalar(vx_scalar scalar, void* user_ptr)
{
    vx_status status = VX_SUCCESS;

    if (vx_false_e == vxAcquireMutex(scalar->base.lock))
        return VX_ERROR_NO_RESOURCES;

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
    case VX_TYPE_FLOAT16:  scalar->data.f16 = *(vx_float16*)user_ptr; break;
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

    //vxPrintScalarValue(scalar);

    if (vx_false_e == vxReleaseMutex(scalar->base.lock))
        return VX_ERROR_NO_RESOURCES;

    vxoReference_IncrementWriteCount(&scalar->base);

    return status;
} /* gcoVX_HostMemToScalar() */

VX_API_ENTRY vx_status VX_API_CALL vxCopyScalar(vx_scalar scalar, void* user_ptr, vx_enum usage, vx_enum user_mem_type)
{
    vx_status status = VX_SUCCESS;

    if (!vxoReference_IsValidAndSpecific(&scalar->base,VX_TYPE_SCALAR)) return VX_ERROR_INVALID_REFERENCE;

    if (NULL == user_ptr || VX_MEMORY_TYPE_HOST != user_mem_type)
        return VX_ERROR_INVALID_PARAMETERS;

    switch (usage)
    {
    case VX_READ_ONLY:  status = gcoVX_ScalarToHostMem(scalar, user_ptr);  break;
    case VX_WRITE_ONLY: status = gcoVX_HostMemToScalar(scalar, user_ptr); break;

    default:
        status = VX_ERROR_INVALID_PARAMETERS;
        break;
    }

    return status;
} /* vxCopyScalar() */

VX_API_ENTRY vx_status VX_API_CALL vxReadScalarValue(vx_scalar scalar, void *ptr)
{
    if (!vxoReference_IsValidAndSpecific(&scalar->base,VX_TYPE_SCALAR)) return VX_ERROR_INVALID_REFERENCE;

    if (ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

    vxAcquireMutex(scalar->base.lock);

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

    vxReleaseMutex(scalar->base.lock);

    vxoReference_IncrementReadCount(&scalar->base);

    vxoScalar_Dump(scalar);

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxWriteScalarValue(vx_scalar scalar, const void *ptr)
{
    return vxoScalar_CommitValue(scalar, ptr);
}

