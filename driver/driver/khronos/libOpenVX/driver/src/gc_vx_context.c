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
#include <gc_vx_nn_util.h>
/*
** Misc
*/
#define VX_IMPLEMENTATION_NAME          "Vivante"
#define _GC_OBJ_ZONE            gcdZONE_VX_CONTEXT

gceSTATUS gcLoadKernelCompiler(IN gcsHWCaps *HWCaps, IN gcePATCH_ID PatchId);
gceSTATUS gcUnloadKernelCompiler(void);
static vx_int32 vxDebugLevel = VX_DEBUG_LEVEL_NONE;
static vx_global_data vxGlobalData = VX_NULL;

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
    {VX_TYPE_BFLOAT16, sizeof(vx_uint16)},
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
    {VX_TYPE_HOG_PARAMS, sizeof(vx_hog_t)},
    {VX_TYPE_LINE_2D, sizeof(vx_line2d_t)},
    {VX_TYPE_HOUGH_LINES_PARAMS, sizeof(vx_hough_lines_p_t)},

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

    {VX_TYPE_WEIGHTS_BIASES_PARAMETER, sizeof(vx_weights_biases_parameter_s)},
    {VX_TYPE_WEIGHTS_BIASES_PARAMETER_BASE, sizeof(vx_weights_biases_parameter_base_s)}
};

VX_INTERNAL_API vx_uint32 vxDataType_GetSize(vx_type_e type)
{
    vx_uint32 i;

    for (i = 0; i < vxmLENGTH_OF(vxDataTypeSizeRecords); i++)
    {
        if (type == vxDataTypeSizeRecords[i].type)
        {
            return (vx_uint32)vxDataTypeSizeRecords[i].size;
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

    vxError("The type, %d, is invalid", type);

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

    {VX_TYPE_WEIGHTS_BIASES_PARAMETER, &vxoWeightsBiases_Destructor},
    {VX_TYPE_WEIGHTS_BIASES_PARAMETER_BASE, &vxoWeightsBiasesBase_Destructor},

    {VX_TYPE_PROGRAM, &vxoProgram_Destructor}
};

VX_INTERNAL_API vx_object_destructor_f vxDataType_GetDestructor(vx_type_e type)
{
    vx_uint32 i = 0;

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
#if gcdUSE_SINGLE_CONTEXT
static vx_context   vxSingletonContext  = VX_NULL;
#endif

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
    gcmHEADER_ARG("arg=%p", arg);
    vxInfo("Created VX Thread: %p\n", gcoOS_GetCurrentThreadID());

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
    vxInfo("Exit VX Thread: %p\n", gcoOS_GetCurrentThreadID());
    gcmFOOTER_NO();
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


VX_PRIVATE_API void vxoGlobalData_FetchOptionsForTransferGraph(vx_global_data globalData, gctSTRING envctrl)
{
    gctSTRING pos = gcvNULL;
    gcmHEADER_ARG("globalData=%p, envctrl=%s", globalData, envctrl);
    if(gcoOS_StrStr(envctrl, "-closeAll", &pos) && pos)
    {
        globalData->options.enableGraphTranform = 0;
        gcmFOOTER_NO();
        return;
    }

    if(gcoOS_StrStr(envctrl, "-Merge:", &pos) && pos)
    {
        pos += sizeof("-Merge:") -1;
        globalData->options.enableGraphMerge  = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-Dump", &pos) && pos)
    {
        globalData->options.enableGraphDump = 1;
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-ConvertBatchFC:", &pos) && pos)
    {
        pos += sizeof("-ConvertBatchFC:") -1;
        globalData->options.enableGraphConvertBatchFC2NNConv = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-ConvertAvgPool:", &pos) && pos)
    {
        pos += sizeof("-ConvertAvgPool:") -1;
        globalData->options.enableGraphConvertAvgPool2Conv = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-unrollDWConv:", &pos) && pos)
    {
        pos += sizeof("-unrollDWConv:") -1;
        globalData->options.enableGraphUnrollDWConv = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-tensorAdd:", &pos) && pos)
    {
        pos += sizeof("-tensorAdd:") -1;
        globalData->options.enableGraphConvertTensorAdd = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-eltwiseOp:", &pos) && pos)
    {
        pos += sizeof("-eltwiseOp:") -1;
        globalData->options.enableGraphEltwiseOpShape = atoi(pos);
    }


    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-swap:", &pos) && pos)
    {
        pos += sizeof("-swap:") -1;
        globalData->options.enableGraphSwaplayer = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-deleteReshape:", &pos) && pos)
    {
        pos += sizeof("-deleteReshape:") -1;
        globalData->options.enableGraphReshapelayer = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-deleteConcat:", &pos) && pos)
    {
        pos += sizeof("-deleteConcat:") -1;
        globalData->options.enableGraphConcalayer = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-alignNMConv:", &pos) && pos)
    {
        pos += sizeof("-alignNMConv:") -1;
        globalData->options.enableTransformNMConv = atoi(pos);
    }
    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-conv2fc:", &pos) && pos)
    {
        pos += sizeof("-conv2fc:") -1;
        globalData->options.enableGraphConvertConv2Fc = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-mergeTranspose:", &pos) && pos)
    {
        pos += sizeof("-mergeTranspose:") -1;
        globalData->options.enableGraphMergeTranspose = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-padConv:", &pos) && pos)
    {
        pos += sizeof("-padConv:") -1;
        globalData->options.enableGraphPadConv = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-deleteRelu:", &pos) && pos)
    {
        pos += sizeof("-deleteRelu:") - 1;
        globalData->options.enableGraphDeleteRelu = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-deleteSqueeze:", &pos) && pos)
    {
        pos += sizeof("-deleteSqueeze:") - 1;
        globalData->options.enableGraphDeleteSqueeze = atoi(pos);
    }

    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-war1x1x1weight:", &pos) && pos)
    {
        pos += sizeof("-war1x1x1weight:") - 1;
        globalData->options.enableGraphWar1x1x1weight = atoi(pos);
    }


    pos = gcvNULL;
    if(gcoOS_StrStr(envctrl, "-WAR", &pos) && pos)
    {
        pos += sizeof("-WAR") -1;
        while(pos[0] == ':')
        {
            ++pos;
            vxInfo("doing WAR%d\n", atoi(pos));
            switch( atoi(pos) )
            {
            case 7:
                globalData->options.enableGraphWAR7 = 1;
                ++pos;
                break;
            default:
                break;
            }
        }
    }
    gcmFOOTER_NO();
}

vx_status gcoVX_ForceTpCoreCount(vx_global_data globalData)
{

    if(globalData->options.tpCoreCount <= globalData->nnConfig.fixedFeature.tpCoreCount)
        globalData->nnConfig.fixedFeature.tpCoreCount = globalData->options.tpCoreCount;
    else
        vxInfo("\nWARNING: VIV_VX_TP_CORE_COUNT(%d) beyound HW configure(%d)\n", globalData->options.tpCoreCount, globalData->nnConfig.fixedFeature.tpCoreCount);

    if(globalData->options.tpLiteCoreCount <= globalData->nnConfig.fixedFeature.tpliteCoreCount)
        globalData->nnConfig.fixedFeature.tpliteCoreCount = globalData->options.tpLiteCoreCount;
    else
        vxInfo("\nWARNING: VIV_VX_TPLITE_CORE_COUNT(%d) beyound HW configure(%d)\n", globalData->options.tpLiteCoreCount, globalData->nnConfig.fixedFeature.tpliteCoreCount);

    return VX_SUCCESS;

}

VX_PRIVATE_API vx_status vxoGlobalData_InitOptions(vx_global_data globalData)
{
    gctSTRING envctrl = gcvNULL;
    gctUINT i;
    char* tpFuncName[] =
    {
        "VIV_VX_NONE",
        "VIV_VX_ENABLE_TP_RESHUFFLE",
        "VIV_VX_ENABLE_TP_SINGLE_FC",
        "VIV_VX_ENABLE_TP_MAX_POOLING",
        "VIV_VX_ENABLE_TP_ACTIVATION",
        "VIV_VX_ENABLE_TP_LRN",
        "VIV_VX_ENABLE_TP_TRANSPOSE",
        "VIV_VX_ENABLE_TP_ROI_POOLING",
        "VIV_VX_ENABLE_TP_REORG",
        "VIV_VX_ENABLE_TP_REORG_DEPTH2SPACE",
        "VIV_VX_ENABLE_TP_REORG_SPACE2DEPTH",
        "VIV_VX_ENABLE_TP_REORG_SPACE2BATCH",
        "VIV_VX_ENABLE_TP_REORG_BATCH2SPACE",
        "VIV_VX_ENABLE_TP_ADD",
        "VIV_VX_ENABLE_TP_REVERSE",
        "VIV_VX_ENABLE_TP_UPSAMPLE",
        "VIV_VX_ENABLE_TP_DILATE_UPSAMPLE",
        "VIV_VX_ENABLE_TP_DILATE_UPSAMPLE2",
        "VIV_VX_ENABLE_TP_DILATE_RESHUFFLE",
        "VIV_VX_ENABLE_TP_BRICK",
        "VIV_VX_ENABLE_TP_RNN_INTERLEAVE",
        "VIV_VX_ENABLE_TP_TENSOR_COPY",
        "VIV_VX_ENABLE_TP_TENSOR_PAD",
        "VIV_VX_ENABLE_TP_LSTM_RESHUFFLE_INPUT",
        "VIV_VX_ENABLE_TP_LSTM_STATE_OUT",
        "VIV_VX_ENABLE_TP_LSTM_RESHUFFLE_STATE_O",
        "VIV_VX_ENABLE_TP_CMD_NUM",
        "VIV_VX_ENABLE_TP_ROI_POOLING_STEP_1",
        "VIV_VX_ENABLE_TP_ROI_POOLING_STEP_2",
        "VIV_VX_ENABLE_TP_UPSAMPLE_CLIP",
        "VIV_VX_ENABLE_TP_TENSOR_SQUEEZE",
        "VIV_VX_ENABLE_TP_TENSOR_PAD_CN",
        "VIV_VX_ENABLE_TP_TENSOR_COPY4CONCAT",
        "VIV_VX_ENABLE_TP_TENSOR_STRIDED_SLICE",
        "VIV_VX_ENABLE_TP_TENSOR_SVDF_MAP",
    };
    vx_bool isV8 = (globalData->nnConfig.derivedFeature.nnXYDPX == 0
                   && globalData->nnConfig.derivedFeature.nnXYDPY == 0)
                   ? vx_true_e : vx_false_e;

    gcmHEADER_ARG("globalData=%p", globalData);

    /* Enable driver TP path by default if HW has this feature */
    globalData->options.enableTP = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_TP", &envctrl)) && envctrl)
    {
        globalData->options.enableTP = atoi(envctrl);
    }

    gcmASSERT(gcmCOUNTOF(tpFuncName) == TP_TENSOR_COUNT);

    globalData->options.flagTPFunc = (vx_uint8_ptr)vxAllocateAndZeroMemory(gcmCOUNTOF(tpFuncName) * sizeof(vx_uint8));
    globalData->options.typeTPFunc = (vx_uint32_ptr)vxAllocateAndZeroMemory(gcmCOUNTOF(tpFuncName) * sizeof(vx_uint32));

    gcmASSERT(globalData->options.flagTPFunc != VX_NULL);
    gcmASSERT(globalData->options.typeTPFunc != VX_NULL);

    /* All TP functions are enabled by default. */
    for (i = 0; i < gcmCOUNTOF(tpFuncName); i++)
    {
        globalData->options.flagTPFunc[i] = 1;
        globalData->options.typeTPFunc[i] = 0;
    }
    /* Disable TP add to wait for HW ready. */
    globalData->options.flagTPFunc[TP_ADD] = 0;

    for (i = 0; i < gcmCOUNTOF(tpFuncName); i++)
    {
        envctrl = gcvNULL;
        if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, tpFuncName[i], &envctrl)) && envctrl)
        {
            globalData->options.flagTPFunc[i] = (vx_uint8)atoi(envctrl);
        }
    }
    envctrl = gcvNULL;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_TP_RESHUFFLE_SEQUENCE", &envctrl)) && envctrl)
    {
        globalData->options.typeTPFunc[TP_RESHUFFLE] = atoi(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.enableMultiTP = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_MULTI_TP", &envctrl)) && envctrl)
    {
        globalData->options.enableMultiTP = atoi(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.enableTPReorder = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_TP_REORDER", &envctrl)) && envctrl)
    {
        globalData->options.enableTPReorder = atoi(envctrl);
    }

    /* Enable driver SRAM path by default if HW has this feature */
    envctrl = gcvNULL;
    globalData->options.enableSRAM = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_SRAM", &envctrl)) && envctrl)
    {
        globalData->options.enableSRAM = atoi(envctrl);
    }
    envctrl = gcvNULL;

    globalData->options.enableSramStreamMode = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_SRAM_STREAM_MODE", &envctrl)) && envctrl)
    {
        globalData->options.enableSramStreamMode = atoi(envctrl);
    }

    /* Disable CNN performance by default */
    envctrl = gcvNULL;
    globalData->options.enableCNNPerf = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "CNN_PERF", &envctrl)) && envctrl)
    {
        globalData->options.enableCNNPerf = atoi(envctrl);
    }

    globalData->options.graphPerfLogFile = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "VIV_VX_GRAPH_PERF", &globalData->options.graphPerfLogFile);


    envctrl = gcvNULL;
    globalData->options.nnZeroRunLen = 6;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_ZERO_RUN_LEN", &envctrl)) && envctrl)
    {
        globalData->options.nnZeroRunLen = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.tpZeroRunLen = -1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_TP_ZERO_RUN_LEN", &envctrl)) && envctrl)
    {
        gctINT value = atoi(envctrl);
        if ((value >= 0) && (value <= 9))
        {
            globalData->options.tpZeroRunLen = value;
        }
    }

    envctrl = gcvNULL;
    globalData->options.ddrReadBWLimit = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_DDR_READ_BW_LIMIT", &envctrl)) && envctrl)
    {
        globalData->options.ddrReadBWLimit = (gctFLOAT) atof(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.ddrWriteBWLimit = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_DDR_WRITE_BW_LIMIT", &envctrl)) && envctrl)
    {
        globalData->options.ddrWriteBWLimit = (gctFLOAT) atof(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.ddrTotalBWLimit = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_DDR_TOTAL_BW_LIMIT", &envctrl)) && envctrl)
    {
        globalData->options.ddrTotalBWLimit = (gctFLOAT) atof(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.axiSramReadBWLimit = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_AXI_SRAM_READ_BW_LIMIT", &envctrl)) && envctrl)
    {
        globalData->options.axiSramReadBWLimit = (gctFLOAT) atof(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.axiSramWriteBWLimit = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_AXI_SRAM_WRITE_BW_LIMIT", &envctrl)) && envctrl)
    {
        globalData->options.axiSramWriteBWLimit = (gctFLOAT) atof(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.axiSramTotalBWLimit = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_AXI_SRAM_TOTAL_BW_LIMIT", &envctrl)) && envctrl)
    {
        globalData->options.axiSramTotalBWLimit = (gctFLOAT) atof(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.axiBusReadBWLimit = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_AXI_BUS_READ_BW_LIMIT", &envctrl)) && envctrl)
    {
        globalData->options.axiBusReadBWLimit = (gctFLOAT) atof(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.axiBusWriteBWLimit = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_AXI_BUS_WRITE_BW_LIMIT", &envctrl)) && envctrl)
    {
        globalData->options.axiBusWriteBWLimit = (gctFLOAT) atof(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.axiBusTotalBWLimit = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_AXI_BUS_TOTAL_BW_LIMIT", &envctrl)) && envctrl)
    {
        globalData->options.axiBusTotalBWLimit = (gctFLOAT) atof(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.vipSRAMSize = VX_INVALID_VALUE;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_VIP_SRAM_SIZE", &envctrl)) && envctrl)
    {
        globalData->options.vipSRAMSize = atoi(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.axiSRAMSize = VX_INVALID_VALUE;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_AXI_SRAM_SIZE", &envctrl)) && envctrl)
    {
        globalData->options.axiSRAMSize = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableNNArchPerfPrint = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_EXT_SHOW_PERF", &envctrl)) && envctrl)
    {
        globalData->options.enableNNArchPerfPrint = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableNNLayerDump = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "NN_LAYER_DUMP", &envctrl)) && envctrl)
    {
        globalData->options.enableNNLayerDump = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableBrickMode = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_BRICK_MODE", &envctrl)) && envctrl)
    {
        globalData->options.enableBrickMode = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableNonZeroBalance = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_NON_ZERO_BALANCE", &envctrl)) && envctrl)
    {
        globalData->options.enableNonZeroBalance = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableBorderMode = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_BORDER_MODE", &envctrl)) && envctrl)
    {
        globalData->options.enableBorderMode = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableInterleave8 = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_INTERLEAVE8", &envctrl)) && envctrl)
    {
        globalData->options.enableInterleave8 = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableShader = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_SHADER", &envctrl)) && envctrl)
    {
        globalData->options.enableShader = atoi(envctrl);
    }

    globalData->options.nnRoundingMode = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "NN_ROUNDING_MODE", &globalData->options.nnRoundingMode);

    globalData->options.vxcShaderSourcePath = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "VX_SHADER_SOURCE_PATH", &globalData->options.vxcShaderSourcePath);

    globalData->options.fcZMax = gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7) ? (1<<20) - 1 : (1<<14) - 1;

    envctrl = gcvNULL;
    globalData->options.enableMemPool = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_VIRTUAL_BUFFER", &envctrl)) && envctrl)
    {
        globalData->options.enableMemPool = atoi(envctrl);
    }
    envctrl = gcvNULL;
    globalData->options.memPoolSize = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_VIRTUAL_BUFFER_SIZE", &envctrl)) && envctrl)
    {
        globalData->options.memPoolSize = atoi(envctrl);
    }
    if (globalData->options.enableNNLayerDump) globalData->options.enableMemPool = 0;

    envctrl = gcvNULL;
    globalData->options.enablePrintOperaTarget = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_PRINT_TARGET", &envctrl)) && envctrl)
    {
        globalData->options.enablePrintOperaTarget = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.collectPerfType = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_COLLECT_PERF_TYPE", &envctrl)) && envctrl)
    {
        globalData->options.collectPerfType = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableHandleBranch = gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_HI_REORDER_FIX) ? 1 : 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_HANDLE_BRANCH", &envctrl)) && envctrl)
    {
        globalData->options.enableHandleBranch = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableSaveBinary = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_SAVE_NETWORK_BINARY", &envctrl)) && envctrl)
    {
        globalData->options.enableSaveBinary = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableGraphAdapter = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_GRAPH_ADAPTER", &envctrl)) && envctrl)
    {
        globalData->options.enableGraphAdapter = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableNNTranspose = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_NN_TRANSPOSE", &envctrl)) && envctrl)
    {
        globalData->options.enableNNTranspose = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableZdpOpt = vxoGlobalData_IsFeatureAvailable(globalData,VX_NN_FEATURE_NN_TRANSPOSE) ? 0 : 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_ZDP_OPT", &envctrl)) && envctrl)
    {
        globalData->options.enableZdpOpt = atoi(envctrl);
    }

    globalData->options.enableGraphCommandBuffer = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_GRAPH_COMMANDBUFFER", &envctrl)) && envctrl)
    {
        globalData->options.enableGraphCommandBuffer = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableNNXYDP9 = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_NN_XYDP9", &envctrl)) && envctrl)
    {
        globalData->options.enableNNXYDP9 = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableNNFirstPixelPooling = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_NN_FIRST_PIXEL_POOLING", &envctrl)) && envctrl)
    {
        globalData->options.enableNNFirstPixelPooling = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableSwtilingPhase1 = VX_SWTILING_OPTION_ALL;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_SWTILING_PHASE1", &envctrl)) && envctrl)
    {
        globalData->options.enableSwtilingPhase1 = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableSwtilingPhase2 = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_SWTILING_PHASE2", &envctrl)) && envctrl)
    {
        globalData->options.enableSwtilingPhase2 = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableSwtilingPhase3 = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_SWTILING_PHASE3", &envctrl)) && envctrl)
    {
        globalData->options.enableSwtilingPhase3 = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.ddrLatency = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_DDR_LATENCY", &envctrl)) && envctrl)
    {
        globalData->options.ddrLatency = (gctFLOAT) atof(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.freqInMHZ = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_FREQ_IN_MHZ", &envctrl)) && envctrl)
    {
        globalData->options.freqInMHZ = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.axiClockFreqInMHZ = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_AXI_CLOCK_FREQ_IN_MHZ", &envctrl)) && envctrl)
    {
        globalData->options.axiClockFreqInMHZ = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableVectorPrune = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_VECTOR_PRUNE", &envctrl)) && envctrl)
    {
        globalData->options.enableVectorPrune = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.maxSocOTNumber = 0;/*max SOC outstanding transfer number*/
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_MAX_SOC_OT_NUMBER", &envctrl)) && envctrl)
    {
        globalData->options.maxSocOTNumber = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableNNXYDP6 = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_NN_XYDP6", &envctrl)) && envctrl)
    {
        globalData->options.enableNNXYDP6 = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.nn1x1To1xN = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_NN_1X1_TO_1XN", &envctrl)) && envctrl)
    {
        globalData->options.nn1x1To1xN = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.do1xnAfterSwtiling = isV8 ? 0 : 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_DO_1XN_AFTER_SWTILING", &envctrl)) && envctrl)
    {
        globalData->options.do1xnAfterSwtiling = atoi(envctrl);
    }

    envctrl = gcvNULL;

    globalData->options.enableHuffmanEnhancement = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_HUFFMAN", &envctrl)) && envctrl)
    {
        globalData->options.enableHuffmanEnhancement = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableTPHuffman = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_TP_HUFFMAN", &envctrl)) && envctrl)
    {
        globalData->options.enableTPHuffman = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableNNDepthWiseSupport = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_NN_DEPTHWISE_SUPPORT", &envctrl)) && envctrl)
    {
        globalData->options.enableNNDepthWiseSupport = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableGraphTranform = 1;
    globalData->options.enableGraphMerge = 1;
    globalData->options.enableGraphDump = 0;
    globalData->options.enableGraphWAR7 = 0;
    globalData->options.enableGraphOptimizationToTest = 0;
    globalData->options.enableGraphConvertBatchFC2NNConv = 1;
    globalData->options.enableGraphConvertAvgPool2Conv = 1;
    globalData->options.enableGraphConvertTensorAdd = 1;
    globalData->options.enableGraphEltwiseOpShape = 1;
    globalData->options.enableGraphUnrollDWConv = 1;
    globalData->options.enableGraphConvertConv2Fc = 1;
    globalData->options.enableGraphSwaplayer = 1;
    globalData->options.enableGraphReshapelayer = 1;
    globalData->options.enableGraphConcalayer = 0;
    globalData->options.enableTransformNMConv = 1;
    globalData->options.enableGraphMergeTranspose = 1;
    globalData->options.enableGraphPadConv = 1;
    globalData->options.enableGraphDeleteRelu = 0;
    globalData->options.enableGraphDeleteSqueeze = 1;
    globalData->options.enableGraphWar1x1x1weight = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_GRAPH_TRANSFORM", &envctrl)) && envctrl)
    {
        vxoGlobalData_FetchOptionsForTransferGraph(globalData, envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableMultiVIPCombined = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_MULTIVIP_COMBINED", &envctrl)) && envctrl)
    {
        globalData->options.enableMultiVIPCombined = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableNNTPParallel = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_NNTP_PARALLEL", &envctrl)) && envctrl)
    {
        globalData->options.enableNNTPParallel = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableYUV2RGBScaler = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_YUV2RGB_SCALER", &envctrl)) && envctrl)
    {
        globalData->options.enableYUV2RGBScaler = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableVIPDEC400 = 1;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_VIP_DEC400", &envctrl)) && envctrl)
    {
        globalData->options.enableVIPDEC400 = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableCacheBinaryGraph = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_CACHE_GRAPH_BINARY", &envctrl)) && envctrl)
    {
        globalData->options.enableCacheBinaryGraph = atoi(envctrl);
    }

    envctrl = gcvNULL;
#if gcdDEBUG
    vxDebugLevel = VX_DEBUG_LEVEL_INFO;
#else
    vxDebugLevel = VX_DEBUG_LEVEL_NONE;
#endif
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_DEBUG_LEVEL", &envctrl)) && envctrl)
    {
        vxDebugLevel = atoi(envctrl);
    }


    globalData->options.enableOpsDebugInfo = gcvNULL;
    gcoOS_GetEnv(gcvNULL, "VIV_VX_OPS_DUMP_PATH", &globalData->options.enableOpsDebugInfo);

    envctrl = gcvNULL;
    globalData->options.tpCoreCount = globalData->nnConfig.fixedFeature.tpCoreCount;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_TP_CORE_COUNT", &envctrl)) && envctrl)
    {
        globalData->options.tpCoreCount = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.tpLiteCoreCount = globalData->nnConfig.fixedFeature.tpliteCoreCount;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_TPLITE_CORE_COUNT", &envctrl)) && envctrl)
    {
        globalData->options.tpLiteCoreCount = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableForce64BitsBiasNN = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_64BITS_BIAS_NN", &envctrl)) && envctrl)
    {
        globalData->options.enableForce64BitsBiasNN = atoi(envctrl);
    }

    envctrl = gcvNULL;
    globalData->options.enableAllocateContigousMemForKernel = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_ENABLE_CONTIGOUS_MEM_FOR_KERNEL", &envctrl)) && envctrl)
    {
        globalData->options.enableAllocateContigousMemForKernel = atoi(envctrl);
    }

    globalData->options.disableTPNNEvis = 0;
    if (gcmIS_SUCCESS(gcoOS_GetEnv(gcvNULL, "VIV_VX_DISABLE_TP_NN_EVIS", &envctrl)) && envctrl)
    {
        globalData->options.disableTPNNEvis = (atoi(envctrl) == 0) ? 0 : 1;
        if (globalData->options.disableTPNNEvis)
        {
            globalData->nnConfig.fixedFeature.nnCoreCount        =
            globalData->nnConfig.fixedFeature.nnCoreCountFloat16 =
            globalData->nnConfig.fixedFeature.nnCoreCountInt16   =
            globalData->nnConfig.fixedFeature.nnCoreCountInt8    =
            globalData->nnConfig.fixedFeature.nnMadPerCore       =
            globalData->nnConfig.fixedFeature.tpCoreCount        =
            globalData->nnConfig.fixedFeature.tpliteCoreCount    =
            globalData->nnConfig.fixedFeature.tpPwlLUTCount      =
            globalData->nnConfig.fixedFeature.vipCoreCount       = 0;
        }
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

#define VX_PRINT_BUFFER_COUNT      2048
void vxPRINT(vx_uint32 level, const char *msg, ...)
{
    va_list args;
    int size;
    char buffer[VX_PRINT_BUFFER_COUNT];
    if (vxDebugLevel == VX_DEBUG_LEVEL_NONE)
    {
        return;
    }
    va_start(args, msg);
    size = vsnprintf(buffer, VX_PRINT_BUFFER_COUNT - 1, msg, args);
    buffer[size] = '\0';
    va_end(args);
    fprintf(stdout, "%s", buffer);
    fflush(stdout);
}


VX_PRIVATE_API vx_status QuerySRAM(
    vx_global_data      globalData,
    gcePOOL             type,
    gctUINT32*          sRamPhysical,
    gctPOINTER*         sRamLogical,
    gctUINT32*          sRamGpuPhysical,
    gctUINT32*          sRamSize,
    gcsSURF_NODE_PTR*   sRamNode
    )
{
    vx_status vxStatus = VX_SUCCESS;
    gceSTATUS status = gcvSTATUS_OK;

    gctUINT32           size = 0;
    gctPOINTER          cpuLogical = gcvNULL;
    gctPHYS_ADDR_T      cpuPhysical = gcvINVALID_PHYSICAL_ADDRESS;
    gctPHYS_ADDR_T      gpuPhysical = gcvINVALID_PHYSICAL_ADDRESS;
    gctUINT32           gpuVirtual = gcvINVALID_ADDRESS;
    gctUINT32           gpuPhysicalName;

    gcmHEADER_ARG("globalData=%p, type=%p, sRamPhysical=%p, sRamLogical=%p, sRamSize=%p, sRamNode=%p",
        globalData, type, sRamPhysical, sRamLogical, sRamSize, sRamNode);

    gcmONERROR(gcoHAL_QuerySRAM(gcvNULL,
                                type,
                                &size,
                                gcvNULL,
                                gcvNULL,
                                gcvNULL,
                                gcvNULL
                                ));

    if ((type != gcvPOOL_EXTERNAL_SRAM) &&
        (type != gcvPOOL_INTERNAL_SRAM))
    {
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (type == gcvPOOL_EXTERNAL_SRAM)
    {
        if (globalData->options.axiSRAMSize != VX_INVALID_VALUE)
        {
            size = gcmMIN(size, globalData->options.axiSRAMSize);
        }

        if (size != 0)
        {
            globalData->nnConfig.customizedFeature.axiSRAMSize = size;

            gcmONERROR(gcoHAL_QuerySRAM(gcvNULL,
                                        type,
                                        &size,
                                        &gpuVirtual,
                                        &gpuPhysical,
                                        &gpuPhysicalName,
                                        &cpuPhysical
                                        ));

            gcmONERROR(gcoHAL_MapMemory(gcvNULL,
                                        gpuPhysicalName,
                                        size,
                                        &cpuLogical
                                        ));
        }
    }
    else if (type == gcvPOOL_INTERNAL_SRAM)
    {
        if (globalData->options.vipSRAMSize != VX_INVALID_VALUE)
        {
            size = gcmMIN(size, globalData->options.vipSRAMSize);
        }

        if (size == 0)
        {
            gcmONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        globalData->nnConfig.customizedFeature.vipSRAMSize = size;

        if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE3))
        {
            gcmONERROR(gcoHAL_QuerySRAM(gcvNULL,
                                        type,
                                        &size,
                                        &gpuVirtual,
                                        VX_NULL,
                                        VX_NULL,
                                        VX_NULL
                                        ));
        }
    }

    if (sRamPhysical)
    {
        *sRamPhysical = gpuVirtual;
    }

    if (sRamLogical)
    {
        *sRamLogical = cpuLogical;
    }

    if (sRamGpuPhysical)
    {
        *sRamGpuPhysical = (gctUINT32)gpuPhysical;
    }

    if (sRamSize)
    {
        *sRamSize = size;
    }

    if (sRamNode)
    {
        *sRamNode = VX_NULL;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);

    return VX_SUCCESS;

OnError:
    vxStatus = VX_FAILURE;

    gcmFOOTER_ARG("%d", vxStatus);

    return vxStatus;
}


VX_PRIVATE_API vx_status vxoGlobalData_InitSRAM(
    vx_global_data globalData
    )
{
    vx_status           status = VX_SUCCESS;
    gcsSURF_NODE_PTR    axiSRAMNode = VX_NULL;
    gctPOINTER          axiSRAMLogical = VX_NULL;
    gctUINT32           axiSRAMPhysical = gcvINVALID_ADDRESS;
    gctUINT32           axiSRAMSize = 0;
    gctUINT32           axiSRAMGpuPhysical = gcvINVALID_ADDRESS;

    gcsSURF_NODE_PTR    vipSRAMNode = VX_NULL;
    gctUINT32           vipSRAMPhysical = gcvINVALID_ADDRESS;
    gctUINT32           vipSRAMSize = 0;

    gcmHEADER_ARG("globalData=%p", globalData);

    vxmONERROR(QuerySRAM(globalData, gcvPOOL_EXTERNAL_SRAM, &axiSRAMPhysical, &axiSRAMLogical, &axiSRAMGpuPhysical, &axiSRAMSize, &axiSRAMNode));
    vxmONERROR(QuerySRAM(globalData, gcvPOOL_INTERNAL_SRAM, &vipSRAMPhysical, gcvNULL, gcvNULL, &vipSRAMSize, &vipSRAMNode));

    vxmASSERT(vipSRAMSize != 0);
    vxmASSERT(axiSRAMSize == 0 || (axiSRAMPhysical != gcvINVALID_ADDRESS && axiSRAMGpuPhysical != gcvINVALID_PHYSICAL_ADDRESS));

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE3))
    {
        vxmASSERT(vipSRAMPhysical != gcvINVALID_ADDRESS);
        gcmVERIFY_OK(gcoVX_SetRemapAddress(vipSRAMPhysical, 0, gcvVX_SRAM_REMAP));
    }

    if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE1))
    {
        if (axiSRAMSize != 0)
        {
            if (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_XYDP0))
            {
                /* v8 */
                gcmVERIFY_OK(gcoVX_SetRemapAddress(axiSRAMPhysical, axiSRAMPhysical + axiSRAMSize, gcvVX_OCB_REMAP));
            }
            else
            {
                gcmVERIFY_OK(gcoVX_SetRemapAddress(axiSRAMGpuPhysical, axiSRAMGpuPhysical + axiSRAMSize, gcvVX_OCB_REMAP));
            }
        }
        else
        {
            gcmVERIFY_OK(gcoVX_SetRemapAddress(0, 0, gcvVX_OCB_REMAP));
        }
    }


    if (axiSRAMSize != 0)
    {
        vx_uint32 i, deviceCount, gpuCountArray[MAX_GPU_CORE_COUNT], gpuTotalCount = 0, quot;
        vxmONERROR(gcoVX_QueryDeviceCount(&deviceCount));

        for (i = 0; i < deviceCount; i++)
        {
            vxmONERROR(gcoVX_QueryCoreCount(i, &gpuCountArray[i]));
            gpuTotalCount += gpuCountArray[i];
        }

        quot = axiSRAMSize / gpuTotalCount;
        vxmASSERT(quot % 64 == 0);

        globalData->axiSRAM[0].size     = quot * gpuCountArray[0];
        globalData->axiSRAM[0].logical  = axiSRAMLogical;
        globalData->axiSRAM[0].physical = axiSRAMPhysical;
        globalData->axiSRAM[0].used     = 0;
        globalData->axiSRAM[0].node     = axiSRAMNode;

        for (i = 1; i < deviceCount; i++)
        {
            globalData->axiSRAM[i].size     = quot * gpuCountArray[i];
            globalData->axiSRAM[i].logical  = (vx_uint8_ptr)axiSRAMLogical + globalData->axiSRAM[i-1].size;
            globalData->axiSRAM[i].physical = axiSRAMPhysical + globalData->axiSRAM[i-1].size;
            globalData->axiSRAM[i].used     = 0;
        }
    }

    if (vipSRAMSize != 0)
    {
        vx_uint32 vipSramImageStreamSize = globalData->nnConfig.fixedFeature.latencyHidingAtFullAxiBw * globalData->nnConfig.fixedFeature.axiBusWidth;
        globalData->vipSRAM.size         = (vipSRAMSize <= vipSramImageStreamSize) ? vipSRAMSize : (vipSRAMSize - vipSramImageStreamSize);
        globalData->vipSRAM.logical      = gcvNULL;
        globalData->vipSRAM.physBase     = vipSRAMPhysical != gcvINVALID_ADDRESS ? vipSRAMPhysical : 0;
        globalData->vipSRAM.physical     = vipSRAMPhysical != gcvINVALID_ADDRESS ?
                                           vipSRAMPhysical + vipSramImageStreamSize : vipSramImageStreamSize;
        globalData->vipSRAM.used         = 0;
        globalData->vipSRAM.node         = vipSRAMNode;
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_status vxoContext_CaptureInitState(
    vx_context context
    )
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0;
    gcmHEADER_ARG("context=%p", context);

    context->binaryGraphInitBuffer = (vx_ptr_ptr)vxAllocateAndZeroMemory(sizeof(vx_ptr) * context->deviceCount);
    vxmONERROR_NULLPTR(context->binaryGraphInitBuffer);

    context->binaryGraphInitSize = (vx_uint32_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * context->deviceCount);
    vxmONERROR_NULLPTR(context->binaryGraphInitSize);

    for (i = 0; i < context->deviceCount; i++)
    {
        context->binaryGraphInitBuffer[i] = (vx_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint8) * VX_MAX_INITIALIZE_COMMAND_SIZE);
        vxmONERROR_NULLPTR(context->binaryGraphInitBuffer[i]);
    }

    gcoVX_CaptureInitState(context->binaryGraphInitBuffer, VX_MAX_INITIALIZE_COMMAND_SIZE,
                            context->binaryGraphInitSize, context->deviceCount);

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_global_data vxoGlobalData_Create()
{
    vx_global_data globalData = VX_NULL;
    globalData = (vx_global_data)vxAllocate(sizeof(vx_global_data_s));
    if (globalData == VX_NULL) return VX_NULL;
    memset(globalData, 0, sizeof(vx_global_data_s));
    return globalData;
}

VX_PRIVATE_API void vxoGlobalData_Initialize(vx_global_data globalData)
{
    if (globalData->refGlobalDataCount != 0) return;

    gcoVX_GetNNConfig(&globalData->nnConfig);
    vxoGlobalData_InitOptions(globalData);
    gcoVX_ForceTpCoreCount(globalData);
    initUndefinedHardwareConfig(globalData);

    vxoGlobalData_InitSRAM(globalData);

}

VX_PRIVATE_API vx_uint32 vxoGlobalData_Release(vx_global_data globalData);
VX_PRIVATE_API vx_uint32 vxoGlobalData_AddRef(vx_global_data globalData)
{
    vx_uint32 refCount = globalData->refGlobalDataCount;

    if (refCount == 0)
    {
         vxoGlobalData_Initialize(globalData);
    }
    globalData->refGlobalDataCount++;
    refCount = globalData->refGlobalDataCount;
    return refCount;
}

VX_PRIVATE_API vx_context vxoContext_Create()
{
    vx_context context = VX_NULL;
    gcsPLS_PTR pls;
    gcmHEADER_ARG("context=%p", context);
    gcoHAL_GetPLS(&pls);
    vxmASSERT(pls->vxContextGlobalLock);

    vxAcquireMutex(pls->vxContextGlobalLock);
    gcoVX_ZeroMemorySize();

#if gcdUSE_SINGLE_CONTEXT
    if (vxSingletonContext == VX_NULL)
#endif
    {
        gctSTRING  oldEnv = gcvNULL;
        gctCHAR    newEnv[128] = {0};
        gctSTRING productName = gcvNULL;
        vxEnableAllTraceTargets(vx_false_e);
        context = (vx_context)vxAllocate(sizeof(vx_context_s));
        if (context == VX_NULL) goto ErrorExit;

        memset(context, 0, sizeof(vx_context_s));

        if (vxGlobalData == NULL)
        {
            vxGlobalData = vxoGlobalData_Create();
            if (vxGlobalData == NULL) goto ErrorExit;
            context->globalData = vxGlobalData;
        }
        else
        {
            context->globalData = vxGlobalData;
        }

        vxoReference_Initialize(&context->base, VX_NULL, VX_TYPE_CONTEXT, VX_NULL);

        vxoReference_Increment(&context->base, VX_REF_EXTERNAL);

        context->immediateBorderMode.mode = VX_BORDER_UNDEFINED;
        context->immediateBorderModePolicy  = VX_BORDER_POLICY_DEFAULT_TO_UNDEFINED;

        context->immediateTargetEnum = VX_TARGET_ANY;
        memset(context->immediateTargetString, 0, sizeof(context->immediateTargetString));

        context->NextDynamicUserKernelID    = 0;
        context->NextDynamicUserLibraryID   = 1;
        context->graphCount = 0;

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

        gcoVX_QueryHWChipInfo(&context->hwChipInfo);

        vxoGlobalData_AddRef(vxGlobalData);

        memcpy(&context->options, &vxGlobalData->options, sizeof(vx_drv_option));
        memcpy(&context->nnConfig, &vxGlobalData->nnConfig, sizeof(vx_nn_config));
        memcpy(&context->axiSRAM, &vxGlobalData->axiSRAM, sizeof(vxnne_sram_s) * MAX_GPU_CORE_COUNT);
        memcpy(&context->vipSRAM, &vxGlobalData->vipSRAM, sizeof(vxnne_sram_s));

        if (context->options.disableTPNNEvis)
        {
            context->evisNoInst.isVX2       = vx_false_e;
            context->evisNoInst.supportEVIS = vx_false_e;
            context->deviceCount = 1;
        }

        gcoHAL_GetProductName(gcvNULL, &productName, &context->pid);
        gcoOS_StrCopySafe(context->productName, 32 , productName);
        gcmOS_SAFE_FREE(gcvNULL, productName);
        vxInfo("#productname=%s, pid=0x%x\n", context->productName, context->pid);
        if (context->options.enableCNNPerf || context->options.enableNNArchPerfPrint)
        {
            gcmDUMP(gcvNULL, "#[info: productname=%s, pid=0x%x", context->productName, context->pid);
        }

        /* memory maps table lock */
        vxCreateMutex(&context->memoryMapsLock);

        gcoOS_GetEnv(gcvNULL, "VC_OPTION", &oldEnv);
        if (oldEnv)
        {
            gcoOS_StrCatSafe(newEnv, 128, oldEnv);
        }
        gcoOS_StrCatSafe(newEnv, 128, " -CLVIRCG:1 -OCLPASS_KERNEL_STRUCT_ARG_BY_VALUE:1");
        gcoOS_SetEnv(gcvNULL, "VC_OPTION", newEnv);

#if gcdUSE_SINGLE_CONTEXT
        vxSingletonContext = context;
#endif

        gcoOS_GetEnv(gcvNULL, "VX_EXTENSION_LIBS", &oldEnv);
        if (oldEnv != NULL)
        {
            if(vxLoadKernels(context, oldEnv) != VX_SUCCESS)
            {
                vxError("VX_EXTENSION_LIBS = %s, but load library failed!", oldEnv);
            }
        }
    }
#if gcdUSE_SINGLE_CONTEXT
    else
    {
        context = vxSingletonContext;

        vxoReference_Increment(&context->base, VX_REF_EXTERNAL);
    }
#endif

    vxReleaseMutex(pls->vxContextGlobalLock);

#if VIVANTE_PROFILER
    vxoProfiler_Initialize(context);
#endif

    if (context->options.enableSaveBinary)
    {
        vxoContext_CaptureInitState(context);
    }
    gcmFOOTER_ARG("%p", context);
    return (vx_context)context;

ErrorExit:
    vxReleaseMutex(pls->vxContextGlobalLock);

    if (context != VX_NULL)
    {
        if (context->globalData != VX_NULL)
        {
            vxoGlobalData_Release(context->globalData);
        }

        if ((&context->base)->lock != VX_NULL)
        {
            vxDestroyMutex((&context->base)->lock);
        }
        vxFree(context);
    }
    gcmFOOTER_NO();
    return VX_NULL;
}

VX_INTERNAL_API vx_bool vxoContext_IsValid(vx_context context)
{
    if (context == VX_NULL
        || context->base.signature != VX_REF_SIGNATURE_ALIVE
        || context->base.type != VX_TYPE_CONTEXT
        || context->base.context != VX_NULL)
    {
        vxError("%s[%d]: The context object, %p, is invalid!\n", __FUNCTION__, __LINE__, context);
        return vx_false_e;
    }

    return vx_true_e;
}

VX_PRIVATE_API void vxoContext_ForceReleaseAllObjects(vx_context context)
{
    vx_reference_item current;
    gcmHEADER_ARG("context=%p", context);

    vxmASSERT(context);

    current = context->refListHead;

    while (current != VX_NULL)
    {
        vx_reference ref = current->ref;

        if (ref != VX_NULL)
        {
            vx_uint32 externalCount = vxoReference_GetExternalCount(ref);

            if (externalCount > 0)
            {
                vxWarning("vxoContext_ForceReleaseAllObjects(): "
                            "The reference, %p, of type, %d, still has %u external count(s)\n",
                            ref, ref->type, externalCount);
            }

            if (ref->type == VX_TYPE_ERROR)
            {
                vxoReference_ReleaseEx(&ref, ref->type, VX_REF_INTERNAL, vx_true_e);
            }

            if (ref != VX_NULL)
            {
                while (vxoReference_GetExternalCount(ref) > 1)
                {
                    vxoReference_Decrement(ref, VX_REF_EXTERNAL);
                }

                if (vxoReference_GetExternalCount(ref) > 0)
                {
                    vxoReference_ReleaseEx(&ref, ref->type, VX_REF_EXTERNAL, vx_true_e);
                }
            }
        }

        current = current->next;
    }
    gcmFOOTER_NO();
}

gceSTATUS
gcfVX_UnloadCompiler(
    vx_global_data globalData
    )
{
    gceSTATUS   status = gcvSTATUS_OK;
    gcmHEADER_ARG("globalData=%p", globalData);
    {
        if (globalData->unloadCompiler != gcvNULL)
        {
            gcmONERROR((*globalData->unloadCompiler)());

            gcmVERIFY_OK(gcoOS_FreeLibrary(gcvNULL, globalData->libCLC));

            globalData->libCLC             = gcvNULL;
            globalData->compileKernel      = gcvNULL;
            globalData->loadCompiler       = gcvNULL;
            globalData->unloadCompiler     = gcvNULL;
        }
    }

OnError:
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_PRIVATE_API vx_uint32 vxoGlobalData_Release(vx_global_data globalData)
{
    vx_uint32 i, j, refCount;
    globalData->refGlobalDataCount--;
    refCount = globalData->refGlobalDataCount;
    if (refCount == 0)
    {
        if (globalData->options.flagTPFunc)
            vxFree(globalData->options.flagTPFunc);

        if (globalData->options.typeTPFunc)
            vxFree(globalData->options.typeTPFunc);

        if (globalData->axiSRAM[0].node)
        {
            gcoVX_FreeMemoryEx(globalData->axiSRAM[0].node, gcvSURF_VERTEX);
        }

        if (globalData->vipSRAM.node)
        {
            gcoVX_FreeMemoryEx(globalData->vipSRAM.node, gcvSURF_VERTEX);
        }

        for (i = 0; i < VXNNE_KERNEL_FIXED_COUNT; i++)
        {
            for (j = 0; j < globalData->kernels[i].kernelShaderCount*2; j++)
            {
                vxoShader_Free(globalData->kernels[i].kernelShader[j]);
            }

            if (globalData->kernels[i].kernelShader) gcoOS_Free(gcvNULL, globalData->kernels[i].kernelShader);
        }

        /* Destroy vir intrinsic library. */
#if (!VSC_LITE_BUILD)
        vscFreeVirIntrinsicLib();
#endif
        gcfVX_UnloadCompiler(globalData);

#ifdef USE_LIB_NN_ARCH_PERF
        if (globalData->apm)
            DestroyAPModel(globalData->apm);
#endif

        vxFree(vxGlobalData);
        vxGlobalData = NULL;
    }

    return refCount;
}

VX_PRIVATE_API vx_status vxoContext_Release(vx_context_ptr contextPtr)
{
    vx_context context;
    vx_uint32 i, refCount;
    vx_reference_item next, current;
    vx_char* flag;
    gcsPLS_PTR pls;
    gcmHEADER_ARG("contextPtr=%p", contextPtr);
    gcoOS_GetEnv(gcvNULL, "ENABLE_TRACE_MEMORY", &flag);
    if (flag)
    {
        vx_uint32 size = 0;
        gcoVX_GetMemorySize(&size);
        if (size > 1000 * 1000)
            vxInfo("Memory size = %.2f Mbyte\n", size/(1000.0f*1000.0f));
        else
            vxInfo("Memory size = %d byte\n", size);
    }

    if (contextPtr == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    context = *contextPtr;
    *contextPtr = VX_NULL;

    if (context == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    gcoHAL_GetPLS(&pls);
    if (pls->vxContextGlobalLock == VX_NULL)
    {
        gcmFOOTER_ARG("%d", VX_FAILURE);
        return VX_FAILURE;
    }
    vxAcquireMutex(pls->vxContextGlobalLock);


    gcfVX_Flush(gcvTRUE);

    if (!vxoContext_IsValid(context))
    {
        vxReleaseMutex(pls->vxContextGlobalLock);

        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
#if VIVANTE_PROFILER
    /* Destroy the profiler. */
    vxoProfiler_Destroy(context);
#endif

    if (context->options.enableSaveBinary)
    {
        if (context->binaryGraphInitBuffer != VX_NULL)
        {
            for (i = 0; i < context->deviceCount; i++)
            {
                if (context->binaryGraphInitBuffer[i] != VX_NULL)
                {
                    vxFree(context->binaryGraphInitBuffer[i]);
                    context->binaryGraphInitBuffer[i] = VX_NULL;
                }
            }
            vxFree(context->binaryGraphInitBuffer);
            context->binaryGraphInitBuffer = VX_NULL;
        }
        if (context->binaryGraphInitSize != VX_NULL)
        {
            vxFree(context->binaryGraphInitSize);
            context->binaryGraphInitSize = VX_NULL;
        }
    }

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

        current = context->refListHead;
        next = current != VX_NULL ? current->next : VX_NULL;
        while (current != VX_NULL)
        {
            if (current->ref != VX_NULL)
            {
                vxError("reference object, %p, failed to be released\n", current->ref);
            }
            vxFree(current);
            current = next;
            next = next != VX_NULL ? next->next : VX_NULL;
        }
        context->refListHead = context->refListTail = VX_NULL;
        context->refTotalCount = 0;
        context->refFreeCount = 0;

        vxmASSERT(context->base.lock);
        vxDestroyMutex(context->base.lock);

        vxDestroyMutex(context->memoryMapsLock);

        refCount = vxoGlobalData_Release(context->globalData);

        vxFree(context);

        /* Free the global resources */
        vxReleaseMutex(pls->vxContextGlobalLock);
#if gcdUSE_SINGLE_CONTEXT
        vxSingletonContext = VX_NULL;
#endif

        if (refCount == 0)
        {
        }
        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }
    else
    {
        vxWarning("vxoContext_Release(): the context, %p, still has %u reference count(s) in total",
                    &context->base,
                    vxoReference_GetTotalCount(&context->base));

        vxReleaseMutex(pls->vxContextGlobalLock);

        gcmFOOTER_ARG("%d", VX_SUCCESS);
        return VX_SUCCESS;
    }
}

VX_INTERNAL_API vx_error vxoContext_GetErrorObject(vx_context context, vx_status status)
{
    vx_reference_item current;
    gcmHEADER_ARG("context=%p, status=0x%x", context, status);
    vxAcquireMutex(context->base.lock);
    current = context->refListHead;

    while (current != VX_NULL)
    {
        if (current->ref != VX_NULL && current->ref->type == VX_TYPE_ERROR)
        {
            vx_error error = (vx_error_s *)current->ref;

            if (error->status == status)
            {
                vxReleaseMutex(context->base.lock);
                gcmFOOTER_ARG("%p", error);
                return error;
            }
        }

        current = current->next;
    }
    vxReleaseMutex(context->base.lock);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("context=%p, ref=%p", context, ref);
    vxmASSERT(context);
    vxmASSERT(ref);

    vxAcquireMutex(context->base.lock);
    if (context->refFreeCount == 0)
    {
        vx_reference_item ritem = (vx_reference_item)vxAllocate(sizeof(vx_reference_item_s));
        if (ritem == VX_NULL)
        {
            vxError("Fail to allocate memory");
            vxReleaseMutex(context->base.lock);
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }

        ritem->ref = ref;
        ritem->next = ritem->prev = VX_NULL;

        if (context->refListHead == VX_NULL && context->refListTail == VX_NULL)
        {
            context->refListHead = context->refListTail = ritem;
        }
        else if (context->refListTail != VX_NULL)
        {
            ritem->prev = context->refListTail;
            context->refListTail->next = ritem;
            context->refListTail = ritem;
        }
        else
        {
            if (ritem)
            {
                vxFree(ritem);
            }

            vxError("Context ref list error");
            vxReleaseMutex(context->base.lock);
            gcmFOOTER_ARG("%d", vx_false_e);
            return vx_false_e;
        }

        context->refTotalCount++;
        vxReleaseMutex(context->base.lock);
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }
    else
    {
        vx_reference_item current = context->refListTail;

        while (current != VX_NULL)
        {
            if (current->ref == VX_NULL)
            {
                current->ref = ref;
                context->refFreeCount--;
                vxReleaseMutex(context->base.lock);
                gcmFOOTER_ARG("%d", vx_true_e);
                return vx_true_e;
            }

            current = current->prev;
        }
    }

    vxReleaseMutex(context->base.lock);
    gcmFOOTER_ARG("%d", vx_true_e);
    return vx_false_e;
}

VX_INTERNAL_API vx_bool vxoContext_RemoveObject(vx_context context, vx_reference ref, vx_bool order)
{
    vx_reference_item current;

    gcmHEADER_ARG("context=%p, ref=%p", context, ref);

    vxmASSERT(context);
    vxmASSERT(ref);
    vxAcquireMutex(context->base.lock);
    if (order)
    {
        current = context->refListHead;

        while (current != VX_NULL)
        {
            if (current->ref == ref)
            {
                current->ref = VX_NULL;
                context->refFreeCount++;
                vxReleaseMutex(context->base.lock);
                gcmFOOTER_ARG("%d", vx_true_e);
                return vx_true_e;
            }

            current = current->next;
        }
    }
    else
    {
        current = context->refListTail;

        while (current != VX_NULL)
        {
            if (current->ref == ref)
            {
                current->ref = VX_NULL;
                context->refFreeCount++;
                vxReleaseMutex(context->base.lock);
                gcmFOOTER_ARG("%d", vx_true_e);
                return vx_true_e;
            }

            current = current->prev;
        }
    }

    vxReleaseMutex(context->base.lock);
    vxError("Can'targetIndex find the reference, %p, in the context, %p.", ref, context);
    gcmFOOTER_ARG("%d", vx_false_e);
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
    vx_uint32            i, offset = 0;
    vx_char              moduleName[VX_MAX_PATH];
    vx_symbol_handle     sym;
    vx_publish_kernels_f publish = NULL;
    vx_status            status;

    gcmHEADER_ARG("context=%p, module=%s", context, module);

    vxmASSERT(module);

    if (isFullModuleName(module))
    {
        gcoOS_PrintStrSafe(moduleName, VX_MAX_PATH, &offset, "%s", module);
    }
    else
    {
        gcoOS_PrintStrSafe(moduleName, VX_MAX_PATH, &offset, VX_MODULE_NAME("%s"), module);
    }

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    for (i = 0; i < context->moduleCount; i++)
    {
        if (context->moduleTable[i].handle != VX_NULL
            && vxIsSameString(module, context->moduleTable[i].name, VX_MAX_PATH))
        {
            gcmFOOTER_ARG("%d", VX_SUCCESS);
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
                gcmFOOTER_ARG("%d", VX_FAILURE);
                return VX_FAILURE;
            }

            sym = vxGetSymbol(context->moduleTable[i].handle, "vxPublishKernels");

            publish = (vx_publish_kernels_f)sym;
            if (publish == NULL)
            {
                vxError("Failed to load symbol vxPublishKernels\n");
                vxUnloadModule(context->moduleTable[i].handle);
                context->moduleTable[i].handle = NULL;
                gcmFOOTER_ARG("%d", VX_ERROR_NO_RESOURCES);
                return VX_ERROR_NO_RESOURCES;
            }

            status = publish((vx_context)context);
            if (status != VX_SUCCESS)
            {
                vxError("Failed to publish kernels in module\n");
                vxUnloadModule(context->moduleTable[i].handle);
                context->moduleTable[i].handle = NULL;
                gcmFOOTER_ARG("%d", VX_ERROR_NO_RESOURCES);
                return VX_ERROR_NO_RESOURCES;
            }

            vxStrCopySafe(context->moduleTable[i].name, VX_MAX_PATH, module);
            context->moduleCount++;
            gcmFOOTER_ARG("%d", VX_SUCCESS);
            return VX_SUCCESS;
        }
    }
    gcmFOOTER_ARG("%d", VX_ERROR_NO_RESOURCES);
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
    gcmHEADER_ARG("context=%p, size=0x%lx, usage=0x%x, ptr=%p, ref=%p, indexPtr=%p, extraDataPtr=%p",
        context, size, usage, ptr, ref, indexPtr, extraDataPtr);
    vxmASSERT(context);
    vxmASSERT(indexPtr);

    for (index = 0; index < vxmLENGTH_OF(context->accessorTable); index++)
    {
        if (context->accessorTable[index].used) continue;

        if (size > 0 && ptr == VX_NULL)
        {
            ptr = vxAllocate(size);

            if (ptr == VX_NULL)
            {
                gcmFOOTER_ARG("%d", vx_false_e);
                return vx_false_e;
            }
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
        gcmFOOTER_ARG("%d", vx_true_e);
        return vx_true_e;
    }

    *indexPtr = 0;
    gcmFOOTER_ARG("%d", vx_false_e);
    return vx_false_e;
}

VX_INTERNAL_API void vxoContext_RemoveAccessor(vx_context context, vx_uint32 index)
{
    gcmHEADER_ARG("context=%p, index=0x%x", context, index);
    vxmASSERT(context);
    vxmASSERT(index < vxmLENGTH_OF(context->accessorTable));

    if (context->accessorTable[index].allocated) vxFree(context->accessorTable[index].ptr);

    if (context->accessorTable[index].extraDataPtr != NULL) vxFree(context->accessorTable[index].extraDataPtr);

    vxZeroMemory(&context->accessorTable[index], sizeof(vx_accessor_s));

    context->accessorTable[index].used = vx_false_e;
    gcmFOOTER_NO();
}

VX_INTERNAL_API vx_bool vxoContext_SearchAccessor(vx_context context, vx_ptr ptr, OUT vx_uint32_ptr indexPtr)
{
    vx_uint32 index;

    gcmHEADER_ARG("context=%p, ptr=%p, indexPtr=%p", context, ptr, indexPtr);

    vxmASSERT(context);
    vxmASSERT(indexPtr);

    for (index = 0; index < vxmLENGTH_OF(context->accessorTable); index++)
    {
        if (!context->accessorTable[index].used) continue;

        if (context->accessorTable[index].ptr == ptr)
        {
            *indexPtr = index;
            gcmFOOTER_ARG("%d", vx_true_e);
            return vx_true_e;
        }
    }

    *indexPtr = 0;
    gcmFOOTER_ARG("%d", vx_false_e);
    return vx_false_e;
}

VX_API_ENTRY vx_context VX_API_CALL vxCreateContext()
{
    gcmDUMP_API("$VX vxCreateContext: ");

    return vxoContext_Create();
}

VX_API_ENTRY vx_status VX_API_CALL vxReleaseContext(vx_context *context)
{
    gcmDUMP_API("$VX vxReleaseContext: context=%p", context);

    return vxoContext_Release(context);
}

VX_API_ENTRY vx_context VX_API_CALL vxGetContext(vx_reference reference)
{
    gcmDUMP_API("$VX vxGetContext: reference=%p", reference);

    return vxoContext_GetFromReference(reference);
}

VX_API_ENTRY vx_status VX_API_CALL vxSetContextAttribute(vx_context context, vx_enum attribute, const void *ptr, vx_size size)
{
    vx_border_t * borderMode;
    vx_enum * immediateBorderModePolicy;
    gcmHEADER_ARG("context=%p, attribute=0x%x ptr=%p size=0x%lx", context, attribute, ptr, size);
    gcmDUMP_API("$VX vxSetContextAttribute: context=%p, attribute=0x%x ptr=%p size=0x%lx", context, attribute, ptr, size);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
    switch (attribute)
    {
        case VX_CONTEXT_IMMEDIATE_BORDER:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_border_t, 0x3);

            borderMode = (vx_border_t *)ptr;

            if (!vxIsValidBorderMode(borderMode->mode))
            {
                vxError("%s[%d]: BorderMode is invalid!\n", __FUNCTION__, __LINE__);
                vxAddLogEntry(&context->base, VX_ERROR_INVALID_VALUE, "%s[%d]: BorderMode is invalid!\n", __FUNCTION__, __LINE__);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_VALUE);
                return VX_ERROR_INVALID_VALUE;
            }

            context->immediateBorderMode = *borderMode;
            break;
        case VX_CONTEXT_IMMEDIATE_BORDER_POLICY:

            vxmVALIDATE_PARAMETERS(ptr, size, vx_enum, 0x3);

            immediateBorderModePolicy = (vx_enum *)ptr;

            if (!vxIsValidBorderModePolicy(*immediateBorderModePolicy))
            {
                vxError("%s[%d]: BorderModePolicy is invalid!\n", __FUNCTION__, __LINE__);
                vxAddLogEntry(&context->base, VX_ERROR_INVALID_VALUE, "%s[%d]: BorderModePolicy is invalid!\n", __FUNCTION__, __LINE__);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_VALUE);
                return VX_ERROR_INVALID_VALUE;
            }

            context->immediateBorderModePolicy = *immediateBorderModePolicy;
            break;

        default:
            vxError("%s[%d]: The attribute parameter, %d, is not supported!\n", __FUNCTION__, __LINE__, attribute);
            vxAddLogEntry(&context->base, VX_ERROR_NOT_SUPPORTED, "%s[%d]: \
                The attribute parameter, %d, is not supported!\n", __FUNCTION__, __LINE__, attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
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
    gcmHEADER_ARG("context=%p, uniqueKernelTable=%p", context, uniqueKernelTable);

    vxmASSERT(context);
    vxmASSERT(uniqueKernelTable);

    for (targetIndex = 0; targetIndex < context->targetCount; targetIndex++)
    {
        for (kernelndex = 0; kernelndex < VX_MAX_KERNEL_COUNT; kernelndex++)
        {
            vx_bool alreadyExisted = vx_false_e;
            vx_kernel kernel = &context->targetTable[targetIndex].kernelTable[kernelndex];

            if(kernel->enabled == vx_false_e)
                continue;

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
    gcmFOOTER_NO();
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryContext(vx_context context, vx_enum attribute, void *ptr, vx_size size)
{
    gcmHEADER_ARG("context=%p, attribute=0x%x, ptr=%p, size=0x%lx", context, attribute, ptr, size);
    gcmDUMP_API("$VX vxQueryContext: context=%p, attribute=0x%x, ptr=%p, size=0x%lx", context, attribute, ptr, size);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
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

            *(vx_uint32 *)ptr = context->refTotalCount - context->refFreeCount;
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
                vxError("%s[%d]: size > VX_MAX_IMPLEMENTATION_NAME || ptr == VX_NULL !\n", __FUNCTION__, __LINE__);
                vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: size > VX_MAX_IMPLEMENTATION_NAME || ptr == VX_NULL !\n", __FUNCTION__, __LINE__);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
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
                vxError("%s[%d]: size >= sizeof(extensions) || ptr == VX_NULL !\n", __FUNCTION__, __LINE__);
                vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: size >= sizeof(extensions) || ptr == VX_NULL !\n", __FUNCTION__, __LINE__);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
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
                vxError("%s[%d]: size != (context->uniqueKernelCount * sizeof(vx_kernel_info_t)) \
                    || ptr == VX_NULL !\n", __FUNCTION__, __LINE__);
                vxAddLogEntry(&context->base, VX_ERROR_INVALID_PARAMETERS, "%s[%d]: size != (context->uniqueKernelCount * sizeof(vx_kernel_info_t)) \
                    || ptr == VX_NULL !\n", __FUNCTION__, __LINE__);
                gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
                return VX_ERROR_INVALID_PARAMETERS;
            }

            vxoContext_GetUniqueKernelTable(context, OUT (vx_kernel_info_t *)ptr);
            break;

        case VX_CONTEXT_NONLINEAR_MAX_DIMENSION:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            *(vx_size *)ptr = VX_INT_MAX_NONLINEAR_DIM;

            break;
        case VX_CONTEXT_DEVICE_COUNT_VIV :
             vxmVALIDATE_PARAMETERS(ptr, size, vx_uint32, 0x3);

             *(vx_uint32 *)ptr = context->deviceCount;
             break;

         case VX_CONTEXT_MAX_TENSOR_DIMS:
            vxmVALIDATE_PARAMETERS(ptr, size, vx_size, 0x3);

            if (VX_CONTEXT_TENSOR_MAX_DIMENSION >= 4)
                *(vx_size *)ptr = 4;
            else
                *(vx_size *)ptr = VX_CONTEXT_TENSOR_MAX_DIMENSION;

            break;

        default:
            vxError("%s[%d]: The attribute parameter, %d, is not supported!\n", __FUNCTION__, __LINE__, attribute);
            vxAddLogEntry(&context->base, VX_ERROR_NOT_SUPPORTED, "%s[%d]: The attribute parameter, %d, is not supported!\n", __FUNCTION__, __LINE__, attribute);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxHint(vx_reference reference, vx_enum hint, const void* data, vx_size data_size)
{
    gcmHEADER_ARG("reference=%p, hint=0x%x, data=%p, data_size=0x%lx", reference, hint, data, data_size);
    gcmDUMP_API("$VX vxHint: reference=%p, hint=0x%x, data=%p, data_size=0x%lx", reference, hint, data, data_size);

    if (!vxoContext_IsValid((vx_context)reference) && !vxoReference_IsValidAndNoncontext(reference))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

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
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_status VX_API_CALL vxDirective(vx_reference reference, vx_enum directive)
{
    vx_context context;
    gcmHEADER_ARG("reference=%p, directive=0x%x", reference, directive);
    gcmDUMP_API("$VX vxDirective: reference=%p, directive=0x%x", reference, directive);

    /*if (!vxoContext_IsValid(reference->context)) return VX_ERROR_INVALID_REFERENCE;*/

    if (!vxoReference_IsValid(reference))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }
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
            {
                gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
                return VX_ERROR_NOT_SUPPORTED;
            }
            break;
        case VX_DIRECTIVE_ENABLE_PERFORMANCE:
            if (reference->type == VX_TYPE_CONTEXT)
                context->options.enableCNNPerf = 1;
            else
            {
                gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
                return VX_ERROR_NOT_SUPPORTED;
            }
            break;

        default:
            vxError("The directive parameter, %d, is not supported", directive);
            gcmFOOTER_ARG("%d", VX_ERROR_NOT_SUPPORTED);
            return VX_ERROR_NOT_SUPPORTED;
    }
    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

VX_API_ENTRY vx_enum VX_API_CALL vxRegisterUserStruct(vx_context context, vx_size size)
{
    vx_uint32 i;
    gcmHEADER_ARG("context=%p, size=0x%lx", context, size);
    gcmDUMP_API("$VX vxRegisterUserStruct: context=%p, size=0x%lx", context, size);

    if (!vxoContext_IsValid(context) || size == 0) {
        gcmFOOTER_ARG("%d", VX_TYPE_INVALID);
        return VX_TYPE_INVALID;
    }
    for (i = 0; i < VX_MAX_USER_STRUCT_COUNT; i++)
    {
        if (context->userStructTable[i].type == VX_TYPE_INVALID)
        {
            context->userStructTable[i].size = size;
            gcmFOOTER_NO();
            return context->userStructTable[i].type = VX_TYPE_USER_STRUCT_START + i;
        }
    }
    gcmFOOTER_ARG("%d", VX_TYPE_INVALID);
    return VX_TYPE_INVALID;
}

static vx_target_s* vxoContext_FindTargetByString(vx_context context, const char* targetString)
{
    vx_uint32 index = 0;
    gcmHEADER_ARG("context=%p, targetString=%s", context, targetString);

    for (index = 0; index < context->targetCount; index++)
    {
        vx_uint32 targetIndex = context->targetPriorityTable[index];
        vx_target_s *target = &context->targetTable[targetIndex];
        if (vxoTarget_MatchTargetNameWithString(target->name, targetString) == vx_true_e)
        {
            gcmFOOTER_ARG("%p", target);
            return target;
        }
    }
    gcmFOOTER_NO();
    /* indicate 'not found' state */
    return (vx_target_s*)NULL;
}

VX_API_ENTRY vx_status VX_API_CALL vxSetImmediateModeTarget(vx_context context, vx_enum target_enum, const char* target_string)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    vx_target_s* target = NULL;
    gcmHEADER_ARG("context=%p, target_enum=0x%x, target_string=%s", context, target_enum, target_string);
    gcmDUMP_API("$VX vxSetImmediateModeTarget: context=%p, target_enum=0x%x, target_string=%s", context, target_enum, target_string);

    if (!vxoContext_IsValid(context)) {
        gcmFOOTER_ARG("%d", VX_TYPE_INVALID);
        return VX_TYPE_INVALID;
    }

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
    gcmFOOTER_ARG("%d", status);
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
    vx_bool found = vx_false_e;

    /* lock the table for modification */
    if (vx_true_e == vxAcquireMutex(context->memoryMapsLock))
    {
        for (id = 0u; id < vxmLENGTH_OF(context->memoryMaps); id++)
        {
            if (context->memoryMaps[id].used == vx_false_e)
            {
                vx_memory_map_extra_s* extra;
                vx_uint8_ptr buf = 0;
                vxInfo("Found free memory map slot[%u]\n", id);

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

                found = vx_true_e;

                break;
            }
        }

        /* we're done, unlock the table */
        vxReleaseMutex(context->memoryMapsLock);
    }
    else
        found = vx_false_e;

    return found;
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
    gcmHEADER_ARG("context=%p, ref=%p, size=0x%lx, usage=0x%x, mem_type=0x%x, flags=0x%x, extra_data=%p, ptr=%p, map_id=%p",
        context, ref, size, usage, mem_type, flags, extra_data, ptr, map_id);

    /* lock the table for modification */
    if (vx_true_e == vxAcquireMutex(context->memoryMapsLock))
    {
        for (id = 0u; id < vxmLENGTH_OF(context->memoryMaps); id++)
        {
            if (context->memoryMaps[id].used == vx_false_e)
            {
                vx_memory_map_extra_s* extra;
                vx_uint8_ptr buf = 0;
                vxInfo("Found free memory map slot[%u]\n", id);

                /* allocate mapped buffer */
                //buf = (vx_uint8*)malloc(size);
                if (size > 0)
                {
                    if (VX_TYPE_WEIGHTS_BIASES_PARAMETER == ref->type)
                    {
                        vx_weights_biases_parameter wb = (vx_weights_biases_parameter)ref;
                        buf = (vx_uint8_ptr)wb->memory.logicals[0] - WB_MEM_HEAD_OFFSET(wb);
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
                        buf = (vx_uint8_ptr)&image->memory.logicals[extra->image_data.plane_index][offset];
                    }
                    else if (VX_TYPE_DISTRIBUTION == ref->type)
                    {
                        buf = (vx_uint8_ptr)((vx_distribution)ref)->memory.logicals[0];
                    }
                    else {
                        buf = (vx_uint8_ptr)vxAllocate(size);
                        context->memoryCount++;
                    }

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
        worked &= vxReleaseMutex(context->memoryMapsLock);
    }
    else
        worked = vx_false_e;
    gcmFOOTER_ARG("%d", worked);
    return worked;
} /* vxoContext_MemoryMap() */

#endif

VX_INTERNAL_API vx_bool vxoContext_FindMemoryMap(
    vx_context   context,
    vx_reference ref,
    vx_map_id    map_id)
{
    vx_bool found = vx_false_e;
    vx_uint32 id = (vx_uint32)map_id;
    gcmHEADER_ARG("context=%p, ref=%p, map_id=0x%x", context, ref, map_id);
    /* check index range */
    if (id < vxmLENGTH_OF(context->memoryMaps))
    {
        /* lock the table for exclusive access */
        if (vx_true_e == vxAcquireMutex(context->memoryMapsLock))
        {
            if ((context->memoryMaps[id].used == vx_true_e) && (context->memoryMaps[id].ref == ref))
            {
                found = vx_true_e;
            }

            /* unlock teh table */
            vxReleaseMutex(context->memoryMapsLock);
        }
    }
    gcmFOOTER_ARG("%d", found);
    return found;
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
            vxInfo("Removed memory mapping[%u]\n", map_id);

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
    gcmHEADER_ARG("context=%p, m_id=0x%x", context, m_id);

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
                context->memoryCount --;
            }
            memset(&context->memoryMaps[map_id], 0, sizeof(vx_memory_map_s));
            vxInfo("Removed memory mapping[%u]\n", map_id);
        }

        context->memoryMaps[map_id].used = vx_false_e;

        /* we're done, unlock the table */
        vxReleaseMutex(context->memoryMapsLock);
    }
    else
        vxError("vxAcquireMutex() failed!\n");
    gcmFOOTER_NO();
    return;
} /* vxoContext_MemoryUnmap() */
#endif

VX_INTERNAL_API vx_bool vxoContext_IsFeatureAvailable(vx_context context, vx_nn_feature_e feature)
{
    return vxoGlobalData_IsFeatureAvailable(context->globalData, feature);
}

VX_INTERNAL_API vx_bool vxoGlobalData_IsFeatureAvailable(vx_global_data globalData, vx_nn_feature_e feature)
{
    switch (feature)
    {
    case VX_NN_FEATURE_SHADER:
        return (globalData->nnConfig.fixedFeature.shaderCoreCount && globalData->options.enableShader) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_MULTI_TP:
        return ((globalData->nnConfig.fixedFeature.tpCoreCount > 1) && globalData->options.enableMultiTP) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_RESHUFFLE:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                globalData->options.flagTPFunc[TP_RESHUFFLE]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_SINGLE_FC:
        return ((globalData->nnConfig.fixedFeature.tpCoreCount + globalData->nnConfig.fixedFeature.tpliteCoreCount) &&
                globalData->options.enableTP && globalData->options.flagTPFunc[TP_SINGLE_FC]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_MAX_POOLING:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                globalData->options.flagTPFunc[TP_MAX_POOLING]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_ACTIVATION:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                globalData->options.flagTPFunc[TP_ACTIVATION]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_LRN:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_LRN) && globalData->options.flagTPFunc[TP_LRN]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_TRANSPOSE:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                globalData->options.flagTPFunc[TP_TRANSPOSE]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_ROI_POOLING:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_ROI_POOLING) && globalData->options.flagTPFunc[TP_ROI_POOLING]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_UPSAMPLE:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                globalData->options.flagTPFunc[TP_UPSAMPLE]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_REORG:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                globalData->options.flagTPFunc[TP_REORG]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_ADD:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                globalData->options.flagTPFunc[TP_ADD]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_REVERSE:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                globalData->options.flagTPFunc[TP_REVERSE]) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_REORDER:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_REORDER) && globalData->options.enableTPReorder) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_RTNE:
        return (globalData->nnConfig.fixedFeature.tpCoreCount && globalData->options.enableTP &&
                gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_RTNE) && globalData->options.enableTPRTNE) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_BRICK_MODE:
        return globalData->options.enableBrickMode ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_INTERLEVE8:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_INTERLEAVE8) && globalData->options.enableInterleave8) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_SRAM:
        return ((globalData->nnConfig.customizedFeature.vipSRAMSize > 0) && globalData->options.enableSRAM) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_BORDER_MODE:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_BORDER_MODE) && globalData->options.enableBorderMode) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_ZDP3:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP3)
             && gcoHAL_GetOption(gcvNULL, gcvOPTION_OVX_ENABLE_NN_ZDP3)) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_ZDP6:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_ZDP6)
             && gcoHAL_GetOption(gcvNULL, gcvOPTION_OVX_ENABLE_NN_ZDP6)) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_XYDP9:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_XYDP9)
            && globalData->options.enableNNXYDP9 ) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_SWTILING_PHASE1:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE1)
             && (globalData->options.enableSwtilingPhase1 != VX_SWTILING_OPTION_OFF)
             && (globalData->nnConfig.customizedFeature.axiSRAMSize > 0 || gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE3))
             && gcoHAL_GetOption(gcvNULL, gcvOPTION_OVX_ENABLE_NN_STRIDE)) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_SWTILING_PHASE2:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE2)
             && globalData->options.enableSwtilingPhase2 ) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_SWTILING_PHASE3:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_SWTILING_PHASE3)
             && globalData->options.enableSwtilingPhase3 ) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TF_QUANT:
        return gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TF_QUANTIZATION) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_FIRST_PIXEL_POOLING:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_FIRST_PIXEL_POOLING) && globalData->options.enableNNFirstPixelPooling) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_NN_STRIDE_SUPPORT:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_STRIDE_SUPPORT)
            && gcoHAL_GetOption(gcvNULL, gcvOPTION_OVX_ENABLE_NN_STRIDE)) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_XYDP6:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_XYDP6) && globalData->options.enableNNXYDP6) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_COEF_COMPRESSION_ENHANCEMENT) && globalData->options.enableHuffmanEnhancement) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_TP_COEF_COMPRESSION_ENHANCEMENT) && globalData->options.enableTPHuffman) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_NN_DEPTHWISE_SUPPORT:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_DEPTHWISE_SUPPORT) && globalData->options.enableNNDepthWiseSupport) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_XYDP0:
        return gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_XYDP0) ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_SCALER:
        return gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_VIP_SCALER) && globalData->options.enableYUV2RGBScaler ? vx_true_e : vx_false_e;

    case VX_NN_FEATURE_VIP_DEC400:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_VIP_DEC400) && globalData->options.enableVIPDEC400) ? vx_true_e : vx_false_e;

    case VX_NN_TP_PARALLEL:
        return (!globalData->options.enableSwtilingPhase1 && globalData->options.enableNNTPParallel) ? vx_true_e : vx_false_e;


        /*if feature ready, add it*/
    case VX_TP_FEATURE_FP32_BIAS:
        return vx_false_e;

    case VX_NN_FEATURE_NN_TRANSPOSE:
        return (gcoHAL_IsFeatureAvailable(gcvNULL, gcvFEATURE_NN_TRANSPOSE)
             && globalData->options.enableNNTranspose ) ? vx_true_e : vx_false_e;

    default:
        return vx_false_e;
    }
}

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelId(vx_context context, vx_enum * pKernelEnumId)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    gcmHEADER_ARG("context=%p, pKernelEnumId=%p", context, pKernelEnumId);
    gcmDUMP_API("$VX vxAllocateUserKernelId: context=%p, pKernelEnumId=%p", context, pKernelEnumId);

    if ((vxoContext_IsValid(context) == vx_true_e) && pKernelEnumId)
    {
        status = VX_ERROR_NO_RESOURCES;
        if(context->NextDynamicUserKernelID <= VX_KERNEL_MASK)
        {
            *pKernelEnumId = VX_KERNEL_BASE(VX_ID_USER,0) + context->NextDynamicUserKernelID++;
            gcmFOOTER_ARG("%d", VX_SUCCESS);
            status = VX_SUCCESS;
        }
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxAllocateUserKernelLibraryId(vx_context context, vx_enum * pLibraryId)
{
    vx_status status = VX_ERROR_INVALID_REFERENCE;
    gcmHEADER_ARG("context=%p, pLibraryId=%p", context, pLibraryId);
    gcmDUMP_API("$VX vxAllocateUserKernelLibraryId: context=%p, pLibraryId=%p", context, pLibraryId);

    if ((vxoContext_IsValid(context) == vx_true_e) && pLibraryId)
    {
        status = VX_ERROR_NO_RESOURCES;
        if(context->NextDynamicUserLibraryID <= VX_LIBRARY(VX_LIBRARY_MASK))
        {
            *pLibraryId = context->NextDynamicUserLibraryID++;
            gcmFOOTER_ARG("%d", VX_SUCCESS);
            status = VX_SUCCESS;
        }
    }
    gcmFOOTER_ARG("%d", status);
    return status;
}

VX_INTERNAL_API vx_int32 vxoContext_GetUserStructIndex(vx_context context, vx_enum dataType)
{
    vx_int32 i;

    for(i=0; i<VX_MAX_USER_STRUCT_COUNT; i++)
    {
        if(dataType == context->userStructTable[i].type)
        {
            break;
        }
    }

    if(i == VX_MAX_USER_STRUCT_COUNT) i = -1;

    return i;
}

VX_API_ENTRY vx_status VX_API_CALL vxQueryHardwareCaps(
    vx_context                          context,
    const vx_hardware_caps_params_t   * hardware_caps_params,
    vx_size                             size_of_hardware_caps_param
    )
{
    gcmHEADER_ARG("context=%p, hardware_caps_params=%p, size_of_hardware_caps_param=0x%lx",
        context, hardware_caps_params, size_of_hardware_caps_param);
    gcmDUMP_API("$VX vxQueryHardwareCaps: context=%p, hardware_caps_params=0x%x, size_of_hardware_caps_param=0x%lx", context, hardware_caps_params, size_of_hardware_caps_param);

    if (!vxoContext_IsValid(context))
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_REFERENCE);
        return VX_ERROR_INVALID_REFERENCE;
    }

    if (size_of_hardware_caps_param == sizeof(vx_hardware_caps_params_t))
    {
        vx_hardware_caps_params_t* hwCapsParams = (vx_hardware_caps_params_t*)hardware_caps_params;

        hwCapsParams->customerID = context->hwChipInfo.customerID;
        hwCapsParams->ecoID = context->hwChipInfo.ecoID;
        hwCapsParams->evis1 = context->evisNoInst.supportEVIS && (!context->evisNoInst.isVX2);
        hwCapsParams->evis2 = context->evisNoInst.isVX2;
    }
    else
    {
        gcmFOOTER_ARG("%d", VX_ERROR_INVALID_PARAMETERS);
        return VX_ERROR_INVALID_PARAMETERS;
    }

    gcmFOOTER_ARG("%d", VX_SUCCESS);
    return VX_SUCCESS;
}

