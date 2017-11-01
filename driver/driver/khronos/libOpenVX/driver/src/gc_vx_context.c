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
        case VX_MEMORY_TYPE_HOST:
        case VX_MEMORY_TYPE_HOST_UNCACHED:
            return vx_true_e;
#if defined(__linux__)
        case VX_MEMORY_TYPE_DMABUF:
            return vx_true_e;
#endif
        case VX_MEMORY_TYPE_NONE:
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
        case VX_BORDER_UNDEFINED:
        case VX_BORDER_CONSTANT:
        case VX_BORDER_REPLICATE:
            return vx_true_e;

        default:
            vxmASSERT(0);
            return vx_false_e;
    }
}

VX_INTERNAL_API vx_bool vxIsValidBorderModePolicy(vx_enum borderModePolicy)
{
    switch (borderModePolicy)
    {
        case VX_BORDER_POLICY_DEFAULT_TO_UNDEFINED:
        case VX_BORDER_POLICY_RETURN_ERROR:
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
    {VX_TYPE_FLOAT16, sizeof(vx_uint16)},
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
    {VX_TYPE_OBJECT_ARRAY, sizeof(vx_object_array_s)},
    {VX_TYPE_CONVOLUTION, sizeof(vx_convolution_s)},
    {VX_TYPE_DELAY, sizeof(vx_delay_s)},
    {VX_TYPE_DISTRIBUTION, sizeof(vx_distribution_s)},
    {VX_TYPE_IMAGE, sizeof(vx_image_s)},
    {VX_TYPE_LUT, sizeof(vx_lut_s)},
    {VX_TYPE_MATRIX, sizeof(vx_matrix_s)},
    {VX_TYPE_PYRAMID, sizeof(vx_pyramid_s)},
    {VX_TYPE_REMAP, sizeof(vx_remap_s)},
    {VX_TYPE_SCALAR, sizeof(vx_scalar_s)},
    {VX_TYPE_THRESHOLD, sizeof(vx_threshold_s)},

    {VX_TYPE_TENSOR, sizeof(vx_tensor_s)},
    {VX_TYPE_TENSOR_VIEW, sizeof(vx_tensor_view_s)},
    {VX_TYPE_TENSOR_ADDRESS, sizeof(vx_tensor_addressing_s)},
    {VX_TYPE_WEIGHTS_BIASES_PARAMETER, sizeof(vx_weights_biases_parameter_s)}
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
    {VX_TYPE_OBJECT_ARRAY, &vxoOA_DestructObjectArray},
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

    {VX_TYPE_TENSOR, &vxoTensor_Destructor},
    {VX_TYPE_TENSOR_VIEW, VX_NULL},
    {VX_TYPE_TENSOR_ADDRESS,VX_NULL},

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
        case VX_TYPE_TARGET:
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


VX_PRIVATE_API vx_status vxoContext_InitOptions(vx_context context)
{
    gctSTRING envctrl = gcvNULL;
    gctUINT i;
    char* tpFuncName[] =
    {
        "VIV_VX_ENABLE_TP_RESHUFFLE",
        "VIV_VX_ENABLE_TP_SINGLE_FC",
        "VIV_VX_ENABLE_TP_MAX_POOLING",
        "VIV_VX_ENABLE_TP_LEAK_RELU",
        "VIV_VX_ENABLE_TP_LEAK_RELU_MAX_POOLING",
        "VIV_VX_ENABLE_TP_LRN",
        "VIV_VX_ENABLE_TP_TRANSPOSE",
        "VIV_VX_ENABLE_TP_ROI_POOLING",
    };

    /* Enable driver TP path by default if HW has this feature */
    context->options.enableTP = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_TP", &envctrl)) && envctrl)
    {
        context->options.enableTP = atoi(envctrl);
    }
    /* All TP functions are enabled by default. */
    for (i = 0; i < TP_CMD_NUM; i++)
    {
        context->options.flagTPFunc[i] = 1;
        context->options.typeTPFunc[i] = 0;
    }
    /* Disable lrn and roi pooling by default */
    context->options.flagTPFunc[TP_LRN] = 0;
    context->options.flagTPFunc[TP_ROI_POOLING] = 0;

    for (i = 0; i < TP_CMD_NUM; i++)
    {
        envctrl = gcvNULL;
        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, tpFuncName[i], &envctrl)) && envctrl)
        {
            context->options.flagTPFunc[i] = atoi(envctrl);
        }
    }
    envctrl = gcvNULL;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_TP_RESHUFFLE_SEQUENCE", &envctrl)) && envctrl)
    {
        context->options.typeTPFunc[TP_RESHUFFLE] = atoi(envctrl);
    }
    envctrl = gcvNULL;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_TP_CROSS_MAP_LRN_PROCESS", &envctrl)) && envctrl)
    {
        context->options.typeTPFunc[TP_LRN] = atoi(envctrl);
    }
    envctrl = gcvNULL;
    context->options.enableMultiTP = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_MULTI_TP", &envctrl)) && envctrl)
    {
        context->options.enableMultiTP = atoi(envctrl);
    }

    /* Enable driver SRAM path by default if HW has this feature */
    envctrl = gcvNULL;
    context->options.enableSRAM = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_SRAM", &envctrl)) && envctrl)
    {
        context->options.enableSRAM = atoi(envctrl);
    }
    envctrl = gcvNULL;
    context->options.enableSramStreamMode = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_SRAM_STREAM_MODE", &envctrl)) && envctrl)
    {
        context->options.enableSramStreamMode = atoi(envctrl);
    }

    /* Disable CNN performance by default */
    envctrl = gcvNULL;
    context->options.enableCNNPerf = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "CNN_PERF", &envctrl)) && envctrl)
    {
        context->options.enableCNNPerf = atoi(envctrl);
    }

    context->options.graphPerfLogFile = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "VIV_VX_GRAPH_PERF", &context->options.graphPerfLogFile);


    envctrl = gcvNULL;
    context->options.nnZeroRunLen = 6;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_ZERO_RUN_LEN", &envctrl)) && envctrl)
    {
        context->options.nnZeroRunLen = atoi(envctrl);
    }

    envctrl = gcvNULL;
    context->options.tpZeroRunLen = -1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_TP_ZERO_RUN_LEN", &envctrl)) && envctrl)
    {
        gctINT value = atoi(envctrl);
        if ((value >= 0) && (value <= 9))
        {
            context->options.tpZeroRunLen = value;
        }
    }

    envctrl = gcvNULL;
    context->options.enableNNFCAccel = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_FC_ACCEL", &envctrl)) && envctrl)
    {
        context->options.enableNNFCAccel = atoi(envctrl);
    }

#define FC_ACCEL_THRESHOLD      926720
    envctrl = gcvNULL;
    context->options.nnFCAccelThreshold = FC_ACCEL_THRESHOLD;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_FCACCEL_THRESHOLD", &envctrl)) && envctrl)
    {
        context->options.nnFCAccelThreshold = atoi(envctrl);
    }

#define SUSTAINED_BANDWIDTH                    2.5f
    envctrl = gcvNULL;
    context->options.sustainedBandwidth = SUSTAINED_BANDWIDTH;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_SUSTAINED_BW", &envctrl)) && envctrl)
    {
        context->options.sustainedBandwidth = (gctFLOAT) atof(envctrl);
    }

    envctrl = gcvNULL;
    context->options.enableIdealPerf = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_IDEAL_PERF", &envctrl)) && envctrl)
    {
        context->options.enableIdealPerf = atoi(envctrl);
    }

    envctrl = gcvNULL;
    context->options.enableNNArchPerfPrint = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_SHOW_PERF", &envctrl)) && envctrl)
    {
        context->options.enableNNArchPerfPrint = atoi(envctrl);
    }

    envctrl = gcvNULL;
    context->options.enableNNLayerDump = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_LAYER_DUMP", &envctrl)) && envctrl)
    {
        context->options.enableNNLayerDump = atoi(envctrl);
    }

    envctrl = gcvNULL;
    context->options.enableBrickMode = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_BRICK_MODE", &envctrl)) && envctrl)
    {
        context->options.enableBrickMode = atoi(envctrl);
    }

    envctrl = gcvNULL;
    context->options.enableBorderMode = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_BORDER_MODE", &envctrl)) && envctrl)
    {
        context->options.enableBorderMode = atoi(envctrl);
    }

    context->options.nnRoundingMode = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "NN_ROUNDING_MODE", &context->options.nnRoundingMode);

    context->options.vxcShaderSourcePath = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "VX_SHADER_SOURCE_PATH", &context->options.vxcShaderSourcePath);

    context->options.fcZMax = (1<<14) - 1;


    return VX_SUCCESS;
}

VX_PRIVATE_API vx_context vxoContext_Create()
{
    vx_context context = VX_NULL;

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

        context->immediateBorderMode.mode = VX_BORDER_UNDEFINED;
        context->immediateBorderModePolicy  = VX_BORDER_POLICY_DEFAULT_TO_UNDEFINED;

        context->NextDynamicUserKernelID    = 0;
        context->NextDynamicUserLibraryID   = 1;

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
        vxoContext_InitOptions(context);
        context->deviceCount = 1;
        gcoVX_GetNNConfig(&context->nnConfig);

        /* memory maps table lock */
        vxCreateMutex(&context->memoryMapsLock);

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

gceSTATUS
gcfVX_UnloadCompiler(
    vx_context context
    )
{
    gceSTATUS   status = gcvSTATUS_OK;

    {
        if (context->unloadCompiler != gcvNULL)
        {
            gcmONERROR((*context->unloadCompiler)());

            gcmVERIFY_OK(gcoOS_FreeLibrary(gcvNULL, context->libCLC));

            context->libCLC             = gcvNULL;
            context->compileKernel      = gcvNULL;
            context->loadCompiler       = gcvNULL;
            context->unloadCompiler     = gcvNULL;
        }
    }

OnError:
    return status;
}

VX_PRIVATE_API vx_status vxoContext_Release(vx_context_ptr contextPtr)
{
    vx_context context;
    vx_uint32 i, j;
    vx_char* flag;
    gcoOS_GetEnv(gcvNULL, "ENABLE_TRACE_MEMORY", &flag);
    if (flag)
    {
        vx_uint32 size = 0;
        gcoVX_GetMemorySize(&size);
        if (size > 1000 * 1000)
            printf("Memory size = %.2f Mbyte\n", size/(1000.0f*1000.0f));
        else
            printf("Memory size = %d byte\n", size);
    }

    if (contextPtr == VX_NULL) return VX_ERROR_INVALID_REFERENCE;

    context = *contextPtr;
    *contextPtr = VX_NULL;

    if (context == VX_NULL) return VX_ERROR_INVALID_REFERENCE;

    vxmASSERT(vxContextGlobalLock);
    vxAcquireMutex(vxContextGlobalLock);

    gcfVX_Flush(gcvTRUE);

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
            vx_kernel kernel = &context->targetTable[0].kernelTable[0];
            context->targetTable[0].funcs.deinitialize(&context->targetTable[0]);

            vxoTarget_Unload(context, 0, vx_true_e);

            context->targetTable[0].enabled = vx_false_e;
            vxoKernel_InternalRelease(&kernel); /* release the first kernel object */
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

        vxReleaseMutex(context->memoryMapsLock);
        vxDestroyMutex(context->memoryMapsLock);

        for (i = 0; i < VXNNE_KERNEL_COUNT; i++)
        {
            for (j = 0; j < context->kernels[i].kernelShaderCount*2; j++)
            {
                vxoShader_Free(context->kernels[i].kernelShader[j]);
            }

            if (context->kernels[i].kernelShader) gcoOS_Free(gcvNULL, context->kernels[i].kernelShader);
        }

        gcfVX_UnloadCompiler(context);

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
    vx_border_t * borderMode;
    vx_enum * immediateBorderModePolicy;

    if (!vxoContext_IsValid(context)) return VX_ERROR_INVALID_REFERENCE;

    switch (attribute)
    {
        case VX_CONTEXT_IMMEDIATE_BORDER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_t, 0x3);

            borderMode = (vx_border_t *)ptr;

            if (!vxIsValidBorderMode(borderMode->mode)) return VX_ERROR_INVALID_VALUE;

            context->immediateBorderMode = *borderMode;
            break;
        case VX_CONTEXT_IMMEDIATE_BORDER_POLICY:

            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            immediateBorderModePolicy = (vx_enum *)ptr;

            if (!vxIsValidBorderModePolicy(*immediateBorderModePolicy)) return VX_ERROR_INVALID_VALUE;

            context->immediateBorderModePolicy = *immediateBorderModePolicy;
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
        case VX_CONTEXT_VENDOR_ID:
             vxmVALIDATE_PARAMETERS(ptr, size, vx_uint16, 0x1);

            *(vx_uint16 *)ptr = VX_ID_VIVANTE;
            break;

        case VX_CONTEXT_VERSION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint16, 0x1);

            *(vx_uint16 *)ptr = (vx_uint16)VX_VERSION;
            break;

        case VX_CONTEXT_MODULES:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = context->moduleCount;
            break;

        case VX_CONTEXT_REFERENCES:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = context->refCount;
            break;

#ifdef OPENVX_USE_TARGET
        /*the value of VX_CONTEXT_TARGETS is repetitous with VX_CONTEXT_IMMEDIATE_BORDER_POLICY's value*/
        /*case VX_CONTEXT_TARGETS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = context->targetCount;
            break;
        */
#endif

        case VX_CONTEXT_IMPLEMENTATION:
            if (size > VX_MAX_IMPLEMENTATION_NAME || ptr == VX_NULL)
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }

            vxStrCopySafe((vx_string)ptr, VX_MAX_IMPLEMENTATION_NAME, VX_IMPLEMENTATION_NAME);
            break;

        case VX_CONTEXT_EXTENSIONS_SIZE:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = sizeof(extensions);
            break;

        case VX_CONTEXT_EXTENSIONS:
            if (size >= sizeof(extensions) || ptr == VX_NULL)
            {
                return VX_ERROR_INVALID_PARAMETERS;
            }

            vxStrCopySafe((vx_string)ptr, sizeof(extensions), extensions);
            break;

        case VX_CONTEXT_CONVOLUTION_MAX_DIMENSION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = VX_MAX_CONVOLUTION_DIM;
            break;

        case VX_CONTEXT_OPTICAL_FLOW_MAX_WINDOW_DIMENSION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = VX_MAX_OPTICAL_FLOW_WINDOW_DIM;
            break;

        case VX_CONTEXT_IMMEDIATE_BORDER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_t, 0x3);

            *(vx_border_t *)ptr = context->immediateBorderMode;
            break;

        case VX_CONTEXT_IMMEDIATE_BORDER_POLICY:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            *(vx_enum *)ptr = context->immediateBorderModePolicy;
            break;

        case VX_CONTEXT_UNIQUE_KERNELS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

            *(vx_uint32 *)ptr = context->uniqueKernelCount;
            break;

        case VX_CONTEXT_UNIQUE_KERNEL_TABLE:
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

VX_API_ENTRY vx_status VX_API_CALL vxHint(vx_reference reference, vx_enum hint, const void* data, vx_size data_size)
{
    if (!vxoContext_IsValid((vx_context)reference) && !vxoReference_IsValidAndNoncontext(reference))
        return VX_ERROR_INVALID_REFERENCE;

    switch (hint)
    {
        case VX_HINT_SERIALIZE:
            if (vxoReference_IsValidAndSpecific(reference, VX_TYPE_GRAPH))

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
    vx_context context;

    /*if (!vxoContext_IsValid(reference->context)) return VX_ERROR_INVALID_REFERENCE;*/

    if (!vxoReference_IsValid(reference)) return VX_ERROR_INVALID_REFERENCE;

    context = ((reference->type == VX_TYPE_CONTEXT))?((vx_context)reference):(reference->context);

    switch (directive)
    {
        case VX_DIRECTIVE_DISABLE_LOGGING:
            context->logEnabled = vx_false_e;
            break;

        case VX_DIRECTIVE_ENABLE_LOGGING:
            context->logEnabled = vx_true_e;
            break;

        case VX_DIRECTIVE_DISABLE_PERFORMANCE:
            if (reference->type == VX_TYPE_CONTEXT)
                context->options.enableCNNPerf = 0;
            else
                return VX_ERROR_NOT_SUPPORTED;

            break;
        case VX_DIRECTIVE_ENABLE_PERFORMANCE:
            if (reference->type == VX_TYPE_CONTEXT)
                context->options.enableCNNPerf = 1;
            else
                return VX_ERROR_NOT_SUPPORTED;
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

static vx_target_s* vxoContext_FindTargetByString(vx_context context, const char* targetString)
{
    vx_uint32 index = 0;
    for (index = 0; index < context->targetCount; index++)
    {
        vx_uint32 targetIndex = context->targetPriorityTable[index];
        vx_target_s *target = &context->targetTable[targetIndex];
        if (vxoTarget_MatchTargetNameWithString(target->name, targetString) == vx_true_e)
        {
            return target;
        }
    }
    /* indicate 'not found' state */
    return (vx_target_s*)NULL;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetImmediateModeTarget(vx_context context, vx_enum target_enum, const char* target_string)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    vx_target_s* target = NULL;

    if (!vxoContext_IsValid(context)) return VX_TYPE_INVALID;


    switch (target_enum)
    {
        case VX_TARGET_ANY:
            context->immediateTargetEnum = VX_TARGET_ANY;
            memset(context->immediateTargetString, 0, sizeof(context->immediateTargetString));
            status = VX_SUCCESS;
            break;
        case VX_TARGET_STRING:
            target = vxoContext_FindTargetByString(context, target_string);
            if (target != NULL)
            {
                context->immediateTargetEnum = VX_TARGET_STRING;
                strncpy(context->immediateTargetString, target_string, sizeof(context->immediateTargetString));
                context->immediateTargetString[sizeof(context->immediateTargetString) - 1] = '\0';
                status = VX_SUCCESS;
            }
            else /* target was not found */
            {
                status = VX_ERROR_NOT_SUPPORTED;
            }
            break;
        default:
            status = VX_ERROR_NOT_SUPPORTED;
            break;
    }

    return status;
}

#if MAP_UNMAP_REFERENCE
VX_INTERNAL_API vx_bool vxoContext_MemoryMap(
    vx_context   context,
    vx_reference ref,
    vx_size      size,
    vx_enum      usage,
    vx_enum      mem_type,
    vx_uint32    flags,
    void*        extra_data,
    void**       ptr,
    vx_map_id*   map_id)
{
    vx_uint32 id;
    vx_bool worked = vx_false_e;

    /* lock the table for modification */
    if (vx_true_e == vxAcquireMutex(context->memoryMapsLock))
    {
        for (id = 0u; id < vxmLENGTH_OF(context->memoryMaps); id++)
        {
            if (context->memoryMaps[id].used == vx_false_e)
            {
                vx_memory_map_extra_s* extra;
                vx_uint8_ptr buf = 0;
                vxError("Found free memory map slot[%u]\n", id);

                /* allocate mapped buffer */
                //buf = (vx_uint8*)malloc(size);
                if (size > 0)
                {
                    buf = (vx_uint8_ptr)vxAllocate(size);
                    context->memoryCount ++;

                    if (buf == NULL)
                        break;
                }

                context->memoryMaps[id].used       = vx_true_e;
                context->memoryMaps[id].ref        = ref;
                context->memoryMaps[id].logical    = buf;
                context->memoryMaps[id].usage      = usage;
                context->memoryMaps[id].mem_type   = mem_type;
                context->memoryMaps[id].flags      = flags;

                extra = (vx_memory_map_extra_s*)extra_data;
                if (VX_TYPE_IMAGE == ref->type)
                {
                    context->memoryMaps[id].extra.image_data.plane_index = extra->image_data.plane_index;
                    context->memoryMaps[id].extra.image_data.rect        = extra->image_data.rect;
                }
                else if (VX_TYPE_ARRAY == ref->type || VX_TYPE_LUT == ref->type)
                {
                    vx_memory_map_extra_s* extra = (vx_memory_map_extra_s*)extra_data;
                    context->memoryMaps[id].extra.array_data.start = extra->array_data.start;
                    context->memoryMaps[id].extra.array_data.end   = extra->array_data.end;
                }

                *ptr = buf;
                *map_id = (vx_map_id)id;

                worked = vx_true_e;

                break;
            }
        }

        /* we're done, unlock the table */
        worked = vxReleaseMutex(context->memoryMapsLock);
    }
    else
        worked = vx_false_e;

    return worked;
} /* vxoContext_MemoryMap() */
#else
VX_INTERNAL_API vx_bool vxoContext_MemoryMap(
    vx_context   context,
    vx_reference ref,
    vx_size      size,
    vx_enum      usage,
    vx_enum      mem_type,
    vx_uint32    flags,
    void*        extra_data,
    void**       ptr,
    vx_map_id*   map_id)
{
    vx_uint32 id;
    vx_bool worked = vx_false_e;

    /* lock the table for modification */
    if (vx_true_e == vxAcquireMutex(context->memoryMapsLock))
    {
        for (id = 0u; id < vxmLENGTH_OF(context->memoryMaps); id++)
        {
            if (context->memoryMaps[id].used == vx_false_e)
            {
                vx_memory_map_extra_s* extra;
                vx_uint8_ptr buf = 0;
                vxError("Found free memory map slot[%u]\n", id);

                /* allocate mapped buffer */
                //buf = (vx_uint8*)malloc(size);
                if (size > 0)
                {
                    if (VX_TYPE_WEIGHTS_BIASES_PARAMETER == ref->type)
                    {
                        buf = (vx_uint8_ptr)&((vx_weights_biases_parameter)ref)->memory.logicals[0][0] - ((vx_weights_biases_parameter)ref)->memroy_head_offset;
                    }
                    else if (VX_TYPE_ARRAY == ref->type || VX_TYPE_LUT == ref->type)
                    {
                        vx_memory_map_extra_s* extra = (vx_memory_map_extra_s*)extra_data;
                        vx_size offset = extra->array_data.start * ((vx_array)ref)->itemSize;
                        buf = (vx_uint8_ptr)&((vx_array)ref)->memory.logicals[0][offset];
                    }
                    else if (VX_TYPE_IMAGE == ref->type)
                    {
                        vx_memory_map_extra_s* extra = (vx_memory_map_extra_s*)extra_data;
                        vx_image image = (vx_image)ref;
                        vx_rectangle_t * rect = &extra->image_data.rect;
                        vx_uint32 offset = vxComputePlaneOffset(image, rect->start_x, rect->start_y, extra->image_data.plane_index);
                        buf = (vx_uint8_ptr)&((vx_image)ref)->memory.logicals[extra->image_data.plane_index][offset];
                    }
                    else if (VX_TYPE_DISTRIBUTION == ref->type)
                    {
                        buf = (vx_uint8_ptr)((vx_distribution)ref)->memory.logicals[0];
                    }
                    else {
                        buf = (vx_uint8_ptr)vxAllocate(size);
                    }

                    context->memoryCount ++;

                    if (buf == NULL)
                        break;
                }

                context->memoryMaps[id].used       = vx_true_e;
                context->memoryMaps[id].ref        = ref;
                context->memoryMaps[id].logical    = buf;
                context->memoryMaps[id].usage      = usage;
                context->memoryMaps[id].mem_type   = mem_type;
                context->memoryMaps[id].flags      = flags;

                extra = (vx_memory_map_extra_s*)extra_data;
                if (VX_TYPE_IMAGE == ref->type)
                {
                    context->memoryMaps[id].extra.image_data.plane_index = extra->image_data.plane_index;
                    context->memoryMaps[id].extra.image_data.rect        = extra->image_data.rect;
                }
                else if (VX_TYPE_ARRAY == ref->type || VX_TYPE_LUT == ref->type)
                {
                    vx_memory_map_extra_s* extra = (vx_memory_map_extra_s*)extra_data;
                    context->memoryMaps[id].extra.array_data.start = extra->array_data.start;
                    context->memoryMaps[id].extra.array_data.end   = extra->array_data.end;
                }

                *ptr = buf;
                *map_id = (vx_map_id)id;

                worked = vx_true_e;

                break;
            }
        }

        /* we're done, unlock the table */
        worked = vxReleaseMutex(context->memoryMapsLock);
    }
    else
        worked = vx_false_e;

    return worked;
} /* vxoContext_MemoryMap() */

#endif

VX_INTERNAL_API vx_bool vxoContext_FindMemoryMap(
    vx_context   context,
    vx_reference ref,
    vx_map_id    map_id)
{
    vx_bool worked = vx_false_e;
    vx_uint32 id = (vx_uint32)map_id;

    /* check index range */
    if (id < vxmLENGTH_OF(context->memoryMaps))
    {
        /* lock the table for exclusive access */
        if (vx_true_e == vxAcquireMutex(context->memoryMapsLock))
        {
            if ((context->memoryMaps[id].used == vx_true_e) && (context->memoryMaps[id].ref == ref))
            {
                worked = vx_true_e;
            }

            /* unlock teh table */
            worked = vxReleaseMutex(context->memoryMapsLock);
        }
    }

    return worked;
} /* vxoContext_FindMemoryMap() */

#if MAP_UNMAP_REFERENCE
VX_INTERNAL_API void vxoContext_MemoryUnmap(vx_context context, vx_map_id m_id)
{
    vx_uint32 map_id = (vx_uint32)m_id;

    /* lock the table for modification */
    if (vx_true_e == vxAcquireMutex(context->memoryMapsLock))
    {
        if (context->memoryMaps[map_id].used == vx_true_e &&
            context->memoryMaps[map_id].logical != NULL)
        {
            /* freeing mapped buffer */
            //free(context->memoryMaps[map_id].ptr);

            vxFree(context->memoryMaps[map_id].logical);

            memset(&context->memoryMaps[map_id], 0, sizeof(vx_memory_map_s));
            vxError("Removed memory mapping[%u]\n", map_id);

            context->memoryCount --;
        }

        context->memoryMaps[map_id].used = vx_false_e;

        /* we're done, unlock the table */
        vxReleaseMutex(context->memoryMapsLock);
    }
    else
        vxError("vxAcquireMutex() failed!\n");

    return;
} /* vxoContext_MemoryUnmap() */
#else
VX_INTERNAL_API void vxoContext_MemoryUnmap(vx_context context, vx_map_id m_id)
{
    vx_uint32 map_id = (vx_uint32)m_id;

    /* lock the table for modification */
    if (vx_true_e == vxAcquireMutex(context->memoryMapsLock))
    {
        if (context->memoryMaps[map_id].used == vx_true_e &&
            context->memoryMaps[map_id].logical != NULL)
        {
            /* freeing mapped buffer */
            /*free(context->memoryMaps[map_id].ptr);*/
            if ((VX_TYPE_WEIGHTS_BIASES_PARAMETER != context->memoryMaps[map_id].ref->type)
                && (VX_TYPE_ARRAY != context->memoryMaps[map_id].ref->type)
                && (VX_TYPE_DISTRIBUTION != context->memoryMaps[map_id].ref->type)
                && (VX_TYPE_IMAGE != context->memoryMaps[map_id].ref->type)
                && (VX_TYPE_LUT != context->memoryMaps[map_id].ref->type)
                )
            {
                vxFree(context->memoryMaps[map_id].logical);
            }

            memset(&context->memoryMaps[map_id], 0, sizeof(vx_memory_map_s));
            vxError("Removed memory mapping[%u]\n", map_id);

            context->memoryCount --;
        }

        context->memoryMaps[map_id].used = vx_false_e;

        /* we're done, unlock the table */
        vxReleaseMutex(context->memoryMapsLock);
    }
    else
        vxError("vxAcquireMutex() failed!\n");

    return;
} /* vxoContext_MemoryUnmap() */
#endif

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelId(vx_context context, vx_enum * pKernelEnumId)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if ((vxoContext_IsValid(context) == vx_true_e) && pKernelEnumId)
    {
        status = VX_ERROR_NO_RESOURCES;
        if(context->NextDynamicUserKernelID <= VX_KERNEL_MASK)
        {
            *pKernelEnumId = VX_KERNEL_BASE(VX_ID_USER,0) + context->NextDynamicUserKernelID++;
            status = VX_SUCCESS;
        }
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelLibraryId(vx_context context, vx_enum * pLibraryId)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    if ((vxoContext_IsValid(context) == vx_true_e) && pLibraryId)
    {
        status = VX_ERROR_NO_RESOURCES;
        if(context->NextDynamicUserLibraryID <= VX_LIBRARY(VX_LIBRARY_MASK))
        {
            *pLibraryId = context->NextDynamicUserLibraryID++;
            status = VX_SUCCESS;
        }
    }
    return status;
}

