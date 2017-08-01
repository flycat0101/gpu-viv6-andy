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
#include <stdio.h>
#include <gc_hal_user_precomp.h>
#include <gc_hal_vx.h>

extern vx_kernel_description_s *    target_kernels[];
extern vx_uint32                    num_target_kernels;



VX_PRIVATE_API vx_status vxInitializeTarget(
    vx_target target,
    vx_kernel_description_s *kernelDescTable[],
    vx_uint32 kernelCount)
{
    vx_uint32   index;

    vxmASSERT(target);
    vxmASSERT(kernelDescTable);
    vxmASSERT(kernelCount > 0);

    vxStrCopySafe(target->name, VX_MAX_TARGET_NAME, VX_DEFAULT_TARGET_NAME);

    for (index = 0; index < kernelCount; index++)
    {
        vx_status status = vxoKernel_Initialize(target->base.context,
                                                &target->kernelTable[index],
                                                kernelDescTable[index]->name,
                                                kernelDescTable[index]->enumeration,
                                                VX_NULL,
                                                kernelDescTable[index]->function,
                                                kernelDescTable[index]->parameters,
                                                kernelDescTable[index]->numParams,
                                                kernelDescTable[index]->validate,
                                                kernelDescTable[index]->inputValidate,
                                                kernelDescTable[index]->outputValidate,
                                                kernelDescTable[index]->initialize,
                                                kernelDescTable[index]->deinitialize
#if gcdVX_OPTIMIZER
, kernelDescTable[index]->optAttributes
#endif
                                                );

        if (status != VX_SUCCESS)
            return status;

        if (vxoKernel_IsUnique(&target->kernelTable[index])) target->base.context->uniqueKernelCount++;
        target->kernelCount++;
        target->base.context->kernelCount++;

        status = vxFinalizeKernel(&target->kernelTable[index]);
        if (status != VX_SUCCESS)
            return status;
    }

    /* ToDo : Add more specific return status check */
    if (gcoVX_Initialize(&target->base.context->evisNoInst) != gcvSTATUS_OK)
    {
        return VX_FAILURE;
    }

    return VX_SUCCESS;
}



VX_PRIVATE_API vx_status vxDeinitializeTarget(vx_target target)
{
    vx_uint32 index;

    vxmASSERT(target);

    for (index = 0; index < target->kernelCount; index++)
    {
        vx_kernel kernel = &target->kernelTable[index];

        if (!kernel->enabled || kernel->enumeration == VX_KERNEL_INVALID) continue;

        kernel->enabled = vx_false_e;

        if (vxoKernel_IsUnique(&target->kernelTable[index])) target->base.context->uniqueKernelCount--;

        if (vxoKernel_InternalRelease(&kernel) != VX_SUCCESS) return VX_FAILURE;
    }

    target->kernelCount = 0;
    target->base.context->kernelCount -= target->kernelCount;

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoTarget_Initialize(vx_target target)
{
    if (target == VX_NULL)
    {
        vxStrCopySafe(target->name, VX_MAX_TARGET_NAME, VX_DEFAULT_TARGET_NAME);

        target->priority = VX_TARGET_PRIORITY_DEFAULT;
    }

    return vxInitializeTarget(target, target_kernels, num_target_kernels);
}

VX_PRIVATE_API vx_status vxoTarget_Deinitialize(vx_target target)
{
    return vxDeinitializeTarget(target);
}

VX_PRIVATE_API vx_status vxoTarget_IsKernelSupported(
        vx_target target, vx_char targetName[VX_MAX_TARGET_NAME], vx_char kernelName[VX_MAX_KERNEL_NAME],
#if defined(OPENVX_USE_VARIANTS)
        vx_char variantName[VX_MAX_VARIANT_NAME],
#endif
        OUT vx_uint32_ptr indexPtr)
{
    vx_uint32 index;

    if (vxIsSameString(targetName, VX_DEFAULT_TARGET_NAME, VX_MAX_TARGET_NAME)) return VX_ERROR_NOT_SUPPORTED;

    for (index = 0; index < target->kernelCount; index++)
    {
        vx_char         kernelFullName[VX_MAX_KERNEL_NAME+1] = {'\0'};
        vx_char_ptr     kernelNamePtr;

#if defined(OPENVX_USE_VARIANTS)
        vx_char_ptr     variantNamePtr;
#endif

        strncpy(kernelFullName, target->kernelTable[index].name, VX_MAX_KERNEL_NAME);

        kernelNamePtr = strtok(kernelFullName, ":");

#if defined(OPENVX_USE_VARIANTS)
        variantNamePtr = strtok(VX_NULL, ":");
        if (variantNamePtr == VX_NULL) variantNamePtr = "default";
#endif

        if (vxIsSameString(kernelName, kernelNamePtr, VX_MAX_KERNEL_NAME)
#if defined(OPENVX_USE_VARIANTS)
            && vxIsSameString(variantName, variantNamePtr, VX_MAX_VARIANT_NAME)
#endif
            )
        {
            if (indexPtr != VX_NULL) *indexPtr = index;
            return VX_SUCCESS;
        }
    }

    return VX_ERROR_NOT_SUPPORTED;
}

VX_PRIVATE_API vx_bool isGPUNode(vx_node node)
{
    if ((node->layer && node->layer->operations[0]->target != VXNNE_OPERATION_TARGET_SW)  ||
        (!node->layer && node->kernelAttributes.isGPUKernel))
    {
        return vx_true_e;
    }
    else
    {
        return vx_false_e;
    }
}

VX_PRIVATE_API vx_action vxoTarget_ProcessNodes(
        vx_target target, vx_node nodes[], vx_size startIndex, vx_size nodeCount)
{
    vx_size index;

    vxmASSERT(target);
    vxmASSERT(nodes);

    for (index = startIndex; index < startIndex + nodeCount; index++)
    {
        vx_status   status = VX_SUCCESS;
        vx_node     node = nodes[index];

        vxoPerf_Begin(&node->perf);

        if (!isGPUNode(node))
        {
            status = gcfVX_Flush(gcvTRUE);
        }

        if (status == VX_SUCCESS)
        {
            if (node->isReplicated == vx_true_e)
            {
                vx_size num_replicas = 0;
                vx_uint32 param;
                vx_uint32 num_parameters = node->kernel->signature.paramCount;
                vx_reference parameters[VX_MAX_PARAMETERS] = { NULL };

                for (param = 0; param < num_parameters; ++param)
                {
                    if (node->replicated_flags[param] == vx_true_e)
                    {
                        vx_size numItems = 0;
                        if ((node->paramTable[param])->scope->type == VX_TYPE_PYRAMID)
                        {
                            vx_pyramid pyr = (vx_pyramid)(node->paramTable[param])->scope;
                            numItems = pyr->levelCount;
                        }
                        else if ((node->paramTable[param])->scope->type == VX_TYPE_OBJECT_ARRAY)
                        {
                            vx_object_array arr = (vx_object_array)(node->paramTable[param])->scope;
                            numItems = arr->itemCount;
                        }
                        else
                        {
                            status = VX_ERROR_INVALID_PARAMETERS;
                            break;
                        }

                        if (num_replicas == 0)
                            num_replicas = numItems;
                        else if (numItems != num_replicas)
                        {
                            status = VX_ERROR_INVALID_PARAMETERS;
                            break;
                        }
                    }
                    else
                    {
                        parameters[param] = node->paramTable[param];
                    }
                }

                if (status == VX_SUCCESS)
                {
                    vx_size replica;
                    for (replica = 0; replica < num_replicas; ++replica)
                    {
                        for (param = 0; param < num_parameters; ++param)
                        {
                            if (node->replicated_flags[param] == vx_true_e)
                            {
                                if ((node->paramTable[param])->scope->type == VX_TYPE_PYRAMID)
                                {
                                    vx_pyramid pyr = (vx_pyramid)(node->paramTable[param])->scope;
                                    parameters[param] = (vx_reference)pyr->levels[replica];
                                }
                                else if ((node->paramTable[param])->scope->type == VX_TYPE_OBJECT_ARRAY)
                                {
                                    vx_object_array arr = (vx_object_array)(node->paramTable[param])->scope;
                                    parameters[param] = (vx_reference)arr->itemsTable[replica];
                                }
                            }
                        }

                        status = node->kernel->function((vx_node)node,
                            parameters,
                            num_parameters);
                    }
                }
            }
            else
            {
                status = node->kernel->function(node, (vx_reference *)node->paramTable, node->kernel->signature.paramCount);
            }
        }

        node->executed = vx_true_e;
        node->status = status;

        vxoPerf_End(&node->perf);
#if VIVANTE_PROFILER
        vxoProfiler_End((vx_reference)target);
#endif
        if (status != VX_SUCCESS) return VX_ACTION_ABANDON;

        if (node->completeCallback != VX_NULL)
        {
            vx_action action = node->completeCallback(node);

            if (action != VX_ACTION_CONTINUE) return action;
        }

    }

    return VX_ACTION_CONTINUE;
}


VX_PRIVATE_API vx_status vxoTarget_VerifyNode(vx_target target, vx_node node)
{
    vxmASSERT(target);
    vxmASSERT(node);

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_kernel vxoTarget_AddKernel(
        vx_target target, const vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_program program, vx_kernel_f funcPtr, vx_uint32 paramCount,
        vx_kernel_validate_f validate, vx_kernel_input_validate_f input, vx_kernel_output_validate_f output,
        vx_kernel_initialize_f initialize, vx_kernel_deinitialize_f deinitialize)
{
    vx_uint32 index;

    vxmASSERT(target);
    vxmASSERT(name);
    vxmASSERT(funcPtr);

    for (index = target->kernelCount;index < VX_MAX_KERNEL_COUNT; index++)
    {
        vx_kernel kernel = &(target->kernelTable[index]);

        if (!kernel->enabled && kernel->enumeration == VX_KERNEL_INVALID)
        {
#if gcdVX_OPTIMIZER
            vx_kernel_optimization_attribute_s optAttributes = {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0};
#endif

            vx_status status = vxoKernel_Initialize(target->base.context, kernel,
                                                    name, enumeration,
                                                    program, funcPtr,
                                                    VX_NULL, paramCount,
                                                    validate, input, output, initialize, deinitialize
#if gcdVX_OPTIMIZER
, optAttributes
#endif
                                                    );

            if (status != VX_SUCCESS)
            {
                return (vx_kernel)vxoContext_GetErrorObject(target->base.context, status);
            }

            target->kernelCount++;
            return kernel;
        }
    }

    return (vx_kernel)vxoContext_GetErrorObject(target->base.context, VX_ERROR_NO_RESOURCES);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxTilingKernelFunction(vx_node node, const vx_reference paramTable[], vx_uint32 num);

VX_PRIVATE_API vx_kernel vxoTarget_AddTilingKernel(
        vx_target target, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_tiling_kernel_f flexibleFuncPtr, vx_tiling_kernel_f fastFuncPtr, vx_uint32 paramCount,
        vx_kernel_input_validate_f input, vx_kernel_output_validate_f output)
{
    vx_uint32 index;

    vxmASSERT(target);
    vxmASSERT(name);
    vxmASSERT(flexibleFuncPtr);

    for (index = target->kernelCount;index < VX_MAX_KERNEL_COUNT; index++)
    {
        vx_kernel kernel = &(target->kernelTable[index]);

        if (!kernel->enabled && kernel->enumeration == VX_KERNEL_INVALID)
        {
#if gcdVX_OPTIMIZER
            vx_kernel_optimization_attribute_s optAttributes = {vx_false_e, vx_false_e, 0, 0, 0, 0};
#endif
            vx_status status = vxoKernel_Initialize(target->base.context, kernel,
                                                    name, enumeration,
                                                    VX_NULL, vxTilingKernelFunction,
                                                    VX_NULL, paramCount,
                                                    VX_NULL, input, output, VX_NULL, VX_NULL
#if gcdVX_OPTIMIZER
, optAttributes
#endif
                                                    );

            if (status != VX_SUCCESS)
            {
                return (vx_kernel)vxoContext_GetErrorObject(target->base.context, status);
            }

            kernel->tilingFunction = flexibleFuncPtr;

            target->kernelCount++;
            return kernel;
        }
    }

    return (vx_kernel)vxoContext_GetErrorObject(target->base.context, VX_ERROR_NO_RESOURCES);
}

VX_PRIVATE_API vx_status vxGetImagePatchToTile(vx_image image, vx_rectangle_t *rect, vx_tile_t *tile)
{
    vx_uint32 index;

    vxmASSERT(image);
    vxmASSERT(tile);

    for (index = 0; index < image->planeCount; index++)
    {
        vx_status status;

        tile->base[index] = VX_NULL;

        status = vxAccessImagePatch(image, rect, 0,
                                    &tile->addr[index], (vx_ptr_ptr)&tile->base[index], VX_READ_AND_WRITE);
        if (status != VX_SUCCESS) return status;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxSetTileToImagePatch(vx_image image, vx_rectangle_t *rect, vx_tile_t *tile)
{
    vx_uint32 index;

    vxmASSERT(image);
    vxmASSERT(tile);

    for (index = 0; index < image->planeCount; index++)
    {
        vx_status status = vxCommitImagePatch(image, rect, 0, &tile->addr[index], tile->base[index]);
         if (status != VX_SUCCESS) return status;
    }

    return VX_SUCCESS;
}

#define VX_TILE_BLOCK_MULTIPLER                     64
#define VX_INVALID_INDEX                            ((vx_uint32)-1)

VX_PRIVATE_API vx_status VX_CALLBACK vxTilingKernelFunction(vx_node node, const vx_reference paramTable[], vx_uint32 paramCount)
{
    vx_uint32           paramIndex;
    vx_status           status;
    vx_enum             dirTable[VX_MAX_PARAMETERS];
    vx_enum             typeTable[VX_MAX_PARAMETERS];
    vx_tile_t           tileTable[VX_MAX_PARAMETERS];
    vx_image            imageTable[VX_MAX_PARAMETERS];
    size_t              scalarTable[VX_MAX_PARAMETERS];
    vx_ptr              tileKernelParams[VX_MAX_PARAMETERS];
    vx_uint32           firstOutputImageIndex = VX_INVALID_INDEX;
    vx_uint32           imageWidth, imageHeight;
    vx_border_t         borderMode;
    vx_size             tileMemorySize;
    vx_uint32           tileWidth, tileHeight;
    vx_uint32           x, y;


    for (paramIndex = 0; paramIndex < paramCount; paramIndex++)
    {
        vx_parameter param = vxGetParameterByIndex(node, paramIndex);

        if (vxoReference_GetStatus((vx_reference)param) != VX_SUCCESS) return VX_ERROR_INVALID_PARAMETERS;

        status = vxQueryParameter(param, VX_PARAMETER_DIRECTION,
                                    &dirTable[paramIndex], sizeof(dirTable[paramIndex]));
        if (status != VX_SUCCESS) return status;

        status = vxQueryParameter(param, VX_PARAMETER_TYPE,
                                    &typeTable[paramIndex], sizeof(typeTable[paramIndex]));
        if (status != VX_SUCCESS) return status;

        vxReleaseParameter(&param);

        switch (typeTable[paramIndex])
        {
            case VX_TYPE_IMAGE:
                status = vxQueryNode(node, VX_NODE_OUTPUT_TILE_BLOCK_SIZE,
                            &tileTable[paramIndex].tile_block, sizeof(vx_tile_block_size_t));
                if (status != VX_SUCCESS) return status;

                status = vxQueryNode(node, VX_NODE_INPUT_NEIGHBORHOOD,
                            &tileTable[paramIndex].neighborhood, sizeof(vx_neighborhood_size_t));
                if (status != VX_SUCCESS) return status;

                imageTable[paramIndex] = (vx_image)paramTable[paramIndex];

                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_WIDTH,
                                        &tileTable[paramIndex].image.width, sizeof(vx_uint32));
                if (status != VX_SUCCESS) return status;

                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_HEIGHT,
                                        &tileTable[paramIndex].image.height, sizeof(vx_uint32));
                if (status != VX_SUCCESS) return status;

                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_FORMAT,
                                        &tileTable[paramIndex].image.format, sizeof(vx_df_image));
                if (status != VX_SUCCESS) return status;

                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_SPACE,
                                        &tileTable[paramIndex].image.space, sizeof(vx_enum));
                if (status != VX_SUCCESS) return status;

                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_RANGE,
                                        &tileTable[paramIndex].image.range, sizeof(vx_enum));
                if (status != VX_SUCCESS) return status;

                tileKernelParams[paramIndex] = &tileTable[paramIndex];

                if (dirTable[paramIndex] == VX_OUTPUT)
                {
                    if (firstOutputImageIndex == VX_INVALID_INDEX) firstOutputImageIndex = paramIndex;
                }
                break;

            case VX_TYPE_SCALAR:
                status = vxReadScalarValue((vx_scalar)paramTable[paramIndex], (void *)&scalarTable[paramIndex]);
                if (status != VX_SUCCESS) return status;

                tileKernelParams[paramIndex] = &scalarTable[paramIndex];
                break;
        }
    }

    if (firstOutputImageIndex == VX_INVALID_INDEX) return VX_ERROR_INVALID_PARAMETERS;

    status = vxQueryImage(imageTable[firstOutputImageIndex], VX_IMAGE_WIDTH,
                            &imageWidth, sizeof(imageWidth));
    if (status != VX_SUCCESS) return status;

    status = vxQueryImage(imageTable[firstOutputImageIndex], VX_IMAGE_HEIGHT,
                            &imageHeight, sizeof(imageHeight));
    if (status != VX_SUCCESS) return status;

    status = vxQueryNode(node, VX_NODE_BORDER, &borderMode, sizeof(borderMode));
    if (status != VX_SUCCESS) return status;

    if (borderMode.mode != VX_BORDER_UNDEFINED && borderMode.mode != VX_BORDER_SELF)
    {
        return VX_ERROR_NOT_SUPPORTED;
    }

    status = vxQueryNode(node, VX_NODE_TILE_MEMORY_SIZE, &tileMemorySize, sizeof(tileMemorySize));
    if (status != VX_SUCCESS) return status;

    tileHeight  = imageHeight / VX_TILE_BLOCK_MULTIPLER;
    tileWidth   = imageWidth;

    for (y = 0; y < imageHeight; y += tileHeight)
    {
        for (x = 0; x < imageWidth; x += tileWidth)
        {
            vx_rectangle_t  rect;
            vx_ptr          tileMemoryPtr = VX_NULL;

            rect.start_x    = x;
            rect.start_y    = y;
            rect.end_x      = x + tileWidth;
            rect.end_y      = y + tileHeight;

            for (paramIndex = 0; paramIndex < paramCount; paramIndex++)
            {
                if (typeTable[paramIndex] == VX_TYPE_IMAGE)
                {
                    tileTable[paramIndex].tile_x = x;
                    tileTable[paramIndex].tile_y = y;

                    status = vxGetImagePatchToTile(imageTable[paramIndex], &rect, &tileTable[paramIndex]);
                    if (status != VX_SUCCESS) return status;
                }
            }

            status = vxQueryNode(node, VX_NODE_TILE_MEMORY_PTR, &tileMemoryPtr, sizeof(vx_ptr));
            if (status != VX_SUCCESS) return status;

            node->kernel->tilingFunction(tileKernelParams, tileMemoryPtr, tileMemorySize);

            for (paramIndex = 0; paramIndex < paramCount; paramIndex++)
            {
                if (typeTable[paramIndex] == VX_TYPE_IMAGE)
                {
                    if (dirTable[paramIndex] == VX_INPUT)
                    {
                        status = vxSetTileToImagePatch(imageTable[paramIndex], VX_NULL, &tileTable[paramIndex]);
                    }
                    else
                    {
                        status = vxSetTileToImagePatch(imageTable[paramIndex], &rect, &tileTable[paramIndex]);
                    }

                    if (status != VX_SUCCESS) return status;
                }
            }
        }
    }

    return VX_SUCCESS;
}

VX_INTERNAL_API void vxoTarget_Dump(vx_target target, vx_uint32 index)
{
    if (target == VX_NULL)
    {
        vxTrace(VX_TRACE_TARGET, "<target>null</target>\n");
    }
    else
    {
        vxoReference_Dump((vx_reference)target);

        vxTrace(VX_TRACE_TARGET,
                "<target>\n"
                "   <address>"VX_FORMAT_HEX"</address>\n"
                "   <index>%d</index>\n"
                "   <enabled>%s</enabled>\n"
                "   <name>\"%s\"</name>\n"
                "</target>",
                target, index, vxmBOOL_TO_STRING(target->enabled),
                target->name);
    }
}

#define VX_DEFAULT_TARGET_MODULE_NAME       "openvx.target"

VX_INTERNAL_API vx_status vxoTarget_Load(vx_context context, vx_string moduleName)
{
    vx_uint32   index;
    vx_target   target;
    vx_char     modulePath[VX_MAX_PATH];

    vxmASSERT(context);

    for (index = 0; index < VX_MAX_TARGET_COUNT; index++)
    {
        if (context->targetTable[index].module.handle == VX_NULL_MODULE_HANDLE) break;
    }

    if (index == VX_MAX_TARGET_COUNT) return VX_FAILURE;

    target = &context->targetTable[index];

    if (moduleName == VX_NULL) moduleName = VX_DEFAULT_TARGET_MODULE_NAME;

#if defined(WIN32)
    _snprintf(modulePath, VX_MAX_PATH, VX_MODULE_NAME("%s"), moduleName);
    modulePath[VX_MAX_PATH - 1] = '\0';
#else
    snprintf(modulePath, VX_MAX_PATH, VX_MODULE_NAME("%s"), moduleName);
#endif

    target->module.handle = (vx_module_handle)1;

    if (target->module.handle == VX_NULL)
    {
        vxError("Failed to load module \"%s\"\n", modulePath);
        return VX_ERROR_NO_RESOURCES;
    }

    vxoReference_Initialize(&target->base, context, (vx_type_e)VX_TYPE_TARGET, &context->base);

    vxoReference_Increment(&target->base, VX_REF_INTERNAL);

    vxoContext_AddObject(context, &target->base);

    target->funcs.initialize        = vxoTarget_Initialize;
    target->funcs.deinitialize      = vxoTarget_Deinitialize;
    target->funcs.iskernelsupported = vxoTarget_IsKernelSupported;
    target->funcs.addkernel         = vxoTarget_AddKernel;
    target->funcs.addtilingkernel   = vxoTarget_AddTilingKernel;
    target->funcs.verifynode        = vxoTarget_VerifyNode;
    target->funcs.processnodes      = vxoTarget_ProcessNodes;
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoTarget_Unload(vx_context context, vx_uint32 index, vx_bool unloadModule)
{
    vx_target target;

    vxmASSERT(context);

    if (index >= VX_MAX_TARGET_COUNT) return VX_ERROR_INVALID_PARAMETERS;

    target = &context->targetTable[index];

    vxmASSERT(target);

    vxZeroMemory(&target->funcs, sizeof(vx_target_funcs_s));

    if (vxoReference_Decrement(&target->base, VX_REF_INTERNAL) == 0)
    {
        vxoReference_Increment(&target->base, VX_REF_INTERNAL);

        if (unloadModule)
        {
            vxUnloadModule(target->module.handle);
            target->module.handle = VX_NULL_MODULE_HANDLE;
        }

        vxZeroMemory(&target->module.name, sizeof(target->module.name));

        vxoReference_Release((vx_reference_ptr)&target, (vx_type_e)VX_TYPE_TARGET, VX_REF_INTERNAL);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_uint32 vxoTarget_GetIndex(vx_target target)
{
    vx_uint32 index;

    vxmASSERT(target);

    for (index = 0; index < target->base.context->kernelCount; index++)
    {
        if (target == &target->base.context->targetTable[index]) return index;
    }

    return index;
}

#if defined(OPENVX_USE_TARGET)

VX_API_ENTRY vx_target VX_API_CALL VX_API_CALL vxGetTargetByIndex(vx_context context, vx_uint32 index)
{
    vx_target target;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (index >= context->targetCount)
    {
        vxError("Invalid target index: %d", index);
        return (vx_target)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    target = &context->targetTable[index];

    vxoReference_Increment(&target->base, VX_REF_EXTERNAL);

    vxoTarget_Dump(target, index);

    return target;
}

VX_API_ENTRY vx_target VX_API_CALL vxGetTargetByName(vx_context context, const vx_char *name)
{
    vx_uint32 index;
    vx_target target;

    if (!vxoContext_IsValid(context)) return VX_NULL;

    if (name == VX_NULL)
    {
        vxError("Target name is null");
        return (vx_target)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    for (index = 0; index < context->targetCount; index++)
    {
        if (vxIsSameString(name, context->targetTable[index].name, VX_MAX_TARGET_NAME))
        {
            target = &context->targetTable[index];

            vxoReference_Increment(&target->base, VX_REF_EXTERNAL);

            vxoTarget_Dump(target, index);

            return target;
        }
    }

    vxError("Invalid target name: \"%s\"", name);
    return (vx_target)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseTarget(vx_target *target)
{
    return vxoReference_Release((vx_reference_ptr)target, (vx_type_e)VX_TYPE_TARGET, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryTarget(vx_target target, vx_enum attribute, void *ptr, vx_size size)
{
    vx_uint32           targetIndex;
    vx_uint32           kernelIndex;
    vx_kernel_info_t *  kernelTable;

    if (!vxoReference_IsValidAndSpecific((vx_reference)target, (vx_type_e)VX_TYPE_TARGET))
    {
        return VX_ERROR_INVALID_REFERENCE;
    }

    switch (attribute)
    {
        case VX_TARGET_ATTRIBUTE_INDEX:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            targetIndex = vxoTarget_GetIndex(target);

            if (targetIndex == target->base.context->targetCount) return VX_ERROR_INVALID_PARAMETERS;

            *(vx_uint32 *)ptr = targetIndex;
            break;

        case VX_TARGET_ATTRIBUTE_NAME:
            if (size > VX_MAX_TARGET_NAME || ptr == VX_NULL) return VX_ERROR_INVALID_PARAMETERS;

            vxStrCopySafe((vx_string)ptr, size, target->name);
            break;

        case VX_TARGET_ATTRIBUTE_NUMKERNELS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = target->kernelCount;
            break;

        case VX_TARGET_ATTRIBUTE_KERNELTABLE:
            if (size != target->kernelCount * sizeof(vx_kernel_info_t) || ptr == VX_NULL)
            {
                 return VX_ERROR_INVALID_PARAMETERS;
            }

            kernelTable = (vx_kernel_info_t *)ptr;

            for (kernelIndex = 0; kernelIndex < target->kernelCount; kernelIndex++)
            {
                kernelTable[kernelIndex].enumeration = target->kernelTable[kernelIndex].enumeration;

                vxStrCopySafe(kernelTable[kernelIndex].name, VX_MAX_KERNEL_NAME, target->kernelTable[kernelIndex].name);
            }
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxAssignNodeAffinity(vx_node node, vx_target target)
{
    vx_uint32 kernelIndex;

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) return VX_ERROR_INVALID_REFERENCE;

    if (!vxoReference_IsValidAndSpecific(&target->base, (vx_type_e)VX_TYPE_TARGET)) return VX_ERROR_INVALID_REFERENCE;

    for (kernelIndex = 0; kernelIndex < target->kernelCount; kernelIndex++)
    {
        if (node->kernel->enumeration == target->kernelTable[kernelIndex].enumeration)
        {
            node->targetIndex = vxoTarget_GetIndex(target);
            return VX_SUCCESS;
        }
    }

    return VX_ERROR_NOT_SUPPORTED;
}

static vx_const_string gcoOS_ReverseStrstr(vx_const_string string, vx_const_string substr)
{
    vx_const_string last = NULL;
    vx_const_string cur = string;
    do {
        cur = (vx_const_string) strstr(cur, substr);
        if (cur != NULL)
        {
            last = cur;
            cur = cur+1;
        }
    } while (cur != NULL);
    return last;
}

vx_bool vxoTarget_MatchTargetNameWithString(const char* targetName, const char* targetString)
{
    /* 1. find latest occurrence of target_string in target_name;
       2. match only the cases: target_name == "[smth.]<target_string>[.smth]"
     */
    const char dot = '.';
    vx_bool match = vx_false_e;
    size_t len = strlen(targetString);
    vx_string lowerString = (char*)calloc(len + 1, sizeof(char));

    if (lowerString != VX_NULL) vxStrToLower(targetString, lowerString);

    {
        vx_const_string ptr = gcoOS_ReverseStrstr(targetName, lowerString);
        if (ptr != VX_NULL)
        {
            vx_size name_len = strlen(targetName);
            vx_size string_len = strlen(targetString);
            vx_size begin = (vx_size)(ptr - targetName);
            vx_size end = begin + string_len;
            if ((!(begin > 0) || ((begin > 0) && (targetName[begin-1] == dot))) ||
                (!(end < name_len) || ((end < name_len) && (targetName[end] == dot))))
            {
                match = vx_true_e;
            }
        }
    }

    free(lowerString);

    return match;
}

#endif /* defined(OPENVX_USE_TARGET) */


