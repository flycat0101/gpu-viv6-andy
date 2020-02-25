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


/***********************************************************************************
* Copyright:    Verisilicon
* FileName:     archModelInterface.h
* Author:       JinboHuang
* Data:         2019-05-27
* Version:      0.5.00
* Description:  Head file of Interface for driver calling Arch Model to support the original struction of the driver.
*               provide several API to transfer the parameters from driver for Standard Arch Model Lib
*
*/

#ifndef _ARCH_MODEL_INTERFACE_H_
#define _ARCH_MODEL_INTERFACE_H_

#include "archSwPerf.h"

/* main funcrion called by driver for predict */
ARCH_INTERNAL_API arch_status archGraphPredictPerf(vx_graph graph);

//arch_status calculateArchPerf(APMHandle apm, arch_nn_config *pArchNnConfig,arch_drv_option *pArchOptions, arch_perf perf, archnne_operation_target_e op_target, archnne_operator_e op_type);
vx_status showDriverPerformance(vx_context context,vxnne_layer layer,vxnne_operation op,arch_perf perf);
ARCH_INTERNAL_API void archCalculateArchPerfFromWB(vx_context context,vxnne_operation operation, arch_perf perf,vx_weights_biases_parameter wb,arch_uint32 orig_input_dims[],
                                    arch_uint32 output_dims[],vx_enum output_format, arch_uint32 pad_x_left, arch_uint32 pad_x_right, arch_uint32 pad_y_top,
                                    arch_uint32 pad_y_bottom, arch_uint32 pool_size, arch_uint32 pool_stride, arch_int32* offsets, arch_int32 flush,
                                    arch_uint8 src_buf,arch_uint8 dst_buf, arch_uint8 kernel_buf,arch_int32 cached_space,vxnne_operation_target_e op_target, vxnne_operator_e op_type);

ARCH_INTERNAL_API void archCalculateArchPerfFromTiling(vx_context context,vxnne_layer layer,arch_perf perf,vxnne_tensor_info input_tiling,
                                    vxnne_tensor_info output_tiling,vx_tensor input,vx_tensor output,vx_weights_biases_parameter wb,vxnne_operation_command  op_command,
                                    vxnne_operation_target_e op_target,vxnne_operator_e op_type);

#endif /* _GC_NN_ARCH_INTERFACE_H_ */
