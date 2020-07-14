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


#ifndef _GC_VX_NN_ENCODER_H_
#define _GC_VX_NN_ENCODER_H_

void vxoWeightsBiases_Destroy(
    vx_weights_biases_parameter wb
    );
vx_weights_biases_parameter _createWeightsBiasesParameterFromTensors(
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
vx_weights_biases_parameter _createWeightsBiasesParameterFromParams(
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
VX_INTERNAL_CALLBACK_API void vxoWeightsBiasesBase_Destructor(vx_reference ref);
VX_INTERNAL_CALLBACK_API void vxoWeightsBiases_Destructor(vx_reference ref);
vx_uint32 calcFit1xN(vx_context context, vx_uint32 kernelZ, vx_uint32 inputX, vx_uint32 inputY);
vx_bool calcFitZdp3N(vx_context context,vx_uint32 inputX, vx_uint32 inputY, vx_uint32* fitN, vx_uint32 stride, vx_uint32 poolingSize);
vx_status replaceKernelBufferZOffset(
    vx_weights_biases_parameter wb,
    vx_uint8_ptr wb_base_ptr,
    vx_int32 z_offset
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

#endif

