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


#ifndef _GC_VX_NN_UTIL_H_
#define _GC_VX_NN_UTIL_H_

#include <gc_vx_nn_encoder.h>

#define F16_EXPONENT_BITS 0x1F
#define F16_EXPONENT_BIAS 15

#define F16_EXPONENT_SHIFT 10
#define F16_MANTISSA_BITS ((1 << F16_EXPONENT_SHIFT) - 1)
#define F16_MANTISSA_SHIFT (23 - F16_EXPONENT_SHIFT)
#define F16_MAX_EXPONENT (F16_EXPONENT_BITS << F16_EXPONENT_SHIFT)

#define F21_EXPONENT_SHIFT 15
#define F21_MANTISSA_BITS ((1 << F21_EXPONENT_SHIFT) - 1)
#define F21_MANTISSA_SHIFT (23 - F21_EXPONENT_SHIFT)
#define F21_MAX_EXPONENT (F16_EXPONENT_BITS << F21_EXPONENT_SHIFT)

#define SE8M12_EXPONENT_SHIFT 12
#define SE8M12_MANTISSA_BITS ((1 << SE8M12_EXPONENT_SHIFT) - 1)
#define SE8M12_MANTISSA_SHIFT (23 - SE8M12_EXPONENT_SHIFT)
#define SE8M12_MAX_EXPONENT (SE8M12_MANTISSA_BITS << SE8M12_EXPONENT_SHIFT)

#define BF16_EXPONENT_SHIFT 7
#define BF16_MANTISSA_BITS ((1 << BF16_EXPONENT_SHIFT) - 1)
#define BF16_MANTISSA_SHIFT (23 - BF16_EXPONENT_SHIFT)
#define BF16_MAX_EXPONENT (BF16_MANTISSA_BITS << BF16_EXPONENT_SHIFT)

#define TP_LUT_PARAM_SIZE 2
#define TP_LUT_BUFF_SIZE (1024 + TP_LUT_PARAM_SIZE)

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

typedef struct _vx_arch_model_bw_cost_s
{
    vx_float64 cost;
    vx_float64 tile0;
    vx_float64 tile0VZGroup0;
    vx_float64 vzGroup0;
    vx_float64 residual; /*for write bw*/
}
vx_arch_model_bw_cost_s;

typedef struct _vx_arch_model_cycle_cost_s
{
    vx_float64 cost;
    vx_float64 tile0VZGroup0;
    vx_float64 tile0ResetVZGroup;
    vx_float64 resetTileVZGroup0;
    vx_float64 resetTileResetVZGroup;
}
vx_arch_model_cycle_cost_s;

typedef struct _vx_arch_model_cost_s
{
    vx_arch_model_bw_cost_s readBW;
    vx_arch_model_bw_cost_s writeBW;
    vx_arch_model_cycle_cost_s readCycle;
    vx_arch_model_cycle_cost_s writeCycle;
} vx_arch_model_cost_s;

typedef enum _vx_arch_model_nn_cost_type
{
    ARCH_MODEL_DDR_COST = 0,
    ARCH_MODEL_VIP_SRAM_COST,
    ARCH_MODEL_AXI_SRAM_COST,
    ARCH_MODEL_AXI_BUS_COST,
    ARCH_MODEL_DDR_KERNEL_COST,
    ARCH_MODEL_DDR_IN_IMAGE_COST,
}
vx_arch_model_nn_cost_type;

#define NUMBER_OF_NN_COST_TYPE 6



void calculateSplitSize(vx_uint32 whole_size, vx_uint32 split_num, vx_uint32* split_size_array, vx_uint32* split_offset_array);
vx_uint8 Fp32toUint8(vx_float32 val, vx_int32 zeroPoint, vx_float32 scale, vx_int32 roundMode);
vx_float32 Uint8toFp32(vx_uint8 val, vx_int32 zeroPoint, vx_float32 scale);
vx_float32 Fp16toFp32(const vx_uint16 in);
vx_int16 Fp32toFp16(vx_float32 val);
vx_int8 Fp32toInt8_fc(vx_float32 val);
vx_int8 Fp32toInt8(vx_float32 val, vx_int8 fixedPointPos, vx_int32 roundMode);
vx_int16 Fp32toInt16(vx_float32 val, vx_int8 fixedPointPos, vx_int32 roundMode);
vx_float32 Int8toFp32(vx_int8 val, vx_int8 fixedPointPos);
vx_float32 Int32toFp32(vx_int32 val, vx_int8 fixedPointPos);
vx_float32 Int64toFp32(vx_int64 val, vx_int8 fixedPointPos);
void getFP32M0AndN(vx_float32 mult, vx_uint16 *M0, vx_int8 *N);
void calculateActivationRangeFloat16(vx_int32 activation, vx_int16* act_min, vx_int16* act_max);
void calculateActivationRangeInt16(vx_int32 activation, vx_int8 fixedPointPos, vx_int16* act_min, vx_int16* act_max, vx_int32 roundMode);
void calculateActivationRangeUInt8(vx_int32 activation, vx_float32 scale, vx_int32 zero_point, vx_uint8* act_min, vx_uint8* act_max, vx_float32 range_min, vx_float32 range_max);
void calculateActivationRangeInt8(vx_int32 activation, vx_int8 fixedPointPos, vx_int8* act_min, vx_int8* act_max, vx_int32 roundMode);
vx_bool getFC_1x1xN_to_NN_kxxkyxkz(vx_uint32 input_size, vx_uint32 *kx, vx_uint32 *ky, vx_uint32 *kz);
vx_status checkGetDataFactor(vx_uint32 data, vx_uint32 *factor, vx_uint32 minLimit, vx_uint32 maxLimit, vx_uint32 alignData);
vx_bool checkOutputTensorDoAlu(vx_tensor src, vx_tensor dst);
#if defined(__linux__)
vx_float64 _copysign(vx_float64 number, vx_float64 sign);
#endif
void vxnneBatch_SetCurrentBatchArray(struct _vxnne_operation_s *operation, vx_uint32 batchIndex);
void vxnneMultiChannel_SetCurrentChannel(vxnne_operation_target_e operationTarget);
void vxnneMultiChannel_GetCurrentChannel(vxnne_operation_target_e *operationTarget);
void vxnneMultiChannel_ApplySyncMode(vxnne_sync_mode_e mode, gctUINT32 semaHandle);
vx_float32 roundSimpleRounding(vx_float32 x);
vx_float64 roundRTNE(vx_float64 x);
vx_float32 roundRTZ(vx_float32 x);
vx_float32 roundRTNI(vx_float32 x);
vx_float32 vxnneRound(vx_float32 x, vx_enum roundMode);
vx_uint32 vxnneAlignWithStride(vx_uint32 inputSize, vx_uint32 stride);

vx_int32 vxnneGetTypeSize(vx_type_e format);

vx_uint32 vxnneGetOneNumber(vx_uint64 value);
vx_float32 vxnneGetData(vx_type_e format, vx_int32 index, vx_uint8_ptr data, vx_int8 fixPointPos);
vx_status vxnneSaveData(vx_type_e format, vx_int32 index, vx_float64 data, vx_ptr dst_data, vx_int8 fixedPointPos, vx_enum roundMode);
vx_float32 vxnneGetDataQuant(vx_type_e format, vx_int32 index, vx_uint8_ptr data, vx_int32 zeroPoint, vx_float32 scale);
vx_status vxnneSaveDataQuant(vx_type_e format, vx_int32 index, vx_float64 data, vx_ptr dst_data, vx_int32 zeroPoint, vx_float32 scale, vx_enum roundMode);
vx_float32 vxnneGetDataExt(vx_type_e format, vx_enum quant_format, vx_int32 index, vx_uint8_ptr data, vx_int8 fixPointPos, vx_int32 zeroPoint, vx_float32 scale);
vx_status vxnneSaveDataExt(vx_type_e format, vx_enum quant_format, vx_int32 index, vx_float64 data, vx_ptr dst_data, vx_int8 fixedPointPos, vx_int32 zeroPoint, vx_float32 scale, vx_enum roundMode);
vx_int32 vxoNNExternsionConvlutionRound(vx_float32 in, vx_enum round_type);
void reshuffleData(vx_nn_reshuffle_s *src, vx_uint32 strideStepX, vx_uint32 strideStepY, vx_nn_reshuffle_s *dst);
void initUndefinedHardwareConfig(vx_global_data globalData);
vx_bool WeightBiasBufferAllocate(vx_context context, vx_weights_biases_parameter weight_bias, vx_size size);
void writeBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 data, vx_uint32 dataBits);
void replaceBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 data, vx_uint32 dataBits);
vx_uint32 readBits(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 dataBits);
void packZeros(uint32_t **buffer, vx_uint32 *bitOffset, vx_uint32 alignedOffset);
void _DataGeneralConvert(void* input_ptr, void* output_ptr, vx_uint32 input_size, vx_uint32 output_size);


vx_int32 getUserIDFromOutputTensor(
    vx_tensor tensor);

vx_int32 getHwPoolingType(vx_enum poolingType);
vx_int32 getHWRoundingMode(vx_nn_round_mode_e roundingMode, vx_enum dataFormat, vx_bool isTP);
vx_int32 getHWBorderMode(vx_enum padMode, gceVX_ACCELERATOR_TYPE accelerator);
vx_float32 vxnneConvertDynamicFixPointValueToFloat32(
    vx_float32 value,
    vx_int8 fixedPointPos
    );

VX_INTERNAL_API vx_status vxnneCommandBuffer_GenerateCommands(
    vx_context                   context,
    vx_node                      node,
    vxnne_operation_command      operation_command,
    vxnne_tensor_info            input,
    vxnne_tensor_info            output,
    vxnne_operation_target_e     target,
    vx_op_param                  parameter,
    vxnne_command_buffer         command_buffer
    );

VX_INTERNAL_API vx_status vxnneModifyNNLastNoflushBit(
    vx_context                   context,
    vxnne_command_buffer         command_buffer,
    vxnne_operation              operation,
    vx_uint8                     value
    );

VX_INTERNAL_API vx_status vxnneModifyTPLastNoflushBit(
    vx_context                   context,
    vxnne_command_buffer         command_buffer,
    vxnne_operation              operation,
    vx_uint8                     value
    );

#if gcdDUMP
void dumpNNCommandInfo(vx_uint32 sliceIndex, vx_uint32 sliceNum, vx_nn_cmd_info_u *info, vxnne_operation_command operationCommand);
#endif

void vxoNNExternsionDoReshuffle(
    vx_uint32 batchIndex,
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum   padMode,
    void*     padConst,
    vx_uint32 strideX,
    vx_uint32 strideY,
    vx_uint32 kx,
    vx_uint32 ky);
vx_char* vxnneGetOperatorTypeName(vxnne_operator_e operationType);
vx_char* vxnneGetOperatorTargetName(vxnne_operation_target_e operationTarget);
vx_char* vxnneGetCacheModeName(vx_enum cacheMode);
void _TransposeTensor(vx_uint8_ptr in_addr, vx_uint8_ptr out_addr, vx_uint32 data_size, vx_uint32_ptr dims, vx_uint32_ptr strides, vx_uint32_ptr trans_strides, vx_uint32* perm, vx_uint32 layer);
vx_float64 vxnneGetData64(vx_enum format, vx_int32 index, vx_uint8_ptr src, vx_int8 fixPointPos);
vx_float64 vxnneSaturate(vx_float64 data, vx_enum format);
vx_float64 vxnneWarp(vx_float64 data, vx_enum format);
vx_status eltwise(
    vx_tensor input1,
    vx_tensor input2,
    vx_float32 scale,
    vx_enum overflow,
    vx_enum scaleRounding,
    vx_enum operation,
    vx_tensor output);
void vx_nn_rpn_qsort_box(vx_float32_ptr box, vx_int32 start, vx_int32 end, vx_int32 num_top);
void vx_nn_rpn_qsort_box_fp16(vx_int16_ptr box, vx_int32 start, vx_int32 end, vx_int32 num_top);
int vx_nn_rpn_transform_box(
    vx_float32_ptr box,
    vx_float32 dx, vx_float32 dy,
    vx_float32 d_log_w, vx_float32 d_log_h,
    vx_float32 img_W, vx_float32 img_H,
    vx_float32 min_box_W, vx_float32 min_box_H
    );
vx_float32 vx_nn_rpn_iou_cpu(vx_float32_ptr A, vx_float32_ptr B);
vx_float32 vx_nn_rpn_iou_cpu_f16(vx_int16_ptr a, vx_int16_ptr b);
void vx_nn_rpn_nms_cpu(
    vx_uint32 num_boxes,
    vx_float32_ptr boxes,
    vx_uint32_ptr index_out,
    vx_uint32_ptr num_out,
    vx_int32 base_index,
    vx_float32 nms_thresh,
    vx_uint32 max_num_out
    );
void vx_nn_rpn_nms_cpu_f16(
    vx_uint32 num_boxes,
    vx_int16_ptr boxes,
    vx_uint32_ptr index_out,
    vx_uint32_ptr num_out,
    vx_int32 base_index,
    vx_float32 nms_thresh,
    vx_uint32 max_num_out
    );
vx_status vxnnePoolingCpu(
    vx_uint8_ptr src,
    vx_int8 srcFixPointPos,
    vx_int32 type,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 batch,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride_x,
    vx_int32 stride_y,
    vx_int32 kernel_size_x,
    vx_int32 kernel_size_y,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_int8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat,
    vx_type_e srcQuantFormat,
    vx_type_e dstQuantFormat,
    vx_int32 inputZP,
    vx_float32 inputScale,
    vx_int32 outputZP,
    vx_float32 outputScale);

vx_status vxnnePoolingAvg(
    vx_uint8_ptr src,
    vx_int32 type,
    vx_int8 srcFixPointPos,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 batch,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride_x,
    vx_int32 stride_y,
    vx_int32 kernel_size_x,
    vx_int32 kernel_size_y,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_int8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat,
    vx_type_e srcQuantFormat,
    vx_type_e dstQuantFormat,
    vx_int32 inputZP,
    vx_float32 inputScale,
    vx_int32 outputZP,
    vx_float32 outputScale);

vx_status vxnnePoolingMax(
    vx_uint8_ptr src,
    vx_int8 srcFixPointPos,
    vx_type_e srcFormat,
    vx_int32 input_width,
    vx_int32 input_height,
    vx_int32 batch,
    vx_int32 depth,
    vx_int32_ptr output_width,
    vx_int32_ptr output_height,
    vx_int32 stride_x,
    vx_int32 stride_y,
    vx_int32 kernel_size_x,
    vx_int32 kernel_size_y,
    vx_uint32 padXLeft,
    vx_uint32 padXRight,
    vx_uint32 padYTop,
    vx_uint32 padYBottom,
    vx_enum rounding,
    vx_uint8_ptr dst,
    vx_int8 dstFixPointPos,
    vx_enum dstRoundingMode,
    vx_type_e dstFormat,
    vx_type_e srcQuantFormat,
    vx_type_e dstQuantFormat,
    vx_int32 inputZP,
    vx_float32 inputScale,
    vx_int32 outputZP,
    vx_float32 outputScale);

vx_status vxnneLayer_Free(struct _vxnne_layer_s* layer);

vx_bool vxnneIsNNSupportFormat(
    vx_context context,
    vx_tensor inputTensor,
    vx_weights_biases_parameter wb,
    vx_tensor outputTensor);

vx_bool vxnneIsTPSupportFormat(
    vx_context context,
    vx_tensor inputTensor,
    vx_weights_biases_parameter wb,
    vx_tensor outputTensor);

void vxnneGetPatternBitAndVipSramSizeNeed(
    vx_float32 ratio,
    vx_uint32 kernelCacheSize,
    vx_uint32 kernelStreamSize,
    vx_uint32 dataUnitByte,
    vx_uint32 *oneNumPtr,
    vx_uint32 *zeroNumPtr,
    vx_uint64 *kernelPatternBitsPtr,
    vx_uint32 *vipSramNeedPtr);

void vxnneGetKernelPatternBits(
    vx_uint32 oneNum,
    vx_uint32 zeroNum,
    vx_uint64 *kernelPatternBitPtr);
/* this function has been declared in libarchmodel, remove here */
vx_uint8 MemPoolTypeToPerfType(
    vx_enum memPoolType);

void vxoWeightsBiases_Reshuffle(
    vx_weights_biases_parameter_base wb_base
    );

void vxoWeightsBiases_Clear(
    vx_weights_biases_parameter wb
    );

VX_INTERNAL_API vx_int32 vxoBinaryGraph_SearchPattern(
    gctUINT32_PTR buffer,
    gctUINT32 sizeInUint,
    gctUINT32 pattern,
    vx_uint32 *offset,
    vx_bool multiple
    );

vx_status vxo_insertHandle(vxnne_execution_layer   executionLayer);
vx_status vxo_updateSwapHandle(vx_graph graph);
vx_status vxoFlushTensorImage(vx_graph graph);
vx_bool _IsSameDataType(
    vx_tensor src,
    vx_tensor dst
    );

vx_bool _IsSameQuantType(
    vx_tensor src,
    vx_tensor dst
    );

vx_bool _IsSameType(
    vx_tensor src,
    vx_tensor dst
    );

vx_uint32 caculateInputTransposeBufferSize(
    vx_context context,
    vx_enum imageCacheMode,
    vx_uint32 outputTileXSize,
    vx_uint32 outputTileYSize,
    vx_uint32 kernelX,
    vx_uint32 kernelY,
    vx_uint32 kernelZ,
    vx_uint32 interleaveMode,
    vx_float32 ddrLatency,
    vx_uint32 transposeInChannel,
    vx_enum dataFormat,
    vx_uint32 nnStrideX,
    vx_uint32 nnStrideY
    );

vx_uint32 caculateOutTransposeBufferSize(
    vx_context context,
    vx_uint32 outputTileXSize,
    vx_uint32 outputTileYSize,
    vxnne_convolution_relu_pooling_operation convOperation,
    vx_enum format
    );

void alignTensorChannelToTransposeChannel(
    vx_tensor tensor,
    vx_uint32 transposeChannel
    );

vx_status vxnnePreLoadWeightsBiases(
    vx_context context,
    vx_graph   graph,
    vx_uint32  size
    );

vx_uint32  GetEsitimateWBSize(vx_weights_biases_parameter weightsBiases);
vx_bool estimateNNTransposeSize(vx_context context, vx_graph graph);
vx_status nnTransposeChannel(vx_context context, vx_graph graph);

#endif
vx_status patchNodeParamLocation(vx_node node);

