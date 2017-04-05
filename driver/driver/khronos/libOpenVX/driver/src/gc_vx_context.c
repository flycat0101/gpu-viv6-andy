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

/*
** Misc
*/
#define VX_IMPLEMENTATION_NAME          "Vivante"

gceSTATUS gcLoadKernelCompiler(IN gcsHWCaps *HWCaps, IN gcePATCH_ID PatchId);
gceSTATUS gcUnloadKernelCompiler(void);

VX_INTERNAL_API vx_bool vxIsValidImportType(vx_enum type)
{
    switch (type)
    {
        case VX_IMPORT_TYPE_HOST:
            return vx_true_e;
#if defined(__linux__)
        case VX_IMPORT_TYPE_DMABUF:
            return vx_true_e;
#endif
        case VX_IMPORT_TYPE_NONE:
            return vx_false_e;

        default:
            vxmASSERT(0);
            return vx_false_e;
    }
}

VX_INTERNAL_API vx_bool vxIsValidBorderMode(vx_enum borderMode)
{
    switch (borderMode)
    {
        case VX_BORDER_MODE_UNDEFINED:
        case VX_BORDER_MODE_CONSTANT:
        case VX_BORDER_MODE_REPLICATE:
            return vx_true_e;

        default:
            vxmASSERT(0);
            return vx_false_e;
    }
}

/*
** DataType APIs
*/
typedef struct _vx_datatype_size_record_s
{
    vx_enum     type;
    vx_size     size;
}
vx_datatype_size_record_s;

static vx_datatype_size_record_s vxDataTypeSizeRecords[] = {

    {VX_TYPE_INVALID, 0},

    /* Scalar */
    {VX_TYPE_CHAR, sizeof(vx_char)},
    {VX_TYPE_INT8, sizeof(vx_int8)},
    {VX_TYPE_INT16, sizeof(vx_int16)},
    {VX_TYPE_INT32, sizeof(vx_int32)},
    {VX_TYPE_INT64, sizeof(vx_int64)},
    {VX_TYPE_UINT8, sizeof(vx_uint8)},
    {VX_TYPE_UINT16, sizeof(vx_uint16)},
    {VX_TYPE_UINT32, sizeof(vx_uint32)},
    {VX_TYPE_UINT64, sizeof(vx_uint64)},
    {VX_TYPE_FLOAT32, sizeof(vx_float32)},
    {VX_TYPE_FLOAT64, sizeof(vx_float64)},
    {VX_TYPE_ENUM, sizeof(vx_enum)},
    {VX_TYPE_BOOL, sizeof(vx_bool)},
    {VX_TYPE_SIZE, sizeof(vx_size)},
    {VX_TYPE_DF_IMAGE, sizeof(vx_df_image)},

    /* Structures */
    {VX_TYPE_RECTANGLE, sizeof(vx_rectangle_t)},
    {VX_TYPE_COORDINATES2D, sizeof(vx_coordinates2d_t)},
    {VX_TYPE_COORDINATES3D, sizeof(vx_coordinates3d_t)},
    {VX_TYPE_KEYPOINT, sizeof(vx_keypoint_t)},

    /* Pseudo objects */
    {VX_TYPE_ERROR, sizeof(vx_error_s)},
    {VX_TYPE_META_FORMAT, sizeof(vx_meta_format_s)},

    /* framework objects */
    {VX_TYPE_REFERENCE, sizeof(vx_reference_s)},
    {VX_TYPE_CONTEXT, sizeof(vx_context_s)},
    {VX_TYPE_GRAPH, sizeof(vx_graph_s)},
    {VX_TYPE_NODE, sizeof(vx_node_s)},
    {VX_TYPE_TARGET, sizeof(vx_target_s)},
    {VX_TYPE_PARAMETER, sizeof(vx_parameter_s)},
    {VX_TYPE_KERNEL, sizeof(vx_kernel_s)},

    {VX_TYPE_PROGRAM, sizeof(vx_program_s)},

    /* Data objects */
    {VX_TYPE_ARRAY, sizeof(vx_array_s)},
    {VX_TYPE_CONVOLUTION, sizeof(vx_convolution_s)},
    {VX_TYPE_DELAY, sizeof(vx_delay_s)},
    {VX_TYPE_DISTRIBUTION, sizeof(vx_distribution_s)},
    {VX_TYPE_IMAGE, sizeof(vx_image_s)},
    {VX_TYPE_LUT, sizeof(vx_lut_s)},
    {VX_TYPE_MATRIX, sizeof(vx_matrix_s)},
    {VX_TYPE_PYRAMID, sizeof(vx_pyramid_s)},
    {VX_TYPE_REMAP, sizeof(vx_remap_s)},
    {VX_TYPE_SCALAR, sizeof(vx_scalar_s)},
    {VX_TYPE_THRESHOLD, sizeof(vx_threshold_s)}
};

VX_INTERNAL_API vx_size vxDataType_GetSize(vx_type_e type)
{
    int i;

    for (i = 0; i < vxmLENGTH_OF(vxDataTypeSizeRecords); i++)
    {
        if (type == vxDataTypeSizeRecords[i].type)
        {
            return vxDataTypeSizeRecords[i].size;
        }
    }

    return 0;
}

VX_INTERNAL_API vx_bool vxDataType_IsValid(vx_enum type)
{
    vx_bool ret = vx_false_e;

    if (type <= VX_TYPE_INVALID) return vx_false_e;

    if (vxmIS_SCALAR(type) || vxmIS_STRUCT(type) || vxmIS_OBJECT(type))
    {
        return vx_true_e;
    }

    vxError("The type, "VX_FORMAT_HEX", is invalid", type);

    return ret;
}

typedef struct vx_object_destructor_record_s
{
    vx_enum                     type;
    vx_object_destructor_f      destructor;
}
vx_object_destructor_record_s;

static vx_object_destructor_record_s vxDestructorRecords[] = {
    {VX_TYPE_CONTEXT, VX_NULL},
    {VX_TYPE_GRAPH, &vxoGraph_Destructor},
    {VX_TYPE_NODE, &vxoNode_Destructor},
    {VX_TYPE_TARGET, VX_NULL},
    {VX_TYPE_KERNEL, &vxoKernel_Destructor},
    {VX_TYPE_PARAMETER, &vxoParameter_Destructor},
    {VX_TYPE_ERROR, VX_NULL},
    {VX_TYPE_META_FORMAT, VX_NULL},

    {VX_TYPE_ARRAY, &vxoArray_Destructor},
    {VX_TYPE_CONVOLUTION, &vxoConvolution_Destructor},
    {VX_TYPE_DISTRIBUTION, &vxoDistribution_Destructor},
    {VX_TYPE_DELAY, &vxoDelay_Destructor},
    {VX_TYPE_IMAGE, &vxoImage_Destructor},
    {VX_TYPE_LUT, &vxoArray_Destructor},
    {VX_TYPE_MATRIX, &vxoMatrix_Destructor},
    {VX_TYPE_SCALAR, &vxoScalar_Destructor},
    {VX_TYPE_PYRAMID, &vxoPyramid_Destructor},
    {VX_TYPE_REMAP, &vxoRemap_Destructor},
    {VX_TYPE_THRESHOLD, VX_NULL},

    {VX_TYPE_PROGRAM, &vxoProgram_Destructor}
};

VX_INTERNAL_API vx_object_destructor_f vxDataType_GetDestructor(vx_type_e type)
{
    int i = 0;

    for (i = 0; i < vxmLENGTH_OF(vxDestructorRecords); i++)
    {
        if (type == vxDestructorRecords[i].type)
        {
            return vxDestructorRecords[i].destructor;
        }
    }

    return VX_NULL;
}

VX_INTERNAL_API vx_bool vxDataType_IsStatic(vx_type_e type)
{
    switch (type)
    {
        case VX_TYPE_KERNEL:
            return vx_true_e;

        default:
            return vx_false_e;
    }
}

/*
** Context APIs
*/
static vx_context   vxSingletonContext  = VX_NULL;
static vx_mutex     vxContextGlobalLock = VX_NULL;

#if VX_USE_THREADPOOL
VX_PRIVATE_API vx_bool vxNodeWorkerCallback(vx_threadpool_worker worker)
{
    vx_target   target;
    vx_node     node;
    vx_action   action;

    vxmASSERT(worker);
    vxmASSERT(worker->data);

    target  = (vx_target)worker->data->v1;
    vxmASSERT(target);

    node    = (vx_node)worker->data->v2;
    vxmASSERT(node);
    vxmASSERT(node->kernel);

    action  = (vx_action)worker->data->v3;

    vxTrace(VX_TRACE_GRAPH, "Begin processing kernel \"%s\" (target \"%s\")", node->kernel->name, target->name);

    vxoNode_EnableVirtualAccessible(node);

    action = target->funcs.processnodes(target, &node, 0, 1);

    vxoNode_DisableVirtualAccessible(node);

    vxTrace(VX_TRACE_GRAPH, "End processing kernel \"%s\" (target \"%s\"), return %d", node->kernel->name, target->name, action);

    worker->data->v3 = (vx_return_value)action;

    return (vx_bool)(action == VX_ACTION_CONTINUE);
}
#endif

VX_PRIVATE_API vx_return_value vxGraphThreadRoutine(vx_ptr arg)
{
    vx_processor processor = (vx_processor)arg;

    while (processor->running)
    {
        vx_value_set data = VX_NULL;

        if (vxoQueue_ReadData(&processor->input, OUT &data))
        {
            vx_status status;
            vx_graph graph = (vx_graph)data->v1;

            vxmASSERT(graph);

            status = vxProcessGraph(graph);

            data->v1 = (vx_return_value)graph;
            data->v2 = (vx_status)status;

            if (!vxoQueue_WriteData(&processor->output, data))
            {
                vxError("Failed to write queue for graph %p", graph);
            }
        }
    }

    return 0;
}

VX_PRIVATE_API vx_bool vxoContext_CreateAllPredefinedErrorObjects(vx_context context)
{
    vx_enum errorStatus;

    for (errorStatus = VX_STATUS_MIN; errorStatus < VX_SUCCESS; errorStatus++)
    {
        if (vxoError_Create(context, errorStatus) == VX_NULL) return vx_false_e;
    }

    return vx_true_e;
}

VX_PRIVATE_API vx_context vxoContext_Create()
{
    vx_context context = VX_NULL;
    VSC_HW_CONFIG hwCfg;
    gcePATCH_ID patchId;

    if (vxSingletonContext == VX_NULL)
    {
        vxCreateMutex(OUT &vxContextGlobalLock);
    }

    vxAcquireMutex(vxContextGlobalLock);

    gcoVX_ZeroMemorySize();

    if (vxSingletonContext == VX_NULL)
    {
        gctSTRING  oldEnv = gcvNULL;
        gctCHAR    newEnv[128] = {0};

        vxEnableAllTraceTargets(vx_false_e);

        context = (vx_context)vxAllocate(sizeof(vx_context_s));

        if (context == VX_NULL) goto ErrorExit;

        vxoReference_Initialize(&context->base, VX_NULL, VX_TYPE_CONTEXT, VX_NULL);

        vxoReference_Increment(&context->base, VX_REF_EXTERNAL);

        context->immediateBorderMode.mode = VX_BORDER_MODE_UNDEFINED;

#if VX_USE_THREADPOOL
        context->threadPool = vxoThreadPool_Create(
                                    VX_HOST_CORE_COUNT, VX_MAX_REF_COUNT, sizeof(vx_work_s),
                                    vxNodeWorkerCallback, context);
#endif

        vxoContext_CreateAllPredefinedErrorObjects(context);

        if (vxoTarget_Load(context, VX_NULL) != VX_SUCCESS) goto ErrorExit;

        context->targetCount++;

        if (context->targetTable[0].funcs.initialize(&context->targetTable[0]) != VX_SUCCESS)
        {
            vxoTarget_Unload(context, 0, vx_true_e);
            goto ErrorExit;
        }

        context->targetTable[0].enabled = vx_true_e;
        context->targetPriorityTable[0] = 0;

        vxoQueue_Initialize(&context->processor.input);
        vxoQueue_Initialize(&context->processor.output);

        context->processor.running = vx_true_e;
        context->processor.thread = vxCreateThread((vx_thread_routine_f)&vxGraphThreadRoutine, &context->processor);

        context->cnnAvailableEventID = 1;

        gcoOS_GetEnv(gcvNULL, "CNN_PERF", &oldEnv);
        if (oldEnv)
            context->perfEnable = (atoi(oldEnv) == 1)?vx_true_e:vx_false_e;


        context->deviceCount = 1;
        if (gcmIS_ERROR(gcQueryShaderCompilerHwCfg(gcvNULL, &hwCfg)))
        {
            goto ErrorExit;
        }

        gcoHAL_GetPatchID(gcvNULL, &patchId);

        if (gcmIS_ERROR(gcLoadKernelCompiler(&hwCfg, patchId)))
        {
            goto ErrorExit;
        }

        gcoOS_GetEnv(gcvNULL, "VC_OPTION", &oldEnv);
        if (oldEnv)
        {
            gcoOS_StrCatSafe(newEnv, 128, oldEnv);
        }
        gcoOS_StrCatSafe(newEnv, 128, " -CLVIRCG:1 -OCLPASS_KERNEL_STRUCT_ARG_BY_VALUE:1");
        gcoOS_SetEnv(gcvNULL, "VC_OPTION", newEnv);

        vxSingletonContext = context;

        gcoOS_GetEnv(gcvNULL, "VX_EXTENSION_LIBS", &oldEnv);
        if (oldEnv != NULL)
        {
            if(vxLoadKernels(context, oldEnv) != VX_SUCCESS)
            {
                gcmPRINT("VX_EXTENSION_LIBS = %s, but load library failed!", oldEnv);
            }
        }
    }
    else
    {
        context = vxSingletonContext;

        vxoReference_Increment(&context->base, VX_REF_EXTERNAL);
    }

    vxReleaseMutex(vxContextGlobalLock);

#if VIVANTE_PROFILER
    vxoProfiler_Initialize(context);
#endif

    return (vx_context)context;

ErrorExit:
    vxReleaseMutex(vxContextGlobalLock);

    if (context != VX_NULL)
    {
        if ((&context->base)->lock != VX_NULL)
        {
            vxDestroyMutex((&context->base)->lock);
        }
        vxFree(context);
    }

    return VX_NULL;
}

VX_INTERNAL_API vx_bool vxoContext_IsValid(vx_context context)
{
    if (context == VX_NULL
        || context->base.signature != VX_REF_SIGNATURE_ALIVE
        || context->base.type != VX_TYPE_CONTEXT
        || context->base.context != VX_NULL)
    {
        vxError("The context object, %p, is invalid", context);
        return vx_false_e;
    }

    return vx_true_e;
}

VX_PRIVATE_API void vxoContext_ForceReleaseAllObjects(vx_context context)
{
    int i;

    vxmASSERT(context);

    for (i = 0; i < VX_MAX_REF_COUNT; i++)
    {
        vx_reference ref = context->refTable[i];
        vx_uint32 externalCount;

        if (ref == VX_NULL) continue;

        externalCount = vxoReference_GetExternalCount(ref);

        if (externalCount > 0)
        {
            vxWarning("vxoContext_ForceReleaseAllObjects(): "
                        "The reference, "VX_FORMAT_HEX", of type, "VX_FORMAT_HEX
                        ", still has %u external count(s)",
                        ref, ref->type, externalCount);
        }

        if (ref->type == VX_TYPE_ERROR)
        {
            vxoReference_Release(&ref, ref->type, VX_REF_INTERNAL);

            if (ref == VX_NULL) continue;
        }

        while (vxoReference_GetExternalCount(ref) > 1)
        {
            vxoReference_Decrement(ref, VX_REF_EXTERNAL);
        }

        if (vxoReference_GetExternalCount(ref) > 0)
        {
            vxoReference_Release(&ref, ref->type, VX_REF_EXTERNAL);
        }
    }
}

VX_PRIVATE_API vx_status vxoContext_Release(vx_context_ptr contextPtr)
{
    vx_context context;
    vx_uint32 i;
    vx_char* flag;
    gcoOS_GetEnv(gcvNULL, "ENABLE_TRACE_MEMORY", &flag);
    if (flag)
    {
        vx_uint32 size = 0;
        gcoVX_GetMemorySize(&size);
        printf("Memory size (Mbyte) = %d\n", size/(1000*1000));
    }

    if (contextPtr == VX_NULL) return VX_ERROR_INVALID_REFERENCE;

    context = *contextPtr;
    *contextPtr = VX_NULL;

    if (context == VX_NULL) return VX_ERROR_INVALID_REFERENCE;

    vxmASSERT(vxContextGlobalLock);
    vxAcquireMutex(vxContextGlobalLock);

    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

#if VIVANTE_PROFILER
        /* Destroy the profiler. */
        vxoProfiler_Destroy(context);
#endif

    if (vxoReference_Decrement(&context->base, VX_REF_EXTERNAL) == 0)
    {
#if VX_USE_THREADPOOL
        if (context->threadPool != VX_NULL) vxoThreadPool_Destroy(&context->threadPool);
#endif

        context->processor.running = vx_false_e;
        vxoQueue_Stop(&context->processor.input);
        vxCloseThread(context->processor.thread);

        vxoQueue_Deinitialize(&context->processor.output);
        vxoQueue_Deinitialize(&context->processor.input);

        vxRegisterLogCallback(context, VX_NULL, vx_false_e);

        vxoContext_ForceReleaseAllObjects(context);

        for (i = 0; i < context->moduleCount; i++)
        {
            if (context->moduleTable[i].handle != VX_NULL_MODULE_HANDLE)
            {
                vxUnloadModule(context->moduleTable[i].handle);

                context->moduleTable[i].handle = VX_NULL_MODULE_HANDLE;
                vxZeroMemory(context->moduleTable[i].name, VX_MAX_PATH);
            }
        }

        if (context->targetTable[0].enabled)
        {
            context->targetTable[0].funcs.deinitialize(&context->targetTable[0]);

            vxoTarget_Unload(context, 0, vx_true_e);

            context->targetTable[0].enabled = vx_false_e;
        }

        for (i = 0; i < vxmLENGTH_OF(context->accessorTable); i++)
        {
            if (context->accessorTable[i].used) vxoContext_RemoveAccessor(context, i);
        }

        for (i = 0; i < VX_MAX_REF_COUNT; i++)
        {
            if (context->refTable[i] != VX_NULL)
            {
                vxError("No.%d reference object, %p, failed to be released",
                        i, context->refTable[i]);
            }
        }

        vxmASSERT(context->base.lock);
        vxDestroyMutex(context->base.lock);

        gcUnloadKernelCompiler();

        gcoVX_DestroyDevices(context->deviceCount, context->devices);

        if (vxInRuntimeDebugMode()) vxZeroMemory(context, sizeof(vx_context_s));

        context->base.signature = VX_REF_SIGNATURE_RELEASED;

        vxFree(context);

        /* Free the global resources */
        vxReleaseMutex(vxContextGlobalLock);
        vxDestroyMutex(vxContextGlobalLock);
        vxContextGlobalLock = VX_NULL;

        vxSingletonContext = VX_NULL;

        return VX_SUCCESS;
    }
    else
    {
        vxWarning("vxoContext_Release(): the context, %p, still has %u reference count(s) in total",
                    vxoReference_GetTotalCount(&context->base));

        vxReleaseMutex(vxContextGlobalLock);

        return VX_SUCCESS;
    }
}

VX_INTERNAL_API vx_error vxoContext_GetErrorObject(vx_context context, vx_status status)
{
    vx_uint32 i;

    for (i = 0; i < context->refCount; i++)
    {
        if (context->refTable[i] != VX_NULL
            && context->refTable[i]->type == VX_TYPE_ERROR)
        {
            vx_error error = (vx_error_s *)context->refTable[i];

            if (error->status == status) return error;
        }
    }

    return VX_NULL;
}

VX_INTERNAL_API vx_context vxoContext_GetFromReference(vx_reference ref)
{
    if (vxoReference_IsValidAndNoncontext(ref)) return ref->context;

    if (vxoContext_IsValid((vx_context)ref)) return (vx_context)ref;

    vxError("The reference parameter, %p, is invalid", ref);
    return VX_NULL;
}

VX_INTERNAL_API vx_bool vxoContext_AddObject(vx_context context, vx_reference ref)
{
    int i;

    vxmASSERT(context);
    vxmASSERT(ref);

    for (i = 0; i < VX_MAX_REF_COUNT; i++)
    {
        if (context->refTable[i] == VX_NULL)
        {
            context->refTable[i] = ref;
            context->refCount++;
            return vx_true_e;
        }
    }

    vxError("Too many objects in the context, %p.", context);
    return vx_false_e;
}

VX_INTERNAL_API vx_bool vxoContext_RemoveObject(vx_context context, vx_reference ref)
{
    int i;

    vxmASSERT(context);
    vxmASSERT(ref);

    for (i = 0; i < VX_MAX_REF_COUNT; i++)
    {
        if (context->refTable[i] == ref)
        {
            context->refTable[i] = VX_NULL;
            context->refCount--;
            return vx_true_e;
        }
    }

    vxError("Can'targetIndex find the reference, %p, in the context, %p.", ref, context);
    return vx_false_e;
}

vx_bool isFullModuleName(vx_string module)
{
    vx_char *pos;

    pos = strrchr(module, '.');

    if ((pos != VX_NULL) &&
        (strcmp(pos, VX_MODULE_POSTFIX_NAME) == 0))
    {
        return vx_true_e;
    }

    return vx_false_e;
}

VX_INTERNAL_API vx_status vxContext_LoadKernels(vx_context context, const vx_string module)
{
    vx_uint32            i;
    vx_char              moduleName[VX_MAX_PATH];
    vx_symbol_handle     sym;
    vx_publish_kernels_f publish = NULL;
    vx_status            status;

    vxmASSERT(module);

    if (isFullModuleName(module))
    {
        sprintf(moduleName, "%s", module);
    }
    else
    {
        sprintf(moduleName, VX_MODULE_NAME("%s"), module);
    }

    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

    for (i = 0; i < context->moduleCount; i++)
    {
        if (context->moduleTable[i].handle != VX_NULL
            && vxIsSameString(module, context->moduleTable[i].name, VX_MAX_PATH))
        {
            return VX_SUCCESS;
        }
    }

    for (i = 0; i < VX_MAX_MODULE_COUNT; i++)
    {
        if (context->moduleTable[i].handle == VX_NULL)
        {
            context->moduleTable[i].handle = vxLoadModule(moduleName);

            if (context->moduleTable[i].handle == VX_NULL)
            {
                vxError("Failed to load module: \"%s\"", moduleName);
                return VX_FAILURE;
            }

            sym = vxGetSymbol(context->moduleTable[i].handle, "vxPublishKernels");

            publish = (vx_publish_kernels_f)sym;
            if (publish == NULL)
            {
                vxTrace(VX_TRACE_CONTEXT, "Failed to load symbol vxPublishKernels\n");
                vxUnloadModule(context->moduleTable[i].handle);
                context->moduleTable[i].handle = NULL;
                return VX_ERROR_NO_RESOURCES;
            }

            status = publish((vx_context)context);
            if (status != VX_SUCCESS)
            {
                vxTrace(VX_TRACE_CONTEXT, "Failed to publish kernels in module\n");
                vxUnloadModule(context->moduleTable[i].handle);
                context->moduleTable[i].handle = NULL;
                return VX_ERROR_NO_RESOURCES;
            }

            vxStrCopySafe(context->moduleTable[i].name, VX_MAX_PATH, module);
            context->moduleCount++;

            return VX_SUCCESS;
        }
    }

    return VX_ERROR_NO_RESOURCES;
}

const vx_char extensions[] =
#if defined(OPENVX_USE_TILING)
    OPENVX_KHR_TILING" "
#endif
#if defined(OPENVX_USE_XML)
    OPENVX_KHR_XML" "
#endif
#if defined(OPENVX_USE_OPENCL)
    OPENVX_KHR_OPENCL" "
#endif
#if defined(OPENVX_USE_NODE_MEMORY)
    OPENVX_KHR_NODE_MEMORY" "
#endif
#if defined(OPENVX_USE_S16)
    "vx_khr_s16 "
#endif
#if defined(OPENVX_USE_DOT)
    OPENVX_KHR_DOT" "
#endif
#if defined(OPENVX_USE_TARGET)
    OPENVX_EXT_TARGET" "
#endif
#if defined(OPENVX_USE_VARIANTS)
    OPENVX_KHR_VARIANTS" "
#endif
    " ";

VX_INTERNAL_API vx_bool vxoContext_AddAccessor(
        vx_context context, vx_size size, vx_enum usage, vx_ptr ptr, vx_reference ref, OUT vx_uint32_ptr indexPtr, vx_ptr extraDataPtr)
{
    vx_uint32 index;

    vxmASSERT(context);
    vxmASSERT(indexPtr);

    for (index = 0; index < vxmLENGTH_OF(context->accessorTable); index++)
    {
        if (context->accessorTable[index].used) continue;

        if (size > 0 && ptr == VX_NULL)
        {
            ptr = vxAllocate(size);

            if (ptr == VX_NULL) return vx_false_e;

            context->accessorTable[index].allocated = vx_true_e;
        }
        else
        {
            context->accessorTable[index].allocated = vx_false_e;
        }

        context->accessorTable[index].used         = vx_true_e;
        context->accessorTable[index].usage        = usage;
        context->accessorTable[index].ptr          = ptr;
        context->accessorTable[index].ref          = ref;
        context->accessorTable[index].extraDataPtr = extraDataPtr;

        *indexPtr = index;
        return vx_true_e;
    }

    *indexPtr = 0;
    return vx_false_e;
}

VX_INTERNAL_API void vxoContext_RemoveAccessor(vx_context context, vx_uint32 index)
{
    vxmASSERT(context);
    vxmASSERT(index < vxmLENGTH_OF(context->accessorTable));

    if (context->accessorTable[index].allocated) vxFree(context->accessorTable[index].ptr);

    if (context->accessorTable[index].extraDataPtr != NULL) vxFree(context->accessorTable[index].extraDataPtr);

    vxZeroMemory(&context->accessorTable[index], sizeof(vx_accessor_s));

    context->accessorTable[index].used = vx_false_e;
}

VX_INTERNAL_API vx_bool vxoContext_SearchAccessor(vx_context context, vx_ptr ptr, OUT vx_uint32_ptr indexPtr)
{
    vx_uint32 index;

    vxmASSERT(context);
    vxmASSERT(indexPtr);

    for (index = 0; index < vxmLENGTH_OF(context->accessorTable); index++)
    {
        if (!context->accessorTable[index].used) continue;

        if (context->accessorTable[index].ptr == ptr)
        {
            *indexPtr = index;
            return vx_true_e;
        }
    }

    *indexPtr = 0;
    return vx_false_e;
}

VX_API_ENTRY vx_context VX_API_CALL vxCreateContext()
{
    return vxoContext_Create();
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseContext(vx_context *context)
{
    return vxoContext_Release(context);
}

VX_API_ENTRY vx_context VX_API_CALL vxGetContext(vx_reference reference)
{
    return vxoContext_GetFromReference(reference);
}

VX_API_ENTRY vx_status VX_API_CALL vxSetContextAttribute(vx_context context, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_border_mode_t * borderMode;

    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_CONTEXT_ATTRIBUTE_IMMEDIATE_BORDER_MODE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_mode_t, 0x3);

            borderMode = (vx_border_mode_t *)ptr;

            if (!vxIsValidBorderMode(borderMode->mode)) return VX_ERROR_INVALID_VALUE;

            context->immediateBorderMode = *borderMode;
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API void vxCopyKernelName(vx_string dest, vx_const_string src)
{
    vxStrCopySafe(dest, VX_MAX_KERNEL_NAME, src);

#if defined(OPENVX_USE_TARGET) || defined(OPENVX_USE_VARIANT)
    {
        vx_uint32 i;

        for (i = 0; dest[i] != '\0'; i++)
        {
            if (dest[i] == ';')
            {
                dest[i] = '\0';
                break;
            }
        }
    }
#endif
}

VX_PRIVATE_API void vxoContext_GetUniqueKernelTable(vx_context context, OUT vx_kernel_info_t *uniqueKernelTable)
{
    vx_uint32 targetIndex, kernelndex;
    vx_uint32 uniqueKernelIndex;
    vx_uint32 uniqueKernelTableSize = 0u;

    vxmASSERT(context);
    vxmASSERT(uniqueKernelTable);

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        vx_target target = &context->targetTable[targetIndex];

        for (kernelndex = 0; kernelndex < target->kernelCount; kernelndex++)
        {
            vx_bool alreadyExisted = vx_false_e;
            vx_kernel kernel = &context->targetTable[targetIndex].kernelTable[kernelndex];

            for (uniqueKernelIndex = 0; uniqueKernelIndex < uniqueKernelTableSize; uniqueKernelIndex++)
            {
                if (kernel->enumeration == uniqueKernelTable[uniqueKernelIndex].enumeration)
                {
                    alreadyExisted = vx_true_e;
                    break;
                }
            }

            if (alreadyExisted) continue;

            uniqueKernelTable[uniqueKernelTableSize].enumeration = kernel->enumeration;
            vxCopyKernelName(uniqueKernelTable[uniqueKernelTableSize].name, kernel->name);

            uniqueKernelTableSize++;
        }
    }
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryContext(vx_context context, vx_enum attribute, void *ptr, vx_size size)
{
    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_CONTEXT_ATTRIBUTE_VENDOR_ID:
             vxmVALIDATE_PARAMETERS(ptr, size, vx_uint16, 0x1);

            *(vx_uint16 *)ptr = VX_ID_VIVANTE;
            break;

        case VX_CONTEXT_ATTRIBUTE_VERSION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint16, 0x1);

            *(vx_uint16 *)ptr = (vx_uint16)VX_VERSION;
            break;

        case VX_CONTEXT_ATTRIBUTE_MODULES:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = context->moduleCount;
            break;

        case VX_CONTEXT_ATTRIBUTE_REFERENCES:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = context->refCount;
            break;

#ifdef OPENVX_USE_TARGET
        case VX_CONTEXT_ATTRIBUTE_TARGETS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = context->targetCount;
            break;
#endif

        case VX_CONTEXT_ATTRIBUTE_IMPLEMENTATION:
            if (size > VX_MAX_IMPLEMENTATION_NAME || ptr == VX_NULL)
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }

            vxStrCopySafe((vx_string)ptr, VX_MAX_IMPLEMENTATION_NAME, VX_IMPLEMENTATION_NAME);
            break;

        case VX_CONTEXT_ATTRIBUTE_EXTENSIONS_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = sizeof(extensions);
            break;

        case VX_CONTEXT_ATTRIBUTE_EXTENSIONS:
            if (size >= sizeof(extensions) || ptr == VX_NULL)
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }

            vxStrCopySafe((vx_string)ptr, sizeof(extensions), extensions);
            break;

        case VX_CONTEXT_ATTRIBUTE_CONVOLUTION_MAXIMUM_DIMENSION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = VX_MAX_CONVOLUTION_DIM;
            break;

        case VX_CONTEXT_ATTRIBUTE_OPTICAL_FLOW_WINDOW_MAXIMUM_DIMENSION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = VX_MAX_OPTICAL_FLOW_WINDOW_DIM;
            break;

        case VX_CONTEXT_ATTRIBUTE_IMMEDIATE_BORDER_MODE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_mode_t, 0x3);

            *(vx_border_mode_t *)ptr = context->immediateBorderMode;
            break;

        case VX_CONTEXT_ATTRIBUTE_UNIQUE_KERNELS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = context->uniqueKernelCount;
            break;

        case VX_CONTEXT_ATTRIBUTE_UNIQUE_KERNEL_TABLE:
            if (size != (context->uniqueKernelCount * sizeof(vx_kernel_info_t))
                || ptr == VX_NULL)
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }

            vxoContext_GetUniqueKernelTable(context, OUT (vx_kernel_info_t *)ptr);
            break;

        default:
            vxError("The attribute parameter, %d, is not supported", attribute);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxHint(vx_reference reference, vx_enum hint)
{
    if (!vxoContext_IsValid(reference->context)) return VX_ERROR_INVALID_REFERENCE;

    if (!vxoReference_IsValidAndNoncontext(reference)) return VX_ERROR_INVALID_REFERENCE;

    switch (hint)
    {
        case VX_HINT_SERIALIZE:
            if (!vxoReference_IsValidAndSpecific(reference, VX_TYPE_GRAPH))
            {
                return VX_ERROR_INVALID_REFERENCE;
            }

            {
                vx_graph graph = (vx_graph)reference;

                graph->serialize = (vx_bool)!graph->serialize;
            }
            break;

        default:
            vxError("The hint parameter, %d, is not supported", hint);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxDirective(vx_reference reference, vx_enum directive)
{
    if (!vxoContext_IsValid(reference->context)) return VX_ERROR_INVALID_REFERENCE;

    if (!vxoReference_IsValidAndNoncontext(reference)) return VX_ERROR_INVALID_REFERENCE;

    switch (directive)
    {
        case VX_DIRECTIVE_DISABLE_LOGGING:
            reference->context->logEnabled = vx_false_e;
            break;

        case VX_DIRECTIVE_ENABLE_LOGGING:
            reference->context->logEnabled = vx_true_e;
            break;

        default:
            vxError("The directive parameter, %d, is not supported", directive);
            return VX_ERROR_NOT_SUPPORTED;
    }

    return VX_SUCCESS;
}

VX_API_ENTRY vx_enum VX_API_CALL vxRegisterUserStruct(vx_context context, vx_size size)
{
    vx_uint32 i;

    if (!vxoContext_IsValid(context) || size == 0) return VX_TYPE_INVALID;

    for (i = 0; i < VX_MAX_USER_STRUCT_COUNT; i++)
    {
        if (context->userStructTable[i].type == VX_TYPE_INVALID)
        {
            context->userStructTable[i].size = size;

            return context->userStructTable[i].type = VX_TYPE_USER_STRUCT_START + i;
        }
    }

    return VX_TYPE_INVALID;
}

