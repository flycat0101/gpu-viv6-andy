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


#ifndef _GC_VX_LSTM_H_
#define _GC_VX_LSTM_H_

#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>
#define VX_GET_DATA_FROM_TENSOR(tensor, index) \
    vxnneGetDataExt((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index, TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor))

#define VX_SAVE_DATA_TO_TENSOR(tensor, data, index) \
    vxnneSaveDataExt((vx_type_e)TENSOR_DATA_TYPE(tensor), TENSOR_QUANT_TYPE(tensor), index, data, TENSOR_LOGICAL_ADDR(tensor), TENSOR_POS(tensor), TENSOR_TF_ZEROPOINT(tensor), TENSOR_TF_SCALE(tensor), TENSOR_ROUNDING_MODE(tensor))

#define CHECK_LIFETIME_IS_STATIC(tensor) \
    (((tensor != VX_NULL) && TENSOR_DATA_LIFETIME(tensor) == VX_TENSOR_LIFE_TIME_STATIC) ? vx_true_e : vx_false_e)

#define vxmOPERATION_COUNT(layer)       gcmCOUNTOF(layer->operations)

extern vx_float32 vxnneActivation(vx_enum func_v, vx_float32 a_v, vx_float32 b_v, vx_float32 value);

extern vx_status vxnneOperation_AddReference(
    vxnne_operation_s*            operation,
    vx_reference                  reference,
    vxnne_operation_reference_e   refType
    );

static vx_param_description_s nn_rnnlayer_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e }
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNRNNLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoRNNLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoRNNLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoRNNLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoRNNLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_NNRNNLayer = {
    VX_KERNEL_NN_RNN_LAYER,
    "vivante.nn.rnnlayer.layer",
    vxoNNRNNLayer,
    nn_rnnlayer_params, vxmLENGTH_OF(nn_rnnlayer_params),
    VX_NULL,
    vxoRNNLayer_ValidateInput,
    vxoRNNLayer_ValidateOutput,
    vxoRNNLayer_Initializer,
    vxoRNNLayer_Deinitializer
};

static vx_param_description_s nn_svdflayer_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e }
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoNNSVDFLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSVDFLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSVDFLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSVDFLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSVDFLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_NNSVDFLayer = {
    VX_KERNEL_NN_SVDF_LAYER,
    "vivante.nn.svdflayer.layer",
    vxoNNSVDFLayer,
    nn_svdflayer_params, vxmLENGTH_OF(nn_svdflayer_params),
    VX_NULL,
    vxoSVDFLayer_ValidateInput,
    vxoSVDFLayer_ValidateOutput,
    vxoSVDFLayer_Initializer,
    vxoSVDFLayer_Deinitializer
};


static vx_param_description_s nn_LSTMUnit_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e }
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_LSTMUnit(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMUnit_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMUnit_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMUnit_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMUnit_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NN_LSTMUnit = {
    VX_KERNEL_NN_LSTM_UNIT,
    "vivante.nn.lstm.unit",
    vxoBaseKernel_NN_LSTMUnit,
    nn_LSTMUnit_params, vxmLENGTH_OF(nn_LSTMUnit_params),
    VX_NULL,
    vxoNN_LSTMUnit_ValidateInput,
    vxoNN_LSTMUnit_ValidateOutput,
    vxoNN_LSTMUnit_Initializer,
    vxoNN_LSTMUnit_Deinitializer
};

static vx_param_description_s nn_LSTMLayer_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },


    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },


    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },

    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e }
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_LSTMLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_LSTMLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NN_LSTMLayer = {
    VX_KERNEL_NN_LSTM_LAYER,
    "vivante.nn.lstm.layer",
    vxoBaseKernel_NN_LSTMLayer,
    nn_LSTMLayer_params, vxmLENGTH_OF(nn_LSTMLayer_params),
    VX_NULL,
    vxoNN_LSTMLayer_ValidateInput,
    vxoNN_LSTMLayer_ValidateOutput,
    vxoNN_LSTMLayer_Initializer,
    vxoNN_LSTMLayer_Deinitializer
};
static vx_param_description_s nn_GRUUnit_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_GRUUnit(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRUUnit_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRUUnit_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRUUnit_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRUUnit_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NN_GRUUnit = {
    VX_KERNEL_NN_GRU_UNIT_LAYER,
    "vivante.nn.gru.unit",
    vxoBaseKernel_NN_GRUUnit,
    nn_GRUUnit_params, vxmLENGTH_OF(nn_GRUUnit_params),
    VX_NULL,
    vxoNN_GRUUnit_ValidateInput,
    vxoNN_GRUUnit_ValidateOutput,
    vxoNN_GRUUnit_Initializer,
    vxoNN_GRUUnit_Deinitializer
};

static vx_param_description_s nn_GRULayer_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_GRULayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRULayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRULayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRULayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_GRULayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NN_GRULayer = {
    VX_KERNEL_NN_GRU_LAYER,
    "vivante.nn.gru.layer",
    vxoBaseKernel_NN_GRULayer,
    nn_GRULayer_params, vxmLENGTH_OF(nn_GRULayer_params),
    VX_NULL,
    vxoNN_GRULayer_ValidateInput,
    vxoNN_GRULayer_ValidateOutput,
    vxoNN_GRULayer_Initializer,
    vxoNN_GRULayer_Deinitializer
};

static vx_param_description_s nn_ConvLSTMUnit_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },

    { VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },
    { VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },

    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_ConvLSTMUnit(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMUnit_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMUnit_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMUnit_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMUnit_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NN_ConvLSTMUnit = {
    VX_KERNEL_NN_CONV_LSTM_UNIT_LAYER,
    "vivante.nn.convlstm.unit",
    vxoBaseKernel_NN_ConvLSTMUnit,
    nn_ConvLSTMUnit_params, vxmLENGTH_OF(nn_ConvLSTMUnit_params),
    VX_NULL,
    vxoNN_ConvLSTMUnit_ValidateInput,
    vxoNN_ConvLSTMUnit_ValidateOutput,
    vxoNN_ConvLSTMUnit_Initializer,
    vxoNN_ConvLSTMUnit_Deinitializer
};

static vx_param_description_s nn_ConvLSTMLayer_params[] = {
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },

    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },
    { VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },

    { VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },
    { VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e },

    { VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NN_ConvLSTMLayer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMLayer_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMLayer_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMLayer_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNN_ConvLSTMLayer_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_NN_ConvLSTMLayer = {
    VX_KERNEL_NN_CONV_LSTM_LAYER,
    "vivante.nn.convlstm.layer",
    vxoBaseKernel_NN_ConvLSTMLayer,
    nn_ConvLSTMLayer_params, vxmLENGTH_OF(nn_ConvLSTMLayer_params),
    VX_NULL,
    vxoNN_ConvLSTMLayer_ValidateInput,
    vxoNN_ConvLSTMLayer_ValidateOutput,
    vxoNN_ConvLSTMLayer_Initializer,
    vxoNN_ConvLSTMLayer_Deinitializer
};

#endif

