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


#ifndef _NN_ARCH_PERF_MISC_H_
#define _NN_ARCH_PERF_H_
#pragma once

#define gctUINT32 unsigned int
#define gctINT    int
#define gcvNULL   NULL

#include "gc_feature_database.h"
#endif

#ifndef min
#define min(a, b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)            (((a) > (b)) ? (a) : (b))
#endif

/*enum vxnne_operator_e;*/

// hardware features, fpga info, band width latency info

typedef struct _apm_hw_info_t
{
    gcsFEATURE_DATABASE  *pFeatures; // Features from featureDB, query using chipDef
    FPGA_INFO_T           fpagInfo;  // FPGA info, copy form AM_IN_PARAM_T
    BWL_T                 bwl;

    unsigned int       INT8_MAC_PER_CYCLE_NN;
    // bugs should move to feature DB

} APM_HW_INFO_T;

// for bugs features not in feature DB
typedef struct _apm_hwbug_features
{
    bool IMG_POP_PIPELINE_PAUSE_BUG2029;
    //bool PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG;
    bool LOW_EFFICIENCY_JD_WR_IMGBUF_BUG1992;
    bool COEF_ZERO_POINT_AREA_OPTIMIZATION;
    bool INTERNAL_KERNEL_READ_BOTTLENECK_BUG1998;

    double Internal_Kernel_Read_BYTE_PER_CYCLE;

    bool PREFETCH_NN_COMMAND_KERNEL_HEADER;  // feature small batch phase 1
    bool NO_NARROW_POST_PROCESS_PIPE;
    bool PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007;
    bool Full_KERNEL_CACHE_INTERLEAVE_BUG_2033;
    bool NEIGHBOR_IMG_TRAN_NOT_EFFICIENT_BUG2045;
    bool TP_VIPSRAM_OT1_BUG2050;
    bool NN_LARGE_BURST_SIZE; // feature
    bool Bug_2035; // is it a arch model bug?
    bool new_feature; //Shuangbei: define global variable for the use of c-arch HW model. Its value will be initialized as 0 in initFeatureStatus()

    // features not implimented
    bool Non_Pooling_Pack_1x1;
    bool MxN_interleave_pooling;
    bool VIPSRAM_ASYNC_FIFO;

} APM_BUG_FEATURE_TYPE;

// context
typedef struct _apm_context
{
    unsigned int pooling_stride;
    APM_HW_INFO_T * p_hwInfo;
    APM_BUG_FEATURE_TYPE bf;

    APM_IN_PARAM_T       *pInParams;
    APM_IN_PERF_PARAMS   *pInPerfParams;
    APM_MANUAL_PARAMS_T  *pManualParams;   // self define pamameters
    APM_MANUAL_FEATURE_T *pManualFeatures; // self define features
    bool manualFeatureSetted;
} apm_context_type;

//typedef struct _ByRegion_Type
//{
//    double Total;
//    double vzgroup0;
//    double tile0;
//    double tile0_vzgroup0;
//    double rest_tile_vzgroup0;
//    double tile0_rest_vzgroup;
//    double rest_tile_rest_vzgroup;
//}ByRegion_Type;

//typedef APM_BW_CYCLE_COST_T ByRegion_Type;

apm_context_type context = {0};
apm_context_type *pContext = NULL;

#define SW_TILING_FROM_DDR            0
#define SW_TILING_FROM_AXI_SRAM       1
#define SW_TILING_FROM_VIP_SRAM       2
#define SW_TILING_PERM_AXI_SRAM       3
#define SW_TILING_PERM_VIP_SRAM       4
#define AXI_BURST_SIZE               64
//#define NN_DDR_BURST_SIZE            64

// USC_CACHE_SIZE is defines from driver
// May be changed, or pass from driver is better
// following define may be not correct
// todo

#define NNE_COMMAND_SIZE                    128
#define TP_COMMAND_SIZE                     128

// USC_CACHE_SIZE is defines from driver
#define USC_CACHE_SIZE                         8
#define CACHED_DATA_READ_FROM_SRAM             1
#define DDR_READ_BANDWIDTH_LIMIT               3.8f
#define DDR_WRITE_BANDWIDTH_LIMIT              3.8f
#define DDR_TOTAL_BANDWIDTH_LIMIT              3.8f
#define AXI_SRAM_READ_BANDWIDTH_LIMIT          16.0f
#define AXI_SRAM_WRITE_BANDWIDTH_LIMIT         16.0f
#define AXI_SRAM_TOTAL_BANDWIDTH_LIMIT         16.0f
#define AXI_BUS_READ_BANDWIDTH_LIMIT           16.0f
#define AXI_BUS_WRITE_BANDWIDTH_LIMIT          16.0f
#define AXI_BUS_TOTAL_BANDWIDTH_LIMIT          32.0f
// fix me what's the default of DDR_LATENCY
#define DDR_LATENCY                            0
#define ACCURATE_TILE_BW                       1
#define AXI_SRAM_SLOWED_DOWN_BY_DDR            1
#define FREQ_IN_MHZ                            1000
#define AXI_CLK_FREQ_IN_MHZ                    1000
//#define LANES_PER_CONV                         64
#define MAX_TILE_XSIZE                         64
#define MAX_SOC_OUT_STANDING_NUMBER            32
#define LANES_PER_OUT_CYCLE                    16
#define ZDP_LOOP_COUNT                         3

// nne operator
typedef enum vxnne_operator_e
{
    VXNNE_OPERATOR_NONE = 0,
    VXNNE_OPERATOR_CONVOLUTION,
    VXNNE_OPERATOR_RESHUFFLE,
    VXNNE_OPERATOR_FULLYCONNECTED,
    VXNNE_OPERATOR_ACTIVATION,
    VXNNE_OPERATOR_POOLING,
    VXNNE_OPERATOR_RESIZE,
    VXNNE_OPERATOR_TENSOR_ADD,
    VXNNE_OPERATOR_TENSOR_SUB,
    VXNNE_OPERATOR_TENSOR_MUL,
    VXNNE_OPERATOR_TENSOR_DIV,
    VXNNE_OPERATOR_TENSOR_TRANS,
    VXNNE_OPERATOR_SOFTMAX,
    VXNNE_OPERATOR_NORMALIZATION,
    VXNNE_OPERATOR_BATCHNORM,
    VXNNE_OPERATOR_INPUT2WEIGHT,
    VXNNE_OPERATOR_RPN_SOFTMAX,
    VXNNE_OPERATOR_RPN_REGRESSION,
    VXNNE_OPERATOR_RPN_SORT,
    VXNNE_OPERATOR_RPN_NMS,
    VXNNE_OPERATOR_RPN_SORT_NMS,
    VXNNE_OPERATOR_RPN_RETRIEVE,
    VXNNE_OPERATOR_RPN,
    VXNNE_OPERATOR_ROIPOOL,
    VXNNE_OPERATOR_ROIPOOLRELU,
    VXNNE_OPERATOR_CONCAT2,
    VXNNE_OPERATOR_CONCATINDEFINITE,
    VXNNE_OPERATOR_REORG,
    VXNNE_OPERATOR_VERTMAXPOOL,
    VXNNE_OPERATOR_HORZMAXPOOL,
    VXNNE_OPERATOR_PRETREATEDRECT,
    VXNNE_OPERATOR_BRICK,
    VXNNE_OPERATOR_DECONVOLUTION,
    VXNNE_OPERATOR_L2NORMALIZE,
    VXNNE_OPERATOR_L2NORMALIZE_SUMSQRT,
    VXNNE_OPERATOR_L2NORMALIZE_SUMSCALE,
    VXNNE_OPERATOR_TENSOR_COPY,
    VXNNE_OPERATOR_CONVERT_FORMAT,
    VXNNE_OPERATOR_TENSOR_REDUCE_SUM,
    VXNNE_OPERATOR_TENSOR_PAD,
    VXNNE_OPERATOR_LSTM_UNIT,
    VXNNE_OPERATOR_LSTM_LAYER,
    VXNNE_OPERATOR_REORG2,
    VXNNE_OPERATOR_TENSOR_ROUNDING,
    VXNNE_OPERATOR_HASHLUT,
    VXNNE_OPERATOR_LSH_PROJECTION,
    VXNNE_OPERATOR_TENSOR_RESHAPE,
    VXNNE_OPERATOR_TENSOR_SCALE,
    VXNNE_OPERATOR_YUV2RGB_SCALE,
    VXNNE_OPERATOR_RNN,
    VXNNE_OPERATOR_SVDF,
    VXNNE_OPERATOR_LUT2,
    VXNNE_OPERATOR_UPSAMPLE,
    VXNNE_OPERATOR_DILATION_RESHUFFLE,
    VXNNE_OPERATOR_DILATION_UPSAMPLE,
    VXNNE_OPERATOR_DILATION_UPSAMPLE2,
    VXNNE_OPERATOR_ADAPTER,
    VXNNE_OPERATOR_INTERLEAVE,
    VXNNE_OPERATOR_DEPTHWISE_CONV,
    VXNNE_OPERATOR_LSTM_RESHUFFLE_INPUT,
    VXNNE_OPERATOR_LSTM_STATE_OUT,
    VXNNE_OPERATOR_TENSOR_REVERSE,
    VXNNE_OPERATOR_USER_VXC,
    VXNNE_OPERATOR_USER_CPU,
    VXNNE_OPERATOR_TENSOR_MEAN,
    VXNNE_OPERATOR_TENSOR_SQUEEZE,
    VXNNE_OPERATOR_TENSOR_STRIDE_SLICE,
    VXNNE_OPERATOR_PRELU,
    VXNNE_OPERATOR_GRU,
    VXNNE_OPERATOR_GRU_LAYER,
    VXNNE_OPERATOR_CONV_LSTM,
    VXNNE_OPERATOR_CONV_LSTM_LAYER,
    VXNNE_OPERATOR_DEPTH_WISE_CONV,
    VXNNE_OPERATOR_SVDF_MAP,
    VXNNE_OPERATOR_SVDF_ROTATION,
    VXNNE_OPERATOR_TENSOR_MAX,
}
vxnne_operator_e;

void initBugStatus(
    APM_BUG_FEATURE_TYPE * bf
    );

void initFeatureStatus(
    APM_BUG_FEATURE_TYPE *pBf
    );

void sanityCheck();

double ComputeKernelIdealCache(
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int z,
    double coef_compress_ratio
    );

double ComputeKernelNonIdealCache(
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int z,
    double coef_compress_ratio,
    unsigned int cores
    );

double ComputeKernelStorage(
    double IdealCache,
    double NonIdealCache,
    int    Bug2007,
    int    Bug2033,
    int    cache_space
    );

void UpdateManualParams(
    unsigned int &tile_xsize,
    unsigned int &tile_ysize,
    unsigned int &k,
    unsigned int &inx,
    unsigned int &iny
    );

void InitHWModeling(
    BWL_T &bwl
    );

