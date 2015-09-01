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


#ifndef __GC_VX_PROGRAM_H__
#define __GC_VX_PROGRAM_H__

EXTERN_C_BEGIN

typedef enum vx_ext_program_type_e
{
    VX_TYPE_PROGRAM = 0x900
}
vx_ext_program_type_e;

VX_INTERNAL_CALLBACK_API void vxoProgram_Destructor(vx_reference ref);

VX_PUBLIC_API vx_program vxCreateProgramWithSource(
        vx_context context, vx_uint32 count, vx_const_string strings[], vx_size lengths[]);

VX_PUBLIC_API vx_program vxCreateProgramWithBinary(
        vx_context context, const vx_uint8 * binary, vx_size size);

VX_PUBLIC_API vx_status vxReleaseProgram(vx_program *program);

VX_PUBLIC_API vx_status vxBuildProgram(vx_program program, vx_const_string options);

typedef enum vx_program_attribute_e
{
    VX_PROGRAM_ATTRIBUTE_NUMKERNELS = VX_ATTRIBUTE_BASE(VX_ID_VIVANTE, VX_TYPE_PROGRAM) + 0x0,

    /* TODO */
}
vx_program_attribute_e;

VX_PUBLIC_API vx_status vxQueryProgram(vx_program program, vx_enum attribute, void *ptr, vx_size size);

VX_PUBLIC_API vx_kernel vxAddKernelInProgram(
        vx_program program, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration);

typedef enum vx_ext_kernel_attribute_e
{
    VX_KERNEL_ATTRIBUTE_GLOBAL_GROUP_SIZES = VX_ATTRIBUTE_BASE(VX_ID_VIVANTE, VX_TYPE_KERNEL) + 0x0,

    VX_KERNEL_ATTRIBUTE_LOCAL_GROUP_SIZES = VX_ATTRIBUTE_BASE(VX_ID_VIVANTE, VX_TYPE_KERNEL) + 0x1
}
vx_ext_kernel_attribute_e;

EXTERN_C_END

#endif /* __GC_VX_PROGRAM_H__ */
