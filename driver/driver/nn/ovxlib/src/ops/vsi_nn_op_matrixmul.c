/****************************************************************************
*
*    Copyright (c) 2018 Vivante Corporation
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
#include "utils/vsi_nn_util.h"
#include "vsi_nn_prv.h"
#include "vsi_nn_log.h"
#include "client/vsi_nn_vxkernel.h"

#define _ARG_NUM            (7)
#define _INPUT_NUM          (2)
#define _OUTPUT_NUM         (1)
#define _IO_NUM             (_INPUT_NUM + _OUTPUT_NUM)
#define _PARAM_NUM          (_ARG_NUM + _IO_NUM)

extern vx_kernel_description_t * vx_kernel_MATRIXMUL_list[];

static void _set_inputs_outputs
    (
    vx_reference * params,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs
    )
{
    uint32_t i;
    uint32_t cnt;

    /* Set inputs */
    cnt = 0;
    for( i = 0; i < _INPUT_NUM; i ++, cnt ++ )
    {
        params[cnt] = (vx_reference)inputs[i]->t;
    }

    /* Set outputs */
    for( i = 0; i < _OUTPUT_NUM; i ++, cnt ++ )
    {
        params[cnt] = (vx_reference)outputs[i]->t;
    }
} /* _set_inputs_outputs() */

static vsi_status _create_params
    (
    vsi_nn_node_t * node,
    vsi_nn_tensor_t ** inputs,
    vx_reference * params,
    uint32_t num
    )
{
    vsi_status status;
    vx_context ctx;
    //vsi_nn_matrixmul_param * p;
    if( 0 == num )
    {
        return VSI_SUCCESS;
    }
    memset( params, 0, sizeof( vx_reference * ) * num );
    //p = &(node->nn_param.matrixmul);
    ctx = vxGetContext( (vx_reference)node->graph->g );
    /* Init parameters */
#define _SET_PARAM( i, type, arg ) do{ \
    params[i] = (vx_reference)vxCreateScalar( ctx, type, &arg ); \
    status = vxGetStatus( params[i] ); \
    if( VSI_SUCCESS != status ) { \
    goto set_param_error; \
    } \
    } while(0)
    _SET_PARAM( 0, VX_TYPE_BOOL, node->nn_param.matrixmul.transpose[0] );
    _SET_PARAM( 1, VX_TYPE_BOOL, node->nn_param.matrixmul.transpose[1] );
    _SET_PARAM( 2, VX_TYPE_BOOL, node->nn_param.matrixmul.adjoint[0] );
    _SET_PARAM( 3, VX_TYPE_BOOL, node->nn_param.matrixmul.adjoint[1] );
    _SET_PARAM( 4, VX_TYPE_UINT32, inputs[0]->attr.size[1] );
    _SET_PARAM( 5, VX_TYPE_UINT32, inputs[0]->attr.size[0] );
    _SET_PARAM( 6, VX_TYPE_UINT32, inputs[1]->attr.size[0] );
#undef _SET_PARAM
set_param_error:

    return status;
} /* _create_params */

static void _release_params
    (
    vx_reference * params,
    uint32_t num
    )
{
    uint32_t i;
    vx_scalar scalar;
    for( i = 0; i < num; i ++ )
    {
        scalar = (vx_scalar)params[i];
        vxReleaseScalar( &scalar );
    }
} /* _release_params() */

static vsi_status vx_op_pre_compute
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs,
    vsi_nn_kernel_info_t * kernel_info
    )
{
    vsi_nn_type_e inputADataformat = inputs[0]->attr.dtype.vx_type;
    vsi_nn_type_e inputBDataformat = inputs[1]->attr.dtype.vx_type;
    vsi_nn_type_e outputDataformat = outputs[0]->attr.dtype.vx_type;

    if (self->nn_param.matrixmul.transpose[0] == FALSE
        && self->nn_param.matrixmul.transpose[1] == FALSE)
    {
        if ((inputADataformat == VSI_NN_TYPE_FLOAT16) && (inputBDataformat == VSI_NN_TYPE_FLOAT16)
            && (outputDataformat == VSI_NN_TYPE_FLOAT16))
        {
            kernel_info->resource_name[0] = "vsi_nn_kernel_matrixmul_fp16";
            kernel_info->kernel_index = 1;
        }
        else if ((inputADataformat == VSI_NN_TYPE_UINT8) && (inputBDataformat == VSI_NN_TYPE_UINT8)
            && (outputDataformat == VSI_NN_TYPE_UINT8))
        {
            kernel_info->kernel_index = 2;
        }
        else if ((inputADataformat == VSI_NN_TYPE_FLOAT16) && (inputBDataformat == VSI_NN_TYPE_UINT8)
            && (outputDataformat == VSI_NN_TYPE_FLOAT16))
        {
            kernel_info->kernel_index = 3;
        }
        else if ((inputADataformat == VSI_NN_TYPE_FLOAT16) && (inputBDataformat == VSI_NN_TYPE_UINT8)
            && (outputDataformat == VSI_NN_TYPE_UINT8))
        {
            kernel_info->kernel_index = 4;
        }
        else if ((inputADataformat == VSI_NN_TYPE_FLOAT16) && (inputBDataformat == VSI_NN_TYPE_FLOAT16)
            && (outputDataformat == VSI_NN_TYPE_UINT8))
        {
            kernel_info->resource_name[0] = "vsi_nn_kernel_matrixmul_fp16";
            kernel_info->kernel_index = 8;
        }
        else
        {
            VSILOGE("Not support input or output data format!(MATRIXMUL)\n");
            return VSI_FAILURE;
        }
    }
    else if (self->nn_param.matrixmul.transpose[0] == FALSE
        && self->nn_param.matrixmul.transpose[1] == TRUE)
    {
        kernel_info->resource_name[0] = "vsi_nn_kernel_matrixmul_transbp1";
        if ((inputADataformat == VSI_NN_TYPE_FLOAT16) && (inputBDataformat == VSI_NN_TYPE_UINT8)
            && (outputDataformat == VSI_NN_TYPE_FLOAT16))
        {
            kernel_info->kernel_index = 5;
        }
        else if ((inputADataformat == VSI_NN_TYPE_FLOAT16) && (inputBDataformat == VSI_NN_TYPE_UINT8)
            && (outputDataformat == VSI_NN_TYPE_UINT8))
        {
            kernel_info->kernel_index = 6;
        }
        else if ((inputADataformat == VSI_NN_TYPE_UINT8) && (inputBDataformat == VSI_NN_TYPE_UINT8)
            && (outputDataformat == VSI_NN_TYPE_FLOAT16))
        {
            kernel_info->resource_name[0] = "vsi_nn_kernel_matrixmul_transbp2";
            kernel_info->kernel_index = 7;
        }
        else
        {
            VSILOGE("Not support input or output data format!(MATRIXMUL)\n");
            return VSI_FAILURE;
        }
    }
    else
    {
        VSILOGE("[%s : %d]Not support transpose format!(MATRIXMUL)\n",__FILE__, __LINE__);
        return VSI_FAILURE;
    }

    return VSI_SUCCESS;
}

static vsi_status vx_op_compute
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs
    )
{
    vsi_status status = VSI_SUCCESS;
    vx_reference params[_PARAM_NUM];
    vx_border_t border;
    vx_reference * args;

    args = &params[_IO_NUM];
    if( NULL == self->n )
    {
        return VSI_FAILURE;
    }

    /* Set inputs and outputs */
    _set_inputs_outputs( params, inputs, outputs );

    /* Init parameters. */
    _create_params( self, inputs, args, _ARG_NUM );

    /* Pass parameters to node. */
    status = vsi_nn_ClientNodePassParameters( self->n, params, _PARAM_NUM );

    border.constant_value.U32 = 0;
    border.constant_value.U8 = 0;
    border.constant_value.S16 = 0;
    border.mode = VX_BORDER_CONSTANT;
    status |= vxSetNodeAttribute(self->n, VX_NODE_BORDER, &border, sizeof(border));

    _release_params( args, _ARG_NUM );

    return status;
}

static vsi_nn_op_compute_t op_compute_list[] =
{
    NULL,
    vx_op_compute,
    NULL
};

static vsi_status op_compute
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs
    )
{
    vsi_status status;
    vsi_nn_kernel_info_t kernel_info = {0};

    status = VSI_FAILURE;
    kernel_info.resource_num = 1;
    kernel_info.resource_name = (char **)malloc(kernel_info.resource_num * sizeof(char *));
    kernel_info.resource_name[0] = "vsi_nn_kernel_matrixmul";
    kernel_info.type = vsi_nn_GetVXKernelTypeForShader();
    kernel_info.kernel = vx_kernel_MATRIXMUL_list;
    kernel_info.init_index = 1;

    if (vsi_nn_is_do_vx_op_pre_init(kernel_info.type))
    {
        vx_op_pre_compute(self, inputs, outputs, &kernel_info);
    }

    self->n = vsi_nn_RegisterClientKernelAndNewNode(
        self->graph, &kernel_info);
    if (kernel_info.resource_name) free(kernel_info.resource_name);
    if( NULL == self->n )
    {
        return VSI_FAILURE;
    }
    if (NULL != op_compute_list[kernel_info.init_index])
    {
        status = op_compute_list[kernel_info.init_index](self, inputs, outputs);
    }
    return status;
} /* op_compute() */

static vsi_bool op_check
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs
    )
{
    uint32_t i;
    vx_bool status = TRUE;

    if(inputs[0]->attr.dim_num != inputs[1]->attr.dim_num)
    {
         VSILOGE("input tensors have different dim_num");
         return FALSE;
    }
    if (self->nn_param.matrixmul.transpose[0] == FALSE
        && self->nn_param.matrixmul.transpose[1] == FALSE
        && inputs[0]->attr.size[0] != inputs[1]->attr.size[1])
    {
         VSILOGE("1st input tensor's size[0] is not equal to 2nd input tensor's size[1]");
         return FALSE;
    }
    for (i = 2; i < inputs[0]->attr.dim_num; i++)
    {
        if (inputs[0]->attr.size[i] != inputs[1]->attr.size[i])
        {
            VSILOGE("1st input tensor's size[%d] is not equal to 2nd input tensor's size[%d]", i , i);
            return FALSE;
        }
    }
    return status;
} /* op_check() */

static vsi_bool op_setup
    (
    vsi_nn_node_t * node,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs
    )
{
    uint32_t i;
    if( VSI_NN_DIM_AUTO == outputs[0]->attr.dim_num )
    {
        outputs[0]->attr.dim_num = inputs[0]->attr.dim_num;
        if (node->nn_param.matrixmul.transpose[0] == FALSE
            && node->nn_param.matrixmul.transpose[1] == FALSE)
        {
            outputs[0]->attr.size[0] = inputs[1]->attr.size[0];
            outputs[0]->attr.size[1] = inputs[0]->attr.size[1];
        }
        else if (node->nn_param.matrixmul.transpose[0] == TRUE
            && node->nn_param.matrixmul.transpose[1] == FALSE)
        {
            outputs[0]->attr.size[0] = inputs[1]->attr.size[0];
            outputs[0]->attr.size[1] = inputs[0]->attr.size[0];
        }
        else if (node->nn_param.matrixmul.transpose[0] == FALSE
            && node->nn_param.matrixmul.transpose[1] == TRUE)
        {
            outputs[0]->attr.size[0] = inputs[1]->attr.size[1];
            outputs[0]->attr.size[1] = inputs[0]->attr.size[1];
        }
        else
        {
            VSILOGE("Not support transpose A and B both TRUE!(MATRIXMUL) at [%s : %d]\n", __FILE__, __LINE__);
            return FALSE;
        }
        for (i = 2; i < inputs[0]->attr.dim_num; i++)
        {
            outputs[0]->attr.size[i] = inputs[0]->attr.size[i];
        }
    }
    return TRUE;
} /* op_setup() */

#ifdef __cplusplus
extern "C" {
#endif
/* Registrar */
DEF_OP_REG
    (
    /* op_name    */ MATRIXMUL,
    /* init       */ NULL,
    /* compute    */ op_compute,
    /* deinit     */ vsi_nn_op_common_deinit,
    /* check      */ op_check,
    /* setup      */ op_setup,
    /* optimize   */ NULL,
    /* input_num  */ 2,
    /* output_num */ 1
    );
#ifdef __cplusplus
}
#endif