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


#ifndef __GC_VX_KERNEL_H__
#define __GC_VX_KERNEL_H__

EXTERN_C_BEGIN

VX_INTERNAL_API vx_status vxoKernel_Initialize(
    vx_context context,
    vx_kernel kernel,
    const vx_char name[VX_MAX_KERNEL_NAME],
    vx_enum kernelEnum,
    vx_program program,
    vx_kernel_f function,
    vx_param_description_s * parameters,
    vx_uint32 paramCount,
    vx_kernel_input_validate_f inputValidateFunction,
    vx_kernel_output_validate_f outputValidateFunction,
    vx_kernel_initialize_f initializeFunction,
    vx_kernel_deinitialize_f deinitializeFunction
#if gcdVX_OPTIMIZER
, vx_kernel_optimization_attribute_s optAttributes
#endif
    );

VX_INTERNAL_API vx_status vxoDumpOutput(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

VX_INTERNAL_API void vxoKernel_Dump(vx_kernel kernel);

VX_INTERNAL_API vx_status vxoKernel_InternalRelease(vx_kernel_ptr kernelPtr);

VX_INTERNAL_API vx_bool vxoKernel_IsUnique(vx_kernel kernel);

VX_INTERNAL_API vx_kernel vxoKernel_GetByEnum(vx_context context, vx_enum kernelEnum);

VX_INTERNAL_API vx_status vxoKernel_ExternalRelease(vx_kernel_ptr kernelPtr);

VX_INTERNAL_CALLBACK_API void vxoKernel_Destructor(vx_reference ref);

VX_API_ENTRY vx_kernel VX_API_CALL vxAddKernelInProgramEx(
        vx_program program, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration, vx_kernel_f func_ptr,
        vx_uint32 num_params, vx_kernel_input_validate_f input,
        vx_kernel_output_validate_f output, vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize);

VX_API_ENTRY vx_status VX_CALLBACK vxProgramKernel_Function(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

EXTERN_C_END

#endif /* __GC_VX_KERNEL_H__ */

