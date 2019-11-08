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
#include "vsi_nn_log.h"
#include "vsi_nn_graph.h"
#include "vsi_nn_node.h"
#include "vsi_nn_prv.h"
#include "utils/vsi_nn_math.h"
#include "vsi_nn_ops.h"
#include "vsi_nn_tensor.h"
#include "vsi_nn_tensor_util.h"
#include "client/vsi_nn_vxkernel.h"

#define _ARG_NUM            (0)
#define _INPUT_NUM          (1)
#define _OUTPUT_NUM         (1)
#define _IO_NUM             (_INPUT_NUM + _OUTPUT_NUM + 1)
#define _PARAM_NUM          (_ARG_NUM + _IO_NUM)
#define TILE_MULTIPLE_VX_SIZE (4)

extern vx_kernel_description_t * vx_kernel_TILE_list[];

static void check_tensor_shape
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t * input,
    vx_reference * params,
    uint32_t index
    )
{
    vsi_nn_tensor_attr_t attr;
    vsi_nn_tile_param * p;

    p = (vsi_nn_tile_param *)&(self->nn_param.tile);

    if (index == 0)
    {
        if( input->attr.dim_num == 1)
        {
            memcpy(&attr, &(input->attr), sizeof(vsi_nn_tensor_attr_t));
            attr.size[1] = 1;
            attr.dim_num = 2;
            p->local.local_tensor[index] =
                vxReshapeTensor(input->t, (int32_t*)(attr.size), attr.dim_num);
            params[index] =  (vx_reference)p->local.local_tensor[index];
        }
        else
            params[index] = (vx_reference)input->t;
    }
    else if (index == 1)
    {
        if( input->attr.dim_num == 1)
        {
            memcpy(&attr, &(input->attr), sizeof(vsi_nn_tensor_attr_t));
            attr.size[1] = 1;
            attr.dim_num = 2;
            p->local.local_tensor[index] =
                vxReshapeTensor(input->t, (int32_t*)(attr.size), attr.dim_num);
            params[index] =  (vx_reference)p->local.local_tensor[index];
        }
        else if( input->attr.dim_num == 4)
        {
            memcpy(&attr, &(input->attr), sizeof(vsi_nn_tensor_attr_t));
            attr.size[2] *= attr.size[3];
            attr.size[3] = 1;
            attr.dim_num = 3;
            p->local.local_tensor[index] =
                vxReshapeTensor(input->t, (int32_t*)(attr.size), attr.dim_num);
            params[index] =  (vx_reference)p->local.local_tensor[index];
        }
        else
            params[index] = (vx_reference)input->t;
    }
    else
    {
        VSILOGE("No more local tensor!(tile) at [%s : %d]\n", __FILE__, __LINE__);
    }
}

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

static void prepare_multiples
    (
    vsi_nn_node_t * self,
    vx_reference * params
    )
{
    vsi_nn_tile_param * p;
    vsi_nn_tensor_attr_t attr;
    vsi_nn_tensor_t *multiples_tensor = NULL;
    uint32_t i = 0;
    int32_t multiples[TILE_MULTIPLE_VX_SIZE] = {1, 1, 1, 1};

    p = &(self->nn_param.tile);

    memset(&attr, 0, sizeof(attr));
    attr.size[0] = TILE_MULTIPLE_VX_SIZE;
    attr.size[1] = 1;
    attr.dim_num = 2;
    attr.is_const = TRUE;
    attr.dtype.vx_type = VSI_NN_TYPE_INT32;
    attr.dtype.qnt_type = VSI_NN_QNT_TYPE_NONE;

    // check multiples
    {
        for(i = 0; i < p->multiples_num; i++)
        {
            multiples[i] = p->multiples[i] == 0 ? 1 : p->multiples[i];
        }
    }

    multiples_tensor = vsi_nn_CreateTensorFromData(
        self->graph,
        (uint8_t *)multiples,
        &attr);
    if( NULL == multiples_tensor )
    {
        VSILOGE("Create multiples_tensor fail.(tile)");
        return;
    }

    p->local.multiples_tensor = multiples_tensor;
    params[_INPUT_NUM + _OUTPUT_NUM] = (vx_reference)(multiples_tensor->t);
}

static vsi_status cpu_op_compute
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs
    )
{
    vsi_status status = VSI_SUCCESS;
    vx_reference params[_PARAM_NUM];

    if( NULL == self->n )
    {
        return VSI_FAILURE;
    }

    /* Set inputs and outputs */
    _set_inputs_outputs( params, inputs, outputs );
    prepare_multiples(self, params);

    /* Pass parameters to node. */
    status = vsi_nn_ClientNodePassParameters( self->n, params, _PARAM_NUM );

    return status;
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

    if( NULL == self->n )
    {
        return VSI_FAILURE;
    }

    /* Set inputs and outputs */
    check_tensor_shape(self, inputs[0], params, 0);
    check_tensor_shape(self, outputs[0], params, 1);
    prepare_multiples(self, params);

    /* Pass parameters to node. */
    status = vsi_nn_ClientNodePassParameters( self->n, params, _PARAM_NUM );

    return status;
}

static vsi_nn_op_compute_t op_compute_list[] =
{
    cpu_op_compute,
    vx_op_compute,
    NULL
};

static vsi_status vx_op_pre_compute
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs,
    vsi_nn_kernel_info_t * kernel_info
    )
{
    vsi_nn_type_e inputDataFormat     = inputs[0]->attr.dtype.vx_type;
    uint32_t width = inputs[0]->attr.size[0];

    if (inputDataFormat == VSI_NN_TYPE_UINT8 || inputDataFormat == VSI_NN_TYPE_INT8)
    {
        if(width % 8 == 0)
            kernel_info->kernel_index = 1;
        else if(width % 4 == 0)
            kernel_info->kernel_index = 2;
        else if(width % 8 == 1)
            kernel_info->kernel_index = 3;
        else if(width % 8 == 2)
            kernel_info->kernel_index = 4;
        else if(width % 8 == 3)
            kernel_info->kernel_index = 5;
        else if(width % 8 == 5)
            kernel_info->kernel_index = 6;
        else if(width % 8 == 6)
            kernel_info->kernel_index = 7;
        else if(width % 8 == 7)
            kernel_info->kernel_index = 8;
    }
    else if(inputDataFormat == VSI_NN_TYPE_INT16 || inputDataFormat == VSI_NN_TYPE_FLOAT16)
    {
        if(width % 8 == 0)
            kernel_info->kernel_index = 9;
        else if(width % 4 == 0)
            kernel_info->kernel_index = 10;
        else if(width % 8 == 1)
            kernel_info->kernel_index = 11;
        else if(width % 8 == 2)
            kernel_info->kernel_index = 12;
        else if(width % 8 == 3)
            kernel_info->kernel_index = 13;
        else if(width % 8 == 5)
            kernel_info->kernel_index = 14;
        else if(width % 8 == 6)
            kernel_info->kernel_index = 15;
        else if(width % 8 == 7)
            kernel_info->kernel_index = 16;
    }
    else
    {
        VSILOGE("Not support input or output data format!(tile) at [%s : %d]\n", __FILE__, __LINE__);
        return VSI_FAILURE;
    }

    return VSI_SUCCESS;
}

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
    kernel_info.type = VX_KERNEL_TYPE_CPU;
    kernel_info.kernel = vx_kernel_TILE_list;
    kernel_info.resource_num = 1;
    kernel_info.resource_name = (char **)malloc(kernel_info.resource_num * sizeof(char *));
    kernel_info.resource_name[0] = "vsi_nn_kernel_tile";

    {
        vsi_nn_type_e inputDataFormat   = inputs[0]->attr.dtype.vx_type;

        if(inputs[0]->attr.size[3] < 2
            && inputs[0]->attr.dim_num <=4
            && (inputDataFormat == VSI_NN_TYPE_UINT8
                || inputDataFormat == VSI_NN_TYPE_INT8
                || inputDataFormat == VSI_NN_TYPE_INT16
                || inputDataFormat == VSI_NN_TYPE_FLOAT16))
            kernel_info.type = VX_KERNEL_TYPE_VX;
    }

    if( kernel_info.type == VX_KERNEL_TYPE_VX)
    {
        kernel_info.kernel_index = 1;
        kernel_info.init_index = 1;
        vx_op_pre_compute(self, inputs, outputs, &kernel_info);
    }
    else /*kernel_info.type = VX_KERNEL_TYPE_CPU;*/
    {
        kernel_info.kernel_index = 0;
        kernel_info.init_index = 0;
    }

    self->n = vsi_nn_RegisterClientKernelAndNewNode(
            self->graph, &kernel_info);
    if (kernel_info.resource_name)
    {
        free(kernel_info.resource_name);
    }
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
    /*TODO: Check tensor shapes. */
    return TRUE;
} /* op_check() */

static vsi_bool op_setup
    (
    vsi_nn_node_t * self,
    vsi_nn_tensor_t ** inputs,
    vsi_nn_tensor_t ** outputs
    )
{
    /* TODO: Add code to comput outputs' shape. */
    uint32_t i;

    if( VSI_NN_DIM_AUTO == outputs[0]->attr.dim_num )
    {
        vsi_nn_tile_param * p;

        p = &(self->nn_param.tile);
        if (inputs[0]->attr.dim_num != p->multiples_num)
        {
            VSILOGE("multiples_num MUST match the dims of input tensor!");
            return FALSE;
        }
        outputs[0]->attr.dim_num = inputs[0]->attr.dim_num;
        for (i = 0; i < inputs[0]->attr.dim_num; i++)
        {
            outputs[0]->attr.size[i] = inputs[0]->attr.size[i] * p->multiples[i];
        }
    }
    return TRUE;
} /* op_setup() */

static vsi_status op_deinit
    (
    vsi_nn_node_t * self
    )
{
    uint32_t i;

    if (self->nn_param.tile.local.multiples_tensor != NULL)
    {
        vsi_nn_ReleaseTensor(&(self->nn_param.tile.local.multiples_tensor));
    }

    for (i = 0; i < _VSI_NN_TILE_LOCAL_TENSOR_NUM; i++)
    {
        if (self->nn_param.tile.local.local_tensor[i] != NULL)
        {
            vxReleaseTensor(&(self->nn_param.tile.local.local_tensor[i]));
            self->nn_param.tile.local.local_tensor[i] = NULL;
        }
    }

    vsi_nn_op_common_deinit(self);

    return VSI_SUCCESS;
} /* op_deinit() */

#ifdef __cplusplus
extern "C" {
#endif
/* Registrar */
DEF_OP_REG
    (
    /* op_name    */ TILE,
    /* init       */ NULL,
    /* compute    */ op_compute,
    /* deinit     */ op_deinit,
    /* check      */ op_check,
    /* setup      */ op_setup,
    /* optimize   */ NULL,
    /* input_num  */ _INPUT_NUM,
    /* output_num */ _OUTPUT_NUM
    );
#ifdef __cplusplus
}
#endif