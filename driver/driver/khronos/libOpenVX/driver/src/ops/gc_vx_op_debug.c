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


#include <ops/gc_vx_op_debug.h>

vx_status vxnneOpDebug_DumpOperation(vxnne_execution_layer layer, vx_int32 opIndex)
{
    vx_status status = VX_SUCCESS;
    vx_int32 operationId = layer->opIndices[opIndex].operationID;
    vxnne_operation operation = layer->operations[operationId];
    /*
    layer {
    name: "relu1"
    type: "ReLU"
    bottom: "conv1"
    top: "conv1"
    }
    */
    if (operation->currBatchIndex == 0)
    {
        vx_char opName[][32] =
        {
            "NONE",
            "CONVOLUTION",
            "RESHUFFLE",
            "InnerProduct",
            "ACTIVATION",
            "POOLING",
            "RESIZE",
            "TENSOR_ADD",
            "TENSOR_SUB",
            "TENSOR_MUL",
            "TENSOR_DIV",
            "TENSOR_TRANS",
            "SOFTMAX",
            "NORMALIZATION",
            "BATCHNORM",
            "INPUT2WEIGHT",
            "RPN_SOFTMAX",
            "RPN_REGRESSION",
            "RPN_SORT",
            "RPN_NMS",
            "RPN_SORT_NMS",
            "RPN_RETRIEVE",
            "RPN",
            "ROIPOOL",
            "ROIPOOLRELU",
            "CONCAT2",
            "CONCATINDEFINITE",
            "REORG",
            "VERTMAXPOOL",
            "HORZMAXPOOL",
            "PRETREATEDRECT",
            "BRICK",
            "DECONVOLUTION",
            "L2NORMALIZE",
            "L2NORMALIZE_SUMSQRT",
            "L2NORMALIZE_SUMSCALE",
            "TENSOR_COPY",
            "CONVERT_FORMAT",
            "TENSOR_REDUCE_SUM",
            "TENSOR_PAD",
            "LSTM_UNIT",
            "LSTM_LAYER",
            "REORG2",
            "TENSOR_ROUNDING",
            "HASHLUT",
            "LSH_PROJECTION",
            "TENSOR_RESHAPE",
            "TENSOR_SCALE",
            "YUV2RGB_SCALE",
            "RNN",
            "SVDF",
            "LUT2",
            "UPSAMPLE",
            "DILATION_RESHUFFLE",
            "DILATION_UPSAMPLE",
            "DILATION_UPSAMPLE2",
            "ADAPTER",
            "INTERLEAVE",
            "DEPTHWISE_CONV",
            "LSTM_RESHUFFLE_INPUT",
            "LSTM_STATE_OUT",
            "TENSOR_REVERSE",
            "USER_VXC",
            "USER_CPU",
            "TENSOR_MEAN",
            "TENSOR_SQUEEZE",
            "TENSOR_STRIDE_SLICE",
            "PRELU",
            "GRU",
            "GRU_LAYER",
            "CONV_LSTM",
            "CONV_LSTM_LAYER",
            "DEPTH_WISE_CONV",
            "SVDF_MAP",
            "SVDF_ROTATION",
        };

        vx_char opTargets[][8] =
        {
            "NONE",
            "SH",
            "NN",
            "TP",
            "SW",
            "SC",
        };

        FILE* fp = fopen(layer->graph->base.context->options.enableOpsDebugInfo, "a+");
        vx_uint32 i = 0;
        fprintf(fp, "layer {\n");

        fprintf(fp, "\t name: \"%d_%s_%s_[%d]\"\n", operationId, operation->layer->name, opName[operation->operatorType], operation->batchCount);
        fprintf(fp, "\t type: \"%s\"\n", opName[operation->operatorType]/*"InnerProduct"*/);
        fprintf(fp, "\t target: \"%s\"\n", opTargets[operation->target]);
        for (i = 0; i < operation->inputsNum; i++)
        {
            if (operation->inputs[i]->type == VX_TYPE_TENSOR)
                fprintf(fp, "\t bottom: \"tensor_%p\"\n", ((vx_tensor)operation->inputs[i])->tensorBuffer);
        }

        for (i = 0; i < operation->outputsNum; i++)
        {
            if (operation->outputs[i]->type == VX_TYPE_TENSOR)
                fprintf(fp, "\t top: \"tensor_%p\"\n", ((vx_tensor)operation->outputs[i])->tensorBuffer);
        }

        fprintf(fp, "}\n\n");

        fclose(fp);
    }
    return status;
}

vx_status vxnneOpDebug_DumpInputs(vx_graph graph, vx_enum type)
{
    vx_status status = VX_SUCCESS;
    vx_uint32 i = 0, j = 0;
    /*
    name: "LSTM"
    layer {
    name: "output_state_in"
    type: "Input"
    top: "tensor_581e408"
    input_param {
    shape {
    dim: 1
    dim: 1
    dim: 2
    dim: 5
    }
    }
    }*/
    vx_uint32 count = (type == VX_INPUT) ? graph->headNodeCount : graph->tailNodeCount;
    FILE* fp = fopen(graph->base.context->options.enableOpsDebugInfo, (type == VX_INPUT) ? "w+" : "a+");

    if (type == VX_INPUT)
        fprintf(fp, "name: \"%s\"\n", "LSTM");

    for (i = 0; i < count; i++)
    {
        vx_int32 index = (type == VX_INPUT) ? graph->headNodeIndexTable[i] : graph->tailNodeIndexTable[i];
        vx_node node = graph->nodeTable[index];
        for (j = 0; j < node->numParameters; j++)
        {
            if (node->kernel->signature.dataTypeTable[j] == VX_TYPE_TENSOR && node->kernel->signature.directionTable[j] == type)
            {
                vx_char name[][8] = { "input", "output" };
                vx_char types[][8] = { "input", "data" };
                vx_char parms[][8] = { "top", "bottom" };
                vx_tensor tensor = (vx_tensor)node->paramTable[j];
                fprintf(fp, "layer {\n");

                fprintf(fp, "\t name: \"%s_%d\"\n", name[type - VX_INPUT], i);
                fprintf(fp, "\t type: \"%s\"\n", types[type - VX_INPUT]);
                fprintf(fp, "\t %s: \"tensor_%p\"\n", parms[type - VX_INPUT], tensor->tensorBuffer);
                fprintf(fp, "\t target: \"%s\"\n", "DATA");

                fprintf(fp, "\t %s_param{\n", types[type - VX_INPUT]);
                fprintf(fp, "\t\t shape{ \n \t\t\tdim:%d\n \t\t\tdim:%d\n \t\t\tdim:%d\n \t\t\tdim:%d\n  \t\t }\n",
                    TENSOR_SIZE_INDEX(tensor, 0), TENSOR_SIZE_INDEX(tensor, 1),
                    TENSOR_SIZE_INDEX(tensor, 2), TENSOR_SIZE_INDEX(tensor, 3));

                fprintf(fp, "\t}\n");

                fprintf(fp, "}\n");
            }
        }
    }

    fclose(fp);

    return status;
}

vx_status vxnneOpDebug_DumpOperatoinInfos(vxnne_layer layer)
{
    vx_status status = VX_SUCCESS;
    vxnne_execution_layer   executionLayer = (vxnne_execution_layer)layer;

    if ((executionLayer != gcvNULL) && (executionLayer->graph->base.context->options.enableOpsDebugInfo != gcvNULL))
    {
        vx_uint32 i = 0;

        vxnneOpDebug_DumpInputs(executionLayer->graph, VX_INPUT);

        for (i = 0; i < executionLayer->opIndicesNum; i++)
        {
            vxnneOpDebug_DumpOperation(executionLayer, i);
        }

        vxnneOpDebug_DumpInputs(executionLayer->graph, VX_OUTPUT);
    }
    return status;
}

