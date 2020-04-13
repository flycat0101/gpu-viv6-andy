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


#ifndef _GC_VX_NN_WB_H_
#define _GC_VX_NN_WB_H_

#include <gc_hal.h>
#include <gc_hal_user.h>

#include <VX/vx.h>
#include <VX/vx_khr_cnn.h>

#include <gc_vx_layer.h>
#include "gc_vxk_common.h"

typedef struct _vx_weight_bias_z_offset_s
{
    vx_uint32 ptr_offset;
    vx_uint32 bit_offset;
}
vx_weight_bias_z_offset_s, *vx_weight_bias_z_offset;

#define LOG_RUN_SIZE1 4    /*The 2nd set of run-length can have 1<<LOG_RUN_SIZE1 of run-length*/
#define HUFFMAN_CODING_TYPE_NON_RUN_LEN 0
#define HUFFMAN_CODING_TYPE_RUN_LEN 1
#define MAX_RUNLEN_SIZE (2+(1<<LOG_RUN_SIZE1)) /*allow 18 run-length in maximum.*/

typedef struct _vx_weight_bias_huffman_cfg_s
{
    vx_uint8                                 pre_encode;         /*true means using prevEncode, prediction from previous pixel*/
    vx_uint8                                 coding_type;        /*false means no zero run length mode*/
    vx_uint8                                 bit16_flag;
    vx_uint8                                 fp16_flag;

    vx_uint8                                 version;            /*Version Number: Version.SubVersion, Now it is 1.0*/
    vx_uint8                                 run_len_table_size;
    vx_uint8                                 run_len_table[MAX_RUNLEN_SIZE];
    vx_uint8                                 map_to_huffman[4];  /*8 mapping to Huffman Symbols, each bytes take 2 mapping*/
    vx_int16                                 avg_bias;           /*The bias for*/
    vx_uint8                                 reserved;
    vx_uint8                                 shift_back;
    vx_uint32                                cmp_bit_length;     /*number of Symbols in the stream*/
}
vx_weight_bias_huffman_cfg_s, *vx_weight_bias_huffman_cfg;

typedef struct _vx_weight_bias_general_param_s
{
    vx_uint32                                num_of_dims;
    vx_uint32                                dims_sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_uint32                                org_dims_sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];
    vx_enum                                  data_format;
    vx_int8                                  fixed_point_pos;
    vx_enum                                  quant_format;
    vx_float32                               quant_scale;
    vx_int32                                 quant_zero_point;
}
vx_weight_bias_general_param_s, *vx_weight_bias_general_param;

typedef struct _vx_weight_bias_external_param_s
{
    vx_weight_bias_general_param_s           weight_param;
    vx_weight_bias_general_param_s           bias_param;
    vx_weight_bias_general_param_s           alpha_param;

    vx_uint32                                stride_x;
    vx_uint32                                stride_y;

    vx_uint32                                orig_stride_x;
    vx_uint32                                orig_stride_y;

    vx_int32                                 input_zp;

    vx_bool                                  is_depth_wise;
    vx_int8                                  set_zero_length;
    vx_uint32                                skip_value;

    vx_bool                                  do_1xN_config;
    vx_enum                                  orig_layer_type;
}
vx_weight_bias_external_param_s, *vx_weight_bias_external_param;

typedef struct _vx_weight_bias_slice_item_s
{
    vx_uint32                                kz_count;
    vx_uint32                                z_count;

    vx_size                                  memory_offset;
    vx_size                                  memory_size;

    vx_size                                  kernel_align_stream_size;
    vx_size                                  kernel_stream_full_cache_size;

    vx_size                                  kernel_max_stream_size_percore;

    vx_weight_bias_z_offset                  z_offset_handle; /* only use in NN compress */
    vx_uint32                                num_of_vz;       /* only use in NN compress */

    vx_weight_bias_huffman_cfg               huffman_config;
}
vx_weight_bias_slice_item_s, *vx_weight_bias_slice_item;

typedef struct _vx_weight_bias_compress_param_s
{
    vx_enum                                  target;
    vx_uint8                                 step;

    vx_memory_s                              whole_memory;
    vx_size                                  whole_memory_size;

    vx_uint32                                total_slice_num;
    vx_uint32                                kz_slice_num;
    vx_uint32                                z_slice_num;
    vx_weight_bias_slice_item                slice_array;     /*TP FC may split z or kz, NN may split z */

    vx_float64                               whole_non_zero_ratio;
    vx_float64                               whole_general_compression_ratio;

    /* for record */
    vx_uint32                                kernel_per_core;
    vx_uint32                                z_offset;
}
vx_weight_bias_compress_param_s, *vx_weight_bias_compress_param;

typedef vx_status (*vx_weight_bias_initialize_f)(vx_weights_biases_parameter wb, vx_weight_bias_general_param weight_param, vx_weight_bias_general_param bias_param, vx_uint32 orig_stride_x, vx_uint32 orig_stride_y, vx_uint32 stride_x, vx_uint32 stride_y, vx_int8 zero_len, vx_uint32 skip_value, vx_bool is_depth_wise, vx_bool do_1xN_config, vx_enum orig_layer_type);

typedef vx_status (*vx_weight_bias_deinitialize_f)(vx_weights_biases_parameter wb);

typedef vx_status (*vx_weight_bias_compress_f)(vx_weights_biases_parameter wb, vx_enum target, vx_uint32 kernel_per_core, vx_uint32 z_offset);
typedef vx_status (*vx_weight_bias_set_1_tensor_f)(vx_weights_biases_parameter wb, vx_tensor tensor);
typedef vx_status (*vx_weight_bias_set_2_tensor_f)(vx_weights_biases_parameter wb, vx_tensor tensor1, vx_tensor tensor2);
typedef vx_status (*vx_weight_bias_set_struct_ptr_f)(vx_weights_biases_parameter wb, vx_ptr struct_ptr, vx_uint32 struct_size);

typedef struct _vx_weights_biases_parameter_s
{
    vx_reference_s                           base;

    vx_weight_bias_external_param_s          external_param;
    vx_weight_bias_compress_param_s          compress_param;

    vx_tensor                                weight_tensor;
    vx_tensor                                bias_tensor;
    vx_tensor                                alpha_tensor;

    vx_uint8_ptr                             weight_data_ptr;
    vx_uint32_ptr                            bias_data_ptr;
    vx_uint8_ptr                             alpha_data_ptr;

    /* private function */
    vx_weight_bias_initialize_f              initialize;
    vx_weight_bias_deinitialize_f            deinitialize;

    /* public function */
    vx_weight_bias_compress_f                calc_compress_ratio;
    vx_weight_bias_compress_f                compress;
    vx_weight_bias_set_2_tensor_f            set_weight_bias_tensor;
    vx_weight_bias_set_1_tensor_f            set_alpha_tensor;
    vx_weight_bias_set_struct_ptr_f          set_optimization; /* vx_weights_biases_parameter_optimizations_ext2_t */
}
vx_weights_biases_parameter_s;


#define WB_EXTERNAL_PARAM(wb)                  (wb)->external_param
#define WB_IS_DEPTH_WISE(wb)                   (wb)->external_param.is_depth_wise
#define WB_DO_1XN_CONFIG(wb)                   (wb)->external_param.do_1xN_config
#define WB_ORG_LAYER_TYPE(wb)                  (wb)->external_param.orig_layer_type
#define WB_SKIP_VALUE(wb)                      (wb)->external_param.skip_value
#define WB_SET_ZERO_LENGTH(wb)                 (wb)->external_param.set_zero_length
#define WB_INPUT_ZP(wb)                        (wb)->external_param.input_zp

#define WB_STRIDE_X(wb)                        (wb)->external_param.stride_x
#define WB_STRIDE_Y(wb)                        (wb)->external_param.stride_y
#define WB_ORG_STRIDE_X(wb)                    (wb)->external_param.orig_stride_x
#define WB_ORG_STRIDE_Y(wb)                    (wb)->external_param.orig_stride_y

#define WB_WEIGHT_PARAM(wb)                    (wb)->external_param.weight_param
#define WB_WEIGHT_DIMS_NUM(wb)                 (wb)->external_param.weight_param.num_of_dims
#define WB_WEIGHT_DIMS_SIZES(wb)               (wb)->external_param.weight_param.dims_sizes
#define WB_WEIGHT_ORG_DIMS_SIZES(wb)           (wb)->external_param.weight_param.org_dims_sizes
#define WB_WEIGHT_DATA_FORMAT(wb)              (wb)->external_param.weight_param.data_format
#define WB_WEIGHT_FPP(wb)                      (wb)->external_param.weight_param.fixed_point_pos
#define WB_WEIGHT_QUANT_FORMAT(wb)             (wb)->external_param.weight_param.quant_format
#define WB_WEIGHT_SCALE(wb)                    (wb)->external_param.weight_param.quant_scale
#define WB_WEIGHT_ZP(wb)                       (wb)->external_param.weight_param.quant_zero_point

#define WB_BIAS_PARAM(wb)                      (wb)->external_param.bias_param
#define WB_BIAS_DIMS_NUM(wb)                   (wb)->external_param.bias_param.num_of_dims
#define WB_BIAS_DIMS_SIZES(wb)                 (wb)->external_param.bias_param.dims_sizes
#define WB_BIAS_DATA_FORMAT(wb)                (wb)->external_param.bias_param.data_format
#define WB_BIAS_FPP(wb)                        (wb)->external_param.bias_param.fixed_point_pos
#define WB_BIAS_QUANT_FORMAT(wb)               (wb)->external_param.bias_param.quant_format
#define WB_BIAS_SCALE(wb)                      (wb)->external_param.bias_param.quant_scale
#define WB_BIAS_ZP(wb)                         (wb)->external_param.bias_param.quant_zero_point

#define WB_ALPHA_DIMS_NUM(wb)                  (wb)->external_param.alpha_param.num_of_dims
#define WB_ALPHA_DIMS_SIZES(wb)                (wb)->external_param.alpha_param.dims_sizes
#define WB_ALPHA_DATA_FORMAT(wb)               (wb)->external_param.alpha_param.data_format
#define WB_ALPHA_FPP(wb)                       (wb)->external_param.alpha_param.fixed_point_pos
#define WB_ALPHA_QUANT_FORMAT(wb)              (wb)->external_param.alpha_param.quant_format
#define WB_ALPHA_SCALE(wb)                     (wb)->external_param.alpha_param.quant_scale
#define WB_ALPHA_ZP(wb)                        (wb)->external_param.alpha_param.quant_zero_point

#define WB_KERNEL_X(wb)                        (wb)->external_param.weight_param.dims_sizes[0]
#define WB_KERNEL_Y(wb)                        (wb)->external_param.weight_param.dims_sizes[1]
#define WB_KERNEL_Z(wb)                        (wb)->external_param.weight_param.dims_sizes[2]
#define WB_OUTPUT_Z(wb)                        (wb)->external_param.weight_param.dims_sizes[3]
#define WB_ORG_KERNEL_X(wb)                    (wb)->external_param.weight_param.org_dims_sizes[0]
#define WB_ORG_KERNEL_Y(wb)                    (wb)->external_param.weight_param.org_dims_sizes[1]
#define WB_ORG_KERNEL_Z(wb)                    (wb)->external_param.weight_param.org_dims_sizes[2]
#define WB_ORG_OUTPUT_Z(wb)                    (wb)->external_param.weight_param.org_dims_sizes[3]

#define WB_MEMORY_SIZE(wb)                     (wb)->compress_param.whole_memory_size
#define WB_TOTAL_SLICE_NUM(wb)                 (wb)->compress_param.total_slice_num
#define WB_KERNEL_Z_SLICE_NUM(wb)              (wb)->compress_param.kz_slice_num
#define WB_OUTPUT_Z_SLICE_NUM(wb)              (wb)->compress_param.z_slice_num

#define WB_KERNEL_Z_INDEX(wb, index)           (wb)->compress_param.slice_array[index].kz_count
#define WB_OUTPUT_Z_INDEX(wb, index)           (wb)->compress_param.slice_array[index].z_count

#define WB_MEM_LOGICAL_BASE_ADDR(wb)           (wb)->compress_param.whole_memory.logicals[0]
#define WB_MEM_PHYSICAL_BASE_ADDR(wb)          (wb)->compress_param.whole_memory.physicals[0]

#define WB_MEM_LOGICAL_ADDR_INDEX(wb, index)   ((wb)->compress_param.whole_memory.logicals[0] + (wb)->compress_param.slice_array[index].memory_offset)
#define WB_MEM_PHYSICAL_ADDR_INDEX(wb, index)  ((wb)->compress_param.whole_memory.physicals[0] + (wb)->compress_param.slice_array[index].memory_offset)

#define WB_MEM_OFFSET_INDEX(wb, index)              (wb)->compress_param.slice_array[index].memory_offset
#define WB_MEM_SIZE_INDEX(wb, index)                (wb)->compress_param.slice_array[index].memory_size
#define WB_STREAM_ALIGN_SIZE_INDEX(wb, index)       (wb)->compress_param.slice_array[index].kernel_align_stream_size
#define WB_STREAM_FULL_CACHE_SIZE_INDEX(wb, index)  (wb)->compress_param.slice_array[index].kernel_stream_full_cache_size
#define WB_STREAM_MAX_SIZE_PERCORE_INDEX(wb, index) (wb)->compress_param.slice_array[index].kernel_max_stream_size_percore

#define WB_CONV_STREAM_ALIGN_SIZE(wb)               WB_STREAM_ALIGN_SIZE_INDEX(wb, 0)

#define WB_NUM_OF_VZ_INDEX(wb, index)          (wb)->compress_param.slice_array[index].num_of_vz
#define WB_ZOFFSET_HANDLE_INDEX(wb, index)     (wb)->compress_param.slice_array[index].z_offset_handle
#define WB_HUFFMAN_CONFIG_INDEX(wb, index)     (wb)->compress_param.slice_array[index].huffman_config

#define WB_NON_ZERO_RATIO(wb)                  (wb)->compress_param.whole_non_zero_ratio
#define WB_COMPRESS_RATIO(wb)                  (wb)->compress_param.whole_general_compression_ratio

#define WB_COMPRESS_TARGET(wb)                 (wb)->compress_param.target
#define WB_COMPRESS_STEP(wb)                   (wb)->compress_param.step
#define WB_KERNEL_PER_CORE(wb)                 (wb)->compress_param.kernel_per_core
#define WB_Z_OFFSET(wb)                        (wb)->compress_param.z_offset

#define WB_IS_TP_COMPRESS(wb)                  (WB_COMPRESS_TARGET(wb) == VXNNE_OPERATION_TARGET_TP ? vx_true_e : vx_false_e)

#define WB_IS_CALCULATED(wb)                   (WB_COMPRESS_STEP(wb) >= 1 ? vx_true_e : vx_false_e)
#define WB_IS_COMPRESSED(wb)                   (WB_COMPRESS_STEP(wb) == 2 ? vx_true_e : vx_false_e)

#define RESET_WB_COMPRESS_FLAG(wb)             WB_COMPRESS_STEP(wb) = 0
#define SET_WB_CALCULATED_FLAG(wb)             WB_COMPRESS_STEP(wb) = 1
#define SET_WB_COMPRESS_FLAG(wb)               WB_COMPRESS_STEP(wb) = 2

#define WB_MEMORY(wb)                          (wb)->compress_param.whole_memory
#define WB_MEMORY_NODE(wb)                     (wb)->compress_param.whole_memory.nodePtrs[0]
#define WB_MEMORY_LOCK(wb)                     (wb)->compress_param.whole_memory.writeLocks[0]
#define WB_SLICE_ARRAY(wb)                     (wb)->compress_param.slice_array

#define WB_WEIGHT_TENSOR(wb)                   (wb)->weight_tensor
#define WB_BIAS_TENSOR(wb)                     (wb)->bias_tensor
#define WB_ALPHA_TENSOR(wb)                    (wb)->alpha_tensor

#define WB_WEIGHT_DATA(wb)                     (wb)->weight_data_ptr
#define WB_BIAS_DATA(wb)                       (wb)->bias_data_ptr
#define WB_ALPHA_DATA(wb)                      (wb)->alpha_data_ptr


vx_weights_biases_parameter vxoCreateWeightsBiasesParameterFromTensors(
    vx_context  context,
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32   num_of_input_dims,
    vx_uint32   num_of_output_dims,
    vx_uint32   pad_x_left,
    vx_uint32   pad_x_right,
    vx_uint32   pad_y_top,
    vx_uint32   pad_y_bottom,
    vx_uint32   pooling_size_x,
    vx_uint32   pooling_size_y,
    vx_uint32   stride_x,
    vx_uint32   stride_y,
    vx_enum     down_scale_size_rounding,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_enum     output_format,
    vx_enum     convert_format,
    vx_enum     rank_mode,
    vx_tensor   weights,
    vx_tensor   biases,
    vx_tensor   alpha,
    vx_bool     do_prelu,
    vx_bool     do_1xN
    );

vx_weights_biases_parameter vxoCreateWeightsBiasesParameterFromParams(
    vx_context  context,
    vx_enum     layer_type,
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
    vx_uint32   weights_num_of_dims,
    vx_uint32 * weights_base_dims,
    vx_uint32 * weights_dims,
    vx_enum     weights_data_format,
    vx_enum     weights_quant_format,
    vx_int8     weights_fixed_point_pos,
    vx_uint32   biases_num_of_dims,
    vx_uint32 * biases_base_dims,
    vx_uint32 * biases_dims,
    vx_enum     biases_data_format,
    vx_enum     biases_quant_format,
    vx_int8     biases_fixed_point_pos
    );

vx_weights_biases_parameter vxoCreateWeightsBiasesParameterFromTensorsPRelu(
    vx_enum     layer_type,
    vx_uint32 * inputs_dims,
    vx_uint32 * convolution_outputs_dims,
    vx_uint32 * pool_outputs_dims,
    const vx_nn_convolution_relu_pooling_params convolution_relu_pooling_params,
    vx_size size_of_convolution_relu_pooling_params,
    vx_weights_biases_parameter_optimizations_t *optimizations,
    vx_size size_of_optimizations,
    vx_tensor   weights,
    vx_tensor   biases,
    vx_tensor   alpha
    );

vx_weights_biases_parameter vxoCreateWeightsBiasesParameterFromWeightBias(
    vx_context                  context,
    vx_weights_biases_parameter old_wb,
    vx_uint32*                  weight_dims,
    vx_uint32                   weight_num_of_dims
    );

vx_status vxoReleaseWeightsBiases(
    vx_weights_biases_parameter*  wb_ptr
    );

void vxoWeightBias_Destructor(
    vx_reference ref
    );

vx_status vxoCalculateNNCompressionFirstTime(
    vx_context                   context,
    vx_weights_biases_parameter  wb,
    vx_tensor                    output
    );
#endif

