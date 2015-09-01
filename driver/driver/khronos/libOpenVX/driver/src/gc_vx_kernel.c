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

VX_INTERNAL_API vx_status vxoKernel_Initialize(
        vx_context context, vx_kernel kernel,
        vx_char name[VX_MAX_KERNEL_NAME], vx_enum kernelEnum,
        vx_program program, vx_kernel_f function,
        vx_param_description_s *parameters, vx_uint32 paramCount,
        vx_kernel_input_validate_f inputValidateFunction, vx_kernel_output_validate_f outputValidateFunction,
        vx_kernel_initialize_f initializeFunction,  vx_kernel_deinitialize_f deinitializeFunction)
{
    vx_uint32 i;

    vxmASSERT(context);
    vxmASSERT(paramCount <= VX_MAX_PARAMETERS);

    if (kernel == VX_NULL) return VX_FAILURE;

    vxoReference_Initialize(&kernel->base, context, VX_TYPE_KERNEL, &context->base);

    vxoContext_AddObject(context, &kernel->base);

    vxoReference_Increment(&kernel->base, VX_REF_INTERNAL);

    vxStrCopySafe(kernel->name, VX_MAX_KERNEL_NAME, name);
    kernel->enumeration             = kernelEnum;

    kernel->program                 = program;

    kernel->function                = function;

    kernel->signature.paramCount    = paramCount;

    kernel->inputValidateFunction   = inputValidateFunction;
    kernel->outputValidateFunction  = outputValidateFunction;
    kernel->initializeFunction      = initializeFunction;
    kernel->deinitializeFunction    = deinitializeFunction;

    kernel->attributes.borderMode.mode              = VX_BORDER_MODE_UNDEFINED;
    kernel->attributes.borderMode.constant_value    = 0;

    if (parameters != VX_NULL)
    {
        for (i = 0; i < paramCount; i++)
        {
            kernel->signature.directionTable[i] = parameters[i].direction;
            kernel->signature.dataTypeTable[i]  = parameters[i].dataType;
            kernel->signature.stateTable[i]     = parameters[i].state;
        }

        kernel->enabled = vx_true_e;
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API void vxoKernel_Dump(vx_kernel kernel)
{
    if (kernel == VX_NULL)
    {
        vxTrace(VX_TRACE_KERNEL, "<kernel>null</kernel>\n");
    }
    else
    {
        vxoReference_Dump((vx_reference)kernel);

        vxTrace(VX_TRACE_KERNEL,
                "<kernel>\n"
                "   <address>"VX_FORMAT_HEX"</address>\n"
                "   <name>%s</name>\n"
                "   <enumeration>"VX_FORMAT_HEX"</enumeration>\n"
                "   <enabled>%s</enabled>\n"
                "</kernel>",
                kernel, kernel->name, kernel->enumeration, vxmBOOL_TO_STRING(kernel->enabled));
    }
}

VX_INTERNAL_API vx_status vxoKernel_InternalRelease(vx_kernel_ptr kernelPtr)
{
    vx_kernel kernel;

    vxmASSERT(kernelPtr);

    kernel = *kernelPtr;

    if (kernel == VX_NULL) return VX_ERROR_INVALID_REFERENCE;

    *kernelPtr = VX_NULL;

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    return vxoReference_Release((vx_reference_ptr)&kernel, VX_TYPE_KERNEL, VX_REF_INTERNAL);
}

VX_INTERNAL_API vx_status vxoKernel_ExternalRelease(vx_kernel_ptr kernelPtr)
{
    vx_kernel kernel;

    vxmASSERT(kernelPtr);

    kernel = *kernelPtr;

    if (kernel == VX_NULL) return VX_ERROR_INVALID_REFERENCE;

    *kernelPtr = VX_NULL;

    return vxoReference_Release((vx_reference_ptr)&kernel, VX_TYPE_KERNEL, VX_REF_EXTERNAL);
}

VX_PRIVATE_API vx_status vxoKernel_Remove(vx_kernel kernel)
{
    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    if (kernel->enabled)
    {
        vxError("Can't remove the enabled kernel, \"%s\"", kernel->name);
        return VX_ERROR_INVALID_REFERENCE;
    }

    kernel->enumeration = VX_KERNEL_INVALID;
    kernel->base.context->kernelCount--;

    return vxoReference_Release((vx_reference*)&kernel, VX_TYPE_KERNEL, VX_REF_INTERNAL);
}

VX_INTERNAL_API vx_bool vxoKernel_IsUnique(vx_kernel kernel)
{
    vx_context context;
    vx_uint32 i, k;

    vxmASSERT(kernel);

    context = kernel->base.context;

    vxmASSERT(context);

    for (i = 0u; i < context->targetCount; i++)
    {
        for (k = 0u; k < context->targetTable[i].kernelCount; k++)
        {
            if (context->targetTable[i].kernelTable[k].enabled
                && context->targetTable[i].kernelTable[k].enumeration == kernel->enumeration)
            {
                return vx_false_e;
            }
        }
    }

    return vx_true_e;
}

VX_INTERNAL_API vx_kernel vxoKernel_GetByEnum(vx_context context, vx_enum kernelEnum)
{
    vx_uint32 targetIndex, kernelIndex;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (kernelEnum < VX_KERNEL_INVALID)
    {
        return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[context->targetPriorityTable[targetIndex]];

        if (target == VX_NULL || !target->enabled) continue;

        for (kernelIndex = 0; kernelIndex < target->kernelCount; kernelIndex++)
        {
            if (target->kernelTable[kernelIndex].enumeration == kernelEnum)
            {
                vx_kernel kernel = &target->kernelTable[kernelIndex];

                if (!kernel->enabled) continue;

                kernel->targetIndex = context->targetPriorityTable[targetIndex];

                vxoReference_Increment(&kernel->base, VX_REF_EXTERNAL);

                vxoKernel_Dump(kernel);

                return kernel;
            }
        }
    }

    vxError("Kernel enum %d does not exist", kernelEnum);

    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_PRIVATE_API vx_size vxString_GetCharIndex(vx_const_string string, vx_char ch, vx_size limit)
{
    vx_size index;

    vxmASSERT(string);

    for (index = 0; index < limit; index++)
    {
        if (string[index] == '\0') return limit;

        if (string[index] == ch) break;
    }

    return index;
}

VX_PRIVATE_API vx_size vxString_GetCharCount(vx_const_string string, vx_size size, vx_char ch)
{
    vx_size index;
    vx_size count = 0;

    for (index = 0; index < size; index++)
    {
        if (string[index] == '\0') break;

        if (string[index] == ch) count++;
    }

    return count;
}

VX_PUBLIC_API vx_status vxLoadKernels(vx_context context, vx_char *module)
{
    return vxContext_LoadKernels(context, module);
}

VX_PUBLIC_API vx_kernel vxGetKernelByName(vx_context context, vx_char string[VX_MAX_KERNEL_NAME])
{
    vx_char     tempString[VX_MAX_KERNEL_NAME];
    vx_uint32   targetIndex;

    vx_char     targetName[VX_MAX_TARGET_NAME] = "default";
    vx_char     kernelName[VX_MAX_KERNEL_NAME];

#if defined(OPENVX_USE_VARIANTS)
    vx_char     variantName[VX_MAX_VARIANT_NAME] = "default";

#if defined(OPENVX_USE_TARGET)
    vx_char     defaultTargets[][VX_MAX_TARGET_NAME] = {
        "default",
        "power",
        "performance",
        "memory",
        "bandwidth",
    };
#endif

#endif

    if (!vxoContext_IsValid(context)) return VX_NULL;

    vxStrCopySafe(tempString, VX_MAX_KERNEL_NAME, string);

    switch (vxString_GetCharCount(string, VX_MAX_KERNEL_NAME, ':'))
    {
        case 0:
            vxStrCopySafe(kernelName, VX_MAX_KERNEL_NAME, string);
            break;

        case 1:
            {
#if defined(OPENVX_USE_TARGET) || defined(OPENVX_USE_VARIANTS)

                vx_string   firstName   = strtok(tempString, ":");
                vx_string   lastName    = strtok(VX_NULL, ":");

#if defined(OPENVX_USE_TARGET) && defined(OPENVX_USE_VARIANTS)

                vx_bool     isTarget = vx_false_e;

                for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
                {
                    if (vxIsSameString(firstName, context->targetTable[targetIndex].name, VX_MAX_TARGET_NAME))
                    {
                        isTarget = vx_true_e;
                        break;
                    }
                }

                if (!isTarget)
                {
                    for (targetIndex = 0u; targetIndex < vxmLENGTH_OF(defaultTargets); targetIndex++)
                    {
                        if (vxIsSameString(firstName, defaultTargets[targetIndex], VX_MAX_TARGET_NAME))
                        {
                            isTarget = vx_true_e;
                            break;
                        }
                    }
                }

                if (isTarget)
                {
                    vxStrCopySafe(targetName,   VX_MAX_TARGET_NAME, firstName);
                    vxStrCopySafe(kernelName,   VX_MAX_KERNEL_NAME, lastName);
                }
                else
                {
                    vxStrCopySafe(kernelName,   VX_MAX_KERNEL_NAME, firstName);
                    vxStrCopySafe(variantName,  VX_MAX_VARIANT_NAME, lastName);
                }

#elif defined(OPENVX_USE_TARGET)

                vxStrCopySafe(targetName,   VX_MAX_TARGET_NAME, firstName);
                vxStrCopySafe(kernelName,   VX_MAX_KERNEL_NAME, lastName);

#elif defined(OPENVX_USE_VARIANTS)

                vxStrCopySafe(kernelName,   VX_MAX_KERNEL_NAME, firstName);
                vxStrCopySafe(variantName,  VX_MAX_VARIANT_NAME, lastName);

#endif /* defined(OPENVX_USE_TARGET) && defined(OPENVX_USE_VARIANTS) */

#else
                vxError("Invalid kernel name: \"%s\"", string);
                return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
#endif /* defined(OPENVX_USE_TARGET) || defined(OPENVX_USE_VARIANTS) */
            }
            break;

        case 2:
            {
#if defined(OPENVX_USE_TARGET) && defined(OPENVX_USE_VARIANTS)
                vx_string target    = strtok(tempString, ":");
                vx_string kernel    = strtok(VX_NULL, ":");
                vx_string variant   = strtok(VX_NULL, ":");

                vxStrCopySafe(targetName,   VX_MAX_TARGET_NAME, target);
                vxStrCopySafe(kernelName,   VX_MAX_KERNEL_NAME, kernel);
                vxStrCopySafe(variantName,  VX_MAX_VARIANT_NAME, variant);
#else
                vxError("Invalid kernel name: \"%s\"", string);
                return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
#endif
            }
            break;

        default:
            vxError("Invalid kernel name: \"%s\"", string);
            return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_status status;
        vx_uint32 kernelIndex;
        vx_target target = &context->targetTable[context->targetPriorityTable[targetIndex]];
        vx_kernel kernel;

        if (target == VX_NULL || !target->enabled) continue;

        status = target->funcs.iskernelsupported(target, targetName, kernelName,
#if defined(OPENVX_USE_VARIANTS)
                                                    variantName,
#endif
                                                    &kernelIndex);

        if (status != VX_SUCCESS) continue;

        kernel = &target->kernelTable[kernelIndex];

        if (!kernel->enabled) continue;

        kernel->targetIndex = context->targetPriorityTable[targetIndex];

        vxoReference_Increment(&kernel->base, VX_REF_EXTERNAL);

        vxoKernel_Dump(kernel);

        return kernel;
    }

    vxError("Kernel \"%s\" does not exist", string);

    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_PUBLIC_API vx_kernel vxGetKernelByEnum(vx_context context, vx_enum kernel_enum)
{
   return vxoKernel_GetByEnum(context, kernel_enum);
}

VX_PUBLIC_API vx_status vxReleaseKernel(vx_kernel *kernel)
{
    return vxoKernel_ExternalRelease(kernel);
}

VX_PUBLIC_API vx_kernel vxAddKernel(
        vx_context context, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_kernel_f func_ptr, vx_uint32 num_params, vx_kernel_input_validate_f input,
        vx_kernel_output_validate_f output, vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{
    vx_size     colonCharIndex;
    vx_uint32   targetIndex;
    vx_char     targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (name == VX_NULL || strlen(name) == 0) goto ErrorExit;

    if (func_ptr == VX_NULL) goto ErrorExit;

    if (num_params == 0 || num_params > VX_MAX_PARAMETERS) goto ErrorExit;

    /* The initialize and de-initialize function can be null */
    if (input == VX_NULL || output == VX_NULL) goto ErrorExit;

    colonCharIndex = vxString_GetCharIndex(name, ':', VX_MAX_TARGET_NAME);

    if (colonCharIndex != VX_MAX_TARGET_NAME) vxStrCopySafe(targetName, colonCharIndex, name);

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
        {
            vxmASSERT(target->funcs.addkernel);

            return target->funcs.addkernel(target, name, enumeration,
                                            VX_NULL, func_ptr, num_params,
                                            input, output, initialize, deinitialize);
        }
    }

    vxError("Faild to find target \"%s\" for vxAddKernel", targetName);

ErrorExit:
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_PUBLIC_API vx_kernel vxAddTilingKernel(
        vx_context context, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_tiling_kernel_f flexible_func_ptr, vx_tiling_kernel_f fast_func_ptr,
        vx_uint32 num_params, vx_kernel_input_validate_f input, vx_kernel_output_validate_f output)
{
    vx_size     colonCharIndex;
    vx_uint32   targetIndex;
    vx_char     targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (name == VX_NULL || strlen(name) == 0) goto ErrorExit;

    if (flexible_func_ptr == VX_NULL && fast_func_ptr == VX_NULL) goto ErrorExit;

    if (num_params == 0 || num_params > VX_MAX_PARAMETERS) goto ErrorExit;

    /* The initialize and de-initialize function can be null */
    if (input == VX_NULL || output == VX_NULL) goto ErrorExit;

    colonCharIndex = vxString_GetCharIndex(name, ':', VX_MAX_TARGET_NAME);

    if (colonCharIndex != VX_MAX_TARGET_NAME) vxStrCopySafe(targetName, colonCharIndex, name);

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
        {
            if (target && target->funcs.addtilingkernel != VX_NULL)
            {
                return target->funcs.addtilingkernel(target, name, enumeration,
                                                    flexible_func_ptr, fast_func_ptr,
                                                    num_params, input, output);
            }
        }
    }

    vxError("Faild to find target \"%s\" for vxAddTilingKernel", targetName);

ErrorExit:
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_PUBLIC_API vx_status vxRemoveKernel(vx_kernel kernel)
{
    return vxoKernel_Remove(kernel);
}

VX_PUBLIC_API vx_status vxAddParameterToKernel(
        vx_kernel kernel, vx_uint32 index, vx_enum dir, vx_enum dataType, vx_enum state)
{
    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    if (index >= kernel->signature.paramCount) return VX_ERROR_INVALID_PARAMETERS;

    if (kernel->tilingFunction == VX_NULL)
    {
        if (!vxDataType_IsValid(dataType)) return VX_ERROR_INVALID_PARAMETERS;
    }
    else
    {
        if (dataType != VX_TYPE_IMAGE && dataType != VX_TYPE_SCALAR) return VX_ERROR_INVALID_PARAMETERS;
    }

    if (!vxmIS_VALID_DIRECTION_FOR_USER_KERNEL(dir) || !vxmIS_VALID_STATE(state))
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    kernel->signature.directionTable[index] = dir;
    kernel->signature.dataTypeTable[index]  = dataType;
    kernel->signature.stateTable[index]     = state;

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxFinalizeKernel(vx_kernel kernel)
{
    vx_uint32 i;

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    for (i = 0; i < kernel->signature.paramCount; i++)
    {
        if (!vxmIS_VALID_DIRECTION(kernel->signature.directionTable[i])
            || !vxDataType_IsValid(kernel->signature.dataTypeTable[i]))
        {
            return VX_ERROR_INVALID_PARAMETERS;
        }
    }

    kernel->base.context->kernelCount++;

    if (vxoKernel_IsUnique(kernel))
    {
        kernel->base.context->uniqueKernelCount++;
    }

    kernel->enabled = vx_true_e;

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxQueryKernel(vx_kernel kernel, vx_enum attribute, void *ptr, vx_size size)
{
    vx_char name[VX_MAX_KERNEL_NAME];
    vx_char *namePtr;

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_KERNEL_ATTRIBUTE_PARAMETERS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = kernel->signature.paramCount;
            break;

        case VX_KERNEL_ATTRIBUTE_NAME:
            if (ptr == NULL || size > VX_MAX_KERNEL_NAME) return VX_ERROR_INVALID_PARAMETERS;

            vxStrCopySafe(name, VX_MAX_KERNEL_NAME, kernel->name);

            namePtr = strtok(name, ":");

            vxStrCopySafe((vx_string)ptr, VX_MAX_KERNEL_NAME, namePtr);
            break;

        case VX_KERNEL_ATTRIBUTE_ENUM:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = kernel->enumeration;
            break;

        case VX_KERNEL_ATTRIBUTE_LOCAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = kernel->attributes.localDataSize;
            break;

#ifdef OPENVX_KHR_TILING
        case VX_KERNEL_ATTRIBUTE_INPUT_NEIGHBORHOOD:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_neighborhood_size_t, 0x3);

            *(vx_neighborhood_size_t *)ptr = kernel->attributes.inputNeighborhoodSize;
            break;

        case VX_KERNEL_ATTRIBUTE_OUTPUT_TILE_BLOCK_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_tile_block_size_t, 0x3);

            *(vx_tile_block_size_t *)ptr = kernel->attributes.tileBlockSize;
            break;
#endif

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_status vxSetKernelAttribute(vx_kernel kernel, vx_enum attribute, void *ptr, vx_size size)
{
    vx_border_mode_t *borderMode;

    if (!vxoReference_IsValidAndSpecific(&kernel->base, VX_TYPE_KERNEL)) return VX_ERROR_INVALID_REFERENCE;

    if (kernel->enabled) return VX_ERROR_NOT_SUPPORTED;

    switch (attribute)
    {
        case VX_KERNEL_ATTRIBUTE_LOCAL_DATA_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            kernel->attributes.localDataSize = *(vx_size *)ptr;
            break;

        case VX_KERNEL_ATTRIBUTE_LOCAL_DATA_PTR:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_ptr, 0x3);

            kernel->attributes.localDataPtr = *(vx_ptr *)ptr;
            break;

#ifdef OPENVX_KHR_TILING
        case VX_KERNEL_ATTRIBUTE_INPUT_NEIGHBORHOOD:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_neighborhood_size_t, 0x3);

            kernel->attributes.inputNeighborhoodSize = *(vx_neighborhood_size_t *)ptr;
            break;

        case VX_KERNEL_ATTRIBUTE_OUTPUT_TILE_BLOCK_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_tile_block_size_t, 0x3);

            kernel->attributes.tileBlockSize = *(vx_tile_block_size_t *)ptr;
            break;

        case VX_KERNEL_ATTRIBUTE_BORDER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_mode_t, 0x3);

            borderMode = (vx_border_mode_t *)ptr;

            switch (borderMode->mode)
            {
                case VX_BORDER_MODE_SELF:
                case VX_BORDER_MODE_UNDEFINED:
                    break;

                default:
                    vxError("Unsupported border mode: %d", borderMode->mode);
                    return VX_ERROR_INVALID_VALUE;
            }

            kernel->attributes.borderMode = *borderMode;
            break;
#endif

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoProgramKernel_Function(vx_node node, vx_reference parameters[], vx_uint32 paramCount)
{
    /* TODO */

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoProgramKernel_InputValidator(vx_node node, vx_uint32 index)
{
    /* TODO */

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoProgramKernel_OutputValidator(vx_node node, vx_uint32 index, vx_meta_format meta)
{
    /* TODO */

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoProgramKernel_Initialize(vx_node node, vx_reference parameters[], vx_uint32 paramCount)
{
    /* TODO */

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoProgramKernel_Deinitialize(vx_node node, vx_reference parameters[], vx_uint32 paramCount)
{
    /* TODO */

    return VX_SUCCESS;
}

VX_PUBLIC_API vx_kernel vxAddKernelInProgram(
        vx_program program, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration)
{
    vx_context  context;
    vx_uint32   targetIndex;
    vx_char     targetName[VX_MAX_TARGET_NAME] = VX_DEFAULT_TARGET_NAME;

    if (!vxoReference_IsValidAndSpecific(&program->base, VX_TYPE_PROGRAM)) return VX_NULL;

    context = program->base.context;

    if (name == VX_NULL || strlen(name) == 0) goto ErrorExit;

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        if (vxIsSameString(targetName, target->name, VX_MAX_TARGET_NAME))
        {
            vxmASSERT(target->funcs.addkernel);

            return target->funcs.addkernel(target, name, enumeration,
                                            program, vxoProgramKernel_Function, 0,
                                            vxoProgramKernel_InputValidator, vxoProgramKernel_OutputValidator,
                                            vxoProgramKernel_Initialize, vxoProgramKernel_Deinitialize);
        }
    }

    vxError("Faild to find target \"%s\" for vxAddKernelInProgram", targetName);

ErrorExit:
    return (vx_kernel)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}
