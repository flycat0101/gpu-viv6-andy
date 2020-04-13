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


#ifndef _NN_ARCH_PERF_H_
#define _NN_ARCH_PERF_H_
#pragma once

#define bool int
/* feature defines, arch perf features is from feature DB */
/* driver has an structure, use same structure will be better(?) */
typedef struct _CHIP_DEF
{
    unsigned int ChipID;
    unsigned int ChipVersion;
    unsigned int ProductID;
    unsigned int EcoID;
    unsigned int CustomerID;
}CHIP_DEF_T;

typedef struct _compression_info
{
    double coefNonZeroRatio;
    double coefCompression;
    double imageCompression;
    double imageNonZeroRatio;
}COMPRESSION_INFO_T;

/* todo */
typedef struct _fpga_info
{
    int vip_freq;
}FPGA_INFO_T;


/* bandwidth latency */
typedef struct _bandwidth_latency
{
    float ddr_read_bw_in_byte_per_cycle;
    float ddr_write_bw_in_byte_per_cycle;
    float ddr_total_bw_in_byte_per_cycle;

    float axiSramReadBWLimit;
    float axi_sram_write_bw_limit;
    float axi_sram_total_bw_limit;

    float axi_bus_read_bw_limit;
    float axi_bus_write_bw_limit;
    float axi_bus_total_bw_limit;

    float internal_write_bw_limit;

    float ddr_latency;
    float total_latency;
    float maxSocOTNumber;

    /* todo: kernelDDR readbandwidth inImageDDRReadBandwidth, may be more */
    /* following info is for FPGA or Simulation test bench to model the behavior of DDR*/
    float  DDR_READ_BW_IN_BYTE_PER_CYCLE_64B;
    float  DDR_READ_BW_IN_BYTE_PER_CYCLE_128B;
    float  DDR_READ_BW_IN_BYTE_PER_CYCLE_256B;
    float  DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_64B;
    float  DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_128B;
    float  DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_256B;
    float  DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_64B;
    float  DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_128B;
    float  DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_256B;
}BWL_T;

/* ChipDefines and FPGA definations */
typedef struct _apm_in_param_t
{
    CHIP_DEF_T         chipDef;  /* chip defination, used to get featureDB */
    FPGA_INFO_T        fpagInfo; /* fpga info */
    BWL_T              bwl;      /* BandWidth and Latency */

    unsigned int       NN_DDR_BURST_SIZE;
    unsigned int       INT8_MAC_PER_CYCLE_NN;
    unsigned int       specified_ddr_bw_limit_by_burst;
}APM_IN_PARAM_T;

/* infomation can from a NN command. */
typedef struct _commandInfo
{
    union
    {
        struct __nncmd
        {
            unsigned int tile_xsize; /* tileXsize */
            unsigned int tile_ysize; /* tileYsize */
            unsigned int kernel_per_core; /* kernels per core */
        } nncmd;

        struct __tpcmd
        {
            unsigned int x; /* tileXsize */
            unsigned int y; /* tileYsize */
            unsigned int z; /* kernels per core */
        } tpcmd;
    } u;
    unsigned int    outImageXsize;
    unsigned int    outImageYsize;
    unsigned int    inSIXRefined; /* subImage X size */
    unsigned int    inSIYRefined; /* subImage Y size */

    unsigned int    kernelXsize;
    unsigned int    kernelYsize;
    unsigned int    kernelZsize;
    unsigned int    outImageZsize;
    unsigned int    pooling_stride; /* can be NN command or tpcommand, refine me */
    unsigned int    brick_mode;
    unsigned int    is_depth_wise;
    unsigned int    is_depth_wise_merge;
    unsigned int    src_buf;    /* src buf in DDR or SRAM */
    unsigned int    dst_buf;    /* dst buf in DDR or SRAM */
    unsigned int    in_image_stride;
    unsigned int    out_image_stride;
    unsigned int    kernel_buf; /*  kernel buf in DDR or SRAM */
    unsigned int    inImageFp16;

    unsigned int    bInImagePadSlice;
    unsigned int    bOutImagePadSlice;

    unsigned int    TrspInterleaveCh_in;   /* transpose interleave channel input  */
    unsigned int    TrspInterleaveCh_out;  /* transpose interleave channel output */
    bool            isTA_MERGE;
}APM_COMM_INFO_T;

/* do not know where to place them
 refine me, refine me, may from feature DB?*/
typedef struct __apm_perf_misc
{
    bool   asymmetric_quantization;
    /* double Internal_Kernel_Read_BYTE_PER_CYCLE; */
} AMP_PERF_MISC;

typedef struct _apm_perf
{
    unsigned int kernelBitSize[32]; /*max 32 core supported*/
}AMP_KERNEL_SIZE_INFO;

/* input parameters for calculate arch perf */
typedef struct _am_perf_param
{
    COMPRESSION_INFO_T compInfo; /* compresion info, set by Driver, DV. todo: move it  */
    APM_COMM_INFO_T    cmdInfo;
    /* two path, driver set and model computer itself */
    unsigned int       inDataBitSize; /* inImageData Size in bit*/
    unsigned int       outDataBitSize; /* outImageData Size in bit*/
    unsigned int       xydp_x;
    unsigned int       xydp_y;
    unsigned int       interleave_mode; /* computer it self. */
    unsigned int       vip_v7_16bit;            /* vip v7 and it's bit 16 */
    unsigned int       vector_prune; /* set by application. default seems to be always 1, to check it */
    unsigned int       in_image_slice;
    unsigned int       out_image_slice;
    unsigned int       bflush;
    unsigned int       first_cmd;
    unsigned int       l2cache_size;
    int                op_type;
    AMP_PERF_MISC misc;
    AMP_KERNEL_SIZE_INFO kSizes;
    unsigned int       small_batch_enable;

} APM_IN_PERF_PARAMS;


typedef struct _apm_out_bandwidth
{
    double ddr_kernel_read_bandwidth;
    double ddr_in_image_read_bandwidth;
    double ddr_read_bandwidth;
    double ddr_write_bandwidth;
    double axi_read_bandwidth;
    double axi_write_bandwidth;

    double axi_sram_read_bw; /* Only TP use it, why? */
    double axi_sram_write_bw;
    unsigned int  is_compute_bottle_neck;

    double cycle_count;
} APM_OUT_BW_T; /* rename to APM_OUT_COST_T */

typedef enum _bottleneck
{
    COMPUTE        = 0,
    COEF_DECODE    = 1,
    DQ_ARB         = 2,
    XBAR           = 3,
    VIP_SRAM_RD    = 4,
    VIP_SRAM_WR    = 5,
    AXI_BUS        = 6,
    AXI_SRAM       = 7,
    DDRRead        = 8,
    DDRWrite       = 9,
    DDR            = 10,
    INTERNAL_WRITE = 11,
    KERNEL_READ    = 12,
    IMAGE_READ     = 13,
    INTERNAL_KERNEL_READ = 14,
    Rd_Arb         = 15,
    USC_CONTROLLER = 16,
    LP_WRITE       = 17,
    LP_READ        = 18
} APM_BOTTLENECK_T;

typedef struct _cycleCount
{
    double Compute;
    double DDRRead;
    double DDRWrite;
    double DDRTotal;
    double AXISRAMRead;
    double AXISRAMWrite;
    double AXISRAMTotal;
    double AXIBUSRead;
    double AXIBUSWrite;
    double AXIBUSTotal;
    double VIPSRAMRead;
    double VIPSRAMWrite;

    double DDRRead_Combined_Bursts;
    double DDRWrite_Combined_Bursts;

    double Slow_InternalWrite;
    double Slow_Comp;
    double InternalWrite;
    double InternalKernelRead;
    double KernelDDRRead;
    double InImageDDRRead;
    double KernelDecodeBW;
    double DWOut;
    double DQArb;
    double RegTile2DXBar;
    double BottomTile2DXbar;
    double XBAR;
    double CacheController;
    double RdReturnArbiter;
    double Overheads;
    double Overall;
    char BottleNeck[64];

    APM_BOTTLENECK_T BottleNeck_e;
}CycleCounts;

typedef struct _arch_model_cache_type
{
    double sizeCached;
    double sizeNeeded;
    double percentage;
}arch_model_cache_type;

// refine me, this is ByRegion_Type in VB
typedef struct _apm_bw_cycle_cost
{
    double cost; /* Total */
    double tile0;
    double tile0VZGroup0;
    double vzGroup0; /* vzgroup0 */
    double tile0ResetVZGroup;
    double resetTileVZGroup0;
    double resetTileResetVZGroup;
    double residual; /*for write bw*/
} APM_BW_CYCLE_COST_T;

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

typedef APM_BW_CYCLE_COST_T ByRegion_Type;

typedef struct _Outputs
{
    CycleCounts tile0_vzgroup0;
    CycleCounts tile0_rest_vzgroup;
    CycleCounts rest_tile_vzgroup0;
    CycleCounts rest_tile_rest_vzgroup;
    char WARType[64];

    /* add cache type in outputs */
    arch_model_cache_type imageCacheType;
    arch_model_cache_type kernelCacheType;

    ByRegion_Type DDRReadBW_64B;
    ByRegion_Type DDRReadBW_128B;
    ByRegion_Type DDRReadBW_256B;
    ByRegion_Type DDRNonMaskWriteBW_64B;
    ByRegion_Type DDRNonMaskWriteBW_128B;
    ByRegion_Type DDRNonMaskWriteBW_256B;
    ByRegion_Type DDRMaskWriteBW_64B;
    ByRegion_Type DDRMaskWriteBW_128B;
    ByRegion_Type DDRMaskWriteBW_256B;

} Outputs_Type;

/* detail calculate result */
typedef struct _apm_out_result
{
    /* WAR Type */
    unsigned int warType;

    /* All kind of cyclecount */
    double computeCC;
    double ddrRdCC;
    double ddrWrCC;
    double axiSramRdCC;
    double axiSramWrCC;
    double axiBusRdCC;
    double axiBusWrCC;
    double axiBusTotalCC;
    double vipSramRdCC;
    double vipSramWrCC;
    double slowInternalWrCC;
    double slowCompCC;
    double internalWrCC;
    double dWOutCC;
    double kernelDdrRdCC;
    double inImageDdrRdCC;
    double kernelDecodeCC;
    double dqArbCC;
    double regTile2DxBarCC;
    double bottomTile2DXBarCC;
    double xBarCC;
    double cacheControllerCC;
    double overHeadsCC;
    double overAllCC;

    /* region cycles/Bottleneck */
    double cyclesTile0Vzgroup0;
    double cyclesTile0RestVzgroup0;
    double cyclesRestTileVzgroup0;
    double cyclesRestTileRestVzgroup0;

    // should be char[]
    double BottleneckTile0Vzgroup0;
    double BottleneckTile0RestVzgroup0;
    double BottleneckRestTileVzgroup0;
    double BottleneckRestTileRestVzgroup0;

    APM_BOTTLENECK_T BN_BottleNeck_e;
    APM_BOTTLENECK_T BN_Tile0Vzgroup0_e;
    APM_BOTTLENECK_T BN_Tile0RestVzgroup0_e;
    APM_BOTTLENECK_T BN_RestTileVzgroup0_e;
    APM_BOTTLENECK_T BN_RestTileRestVzgroup0_e;

    double DDRRead_Combined_Bursts;  // "LP_READ"
    double DDRWrite_Combined_Bursts; // "LP_WRITE"

    Outputs_Type outputs;
} APM_OUT_RESULT_T;

typedef struct _apm_cost_t
{
    APM_BW_CYCLE_COST_T readBW;
    APM_BW_CYCLE_COST_T writeBW;
    APM_BW_CYCLE_COST_T readCycle;
    APM_BW_CYCLE_COST_T writeCycle;
} APM_COST_T;

typedef struct _arch_model_bw_byburst_type
{
    APM_BW_CYCLE_COST_T BW_64B;
    APM_BW_CYCLE_COST_T BW_128B;
    APM_BW_CYCLE_COST_T BW_256B;
} arch_model_bw_byburst_type;

typedef struct _apm_collection_t
{
    double NMW_S64;
    double NMW_S128;
    double NMW_S256;
}APM_COLLECTION_T;

typedef struct _apm_wr_bw_byburst
{
    arch_model_bw_byburst_type mask;
    arch_model_bw_byburst_type nonmask;
}APM_WR_BW_BYBURST_T;

typedef struct _apm_cycle_byburst
{
    APM_BW_CYCLE_COST_T CYCLE_64B;
    APM_BW_CYCLE_COST_T CYCLE_128B;
    APM_BW_CYCLE_COST_T CYCLE_256B;
}APM_CYCLE_BYBURST_T;

typedef enum _apm_nn_cost
{
    APM_DDR_COST          = 0,
    APM_VIP_SRAM_COST     = 1,
    APM_AXI_SRAM_COST     = 2,
    APM_AXI_BUS_COST      = 3,
    APM_DDR_KERNEL_COST   = 4,
    APM_DDR_IN_IMAGE_COST = 5,
    APM_DDR_READ_BURST_COST          = 6,
    APM_DDR_KERNEL_READ_BURST_COST   = 7,
    APM_DDR_IN_IMAGE_READ_BURST_COST = 8,
    APM_RD_RETURN_ARB_COST           = 9,
    APM_DDR_READ_BURST_COMBINE_COST  = 10,
    APM_TOTAL_NN_COST_TYPE           = 11,
} APM_NN_COST_T;

typedef struct _apm_manual_params
{
    unsigned int k; // kernels per core
    unsigned int outTileXsize;
    unsigned int outTileYsize;
    unsigned int inX;
    unsigned int inY;
} APM_MANUAL_PARAMS_T;

typedef struct _apm_manual_feature
{
    bool small_bach_enable;
    unsigned int lanes_per_conv;
    unsigned int in_lines_per_cycle;
    bool prefetchNNComandKernelHeader;
    bool no_narrow_post_process_pipe;

    bool spliteZDimension;
    bool DDRAlighment;
    bool SWAlignment;
    bool HWAlgnment;
    bool new_feature; //Shuangbei: define var in the structure of manual features
} APM_MANUAL_FEATURE_T;

#ifndef IN
#define  IN
#endif
#ifndef OUT
#define  OUT
#endif

/* function declaration, to be exported */

typedef void * APMHandle;

#ifdef _WIN32
#ifdef DLL_EXPORT
#define ARCH_PERF_MODEL_API __declspec (dllexport)
#else
#define ARCH_PERF_MODEL_API __declspec (dllimport)
#endif
#else
#define ARCH_PERF_MODEL_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

ARCH_PERF_MODEL_API APMHandle CreateAPModel(
    IN APM_IN_PARAM_T      inParam
    );

ARCH_PERF_MODEL_API void APMSetManualParams(
    IN APM_MANUAL_PARAMS_T manualParams
    );

ARCH_PERF_MODEL_API void APMSetManualFeatures(
    IN APM_MANUAL_FEATURE_T manualFeatures
    );

ARCH_PERF_MODEL_API double APMCalcNNCycleCountBandWidth(
    IN  APMHandle          handle,
    IN  APM_IN_PERF_PARAMS inPerfParams,
    OUT APM_OUT_BW_T       *outBandWidth,
    OUT APM_OUT_RESULT_T    *outResult
    );

ARCH_PERF_MODEL_API double APMCalcTPCycleCountCore(
    IN  APMHandle          apmHandle,
    IN  APM_IN_PERF_PARAMS inPerfParams,
    OUT APM_OUT_BW_T       *outBandWidth,
    OUT APM_OUT_RESULT_T   *outResult
    );

/* Add new function for export */
ARCH_PERF_MODEL_API unsigned int APMCalcImageInterleaveMode(
    unsigned int x,
    unsigned int mad_per_core,
    unsigned int kxy,
    unsigned int vip7_fp16,
    unsigned int interleave8);

ARCH_PERF_MODEL_API unsigned int APMCalcNumOfKernel(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int z,
    unsigned int accu_buf_depth,
    unsigned int cores,
    unsigned int interleave_mode,
    unsigned int zdp,
    unsigned int kx,
    unsigned int ky,
    unsigned int xdpx,
    unsigned int isV8,
    /*unsigned int data_size, */
    unsigned int lanes_per_conv,
    unsigned int pooling_stride,
    bool isDepthWise,
    bool isDepthWiseMerge,
    bool kernel_per_core_lt_one_third_coef_fix /* bug2000 */,
    bool asymmetricQuantization
    );

ARCH_PERF_MODEL_API double APMCalcKernelCachePercentage(
    unsigned int kx, unsigned int ky, unsigned int kz,
    unsigned int z,
    unsigned int cores,
    double coef_compress_ratio,
    unsigned int cache_size_in_pixel,
    OUT double *adj_cache_size_in_pixel,
    OUT double *KernelIdealCache
    /*,
    bool full_cach_kernel_head_fix,
    bool is_depth_wise */);

ARCH_PERF_MODEL_API float APMCalcImageIdealCacheInPixel(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int x,
    unsigned int y,
    int          xoffset,
    int          yoffset,
    unsigned int sub_x,
    unsigned int sub_y,
    unsigned int data_size,
    bool         image_not_packed_in_sram,
    unsigned int equivalent_vip_sram_width_in_byte
    );

ARCH_PERF_MODEL_API void DestroyAPModel(
    IN APMHandle           apmHandle
    );

#ifdef __cplusplus
}
#endif
#endif /*_NN_ARCH_PERF_H_*/
