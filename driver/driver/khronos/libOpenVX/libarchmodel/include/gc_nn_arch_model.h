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


#ifndef _GC_NN_ARCH_MODEL_H_
#define _GC_NN_ARCH_MODEL_H_

/*#pragma pack(push, 8)*/
typedef struct _vx_general_info_s
{
    vx_uint32                                kx;
    vx_uint32                                ky;
    vx_uint32                                kz;
    vx_uint32                                oinx;
    vx_uint32                                oiny;
    vx_uint32                                oinz;
    vx_uint32                                inx;
    vx_uint32                                iny;
    vx_uint32                                inz;
    vx_uint32                                outx;
    vx_uint32                                outy;
    vx_uint32                                outz;
    vx_uint32                                stridex;
    vx_uint32                                stridey;
    vx_uint32                                inputDataSize;
    vx_uint32                                outputDataSize;
    vx_uint32                                poolingSize;
    vx_uint32                                poolingStride;
    vx_int32                                 xOffSet;
    vx_int32                                 yOffSet;
    vx_int32                                 inputDataFormat;
    vx_int32                                 outputDataFormat;
    vx_int32                                 nnCores;
    vx_int32                                 convOutFifoDepth;
    vx_uint32                                kernelSize;
    vx_uint32                                pix;
    vx_uint32                                piy;
    vx_uint32                                p3;
    vx_uint32                                nextKY;/*the Y size of output subimage in AXI SRAM need it to add the subimage Y overlap*/
    vx_int32                                 flush;
}
vx_general_info_s;


typedef struct _vx_swtiling_info_s
{
    vx_uint8    srcBuf;
    vx_uint8    dstBuf;
    vx_uint8    kernelBuf;

    vx_int32    cacheSpace;
    vx_bool     calcNonFirstCmd;
    vx_bool     isNonFirstComputeBottleNeck;

    vx_float64  perfNonFirstCycleCount;
    vx_float64  perfNonFirstKernelReadBandWidth;
    vx_float64  perfNonFirstInImageReadBandWidth;
    vx_float64  perfNonFirstReadBandWidth;
    vx_float64  perfNonFirstWriteBandWidth;
    vx_float64  perfNonFirstAXIReadBandWidth;
    vx_float64  perfNonFirstAXIWriteBandWidth;

    vx_uint32   origInX;
    vx_uint32   origInY;
    vx_uint32   origOutX;
    vx_uint32   origOutY;
    vx_uint32   origOutZ;

    vx_enum     kernelCacheMode;
    vx_enum     imageCacheMode;


    vx_uint32   outImageStride;
    vx_uint32   outImageSlice;

    vx_uint32   swTilingSegKernelBufSizeInPixel;
    vx_uint32   segTotalBufferSizeInPixel;
}
vx_swtiling_info_s;

#define VIV_MAX_NN_CORE_COUNT               128
typedef struct _vx_arch_perf_s
{
    vx_general_info_s                        info;

    vx_swtiling_info_s                       swTilingInfo;

    vx_float64                               coefNonZeroRatio;
    vx_float64                               coefCompressRatio;
    vx_float64                               imageCompressRatio;
    vx_float64                               imageNonZeroRatio;
    vx_float64                               maxPerCoreCompressionRatio;
    vx_float64                               maxPerCorePerVZGroupNonZeroRatios[VIV_MAX_NN_CORE_COUNT];

    vx_performance_info_s                    resultInfo;

    vxnne_operator_e                         opType;
    vxnne_operation_target_e                 opTarget;

    vx_bool                                  calculated;
}
vx_arch_perf_s;
/*#pragma pack(pop)*/

VX_INTERNAL_API vx_status vxoGraph_PredictPerf(
    vx_graph graph);

VX_INTERNAL_API void calculateArchPerfFromWB(
    vx_context context,
    vx_arch_perf perf,
    vx_weights_biases_parameter wb,
    vx_uint32 orig_input_dims[],
    vx_uint32 output_dims[],
    vx_enum output_format,
    vx_int32* offsets,
    vx_int32 flush,
    vx_uint8 src_buf,
    vx_uint8 dst_buf,
    vx_uint8 kernel_buf,
    vx_int32 cached_space,
    vxnne_operation_target_e op_target,
    vxnne_operator_e op_type);

VX_INTERNAL_API void calculateArchPerfFromTiling(
    vx_context context,
    vxnne_layer layer,
    vx_arch_perf perf,
    vxnne_tensor_info input_tiling,
    vxnne_tensor_info output_tiling,
    vx_tensor input,
    vx_tensor output,
    vx_weights_biases_parameter wb,
    vxnne_operation_command  op_command,
    vxnne_operation_target_e op_target,
    vxnne_operator_e op_type);

VX_INTERNAL_API vx_status showArchPerformance(
    vx_context context,
    vxnne_layer layer,
    vxnne_operation op,
    vx_arch_perf perf
    );

#endif /* _GC_NN_ARCH_MODEL_H_ */
