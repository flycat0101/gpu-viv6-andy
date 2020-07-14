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
#include <gc_vx_nn_util.h>
#include "gc_nn_arch_model.h"

#define ENABLE_ARCH_MODEL_DUMP 0

/*******************************************Predict arch performance***************************************************/
#define DISABLE_TP_RESHUFFLE_SPLIT 0
#define MAX_COST 2147483647
#define MAX_LAYERS_OF_BLOCK 100

vx_status calculateArchPerf(vx_context context, vxnne_layer layer, vxnne_operation operation, vx_arch_perf perf, vx_weights_biases_parameter wb, vxnne_operation_target_e op_target, vxnne_operator_e op_type);
vx_status showArchPerformance(vx_context context, vxnne_layer layer, vxnne_operation op, vx_arch_perf perf);

struct _archModelOpInfo
{
    /* fixed */
    vx_node     node;
    vxnne_operation opt;
    vx_enum     op;
    vx_enum     target;
    vx_uint32   inx;
    vx_uint32   iny;
    vx_uint32   inz;
    vx_uint32   origx;
    vx_uint32   origy;
    vx_uint32   origoutx;
    vx_uint32   origouty;
    vx_uint32   stridex;
    vx_uint32   stridey;
    vx_uint32   kx;
    vx_uint32   ky;
    vx_uint32   kz;
    vx_uint32   bfy;
    vx_uint32   bfz;
    vx_uint32   oz;
    vx_uint32   siz;
    vx_uint32   psize;
    vx_uint32   pstride;
    vx_uint32   xpad;
    vx_uint32   ypad;
    vx_uint32   inputDataSize;
    vx_uint32   outputDataSize;
    vx_uint8    fcmd;
    vx_enum     inputDataFormat;
    vx_enum     outputDataFormat;
    vx_uint32   nnCores;
    vx_weights_biases_parameter weight_bias;
    vx_arch_perf_s perf;
    vx_uint32   xsize;
    vx_uint32   ysize;
    /* tmp */
    vx_uint32   pix;
    vx_uint32   piy;
    vx_uint32   p3;
    vx_uint32   psix;
    vx_uint32   psiy;
    /* calculate */
    vx_uint8    sbuf;
    vx_uint8    dbuf;
    vx_uint8    kbuf;
    vx_uint32   swTilingSegKernelBufSizeInPixel;
    vx_uint32   swTilingSegOutBufSizeInPixel;
    vx_uint32   segTotalBufferSizeInPixel;
    vx_int32    swTilingType;/*-1: none, 1: AB, 0: Sub-IMAGE*/
    vx_uint32   upStreamLayerCount;
    vx_uint32   downStreamLayerCount;
    vx_int32    upStreamLayer[MAX_PARENT_CHILD_OP_NUM];
    vx_int32    downStreamLayer[MAX_PARENT_CHILD_OP_NUM];
};

struct _archModelCost
{
    vx_float64 cycle;
    vx_float64 bw;
};

struct _archModelSplitInfo
{
    struct _archModelCost *savedSegmentCost;
    vx_uint32   **savedSIX;
    vx_uint32   **savedSIY;
    vx_uint32   **savedSIZ;
    struct _archModelCost   **savedCost;
    vx_uint8    **split_array;
    vx_int32    *bestCostSWTilingType;
};

struct _archModelInfo
{
    struct _archModelOpInfo **opInfoArray;
    struct _archModelSplitInfo **splitInfoArray;
    vx_uint32 totalOpCount;
};

#define CYCLE_WEIGHT 20
#define BW_WEIGHT 1
vx_bool _cur_cost_is_more_better(struct _archModelCost *cost, struct _archModelCost *cur, vx_uint32 cycle_weight, vx_uint32 bw_weight)
{
    vx_float64 f;
    vx_float64 cycleDiff = cur->cycle - cost->cycle;
    vx_float64 bwDiff = cur->bw - cost->bw;
    if (cycleDiff > 0 && cycleDiff < 1)
    {
        cycleDiff = (vx_float64)((vx_uint64)(cycleDiff * 100000000 + 0.5))/100000000;
    }
    else if (cycleDiff < 0 && cycleDiff > -1)
    {
        cycleDiff = -(vx_float64)((vx_uint64)(-1 * cycleDiff * 100000000 + 0.5))/100000000;
    }
    if (bwDiff > 0 && bwDiff < 1)
    {
        bwDiff = (vx_float64)((vx_uint64)(bwDiff * 100000000 + 0.5))/100000000;
    }
    else if (bwDiff < 0 && bwDiff > -1)
    {
        bwDiff = -(vx_float64)((vx_uint64)(-1 * bwDiff * 100000000 + 0.5))/100000000;
    }

    f = -(1.0f * cycleDiff / gcmMAX(cur->cycle, cost->cycle) * cycle_weight + 1.0f * bwDiff / gcmMAX(cur->bw, cost->bw) * bw_weight);
    if (f > 0) return vx_true_e;
    return vx_false_e;
}

void deInitArchModelSplitInfo(struct _archModelSplitInfo * splitInfo, vx_uint32 operationCount)
{
    vx_uint32 i;
    if (splitInfo == NULL)
    {
        return;
    }

    if (splitInfo->savedSIX)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedSIX[i] != NULL) vxFree(splitInfo->savedSIX[i]);
        }
        vxFree(splitInfo->savedSIX);
    }
    if (splitInfo->savedSIY)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedSIY[i] != NULL) vxFree(splitInfo->savedSIY[i]);
        }
        vxFree(splitInfo->savedSIY);
    }
    if (splitInfo->savedSIZ)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedSIZ[i] != NULL) vxFree(splitInfo->savedSIZ[i]);
        }
        vxFree(splitInfo->savedSIZ);
    }

    if (splitInfo->savedCost)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedCost[i] != NULL) vxFree(splitInfo->savedCost[i]);
        }
        vxFree(splitInfo->savedCost);
    }

    if (splitInfo->savedSegmentCost) vxFree(splitInfo->savedSegmentCost);
    if (splitInfo->bestCostSWTilingType) vxFree(splitInfo->bestCostSWTilingType);
    if (splitInfo->split_array)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->split_array[i] != NULL) vxFree(splitInfo->split_array[i]);
        }
        vxFree(splitInfo->split_array);
    }

    vxFree(splitInfo);
}

void emptyArchModelSplitInfo(struct _archModelSplitInfo * splitInfo, vx_uint32 operationCount)
{
    vx_uint32 i;
    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedSIX[i], 0, gcmSIZEOF(vx_uint32) * operationCount);
    }

    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedSIY[i], 0, gcmSIZEOF(vx_uint32) * operationCount);
    }

    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedSIZ[i], 0, gcmSIZEOF(vx_uint32) * operationCount);
    }

    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedCost[i], 0, gcmSIZEOF(struct _archModelCost) * operationCount);
    }

    memset(splitInfo->savedSegmentCost, 0, gcmSIZEOF(struct _archModelCost) * operationCount);
    memset(splitInfo->bestCostSWTilingType, 0, gcmSIZEOF(vx_bool) * operationCount);

    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->split_array[i], 0, sizeof(vx_uint8) * operationCount);
    }
}

void resetArchModelSplitInfo(struct _archModelInfo *archModel)
{
    vx_uint32 i;
    for (i = 0; i < ((archModel->totalOpCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : archModel->totalOpCount); i++)
    {
        emptyArchModelSplitInfo(archModel->splitInfoArray[i], ((archModel->totalOpCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : archModel->totalOpCount));
    }
}

struct _archModelSplitInfo * initArchModelSplitInfo(vx_uint32 operationCount)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 i;
    struct _archModelSplitInfo *splitInfo = NULL;
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelSplitInfo),
        (gctPOINTER *)&splitInfo
        );
    if (gcmIS_ERROR(status))
    {
        return NULL;
    }

    memset(splitInfo, 0, gcmSIZEOF(struct _archModelSplitInfo));

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32 *) * operationCount,
        (gctPOINTER *)&splitInfo->savedSIX
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedSIX, 0, sizeof(vx_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_uint32) * operationCount,
            (gctPOINTER *)&splitInfo->savedSIX[i]
            );
        if (gcmIS_ERROR(status)) goto error;

        memset(splitInfo->savedSIX[i], 0, gcmSIZEOF(vx_uint32) * operationCount);
    }

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32 *) * operationCount,
        (gctPOINTER *)&splitInfo->savedSIY
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedSIY, 0, gcmSIZEOF(vx_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_uint32) * operationCount,
            (gctPOINTER *)&splitInfo->savedSIY[i]
            );
        if (gcmIS_ERROR(status)) goto error;

        memset(splitInfo->savedSIY[i], 0, gcmSIZEOF(vx_uint32) * operationCount);
    }

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32 *) * operationCount,
        (gctPOINTER *)&splitInfo->savedSIZ
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedSIZ, 0, sizeof(vx_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_uint32) * operationCount,
            (gctPOINTER *)&splitInfo->savedSIZ[i]
            );
        if (gcmIS_ERROR(status)) goto error;

        memset(splitInfo->savedSIZ[i], 0, gcmSIZEOF(vx_uint32) * operationCount);
    }

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelCost *) * operationCount,
        (gctPOINTER *)&splitInfo->savedCost
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedCost, 0, sizeof(struct _archModelCost *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(struct _archModelCost) * operationCount,
            (gctPOINTER *)&splitInfo->savedCost[i]
            );
        if (gcmIS_ERROR(status)) goto error;
        memset(splitInfo->savedCost[i], 0, gcmSIZEOF(struct _archModelCost) * operationCount);
    }

    status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(struct _archModelCost) * operationCount,
            (gctPOINTER *)&splitInfo->savedSegmentCost
            );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->savedSegmentCost, 0, gcmSIZEOF(struct _archModelCost) * operationCount);

    status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_bool) * operationCount,
            (gctPOINTER *)&splitInfo->bestCostSWTilingType
            );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->bestCostSWTilingType, 0, gcmSIZEOF(vx_bool) * operationCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint8 *) * operationCount,
        (gctPOINTER *)&splitInfo->split_array
        );
    if (gcmIS_ERROR(status)) goto error;
    memset(splitInfo->split_array, 0, gcmSIZEOF(vx_uint8 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(vx_uint8) * operationCount,
            (gctPOINTER *)&splitInfo->split_array[i]
            );
        if (gcmIS_ERROR(status)) goto error;

        memset(splitInfo->split_array[i], 0, sizeof(vx_uint8) * operationCount);
    }

    return splitInfo;

error:
    deInitArchModelSplitInfo(splitInfo, operationCount);
    vxInfo("ERROR: initArchModelSplitInfo() return out-of-memory\n");
    return NULL;
}

void deInitArchModelInfo(struct _archModelInfo *archModel, vx_uint32 operationCount)
{
    vx_uint32 i;
    if (archModel == NULL)
    {
        return;
    }

    for (i = 0; i < operationCount; i++)
    {
        if (archModel->opInfoArray && archModel->opInfoArray[i] != NULL)
        {
            vxFree(archModel->opInfoArray[i]);
        }
    }
    for (i = 0; i < ((operationCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : operationCount); i++)
    {
        if (archModel->splitInfoArray && archModel->splitInfoArray[i] != NULL)
        {
            deInitArchModelSplitInfo(archModel->splitInfoArray[i], ((operationCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : operationCount));
        }
    }
    if (archModel->opInfoArray != NULL) vxFree(archModel->opInfoArray);
    if (archModel->splitInfoArray != NULL) vxFree(archModel->splitInfoArray);

    vxFree(archModel);
}

struct _archModelInfo * initArchModelInfo(vx_uint32 operationCount)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 i;
    struct _archModelInfo *archModel;
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelInfo),
        (gctPOINTER *)&archModel
        );
    if (gcmIS_ERROR(status)) goto error;
    archModel->totalOpCount = operationCount;

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelOpInfo *) * operationCount,
        (gctPOINTER *)&archModel->opInfoArray
        );
    if (gcmIS_ERROR(status)) goto error;

    for (i = 0; i < operationCount; i++)
    {
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(struct _archModelOpInfo),
            (gctPOINTER *)&archModel->opInfoArray[i]
            );
        if (gcmIS_ERROR(status)) goto error;
        memset(archModel->opInfoArray[i], 0, gcmSIZEOF(struct _archModelOpInfo));
    }

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(struct _archModelSplitInfo *) * ((operationCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : operationCount),
        (gctPOINTER *)&archModel->splitInfoArray
        );
    if (gcmIS_ERROR(status)) goto error;

    for (i = 0; i < ((operationCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : operationCount); i++)
    {
        archModel->splitInfoArray[i]= initArchModelSplitInfo(((operationCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : operationCount));
        if (archModel->splitInfoArray[i] == NULL) goto error;
    }

    return archModel;

error:
    if (archModel != NULL) {
        deInitArchModelInfo(archModel, operationCount);
    }
    vxInfo("ERROR: initArchModelInfo() return out-of-memory\n");
    return NULL;
}

static void initSegmentCostResult(struct _archModelInfo * archModel)
{
    vx_uint32 i, j;
    vx_uint32 count = archModel->totalOpCount > MAX_LAYERS_OF_BLOCK ? MAX_LAYERS_OF_BLOCK : archModel->totalOpCount;
    for (i = 0; i < count; i++)
    {
        for (j = 0; j < count; j++)
        {
            archModel->splitInfoArray[i]->savedSegmentCost[j].bw = -1;
            archModel->splitInfoArray[i]->savedSegmentCost[j].cycle = -1;
        }
    }
}

static void getSegmentCostResult(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    struct _archModelCost *cost)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    cost->cycle = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSegmentCost[segment_last % MAX_LAYERS_OF_BLOCK].cycle;
    cost->bw = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSegmentCost[segment_last % MAX_LAYERS_OF_BLOCK].bw;
}

static void setSegmentCostResult(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    struct _archModelCost *cost)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSegmentCost[segment_last % MAX_LAYERS_OF_BLOCK].cycle = cost->cycle;
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSegmentCost[segment_last % MAX_LAYERS_OF_BLOCK].bw = cost->bw;
}


static void setBestCostSWTilingTypeInfo(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_int32 bestCostSWTilingType)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->bestCostSWTilingType[segment_last % MAX_LAYERS_OF_BLOCK] = bestCostSWTilingType;
}
static void setSplitArrayInfo(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_int32 pos,
    vx_int8 split)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->split_array[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = split;
}


static void saveCalculationArgs(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 pos,
    struct _archModelCost *cost,
    vx_uint32 six,
    vx_uint32 siy,
    vx_uint32 siz,
    vx_uint8 split)
{
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSIX[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = six;
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSIY[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = siy;
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSIZ[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = siz;
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->split_array[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = split;

    if (cost != NULL)
    {
         archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCost[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK].cycle = cost->cycle;
         archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCost[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK].bw = cost->bw;
    }
}

static vx_float64 _calc_cost(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_int32 index,
    vx_uint32 x,
    vx_uint32 y,
    vx_uint32 z,
    vx_uint8 src_buf,
    vx_uint8 dst_buf,
    vx_uint8 kenerl_buf,
    vx_int32 cache_space)
{
    vx_status status;
    vx_arch_perf perf = &archModel->opInfoArray[index]->perf;
    struct _archModelOpInfo * opInfo = archModel->opInfoArray[index];

    gcmASSERT(index >= 0);
    perf->calculated = vx_false_e;
    perf->info.kx   = opInfo->kx;
    perf->info.ky   = opInfo->ky;
    perf->info.kz   = opInfo->kz;
    perf->info.inputDataFormat = opInfo->inputDataFormat;

    /*save original input x/y/z*/
    perf->info.oinx = opInfo->inx;
    perf->info.oiny = opInfo->iny;
    perf->info.oinz = opInfo->inz;
    perf->info.inx = x;
    perf->info.iny = y;
    perf->info.inz = opInfo->kz;
    perf->info.outx = x;
    perf->info.outy = y;
    perf->info.outz = z;
    perf->info.stridex = opInfo->stridex ? opInfo->stridex : 1;
    perf->info.stridey = opInfo->stridey ? opInfo->stridey : 1;
    perf->info.poolingSize   = opInfo->psize;
    perf->info.poolingStride = opInfo->pstride;
    perf->info.xOffSet = (-1) * opInfo->xpad;
    perf->info.yOffSet = (-1) * opInfo->ypad;
    perf->info.inputDataSize = opInfo->inputDataSize;
    perf->info.outputDataSize = opInfo->outputDataSize;
    perf->info.outputDataFormat = opInfo->outputDataSize;
    perf->info.nnCores = opInfo->nnCores;
    if (opInfo->target == VXNNE_OPERATION_TARGET_TP && opInfo->op == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        perf->info.nnCores += context->nnConfig.fixedFeature.tpliteCoreCount;
    }
    perf->swTilingInfo.origInX = opInfo->origx;
    perf->swTilingInfo.origInY = opInfo->origy;
    perf->swTilingInfo.origOutX = opInfo->origoutx;
    perf->swTilingInfo.origOutY = opInfo->origouty;
    perf->swTilingInfo.origOutZ = opInfo->oz;
    perf->swTilingInfo.srcBuf    = src_buf;
    perf->swTilingInfo.dstBuf    = dst_buf;
    perf->swTilingInfo.kernelBuf = kenerl_buf;
    perf->swTilingInfo.calcNonFirstCmd = opInfo->fcmd ? vx_true_e : vx_false_e;
    perf->swTilingInfo.cacheSpace = cache_space;
    perf->swTilingInfo.swTilingSegKernelBufSizeInPixel = opInfo->swTilingSegKernelBufSizeInPixel;
    perf->swTilingInfo.segTotalBufferSizeInPixel = opInfo->segTotalBufferSizeInPixel;
    perf->info.pix = opInfo->pix;
    perf->info.piy = opInfo->piy;
    perf->info.p3 = opInfo->p3;
    perf->info.nextKY = ((vx_uint32)(index + 1) < archModel->totalOpCount) ? archModel->opInfoArray[index + 1]->ky : 0;
    if ((opInfo->target != VXNNE_OPERATION_TARGET_TP || opInfo->op == VXNNE_OPERATOR_FULLYCONNECTED) && opInfo->weight_bias)
        perf->info.kernelSize = (vx_uint32)(gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(opInfo->weight_bias, 0), CACHE_ALIGNMENT_SIZE));

    if ((opInfo->target == VXNNE_OPERATION_TARGET_TP) && (opInfo->op == VXNNE_OPERATOR_FULLYCONNECTED))
    {
        perf->imageNonZeroRatio = opInfo->opt->imgNonZeroRatio;
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1))
    {
        vx_uint32 repeatePerFrame = 1;
        vx_uint32 cnum = (vx_uint32)(ceilf((vx_float32)opInfo->xsize / x)
                            * ceilf((vx_float32)opInfo->ysize / y)
                            * ceilf((vx_float32)opInfo->oz / z) * repeatePerFrame);

        memset(&perf->resultInfo, 0, sizeof(vx_performance_info_s));
        status = calculateArchPerf(context,
            opInfo->opt->layer,
            VX_NULL,
            perf,
            opInfo->weight_bias,
            opInfo->target,
            (opInfo->op == VXNNE_OPERATOR_ROIPOOL
             && opInfo->opt->parameter.tpType == TP_ROI_POOLING_STEP_1) ? VXNNE_OPERATOR_POOLING :
            opInfo->op);
        if (status != VX_SUCCESS)
        {
            return (vx_float64)MAX_COST;
        }
        if (perf->opTarget == VXNNE_OPERATION_TARGET_NN)
        {
            perf->resultInfo.perfCycleCount     += (cnum - 1) * perf->swTilingInfo.perfNonFirstCycleCount;
            perf->resultInfo.perfReadBandWidth  += (cnum - 1) * perf->swTilingInfo.perfNonFirstReadBandWidth;
            perf->resultInfo.perfWriteBandWidth += (cnum - 1) * perf->swTilingInfo.perfNonFirstWriteBandWidth;
            perf->resultInfo.perfAXIReadBandWidth  += (cnum - 1) * perf->swTilingInfo.perfNonFirstAXIReadBandWidth;
            perf->resultInfo.perfAXIWriteBandWidth += (cnum - 1) * perf->swTilingInfo.perfNonFirstAXIWriteBandWidth;
            perf->resultInfo.perfKernelReadBandWidth += (cnum - 1) * perf->swTilingInfo.perfNonFirstKernelReadBandWidth;
            perf->resultInfo.perfInImageReadBandWidth += (cnum - 1) * perf->swTilingInfo.perfNonFirstInImageReadBandWidth;
        }
        else
        {
            perf->resultInfo.perfCycleCount     = cnum * perf->resultInfo.perfCycleCount;
            perf->resultInfo.perfReadBandWidth  = cnum * perf->resultInfo.perfReadBandWidth;
            perf->resultInfo.perfWriteBandWidth = cnum * perf->resultInfo.perfWriteBandWidth;
            perf->resultInfo.perfKernelReadBandWidth += cnum * perf->swTilingInfo.perfNonFirstKernelReadBandWidth;
            perf->resultInfo.perfInImageReadBandWidth += cnum * perf->swTilingInfo.perfNonFirstInImageReadBandWidth;
        }
    }
    else
    {
        memset(&perf->resultInfo, 0, sizeof(vx_performance_info_s));
        status = calculateArchPerf(context,
            opInfo->node->layer,
            opInfo->opt,
            perf,
            opInfo->weight_bias,
            opInfo->target,
            (opInfo->op == VXNNE_OPERATOR_ROIPOOL
            && opInfo->opt->parameter.tpType == TP_ROI_POOLING_STEP_1) ? VXNNE_OPERATOR_POOLING :
            opInfo->op);
        if (status != VX_SUCCESS)
        {
            return (vx_float64)MAX_COST;
        }
    }
    return perf->resultInfo.perfCycleCount;
}

static vx_uint32 _kernel_size_in_pixel(struct _archModelInfo *archModel, vx_int32 index, vx_uint32 cores, vx_bool full_chache_kernel_head_fix)
{
    struct _archModelOpInfo * opInfo = archModel->opInfoArray[index];
    vx_float64 coefCompressionRatio = 0;
    vx_float64 margin_ratio = 1.05;
    if (opInfo->weight_bias != NULL)
    {
        coefCompressionRatio = WB_COMPRESS_RATIO(opInfo->weight_bias);
        margin_ratio = (1.25 - 1.05) * (1.0 - coefCompressionRatio) / (1.0 - 0.02) + 1.05;
    }

    if (opInfo->op == VXNNE_OPERATOR_DEPTH_WISE_CONV)
    {
        if (full_chache_kernel_head_fix)
        {
            return (vx_uint32)(opInfo->kx
                      * opInfo->ky
                      * opInfo->oz
                      * coefCompressionRatio * margin_ratio + 0.5f);
        }
        else
        {
            return (vx_uint32)(opInfo->kx
                      * opInfo->ky
                      * ceilf((vx_float32)opInfo->oz / cores) * cores
                      * coefCompressionRatio * margin_ratio + 0.5f);
        }
    }

    if (opInfo->target != VXNNE_OPERATION_TARGET_TP || opInfo->op == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        if (full_chache_kernel_head_fix)
        {
            return (vx_uint32)(opInfo->kx
                      * opInfo->ky
                      * opInfo->kz
                      * opInfo->oz
                      * coefCompressionRatio * margin_ratio * 1.05f + 0.5f);
        }
        else
        {
           return (vx_uint32)(opInfo->kx
                      * opInfo->ky
                      * opInfo->kz
                      * ceilf((vx_float32)opInfo->oz / cores) * cores
                      * coefCompressionRatio * margin_ratio * 1.05f + 0.5f);
        }
    }
    return 0;
}

static vx_uint32 _outbuf_needed_ex(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 psixArray[],
    vx_uint32 psiyArray[],
    vx_uint32 psizArray[])
{
    vx_int32 outBufNeeded = 0, i;

    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    for (i = segment_first; i < segment_last; i++)
    {
        if (archModel->opInfoArray[i + 1]->target != VXNNE_OPERATION_TARGET_TP || (archModel->opInfoArray[i + 1]->op == VXNNE_OPERATOR_FULLYCONNECTED)) {
            if (archModel->opInfoArray[i + 1]->kx == 1 && archModel->opInfoArray[i + 1]->ky == 1)
            {
                outBufNeeded += archModel->opInfoArray[i]->bfy
                    * ((psixArray == NULL) ? archModel->opInfoArray[i]->pix : psixArray[i])
                    * ((psiyArray == NULL) ? archModel->opInfoArray[i]->piy : psiyArray[i])
                    * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i]->oz;
            }
            else
            {
                outBufNeeded += ((psixArray == NULL) ? archModel->opInfoArray[i]->pix : psixArray[i])
                    * gcmMIN((gcmMAX(
                        archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i]->piy : psiyArray[i]),
                        archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i + 1]->piy : psiyArray[i + 1])
                        * archModel->opInfoArray[i + 1]->pstride)
                        + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1),
                        archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->piy)
                    * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
            }
        }
        else
        {
            outBufNeeded += ((psixArray == NULL) ? archModel->opInfoArray[i]->pix : psixArray[i])
                * gcmMIN((gcmMAX(
                    archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i]->piy : psiyArray[i]),
                    archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i + 1]->piy : psiyArray[i + 1])
                    * archModel->opInfoArray[i + 1]->pstride)
                    + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1),
                    archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->piy)
                * (archModel->opInfoArray[i]->bfz * ((psizArray == NULL) ? archModel->opInfoArray[i]->siz : psizArray[i]) + 1 /*tpkz*/ - 1);
        }
    }
    return outBufNeeded;
}

vx_uint32 _calc_full_cached_space_needed(struct _archModelInfo *archModel, vx_uint32 segment_index, vx_uint32 psixArray[], vx_uint32 psiyArray[], vx_uint32 max_tile_size)
{
    if (archModel->opInfoArray[segment_index]->target != VXNNE_OPERATION_TARGET_TP
       || (archModel->opInfoArray[segment_index]->opt->operatorType == VXNNE_OPERATOR_FULLYCONNECTED)
       )
    {
        return (gcmMIN(psixArray[segment_index] * archModel->opInfoArray[segment_index]->pstride, max_tile_size) + archModel->opInfoArray[segment_index]->kx - 1)
               * (psiyArray[segment_index] * archModel->opInfoArray[segment_index]->pstride + archModel->opInfoArray[segment_index]->ky - 1) * archModel->opInfoArray[segment_index]->kz;
    }
    return 0;
}

static vx_bool _calc_y_subimage(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_int32 vip_sram_left,
    vx_int32 axi_sram_left,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint32 max_tile_size)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 *sixArray, *siyArray;
    vx_int32 i, termA = 0, termB = 0;
    vx_int32 m, firstLayerInputCacheSize;
    vx_bool doSubImg = vx_false_e;

    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&sixArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _calc_y_subimage() return out-of-memory\n");
        return vx_false_e;
    }

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&siyArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _calc_y_subimage() return out-of-memory\n");
        goto OnError;
    }

    for (i = segment_first; i <= segment_last - 1; i++)
    {
        vx_int32 termAIn, termBIn;
        if (archModel->opInfoArray[i + 1]->target != VXNNE_OPERATION_TARGET_TP ||
            (archModel->opInfoArray[i + 1]->op == VXNNE_OPERATOR_FULLYCONNECTED))
        {
            if (archModel->opInfoArray[i]->target != VXNNE_OPERATION_TARGET_TP ||
            (archModel->opInfoArray[i]->op == VXNNE_OPERATOR_FULLYCONNECTED))
            {
                termAIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * archModel->opInfoArray[i + 1]->pstride * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
                termBIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * (archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
            }
            else
            {
                termAIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * archModel->opInfoArray[i + 1]->pstride * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i]->oz;
                termBIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * (archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i]->oz;
            }
        }
        else
        {
            termAIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * archModel->opInfoArray[i + 1]->pstride * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);
            termBIn = archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * (archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);
        }

        termA += termAIn;
        termB += termBIn;
    }

    /* Calculate largest M that fits */
    if ((axi_sram_left == 0) &&
        ((archModel->opInfoArray[segment_first]->target != VXNNE_OPERATION_TARGET_TP) ||
         (archModel->opInfoArray[segment_first]->op == VXNNE_OPERATOR_FULLYCONNECTED)))
    {
        vx_int32 termC = (gcmMIN(archModel->opInfoArray[segment_first]->xsize, max_tile_size) + archModel->opInfoArray[segment_first]->kx - 1);
        vx_int32 termD = (gcmMIN(archModel->opInfoArray[segment_first]->xsize, max_tile_size) + archModel->opInfoArray[segment_first]->kx - 1);
        termC = vip_sram_left - (termB + termC * (archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz);
        termD = termA + termD * archModel->opInfoArray[segment_first]->pstride * archModel->opInfoArray[segment_first]->kz;
        m = (vx_int32)(termC / termD);
    }
    else if (axi_sram_left == 0)
    {
        m = (vx_int32)((vip_sram_left - termB) / termA);
    }
    else
    {
        m = (vx_int32)((axi_sram_left - termB) / termA);
    }

    {
       vx_int32 min_m = (vx_int32)(192 / archModel->opInfoArray[segment_last]->pix);
       if (m > 0)
       {
           vx_int32 temp_m = m;
           while (((archModel->opInfoArray[segment_last]->piy % temp_m) > 0) && (temp_m > min_m))
           {
               temp_m = temp_m - 1;
           }
           m = (temp_m == min_m) ? gcmMAX(m, temp_m) : temp_m;
       }
    }

#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("M=%d\n", m);
#endif
    if (axi_sram_left > 0)
    {
        sixArray[segment_first] = archModel->opInfoArray[segment_first]->xsize;
        siyArray[segment_first] = gcmMIN(m, (vx_int32)archModel->opInfoArray[segment_first]->piy);
        firstLayerInputCacheSize = _calc_full_cached_space_needed(archModel, segment_first, sixArray, siyArray, max_tile_size);
        if (m > 0 && firstLayerInputCacheSize < vip_sram_left)
        {
            doSubImg = vx_true_e;
        }
    }
    else
    {
        if (m > 0)
            doSubImg = vx_true_e;
    }

    if (doSubImg)
    {
        for (i = segment_first; i <= segment_last; i++)
        {
            sixArray[i] = archModel->opInfoArray[i]->xsize;
            siyArray[i] = gcmMIN((vx_uint32)m, archModel->opInfoArray[i]->piy);
        }

        for (i = segment_first; i <= segment_last; i++)
        {
            vx_int32 outbufNeeded;
            siyArray[i] = gcmMIN((vx_uint32)(m * 2), archModel->opInfoArray[i]->piy);
            outbufNeeded = _outbuf_needed_ex(archModel, segment_first, segment_last, sixArray, siyArray, z_array);
            firstLayerInputCacheSize = _calc_full_cached_space_needed(archModel, segment_first, sixArray, siyArray, max_tile_size);
            if ((axi_sram_left == 0 && vip_sram_left < (outbufNeeded + firstLayerInputCacheSize)) ||
                (axi_sram_left > 0 && ((axi_sram_left < outbufNeeded) || (vip_sram_left < firstLayerInputCacheSize))))
            {
                siyArray[i] = gcmMIN((vx_int32)m, (vx_int32)archModel->opInfoArray[i]->piy); /* put M back */
                break;
            }
        }
    }
    /* generate SIY */
    for (i = segment_first; i <= segment_last; i++)
    {
        x_array[i] = archModel->opInfoArray[i]->xsize;
        if (doSubImg)
        {
            y_array[i] = gcmMIN(siyArray[i] * archModel->opInfoArray[i]->pstride + archModel->opInfoArray[i]->p3, archModel->opInfoArray[i]->ysize);
        }
        else
        {
            y_array[i] = 0;
        }
    }

    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    return vx_true_e;

OnError:
    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    return vx_false_e;
}


static vx_bool _calc_x_subimage(
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_int32 sram_left,
    vx_uint32 ppsiy,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint32 max_tile_size)
{
    /* This routine compute the SubImageYSize assuming SubImageXSize = ImageXSize */
    vx_int32 termC = 0, termD = 0, termE = 0, termF = 0;
    vx_uint32 sumProdPSKX = archModel->opInfoArray[segment_last]->p3 + archModel->opInfoArray[segment_last]->kx - 1, prevProdPS = 0, prevSumProdPSKX = 0;
    vx_uint32 prodPoolStride = archModel->opInfoArray[segment_last]->pstride;
    vx_int32 i, n, /*ds = archModel->opInfoArray[segment_first]->dsize / 8, lcm = 1,*/ psix = 0;
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);

    for (i = (vx_int32)segment_last - 1; i >= (vx_int32)segment_first; i--)
    {
        vx_int32 termCIn, termDIn;
        /*vx_uint32 d = archModel->opInfoArray[i]->target == archModel->opInfoArray[i - 1]->target ? 1 : 2;*/
        if (archModel->opInfoArray[i + 1]->target != VXNNE_OPERATION_TARGET_TP ||
            (archModel->opInfoArray[i + 1]->op == VXNNE_OPERATOR_FULLYCONNECTED))
        {
            termCIn = prodPoolStride * (ppsiy * archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i + 1]->pstride + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
            termDIn = sumProdPSKX * (ppsiy * archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i + 1]->pstride + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
        }
        else
        {
            termCIn = prodPoolStride * (ppsiy * archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i + 1]->pstride + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);
            termDIn = sumProdPSKX * (ppsiy * archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i + 1]->pstride + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);
        }

        termC += termCIn;
        termD += termDIn;
        prevProdPS = prodPoolStride;
        prevSumProdPSKX = sumProdPSKX;
        prodPoolStride = prodPoolStride * archModel->opInfoArray[i]->pstride;
        sumProdPSKX = sumProdPSKX * archModel->opInfoArray[i]->pstride + archModel->opInfoArray[i]->p3 + archModel->opInfoArray[i]->kx - 1 + 2 * (1 - 1);
    }

    termE = prodPoolStride * (ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz + termC;
    termF = sumProdPSKX * (ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz + termD;

    /* Calculate largest M that fits */
    if (archModel->opInfoArray[segment_first]->target != VXNNE_OPERATION_TARGET_TP ||
        (archModel->opInfoArray[segment_first]->op == VXNNE_OPERATOR_FULLYCONNECTED))
    {
        n = gcmMAX((vx_int32)(1.0f * (sram_left - termF)/termE),
            (vx_int32)((sram_left - (vx_int32)(termD + (64 + archModel->opInfoArray[segment_first]->kx - 1) * (ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz)) / termC));
    }
    else
    {
        n = (vx_int32)((sram_left - termD)/termC);
    }

    psix = n * prevProdPS + prevSumProdPSKX;
#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("N=%d, PSIX=%d, segment(%d, %d), PPSIY: %d\n", n, psix, segment_first + 1, segment_last + 1, ppsiy);
#endif
    x_array[segment_first] = gcmMIN(psix * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3, archModel->opInfoArray[segment_first]->xsize);
    x_array[segment_first] = (vx_uint32)(x_array[segment_first] / 64) * 64;
    y_array[segment_first] = ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3;
    for (i = (vx_int32)(segment_first + 1); i <= (vx_int32)segment_last; i++)
    {
        psix = (vx_uint32)ceilf((vx_float32)(x_array[i-1] - archModel->opInfoArray[i - 1]->p3) / archModel->opInfoArray[i - 1]->pstride);
        if (n > 0)
        {
            x_array[i] = gcmMIN(psix - (archModel->opInfoArray[i]->kx - 1), archModel->opInfoArray[i]->xsize);
        }
        else
        {
            x_array[i] = 0;
        }

        y_array[i]  = ppsiy * archModel->opInfoArray[i]->pstride + archModel->opInfoArray[i]->p3;
    }

    return vx_true_e;
}


/* return sram space left size*/
static vx_int32 _calc_ab_buffer(
    struct _archModelInfo *archModel,
    vx_int32 segment_first, vx_int32 segment_last,
    vx_uint32 sram_space,
    vx_uint32 x_array[], vx_uint32 y_array[])
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_uint32 abBufSize[2] = {0, 0}, outBufNeeded = 0, abBufPairSize = 0;
    vx_uint32 *sixArray, *siyArray;
    vx_int32 i, leftSramSize;
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&sixArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _calc_ab_buffer() return out-of-memory\n");
        return 0;
    }
    gcmASSERT(sixArray != NULL);
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&siyArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _calc_ab_buffer() return out-of-memory\n");
        goto OnError;
    }

    for (i = segment_first; i <= segment_last; i++)
    {
        sixArray[i] = (vx_uint32)(ceilf((vx_float32)(archModel->opInfoArray[i]->xsize - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
        siyArray[i] = (vx_uint32)(ceilf((vx_float32)(archModel->opInfoArray[i]->ysize - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
    }

    for (i = segment_first; i <= segment_last - 1; i++)
    {
        outBufNeeded = _outbuf_needed_ex(archModel, i, i + 1, sixArray, siyArray, NULL);
        abBufSize[i % 2] = outBufNeeded;
        abBufPairSize = gcmMAX(abBufPairSize, abBufSize[0] + abBufSize[1]);
    }
    leftSramSize = (vx_int32)(sram_space - abBufPairSize);
    if (leftSramSize < 0)
    {
        for (i = segment_first; i <= segment_last; i++)
        {
            x_array[i] = 0;
            y_array[i] = 0;
        }
    }
    else
    {
        for (i = segment_first; i <= segment_last; i++)
        {
            x_array[i] = archModel->opInfoArray[i]->xsize;
            y_array[i] = archModel->opInfoArray[i]->ysize;
        }
    }

    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    return leftSramSize;

OnError:
    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    return 0;
}
static void _subimage_segment_cost(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_int32 *best_cost_swtiling_type,
    struct _archModelCost *cost)
{
    gceSTATUS status = gcvSTATUS_OK;
    vx_int32 i, j;
    struct _archModelCost bestCost = {MAX_COST, MAX_COST};
    struct _archModelCost cur_cost = {MAX_COST, MAX_COST};
    vx_uint32 *xArray = NULL, *yArray = NULL, *zArray = NULL;
    vx_uint32 kernelBufNeeded = 0;
    vx_uint32 sramLeft/*, ds = archModel->opInfoArray[segment_first]->dsize / 8*/;
    vx_uint32 vipSramSpaceSize = vxmARCH_VIP_SRAM_SIZE;
    vx_uint32 axiSramSpaceSize = vxmARCH_AXI_SRAM_SIZE;
    vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling ? vx_true_e : vx_false_e;
    vx_uint32 sramSpaceSize = axiSramOnlySWTiling ? (vx_int32)axiSramSpaceSize : (vx_int32)vipSramSpaceSize;
    vx_int32 abBufferSpaceLeft = 0;
    vx_bool fullCacheKernelHeadFix = context->nnConfig.unifiedFeature.fullCacheKernelHeadFix ? vx_true_e : vx_false_e;
    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&xArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _subimage_segment_cost(1) return out-of-memory\n");
        return;
    }
    memset(xArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&yArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _subimage_segment_cost(2) return out-of-memory\n");
        goto exit;
    }
    memset(yArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&zArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _subimage_segment_cost(3) return out-of-memory\n");
        goto exit;
    }
    memset(zArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    if (segment_first == segment_last)
    {
        x_array[segment_first] = archModel->opInfoArray[segment_first]->xsize;
        y_array[segment_first] = archModel->opInfoArray[segment_first]->ysize;
        z_array[segment_first] = archModel->opInfoArray[segment_first]->oz;
        archModel->opInfoArray[segment_first]->perf.info.flush = 1;
        cost->cycle = _calc_cost(
            context,
            archModel,
            segment_first,
            x_array[segment_first],
            y_array[segment_first],
            z_array[segment_first],
            SW_TILING_FROM_DDR,
            SW_TILING_FROM_DDR,
            SW_TILING_FROM_DDR,
            vipSramSpaceSize);
        cost->bw = archModel->opInfoArray[segment_first]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[segment_first]->perf.resultInfo.perfWriteBandWidth;
        goto exit;
    }
    else
    {
        vx_bool hasUnsupportedTPLayer = vx_false_e;
        vx_uint32 tpCircularBuf = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE2) ? 1 : 0;
        for (i = segment_first; i <= segment_last; i++)
        {
            zArray[i] = archModel->opInfoArray[i]->oz;
        }

        for (i = segment_first; i <= segment_last; i++)
        {
            kernelBufNeeded += _kernel_size_in_pixel(archModel, i, archModel->opInfoArray[i]->nnCores, fullCacheKernelHeadFix);
            if ((archModel->opInfoArray[i]->target == VXNNE_OPERATION_TARGET_TP) && (tpCircularBuf == 0))
            {
                hasUnsupportedTPLayer = vx_true_e;
            }
        }

        if (kernelBufNeeded <= sramSpaceSize || 1)
        {
            vx_int32 starti = 0, endi = 8;
            if (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_ALL) /*AB + SUBIMAGE for SWTiling*/
            {
                starti = 0;
                endi = 8;
            }
            else if (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_AB)/*AB for SWTiling*/
            {
                starti = 0;
                endi = 0;
            }
            else if (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_TILING) /*SUBIMAGE for SWTiling*/
            {
                starti = 1;
                endi = 8;
            }

            axiSramSpaceSize = axiSramOnlySWTiling ? axiSramSpaceSize : 0;
            vipSramSpaceSize = vipSramSpaceSize - kernelBufNeeded;
            sramLeft = (vx_uint32)gcmMAX((vx_int32)(axiSramSpaceSize - kernelBufNeeded), (vx_int32)vipSramSpaceSize);
            for (i = starti; i <= endi; i++)
            {
                vx_float64 cost_cycle = 0;
                vx_float64 cost_bw = 0;
                if ((i == 0)
                    || ((i == 1) && (kernelBufNeeded < vxmARCH_VIP_SRAM_SIZE) && !hasUnsupportedTPLayer)
                  || ((i >= 3) && ((i % 1 /*LcmPSIYSmallUnit*/) == 0) && (kernelBufNeeded < vxmARCH_VIP_SRAM_SIZE) && !hasUnsupportedTPLayer))
                {
                    if (!i)
                    {
                        abBufferSpaceLeft = _calc_ab_buffer(archModel, segment_first, segment_last, sramSpaceSize, xArray, yArray);
                    }
                    else if (i == 1)
                    {
                        _calc_y_subimage(archModel, segment_first, segment_last, vipSramSpaceSize, axiSramSpaceSize, xArray, yArray, zArray, context->nnConfig.unifiedFeature.maxTileSize);
                    }
                    else
                    {
                        _calc_x_subimage(archModel, segment_first, segment_last, sramLeft, i, xArray, yArray, zArray, context->nnConfig.unifiedFeature.maxTileSize);
                    }

                    for (j = segment_first; j <= segment_last; j++)
                    {
                        vx_float64 c = 0;
                        vx_bool flush_and_wait = vx_false_e;
                        if ((segment_first + 1) == segment_last)
                        {
                            flush_and_wait = vx_true_e;
                        }
                        archModel->opInfoArray[j]->perf.info.flush = 0;
                        if (xArray[j] > 0 && yArray[j] > 0)
                        {
                            if (i == 0)
                            {
                                if (axiSramOnlySWTiling)
                                {
                                    abBufferSpaceLeft = vxmARCH_VIP_SRAM_SIZE;
                                }
                                if (j == segment_first)
                                {
                                    archModel->opInfoArray[j]->perf.info.flush = flush_and_wait ? 1 : 0;
                                    c = _calc_cost(context,
                                        archModel,
                                        j,
                                        archModel->opInfoArray[j]->xsize,
                                        archModel->opInfoArray[j]->ysize,
                                        archModel->opInfoArray[j]->oz,
                                        SW_TILING_FROM_DDR,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        abBufferSpaceLeft);
                                }
                                else if (j == segment_last)
                                {
                                    c = _calc_cost(
                                        context,
                                        archModel,
                                        j,
                                        archModel->opInfoArray[j]->xsize,
                                        archModel->opInfoArray[j]->ysize,
                                        archModel->opInfoArray[j]->oz,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        SW_TILING_FROM_DDR,
                                        abBufferSpaceLeft);
                                }
                                else
                                {
                                    c = _calc_cost(
                                        context,
                                        archModel,
                                        j,
                                        archModel->opInfoArray[j]->xsize,
                                        archModel->opInfoArray[j]->ysize,
                                        archModel->opInfoArray[j]->oz,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        abBufferSpaceLeft);
                                }
                            }
                            else
                            {
                                if (j == segment_first)
                                {
                                    archModel->opInfoArray[j]->perf.info.flush = flush_and_wait ? 1 : 0;
                                    c = _calc_cost(context,
                                        archModel,
                                        j,
                                        xArray[j],
                                        yArray[j],
                                        zArray[j],
                                        SW_TILING_FROM_DDR,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_VIP_SRAM,
                                        vxmARCH_VIP_SRAM_SIZE);
                                }
                                else if (j == segment_last)
                                {
                                    c = _calc_cost(context,
                                        archModel,
                                        j,
                                        xArray[j],
                                        yArray[j],
                                        zArray[j],
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        SW_TILING_FROM_VIP_SRAM,
                                        vxmARCH_VIP_SRAM_SIZE);
                                }
                                else
                                {
                                    c = _calc_cost(context,
                                        archModel,
                                        j,
                                        xArray[j],
                                        yArray[j],
                                        zArray[j],
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_VIP_SRAM,
                                        vxmARCH_VIP_SRAM_SIZE);
                                }
                            }

                            if (c == MAX_COST)
                            {
                                cost_cycle = MAX_COST;
                                cost_bw = MAX_COST;
                                break;
                            }

                            cost_cycle += c;
                            cost_bw += archModel->opInfoArray[j]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[j]->perf.resultInfo.perfWriteBandWidth;
                        }
                        else
                        {
                            cost_cycle = MAX_COST;
                            cost_bw = MAX_COST;
                            break;
                        }
                    }
                    cur_cost.cycle = cost_cycle;
                    cur_cost.bw = cost_bw;
#if ENABLE_ARCH_MODEL_DUMP
                    vxInfo("bestCost: %llu, bestBW: %llu, cost(%d, %d): %llu, bw: %llu\n", (vx_uint64)bestCost.cycle, (vx_uint64)bestCost.bw, segment_first + 1, segment_last + 1, (vx_uint64)cost_cycle, (vx_uint64)cost_bw);
#endif
                    if (_cur_cost_is_more_better(&bestCost, &cur_cost, CYCLE_WEIGHT, BW_WEIGHT))
                    {
                        bestCost.cycle = cur_cost.cycle;
                        bestCost.bw = cur_cost.bw;
                        for (j = segment_first; j <= segment_last; j++)
                        {
                            if (i == 0)
                            {
                                x_array[j] = archModel->opInfoArray[j]->xsize;
                                y_array[j] = archModel->opInfoArray[j]->ysize;
                                z_array[j] = archModel->opInfoArray[j]->oz;
                            }
                            else
                            {
                                x_array[j] = xArray[j];
                                y_array[j] = yArray[j];
                                z_array[j] = zArray[j];
                            }
                        }
                        *best_cost_swtiling_type = (i == 0) ? ((segment_first == segment_last) ?  -1 : 1) : 0;
                    }
                }

                if ((i <= 1) && (bestCost.cycle < MAX_COST))
                {
                    break;
                }
            }
        }
    }
    cost->cycle = bestCost.cycle;
    cost->bw = bestCost.bw;
exit:
    if (xArray)
        vxFree(xArray);

    if (yArray)
        vxFree(yArray);

    if (zArray)
        vxFree(zArray);
    return;
}

void getUpstreamLayer(
    struct _archModelInfo *archModel,
    vx_uint32 index,
    vx_uint32 id,
    vx_int32 *upstream)
{
    gcmASSERT(upstream != NULL);
    if (id < archModel->opInfoArray[index]->upStreamLayerCount)
    {
        *upstream = archModel->opInfoArray[index]->upStreamLayer[id];
    }
    else
    {
        *upstream = -1;
    }
}

void getDownstreamLayer(
    struct _archModelInfo *archModel,
    vx_uint32 index,
    vx_uint32 id,
    vx_int32 *downstream)
{
    gcmASSERT(downstream != NULL);
    if (id < archModel->opInfoArray[index]->downStreamLayerCount)
    {
        *downstream = archModel->opInfoArray[index]->downStreamLayer[id];
    }
    else
    {
        *downstream = -1;
    }
}


void updateStreamLayer(
    struct _archModelInfo *archModel,
    vx_uint32 count)
{
    vx_uint32 i, j;
    for (i = 0; i < count; i++)
    {
        vx_uint32 k = 0;
        for (j = 0; j < archModel->opInfoArray[i]->opt->parentOpNum; j++)
        {
            if (archModel->opInfoArray[i]->opt->parentOps[j]->target == VXNNE_OPERATION_TARGET_SH
                || archModel->opInfoArray[i]->opt->parentOps[j]->target == VXNNE_OPERATION_TARGET_SW)
            {
                archModel->opInfoArray[i]->upStreamLayer[k++] = -1;
            }
            else
            {
                vx_int32 segIndex = archModel->opInfoArray[i]->opt->parentOps[j]->segIndex;
                if (k > 0)
                {
                    if (archModel->opInfoArray[i]->upStreamLayer[k] < segIndex)
                    {
                        vx_int32 m;
                        for (m = k - 1; m >= 0; m--)
                        {
                            archModel->opInfoArray[i]->upStreamLayer[m + 1] = archModel->opInfoArray[i]->upStreamLayer[m];
                        }
                        archModel->opInfoArray[i]->upStreamLayer[0] = segIndex;
                        k++;
                    }
                    else
                    {
                        archModel->opInfoArray[i]->upStreamLayer[k++] = segIndex;
                    }
                }
                else {
                    archModel->opInfoArray[i]->upStreamLayer[k++] = segIndex;
                }
            }
        }

        if (i == 0)
        {
            archModel->opInfoArray[i]->upStreamLayer[k++] = -1;
        }
        archModel->opInfoArray[i]->upStreamLayerCount = k;
    }

    for (i = 0; i < count; i++)
    {
        vx_uint32 k = 0;
        for (j = 0; j < archModel->opInfoArray[i]->opt->childOpNum; j++)
        {
            if (archModel->opInfoArray[i]->opt->childOps[j]->target == VXNNE_OPERATION_TARGET_SH
                || archModel->opInfoArray[i]->opt->childOps[j]->target == VXNNE_OPERATION_TARGET_SW)
            {
                if (archModel->opInfoArray[i]->opt->childOpNum == 1) {
                    archModel->opInfoArray[i]->downStreamLayer[k++] = -1;
                }
            }
            else
            {
                vx_int32 m;
                vx_int32 segIndex = archModel->opInfoArray[i]->opt->childOps[j]->segIndex;
                vx_bool hasDownLayer = vx_false_e;
                /*check childOperation's upStreamLayer to determin if segIndex is a real downStreamLayer*/
                for (m = 0; m < (vx_int32)archModel->opInfoArray[segIndex]->upStreamLayerCount; m++)
                {
                    if (archModel->opInfoArray[segIndex]->upStreamLayer[m] == (vx_int32)i)
                    {
                        hasDownLayer = vx_true_e;
                        break;
                    }
                }
                if (!hasDownLayer)
                {
                    continue;
                }
                if (k > 0)
                {
                    if (archModel->opInfoArray[i]->downStreamLayer[k] < segIndex)
                    {
                        archModel->opInfoArray[i]->downStreamLayer[k++] = segIndex;
                    }
                    else
                    {
                        for (m = k - 1; m >= 0; m--)
                        {
                            archModel->opInfoArray[i]->downStreamLayer[m + 1] = archModel->opInfoArray[i]->downStreamLayer[m];
                        }
                        archModel->opInfoArray[i]->downStreamLayer[0] = segIndex;
                        k++;
                    }
                }
                else
                {
                    archModel->opInfoArray[i]->downStreamLayer[k++] = segIndex;
                }
            }
        }
        archModel->opInfoArray[i]->downStreamLayerCount = k;
    }
}



static void _split_segment_loop(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[])
{
    struct _archModelCost cost, upcost, downcost;
    struct _archModelCost cur_segment_cost = {MAX_COST, MAX_COST};
    vx_int32 len_segment, cur_pos, temp_pos;
#if ENABLE_ARCH_MODEL_DUMP
    vx_int32 debug = 0;
    if (segment_first == 0 && segment_last == 22)
    {
        debug = 1;
    }
#endif
    for (len_segment = 0; len_segment <= (segment_last - segment_first); len_segment++)
    {
        for (cur_pos = segment_first; cur_pos <= (segment_last - len_segment); cur_pos++)
        {
            getSegmentCostResult(archModel, cur_pos, cur_pos + len_segment, &cost);
            setSplitArrayInfo(archModel, cur_pos, cur_pos + len_segment, cur_pos, 1);
            if (cost.cycle != -1)
            {
                vx_int32 split_pos;
                for (split_pos = cur_pos; split_pos <= (cur_pos + len_segment); split_pos++)
                {
                    if ((archModel->opInfoArray[split_pos]->target != VXNNE_OPERATION_TARGET_TP ||
                        archModel->opInfoArray[split_pos]->opt->operatorType != VXNNE_OPERATOR_FULLYCONNECTED)
                        && split_pos != cur_pos)
                    {
                        getSegmentCostResult(archModel, cur_pos, split_pos - 1, &upcost);
                        if (upcost.cycle == MAX_COST)
                        {
                            break;
                        }
                        getSegmentCostResult(archModel, split_pos, cur_pos + len_segment, &downcost);
                        if (downcost.cycle == MAX_COST || upcost.cycle < 0 || downcost.cycle < 0)
                        {
                            cur_segment_cost.cycle = MAX_COST;
                            cur_segment_cost.bw = MAX_COST;
                        }
                        else
                        {
                            cur_segment_cost.cycle = upcost.cycle + downcost.cycle;
                            cur_segment_cost.bw = upcost.bw + downcost.bw;
                        }
#if ENABLE_ARCH_MODEL_DUMP
                        if (debug)
                        {
                            vxInfo("split_pos: %d, old_cost: %.7f, old_bw: %.7f, cur_cost: %.7f, cur_bw: %.7f\n", split_pos + 1, cost.cycle, cost.bw, cur_segment_cost.cycle, cur_segment_cost.bw);
                        }
#endif
                        if (_cur_cost_is_more_better(&cost, &cur_segment_cost, CYCLE_WEIGHT, BW_WEIGHT))
                        {
#if ENABLE_ARCH_MODEL_DUMP
                            vxInfo("found better cost: split_pos: %d, cur_cost: %.7f, cur_bw: %.7f\n", split_pos + 1, cur_segment_cost.cycle, cur_segment_cost.bw);
#endif
                            cost.cycle = cur_segment_cost.cycle;
                            cost.bw = cur_segment_cost.bw;
                            setBestCostSWTilingTypeInfo(archModel, cur_pos, cur_pos + len_segment, -1);
                            for (temp_pos = cur_pos; temp_pos <= cur_pos + len_segment; temp_pos++)
                            {
                                if (temp_pos < split_pos)
                                {
                                    saveCalculationArgs(
                                            archModel,
                                            cur_pos,
                                            cur_pos + len_segment,
                                            temp_pos,
                                            &archModel->splitInfoArray[cur_pos % MAX_LAYERS_OF_BLOCK]->savedCost[(split_pos - 1 )  % MAX_LAYERS_OF_BLOCK][temp_pos  % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[cur_pos % MAX_LAYERS_OF_BLOCK]->savedSIX[(split_pos - 1)  % MAX_LAYERS_OF_BLOCK][temp_pos  % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[cur_pos % MAX_LAYERS_OF_BLOCK]->savedSIY[(split_pos - 1)  % MAX_LAYERS_OF_BLOCK][temp_pos  % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[cur_pos % MAX_LAYERS_OF_BLOCK]->savedSIZ[(split_pos - 1)  % MAX_LAYERS_OF_BLOCK][temp_pos  % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[cur_pos % MAX_LAYERS_OF_BLOCK]->split_array[(split_pos - 1)  % MAX_LAYERS_OF_BLOCK][temp_pos  % MAX_LAYERS_OF_BLOCK]);
                                }
                                else
                                {
                                    saveCalculationArgs(
                                            archModel,
                                            cur_pos,
                                            cur_pos + len_segment,
                                            temp_pos,
                                            &archModel->splitInfoArray[split_pos % MAX_LAYERS_OF_BLOCK]->savedCost[(cur_pos + len_segment) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[split_pos % MAX_LAYERS_OF_BLOCK]->savedSIX[(cur_pos + len_segment) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[split_pos % MAX_LAYERS_OF_BLOCK]->savedSIY[(cur_pos + len_segment) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[split_pos % MAX_LAYERS_OF_BLOCK]->savedSIZ[(cur_pos + len_segment) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[split_pos % MAX_LAYERS_OF_BLOCK]->split_array[(cur_pos + len_segment) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK]);
                                }
                            }
                        }
                        setSegmentCostResult(archModel, cur_pos, cur_pos + len_segment, &cost);
                    }
                }
            }
            else
            {
                setSegmentCostResult(archModel, cur_pos, cur_pos + len_segment, &cost);
            }
        }
    }

    for (temp_pos = segment_first; temp_pos <= segment_last; temp_pos++)
    {
        x_array[temp_pos] = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSIX[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
        y_array[temp_pos] = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSIY[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
        z_array[temp_pos] = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSIZ[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
        /* to save best cost */
        archModel->opInfoArray[temp_pos]->perf.resultInfo.perfCycleCount = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCost[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK].cycle;
        archModel->opInfoArray[temp_pos]->perf.resultInfo.perfWriteBandWidth = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCost[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK].bw;
        archModel->opInfoArray[temp_pos]->perf.resultInfo.perfReadBandWidth = 0;
    }
}

static void _split_segment(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_int32 segment_first,
    vx_int32 segment_last,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint8 split_array[])
{
    vx_int32 i, j;
    struct _archModelCost cost = {MAX_COST, MAX_COST};
    vx_int32 split_pos = 0;
    vx_bool fullCacheKernelHeadFix = context->nnConfig.unifiedFeature.fullCacheKernelHeadFix ? vx_true_e : vx_false_e;
    vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling ? vx_true_e : vx_false_e;
    vx_int32 first, original_first = segment_first, prev_split = segment_first;
    vx_int32 temp_pos, totalSegments = segment_last -  segment_first + 1;
    vx_uint32 * cur_xArray = NULL;
    vx_uint32 * cur_yArray = NULL;
    vx_uint32 * cur_zArray = NULL;


    gcmASSERT(segment_first >= 0);
    gcmASSERT(segment_last >= 0);

    if (segment_first != segment_last)
    {
        for (split_pos = segment_first + 1; split_pos <= segment_last; split_pos++)
        {
            vx_bool connected = vx_false_e;
            vx_int32 downstream_pos;
            for (downstream_pos = split_pos; downstream_pos <= segment_last; downstream_pos++)
            {
                for (i = 0; i < (vx_int32)archModel->opInfoArray[downstream_pos]->upStreamLayerCount; i++)
                {
                    vx_int32 upStreamLayer;
                    getUpstreamLayer(archModel, downstream_pos, i, &upStreamLayer);
                    if ((upStreamLayer >= segment_first) && (upStreamLayer < split_pos))
                    {
                        connected = vx_true_e;
                        break;
                    }
                }
                if (connected)
                {
                    break;
                }
            }

            if (!connected)
            {
                split_array[split_pos] = 1;
            }
        }
    }

    if (segment_first != segment_last)
    {
        vx_int32 upStreamLayer;
        vx_int32 sramForSWTiling = axiSramOnlySWTiling
            ? vxmARCH_AXI_SRAM_SIZE
            : vxmARCH_VIP_SRAM_SIZE;
        for (split_pos = segment_first + 1; split_pos <= segment_last; split_pos++)
        {
            vx_uint32 size1, size2, outbufNeeded;

            if (archModel->opInfoArray[split_pos]->upStreamLayerCount > 1)
            {
                split_array[split_pos] = 1;
                continue;
            }

            if (archModel->opInfoArray[split_pos]->upStreamLayerCount > 0)
            {
                getUpstreamLayer(archModel, split_pos, 0, &upStreamLayer);
                if (upStreamLayer == -1)
                {
                    split_array[split_pos] = 1;
                    continue;
                }

                {
                    if (archModel->opInfoArray[upStreamLayer]->downStreamLayerCount > 1)
                    {
                        split_array[split_pos] = 1;
                        continue;
                    }
                    else
                    {
                        if (upStreamLayer != (split_pos - 1))
                        {
                            split_array[split_pos] = 1;
                            continue;
                        }
                    }
                }
            }
            else
            {
                split_array[split_pos] = 1;
                continue;
            }

            if ((archModel->opInfoArray[split_pos]->target == VXNNE_OPERATION_TARGET_TP && archModel->opInfoArray[split_pos]->op == VXNNE_OPERATOR_FULLYCONNECTED) ||
                (archModel->opInfoArray[split_pos - 1]->target == VXNNE_OPERATION_TARGET_TP && archModel->opInfoArray[split_pos - 1]->op == VXNNE_OPERATOR_FULLYCONNECTED))
            {
                split_array[split_pos] = 1;
                continue;
            }
            size1 = _kernel_size_in_pixel(archModel, split_pos, archModel->opInfoArray[split_pos]->nnCores, fullCacheKernelHeadFix);
            size2 = _kernel_size_in_pixel(archModel, split_pos - 1, archModel->opInfoArray[split_pos - 1]->nnCores, fullCacheKernelHeadFix);
            outbufNeeded = _outbuf_needed_ex(archModel, split_pos - 1, split_pos, NULL, NULL, NULL);
            if (((size1 + size2) > vxmARCH_VIP_SRAM_SIZE)
                && ((vx_int32)outbufNeeded > sramForSWTiling))
            {
                split_array[split_pos] = 1;
                continue;
            }
        }
    }

#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("\n++init is_split==\n");
    for (i = 0; i <= (segment_last - segment_first); i++)
    {
        vxInfo("layer[%d]: %d\n", i+1, split_array[i]);
    }
    vxInfo("--init is_split==\n");
#endif

    for (i = segment_first; i <= (segment_last + 1); i++)
    {
        if (cur_xArray == NULL)
            cur_xArray = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_uint32) * archModel->totalOpCount);
        else
            memset(cur_xArray, 0, sizeof(vx_uint32) * archModel->totalOpCount);
        if (cur_yArray == NULL)
            cur_yArray = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_uint32) * archModel->totalOpCount);
        else
            memset(cur_yArray, 0, sizeof(vx_uint32) * archModel->totalOpCount);
        if (cur_zArray == NULL)
            cur_zArray = (vx_uint32 *)vxAllocateAndZeroMemory(sizeof(vx_uint32) * archModel->totalOpCount);
        else
            memset(cur_zArray, 0, sizeof(vx_uint32) * archModel->totalOpCount);
        gcmASSERT(cur_xArray != NULL && cur_yArray != NULL && cur_zArray != NULL);

        if (split_array[i] || (i == (segment_last + 1)))
        {
            vx_int32 last = i - 1;
            first = original_first;
            if (last >= first) {

                while (last >= first)
                {
                    while (first <= last)
                    {
                        vx_int32 bestCostSWTilingType = -1;
                        _subimage_segment_cost(context, archModel, first, last, cur_xArray, cur_yArray, cur_zArray, &bestCostSWTilingType, &cost);
#if ENABLE_ARCH_MODEL_DUMP
                        vxInfo("++_subimage_segment_cost(%d, %d)=%.7f, bw: %.7f\n", first + 1, last + 1, cost.cycle, cost.bw);
#endif
                        setSegmentCostResult(archModel, first, last, &cost);
                        setBestCostSWTilingTypeInfo(archModel, first, last, bestCostSWTilingType);
                        for (temp_pos = first; temp_pos <= last; temp_pos++)
                        {
                            struct _archModelCost cur_cost;
                            cur_cost.cycle = archModel->opInfoArray[temp_pos]->perf.resultInfo.perfCycleCount;
                            cur_cost.bw = archModel->opInfoArray[temp_pos]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[temp_pos]->perf.resultInfo.perfWriteBandWidth;
                            saveCalculationArgs(
                                archModel,
                                first,
                                last,
                                temp_pos,
                                &cur_cost,
                                cur_xArray[temp_pos],
                                cur_yArray[temp_pos],
                                cur_zArray[temp_pos],
                                split_array[temp_pos]);
                        }
                        first++;
                    }

                    last--;
                    first = original_first;
                }


            }
            original_first = i;
        }
    }

    if (cur_xArray) vxFree(cur_xArray);
    if (cur_yArray) vxFree(cur_yArray);
    if (cur_zArray) vxFree(cur_zArray);

    for (i = segment_first; i <= (segment_last + 1); i++)
    {
        if ((split_array[i] || (i == (segment_last + 1))) && (i > segment_first))
        {
            {
                vx_int32 segStart, segEnd;
                _split_segment_loop(context, archModel, prev_split, i - 1, x_array, y_array, z_array);
#if ENABLE_ARCH_MODEL_DUMP
                vxInfo("++_split_segment_loop(%d, %d)\n", prev_split + 1, i - 1 + 1);
#endif
                for (temp_pos = prev_split; temp_pos <= (i - 1); temp_pos++)
                {
                    if (archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->split_array[(i - 1) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK]
                        && (archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->bestCostSWTilingType[(i - 1) % MAX_LAYERS_OF_BLOCK] != 1)
                        && (temp_pos != prev_split))
                    {
                        split_array[temp_pos] = archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->split_array[(i - 1) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
#if ENABLE_ARCH_MODEL_DUMP
                        vxInfo("update _split_array[%d]: %d\n", temp_pos + 1, split_array[temp_pos]);
#endif
                    }
                }
                segStart = prev_split;
                for (temp_pos = prev_split + 1; temp_pos <= i; temp_pos++)
                {
                    if (temp_pos == totalSegments || split_array[temp_pos])
                    {
                        segEnd = temp_pos - 1;
                        if (segEnd > segStart)
                        {
                            for (j = segStart; j <= segEnd; j++)
                            {
                                archModel->opInfoArray[j]->swTilingType = archModel->splitInfoArray[segStart % MAX_LAYERS_OF_BLOCK]->bestCostSWTilingType[segEnd % MAX_LAYERS_OF_BLOCK];
                            }
                        }
                        else
                        {
                            archModel->opInfoArray[segEnd]->swTilingType = -1;
                        }
                        segStart = temp_pos;
                    }
                }
            }
            prev_split = i;
        }
    }
#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("++split_end is_split==\n");
    for (i = 0; i <= (segment_last - segment_first); i++)
    {
        vxInfo("layer[%d]: %d\n", i+1, split_array[i]);
    }
    vxInfo("--split_end is_split==\n");
#endif
}

vx_uint32 _seg_buf_needed(
    struct _archModelInfo *archModel,
    vx_uint32 segment_first,
    vx_uint32 segment_last,
    vx_uint32 sixArray[],
    vx_uint32 siyArray[],
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[])
{
    vx_uint32 segBufNeeded = 0;
    if (segment_first == segment_last)
    {
        segBufNeeded = 0;
    }
    else
    {
        vx_bool allTypeABBuf = vx_true_e;
        vx_uint32 i;
        for (i = segment_first; i <= (segment_last - 1); i++)
        {
            if (archModel->opInfoArray[i]->swTilingType == 0) /*0: SUB-IMG, 1: AB*/
            {
                allTypeABBuf = vx_false_e;
            }
        }
        if (allTypeABBuf)
        {
            vx_uint32 abBufSize[2] = {0, 0};
            vx_uint32 abBufPairSize = 0;
            for (i = segment_first; i <= (segment_last - 1); i++)
            {
                abBufSize[i % 2] = _outbuf_needed_ex(archModel, i, i+1, sixArray, siyArray, z_array);
                abBufPairSize = gcmMAX(abBufPairSize, abBufSize[0] + abBufSize[1]);
            }
            segBufNeeded = abBufPairSize;
        }
        else
        {
            segBufNeeded = _outbuf_needed_ex(archModel, segment_first, segment_last, sixArray, siyArray, z_array);
        }
    }
    return segBufNeeded;
}

typedef struct _GIBIO {
    vx_int32 gib_input[MAX_PARENT_CHILD_OP_NUM];
    vx_int32 gib_output[MAX_PARENT_CHILD_OP_NUM];
    vx_uint32 gib_input_count;
    vx_uint32 gib_output_count;
} GIBIO;

typedef struct _GIBObj
{
    vx_uint32 gid;
    vx_uint32 totalBufferNeeded;
    vx_uint32 layerInBufferSize;
    vx_uint32 layerOutBufferSize;
} GIBObj;

vx_bool _gib_io_overlap(
    GIBIO *gibIO,
    vx_uint32 gib,
    struct _archModelInfo *archModel,
    vx_uint32 layer)
{
    vx_uint32 i, j;
    for (i = 0; i < gibIO[gib].gib_input_count; i++)
    {
        for (j = 0; j < archModel->opInfoArray[layer]->upStreamLayerCount; j++)
        {
            vx_int32 upStreamLayer;
            getUpstreamLayer(archModel, layer, j, &upStreamLayer);
            if (upStreamLayer > 0)
            {
                if (gibIO[gib].gib_input[i] == upStreamLayer)
                {
                    return vx_true_e;
                }
            }
        }
    }

    return vx_false_e;
}

void _append_gib_layer(
    GIBIO *gibIO,
    vx_uint32 gib,
    vx_int32 layer,
    vx_bool input)
{
    vx_bool  present = vx_false_e;
    vx_uint32 i;
    vx_uint32 count = input ? gibIO[gib].gib_input_count : gibIO[gib].gib_output_count;
    vx_int32 *target = input ? gibIO[gib].gib_input : gibIO[gib].gib_output;
    for (i = 0; i < count; i++)
    {
        if (target[i] == layer)
        {
            present = vx_true_e;
            break;
        }
    }

    if (!present)
    {
        if (input)
        {
            gibIO[gib].gib_input[gibIO[gib].gib_input_count] = layer;
            gibIO[gib].gib_input_count++;
        }
        else
        {
            gibIO[gib].gib_output[gibIO[gib].gib_output_count] = layer;
            gibIO[gib].gib_output_count++;
        }
    }
}

void _merge_gib_io(
    GIBIO *gibIO,
    vx_uint32 gib,
    struct _archModelInfo *archModel,
    vx_uint32 layer)
{
    vx_uint32 i;
    vx_int32 upStreamLayer;
    for (i = 0; i < archModel->opInfoArray[layer]->upStreamLayerCount; i++)
    {
        getUpstreamLayer(archModel, layer, i, &upStreamLayer);
        if (upStreamLayer > 0)
        {
            _append_gib_layer(gibIO, gib, upStreamLayer, vx_true_e);
        }
    }
}

vx_uint32 _create_gib(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_uint32 count,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint8 split_array[],
    GIBIO *gib_io,
    GIBObj *gib_obj)
{
    vx_int32 i, j, end_index;
    vx_uint32 gib, gib_last = 0;
    vx_bool *layerChecked = NULL;
    vx_uint32 segmentBufferNeeded, id;
    vx_uint32 *sixArray = NULL, *siyArray = NULL;
    gceSTATUS status = gcvSTATUS_OK;
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&sixArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _create_gib(1) return out-of-memory\n");
        return 0;
    }
    memset(sixArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&siyArray
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _create_gib(2) return out-of-memory\n");
        goto OnError;
    }
    memset(siyArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_bool) * archModel->totalOpCount,
        (gctPOINTER *)&layerChecked
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _create_gib(3) return out-of-memory\n");
        goto OnError;
    }
    memset(layerChecked, 0, gcmSIZEOF(vx_bool) * archModel->totalOpCount);

    for (i = 0; i < (vx_int32)count; i++)
    {
        sixArray[i] = (vx_uint32)(ceilf((vx_float32)(x_array[i] - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
        siyArray[i] = (vx_uint32)(ceilf((vx_float32)(y_array[i] - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
        layerChecked[i] = vx_false_e;
    }

    id = 0;
    end_index = count - 1;
    if (archModel->opInfoArray[0]->target == VXNNE_OPERATION_TARGET_NN || archModel->opInfoArray[0]->target  == VXNNE_OPERATION_TARGET_TP)
    {
        split_array[0] = 1;
    }
    for (i = count - 1; i >= 0; i--)
    {
        if (split_array[i]) {
            if (i == end_index)
            {
                segmentBufferNeeded = 0;
            }
            else
            {
                segmentBufferNeeded = _seg_buf_needed(archModel, i, end_index, sixArray, siyArray, x_array, y_array, z_array);
            }

            for (j = i; j <= end_index; j++)
            {
                gib_obj[j].gid = id;
                gib_obj[j].totalBufferNeeded = segmentBufferNeeded;
                gib_obj[j].layerOutBufferSize = archModel->opInfoArray[j]->bfy * archModel->opInfoArray[j]->pix * archModel->opInfoArray[j]->piy * archModel->opInfoArray[j]->bfz * archModel->opInfoArray[j]->oz;

                if (j == i)
                {
                    gib_obj[j].layerInBufferSize = 0;
                }
                else
                {
                    gib_obj[j].layerInBufferSize = gib_obj[j - 1].layerOutBufferSize;
                }
            }
            id++;
            end_index = i - 1;
        }
    }


    gib = 0;
    gib_last = 0;
    if (id == 1)
    {
        if (split_array[0] && !layerChecked[0])
        {
            gib_io[0].gib_input_count = archModel->opInfoArray[0]->upStreamLayerCount;
            gib_io[0].gib_output_count = archModel->opInfoArray[0]->downStreamLayerCount;
            gib_io[0].gib_output[0] = -1;
            gib_io[0].gib_input[0] = -1;
            layerChecked[0] = vx_true_e;
        }
        gib_last = gib;
        if (sixArray) vxFree(sixArray);
        if (siyArray) vxFree(siyArray);
        if (layerChecked) vxFree(layerChecked);
        return gib_last;
    }
    for (i = count - 1; i >= 1; i--)
    {
        if (split_array[i] && !layerChecked[i])
        {
            gib_io[gib].gib_input_count = archModel->opInfoArray[i]->upStreamLayerCount;
            if (gib_io[gib].gib_input_count != 0)
            {
                gib_last = gib;
                for (j = 0; j < (vx_int32)archModel->opInfoArray[i]->upStreamLayerCount; j++)
                {
                    vx_int32 upStreamLayer;
                    getUpstreamLayer(archModel, i, j, &upStreamLayer);
                    gib_io[gib].gib_input[j] = upStreamLayer;
                }
                gib_io[gib].gib_output_count = 1;
                gib_io[gib].gib_output[0] = i;

                for (j = (i - 1); j >= 0; j--)
                {
                    if (split_array[j])
                    {
                        if (archModel->opInfoArray[j]->target != VXNNE_OPERATION_TARGET_SH &&
                           archModel->opInfoArray[j]->target != VXNNE_OPERATION_TARGET_SW)
                        {
                            if (_gib_io_overlap(gib_io, gib, archModel, j))
                            {
                                _merge_gib_io(gib_io, gib, archModel, j);
                                _append_gib_layer(gib_io, gib, j, vx_false_e);
                                layerChecked[j] = vx_true_e;
                            }
                        }
                    }
                }
                gib++;
            }
        }
    }
    if (sixArray) vxFree(sixArray);
    if (siyArray) vxFree(siyArray);
    if (layerChecked) vxFree(layerChecked);
    return gib_last;

OnError:
    if (sixArray)
        vxFree(sixArray);

    if (siyArray)
        vxFree(siyArray);

    return 0;
}

void _merge_sub_graph(
    vx_context context,
    struct _archModelInfo *archModel,
    vx_uint32 count,
    vx_uint32 x_array[],
    vx_uint32 y_array[],
    vx_uint32 z_array[],
    vx_uint8 split_array[],
    GIBIO *gib_io,
    GIBObj *gib_obj,
    vx_uint32 gib_last)
{
    vx_uint32 gib, j, k;
    vx_bool *gib_input_checked = NULL;
    vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling;
    gceSTATUS status = gcvSTATUS_OK;
    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_bool) * archModel->totalOpCount,
        (gctPOINTER *)&gib_input_checked
        );
    if (gcmIS_ERROR(status))
    {
        vxInfo("ERROR: _merge_sub_graph() return out-of-memory\n");
        return;
    }
    memset(gib_input_checked, 0, gcmSIZEOF(vx_bool) * archModel->totalOpCount);

    for (gib = 0; gib <= gib_last; gib++)
    {
        vx_bool anyLayerExternal = vx_false_e;
        vx_uint32 outputSubimageSegmentCount = 0, inputSubimageSegmentCount = 0, outputABSegmentCount = 0, inputABSegmentCount = 0;
        struct _archModelCost baseCost = {0,0}, curCost = {0,0};

        for (j = 0; j < gib_io[gib].gib_input_count; j++)
        {
            vx_int32 layer = gib_io[gib].gib_input[j];
            if (layer == -1)
            {
                anyLayerExternal = vx_true_e;
                break;
            }
        }
        if (anyLayerExternal)
        {
            continue;
        }

        for (j = 0; j < gib_io[gib].gib_output_count; j++)
        {
            vx_uint32 layer = gib_io[gib].gib_output[j];
            if (archModel->opInfoArray[layer]->swTilingType == 0) /*0: SUB-IMG, 1: AB*/
            {
                outputSubimageSegmentCount++;
            }
            else
            {
                outputABSegmentCount++;
            }

            baseCost.cycle += archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount;
            baseCost.bw += archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth;
            baseCost.bw += archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth;
#if ENABLE_ARCH_MODEL_DUMP
            vxInfo("gib_io[%d, %d].output, cost.cycle(%d): %llu, cost.bw(%d): %llu\n", gib+1, j + 1, layer + 1, (vx_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount + 0.5f), layer + 1, (vx_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth + 0.5f));
#endif
        }

        for (j = 0; j < gib_io[gib].gib_input_count; j++)
        {
            vx_uint32 layer = gib_io[gib].gib_input[j];
            if (archModel->opInfoArray[layer]->swTilingType == 0) /*0: SUB-IMG, 1: AB*/
            {
                inputSubimageSegmentCount++;
            }
            else
            {
                inputABSegmentCount++;
            }
            baseCost.cycle += archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount;
            baseCost.bw += archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth;
            baseCost.bw += archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth;
#if ENABLE_ARCH_MODEL_DUMP
            vxInfo("gib_io[%d, %d].input, cost.cycle(%d): %llu, cost.bw(%d): %llu\n", gib+1, j + 1, layer + 1, (vx_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount + 0.5f), layer + 1, (vx_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth + 0.5f));
#endif
        }

        if (inputSubimageSegmentCount < 1 && outputSubimageSegmentCount < 1)
        {
            vx_uint32 gibBufferSize = 0;
            vx_uint32 bufferNeeded = 0;
            vx_int32 sramSize, abBufferSpaceLeft;
            for (j = 0; j < gib_io[gib].gib_input_count; j++)
            {
                gib_input_checked[j] = vx_false_e;
            }
            for (j = 0; j < gib_io[gib].gib_input_count; j++)
            {
                vx_int32 largest_GraphTotalBufferNeeded = -1;
                vx_uint32 largest_input = 0;
                for (k = 0; k < gib_io[gib].gib_input_count; k++)
                {
                    if (!gib_input_checked[k]
                    && largest_GraphTotalBufferNeeded < (vx_int32)gib_obj[gib_io[gib].gib_input[k]].totalBufferNeeded)
                    {
                        largest_GraphTotalBufferNeeded = (vx_int32)gib_obj[gib_io[gib].gib_input[k]].totalBufferNeeded;
                        largest_input = k;
                    }
                }
                gib_input_checked[largest_input] = vx_true_e;
                gibBufferSize += gib_obj[gib_io[gib].gib_input[largest_input]].layerOutBufferSize;
                bufferNeeded = gcmMAX(bufferNeeded,
                    gcmMAX(gib_obj[gib_io[gib].gib_input[largest_input]].totalBufferNeeded,
                    gib_obj[gib_io[gib].gib_input[largest_input]].layerInBufferSize + gibBufferSize));
            }

            for (j = 0; j < gib_io[gib].gib_output_count; j++)
            {
                if (archModel->opInfoArray[gib_io[gib].gib_output[j]]->swTilingType == -1)
                {
                    bufferNeeded = gcmMAX(bufferNeeded, gibBufferSize);
                }
                else
                {
                    bufferNeeded = gcmMAX(bufferNeeded,
                        gcmMAX(gib_obj[gib_io[gib].gib_output[j]].totalBufferNeeded,
                        gib_obj[gib_io[gib].gib_output[j]].layerOutBufferSize + gibBufferSize));
                }
            }

            sramSize = axiSramOnlySWTiling ? vxmARCH_AXI_SRAM_SIZE
                                           : vxmARCH_VIP_SRAM_SIZE;

            if ((vx_int32)bufferNeeded < sramSize)
            {
                vx_uint8 kbuf = SW_TILING_FROM_DDR, sbuf, dbuf;
                vx_bool allOutputSegmentsAreShort = vx_true_e;
                vx_uint32 dst_graph_id = gib_obj[gib_io[gib].gib_input[0]].gid;

                abBufferSpaceLeft = axiSramOnlySWTiling ? vxmARCH_VIP_SRAM_SIZE
                                                               : (vx_int32)(sramSize - bufferNeeded);
                for (j = 0; j < gib_io[gib].gib_output_count; j++)
                {
                    vx_uint32 my_gib_output_layer = gib_io[gib].gib_output[j];
                    vx_bool my_gib_output_layer_is_last_in_segment = vx_false_e;
                    vx_uint8 my_sbuf, my_dbuf, my_kbuf = SW_TILING_FROM_DDR;
                    vx_arch_perf_s perf;

                    if (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 0 ||
                        (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 1 && archModel->opInfoArray[my_gib_output_layer]->downStreamLayer[0] == -1)
                        )
                    {
                        allOutputSegmentsAreShort = vx_false_e;
                        my_gib_output_layer_is_last_in_segment = vx_true_e;
                    }
                    else
                    {
                        vx_int32 downStreamLayer;
                        getDownstreamLayer(archModel, my_gib_output_layer, 0, &downStreamLayer);
                        if (downStreamLayer > 0 && split_array[downStreamLayer])
                        {
                            allOutputSegmentsAreShort = vx_false_e;
                            my_gib_output_layer_is_last_in_segment = vx_true_e;
                        }
                    }


                    my_sbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    my_dbuf = my_gib_output_layer_is_last_in_segment ? SW_TILING_FROM_DDR
                                                                     : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);

                    archModel->opInfoArray[my_gib_output_layer]->perf.info.flush = 0;
                    /*back up original profile data, and use archModel->opInfoArray[x]->perf to save profile data with new configuration */
                    memcpy(&perf, &archModel->opInfoArray[my_gib_output_layer]->perf, sizeof(vx_arch_perf_s));
                    _calc_cost(context,
                            archModel,
                            my_gib_output_layer,
                            x_array[my_gib_output_layer],
                            y_array[my_gib_output_layer],
                            z_array[my_gib_output_layer],
                            my_sbuf,
                            my_dbuf,
                            my_kbuf,
                            abBufferSpaceLeft);

                    curCost.cycle += archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfCycleCount;
                    curCost.bw += archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfReadBandWidth;
                    curCost.bw += archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfWriteBandWidth;
                    /*restore original profile data for sub-graph merging*/
                    memcpy(&archModel->opInfoArray[my_gib_output_layer]->perf, &perf, sizeof(vx_arch_perf_s));
                }

                for (j = 0; j < gib_io[gib].gib_input_count; j++)
                {
                    vx_uint32 my_gib_input_layer = gib_io[gib].gib_input[j];
                    vx_uint8 my_sbuf, my_dbuf, my_kbuf = SW_TILING_FROM_DDR;
                    vx_arch_perf_s perf;

                    my_sbuf = split_array[my_gib_input_layer] ? SW_TILING_FROM_DDR
                                                                    : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);
                    my_dbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    archModel->opInfoArray[my_gib_input_layer]->perf.info.flush = allOutputSegmentsAreShort;
                    /*back up original profile data, and use archModel->opInfoArray[x]->perf to save profile data with new configuration */
                    memcpy(&perf, &archModel->opInfoArray[my_gib_input_layer]->perf, sizeof(vx_arch_perf_s));
                    _calc_cost(context,
                            archModel,
                            my_gib_input_layer,
                            x_array[my_gib_input_layer],
                            y_array[my_gib_input_layer],
                            z_array[my_gib_input_layer],
                            my_sbuf,
                            my_dbuf,
                            my_kbuf,
                            abBufferSpaceLeft);
                    curCost.cycle += archModel->opInfoArray[my_gib_input_layer]->perf.resultInfo.perfCycleCount;
                    curCost.bw += archModel->opInfoArray[my_gib_input_layer]->perf.resultInfo.perfReadBandWidth;
                    curCost.bw += archModel->opInfoArray[my_gib_input_layer]->perf.resultInfo.perfWriteBandWidth;
                    /*restore original profile data for sub-graph merging*/
                    memcpy(&archModel->opInfoArray[my_gib_input_layer]->perf, &perf, sizeof(vx_arch_perf_s));
                }
#if ENABLE_ARCH_MODEL_DUMP
                vxInfo("gib: %d, base_cycle: %llu, base_bw: %llu\n", gib+1, (vx_uint64)(baseCost.cycle + 0.5), (vx_uint64)(baseCost.bw + 0.5));
                vxInfo("gib: %d, cur_cycle: %llu, cur_bw: %llu\n", gib+1, (vx_uint64)(curCost.cycle + 0.5), (vx_uint64)(curCost.bw + 0.5));
#endif
                if (_cur_cost_is_more_better(&baseCost, &curCost, 1, 0))
                {
                    kbuf = SW_TILING_FROM_DDR;
                    abBufferSpaceLeft = axiSramOnlySWTiling ? vxmARCH_VIP_SRAM_SIZE
                                                                   : (sramSize - bufferNeeded);
                    allOutputSegmentsAreShort = vx_true_e;
                    dst_graph_id = gib_obj[gib_io[gib].gib_input[0]].gid;

                    for (j = 0; j < gib_io[gib].gib_output_count; j++)
                    {
                        vx_uint32 src_graph_id;
                        vx_uint32 my_gib_output_layer = gib_io[gib].gib_output[j];
                        vx_bool my_gib_output_layer_is_last_in_segment = vx_false_e;
                        if (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 0 ||
                            (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 1 && archModel->opInfoArray[my_gib_output_layer]->downStreamLayer[0] == -1)
                           )
                        {
                            allOutputSegmentsAreShort = vx_false_e;
                            my_gib_output_layer_is_last_in_segment = vx_true_e;
                        }
                        else
                        {
                            vx_int32 downStreamLayer;
                            getDownstreamLayer(archModel, my_gib_output_layer, 0, &downStreamLayer);
                            if (downStreamLayer > 0 && split_array[downStreamLayer])
                            {
                                allOutputSegmentsAreShort = vx_false_e;
                                my_gib_output_layer_is_last_in_segment = vx_true_e;
                            }
                        }

                        src_graph_id = gib_obj[my_gib_output_layer].gid;
                        for (k = 0; k < count; k++)
                        {
                            if (src_graph_id == gib_obj[k].gid)
                            {
                                gib_obj[k].gid = dst_graph_id;
                                gib_obj[k].totalBufferNeeded = bufferNeeded;
                            }
                        }

                        {
                            sbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                            dbuf = my_gib_output_layer_is_last_in_segment ? SW_TILING_FROM_DDR :
                                (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);
                            archModel->opInfoArray[my_gib_output_layer]->sbuf = sbuf;
                            archModel->opInfoArray[my_gib_output_layer]->dbuf = dbuf;
                            archModel->opInfoArray[my_gib_output_layer]->kbuf = kbuf;
                            archModel->opInfoArray[my_gib_output_layer]->perf.info.flush = 0;
                            _calc_cost(context,
                                archModel,
                                my_gib_output_layer,
                                x_array[my_gib_output_layer],
                                y_array[my_gib_output_layer],
                                z_array[my_gib_output_layer],
                                sbuf,
                                dbuf,
                                kbuf,
                                abBufferSpaceLeft);
                            split_array[my_gib_output_layer] = vx_false_e;
                            archModel->opInfoArray[my_gib_output_layer]->swTilingType = 1;
#if ENABLE_ARCH_MODEL_DUMP
                            vxInfo("== merged: %d\n", my_gib_output_layer + 1);
#endif
                        }
                    }

                    for (j = 0; j < gib_io[gib].gib_input_count; j++)
                    {
                        vx_uint32 src_graph_id = gib_obj[gib_io[gib].gib_input[j]].gid;
                        for (k = 0; k < count; k++)
                        {
                            if (src_graph_id == gib_obj[k].gid)
                            {
                                gib_obj[k].gid = dst_graph_id;
                                gib_obj[k].totalBufferNeeded = bufferNeeded;
                            }
                        }
                        {
                            if (split_array[gib_io[gib].gib_input[j]])
                            {
                                sbuf = SW_TILING_FROM_DDR;
                            }
                            else if (axiSramOnlySWTiling)
                            {
                                sbuf = SW_TILING_FROM_AXI_SRAM;
                            }
                            else
                            {
                                sbuf = SW_TILING_FROM_VIP_SRAM;
                            }
                            dbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;


                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->sbuf = sbuf;
                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->dbuf = dbuf;
                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->kbuf = kbuf;
                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->perf.info.flush = allOutputSegmentsAreShort ? 1 : 0;
                            _calc_cost(context,
                                archModel,
                                gib_io[gib].gib_input[j],
                                x_array[gib_io[gib].gib_input[j]],
                                y_array[gib_io[gib].gib_input[j]],
                                z_array[gib_io[gib].gib_input[j]],
                                sbuf,
                                dbuf,
                                kbuf,
                                abBufferSpaceLeft);
                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->swTilingType = 1;
#if ENABLE_ARCH_MODEL_DUMP
                            vxInfo("== merged: %d\n", gib_io[gib].gib_input[j] + 1);
#endif
                        }
                    }
                }
            }
        }
    }

#if ENABLE_ARCH_MODEL_DUMP
    vxInfo("++split_status after merged==\n");
    for (j = 0; j < count; j++)
    {
        vxInfo("layer[%d]: %d\n", j+1, split_array[j]);
    }
    vxInfo("--split_status after merged==\n");
#endif
    if (gib_input_checked) vxFree(gib_input_checked);
}

VX_INTERNAL_API vx_status vxoGraph_PredictPerf(vx_graph graph)
{
    vx_uint32 i, j, count=0;
    struct _archModelInfo * archModel;
    struct _archModelOpInfo ** opInfo;
    vx_context context = vxoContext_GetFromReference(&graph->base);
    vx_uint32 *xArray = NULL, *yArray = NULL, *zArray = NULL;
    vx_uint8 *sArray = NULL;
    vx_bool hasVXC = vx_false_e;
    gceSTATUS status = gcvSTATUS_OK;
    vx_status vxStatus = VX_SUCCESS;

    if (!graph->layer) return status;

    archModel = initArchModelInfo(graph->layer->base.num_operations);
    if (archModel == NULL)
    {
        vxStatus = VX_FAILURE;
        goto error;
    }
    opInfo = archModel->opInfoArray;
    initSegmentCostResult(archModel);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&xArray
        );
    if (gcmIS_ERROR(status))
    {
        vxStatus = VX_FAILURE;
        vxInfo("ERROR: vxoGraph_PredictPerf(1) return out-of-memory\n");
        goto error;
    }
    memset(xArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&yArray
        );
    if (gcmIS_ERROR(status))
    {
        vxStatus = VX_FAILURE;
        vxInfo("ERROR: vxoGraph_PredictPerf(2) return out-of-memory\n");
        goto error;
    }
    memset(yArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint32) * archModel->totalOpCount,
        (gctPOINTER *)&zArray
        );
    if (gcmIS_ERROR(status))
    {
        vxStatus = VX_FAILURE;
        vxInfo("ERROR: vxoGraph_PredictPerf(3) return out-of-memory\n");
        goto error;
    }
    memset(zArray, 0, gcmSIZEOF(vx_uint32) * archModel->totalOpCount);

    status = gcoOS_Allocate(
        gcvNULL,
        gcmSIZEOF(vx_uint8) * archModel->totalOpCount,
        (gctPOINTER *)&sArray
        );
    if (gcmIS_ERROR(status))
    {
        vxStatus = VX_FAILURE;
        vxInfo("ERROR: vxoGraph_PredictPerf(4) return out-of-memory\n");
        goto error;
    }
    memset(sArray, 0, gcmSIZEOF(vx_uint8) * archModel->totalOpCount);

    for (i = 0; i < graph->nodeCount; i++)
    {
        vx_node node = graph->nodeTable[graph->allNodeIndexTable[i]];
        if (node->layer == NULL)
        {
            hasVXC = vx_true_e;
            continue; /* skipped node without layer/operation */
        }

        for (j = 0; j < node->layer->num_operations; j++)
        {
            vxnne_operation operation = node->layer->operations[j];
            vx_weights_biases_parameter wb;
            opInfo[count]->swTilingType = -1;
            if ((operation->operatorType == VXNNE_OPERATOR_CONVOLUTION || operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV) &&
                operation->target == VXNNE_OPERATION_TARGET_NN)
            {
                vx_uint32 outXSize, outYSize;
                vxnne_convolution_relu_pooling_operation convOp = (vxnne_convolution_relu_pooling_operation)operation;

                wb = convOp->weights_biases;
                gcmASSERT(wb != NULL);
                gcmASSERT(convOp->orig_inputs != NULL);
                gcmASSERT(convOp->inputs != NULL);
                gcmASSERT(convOp->outputs != NULL);

                ComputeInputSize(
                     TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 0),
                     wb->weights_sizes[0],
                     WB_PAD_LEFT(wb),
                     WB_PAD_RIGHT(wb),
                     WB_POOLING_SIZE_X(wb),
                     WB_POOLING_STRIDE(wb),
                     &outXSize,
                     1);

                ComputeInputSize(
                     TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 1),
                     wb->weights_sizes[1],
                     WB_PAD_TOP(wb),
                     WB_PAD_BOTTOM(wb),
                     WB_POOLING_SIZE_Y(wb),
                     WB_POOLING_STRIDE(wb),
                     &outYSize,
                     1);

                opInfo[count]->node    = node;
                opInfo[count]->opt     = operation;
                opInfo[count]->op      = operation->operatorType;
                opInfo[count]->target  = operation->target;
                opInfo[count]->psize   = gcmMAX(WB_POOLING_SIZE_X(wb), 1);
                opInfo[count]->pstride = WB_POOLING_SIZE_X(wb) ? 2 : 1;
                opInfo[count]->xpad    = convOp->pad_x_left;
                opInfo[count]->ypad    = convOp->pad_y_top;
                opInfo[count]->inputDataFormat = TENSOR_DATA_TYPE(convOp->inputs);
                opInfo[count]->inputDataSize   = TENSOR_DATA_SIZE(convOp->inputs) * 8;
                opInfo[count]->outputDataFormat = TENSOR_DATA_TYPE(convOp->outputs);
                opInfo[count]->outputDataSize   = TENSOR_DATA_SIZE(convOp->outputs) * 8;
                opInfo[count]->weight_bias = wb;
                opInfo[count]->kx = wb->weights_sizes[0];
                opInfo[count]->ky = wb->weights_sizes[1];
                opInfo[count]->kz = (operation->operatorType == VXNNE_OPERATOR_DEPTH_WISE_CONV && wb->wb_base->hw_depth_wise) ? wb->weights_sizes[3] : wb->weights_sizes[2];
                opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 2);
                opInfo[count]->siz = opInfo[count]->oz;
                opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 0);
                opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 1);
                opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(convOp->orig_inputs, 2);
                opInfo[count]->origx = outXSize;
                opInfo[count]->origy = outYSize;
                opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 0);
                opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(convOp->outputs, 1);
                opInfo[count]->p3 = convOp->pool_size_x == 3 ? 1 : 0;
                opInfo[count]->xsize = opInfo[count]->origx;
                opInfo[count]->ysize = opInfo[count]->origy;
                opInfo[count]->pix = (vx_uint32)ceilf((vx_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
                opInfo[count]->piy = (vx_uint32)ceilf((vx_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);
                xArray[count] = opInfo[count]->pix;
                yArray[count] = opInfo[count]->piy;
                zArray[count] = opInfo[count]->oz;
                /* init buf */
                opInfo[count]->sbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->dbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->kbuf = SW_TILING_FROM_VIP_SRAM;
                opInfo[count]->fcmd = vx_true_e;
                opInfo[count]->bfy = 1;
                opInfo[count]->bfz = 1;


                opInfo[count]->downStreamLayerCount = operation->childOpNum;
                opInfo[count]->upStreamLayerCount = operation->parentOpNum;
                operation->segIndex = count;

                if (opInfo[count]->inputDataFormat == VX_TYPE_INT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountInt16;
                else if (opInfo[count]->inputDataFormat == VX_TYPE_FLOAT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountFloat16;
                else
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCount;

                count++;
            }
            else if (operation->target == VXNNE_OPERATION_TARGET_TP &&
                (operation->operatorType == VXNNE_OPERATOR_RESHUFFLE ||
                operation->operatorType == VXNNE_OPERATOR_DILATION_RESHUFFLE ||
                operation->operatorType == VXNNE_OPERATOR_DILATION_UPSAMPLE ||
                operation->operatorType == VXNNE_OPERATOR_DILATION_UPSAMPLE2 ||
                operation->operatorType == VXNNE_OPERATOR_POOLING ||
                operation->operatorType == VXNNE_OPERATOR_ROIPOOL ||
                operation->operatorType == VXNNE_OPERATOR_NORMALIZATION ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_ADD ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_TRANS ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_COPY ||
                operation->operatorType == VXNNE_OPERATOR_REORG ||
                operation->operatorType == VXNNE_OPERATOR_LSTM_RESHUFFLE_INPUT ||
                operation->operatorType == VXNNE_OPERATOR_UPSAMPLE ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_PAD ||
                operation->operatorType == VXNNE_OPERATOR_REORG2 ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_REVERSE ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_SQUEEZE ||
                operation->operatorType == VXNNE_OPERATOR_TENSOR_STRIDE_SLICE ||
                operation->operatorType == VXNNE_OPERATOR_CONCATINDEFINITE ||
                operation->operatorType == VXNNE_OPERATOR_SVDF_MAP ||
                operation->operatorType == VXNNE_OPERATOR_INTERLEAVE ||
                operation->operatorType == VXNNE_OPERATOR_ACTIVATION))
            {
                vxnne_tp_operation tpOp  = (vxnne_tp_operation)operation;

                gcmASSERT(tpOp->input != NULL);
                gcmASSERT(tpOp->output != NULL);
                if (operation->operatorType == VXNNE_OPERATOR_RESHUFFLE)
                {
                    wb = tpOp->weights_biases;
                    gcmASSERT(wb != NULL);
                    gcmASSERT(tpOp->weights_biases != NULL);
                    opInfo[count]->kz = tpOp->weights_biases->weights_sizes[2];
                    opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 2);
                    opInfo[count]->stridex = WB_STRIDE_X(wb);
                    opInfo[count]->stridey = WB_STRIDE_Y(wb);
                }
                else
                {
                    opInfo[count]->kz = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 2);
                    opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 2);
                }

                opInfo[count]->origx = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 0);
                opInfo[count]->origy = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 1);
                opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 0);
                opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(tpOp->output, 1);
                opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 0);
                opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 1);
                opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(tpOp->input, 2);
                opInfo[count]->node    = node;
                opInfo[count]->opt     = operation;
                opInfo[count]->op      = operation->operatorType;
                opInfo[count]->target  = operation->target;
                opInfo[count]->psize   = operation->parameter.pool_size_x;
                opInfo[count]->pstride = gcmMAX(1, operation->parameter.pool_stride);
                opInfo[count]->xpad    = tpOp->base.parameter.pad_x_left;
                opInfo[count]->ypad    = tpOp->base.parameter.pad_y_top;
                opInfo[count]->inputDataFormat = TENSOR_DATA_TYPE(tpOp->input);
                opInfo[count]->inputDataSize = TENSOR_DATA_SIZE(tpOp->input) * 8;
                opInfo[count]->outputDataFormat = TENSOR_DATA_TYPE(tpOp->output);
                opInfo[count]->outputDataSize   = TENSOR_DATA_SIZE(tpOp->output) * 8;
                opInfo[count]->weight_bias = VX_NULL;
                opInfo[count]->kx = 1;
                opInfo[count]->ky = 1;
                opInfo[count]->p3 = (operation->parameter.pool_size_x == 3) ? 1 : 0;
                if (operation->operatorType == VXNNE_OPERATOR_POOLING)
                {
                    opInfo[count]->xsize = opInfo[count]->inx;
                    opInfo[count]->ysize = opInfo[count]->iny;
                }
                else
                {
                    opInfo[count]->xsize = opInfo[count]->origoutx;
                    opInfo[count]->ysize = opInfo[count]->origouty;
                }
                opInfo[count]->pix = (vx_uint32)ceilf((vx_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
                opInfo[count]->piy = (vx_uint32)ceilf((vx_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);
                opInfo[count]->sbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->dbuf = SW_TILING_FROM_AXI_SRAM;
                opInfo[count]->kbuf = SW_TILING_FROM_VIP_SRAM;
                opInfo[count]->fcmd = vx_false_e;

                xArray[count] = opInfo[count]->pix;
                yArray[count] = opInfo[count]->piy;
                zArray[count] = opInfo[count]->oz;
                opInfo[count]->siz = opInfo[count]->oz;
                opInfo[count]->bfy = 1;
                opInfo[count]->bfz = 1;

                opInfo[count]->downStreamLayerCount = operation->childOpNum;
                opInfo[count]->upStreamLayerCount = operation->parentOpNum;
                if (opInfo[count]->inputDataFormat == VX_TYPE_INT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountInt16;
                else if (opInfo[count]->inputDataFormat == VX_TYPE_FLOAT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountFloat16;
                else
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCount;

                operation->segIndex = count;
                count++;
            }
            else if (operation->operatorType == VXNNE_OPERATOR_FULLYCONNECTED)
            {
                if (operation->target == VXNNE_OPERATION_TARGET_TP)
                {
                    vxnne_tp_operation fcOp = (vxnne_tp_operation)operation;
                    vx_uint32 inDims = fcOp->input->dimCount;
                    vx_uint32 outDims = fcOp->output->dimCount;

                    wb = fcOp->weights_biases;
                    opInfo[count]->inputDataSize = TENSOR_DATA_SIZE(fcOp->input) * 8;
                    opInfo[count]->inputDataFormat = TENSOR_DATA_TYPE(fcOp->input);
                    opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0);
                    opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1);
                    opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 2);
                    opInfo[count]->origx = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0);
                    opInfo[count]->origy = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1);

                    opInfo[count]->outputDataSize = TENSOR_DATA_SIZE(fcOp->output) * 8;
                    opInfo[count]->outputDataFormat = TENSOR_DATA_TYPE(fcOp->output);
                    opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(fcOp->output, 0);
                    opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(fcOp->output, 1);
                    opInfo[count]->oz = TENSOR_VIEW_SIZE_INDEX(fcOp->output, 2);

                    opInfo[count]->psize   = operation->parameter.pool_size_x;
                    opInfo[count]->pstride = operation->parameter.pool_stride;

                    {
                        /*convert TP FC input/output info for arch model analysis when dims<=2 */
                        if ((inDims == 2) || (inDims == 1))
                        {
                            opInfo[count]->inx = 1;
                            opInfo[count]->iny = 1;
                            opInfo[count]->origx = 1;
                            opInfo[count]->origy = 1;
                            opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(fcOp->input, 0) * TENSOR_VIEW_SIZE_INDEX(fcOp->input, 1) * TENSOR_VIEW_SIZE_INDEX(fcOp->input, 2);
                        }

                        if ((outDims == 2) || (outDims == 1))
                        {
                            opInfo[count]->origoutx = 1;
                            opInfo[count]->origouty = 1;
                            opInfo[count]->oz = wb->weights_sizes[3];
                        }
                    }

                }
                else if (operation->target == VXNNE_OPERATION_TARGET_NN)
                {
                    vxnne_convolution_relu_pooling_operation fcOp = (vxnne_convolution_relu_pooling_operation)operation;
                    wb = fcOp->weights_biases;
                    opInfo[count]->inputDataSize = TENSOR_DATA_SIZE(fcOp->inputs) * 8;
                    opInfo[count]->inputDataFormat = TENSOR_DATA_TYPE(fcOp->inputs);
                    opInfo[count]->outputDataSize = TENSOR_DATA_SIZE(fcOp->outputs) * 8;
                    opInfo[count]->outputDataFormat = TENSOR_DATA_TYPE(fcOp->outputs);
                    opInfo[count]->inx = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 0);
                    opInfo[count]->iny = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 1);
                    opInfo[count]->inz = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 2);
                    opInfo[count]->origx = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 0);
                    opInfo[count]->origy = TENSOR_VIEW_SIZE_INDEX(fcOp->inputs, 1);
                    opInfo[count]->origoutx = TENSOR_VIEW_SIZE_INDEX(fcOp->outputs, 0);
                    opInfo[count]->origouty = TENSOR_VIEW_SIZE_INDEX(fcOp->outputs, 1);
                    opInfo[count]->oz = wb->weights_sizes[3];
                    opInfo[count]->psize   = gcmMAX(WB_POOLING_SIZE_X(wb), 1);
                    opInfo[count]->pstride = WB_POOLING_SIZE_X(wb) ? 2 : 1;
                }
                else
                {
                    continue;
                }

                opInfo[count]->node    = node;
                opInfo[count]->opt     = operation;
                opInfo[count]->op      = operation->operatorType;
                opInfo[count]->target  = operation->target;
                opInfo[count]->xpad    = WB_STRIDE_X(wb) > 1 ? 0 : ((-1) * WB_PAD_LEFT(wb));
                opInfo[count]->ypad    = WB_STRIDE_Y(wb) > 1 ? 0 : ((-1) *  WB_PAD_TOP(wb));
                opInfo[count]->weight_bias = wb;
                opInfo[count]->kx = 1;
                opInfo[count]->ky = 1;
                opInfo[count]->kz = wb->weights_sizes[2];
                opInfo[count]->p3 = 0;
                opInfo[count]->xsize = opInfo[count]->origoutx;
                opInfo[count]->ysize = opInfo[count]->origouty;
                opInfo[count]->pix = (vx_uint32)ceilf((vx_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
                opInfo[count]->piy = (vx_uint32)ceilf((vx_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);
                xArray[count] = opInfo[count]->pix;
                yArray[count] = opInfo[count]->piy;
                zArray[count] = opInfo[count]->oz;
                opInfo[count]->siz = opInfo[count]->oz;
                opInfo[count]->sbuf = SW_TILING_FROM_DDR;
                opInfo[count]->dbuf = SW_TILING_FROM_DDR;
                opInfo[count]->kbuf = SW_TILING_FROM_DDR;
                opInfo[count]->fcmd = vx_false_e;
                opInfo[count]->bfy = 1;
                opInfo[count]->bfz = 1;

                opInfo[count]->downStreamLayerCount = operation->childOpNum;
                opInfo[count]->upStreamLayerCount = operation->parentOpNum;
                operation->segIndex = count;

                if (opInfo[count]->inputDataFormat == VX_TYPE_INT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountInt16;
                else if (opInfo[count]->inputDataFormat == VX_TYPE_FLOAT16)
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCountFloat16;
                else
                    opInfo[count]->nnCores = context->nnConfig.fixedFeature.nnCoreCount;

                count++;
            }
        }
    }

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1) && !hasVXC && (count > 1))
    {
        vx_uint32 first = 0;
        vx_uint32 last = count > 0 ? count - 1 : 0;
        GIBIO *gibIO = NULL;
        GIBObj *gibObj = NULL;
        vx_uint32 gibLast = 0;
        vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling ? vx_true_e : vx_false_e;
        vx_uint32 swTilingSegKernelBufSizeInPixel = 0, swTilingSegStart = 0;
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(GIBIO) * count,
            (gctPOINTER *)&gibIO
            );
        if (gcmIS_ERROR(status))
        {
            vxStatus = VX_FAILURE;
            vxInfo("ERROR: vxoGraph_PredictPerf(5) return out-of-memory\n");
            goto error;
        }
        memset(gibIO, 0, sizeof(GIBIO) * count);
        status = gcoOS_Allocate(
            gcvNULL,
            gcmSIZEOF(GIBObj) * count,
            (gctPOINTER *)&gibObj
            );
        if (gcmIS_ERROR(status))
        {
            vxStatus = VX_FAILURE;
            vxInfo("ERROR: vxoGraph_PredictPerf(6) return out-of-memory\n");
            vxFree(gibIO);
            goto error;
        }
        memset(gibObj, 0, sizeof(GIBObj) * count);
        updateStreamLayer(archModel, count);

        for (i = 0; i < count; i++)
        {
            if (opInfo[i]->target == VXNNE_OPERATION_TARGET_NN ||
                opInfo[i]->target == VXNNE_OPERATION_TARGET_TP)
            {
                first = i;
                sArray[i] = 1;
                break;
            }
        }

        if (last < count)
        {
            vx_uint32 numOfBlocks = (vx_uint32)ceilf(1.0f * (count - 1) / MAX_LAYERS_OF_BLOCK);
            vx_uint32 block;
            for (block = 0; block < numOfBlocks; block++)
            {
                vx_uint32 block_first = block * MAX_LAYERS_OF_BLOCK;
                vx_uint32 block_last = gcmMIN((count - 1), (block + 1) * MAX_LAYERS_OF_BLOCK - 1);
                if ((block_last + 1) < (count - 1))
                    sArray[block_last + 1] = 1;
                resetArchModelSplitInfo(archModel);
                initSegmentCostResult(archModel);

                _split_segment(context, archModel, block_first, block_last, xArray, yArray, zArray, sArray);
            }

            if (/*context->options.enableHandleBranch*/ vx_true_e && /*always turn on branch merge feature for arch model algorithm*/
               (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_ALL || context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_AB))
            {
                gibLast = _create_gib(context, archModel, count, xArray, yArray, zArray, sArray, gibIO, gibObj);
                _merge_sub_graph(context, archModel, count,  xArray, yArray, zArray, sArray, gibIO, gibObj, gibLast);
            }

            for (i = first; i < count; i++)
            {
                if (opInfo[i]->upStreamLayerCount == 0)
                {
                    opInfo[i]->sbuf = SW_TILING_FROM_DDR;
                }
                else
                {
                    opInfo[i]->sbuf = (sArray[i]) ? SW_TILING_FROM_DDR : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);
                }
                if (opInfo[i]->downStreamLayerCount == 0)
                {
                    opInfo[i]->dbuf = SW_TILING_FROM_DDR;
                }
                else
                {
                    opInfo[i]->dbuf = (sArray[i]) ? SW_TILING_FROM_DDR : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);
                }
            }
            /* first sbuf and last dbuf is DDR */
            opInfo[first]->sbuf = SW_TILING_FROM_DDR;
            opInfo[last]->dbuf = SW_TILING_FROM_DDR;

            for (i = first; i < count; i++)
            {
                opInfo[i]->sbuf = SW_TILING_FROM_DDR;
                if (opInfo[i]->upStreamLayerCount > 0)
                {
                    vx_int32 uplayer = -1;
                    getUpstreamLayer(archModel, i, opInfo[i]->upStreamLayerCount - 1, &uplayer);
                    if (uplayer >= 0) {
                        opInfo[i]->sbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    }
                }

                opInfo[i]->dbuf = SW_TILING_FROM_DDR;
                if (opInfo[i]->downStreamLayerCount > 0)
                {
                    vx_int32 downlayer = -1;
                    getDownstreamLayer(archModel, i, 0, &downlayer);
                    if (downlayer > 0)
                    {
                        opInfo[i]->dbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    }
                }

                if (opInfo[i]->swTilingType == 0)
                {
                    opInfo[i]->kbuf = SW_TILING_FROM_VIP_SRAM;
                }
                else
                {
                    opInfo[i]->kbuf = SW_TILING_FROM_DDR;
                }

                if (sArray[i])
                {
                    opInfo[i]->sbuf = SW_TILING_FROM_DDR;
                    if (opInfo[i]->upStreamLayerCount > 0)
                    {
                        for (j = 0; j < opInfo[i]->upStreamLayerCount; j++)
                        {
                            vx_int32 uplayer;
                            getUpstreamLayer(archModel, i, j, &uplayer);
                            if (uplayer >= 0) {
                                opInfo[uplayer]->dbuf = SW_TILING_FROM_DDR;
                            }
                        }
                    }
                }
            }
        }

        for (i = first; i < count; i++)
        {
            if (opInfo[i]->target == VXNNE_OPERATION_TARGET_SH ||
                opInfo[i]->target == VXNNE_OPERATION_TARGET_SW)
            {
                if (context->options.enableNNArchPerfPrint)
                {
                    showArchPerformance(context, opInfo[i]->opt->layer, opInfo[i]->opt, VX_NULL);
                }
                continue;
            }

            {
                vx_int32 spaceLeft = axiSramOnlySWTiling ? vxmARCH_VIP_SRAM_SIZE
                    : (vxmARCH_VIP_SRAM_SIZE - gibObj[i].totalBufferNeeded);
                vx_int32 flush_and_wait = 0;
                archModel->opInfoArray[i]->segTotalBufferSizeInPixel = gibObj[i].totalBufferNeeded;
                if (opInfo[i]->sbuf == SW_TILING_FROM_DDR)
                {
                    if (opInfo[i]->swTilingType == 0)
                    {
                        /*SWTiling segment start point*/
                        swTilingSegKernelBufSizeInPixel = _kernel_size_in_pixel(archModel, i, archModel->opInfoArray[i]->nnCores, context->nnConfig.unifiedFeature.fullCacheKernelHeadFix);
                        swTilingSegStart = i;
                    }
                    else
                    {
                        swTilingSegKernelBufSizeInPixel = 0;
                    }

                    if (opInfo[i]->downStreamLayerCount == 0)
                    {
                        flush_and_wait = 1;
                    }
                    else
                    {
                        for (j = 0; j < opInfo[i]->downStreamLayerCount; j++)
                        {
                            vx_int32 downlayer = -1;
                            getDownstreamLayer(archModel, i, j, &downlayer);
                            if (downlayer == -1)
                            {
                                flush_and_wait = 1;
                                break;
                            }
                            else
                            {
                                if (opInfo[downlayer]->dbuf == SW_TILING_FROM_DDR)
                                {
                                    flush_and_wait = 1;
                                    break;
                                }
                            }
                        }
                    }
                    opInfo[i]->perf.info.flush = flush_and_wait;
                }
                else
                {
                    if (opInfo[i]->swTilingType == 0)
                    {
                        /*SWTiling segment start point*/
                        swTilingSegKernelBufSizeInPixel += _kernel_size_in_pixel(archModel, i, archModel->opInfoArray[i]->nnCores, context->nnConfig.unifiedFeature.fullCacheKernelHeadFix);

                        if (opInfo[i]->dbuf == SW_TILING_FROM_DDR)
                        {
                            vx_uint32 swTilingSegOutBufSize = _outbuf_needed_ex(archModel, swTilingSegStart, i, xArray, yArray, zArray);
                            for (j = swTilingSegStart; j <= i; j++)
                            {
                                archModel->opInfoArray[i]->swTilingSegKernelBufSizeInPixel = swTilingSegKernelBufSizeInPixel;
                                archModel->opInfoArray[i]->swTilingSegOutBufSizeInPixel = swTilingSegOutBufSize;
                            }
                        }
                    }

                    opInfo[i]->perf.info.flush = flush_and_wait;
                }



#if ENABLE_ARCH_MODEL_DUMP
                vxInfo("table[%d]: sbuf: %s, dbuf: %s, kbuf: %s, abBufferSpaceLeft: %u, flush_and_waith: %d\n", i + 1,
                opInfo[i]->sbuf == 0 ? "DDR" : (opInfo[i]->sbuf == 1) ? "AXI_SRAM" : "VIP_SRAM",
                opInfo[i]->dbuf == 0 ? "DDR" : (opInfo[i]->dbuf == 1) ? "AXI_SRAM" : "VIP_SRAM",
                opInfo[i]->kbuf == 0 ? "DDR" : (opInfo[i]->kbuf == 1) ? "AXI_SRAM" : "VIP_SRAM",
                spaceLeft,
                opInfo[i]->perf.info.flush);
#endif

                _calc_cost(context, archModel, i,
                    xArray[i], yArray[i], zArray[i],
                    opInfo[i]->sbuf,
                    opInfo[i]->dbuf,
                    opInfo[i]->kbuf,
                    spaceLeft);
            }

            if (context->options.enableNNArchPerfPrint)
            {
                showArchPerformance(context, opInfo[i]->opt->layer, opInfo[i]->opt, &opInfo[i]->perf);
            }

        }
        {
            /*swTilingType: -1: none, 1: AB, 0: Sub-IMAGE*/
            vx_uint32 swtStart = 0, abStart = 0, abEnd = 0;
            vx_uint32 abFlag = 0;
            for (i = first; i < count; i++)
            {
                if (opInfo[i]->target == VXNNE_OPERATION_TARGET_SH ||
                    opInfo[i]->target == VXNNE_OPERATION_TARGET_SW)
                {
                   vxInfo("abs_op_id[%d]: %c->%c, %s\n",
                          opInfo[i]->opt->absoluteOperationID,
                          'D', 'D', "DDR");
                }
                else
                {
                   vxInfo("abs_op_id[%d]: %c->%c, %s\n",
                          opInfo[i]->opt->absoluteOperationID,
                          (opInfo[i]->sbuf == 0) ? 'D' : 'S',
                          (opInfo[i]->dbuf == 0) ? 'D' : 'S',
                          (opInfo[i]->swTilingType == 1) ? "AB" :
                          (opInfo[i]->swTilingType == 0) ? "SWT" : "DDR");
                }
            }
            vxInfo("@@detected AB/SWT blocks:");
            for (i = first; i < count; i++)
            {
                if (opInfo[i]->swTilingType != -1)
                {
                    if (opInfo[i]->sbuf == 0) /*AB or SWT block start point*/
                    {
                        if (opInfo[i]->dbuf == 0)
                        {
                            continue;
                        }
                        else
                        {
                            if (opInfo[i]->swTilingType == 1)
                            {
                                if (abFlag == 0)
                                {
                                    abStart = opInfo[i]->opt->absoluteOperationID;
                                    abFlag = 1;
                                }
                            }
                            else
                            {
                                swtStart = opInfo[i]->opt->absoluteOperationID;
                                if (abFlag == 2)
                                {
                                    vxInfo("[%d,%d,%s]", abStart, abEnd, "AB");
                                    abFlag = 0;
                                    abStart = abEnd = 0;
                                }
                            }
                        }
                    }

                    if (opInfo[i]->dbuf == 0) /*AB or SWT block end point*/
                    {
                        if (opInfo[i]->swTilingType == 0)
                        {
                            vxInfo("[%d,%d,%s]", swtStart, opInfo[i]->opt->absoluteOperationID, "SWT");
                        }
                        else
                        {
                            if (opInfo[i]->swTilingType == 1)
                            {
                                abEnd = opInfo[i]->opt->absoluteOperationID;
                                abFlag = 2;
                            }
                        }
                    }
                }
            }
            if (abFlag == 2)
            {
                vxInfo("[%d,%d,%s]", abStart, abEnd, "AB");
            }
            vxInfo("\n");
        }
        vxFree(gibIO);
        vxFree(gibObj);
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (opInfo[i]->target == VXNNE_OPERATION_TARGET_SH ||
                opInfo[i]->target == VXNNE_OPERATION_TARGET_SW)
            {
                if (context->options.enableNNArchPerfPrint)
                {
                    showArchPerformance(context, opInfo[i]->opt->layer, opInfo[i]->opt, VX_NULL);
                }
                continue;
            }
#if ENABLE_ARCH_MODEL_DUMP
                vxInfo("table[%d]: sbuf: %s, dbuf: %s, kbuf: %s, abBufferSpaceLeft: %u, flush_and_waith: %d\n", i + 1,
                "DDR",
                "DDR",
                "DDR",
                vxmARCH_VIP_SRAM_SIZE,
                1);
#endif
            opInfo[i]->perf.info.flush = 1;
            _calc_cost(context, archModel, i,
                opInfo[i]->xsize, opInfo[i]->ysize, opInfo[i]->oz,
                SW_TILING_FROM_DDR, SW_TILING_FROM_DDR, SW_TILING_FROM_DDR, vxmARCH_VIP_SRAM_SIZE);

            if (context->options.enableNNArchPerfPrint)
            {
                showArchPerformance(context, opInfo[i]->opt->layer, opInfo[i]->opt, &opInfo[i]->perf);
            }

        }
    }

error:
    if (xArray) vxFree(xArray);
    if (yArray) vxFree(yArray);
    if (zArray) vxFree(zArray);
    if (sArray) vxFree(sArray);
    deInitArchModelInfo(archModel, graph->layer->base.num_operations);
    return vxStatus;
}
