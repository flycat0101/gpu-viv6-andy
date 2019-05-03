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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "vsi_nn_platform.h"

#include "vsi_nn_prv.h"
#include "vsi_nn_log.h"
#include "vsi_nn_tensor_util.h"
#include "utils/vsi_nn_util.h"
#include "utils/vsi_nn_dtype_util.h"
#include "utils/vsi_nn_math.h"
#include "client/vsi_nn_vxkernel.h"
#include "libnnext/vx_lib_nnext.h"

#define _VX_KERNEL_ID           (VX_KERNEL_ENUM_SELECT)
#define _VX_KERNEL_FUNC_KERNEL  (vxSelectKernel)

void mySelectFunc
    (
    void* imgCond,
    void* imgIn,
    void* imgIn1,
    void* imgOut,
    uint32_t input_dim,
    uint32_t width,
    uint32_t height,
    uint32_t channel,
    uint32_t batch,
    vsi_nn_type_e type
    )
{
    uint32_t k;
    uint32_t iter = batch * channel * height * width;

    if(type == VSI_NN_TYPE_INT16 || type == VSI_NN_TYPE_FLOAT16)
    {
        int16_t* tmpIn = (int16_t*)imgIn;
        int16_t* tmpIn1 = (int16_t*)imgIn1;
        int16_t* tmpOut = (int16_t*)imgOut;
        int16_t data0, data1;
        int16_t* tmpCond = (int16_t*)imgCond;
        int16_t cond = 0;

        for(k = 0; k < iter; k++)
        {
            cond = tmpCond[k];
            data0 = tmpIn[k];
            data1 = tmpIn1[k];
            tmpOut[k] = cond ? data0 : data1;
        }
    }
    else if(type == VSI_NN_TYPE_INT8)
    {
        int8_t* tmpIn = (int8_t*)imgIn;
        int8_t* tmpIn1 = (int8_t*)imgIn1;
        int8_t* tmpOut = (int8_t*)imgOut;
        int8_t data0, data1;
        int8_t* tmpCond = (int8_t*)imgCond;
        int8_t cond = 0;

        for(k = 0; k < iter; k++)
        {
            cond = tmpCond[k];
            data0 = tmpIn[k];
            data1 = tmpIn1[k];
            tmpOut[k] = cond ? data0 : data1;
        }
    }
    else if(type == VSI_NN_TYPE_UINT8)
    {
        uint8_t* tmpIn = (uint8_t*)imgIn;
        uint8_t* tmpIn1 = (uint8_t*)imgIn1;
        uint8_t* tmpOut = (uint8_t*)imgOut;
        uint8_t data0, data1;
        uint8_t* tmpCond = (uint8_t*)imgCond;
        uint8_t cond = 0;

        for(k = 0; k < iter; k++)
        {
            cond = tmpCond[k];
            data0 = tmpIn[k];
            data1 = tmpIn1[k];
            tmpOut[k] = cond ? data0 : data1;
        }
    }

    return;
}

static vsi_status VX_CALLBACK vxSelectKernel
    (
    vx_node node,
    const vx_reference* paramObj,
    uint32_t paramNum
    )
{
    vsi_status status = VX_ERROR_INVALID_PARAMETERS;

    if(paramNum == 4)
    {
        vx_context context = NULL;
        // tensor
        vx_tensor imgObj[4] = { NULL };
#if INPUT_FP16
        int16_t *input = NULL;
#else
        uint8_t *condition = NULL;
        uint8_t *input = NULL;
        uint8_t *input1 = NULL;
#endif
#if OUTPUT_FP16
        int16_t *output = NULL;
#else
        uint8_t *output = NULL;
#endif

        uint32_t input_size[DIM_SIZE] = {0}, output_size[DIM_SIZE] = {0};
        uint32_t cond_stride_size[4]  = {0};
        uint32_t input_stride_size[4]  = {0};
        uint32_t output_stride_size[4] = {0};

        vx_tensor_addressing condition_user_addr = NULL;
        vx_tensor_addressing input_user_addr = NULL;
        vx_tensor_addressing input1_user_addr = NULL;
        vx_tensor_addressing output_user_addr = NULL;

        vsi_nn_type_e inputFormat = VSI_NN_TYPE_FLOAT16, outputFormat = VSI_NN_TYPE_FLOAT16;
        vsi_nn_type_e condFormat = VSI_NN_TYPE_INT8;
        uint32_t input_dims = 0, output_dims = 0, tmpDim = 0;
        uint32_t i;
        int32_t in_zp, out_zp;
        float in_scale, out_scale;

        imgObj[0] = (vx_tensor)paramObj[0];
        imgObj[1] = (vx_tensor)paramObj[1];  //output
        imgObj[2] = (vx_tensor)paramObj[2];
        imgObj[3] = (vx_tensor)paramObj[3];
        context = vxGetContext((vx_reference)node);
        if (context == NULL)
        {
            VSILOGE("vxGetContext failure! at line %d\n", __LINE__);
            return status;
        }

        //condition
        status = vxQueryTensor(imgObj[0], VX_TENSOR_DATA_TYPE, &condFormat, sizeof(condFormat));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor condFormat failure! at line %d\n", __LINE__);
            return status;
        }

        //input
        status = vxQueryTensor(imgObj[1], VX_TENSOR_NUM_OF_DIMS, &input_dims, sizeof(input_dims));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor input_dims failure! at line %d\n", __LINE__);
            return status;
        }
        status = vxQueryTensor(imgObj[1], VX_TENSOR_DATA_TYPE, &inputFormat, sizeof(inputFormat));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor inputFormat failure! at line %d\n", __LINE__);
            return status;
        }
        status = vxQueryTensor(imgObj[1], VX_TENSOR_DIMS, input_size, sizeof(input_size));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor input_size failure! at line %d\n", __LINE__);
            return status;
        }
        status = vxQueryTensor(imgObj[1], VX_TENSOR_ZERO_POINT, &in_zp, sizeof(in_zp));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor input_size failure! at line %d\n", __LINE__);
            return status;
        }
        status = vxQueryTensor(imgObj[1], VX_TENSOR_SCALE, &in_scale, sizeof(in_scale));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor input_size failure! at line %d\n", __LINE__);
            return status;
        }
        //output
        status  = vxQueryTensor(imgObj[3], VX_TENSOR_DATA_TYPE, &outputFormat, sizeof(outputFormat));
        status |= vxQueryTensor(imgObj[3], VX_TENSOR_NUM_OF_DIMS, &output_dims, sizeof(output_dims));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor outputFormat failure! at line %d\n", __LINE__);
            return status;
        }
        status = vxQueryTensor(imgObj[3], VX_TENSOR_DIMS, output_size, sizeof(output_size));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor output_size failure! at line %d\n", __LINE__);
            return status;
        }
        status = vxQueryTensor(imgObj[3], VX_TENSOR_ZERO_POINT, &out_zp, sizeof(out_zp));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor input_size failure! at line %d\n", __LINE__);
            return status;
        }
        status = vxQueryTensor(imgObj[3], VX_TENSOR_SCALE, &out_scale, sizeof(out_scale));
        if (status != VX_SUCCESS)
        {
            VSILOGE("vxQueryTensor input_size failure! at line %d\n", __LINE__);
            return status;
        }

        input_size[2] = (input_dims <= 2)?1:input_size[2];
        input_size[3] = (input_dims <= 3)?1:input_size[3];

        cond_stride_size[0] = vsi_nn_GetTypeBytes(condFormat);
        input_stride_size[0]  = vsi_nn_GetTypeBytes(inputFormat);
        output_stride_size[0] = vsi_nn_GetTypeBytes(outputFormat);
        //length_stride_size[0] = vsi_nn_GetTypeBytes(paraFormat);
        for (i=1; i< input_dims; i++)
        {
            cond_stride_size[i]  = cond_stride_size[i-1] * input_size[i-1];
        }
        for (i=1; i< input_dims; i++)
        {
            input_stride_size[i]  = input_stride_size[i-1] * input_size[i-1];
        }
        for (i=1; i< output_dims; i++)
        {
            output_stride_size[i] = output_stride_size[i-1] * output_size[i-1];
        }

#if INPUT_FP16
        input  = (int16_t*)malloc(input_size[0]*input_size[1]*input_size[2]*sizeof(int16_t));
#else
        condition = (uint8_t*)malloc(input_size[0]*input_size[1]*input_size[2]*vsi_nn_GetTypeBytes(condFormat));
        input  = (uint8_t*)malloc(input_size[0]*input_size[1]*input_size[2]*vsi_nn_GetTypeBytes(inputFormat));
        input1  = (uint8_t*)malloc(input_size[0]*input_size[1]*input_size[2]*vsi_nn_GetTypeBytes(inputFormat));
#endif
#if OUTPUT_FP16
        output = (int16_t*)malloc(output_size[0]*output_size[1]*output_size[2]*sizeof(int16_t));
#else
        output = (uint8_t*)malloc(output_size[0]*output_size[1]*output_size[2]*vsi_nn_GetTypeBytes(outputFormat));
#endif

        condition_user_addr = vxCreateTensorAddressing(context, input_size, cond_stride_size, input_dims);
        vxCopyTensorPatch(imgObj[0], NULL, condition_user_addr, condition, VX_READ_ONLY, 0);

        input_user_addr = vxCreateTensorAddressing(context, input_size, input_stride_size, input_dims);
        vxCopyTensorPatch(imgObj[1], NULL, input_user_addr, input, VX_READ_ONLY, 0);

        input1_user_addr = vxCreateTensorAddressing(context, input_size, input_stride_size, input_dims);
        vxCopyTensorPatch(imgObj[2], NULL, input1_user_addr, input1, VX_READ_ONLY, 0);

        // Call C Prototype
        mySelectFunc(condition, input, input1, output, tmpDim, input_size[0],
            input_size[1], input_size[2], input_size[3], inputFormat);

        //output tensor
        output_user_addr = vxCreateTensorAddressing(context, output_size,
            output_stride_size, output_dims);
        vxCopyTensorPatch(imgObj[3], NULL, output_user_addr, output, VX_WRITE_ONLY, 0);

        if(condition) free(condition);
        if(input) free(input);
        if(input1) free(input1);
        if(output) free(output);

        if(condition_user_addr) vxReleaseTensorAddressing(&condition_user_addr);
        if(input_user_addr) vxReleaseTensorAddressing(&input_user_addr);
        if(input1_user_addr) vxReleaseTensorAddressing(&input1_user_addr);
        if(output_user_addr) vxReleaseTensorAddressing(&output_user_addr);
    }

    return status;
} /* _VX_KERNEL_FUNC_KERNEL() */

static vx_param_description_t vxSelectKernelParam[] =
{
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
};

vx_status VX_CALLBACK vxSelectInitializer
    (
    vx_node nodObj,
    const vx_reference *paramObj,
    vx_uint32 paraNum
    )
{
    vsi_status status = VX_SUCCESS;
    // Alignment with a power of two value.
#define gcmALIGN(n, align) ((n) + ((align) - 1)) & ~((align) - 1)
    vx_kernel_execution_parameters_t shaderParam = {
        3,          // workdim
        {0, 0, 0},  // globalWorkOffset: control the start location be processed in the image
        {0, 0, 0},  // globalWorkScale: how many pixels could be processed by a single thread
        {0, 0, 0},  // localWorkSize: local group size in thread
        {0, 0, 0}}; // globalWorkSize: image size in thread

    vx_tensor     input0          = (vx_tensor)paramObj[1];
    vx_tensor     output          = (vx_tensor)paramObj[3];

    uint32_t      input_size[DIM_SIZE]   = {0};
    uint32_t      input_dims      = 0;
    uint32_t      output_dims     = 0;
    uint32_t      zAx             = 1;
    vsi_nn_type_e inputDataFormat = VSI_NN_TYPE_FLOAT16;
    vsi_nn_type_e outputDataFormat = VSI_NN_TYPE_FLOAT16;
    //vx_uint32 factor = 1;
    //vx_uint32 maxWorkGroupSize = 8;

    status  = vxQueryTensor(input0, VX_TENSOR_DIMS, input_size, sizeof(input_size));
    status |= vxQueryTensor(input0, VX_TENSOR_NUM_OF_DIMS, &input_dims, sizeof(input_dims));
    status |= vxQueryTensor(input0, VX_TENSOR_DATA_TYPE, &inputDataFormat, sizeof(inputDataFormat));
    status |= vxQueryTensor(output, VX_TENSOR_NUM_OF_DIMS, &output_dims, sizeof(output_dims));
    status |= vxQueryTensor(output, VX_TENSOR_DATA_TYPE, &outputDataFormat, sizeof(outputDataFormat));
    if(VX_SUCCESS != status)
    {
        VSILOGE("[%s : %d]Initializer  failure! \n",__FILE__, __LINE__);
        return status;
    }
    if(input_dims == 4)
        zAx = input_size[3] * input_size[2];
    else if(input_dims == 3)
        zAx = input_size[2];

    shaderParam.globalWorkOffset[0] = 0;
    shaderParam.globalWorkOffset[1] = 0;
    shaderParam.globalWorkOffset[2] = 0;
    shaderParam.globalWorkScale[0]  = 8;
    shaderParam.globalWorkScale[1]  = 1;
    shaderParam.globalWorkScale[2]  = 1;
    shaderParam.localWorkSize[0]    = 4;
    shaderParam.localWorkSize[1]    = 2;
    shaderParam.localWorkSize[2]    = 1;
    shaderParam.globalWorkSize[0]   = gcmALIGN((input_size[0] + shaderParam.globalWorkScale[0] - 1)
        / shaderParam.globalWorkScale[0], shaderParam.localWorkSize[0]);
    shaderParam.globalWorkSize[1]   = gcmALIGN((input_size[1] + shaderParam.globalWorkScale[1] - 1)
        / shaderParam.globalWorkScale[1], shaderParam.localWorkSize[1]);
    shaderParam.globalWorkSize[2]   = gcmALIGN((zAx + shaderParam.globalWorkScale[2] - 1)
        / shaderParam.globalWorkScale[2], shaderParam.localWorkSize[2]);
    if(inputDataFormat == VSI_NN_TYPE_INT8 || inputDataFormat == VSI_NN_TYPE_UINT8)
    {
        shaderParam.globalWorkScale[0]  = 16;
        shaderParam.globalWorkSize[0]   = gcmALIGN((input_size[0] + shaderParam.globalWorkScale[0] - 1)
        / shaderParam.globalWorkScale[0], shaderParam.localWorkSize[0]);
    }

    status |= vxSetNodeAttribute(nodObj, VX_NODE_ATTRIBUTE_KERNEL_EXECUTION_PARAMETERS,
        &shaderParam, sizeof(vx_kernel_execution_parameters_t));

    if(status < 0)
    {
        VSILOGE("[%s : %d]Initializer  failure! \n",__FILE__, __LINE__);
    }
    return status;
}


#ifdef __cpluplus
extern "C" {
#endif
vx_kernel_description_t vxSelect_CPU =
{
    _VX_KERNEL_ID,
    VX_KERNEL_NAME_SELECT_INT8,
    _VX_KERNEL_FUNC_KERNEL,
    vxSelectKernelParam,
    _cnt_of_array( vxSelectKernelParam ),
    vsi_nn_KernelValidator,
    NULL,
    NULL,
    vsi_nn_KernelInitializer,
    vsi_nn_KernelDeinitializer
};

vx_kernel_description_t vxSelect_Int8 =
{
    _VX_KERNEL_ID,
    VX_KERNEL_NAME_SELECT_INT8,
    NULL,
    vxSelectKernelParam,
    _cnt_of_array( vxSelectKernelParam ),
    vsi_nn_KernelValidator,
    NULL,
    NULL,
    vxSelectInitializer,
    vsi_nn_KernelDeinitializer
};

vx_kernel_description_t vxSelect_Int16 =
{
    _VX_KERNEL_ID,
    VX_KERNEL_NAME_SELECT_INT16,
    NULL,
    vxSelectKernelParam,
    _cnt_of_array( vxSelectKernelParam ),
    vsi_nn_KernelValidator,
    NULL,
    NULL,
    vxSelectInitializer,
    vsi_nn_KernelDeinitializer
};

vx_kernel_description_t vxSelect_Uint8 =
{
    _VX_KERNEL_ID,
    VX_KERNEL_NAME_SELECT_UINT8,
    NULL,
    vxSelectKernelParam,
    _cnt_of_array( vxSelectKernelParam ),
    vsi_nn_KernelValidator,
    NULL,
    NULL,
    vxSelectInitializer,
    vsi_nn_KernelDeinitializer
};

vx_kernel_description_t * vx_kernel_SELECT_list[] =
{
    &vxSelect_CPU,
    &vxSelect_Int8,
    &vxSelect_Int16,
    &vxSelect_Uint8,
    NULL
};
#ifdef __cpluplus
}
#endif
