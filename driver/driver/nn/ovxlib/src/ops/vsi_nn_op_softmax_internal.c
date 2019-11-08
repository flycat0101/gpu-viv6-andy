/****************************************************************************
*
*    Copyright (c) 2019 Vivante Corporation
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************/
#include <string.h>
#include <stdlib.h>

#include "vsi_nn_types.h"
#include "vsi_nn_platform.h"
#include "vsi_nn_graph.h"
#include "vsi_nn_node.h"
#include "vsi_nn_ops.h"
#include "vsi_nn_tensor.h"
#include "vsi_nn_tensor_util.h"
#include "vsi_nn_log.h"
#include "platform/vsi_nn_pf_softmax.h"
#include "utils/vsi_nn_util.h"
#include "utils/vsi_nn_link_list.h"

#define MAX_SOFTMAX_BATCH 65535

static vsi_bool _need_split_softmax
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs
    )
{
    vsi_bool ret = FALSE;
    if(inputs[0]->attr.dim_num == 2 && inputs[0]->attr.size[1] > MAX_SOFTMAX_BATCH)
    {
        ret = TRUE;
    }

    return ret;
} /* _need_split_softmax() */

static vsi_status _create_split_softmax
    (
    vsi_nn_node_t * self,
    vx_tensor src,
    vx_tensor dst
    )
{
    vsi_nn_softmax_internal_lcl_data * data;

    data = (vsi_nn_softmax_internal_lcl_data *)malloc( sizeof(vsi_nn_softmax_internal_lcl_data) );
    if( NULL == data )
    {
        VSILOGE( "Create softmax local data fail." );
        return VSI_FAILURE;
    }
    memset( data, 0, sizeof(vsi_nn_softmax_internal_lcl_data) );
    data->src_tensor = src;
    data->dst_tensor = dst;
    data->node = NULL;

    /* Store input & output */
    vsi_nn_LinkListPushStart(
        (vsi_nn_link_list_t **)&self->nn_param.softmax_internal.data,
        (vsi_nn_link_list_t *)data );

    return VSI_SUCCESS;
} /* _create_split_softmax() */

static vsi_status op_optimize
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs,
    vsi_nn_opt_direction_e direction
    )
{
    vsi_status status;
    vx_tensor in_view_tensor,out_view_tensor;
    uint32_t start[VSI_NN_MAX_DIM_NUM],end[VSI_NN_MAX_DIM_NUM];
    uint32_t axis, batch_size;

    in_view_tensor = NULL;
    out_view_tensor = NULL;
    status = VSI_SUCCESS;
    if(direction == VSI_NN_OPTIMIZE_BACKWARD)
    {
        return status;
    }
    if(_need_split_softmax(self, inputs) == FALSE)
    {
        return status;
    }

    VSILOGD("Optimize %s, uid %u", vsi_nn_OpGetName(self->op), self->uid);
    if( NULL == inputs[0]->t )
    {
        vsi_nn_TensorReinit( self->graph, inputs[0] );
    }
    if( NULL == outputs[0]->t )
    {
        vsi_nn_TensorReinit( self->graph, outputs[0] );
    }

    axis = 1; /* we only split 2D softmax, so the axis = batch dim */
    batch_size = inputs[0]->attr.size[1];
    memset( start, 0, sizeof( uint32_t ) * VSI_NN_MAX_DIM_NUM );
    memset( end, 0, sizeof( uint32_t ) * VSI_NN_MAX_DIM_NUM );
    end[0] = inputs[0]->attr.size[0];
    end[1] = inputs[0]->attr.size[1];
    end[2] = inputs[0]->attr.size[2];
    end[3] = inputs[0]->attr.size[3];
    end[axis] = 0;
    while(end[axis] < batch_size)
    {
        start[axis] = end[axis];
        end[axis] += MAX_SOFTMAX_BATCH;
        if(end[axis] > inputs[0]->attr.size[axis])
        {
            end[axis] = inputs[0]->attr.size[axis];
        }

        in_view_tensor = vsi_nn_CreateViewTensor(self->graph, start, end, inputs[0]);
        if(NULL == in_view_tensor)
        {
            VSILOGE( "Create inputs view tensor fail.");
            break;
        }
        out_view_tensor = vsi_nn_CreateViewTensor(self->graph, start, end, outputs[0]);
        if(NULL == out_view_tensor)
        {
            VSILOGE( "Create outputs view tensor fail.");
            break;
        }

        status = _create_split_softmax(self, in_view_tensor, out_view_tensor);
        if(VSI_SUCCESS != status)
        {
            VSILOGE( "Create split softmax data struct fail.");
            break;
        }
    }

    return status;
}

static vsi_bool op_check
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs
    )
{
    //TODO: Check tensor shapes.
    return TRUE;
} /* op_check() */

static vsi_status op_deinit
    (
    vsi_nn_node_t * self
    )
{
    vsi_status status;
    vsi_nn_softmax_internal_lcl_data * data;
    vsi_nn_softmax_internal_lcl_data * tmp;

    if(NULL == self)
    {
        return VSI_FAILURE;
    }
    data = self->nn_param.softmax_internal.data;

    status = VSI_SUCCESS;
    if(data)
    {
        while( NULL != data )
        {
            tmp = (vsi_nn_softmax_internal_lcl_data *)vsi_nn_LinkListPopStart(
                (vsi_nn_link_list_t **)&data );
            vxReleaseNode( &tmp->node );
            vxReleaseTensor( &tmp->src_tensor );
            vxReleaseTensor( &tmp->dst_tensor );
            free( tmp );
            tmp = NULL;
        }
    }
    status = vsi_nn_op_common_deinit(self);

    return status;
} /* op_deinit() */

#ifdef __cplusplus
extern "C" {
#endif
/* Registrar */
DEF_OP_REG
    (
    /* op_name    */ SOFTMAX_INTERNAL,
    /* init       */ NULL,
    /* compute    */ vsi_nn_softmax_compute,
    /* deinit     */ op_deinit,
    /* check      */ op_check,
    /* setup      */ vsi_nn_op_common_setup,
    /* optimize   */ op_optimize,
    /* input_num  */ 1,
    /* output_num */ 1
    );
#ifdef __cplusplus
}
#endif