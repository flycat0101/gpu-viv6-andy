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


#ifndef __GC_VX_LAYER_LSTM_H__
#define __GC_VX_LAYER_LSTM_H__

#include <gc_vx_common.h>
#include <gc_vx_layer.h>

vx_status vxoLSTMLayer_Create(
    vxnne_lstm_layer *lstm_layer
    );

vx_status vxoLSTMLayer_Destroy(
    vxnne_lstm_layer lstm_layer
    );

vx_status vxoLSTMLayer_Initialize(
    vxnne_lstm_layer lstm_layer,
    vx_node node,
    vx_tensor inputs,
    vx_tensor input2input_weights,
    vx_tensor input2forget_weights,
    vx_tensor input2cell_weights,
    vx_tensor input2output_weights,
    vx_tensor recurrent2input_weights,
    vx_tensor recurrent2forget_weights,
    vx_tensor recurrent2cell_weights,
    vx_tensor recurrent2output_weights,
    vx_tensor cell2input_weights,
    vx_tensor cell2forget_weights,
    vx_tensor cell2output_weights,
    vx_tensor input_gate_biases,
    vx_tensor forget_gate_biases,
    vx_tensor cell_biases,
    vx_tensor output_gate_biases,
    vx_tensor projection_weights,
    vx_tensor projection_biases,
    vx_tensor activation,
    vx_tensor forget_biases,
    vx_tensor cell_clip,
    vx_tensor proj_clip,
    vx_tensor outputs
    );

vx_status vxoLSTMLayer_Deinitialize(
    vxnne_lstm_layer lstm_layer
    );

#endif

