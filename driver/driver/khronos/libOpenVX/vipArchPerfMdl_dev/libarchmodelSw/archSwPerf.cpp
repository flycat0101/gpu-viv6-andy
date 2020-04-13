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


/***********************************************************************************
* Copyright:    Verisilicon
* FileName:     archSwPerf.c
* Author:       JinboHuang
* Data:         2019-05-27
* Version:      0.5.00
* Description:  Definition of the interface in Arch Model Library to do the predict for both driver or Python
*
***************************************************************************************/

#include "archSwPerf.h"
#include "archSwCommon.h"

/*******************************************DEBUG***************************************************/
#define ENABLE_ARCH_MODEL_DUMP 0
/*#define DUMP_PARAMETERS 1*/

/*******************************************Global var definition*****************************************************/
arch_uint32 gAlign64BExceted = 1;               /* fix by jinbo, TBD to set the value */
archSwLibContext gArchSwLibContext = {0};

arch_uint32 G_JINBO_WR = 0;
/*arch_int32 tempdata[100] = {0};*/
/*******************************************MARCO definition***************************************************/
/* MAX_COST 0x7fff_ffff */
#define MAX_COST 2147483647
#define MAX_LAYERS_OF_BLOCK 100

/* for calculate best cost */
#define CYCLE_WEIGHT 20
#define BW_WEIGHT 1

#define CACHE_ALIGNMENT_SIZE                128
#define RESERVE_16KB_FOR_TRANSPOSE          16 * 1024

#define TRSP_MAX_INTERLEAVE_CH              16
/*#define NN_TRANSPOSE    1*/
/*static arch_uint32 gNNTransposeEnable = 0;*/

/* May be deleted */
#define DISABLE_TP_RESHUFFLE_SPLIT 0
/* May be deleted done */
/****************************************** Local function declaration ***********************************************/
static void deInitArchModelSplitInfo(
    archModelSplitInfo * splitInfo,
    arch_uint32 operationCount
    );

static void emptyArchModelSplitInfo(
    archModelSplitInfo * splitInfo,
    arch_uint32 operationCount
    );

static void resetArchModelSplitInfo(
    archModelInfo *archModel
    );
static archModelSplitInfo * initArchModelSplitInfo(
    arch_uint32 operationCount
    );

static void deInitArchModelInfo(
    archModelInfo *archModel,
    arch_uint32 operationCount
    );

static archModelInfo * initArchModelInfo(
    arch_uint32 operationCount
    );

static void initSegmentCostResult(
    archModelInfo * archModel
    );

static void getSegmentCostResult(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    archModelCost *cost
    );

static void setSegmentCostResult(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    archModelCost *cost);

static arch_bool _cur_cost_is_more_better(
    archModelCost *cost,
    archModelCost *cur,
    arch_uint32 cycle_weight,
    arch_uint32 bw_weight
    );

static arch_int32 getBestCostSWTilingTypeInfo(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last
    );

static void setBestCostSWTilingTypeInfo(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_int32 bestCostSWTilingType
    );

static void setSplitArrayInfo(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_int32 pos,
    arch_int8 split
    );

static arch_uint32 _kernel_size_in_pixel(
    archModelInfo *archModel,
    arch_int32 index,
    arch_uint32 cores,
    arch_bool fullCacheIntervalFix
    );

static arch_uint32 _outbuf_needed_ex(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 psixArray[],
    arch_uint32 psiyArray[],
    arch_uint32 psizArray[],
    arch_uint32 trspIvLayerChsOut[]
);

static arch_uint32 _calc_full_cached_space_needed(
    archModelInfo *archModel,
    arch_uint32 segment_index,
    arch_uint32 psixArray[],
    arch_uint32 psiyArray[],
    arch_uint32 max_tile_size,
    arch_uint32 n
    );

static arch_float64 _calc_cost(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archModelInfo *archModel,
    arch_int32 index,
    arch_uint32 x,
    arch_uint32 y,
    arch_uint32 z,
    arch_uint8 src_buf,
    arch_uint8 dst_buf,
    arch_uint8 kenerl_buf,
    arch_int32 cache_space,
    arch_uint32 trspIvLayerChsIn,
    arch_uint32 trspIvLayerChsOut
    );

#ifdef CACHE_SPACE_ENABLE
static void _subimage_segment_cost(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint32 cacheSpace[],
    arch_uint32 curTrspIvLayerChsIn[],
    arch_uint32 curTrspIvLayerChsOut[],
    arch_int32 *best_cost_swtiling_type,
    archModelCost *cost
    );

static arch_bool ComputeSubimageYSize(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_int32 vip_sram_left,
    arch_int32 axi_sram_left,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint32 vipSramRequired[],
    arch_bool axiSramOnlySWTiling,
    arch_uint32 trspIvLayerChsOut[],
    arch_uint32 max_tile_size
    );

static arch_int32 ComputeABBuffer(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 sram_space,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 vipSramRequired[],
    arch_bool axiSramOnlySWTiling,
    arch_uint32 trspIvLayerChsOut[]
);

static void _split_segment_loop(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint32 cacheSpace[],
    arch_uint32 trspIvLayerChsIn[],
    arch_uint32 trspIvLayerChsOut[]
);

static void saveCalculationArgs(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 pos,
    archModelCost *cost,
    arch_uint32 six,
    arch_uint32 siy,
    arch_uint32 siz,
    arch_uint32 savedCacheSpace,
    arch_uint32 savedTrspIvLayerChsIn,
    arch_uint32 savedTrspIvLayerChsOut,
    arch_uint8 split
    );

static void _split_segment(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint32 cacheSpace[],
    arch_uint32 trspIvLayerChsIn[],
    arch_uint32 trspIvLayerChsOut[],
    arch_uint8 split_array[]
);

static void _merge_sub_graph(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archModelInfo *archModel,
    arch_uint32 count,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint32 cacheSpace[],
    arch_uint32 curTrspIvLayerChsIn[],
    arch_uint32 curTrspIvLayerChsOut[],
    arch_uint8 split_array[],
    archGIBIO *gib_io,archGIBObj *gib_obj,
    arch_uint32 gib_last
    );

arch_uint32 isTensorAdd(
    arch_uint32 opTarget,
    arch_uint32 kz,
    arch_uint32 z
    );

#else

static void _subimage_segment_cost(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_int32 *best_cost_swtiling_type,
    archModelCost *cost
    );

static arch_bool ComputeSubimageYSize(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_int32 vip_sram_left,
    arch_int32 axi_sram_left,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint32 max_tile_size
    );

static arch_int32 ComputeABBuffer(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 sram_space,
    arch_uint32 x_array[],
    arch_uint32 y_array[]
);

static void _split_segment_loop(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[]
);

static void saveCalculationArgs(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 pos,
    archModelCost *cost,
    arch_uint32 six,
    arch_uint32 siy,
    arch_uint32 siz,
    arch_uint8 split
    );

static void _split_segment(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint8 split_array[]
);

static void _merge_sub_graph(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archModelInfo *archModel,
    arch_uint32 count,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint8 split_array[],
    archGIBIO *gib_io,
    archGIBObj *gib_obj,
    arch_uint32 gib_last
    );

#endif

#ifdef ALIGNMENT_64B
static arch_uint32 vipSram64BAlignEnhance(
    arch_nn_config *pArchNnConfig,
struct _archModelInfo *archModel,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 cacheSpace[],
    arch_uint8 sArray[],
    arch_uint32 flushWait[]
);

#endif

#ifdef SPLIT_Z_DIMENSION
static arch_uint32 splitZDimension(
    APMHandle apm,
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archModelInfo *archModel,
    arch_uint32 z_array[],
    arch_uint32 cacheSpace[],
    arch_uint8 sArray[],
    arch_uint32 flushWait[]
);

#endif
#ifndef CACHE_SPACE_ENABLE
static arch_bool _calc_x_subimage(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_int32 sram_left,
    arch_uint32 ppsiy,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[]
);

#endif
static arch_uint32 _seg_buf_needed(
    archModelInfo *archModel,
    arch_uint32 segment_first,
    arch_uint32 segment_last,
    arch_uint32 sixArray[],
    arch_uint32 siyArray[],
    arch_uint32 z_array[],
    arch_uint32 trspIvLayerChsOut[]
);

static void getUpstreamLayer(
    archModelInfo *archModel,
    arch_uint32 index,
    arch_uint32 id,
    arch_int32 *upstream
    );

static void getDownstreamLayer(
    archModelInfo *archModel,
    arch_uint32 index,
    arch_uint32 id,
    arch_int32 *downstream
    );

static ARCH_INTERNAL_API arch_status archPerfAnalysing(
    archModelInfo * archModel,
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions
    );

APMHandle archApmInit(
    arch_nn_config *pArchNnConfig,
    archHAL_CHIPIDENTITY *pChipIdentity,
    archNN_DATABASE_FEATURE *pArchDataFeature
    );
/* GIB related */


/* for debug */
void printOpInfo(
    archModelOpInfo ** archOp,
    arch_uint32 totalCount
    );

void printConfig(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archHAL_CHIPIDENTITY *pChipIdentity,
    archNN_DATABASE_FEATURE *pArchDataFeature
    );

/***********************************************************************************
* Function:     _cur_cost_is_more_better
* Description:  Check if the new cost result is better than the current best cost
* Input:        cost:   The previous best cost
*               cur:    The new cost value
*               cycle_weight:       cycleCount weight
*               bw_weight:          BW weight
* Ouput:        arch_bool:          better for true and not for worst
***************************************************************************************/
static arch_bool _cur_cost_is_more_better(
    archModelCost *cost,
    archModelCost *cur,
    arch_uint32 cycle_weight,
    arch_uint32 bw_weight
    )
{
    arch_float64 f;
    arch_float64 cycleDiff = cur->cycle - cost->cycle;
    arch_float64 bwDiff = cur->bw - cost->bw;
    if (cycleDiff > 0 && cycleDiff < 1)
    {
        cycleDiff = (arch_float64)((arch_uint64)(cycleDiff * 100000000 + 0.5))/100000000;
    }
    else if (cycleDiff < 0 && cycleDiff > -1)
    {
        cycleDiff = -(arch_float64)((arch_uint64)(-1 * cycleDiff * 100000000 + 0.5))/100000000;
    }
    if (bwDiff > 0 && bwDiff < 1)
    {
        cycleDiff = (arch_float64)((arch_uint64)(bwDiff * 100000000 + 0.5))/100000000;
    }
    else if (bwDiff < 0 && bwDiff > -1)
    {
        bwDiff = -(arch_float64)((arch_uint64)(-1 * bwDiff * 100000000 + 0.5))/100000000;
    }

    f = -(1.0f * cycleDiff / archMAX(cur->cycle, cost->cycle) * cycle_weight + 1.0f * bwDiff / archMAX(cur->bw, cost->bw) * bw_weight);
    if (f > 0) return arch_true_e;
    return arch_false_e;
}


/***********************************************************************************
* Function:     deInitArchOpInfo
* Description:  DeInit the OpInfo struction and free the memory
* Input:        archOpinfo:
*               operationCount:
* Ouput:        NULL
***************************************************************************************/
void deInitArchOpInfo(
    archModelOpInfo **archOpInfo,
    arch_uint32 operationCount
    )
{
    arch_uint32 i;
    /* free every OpInfo, total count is "operationCount" */
    for (i = 0; i < operationCount; i++)
    {
        if (archOpInfo && archOpInfo[i] != NULL)
        {
            archFreeMemory(archOpInfo[i]);
        }
    }

    if (archOpInfo != NULL) archFreeMemory(archOpInfo);
}

/***********************************************************************************
* Function:     initArchOpInfo
* Description:  Init the OpInfo struction, allocate the memory for both OpInfo struct and every OpInfo
* Input:        operationCount:       Total operation count
* Ouput:        archModelOpInfo**:    return the Opinfo pointer
***************************************************************************************/
archModelOpInfo ** initArchOpInfo(
    arch_uint32 operationCount
    )
{
    archSTATUS status = archSTATUS_OK;
    arch_uint32 i = 0;
    archModelOpInfo **archOpInfo = NULL;

    status = archAllocateMemory(sizeof(struct _archModelOpInfo *) * operationCount, (archPOINTER *)&archOpInfo);
    if (archIS_ERROR(status)) goto error;

    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(struct _archModelOpInfo), (archPOINTER *)&archOpInfo[i]);
        if (archIS_ERROR(status)) goto error;
        memset(archOpInfo[i], 0, sizeof(struct _archModelOpInfo));
    }

    return archOpInfo;

error:
    if (archOpInfo != NULL) {
        deInitArchOpInfo(archOpInfo, operationCount);
    }
    archInfo("ERROR: initArchOpInfo() return out-of-memory\n");
    return NULL;
}



/***********************************************************************************
* Function:     deInitArchModelSplitInfo
* Description:  DeInit the SplitInfo struction and free the memory. Split Info used for
*                    store the whole graph after splited
* Input:        splitInfo:
*               operationCount:
* Ouput:        NULL
***************************************************************************************/
static void deInitArchModelSplitInfo(
    archModelSplitInfo * splitInfo,
    arch_uint32 operationCount
    )
{
    arch_uint32 i;
    if(splitInfo == NULL) return;

    if (splitInfo->savedSIX)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedSIX[i] != NULL) archFreeMemory(splitInfo->savedSIX[i]);
        }
        archFreeMemory(splitInfo->savedSIX);
    }
    if (splitInfo->savedSIY)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedSIY[i] != NULL) archFreeMemory(splitInfo->savedSIY[i]);
        }
        archFreeMemory(splitInfo->savedSIY);
    }
    if (splitInfo->savedSIZ)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedSIZ[i] != NULL) archFreeMemory(splitInfo->savedSIZ[i]);
        }
        archFreeMemory(splitInfo->savedSIZ);
    }

#ifdef CACHE_SPACE_ENABLE
    if (splitInfo->savedCacheSpace)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedCacheSpace[i] != NULL) archFreeMemory(splitInfo->savedCacheSpace[i]);
        }
        archFreeMemory(splitInfo->savedCacheSpace);
    }
#endif
    /* NN transpose */
    if (splitInfo->savedTrspIvLayerChsIn)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedTrspIvLayerChsIn[i] != NULL) archFreeMemory(splitInfo->savedTrspIvLayerChsIn[i]);
        }
        archFreeMemory(splitInfo->savedTrspIvLayerChsIn);
    }
    if (splitInfo->savedTrspIvLayerChsOut)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedTrspIvLayerChsOut[i] != NULL) archFreeMemory(splitInfo->savedTrspIvLayerChsOut[i]);
        }
        archFreeMemory(splitInfo->savedTrspIvLayerChsOut);
    }

    if (splitInfo->savedCost)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->savedCost[i] != NULL) archFreeMemory(splitInfo->savedCost[i]);
        }
        archFreeMemory(splitInfo->savedCost);
    }

    if (splitInfo->savedSegmentCost) archFreeMemory(splitInfo->savedSegmentCost);
    if (splitInfo->bestCostSWTilingType) archFreeMemory(splitInfo->bestCostSWTilingType);
    if (splitInfo->split_array)
    {
        for (i = 0; i < operationCount; i++)
        {
            if (splitInfo->split_array[i] != NULL) archFreeMemory(splitInfo->split_array[i]);
        }
        archFreeMemory(splitInfo->split_array);
    }
    if (splitInfo) archFreeMemory(splitInfo);
}

/***********************************************************************************
* Function:     emptyArchModelSplitInfo
* Description:  Clear the Split Info struct after created
* Input:        splitInfo:
*               operationCount:
* Ouput:        NULL
***************************************************************************************/
static void emptyArchModelSplitInfo(
    archModelSplitInfo * splitInfo,
    arch_uint32 operationCount
    )
{
    arch_uint32 i;
    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedSIX[i], 0, sizeof(arch_uint32) * operationCount);
    }

    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedSIY[i], 0, sizeof(arch_uint32) * operationCount);
    }

    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedSIZ[i], 0, sizeof(arch_uint32) * operationCount);
    }
#ifdef CACHE_SPACE_ENABLE
    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedCacheSpace[i], 0, sizeof(arch_uint32) * operationCount);
    }
#endif
    /* NN transpose */
    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedTrspIvLayerChsIn[i], 0, sizeof(arch_uint32) * operationCount);
    }
    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedTrspIvLayerChsOut[i], 0, sizeof(arch_uint32) * operationCount);
    }

    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->savedCost[i], 0, sizeof(struct _archModelCost) * operationCount);
    }

    memset(splitInfo->savedSegmentCost, 0, sizeof(struct _archModelCost) * operationCount);
    memset(splitInfo->bestCostSWTilingType, 0, sizeof(arch_bool) * operationCount);

    for (i = 0; i < operationCount; i++)
    {
        memset(splitInfo->split_array[i], 0, sizeof(arch_uint8) * operationCount);
    }
}

/***********************************************************************************
* Function:     resetArchModelSplitInfo
* Description:  Call emptyArchModelSplitInfo to reset the Split Info
* Input:        archModel:
* Ouput:        NULL
***************************************************************************************/
static void resetArchModelSplitInfo(
    archModelInfo *archModel
    )
{
    arch_uint32 opCount = (archModel->totalOpCount > MAX_LAYERS_OF_BLOCK) ?
                           MAX_LAYERS_OF_BLOCK : archModel->totalOpCount;

    for (arch_uint32 i = 0; i < opCount; i++)
    {
        emptyArchModelSplitInfo(archModel->splitInfoArray[i], opCount);
    }
}

/***********************************************************************************
* Function:     initArchModelSplitInfo
* Description:  Init the Split Info struction, allocate the memory for both OpInfo struct and every OpInfo
* Input:        operationCount:        Total operation count
* Ouput:        _archModelSplitInfo*:    return the Split Info pointer
***************************************************************************************/
static archModelSplitInfo * initArchModelSplitInfo(
    arch_uint32 operationCount
    )
{
    archSTATUS status = archSTATUS_OK;
    arch_uint32 i;
    struct _archModelSplitInfo *splitInfo = NULL;
    status = archAllocateMemory(sizeof(struct _archModelSplitInfo),(archPOINTER *)&splitInfo);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo, 0, sizeof(struct _archModelSplitInfo));

    status = archAllocateMemory(sizeof(arch_uint32 *) * operationCount, (archPOINTER *)&splitInfo->savedSIX);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->savedSIX, 0, sizeof(arch_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(arch_uint32) * operationCount,(archPOINTER *)&splitInfo->savedSIX[i]);
        if (archIS_ERROR(status)) goto error;

        memset(splitInfo->savedSIX[i], 0, sizeof(arch_uint32) * operationCount);
    }

    status = archAllocateMemory(sizeof(arch_uint32 *) * operationCount,(archPOINTER *)&splitInfo->savedSIY);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->savedSIY, 0, sizeof(arch_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(arch_uint32) * operationCount,(archPOINTER *)&splitInfo->savedSIY[i]);
        if (archIS_ERROR(status)) goto error;

        memset(splitInfo->savedSIY[i], 0, sizeof(arch_uint32) * operationCount);
    }

    status = archAllocateMemory(sizeof(arch_uint32 *) * operationCount,(archPOINTER *)&splitInfo->savedSIZ);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->savedSIZ, 0, sizeof(arch_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(arch_uint32) * operationCount,(archPOINTER *)&splitInfo->savedSIZ[i]);
        if (archIS_ERROR(status)) goto error;

        memset(splitInfo->savedSIZ[i], 0, sizeof(arch_uint32) * operationCount);
    }
#ifdef CACHE_SPACE_ENABLE
    status = archAllocateMemory(sizeof(arch_uint32 *) * operationCount,(archPOINTER *)&splitInfo->savedCacheSpace);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->savedCacheSpace, 0, sizeof(arch_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(arch_uint32) * operationCount,(archPOINTER *)&splitInfo->savedCacheSpace[i]);
        if (archIS_ERROR(status)) goto error;

        memset(splitInfo->savedCacheSpace[i], 0, sizeof(arch_uint32) * operationCount);
    }
#endif

    /* NN transpose */
    status = archAllocateMemory(sizeof(arch_uint32 *) * operationCount,(archPOINTER *)&splitInfo->savedTrspIvLayerChsIn);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->savedTrspIvLayerChsIn, 0, sizeof(arch_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(arch_uint32) * operationCount,(archPOINTER *)&splitInfo->savedTrspIvLayerChsIn[i]);
        if (archIS_ERROR(status)) goto error;

        memset(splitInfo->savedTrspIvLayerChsIn[i], 0, sizeof(arch_uint32) * operationCount);
    }
    status = archAllocateMemory(sizeof(arch_uint32 *) * operationCount,(archPOINTER *)&splitInfo->savedTrspIvLayerChsOut);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->savedTrspIvLayerChsOut, 0, sizeof(arch_uint32 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(arch_uint32) * operationCount,(archPOINTER *)&splitInfo->savedTrspIvLayerChsOut[i]);
        if (archIS_ERROR(status)) goto error;

        memset(splitInfo->savedTrspIvLayerChsOut[i], 0, sizeof(arch_uint32) * operationCount);
    }

    status = archAllocateMemory(sizeof(archModelCost *) * operationCount,(archPOINTER *)&splitInfo->savedCost);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->savedCost, 0, sizeof(archModelCost *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(archModelCost) * operationCount,(archPOINTER *)&splitInfo->savedCost[i]);
        if (archIS_ERROR(status)) goto error;
        memset(splitInfo->savedCost[i], 0, sizeof(archModelCost) * operationCount);
    }

    status = archAllocateMemory(sizeof(archModelCost) * operationCount,(archPOINTER *)&splitInfo->savedSegmentCost);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->savedSegmentCost, 0, sizeof(archModelCost) * operationCount);

    status = archAllocateMemory(sizeof(arch_bool) * operationCount,(archPOINTER *)&splitInfo->bestCostSWTilingType);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->bestCostSWTilingType, 0, sizeof(arch_bool) * operationCount);

    status = archAllocateMemory(sizeof(arch_uint8 *) * operationCount,(archPOINTER *)&splitInfo->split_array);
    if (archIS_ERROR(status)) goto error;
    memset(splitInfo->split_array, 0, sizeof(arch_uint8 *) * operationCount);
    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(arch_uint8) * operationCount,(archPOINTER *)&splitInfo->split_array[i]);
        if (archIS_ERROR(status)) goto error;

        memset(splitInfo->split_array[i], 0, sizeof(arch_uint8) * operationCount);
    }

    return splitInfo;

error:
    deInitArchModelSplitInfo(splitInfo, operationCount);
    archInfo("ERROR: initArchModelSplitInfo() return out-of-memory\n");
    return NULL;
}


/***********************************************************************************
* Function:     deInitArchModelInfo
* Description:  Function for deinit the Arch Model
* Input:        operationCount:        Total operation count
* Ouput:        _archModelSplitInfo*:    return the Split Info pointer
***************************************************************************************/
static void deInitArchModelInfo(
    archModelInfo *archModel,
    arch_uint32 operationCount
    )
{
    arch_uint32 i;

    for (i = 0; i < ((operationCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : operationCount); i++)
    {
        if (archModel->splitInfoArray && archModel->splitInfoArray[i] != NULL)
        {
            deInitArchModelSplitInfo(archModel->splitInfoArray[i], ((operationCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : operationCount));
        }
    }

    if (archModel->splitInfoArray != NULL) archFreeMemory(archModel->splitInfoArray);

    if (archModel != NULL) archFreeMemory(archModel);
}


/***********************************************************************************
* Function:     initArchModelInfo
* Description:  Init the Arch Model struct
* Input:        operationCount:        Total operation count
* Ouput:        _archModelInfo*:    return the Arch Model pointer
***************************************************************************************/
static struct _archModelInfo * initArchModelInfo(
    arch_uint32 operationCount
    )
{
    archSTATUS status = archSTATUS_OK;
    arch_uint32 i;
    struct _archModelInfo *archModel;
    status = archAllocateMemory(sizeof(struct _archModelInfo),(archPOINTER *)&archModel);
    if (archIS_ERROR(status)) goto error;
    archModel->totalOpCount = operationCount;
    archModel->actualCount = operationCount;
    archModel->apm = NULL;

    /* Do not need OpInfo, should be passed from outside */

    status = archAllocateMemory(sizeof(struct _archModelSplitInfo *) * ((operationCount > MAX_LAYERS_OF_BLOCK) ? MAX_LAYERS_OF_BLOCK : operationCount),(archPOINTER *)&archModel->splitInfoArray);
    if (archIS_ERROR(status)) goto error;

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
    archInfo("ERROR: initArchModelInfo() return out-of-memory\n");
    return NULL;
}

/***********************************************************************************
* Function:     initSegmentCostResult
* Description:  Set the Cost result to Init value(-1) for Arch Model
* Input:        _archModelInfo*:    return the Arch Model pointer
* Ouput:        NULL
***************************************************************************************/
static void initSegmentCostResult(
    archModelInfo * archModel
    )
{
    arch_uint32 i, j;
    arch_uint32 count = archModel->totalOpCount > MAX_LAYERS_OF_BLOCK ? MAX_LAYERS_OF_BLOCK : archModel->totalOpCount;
    for (i = 0; i < count; i++)
    {
        for (j = 0; j < count; j++)
        {
            archModel->splitInfoArray[i]->savedSegmentCost[j].bw = -1;
            archModel->splitInfoArray[i]->savedSegmentCost[j].cycle = -1;
        }
    }
}


/***********************************************************************************
* Function:     getSegmentCostResult
* Description:  Get the Cost result for Arch Model
* Input:        _archModelInfo*:    return the Arch Model pointer
*                segment_first:
                segment_last:
                cost:
* Ouput:        NULL
***************************************************************************************/
static void getSegmentCostResult(
    struct _archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    archModelCost *cost
    )
{
    assert(segment_first >= 0);
    assert(segment_last >= 0);
    cost->cycle = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSegmentCost[segment_last % MAX_LAYERS_OF_BLOCK].cycle;
    cost->bw = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSegmentCost[segment_last % MAX_LAYERS_OF_BLOCK].bw;
}


/***********************************************************************************
* Function:     setSegmentCostResult
* Description:  Set the Cost result for Arch Model
* Input:        _archModelInfo*:    return the Arch Model pointer
*                segment_first:
                segment_last:
                cost:
* Ouput:        NULL
***************************************************************************************/
static void setSegmentCostResult(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    archModelCost *cost
    )
{
    assert(segment_first >= 0);
    assert(segment_last >= 0);
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSegmentCost[segment_last % MAX_LAYERS_OF_BLOCK].cycle = cost->cycle;
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSegmentCost[segment_last % MAX_LAYERS_OF_BLOCK].bw = cost->bw;
}

/* get best cost result from Arch Model */
static arch_int32 getBestCostSWTilingTypeInfo(
    struct _archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last)
{
    assert(segment_first >= 0);
    assert(segment_last >= 0);
    return archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->bestCostSWTilingType[segment_last % MAX_LAYERS_OF_BLOCK];
}

/* Set best cost result from Arch Model */
static void setBestCostSWTilingTypeInfo(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_int32 bestCostSWTilingType
    )
{
    assert(segment_first >= 0);
    assert(segment_last >= 0);
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->bestCostSWTilingType[segment_last % MAX_LAYERS_OF_BLOCK] = bestCostSWTilingType;
}

/* Set Split array info into Arch Model */
static void setSplitArrayInfo(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_int32 pos,
    arch_int8 split
    )
{
    assert(segment_first >= 0);
    assert(segment_last >= 0);
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->split_array[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = split;
}

/* calc transpose inter leave according to Z */
static arch_uint32 getNon1x1TransposeInterleaveCH(
    arch_uint32 z
    )
{
    arch_uint32 n = 0;

    n = archMIN(z,TRSP_MAX_INTERLEAVE_CH);
    while((z % n) != 0 && n >=8)
    {
        n = n - 1;
    }

    if(n < 8)
        n = archMIN(z, TRSP_MAX_INTERLEAVE_CH);

    return n;
}

static unsigned int getInputTransposeInterleaveCH(
    archModelInfo *archModel,
    unsigned int           upLayer,
    int                    layer
    )
{
    unsigned int n = 0;
    if(layer == -1)
    {
        n = getNon1x1TransposeInterleaveCH(archModel->opInfoArray[upLayer]->oz);
    }
    else
    {
        if(archModel->opInfoArray[layer]->perf.SiblingHas1x1 == 1)      /* when sibling layer is 1x1, force to use 9 interleave channels */
        {
            n = 9;
        }
        else
        {
            n = getNon1x1TransposeInterleaveCH(archModel->opInfoArray[layer]->kz);
        }
    }

    return n;
}

/* Save the calculated result into Arch Model */
static void saveCalculationArgs(
    archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 pos,
    struct _archModelCost *cost,
    arch_uint32 six,
    arch_uint32 siy,
    arch_uint32 siz,
#ifdef CACHE_SPACE_ENABLE
    arch_uint32 savedCacheSpace,
#endif
    arch_uint32 savedTrspIvLayerChsIn,
    arch_uint32 savedTrspIvLayerChsOut,
    arch_uint8 split)
{
    assert(segment_first >= 0);
    assert(segment_last >= 0);
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSIX[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = six;
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSIY[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = siy;
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedSIZ[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = siz;
#ifdef CACHE_SPACE_ENABLE
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCacheSpace[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = savedCacheSpace;
#endif
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedTrspIvLayerChsIn[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = savedTrspIvLayerChsIn;
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedTrspIvLayerChsOut[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = savedTrspIvLayerChsOut;
    archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->split_array[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK] = split;

    if (cost != NULL)
    {
         archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCost[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK].cycle = cost->cycle;
         archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCost[segment_last % MAX_LAYERS_OF_BLOCK][pos % MAX_LAYERS_OF_BLOCK].bw = cost->bw;
    }
}

/* Arch detail result process for SWTiling */
void archSWDetailProcess(arch_perf perf, arch_uint32 element)
{
    /* All kind of cyclecount */
    perf->archPerfDetailResult.computeCC += element * perf->archPerfDetailResult.computeCC;
    perf->archPerfDetailResult.ddrRdCC += element * perf->archPerfDetailResult.ddrRdCC;
    perf->archPerfDetailResult.ddrWrCC += element * perf->archPerfDetailResult.ddrWrCC;
    perf->archPerfDetailResult.axiSramRdCC += element * perf->archPerfDetailResult.axiSramRdCC;
    perf->archPerfDetailResult.axiSramWrCC += element * perf->archPerfDetailResult.axiSramWrCC;
    perf->archPerfDetailResult.axiBusRdCC += element * perf->archPerfDetailResult.axiBusRdCC;
    perf->archPerfDetailResult.axiBusWrCC += element * perf->archPerfDetailResult.axiBusWrCC;
    perf->archPerfDetailResult.vipSramRdCC += element * perf->archPerfDetailResult.vipSramRdCC;
    perf->archPerfDetailResult.vipSramWrCC += element * perf->archPerfDetailResult.vipSramWrCC;
    perf->archPerfDetailResult.slowInternalWrCC += element * perf->archPerfDetailResult.slowInternalWrCC;
    perf->archPerfDetailResult.slowCompCC += element * perf->archPerfDetailResult.slowCompCC;
    perf->archPerfDetailResult.internalWrCC += element * perf->archPerfDetailResult.internalWrCC;
    perf->archPerfDetailResult.dWOutCC += element * perf->archPerfDetailResult.dWOutCC;
    perf->archPerfDetailResult.kernelDdrRdCC += element * perf->archPerfDetailResult.kernelDdrRdCC;
    perf->archPerfDetailResult.inImageDdrRdCC += element * perf->archPerfDetailResult.inImageDdrRdCC;
    perf->archPerfDetailResult.kernelDecodeCC += element * perf->archPerfDetailResult.kernelDecodeCC;
    perf->archPerfDetailResult.dqArbCC += element * perf->archPerfDetailResult.dqArbCC;
    perf->archPerfDetailResult.regTile2DxBarCC += element * perf->archPerfDetailResult.regTile2DxBarCC;
    perf->archPerfDetailResult.bottomTile2DXBarCC += element * perf->archPerfDetailResult.bottomTile2DXBarCC;
    perf->archPerfDetailResult.xBarCC += element * perf->archPerfDetailResult.xBarCC;
    perf->archPerfDetailResult.cacheControllerCC += element * perf->archPerfDetailResult.cacheControllerCC;
    perf->archPerfDetailResult.overHeadsCC += element * perf->archPerfDetailResult.overHeadsCC;
    perf->archPerfDetailResult.overAllCC += element * perf->archPerfDetailResult.overAllCC;
    /* region cycles/Bottleneck */
    perf->archPerfDetailResult.cyclesTile0Vzgroup0 += element * perf->archPerfDetailResult.cyclesTile0Vzgroup0;
    perf->archPerfDetailResult.cyclesTile0RestVzgroup0 += element * perf->archPerfDetailResult.cyclesTile0RestVzgroup0;
    perf->archPerfDetailResult.cyclesRestTileVzgroup0 += element * perf->archPerfDetailResult.cyclesRestTileVzgroup0;
    perf->archPerfDetailResult.cyclesRestTileRestVzgroup0 += element * perf->archPerfDetailResult.cyclesRestTileRestVzgroup0;
    perf->archPerfDetailResult.BottleneckTile0Vzgroup0 += element * perf->archPerfDetailResult.BottleneckTile0Vzgroup0;
    perf->archPerfDetailResult.BottleneckTile0RestVzgroup0 += element * perf->archPerfDetailResult.BottleneckTile0RestVzgroup0;
    perf->archPerfDetailResult.BottleneckRestTileVzgroup0 += element * perf->archPerfDetailResult.BottleneckRestTileVzgroup0;
    perf->archPerfDetailResult.BottleneckRestTileRestVzgroup0 += element * perf->archPerfDetailResult.BottleneckRestTileRestVzgroup0;
}

/***********************************************************************************
* Function:     _calc_cost (function layer_cost in Arch Model)
* Description:  Calculate the performance for one specific Op
* Input:        pArchNnConfig:
*               pArchOptions:
*               archModel:          Main struction of ArchModel
*               index:              Operation index
* Ouput:
***************************************************************************************/
static arch_float64 _calc_cost(
    arch_nn_config  *pArchNnConfig,
    arch_drv_option *pArchOptions,
    struct _archModelInfo *archModel,
    arch_int32      index,
    arch_uint32     x,
    arch_uint32     y,
    arch_uint32     z,
    arch_uint8      src_buf,
    arch_uint8      dst_buf,
    arch_uint8      kenerl_buf,
    arch_int32      cache_space,
    arch_uint32     trspIvLayerChsIn,
    arch_uint32     trspIvLayerChsOut
    )
{
    arch_status status;
    arch_perf perf = &archModel->opInfoArray[index]->perf;
    struct _archModelOpInfo * opInfo = archModel->opInfoArray[index];
    assert(index >= 0);

    // refine me, do we need fill following perf infomation here?
    perf->calculated = arch_false_e;
    perf->info.kx    = opInfo->kx;
    perf->info.ky    = opInfo->ky;
    perf->info.kz    = opInfo->kz;
    perf->info.inputDataFormat = opInfo->inputDataFormat;

    /*save original input x/y/z*/
    perf->info.oinx = opInfo->inx;
    perf->info.oiny = opInfo->iny;
    perf->info.oinz = opInfo->inz;
    perf->info.inx  = x; /* fix me: inx = x + kx - 1 */
    perf->info.iny  = y; /* fix me: iny = y + ky - 1 */
    perf->info.inz  = opInfo->kz;
    perf->info.outx = x;
    perf->info.outy = y;
    perf->info.outz = z;
    /* fix me, should always have stridex, stridey. Can not be 1 */
    perf->info.stridex = opInfo->stridex ? opInfo->stridex : 1;
    perf->info.stridey = opInfo->stridey ? opInfo->stridey : 1;
    perf->info.poolingSize   = opInfo->psize;
    perf->info.poolingStride = opInfo->pstride;
    perf->info.xOffSet = (-1) * opInfo->xpad;
    perf->info.yOffSet = (-1) * opInfo->ypad;
    perf->info.inputDataSize    = opInfo->inputDataSize;
    perf->info.outputDataSize   = opInfo->outputDataSize;
    perf->info.outputDataFormat = opInfo->outputDataFormat;
    perf->info.nnCores          = opInfo->nnCores;
    if (   (opInfo->target == ARCHNNE_OPERATION_TARGET_TP)
        && (opInfo->op == ARCHNNE_OPERATOR_FULLYCONNECTED))
    {
        perf->info.nnCores += pArchNnConfig->fixedFeature.tpliteCoreCount;
    }
    perf->swTilingInfo.origInX   = opInfo->calcinx;
    perf->swTilingInfo.origInY   = opInfo->calciny;
    perf->swTilingInfo.origOutX  = opInfo->origoutx;
    perf->swTilingInfo.origOutY  = opInfo->origouty;
    perf->swTilingInfo.origOutZ  = opInfo->oz;
    perf->swTilingInfo.srcBuf    = src_buf;
    perf->swTilingInfo.dstBuf    = dst_buf;
    perf->swTilingInfo.kernelBuf = kenerl_buf;
    perf->swTilingInfo.calcNonFirstCmd   = opInfo->fcmd ? arch_true_e : arch_false_e;
    perf->swTilingInfo.cacheSpace        = cache_space;
    perf->swTilingInfo.trspIvLayerChsIn  = trspIvLayerChsIn;
    perf->swTilingInfo.trspIvLayerChsOut = trspIvLayerChsOut;
    perf->swTilingInfo.swTilingSegKernelBufSizeInPixel = opInfo->swTilingSegKernelBufSizeInPixel;
    perf->swTilingInfo.segTotalBufferSizeInPixel       = opInfo->segTotalBufferSizeInPixel;
    perf->info.pix = opInfo->pix;
    perf->info.piy = opInfo->piy;
    perf->info.p3  = opInfo->p3;
    perf->info.nextKY = ((arch_uint32)(index + 1) < archModel->totalOpCount) ?
                        archModel->opInfoArray[index + 1]->ky : 0;

    /* Merge from CL 213817 */
    perf->upStreamLayerCount   = opInfo->upStreamLayerCount;
    perf->downStreamLayerCount = opInfo->downStreamLayerCount;
    /* first child AllSibling1x1 */
    if (   (opInfo->downStreamLayerCount > 0)
        && (opInfo->downStreamLayer[0] != -1))
    {
        perf->firstChildAllSibling1x1 = archModel->opInfoArray[opInfo->downStreamLayer[0]]->perf.allSibling1x1;
    }

    /* perf->info.kernelSize is the kernel size which calculated by driver, and used to reference to archModel_kernelSize */
    if (   (opInfo->target != ARCHNNE_OPERATION_TARGET_TP)
        || (opInfo->op == ARCHNNE_OPERATOR_FULLYCONNECTED))
    {
        perf->info.kernelSize = (arch_uint32)(archALIGN_NP2(0, CACHE_ALIGNMENT_SIZE));
    }

    /* refine me : is following if really usefull? */
    if (archIsFeatureAvailable(pArchNnConfig, pArchOptions, archModel->pArchDataFeature, ARCH_NN_FEATURE_SWTILING_PHASE1))
    {
        /* LSTM repeat frames: todo */
        arch_uint32 repeatePerFrame = 1;
        arch_uint32 cnum =   (arch_uint32)(ceilf((arch_float32)opInfo->xsize / x)
                           * ceilf((arch_float32)opInfo->ysize / y)
                           * ceilf((arch_float32)opInfo->oz / z) * repeatePerFrame);

        memset(&perf->resultInfo, 0, sizeof(arch_performance_info_s));
        status = archCalculateArchPerf(archModel->apm,
                                       pArchNnConfig,
                                       pArchOptions,
                                       archModel->pArchDataFeature,
                                       perf,
                                       opInfo->target,
                                       (   (opInfo->op == ARCHNNE_OPERATOR_ROIPOOL)
                                        && (opInfo->tpType == TP_ROI_POOLING_STEP_1)) ?
                                       ARCHNNE_OPERATOR_POOLING : opInfo->op
                                        );

        if (status != ARCH_SUCCESS)
        {
            return (arch_float64)MAX_COST;
        }

        if (perf->opTarget == ARCHNNE_OPERATION_TARGET_NN)
        {
            perf->resultInfo.perfCycleCount           += (cnum - 1) * perf->swTilingInfo.perfNonFirstCycleCount;
            perf->resultInfo.perfReadBandWidth        += (cnum - 1) * perf->swTilingInfo.perfNonFirstReadBandWidth;
            perf->resultInfo.perfWriteBandWidth       += (cnum - 1) * perf->swTilingInfo.perfNonFirstWriteBandWidth;
            perf->resultInfo.perfAXIReadBandWidth     += (cnum - 1) * perf->swTilingInfo.perfNonFirstAXIReadBandWidth;
            perf->resultInfo.perfAXIWriteBandWidth    += (cnum - 1) * perf->swTilingInfo.perfNonFirstAXIWriteBandWidth;
            perf->resultInfo.perfKernelReadBandWidth  += (cnum - 1) * perf->swTilingInfo.perfNonFirstKernelReadBandWidth;
            perf->resultInfo.perfInImageReadBandWidth += (cnum - 1) * perf->swTilingInfo.perfNonFirstInImageReadBandWidth;

            /* arch detail info process */
            archSWDetailProcess(perf, (cnum - 1));
        }
        else
        {
            perf->resultInfo.perfCycleCount     = cnum * perf->resultInfo.perfCycleCount;
            perf->resultInfo.perfReadBandWidth  = cnum * perf->resultInfo.perfReadBandWidth;
            perf->resultInfo.perfWriteBandWidth = cnum * perf->resultInfo.perfWriteBandWidth;
            perf->resultInfo.perfKernelReadBandWidth  += cnum * perf->swTilingInfo.perfNonFirstKernelReadBandWidth;
            perf->resultInfo.perfInImageReadBandWidth += cnum * perf->swTilingInfo.perfNonFirstInImageReadBandWidth;
            /* arch detail info process */
            //archSWDetailProcess(perf, cnum);
        }
    }
    else /* else of if sw_tiling phase1 */
    {
        memset(&perf->resultInfo, 0, sizeof(arch_performance_info_s));
        status = archCalculateArchPerf(archModel->apm,
                                       pArchNnConfig,
                                       pArchOptions,
                                       archModel->pArchDataFeature,
                                       perf,
                                       opInfo->target,
                                       (  opInfo->op == ARCHNNE_OPERATOR_ROIPOOL
                                        && opInfo->tpType == TP_ROI_POOLING_STEP_1) ?
                                        ARCHNNE_OPERATOR_POOLING : opInfo->op);

        if (status != ARCH_SUCCESS)
        {
            return (arch_float64)MAX_COST;
        }
    }

    /* calc imageIdealCacheSizeInPixel */
    _calcArchModelCacheMode(archModel->apm, pArchNnConfig, perf, &(perf->swTilingInfo.imageIdealCacheSizeInPixel));
    return perf->resultInfo.perfCycleCount;
}

/***********************************************************************************
* Function:       _kernel_size_in_pixel
* Description:    Calculate the kernel size in pixel
* Input:
*
* Ouput:
***************************************************************************************/
static arch_uint32 _kernel_size_in_pixel(
    archModelInfo *archModel,
    arch_int32 index,
    arch_uint32 cores,
    arch_bool fullCacheIntervalFix
    )
{
    struct _archModelOpInfo * opInfo = archModel->opInfoArray[index];
    arch_float64 coefCompressionRatio = opInfo->perf.coefCompressRatio;
    arch_float64 marginRatio = (1.25f - 1.05f) * (1.0f - archMIN(1,coefCompressionRatio)) / (1.0f - 0.02f) + 1.05f;

    coefCompressionRatio = opInfo->perf.coefCompressRatio;

    if (opInfo->op == ARCHNNE_OPERATOR_DEPTH_WISE_CONV)
    {
        /* Merge from CL 213850 */
        arch_bool isDepthWise = 1;
        if (fullCacheIntervalFix)
        {
            if(isDepthWise)
            {
                return (arch_uint32)ceil((opInfo->kx
                      * opInfo->ky
                      * opInfo->oz
                      * coefCompressionRatio * marginRatio)); /* 1.05 is added to give some margin for now) */
            }
            else
            {
                return (arch_uint32)ceil((opInfo->kx
                      * opInfo->ky
                      * opInfo->kz
                      * opInfo->oz
                      * coefCompressionRatio * marginRatio)); /* 1.05 is added to give some margin for now) */
            }
        }
        else
        {
            if(isDepthWise)
            {
                return (arch_uint32)ceil((opInfo->kx
                      * opInfo->ky
                      * ceilf((arch_float32)opInfo->oz / cores) * cores
                      * coefCompressionRatio * marginRatio));
            }
            else
            {
                return (arch_uint32)ceil((opInfo->kx
                      * opInfo->ky
                      * opInfo->kz
                      * ceilf((arch_float32)opInfo->oz / cores) * cores
                      * coefCompressionRatio * marginRatio));
            }
        }
    }

    if (opInfo->target != ARCHNNE_OPERATION_TARGET_TP || opInfo->op == ARCHNNE_OPERATOR_FULLYCONNECTED)
    {
        if (fullCacheIntervalFix)
        {
            return (arch_uint32)ceil((opInfo->kx
                      * opInfo->ky
                      * opInfo->kz
                      * opInfo->oz
                      * coefCompressionRatio * marginRatio));
        }
        else
        {
           return (arch_uint32)ceil((opInfo->kx
                      * opInfo->ky
                      * opInfo->kz
                      * ceilf((arch_float32)opInfo->oz / cores) * cores
                      * coefCompressionRatio * marginRatio));
        }
    }
    return 0;
}

/***********************************************************************************
* Function:        _outbuf_needed_ex
* Description:
* Input:
*
* Ouput:
***************************************************************************************/
static arch_uint32 _outbuf_needed_ex(
    struct _archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 psixArray[],
    arch_uint32 psiyArray[],
    arch_uint32 psizArray[],
    arch_uint32 trspIvLayerChsOut[]
    )
{
    arch_int32 outBufNeeded = 0, i;

    assert(segment_first >= 0);
    assert(segment_last >= 0);
    for (i = segment_first; i < segment_last; i++)
    {
        if (archModel->opInfoArray[i + 1]->target != ARCHNNE_OPERATION_TARGET_TP || (archModel->opInfoArray[i + 1]->op == ARCHNNE_OPERATOR_FULLYCONNECTED)) {
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
                    * archMIN((archMAX(
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
                * archMIN((archMAX(
                    archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i]->piy : psiyArray[i]),
                    archModel->opInfoArray[i]->bfy * ((psiyArray == NULL) ? archModel->opInfoArray[i + 1]->piy : psiyArray[i + 1])
                    * archModel->opInfoArray[i + 1]->pstride)
                    + archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1),
                    archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->piy)
                * (archModel->opInfoArray[i]->bfz * ((psizArray == NULL) ? archModel->opInfoArray[i]->siz : psizArray[i]) + 1 /*tpkz*/ - 1);
        }
    }

    /* reserve 16KB for Transpose output */
    if(trspIvLayerChsOut[segment_last] > 1 && G_JINBO_WR)
        outBufNeeded = outBufNeeded + RESERVE_16KB_FOR_TRANSPOSE;

    return outBufNeeded;
}

/* Calculate full cached space needed buffer */
static arch_uint32 _calc_full_cached_space_needed(
    archModelInfo *archModel,
    arch_uint32 segment_index,
    arch_uint32 psixArray[],
    arch_uint32 psiyArray[],
    arch_uint32 max_tile_size,
    arch_uint32 trspInterleaveCh
    )
{
    if (archModel->opInfoArray[segment_index]->target != ARCHNNE_OPERATION_TARGET_TP
       || (archModel->opInfoArray[segment_index]->op == ARCHNNE_OPERATOR_FULLYCONNECTED)
       )
    {
        if(archModel->pArchDataFeature->nnTranspose == 1)
        {
            /* using equation "2*N*Ceil(Ceil(KZ/N)*InTX*InTY/16)*16", where N = 16. see 8.26.2 in PRD v8 */
            return (arch_uint32)(2 * trspInterleaveCh * ceilf((archMIN(psixArray[segment_index] * archModel->opInfoArray[segment_index]->pstride, max_tile_size) + archModel->opInfoArray[segment_index]->kx - 1)
                * (psiyArray[segment_index] * archModel->opInfoArray[segment_index]->pstride + archModel->opInfoArray[segment_index]->ky - 1) * ceilf((arch_float32)archModel->opInfoArray[segment_index]->kz / trspInterleaveCh)) * 16);
        }
        else
        {
            return (archMIN(psixArray[segment_index] * archModel->opInfoArray[segment_index]->pstride, max_tile_size) + archModel->opInfoArray[segment_index]->kx - 1)
               * (psiyArray[segment_index] * archModel->opInfoArray[segment_index]->pstride + archModel->opInfoArray[segment_index]->ky - 1) * archModel->opInfoArray[segment_index]->kz;
        }
    }
    return 0;
}

/***********************************************************************************
* Function:       ComputeSubimageYSize/_calc_x_subimage (ComputeSubimageYSize)
* Description:    Calculate x/y subimage for SWTiling
* Input:
*
* Ouput:
***************************************************************************************/
static arch_bool ComputeSubimageYSize(
    struct _archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_int32 vip_sram_left,
    arch_int32 axi_sram_left,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
#ifdef CACHE_SPACE_ENABLE
    arch_uint32 vipSramRequired[],
    arch_bool axiSramOnlySWTiling,
#endif
    arch_uint32 trspIvLayerChsOut[],
    arch_uint32 max_tile_size)
{
    archSTATUS status = archSTATUS_OK;
    arch_uint32 *sixArray, *siyArray;
    arch_int32 i = 0;
    arch_float32 termA = 0, termB = 0;
    arch_int32 m, firstLayerInputCacheSize;
    arch_bool doSubImg = arch_false_e;
    arch_uint32 n = 0;
#ifdef CACHE_SPACE_ENABLE
    arch_uint32 finalBufferNeeded = 0;
#endif
    assert(segment_first >= 0);
    assert(segment_last >= 0);
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&sixArray);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: ComputeSubimageYSize() return out-of-memory\n");
        return arch_false_e;
    }

    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&siyArray);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: ComputeSubimageYSize() return out-of-memory\n");
        goto OnError;
    }

    n = getInputTransposeInterleaveCH(archModel, 0, segment_first);
    for (i = segment_first; i <= segment_last - 1; i++)
    {
        arch_float32 termAIn, termBIn;
        if (archModel->opInfoArray[i + 1]->target != ARCHNNE_OPERATION_TARGET_TP ||
            (archModel->opInfoArray[i + 1]->op == ARCHNNE_OPERATOR_FULLYCONNECTED))
        {
            if (archModel->opInfoArray[i]->target != ARCHNNE_OPERATION_TARGET_TP ||
            (archModel->opInfoArray[i]->op == ARCHNNE_OPERATOR_FULLYCONNECTED))
            {
                termAIn = (arch_float32)archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * archModel->opInfoArray[i + 1]->pstride * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
                termBIn = (arch_float32)archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * (archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i + 1]->kz;
            }
            else
            {
                termAIn = (arch_float32)archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * archModel->opInfoArray[i + 1]->pstride * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i]->oz;
                termBIn = (arch_float32)archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * (archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * archModel->opInfoArray[i]->bfz * archModel->opInfoArray[i]->oz;
            }
        }
        else
        {
            termAIn = (arch_float32)archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * archModel->opInfoArray[i + 1]->pstride * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);
            termBIn = (arch_float32)archModel->opInfoArray[i]->bfy * archModel->opInfoArray[i]->pix * (archModel->opInfoArray[i + 1]->p3 + archModel->opInfoArray[i + 1]->ky - 1) * (archModel->opInfoArray[i]->bfz * z_array[i] + 1 /*tpkz*/ - 1);
        }

        termA += termAIn;
        termB += termBIn;
    }

    /* Calculate largest M that fits */
    if ((axi_sram_left == 0) &&
        ((archModel->opInfoArray[segment_first]->target != ARCHNNE_OPERATION_TARGET_TP) ||
         (archModel->opInfoArray[segment_first]->op == ARCHNNE_OPERATOR_FULLYCONNECTED)))
    {

        arch_int32 termCalc = (archMIN(archModel->opInfoArray[segment_first]->xsize, max_tile_size) + archModel->opInfoArray[segment_first]->kx - 1);
        /*arch_int32 termD = (archMIN(archModel->opInfoArray[segment_first]->xsize, max_tile_size) + archModel->opInfoArray[segment_first]->kx - 1);*/
        if(archModel->pArchDataFeature->nnTranspose == 1)
        {
            arch_float32 termC = 0, termD = 0;
            /* substract 32KB from VIPSRAM for output Transpose */
            termC = RESERVE_16KB_FOR_TRANSPOSE *2 + (termB + 2 * 16 * termCalc * (archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * ceilf((arch_float32)archModel->opInfoArray[segment_first]->kz / 16));
            termD = termA + 2 * 16 * (termCalc * archModel->opInfoArray[segment_first]->pstride * ceilf((arch_float32)archModel->opInfoArray[segment_first]->kz / 16));
            m = (arch_int32)((vip_sram_left - termC) / termD);
       }
        else
        {
            arch_float32 termE = 0, termF = 0;
            termE = termB + termCalc * (archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz;
            termF = termA + termCalc * archModel->opInfoArray[segment_first]->pstride * archModel->opInfoArray[segment_first]->kz;
            m = (arch_int32)((vip_sram_left - termE) / termF);
        }

    }
    else if (axi_sram_left == 0)
    {
        m = (arch_int32)((vip_sram_left - termB) / termA);
    }
    else
    {
        m = (arch_int32)((axi_sram_left - termB) / termA);
    }

    /* find M that Y mod M = 0 */
    {
       arch_int32 min_m = (arch_int32)(192 / archModel->opInfoArray[segment_last]->pix);
       if (m > 0)
       {
           arch_int32 temp_m = m;
           while (((archModel->opInfoArray[segment_last]->piy % temp_m) > 0) && (temp_m > min_m))
           {
               temp_m = temp_m - 1;
           }
           m = (temp_m == min_m) ? archMAX(m, temp_m) : temp_m;
       }
    }
#if ENABLE_ARCH_MODEL_DUMP
    archInfo("M=%d\n", m);
#endif
    if (axi_sram_left > 0)
    {
        sixArray[segment_first] = archModel->opInfoArray[segment_first]->xsize;
        siyArray[segment_first] = archMIN(m, (arch_int32)archModel->opInfoArray[segment_first]->piy);
        firstLayerInputCacheSize = _calc_full_cached_space_needed(archModel, segment_first, sixArray, siyArray, max_tile_size, n);
        if (m > 0 && (firstLayerInputCacheSize + RESERVE_16KB_FOR_TRANSPOSE) < vip_sram_left)
        {
            doSubImg = arch_true_e;
        }
    }
    else
    {
        if (m > 0)
            doSubImg = arch_true_e;
    }

    if (doSubImg)
    {
        for (i = segment_first; i <= segment_last; i++)
        {
            sixArray[i] = archModel->opInfoArray[i]->xsize;
            siyArray[i] = archMIN((arch_uint32)m, archModel->opInfoArray[i]->piy);
        }

        for (i = segment_first; i <= segment_last; i++)
        {
            arch_int32 outbufNeeded;
            siyArray[i] = archMIN((arch_uint32)(m * 2), archModel->opInfoArray[i]->piy);
            outbufNeeded = _outbuf_needed_ex(archModel, segment_first, segment_last, sixArray, siyArray, z_array, trspIvLayerChsOut);
            firstLayerInputCacheSize = _calc_full_cached_space_needed(archModel, segment_first, sixArray, siyArray, max_tile_size,n);
            if ((axi_sram_left == 0 && vip_sram_left < (outbufNeeded + firstLayerInputCacheSize)) ||
                (axi_sram_left > 0 && ((axi_sram_left < outbufNeeded) || (vip_sram_left < firstLayerInputCacheSize))))
            {
                siyArray[i] = archMIN((arch_int32)m, (arch_int32)archModel->opInfoArray[i]->piy); /* put M back */
                break;
            }

#ifdef CACHE_SPACE_ENABLE
            finalBufferNeeded = outbufNeeded;
#endif
        }
    }
    /* generate SIY */
    for (i = segment_first; i <= segment_last; i++)
    {
        x_array[i] = archModel->opInfoArray[i]->xsize;
        if (doSubImg)
        {
            y_array[i] = archMIN(siyArray[i] * archModel->opInfoArray[i]->pstride + archModel->opInfoArray[i]->p3, archModel->opInfoArray[i]->ysize);
#ifdef CACHE_SPACE_ENABLE
            vipSramRequired[i] = finalBufferNeeded * (1 - axiSramOnlySWTiling);
#endif
        }
        else
        {
            y_array[i] = 0;
#ifdef CACHE_SPACE_ENABLE
            vipSramRequired[i] = 0;
#endif
        }
    }

    if (sixArray) archFreeMemory(sixArray);
    if (siyArray) archFreeMemory(siyArray);
    return arch_true_e;

OnError:
    if (sixArray) archFreeMemory(sixArray);
    if (siyArray) archFreeMemory(siyArray);
    return arch_false_e;
}

/* ComputeSubimageXSize */
#ifndef CACHE_SPACE_ENABLE
static arch_bool _calc_x_subimage(
    struct _archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_int32 sram_left,
    arch_uint32 ppsiy,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[])
{
    /* This routine compute the SubImageYSize assuming SubImageXSize = ImageXSize */
    arch_int32 termC = 0, termD = 0, termE = 0, termF = 0;
    arch_uint32 sumProdPSKX = archModel->opInfoArray[segment_last]->p3 + archModel->opInfoArray[segment_last]->kx - 1, prevProdPS = 0, prevSumProdPSKX = 0;
    arch_uint32 prodPoolStride = archModel->opInfoArray[segment_last]->pstride;
    arch_int32 i, n, /*ds = archModel->opInfoArray[segment_first]->dsize / 8, lcm = 1,*/ psix = 0;
    assert(segment_first >= 0);
    assert(segment_last >= 0);

    for (i = (arch_int32)segment_last - 1; i >= (arch_int32)segment_first; i--)
    {
        arch_int32 termCIn, termDIn;
        /*arch_uint32 d = archModel->opInfoArray[i]->target == archModel->opInfoArray[i - 1]->target ? 1 : 2;*/
        if (archModel->opInfoArray[i + 1]->target != ARCHNNE_OPERATION_TARGET_TP ||
            (archModel->opInfoArray[i + 1]->op == ARCHNNE_OPERATOR_FULLYCONNECTED))
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
    if (archModel->opInfoArray[segment_first]->target != ARCHNNE_OPERATION_TARGET_TP ||
        (archModel->opInfoArray[segment_first]->op == ARCHNNE_OPERATOR_FULLYCONNECTED))
    {
        n = archMAX((arch_int32)(1.0f * (sram_left - termF)/termE),
            (arch_int32)((sram_left - (arch_int32)(termD + (64 + archModel->opInfoArray[segment_first]->kx - 1) * (ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3 + archModel->opInfoArray[segment_first]->ky - 1) * archModel->opInfoArray[segment_first]->kz)) / termC));
    }
    else
    {
        n = (arch_int32)((sram_left - termD)/termC);
    }

    psix = n * prevProdPS + prevSumProdPSKX;
#if ENABLE_ARCH_MODEL_DUMP
    archInfo("N=%d, PSIX=%d, segment(%d, %d), PPSIY: %d\n", n, psix, segment_first + 1, segment_last + 1, ppsiy);
#endif
    x_array[segment_first] = archMIN(psix * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3, archModel->opInfoArray[segment_first]->xsize);
    x_array[segment_first] = (arch_uint32)(x_array[segment_first] / 64) * 64;
    y_array[segment_first] = ppsiy * archModel->opInfoArray[segment_first]->pstride + archModel->opInfoArray[segment_first]->p3;
    for (i = (arch_int32)(segment_first + 1); i <= (arch_int32)segment_last; i++)
    {
        psix = (arch_uint32)ceilf((arch_float32)(x_array[i-1] - archModel->opInfoArray[i - 1]->p3) / archModel->opInfoArray[i - 1]->pstride);
        if (n > 0)
        {
            x_array[i] = archMIN(psix - (archModel->opInfoArray[i]->kx - 1), archModel->opInfoArray[i]->xsize);
        }
        else
        {
            x_array[i] = 0;
        }

        y_array[i]  = ppsiy * archModel->opInfoArray[i]->pstride + archModel->opInfoArray[i]->p3;
    }

    return arch_true_e;
}
#endif

/* return sram space left size*/
/***********************************************************************************
* Function:       _calc_ab_buffer (ComputeABBuffer)
* Description:    Calculate AB buffer for AB segment
* Input:
*
* Ouput:
***************************************************************************************/
static arch_int32 ComputeABBuffer(
    struct _archModelInfo *archModel,
    arch_int32  segment_first,
    arch_int32  segment_last,
    arch_uint32 sram_space,
#ifdef CACHE_SPACE_ENABLE
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 vipSramRequired[],
    arch_bool   axiSramOnlySWTiling,
    arch_uint32 trspIvLayerChsOut[])
#else
    arch_uint32 x_array[], arch_uint32 y_array[])
#endif
{
    archSTATUS status = archSTATUS_OK;
    arch_uint32 abBufSize[2] = {0, 0}, outBufNeeded = 0, abBufPairSize = 0;
    arch_uint32 *sixArray, *siyArray;
    arch_int32 i, SramSpaceLeft, reserved_space_for_transpose = 0;
    assert(segment_first >= 0);
    assert(segment_last >= 0);

    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&sixArray);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: ComputeABBuffer() return out-of-memory\n");
        return 0;
    }
    assert(sixArray != NULL);
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&siyArray);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: ComputeABBuffer() return out-of-memory\n");
        goto OnError;
    }

    if(archModel->pArchDataFeature->nnTranspose == 1)
    {
        reserved_space_for_transpose = RESERVE_16KB_FOR_TRANSPOSE * 2;
    }
    else
    {
        reserved_space_for_transpose = 0;
    }

    for (i = segment_first; i <= segment_last; i++)
    {
        sixArray[i] = (arch_uint32)(ceilf((arch_float32)(archModel->opInfoArray[i]->xsize - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
        siyArray[i] = (arch_uint32)(ceilf((arch_float32)(archModel->opInfoArray[i]->ysize - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
    }

    for (i = segment_first; i <= segment_last - 1; i++)
    {
        outBufNeeded = _outbuf_needed_ex(archModel, i, i + 1, sixArray, siyArray, NULL, trspIvLayerChsOut);
        abBufSize[i % 2] = outBufNeeded;
        abBufPairSize = archMAX(abBufPairSize, abBufSize[0] + abBufSize[1]);
#ifdef CACHE_SPACE_ENABLE
        vipSramRequired[i] = (abBufSize[0] + abBufSize[1]) * (1 - axiSramOnlySWTiling);
#endif
    }
#ifdef CACHE_SPACE_ENABLE
    vipSramRequired[i] = abBufSize[1 - i%2] * (1 - axiSramOnlySWTiling);            /* Need to double check (i-1)%2 */
#endif
    SramSpaceLeft = (arch_int32)archMAX(-1,(arch_int32)(sram_space - abBufPairSize - reserved_space_for_transpose));
    if (SramSpaceLeft < 0)
    {
        for (i = segment_first; i <= segment_last; i++)
        {
            x_array[i] = 0;
            y_array[i] = 0;
#ifdef CACHE_SPACE_ENABLE
            vipSramRequired[i] = 0;
#endif
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

    if (sixArray) archFreeMemory(sixArray);
    if (siyArray) archFreeMemory(siyArray);
    return SramSpaceLeft;

OnError:
    if (sixArray) archFreeMemory(sixArray);
    if (siyArray) archFreeMemory(siyArray);
    return 0;
}

/***********************************************************************************
* Function:     _subimage_segment_cost
* Description:  Calculate the segment (from first to last) cost after split, will call _calc_cost for detail calc
* Input:        pArchNnConfig:
*               pArchOptions:
*               archModel:
*               segment_first:
*               segment_last:
* Ouput:        NULL
***************************************************************************************/
static void _subimage_segment_cost(
    arch_nn_config        *pArchNnConfig,
    arch_drv_option       *pArchOptions,
    struct _archModelInfo *archModel,
    arch_int32             segment_first,
    arch_int32             segment_last,
    arch_uint32            x_array[],
    arch_uint32            y_array[],
    arch_uint32            z_array[],
#ifdef CACHE_SPACE_ENABLE
    arch_uint32            cacheSpace[],
#endif
    arch_uint32            trspIvLayerChsIn[],
    arch_uint32            trspIvLayerChsOut[],
    arch_int32            *best_cost_swtiling_type,
    struct _archModelCost *cost
    )
{
    archSTATUS status = archSTATUS_OK;
    arch_int32 i, j;
    struct _archModelCost bestCost = {MAX_COST, MAX_COST};
    struct _archModelCost cur_cost = {MAX_COST, MAX_COST};
    arch_uint32 *xArray = NULL, *yArray = NULL, *zArray = NULL;
#ifdef CACHE_SPACE_ENABLE
    arch_uint32 *vipSramRequired = NULL, *curCacheSpace = NULL;
#endif
    /* arch_uint32 *curTrspIvLayerChsIn = NULL, *curTrspIvLayerChsOut = NULL; */
    arch_uint32 kernelBufNeeded = 0;
    arch_int32 vipSramSpaceSize = ARCH_VIP_SRAM_SIZE;
    arch_int32 axiSramSpaceSize = ARCH_AXI_SRAM_SIZE;
    arch_bool axiSramOnlySWTiling = pArchNnConfig->unifiedFeature.axiSramOnlySWTiling ? arch_true_e : arch_false_e;
    arch_uint32 sramSpaceSize = axiSramOnlySWTiling ? (arch_int32)axiSramSpaceSize : (arch_int32)vipSramSpaceSize;
#ifndef CACHE_SPACE_ENABLE
    arch_int32 abBufferSpaceLeft = 0;
    arch_uint32 sramLeft/*, ds = archModel->opInfoArray[segment_first]->dsize / 8*/;
#endif
    /*arch_bool fullCacheKernelHeadFix = pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix ? arch_true_e : arch_false_e;*/
    assert(segment_first >= 0);
    assert(segment_last >= 0);

    // refine me!
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&xArray);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _subimage_segment_cost(1) return out-of-memory\n");
        return;
    }
    memset(xArray, 0, sizeof(arch_uint32) * archModel->totalOpCount);
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&yArray);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _subimage_segment_cost(2) return out-of-memory\n");
        goto exit;
    }
    memset(yArray, 0, sizeof(arch_uint32) * archModel->totalOpCount);
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&zArray);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _subimage_segment_cost(3) return out-of-memory\n");
        goto exit;
    }
    memset(zArray, 0, sizeof(arch_uint32) * archModel->totalOpCount);
#ifdef CACHE_SPACE_ENABLE
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&curCacheSpace);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _subimage_segment_cost(4) return out-of-memory\n");
        goto exit;
    }
    memset(curCacheSpace, 0, sizeof(arch_uint32) * archModel->totalOpCount);

    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&vipSramRequired);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _subimage_segment_cost(4) return out-of-memory\n");
        goto exit;
    }
    memset(vipSramRequired, 0, sizeof(arch_uint32) * archModel->totalOpCount);
#endif

    if (segment_first == segment_last)
    {
        x_array[segment_first] = archModel->opInfoArray[segment_first]->xsize;
        y_array[segment_first] = archModel->opInfoArray[segment_first]->ysize;
        z_array[segment_first] = archModel->opInfoArray[segment_first]->oz;
        if(archModel->pArchDataFeature->nnTranspose == 1
            && /*archModel->opInfoArray[segment_first]->op != ARCHNNE_OPERATOR_TENSOR_ADD */
            !isTensorAdd(archModel->opInfoArray[segment_first]->target,archModel->opInfoArray[segment_first]->kz, z_array[segment_first])
            && archModel->opInfoArray[segment_first]->target != ARCHNNE_OPERATION_TARGET_TP)     /* NN_TRANSPOSE == 1 */
        {
#ifdef CACHE_SPACE_ENABLE
            cacheSpace[segment_first] = ARCH_VIP_SRAM_SIZE - RESERVE_16KB_FOR_TRANSPOSE * 3;      /* 16K for input and 32K for output */
#endif
            trspIvLayerChsIn[segment_first]  = getInputTransposeInterleaveCH(archModel,0, segment_first);
            trspIvLayerChsOut[segment_first] = getInputTransposeInterleaveCH(archModel, segment_first,archModel->opInfoArray[segment_first]->downStreamLayer[0]);
        }
        else
        {
            cacheSpace[segment_first] = ARCH_VIP_SRAM_SIZE;
            trspIvLayerChsIn[segment_first] = 1;
            trspIvLayerChsOut[segment_first] = 1;
        }
        archModel->opInfoArray[segment_first]->perf.info.flush = 1;
        cost->cycle = _calc_cost(
            pArchNnConfig,
            pArchOptions,
            archModel,
            segment_first,
            x_array[segment_first],
            y_array[segment_first],
            z_array[segment_first],
            SW_TILING_FROM_DDR,
            SW_TILING_FROM_DDR,
            SW_TILING_FROM_DDR,
#ifdef CACHE_SPACE_ENABLE
            cacheSpace[segment_first],
            trspIvLayerChsIn[segment_first],
            trspIvLayerChsOut[segment_first]);
#else
            vipSramSpaceSize);
#endif
        cost->bw = archModel->opInfoArray[segment_first]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[segment_first]->perf.resultInfo.perfWriteBandWidth;
        goto exit;
    }
    else
    {
        arch_bool hasUnsupportedTPLayer = arch_false_e;
        arch_uint32 tpCircularBuf = archIsFeatureAvailable(pArchNnConfig, pArchOptions,archModel->pArchDataFeature, ARCH_NN_FEATURE_SWTILING_PHASE2)? 1 : 0;
        for (i = segment_first; i <= segment_last; i++)
        {
            zArray[i] = archModel->opInfoArray[i]->oz;
        }

        for (i = segment_first; i <= segment_last; i++)
        {
            kernelBufNeeded += _kernel_size_in_pixel(archModel, i, archModel->opInfoArray[i]->nnCores, archModel->pArchDataFeature->fullCacheIntervalFix); /* Check bug 2033 */
            if ((archModel->opInfoArray[i]->target == ARCHNNE_OPERATION_TARGET_TP) && (tpCircularBuf == 0))
            {
                hasUnsupportedTPLayer = arch_true_e;
            }
            /* initialize Transpose Interleave Channels */
            trspIvLayerChsIn[i]  = 1;
            trspIvLayerChsOut[i] = 1;
        }

        /* Set transpose interleave channels in/out */
        if (archModel->pArchDataFeature->nnTranspose == 1)
        {
            trspIvLayerChsIn[segment_first] = getInputTransposeInterleaveCH(archModel,0,segment_first);
            trspIvLayerChsOut[segment_last] = getInputTransposeInterleaveCH(archModel,segment_last, archModel->opInfoArray[segment_last]->downStreamLayer[0]);
        }

        if (kernelBufNeeded <= sramSpaceSize || 1)
        {
            arch_int32 starti = 0, endi = 8;
            /* Merge from CL 213829 */
            arch_uint32 splitXDimension = 0;
            if (pArchOptions->enableSwtilingPhase1 == ARCH_SWTILING_OPTION_ALL) /*AB + SUBIMAGE for SWTiling*/
            {
                starti = 0;
                if(splitXDimension == 1)
                    endi = 8;
                else
                    endi = 1;
            }
            else if (pArchOptions->enableSwtilingPhase1 == ARCH_SWTILING_OPTION_AB)/*AB for SWTiling*/
            {
                starti = 0;
                endi = 0;
            }
            else if (pArchOptions->enableSwtilingPhase1 == ARCH_SWTILING_OPTION_TILING) /*SUBIMAGE for SWTiling*/
            {
                starti = 1;
                if(splitXDimension == 1)
                    endi = 8;
                else
                    endi = 1;
            }

            axiSramSpaceSize = axiSramOnlySWTiling ? axiSramSpaceSize : 0;
            vipSramSpaceSize = vipSramSpaceSize - kernelBufNeeded;
#ifndef CACHE_SPACE_ENABLE
            sramLeft = (arch_uint32)archMAX((arch_int32)(axiSramSpaceSize - kernelBufNeeded), (arch_int32)vipSramSpaceSize);
#endif
            for (i = starti; i <= endi; i++)
            {
                arch_float64 cost_cycle = 0;
                arch_float64 cost_bw = 0;
                if ((i == 0)
                    || ((i == 1) && (kernelBufNeeded < ARCH_VIP_SRAM_SIZE) && !hasUnsupportedTPLayer)
                    || ((i >= 3) && ((i % 1 /*LcmPSIYSmallUnit*/) == 0) && (kernelBufNeeded < ARCH_VIP_SRAM_SIZE) && !hasUnsupportedTPLayer))
                {
                    if (i == 0)
                    {
#ifdef CACHE_SPACE_ENABLE
                        ComputeABBuffer(archModel, segment_first, segment_last, sramSpaceSize, xArray, yArray, vipSramRequired,axiSramOnlySWTiling,trspIvLayerChsOut);
#else
                        abBufferSpaceLeft = ComputeABBuffer(archModel, segment_first, segment_last, sramSpaceSize, xArray, yArray);
#endif
                    }
                    else if (i == 1)
                    {
#ifdef CACHE_SPACE_ENABLE
                        ComputeSubimageYSize(archModel, segment_first, segment_last, vipSramSpaceSize, axiSramSpaceSize, xArray, yArray, zArray, vipSramRequired, axiSramOnlySWTiling, trspIvLayerChsOut, pArchNnConfig->unifiedFeature.maxTileSize);
#else
                        ComputeSubimageYSize(archModel, segment_first, segment_last, vipSramSpaceSize, axiSramSpaceSize, xArray, yArray, zArray, pArchNnConfig->unifiedFeature.maxTileSize);
#endif
                    }
                    else
                    {
                        /* Need to fix SramSpaceLeft */
#ifdef CACHE_SPACE_ENABLE
#else
                        _calc_x_subimage(archModel, segment_first, segment_last, sramLeft, i, xArray, yArray, zArray);
#endif
                    }

                    for (j = segment_first; j <= segment_last; j++)
                    {
                        arch_float64 c = 0;
                        arch_bool flush_and_wait = arch_false_e;

                        if ((segment_first + 1) == segment_last)
                        {
                            flush_and_wait = arch_true_e;
                        }
                        //archModel->opInfoArray[j]->perf.info.flush = 0;

                        if (i == 0)
                        {
                            flush_and_wait = 1;
                        }
                        else
                        {
                            if ((segment_first + 1) == segment_last)
                            {
                                if (j == segment_first )
                                {
                                    flush_and_wait = 1;
                                }
                                else if (j == segment_last )
                                {
                                    flush_and_wait = 0;
                                }
                            }
                            else
                            {
                                if ((j == segment_first) || (j == segment_last))
                                {
                                    flush_and_wait = 2;
                                }
                                else
                                {
                                    flush_and_wait = 0;
                                }
                            }
                        }

                        if (xArray[j] > 0 && yArray[j] > 0)
                        {
                            if (i == 0)
                            {
                                if (axiSramOnlySWTiling)
                                {
#ifdef CACHE_SPACE_ENABLE
                                    curCacheSpace[j] = ARCH_VIP_SRAM_SIZE;
#else
                                    abBufferSpaceLeft = ARCH_VIP_SRAM_SIZE;
#endif
                                }
#ifdef CACHE_SPACE_ENABLE
                                else
                                {
                                    curCacheSpace[j] = ARCH_VIP_SRAM_SIZE - vipSramRequired[j];
                                }
#endif
                                if (j == segment_first)
                                {
                                    archModel->opInfoArray[j]->perf.info.flush = flush_and_wait;
                                    c = _calc_cost(pArchNnConfig,
                                        pArchOptions,
                                        archModel,
                                        j,
                                        archModel->opInfoArray[j]->xsize,
                                        archModel->opInfoArray[j]->ysize,
                                        archModel->opInfoArray[j]->oz,
                                        SW_TILING_FROM_DDR,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
#ifdef CACHE_SPACE_ENABLE
                                        curCacheSpace[j],
                                        trspIvLayerChsIn[j],
                                        trspIvLayerChsOut[j]);
#else
                                        abBufferSpaceLeft);
#endif
                                }
                                else if (j == segment_last)
                                {
                                    c = _calc_cost(
                                        pArchNnConfig,
                                        pArchOptions,
                                        archModel,
                                        j,
                                        archModel->opInfoArray[j]->xsize,
                                        archModel->opInfoArray[j]->ysize,
                                        archModel->opInfoArray[j]->oz,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        SW_TILING_FROM_DDR,
#ifdef CACHE_SPACE_ENABLE
                                        curCacheSpace[j],
                                        trspIvLayerChsIn[j],
                                        trspIvLayerChsOut[j]);
#else
                                        abBufferSpaceLeft);
#endif
                                }
                                else
                                {
                                    c = _calc_cost(
                                        pArchNnConfig,
                                        pArchOptions,
                                        archModel,
                                        j,
                                        archModel->opInfoArray[j]->xsize,
                                        archModel->opInfoArray[j]->ysize,
                                        archModel->opInfoArray[j]->oz,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
#ifdef CACHE_SPACE_ENABLE
                                        curCacheSpace[j],
                                        trspIvLayerChsIn[j],
                                        trspIvLayerChsOut[j]);
#else
                                        abBufferSpaceLeft);
#endif
                                }
                            }
                            else
                            {
#ifdef CACHE_SPACE_ENABLE
                                if (axiSramOnlySWTiling)
                                {
                                    curCacheSpace[j] = ARCH_VIP_SRAM_SIZE - kernelBufNeeded;
                                }
                                else
                                {
                                    curCacheSpace[j] = ARCH_VIP_SRAM_SIZE - vipSramRequired[j] - kernelBufNeeded;
                                }
#endif
                                if (j == segment_first)
                                {
                                    archModel->opInfoArray[j]->perf.info.flush = flush_and_wait ? 1 : 0;
                                    c = _calc_cost(
                                        pArchNnConfig,
                                        pArchOptions,
                                        archModel,
                                        j,
                                        xArray[j],
                                        yArray[j],
                                        zArray[j],
                                        SW_TILING_FROM_DDR,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_VIP_SRAM,
#ifdef CACHE_SPACE_ENABLE
                                        curCacheSpace[j],
                                        trspIvLayerChsIn[j],
                                        trspIvLayerChsOut[j]);
#else
                                        ARCH_VIP_SRAM_SIZE);
#endif
                                }
                                else if (j == segment_last)
                                {
                                    c = _calc_cost(
                                        pArchNnConfig,
                                        pArchOptions,
                                        archModel,
                                        j,
                                        xArray[j],
                                        yArray[j],
                                        zArray[j],
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_DDR,
                                        SW_TILING_FROM_VIP_SRAM,
#ifdef CACHE_SPACE_ENABLE
                                        curCacheSpace[j],
                                        trspIvLayerChsIn[j],
                                        trspIvLayerChsOut[j]);
#else
                                        ARCH_VIP_SRAM_SIZE);
#endif
                                }
                                else
                                {
                                    c = _calc_cost(
                                        pArchNnConfig,
                                        pArchOptions,
                                        archModel,
                                        j,
                                        xArray[j],
                                        yArray[j],
                                        zArray[j],
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM,
                                        SW_TILING_FROM_VIP_SRAM,
#ifdef CACHE_SPACE_ENABLE
                                        curCacheSpace[j],
                                        trspIvLayerChsIn[j],
                                        trspIvLayerChsOut[j]);
#else
                                        ARCH_VIP_SRAM_SIZE);
#endif
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
#if ENABLE_ARCH_MODEL_DUMP
                            archInfo("calc_cost(%d, %d)(%d): %.7f, readBW: %.7f, writeBW: %.7f\n", segment_first + 1, segment_last + 1, j + 1, c, archModel->opInfoArray[j]->perf.resultInfo.perfReadBandWidth, archModel->opInfoArray[j]->perf.resultInfo.perfWriteBandWidth);
#endif
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
                    archInfo("bestCost: %llu, bestBW: %llu, cost(%d, %d): %llu, bw: %llu\n", (arch_uint64)bestCost.cycle, (arch_uint64)bestCost.bw, segment_first + 1, segment_last + 1, (arch_uint64)cost_cycle, (arch_uint64)cost_bw);
#endif
                    if (_cur_cost_is_more_better(&bestCost, &cur_cost, CYCLE_WEIGHT, BW_WEIGHT))
                    {
                        bestCost.cycle = cur_cost.cycle;
                        bestCost.bw = cur_cost.bw;
                        for (int j = segment_first; j <= segment_last; j++)
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
#ifdef CACHE_SPACE_ENABLE
                            cacheSpace[j] = curCacheSpace[j];
                            /*
                            trspIvLayerChsIn[j] = curTrspIvLayerChsIn[j];
                            trspIvLayerChsOut[j] = curTrspIvLayerChsOut[j];
                            */
#endif
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
        archFreeMemory(xArray);

    if (yArray)
        archFreeMemory(yArray);

    if (zArray)
        archFreeMemory(zArray);

#ifdef CACHE_SPACE_ENABLE
    if(curCacheSpace)
        archFreeMemory(curCacheSpace);

    if(vipSramRequired)
        archFreeMemory(vipSramRequired);
#endif

    return;
}


/***********************************************************************************
* Function:     getUpstreamLayer
* Description:  Get upstream information for spcific Op
* Input:        archModel:
*               index:
* Ouput:        upstream:
***************************************************************************************/
static void getUpstreamLayer(
    struct _archModelInfo *archModel,
    arch_uint32 index,
    arch_uint32 id,
    arch_int32 *upstream
    )
{
    assert(upstream != NULL);
    if (id < archModel->opInfoArray[index]->upStreamLayerCount)
    {
        *upstream = archModel->opInfoArray[index]->upStreamLayer[id];
    }
    else
    {
        *upstream = -1;
    }
}

/***********************************************************************************
* Function:     getDownstreamLayer
* Description:  Get downstream information for spcific Op
* Input:        archModel:
*               index:
* Ouput:        downstream:
***************************************************************************************/
static void getDownstreamLayer(
    struct _archModelInfo *archModel,
    arch_uint32 index,
    arch_uint32 id,
    arch_int32 *downstream
    )
{
    assert(downstream != NULL);
    if (id < archModel->opInfoArray[index]->downStreamLayerCount)
    {
        *downstream = archModel->opInfoArray[index]->downStreamLayer[id];
    }
    else
    {
        *downstream = -1;
    }
}

/***********************************************************************************
* Function:     _split_segment_loop
* Description:  Loop to split segment from first to last
* Input:        pArchNnConfig:
*               pArchOptions:
*               archModel:
*               segment_first:
*               segment_last:
* Ouput:        downstream:
***************************************************************************************/
static void _split_segment_loop(
    struct _archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint32 cacheSpace[],
    arch_uint32 trspIvLayerChsIn[],
    arch_uint32 trspIvLayerChsOut[]
    )
{
    struct _archModelCost cost, upcost, downcost;
    struct _archModelCost cur_segment_cost = {MAX_COST, MAX_COST};
    arch_int32 len_segment, cur_pos, temp_pos;
#if ENABLE_ARCH_MODEL_DUMP
    arch_int32 debug = 0;
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
                arch_int32 split_pos;
                for (split_pos = cur_pos; split_pos <= (cur_pos + len_segment); split_pos++)
                {
                    if ((archModel->opInfoArray[split_pos]->target != ARCHNNE_OPERATION_TARGET_TP ||
                        archModel->opInfoArray[split_pos]->op != ARCHNNE_OPERATOR_FULLYCONNECTED)
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
                            archInfo("split_pos: %d, old_cost: %.7f, old_bw: %.7f, cur_cost: %.7f, cur_bw: %.7f\n", split_pos + 1, cost.cycle, cost.bw, cur_segment_cost.cycle, cur_segment_cost.bw);
                        }
#endif
                        if (_cur_cost_is_more_better(&cost, &cur_segment_cost, CYCLE_WEIGHT, BW_WEIGHT))
                        {
#if ENABLE_ARCH_MODEL_DUMP
                            archInfo("found better cost: split_pos: %d, cur_cost: %.7f, cur_bw: %.7f\n", split_pos + 1, cur_segment_cost.cycle, cur_segment_cost.bw);
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
#ifdef CACHE_SPACE_ENABLE
                                            archModel->splitInfoArray[cur_pos % MAX_LAYERS_OF_BLOCK]->savedCacheSpace[(split_pos - 1)  % MAX_LAYERS_OF_BLOCK][temp_pos  % MAX_LAYERS_OF_BLOCK],
#endif
                                            archModel->splitInfoArray[cur_pos % MAX_LAYERS_OF_BLOCK]->savedTrspIvLayerChsIn[(split_pos - 1)  % MAX_LAYERS_OF_BLOCK][temp_pos  % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[cur_pos % MAX_LAYERS_OF_BLOCK]->savedTrspIvLayerChsOut[(split_pos - 1)  % MAX_LAYERS_OF_BLOCK][temp_pos  % MAX_LAYERS_OF_BLOCK],
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
#ifdef CACHE_SPACE_ENABLE
                                            archModel->splitInfoArray[split_pos % MAX_LAYERS_OF_BLOCK]->savedCacheSpace[(cur_pos + len_segment) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK],
#endif
                                            archModel->splitInfoArray[split_pos % MAX_LAYERS_OF_BLOCK]->savedTrspIvLayerChsIn[(cur_pos + len_segment) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK],
                                            archModel->splitInfoArray[split_pos % MAX_LAYERS_OF_BLOCK]->savedTrspIvLayerChsOut[(cur_pos + len_segment) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK],
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
#ifdef CACHE_SPACE_ENABLE
        cacheSpace[temp_pos] = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCacheSpace[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
#endif
        trspIvLayerChsIn[temp_pos] = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedTrspIvLayerChsIn[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
        trspIvLayerChsOut[temp_pos] = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedTrspIvLayerChsOut[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
        /* to save best cost */
        archModel->opInfoArray[temp_pos]->perf.resultInfo.perfCycleCount = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCost[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK].cycle;
        archModel->opInfoArray[temp_pos]->perf.resultInfo.perfWriteBandWidth = archModel->splitInfoArray[segment_first % MAX_LAYERS_OF_BLOCK]->savedCost[segment_last % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK].bw;
        archModel->opInfoArray[temp_pos]->perf.resultInfo.perfReadBandWidth = 0;
    }
}

/***********************************************************************************
* Function:     _split_segment
* Description:  Split the graph to segment, will call _split_segment_loop for detail process
* Input:        pArchNnConfig:
*               pArchOptions:
*               archModel:
*               segment_first:
*               segment_last:
* Ouput:        downstream:
***************************************************************************************/
static void _split_segment(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    struct _archModelInfo *archModel,
    arch_int32 segment_first,
    arch_int32 segment_last,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
#ifdef CACHE_SPACE_ENABLE
    arch_uint32 cacheSpace[],
#endif
    arch_uint32 trspIvLayerChsIn[],
    arch_uint32 trspIvLayerChsOut[],
    arch_uint8 split_array[]
    )
{
    arch_int32 i, j;
    archModelCost cost = {MAX_COST, MAX_COST};
    arch_int32 split_pos = 0;
    /*arch_bool fullCacheKernelHeadFix = pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix ? arch_true_e : arch_false_e;*/
    arch_bool axiSramOnlySWTiling = pArchNnConfig->unifiedFeature.axiSramOnlySWTiling ? arch_true_e : arch_false_e;
    arch_int32 first, original_first = segment_first, prev_split = segment_first;
    arch_int32 temp_pos, totalSegments = segment_last -  segment_first + 1;
    arch_uint32 * cur_xArray = NULL;
    arch_uint32 * cur_yArray = NULL;
    arch_uint32 * cur_zArray = NULL;

#ifdef CACHE_SPACE_ENABLE
    /* Merge from CL 213830 */
    arch_uint32 * curCacheSpace = NULL;
#endif

    /* NN transpose */
    arch_uint32 * curTrspIvLayerChsIn = NULL;
    arch_uint32 * curTrspIvLayerChsOut = NULL;

    assert(segment_first >= 0);
    assert(segment_last >= 0);

    if (segment_first != segment_last)
    {
        for (split_pos = segment_first + 1; split_pos != segment_last; split_pos++)
        {
            arch_bool connected = arch_false_e;
            arch_int32 downstream_pos;
            for (downstream_pos = split_pos; downstream_pos != segment_last; downstream_pos++)
            {
                for (i = 0; i < (arch_int32)archModel->opInfoArray[downstream_pos]->upStreamLayerCount; i++)
                {
                    arch_int32 upStreamLayer;
                    getUpstreamLayer(archModel, downstream_pos, i, &upStreamLayer);
                    if ((upStreamLayer >= segment_first) && (upStreamLayer < split_pos))
                    {
                        connected = arch_true_e;
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
        arch_int32 upStreamLayer;
        arch_int32 sramForSWTiling = axiSramOnlySWTiling? ARCH_AXI_SRAM_SIZE: ARCH_VIP_SRAM_SIZE;
        for (split_pos = segment_first + 1; split_pos <= segment_last; split_pos++)
        {
            arch_uint32 size1, size2, outbufNeeded;

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
            else
            {
                split_array[split_pos] = 1;
                continue;
            }

            if (   (   archModel->opInfoArray[split_pos]->target == ARCHNNE_OPERATION_TARGET_TP
                    && archModel->opInfoArray[split_pos]->op == ARCHNNE_OPERATOR_FULLYCONNECTED)
                || (   archModel->opInfoArray[split_pos - 1]->target == ARCHNNE_OPERATION_TARGET_TP
                    && archModel->opInfoArray[split_pos - 1]->op == ARCHNNE_OPERATOR_FULLYCONNECTED)
                )
            {
                split_array[split_pos] = 1;
                continue;
            }
            size1 = _kernel_size_in_pixel(archModel, split_pos, archModel->opInfoArray[split_pos]->nnCores, archModel->pArchDataFeature->fullCacheIntervalFix);
            size2 = _kernel_size_in_pixel(archModel, split_pos - 1, archModel->opInfoArray[split_pos - 1]->nnCores, archModel->pArchDataFeature->fullCacheIntervalFix);
            outbufNeeded = _outbuf_needed_ex(archModel, split_pos - 1, split_pos, NULL, NULL, NULL, trspIvLayerChsOut);

            if ((size1 + size2 > ARCH_VIP_SRAM_SIZE) && ((arch_int32)outbufNeeded > sramForSWTiling))
            {
                split_array[split_pos] = 1;
                continue;
            }
        }
    }

#if ENABLE_ARCH_MODEL_DUMP
    archInfo("\n++init is_split==\n");
    for (i = 0; i <= (segment_last - segment_first); i++)
    {
        archInfo("layer[%d]: %d\n", i+1, split_array[i]);
    }
    archInfo("--init is_split==\n");
#endif

    for (i = segment_first; i <= (segment_last + 1); i++)
    {
        arch_uint32 arraySize = sizeof(arch_uint32) * archModel->totalOpCount;
        if (cur_xArray == NULL)
            cur_xArray = (arch_uint32 *)archAllocateAndZeroMemory(arraySize);
        else
            memset(cur_xArray, 0, arraySize);

        if (cur_yArray == NULL)
            cur_yArray = (arch_uint32 *)archAllocateAndZeroMemory(arraySize);
        else
            memset(cur_yArray, 0, arraySize);

        if (cur_zArray == NULL)
            cur_zArray = (arch_uint32 *)archAllocateAndZeroMemory(arraySize);
        else
            memset(cur_zArray, 0, arraySize);

#ifdef CACHE_SPACE_ENABLE
        /* Merge from CL 213830 */
        if (curCacheSpace == NULL)
            curCacheSpace = (arch_uint32 *)archAllocateAndZeroMemory(arraySize);
        else
            memset(curCacheSpace, 0, arraySize);

        assert(cur_xArray != NULL && cur_yArray != NULL && cur_zArray != NULL && curCacheSpace != NULL);
#else
        assert(cur_xArray != NULL && cur_yArray != NULL && cur_zArray != NULL);
#endif
        /* NN transpose */
        if (curTrspIvLayerChsIn == NULL)
            curTrspIvLayerChsIn = (arch_uint32 *)archAllocateAndZeroMemory(arraySize);
        else
            memset(curTrspIvLayerChsIn, 0, arraySize);

        if (curTrspIvLayerChsOut == NULL)
            curTrspIvLayerChsOut = (arch_uint32 *)archAllocateAndZeroMemory(arraySize);
        else
            memset(curTrspIvLayerChsOut, 0, arraySize);

        if (split_array[i])
            /*|| (i == (segment_last + 1)))*/
        {
            arch_int32 last = i - 1;
            first = original_first;
            if (last >= first)
            {
                while (last >= first)
                {
                    while (first <= last)
                    {
                        arch_int32 bestCostSWTilingType = -1;
#ifdef CACHE_SPACE_ENABLE
                        /* trspIvLayerChsIn and trspIvLayerChsOut no need to save the best */
                        _subimage_segment_cost(pArchNnConfig, pArchOptions, archModel, first, last,
                                               cur_xArray,cur_yArray,cur_zArray, curCacheSpace,
                                               curTrspIvLayerChsIn, curTrspIvLayerChsOut,
                                               &bestCostSWTilingType, &cost);
#else
                        _subimage_segment_cost(pArchNnConfig, pArchOptions, archModel, first, last,
                                               cur_xArray,cur_yArray,cur_zArray,
                                               &bestCostSWTilingType, &cost);
#endif
#if ENABLE_ARCH_MODEL_DUMP
                        archInfo("++_subimage_segment_cost(%d, %d)=%.7f, bw: %.7f\n", first + 1, last + 1, cost.cycle, cost.bw);
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
#ifdef CACHE_SPACE_ENABLE
                                curCacheSpace[temp_pos],
#endif
                                curTrspIvLayerChsIn[temp_pos],
                                curTrspIvLayerChsOut[temp_pos],
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

    if (cur_xArray) archFreeMemory(cur_xArray);
    if (cur_yArray) archFreeMemory(cur_yArray);
    if (cur_zArray) archFreeMemory(cur_zArray);
#ifdef CACHE_SPACE_ENABLE
    if (curCacheSpace)    archFreeMemory(curCacheSpace);
#endif
    if (curTrspIvLayerChsIn) archFreeMemory(curTrspIvLayerChsIn);
    if (curTrspIvLayerChsOut) archFreeMemory(curTrspIvLayerChsOut);

    for (i = segment_first; i <= (segment_last + 1); i++)
    {
        if ((split_array[i] || (i == (segment_last + 1))) && (i > segment_first))
        {
            arch_int32 bestCostSWTilingType = getBestCostSWTilingTypeInfo(archModel, prev_split, i - 1);
            arch_bool alwaysSplit = arch_true_e;
            if (bestCostSWTilingType != 1 || alwaysSplit)
            {
                arch_int32 segStart, segEnd;
#ifdef CACHE_SPACE_ENABLE
                 _split_segment_loop(archModel, prev_split, i - 1, x_array, y_array, z_array,cacheSpace,trspIvLayerChsIn,trspIvLayerChsOut);
#else
                _split_segment_loop(archModel, prev_split, i - 1, x_array, y_array, z_array);
#endif
#if ENABLE_ARCH_MODEL_DUMP
                archInfo("++_split_segment_loop(%d, %d)\n", prev_split + 1, i - 1 + 1);
#endif
                for (temp_pos = prev_split; temp_pos <= (i - 1); temp_pos++)
                {
                    if (archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->split_array[(i - 1) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK]
                        && (archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->bestCostSWTilingType[(i - 1) % MAX_LAYERS_OF_BLOCK] != 1)
                        && (temp_pos != prev_split))
                    {
                        split_array[temp_pos] = archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->split_array[(i - 1) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
#if ENABLE_ARCH_MODEL_DUMP
                        archInfo("update _split_array[%d]: %d\n", temp_pos + 1, split_array[temp_pos]);
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
            else
            {
                for (temp_pos = prev_split; temp_pos <= (i - 1); temp_pos++)
                {
                    x_array[temp_pos] = archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->savedSIX[(i - 1) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
                    y_array[temp_pos] = archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->savedSIY[(i - 1) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
                    z_array[temp_pos] = archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->savedSIZ[(i - 1) % MAX_LAYERS_OF_BLOCK][temp_pos % MAX_LAYERS_OF_BLOCK];
#ifdef CACHE_SPACE_ENABLE
                    /* set value for cacheSpace */
#endif
                    archModel->opInfoArray[temp_pos]->swTilingType = archModel->splitInfoArray[prev_split % MAX_LAYERS_OF_BLOCK]->bestCostSWTilingType[(i - 1) % MAX_LAYERS_OF_BLOCK];
                }
            }
            prev_split = i;
        }
    }
#if ENABLE_ARCH_MODEL_DUMP
    archInfo("++split_end is_split==\n");
    for (i = 0; i <= (segment_last - segment_first); i++)
    {
        archInfo("layer[%d]: %d\n", i+1, split_array[i]);
    }
    archInfo("--split_end is_split==\n");
#endif
}


/***********************************************************************************
* Function:     _seg_buf_needed
* Description:  Find out the segment buffer needed
* Input:        archModel:
*               segment_first:
*               segment_last:
* Ouput:        :
***************************************************************************************/
static arch_uint32 _seg_buf_needed(
    struct _archModelInfo *archModel,
    arch_uint32 segment_first,
    arch_uint32 segment_last,
    arch_uint32 sixArray[],
    arch_uint32 siyArray[],
    arch_uint32 z_array[],
    arch_uint32 trspIvLayerChsOut[])
{
    arch_uint32 segBufNeeded = 0;
    if (segment_first == segment_last)
    {
        segBufNeeded = 0;
    }
    else
    {
        arch_bool allTypeABBuf = arch_true_e;
        arch_uint32 i;
        for (i = segment_first; i <= (segment_last - 1); i++)
        {
            if (archModel->opInfoArray[i]->swTilingType == 0) /*0: SUB-IMG, 1: AB*/
            {
                allTypeABBuf = arch_false_e;
            }
        }
        if (allTypeABBuf)
        {
            arch_uint32 abBufSize[2] = {0, 0};
            arch_uint32 abBufPairSize = 0;
            for (i = segment_first; i <= (segment_last - 1); i++)
            {
                abBufSize[i % 2] = _outbuf_needed_ex(archModel, i, i+1, sixArray, siyArray, z_array, trspIvLayerChsOut);
                abBufPairSize = archMAX(abBufPairSize, abBufSize[0] + abBufSize[1]);
            }
            segBufNeeded = abBufPairSize;
        }
        else
        {
            segBufNeeded = _outbuf_needed_ex(archModel, segment_first, segment_last, sixArray, siyArray, z_array, trspIvLayerChsOut);
        }
    }
    return segBufNeeded;
}

/************************************************  GIB related definition *******************************************/

static arch_bool _gib_io_overlap(
    archGIBIO *gibIO,
    arch_uint32 gib,
    struct _archModelInfo *archModel,
    arch_uint32 layer)
{
    arch_uint32 i, j;
    for (i = 0; i < gibIO[gib].gib_input_count; i++)
    {
        for (j = 0; j < archModel->opInfoArray[layer]->upStreamLayerCount; j++)
        {
            arch_int32 upStreamLayer;
            getUpstreamLayer(archModel, layer, j, &upStreamLayer);
            if (upStreamLayer > 0)
            {
                if (gibIO[gib].gib_input[i] == upStreamLayer)
                {
                    return arch_true_e;
                }
            }
        }
    }

    return arch_false_e;
}

static void _append_gib_layer(
    archGIBIO *gibIO,
    arch_uint32 gib,
    arch_int32 layer,
    arch_bool input)
{
    arch_bool  present = arch_false_e;
    arch_uint32 i;
    arch_uint32 count = input ? gibIO[gib].gib_input_count : gibIO[gib].gib_output_count;
    arch_int32 *target = input ? gibIO[gib].gib_input : gibIO[gib].gib_output;
    for (i = 0; i < count; i++)
    {
        if (target[i] == layer)
        {
            present = arch_true_e;
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

static void _merge_gib_io(
    archGIBIO *gibIO,
    arch_uint32 gib,
    struct _archModelInfo *archModel,
    arch_uint32 layer)
{
    arch_uint32 i;
    arch_int32 upStreamLayer;
    for (i = 0; i < archModel->opInfoArray[layer]->upStreamLayerCount; i++)
    {
        getUpstreamLayer(archModel, layer, i, &upStreamLayer);
        if (upStreamLayer > 0)
        {
            _append_gib_layer(gibIO, gib, upStreamLayer, arch_true_e);
        }
    }
}

static arch_uint32 _create_gib(
    struct _archModelInfo *archModel,
    arch_uint32 count,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
    arch_uint32 trspIvLayerChsOut[],
    arch_uint8 split_array[],
    archGIBIO *gib_io,
    archGIBObj *gib_obj)
{
    arch_int32 i, j, end_index;
    arch_uint32 gib, gib_last = 0;
    arch_bool *layerChecked = NULL;
    arch_uint32 segmentBufferNeeded, id;
    arch_uint32 *sixArray = NULL, *siyArray = NULL;
    archSTATUS status = archSTATUS_OK;
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&sixArray);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _create_gib(1) return out-of-memory\n");
        return 0;
    }
    memset(sixArray, 0, sizeof(arch_uint32) * archModel->totalOpCount);

    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount, (archPOINTER *)&siyArray);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _create_gib(2) return out-of-memory\n");
        goto OnError;
    }
    memset(siyArray, 0, sizeof(arch_uint32) * archModel->totalOpCount);

    status = archAllocateMemory(sizeof(arch_bool) * archModel->totalOpCount,(archPOINTER *)&layerChecked);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _create_gib(3) return out-of-memory\n");
        goto OnError;
    }
    memset(layerChecked, 0, sizeof(arch_bool) * archModel->totalOpCount);

    for (i = 0; i < (arch_int32)count; i++)
    {
        sixArray[i] = (arch_uint32)(ceilf((arch_float32)(x_array[i] - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
        siyArray[i] = (arch_uint32)(ceilf((arch_float32)(y_array[i] - archModel->opInfoArray[i]->p3) / archModel->opInfoArray[i]->pstride));
        layerChecked[i] = arch_false_e;
    }

    id = 0;
    end_index = count - 1;
    if (archModel->opInfoArray[0]->target == ARCHNNE_OPERATION_TARGET_NN || archModel->opInfoArray[0]->target  == ARCHNNE_OPERATION_TARGET_TP)
    {
        split_array[0] = 1;
    }
    for (i = count - 1; i >= 0; i--)
    {
        if (split_array[i])
        {
            if (i == end_index)
            {
                segmentBufferNeeded = 0;
            }
            else
            {
                segmentBufferNeeded = _seg_buf_needed(archModel, i, end_index, sixArray, siyArray, z_array, trspIvLayerChsOut);
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
            layerChecked[0] = arch_true_e;
        }
        gib_last = gib;
        if (sixArray) archFreeMemory(sixArray);
        if (siyArray) archFreeMemory(siyArray);
        if (layerChecked) archFreeMemory(layerChecked);
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
                for (j = 0; j < (arch_int32)archModel->opInfoArray[i]->upStreamLayerCount; j++)
                {
                    arch_int32 upStreamLayer;
                    getUpstreamLayer(archModel, i, j, &upStreamLayer);
                    gib_io[gib].gib_input[j] = upStreamLayer;
                }
                gib_io[gib].gib_output_count = 1;
                gib_io[gib].gib_output[0] = i;

                for (j = (i - 1); j >= 0; j--)
                {
                    if (split_array[j])
                    {
                        if (archModel->opInfoArray[j]->target != ARCHNNE_OPERATION_TARGET_SH &&
                           archModel->opInfoArray[j]->target != ARCHNNE_OPERATION_TARGET_SW)
                        {
                            if (_gib_io_overlap(gib_io, gib, archModel, j))
                            {
                                _merge_gib_io(gib_io, gib, archModel, j);
                                _append_gib_layer(gib_io, gib, j, arch_false_e);
                                layerChecked[j] = arch_true_e;
                            }
                        }
                    }
                }
                gib++;
            }
        }
    }
    if (sixArray) archFreeMemory(sixArray);
    if (siyArray) archFreeMemory(siyArray);
    if (layerChecked) archFreeMemory(layerChecked);
    return gib_last;

OnError:
    if (sixArray)
        archFreeMemory(sixArray);

    if (siyArray)
        archFreeMemory(siyArray);

    if (layerChecked)
        archFreeMemory(layerChecked);
    return 0;
}

/***********************************************************************************
* Function:     _merge_sub_graph
* Description:  Check if the sub graph can be merged. If so, merge to reduce cost
* Input:        archModel:
* Ouput:        :
***************************************************************************************/
static void _merge_sub_graph(
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    struct _archModelInfo *archModel,
    arch_uint32 count,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
    arch_uint32 z_array[],
#ifdef CACHE_SPACE_ENABLE
    arch_uint32 cacheSpace[],
#endif
    arch_uint32 curTrspIvLayerChsIn[],
    arch_uint32 curTrspIvLayerChsOut[],
    arch_uint8 split_array[],
    archGIBIO *gib_io,
    archGIBObj *gib_obj,
    arch_uint32 gib_last)
{
    arch_uint32 gib, j, k;
    arch_bool *gib_input_checked = NULL;
    arch_bool axiSramOnlySWTiling = pArchNnConfig->unifiedFeature.axiSramOnlySWTiling;
    archSTATUS status = archSTATUS_OK;
    arch_int32 *curCacheSpace = NULL;
    /* temp for error */
    /*arch_uint32 tempLayer = 0;*/
    /*arch_float64 tempReadBw = 0, tempWriteBw = 0;*/

    status = archAllocateMemory(sizeof(arch_bool) * archModel->totalOpCount,(archPOINTER *)&gib_input_checked);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _merge_sub_graph(1) return out-of-memory\n");
        return;
    }
    memset(gib_input_checked, 0, sizeof(arch_bool) * archModel->totalOpCount);

    status = archAllocateMemory(sizeof(arch_int32) * archModel->totalOpCount,(archPOINTER *)&curCacheSpace);
    if (archIS_ERROR(status))
    {
        archInfo("ERROR: _merge_sub_graph(2) return out-of-memory\n");
        if (gib_input_checked) archFreeMemory(gib_input_checked);
        return;
    }
    memset(curCacheSpace, 0, sizeof(arch_int32) * archModel->totalOpCount);



    for (gib = 0; gib <= gib_last; gib++)
    {
        arch_bool anyLayerExternal = arch_false_e;
        arch_uint32 outputSubimageSegmentCount = 0, inputSubimageSegmentCount = 0, outputABSegmentCount = 0, inputABSegmentCount = 0;
        struct _archModelCost baseCost = {0,0}, curCost = {0,0};

        /* copy cache space */
        memcpy(curCacheSpace, cacheSpace, sizeof(arch_int32) * archModel->totalOpCount);

        for (j = 0; j < gib_io[gib].gib_input_count; j++)
        {
            arch_int32 layer = gib_io[gib].gib_input[j];
            if (layer == -1)
            {
                anyLayerExternal = arch_true_e;
                break;
            }
        }
        if (anyLayerExternal || gib_io[gib].gib_input_count == 0)
        {
            continue;
            /*Graph from Excel/log will have gib_input_count = 0 for the first layer. Need to check and skip this case, otherwise there will be errors while looping gib_input_count*/
        }

        for (j = 0; j < gib_io[gib].gib_output_count; j++)
        {
            arch_uint32 layer = gib_io[gib].gib_output[j];
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
            archInfo("gib_io[%d, %d].output, cost.cycle(%d): %llu, cost.bw(%d): %llu\n", gib+1, j + 1, layer + 1, (arch_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount + 0.5f), layer + 1, (arch_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth + 0.5f));
#endif
        }

        for (j = 0; j < gib_io[gib].gib_input_count; j++)
        {
            arch_uint32 layer = gib_io[gib].gib_input[j];
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
            archInfo("gib_io[%d, %d].input, cost.cycle(%d): %llu, cost.bw(%d): %llu\n", gib+1, j + 1, layer + 1, (arch_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfCycleCount + 0.5f), layer + 1, (arch_uint64)(archModel->opInfoArray[layer]->perf.resultInfo.perfReadBandWidth + archModel->opInfoArray[layer]->perf.resultInfo.perfWriteBandWidth + 0.5f));
#endif
        }

        if (inputSubimageSegmentCount < 1 && outputSubimageSegmentCount < 1)
        {
            arch_uint32 gibBufferSize = 0;
            arch_uint32 bufferNeeded = 0;
            arch_int32 sramSize, abBufferSpaceLeft;
            for (j = 0; j < gib_io[gib].gib_input_count; j++)
            {
                gib_input_checked[j] = arch_false_e;
            }
            for (j = 0; j < gib_io[gib].gib_input_count; j++)
            {
                arch_int32 largest_GraphTotalBufferNeeded = -1;
                arch_uint32 largest_input = 0;
                for (k = 0; k < gib_io[gib].gib_input_count; k++)
                {
                    if (!gib_input_checked[k]
                    && largest_GraphTotalBufferNeeded < (arch_int32)gib_obj[gib_io[gib].gib_input[k]].totalBufferNeeded)
                    {
                        largest_GraphTotalBufferNeeded = (arch_int32)gib_obj[gib_io[gib].gib_input[k]].totalBufferNeeded;
                        largest_input = k;
                    }
                }
                gib_input_checked[largest_input] = arch_true_e;
                gibBufferSize += gib_obj[gib_io[gib].gib_input[largest_input]].layerOutBufferSize;
                bufferNeeded = archMAX(bufferNeeded,
                    archMAX(gib_obj[gib_io[gib].gib_input[largest_input]].totalBufferNeeded,
                    gib_obj[gib_io[gib].gib_input[largest_input]].layerInBufferSize + gibBufferSize));
            }

            for (j = 0; j < gib_io[gib].gib_output_count; j++)
            {
                if (archModel->opInfoArray[gib_io[gib].gib_output[j]]->swTilingType == -1)
                {
                    bufferNeeded = archMAX(bufferNeeded, gibBufferSize);
                }
                else
                {
                    bufferNeeded = archMAX(bufferNeeded,
                        archMAX(gib_obj[gib_io[gib].gib_output[j]].totalBufferNeeded,
                        gib_obj[gib_io[gib].gib_output[j]].layerOutBufferSize + gibBufferSize));
                }
            }

            sramSize = axiSramOnlySWTiling ? ARCH_AXI_SRAM_SIZE
                                           : ARCH_VIP_SRAM_SIZE;

            if ((arch_int32)bufferNeeded < sramSize)
            {
                arch_uint8 kbuf = SW_TILING_FROM_DDR, sbuf, dbuf;
                arch_bool allOutputSegmentsAreShort = arch_true_e;
                arch_uint32 dst_graph_id = gib_obj[gib_io[gib].gib_input[0]].gid;

                abBufferSpaceLeft = axiSramOnlySWTiling ? ARCH_VIP_SRAM_SIZE
                                                               : (arch_int32)(sramSize - bufferNeeded);
                for (j = 0; j < gib_io[gib].gib_output_count; j++)
                {
                    arch_uint32 my_gib_output_layer = gib_io[gib].gib_output[j];
                    arch_bool my_gib_output_layer_is_last_in_segment = arch_false_e;
                    arch_uint8 my_sbuf, my_dbuf, my_kbuf = SW_TILING_FROM_DDR;
                    arch_perf_s perf;
                    /*tempLayer = my_gib_output_layer;*/
                    if (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 0 ||
                        (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 1 && archModel->opInfoArray[my_gib_output_layer]->downStreamLayer[0] == -1)
                        )
                    {
                        allOutputSegmentsAreShort = arch_false_e;
                        my_gib_output_layer_is_last_in_segment = arch_true_e;
                    }
                    else
                    {
                        arch_int32 downStreamLayer;
                        getDownstreamLayer(archModel, my_gib_output_layer, 0, &downStreamLayer);
                        if (downStreamLayer > 0 && split_array[downStreamLayer])
                        {
                            allOutputSegmentsAreShort = arch_false_e;
                            my_gib_output_layer_is_last_in_segment = arch_true_e;
                        }
                    }


                    my_sbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    my_dbuf = my_gib_output_layer_is_last_in_segment ? SW_TILING_FROM_DDR
                                                                     : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);

                    archModel->opInfoArray[my_gib_output_layer]->perf.info.flush = 0;
                    /*back up original profile data, and use archModel->opInfoArray[x]->perf to save profile data with new configuration */
                    memcpy(&perf, &archModel->opInfoArray[my_gib_output_layer]->perf, sizeof(arch_perf_s));

                    curCacheSpace[my_gib_output_layer] = abBufferSpaceLeft;
                    /* double check */
                    /* trsp_interleave_ch_in = 1 */
                    _calc_cost(pArchNnConfig,
                            pArchOptions,
                            archModel,
                            my_gib_output_layer,
                            x_array[my_gib_output_layer],
                            y_array[my_gib_output_layer],
                            z_array[my_gib_output_layer],
                            my_sbuf,
                            my_dbuf,
                            my_kbuf,
                            curCacheSpace[my_gib_output_layer],
                            curTrspIvLayerChsIn[my_gib_output_layer],
                            curTrspIvLayerChsOut[my_gib_output_layer]);

                    curCost.cycle += archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfCycleCount;
                    curCost.bw += archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfReadBandWidth;
                    curCost.bw += archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfWriteBandWidth;

                    /* temp for VB issue */
                    /*tempReadBw = archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfReadBandWidth;
                    tempWriteBw = archModel->opInfoArray[my_gib_output_layer]->perf.resultInfo.perfWriteBandWidth;*/
                    /*restore original profile data for sub-graph merging*/
                    memcpy(&archModel->opInfoArray[my_gib_output_layer]->perf, &perf, sizeof(arch_perf_s));
                }

                for (j = 0; j < gib_io[gib].gib_input_count; j++)
                {
                    arch_uint32 my_gib_input_layer = gib_io[gib].gib_input[j];
                    arch_uint8 my_sbuf, my_dbuf, my_kbuf = SW_TILING_FROM_DDR;
                    arch_perf_s perf;

                    my_sbuf = split_array[my_gib_input_layer] ? SW_TILING_FROM_DDR
                                                                    : (axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM);
                    my_dbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    archModel->opInfoArray[my_gib_input_layer]->perf.info.flush = allOutputSegmentsAreShort;
                    /*back up original profile data, and use archModel->opInfoArray[x]->perf to save profile data with new configuration */
                    memcpy(&perf, &archModel->opInfoArray[my_gib_input_layer]->perf, sizeof(arch_perf_s));

                    /* double check this */
                    /* trsp_interleave_ch_out = 1 */
                    curCacheSpace[my_gib_input_layer] = abBufferSpaceLeft;
                    _calc_cost(pArchNnConfig,
                            pArchOptions,
                            archModel,
                            my_gib_input_layer,
                            x_array[my_gib_input_layer],
                            y_array[my_gib_input_layer],
                            z_array[my_gib_input_layer],
                            my_sbuf,
                            my_dbuf,
                            my_kbuf,
                            curCacheSpace[my_gib_input_layer],
                            curTrspIvLayerChsIn[my_gib_input_layer],
                            curTrspIvLayerChsIn[my_gib_input_layer]);

                    /* I guess should be error */
                    curCost.cycle += archModel->opInfoArray[my_gib_input_layer]->perf.resultInfo.perfCycleCount;
                    curCost.bw += archModel->opInfoArray[my_gib_input_layer]->perf.resultInfo.perfReadBandWidth;
                    curCost.bw += archModel->opInfoArray[my_gib_input_layer]->perf.resultInfo.perfWriteBandWidth;
                    /*restore original profile data for sub-graph merging*/
                    memcpy(&archModel->opInfoArray[my_gib_input_layer]->perf, &perf, sizeof(arch_perf_s));
                }
#if ENABLE_ARCH_MODEL_DUMP
                archInfo("gib: %d, base_cycle: %llu, base_bw: %llu\n", gib+1, (arch_uint64)(baseCost.cycle + 0.5), (arch_uint64)(baseCost.bw + 0.5));
                archInfo("gib: %d, cur_cycle: %llu, cur_bw: %llu\n", gib+1, (arch_uint64)(curCost.cycle + 0.5), (arch_uint64)(curCost.bw + 0.5));
#endif
                if (_cur_cost_is_more_better(&baseCost, &curCost, 1, 0))
                {
                    /* copy back the cacheSpace */
                    memcpy(cacheSpace, curCacheSpace, sizeof(arch_int32) * archModel->totalOpCount);

                    kbuf = SW_TILING_FROM_DDR;
                    abBufferSpaceLeft = axiSramOnlySWTiling ? ARCH_VIP_SRAM_SIZE
                                                                   : (sramSize - bufferNeeded);
                    allOutputSegmentsAreShort = arch_true_e;
                    dst_graph_id = gib_obj[gib_io[gib].gib_input[0]].gid;

                    for (j = 0; j < gib_io[gib].gib_output_count; j++)
                    {
                        arch_uint32 src_graph_id;
                        arch_uint32 my_gib_output_layer = gib_io[gib].gib_output[j];
                        arch_bool my_gib_output_layer_is_last_in_segment = arch_false_e;
                        if (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 0 ||
                            (archModel->opInfoArray[my_gib_output_layer]->downStreamLayerCount == 1 && archModel->opInfoArray[my_gib_output_layer]->downStreamLayer[0] == -1)
                           )
                        {
                            allOutputSegmentsAreShort = arch_false_e;
                            my_gib_output_layer_is_last_in_segment = arch_true_e;
                        }
                        else
                        {
                            arch_int32 downStreamLayer;
                            getDownstreamLayer(archModel, my_gib_output_layer, 0, &downStreamLayer);
                            if (downStreamLayer > 0 && split_array[downStreamLayer])
                            {
                                allOutputSegmentsAreShort = arch_false_e;
                                my_gib_output_layer_is_last_in_segment = arch_true_e;
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
#ifdef CACHE_SPACE_ENABLE
                            /*cacheSpace[my_gib_output_layer] = abBufferSpaceLeft;*/
#endif
                            curTrspIvLayerChsIn[my_gib_output_layer] = 1;
                            _calc_cost(pArchNnConfig,
                                pArchOptions,
                                archModel,
                                my_gib_output_layer,
                                x_array[my_gib_output_layer],
                                y_array[my_gib_output_layer],
                                z_array[my_gib_output_layer],
                                sbuf,
                                dbuf,
                                kbuf,
#ifdef CACHE_SPACE_ENABLE
                                cacheSpace[my_gib_output_layer],
                                curTrspIvLayerChsIn[my_gib_output_layer],
                                curTrspIvLayerChsOut[my_gib_output_layer]);
#else
                                abBufferSpaceLeft);
#endif
                            split_array[my_gib_output_layer] = arch_false_e;
                            archModel->opInfoArray[my_gib_output_layer]->swTilingType = 1;
#if ENABLE_ARCH_MODEL_DUMP
                            archInfo("== merged: %d\n", my_gib_output_layer + 1);
#endif
                        }
                    }

                    for (j = 0; j < gib_io[gib].gib_input_count; j++)
                    {
                        arch_uint32 src_graph_id = gib_obj[gib_io[gib].gib_input[j]].gid;
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
#ifdef CACHE_SPACE_ENABLE
                            /*cacheSpace[gib_io[gib].gib_input[j]] = abBufferSpaceLeft;*/
#endif
                            curTrspIvLayerChsOut[gib_io[gib].gib_input[j]] = 1;
                            _calc_cost(pArchNnConfig,
                                pArchOptions,
                                archModel,
                                gib_io[gib].gib_input[j],
                                x_array[gib_io[gib].gib_input[j]],
                                y_array[gib_io[gib].gib_input[j]],
                                z_array[gib_io[gib].gib_input[j]],
                                sbuf,
                                dbuf,
                                kbuf,
#ifdef CACHE_SPACE_ENABLE
                                cacheSpace[gib_io[gib].gib_input[j]],
                                curTrspIvLayerChsIn[gib_io[gib].gib_input[j]],
                                curTrspIvLayerChsOut[gib_io[gib].gib_input[j]]);
#else
                                abBufferSpaceLeft);
#endif
                            archModel->opInfoArray[gib_io[gib].gib_input[j]]->swTilingType = 1;
#if ENABLE_ARCH_MODEL_DUMP
                            archInfo("== merged: %d\n", gib_io[gib].gib_input[j] + 1);
#endif
                        }
                    }
                }
            }
        }
    }

#if ENABLE_ARCH_MODEL_DUMP
    archInfo("++split_status after merged==\n");
    for (j = 0; j < count; j++)
    {
        archInfo("layer[%d]: %d\n", j+1, split_array[j]);
    }
    archInfo("--split_status after merged==\n");
#endif
    if (gib_input_checked) archFreeMemory(gib_input_checked);
    if(curCacheSpace) archFreeMemory(curCacheSpace);
}


/***********************************************************************************
* Function:     archPerfAnalysing
* Description:  Performance Analysing for Arch Model Library
* Input:        archModel:
*               pArchNnConfig:
*               pArchOptions:
* Ouput:        :
***************************************************************************************/
static ARCH_INTERNAL_API arch_status archPerfAnalysing(
    archModelInfo   *archModel,
    arch_nn_config  *pArchNnConfig,
    arch_drv_option *pArchOptions
    )
{
    arch_uint32 i, j, count = 0;
    arch_status vxStatus = ARCH_SUCCESS;
    archSTATUS status    = archSTATUS_OK;
    arch_uint32 *xArray  = NULL, *yArray = NULL, *zArray = NULL;
    arch_uint8 *sArray   = NULL;
    struct _archModelOpInfo ** opInfo;
    arch_bool hasVXC     = arch_false_e;

#ifdef CACHE_SPACE_ENABLE
    /* merge from CL 213830 */
    arch_uint32 *cacheSpace = NULL;
#endif
    arch_uint32 *trspIvLayerChsIn = NULL, *trspIvLayerChsOut = NULL;
#ifdef SPLIT_Z_DIMENSION
    arch_uint32 *flushWait = NULL;
#endif

    // refine me, seperate them to functions
    /* Init Spit Info */
#ifdef CACHE_SPACE_ENABLE
    /* merge from CL 213830 */
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,
                                (archPOINTER *)&cacheSpace);
    if (archIS_ERROR(status))
    {
        vxStatus = ARCH_FAILURE;
        archInfo("ERROR: vxoGraph_PredictPerf(0) return out-of-memory\n");
        goto error;
    }
    memset(cacheSpace, 0, sizeof(arch_uint32) * archModel->totalOpCount);
#endif

    /* NN transpose */
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,
                                (archPOINTER *)&trspIvLayerChsIn);
    if (archIS_ERROR(status))
    {
        vxStatus = ARCH_FAILURE;
        archInfo("ERROR: vxoGraph_PredictPerf(0) return out-of-memory\n");
        goto error;
    }
    memset(trspIvLayerChsIn, 0, sizeof(arch_uint32) * archModel->totalOpCount);
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,
                                (archPOINTER *)&trspIvLayerChsOut);
    if (archIS_ERROR(status))
    {
        vxStatus = ARCH_FAILURE;
        archInfo("ERROR: vxoGraph_PredictPerf(0) return out-of-memory\n");
        goto error;
    }
    memset(trspIvLayerChsOut, 0, sizeof(arch_uint32) * archModel->totalOpCount);

#ifdef SPLIT_Z_DIMENSION
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,(archPOINTER *)&flushWait);
    if (archIS_ERROR(status))
    {
        vxStatus = ARCH_FAILURE;
        archInfo("ERROR: vxoGraph_PredictPerf(0) return out-of-memory\n");
        goto error;
    }
    memset(flushWait, 0, sizeof(arch_uint32) * archModel->totalOpCount);
#endif
    /* xArray/yArray/zArray/sArray allocate */
    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,
                                (archPOINTER *)&xArray);
    if (archIS_ERROR(status))
    {
        vxStatus = ARCH_FAILURE;
        archInfo("ERROR: vxoGraph_PredictPerf(1) return out-of-memory\n");
        goto error;
    }
    memset(xArray, 0, sizeof(arch_uint32) * archModel->totalOpCount);

    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,
                                (archPOINTER *)&yArray);
    if (archIS_ERROR(status))
    {
        vxStatus = ARCH_FAILURE;
        archInfo("ERROR: vxoGraph_PredictPerf(2) return out-of-memory\n");
        goto error;
    }
    memset(yArray, 0, sizeof(arch_uint32) * archModel->totalOpCount);

    status = archAllocateMemory(sizeof(arch_uint32) * archModel->totalOpCount,
                                (archPOINTER *)&zArray);
    if (archIS_ERROR(status))
    {
        vxStatus = ARCH_FAILURE;
        archInfo("ERROR: vxoGraph_PredictPerf(3) return out-of-memory\n");
        goto error;
    }
    memset(zArray, 0, sizeof(arch_uint32) * archModel->totalOpCount);

    /* sArray count should be equal to totalcount plus 1. */
    status = archAllocateMemory(sizeof(arch_uint8) * (archModel->totalOpCount+1),
                                (archPOINTER *)&sArray);
    if (archIS_ERROR(status))
    {
        vxStatus = ARCH_FAILURE;
        archInfo("ERROR: vxoGraph_PredictPerf(4) return out-of-memory\n");
        goto error;
    }
    memset(sArray, 0, sizeof(arch_uint8) * (archModel->totalOpCount+1));
    /* Set the first and the last split array value to 1 */
    sArray[0] = 1;
    sArray[archModel->totalOpCount] = 1;

    /* allocate done */

    /* fill xArray/yArray/zArray/sArray */
    for(i = 0; i< archModel->actualCount; i++)
    {
        xArray[i] = archModel->opInfoArray[i]->pix;
        yArray[i] = archModel->opInfoArray[i]->piy;
        zArray[i] = archModel->opInfoArray[i]->oz;
    }

    count = archModel->actualCount;
    opInfo = archModel->opInfoArray;

    /* check condition */
    if (archIsFeatureAvailable(pArchNnConfig, pArchOptions, archModel->pArchDataFeature, ARCH_NN_FEATURE_SWTILING_PHASE1) && !hasVXC && (count > 1))
    {
        arch_uint32 first = 0;
        arch_uint32 last = count > 0 ? count - 1 : 0;
        archGIBIO *gibIO = NULL;
        archGIBObj *gibObj = NULL;
        arch_uint32 gibLast = 0;
        arch_bool axiSramOnlySWTiling = pArchNnConfig->unifiedFeature.axiSramOnlySWTiling ? arch_true_e : arch_false_e;
        arch_uint32 swTilingSegKernelBufSizeInPixel = 0, swTilingSegStart = 0;
        status = archAllocateMemory(sizeof(archGIBIO) * count, (archPOINTER *)&gibIO);
        if (archIS_ERROR(status))
        {
            vxStatus = ARCH_FAILURE;
            archInfo("ERROR: vxoGraph_PredictPerf(5) return out-of-memory\n");
            goto error;
        }
        memset(gibIO, 0, sizeof(archGIBIO) * count);
        status = archAllocateMemory(sizeof(archGIBObj) * count, (archPOINTER *)&gibObj);
        if (archIS_ERROR(status))
        {
            vxStatus = ARCH_FAILURE;
            archInfo("ERROR: vxoGraph_PredictPerf(6) return out-of-memory\n");
            archFreeMemory(gibIO);
            goto error;
        }
        memset(gibObj, 0, sizeof(archGIBObj) * count);
        /*updateStreamLayer(archModel, count);*/

        for (i = 0; i < count; i++)
        {
            if (opInfo[i]->target == ARCHNNE_OPERATION_TARGET_NN ||
                opInfo[i]->target == ARCHNNE_OPERATION_TARGET_TP)
            {
                first = i;
                sArray[i] = 1;
                break;
            }
        }

        if (last < count)
        {
            arch_uint32 numOfBlocks = (arch_uint32)ceilf(1.0f * (count /*- 1*/) / MAX_LAYERS_OF_BLOCK);
            arch_uint32 block;
            for (block = 0; block < numOfBlocks; block++)
            {
                arch_uint32 block_first = block * MAX_LAYERS_OF_BLOCK;
                arch_uint32 block_last = archMIN((count - 1), (block + 1) * MAX_LAYERS_OF_BLOCK - 1);
                if ((block_last + 1) < (count /*- 1*/))
                    sArray[block_last + 1] = 1;
                resetArchModelSplitInfo(archModel);
                initSegmentCostResult(archModel);
#ifdef CACHE_SPACE_ENABLE
                _split_segment(pArchNnConfig,pArchOptions, archModel, block_first, block_last, xArray, yArray, zArray, cacheSpace, trspIvLayerChsIn, trspIvLayerChsOut, sArray);
#else
                _split_segment(pArchNnConfig,pArchOptions, archModel, block_first, block_last, xArray, yArray, zArray, sArray);
#endif
            }

            /*
            for(i = 0; i < count ; i++)
            {
                tempdata[i] = archModel->opInfoArray[i]->perf.swTilingInfo.cacheSpace;
            }
            */
            if (/*context->options.enableHandleBranch*/ arch_true_e && /*always turn on branch merge feature for arch model algorithm*/
               (pArchOptions->enableSwtilingPhase1 == ARCH_SWTILING_OPTION_ALL || pArchOptions->enableSwtilingPhase1 == ARCH_SWTILING_OPTION_AB))
            {
                G_JINBO_WR = 1;
                gibLast = _create_gib(archModel, count, xArray, yArray, zArray,trspIvLayerChsOut, sArray, gibIO, gibObj);
#ifdef CACHE_SPACE_ENABLE
                _merge_sub_graph(pArchNnConfig,pArchOptions, archModel, count,  xArray, yArray, zArray, cacheSpace,trspIvLayerChsIn,trspIvLayerChsOut, sArray, gibIO, gibObj, gibLast);
#else
                _merge_sub_graph(pArchNnConfig,pArchOptions, archModel, count,  xArray, yArray, zArray, sArray, gibIO, gibObj, gibLast);
#endif
#ifdef ALIGNMENT_64B
                vipSram64BAlignEnhance(pArchNnConfig, archModel, xArray, yArray, cacheSpace,sArray,flushWait);
#endif
            }
#ifdef SPLIT_Z_DIMENSION
                /* SplitZDimension */
                splitZDimension(archModel->apm,pArchNnConfig,pArchOptions, archModel, zArray,cacheSpace,sArray,flushWait);
#endif
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
                    arch_int32 uplayer = -1;
                    getUpstreamLayer(archModel, i, opInfo[i]->upStreamLayerCount - 1, &uplayer);
                    if (uplayer >= 0) {
                        opInfo[i]->sbuf = axiSramOnlySWTiling ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
                    }
                }

                opInfo[i]->dbuf = SW_TILING_FROM_DDR;
                if (opInfo[i]->downStreamLayerCount > 0)
                {
                    arch_int32 downlayer = -1;
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
                            arch_int32 uplayer;
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
            if (opInfo[i]->target == ARCHNNE_OPERATION_TARGET_SH ||
                opInfo[i]->target == ARCHNNE_OPERATION_TARGET_SW)
            {
                if (pArchOptions->enableNNArchPerfPrint)
                {
                    showArchPerformanceLib(&(archModel->chipDentity), pArchNnConfig,pArchOptions, archModel->pArchDataFeature,opInfo,i, ARCH_NULL);
                }
                continue;
            }

            {
#ifndef CACHE_SPACE_ENABLE
                arch_int32 spaceLeft = axiSramOnlySWTiling ? ARCH_VIP_SRAM_SIZE
                    : (ARCH_VIP_SRAM_SIZE - gibObj[i].totalBufferNeeded);
#endif
                arch_int32 flush_and_wait = 0;
                archModel->opInfoArray[i]->segTotalBufferSizeInPixel = gibObj[i].totalBufferNeeded;
                if (opInfo[i]->sbuf == SW_TILING_FROM_DDR)
                {
                    if (opInfo[i]->swTilingType == 0)
                    {
                        /*SWTiling segment start point*/
                        swTilingSegKernelBufSizeInPixel = _kernel_size_in_pixel(archModel, i, archModel->opInfoArray[i]->nnCores, archModel->pArchDataFeature->fullCacheIntervalFix);
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
                            arch_int32 downlayer = -1;
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
                        swTilingSegKernelBufSizeInPixel += _kernel_size_in_pixel(archModel, i, archModel->opInfoArray[i]->nnCores, archModel->pArchDataFeature->fullCacheIntervalFix);

                        if (opInfo[i]->dbuf == SW_TILING_FROM_DDR)
                        {
                            arch_uint32 swTilingSegOutBufSize = _outbuf_needed_ex(archModel, swTilingSegStart, i, xArray, yArray, zArray, trspIvLayerChsOut);
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
                archInfo("table[%d]: sbuf: %s, dbuf: %s, kbuf: %s, abBufferSpaceLeft: %u, flush_and_waith: %d\n", i + 1,
                opInfo[i]->sbuf == 0 ? "DDR" : (opInfo[i]->sbuf == 1) ? "AXI_SRAM" : "VIP_SRAM",
                opInfo[i]->dbuf == 0 ? "DDR" : (opInfo[i]->dbuf == 1) ? "AXI_SRAM" : "VIP_SRAM",
                opInfo[i]->kbuf == 0 ? "DDR" : (opInfo[i]->kbuf == 1) ? "AXI_SRAM" : "VIP_SRAM",
                cacheSpace[i],
                opInfo[i]->perf.info.flush);
#endif

                _calc_cost(pArchNnConfig,pArchOptions,archModel, i,
                    xArray[i], yArray[i], zArray[i],
                    opInfo[i]->sbuf,
                    opInfo[i]->dbuf,
                    opInfo[i]->kbuf,
#ifdef CACHE_SPACE_ENABLE
                    cacheSpace[i],
                    trspIvLayerChsIn[i],
                    trspIvLayerChsOut[i]
#else
                    spaceLeft
#endif
                    );
            }

            if (pArchOptions->enableNNArchPerfPrint)
            {
                showArchPerformanceLib(&(archModel->chipDentity),pArchNnConfig,pArchOptions,archModel->pArchDataFeature, opInfo,i, &opInfo[i]->perf);
            }
        }
        {
            /*swTilingType: -1: none, 1: AB, 0: Sub-IMAGE*/
            arch_uint32 swtStart = 0, abStart = 0, abEnd = 0;
            arch_uint32 abFlag = 0;
            for (i = first; i < count; i++)
            {
                if (opInfo[i]->target == ARCHNNE_OPERATION_TARGET_SH ||
                    opInfo[i]->target == ARCHNNE_OPERATION_TARGET_SW)
                {
                    if (pArchOptions->enableNNArchPerfPrint)
                    {
                        archInfo("abs_op_id[%d]: %c->%c, %s\n",
                            opInfo[i]->absoluteOperationID,
                            'D', 'D', "DDR");
                    }
                }
                else
                {
                    if (pArchOptions->enableNNArchPerfPrint)
                    {
                        archInfo("abs_op_id[%d]: %c->%c, %s\n",
                            opInfo[i]->absoluteOperationID,
                            (opInfo[i]->sbuf == 0) ? 'D' : 'S',
                            (opInfo[i]->dbuf == 0) ? 'D' : 'S',
                            (opInfo[i]->swTilingType == 1) ? "AB" :
                            (opInfo[i]->swTilingType == 0) ? "SWT" : "DDR");
                    }
                }
            }
            archInfo("@@detected AB/SWT blocks:");
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
                                    abStart = opInfo[i]->absoluteOperationID;
                                    abFlag = 1;
                                }
                            }
                            else
                            {
                                swtStart = opInfo[i]->absoluteOperationID;
                                if (abFlag == 2)
                                {
                                    archInfo("[%d,%d,%s]", abStart, abEnd, "AB");
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
                            archInfo("[%d,%d,%s]", swtStart, opInfo[i]->absoluteOperationID, "SWT");
                        }
                        else
                        {
                            if (opInfo[i]->swTilingType == 1)
                            {
                                abEnd = opInfo[i]->absoluteOperationID;
                                abFlag = 2;
                            }
                        }
                    }
                }
            }
            if (abFlag == 2)
            {
                archInfo("[%d,%d,%s]", abStart, abEnd, "AB");
            }
            archInfo("\n");
        }
        archFreeMemory(gibIO);
        archFreeMemory(gibObj);
    }
    else
    {
        for (i = 0; i < count; i++)
        {
            if (opInfo[i]->target == ARCHNNE_OPERATION_TARGET_SH ||
                opInfo[i]->target == ARCHNNE_OPERATION_TARGET_SW)
            {
                if (pArchOptions->enableNNArchPerfPrint)
                {
                    showArchPerformanceLib(&(archModel->chipDentity),pArchNnConfig,pArchOptions,archModel->pArchDataFeature, opInfo,i, ARCH_NULL);
                }
                continue;
            }
#if ENABLE_ARCH_MODEL_DUMP
                archInfo("table[%d]: sbuf: %s, dbuf: %s, kbuf: %s, abBufferSpaceLeft: %u, flush_and_waith: %d\n", i + 1,
                "DDR",
                "DDR",
                "DDR",
                ARCH_VIP_SRAM_SIZE,
                1);
#endif
            opInfo[i]->perf.info.flush = 1;
            _calc_cost(pArchNnConfig, pArchOptions, archModel, i,
                       opInfo[i]->xsize, opInfo[i]->ysize, opInfo[i]->oz,
                       SW_TILING_FROM_DDR, SW_TILING_FROM_DDR, SW_TILING_FROM_DDR,
                       ARCH_VIP_SRAM_SIZE, trspIvLayerChsIn[i], trspIvLayerChsOut[i]);

            if (pArchOptions->enableNNArchPerfPrint)
            {
                showArchPerformanceLib(&(archModel->chipDentity),pArchNnConfig,pArchOptions,archModel->pArchDataFeature,opInfo,i, &opInfo[i]->perf);
            }
        }
    }

error:
    if (xArray) archFreeMemory(xArray);
    if (yArray) archFreeMemory(yArray);
    if (zArray) archFreeMemory(zArray);
    if (sArray) archFreeMemory(sArray);
    if (flushWait) archFreeMemory(flushWait);
    if (trspIvLayerChsIn) archFreeMemory(trspIvLayerChsIn);
    if (trspIvLayerChsOut) archFreeMemory(trspIvLayerChsOut);
#ifdef CACHE_SPACE_ENABLE
    if (cacheSpace) archFreeMemory(cacheSpace);
#endif
    /*deInitArchModelInfo(archModel, count);*/
    /* DeInit Spit Info */
    return vxStatus;
}

/* Get environment variable value. */
arch_status
archGetEnv(
    const char * VarName,
    char ** Value
    )
{
    *Value = getenv(VarName);

    return archSTATUS_OK;
}

/***********************************************************************************
* Function:     archApmInit
* Description:  Arch Model apm init for call NNArchPerf
* Input:        pArchNnConfig:
* Ouput:        APMHandle:
***************************************************************************************/
APMHandle archApmInit(arch_nn_config *pArchNnConfig,archHAL_CHIPIDENTITY *pChipIdentity,archNN_DATABASE_FEATURE *pArchDataFeature)
{
    /*char *useLibNNArchPerf = ARCH_NULL;*/
    arch_float32 internalLatency = 0, totalLatency = 0;
    APMHandle apm = NULL;
    BWL_T bwl = {0};
    APM_IN_PARAM_T inParam;
    memset(&inParam,0,sizeof(APM_IN_PARAM_T));

    /* calculate lantency */
    internalLatency = (arch_float32)(20.0 + (11.0 + 6.0) * pArchNnConfig->customizedFeature.freqInMHZ / pArchNnConfig->customizedFeature.axiClockFreqInMHZ);
    totalLatency = 1.0f * (arch_uint32)(pArchNnConfig->customizedFeature.ddrLatency + internalLatency + 0.5f);

    {
        bwl.ddr_read_bw_in_byte_per_cycle = pArchNnConfig->customizedFeature.ddrReadBWLimit;   /*ddr bw limit*/
        bwl.ddr_write_bw_in_byte_per_cycle = pArchNnConfig->customizedFeature.ddrWriteBWLimit;

        bwl.ddr_total_bw_in_byte_per_cycle = pArchNnConfig->customizedFeature.ddrTotalBWLimit;
        bwl.axiSramReadBWLimit = pArchNnConfig->customizedFeature.axiSramReadBWLimit;          /*axi bw limit*/
        bwl.axi_sram_write_bw_limit = pArchNnConfig->customizedFeature.axiSramWriteBWLimit;
        bwl.axi_sram_total_bw_limit = pArchNnConfig->customizedFeature.axiSramTotalBWLimit;
        bwl.axi_bus_read_bw_limit = pArchNnConfig->customizedFeature.axiBusReadBWLimit;        /*axi-bus bw limit*/
        bwl.axi_bus_write_bw_limit = pArchNnConfig->customizedFeature.axiBusWriteBWLimit;
        bwl.axi_bus_total_bw_limit = pArchNnConfig->customizedFeature.axiBusTotalBWLimit;
        bwl.internal_write_bw_limit = (arch_float32)pArchNnConfig->fixedFeature.nnLanesPerOutCycle;    /*internal write bw limite*/
        bwl.ddr_latency = pArchNnConfig->customizedFeature.ddrLatency;                         /*ddr latency*/
        bwl.total_latency = totalLatency;                                                       /*total latency*/
        bwl.maxSocOTNumber = (float)pArchNnConfig->customizedFeature.maxSocOTNumber;

        /* burst setting */
        bwl.DDR_READ_BW_IN_BYTE_PER_CYCLE_64B = pArchDataFeature->ddrReadSustainedBw64BBurst;
        bwl.DDR_READ_BW_IN_BYTE_PER_CYCLE_128B = pArchDataFeature->ddrReadSustainedBw128BBurst;
        bwl.DDR_READ_BW_IN_BYTE_PER_CYCLE_256B = pArchDataFeature->ddrReadSustainedBw256BBurst;
        bwl.DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_64B = pArchDataFeature->ddrMaskWriteSustainedBw64BBurst;
        bwl.DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_128B = pArchDataFeature->ddrMaskWriteSustainedBw128BBurst;
        bwl.DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_256B = pArchDataFeature->ddrMaskWriteSustainedBw256BBurst;
        bwl.DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_64B = pArchDataFeature->ddrNonMaskWriteSustainedBw64BBurst;
        bwl.DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_128B = pArchDataFeature->ddrNonMaskWriteSustainedBw128BBurst;
        bwl.DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_256B = pArchDataFeature->ddrNonMaskWriteSustainedBw256BBurst;


        inParam.chipDef.ChipID = (arch_uint32)pChipIdentity->chipModel;
        inParam.chipDef.ChipVersion = pChipIdentity->chipRevision;
        inParam.chipDef.ProductID = pChipIdentity->productID;
        inParam.chipDef.EcoID = pChipIdentity->ecoID;
        inParam.chipDef.CustomerID = pChipIdentity->customerID;

        /* Add new parameters */
        inParam.NN_DDR_BURST_SIZE = 64;
        inParam.specified_ddr_bw_limit_by_burst = pArchDataFeature->specificDDRLimitByBurst;
        memcpy(&inParam.bwl, &bwl, sizeof(bwl));
        apm = CreateAPModel(inParam);
    }

    return apm;
}

#ifdef ALIGNMENT_64B
/* Merge from CL 213845 */
/***********************************************************************************
* Function:     updateAlignmentInfo
* Description:  update some Opinfo for predict for Arch Model
* Input:        opInfo:
*               totalOpCount:
* Ouput:        arch_status:
***************************************************************************************/
/* May not needed. should be already clear to 0 when allocated */
static arch_uint32 updateAlignmentInfo(archModelOpInfo ** opInfo, arch_uint32 totalOpCount)
{
    arch_uint32 i = 0;
    for (i= 0; i< totalOpCount; i++)
    {
        opInfo[i]->perf.inputAlignment = 0;
        opInfo[i]->perf.outputAlignment = 0;
    }
    return 0;
}
#endif
/***********************************************************************************
* Function:     archPredictPerf
* Description:  Function for doing Arch Model Predict
* Input:        opInfo:         Operation Information for the whole graph
*               totalOpCount:   total operation count
*               pArchNnConfig: Configration for specific case
*               pArchOptions:   Options for env
*               pChipIdentity:  Chip Identify for HW info
*               pArchDataFeature:Extra feature for the case
* Ouput:        arch_status:
***************************************************************************************/
arch_status archPredictPerf(
    APMHandle apm,
    archModelOpInfo ** opInfo,
    arch_uint32 totalOpCount,
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archNN_DATABASE_FEATURE *pArchDataFeature,
    archHAL_CHIPIDENTITY *pChipDentity
    )
{
    arch_status vxStatus = ARCH_SUCCESS;
    struct _archModelInfo * archModel;

#if gcdDEBUG
    archSetDebugLevel(ARCH_DEBUG_LEVEL_INFO);
#else
    archSetDebugLevel(ARCH_DEBUG_LEVEL_NONE);
#endif
#if DUMP_PARAMETERS
    /* print config */
    printConfig(pArchNnConfig,pArchOptions,pChipDentity,pArchDataFeature);

    printOpInfo(opInfo,totalOpCount);
#endif

    archModel = initArchModelInfo(totalOpCount);
    if (archModel == NULL)
    {
        vxStatus = ARCH_FAILURE;
        goto error;
    }

#ifdef ALIGNMENT_64B
    updateAlignmentInfo(opInfo,totalOpCount);
#endif

    archModel->opInfoArray = opInfo;
    initSegmentCostResult(archModel);

    /* Init opinfo from parameters */

    /* update alignment for merge */
    archInfo("Library: Arch SW Library Enter.\n");

    /* init chip info for output */
    archModel->chipDentity.chipModel = pChipDentity->chipModel;
    archModel->chipDentity.chipRevision = pChipDentity->chipRevision;
    archModel->chipDentity.productID = pChipDentity->productID;
    archModel->chipDentity.customerID = pChipDentity->customerID;
    archModel->chipDentity.ecoID = pChipDentity->ecoID;

    /* init data feature */
    archModel->pArchDataFeature = pArchDataFeature;
    archModel->apm = apm;
    vxStatus = archPerfAnalysing(archModel,pArchNnConfig,pArchOptions);        /* graph do not need */

error:
    deInitArchModelInfo(archModel, totalOpCount);
    return vxStatus;
}


/***********************************************************************************
* Function:     archIsFeatureAvailable
* Description:  Check if Feature is avalible for current case
* Input:        pArchNnConfig:
*               pArchOptions:
*               feature:
* Ouput:        arch_bool: Ture for avalible while False for unavalible
***************************************************************************************/
arch_bool archIsFeatureAvailable(arch_nn_config *pArchNnConfig, arch_drv_option *pArchOptions,archNN_DATABASE_FEATURE *pArchDataFeature,arch_nn_feature_e feature)
{
    switch (feature)
    {
    case ARCH_NN_FEATURE_SWTILING_PHASE1:
        return (pArchDataFeature->swtilingPhase1Enable
             && (pArchOptions->enableSwtilingPhase1 != ARCH_SWTILING_OPTION_OFF)
             && (pArchNnConfig->customizedFeature.axiSRAMSize > 0 || pArchDataFeature->swtilingPhase3Enable)
             && pArchDataFeature->nnStrideEnable) ? arch_true_e : arch_false_e;

    case ARCH_NN_FEATURE_SWTILING_PHASE2:
        return (pArchDataFeature->swtilingPhase2Enable
             && pArchOptions->enableSwtilingPhase2 ) ? arch_true_e : arch_false_e;

    case ARCH_NN_FEATURE_SWTILING_PHASE3:
        return (pArchDataFeature->swtilingPhase3Enable
             && pArchOptions->enableSwtilingPhase3 ) ? arch_true_e : arch_false_e;
    case ARCH_NN_FEATURE_ZDP3:
        return (pArchDataFeature->zdp3Enable
             && pArchDataFeature->nnZdp3Enable) ? arch_true_e : arch_false_e;

    case ARCH_NN_FEATURE_ZDP6:
        return (pArchDataFeature->zdp6Enable
             && pArchDataFeature->nnZdp6Enable) ? arch_true_e : arch_false_e;

    case ARCH_NN_FEATURE_XYDP9:
        return (pArchDataFeature->xydp9Enable
            && pArchOptions->enableNNXYDP9 ) ? arch_true_e : arch_false_e;

    case ARCH_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT:
        return (pArchDataFeature->coefComEnhancement && pArchOptions->enableHuffmanEnhancement) ? arch_true_e : arch_false_e;

    case ARCH_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT:
        return (pArchDataFeature->tpComEnhancement && pArchOptions->enableTPHuffman) ? arch_true_e : arch_false_e;
    case ARCH_NN_FEATURE_BRICK_MODE:
        return pArchOptions->enableBrickMode ? arch_true_e : arch_false_e;
    case ARCH_NN_FEATURE_VIP_DEC400:
        return (pArchDataFeature->vipDec400Enable && pArchOptions->enableVIPDEC400) ? arch_true_e : arch_false_e;
    default:
        return arch_false_e;
    }
}

#define ENABLE_FROM_LOG
#ifdef ENABLE_FROM_LOG
/* Get data formant from dataSize and isFp16. TBD to delete */
arch_uint32 archGetDataFormat(
    arch_uint32 dataSize,
    arch_uint32 isFp16
    )
{
    if(dataSize == 8)
        return ARCH_TYPE_UINT8;            /* UINT8 or IN8 are the same to predict */
    else if(dataSize == 16)
    {
        if(isFp16 == 1)
            return ARCH_TYPE_FLOAT16;
        else
            return ARCH_TYPE_INT16;
    }
    else
    {
        /* error case */
        return ARCH_TYPE_INVALID;
    }
}

/* get layer name from layer type */
const arch_char * archGetLayerName(
    arch_uint32 type
    )
{
    switch(type)
    {
        case ARCHNNE_LAYER_CONV_RELU_POOLING2:
            return "ConvolutionReluPoolingLayer2";
        case ARCHNNE_LAYER_TENSOR_SCALE:
            return "TensorScale";
        case ARCNNNE_LAYER_CONV_RELU:
            return "ConvolutionReluLayer";
        case ARCHNNE_LAYER_CONV_RELU_POOLING:
            return "ConvolutionReluPoolingLayer";
        case ARCNNNE_LAYER_SOFT_MAX:
            return "SoftmaxLayer";
        case ARCHNNE_LAYER_SOFT_MAX2:
            return "SoftMax2";
        case ARCNNNE_LAYER_POOLING:
            return "PoolingLayer";
        case ARCNNNE_LAYER_POOLING2:
            return "PoolingLayer2";
        case ARCNNNE_LYAER_DW_CONV:
            return "DepthwiseConvolutionLayer";
        case ARCNNNE_LYAER_CONV:
            return "ConvolutionLayer";
        case ARCNNNE_LYAER_FC:
            return "FullyConnectedLayer";
        case ARCNNNE_LAYER_FC_RELU:
            return "FullyConnectedReluLayer";
        case ARCNNNE_LYAER_ACTIVATE:
            return "ActivationLayer";
        case ARCNNNE_LYAER_LEAKY_RELU:
            return "LeakyReluLayer";
        case ARCNNNE_LYAER_PRE_RELU:
            return "PReluLayer";
        case ARCHNNE_LAYER_RPN:
            return "RpnLayer";
        case ARCHNNE_LAYER_RIO_POOLING:
            return "ROIPoolLayer";
        case ARCHNNE_LAYER_RIO_POOLING_RELU:
            return "ROIPoolReluLayer";
        case ARCHNNE_LAYER_CONCAT:
            return "ConcatLayer";
        case ARCHNNE_LAYER_REORG:
            return "ReorgLayer";
        case ARCHNNE_LAYER_REORG2:
            return "ReorgLayer2";
        case ARCHNNE_LAYER_DE_CONV:
            return "DeConvolutionLayer";
        case ARCNNNE_LAYER_NORMALIZE:
            return "NormalizationLayer";
        case ARCNNNE_LYAER_L2_NORMALIZE:
            return "L2NormalizeLayer";
        case ARCNNNE_LYAER_BATCH_NORMALIZE:
            return "BatchNormalizationLayer";
        case ARCHNNE_LAYER_TENSOR_ADD:
            return "TensorAdd";
        case ARCHNNE_LAYER_TENSOR_SUB:
            return "TensorSub";
        case ARCHNNE_LAYER_TENSOR_MUL:
            return "TensorMul";
        case ARCHNNE_LAYER_TENSOR_DIV:
            return "TensorDiv";
        case ARCHNNE_LAYER_TENSOR_TRANSPOSE:
            return "TensorTranspose";
        case ARCHNNE_LAYER_TENSOR_REDUCE_SUM:
            return "TensorReduceSum";
        case ARCHNNE_LAYER_TENSOR_PAD:
            return "TensorPadOperation";
        case ARCHNNE_LAYER_TENSOR_PAD2:
            return "TensorPadOperation2";
        case ARCHNNE_LAYER_TENSOR_COPY:
            return "TensorCopy";
        case ARCHNNE_LAYER_TENSOR_REVERSE:
            return "TensorReverse";
        case ARCHNNE_LAYER_TENSOR_MEAN:
            return "TensorMean";
        case ARCHNNE_LAYER_TENSOR_SQUEEZE:
            return "TensorSqueeze";
        case ARCHNNE_LAYER_TENSOR_STRIDE_SLICE:
            return "TensorStrideSlice";
        case ARCHNNE_LAYER_TENSOR_ROUNDING:
            return "TensorRounding";
        case ARCHNNE_LAYER_HASHLUT:
            return "HashLUT";
        case ARCHNNE_LAYER_LSH_PROJECT:
            return "LSHProjection";
        case ARCHNNE_LAYER_RESHAPE:
            return "Reshape";
        case ARCHNNE_LAYER_LUT2:
            return "LUT2";
        case ARCHNNE_LAYER_NORMALIZE2:
            return "NormalizationLayer2";
        case ARCHNNE_LAYER_ADAPTER:
            return "AdapterLayer";
        case ARCHNNE_LAYER_YUV2RGB_SCALE:
            return "YUV2RGBScale";
        case ARCHNNE_LAYER_LSTM:
            return "_LSTM_LAYER";
        default:
            return "Unknown";
    }
}

/* Check if current operation is a valid FC */
static arch_uint32 isValidFC(arch_uint32 opType)
{
    arch_uint32 result = arch_false_e;
    if (opType == ARCHNNE_OPERATOR_FULLYCONNECTED)
    {
        result = arch_true_e;
    }

    return result;
}


/* Check if current operation is a valid tensor add */
arch_uint32 isTensorAdd(arch_uint32 opTarget, arch_uint32 kz, arch_uint32 z)
{
    arch_uint32 result = arch_false_e;
    if (opTarget != ARCHNNE_OPERATION_TARGET_TP
        && kz <= 8
        && kz == 2*z)
    {
        result = arch_true_e;
    }

    return result;
}

arch_int32 getAbsIdFromLayer(archMappingTableStr *mappingTable,arch_uint32 layerId, arch_uint32 minAbsId, arch_uint32 maxAbsId, arch_int32 opIndex)
{
    arch_uint32 i = 0;
    arch_int32 findId = -1;
    /* the needed absId should be bigger that minAbsId, and less than maxAbsId.
    for find child id, it should be from current id plus 1 to the end;
    for find parent id, it should be from 0 to current id */
    for(i = minAbsId; i< maxAbsId; i++)
    {
        if(layerId == mappingTable[i].layerId && mappingTable[i].operationId == opIndex)
        {
            findId = i;
            break;
        }
    }
    return findId;
}


#ifdef DUMP_PARAMETERS
arch_status dumpNetWrok(
    archModelOpInfo **opInfo,
    arch_uint32 totalCount)
{
    arch_uint32 index = 0, j = 0;
    arch_uint32 *pDisplayed = NULL;
    /* for debug, output network */
    /* output head file */

    archAllocateMemory(sizeof(arch_uint32)*totalCount, (archPOINTER *)&pDisplayed);
    memset(pDisplayed, 0, sizeof(arch_uint32)*totalCount);
    archInfo("layerId: %d, AbsId: %d, opType %d.\n",opInfo[0]->layerId, opInfo[0]->absoluteOperationID, opInfo[0]->op);
    pDisplayed[0] = 1;
    for(index = 0; index < totalCount; index++)
    {
        arch_uint32 index = 0, id = 0;
        for(j = 0; j <opInfo[index]->downStreamLayerCount; j++)
        {
            /* output info */
            id = opInfo[index]->childAbsId[j];
            /* If this layer has been displayed, ignore it */
            if(pDisplayed[id] == 0)
            {
                archInfo("layerId: %d, AbsId: %d, opType %d.    ####    ",opInfo[id]->layerId, opInfo[id]->absoluteOperationID, opInfo[id]->op);
            }
        }
        archInfo("\n");
    }
    archFreeMemory(pDisplayed);
    return 0;
}
#endif


/***********************************************************************************
* Function:     updateSingleAllSilbling1X1
* Description:  update the silbling 1X1 info for a single layer
* Input:         operation:        The specific operation for loop
*                OpInfo:            OpInfo struction
*                index:            Indicate the layer index
* Ouput:        void
***************************************************************************************/
static void updateSingleAllSilbling1X1(archModelOpInfo ** OpInfo,arch_uint32 index, arch_uint32 totalCount)
{
    arch_uint32 i=0;
    OpInfo[index]->perf.allSibling1x1 = 1;
    OpInfo[index]->perf.SiblingHas1x1 = 0;
    for (i = 0; i < OpInfo[index]->upOpCount; i++)
    {
        if(OpInfo[index]->parentOpId[i] == -1 && index == 0)
        {
            OpInfo[index]->perf.allSibling1x1 = 0;
            /*  Compute SiblingHas1x1 array  */
            if(OpInfo[index]->target != ARCHNNE_OPERATION_TARGET_TP
                && OpInfo[index]->kx == 1 && OpInfo[index]->ky == 1)
            {
                OpInfo[index]->perf.SiblingHas1x1 = 1;
            }
        }
        else
        {
            /* Loop every parent op's child op */

            /* get the parent layer. Since the parent layer may be SH and the absId will be -1, need to user the layer id for check */
            arch_int32 layerId = 0;
            arch_uint32 loopIndex = 0, parentIndex = 0, parentCount = 0;
            layerId = OpInfo[index]->parentLayer[i];

            /* find all child for this layerId, need to loop the whole network */
            for(loopIndex = 0; loopIndex < totalCount; loopIndex++)
            {
                parentCount = OpInfo[loopIndex]->upOpCount;
                /* for every layer, loop all parent layer */
                for(parentIndex = 0; parentIndex < parentCount; parentIndex++)
                {
                    if(OpInfo[loopIndex]->parentLayer[parentIndex] == layerId)
                    {
                        /* find a child layer */
                        if (OpInfo[loopIndex]->target == ARCHNNE_OPERATION_TARGET_NN || isValidFC(OpInfo[loopIndex]->op))
                        {
                            if (!isValidFC(OpInfo[loopIndex]->op))
                            {   /* NN none fc */
                                if(OpInfo[loopIndex]->kx != 1 || OpInfo[loopIndex]->ky != 1 || OpInfo[index]->target == ARCHNNE_OPERATION_TARGET_TP)
                                {
                                    OpInfo[index]->perf.allSibling1x1 = 0;
                                }
                            }
                            /*  Compute SiblingHas1x1 array  */
                            if(OpInfo[index]->target != ARCHNNE_OPERATION_TARGET_TP
                                && OpInfo[index]->kx == 1 && OpInfo[index]->ky == 1)
                            {
                                OpInfo[index]->perf.SiblingHas1x1 = 1;
                            }
                        }
                    }
                }
            }

        }
    }
}

arch_status archUpdateOperationID(
    archModelOpInfo **opInfo,
    arch_uint32 totalCount
    )
{
    arch_uint32 index = 0;

    for(index = 0; index < totalCount; index++)
    {
        /* update operation ID */
        if(index > 0)           /* do not need to check the operation 0 */
        {
            /* Check if the layer ID is the same as previous one */
            if(opInfo[index]->layerId == opInfo[index - 1]->layerId)
            {
                opInfo[index]->operationId = opInfo[index - 1]->operationId +1;
            }
        }
    }
    return 0;
}
/***********************************************************************************
* Function:     archUpdateUpDownLayerInfo
* Description:  Update the upstream and downstream info according to the layer info from excel
                from python
* Input:        opInfo:
*               totalCount:
*
* Ouput:        arch_status:
***************************************************************************************/
arch_status archUpdateUpDownLayerInfo(
    archModelOpInfo **opInfo,
    arch_uint32 totalCount
    )
{
    arch_uint32 index = 0, j = 0;
    arch_int32 indicateIndex = 0, currentDownIndex = 0;
    /* calcute up/down stream based on upstream count and parent abs ID */

    for(index = 0; index < totalCount; index++)
    {
        if(opInfo[index]->upStreamLayerCount > 0)
        {
            /* calcute upstream info */
            for(j = 0; j <opInfo[index]->upStreamLayerCount; j++)
            {
                /* find the upstream layer */
                indicateIndex = opInfo[index]->parentAbsId[j];
                if(indicateIndex < 0)
                    continue;

                opInfo[index]->parentOpId[j] = opInfo[indicateIndex]->operationId;               /* There is no operation info in excel, so set to 0 */
                opInfo[index]->parentOpType[j] = opInfo[indicateIndex]->op;             /* There is no operation info in excel, so set to 0 */
                opInfo[index]->parentLayer[j] = opInfo[indicateIndex]->layerId;         /* the index is the abs id */
                opInfo[index]->parentLayerType[j] = 0;
                /* update upstream layer, set in fillOp */
                /*opInfo[index]->upStreamLayer[j] = opInfo[index]->parentAbsId[j];*/
            }

            /* calcute downstream info */
            for(j = 0; j <opInfo[index]->upStreamLayerCount; j++)
            {
                /* find the upstream layer */
                indicateIndex = opInfo[index]->parentAbsId[j];
                if(indicateIndex < 0)
                    continue;
                /* update the downstream info */
                currentDownIndex = opInfo[indicateIndex]->downStreamLayerCount;
                opInfo[indicateIndex]->childOpId[currentDownIndex] = opInfo[index]->operationId;        /* There is no operation info in excel, so set to 0 */
                opInfo[indicateIndex]->childOpType[currentDownIndex] = opInfo[index]->op;      /* There is no operation info in excel, so set to 0 */

                opInfo[indicateIndex]->childLayer[currentDownIndex] = opInfo[index]->layerId;
                opInfo[indicateIndex]->childLayerType[currentDownIndex] = 0;
                opInfo[indicateIndex]->childAbsId[currentDownIndex] = opInfo[index]->absoluteOperationID;
                opInfo[indicateIndex]->downStreamLayer[currentDownIndex] = opInfo[index]->absoluteOperationID;


                opInfo[indicateIndex]->downStreamLayerCount = opInfo[indicateIndex]->downStreamLayerCount +1 ;
                opInfo[indicateIndex]->downOpCount  = opInfo[indicateIndex]->downStreamLayerCount;
                opInfo[indicateIndex]->downLayerCount = opInfo[indicateIndex]->downStreamLayerCount;

            }
        }
        else
        {
            /* no upstream count */
            opInfo[index]->parentOpId[0] = 0;
            opInfo[index]->parentOpType[0] = 0;
            opInfo[index]->parentLayer[0] = 0;
            opInfo[index]->parentLayerType[0] = 0;
            opInfo[index]->parentAbsId[0] = -1;
            opInfo[index]->upStreamLayer[0] = -1;
        }

    }


    return 0;
}

/***********************************************************************************
* Function:     archFillOpInfo
* Description:  Main Entry of Arch Model Library Called from python, will call archPredictPerf
                for detail analyzing
* Input:        graphInfo:
*               pArchNnConfig:
*               pArchOptions
* Ouput:        arch_status:
***************************************************************************************/
arch_status archFillOpInfo(
    archModelOpInfo **opInfo,
    archModelGraphInfo ** graphInfo,
    arch_nn_config *pArchNnConfig,
    arch_uint32 totalCount
    )
{
    arch_uint32 i = 0, j = 0, opIndex = 0; /*k = 0;*/
    arch_int32 opAbsId = 0;
    arch_uint32 count = 0, opCount = 0;
    arch_bool supportNNTPParallel = arch_false_e;
    archMappingTableStr *mappingTable = NULL;
    archSTATUS status = archSTATUS_OK;

    /* build the layer_id to abs id mapping table */
    status = archAllocateMemory(sizeof(archMappingTableStr)*totalCount, (archPOINTER *)&mappingTable);
    if(archIS_ERROR(status) || mappingTable == NULL)
    {
        archInfo("allocate mapping table failed.\n");
        return -1;
    }
    memset(mappingTable,0,sizeof(archMappingTableStr)*totalCount);
    for (i = 0; i < totalCount; i++)
    {
        /*mappingTable[graphInfo[i]->layerId] = i;*/
        mappingTable[i].operationId = graphInfo[i]->operationId;
        mappingTable[i].layerId = graphInfo[i]->layerId;
    }

#ifdef DUMP_PARAMETERS
#endif

    /* update allSibling1x1 info */
    opInfo[0]->perf.allSibling1x1 = 0;
    /* Fill data for every node */
    for (i = 0; i < totalCount; i++)
    {
        {
            /*opInfo[count]->tpType = operation->parameter.tpType;*/   /* TBD */
            /* non-related to graphInfo */
            opInfo[count]->swTilingType = -1;

            /* Basic information */
            opInfo[count]->absoluteOperationID = graphInfo[count]->absId;
            opInfo[count]->layerId = graphInfo[count]->layerId;
            opInfo[count]->layerName = archGetLayerName(graphInfo[count]->layerType);
            opInfo[count]->operationId = graphInfo[count]->operationId;
            opInfo[count]->op      = graphInfo[count]->opType;
            opInfo[count]->target  = graphInfo[count]->opTarget;

            /* kernel information */
            opInfo[count]->kx = graphInfo[count]->kx;
            opInfo[count]->ky = graphInfo[count]->ky;
            opInfo[count]->kz = graphInfo[count]->kz;

            /* Pooling info */
            opInfo[count]->psize   = graphInfo[count]->pollingSize;
            opInfo[count]->pstride = graphInfo[count]->pollingStride;

            /* data size */
            opInfo[count]->inputDataSize   = graphInfo[count]->inputDataSize;
            opInfo[count]->outputDataSize   = graphInfo[count]->outputDataSize;
            opInfo[count]->inputDataFormat = archGetDataFormat(graphInfo[count]->inputDataSize,graphInfo[count]->isFp16);
            opInfo[count]->outputDataFormat = archGetDataFormat(graphInfo[count]->outputDataSize,graphInfo[count]->isFp16);

            /* input */
            opInfo[count]->inx = graphInfo[count]->origInX;
            opInfo[count]->iny = graphInfo[count]->origInY;
            opInfo[count]->inz = graphInfo[count]->origInZ;

            /* init buf */
            opInfo[count]->sbuf = SW_TILING_FROM_AXI_SRAM;
            opInfo[count]->dbuf = SW_TILING_FROM_AXI_SRAM;
            opInfo[count]->kbuf = SW_TILING_FROM_VIP_SRAM;

            opInfo[count]->stridex = graphInfo[count]->stridex;
            opInfo[count]->stridey = graphInfo[count]->stridey;

            /* Since all data from log or excel should be valid, only need to check the target and operation type */
            if(opInfo[count]->target == ARCHNNE_OPERATION_TARGET_NN)
            {
                opInfo[count]->calcinx = graphInfo[count]->nnOutX;
                opInfo[count]->calciny = graphInfo[count]->nnOutY;

                /* out */
                opInfo[count]->origoutx = graphInfo[count]->finalOutX;
                opInfo[count]->origouty = graphInfo[count]->finalOutY;
                opInfo[count]->oz = graphInfo[count]->finalOutZ;

                opInfo[count]->xsize = opInfo[count]->calcinx;
                opInfo[count]->ysize = opInfo[count]->calciny;

                opInfo[count]->fcmd = arch_true_e;
            }
            else if (opInfo[count]->target == ARCHNNE_OPERATION_TARGET_TP && !(isValidFC(opInfo[count]->op)))
            {
                opInfo[count]->calcinx = graphInfo[count]->origInX;
                opInfo[count]->calciny = graphInfo[count]->origInY;

                opInfo[count]->origoutx = graphInfo[count]->nnOutX;
                opInfo[count]->origouty = graphInfo[count]->nnOutY;
                opInfo[count]->oz = graphInfo[count]->nnOutZ;
                if (graphInfo[count]->opType == ARCHNNE_OPERATOR_POOLING)
                {
                    opInfo[count]->xsize = opInfo[count]->inx;
                    opInfo[count]->ysize = opInfo[count]->iny;
                }
                else
                {
                    opInfo[count]->xsize = opInfo[count]->origoutx;
                    opInfo[count]->ysize = opInfo[count]->origouty;
                }

                opInfo[count]->fcmd = arch_false_e;
            }
            else if(isValidFC(opInfo[count]->op))
            {
                opInfo[count]->calcinx = graphInfo[count]->origInX;
                opInfo[count]->calciny = graphInfo[count]->origInY;

                /* out */
                opInfo[count]->origoutx = graphInfo[count]->nnOutX;
                opInfo[count]->origouty = graphInfo[count]->nnOutY;
                opInfo[count]->oz = graphInfo[count]->nnOutZ;

                opInfo[count]->xsize = opInfo[count]->origoutx;
                opInfo[count]->ysize = opInfo[count]->origouty;

                opInfo[count]->fcmd = arch_false_e;
            }
            else
            {
                continue;
            }

            opInfo[count]->xpad = (-1) * graphInfo[count]->xOffset;            /* for calculate xOffset*/
            opInfo[count]->ypad = (-1) * graphInfo[count]->yOffset;            /* for calculate yOffset */
            opInfo[count]->siz = opInfo[count]->oz;

            opInfo[count]->bfy = 1;
            opInfo[count]->bfz = 1;

            if (count > 0 && supportNNTPParallel)
            {
                if (graphInfo[count]->opTarget == ARCHNNE_OPERATION_TARGET_TP)
                {
                    if (opInfo[count - 1]->target != ARCHNNE_OPERATION_TARGET_TP)
                    {
                        opInfo[count - 1]->bfy = 2;
                    }
                }
                else
                {
                    if (opInfo[count - 1]->target == ARCHNNE_OPERATION_TARGET_TP)
                    {
                        opInfo[count - 1]->bfy = 2;
                    }
                }
            }

            /* other */
            opInfo[count]->p3 = graphInfo[count]->pollingSize == 3 ? 1 : 0;
            opInfo[count]->pix = (arch_uint32)ceilf((arch_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
            opInfo[count]->piy = (arch_uint32)ceilf((arch_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);

            /* calculated, move to outside the function */
            if (opInfo[count]->inputDataFormat == ARCH_TYPE_INT16)
                opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountInt16;
            else if (opInfo[count]->inputDataFormat == ARCH_TYPE_FLOAT16)
                opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountFloat16;
            else
                opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCount;

            /* parent/chind info */
            opInfo[count]->upLayerCount = graphInfo[count]->upStreamLayerCount;        /* up layer count, including SH */
            opInfo[count]->upOpCount = graphInfo[count]->upStreamOpCount;            /* up OP count, real op count */
            opInfo[count]->downLayerCount = graphInfo[count]->downStreamLayerCount;
            opInfo[count]->downOpCount = graphInfo[count]->downStreamOpCount;            /* TBD */

            opInfo[count]->downStreamLayerCount = opInfo[count]->downOpCount;
            opInfo[count]->upStreamLayerCount = opInfo[count]->upOpCount;           /* Need to find out these kind of count later */

            /* update up/down stream layer info */
            opCount = graphInfo[count]->upStreamOpCount;
            for(j = 0; j < opCount; j++)
            {
                opInfo[count]->parentLayer[j] = graphInfo[count]->upStreamLayer[j];
                opInfo[count]->parentLayerType[j] = graphInfo[count]->upStreamLayerType[j];
                opInfo[count]->parentOpId[j] = graphInfo[count]->upStreamOp[j];
                opInfo[count]->parentOpType[j] = graphInfo[count]->upStreamOpType[j];

                opInfo[count]->upStreamLayer[j] = getAbsIdFromLayer(mappingTable, opInfo[count]->parentLayer[j],0, count, opInfo[count]->parentOpId[j]);
                opInfo[count]->parentAbsId[j] =  getAbsIdFromLayer(mappingTable, opInfo[count]->parentLayer[j],0, count, opInfo[count]->parentOpId[j]);
            }
            if (graphInfo[count]->upStreamOpCount == 0)
            {
                opInfo[count]->upStreamLayer[0] = -1;
                opInfo[count]->parentAbsId[0] = -1;
            }
            /*opIndex = graphInfo[count]->downStreamOpCount;*/
            opCount = graphInfo[count]->downStreamOpCount;
            /* If the downstream layer is SH, do not need to revert, currently the case either all SH, or all non-SH */
            /* not in mapping table, should be SH */
            opIndex = 0;
            for(j = 0; j < opCount; j++)
            {
                opInfo[count]->childLayer[j] = graphInfo[count]->downStreamLayer[j];
                opInfo[count]->childLayerType[j] = graphInfo[count]->downStreamLayerType[j];
                opInfo[count]->childOpId[j] = graphInfo[count]->downStreamOp[j];
                opInfo[count]->childOpType[j] = graphInfo[count]->downStreamOpType[j];

                /* If the op Abs ID is -1, means this parent layer is SH, need to move it to the end. the upStreamLayer should be all valid layer, parentAbs do not need*/
                opAbsId = getAbsIdFromLayer(mappingTable, opInfo[count]->childLayer[j], 0, totalCount, opInfo[count]->childOpId[j]);
                opInfo[count]->childAbsId[j] = opAbsId;
                if(opAbsId != -1)
                {
                    opInfo[count]->downStreamLayer[opIndex] = opAbsId;
                    opIndex++;
                }
                /*opInfo[count]->childAbsId[j] = getAbsIdFromLayer(mappingTable,opInfo[count]->childLayer[j], 0, totalCount, opInfo[count]->childOpId[j]);*/

                /* add for adjust down stream count. If the down stream is not show in low (SH), will reduce the count */
                if(opAbsId == -1 && opInfo[count]->downStreamLayerCount > 1)
                {
                    opInfo[count]->downStreamLayerCount = opInfo[count]->downStreamLayerCount -1;
                }
            }
            /* If there is SH, set the last downStreamLayer as -1 */
            if(opIndex != opCount || opIndex == 0)
            {
                opInfo[count]->childAbsId[opIndex] = -1;
                opInfo[count]->downStreamLayer[opIndex] = -1;
            }

            if (graphInfo[count]->downStreamOpCount == 0)
            {
                opInfo[count]->downStreamLayer[0] = -1;
                opInfo[count]->childAbsId[0] = -1;
            }
            /* fill perf compress info */
            opInfo[count]->perf.coefNonZeroRatio  = graphInfo[count]->coefNonZeroRatio;
            opInfo[count]->perf.coefCompressRatio = graphInfo[count]->coefCompression;
            opInfo[count]->perf.imageCompressRatio = graphInfo[count]->imageCompression;
            opInfo[count]->perf.imageNonZeroRatio  = graphInfo[count]->imageNonZeroRatio;

            /* update allSibling1x1 info, may not needed */
            if(opInfo[count]->target == ARCHNNE_OPERATION_TARGET_NN)
            {
                if(opInfo[count]->kx != 1 || opInfo[count]->ky != 1)
                {
                    opInfo[count]->perf.allSibling1x1 = 0;
                }
            }
        }
        count++;
    }

    /* after fill all opInfo,update silbling1x1 */
    opInfo[0]->perf.allSibling1x1 = 0;
    for (i = 0; i < totalCount; i++)
    {
        updateSingleAllSilbling1X1(opInfo, i, totalCount);
#ifdef DUMP_PARAMETERS
        archInfo("index %d: allSibling1x1 is %d,  SiblingHas1x1 is %d.\n",i,opInfo[i]->perf.allSibling1x1, opInfo[i]->perf.SiblingHas1x1);
#endif
    }

    /* release resource */
    if (mappingTable) archFreeMemory(mappingTable);
    return 0;
}



/***********************************************************************************
* Function:     archFillOpInfoFromExcel
* Description:  Main Entry of Arch Model Library Called from python, will call archPredictPerf
                for detail analyzing
* Input:        graphInfo:
*               pArchNnConfig:
*               pArchOptions
* Ouput:        arch_status:
***************************************************************************************/
arch_status archFillOpInfoFromExcel(
    archModelOpInfo **opInfo,
    archModelGraphInfo ** graphInfo,
    arch_nn_config *pArchNnConfig,
    arch_uint32 totalCount
    )
{
    arch_uint32 i = 0, j = 0, tempCount = 0;
    arch_uint32 count = 0;
    arch_bool supportNNTPParallel = arch_false_e;
    archSTATUS status = archSTATUS_OK;

    /* update allSibling1x1 info */
    opInfo[0]->perf.allSibling1x1 = 0;

    /* Fill data for every node */
    for (i = 0; i < totalCount; i++)
    {
        {
            /*opInfo[count]->tpType = operation->parameter.tpType;*/   /* TBD */
            /* non-related to graphInfo */
            opInfo[count]->swTilingType = -1;

            /* Basic information */
            opInfo[count]->absoluteOperationID = graphInfo[count]->absId;
            opInfo[count]->layerId = graphInfo[count]->layerId;
            opInfo[count]->layerName = archGetLayerName(graphInfo[count]->layerType);
            opInfo[count]->operationId = 0;                                                 /* set op to 0 */
            opInfo[count]->op      = graphInfo[count]->opType;
            opInfo[count]->target  = graphInfo[count]->opTarget;

            /* kernel information */
            opInfo[count]->kx = graphInfo[count]->kx;
            opInfo[count]->ky = graphInfo[count]->ky;
            opInfo[count]->kz = graphInfo[count]->kz;

            /* Pooling info */
            opInfo[count]->psize   = graphInfo[count]->pollingSize;
            opInfo[count]->pstride = graphInfo[count]->pollingStride;

            /* data size */
            opInfo[count]->inputDataSize   = graphInfo[count]->inputDataSize;
            opInfo[count]->outputDataSize   = graphInfo[count]->outputDataSize;
            opInfo[count]->inputDataFormat = archGetDataFormat(graphInfo[count]->inputDataSize,graphInfo[count]->isFp16);
            opInfo[count]->outputDataFormat = archGetDataFormat(graphInfo[count]->outputDataSize,graphInfo[count]->isFp16);

            opInfo[count]->stridex = graphInfo[count]->stridex;
            opInfo[count]->stridey = graphInfo[count]->stridey;

            /* init buf */
            opInfo[count]->sbuf = SW_TILING_FROM_AXI_SRAM;
            opInfo[count]->dbuf = SW_TILING_FROM_AXI_SRAM;
            opInfo[count]->kbuf = SW_TILING_FROM_VIP_SRAM;


            /* Since all data from log or excel should be valid, only need to check the target and operation type */
            if(opInfo[count]->target == ARCHNNE_OPERATION_TARGET_NN)
            {
                /* calc NN input */
                opInfo[count]->inx = graphInfo[count]->nnOutX + graphInfo[count]->kx -1 + 2 * graphInfo[count]->xOffset;
                opInfo[count]->iny = graphInfo[count]->nnOutY + graphInfo[count]->ky -1 + 2 * graphInfo[count]->yOffset;
                opInfo[count]->inz = graphInfo[count]->kz;

                opInfo[count]->calcinx = graphInfo[count]->nnOutX;
                opInfo[count]->calciny = graphInfo[count]->nnOutY;
                opInfo[count]->oz = graphInfo[count]->nnOutZ;

                /* out */
                opInfo[count]->origoutx = opInfo[count]->calcinx;    /*graphInfo[count]->finalOutX*/
                opInfo[count]->origouty = opInfo[count]->calciny;    /*graphInfo[count]->finalOutY*/

                opInfo[count]->xsize = opInfo[count]->calcinx;
                opInfo[count]->ysize = opInfo[count]->calciny;

                opInfo[count]->fcmd = arch_true_e;

            }
            else if (opInfo[count]->target == ARCHNNE_OPERATION_TARGET_TP && !(isValidFC(opInfo[count]->op)))
            {
                /* calc TP input */
                opInfo[count]->inx = graphInfo[count]->nnOutX;
                opInfo[count]->iny = graphInfo[count]->nnOutY;
                opInfo[count]->inz = graphInfo[count]->kz;

                if(graphInfo[count]->kz != graphInfo[count]->nnOutZ)
                {
                    /* may meet resharper */
                    opInfo[count]->inx = (graphInfo[count]->nnOutX * graphInfo[count]->nnOutZ)/graphInfo[count]->kz;
                }

                opInfo[count]->calcinx = opInfo[count]->inx;
                opInfo[count]->calciny = opInfo[count]->iny;

                opInfo[count]->origoutx = graphInfo[count]->nnOutX;
                opInfo[count]->origouty = graphInfo[count]->nnOutY;
                opInfo[count]->oz = graphInfo[count]->nnOutZ;
                if (graphInfo[count]->opType == ARCHNNE_OPERATOR_POOLING)
                {
                    opInfo[count]->xsize = opInfo[count]->inx;
                    opInfo[count]->ysize = opInfo[count]->iny;
                }
                else
                {
                    opInfo[count]->xsize = opInfo[count]->origoutx;
                    opInfo[count]->ysize = opInfo[count]->origouty;
                }

                opInfo[count]->fcmd = arch_false_e;
            }
            else if(isValidFC(opInfo[count]->op))
            {
                /* calc FC input */
                opInfo[count]->inx = graphInfo[count]->nnOutX;
                opInfo[count]->iny = graphInfo[count]->nnOutY;
                opInfo[count]->inz = graphInfo[count]->kz;

                opInfo[count]->calcinx = opInfo[count]->inx;
                opInfo[count]->calciny = opInfo[count]->iny;

                /* out */
                opInfo[count]->origoutx = graphInfo[count]->nnOutX;
                opInfo[count]->origouty = graphInfo[count]->nnOutY;
                opInfo[count]->oz = graphInfo[count]->nnOutZ;

                opInfo[count]->xsize = opInfo[count]->origoutx;
                opInfo[count]->ysize = opInfo[count]->origouty;

                opInfo[count]->fcmd = arch_false_e;
            }
            else
            {
                continue;
            }

            /* Xpad/Ypad */
            opInfo[count]->xpad = (-1) * graphInfo[count]->xOffset;            /* for calculate xOffset*/
            opInfo[count]->ypad = (-1) * graphInfo[count]->yOffset;            /* for calculate yOffset */
            opInfo[count]->siz = opInfo[count]->oz;

            opInfo[count]->bfy = 1;
            opInfo[count]->bfz = 1;

            if (count > 0 && supportNNTPParallel)
            {
                if (graphInfo[count]->opTarget == ARCHNNE_OPERATION_TARGET_TP)
                {
                    if (opInfo[count - 1]->target != ARCHNNE_OPERATION_TARGET_TP)
                    {
                        opInfo[count - 1]->bfy = 2;
                    }
                }
                else
                {
                    if (opInfo[count - 1]->target == ARCHNNE_OPERATION_TARGET_TP)
                    {
                        opInfo[count - 1]->bfy = 2;
                    }
                }
            }

            /* other */
            opInfo[count]->p3 = graphInfo[count]->pollingSize == 3 ? 1 : 0;
            opInfo[count]->pix = (arch_uint32)ceilf((arch_float32)(opInfo[count]->xsize - opInfo[count]->p3) / opInfo[count]->pstride);
            opInfo[count]->piy = (arch_uint32)ceilf((arch_float32)(opInfo[count]->ysize - opInfo[count]->p3) / opInfo[count]->pstride);

            /* calculated, move to outside the function */
            if (opInfo[count]->inputDataFormat == ARCH_TYPE_INT16)
                opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountInt16;
            else if (opInfo[count]->inputDataFormat == ARCH_TYPE_FLOAT16)
                opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCountFloat16;
            else
                opInfo[count]->nnCores = pArchNnConfig->fixedFeature.nnCoreCount;


            /* check if upstream is valid */
            if (graphInfo[count]->upStreamLayer[0] == 0 && graphInfo[count]->upStreamLayerCount > 0)
            {
                /* upstream layer invalid */
                graphInfo[count]->upStreamLayerCount = 0;
                graphInfo[count]->upStreamLayer[0] = -1;
            }
            /* parent/chind info */
            opInfo[count]->upLayerCount = graphInfo[count]->upStreamLayerCount;        /* up layer count, including SH */
            opInfo[count]->upOpCount = graphInfo[count]->upStreamLayerCount;            /* up OP count, real op count */
            opInfo[count]->upStreamLayerCount = graphInfo[count]->upStreamLayerCount;

            /* the upstream layer info is reverted in excel */
            tempCount = graphInfo[count]->upStreamLayerCount;
            for(j = 0; j < tempCount; j++)
            {
                /*graphInfo[count]->upStreamLayer[j] = graphInfo[count]->upStreamLayer[tempCount-j-1] - 1;*/
                opInfo[count]->upStreamLayer[j] = graphInfo[count]->upStreamLayer[j]-1;
                opInfo[count]->parentAbsId[j] = graphInfo[count]->upStreamLayer[tempCount-j-1] - 1;
            }

            /* fill perf compress info */
            opInfo[count]->perf.coefNonZeroRatio  = graphInfo[count]->coefNonZeroRatio;
            opInfo[count]->perf.coefCompressRatio = graphInfo[count]->coefCompression;
            opInfo[count]->perf.imageCompressRatio = graphInfo[count]->imageCompression;
            opInfo[count]->perf.imageNonZeroRatio  = graphInfo[count]->imageNonZeroRatio;

            /* update allSibling1x1 info, may not needed */
            if(opInfo[count]->target == ARCHNNE_OPERATION_TARGET_NN)
            {
                if(opInfo[count]->kx != 1 || opInfo[count]->ky != 1)
                {
                    opInfo[count]->perf.allSibling1x1 = 0;
                }
            }
        }
        count++;
    }

    /* update operation ID */
    archUpdateOperationID(opInfo, totalCount);

    /* update up down relation */
    archUpdateUpDownLayerInfo(opInfo, totalCount);

    /* after fill all opInfo,update silbling1x1 */
    opInfo[0]->perf.allSibling1x1 = 0;
    for (i = 0; i < totalCount; i++)
    {
        updateSingleAllSilbling1X1(opInfo, i, totalCount);
#ifdef DUMP_PARAMETERS
        archInfo("index %d: allSibling1x1 is %d,  SiblingHas1x1 is %d.\n",i,opInfo[i]->perf.allSibling1x1, opInfo[i]->perf.SiblingHas1x1);
#endif
    }


    return status;
}


/***********************************************************************************
* Function:     deInitArchGraphInfo
* Description:  DeInit the graphInfo struction and free the memory
* Input:        archModelGraphInfo:
*               operationCount:
* Ouput:        NULL
***************************************************************************************/
void deInitArchGraphInfo(
    archModelGraphInfo **archGraphInfo,
    arch_uint32 operationCount
    )
{
    arch_uint32 i;
    /* free every OpInfo, total count is "operationCount" */
    for (i = 0; i < operationCount; i++)
    {
        if (archGraphInfo && archGraphInfo[i] != NULL)
        {
            archFreeMemory(archGraphInfo[i]);
        }
    }

    if (archGraphInfo != NULL) archFreeMemory(archGraphInfo);
}

/***********************************************************************************
* Function:     initArchOpInfo
* Description:  Init the OpInfo struction, allocate the memory for both OpInfo struct and every OpInfo
* Input:        operationCount:        Total operation count
* Ouput:        archModelGraphInfo**:  return the Opinfo pointer
***************************************************************************************/
archModelGraphInfo ** initArchGraphInfo(
    arch_uint32 operationCount
    )
{
    archSTATUS status = archSTATUS_OK;
    arch_uint32 i = 0;
    archModelGraphInfo **archGraphInfo = NULL;

    status = archAllocateMemory(sizeof(archModelGraphInfo *) * operationCount,(archPOINTER *)&archGraphInfo);
    if (archIS_ERROR(status)) goto error;

    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(archModelGraphInfo),(archPOINTER *)&archGraphInfo[i]);
        if (archIS_ERROR(status)) goto error;
        memset(archGraphInfo[i], 0, sizeof(archModelGraphInfo));
    }

    return archGraphInfo;

error:
    if (archGraphInfo != NULL) {
        deInitArchGraphInfo(archGraphInfo, operationCount);
    }
    archInfo("ERROR: initArchGraphInfo() return out-of-memory\n");
    return NULL;
}



/***********************************************************************************
* Function:     deInitArchPerfResult
* Description:  DeInit the perf result struction and free the memory
* Input:        archPerfResult:
*               operationCount:
* Ouput:        NULL
***************************************************************************************/
void deInitArchPerfResult(
    arch_perf_result_s **archPerfResult,
    arch_uint32 operationCount
    )
{
    arch_uint32 i;
    /* free every OpInfo, total count is "operationCount" */
    for (i = 0; i < operationCount; i++)
    {
        if (archPerfResult && archPerfResult[i] != NULL)
        {
            archFreeMemory(archPerfResult[i]);
        }
    }

    if (archPerfResult != NULL) archFreeMemory(archPerfResult);
}

/***********************************************************************************
* Function:     initArchPerfResult
* Description:  Init the perf result struction, allocate the memory.
* Input:        operationCount:        Total operation count
* Ouput:        arch_perf_result_s**:  return the perf result pointer
***************************************************************************************/
arch_perf_result_s ** initArchPerfResult(arch_uint32 operationCount)
{
    archSTATUS status = archSTATUS_OK;
    arch_uint32 i = 0;
    arch_perf_result_s **archPerfResult = NULL;

    status = archAllocateMemory(sizeof(arch_perf_result_s *) * operationCount,(archPOINTER *)&archPerfResult);
    if (archIS_ERROR(status)) goto error;

    for (i = 0; i < operationCount; i++)
    {
        status = archAllocateMemory(sizeof(arch_perf_result_s),(archPOINTER *)&archPerfResult[i]);
        if (archIS_ERROR(status)) goto error;
        memset(archPerfResult[i], 0, sizeof(arch_perf_result_s));
    }

    return archPerfResult;

error:
    if (archPerfResult != NULL) {
        deInitArchPerfResult(archPerfResult, operationCount);
    }
    archInfo("ERROR: initArchPerfResult() return out-of-memory\n");
    return NULL;
}


/* Since there is only upstream layer info in excel, need to calc down stream depends on up stream */
arch_status calcDownStreamInfo(archModelGraphInfo ** graphInfo, arch_uint32 totalCount)
{
    arch_uint32 i = 0, j = 0, upLayerCount = 0, layerIndex = 0, k = 0;

    for(i = 0; i< totalCount; i++)
    {
        /* Loop every layer */
        upLayerCount = graphInfo[i]->upStreamLayerCount;
        for(j = 0; j < upLayerCount; j++)
        {
            /* check every upstreamlayer, and current layer should be the downstreamlayer of the upstreamlayer */
            layerIndex = graphInfo[i]->upStreamLayer[j];
            k = graphInfo[layerIndex]->downStreamLayerCount;
            graphInfo[layerIndex]->downStreamLayer[k] = i;
            k++;
            graphInfo[layerIndex]->downStreamLayerCount  = k;
        }
    }
    return 0;
}

/***********************************************************************************
* Function:     initArchSwLibContext/DeInitArchSwLibContext
* Description:  Init/Deinit function of Arch Model Library Called from python
* Input:        totalCount:
* Ouput:        archSwLibContext *:
***************************************************************************************/

arch_status DeInitArchSwLibContext(
    archSwLibContext *pArchSwLibContext,
    arch_uint32 totalCount
    )
{
    arch_status status = 0;

    if(pArchSwLibContext != NULL)
    {
        /* Arch Data Feature */
        if(pArchSwLibContext->pArchDataFeature != NULL)
            archFreeMemory(pArchSwLibContext->pArchDataFeature);

        /* Arch NN Config */
        if(pArchSwLibContext->pArchNnConfig != NULL)
            archFreeMemory(pArchSwLibContext->pArchNnConfig);

        /* Arch Option */
        if(pArchSwLibContext->pArchOptions != NULL)
            archFreeMemory(pArchSwLibContext->pArchOptions);

        /* Chip Identify */
        if(pArchSwLibContext->pChipIdentity != NULL)
            archFreeMemory(pArchSwLibContext->pChipIdentity);

        /*deInit Graph*/
        deInitArchGraphInfo(pArchSwLibContext->graphInfo, totalCount);

        /*deInit Perf Result*/
        deInitArchPerfResult(pArchSwLibContext->archPerfResult, totalCount);
    }

    return status;
}

archSwLibContext * initArchSwLibContext(
    arch_uint32 totalCount
    )
{
    archSTATUS status = archSTATUS_OK;
    archSwLibContext *pArchSwLibContext = &gArchSwLibContext;
    /* Init graphInfo */
    pArchSwLibContext->graphInfo = initArchGraphInfo(totalCount);
    if(pArchSwLibContext->graphInfo == NULL)
        goto error;

    /* Init parameters */
    pArchSwLibContext->totalCount = totalCount;
    /* Arch Data Feature */
    status = archAllocateMemory(sizeof(archNN_DATABASE_FEATURE),(archPOINTER *)&(pArchSwLibContext->pArchDataFeature));
    if(archIS_ERROR(status) || pArchSwLibContext->pArchDataFeature == NULL)
        goto error;
    memset(pArchSwLibContext->pArchDataFeature,0,sizeof(archNN_DATABASE_FEATURE));

    /* Arch NN Config */
    status = archAllocateMemory(sizeof(arch_nn_config),(archPOINTER *)&(pArchSwLibContext->pArchNnConfig));
    if(archIS_ERROR(status) || pArchSwLibContext->pArchNnConfig == NULL)
        goto error;
    memset(pArchSwLibContext->pArchNnConfig,0,sizeof(arch_nn_config));

    /* Arch Option */
    status = archAllocateMemory(sizeof(arch_drv_option),(archPOINTER *)&(pArchSwLibContext->pArchOptions));
    if(archIS_ERROR(status) || pArchSwLibContext->pArchOptions == NULL)
        goto error;
    memset(pArchSwLibContext->pArchOptions,0,sizeof(arch_drv_option));

    /* Chip Identify */
    status = archAllocateMemory(sizeof(archHAL_CHIPIDENTITY),(archPOINTER *)&(pArchSwLibContext->pChipIdentity));
    if(archIS_ERROR(status) || pArchSwLibContext->pChipIdentity == NULL)
        goto error;
    memset(pArchSwLibContext->pChipIdentity,0,sizeof(archHAL_CHIPIDENTITY));

    /* Init arch perf result struction */
    pArchSwLibContext->archPerfResult = initArchPerfResult(totalCount);
    if(pArchSwLibContext->archPerfResult == NULL)
        goto error;

    return pArchSwLibContext;
error:
    DeInitArchSwLibContext(pArchSwLibContext,totalCount);
    return NULL;
}


#if DUMP_PARAMETERS
/* for test only */
void printConfig(arch_nn_config *pArchNnConfig,arch_drv_option *pArchOptions, archHAL_CHIPIDENTITY *pChipIdentity,archNN_DATABASE_FEATURE *pArchDataFeature)
{
    /* arch_nn_config */
    archInfo("************************************  arch_nn_config  ********************************************\n");
    archInfo("isSet is %d.\n",pArchNnConfig->isSet);
    /* archNN_FIXED_FEATURE */
    archInfo("============================== archNN_FIXED_FEATURE ==============================\n");
    archInfo("vipCoreCount is %d.\n",pArchNnConfig->fixedFeature.vipCoreCount);
    archInfo("nnCoreCount is %d.\n",pArchNnConfig->fixedFeature.nnCoreCount);
    archInfo("nnCoreCountInt8 is %d.\n",pArchNnConfig->fixedFeature.nnCoreCountInt8);
    archInfo("nnCoreCountInt16 is %d.\n",pArchNnConfig->fixedFeature.nnCoreCountInt16);
    archInfo("nnCoreCountFloat16 is %d.\n",pArchNnConfig->fixedFeature.nnCoreCountFloat16);
    archInfo("nnMadPerCore is %d.\n",pArchNnConfig->fixedFeature.nnMadPerCore);
    archInfo("nnInputBufferDepth is %d.\n",pArchNnConfig->fixedFeature.nnInputBufferDepth);
    archInfo("nnAccumBufferDepth is %d.\n",pArchNnConfig->fixedFeature.nnAccumBufferDepth);
    archInfo("nnFCNonPrunAccel is %d.\n",pArchNnConfig->fixedFeature.nnFCNonPrunAccel);
    archInfo("nnInImageOffsetBits is %d.\n",pArchNnConfig->fixedFeature.nnInImageOffsetBits);
    archInfo("tpCoreCount is %d.\n",pArchNnConfig->fixedFeature.tpCoreCount);
    archInfo("tpPwlLUTCount is %d.\n",pArchNnConfig->fixedFeature.tpPwlLUTCount);
    archInfo("tpPwlLUTSize is %d.\n",pArchNnConfig->fixedFeature.tpPwlLUTSize);
    archInfo("vip7Version is %d.\n",pArchNnConfig->fixedFeature.vip7Version);
    archInfo("vipBrickMode is %d.\n",pArchNnConfig->fixedFeature.vipBrickMode);
    archInfo("tpReorderInImageSize is %d.\n",pArchNnConfig->fixedFeature.tpReorderInImageSize);
    archInfo("tpliteCoreCount is %d.\n",pArchNnConfig->fixedFeature.tpliteCoreCount);
    archInfo("nnFP16XYDPX is %d.\n",pArchNnConfig->fixedFeature.nnFP16XYDPX);
    archInfo("nnFP16XYDPY is %d.\n",pArchNnConfig->fixedFeature.nnFP16XYDPY);
    archInfo("nnFP16ZDP is %d.\n",pArchNnConfig->fixedFeature.nnFP16ZDP);
    archInfo("zrlBits is %d.\n",pArchNnConfig->fixedFeature.zrlBits);
    archInfo("uscCacheControllers is %d.\n",pArchNnConfig->fixedFeature.uscCacheControllers);
    archInfo("uscBanks is %d.\n",pArchNnConfig->fixedFeature.uscBanks);
    archInfo("nnLanesPerOutCycle is %d.\n",pArchNnConfig->fixedFeature.nnLanesPerOutCycle);
    archInfo("maxOTNumber is %d.\n",pArchNnConfig->fixedFeature.maxOTNumber);
    archInfo("equivalentVipsramWidthInByte is %d.\n",pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte);
    archInfo("shaderCoreCount is %d.\n",pArchNnConfig->fixedFeature.shaderCoreCount);

    /* archNN_CUSTOMIZED_FEATURE */
    archInfo("============================== archNN_CUSTOMIZED_FEATURE ==============================\n");
    archInfo("vipSRAMSize is 0x%x.\n",pArchNnConfig->customizedFeature.vipSRAMSize);
    archInfo("axiSRAMSize is 0x%x.\n",pArchNnConfig->customizedFeature.axiSRAMSize);
    archInfo("ddrReadBWLimit is %f.\n",pArchNnConfig->customizedFeature.ddrReadBWLimit);
    archInfo("ddrWriteBWLimit is %f.\n",pArchNnConfig->customizedFeature.ddrWriteBWLimit);
    archInfo("ddrTotalBWLimit is %f.\n",pArchNnConfig->customizedFeature.ddrTotalBWLimit);
    archInfo("axiSramReadBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiSramReadBWLimit);
    archInfo("axiSramWriteBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiSramWriteBWLimit);
    archInfo("axiSramTotalBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiSramTotalBWLimit);
    archInfo("axiBusReadBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiBusReadBWLimit);
    archInfo("axiBusWriteBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiBusWriteBWLimit);
    archInfo("axiBusTotalBWLimit is %f.\n",pArchNnConfig->customizedFeature.axiBusTotalBWLimit);
    archInfo("vipSWTiling is %d.\n",pArchNnConfig->customizedFeature.vipSWTiling);
    archInfo("ddrLatency is %d.\n",pArchNnConfig->customizedFeature.ddrLatency);
    archInfo("freqInMHZ is %d.\n",pArchNnConfig->customizedFeature.freqInMHZ);
    archInfo("axiClockFreqInMHZ is %d.\n",pArchNnConfig->customizedFeature.axiClockFreqInMHZ);
    archInfo("maxSocOTNumber is %d.\n",pArchNnConfig->customizedFeature.maxSocOTNumber);
    archInfo("nnWriteWithoutUSC is %d.\n",pArchNnConfig->customizedFeature.nnWriteWithoutUSC);
    archInfo("depthWiseSupport is %d.\n",pArchNnConfig->customizedFeature.depthWiseSupport);
    archInfo("vipVectorPrune is %d.\n",pArchNnConfig->customizedFeature.vipVectorPrune);

    /* archNN_UNIFIED_FEATURE */
    archInfo("============================== archNN_UNIFIED_FEATURE ==============================\n");
    archInfo("nnUSCCacheSize is %d.\n",pArchNnConfig->unifiedFeature.nnUSCCacheSize);
    archInfo("nnCmdSizeInBytes is %d.\n",pArchNnConfig->unifiedFeature.nnCmdSizeInBytes);
    archInfo("tpCmdSizeInBytes is %d.\n",pArchNnConfig->unifiedFeature.tpCmdSizeInBytes);
    archInfo("vipCoefDecodePerf is %d.\n",pArchNnConfig->unifiedFeature.vipCoefDecodePerf);
    archInfo("vipCachedReadFromSram is %d.\n",pArchNnConfig->unifiedFeature.vipCachedReadFromSram);
    archInfo("vipImagePartialCache is %d.\n",pArchNnConfig->unifiedFeature.vipImagePartialCache);
    archInfo("lanesPerConv is %d.\n",pArchNnConfig->unifiedFeature.lanesPerConv);
    archInfo("maxTileSize is %d.\n",pArchNnConfig->unifiedFeature.maxTileSize);
    archInfo("fullCacheKernelHeadFix is %d.\n",pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix);
    archInfo("conv1x1HalfPerformance is %d.\n",pArchNnConfig->unifiedFeature.conv1x1HalfPerformance);
    archInfo("per3DTileBubbleFix is %d.\n",pArchNnConfig->unifiedFeature.per3DTileBubbleFix);
    archInfo("cacheLineModeDisabled is %d.\n",pArchNnConfig->unifiedFeature.cacheLineModeDisabled);
    archInfo("tpReOrderFix is %d.\n",pArchNnConfig->unifiedFeature.tpReOrderFix);
    archInfo("zdp3NoCompressFix is %d.\n",pArchNnConfig->unifiedFeature.zdp3NoCompressFix);
    archInfo("asyncCopyPerfFix is %d.\n",pArchNnConfig->unifiedFeature.asyncCopyPerfFix);
    archInfo("accurateTileBW is %d.\n",pArchNnConfig->unifiedFeature.accurateTileBW);
    archInfo("zxdp3KernelReadConflictFix is %d.\n",pArchNnConfig->unifiedFeature.zxdp3KernelReadConflictFix);
    archInfo("axiSramSlowedDownByAddr is %d.\n",pArchNnConfig->unifiedFeature.axiSramSlowedDownByAddr);
    archInfo("slowNNReqArbitrationFix is %d.\n",pArchNnConfig->unifiedFeature.slowNNReqArbitrationFix);
    archInfo("singlePortAccBuffer is %d.\n",pArchNnConfig->unifiedFeature.singlePortAccBuffer);
    archInfo("convOutFifoDepthFix is %d.\n",pArchNnConfig->unifiedFeature.convOutFifoDepthFix);
    archInfo("smallBatchEnable is %d.\n",pArchNnConfig->unifiedFeature.smallBatchEnable);
    archInfo("axiSramOnlySWTiling is %d.\n",pArchNnConfig->unifiedFeature.axiSramOnlySWTiling);
    archInfo("imageNotPackedInSram is %d.\n",pArchNnConfig->unifiedFeature.imageNotPackedInSram);
    archInfo("coefDeltaCordOverFlowZRL8BitFix is %d.\n",pArchNnConfig->unifiedFeature.coefDeltaCordOverFlowZRL8BitFix);
    archInfo("lowEfficiencyOfIDWriteImgBufFix is %d.\n",pArchNnConfig->unifiedFeature.lowEfficiencyOfIDWriteImgBufFix);
    archInfo("xyOffsetLimitationFix is %d.\n",pArchNnConfig->unifiedFeature.xyOffsetLimitationFix);
    archInfo("kernelPerCoreLTOneThirdCoefFix is %d.\n",pArchNnConfig->unifiedFeature.kernelPerCoreLTOneThirdCoefFix);

    /* archNN_DERIVED_FEATURE */
    archInfo("============================== archNN_DERIVED_FEATURE ==============================\n");
    archInfo("nnDPAmount is %d.\n",pArchNnConfig->derivedFeature.nnDPAmount);
    archInfo("nnXYDPX is %d.\n",pArchNnConfig->derivedFeature.nnXYDPX);
    archInfo("nnXYDPY is %d.\n",pArchNnConfig->derivedFeature.nnXYDPY);
    archInfo("nnZDP is %d.\n",pArchNnConfig->derivedFeature.nnZDP);
    archInfo("totalLatency is %d.\n",pArchNnConfig->derivedFeature.totalLatency);
    archInfo("internalLatency is %d.\n",pArchNnConfig->derivedFeature.internalLatency);
    archInfo("ddrReadBWInBytePerCycle is %f.\n",pArchNnConfig->derivedFeature.ddrReadBWInBytePerCycle);
    archInfo("ddrWriteBWInBytePerCycle is %f.\n",pArchNnConfig->derivedFeature.ddrWriteBWInBytePerCycle);

    /* arch_drv_option */
    archInfo("************************************  arch_drv_option  ********************************************\n");
    archInfo("enableTP is %d.\n",pArchOptions->enableTP);
    archInfo("enableMultiTP is %d.\n",pArchOptions->enableMultiTP);
    archInfo("flagTPFunc is %s.\n",pArchOptions->flagTPFunc);
    archInfo("typeTPFunc is %s.\n",pArchOptions->typeTPFunc);
    archInfo("enableSRAM is %d.\n",pArchOptions->enableSRAM);
    archInfo("enableSramStreamMode is %d.\n",pArchOptions->enableSramStreamMode);
    archInfo("enableCNNPerf is %d.\n",pArchOptions->enableCNNPerf);
    archInfo("enableBrickMode is %d.\n",pArchOptions->enableBrickMode);
    archInfo("enableNonZeroBalance is %d.\n",pArchOptions->enableNonZeroBalance);
    archInfo("enableBorderMode is %d.\n",pArchOptions->enableBorderMode);
    archInfo("enableTPReorder is %d.\n",pArchOptions->enableTPReorder);
    archInfo("enableTPInterleave8 is %d.\n",pArchOptions->enableTPInterleave8);
    archInfo("enableTPRTNE is %d.\n",pArchOptions->enableTPRTNE);
    archInfo("enableShader is %d.\n",pArchOptions->enableShader);
    archInfo("enableNNXYDP9 is %d.\n",pArchOptions->enableNNXYDP9);
    archInfo("enableNNXYDP6 is %d.\n",pArchOptions->enableNNXYDP6);
    archInfo("enableSwtilingPhase1 is %d.\n",pArchOptions->enableSwtilingPhase1);
    archInfo("enableSwtilingPhase2 is %d.\n",pArchOptions->enableSwtilingPhase2);
    archInfo("enableSwtilingPhase3 is %d.\n",pArchOptions->enableSwtilingPhase3);
    archInfo("enableHandleBranch is %d.\n",pArchOptions->enableHandleBranch);
    archInfo("enableNNFirstPixelPooling is %d.\n",pArchOptions->enableNNFirstPixelPooling);
    archInfo("enableNNDepthWiseSupport is %d.\n",pArchOptions->enableNNDepthWiseSupport);
    archInfo("enablePrintOperaTarget is %d.\n",pArchOptions->enablePrintOperaTarget);
    archInfo("enableSaveBinary is %d.\n",pArchOptions->enableSaveBinary);
    archInfo("enableGraphCommandBuffer is %d.\n",pArchOptions->enableGraphCommandBuffer);
    archInfo("nnFormulaOpt is %d.\n",pArchOptions->nnFormulaOpt);
    archInfo("ddrLatency is %d.\n",pArchOptions->ddrLatency);
    archInfo("ddrReadBWLimit is %d.\n",pArchOptions->ddrReadBWLimit);
    archInfo("ddrWriteBWLimit is %d.\n",pArchOptions->ddrWriteBWLimit);
    archInfo("ddrTotalBWLimit is %d.\n",pArchOptions->ddrTotalBWLimit);
    archInfo("axiSramReadBWLimit is %d.\n",pArchOptions->axiSramReadBWLimit);
    archInfo("axiSramWriteBWLimit is %d.\n",pArchOptions->axiSramWriteBWLimit);
    archInfo("axiSramTotalBWLimit is %d.\n",pArchOptions->axiSramTotalBWLimit);
    archInfo("axiBusReadBWLimit is %d.\n",pArchOptions->axiBusReadBWLimit);
    archInfo("axiBusWriteBWLimit is %d.\n",pArchOptions->axiBusWriteBWLimit);
    archInfo("axiBusTotalBWLimit is %d.\n",pArchOptions->axiBusTotalBWLimit);
    archInfo("vipSRAMSize is 0x%x.\n",pArchOptions->vipSRAMSize);
    archInfo("axiSRAMSize is 0x%x.\n",pArchOptions->axiSRAMSize);
    archInfo("graphPerfLogFile is %s.\n",pArchOptions->graphPerfLogFile);
    archInfo("nnZeroRunLen is %d.\n",pArchOptions->nnZeroRunLen);
    archInfo("tpZeroRunLen is %d.\n",pArchOptions->tpZeroRunLen);
    archInfo("enableNNArchPerfPrint is %d.\n",pArchOptions->enableNNArchPerfPrint);
    archInfo("enableNNLayerDump is %d.\n",pArchOptions->enableNNLayerDump);
    archInfo("enableInterleave8 is %d.\n",pArchOptions->enableInterleave8);
    archInfo("nnRoundingMode is %s.\n",pArchOptions->nnRoundingMode);
    archInfo("vxcShaderSourcePath is %s.\n",pArchOptions->vxcShaderSourcePath);
    archInfo("fcZMax is %d.\n",pArchOptions->fcZMax);
    archInfo("enableMemPool is %d.\n",pArchOptions->enableMemPool);
    archInfo("memPoolSize is %d.\n",pArchOptions->memPoolSize);
    archInfo("collectPerfType is %d.\n",pArchOptions->collectPerfType);
    archInfo("enableGraphAdapter is %d.\n",pArchOptions->enableGraphAdapter);
    archInfo("enableZdpOpt is %d.\n",pArchOptions->enableZdpOpt);
    archInfo("do1xnAfterSwtiling is %d.\n",pArchOptions->do1xnAfterSwtiling);
    archInfo("nn1x1To1xN is %d.\n",pArchOptions->nn1x1To1xN);
    archInfo("enableGraphTranform is %d.\n",pArchOptions->enableGraphTranform);
    archInfo("enableGraphWAR7 is %d.\n",pArchOptions->enableGraphWAR7);
    archInfo("enableGraphPadConv is %d.\n",pArchOptions->enableGraphPadConv);
    archInfo("enableGraphMerge is %d.\n",pArchOptions->enableGraphMerge);
    archInfo("enableGraphDump is %d.\n",pArchOptions->enableGraphDump);
    archInfo("enableTransformNMConv is %d.\n",pArchOptions->enableTransformNMConv);
    archInfo("enableGraphConvertAvgPool2Conv is %d.\n",pArchOptions->enableGraphConvertAvgPool2Conv);
    archInfo("enableGraphUnrollDWConv is %d.\n",pArchOptions->enableGraphUnrollDWConv);
    archInfo("enableGraphOptimizationToTest is %d.\n",pArchOptions->enableGraphOptimizationToTest);
    archInfo("enableGraphConvertBatchFC2NNConv is %d.\n",pArchOptions->enableGraphConvertBatchFC2NNConv);
    archInfo("enableGraphConvertTensorAdd is %d.\n",pArchOptions->enableGraphConvertTensorAdd);
    archInfo("enableGraphEltwiseOpShape is %d.\n",pArchOptions->enableGraphEltwiseOpShape);
    archInfo("enableGraphConvertConv2Fc is %d.\n",pArchOptions->enableGraphConvertConv2Fc);
    archInfo("enableGraphSwaplayer is %d.\n",pArchOptions->enableGraphSwaplayer);
    archInfo("enableGraphReshapelayer is %d.\n",pArchOptions->enableGraphReshapelayer);
    archInfo("enableGraphConcalayer is %d.\n",pArchOptions->enableGraphConcalayer);
    archInfo("enableGraphMergeTranspose is %d.\n",pArchOptions->enableGraphMergeTranspose);
    archInfo("enableGraphDeleteRelu is %d.\n",pArchOptions->enableGraphDeleteRelu);
    archInfo("enableGraphDeleteSqueeze is %d.\n",pArchOptions->enableGraphDeleteSqueeze);
    archInfo("enableGraphAvgPoolandPWConv is %d.\n",pArchOptions->enableGraphAvgPoolandPWConv);
    archInfo("freqInMHZ is %d.\n",pArchOptions->freqInMHZ);
    archInfo("axiClockFreqInMHZ is %d.\n",pArchOptions->axiClockFreqInMHZ);
    archInfo("maxSocOTNumber is %d.\n",pArchOptions->maxSocOTNumber);
    archInfo("enableHuffmanEnhancement is %d.\n",pArchOptions->enableHuffmanEnhancement);
    archInfo("enableTPHuffman is %d.\n",pArchOptions->enableTPHuffman);
    archInfo("enableMultiVIPCombined is %d.\n",pArchOptions->enableMultiVIPCombined);
    /*archInfo("enableNNTPParallel is %d.\n",pArchOptions->enableNNTPParallel);*/
    archInfo("enableVectorPrune is %d.\n",pArchOptions->enableVectorPrune);
    archInfo("enableYUV2RGBScaler is %d.\n",pArchOptions->enableYUV2RGBScaler);
    archInfo("enableVIPDEC400 is %d.\n",pArchOptions->enableVIPDEC400);
    archInfo("enableCacheBinaryGraph is %d.\n",pArchOptions->enableCacheBinaryGraph);
    archInfo("enableOpsDebugInfo is %s.\n",pArchOptions->enableOpsDebugInfo);
    /*archInfo("tpCoreCount is %d.\n",pArchOptions->tpCoreCount);*/

    /* archHAL_CHIPIDENTITY */
    if(pChipIdentity != NULL)
    {
        archInfo("************************************  archHAL_CHIPIDENTITY  ********************************************\n");
        archInfo("chipModel is %d.\n",pChipIdentity->chipModel);
        archInfo("chipRevision is %d.\n",pChipIdentity->chipRevision);
        archInfo("productID is %d.\n",pChipIdentity->productID);
        archInfo("customerID is %d.\n",pChipIdentity->customerID);
        archInfo("ecoID is %d.\n",pChipIdentity->ecoID);
        archInfo("chipFlags is %d.\n",pChipIdentity->chipFlags);
        archInfo("platformFlagBits is %d.\n",pChipIdentity->platformFlagBits);
    }
    /* archNN_DATABASE_FEATURE */
    archInfo("************************************  archNN_DATABASE_FEATURE  ********************************************\n");
    archInfo("swtilingPhase1Enable is %d.\n",pArchDataFeature->swtilingPhase1Enable);
    archInfo("swtilingPhase2Enable is %d.\n",pArchDataFeature->swtilingPhase2Enable);
    archInfo("swtilingPhase3Enable is %d.\n",pArchDataFeature->swtilingPhase3Enable);
    archInfo("zdp3Enable is %d.\n",pArchDataFeature->zdp3Enable);
    archInfo("zdp6Enable is %d.\n",pArchDataFeature->zdp6Enable);
    archInfo("xydp9Enable is %d.\n",pArchDataFeature->xydp9Enable);
    archInfo("coefComEnhancement is %d.\n",pArchDataFeature->coefComEnhancement);
    archInfo("tpComEnhancement is %d.\n",pArchDataFeature->tpComEnhancement);
    archInfo("vipDec400Enable is %d.\n",pArchDataFeature->vipDec400Enable);
    archInfo("nnStrideEnable is %d.\n",pArchDataFeature->nnStrideEnable);
    archInfo("nnZdp3Enable is %d.\n",pArchDataFeature->nnZdp3Enable);
    archInfo("nnZdp6Enable is %d.\n",pArchDataFeature->nnZdp6Enable);
    archInfo("depthWiseMergeSupport is %d.\n",pArchDataFeature->depthWiseMergeSupport);
    archInfo("nnSlowOutput is %d.\n",pArchDataFeature->nnSlowOutput);
    archInfo("noNarrowPostProcessPipe is %d.\n",pArchDataFeature->noNarrowPostProcessPipe);
    archInfo("prefetchNNCommandKernelHeader is %d.\n",pArchDataFeature->prefetchNNCommandKernelHeader);
    archInfo("partialKernelCacheInternalFix is %d.\n",pArchDataFeature->partialKernelCacheInternalFix);
    archInfo("internalKernelReadBottleneckFix is %d.\n",pArchDataFeature->internalKernelReadBottleneckFix);
    archInfo("ImgPopPipelinePauseFix is %d.\n",pArchDataFeature->ImgPopPipelinePauseFix);
    archInfo("fullCacheIntervalFix is %d.\n",pArchDataFeature->fullCacheIntervalFix);
    archInfo("ddrAlign is %d.\n",pArchDataFeature->ddrAlign);
    archInfo("inlinesPerCycle is %d.\n",pArchDataFeature->inlinesPerCycle);
    archInfo("nnTranspose is %d.\n",pArchDataFeature->nnTranspose);
    archInfo("specificDDRLimitByBurst is %d.\n",pArchDataFeature->specificDDRLimitByBurst);
}

void printGraphInfo(archModelGraphInfo * GraphInfo, arch_uint32 index)
{
    arch_uint32 i = 0;
    archInfo("Index is %d, Print Graph Info:==================================================\n",index);
    archInfo("layerId is %d.\n",GraphInfo->layerId);
    archInfo("layerType is %d.\n",GraphInfo->layerType);
    archInfo("absId is %d.\n",GraphInfo->absId);
    archInfo("operationId is %d.\n",GraphInfo->operationId);
    archInfo("opType is %d.\n",GraphInfo->opType);
    archInfo("opTarget is %d.\n",GraphInfo->opTarget);

    /* up and down */
    archInfo("upStreamLayerCount is %d.\n",GraphInfo->upStreamLayerCount);
    archInfo("upStreamLayer:    \n");
    for(i = 0; i < GraphInfo->upStreamLayerCount; i++)
        archInfo("        %d: layer_id is %d, type is %d\n",i,GraphInfo->upStreamLayer[i],GraphInfo->upStreamLayerType[i]);

    archInfo("upOpCount is %d.\n",GraphInfo->upStreamOpCount);
    archInfo("upOp:    \n");
    for(i = 0; i < GraphInfo->upStreamOpCount; i++)
        archInfo("        %d: upop_id is %d, type is %d, absId is %d.\n",i,GraphInfo->upStreamOp[i],GraphInfo->upStreamOpType[i],GraphInfo->parentAbsId[i]);

    archInfo("downStreamLayerCount is %d.\n",GraphInfo->downStreamLayerCount);
    archInfo("downStreamLayer:    \n");
    for(i = 0; i < GraphInfo->downStreamLayerCount; i++)
        archInfo("        %d: op_id is %d, type is %d\n",i,GraphInfo->downStreamLayer[i],GraphInfo->downStreamLayerType[i]);

    archInfo("dpwnOpCount is %d.\n",GraphInfo->downStreamOpCount);
    archInfo("upOp:    \n");
    for(i = 0; i < GraphInfo->downStreamOpCount; i++)
        archInfo("        %d: downop_id is %d, type is %d, absId is %d.\n",i,GraphInfo->downStreamOp[i],GraphInfo->downStreamOpType[i],GraphInfo->childAbsId[i]);

    /* In/Out */
    archInfo("origInX is %d\n",GraphInfo->origInX);
    archInfo("origInY is %d\n",GraphInfo->origInY);
    archInfo("origInZ is %d\n",GraphInfo->origInZ);
    archInfo("nnOutX is %d\n",GraphInfo->nnOutX);
    archInfo("nnOutY is %d\n",GraphInfo->nnOutY);
    archInfo("nnOutZ is %d\n",GraphInfo->nnOutZ);
    archInfo("subNNOutX is %d\n",GraphInfo->subNNOutX);
    archInfo("subNNOutY is %d\n",GraphInfo->subNNOutY);
    archInfo("subNNOutZ is %d\n",GraphInfo->subNNOutZ);
    archInfo("finalOutX is %d\n",GraphInfo->finalOutX);
    archInfo("finalOutY is %d\n",GraphInfo->finalOutY);
    archInfo("finalOutZ is %d\n",GraphInfo->finalOutZ);
    archInfo("kx is %d\n",GraphInfo->kx);
    archInfo("ky is %d\n",GraphInfo->ky);
    archInfo("kz is %d\n",GraphInfo->kz);
    archInfo("pollingSize is %d\n",GraphInfo->pollingSize);
    archInfo("pollingStride is %d\n",GraphInfo->pollingStride);
    archInfo("inputDataSize is %d\n",GraphInfo->inputDataSize);
    archInfo("outputDataSize is %d\n",GraphInfo->outputDataSize);
    archInfo("isFp16 is %d\n",GraphInfo->isFp16);
    archInfo("xOffset is %d\n",GraphInfo->xOffset);
    archInfo("yOffset is %d\n",GraphInfo->yOffset);
    archInfo("coefNonZeroRatio is %f\n",GraphInfo->coefNonZeroRatio);
    archInfo("coefCompression is %f\n",GraphInfo->coefCompression);
    archInfo("imageCompression is %f\n",GraphInfo->imageCompression);
    archInfo("imageNonZeroRatio is %f\n",GraphInfo->imageNonZeroRatio);
}


void printOpInfo(
    archModelOpInfo ** archOp,
    arch_uint32 totalCount
    )
{
    arch_uint32 i = 0;
    archInfo("Print ArchOp, totalCount is %d.\n",totalCount);
    for (i = 0; i< totalCount; i++)
    {
        archInfo("\nArchOp index %d:\n",i);
        archInfo("absoluteOperationID is %d\n",archOp[i]->absoluteOperationID);
        archInfo("layerId is %d\n",archOp[i]->layerId);
        archInfo("layerName is %s\n",archOp[i]->layerName);
        archInfo("operationId is %d\n",archOp[i]->operationId);
        archInfo("upLayerCount is %d\n",archOp[i]->upLayerCount);
        archInfo("upOpCount is %d\n",archOp[i]->upOpCount);
        archInfo("parentOpId is %d -- %d -- %d\n",archOp[i]->parentOpId[0],archOp[i]->parentOpId[1],archOp[i]->parentOpId[2]);
        archInfo("parentOpType is %d -- %d -- %d\n",archOp[i]->parentOpType[0],archOp[i]->parentOpType[1],archOp[i]->parentOpType[2]);
        archInfo("parentLayer is %d -- %d -- %d\n",archOp[i]->parentLayer[0],archOp[i]->parentLayer[1],archOp[i]->parentLayer[2]);
        archInfo("parentAbsId is %d -- %d -- %d\n",archOp[i]->parentAbsId[0],archOp[i]->parentAbsId[1],archOp[i]->parentAbsId[2]);
        archInfo("parentLayerType is %d -- %d -- %d\n",archOp[i]->parentLayerType[0],archOp[i]->parentLayerType[1],archOp[i]->parentLayerType[2]);
        archInfo("downLayerCount is %d\n",archOp[i]->downLayerCount);
        archInfo("downOpCount is %d\n",archOp[i]->downOpCount);
        archInfo("childOpId is %d -- %d -- %d\n",archOp[i]->childOpId[0],archOp[i]->childOpId[1],archOp[i]->childOpId[2]);
        archInfo("childOpType is %d -- %d -- %d\n",archOp[i]->childOpType[0],archOp[i]->childOpType[1],archOp[i]->childOpType[2]);
        archInfo("childLayer is %d -- %d -- %d\n",archOp[i]->childLayer[0],archOp[i]->childLayer[1],archOp[i]->childLayer[2]);
        archInfo("childAbsId is %d -- %d -- %d\n",archOp[i]->childAbsId[0],archOp[i]->childAbsId[1],archOp[i]->childAbsId[2]);
        archInfo("childLayerType is %d -- %d -- 5d\n",archOp[i]->childLayerType[0],archOp[i]->childLayerType[1],archOp[i]->childLayerType[2]);

        archInfo("op is %d\n",archOp[i]->op);
        archInfo("target is %d\n",archOp[i]->target);
        archInfo("inx is %d\n",archOp[i]->inx);
        archInfo("iny is %d\n",archOp[i]->iny);
        archInfo("inz is %d\n",archOp[i]->inz);
        archInfo("calcinx is %d\n",archOp[i]->calcinx);
        archInfo("calciny is %d\n",archOp[i]->calciny);
        archInfo("origoutx is %d\n",archOp[i]->origoutx);
        archInfo("origouty is %d\n",archOp[i]->origouty);
        archInfo("stridex is %d\n",archOp[i]->stridex);
        archInfo("stridey is %d\n",archOp[i]->stridey);

        archInfo("kx is %d\n",archOp[i]->kx);
        archInfo("ky is %d\n",archOp[i]->ky);
        archInfo("kz is %d\n",archOp[i]->kz);
        archInfo("bfy is %d\n",archOp[i]->bfy);
        archInfo("bfz is %d\n",archOp[i]->bfz);
        archInfo("oz is %d\n",archOp[i]->oz);
        archInfo("siz is %d\n",archOp[i]->siz);

        archInfo("psize is %d\n",archOp[i]->psize);
        archInfo("pstride is %d\n",archOp[i]->pstride);
        archInfo("xpad is %d\n",archOp[i]->xpad);
        archInfo("ypad is %d\n",archOp[i]->ypad);
        archInfo("inputDataSize is %d\n",archOp[i]->inputDataSize);
        archInfo("outputDataSize is %d\n",archOp[i]->outputDataSize);
        archInfo("fcmd is %d\n",archOp[i]->fcmd);
        archInfo("inputDataFormat is %d\n",archOp[i]->inputDataFormat);
        archInfo("outputDataFormat is %d\n",archOp[i]->outputDataFormat);
        archInfo("nnCores is %d\n",archOp[i]->nnCores);
        archInfo("xsize is %d\n",archOp[i]->xsize);
        archInfo("ysize is %d\n",archOp[i]->ysize);

        archInfo("pix is %d\n",archOp[i]->pix);
        archInfo("piy is %d\n",archOp[i]->piy);
        archInfo("p3 is %d\n",archOp[i]->p3);
        archInfo("psix is %d\n",archOp[i]->psix);
        archInfo("psiy is %d\n",archOp[i]->psiy);

        archInfo("sbuf is %d\n",archOp[i]->sbuf);
        archInfo("dbuf is %d\n",archOp[i]->dbuf);
        archInfo("kbuf is %d\n",archOp[i]->kbuf);
        archInfo("swTilingSegKernelBufSizeInPixel is %d\n",archOp[i]->swTilingSegKernelBufSizeInPixel);
        archInfo("swTilingSegOutBufSizeInPixel is %d\n",archOp[i]->swTilingSegOutBufSizeInPixel);
        archInfo("segTotalBufferSizeInPixel is %d\n",archOp[i]->segTotalBufferSizeInPixel);
        archInfo("swTilingType is %d\n",archOp[i]->swTilingType);

        archInfo("upStreamLayerCount is %d\n",archOp[i]->upStreamLayerCount);
        archInfo("downStreamLayerCount is %d\n",archOp[i]->downStreamLayerCount);
        archInfo("upStreamLayer is %d -- %d -- %d\n",archOp[i]->upStreamLayer[0],archOp[i]->upStreamLayer[1],archOp[i]->upStreamLayer[2]);
        archInfo("downStreamLayer is %d -- %d -- %d\n",archOp[i]->downStreamLayer[0],archOp[i]->downStreamLayer[1],archOp[i]->downStreamLayer[2]);
    }
}
#endif

arch_uint32 updateDefaultCongfigration(arch_drv_option *pArchOptions,archNN_DATABASE_FEATURE *pArchDataFeature)
{
    pArchDataFeature->nnSlowOutput = 1;
    pArchOptions->enableSwtilingPhase1 = 1;
    pArchOptions->enableSwtilingPhase2 = 1;
    pArchOptions->enableSwtilingPhase3 = 1;
    pArchOptions->vipSRAMSize = 0xDEADDEAD;
    pArchOptions->axiSRAMSize = 0xDEADDEAD;
    pArchDataFeature->swtilingPhase1Enable = 1;
    pArchDataFeature->swtilingPhase2Enable = 1;
    pArchDataFeature->swtilingPhase3Enable = 1;
    pArchDataFeature->nnStrideEnable = 1;
    pArchOptions->enableNNArchPerfPrint = 0;
    pArchOptions->collectPerfType = 1;
    /* pArchNnConfig->unifiedFeature.convOutFifoDepthFix = 1; */

    return 0;
}

/***********************************************************************************
* Function:     archSwLibInit/archSwLibDeInit
* Description:  Init/Deinit function of Arch Model Library Called from python
* Input:        graphInfo:
*               pArchNnConfig:
*               pArchOptions
* Ouput:        arch_status:
***************************************************************************************/
archSwLibHandle archSwLibInit(arch_nn_config *pArchNnConfig,arch_drv_option *pArchOptions, arch_uint32 totalCount,archHAL_CHIPIDENTITY *pChipIdentity,archNN_DATABASE_FEATURE *pArchDataFeature, arch_uint32 flag)
{
    archSwLibContext *pArchSwLibContext = NULL;
#if gcdDEBUG
    archSetDebugLevel(ARCH_DEBUG_LEVEL_INFO);
#else
    archSetDebugLevel(ARCH_DEBUG_LEVEL_NONE);
#endif
    pArchSwLibContext = initArchSwLibContext(totalCount);
    if(pArchSwLibContext == NULL)
    {
        /* handle invalid */
        archInfo("Invalid Handle for Arch Sw Lib.\n");
        return NULL;
    }

    /* Init configration */
    /* Set some default value for configration */
    updateDefaultCongfigration(pArchOptions,pArchDataFeature);

    /* nnConfig */
    memcpy(pArchSwLibContext->pArchNnConfig,pArchNnConfig, sizeof(arch_nn_config));

    /* Options */
    memcpy(pArchSwLibContext->pArchOptions,pArchOptions,sizeof(arch_drv_option));

    /* ChipIdentify */
    memcpy(pArchSwLibContext->pChipIdentity,pChipIdentity,sizeof(archHAL_CHIPIDENTITY));

    /* Data Feature */
    memcpy(pArchSwLibContext->pArchDataFeature,pArchDataFeature, sizeof(archNN_DATABASE_FEATURE));

    /* init apm handle */
    pArchSwLibContext->apm = archApmInit(pArchNnConfig,pChipIdentity,pArchDataFeature);

    /* temp flag for divide data from log or excel */
    pArchSwLibContext->flag = flag;

    archInfo("Arch Model Sw Lib Init done.\n");
    return (archSwLibHandle)pArchSwLibContext;
}

/* Need to export this function from nnArchPerf for linux compile */

ARCH_MODEL_SW_API arch_status archSetManualFeatures(
    APM_MANUAL_FEATURE_T features
    )
{
    arch_status status = 0;

    APMSetManualFeatures(features);

    return status;
}


arch_status archSwLibDeInit(
    archSwLibHandle pArchSwLibHandle
    )
{
    arch_status status = 0;
    archSwLibContext *pArchSwLibContext = (archSwLibContext *)pArchSwLibHandle;
    if(pArchSwLibContext == NULL)
    {
        /* handle invalid */
        archInfo("Invalid Handle for Arch Sw Lib.\n");
        return archSTATUS_INVALID_ARGUMENT;
    }

    DeInitArchSwLibContext(pArchSwLibContext,pArchSwLibContext->totalCount);

    /* deinit apm handle */
    if(pArchSwLibContext->apm)
        DestroyAPModel(pArchSwLibContext->apm);

    pArchSwLibContext = NULL;
    return status;
}

/***********************************************************************************
* Function:     archPredictPerfFillLayer
* Description:  Arch Model Library Called by python, process one layer everytime
* Input:        graphInfo:
*               pArchSwLibHandle:
*               index:
*               flag:
* Ouput:        arch_status:
***************************************************************************************/
arch_status archPredictPerfFillLayer(
    archSwLibHandle pArchSwLibHandle,
    archModelGraphInfo * layerInfo,
    arch_uint32 index
    )
{
    arch_status status = 0;
    archSwLibContext *pArchSwLibContext = (archSwLibContext *)pArchSwLibHandle;
    if(pArchSwLibContext == NULL)
    {
        /* handle invalid */
        archInfo("Invalid Handle for Arch Sw Lib.\n");
        return archSTATUS_INVALID_ARGUMENT;
    }

    /* Fill one layer */
    memcpy(pArchSwLibContext->graphInfo[index],layerInfo,sizeof(archModelGraphInfo));
    return status;
}


/***********************************************************************************
* Function:     archGetPerfResult
* Description:  Arch Model Library Called by python, will return the perf result by specified index

* Input:        pArchSwLibHandle:
*               index:
*               archPerfResult
* Ouput:        arch_status:
***************************************************************************************/
ARCH_MODEL_SW_API arch_status archGetPerfResult(
    archSwLibHandle pArchSwLibHandle,
    arch_uint32 index,
    arch_perf_result_s *archPerfResult
    )
{
    archSwLibContext *pArchSwLibContext = (archSwLibContext *)pArchSwLibHandle;
    if (pArchSwLibContext == NULL || archPerfResult == NULL)
    {
        /* handle invalid */
        archInfo("Invalid Handle for Arch Sw Lib.\n");
        return archSTATUS_INVALID_ARGUMENT;
    }

    memcpy(archPerfResult, pArchSwLibContext->archPerfResult[index], sizeof(arch_perf_result_s));

    return archSTATUS_OK;
}

/***********************************************************************************
* Function:     archFillPerfResult
* Description:  Fill perf result to the handle after predict

* Input:        pArchSwLibHandle:
*               index:
*               archPerfResult
* Ouput:        arch_status:
***************************************************************************************/
arch_status archFillPerfResult(
    archSwLibHandle pArchSwLibHandle,
    archModelOpInfo **opInfo
    )
{
    arch_uint32 i = 0;
    arch_perf_result_s *pArchPerfResult = NULL;
    arch_perf_s *pArchPerf = NULL;
    arch_float32 mutiGpuFactor = 0;
    arch_uint32 totalCount = 0;

    archSwLibContext *pArchSwLibContext = (archSwLibContext *)pArchSwLibHandle;
    if(pArchSwLibContext == NULL || opInfo == NULL)
    {
        /* handle invalid */
        archInfo("Invalid parameters for function archFillPerfResult.\n");
        return archSTATUS_INVALID_ARGUMENT;
    }
    mutiGpuFactor = (pArchSwLibContext->pArchNnConfig->fixedFeature.vipCoreCount > 1) ? (0.7f * pArchSwLibContext->pArchNnConfig->fixedFeature.vipCoreCount) : 1.0f;
    totalCount = pArchSwLibContext->totalCount;

    arch_float64 NetworkDDR_RdBW_PF_64B = 0, NetworkDDR_RdBW_PF_128B = 0, NetworkDDR_RdBW_PF_256B = 0;
    arch_float64 NetworkDDR_MaskWrBW_PF_64B = 0, NetworkDDR_MaskWrBW_PF_128B = 0, NetworkDDR_MaskWrBW_PF_256B = 0;
    arch_float64 NetworkDDR_NonMaskWrBW_PF_64B = 0, NetworkDDR_NonMaskWrBW_PF_128B = 0, NetworkDDR_NonMaskWrBW_PF_256B = 0;
    for (i = 0; i < totalCount; i++)
    {
        /* fill perf result for every layey */
        pArchPerfResult = pArchSwLibContext->archPerfResult[i];
        pArchPerf = &(opInfo[i]->perf);

        /* fill basic result */
        pArchPerfResult->subImageX = pArchPerf->info.inx;
        pArchPerfResult->subImageY = pArchPerf->info.iny;
        pArchPerfResult->subImageZ = pArchPerf->info.outz;
        pArchPerfResult->tileX = pArchPerf->resultInfo.outImageTileXSize;
        pArchPerfResult->tileY = pArchPerf->resultInfo.outImageTileYSize;
        pArchPerfResult->numOfKernel = pArchPerf->resultInfo.kernelsPerCore;
        pArchPerfResult->cycleCount = (arch_uint64)(pArchPerf->resultInfo.perfCycleCount / mutiGpuFactor + 0.5f);
        pArchPerfResult->ddrRdBw = (arch_uint64)(pArchPerf->resultInfo.perfReadBandWidth + 0.5f);
        pArchPerfResult->ddrWrBw = (arch_uint64)(pArchPerf->resultInfo.perfWriteBandWidth + 0.5f);
        pArchPerfResult->axiSramRdBw = (arch_uint64)(pArchPerf->resultInfo.perfAXIReadBandWidth + 0.5f);
        pArchPerfResult->axiSramWrBw = (arch_uint64)(pArchPerf->resultInfo.perfAXIWriteBandWidth + 0.5f);

        pArchPerfResult->kernelDdrRdBw = (arch_uint64)(pArchPerf->resultInfo.perfKernelReadBandWidth + 0.5f);
        pArchPerfResult->inImageDdrRdBw = (arch_uint64)(pArchPerf->resultInfo.perfInImageReadBandWidth + 0.5f);

        /* NN Transpose */
        pArchPerfResult->nnTransposeChannelIn = pArchPerf->swTilingInfo.trspIvLayerChsIn;
        pArchPerfResult->nnTransposeChannelOut = pArchPerf->swTilingInfo.trspIvLayerChsOut;

        /* addtional parameters */
        pArchPerfResult->p3 = opInfo[i]->p3;
        pArchPerfResult->poolXStride = opInfo[i]->pstride;
        pArchPerfResult->poolYStride = opInfo[i]->pstride;      /* currently the poolXStride and poolYStride are the same */
        pArchPerfResult->outBufNeeded = opInfo[i]->swTilingSegOutBufSizeInPixel;                            /* out buffer needed for every layer */
        pArchPerfResult->graphTotalBufNeeded = pArchPerf->swTilingInfo.segTotalBufferSizeInPixel;           /* segment total buffer needed */
        pArchPerfResult->imageIdealCacheInPixel = pArchPerf->swTilingInfo.imageIdealCacheSizeInPixel;
        pArchPerfResult->archModelKernelSize = pArchPerf->swTilingInfo.archModelKernelSize;

       /* buffer info */
        pArchPerfResult->srcBuf = pArchPerf->swTilingInfo.srcBuf;
        pArchPerfResult->dstBuf = pArchPerf->swTilingInfo.dstBuf;
        pArchPerfResult->kernelBuf = pArchPerf->swTilingInfo.kernelBuf;
        /* Bottleneck */
        pArchPerfResult->firstCmdBottleNeck = pArchPerf->swTilingInfo.firstCmdBottleNeck;
        pArchPerfResult->nonFirstCmdBottleNeck = pArchPerf->swTilingInfo.nonFirstCmdBottleNeck;
        /* Cache info */
        pArchPerfResult->kernelCachePercentage = pArchPerf->swTilingInfo.kernelCachePercentage;
        pArchPerfResult->kernelCacheNeeded = pArchPerf->swTilingInfo.kernelIdalCache;
        /* kernelCacheSize should be the real cached size. If percentage is 1, it is equal to needed */
        pArchPerfResult->kernelCacheSize = (pArchPerfResult->kernelCachePercentage == 1.0f) ?
                        pArchPerfResult->kernelCacheNeeded: pArchPerf->swTilingInfo.kernelSizeInPixel;
        pArchPerfResult->imageCachePercentage = 0;
        pArchPerfResult->imageCacheSize = 0;
        pArchPerfResult->imageCacheNeeded = 0;

        /* fill detail result */
        pArchPerfResult->warType = pArchPerf->archPerfDetailResult.warType;
        pArchPerfResult->computeCC = pArchPerf->archPerfDetailResult.computeCC;
        pArchPerfResult->ddrRdCC = pArchPerf->archPerfDetailResult.ddrRdCC;
        pArchPerfResult->ddrWrCC = pArchPerf->archPerfDetailResult.ddrWrCC;
        pArchPerfResult->axiSramRdCC = pArchPerf->archPerfDetailResult.axiSramRdCC;
        pArchPerfResult->axiSramWrCC = pArchPerf->archPerfDetailResult.axiSramWrCC;
        pArchPerfResult->axiBusRdCC = pArchPerf->archPerfDetailResult.axiBusRdCC;
        pArchPerfResult->axiBusWrCC = pArchPerf->archPerfDetailResult.axiBusWrCC;
        pArchPerfResult->axiBusTotalCC = pArchPerf->archPerfDetailResult.axiBusTotalCC;
        pArchPerfResult->vipSramRdCC = pArchPerf->archPerfDetailResult.vipSramRdCC;
        pArchPerfResult->vipSramWrCC = pArchPerf->archPerfDetailResult.vipSramWrCC;
        pArchPerfResult->slowInternalWrCC = pArchPerf->archPerfDetailResult.slowInternalWrCC;
        pArchPerfResult->slowCompCC = pArchPerf->archPerfDetailResult.slowCompCC;
        pArchPerfResult->internalWrCC = pArchPerf->archPerfDetailResult.internalWrCC;
        pArchPerfResult->dWOutCC = pArchPerf->archPerfDetailResult.dWOutCC;
        pArchPerfResult->kernelDdrRdCC = pArchPerf->archPerfDetailResult.kernelDdrRdCC;
        pArchPerfResult->inImageDdrRdCC = pArchPerf->archPerfDetailResult.inImageDdrRdCC;
        pArchPerfResult->kernelDecodeCC = pArchPerf->archPerfDetailResult.kernelDecodeCC;
        pArchPerfResult->dqArbCC = pArchPerf->archPerfDetailResult.dqArbCC;
        pArchPerfResult->regTile2DxBarCC = pArchPerf->archPerfDetailResult.regTile2DxBarCC;
        pArchPerfResult->bottomTile2DXBarCC = pArchPerf->archPerfDetailResult.bottomTile2DXBarCC;
        pArchPerfResult->xBarCC = pArchPerf->archPerfDetailResult.xBarCC;
        pArchPerfResult->cacheControllerCC = pArchPerf->archPerfDetailResult.cacheControllerCC;
        pArchPerfResult->overHeadsCC = pArchPerf->archPerfDetailResult.overHeadsCC;
        pArchPerfResult->overallCC = pArchPerf->archPerfDetailResult.overAllCC;

        /* region cycles/Bottleneck */
        pArchPerfResult->cyclesTile0Vzgroup0 = pArchPerf->archPerfDetailResult.cyclesTile0Vzgroup0;
        pArchPerfResult->cyclesTile0RestVzgroup0 = pArchPerf->archPerfDetailResult.cyclesTile0RestVzgroup0;
        pArchPerfResult->cyclesRestTileVzgroup0 = pArchPerf->archPerfDetailResult.cyclesRestTileVzgroup0;
        pArchPerfResult->cyclesRestTileRestVzgroup0 = pArchPerf->archPerfDetailResult.cyclesRestTileRestVzgroup0;
        pArchPerfResult->BottleneckTile0Vzgroup0 = pArchPerf->archPerfDetailResult.BottleneckTile0Vzgroup0;
        pArchPerfResult->BottleneckTile0RestVzgroup0 = pArchPerf->archPerfDetailResult.BottleneckTile0RestVzgroup0;
        pArchPerfResult->BottleneckRestTileVzgroup0 = pArchPerf->archPerfDetailResult.BottleneckRestTileVzgroup0;
        pArchPerfResult->BottleneckRestTileRestVzgroup0 = pArchPerf->archPerfDetailResult.BottleneckRestTileRestVzgroup0;

        pArchPerfResult->BN_BottleNeck_e = pArchPerf->archPerfDetailResult.BN_BottleNeck_e;
        pArchPerfResult->BN_Tile0Vzgroup0_e = pArchPerf->archPerfDetailResult.BN_Tile0Vzgroup0_e;
        pArchPerfResult->BN_Tile0RestVzgroup0_e = pArchPerf->archPerfDetailResult.BN_Tile0RestVzgroup0_e;
        pArchPerfResult->BN_RestTileVzgroup0_e = pArchPerf->archPerfDetailResult.BN_RestTileVzgroup0_e;
        pArchPerfResult->BN_RestTileRestVzgroup0_e = pArchPerf->archPerfDetailResult.BN_RestTileRestVzgroup0_e;

        pArchPerfResult->LP_READ = pArchPerf->archPerfDetailResult.DDRRead_Combined_Bursts;
        pArchPerfResult->LP_WRITE = pArchPerf->archPerfDetailResult.DDRWrite_Combined_Bursts;

        //memcpy(&pArchPerfResult->BottleneckTile0Vzgroup0, &pArchPerf->archPerfDetailResult.BottleneckTile0Vzgroup0, sizeof(&pArchPerf->archPerfDetailResult.BottleneckTile0Vzgroup0));

        // refine me, so dirty!
        pArchPerfResult->DDRReadBW_64B_total = pArchPerf->archPerfDetailResult.outputs.DDRReadBW_64B.cost;
        pArchPerfResult->DDRReadBW_128B_total = pArchPerf->archPerfDetailResult.outputs.DDRReadBW_128B.cost;
        pArchPerfResult->DDRReadBW_256B_total = pArchPerf->archPerfDetailResult.outputs.DDRReadBW_256B.cost;

        pArchPerfResult->DDRMaskWriteBW_64B_total = pArchPerf->archPerfDetailResult.outputs.DDRMaskWriteBW_64B.cost;
        pArchPerfResult->DDRMaskWriteBW_128B_total = pArchPerf->archPerfDetailResult.outputs.DDRMaskWriteBW_128B.cost;
        pArchPerfResult->DDRMaskWriteBW_256B_total = pArchPerf->archPerfDetailResult.outputs.DDRMaskWriteBW_256B.cost;

        pArchPerfResult->DDRNonMaskWriteBW_64B_total = pArchPerf->archPerfDetailResult.outputs.DDRNonMaskWriteBW_64B.cost;
        pArchPerfResult->DDRNonMaskWriteBW_128B_total = pArchPerf->archPerfDetailResult.outputs.DDRNonMaskWriteBW_128B.cost;
        pArchPerfResult->DDRNonMaskWriteBW_256B_total = pArchPerf->archPerfDetailResult.outputs.DDRNonMaskWriteBW_256B.cost;

        // update network ddr read write bandwidth per frame it's sum of all layer, can we move it into excel?
        pArchPerfResult->NetworkDDR_RdBW_PF_64B = (NetworkDDR_RdBW_PF_64B += pArchPerfResult->DDRReadBW_64B_total);
        pArchPerfResult->NetworkDDR_RdBW_PF_128B = (NetworkDDR_RdBW_PF_128B += pArchPerfResult->DDRReadBW_128B_total);
        pArchPerfResult->NetworkDDR_RdBW_PF_256B = (NetworkDDR_RdBW_PF_256B += pArchPerfResult->DDRReadBW_256B_total);

        pArchPerfResult->NetworkDDR_MaskWrBW_PF_64B = (NetworkDDR_MaskWrBW_PF_64B += pArchPerfResult->DDRMaskWriteBW_64B_total);
        pArchPerfResult->NetworkDDR_MaskWrBW_PF_128B = (NetworkDDR_MaskWrBW_PF_128B += pArchPerfResult->DDRMaskWriteBW_128B_total);
        pArchPerfResult->NetworkDDR_MaskWrBW_PF_256B = (NetworkDDR_MaskWrBW_PF_256B += pArchPerfResult->DDRMaskWriteBW_256B_total);

        pArchPerfResult->NetworkDDR_NonMaskWrBW_PF_64B = (NetworkDDR_NonMaskWrBW_PF_64B += pArchPerfResult->DDRNonMaskWriteBW_64B_total);
        pArchPerfResult->NetworkDDR_NonMaskWrBW_PF_128B = (NetworkDDR_NonMaskWrBW_PF_128B += pArchPerfResult->DDRNonMaskWriteBW_128B_total);
        pArchPerfResult->NetworkDDR_NonMaskWrBW_PF_256B = (NetworkDDR_NonMaskWrBW_PF_256B += pArchPerfResult->DDRNonMaskWriteBW_256B_total);
    }

    return archSTATUS_OK;
}

/***********************************************************************************
* Function:     archPredictPerfAnalysing
* Description:  Arch Model Library Called by python, will do archPredictPerf
                for detail analyzing
* Input:        graphInfo:
*               pArchNnConfig:
*               pArchOptions
* Ouput:        arch_status:
***************************************************************************************/
arch_status archPredictPerfAnalysing(
    archSwLibHandle pArchSwLibHandle
    )
{
    arch_status status = 0;
    archModelOpInfo **opInfo = NULL;

    archSwLibContext *pArchSwLibContext = (archSwLibContext *)pArchSwLibHandle;
    if(pArchSwLibContext == NULL)
    {
        /* handle invalid */
        archInfo("Invalid Handle for Arch Sw Lib.\n");
        return archSTATUS_INVALID_ARGUMENT;
    }
    opInfo = initArchOpInfo(pArchSwLibContext->totalCount);
    if(opInfo == NULL)
    {
        /* Init Arch op Info failed */
        archError("Init Arch op Info failed.\n");
        goto error;
    }

    /* Fill in opInfo based on graphInfo */
    if (pArchSwLibContext->flag == 0)
    {
        status = archFillOpInfo(opInfo, pArchSwLibContext->graphInfo, pArchSwLibContext->pArchNnConfig, pArchSwLibContext->totalCount);
    }
    else if (pArchSwLibContext->flag == 1)
    {
        status = archFillOpInfoFromExcel(opInfo, pArchSwLibContext->graphInfo, pArchSwLibContext->pArchNnConfig, pArchSwLibContext->totalCount);
    }
    status = archPredictPerf(pArchSwLibContext->apm, opInfo,pArchSwLibContext->totalCount,pArchSwLibContext->pArchNnConfig,pArchSwLibContext->pArchOptions,pArchSwLibContext->pArchDataFeature,pArchSwLibContext->pChipIdentity);

    /* fill result */
    status = archFillPerfResult(pArchSwLibContext,opInfo);
error:
    deInitArchOpInfo(opInfo,pArchSwLibContext->totalCount);
    return status;
}
#endif


#ifdef SPLIT_Z_DIMENSION
/* Merge from CL 213841 */
/* SplitZDimension */
static arch_uint32 labelBuffer(
    arch_nn_config *pArchNnConfig,
    struct _archModelInfo *archModel,
    arch_uint8 sArray[],
    arch_uint32 flushWait[]
    )
{
    struct _archModelOpInfo * opInfo = NULL;
    arch_perf perf = NULL;
    arch_uint8 sramForImage = 0, sramForKernel = 0;
    arch_uint32 i = 0,j = 0, layerCount = archModel->actualCount;
    arch_bool axiSramOnlySWTiling = pArchNnConfig->unifiedFeature.axiSramOnlySWTiling ? arch_true_e : arch_false_e;
    if(axiSramOnlySWTiling)
    {
        sramForImage = SW_TILING_FROM_AXI_SRAM;
    }
    else
    {
        sramForImage = SW_TILING_FROM_VIP_SRAM;
    }

    sramForKernel = SW_TILING_FROM_VIP_SRAM;

    /* label all image_buffer with sram */
    for(i = 0; i < layerCount; i++)
    {
        opInfo = archModel->opInfoArray[i];
        perf = &(opInfo->perf);
        perf->swTilingInfo.srcBuf = sramForImage;
        perf->swTilingInfo.dstBuf = sramForImage;
        perf->swTilingInfo.kernelBuf = SW_TILING_FROM_DDR;
    }

    /* set src_buf = DDR for layers that is_split(layer) = 1, and set dst_buf = DDR for its upstream layers */
    /* set knl_buf = DDR if SWTiling_Type is Not subimage tiling */
    int num_of_layers_in_subimage_segment = 0;
    for(i = 0; i < layerCount; i++)
    {
        opInfo = archModel->opInfoArray[i];
        perf = &(opInfo->perf);
        flushWait[i] = 0;

        if (sArray[i] == 1)
        {
            perf->swTilingInfo.srcBuf = SW_TILING_FROM_DDR;
            for (j = 0; j < opInfo->upStreamLayerCount; j++)
            {
                if (opInfo->upStreamLayer[j] > 0)
                {
                    archModel->opInfoArray[opInfo->upStreamLayer[j]]->perf.swTilingInfo.dstBuf = SW_TILING_FROM_DDR;
                }
            }
            num_of_layers_in_subimage_segment = 0;
        }

        if (i == layerCount)
        {
            perf->swTilingInfo.dstBuf = SW_TILING_FROM_DDR;
            flushWait[i] = 1;
        }
        else
        {
            for (j = 0; j <  opInfo->downStreamLayerCount; j++)
            {
                if (sArray[opInfo->downStreamLayer[j]] == 1)
                {
                    flushWait[i] = 1;
                }
            }

            if (opInfo->downStreamLayerCount == 0)
            {
                perf->swTilingInfo.dstBuf = SW_TILING_FROM_DDR;
            }
        }

        if (opInfo->swTilingType == 0)  /*0: SUB-IMG, 1: AB*/
        {
            num_of_layers_in_subimage_segment = num_of_layers_in_subimage_segment + 1;
            //int downLayer = opInfo->downStreamLayer[0]; // DownstreamLayer(1, i);
            int upLayer = opInfo->upStreamLayer[0];// UpstreamLayer(1, i);

            if (num_of_layers_in_subimage_segment == 1)
            {
                flushWait[i] = 2;
            }
            else if ((num_of_layers_in_subimage_segment == 2) && perf->swTilingInfo.dstBuf == SW_TILING_FROM_DDR )
            {
                flushWait[upLayer] = 1;
                flushWait[i] = 0;
            }
            else if (perf->swTilingInfo.dstBuf == SW_TILING_FROM_DDR)
            {
                flushWait[i] = 2;
            }

            perf->swTilingInfo.kernelBuf = sramForKernel;
        }
    }

    return 0;
}

//static arch_uint32 gSwSplitZDimension = 0;
static arch_uint32 splitZDimension(
    APMHandle apm,
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    struct _archModelInfo *archModel,
    arch_uint32 z_array[],
    arch_uint32 cacheSpace[],
    arch_uint8 sArray[],
    arch_uint32 flushWait[]
    )
{
    arch_uint32 i = 0;

    arch_uint32 imageIdealCache = 0;
    arch_uint32 cacheSpaceForKernel = 0;
    arch_float64 kernelCachePercentage = 0, kernelSizeInPixel = 0, kernelIdalCache = 0;
    arch_uint32 curZ = 0;
    arch_status status = 0;

    struct _archModelOpInfo * opInfo = NULL;
    arch_perf perf = NULL;
    arch_uint32 totalCount = archModel->actualCount;
    if (pArchNnConfig->unifiedFeature.splitZ)
    {
        labelBuffer(pArchNnConfig,archModel,sArray,flushWait);
        for(i = 0; i<totalCount; i++)
        {
            opInfo = archModel->opInfoArray[i];
            perf = &(opInfo->perf);

            if(opInfo->target == ARCHNNE_OPERATION_TARGET_TP)
                continue;

            cacheSpaceForKernel = cacheSpace[i];
            if(perf->swTilingInfo.srcBuf == SW_TILING_FROM_DDR)
            {

                if(apm)
                {
                    imageIdealCache = (arch_uint32)APMCalcImageIdealCacheInPixel(
                        perf->resultInfo.outImageTileXSize,
                        perf->resultInfo.outImageTileYSize,
                        perf->info.kx, perf->info.ky, perf->info.kz,
                        perf->swTilingInfo.origInX,
                        perf->swTilingInfo.origInY,
                        perf->info.xOffSet, perf->info.yOffSet,
                        perf->info.inx, perf->info.inx,
                        perf->info.inputDataSize,
                        pArchNnConfig->unifiedFeature.imageNotPackedInSram,
                        pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte);
                }


/*#ifdef DEPENDS_ON_UPDATE*/
                if(imageIdealCache < cacheSpaceForKernel)
                {
                    cacheSpaceForKernel = cacheSpaceForKernel - imageIdealCache;
                }
/*#endif*/
            }

            /* Check KernelCache percentage */
            if(apm)
            {
                kernelCachePercentage = APMCalcKernelCachePercentage(perf->info.kx, perf->info.ky, perf->info.kz,
                                                            perf->swTilingInfo.origOutZ, perf->info.nnCores,
                                                            perf->coefCompressRatio, cacheSpaceForKernel,
                                                            &kernelSizeInPixel, &kernelIdalCache /*,
                                                            pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix,
                                                            (perf->opType == ARCHNNE_OPERATOR_DEPTH_WISE_CONV)*/);

                /* save kernel cache percentage  */
                perf->swTilingInfo.kernelSizeInPixel = kernelSizeInPixel;
                perf->swTilingInfo.kernelIdalCache = kernelIdalCache;
                perf->swTilingInfo.kernelCachePercentage = kernelCachePercentage;

            }

            /* if it's not kernel full cache, split Z dimension */
            if(kernelCachePercentage < 1 && perf->swTilingInfo.kernelBuf == SW_TILING_FROM_DDR)
            {
                curZ = z_array[i];
                while(curZ > 4)
                {
                    curZ = (arch_uint32)ceilf((arch_float32)curZ / 2);
                    status = archCalculateArchPerf(archModel->apm,
                            pArchNnConfig,
                            pArchOptions,
                            archModel->pArchDataFeature,
                            perf,
                            opInfo->target,
                            (opInfo->op == ARCHNNE_OPERATOR_ROIPOOL
                            && opInfo->tpType == TP_ROI_POOLING_STEP_1) ? ARCHNNE_OPERATOR_POOLING :
                            opInfo->op);
                    if(status != ARCH_SUCCESS)
                    {
                        archInfo("Error: archCalculateArchPerf failed.\n");
                    }
                }
            }
        }
    }
    return 0;
}
#endif


#ifdef ALIGNMENT_64B
#define HW_64B_Alignment    0
#define SW_64B_Alignment    1
/* Merge from 213845 */
static arch_uint32 vipSram64BAlignEnhance(
    arch_nn_config *pArchNnConfig,
    struct _archModelInfo *archModel,
    arch_uint32 x_array[],
    arch_uint32 y_array[],
#ifdef CACHE_SPACE_ENABLE
    arch_uint32 cacheSpace[],
#endif
    arch_uint8 sArray[],
    arch_uint32 flushWait[]
    )
{
    arch_uint32 i = 0, j = 0;    /* Currently only support Non-Branch/Merge networks like mobilenet */
    arch_uint32 totalCount = archModel->actualCount;
    arch_uint32 inx = 0, iny = 0, inSix = 0, inSiy = 0, inImageStride = 0, inImageSlice = 0,alignedInimageSlice = 0;
    arch_uint32 delta = 0;
    arch_bool deltaCanBeFit = arch_false_e;
    struct _archModelOpInfo * opInfo = NULL;
    arch_uint32 equivalentVipSramWidthInByte = pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte;

    labelBuffer(pArchNnConfig, archModel, sArray, flushWait);

    /*  Set a global variable so that all the layer_cost() excecuted after this will consider Alignment enhancement */
    gAlign64BExceted = 1;
    for(i = 0; i< totalCount; i++)
    {
        opInfo = archModel->opInfoArray[i];
        inx = opInfo->perf.swTilingInfo.origInX + opInfo->kx - 1 + 2 * opInfo->perf.info.xOffSet;
        iny = opInfo->perf.swTilingInfo.origInY + opInfo->ky - 1 + 2 * opInfo->perf.info.yOffSet;

        inSix = archMIN(inx, x_array[i] + opInfo->kx - 1);
        inSiy = archMIN(iny, y_array[i] + opInfo->ky - 1);

        inImageStride = inSix;
        inImageSlice = inSix * inSiy;

        if(opInfo->perf.swTilingInfo.srcBuf == SW_TILING_FROM_VIP_SRAM &&
            (opInfo->upStreamLayerCount == 0 || opInfo->perf.allSibling1x1 == 1) &&
            (opInfo->kx == 1) &&
            (opInfo->ky == 1) &&
            opInfo->perf.inputAlignment == 0 &&
            inImageSlice > opInfo->perf.vipSramAccUnitSizeInByte)
        {
            opInfo->perf.vipSramAccUnitSizeInByte = equivalentVipSramWidthInByte * 2;
            alignedInimageSlice = inImageSlice;

            if (pArchNnConfig->unifiedFeature.HW_Alignment)
            {
                alignedInimageSlice = (arch_uint32)ceilf((float)inImageSlice / opInfo->perf.vipSramAccUnitSizeInByte) * opInfo->perf.vipSramAccUnitSizeInByte;
            }
            else if (pArchNnConfig->unifiedFeature.SW_Alignment)
            {
                while((alignedInimageSlice % opInfo->perf.vipSramAccUnitSizeInByte) != 0)
                    alignedInimageSlice = alignedInimageSlice + inImageStride;
            }

            /* If we find aligned_outimage_slice, Check CacheSpace and see if adjusted image can be fit into VIPSRAM or not Check CacheSpace */
            if(alignedInimageSlice > inImageSlice)
            {
                delta = (alignedInimageSlice - inImageSlice) * opInfo->kz; /*Calculate extra space needed*/
                deltaCanBeFit = arch_true_e;
                if(delta > cacheSpace[i])
                    deltaCanBeFit = arch_false_e;

                for(j = 0; j< opInfo->upStreamLayerCount; j++)
                {
                    if(opInfo->upStreamLayer[j] != 0 && delta > cacheSpace[opInfo->upStreamLayer[j]])
                        deltaCanBeFit = arch_false_e;
                }

                /* If Delta can be fit in VIPSRAM, modify cachespace for each related layer and set Input/Output Alignment bit to 1 */
                if(deltaCanBeFit)
                    continue;
#ifdef DEPENDS_ON_213818
                if(deltaCanBeFit)
                {
                    arch_uint32 base_cycle = 0;
                    arch_uint32 base_readbw = 0;
                    arch_uint32 base_writebw = 0;
                    arch_uint32 cur_cycle = 0;
                    arch_uint32 cur_readbw = 0;
                    arch_uint32  cur_writebw = 0;
                    arch_uint32 LayerIB = LayerInputBufferID(layer);

                    for(i = 0; i< LayerIB_input_count(LayerIB); i++)
                    {
                        inLayer = LayerIB_input(LayerIB, i)
                        cur_CacheSpace(inLayer) = cur_CacheSpace(inLayer) - Delta
                        cur_OutputAlignment(inLayer) = 1
                        Alignment = Array(cur_InputAlignment, cur_OutputAlignment)
                        base_cycle = base_cycle + CycleCount(inLayer)
                        base_readbw = base_readbw + DDRReadBandWidth(inLayer)
                        base_writebw = base_writebw + DDRWriteBandWidth(inLayer)
                        cur_cycle = cur_cycle + layer_cost(inLayer, src_buf(inLayer), dst_buf(inLayer), knl_buf(inLayer), cur_CacheSpace, trspIvLayerChsIn, trspIvLayerChsOut, flush_and_wait(inLayer), SIX, SIY, SIZ, cur_InputAlignment, cur_OutputAlignment, cur_DDRRead, cur_DDRWrite, empty_var, empty_var, empty_var, empty_var, empty_var, empty_var, empty_var, empty_var, empty_cyclecounts, empty_outputs)
                        cur_readbw = cur_readbw + cur_DDRRead(inLayer)
                        cur_writebw = cur_writebw + cur_DDRWrite(inLayer)
                    }

                    for(i = 0; i< LayerIB_output_count(LayerIB); i++)
                    {
                        outLayer = LayerIB_output(LayerIB, i)
                        cur_CacheSpace(outLayer) = cur_CacheSpace(outLayer) - Delta
                        cur_InputAlignment(outLayer) = 1
                        Alignment = Array(cur_InputAlignment, cur_OutputAlignment)
                        base_cycle = base_cycle + CycleCount(outLayer)
                        base_readbw = base_readbw + DDRReadBandWidth(outLayer)
                        base_writebw = base_writebw + DDRWriteBandWidth(outLayer)
                        cur_cycle = cur_cycle + layer_cost(outLayer, src_buf(outLayer), dst_buf(outLayer), knl_buf(outLayer), cur_CacheSpace, trspIvLayerChsIn, trspIvLayerChsOut, flush_and_wait(outLayer), SIX, SIY, SIZ, cur_InputAlignment, cur_OutputAlignment, cur_DDRRead, cur_DDRWrite, empty_var, empty_var, empty_var, empty_var, empty_var, empty_var, empty_var, empty_var, empty_cyclecounts, empty_outputs)
                        cur_readbw = cur_readbw + cur_DDRRead(outLayer)
                        cur_writebw = cur_writebw + cur_DDRWrite(outLayer)
                    }

                    base_cost = Array(base_cycle, base_readbw + base_writebw)
                    cur_cost = Array(cur_cycle, cur_readbw + cur_writebw)
                    if(CurCostIsBetter(base_cost, cur_cost))
                    {
                        CacheSpace = cur_CacheSpace
                        InputAlignment = cur_InputAlignment
                        OutputAlignment = cur_OutputAlignment
                    }
                }
#endif
            }
        }
    }
    return 0;
}
#endif

