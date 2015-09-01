/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
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

VX_INTERNAL_CALLBACK_API void vxoProgram_Destructor(vx_reference ref)
{
    /* TODO */
}

VX_PUBLIC_API vx_program vxCreateProgramWithSource(
        vx_context context, vx_uint32 count, const vx_char * strings[], vx_size lengths[])
{
    vx_program program;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    program = (vx_program)vxoReference_Create(context, VX_TYPE_PROGRAM, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)program) != VX_SUCCESS) return program;

    /* TODO */

    return program;
}

VX_PUBLIC_API vx_program vxCreateProgramWithBinary(
        vx_context context, const vx_uint8 * binary, vx_size size)
{
    vx_program program;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    program = (vx_program)vxoReference_Create(context, VX_TYPE_PROGRAM, VX_REF_EXTERNAL, &context->base);

    if (vxoReference_GetStatus((vx_reference)program) != VX_SUCCESS) return program;

    /* TODO */

    return program;
}

VX_PUBLIC_API vx_status vxReleaseProgram(vx_program *program)
{
    return vxoReference_Release((vx_reference_ptr)program, VX_TYPE_GRAPH, VX_REF_EXTERNAL);
}

VX_PUBLIC_API vx_status vxBuildProgram(vx_program program, vx_const_string options)
{
    if (!vxoReference_IsValidAndSpecific(&program->base, VX_TYPE_PROGRAM)) return VX_ERROR_INVALID_REFERENCE;

    /* TODO */

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxQueryProgram(vx_program program, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoReference_IsValidAndSpecific(&program->base, VX_TYPE_PROGRAM)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_PROGRAM_ATTRIBUTE_NUMKERNELS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            /* TODO */
            *(vx_uint32 *)ptr = 0;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}
