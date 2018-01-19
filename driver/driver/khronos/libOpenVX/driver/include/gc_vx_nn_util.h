/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef _GC_VX_NN_UTIL_H_
#define _GC_VX_NN_UTIL_H_

#include <gc_vxk_common.h>

#define F16_EXPONENT_BITS 0x1F
#define F16_EXPONENT_SHIFT 10
#define F16_EXPONENT_BIAS 15
#define F16_MANTISSA_BITS 0x3ff
#define F16_MANTISSA_SHIFT (23 - F16_EXPONENT_SHIFT)
#define F16_MAX_EXPONENT (F16_EXPONENT_BITS << F16_EXPONENT_SHIFT)

#define FC_SIZE_MAX 134217728

#define NNE_COMMAND_SIZE 64

#define TP_COMMAND_SIZE 128
#define TP_LUT_BUFF_SIZE 1024

#define FP32_MIN -3.402823466e+38F

#define FP16_MAX 3.402823466e+38F
#define FP16_MIN -3.402823466e+38F

#define SATURATE_SIGN(type) \
    if (value > type##_MAX) \
            value = type##_MAX; \
        else if (value < type##_MIN) \
            value = type##_MIN;

#define SATURATE_UNSIGN(type) \
    if (value > type##_MAX) \
            value = type##_MAX; \
        else if (value < 0) \
            value = 0;

#define FP32_MAX 3.402823466e+38F

typedef struct _vx_tp_conv_cmd
{
    vx_uint32  pad_x;
    vx_uint32  pad_y;
    vx_uint32  pool_size_x;
    vx_uint32  pool_size_y;
    vx_uint32  pool_stride;
    vx_bool    enable_relu;
    vx_enum    conv_rounding_type;
}
vx_tp_conv_cmd;

typedef struct _vx_tp_value_cmd
{
    vx_float32    f32[2];
    vx_uint32     u32[7];
    vx_enum       e32[1];
    vx_uint8_ptr  p8[1];
}
vx_tp_value_cmd;

typedef struct _vx_tp_roi_pool
{
    vx_uint32 xcoord        :  8;   // [7:0]
    vx_uint32 ycoord        :  8;   // [15:8]
    vx_uint32 last          :  1;   // [16]
    vx_uint32 _pad0         : 15;   // [31:17]
    vx_uint32 poolingHInc   : 16;   // [15:0]
    vx_uint32 poolingVInc   : 16;   // [31:16]
}
vx_tp_roi_pool;

typedef struct _vx_nn_reshuffleStruct_s
{
    void *data;
    vx_uint32 xSize;
    vx_uint32 ySize;
    vx_uint32 zSize;
    vx_uint32 wSize;
    vx_type_e type;
}
vx_nn_reshuffle_s;

typedef enum
{
    VX_TENSOR_OP_ADD,
    VX_TENSOR_OP_MUL,
    VX_TENSOR_OP_SUB,
    VX_TENSOR_OP_DIV,
}
vx_op_type;

vx_float32 Fp16toFp32(const vx_uint16 in);
vx_int16 Fp32toFp16(vx_float32 val);
vx_int8 Fp32toInt8_fc(vx_float32 val);
vx_int8 Fp32toInt8(vx_float32 val, vx_int8 fixedPointPos, vx_int32 roundMode);
vx_float32 Int8toFp32(vx_int8 val, vx_int8 fixedPointPos);
vx_float32 Int32toFp32(vx_int32 val, vx_int8 fixedPointPos);
#if defined(__linux__)
vx_float64 _copysign(vx_float64 number, vx_float64 sign);
#endif
vx_float32 roundSimpleRounding(vx_float32 x);
vx_float64 roundRTNE(vx_float64 x);
vx_float32 roundRTZ(vx_float32 x);
vx_float32 roundRTNI(vx_float32 x);
vx_float32 vxnneRound(vx_float32 x, vx_enum roundMode);
vx_uint32 vxnneAlignWithStride(vx_uint32 inputSize, vx_uint32 stride);
void vxnneGetPadValue(
    vx_int32 padX,
    vx_int32 padY,
    vx_uint32 *padXLeft,
    vx_uint32 *padXRight,
    vx_uint32 *padYTop,
    vx_uint32 *padYBottom);
vx_int32 vxnneGetTypeSize(vx_type_e format);
void vxnneSRAMGetkernelPattenField(
    vx_uint32 vipSRAMSize,
    vx_uint32 kernelStreamSize,
    vx_uint32_ptr kernelCacheStartAddress,
    vx_uint32_ptr kernelCachingMode,
    vx_uint32_ptr kernelPatternLow32Bits,
    vx_uint32_ptr kernelPatternHigh32Bits,
    vx_uint32_ptr kernelPatternMsb
    );
vx_uint32 vxnneGetOneNumber(vx_uint32 value);
vx_float32 vxnneGetData(vx_type_e format, vx_int32 index, vx_uint8_ptr data, vx_uint8 fixPointPos);
vx_status vxnneSaveData(vx_type_e format, vx_int32 index, vx_float64 data, vx_ptr dst_data, vx_uint8 fixedPointPos, vx_enum roundMode);
vx_int32 vxoNNExternsionConvlutionRound(vx_float32 in, vx_enum round_type);
void vxoNNExternsionInputOutputArguments(
    vx_uint32                    org_input_x,
    vx_uint32                    org_input_y,
    vx_uint32                    stride_x,
    vx_uint32                    stride_y,
    vx_uint32                    pad_x_left,
    vx_uint32                    pad_x_right,
    vx_uint32                    pad_y_top,
    vx_uint32                    pad_y_bottom,
    vx_uint32                    weight_x,
    vx_uint32                    weight_y,
    vx_enum                      rounding_type,
    vx_uint32                    pool_size_x,
    vx_uint32 *                  input_x,
    vx_uint32 *                  input_y,
    vx_uint32 *                  output_x,
    vx_uint32 *                  output_y,
    vx_uint32 *                  finalInput_x,
    vx_uint32 *                  finalInput_y,
    vx_uint32 *                  finalOutput_x,
    vx_uint32 *                  finalOutput_y
    );
vx_bool vxoNNExternsionAdjustWeightsBiases(
    vx_weights_biases_parameter  wb,
    vx_bool                      process_size,
    vx_bool                      viewed,
    vx_size                      wb_size
    );
void reshuffleData(vx_nn_reshuffle_s *src, vx_uint32 strideStepX, vx_uint32 strideStepY, vx_nn_reshuffle_s *dst);
vx_uint32 _calcInImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core, vx_uint32 kernel_xy, vx_bool vip7_fp16);
vx_uint32 _calcOutImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core, vx_bool vip7_fp16);
vx_uint32 _calcImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core, vx_uint32 kxy, vx_bool vip7_fp16);
vx_uint32 _calcNumOfKernel(vx_uint32 tile_x, vx_uint32 tile_y, vx_uint32 z, vx_uint32 accu_buf_depth, vx_uint32 cores, vx_uint32 interleave_mode);
vx_uint32 _calcComputeCycleCount(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kernel_per_core,
    vx_uint32 x,
    vx_uint32 y,
    vx_uint32 z,
    vx_uint32 kx,
    vx_uint32 ky,
    vx_uint32 kz,
    vx_uint32 mad_per_core,
    vx_uint32 data_size,
    vx_uint32 dp_amount,
    vx_float32 non_zero_ratio,
    vx_uint32 interleave_mode,
    vx_bool vip7_fp16);
vx_float32 _calcKernel4DSingleReadRepeated(vx_uint32 tile_x, vx_uint32 tile_y, vx_uint32 x, vx_uint32 y);
vx_float32 _calcKernel4DSingleReadBW(vx_uint32 kx, vx_uint32 ky, vx_uint32 kz, vx_uint32 z, vx_float32 coef_compress_ratio);
vx_float32 _calcTile3DImageSingleReadRepeated(vx_uint32 z, vx_uint32 kernel_per_core, vx_uint32 cores);
vx_float32 _calcTile3DImageSingleReadBW(
    vx_uint32 tile_x, vx_uint32 tile_y,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 x, vx_uint32 y,
    vx_uint32 inx, vx_uint32 iny,
    vx_uint32 brick_mode,
    vx_uint32 data_size,
    vx_float32 image_compress_ratio);
vx_uint32 _calcReadBandWidth(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kernel_per_core,
    vx_uint32 x, vx_uint32 y, vx_uint32 z,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 inx, vx_uint32 iny,
    vx_uint32 cores,
    vx_uint32 brick_mode,
    vx_uint32 data_size,
    vx_float32 coef_compress_ratio,
    vx_float32 image_compress_ratio,
    vx_uint32 l2cache_size);
vx_uint32 _calcWriteBandWidth(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 x,
    vx_uint32 y,
    vx_uint32 z,
    vx_uint32 data_size,
    vx_float32 image_compress_ratio,
    vx_uint32 usc_cache_size,
    vx_uint32 pooling_stride);
vx_uint32  _calcTPReadBandWidth(vx_enum type, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 kz, vx_float32 coef_compress_ratio, vx_float32 image_nonzero_ratio);
vx_uint32 _calcTPWriteBandWidth(vx_enum type, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 pooling_stride);
vx_uint32 _calcTPCycleCount(vx_enum type, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 kz, vx_uint32 cores, vx_float32 coef_nonzero_ratio, vx_float32 image_nonzero_ratio);
void calculateFilterPerCore(vx_context context, vx_weights_biases_parameter wb, vx_uint32 zindex, vx_bool calctp, vx_enum ltype);
vx_status vxoWeightsBiasesParameter_ShowPerformance(
    vx_context context,
    vx_weights_biases_parameter wb
    );
vx_bool WeightBiasBufferAllocate(vx_context context, vx_weights_biases_parameter weight_bias, vx_size size, vx_bool raw_data);
vx_size calculateWeightBiasBufferSizeForZeroRunLen(vx_weights_biases_parameter wb, vx_tensor weights, uint8_t zeroRunLen, vx_uint32 filtersPerCore, vx_uint32 sliceCount, vx_uint32 z_count, vx_uint32 index, void* weightData);
void writeBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 data, vx_uint32 dataBits);
vx_uint32 readBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 dataBits);
void packZeros(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 alignedOffset);
void _DataGeneralConvert(void* input_ptr, void* output_ptr, vx_uint32 input_size, vx_uint32 output_size);
void calculateWeightBiasStreamRelatedSize(
    vx_weights_biases_parameter wb,
    vx_tensor weights,
    vx_uint32 kernels_per_core,
    vx_uint32 slice_count,
    vx_int32 output_z_index,
    void* weight_data,
    vx_size* min_kernel_buf_size,
    vx_uint8* min_zero_run_len,
    vx_uint32* max_zero_run_len,
    vx_int8 setZrl
    );
vx_size calculateTPWeightStreamSizeForZeroRunLen(
    vx_weights_biases_parameter wb,
    uint8_t zeroRunLenBitWidth,
    vx_uint32 sliceCount,
    vx_uint32 filter_count,
    vx_uint8_ptr weight_base_ptr
    );
void calculateWeightBiasTPBufferRelatedSize(
    vx_weights_biases_parameter wb,
    vx_int8 setZrl,
    vx_uint32 slice_count,
    vx_uint32 filter_count,
    vx_int32 zgroup_index,
    vx_uint8_ptr weight_base_ptr,
    vx_size* min_kernel_buf_size,
    vx_uint8* min_zero_run_len
    );
vx_weights_biases_parameter _createWeightsBiasesParameterFromTensors(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_tensor   weights,
    vx_tensor   biases
    );
vx_weights_biases_parameter vxoWeightsBiases_Create(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_dims,
    vx_enum     weights_data_format,
    vx_uint8    weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_uint8    biases_fixed_point_pos
    );
vx_weights_biases_parameter vxoWeightsBiasesFromStream_Create(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32   num_of_dims,
    vx_uint32 * inputs_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * outputs_dims,
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_dims,
    vx_uint32 * weights_org_dims,
    vx_enum     weights_data_format,
    vx_uint8    weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_uint8    biases_fixed_point_pos
    );
void fillinKernelBuffer(
    vx_weights_biases_parameter wb,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_uint32 output_final_x,
    vx_uint32 output_final_y,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32 outputSize,
    vx_uint32* bias_base_ptr
    );
void fillinTPKernelBuffer(
    vx_weights_biases_parameter wb,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32 index
    );
vx_status vxoWeightsBiasesParameter_ProcessHead(
    vx_weights_biases_parameter     weights_bias,
    vx_enum                         usage
    );
vx_status vxoWeightsBiasesParameter_Map(
    vx_weights_biases_parameter     weights_biases,
    vx_map_id *                     map_id,
    vx_uint32 *                     stride,
    void **                         ptr,
    vx_enum                         usage,
    vx_enum                         mem_type,
    vx_uint32                       flags
    );
vx_status vxoWeightsBiasesParameter_Unmap(
    vx_weights_biases_parameter weights_biases,
    vx_map_id                   map_id
    );
vx_bool vxoWeightsBiasesParameter_IsValid(
    vx_weights_biases_parameter     weights_biases
    );
vx_int32 getHwPoolingType(vx_enum poolingType);
vx_int32 getHWRoundingMode(vx_nn_round_mode_e roundingMode);
vx_int32 getHWBorderMode(vx_enum padMode);
void fillinCmmdBuff(
    vx_tensor                    inputs,
    vx_weights_biases_parameter  weights_biases,
    vx_uint32                    pad_x_left,
    vx_uint32                    pad_x_right,
    vx_uint32                    pad_y_top,
    vx_uint32                    pad_y_bottom,
    vx_enum                      pad_mode,
    void                        *pad_const,
    vx_enum                      conv_rounding_type,
    vx_bool                      enable_relu,
    vx_enum                      pool_type,
    vx_uint32                    pool_size_x,
    vx_uint32                    pool_size_y,
    vx_tensor                    outputs,
    vx_array                     cmd_buff,
    vx_bool                      isFullyConnectedLayer,
    vx_uint32                    index
    );
void fillInCmdTPBuffer(
    vx_tensor                    inputs,
    vx_reference                 other_tensor,
    vx_tensor                    outputs,
    vx_array                     cmd_buff,
    vx_array                     data_buff,
    vx_tp_conv_cmd *             conv_cmd_ptr,
    vx_tp_value_cmd *            value_cmd_ptr,
    vx_enum                      tp_type,
    vx_uint32                    index,
    vx_bool                      multi_tp,
    vx_bool                      last
    );
void vxoNNExternsionDoReshuffle(
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum   padMode,
    void*     padConst,
    vx_uint32 strideX,
    vx_uint32 strideY);
vx_char* vxnneGetOperatorTypeName(vxnne_operator_e operationType);
vx_char* vxnneGetOperatorTargetName(vxnne_operation_target_e operationTarget);
void _TransposeTensor(vx_uint8_ptr in_addr, vx_uint8_ptr out_addr, vx_uint32 data_size, vx_uint32_ptr dims, vx_uint32_ptr strides, vx_uint32_ptr trans_strides, vx_uint32* perm, vx_uint32 layer);
vx_float64 vxnneGetData64(vx_enum format, vx_int32 index, vx_uint8_ptr src, vx_uint8 fixPointPos);
vx_float64 vxnneSaturate(vx_float64 data, vx_enum format);
vx_float64 vxnneWarp(vx_float64 data, vx_enum format);
vx_status eltwise(
    vx_uint8_ptr input1_ptr,
    vx_uint8 input1FixPointPos,
    vx_enum input1Format,
    vx_uint8_ptr input2_ptr,
    vx_uint8 input2FixPointPos,
    vx_enum input2Format,
    vx_int32 size,
    vx_float32 scale,
    vx_enum overflow,
    vx_enum scaleRounding,
    vx_enum operation,
    vx_uint8_ptr output_ptr,
    vx_uint8 outputFixPointPos,
    vx_enum outputRoundingMode,
    vx_enum outputFormat);
void vx_nn_rpn_qsort_box(vx_float32_ptr box, vx_int32 start, vx_int32 end, vx_int32 num_top);
int vx_nn_rpn_transform_box(
    vx_float32_ptr box,
    vx_float32 dx, vx_float32 dy,
    vx_float32 d_log_w, vx_float32 d_log_h,
    vx_float32 img_W, vx_float32 img_H,
    vx_float32 min_box_W, vx_float32 min_box_H
    );
vx_float32 vx_nn_rpn_iou_cpu(vx_float32_ptr A, vx_float32_ptr B);
void vx_nn_rpn_nms_cpu(
    vx_uint32 num_boxes,
    vx_float32_ptr boxes,
    vx_uint32_ptr index_out,
    vx_uint32_ptr num_out,
    vx_int32 base_index,
    vx_float32 nms_thresh,
    vx_uint32 max_num_out
    );
vx_status vxnnePoolingCpu(
    vx_uint8_ptr src,
    vx_uint8 srcFixPointPos,
    vx_int32 type,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride,
    vx_int32 kernel_size,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_uint8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat);
vx_status vxnnePoolingAvg(
    vx_uint8_ptr src,
    vx_uint8 srcFixPointPos,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride,
    vx_int32 kernel_size,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_uint8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat
    );
vx_status vxnnePoolingMax(
    vx_uint8_ptr src,
    vx_uint8 srcFixPointPos,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride,
    vx_int32 kernel_size,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_uint8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat
    );

extern vx_status vxnneLayer_Free(struct _vxnne_layer_s* layer);
extern vx_status vxnneLayer_Execute(vxnne_layer layer);


#endif


