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
* FileName:        archSwPerf.h
* Author:        JinboHuang
* Data:            2019-05-27
* Version:        0.5.00
* Description:    Head file of Arch Model SW Library for caller
*
***************************************************************************************/
#ifndef _ARCH_SW_PERF_H_
#define _ARCH_SW_PERF_H_

#include "archSwType.h"

/* #include "archSwCommon.h" */
#include "nnArchPerf.h"

/* Arch Sw Lib handle definition */
typedef void * archSwLibHandle;

/*************************************** MACRO definition ************************************/
#define MAX_PARENT_CHILD_OP_NUM             400
#define VIV_MAX_NN_CORE_COUNT               128

// fix me because NN transpose
#define ARCH_VIP_SRAM_SIZE \
    ((pArchNnConfig->customizedFeature.vipSRAMSize <= (arch_uint32)ARCH_VIP_SRAM_IMAGE_STREAM_SIZE) ? pArchNnConfig->customizedFeature.vipSRAMSize : \
    (pArchNnConfig->customizedFeature.vipSRAMSize - ARCH_VIP_SRAM_IMAGE_STREAM_SIZE))

#define ARCH_AXI_SRAM_SIZE \
    (pArchNnConfig->customizedFeature.axiSRAMSize)
/*************************************** Struction definition ************************************/
/*#pragma pack(push, 8)*/


/******* Result return to Caller struction definition ***************************/

// remove me, this is same as APM_OUT_RESULT_T
typedef struct _arch_perf_result_s
{
    /* layer basic info */

    /* result */
    arch_uint32 subImageX;
    arch_uint32 subImageY;
    arch_uint32 subImageZ;
    arch_uint32 tileX;
    arch_uint32 tileY;
    arch_uint32 numOfKernel;                /* Kernels Per Core */
    arch_uint64 cycleCount;
    arch_uint64 ddrRdBw;
    arch_uint64 ddrWrBw;
    arch_uint64 axiSramRdBw;
    arch_uint64 axiSramWrBw;
    arch_uint64 kernelDdrRdBw;
    arch_uint64 inImageDdrRdBw;

    /* NN_Transpose */
    arch_uint32 nnTransposeChannelIn;
    arch_uint32 nnTransposeChannelOut;

    /* addtional parameters */
    arch_uint32 p3;
    arch_uint32 poolXStride;
    arch_uint32 poolYStride;
    arch_uint32 outBufNeeded;
    arch_uint32 graphTotalBufNeeded;
    arch_uint32 imageIdealCacheInPixel;
    arch_uint32 archModelKernelSize;

    /* Buffer info */
    arch_uint32 srcBuf;
    arch_uint32 dstBuf;
    arch_uint32 kernelBuf;

    /* Bottleneck */
    arch_uint32 firstCmdBottleNeck;
    arch_uint32 nonFirstCmdBottleNeck;

    /* Cache info */
    arch_float64 kernelCachePercentage;
    arch_float64 kernelCacheSize;
    arch_float64 kernelCacheNeeded;
    arch_float64 imageCachePercentage;
    arch_float64 imageCacheSize;
    arch_float64 imageCacheNeeded;

    /* WAR Type */
    arch_uint32 warType;

    /* All kind of cyclecount */
    arch_float64 computeCC;
    arch_float64 ddrRdCC;
    arch_float64 ddrWrCC;
    arch_float64 axiSramRdCC;
    arch_float64 axiSramWrCC;
    arch_float64 axiBusRdCC;
    arch_float64 axiBusWrCC;
    arch_float64 axiBusTotalCC;
    arch_float64 vipSramRdCC;
    arch_float64 vipSramWrCC;
    arch_float64 slowInternalWrCC;
    arch_float64 slowCompCC;
    arch_float64 internalWrCC;
    arch_float64 dWOutCC;
    arch_float64 kernelDdrRdCC;
    arch_float64 inImageDdrRdCC;
    arch_float64 kernelDecodeCC;
    arch_float64 dqArbCC;
    arch_float64 regTile2DxBarCC;
    arch_float64 bottomTile2DXBarCC;
    arch_float64 xBarCC;
    arch_float64 cacheControllerCC;
    arch_float64 overHeadsCC;
    arch_float64 overallCC;

    /* region cycles/Bottleneck */
    arch_float64 cyclesTile0Vzgroup0;
    arch_float64 cyclesTile0RestVzgroup0;
    arch_float64 cyclesRestTileVzgroup0;
    arch_float64 cyclesRestTileRestVzgroup0;

    arch_float64 BottleneckTile0Vzgroup0;
    arch_float64 BottleneckTile0RestVzgroup0;
    arch_float64 BottleneckRestTileVzgroup0;
    arch_float64 BottleneckRestTileRestVzgroup0;

    APM_BOTTLENECK_T BN_BottleNeck_e;
    APM_BOTTLENECK_T BN_Tile0Vzgroup0_e;
    APM_BOTTLENECK_T BN_Tile0RestVzgroup0_e;
    APM_BOTTLENECK_T BN_RestTileVzgroup0_e;
    APM_BOTTLENECK_T BN_RestTileRestVzgroup0_e;

    arch_float64 DDRReadBW_64B_total;
    arch_float64 DDRReadBW_128B_total;
    arch_float64 DDRReadBW_256B_total;
    arch_float64 DDRNonMaskWriteBW_64B_total;
    arch_float64 DDRNonMaskWriteBW_128B_total;
    arch_float64 DDRNonMaskWriteBW_256B_total;
    arch_float64 DDRMaskWriteBW_64B_total;
    arch_float64 DDRMaskWriteBW_128B_total;
    arch_float64 DDRMaskWriteBW_256B_total;

    //// PF means Per Frame
    arch_float64 NetworkDDR_RdBW_PF_64B;
    arch_float64 NetworkDDR_RdBW_PF_128B;
    arch_float64 NetworkDDR_RdBW_PF_256B;
    arch_float64 NetworkDDR_MaskWrBW_PF_64B;
    arch_float64 NetworkDDR_MaskWrBW_PF_128B;
    arch_float64 NetworkDDR_MaskWrBW_PF_256B;
    arch_float64 NetworkDDR_NonMaskWrBW_PF_64B;
    arch_float64 NetworkDDR_NonMaskWrBW_PF_128B;
    arch_float64 NetworkDDR_NonMaskWrBW_PF_256B;

    arch_float64 LP_READ; //"DDRRead_Combined_Bursts"
    arch_float64 LP_WRITE; //"DDRWrite_Combined_Bursts"
}
arch_perf_result_s;

/*********** Done: Result return to Caller struction definition********************/

/******* Perf struction definition in ArchModel ***************************/
/* General Information for one layer */
typedef struct _arch_general_info_s
{
    arch_uint32     kx;
    arch_uint32     ky;
    arch_uint32     kz;
    arch_uint32     oinx;
    arch_uint32     oiny;
    arch_uint32     oinz;
    arch_uint32     inx;
    arch_uint32     iny;
    arch_uint32     inz;
    arch_uint32     outx;
    arch_uint32     outy;
    arch_uint32     outz;
    arch_uint32     stridex;
    arch_uint32     stridey;
    /*arch_uint32    dataSize;*/
    arch_uint32     inputDataSize;
    arch_uint32     outputDataSize;
    arch_uint32     poolingSize;
    arch_uint32     poolingStride;

    arch_int32      xOffSet; // left
    arch_int32      yOffSet; // top
    arch_int32      xOffset_right;
    arch_int32      yOffset_bottom;

    arch_int32      inputDataFormat;
    arch_int32      outputDataFormat;
    arch_int32      nnCores;
    arch_int32      convOutFifoDepth;
    arch_uint32     kernelSize;
    arch_uint32     pix;
    arch_uint32     piy;
    arch_uint32     p3;
    arch_uint32     nextKY;/*the Y size of output subimage in AXI SRAM need it to add the subimage Y overlap*/
    arch_int32      flush;

    /* addtional info */
    arch_uint32     inSIXReFined;
    arch_uint32     inSIYReFined;
    arch_uint32     inImageStride;
    arch_uint32     inImageSlice;
    arch_uint32     outImageStride;
    arch_uint32     outImageSlice;
    arch_uint32     interleaveMode;
}
arch_general_info_s;

/* SW tilling information for one layer */
typedef struct _arch_swtiling_info_s
{
    arch_uint8      srcBuf;
    arch_uint8      dstBuf;
    arch_uint8      kernelBuf;

    arch_int32      cacheSpace;
    arch_uint32     trspIvLayerChsIn;
    arch_uint32     trspIvLayerChsOut;
    arch_bool       calcNonFirstCmd;
    arch_bool       isNonFirstComputeBottleNeck;

    arch_float64    perfNonFirstCycleCount;
    arch_float64    perfNonFirstKernelReadBandWidth;
    arch_float64    perfNonFirstInImageReadBandWidth;
    arch_float64    perfNonFirstReadBandWidth;
    arch_float64    perfNonFirstWriteBandWidth;
    arch_float64    perfNonFirstAXIReadBandWidth;
    arch_float64    perfNonFirstAXIWriteBandWidth;

    APM_BOTTLENECK_T firstCmdBottleNeck;
    APM_BOTTLENECK_T nonFirstCmdBottleNeck;

    arch_uint32     origInX;
    arch_uint32     origInY;
    arch_uint32     origOutX;
    arch_uint32     origOutY;
    arch_uint32     origOutZ;

    arch_int32      kernelCacheMode;
    arch_int32      imageCacheMode;
    arch_float64    kernelCachePercentage;
    arch_float64    kernelSizeInPixel;
    arch_float64    kernelIdalCache;

    arch_int32      imageIdealCacheSizeInPixel;
    arch_uint32     archModelKernelSize;
    arch_uint32     outImageStride;
    arch_uint32     outImageSlice;

    arch_uint32     swTilingSegKernelBufSizeInPixel;
    arch_uint32     segTotalBufferSizeInPixel;
}
arch_swtiling_info_s;

/*************************************** Performance Information definition ************************************/
typedef struct _arch_performance_info_s
{
#define HALF_DONE 0x5
    arch_uint8      calculated;
    arch_uint32     kernelsPerCore;
    arch_uint32     outImageTileXSize;
    arch_uint32     outImageTileYSize;
    arch_uint32     interleaveMode;
    arch_uint32     nnCoreCount; /* refine me, this can be removed*/
    arch_float64    perfCycleCount;
    arch_float64    perfReadBandWidth;
    arch_float64    perfWriteBandWidth;
    arch_float64    perfAXIReadBandWidth;
    arch_float64    perfAXIWriteBandWidth;
    arch_float64    perfKernelReadBandWidth;
    arch_float64    perfInImageReadBandWidth;
    arch_bool       isFirstComputeBottleNeck;
}
arch_performance_info_s, *arch_performance_info;


/* Compress Ratio Information */
typedef struct _archCompressRatio
{
    arch_float64    efNonZeroRatio;
    arch_float64    coefCompressRatio;
    arch_float64    imageCompressRatio;
    arch_float64    imageNonZeroRatio;
    arch_float64    maxPerCoreCompressionRatio;
    arch_float64    maxPerCorePerVZGroupNonZeroRatios[VIV_MAX_NN_CORE_COUNT];
}archCompressRatio;

/* Arch Model Performance Information */
typedef struct _arch_perf_s
{
    arch_general_info_s  info;
    arch_swtiling_info_s swTilingInfo;
    arch_float64         coefNonZeroRatio;
    arch_float64         coefCompressRatio;
    arch_float64         imageCompressRatio;
    arch_float64         imageNonZeroRatio;
    arch_float64         maxPerCoreCompressionRatio;
    arch_float64         maxPerCorePerVZGroupNonZeroRatios[VIV_MAX_NN_CORE_COUNT];

    arch_performance_info_s    resultInfo;
    archnne_operator_e         opType;
    archnne_operation_target_e opTarget;

    arch_bool                                  calculated;

    /* Merge from cl 213817 */
    arch_uint32 allSibling1x1;
    arch_uint32 SiblingHas1x1;
    arch_uint32 upStreamLayerCount;        /* fro temp user, need to optimize */
    arch_uint32 downStreamLayerCount;
    arch_uint32 firstChildAllSibling1x1;

    arch_uint32 vipSramAccUnitSizeInByte;

    /* Merge from 213845 */
    arch_uint32 inputAlignment;
    arch_uint32 outputAlignment;

    /* for NN transpose */
    arch_uint32 trspIvLayerChsIn;

    /* Add perf detail result */
    APM_OUT_RESULT_T archPerfDetailResult;
}
arch_perf_s;
/******* DONE:Perf struction definition in ArchModel ***************************/

/*****************************************configration struction definition *************************/

/******* Configration struction definition ***************************/
/* Immuatable features from database */
typedef struct _archNN_FIXED_FEATURE
{
    arch_uint32  vipCoreCount;
    arch_uint32  nnCoreCount;           /* total nn core count */
    arch_uint32  nnCoreCountInt8;       /* total nn core count supporting int8 */
    arch_uint32  nnCoreCountInt16;      /* total nn core count supporting int16 */
    arch_uint32  nnCoreCountFloat16;    /* total nn core count supporting float16 */
    arch_uint32  nnCoreCountBFloat16;    /* total nn core count supporting Bfloat16 */
    arch_uint32  nnMadPerCore;
    arch_uint32  nnInputBufferDepth;
    arch_uint32  nnAccumBufferDepth;
    arch_uint32  nnFCNonPrunAccel;      /* NO */
    arch_uint32  nnInImageOffsetBits;   /* can be deleted */
    arch_uint32  tpCoreCount; /* full-function core count */
    arch_uint32  tpPwlLUTCount;         /* can be deleted */
    arch_uint32  tpPwlLUTSize;          /* NO */
    arch_uint32  vip7Version;
    arch_uint32  vipBrickMode;
    arch_uint32  tpReorderInImageSize;  /* can be deteled */
    arch_uint32  tpliteCoreCount; /* fc-only core count */
    arch_uint32  nnFP16XYDPX;
    arch_uint32  nnFP16XYDPY;
    arch_uint32  nnFP16ZDP;
    arch_uint32  zrlBits;
    arch_uint32  uscCacheControllers;
    arch_uint32  uscBanks;              /* can be deleted */
    arch_uint32  nnLanesPerOutCycle;
    arch_uint32  maxOTNumber;
    arch_uint32  equivalentVipsramWidthInByte;
    arch_uint32  shaderCoreCount;

    /*multi Core count*/
    arch_uint32  multiVIPnum;

    /* gcFEATURE_BIT_PREPROCESS_IMG_BUF_640BYTE_LIMIT */
    arch_uint32 preprocessImgBuf640BLimit;
} archNN_FIXED_FEATURE;

/* Features can be customized from outside */
typedef struct _archNN_CUSTOMIZED_FEATURE
{
    arch_uint32  vipSRAMSize;
    arch_uint32  axiSRAMSize;
    arch_float32 ddrReadBWLimit;
    arch_float32 ddrWriteBWLimit;
    arch_float32 ddrTotalBWLimit;
    arch_float32 axiSramReadBWLimit;
    arch_float32 axiSramWriteBWLimit;
    arch_float32 axiSramTotalBWLimit;
    arch_float32 axiBusReadBWLimit;
    arch_float32 axiBusWriteBWLimit;
    arch_float32 axiBusTotalBWLimit;
    arch_uint32  vipSWTiling;
    arch_float32 ddrLatency;
    arch_uint32  freqInMHZ;
    arch_uint32  axiClockFreqInMHZ;
    arch_uint32  maxSocOTNumber;/*max SOC outstanding transfer number*/
    arch_uint32  nnWriteWithoutUSC;
    arch_uint32  depthWiseSupport;
    arch_uint32  vipVectorPrune;
    arch_uint32  ddrKernelBurstSize;
} archNN_CUSTOMIZED_FEATURE;

/* Features are unified (hardcoded) for hardwares */
typedef struct _archNN_UNIFIED_FEATURE
{
    arch_uint32  nnUSCCacheSize;
    arch_uint32  nnCmdSizeInBytes;
    arch_uint32  tpCmdSizeInBytes;
    arch_uint32  vipCoefDecodePerf;
    arch_uint32  vipCachedReadFromSram;
    arch_uint32  vipImagePartialCache;
    arch_uint32  lanesPerConv;
    arch_uint32  maxTileSize;
    /* change bit field back to arch_uint32 for python call */
    arch_uint32  fullCacheKernelHeadFix;
    arch_uint32  conv1x1HalfPerformance;
    arch_uint32  per3DTileBubbleFix;
    arch_uint32  cacheLineModeDisabled;
    arch_uint32  tpReOrderFix;
    arch_uint32  zdp3NoCompressFix;
    arch_uint32  asyncCopyPerfFix;
    arch_uint32  accurateTileBW;
    arch_uint32  zxdp3KernelReadConflictFix;
    arch_uint32  axiSramSlowedDownByAddr;
    arch_uint32  slowNNReqArbitrationFix;
    arch_uint32  singlePortAccBuffer;
    arch_uint32  convOutFifoDepthFix;
    arch_uint32  smallBatchEnable;
    arch_uint32  axiSramOnlySWTiling;
    arch_uint32  imageNotPackedInSram;
    arch_uint32  coefDeltaCordOverFlowZRL8BitFix;
    arch_uint32  xyOffsetLimitationFix;
    arch_uint32  kernelPerCoreLTOneThirdCoefFix;
    arch_uint32  lowEfficiencyOfIDWriteImgBufFix;

    arch_uint32  splitZ;
    //arch_uint32  DDR_Alignment;
    arch_uint32  SW_Alignment; // sw 64 byte alignment
    arch_uint32  HW_Alignment; // hw 64 byte alignment
    arch_uint32  BUG_2112; //cacheLineModeNeed64BAlignFix;//war of bug 2112
    arch_uint32  singlePortVipSram;    /* use single port vipsram */
    arch_uint32  tileAccessCapbility;
    arch_uint32  fastDp3Preprocessor;
    arch_uint32  tensorAddMerge;        /* match to tensor_add in feature DB */
    arch_uint32  fastFirstPixelPooling; /* match to NN_NATIVE_STRIDE_TWO */
} archNN_UNIFIED_FEATURE;

/* Features are derived from above ones */
typedef struct _archNN_DERIVIED_FEATURE
{
    arch_uint32  nnDPAmount;
    arch_uint32  nnXYDPX;
    arch_uint32  nnXYDPY;
    arch_uint32  nnZDP;
    arch_float32 totalLatency;
    arch_float32 internalLatency;
    arch_float32 ddrReadBWInBytePerCycle;
    arch_float32 ddrWriteBWInBytePerCycle;
} archNN_DERIVED_FEATURE;

typedef struct _archNN_DATABASE_FEATURE
{
    /* extend for feature */
    arch_uint32 swtilingPhase1Enable;
    arch_uint32 swtilingPhase2Enable;
    arch_uint32 swtilingPhase3Enable;
    arch_uint32 zdp3Enable;
    arch_uint32 zdp6Enable;
    arch_uint32 xydp9Enable;
    arch_uint32 coefComEnhancement;
    arch_uint32 tpComEnhancement;
    arch_uint32 vipDec400Enable;

    /* extend for option */
    arch_uint32 nnStrideEnable;
    arch_uint32 nnZdp3Enable;
    arch_uint32 nnZdp6Enable;

    /* add for depthwiseMerge, set in updateConfigration() or from python */
    arch_uint32  depthWiseMergeSupport;
    arch_uint32  nnSlowOutput;                              /* HW feature */
    arch_uint32  noNarrowPostProcessPipe;                   /* HW feature */
    arch_uint32  prefetchNNCommandKernelHeader;             /* HW feature -- NN small batch phase1 */

    /* fix may not needed here */
    arch_uint32  partialKernelCacheInternalFix;             /* bug 2007 */
    arch_uint32  internalKernelReadBottleneckFix;           /* bug 1998 */
    arch_uint32  ImgPopPipelinePauseFix;                    /* bug 2007 */
    arch_uint32  fullCacheIntervalFix;                      /* bug 2033 */
    arch_uint32  drJdDiffConditionForCacheLineModePreFix;   /* bug2046 */
    arch_uint32  readReturnArbiterBubbleFix;                /* 2038 */
    arch_uint32  nerghborImageDataTransferNotEfficientFix;  /* 2045 */
    arch_uint32  tpVipSramOt1Fix;                           /* 2050 */
    arch_uint32  v8AccumulationBufRwConfictZeroSkipPerfFix; /* 2044 */
    arch_uint32  burstCollectDummyDataWasteCyclesFix;       /* 2111 */
    arch_uint32  tpAccessVipSramOtIsOneFix;                 /* 2050 */
    arch_uint32  nnInTileDataIsAllPadFix;                   /* bug 2131, NN_IN_TILE_DATA_IS_ALL_PAD_FIX */
    /* New for log output */
    arch_uint32 ddrAlign;
    arch_uint32 inlinesPerCycle;

    /* new for nntranspose */
    arch_uint32 nnTranspose;
    arch_uint32 specificDDRLimitByBurst;

    /* DDR sustained BW burst */
    arch_float32 ddrReadSustainedBw64BBurst;
    arch_float32 ddrReadSustainedBw128BBurst;
    arch_float32 ddrReadSustainedBw256BBurst;
    arch_float32 ddrMaskWriteSustainedBw64BBurst;
    arch_float32 ddrMaskWriteSustainedBw128BBurst;
    arch_float32 ddrMaskWriteSustainedBw256BBurst;
    arch_float32 ddrNonMaskWriteSustainedBw64BBurst;
    arch_float32 ddrNonMaskWriteSustainedBw128BBurst;
    arch_float32 ddrNonMaskWriteSustainedBw256BBurst;

    /* VIPSRAM ASYNC FIFO */
    arch_uint32 vipSramAsyncFifo;

    /* Burst size */
    arch_uint32 nnDDRBurstSize;
    arch_uint32 nnLargeBurstSize;

    /* tp comp 2pixel per cycle */
    arch_uint32 tpComp2pixelPerCycle;

    /* graph batch count */
    arch_uint32 grachBatchCount;

}archNN_DATABASE_FEATURE;

typedef struct _arch_nn_config
{
    arch_bool  isSet;           /* can be deleted */
    archNN_FIXED_FEATURE      fixedFeature;
    archNN_CUSTOMIZED_FEATURE customizedFeature;
    archNN_UNIFIED_FEATURE    unifiedFeature;
    archNN_DERIVED_FEATURE    derivedFeature;
}
arch_nn_config;

typedef struct _arch_drv_option
{
    arch_uint32    enableTP;               /* NO */
    arch_uint32    enableMultiTP;          /* NO */
    arch_uint8    *flagTPFunc;            /* NO */
    arch_uint32   *typeTPFunc;          /* NO */
    arch_uint32    enableSRAM;             /* NO */
    arch_uint32    enableSramStreamMode;   /* NO */
    arch_uint32    enableCNNPerf;
    arch_uint32    enableBrickMode;
    arch_uint32    enableNonZeroBalance;   /* NO */
    arch_uint32    enableBorderMode;       /* NO */
    arch_uint32    enableTPReorder;        /* NO */
    arch_uint32    enableTPInterleave8;    /* NO */
    arch_uint32    enableTPRTNE;           /* NO */
    arch_uint32    enableShader;           /* NO */
    arch_uint32    enableNNXYDP9;
    arch_uint32    enableNNXYDP6;
    arch_uint32    enableSwtilingPhase1;
    arch_uint32    enableSwtilingPhase2;
    arch_uint32    enableSwtilingPhase3;
    arch_uint32    enableHandleBranch; /*merge more branches to use AB Buffer for SWTiling for arch model*/
    arch_uint32    enableNNFirstPixelPooling;      /* NO */
    arch_uint32    enableNNDepthWiseSupport;       /* NO or YES?? */
    arch_uint32    enablePrintOperaTarget;         /* NO */
    arch_uint32    enableSaveBinary;               /* NO */
    arch_uint32    enableGraphCommandBuffer;       /* NO */
    arch_uint32    nnFormulaOpt;                   /* NO */
    arch_float32   ddrLatency;
    arch_float32   ddrReadBWLimit;
    arch_float32   ddrWriteBWLimit;
    arch_float32   ddrTotalBWLimit;
    arch_float32   axiSramReadBWLimit;
    arch_float32   axiSramWriteBWLimit;
    arch_float32   axiSramTotalBWLimit;
    arch_float32   axiBusReadBWLimit;
    arch_float32   axiBusWriteBWLimit;
    arch_float32   axiBusTotalBWLimit;
    arch_uint32    vipSRAMSize;
    arch_uint32    axiSRAMSize;
    char          *graphPerfLogFile;                     /* NO */
    arch_uint32    nnZeroRunLen;                   /* NO */
    arch_int32     tpZeroRunLen;                   /* NO */
    arch_uint32    enableNNArchPerfPrint;
    arch_uint32    enableNNLayerDump;              /* NO */
    arch_uint32    enableNNLayerDump_Int;          /* NO */
    arch_uint32    enableInterleave8;              /* NO */
    char          *nnRoundingMode;                       /* NO */
    char          *vxcShaderSourcePath;                  /* NO */
    arch_uint32    fcZMax;                         /* NO */
    arch_uint32    enableMemPool;                  /* NO */
    arch_uint32    memPoolSize;                    /* NO */
#define COLLECT_PERF_RUN       0                /* NO */
#define COLLECT_PERF_ESTIMATE  1
    arch_uint32    collectPerfType;
    arch_uint32    enableGraphAdapter;             /* NO */
    arch_uint32    enableZdpOpt;                   /* NO */
    arch_uint32    do1xnAfterSwtiling;             /* NO */
    arch_uint32    nn1x1To1xN;                     /* NO */
    arch_uint32    enableGraphTranform;            /* NO */
    arch_uint32    enableGraphWAR7;                /* NO */
    arch_uint32    enableGraphPadConv;             /* NO */
    arch_uint32    enableGraphMerge;               /* NO */
    arch_uint32    enableGraphDump;                /* NO */
    arch_uint32    enableTransformNMConv;          /* NO */
    arch_uint32    enableGraphConvertAvgPool2Conv; /* NO */
    arch_uint32    enableGraphUnrollDWConv;        /* NO */
    arch_uint32    enableGraphOptimizationToTest;  /* NO */
    arch_uint32    enableGraphConvertBatchFC2NNConv;/* NO */
    arch_uint32    enableGraphConvertTensorAdd;    /* NO */
    arch_uint32    enableGraphEltwiseOpShape;      /* NO */
    arch_uint32    enableGraphConvertConv2Fc;      /* NO */
    arch_uint32    enableGraphSwaplayer;           /* NO */
    arch_uint32    enableGraphReshapelayer;        /* NO */
    arch_uint32    enableGraphConcalayer;          /* NO */
    arch_uint32    enableGraphMergeTranspose;      /* NO */
    arch_uint32    enableGraphDeleteRelu;          /* NO */
    arch_uint32    enableGraphDeleteSqueeze;       /* NO */
    arch_uint32    enableGraphAvgPoolandPWConv;    /* NO */
    arch_uint32    freqInMHZ;
    arch_uint32    axiClockFreqInMHZ;
    arch_uint32    maxSocOTNumber;
    arch_uint32    enableHuffmanEnhancement; /* refine me: not used, can be removed*/
    arch_uint32    enableTPHuffman;
    arch_uint32    enableMultiVIPCombined;         /* NO */
    arch_uint32    enableVectorPrune;              /* NO */
    arch_uint32    enableYUV2RGBScaler;            /* NO */
    arch_uint32    enableVIPDEC400;
    arch_uint32    enableCacheBinaryGraph;         /* NO */
    char          *enableOpsDebugInfo;                  /* NO */
    arch_uint32    enableSubnetworkSplitting;
    arch_uint32    enableMPSplitZ; /* if 0, will perform splitX */

    arch_uint32    setCoreCount;
    arch_uint32    graphBatchCount;
    arch_uint32    specificDDRLimitByBurst;

    /* add env setting for DDR Burst */
    arch_float32   ddrReadSustainedBw64BBurst;
    arch_float32   ddrReadSustainedBw128BBurst;
    arch_float32   ddrReadSustainedBw256BBurst;
    arch_float32   ddrMaskWriteSustainedBw64BBurst;
    arch_float32   ddrMaskWriteSustainedBw128BBurst;
    arch_float32   ddrMaskWriteSustainedBw256BBurst;
    arch_float32   ddrNonMaskWriteSustainedBw64BBurst;
    arch_float32   ddrNonMaskWriteSustainedBw128BBurst;
    arch_float32   ddrNonMaskWriteSustainedBw256BBurst;
}
arch_drv_option;
/******* DONE:Configration struction definition ***************************/

/*************************** maping table *******************************/
typedef struct _archMappingTableStr
{
    arch_int32 operationId;
    arch_uint32 layerId;
}archMappingTableStr;



/************************************************  GIB related definition *******************************************/
typedef struct _archGIBIO {
    arch_int32  gib_input[MAX_PARENT_CHILD_OP_NUM];
    arch_int32  gib_output[MAX_PARENT_CHILD_OP_NUM];
    arch_uint32 gib_input_count;
    arch_uint32 gib_output_count;
} archGIBIO;

typedef struct _archGIBObj
{
    arch_uint32 gid;
    arch_uint32 totalBufferNeeded;
    arch_uint32 layerInBufferSize;
    arch_uint32 layerOutBufferSize;
} archGIBObj;
/*************** DONE:GIB related definition *******************************************/

/*************** ArchModel struction definition *******************************************/
/* Basic Operation Information in Arch Model,will used for store the full detail for one operation */
struct _archModelOpInfo
{
    /* add for remove opt */
    arch_uint32 tpType;                        /* used for driver to decide op target */
    arch_int32  absoluteOperationID;
    arch_int32  uid;
    arch_int32  layerId;
    const char  *layerName;
    arch_int32  operationId;                /* The operation index for a specific layer */

    arch_uint32 upLayerCount;
    arch_uint32 upOpCount;
    arch_int32  parentOpId[MAX_PARENT_CHILD_OP_NUM];         /* operation ID */
    arch_uint32 parentOpType[MAX_PARENT_CHILD_OP_NUM];      /* operation type */
    arch_int32  parentLayer[MAX_PARENT_CHILD_OP_NUM];       /* layer Id */
    arch_int32  parentAbsId[MAX_PARENT_CHILD_OP_NUM];       /* parent abs Id */
    /* Some of the layer will not save into Opinfo (Tensor scale), so we can not find the name in Opinfo  */
    arch_uint32 parentLayerType[MAX_PARENT_CHILD_OP_NUM];   /* layer type */

    arch_uint32 downLayerCount;
    arch_uint32 downOpCount;
    arch_int32  childOpId[MAX_PARENT_CHILD_OP_NUM];
    arch_uint32 childOpType[MAX_PARENT_CHILD_OP_NUM];
    arch_int32  childLayer[MAX_PARENT_CHILD_OP_NUM];
    arch_int32  childAbsId[MAX_PARENT_CHILD_OP_NUM];       /* child abs Id */
    /* Some of the layer will not save into Opinfo (Tensor scale), so we can not find the name in Opinfo  */
    arch_uint32 childLayerType[MAX_PARENT_CHILD_OP_NUM];

    /* fixed */
    archnne_operator_e     op;                        /* optype (convolution), may the same with operatorType */
    archnne_operation_target_e     target;                    /* target type (NN/TP/SH) */
    arch_uint32   inx;                            /* Original InputX, get from driver */
    arch_uint32   iny;                            /* Original InputY, get from driver */
    arch_uint32   inz;                            /* Original InputZ, get from driver */
    arch_uint32   calcinx;                        /* calculated inx based on output in NN. Equal to original input in TP and FC */
    arch_uint32   calciny;                        /* calculated iny based on output in NN. Equal to original input in TP and FC */
    arch_uint32   origoutx;                        /* Original OutputX, get from driver */
    arch_uint32   origouty;                        /* Original OutputY, get from driver */
    arch_uint32   stridex;                    /* from WB,remove TBD */
    arch_uint32   stridey;                    /* from WB,remove TBD */
    arch_uint32   kx;                        /* kernel X */
    arch_uint32   ky;                        /* kernel Y */
    arch_uint32   kz;                        /* kernel Z */
    arch_uint32   bfy;                        /* Need to calc in init */
    arch_uint32   bfz;                        /* Need to calc in init */
    arch_uint32   oz;                        /** OUTZ?? **/
    arch_uint32   siz;                        /* siz == oz */
    arch_uint32   psize;                    /* Polling Size */
    arch_uint32   pstride;                    /* Polling Stride */
    arch_uint32   xpad;                        /* for calclate XOffset */                        /* from operation?? */
    arch_uint32   ypad;                        /* for calclate YOffset */                        /* from operation?? */
    arch_uint32   inputDataSize;
    arch_uint32   outputDataSize;                    /* data size */
    arch_uint8    fcmd;                        /* set to ture now */
    arch_uint32   inputDataFormat;
    arch_uint32   outputDataFormat;        /* FP16 (1 or 0) */
    arch_uint32   nnCores;                    /* From nnConfig */
    arch_perf_s   perf;                        /* Calc during predic*/
    arch_uint32   xsize;                    /* xsize == origX */
    arch_uint32   ysize;                    /* ysize == origY */
    /* tmp */
    arch_uint32   pix;                        /* Need to calc */
    arch_uint32   piy;                        /* Need to calc */
    arch_uint32   p3;                        /* polling size is 3 or not */
    arch_uint32   psix;                        /* unused, remove TBD */
    arch_uint32   psiy;                        /* unused, remove TBD */
    /* calculate during predict*/
    arch_uint8    sbuf;
    arch_uint8    dbuf;
    arch_uint8    kbuf;
    arch_uint32   swTilingSegKernelBufSizeInPixel;
    arch_uint32   swTilingSegOutBufSizeInPixel;
    arch_uint32   segTotalBufferSizeInPixel;
    arch_int32    swTilingType;/*-1: none, 1: AB, 0: Sub-IMAGE*/
    arch_uint32   upStreamLayerCount;
    arch_uint32   downStreamLayerCount;
    arch_int32    upStreamLayer[MAX_PARENT_CHILD_OP_NUM];           /* abs id for up */
    arch_int32    downStreamLayer[MAX_PARENT_CHILD_OP_NUM];         /* abs id for down */
};
/*************** DONE: ArchModel struction definition *******************************************/

/*************** Graph Info struction definition *******************************************/
/* Basic Operation Information in Arch Model,will used for store the full detail for one operation */
typedef struct _archModelGraphInfo
{
    /* basic info */
    arch_uint32 totalCount;
    arch_int32  layerId;
    arch_uint32 layerType;            /* convert to layerName */
    arch_int32  absId;
    arch_int32  operationId;
    archnne_operator_e opType;                /* convert to operation name */
    archnne_operation_target_e opTarget;            /* convert to operation target name */

    /* layer up/down stream */
    arch_uint32 upStreamLayerCount;        /* up layer count, including SH or invalid layer */
    arch_int32  upStreamLayer[MAX_PARENT_CHILD_OP_NUM];           /* up layer id array */
    arch_uint32 upStreamLayerType[MAX_PARENT_CHILD_OP_NUM];      /* up layer type array */
    arch_uint32 upStreamOpCount;        /* up OP count, real valid op count, can be deleted */
    arch_int32  upStreamOp[MAX_PARENT_CHILD_OP_NUM];              /* upstream operation ID, should be 0,1,2,3.... */
    arch_uint32 upStreamOpType[MAX_PARENT_CHILD_OP_NUM];         /* upstream operation type array */
    arch_int32  parentAbsId[MAX_PARENT_CHILD_OP_NUM];            /* parent abs Id */

    arch_uint32 downStreamLayerCount;
    arch_int32  downStreamLayer[MAX_PARENT_CHILD_OP_NUM];
    arch_uint32 downStreamLayerType[MAX_PARENT_CHILD_OP_NUM];
    arch_uint32 downStreamOpCount;      /* can be deleted */
    arch_int32  downStreamOp[MAX_PARENT_CHILD_OP_NUM];
    arch_uint32 downStreamOpType[MAX_PARENT_CHILD_OP_NUM];
    arch_int32  childAbsId[MAX_PARENT_CHILD_OP_NUM];       /* child abs Id */

    /* parameters */
    arch_uint32    origInX;
    arch_uint32    origInY;
    arch_uint32    origInZ;
    arch_uint32    nnOutX;
    arch_uint32    nnOutY;
    arch_uint32    nnOutZ;
    arch_uint32    subNNOutX;
    arch_uint32    subNNOutY;
    arch_uint32    subNNOutZ;
    arch_uint32    finalOutX;
    arch_uint32    finalOutY;
    arch_uint32    finalOutZ;
    arch_uint32    kx;
    arch_uint32    ky;
    arch_uint32    kz;
    arch_uint32    pollingSize;
    arch_uint32    pollingStride;
    arch_uint32    inputDataSize;
    arch_uint32    outputDataSize;
    arch_uint32    isFp16;
    arch_uint32    stridex;
    arch_uint32    stridey;
    /* int32 */
    arch_int32    xOffset;
    arch_int32    yOffset;

    /* Compress */
    arch_float64 coefNonZeroRatio;
    arch_float64 coefCompression;
    arch_float64 imageCompression;
    arch_float64 imageNonZeroRatio;
}archModelGraphInfo;
/*#pragma pack(pop)*/
/*************** DONE: Graph Info struction definition *******************************************/

/****************************************Hardware Information***********************************/

/* Chip models. */
typedef enum _archCHIPMODEL
{
    arch200  = 0x0200,
    arch300  = 0x0300,
    arch320  = 0x0320,
    arch328  = 0x0328,
    arch350  = 0x0350,
    arch355  = 0x0355,
    arch400  = 0x0400,
    arch410  = 0x0410,
    arch420  = 0x0420,
    arch428  = 0x0428,
    arch450  = 0x0450,
    arch500  = 0x0500,
    arch520  = 0x0520,
    arch530  = 0x0530,
    arch600  = 0x0600,
    arch620  = 0x0620,
    arch700  = 0x0700,
    arch800  = 0x0800,
    arch860  = 0x0860,
    arch880  = 0x0880,
    arch900  = 0x0900,
    arch1000 = 0x1000,
    arch1500 = 0x1500,
    arch2000 = 0x2000,
    arch2100 = 0x2100,
    arch2200 = 0x2200,
    arch2500 = 0x2500,
    arch3000 = 0x3000,
    arch4000 = 0x4000,
    arch5000 = 0x5000,
    arch5200 = 0x5200,
    arch6400 = 0x6400,
    arch7000 = 0x7000,
    arch7400 = 0x7400,
    arch8000 = 0x8000,
}
archCHIPMODEL;

typedef enum _archCHIP_FLAG
{
    archCHIP_FLAG_MSAA_COHERENCEY_ECO_FIX = 1 << 0,
    archCHIP_FLAG_GC2000_R2               = 1 << 1,
    archCHIP_AXI_BUS128_BITS              = 1 << 2,
}
archCHIP_FLAG;

typedef struct _archHAL_CHIPIDENTITY
{
    archCHIPMODEL  chipModel;
    arch_uint32    chipRevision;
    arch_uint32    productID;
    arch_uint32    customerID;
    arch_uint32    ecoID;
    /* May not needed by APM */
    archCHIP_FLAG  chipFlags;
    arch_uint64    platformFlagBits;
}
archHAL_CHIPIDENTITY;
/***************DONE: Hardware Information****************************************/

/********************************* typedef definition *****************************/
typedef struct _arch_perf_s* arch_perf;
typedef struct _archModelInfo archModelInfo;
typedef struct _archModelOpInfo archModelOpInfo;


/****************Arch Sw Lib Handle Context definition******************************/
/* Context information for archSwLibContext */
typedef struct _archSwLibContext
{
    archModelGraphInfo     **graphInfo;
    arch_uint32              totalCount;
    arch_nn_config          *pArchNnConfig;
    arch_drv_option         *pArchOptions;
    archHAL_CHIPIDENTITY    *pChipIdentity;
    archNN_DATABASE_FEATURE *pArchDataFeature;

    /* arch perf result for return */
    arch_perf_result_s **archPerfResult;

    /* HW handle */
    APMHandle apm;

    /* flag to indicate the information from log or excel */
    arch_uint32 flag;
}archSwLibContext;
/****************DONE: Arch Sw Lib Handle Context definition************************/

#ifdef _WIN32
#ifdef DLL_EXPORT
#define ARCH_MODEL_SW_API __declspec (dllexport)
#else
#define ARCH_MODEL_SW_API __declspec (dllimport)
#endif
#else
#define ARCH_MODEL_SW_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/********************** API definition ************************/
/* Standard Interface for external call */
ARCH_MODEL_SW_API arch_status archSwLibDeInit(
    archSwLibHandle pArchSwLibHandle
    );

ARCH_MODEL_SW_API archSwLibHandle archSwLibInit(
    arch_nn_config          *pArchNnConfig,
    arch_drv_option         *pArchOptions,
    arch_uint32              totalCount,
    archHAL_CHIPIDENTITY    *pChipIdentity,
    archNN_DATABASE_FEATURE *pArchDataFeature,
    arch_uint32             flag
    );

ARCH_MODEL_SW_API arch_status archSetManualParams(
    APM_MANUAL_PARAMS_T params
    );

ARCH_MODEL_SW_API arch_status archSetManualFeatures(
    APM_MANUAL_FEATURE_T features
    );

ARCH_MODEL_SW_API arch_status archPredictPerfFillLayer(
    archSwLibHandle     pArchSwLibHandle,
    archModelGraphInfo *layerInfo,
    arch_uint32         index
    );

ARCH_MODEL_SW_API arch_status archPredictPerfAnalysing(
    archSwLibHandle pArchSwLibHandle
    );

ARCH_MODEL_SW_API arch_status archGetPerfResult(
    archSwLibHandle     pArchSwLibHandle,
    arch_uint32         index,
    arch_perf_result_s *archPerfResult
    );

/* Function for doing Arch Model Predict, predict mode */
ARCH_MODEL_SW_API arch_status archPredictPerf(
    APMHandle                apm,
    archModelOpInfo        **archOpInfo,
    arch_uint32              totalOpCount,
    arch_nn_config          *pInArchNnConfig,
    arch_drv_option         *pInArchOptions,
    archNN_DATABASE_FEATURE *pInArchDataFeature,
    archHAL_CHIPIDENTITY *pInChipDentity
    );

ARCH_MODEL_SW_API void archSwLibDeInitFromDriver(
    archSwLibHandle *archSwLibHandle
    );

ARCH_MODEL_SW_API archSwLibHandle archSwLibInitFromDriver(
    archHAL_CHIPIDENTITY *pChipIdentity,
    APMHandle apm
    );

/* for check feature, be internal used and by archModelInterface */
ARCH_MODEL_SW_API arch_bool archIsFeatureAvailable(
    arch_nn_config          *pArchNnConfig,
    arch_drv_option         *pArchOptions,
    archNN_DATABASE_FEATURE *pArchDataFeature,
    arch_nn_feature_e        feature
    );

/* export for driver, this interface used when doing SWTLING or AB segment */
ARCH_MODEL_SW_API arch_status archCalculateArchPerf(
    APMHandle                apm,
    arch_nn_config          *pArchNnConfig,
    arch_drv_option         *pArchOptions,
    archNN_DATABASE_FEATURE *pArchDataFeature,
    arch_perf                perf,
    archnne_operation_target_e op_target,
    archnne_operator_e       op_type
    );

/* Fill OpInfo from Graph Info */
ARCH_MODEL_SW_API arch_status archFillOpInfo(
    archModelOpInfo    **opInfo,
    archModelGraphInfo **graphInfo,
    arch_nn_config      *pArchNnConfig,
    arch_uint32          totalCount
    );

/* Init/Deinit OpInfo */
ARCH_MODEL_SW_API archModelOpInfo ** initArchOpInfo(
    arch_uint32 operationCount
    );

ARCH_MODEL_SW_API void deInitArchOpInfo(
    archModelOpInfo **archOpInfo,
    arch_uint32 operationCount
    );

/* Init/Deinit graph Info */
ARCH_MODEL_SW_API archModelGraphInfo ** initArchGraphInfo(
    arch_uint32 operationCount
    );

ARCH_MODEL_SW_API void deInitArchGraphInfo(
    archModelGraphInfo **archGraphInfo,
    arch_uint32 operationCount
    );

/* Init/Deinit moveHistory/moveReverseMap */
ARCH_MODEL_SW_API arch_int32 * initOptimizeGraphInfo(
    arch_int32 operationCount
    );

ARCH_MODEL_SW_API void deInitOptimizeGraphInfo(
    arch_int32 *pTMP
    );

ARCH_MODEL_SW_API arch_status optimizeGraph(
    archModelOpInfo **    opInfo,
    arch_uint32        *    pTotalcount,
    int                *    moveHistory,
    int                *    moveReverseMap,
    int                *    opGraphIdMap
    );

ARCH_MODEL_SW_API arch_status optimizeGraph_recoverPosition(
    archModelOpInfo **    opInfo,
    int                *    moveReverseMap,
    int                    nonOpt_totalcount,
    arch_uint32        *    totalCount,
    int                *    opGraphIdMap
    );

/* show performance in Lib, internal use */
arch_status showArchPerformanceLib(
    archHAL_CHIPIDENTITY *pChipDentify,
    arch_nn_config    *pArchNnConfig,
    arch_drv_option   *pArchOptions,
    archNN_DATABASE_FEATURE *pArchDataFeature,
    archModelOpInfo **opInfo,
    arch_uint32       index,
    arch_perf         perf
    );

ARCH_MODEL_SW_API void updateAxiSram(
    arch_uint32 size
    );

#ifdef __cplusplus
}
#endif
#endif /* _ARCH_SW_PERF_H_ */
