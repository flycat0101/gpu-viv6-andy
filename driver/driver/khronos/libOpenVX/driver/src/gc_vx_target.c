/****************************************************************************
*
*    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
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
#include <gc_vx_nn_util.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_TARGET

extern vx_status VX_CALLBACK vxoProgramKernel_FunctionVX(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

extern vx_kernel_description_s *    target_kernels[];
extern vx_uint32                    num_target_kernels;

VX_PRIVATE_API vx_status vxInitializeTarget(
    vx_target target,
    vx_kernel_description_s *kernelDescTable[],
    vx_uint32 kernelCount)
{
    vx_uint32   index;
    gcmHEADER_ARG("target=%p, kernelDescTable=%p, kernelCount=0x%x", target, kernelDescTable, kernelCount);
    vxmASSERT(target);
    vxmASSERT(kernelDescTable);
    vxmASSERT(kernelCount > 0);

    /* ToDo : Add more specific return status check */
    if (gcoVX_Initialize(&target->base.context->evisNoInst) != gcvSTATUS_OK)
    {
        gcmFOOTER_NO();
        return VX_FAILURE;
    }

    vxStrCopySafe(target->name, VX_MAX_TARGET_NAME, VX_DEFAULT_TARGET_NAME);

    for (index = 0; index < kernelCount; index++)
    {
        vx_status status = VX_FAILURE;
        vx_kernel kernel = &target->kernelTable[index];

        if (kernel != NULL && kernel->base.type != VX_TYPE_ERROR && kernel->base.type != VX_TYPE_INVALID)
        {
            kernel->enabled = vx_false_e;
            vxRemoveKernel(kernel);
        }

        status = vxoKernel_Initialize(target->base.context,
                                            kernel,
                                            kernelDescTable[index]->name,
                                            kernelDescTable[index]->enumeration,
                                            VX_NULL,
                                            kernelDescTable[index]->function ? kernelDescTable[index]->function : vxoProgramKernel_FunctionVX,
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
        {
            gcmFOOTER_NO();
            return status;
        }
        status = vxFinalizeKernel(&target->kernelTable[index]);
        if (status != VX_SUCCESS)
        {
            gcmFOOTER_NO();
            return status;
        }
        target->kernelCount++;
        target->base.context->kernelCount++;
    }
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxDeinitializeTarget(vx_target target)
{
    vx_uint32 index;
    gcmHEADER_ARG("target=%p", target);
    vxmASSERT(target);

    for (index = 0; index < VX_MAX_KERNEL_COUNT; index++)
    {
        vx_kernel kernel = &target->kernelTable[index];

        if (!kernel->enabled) continue;

        kernel->enabled = vx_false_e;

        if (vxoKernel_IsUnique(&target->kernelTable[index])) target->base.context->uniqueKernelCount--;

        if (vxoKernel_InternalRelease(&kernel) != VX_SUCCESS)
        {
            gcmFOOTER_NO();
            return VX_FAILURE;
        }
    }

    target->base.context->kernelCount -= target->kernelCount;
    target->kernelCount = 0;
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxoTarget_Initialize(vx_target target)
{
    vxmASSERT(target);

    vxStrCopySafe(target->name, VX_MAX_TARGET_NAME, VX_DEFAULT_TARGET_NAME);

    target->priority = VX_TARGET_PRIORITY_DEFAULT;

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
    gcmHEADER_ARG("target=%p, targetName=%s, kernelName=%s", target, targetName, kernelName);

    if (vxIsSameString(targetName, VX_DEFAULT_TARGET_NAME, VX_MAX_TARGET_NAME))
    {
        gcmFOOTER_NO();
        return VX_ERROR_NOT_SUPPORTED;
    }
    for (index = 0; index < VX_MAX_KERNEL_COUNT; index++)
    {
        vx_char         kernelFullName[VX_MAX_KERNEL_NAME+1] = {'\0'};
        vx_char_ptr     kernelNamePtr;

#if defined(OPENVX_USE_VARIANTS)
        vx_char_ptr     variantNamePtr;
#endif

        if(target->kernelTable[index].enabled == vx_false_e
            || target->kernelTable[index].enumeration == VX_KERNEL_INVALID) continue;

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
            gcmFOOTER_NO();
            return VX_SUCCESS;
        }
    }
    gcmFOOTER_NO();
    return VX_ERROR_NOT_SUPPORTED;
}



VX_PRIVATE_API vx_status vxoTarget_ExecuteKernel(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
    vxnne_operation_target_e operationTarget = VXNNE_OPERATION_TARGET_NONE;
    gcmHEADER_ARG("node=%p, parameters=%p, num=0x%x", node, parameters, num);

    if (!node->layer)
    {
        /* means a normal node (no operation) */
        vxnneMultiChannel_GetCurrentChannel(&operationTarget);

        vxnneMultiChannel_SetCurrentChannel(node->kernelAttributes.isGPUKernel ? VXNNE_OPERATION_TARGET_SH : VXNNE_OPERATION_TARGET_SW);

        vxnneMultiChannel_ApplySyncMode(node->waitMode, node->semaWaitHandle);
    }

    status = node->kernel->function((vx_node)node,
                            parameters,
                            num);

    if (!node->layer)
    {
        vxnneMultiChannel_ApplySyncMode(node->wakeMode, node->semaWakeHandle);

        /* restore the previous opt target */
        vxnneMultiChannel_SetCurrentChannel(operationTarget);
    }

    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_action vxoTarget_ProcessNodes(
        vx_target target, vx_node nodes[], vx_size startIndex, vx_size nodeCount)
{
    vx_size index;
    gcmHEADER_ARG("target=%p, nodes=%p, startIndex=0x%lx, nodeCount=0x%lx", target, nodes, startIndex, nodeCount);
    vxmASSERT(target);
    vxmASSERT(nodes);

    for (index = startIndex; index < startIndex + nodeCount; index++)
    {
        vx_status   status = VX_SUCCESS;
        vx_node     node = nodes[index];

        if (target->base.context->options.enableCNNPerf)
            vxoPerf_Begin(&node->perf);
        if (status == VX_SUCCESS)
        {
            if (node->isReplicated == vx_true_e)
            {
                vx_size num_replicas = 0;
                vx_uint32 param;
                vx_uint32 num_parameters = node->kernel->signature.paramCount;
                vx_reference *parameters = vxAllocateAndZeroMemory(num_parameters * gcmSIZEOF(vx_reference));
                if (parameters == VX_NULL)
                {
                    vxError("Error: out of memory at %s:%d\n", __FUNCTION__, __LINE__);
                    gcmASSERT(0);
                    break;
                }

                for (param = 0; param < num_parameters; ++param)
                {
                    vxmASSERT(node->replicated_flags != VX_NULL);
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

                        status =  vxoTarget_ExecuteKernel((vx_node)node,
                            parameters,
                            num_parameters);
                    }
                }
                if (parameters != VX_NULL)
                {
                    vxFree(parameters);
                }
            }
            else
            {
                status = vxoTarget_ExecuteKernel(node, (vx_reference *)node->paramTable, node->kernel->signature.paramCount);
            }
        }

        node->executed = vx_true_e;
        node->status = status;

        if (target->base.context->options.enableCNNPerf)
        {
            gcfVX_Flush(gcvTRUE); /* flush GPU command to get correct node execution time */
            vxoPerf_End(&node->perf);
        }

#if VIVANTE_PROFILER
        vxoProfiler_End((vx_reference)target);
#endif
        if (status != VX_SUCCESS)
        {
            gcmFOOTER_NO();
            return VX_ACTION_ABANDON;
        }
        if (node->completeCallback != VX_NULL)
        {
            vx_action action = node->completeCallback(node);

            if (action != VX_ACTION_CONTINUE)
            {
                gcmFOOTER_NO();
                return action;
            }
        }

    }
    gcmFOOTER_NO();
    return VX_ACTION_CONTINUE;
}

VX_PRIVATE_API vx_action vxoTarget_ProcessNodesBlock(
        vx_target target, vx_node_block nodeBlock)
{
    vx_status status;
    vx_size i;

    gcmHEADER_ARG("target=%p, nodeBlock=%p", target, nodeBlock);
    vxmASSERT(target);
    vxmASSERT(nodeBlock);



    if (nodeBlock->executed)
    {
        gcmFOOTER_NO();
        return VX_ACTION_CONTINUE;
    }
    status = nodeBlock->execute(nodeBlock);

    nodeBlock->executed = vx_true_e;
    nodeBlock->status = status;

    for (i = 0; i < nodeBlock->nodeNum; i++)
    {
        nodeBlock->nodes[i]->executed = vx_true_e;
        nodeBlock->nodes[i]->status = status;
    }

    if (status != VX_SUCCESS)
    {
        gcmFOOTER_ARG("%d", status);
        return VX_ACTION_ABANDON;
    }
    gcmFOOTER_ARG("%d", status);
    return VX_ACTION_CONTINUE;
}

VX_PRIVATE_API vx_action vxoTarget_ProcessLayer(
        vx_target target, vxnne_layer layer)
{
    vx_status status;

    status = layer->execute(layer);

    if (status != VX_SUCCESS) return VX_ACTION_ABANDON;

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

    gcmHEADER_ARG("target=%s, name=%s, enumeration=0x%x, program=%p, funcPtr=%p, paramCount=0x%x, validate=%p, input=%p, output=%p, initialize=%p, deinitialize=%p",
        target, name, enumeration, program, funcPtr, paramCount, validate, input, output, initialize, deinitialize);

    vxmASSERT(target);
    vxmASSERT(name);
    vxmASSERT(funcPtr);

    for (index = 0;index < VX_MAX_KERNEL_COUNT; index++)
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
                gcmFOOTER_NO();
                return (vx_kernel)vxoContext_GetErrorObject(target->base.context, status);
            }

            target->kernelCount++;

            target->base.context->kernelCount++;
            gcmFOOTER_NO();
            return kernel;
        }
    }
    gcmFOOTER_NO();
    return (vx_kernel)vxoContext_GetErrorObject(target->base.context, VX_ERROR_NO_RESOURCES);
}

VX_PRIVATE_API vx_status VX_CALLBACK vxTilingKernelFunction(vx_node node, const vx_reference paramTable[], vx_uint32 num);

VX_PRIVATE_API vx_kernel vxoTarget_AddTilingKernel(
        vx_target target, vx_char name[VX_MAX_KERNEL_NAME], vx_enum enumeration,
        vx_tiling_kernel_f flexibleFuncPtr, vx_tiling_kernel_f fastFuncPtr, vx_uint32 paramCount,
        vx_kernel_input_validate_f input, vx_kernel_output_validate_f output)
{
    vx_uint32 index;

    gcmHEADER_ARG("target=%s, name=%s, enumeration=0x%x, flexibleFuncPtr=%p, fastFuncPtr=%p, paramCount=0x%x, input=%p, output=%p",
        target, name, enumeration, flexibleFuncPtr, fastFuncPtr, paramCount, input, output);
    vxmASSERT(target);
    vxmASSERT(name);
    vxmASSERT(flexibleFuncPtr);

    for (index = 0;index < VX_MAX_KERNEL_COUNT; index++)
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
                gcmFOOTER_ARG("%d", status);
                return (vx_kernel)vxoContext_GetErrorObject(target->base.context, status);
            }

            kernel->tilingFunction = flexibleFuncPtr;

            target->kernelCount++;

            target->base.context->kernelCount++;
            gcmFOOTER_ARG("%d", status);
            return kernel;
        }
    }
    gcmFOOTER_NO();
    return (vx_kernel)vxoContext_GetErrorObject(target->base.context, VX_ERROR_NO_RESOURCES);
}

VX_PRIVATE_API vx_status vxGetImagePatchToTile(vx_image image, vx_rectangle_t *rect, vx_tile_t *tile)
{
    vx_uint32 index;
    gcmHEADER_ARG("image=%p, rect=%p, tile=%p", image, rect, tile);
    vxmASSERT(image);
    vxmASSERT(tile);

    for (index = 0; index < image->planeCount; index++)
    {
        vx_status status;

        tile->base[index] = VX_NULL;

        status = vxAccessImagePatch(image, rect, 0,
                                    &tile->addr[index], (vx_ptr_ptr)&tile->base[index], VX_READ_AND_WRITE);
        if (status != VX_SUCCESS) {
            gcmFOOTER_ARG("%d", status);
            return status;
        }
    }
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status vxSetTileToImagePatch(vx_image image, vx_rectangle_t *rect, vx_tile_t *tile)
{
    vx_uint32 index;

    gcmHEADER_ARG("image=%p, rect=%p, tile=%p", image, rect, tile);

    vxmASSERT(image);
    vxmASSERT(tile);

    for (index = 0; index < image->planeCount; index++)
    {
        vx_status status = vxCommitImagePatch(image, rect, 0, &tile->addr[index], tile->base[index]);
         if (status != VX_SUCCESS) {
             gcmFOOTER_ARG("%d", status);
             return status;

         }
    }
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

#define VX_TILE_BLOCK_MULTIPLER                     64
#define VX_INVALID_INDEX                            ((vx_uint32)-1)

VX_PRIVATE_API vx_status VX_CALLBACK vxTilingKernelFunction(vx_node node, const vx_reference paramTable[], vx_uint32 paramCount)
{
    vx_uint32           paramIndex;
    vx_status           status;
    vx_enum             *dirTable;
    vx_enum             *typeTable;
    vx_tile_t           *tileTable;
    vx_image            *imageTable;
    size_t              *scalarTable;
    vx_ptr              *tileKernelParams;
    vx_uint32           firstOutputImageIndex = VX_INVALID_INDEX;
    vx_uint32           imageWidth, imageHeight;
    vx_border_t         borderMode;
    vx_size             tileMemorySize;
    vx_uint32           tileWidth, tileHeight;
    vx_uint32           x, y;

    gcmHEADER_ARG("node=%p, paramTable=%p, paramCount=0x%x", node, paramTable, paramCount);


    dirTable = vxAllocateAndZeroMemory(paramCount * gcmSIZEOF(vx_enum));
    typeTable = vxAllocateAndZeroMemory(paramCount * gcmSIZEOF(vx_enum));
    tileTable = vxAllocateAndZeroMemory(paramCount * gcmSIZEOF(vx_tile_t));
    imageTable = vxAllocateAndZeroMemory(paramCount * gcmSIZEOF(vx_image));
    scalarTable = vxAllocateAndZeroMemory(paramCount * gcmSIZEOF(size_t));
    tileKernelParams = vxAllocateAndZeroMemory(paramCount * gcmSIZEOF(vx_ptr));

    if ((dirTable == NULL) || (typeTable == NULL) || (tileTable == NULL) ||
        (imageTable == NULL) || (scalarTable == NULL) || (tileKernelParams == NULL))
    {
        vxError("Error: out of memory at %s:%d\n", __FUNCTION__, __LINE__);
        vxmASSERT(0);
        gcmFOOTER_NO();
        status = VX_ERROR_NO_MEMORY;
        goto onError;
    }

    for (paramIndex = 0; paramIndex < paramCount; paramIndex++)
    {
        vx_parameter param = vxGetParameterByIndex(node, paramIndex);

        if (vxoReference_GetStatus((vx_reference)param) != VX_SUCCESS)
        {
            status = VX_ERROR_INVALID_PARAMETERS;
            goto onError;
        }
        status = vxQueryParameter(param, VX_PARAMETER_DIRECTION,
                                    &dirTable[paramIndex], sizeof(dirTable[paramIndex]));
        if (status != VX_SUCCESS)
        {
            goto onError;
        }
        status = vxQueryParameter(param, VX_PARAMETER_TYPE,
                                    &typeTable[paramIndex], sizeof(typeTable[paramIndex]));
        if (status != VX_SUCCESS)
        {
            goto onError;
        }
        vxReleaseParameter(&param);

        switch (typeTable[paramIndex])
        {
            case VX_TYPE_IMAGE:
                status = vxQueryNode(node, VX_NODE_OUTPUT_TILE_BLOCK_SIZE,
                            &tileTable[paramIndex].tile_block, sizeof(vx_tile_block_size_t));
                if (status != VX_SUCCESS)
                {
                    goto onError;
                }
                status = vxQueryNode(node, VX_NODE_INPUT_NEIGHBORHOOD,
                            &tileTable[paramIndex].neighborhood, sizeof(vx_neighborhood_size_t));
                if (status != VX_SUCCESS)
                {
                    goto onError;
                }
                imageTable[paramIndex] = (vx_image)paramTable[paramIndex];

                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_WIDTH,
                                        &tileTable[paramIndex].image.width, sizeof(vx_uint32));
                if (status != VX_SUCCESS)
                {
                    goto onError;
                }
                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_HEIGHT,
                                        &tileTable[paramIndex].image.height, sizeof(vx_uint32));
                if (status != VX_SUCCESS)
                {
                    goto onError;
                }
                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_FORMAT,
                                        &tileTable[paramIndex].image.format, sizeof(vx_df_image));
                if (status != VX_SUCCESS)
                {
                    goto onError;
                }
                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_SPACE,
                                        &tileTable[paramIndex].image.space, sizeof(vx_enum));
                if (status != VX_SUCCESS)
                {
                    goto onError;
                }
                status = vxQueryImage(imageTable[paramIndex], VX_IMAGE_RANGE,
                                        &tileTable[paramIndex].image.range, sizeof(vx_enum));
                if (status != VX_SUCCESS)
                {
                    goto onError;
                }
                tileKernelParams[paramIndex] = &tileTable[paramIndex];

                if (dirTable[paramIndex] == VX_OUTPUT)
                {
                    if (firstOutputImageIndex == VX_INVALID_INDEX) firstOutputImageIndex = paramIndex;
                }
                break;

            case VX_TYPE_SCALAR:
                status = vxReadScalarValue((vx_scalar)paramTable[paramIndex], (void *)&scalarTable[paramIndex]);
                if (status != VX_SUCCESS)
                {
                    goto onError;
                }
                tileKernelParams[paramIndex] = &scalarTable[paramIndex];
                break;
        }
    }

    if (firstOutputImageIndex == VX_INVALID_INDEX)
    {
        status = VX_ERROR_INVALID_PARAMETERS;
        goto onError;
    }
    status = vxQueryImage(imageTable[firstOutputImageIndex], VX_IMAGE_WIDTH,
                            &imageWidth, sizeof(imageWidth));
    if (status != VX_SUCCESS)
    {
        goto onError;
    }
    status = vxQueryImage(imageTable[firstOutputImageIndex], VX_IMAGE_HEIGHT,
                            &imageHeight, sizeof(imageHeight));
    if (status != VX_SUCCESS)
    {
        goto onError;
    }
    status = vxQueryNode(node, VX_NODE_BORDER, &borderMode, sizeof(borderMode));
    if (status != VX_SUCCESS)
    {
        goto onError;
    }
    if (borderMode.mode != VX_BORDER_UNDEFINED && borderMode.mode != VX_BORDER_MODE_SELF)
    {
        status = VX_ERROR_NOT_SUPPORTED;
        goto onError;
    }

    status = vxQueryNode(node, VX_NODE_TILE_MEMORY_SIZE, &tileMemorySize, sizeof(tileMemorySize));
    if (status != VX_SUCCESS)
    {
        goto onError;
    }
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
                    if (status != VX_SUCCESS)
                    {
                        goto onError;
                    }
                }
            }

            status = vxQueryNode(node, VX_NODE_TILE_MEMORY_PTR, &tileMemoryPtr, sizeof(vx_ptr));
            if (status != VX_SUCCESS)
            {
                goto onError;
            }
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

                    if (status != VX_SUCCESS)
                    {
                        goto onError;
                    }
                }
            }
        }
    }

onError:
    if (dirTable) vxFree(dirTable);
    if (typeTable) vxFree(typeTable);
    if (tileTable) vxFree(tileTable);
    if (imageTable) vxFree(imageTable);
    if (scalarTable) vxFree(scalarTable);
    if (tileKernelParams) vxFree(tileKernelParams);

    gcmFOOTER_ARG("%d", status);
    return status;
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
                "   <address>%p</address>\n"
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

    gcmHEADER_ARG("context=%p, moduleName=%s", context, moduleName);
    vxmASSERT(context);

    for (index = 0; index < VX_MAX_TARGET_COUNT; index++)
    {
        if (context->targetTable[index].module.handle == VX_NULL_MODULE_HANDLE) break;
    }

    if (index == VX_MAX_TARGET_COUNT)
    {
        gcmFOOTER_NO();
        return VX_FAILURE;
    }
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
        gcmFOOTER_NO();
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
    target->funcs.processnodesBlock = vxoTarget_ProcessNodesBlock;
    target->funcs.processLayer      = vxoTarget_ProcessLayer;
    target->context                 = context;
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_INTERNAL_API vx_status vxoTarget_Unload(vx_context context, vx_uint32 index, vx_bool unloadModule)
{
    vx_target target;

    gcmHEADER_ARG("context=%p, index=0x%x, unloadModule=0x%x", context, index, unloadModule);
    vxmASSERT(context);

    if (index >= VX_MAX_TARGET_COUNT)
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_PARAMETERS;
    }
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

    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_uint32 vxoTarget_GetIndex(vx_target target)
{
    vx_uint32 index;
    gcmHEADER_ARG("target=%p", target);
    vxmASSERT(target);

    for (index = 0; index < target->base.context->targetCount; index++)
    {
        if (target == &target->base.context->targetTable[index])
        {
            gcmFOOTER_NO();
            return index;
        }
    }

    gcmFOOTER_NO();
    return index;
}

#if defined(OPENVX_USE_TARGET)

VX_API_ENTRY vx_target VX_API_CALL VX_API_CALL vxGetTargetByIndex(vx_context context, vx_uint32 index)
{
    vx_target target;

    gcmHEADER_ARG("context=%p, index=0x%x", context, index);
    gcmDUMP_API("$VX vxGetTargetByIndex: context=%p, index=0x%x", context, index);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (index >= context->targetCount)
    {
        vxError("Invalid target index: %d", index);
        gcmFOOTER_NO();
        return (vx_target)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    target = &context->targetTable[index];

    vxoReference_Increment(&target->base, VX_REF_EXTERNAL);

    vxoTarget_Dump(target, index);

    gcmFOOTER_NO();
    return target;
}

VX_API_ENTRY vx_target VX_API_CALL vxGetTargetByName(vx_context context, const vx_char *name)
{
    vx_uint32 index;
    vx_target target;

    gcmHEADER_ARG("context=%p, name=%s", context, name);
    gcmDUMP_API("$VX vxGetTargetByName: context=%p, name=%s", context, name);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if (name == VX_NULL)
    {
        vxError("Target name is null");
        gcmFOOTER_NO();
        return (vx_target)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
    }

    for (index = 0; index < context->targetCount; index++)
    {
        if (vxIsSameString(name, context->targetTable[index].name, VX_MAX_TARGET_NAME))
        {
            target = &context->targetTable[index];

            vxoReference_Increment(&target->base, VX_REF_EXTERNAL);

            vxoTarget_Dump(target, index);

            gcmFOOTER_NO();

            return target;
        }
    }

    vxError("Invalid target name: \"%s\"", name);
    gcmFOOTER_NO();
    return (vx_target)vxoContext_GetErrorObject(context, VX_ERROR_INVALID_PARAMETERS);
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseTarget(vx_target *target)
{
    gcmDUMP_API("$VX vxReleaseTarget: target=%p", target);

    return vxoReference_Release((vx_reference_ptr)target, (vx_type_e)VX_TYPE_TARGET, VX_REF_EXTERNAL);
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryTarget(vx_target target, vx_enum attribute, void *ptr, vx_size size)
{
    vx_uint32           targetIndex;
    vx_uint32           kernelIndex;
    vx_kernel_info_t *  kernelTable;
    vx_uint32           kernelTableSize;

    gcmHEADER_ARG("target=%p, attribute=0x%x, size=0x%x", target, attribute, size);
    gcmDUMP_API("$VX vxQueryTarget: target=%p, attribute=0x%x, size=0x%x", target, attribute, size);

    if (!vxoReference_IsValidAndSpecific((vx_reference)target, (vx_type_e)VX_TYPE_TARGET))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }

    switch (attribute)
    {
        case VX_TARGET_ATTRIBUTE_INDEX:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            targetIndex = vxoTarget_GetIndex(target);

            if (targetIndex == target->base.context->targetCount)
            {
                gcmFOOTER_NO();
                return VX_ERROR_INVALID_PARAMETERS;
            }
            *(vx_uint32 *)ptr = targetIndex;
            break;

        case VX_TARGET_ATTRIBUTE_NAME:
            if (size > VX_MAX_TARGET_NAME || ptr == VX_NULL)
            {
                gcmFOOTER_NO();
                return VX_ERROR_INVALID_PARAMETERS;
            }
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

            kernelTableSize = 0;
            for (kernelIndex = 0; kernelIndex < VX_MAX_KERNEL_COUNT; kernelIndex++)
            {
                if(target->kernelTable[kernelIndex].enabled == vx_false_e)
                    continue;

                kernelTable[kernelTableSize].enumeration = target->kernelTable[kernelIndex].enumeration;

                vxStrCopySafe(kernelTable[kernelTableSize].name, VX_MAX_KERNEL_NAME, target->kernelTable[kernelIndex].name);
                kernelTableSize++;
            }
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            gcmFOOTER_NO();
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_NO();
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxAssignNodeAffinity(vx_node node, vx_target target)
{
    vx_uint32 kernelIndex;

    gcmHEADER_ARG("node=%p, target=%p", node, target);
    gcmDUMP_API("$VX vxAssignNodeAffinity: node=%p, target=%p", node, target);

    if (!vxoReference_IsValidAndSpecific(&node->base, VX_TYPE_NODE)) {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    if (!vxoReference_IsValidAndSpecific(&target->base, (vx_type_e)VX_TYPE_TARGET))
    {
        gcmFOOTER_NO();
        return VX_ERROR_INVALID_REFERENCE;
    }
    for (kernelIndex = 0; kernelIndex < VX_MAX_KERNEL_COUNT; kernelIndex++)
    {
        if(target->kernelTable[kernelIndex].enabled == vx_false_e
          || target->kernelTable[kernelIndex].enumeration == VX_KERNEL_INVALID)
            continue;

        if (node->kernel->enumeration == target->kernelTable[kernelIndex].enumeration)
        {
            node->targetIndex = vxoTarget_GetIndex(target);
            gcmFOOTER_NO();
            return VX_SUCCESS;
        }
    }
    gcmFOOTER_NO();
    return VX_ERROR_NOT_SUPPORTED;
}

static vx_const_string gcoOS_ReverseStrstr(vx_const_string string, vx_const_string substr)
{
    vx_const_string last = NULL;
    vx_const_string cur = string;
    gcmHEADER_ARG("string=%s, substr=%s", string, substr);

    do {
        cur = (vx_const_string) strstr(cur, substr);
        if (cur != NULL)
        {
            last = cur;
            cur = cur+1;
        }
    } while (cur != NULL);

    gcmFOOTER_NO();
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
    gcmHEADER_ARG("targetName=%p, targetString=%p", targetName, targetString);

    if (lowerString != VX_NULL)
    {
        vxStrToLower(targetString, lowerString);
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
    }
    gcmFOOTER_NO();
    return match;
}

#endif /* defined(OPENVX_USE_TARGET) */


