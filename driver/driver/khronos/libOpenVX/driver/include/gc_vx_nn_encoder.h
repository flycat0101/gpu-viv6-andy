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


#ifndef _GC_VX_NN_ENCODER_H_
#define _GC_VX_NN_ENCODER_H_

#include <gc_vx_nn_wb.h>

vx_uint32 calcFit1xN(
    vx_context context,
    vx_uint32 kernelZ,
    vx_uint32 inputX,
    vx_uint32 inputY
    );

vx_bool calcFitZdp3N(
    vx_context context,
    vx_uint32 inputX,
    vx_uint32 inputY,
    vx_uint32* fitN,
    vx_uint32 stride,
    vx_uint32 poolingSize
    );

vx_float32 calculateWeightNonZeroRatio(
    vx_context context,
    vx_uint32 skip_value,
    vx_tensor weights
    );

void calculateWeightBiasTPBufferRelatedSize(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 slice_count,
    vx_uint32 filter_count,
    vx_uint32 total_slice_count,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_uint32 skip_value,
    vx_int8 set_zrl,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* all_count,
    vx_uint32* non_zero_count,
    vx_size* orig_kernel_buf_size,
    vx_size* min_kernel_buf_size,
    vx_uint8* min_zero_run_len
    );

void calculateWeightBiasStreamRelatedSize(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 kernels_per_core,
    vx_enum weight_format,
    vx_uint32 *weight_zp,
    vx_enum bias_format,
    vx_uint32 input_zp,
    vx_uint32 skip_value,
    vx_uint32 z_offset,
    vx_int8  set_zrl,
    vx_uint8 option_zero_run_len,
    vx_bool  is_depth_wise,
    gctPOINTER weight_data,
    gctPOINTER bias_data,
    vx_uint32* all_count,
    vx_uint32* non_zero_count,
    vx_size* orig_kernel_buf_size,
    vx_size* min_kernel_buf_size,
    vx_uint8*  min_zero_run_len,
    vx_uint32* max_zero_run_len,
    vx_weights_biases_parameter  wb
    );

vx_uint32 calcKernelStreamSizeHuffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_int32 weight_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr
    );

vx_uint32 calcKernelSizeV8Huffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum   weight_format,
    vx_uint32 *weight_zp,
    vx_bool   is_depth_wise,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
     vx_weights_biases_parameter  wb
    );

vx_uint32 fillinTPKernelBuffer(
    vx_context context,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int8  bias_fpp,
    vx_uint32 skip_value,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr
    );

vx_uint32 fillinTPKernelBufferHuffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint8* zero_run_len_bit_width,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 total_weight_z,
    vx_enum  weight_format,
    vx_enum  bias_format,
    vx_int8  bias_fpp,
    vx_uint32 skip_value,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr
    );

vx_uint32 fillinKernelBuffer(
    vx_context context,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_int32 coef_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_weight_bias_z_offset z_offset_handle,
    vx_uint32* num_of_vz,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    );

vx_uint32 fillinKernelBufferHuffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_int32 weight_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    );

vx_uint32 fillinKernelBufferV8Huffman(
    vx_context context,
    vx_weight_bias_huffman_cfg huffman_config,
    vx_enum  input_format,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 slice_count,
    vx_uint32 z_count,
    vx_uint32 filters_per_core,
    vx_enum  weight_format,
    vx_enum  weight_quant_format,
    vx_int32 *weight_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_bool  is_depth_wise,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32* post_mul_shift,
    vx_uint32* neg_post_mul_shift,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    );

vx_uint32 fillinDepthWiseKernelBuffer(
    vx_context context,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_enum  weight_quant_format,
    vx_int32 coef_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_uint32* num_of_vz,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    );

vx_uint32 fillinKernelBufferBalance(
    vx_context context,
    vx_uint8 min_zero_run_len,
    vx_uint32 max_zero_run,
    vx_uint32 weight_x,
    vx_uint32 weight_y,
    vx_uint32 weight_z,
    vx_uint32 filter_count,
    vx_uint32 kernels_per_core,
    vx_enum  weight_format,
    vx_int32 coef_zp,
    vx_enum  bias_format,
    vx_int32 input_zp,
    vx_uint32 skip_value,
    vx_int32 z_offset,
    vx_uint8_ptr wb_base_ptr,
    vx_uint8_ptr weight_base_ptr,
    vx_uint32* bias_base_ptr,
    vx_weight_bias_z_offset z_offset_handle,
    vx_uint32* num_of_vz,
    vx_size* kernel_align_stream_size,
    vx_size* kernel_stream_full_cache_size,
    vx_size* kernel_max_stream_size_percore
    );

vx_status replaceKernelBufferZOffset(
    vx_weight_bias_z_offset z_offset_handle,
    vx_uint32 num_of_vz,
    vx_uint8_ptr wb_base_ptr,
    vx_int32 z_offset
    );

vx_status calculateZOffset(
    vx_uint32* pool_outputs_dims,
    vx_enum    output_format,
    vx_enum    weight_format,
    vx_uint32* z_offset
    );

vx_tensor reshuffleKernelTensor(
    vx_context   context,
    vx_tensor    weight,
    vx_uint32*   output_dims,
    vx_uint32    stride_x,
    vx_uint32    stride_y
    );

vx_bool isInt64BiasOverflow(
    vx_context context,
    vx_enum weight_format,
    vx_enum bias_format,
    vx_uint32 z_count,
    vx_tensor bias_tensor
    );
#endif

