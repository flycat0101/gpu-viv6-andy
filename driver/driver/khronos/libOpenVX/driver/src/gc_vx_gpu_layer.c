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


#include <gc_vx_common.h>
#if gcdUSE_VXC_BINARY
#include "nngpu_binary_interface.h"
#endif
#include <gc_vx_layer.h>
#include <gc_vx_nn_util.h>

#define IMG_MAX_WIDTH 65536
#define _vxcFILENAME_MAX 1024

#if gcdUSE_VXC_BINARY
static void * getGPUKernelInfo(vx_context context, nngpu_kernel_enum type, vx_uint32_ptr len)
{
    gceSTATUS status = gcvSTATUS_OK;

    GetBinaryPtr_FUNC funcHandle = VX_NULL;
    status = gcoOS_GetProcAddress(gcvNULL, context->libNNGPUKernelHandle, "GetBinaryPtr", &funcHandle);
    if (status != gcvSTATUS_OK)
    {
        vxError("Can't get binary pointer!\n");
        return VX_NULL;
    }

    void *ptr = funcHandle(type, len);

    return ptr;

}
#endif
extern vx_char* loadSources(const vx_char *filename, vx_size *programSize);
extern vx_status getFilePath(const char subfix[], char path[]);

/********gpuAvgPooling****************************************************/
vxnne_shader_executable vxnneGetGPUAvgPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x,
    vx_scalar               stride_y,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pool_pad_x_left,
    vx_uint32               pool_pad_y_top,
    vx_uint32               pool_pad_x_right,
    vx_uint32               pool_pad_y_bottom,
    vx_scalar               rounding,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size                          programLength    = 0;
#endif
    vx_program                       program = VX_NULL;
    vx_status                        status = VX_FAILURE;
    vxnne_shader_executable          shaderExecutable = VX_NULL;
    vxnne_kernel_shaders             kernel;
    char *                           programSources = NULL;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    vx_enum      inputFormat        = input->tensorBuffer->dataFormat;
    vx_enum      outputFormat       = output->tensorBuffer->dataFormat;
    vx_uint32    in_width           = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    in_height          = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth              = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32    out_width          = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32    out_height         = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_int32     filterValue[2]     = {poolSizeX->value->u32, poolSizeY->value->u32};
    vx_int32     strideValue[2]     = {stride_x->value->u32, stride_y->value->u32};
    vx_uint32    height             = (out_height - 1) * strideValue[1] + filterValue[1] - pool_pad_y_top - pool_pad_y_bottom;
    vx_uint32    width              = (out_width - 1) * strideValue[0] + filterValue[0] - pool_pad_x_left - pool_pad_x_right;
    vx_uint32    maxWorkGroupSize   = 8;

    if (pool_pad_x_left != 0 || pool_pad_y_top != 0
        || pool_pad_x_right != 0 || pool_pad_y_bottom != 0
        || height != in_height || width != in_width)
    {
        borderMode->mode = VX_BORDER_CONSTANT;
        if (inputFormat == VX_TYPE_UINT8)
        {
            borderMode->constant_value.U8 = 0;
        }
        else if (inputFormat == VX_TYPE_FLOAT32)
        {
            borderMode->constant_value.S32 = 0;
        }
    }
    else
    {
        borderMode->mode = VX_BORDER_REPLICATE;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, AvgPooling, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        if ((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16))
        {
            vxmONERROR(getFilePath("nngpu_kernels/AvgPooling.vx", path));

            vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

            vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

            if(programSources)
            {
                vxFree(programSources);
                programSources = NULL;
            }
        }
        else
        {
            vxError("data format not support %s line %d", __FUNCTION__, __LINE__);
            goto OnError;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuPooling", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (pool_pad_x_left == 0 && pool_pad_x_right == 0 && pool_pad_y_top == 0 && pool_pad_y_bottom == 0)
    {
        if(inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            vx_scalar scaleIn, scaleOut, zeroPointIn, zeroPointOut;
            vx_float32 outScaleValue = (vx_float32)1.0/TENSOR_TF_SCALE(output);
            vx_reference parameters[10] = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadQuant8", borderMode);
            if (!shaderExecutable) goto OnError;

            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &input->scale);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                goto OnError;
            }

            scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outScaleValue);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                goto OnError;
            }

            zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &input->zeroPoint);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                goto OnError;
            }

            zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &output->zeroPoint);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }

            parameters[1] = (vx_reference)poolSizeX;
            parameters[2] = (vx_reference)poolSizeY;
            parameters[3] = (vx_reference)stride_x;
            parameters[4] = (vx_reference)stride_y;
            parameters[5] = (vx_reference)scaleIn;
            parameters[6] = (vx_reference)scaleOut;
            parameters[7] = (vx_reference)zeroPointIn;
            parameters[8] = (vx_reference)zeroPointOut;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);

            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
            if(zeroPointOut) vxReleaseScalar(&zeroPointOut);

            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            vx_reference parameters[6] = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, VX_NULL, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadFP32", borderMode);
            if (!shaderExecutable) goto OnError;

            parameters[1] = (vx_reference)poolSizeX;
            parameters[2] = (vx_reference)poolSizeY;
            parameters[3] = (vx_reference)stride_x;
            parameters[4] = (vx_reference)stride_y;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }
    else
    {
         if(inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            vx_scalar scaleIn, scaleOut, zeroPointIn, zeroPointOut, padX, padY;
            vx_float32 outScaleValue = (vx_float32)1.0/TENSOR_TF_SCALE(output);
            vx_reference parameters[12] = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgPadQuant8", borderMode);
            if (!shaderExecutable) goto OnError;

            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &input->scale);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                goto OnError;
            }

            scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outScaleValue);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                goto OnError;
            }

            zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &input->zeroPoint);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                goto OnError;
            }

            zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &output->zeroPoint);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }

            padX = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_x_left);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                if(padX) vxReleaseScalar(&padX);
                goto OnError;
            }

            padY = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_y_top);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                goto OnError;
            }

            parameters[1]  = (vx_reference)poolSizeX;
            parameters[2]  = (vx_reference)poolSizeY;
            parameters[3]  = (vx_reference)stride_x;
            parameters[4]  = (vx_reference)stride_y;
            parameters[5]  = (vx_reference)scaleIn;
            parameters[6]  = (vx_reference)scaleOut;
            parameters[7]  = (vx_reference)zeroPointIn;
            parameters[8]  = (vx_reference)zeroPointOut;
            parameters[9]  = (vx_reference)padX;
            parameters[10] = (vx_reference)padY;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 12);

            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
            if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
            if(padX) vxReleaseScalar(&padX);
            if(padY) vxReleaseScalar(&padY);

            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            vx_reference parameters[8] = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, (vx_reference)output};
            vx_scalar padX, padY;

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgPadFP32", borderMode);
            if (!shaderExecutable) goto OnError;

            padX = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_x_left);
            if (status != VX_SUCCESS)
            {
                if(padX) vxReleaseScalar(&padX);
                goto OnError;
            }

            padY = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_y_top);
            if (status != VX_SUCCESS)
            {
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                goto OnError;
            }

            parameters[1] = (vx_reference)poolSizeX;
            parameters[2] = (vx_reference)poolSizeY;
            parameters[3] = (vx_reference)stride_x;
            parameters[4] = (vx_reference)stride_y;
            parameters[5] = (vx_reference)padX;
            parameters[6] = (vx_reference)padY;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
            if(padX) vxReleaseScalar(&padX);
            if(padY) vxReleaseScalar(&padY);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

    status = vxnneShaderExecutable_GetMaxWorkGroupSize(shaderExecutable, &maxWorkGroupSize);
    if (status != VX_SUCCESS) goto OnError;

    execution_parameters.workDim = 3;
    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkOffset[2] = 0;
    execution_parameters.localWorkSize[0]    = 0;
    execution_parameters.localWorkSize[1]    = 0;
    execution_parameters.localWorkSize[2]    = 0;
    execution_parameters.globalWorkSize[0]   = out_width;
    execution_parameters.globalWorkSize[1]   = out_height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/* tensor copy */
vxnne_shader_executable vxnneGPUTensorCopyShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_enum      outputFormat      = output->tensorBuffer->dataFormat;
    vx_uint32    dimCount          = TENSOR_VIEW_DIM_NUM(input);
    vx_uint32    width             = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    height            = (dimCount > 1) ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
    vx_uint32    depth             = (dimCount > 1) ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_uint32    batch             = (dimCount > 1) ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_uint32    input_dims        = TENSOR_DIM_NUM(input) == 1 ? 2 : TENSOR_DIM_NUM(input);
    vx_tensor    input_rs          = NULL;
    vx_tensor    output_rs         = NULL;
    vx_int32     sizes[4]          = {1, 1, 1, batch};
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_REPLICATE;
    sizes[0]  = width;
    sizes[1]  = height;
    sizes[2]  = depth;
    input_rs  = vxoTensor_ReshapeTensor(input, sizes, input_dims);
    output_rs = vxoTensor_ReshapeTensor(output, sizes, input_dims);

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorCopy, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorCopy.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if(programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorCopy", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {
        vx_reference parameters[2] = {(vx_reference)input_rs, (vx_reference)output_rs};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_reference parameters[2] = {(vx_reference)input_rs, (vx_reference)output_rs};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
    {
        vx_float32 scaleValue = TENSOR_TF_SCALE(input);
        vx_int32 zpValue = TENSOR_TF_ZEROPOINT(input);
        vx_scalar scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);
        vx_scalar zp = vxCreateScalar(context, VX_TYPE_INT32, &zpValue);
        vx_reference parameters[4] = {(vx_reference)input_rs, (vx_reference)scale, (vx_reference)zp, (vx_reference)output_rs};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Q8toFP32", borderMode);
        if (!shaderExecutable)
        {
            if(zp) vxReleaseScalar(&zp);
            if(scale) vxReleaseScalar(&scale);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if(zp) vxReleaseScalar(&zp);
        if(scale) vxReleaseScalar(&scale);
        if (status != VX_SUCCESS) goto OnError;
    }

    execution_parameters.localWorkSize[0]    = 0;
    execution_parameters.localWorkSize[1]    = 0;
    execution_parameters.localWorkSize[2]    = 0;
    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/************************gpuTensorTranspose************************************/
vxnne_shader_executable vxnneGPUTensorTransposeShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               *perm,
    vx_uint32               pnum,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable   = VX_NULL;
    vxnne_kernel_shaders    kernel             = VX_NULL;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_uint32    dims              = TENSOR_DIM_NUM(input);
    vx_uint32    width             = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    height            = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth             = (dims > 2) ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_uint32    tmp               = 0;
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_REPLICATE;
    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);
    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorTranspose, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorTranspose.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if(programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTranspose", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if(inputFormat == VX_TYPE_FLOAT16 && pnum == 3)
    {
        vx_reference parameters[2] = {(vx_reference)input, (vx_reference)output};

        if (perm[0] == 2 && perm[1] == 0 && perm[2] == 1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2CWH", borderMode);
        }
        else if (perm[0] == 2 && perm[1] == 1 && perm[2] == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2CHW", borderMode);
        }
        else if (perm[0] == 0 && perm[1] == 2 && perm[2] == 1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2WCH", borderMode);
        }
        else if (perm[0] == 1 && perm[1] == 2 && perm[2] == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2HCW", borderMode);
        }
        else if (perm[0] == 1 && perm[1] == 0 && perm[2] == 2)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_CHW2CWH", borderMode);
            tmp = width;
            width = height;
            height = tmp;
        }

        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
        if (status != VX_SUCCESS) goto OnError;

    }
    else if(inputFormat == VX_TYPE_UINT8 && pnum == 3)
    {
        vx_reference parameters[2] = {(vx_reference)input, (vx_reference)output};

        if (perm[0] == 2 && perm[1] == 0 && perm[2] == 1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2CWH", borderMode);
        }
        else if (perm[0] == 2 && perm[1] == 1 && perm[2] == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2CHW", borderMode);
        }
        else if (perm[0] == 0 && perm[1] == 2 && perm[2] == 1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2WCH", borderMode);
        }
        else if (perm[0] == 1 && perm[1] == 2 && perm[2] == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2HCW", borderMode);
        }
        else if (perm[0] == 1 && perm[1] == 0 && perm[2] == 2)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_CHW2CWH", borderMode);
            tmp = width;
            width = height;
            height = tmp;
        }

        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
        if (status != VX_SUCCESS) goto OnError;
    }

    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkOffset[2] = 0;

    execution_parameters.globalWorkSize[0]    = width;
    execution_parameters.globalWorkSize[1]    = height;
    execution_parameters.globalWorkSize[2]    = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuEmbeddingLUT****************************************/
vxnne_shader_executable vxnneGetGPUEmbeddingLUTShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               value,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum       valueFormat                = value->tensorBuffer->dataFormat;
    vx_enum       outputFormat               = output->tensorBuffer->dataFormat;
    vx_uint32     dims                       = TENSOR_DIM_NUM(input);
    vx_uint32     width                      = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     input_count                = 0;
    vx_uint32     vh                         = TENSOR_VIEW_SIZE_INDEX(value, 1);
    vx_uint32     vw                         = TENSOR_VIEW_SIZE_INDEX(value, 0);
    vx_tensor     input_rs                   = NULL;
    vx_int32      rs_sizes[4]                = {1, 1, 1, 1};
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_CONSTANT;
    borderMode->constant_value.U8 = 0;
    borderMode->constant_value.S16 = 0;

    if (!((valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        || (valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        ))
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    if(dims == 1)
    {
        rs_sizes[0] = width;
        input_rs = vxoTensor_ReshapeTensor(input, rs_sizes, 2);
    }

    input_count = rs_sizes[0];

    execution_parameters.globalWorkSize[0]   = vw;
    execution_parameters.globalWorkSize[1]   = vh;
    execution_parameters.globalWorkSize[2]   = input_count;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, EmbeddingLUT, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/EmbeddingLUT.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if(programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuEmbeddingLUT", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if(valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_float32 scaleValue = TENSOR_TF_SCALE(value)/TENSOR_TF_SCALE(output);
        vx_int32 zeroPointInValue = TENSOR_TF_ZEROPOINT(value);
        vx_int32 zeroPointOutValue = TENSOR_TF_ZEROPOINT(output);
        vx_scalar zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointInValue);
        vx_scalar zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointOutValue);
        vx_scalar scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);
        vx_reference  parameters[6] = {(vx_reference)input_rs, (vx_reference)value, (vx_reference)scale, (vx_reference)zeroPointIn, (vx_reference)zeroPointOut, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_quant8", borderMode);
        if (!shaderExecutable)
        {
            if(scale) vxReleaseScalar(&scale);
            if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
            if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
        if(scale) vxReleaseScalar(&scale);
        if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
        if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
        if (!shaderExecutable) goto OnError;
    }
    else if(valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {
        vx_reference  parameters[3] = {(vx_reference)input_rs, (vx_reference)value, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
        if (status != VX_SUCCESS) goto OnError;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(input_rs) vxoTensor_ReleaseTensor(&input_rs);

    return shaderExecutable;

OnError:
    if(input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuTensor2Row****************************************/
vxnne_shader_executable vxnneGPUTensor2RowShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_int32                kernelSize_x,
    vx_int32                kernelSize_y,
    vx_int32                dilation_x,
    vx_int32                dilation_y,
    vx_int32                stride_x,
    vx_int32                stride_y,
    vx_int32                padding_x,
    vx_int32                padding_y,
    vx_int32                outputWidth,
    vx_int32                outputHeight,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
#endif
    vx_program program          = VX_NULL;
    vx_status  status           = VX_FAILURE;
    vx_uint32  channels         = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    char *programSources = NULL;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;

    borderMode->mode = VX_BORDER_CONSTANT;
    if (inputFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        borderMode->constant_value.U8 = (vx_uint8)TENSOR_TF_ZEROPOINT(input);
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Tensor2Row, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Tensor2Row.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if(programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif
         status = vxGetStatus((vx_reference)program);
         if (status != VX_SUCCESS) goto OnError;

         status = vxBuildProgram(program, VX_NULL);
         if (status != VX_SUCCESS) goto OnError;

         kernel = vxnneAddKernelShadersInProgram(context, "gpuTensor2Row", program, 0, kernelEnum);
         if (!kernel) goto OnError;

         vxReleaseProgram(&program);
    }

    if(inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_UINT8)
    {
        vx_scalar strideX = vxCreateScalar(context, VX_TYPE_INT32, &stride_x);
        vx_scalar strideY = vxCreateScalar(context, VX_TYPE_INT32, &stride_y);
        vx_scalar padX = vxCreateScalar(context, VX_TYPE_INT32, &padding_x);
        vx_scalar padY = vxCreateScalar(context, VX_TYPE_INT32, &padding_y);
        vx_scalar kernelX = vxCreateScalar(context, VX_TYPE_INT32, &kernelSize_x);
        vx_scalar kernelY = vxCreateScalar(context, VX_TYPE_INT32, &kernelSize_y);
        vx_scalar dilateX = vxCreateScalar(context, VX_TYPE_INT32, &dilation_x);
        vx_scalar dilateY = vxCreateScalar(context, VX_TYPE_INT32, &dilation_y);
        vx_reference parameters[10] = {(vx_reference)input, (vx_reference)strideX, (vx_reference)strideY, (vx_reference)padX, (vx_reference)padY,
            (vx_reference)kernelX, (vx_reference)kernelY, (vx_reference)dilateX, (vx_reference)dilateY, (vx_reference)output};

        if (inputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            if (!shaderExecutable)
            {
                if(strideX) vxReleaseScalar(&strideX);
                if(strideY) vxReleaseScalar(&strideY);
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                if(kernelX) vxReleaseScalar(&kernelX);
                if(kernelY) vxReleaseScalar(&kernelY);
                if(dilateX) vxReleaseScalar(&dilateX);
                if(dilateY) vxReleaseScalar(&dilateY);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);

            if (status != VX_SUCCESS)
            {
                if(strideX) vxReleaseScalar(&strideX);
                if(strideY) vxReleaseScalar(&strideY);
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                if(kernelX) vxReleaseScalar(&kernelX);
                if(kernelY) vxReleaseScalar(&kernelY);
                if(dilateX) vxReleaseScalar(&dilateX);
                if(dilateY) vxReleaseScalar(&dilateY);
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_UINT8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            if (!shaderExecutable)
            {
                if(strideX) vxReleaseScalar(&strideX);
                if(strideY) vxReleaseScalar(&strideY);
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                if(kernelX) vxReleaseScalar(&kernelX);
                if(kernelY) vxReleaseScalar(&kernelY);
                if(dilateX) vxReleaseScalar(&dilateX);
                if(dilateY) vxReleaseScalar(&dilateY);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);

            if (status != VX_SUCCESS)
            {
                if(strideX) vxReleaseScalar(&strideX);
                if(strideY) vxReleaseScalar(&strideY);
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                if(kernelX) vxReleaseScalar(&kernelX);
                if(kernelY) vxReleaseScalar(&kernelY);
                if(dilateX) vxReleaseScalar(&dilateX);
                if(dilateY) vxReleaseScalar(&dilateY);
                goto OnError;
            }
        }

        if(strideX) vxReleaseScalar(&strideX);
        if(strideY) vxReleaseScalar(&strideY);
        if(padX) vxReleaseScalar(&padX);
        if(padY) vxReleaseScalar(&padY);
        if(kernelX) vxReleaseScalar(&kernelX);
        if(kernelY) vxReleaseScalar(&kernelY);
        if(dilateX) vxReleaseScalar(&dilateX);
        if(dilateY) vxReleaseScalar(&dilateY);
    }

    execution_parameters.workDim             = 3;
    execution_parameters.globalWorkSize[0]   = outputWidth;
    execution_parameters.globalWorkSize[1]   = outputHeight;
    execution_parameters.globalWorkSize[2]   = channels;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuGEMM_nobias****************************************/
vxnne_shader_executable vxnneGPUGemm_noBiasShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weight,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program          = VX_NULL;
    vx_status  status           = VX_FAILURE;
    vx_uint32  inputSize        = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32  width            = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32  height           = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_uint32  depth            = TENSOR_VIEW_SIZE_INDEX(output, 2);
    vx_uint32  dims             = TENSOR_DIM_NUM(output) == 1 ? 2 : TENSOR_DIM_NUM(output);
    vx_uint32  kernel_x         = TENSOR_VIEW_SIZE_INDEX(weight, 0);
    vx_uint32  kernel_y         = TENSOR_VIEW_SIZE_INDEX(weight, 1);
    vx_uint32  ifm              = TENSOR_VIEW_SIZE_INDEX(weight, 2);
    vx_uint32  ofm              = TENSOR_VIEW_SIZE_INDEX(weight, 3);
    vx_tensor  weights          = NULL;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_uint32  input_ZP         = TENSOR_TF_ZEROPOINT(input);
    vx_uint32  weight_ZP        = TENSOR_TF_ZEROPOINT(weight);
    vx_uint32  output_ZP        = TENSOR_TF_ZEROPOINT(output);
    vx_float32 input_scale      = TENSOR_TF_SCALE(input);
    vx_float32 weight_scale     = TENSOR_TF_SCALE(weight);
    vx_float32 output_scale     = (vx_float32)1.0/TENSOR_TF_SCALE(output);
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_int32     size[]            = {1, 1, 1, 1};
    char *programSources = NULL;
    vx_scalar cycle = NULL;

    size[0] = kernel_x * kernel_y * ifm;
    size[1] = ofm;
    dims    = TENSOR_DIM_NUM(weight) == 1 ? 2 : TENSOR_DIM_NUM(weight);
    weights = vxoTensor_ReshapeTensor(weight, size, dims);

    borderMode->mode = VX_BORDER_CONSTANT;
    if (inputFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        borderMode->constant_value.U8 = (vx_uint8)input_ZP;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Gemm_noBias, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Gemm_noBias.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if(programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuGemm_noBias", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    cycle = vxCreateScalar(context, VX_TYPE_INT32, &inputSize);

    if (inputFormat == VX_TYPE_FLOAT16)
    {
        vx_reference parameters[4]     = {(vx_reference)input, (vx_reference)weights, (vx_reference)cycle, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &input_scale);
        vx_scalar scaleWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weight_scale);
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &output_scale);
        vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        vx_scalar zpWeight = vxCreateScalar(context, VX_TYPE_INT32, &weight_ZP);
        vx_scalar zpOut = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        vx_reference parameters[10]     = {(vx_reference)input, (vx_reference)weights, (vx_reference)cycle, (vx_reference)scaleIn,
                                            (vx_reference)scaleWeight, (vx_reference)scaleOut, (vx_reference)zpIn,
                                            (vx_reference)zpWeight, (vx_reference)zpOut, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable)
        {
            if (scaleIn) vxReleaseScalar(&scaleIn);
            if (scaleWeight) vxReleaseScalar(&scaleWeight);
            if (scaleOut) vxReleaseScalar(&scaleOut);
            if (zpIn) vxReleaseScalar(&zpIn);
            if (zpWeight) vxReleaseScalar(&zpWeight);
            if (zpOut) vxReleaseScalar(&zpOut);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);

        if (scaleIn) vxReleaseScalar(&scaleIn);
        if (scaleWeight) vxReleaseScalar(&scaleWeight);
        if (scaleOut) vxReleaseScalar(&scaleOut);
        if (zpIn) vxReleaseScalar(&zpIn);
        if (zpWeight) vxReleaseScalar(&zpWeight);
        if (zpOut) vxReleaseScalar(&zpOut);

        if (status != VX_SUCCESS) goto OnError;
    }

    execution_parameters.workDim             = 3;
    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (cycle) vxReleaseScalar(&cycle);

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (cycle) vxReleaseScalar(&cycle);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
    return VX_NULL;
}

/********gpuGEMM**********************************************/
vxnne_shader_executable vxnneGPUGemmShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weight,
    vx_tensor               bias,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program          = VX_NULL;
    vx_status  status           = VX_FAILURE;
    vx_uint32  inputSize        = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32  width            = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32  height           = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_uint32  depth            = TENSOR_VIEW_SIZE_INDEX(output, 2);
    vx_uint32  dims             = TENSOR_DIM_NUM(output) == 1 ? 2 : TENSOR_DIM_NUM(output);
    vx_tensor  outputs          = NULL;
    vx_uint32  kernel_x         = TENSOR_VIEW_SIZE_INDEX(weight, 0);
    vx_uint32  kernel_y         = TENSOR_VIEW_SIZE_INDEX(weight, 1);
    vx_uint32  ifm              = TENSOR_VIEW_SIZE_INDEX(weight, 2);
    vx_uint32  ofm              = TENSOR_VIEW_SIZE_INDEX(weight, 3);
    vx_tensor  weights          = NULL;
    vx_tensor  biases           = NULL;
    vx_scalar  cycle           = NULL;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_uint32  input_ZP         = TENSOR_TF_ZEROPOINT(input);
    vx_uint32  weight_ZP        = TENSOR_TF_ZEROPOINT(weight);
    vx_uint32  output_ZP        = TENSOR_TF_ZEROPOINT(output);
    vx_float32 input_scale      = TENSOR_TF_SCALE(input);
    vx_float32 weight_scale     = TENSOR_TF_SCALE(weight);
    vx_float32 output_scale     = (vx_float32)1.0/TENSOR_TF_SCALE(output);

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_int32     size[]            = {1, 1, 1, 1};
    char *programSources = NULL;

    size[0] = kernel_x * kernel_y * ifm;
    size[1] = ofm;
    dims    = TENSOR_DIM_NUM(weight) == 1 ? 2 : TENSOR_DIM_NUM(weight);
    weights = vxoTensor_ReshapeTensor(weight, size, dims);

    size[0] = ofm;
    size[1] = 1;
    dims    = TENSOR_DIM_NUM(bias) == 1 ? 2 : TENSOR_DIM_NUM(bias);
    biases  = vxoTensor_ReshapeTensor(bias, size, dims);

    borderMode->mode = VX_BORDER_CONSTANT;
    if (inputFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        borderMode->constant_value.U8 = (vx_uint8)input_ZP;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Gemm, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Gemm.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if(programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif
     status = vxGetStatus((vx_reference)program);
     if (status != VX_SUCCESS) goto OnError;

     status = vxBuildProgram(program, VX_NULL);
     if (status != VX_SUCCESS) goto OnError;

     kernel = vxnneAddKernelShadersInProgram(context, "gpuGemm", program, 0, kernelEnum);
     if (!kernel) goto OnError;

     vxReleaseProgram(&program);
    }

    cycle = vxCreateScalar(context, VX_TYPE_INT32, &inputSize);

    if (inputFormat == VX_TYPE_FLOAT16)
    {
        vx_reference parameters[5]     = {(vx_reference)input, (vx_reference)weights, (vx_reference)biases, (vx_reference)cycle, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &input_scale);
        vx_scalar scaleWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weight_scale);
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &output_scale);
        vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        vx_scalar zpWeight = vxCreateScalar(context, VX_TYPE_INT32, &weight_ZP);
        vx_scalar zpOut = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        vx_reference parameters[11] = {(vx_reference)input, (vx_reference)weights, (vx_reference)biases, (vx_reference)cycle,
                                        (vx_reference)scaleIn, (vx_reference)scaleWeight, (vx_reference)scaleOut, (vx_reference)zpIn,
                                        (vx_reference)zpWeight, (vx_reference)zpOut, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable)
        {
            if (scaleIn) vxReleaseScalar(&scaleIn);
            if (scaleWeight) vxReleaseScalar(&scaleWeight);
            if (scaleOut) vxReleaseScalar(&scaleOut);
            if (zpIn) vxReleaseScalar(&zpIn);
            if (zpWeight) vxReleaseScalar(&zpWeight);
            if (zpOut) vxReleaseScalar(&zpOut);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 11);

        if (scaleIn) vxReleaseScalar(&scaleIn);
        if (scaleWeight) vxReleaseScalar(&scaleWeight);
        if (scaleOut) vxReleaseScalar(&scaleOut);
        if (zpIn) vxReleaseScalar(&zpIn);
        if (zpWeight) vxReleaseScalar(&zpWeight);
        if (zpOut) vxReleaseScalar(&zpOut);

        if (status != VX_SUCCESS) goto OnError;
    }

    execution_parameters.globalWorkSize[0] = width;
    execution_parameters.globalWorkSize[1] = height;
    execution_parameters.globalWorkSize[2] = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (biases) vxoTensor_ReleaseTensor(&biases);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (cycle) vxReleaseScalar(&cycle);

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (biases) vxoTensor_ReleaseTensor(&biases);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (cycle) vxReleaseScalar(&cycle);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
    return VX_NULL;
}

/********gpuDepthwiseConvolution****************************************/
vxnne_shader_executable vxnneGetGPUDepthwiseConvShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_tensor               weights,
    vx_tensor               biases,
    vx_scalar               padXLeft,
    vx_scalar               padXRight,
    vx_scalar               padYTop,
    vx_scalar               padYBottom,
    vx_scalar               dilationX,
    vx_scalar               dilationY,
    vx_scalar               channel_multiplier,
    vx_scalar               downScaleSizeRounding,
    vx_tensor               outputs)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program                       program              = VX_NULL;
    vx_status                        status               = VX_FAILURE;
    vxnne_shader_executable          shaderExecutable     = VX_NULL;
    vxnne_kernel_shaders             kernel               = NULL;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    vx_uint32     input_width                = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32     input_height               = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_uint32     output_width               = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    vx_uint32     output_height              = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
    vx_uint32     output_depth               = TENSOR_VIEW_SIZE_INDEX(outputs, 2);
    vx_enum       inputFormat                = inputs->tensorBuffer->dataFormat;
    vx_int32      kernel_width               = TENSOR_VIEW_SIZE_INDEX(weights, 0);
    vx_int32      kernel_height              = TENSOR_VIEW_SIZE_INDEX(weights, 1);
    vx_int32      padLeftv                   = padXLeft->value->n32;
    vx_int32      padRightv                  = padXRight->value->n32;
    vx_int32      padTopv                    = padYTop->value->n32;
    vx_int32      padBottomv                 = padYBottom->value->n32;
    vx_uint32     inputZP                    = TENSOR_TF_ZEROPOINT(inputs);
    vx_float32    inputScale                 = TENSOR_TF_SCALE(inputs);
    vx_uint32     weightZP                   = TENSOR_TF_ZEROPOINT(weights);
    vx_float32    weightScale                = TENSOR_TF_SCALE(weights);
    vx_uint32     biasZP                     = TENSOR_TF_ZEROPOINT(biases);
    vx_float32    biasScale                  = TENSOR_TF_SCALE(biases);
    vx_uint32     outputZP                   = TENSOR_TF_ZEROPOINT(outputs);
    vx_float32    outputScale                = (vx_float32)1.0/TENSOR_TF_SCALE(outputs);
    vx_scalar     strideX                    = VX_NULL;
    vx_scalar     strideY                    = VX_NULL;
    vx_scalar     kernelX                     = vxCreateScalar(context, VX_TYPE_INT32, &kernel_width);
    vx_scalar     kernelY                    = vxCreateScalar(context, VX_TYPE_INT32, &kernel_height);
    vx_int32      strideXvalue               = 0;
    vx_int32      strideYvalue               = 0;
    vx_tensor     reBiases                   = VX_NULL;
    char *programSources = NULL;

    if (fabs(inputScale*weightScale - biasScale) > 0.000001f || biasZP !=0)
    {
        vxError("biase's scale must equal to input's scale multiply weight's scale, and biase's zero point must equal to 0! failed at function %s line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }

    if ((input_width == 1 && input_height ==1) || (output_width == 1 && input_width == (vx_uint32)kernel_width && padLeftv == 0))
        strideXvalue = strideYvalue = 1;
    else
    {
        strideXvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_width + padLeftv + padRightv - kernel_width) / (output_width - 1), downScaleSizeRounding->value->e);
        strideYvalue = vxoNNExternsionConvlutionRound((vx_float32)(input_height + padTopv + padBottomv - kernel_height) / (output_height - 1), downScaleSizeRounding->value->e);
    }
    strideX = vxCreateScalar(context, VX_TYPE_INT32, &strideXvalue);
    strideY = vxCreateScalar(context, VX_TYPE_INT32, &strideYvalue);

    if (biases != VX_NULL)
    {
        vx_uint32 bias_dims = TENSOR_DIM_NUM(biases);
        vx_uint32 bias_w    = TENSOR_VIEW_SIZE_INDEX(biases, 0);
        vx_uint32 bias_h    = (bias_dims > 1) ? TENSOR_VIEW_SIZE_INDEX(biases, 1) : 1;
        vx_uint32 bias_c    = (bias_dims > 2) ? TENSOR_VIEW_SIZE_INDEX(biases, 2) : 1;
        vx_uint32 bias_b    = (bias_dims > 3) ? TENSOR_VIEW_SIZE_INDEX(biases, 3) : 1;
        vx_int32  reSize[4] = {bias_w, bias_h, bias_c, bias_b};

        bias_dims = (bias_dims == 1) ? 2 : bias_dims;
        reBiases  = vxoTensor_ReshapeTensor(biases, reSize, bias_dims);
    }

    borderMode->mode = VX_BORDER_CONSTANT;
    if (inputFormat == VX_TYPE_FLOAT16)
        borderMode->constant_value.S16 = 0;
    else if (inputFormat == VX_TYPE_UINT8)
        borderMode->constant_value.U8 = (vx_uint8)inputZP;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, DepthwiseConv, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/DepthwiseConv.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if(programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuDepthwiseConv", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    {
        if(inputFormat == VX_TYPE_FLOAT16 && biases != VX_NULL)
        {
            vx_reference  parameters[11] = {(vx_reference)inputs, (vx_reference)weights, (vx_reference)reBiases, (vx_reference)channel_multiplier,
                                            (vx_reference)kernelX, (vx_reference)kernelY, (vx_reference)strideX, (vx_reference)strideY,
                                            (vx_reference)padXLeft, (vx_reference)padYTop, (vx_reference)outputs};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 11);
            if (status != VX_SUCCESS) goto OnError;
        }
        else if (inputFormat == VX_TYPE_UINT8 && biases != VX_NULL)
        {
            vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScale);
            vx_scalar scaleWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weightScale);
            vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScale);
            vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_INT32, &inputZP);
            vx_scalar zpWeight = vxCreateScalar(context, VX_TYPE_INT32, &weightZP);
            vx_scalar zpOut = vxCreateScalar(context, VX_TYPE_INT32, &outputZP);
            vx_reference  parameters[17] = {(vx_reference)inputs, (vx_reference)weights, (vx_reference)reBiases, (vx_reference)channel_multiplier,
                                (vx_reference)kernelX, (vx_reference)kernelY, (vx_reference)strideX, (vx_reference)strideY,
                                (vx_reference)padXLeft, (vx_reference)padYTop, (vx_reference)scaleIn, (vx_reference)scaleWeight,
                                (vx_reference)scaleOut, (vx_reference)zpIn, (vx_reference)zpWeight, (vx_reference)zpOut,(vx_reference)outputs};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            if (!shaderExecutable)
            {
                if (scaleIn) vxReleaseScalar(&scaleIn);
                if (scaleWeight) vxReleaseScalar(&scaleWeight);
                if (scaleOut) vxReleaseScalar(&scaleOut);
                if (zpIn) vxReleaseScalar(&zpIn);
                if (zpWeight) vxReleaseScalar(&zpWeight);
                if (zpOut) vxReleaseScalar(&zpOut);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 17);

            if (scaleIn) vxReleaseScalar(&scaleIn);
            if (scaleWeight) vxReleaseScalar(&scaleWeight);
            if (scaleOut) vxReleaseScalar(&scaleOut);
            if (zpIn) vxReleaseScalar(&zpIn);
            if (zpWeight) vxReleaseScalar(&zpWeight);
            if (zpOut) vxReleaseScalar(&zpOut);

            if (status != VX_SUCCESS) goto OnError;
        }
        else if(inputFormat == VX_TYPE_FLOAT16 && biases == VX_NULL)
        {
            vx_reference  parameters[10] = {(vx_reference)inputs, (vx_reference)weights, (vx_reference)channel_multiplier,
                                (vx_reference)kernelX, (vx_reference)kernelY, (vx_reference)strideX, (vx_reference)strideY,
                                (vx_reference)padXLeft, (vx_reference)padYTop, (vx_reference)outputs};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "NoBias_FP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS) goto OnError;
        }
        else if (inputFormat == VX_TYPE_UINT8 && biases == VX_NULL)
        {
            vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScale);
            vx_scalar scaleWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weightScale);
            vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScale);
            vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_INT32, &inputZP);
            vx_scalar zpWeight = vxCreateScalar(context, VX_TYPE_INT32, &weightZP);
            vx_scalar zpOut = vxCreateScalar(context, VX_TYPE_INT32, &outputZP);
            vx_reference  parameters[16] = {(vx_reference)inputs, (vx_reference)weights, (vx_reference)channel_multiplier,
                                (vx_reference)kernelX, (vx_reference)kernelY, (vx_reference)strideX, (vx_reference)strideY,
                                (vx_reference)padXLeft, (vx_reference)padYTop, (vx_reference)scaleIn, (vx_reference)scaleWeight,
                                (vx_reference)scaleOut, (vx_reference)zpIn, (vx_reference)zpWeight, (vx_reference)zpOut,(vx_reference)outputs};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "NoBias_Quant8", borderMode);
            if (!shaderExecutable)
            {
                if (scaleIn) vxReleaseScalar(&scaleIn);
                if (scaleWeight) vxReleaseScalar(&scaleWeight);
                if (scaleOut) vxReleaseScalar(&scaleOut);
                if (zpIn) vxReleaseScalar(&zpIn);
                if (zpWeight) vxReleaseScalar(&zpWeight);
                if (zpOut) vxReleaseScalar(&zpOut);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 16);

            if (scaleIn) vxReleaseScalar(&scaleIn);
            if (scaleWeight) vxReleaseScalar(&scaleWeight);
            if (scaleOut) vxReleaseScalar(&scaleOut);
            if (zpIn) vxReleaseScalar(&zpIn);
            if (zpWeight) vxReleaseScalar(&zpWeight);
            if (zpOut) vxReleaseScalar(&zpOut);

            if (status != VX_SUCCESS) goto OnError;
        }
    }

    execution_parameters.globalWorkSize[0]   = output_width;
    execution_parameters.globalWorkSize[1]   = output_height;
    execution_parameters.globalWorkSize[2]   = output_depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (reBiases) vxoTensor_ReleaseTensor(&reBiases);
    if (kernelX) vxReleaseScalar(&kernelX);
    if (kernelY) vxReleaseScalar(&kernelY);
    if (strideX) vxReleaseScalar(&strideX);
    if (strideY) vxReleaseScalar(&strideY);
    return shaderExecutable;

OnError:
    if (program)  vxReleaseProgram(&program);
    if (reBiases) vxoTensor_ReleaseTensor(&reBiases);
    if (kernelX) vxReleaseScalar(&kernelX);
    if (kernelY) vxReleaseScalar(&kernelY);
    if (strideX) vxReleaseScalar(&strideX);
    if (strideY) vxReleaseScalar(&strideY);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuReorg2 Depth2Space****************************************/
vxnne_shader_executable vxnneGetGPUDepth2SpaceShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               block_sizes,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel = NULL;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_uint32     output_width               = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32     output_height              = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_uint32     output_depth               = TENSOR_VIEW_SIZE_INDEX(output, 2);
    vx_enum       inputFormat                = input->tensorBuffer->dataFormat;
    vx_enum       outputFormat               = output->tensorBuffer->dataFormat;
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_REPLICATE;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Depth2Space, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Depth2Space.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuReorg2", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_float32 scaleValue = TENSOR_TF_SCALE(input)/TENSOR_TF_SCALE(output);
        vx_int32 zeroPointInValue = TENSOR_TF_ZEROPOINT(input);
        vx_int32 zeroPointOutValue = TENSOR_TF_ZEROPOINT(output);
        vx_scalar zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointInValue);
        vx_scalar zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointOutValue);
        vx_scalar scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);

        if(block_sizes->value->u32 == 2)
        {
            vx_reference  parameters[5] = {(vx_reference)input, (vx_reference)scale, (vx_reference)zeroPointIn, (vx_reference)zeroPointOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Depth2SpaceQuant8Block2", borderMode);
            if (!shaderExecutable)
            {
                if(scale) vxReleaseScalar(&scale);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if (status != VX_SUCCESS)
            {
                if(scale) vxReleaseScalar(&scale);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }
        }
        else
        {
            vx_reference  parameters[6] = {(vx_reference)input, (vx_reference)block_sizes, (vx_reference)scale, (vx_reference)zeroPointIn, (vx_reference)zeroPointOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Depth2SpaceQuant8", borderMode);
            if (!shaderExecutable)
            {
                if(scale) vxReleaseScalar(&scale);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS)
            {
                if(scale) vxReleaseScalar(&scale);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }
        }

        if(scale) vxReleaseScalar(&scale);
        if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
        if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
    }
    else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {
        if(block_sizes->value->u32 == 2)
        {
            vx_reference  parameters[2] = {(vx_reference)input, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Depth2SpaceFP32Block2", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_reference  parameters[3] = {(vx_reference)input, (vx_reference)block_sizes, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Depth2SpaceFP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

    execution_parameters.globalWorkSize[0]   = output_width;
    execution_parameters.globalWorkSize[1]   = output_height;
    execution_parameters.globalWorkSize[2]   = output_depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuReorg2 Space2Depth****************************************/
vxnne_shader_executable vxnneGetGPUSpace2DepthShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               stride,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum       inputFormat                = input->tensorBuffer->dataFormat;
    vx_enum       outputFormat               = output->tensorBuffer->dataFormat;
    vx_uint32     input_width                = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     input_height               = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32     input_depth                = TENSOR_VIEW_SIZE_INDEX(input, 2);
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_CONSTANT;
    borderMode->constant_value.U8 = 0;
    borderMode->constant_value.S16 = 0;

    if (!((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
       || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16))
        )
    {
        vxError("input or output's format is not support(space to depth)");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0] = input_width;
    execution_parameters.globalWorkSize[1] = input_height;
    execution_parameters.globalWorkSize[2] = input_depth;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Space2Depth, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Space2Depth.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuReorg2", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_float32 scaleValue = TENSOR_TF_SCALE(input)/TENSOR_TF_SCALE(output);
        vx_int32 zeroPointInValue = TENSOR_TF_ZEROPOINT(input);
        vx_int32 zeroPointOutValue = TENSOR_TF_ZEROPOINT(output);
        vx_scalar zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointInValue);
        vx_scalar zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointOutValue);
        vx_scalar scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);

        if (stride->value->u32 == 2)
        {
            vx_reference  parameters[5] = {(vx_reference)input, (vx_reference)scale, (vx_reference)zeroPointIn, (vx_reference)zeroPointOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Space2DepthQuant8Block2", borderMode);
            if (!shaderExecutable)
            {
                if(scale) vxReleaseScalar(&scale);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if (status != VX_SUCCESS)
            {
                if(scale) vxReleaseScalar(&scale);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }
        }
        else
        {
            vx_reference  parameters[6] = {(vx_reference)input, (vx_reference)stride, (vx_reference)scale, (vx_reference)zeroPointIn, (vx_reference)zeroPointOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Space2DepthQuant8", borderMode);
            if (!shaderExecutable)
            {
                if(scale) vxReleaseScalar(&scale);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS)
            {
                if(scale) vxReleaseScalar(&scale);
                if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
                if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
                goto OnError;
            }
        }

        if(scale) vxReleaseScalar(&scale);
        if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
        if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
    }
    else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {
        if (stride->value->u32 == 2)
        {
            vx_reference  parameters[3] = {(vx_reference)input, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Space2DepthFP32Block2", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_reference  parameters[3] = {(vx_reference)input, (vx_reference)stride, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Space2DepthFP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuFloor****************************************/
vxnne_shader_executable vxnneGetGPUFloorShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               mode,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[3]              = {(vx_reference)input, (vx_reference)mode, (vx_reference)output};
    vx_enum       inputFormat                = TENSOR_DATA_TYPE(input);
    vx_enum       outputFormat               = TENSOR_DATA_TYPE(output);
    vx_tensor     input_rs                   = NULL;
    vx_tensor     output_rs                  = NULL;
    vx_int32      in_sizes[4]                = {1, 1, 1, 1};
    vx_uint32     in_dims                    = TENSOR_DIM_NUM(input);
    vx_int32      out_sizes[4]               = {1, 1, 1, 1};
    vx_uint32     out_dims                   = TENSOR_DIM_NUM(output);
    vx_uint32     input_width                = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     input_height               = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32     input_depth                = TENSOR_VIEW_SIZE_INDEX(input, 2);
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_CONSTANT;
    borderMode->constant_value.U8 = 0;
    borderMode->constant_value.S16 = 0;

    if (!((inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT16)))
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    if (in_dims == 1)
    {
        in_sizes[0] = TENSOR_VIEW_SIZE_INDEX(input, 0);
        input_rs = vxoTensor_ReshapeTensor(input, in_sizes, 2);
        parameters[0] = (vx_reference)input_rs;
    }

    if (out_dims == 1)
    {
        out_sizes[0] = TENSOR_VIEW_SIZE_INDEX(output, 0);
        output_rs = vxoTensor_ReshapeTensor(output, out_sizes, 2);
        parameters[2] = (vx_reference)output_rs;
    }

    execution_parameters.globalWorkSize[0]   = input_width;
    execution_parameters.globalWorkSize[1]   = input_height;
    execution_parameters.globalWorkSize[2]   = input_depth;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Floor, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Floor.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program,VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuFloor", program, 3, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if ((inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT16))
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);

    return shaderExecutable;

OnError:
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuFullyConnected****************************************************/
vxnne_shader_executable vxnneGetGPUFullyConnectedShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               bias,
    vx_int32                activation,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_bool       enable_bias      = (bias == NULL) ? vx_false_e : vx_true_e;
    vx_enum       output_format    = TENSOR_DATA_TYPE(output);
    vx_enum       input_format     = TENSOR_DATA_TYPE(input);
    vx_enum       weights_format   = TENSOR_DATA_TYPE(weights);
    vx_uint32     input_dims       = TENSOR_DIM_NUM(input);
    vx_uint32     inBatch          = (input_dims > 3) ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_uint32     weight_dims      = TENSOR_DIM_NUM(weights) == 1 ? 2 : TENSOR_DIM_NUM(weights);
    vx_uint32     bias_dims        = enable_bias ? (TENSOR_DIM_NUM(bias) == 1 ? 2 : TENSOR_DIM_NUM(bias)) : 0;
    vx_uint32     output_dims      = TENSOR_DIM_NUM(output);
    vx_uint32     width_wei        = TENSOR_VIEW_SIZE_INDEX(weights, 0);
    vx_uint32     height_wei       = (weight_dims > 1) ? TENSOR_VIEW_SIZE_INDEX(weights, 1) : 1;
    vx_uint32     depth_wei        = (weight_dims > 2) ? TENSOR_VIEW_SIZE_INDEX(weights, 2) : 1;
    vx_uint32     weiBatch         = (weight_dims > 3) ? TENSOR_VIEW_SIZE_INDEX(weights, 3) : 1;
    vx_uint32     tensorSize       = width_wei * height_wei * depth_wei * weiBatch;
    vx_uint32     num_units        = 0;
    vx_tensor     input_rs         = NULL;
    vx_tensor     weight_rs        = NULL;
    vx_tensor     bias_rs          = NULL;
    vx_tensor     output_rs        = NULL;
    vx_uint32     inputZPValue     = TENSOR_TF_ZEROPOINT(input);
    vx_uint32     weightZPValue    = TENSOR_TF_ZEROPOINT(weights);
    vx_uint32     outputZPValue    = TENSOR_TF_ZEROPOINT(output);
    vx_float32    inputScaleValue  = TENSOR_TF_SCALE(input);
    vx_float32    weightScaleValue = TENSOR_TF_SCALE(weights);
    vx_float32    outputScaleValue = (vx_float32)1.0/TENSOR_TF_SCALE(output);
    vx_uint32     sizes[4]         = {1, 1, 1, 1};
    vx_uint32     inputSize        = 0;
    vx_uint32     batch            = 0;
    char *programSources = NULL;

    input_dims = TENSOR_DIM_NUM(input);
    switch(input_dims)
    {
    case 1:
        inputSize = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch   = 1;
        break;
    case 2:
        inputSize = TENSOR_VIEW_SIZE_INDEX(input, 0);
        batch     = TENSOR_VIEW_SIZE_INDEX(input, 1);
        break;
    case 3:
        inputSize   = TENSOR_VIEW_SIZE_INDEX(input, 0) * TENSOR_VIEW_SIZE_INDEX(input, 1) * TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = 1;
        break;
    case 4:
        inputSize   = TENSOR_VIEW_SIZE_INDEX(input, 0) * TENSOR_VIEW_SIZE_INDEX(input, 1) * TENSOR_VIEW_SIZE_INDEX(input, 2);
        batch   = inBatch;
        inBatch  = 1;
        break;
    default:
        vxError("Input tensor OnError dimension[%u]\n", input_dims);
        goto OnError;
    }

    num_units     = tensorSize / inputSize;

    input_dims    = (input_dims == 1 ) ? 2 : TENSOR_DIM_NUM(input);
    output_dims   = (output_dims == 1) ? 2 : TENSOR_DIM_NUM(output);

    sizes[0]      = inputSize;
    sizes[1]      = batch;
    sizes[2]      = 1;
    sizes[3]      = inBatch;
    input_rs      = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, input_dims);
    sizes[0]      = inputSize;
    sizes[1]      = num_units;
    sizes[2]      = 1;
    sizes[3]      = 1;
    weight_dims   = 2;
    weight_rs     = vxoTensor_ReshapeTensor(weights, (vx_int32*)sizes, weight_dims);

    if (enable_bias)
    {
        sizes[0]      = num_units;
        sizes[1]      = 1;
        bias_dims     = 2;
        bias_rs       = vxoTensor_ReshapeTensor(bias, (vx_int32*)sizes, bias_dims);
    }

    sizes[0]      = num_units;
    sizes[1]      = batch;
    sizes[2]      = 1;
    sizes[3]      = inBatch;
    output_rs     = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, output_dims);

    if (input_format == VX_TYPE_FLOAT16)
    {
        borderMode->mode = VX_BORDER_CONSTANT;
        borderMode->constant_value.S16 = 0;
    }
    else if (input_format == VX_TYPE_UINT8)
    {
        borderMode->mode = VX_BORDER_CONSTANT;
        borderMode->constant_value.U8 = (vx_uint8)inputZPValue;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, FullyConnected, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/FullyConnected.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuFullyConnected", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (input_format == VX_TYPE_UINT8 &&  weights_format ==  VX_TYPE_UINT8 && output_format == VX_TYPE_UINT8)
    {
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScaleValue);
        vx_scalar scaleWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weightScaleValue);
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScaleValue);
        vx_scalar inZP = vxCreateScalar(context, VX_TYPE_INT32, &inputZPValue);
        vx_scalar weightZP = vxCreateScalar(context, VX_TYPE_INT32, &weightZPValue);
        vx_scalar outZP = vxCreateScalar(context, VX_TYPE_INT32, &outputZPValue);

        if (enable_bias)
        {
            vx_reference  parameters[10] = {(vx_reference)input_rs, (vx_reference)weight_rs, (vx_reference)bias_rs,
                                            (vx_reference)scaleIn, (vx_reference)scaleWeight, (vx_reference)scaleOut,
                                            (vx_reference)inZP, (vx_reference)weightZP, (vx_reference)outZP,
                                            (vx_reference)output_rs};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            if (!shaderExecutable)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleWeight) vxReleaseScalar(&scaleWeight);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(weightZP) vxReleaseScalar(&weightZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleWeight) vxReleaseScalar(&scaleWeight);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(weightZP) vxReleaseScalar(&weightZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }
        }
        else
        {
            vx_reference  parameters[9] = {(vx_reference)input_rs, (vx_reference)weight_rs, (vx_reference)scaleIn,
                                            (vx_reference)scaleWeight, (vx_reference)scaleOut, (vx_reference)inZP,
                                            (vx_reference)weightZP, (vx_reference)outZP, (vx_reference)output_rs};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8Nobias", borderMode);
            if (!shaderExecutable)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleWeight) vxReleaseScalar(&scaleWeight);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(weightZP) vxReleaseScalar(&weightZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 9);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleWeight) vxReleaseScalar(&scaleWeight);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(weightZP) vxReleaseScalar(&weightZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }
        }

        if(scaleIn) vxReleaseScalar(&scaleIn);
        if(scaleWeight) vxReleaseScalar(&scaleWeight);
        if(scaleOut) vxReleaseScalar(&scaleOut);
        if(inZP) vxReleaseScalar(&inZP);
        if(weightZP) vxReleaseScalar(&weightZP);
        if(outZP) vxReleaseScalar(&outZP);
    }
    else if (input_format == VX_TYPE_FLOAT16 && weights_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16)
    {
        if (enable_bias)
        {
            vx_reference  parameters[4] = {(vx_reference)input_rs, (vx_reference)weight_rs, (vx_reference)bias_rs, (vx_reference)output_rs};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_reference  parameters[3] = {(vx_reference)input_rs, (vx_reference)weight_rs, (vx_reference)output_rs};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32Nobias", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS) goto OnError;
        }

    }

    execution_parameters.globalWorkSize[0]   = num_units;
    execution_parameters.globalWorkSize[1]   = batch;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (weight_rs) vxoTensor_ReleaseTensor(&weight_rs);
    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (weight_rs) vxoTensor_ReleaseTensor(&weight_rs);
    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuHashLUT****************************************/
vxnne_shader_executable vxnneGetGPUHashLUTShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               key,
    vx_tensor               value,
    vx_tensor               hit,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[5]              = {(vx_reference)input, (vx_reference)key, (vx_reference)value, (vx_reference)hit, (vx_reference)output};
    vx_enum       valueFormat                = TENSOR_DATA_TYPE(value);
    vx_enum       outputFormat               = TENSOR_DATA_TYPE(output);
    vx_uint32     dims                       = TENSOR_DIM_NUM(input);
    vx_uint32     width                      = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     input_count                = 0;
    vx_uint32     key_count                  = 0;
    vx_uint32     kDims                      = TENSOR_DIM_NUM(key);
    vx_uint32     kw                         = TENSOR_VIEW_SIZE_INDEX(key, 0);
    vx_uint32     vDims                      = TENSOR_DIM_NUM(value);
    vx_uint32     vw                         = TENSOR_VIEW_SIZE_INDEX(value, 0);
    vx_uint32     oDims                      = TENSOR_DIM_NUM(output);
    vx_uint32     ow                         = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32     tDims                      = TENSOR_DIM_NUM(hit);
    vx_uint32     tw                         = TENSOR_VIEW_SIZE_INDEX(hit, 0);
    vx_tensor     input_rs                   = NULL;
    vx_tensor     key_rs                     = NULL;
    vx_tensor     value_rs                   = NULL;
    vx_tensor     hit_rs                     = NULL;
    vx_tensor     output_rs                  = NULL;
    vx_int32      rs_sizes[4]                = {1, 1, 1, 1};
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_CONSTANT;
    borderMode->constant_value.U8 = 0;
    borderMode->constant_value.S16 = 0;
    borderMode->constant_value.S32 = 0;

    if (!((valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        || (valueFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
        || (valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)))
    {
        gcmPRINT("input or output's format is not support");
        goto OnError;
    }

    if (dims == 1)
    {
        rs_sizes[0] = width;
        input_rs = vxoTensor_ReshapeTensor(input, rs_sizes, 2);
        parameters[0] = (vx_reference)input_rs;
    }
    input_count = rs_sizes[0];

    if (kDims == 1)
    {
        rs_sizes[0] = kw;
        key_rs = vxoTensor_ReshapeTensor(key, rs_sizes, 2);
        parameters[1] = (vx_reference)key_rs;
    }
    key_count = rs_sizes[0];

    if (vDims == 1)
    {
        rs_sizes[0] = vw;
        value_rs = vxoTensor_ReshapeTensor(value, rs_sizes, 2);
        parameters[2] = (vx_reference)value_rs;
    }
    else if(vDims == 3)
    {
        vx_uint32 vh = TENSOR_VIEW_SIZE_INDEX(value, 1);
        vx_uint32 vc = TENSOR_VIEW_SIZE_INDEX(value, 2);
        rs_sizes[0] = vw * vh;
        rs_sizes[1] = vc;
        value_rs = vxoTensor_ReshapeTensor(value, rs_sizes, 2);
        parameters[2] = (vx_reference)value_rs;
    }
    else if(vDims == 4)
    {
        vx_uint32 vh = TENSOR_VIEW_SIZE_INDEX(value, 1);
        vx_uint32 vc = TENSOR_VIEW_SIZE_INDEX(value, 2);
        vx_uint32 vn = TENSOR_VIEW_SIZE_INDEX(value, 3);
        rs_sizes[0] = vw * vh * vc;
        rs_sizes[1] = vn;
        value_rs = vxoTensor_ReshapeTensor(value, rs_sizes, 2);
        parameters[2] = (vx_reference)value_rs;
    }

    if (tDims == 1)
    {
        rs_sizes[0] = tw;
        hit_rs = vxoTensor_ReshapeTensor(hit, rs_sizes, 2);
        parameters[3] = (vx_reference)hit_rs;
    }

    if (oDims == 1)
    {
        rs_sizes[0] = ow;
        output_rs = vxoTensor_ReshapeTensor(output, rs_sizes, 2);
        parameters[4] = (vx_reference)output_rs;
    }
    else if(oDims == 3)
    {
        vx_uint32 oh = TENSOR_VIEW_SIZE_INDEX(value, 1);
        vx_uint32 oc = TENSOR_VIEW_SIZE_INDEX(value, 2);
        rs_sizes[0] = ow * oh;
        rs_sizes[1] = oc;
        value_rs = vxoTensor_ReshapeTensor(value, rs_sizes, 2);
        parameters[2] = (vx_reference)value_rs;
    }
    else if(oDims == 4)
    {
        vx_uint32 oh = TENSOR_VIEW_SIZE_INDEX(value, 1);
        vx_uint32 oc = TENSOR_VIEW_SIZE_INDEX(value, 2);
        vx_uint32 on = TENSOR_VIEW_SIZE_INDEX(value, 3);
        rs_sizes[0] = ow * oh * oc;
        rs_sizes[1] = on;
        value_rs = vxoTensor_ReshapeTensor(value, rs_sizes, 2);
        parameters[2] = (vx_reference)value_rs;
    }

    execution_parameters.globalWorkSize[0]   = vw;
    execution_parameters.globalWorkSize[1]   = input_count;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, HashLUT, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/HashLUT.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuHashLUT", program, 5, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if (valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (key_rs) vxoTensor_ReleaseTensor(&key_rs);
    if (value_rs) vxoTensor_ReleaseTensor(&value_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (hit_rs) vxoTensor_ReleaseTensor(&hit_rs);

    return shaderExecutable;

OnError:
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (key_rs) vxoTensor_ReleaseTensor(&key_rs);
    if (value_rs) vxoTensor_ReleaseTensor(&value_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (hit_rs) vxoTensor_ReleaseTensor(&hit_rs);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********L2normalize sum sqrt****************************************************/
vxnne_shader_executable vxnneGPUL2NormSumSqrtShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders kernel;
    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum       srcFormat                  = TENSOR_DATA_TYPE(input);
    vx_uint32     width                      = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     height                     = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_float32    inputScale                 = TENSOR_TF_SCALE(input);
    vx_int32      inputZP                    = TENSOR_TF_ZEROPOINT(input);
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_CONSTANT;

    if (srcFormat == VX_TYPE_UINT8)
        borderMode->constant_value.U8 = (vx_uint8)TENSOR_TF_ZEROPOINT(input);
    else
        borderMode->constant_value.S16 = 0;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, L2NormSumSqrt, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/L2NormSumSqrt.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuL2NormScale", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (srcFormat == VX_TYPE_UINT8)
    {
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScale);
        vx_float32 outputScale = (vx_float32)1.0/inputScale;
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScale);
        vx_scalar zp = vxCreateScalar(context, VX_TYPE_INT32, &inputZP);
        vx_reference  parameters[5] = {(vx_reference)input, (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)zp, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SumRsqrtQuant8", borderMode);
        if (!shaderExecutable)
        {
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(zp) vxReleaseScalar(&zp);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
        if(scaleIn) vxReleaseScalar(&scaleIn);
        if(scaleOut) vxReleaseScalar(&scaleOut);
        if(zp) vxReleaseScalar(&zp);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (srcFormat == VX_TYPE_FLOAT16)
    {
        vx_reference  parameters[2] = {(vx_reference)input, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SumRsqrtFP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
        if (status != VX_SUCCESS) goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

vxnne_shader_executable vxnneGPUL2NormSumScaleShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               sumTmp,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders kernel;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum       srcFormat          = TENSOR_DATA_TYPE(input);
    vx_enum       dstFormat          = TENSOR_DATA_TYPE(output);
    vx_uint32     width              = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     height             = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32     depth              = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_float32    inputScale         = TENSOR_TF_SCALE(input);
    vx_float32    outputScale        = TENSOR_TF_SCALE(output);
    vx_int32      inputZP            = TENSOR_TF_ZEROPOINT(input);
    vx_int32      outputZP           = TENSOR_TF_ZEROPOINT(output);
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_REPLICATE;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, L2NormSumScale, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/L2NormSumScale.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuL2NormScale", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16)
    {
        vx_reference  parameters[3] = {(vx_reference)input, (vx_reference)sumTmp, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_MulScaleFP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
        if (status != VX_SUCCESS) goto OnError;

    }
    else if (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
    {
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScale);
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScale);
        vx_scalar inZP = vxCreateScalar(context, VX_TYPE_INT32, &inputZP);
        vx_scalar outZP = vxCreateScalar(context, VX_TYPE_INT32, &outputZP);
        vx_reference  parameters[7] = {(vx_reference)input, (vx_reference)sumTmp, (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)inZP, (vx_reference)outZP, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_MulScaleQuant8", borderMode);
        if (!shaderExecutable)
        {
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(inZP) vxReleaseScalar(&inZP);
            if(outZP) vxReleaseScalar(&outZP);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 7);
        if(scaleIn) vxReleaseScalar(&scaleIn);
        if(scaleOut) vxReleaseScalar(&scaleOut);
        if(inZP) vxReleaseScalar(&inZP);
        if(outZP) vxReleaseScalar(&outZP);
        if (status != VX_SUCCESS) goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;

}

vxnne_shader_executable vxnneGetGPUL2PoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x,
    vx_scalar               stride_y,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pad_left,
    vx_uint32               pad_top,
    vx_uint32               pad_right,
    vx_uint32               pad_bottom,
    vx_scalar               rounding,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum      inputFormat        = TENSOR_DATA_TYPE(input);
    vx_enum      outputFormat       = TENSOR_DATA_TYPE(output);
    vx_uint32    in_width           = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    in_height          = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth              = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_int32     input_ZP           = TENSOR_TF_ZEROPOINT(input);
    vx_uint32    out_width          = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32    out_height         = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_int32     output_ZP          = TENSOR_TF_ZEROPOINT(output);
    vx_uint32    stride_w           = stride_x->value->u32;
    vx_uint32    stride_h           = stride_y->value->u32;
    vx_uint32    kernel_size_x      = poolSizeX->value->u32;
    vx_uint32    kernel_size_y      = poolSizeY->value->u32;
    vx_float32   scaleInValue       = TENSOR_TF_SCALE(input);
    vx_float32   scaleOutValue      = 1.0f/TENSOR_TF_SCALE(output);
    vx_uint32    height             = (out_height - 1) * stride_w + kernel_size_y - pad_top - pad_bottom;
    vx_uint32    width              = (out_width - 1) * stride_h + kernel_size_x - pad_left - pad_right;
    char *programSources = NULL;

    if (pad_left != 0 || pad_top != 0 ||
        pad_right != 0 || pad_bottom != 0 ||
        height != in_height || width != in_width)
    {
        borderMode->mode = VX_BORDER_CONSTANT;
        if (inputFormat == VX_TYPE_UINT8)
        {
            borderMode->constant_value.U8 = (vx_uint8)input_ZP;
        }
        else if (inputFormat == VX_TYPE_FLOAT16)
        {
            borderMode->constant_value.S16 = 0;
        }
    }
    else
    {
        borderMode->mode = VX_BORDER_REPLICATE;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, L2Pooling, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/L2Pooling.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuL2Pooling", program, 3, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_scalar inZP = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        vx_scalar outZP = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_INT32, &scaleInValue);
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_INT32, &scaleOutValue);

        if (pad_left == 0 && pad_right == 0 && pad_top == 0 && pad_bottom == 0)
        {
            vx_reference parameters[10] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY, (vx_reference)stride_x, (vx_reference)stride_y,
                                          (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)inZP, (vx_reference)outZP, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_NoPadQuant8", borderMode);
            if (!shaderExecutable)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }
        }
        else
        {
            vx_scalar padX = vxCreateScalar(context, VX_TYPE_INT32, &pad_left);
            vx_scalar padY = vxCreateScalar(context, VX_TYPE_INT32, &pad_top);
            vx_reference parameters[12] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY, (vx_reference)stride_x, (vx_reference)stride_y,
                                           (vx_reference)padX, (vx_reference)padY, (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)inZP, (vx_reference)outZP,(vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            if (!shaderExecutable)
            {
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 12);
            if(padX) vxReleaseScalar(&padX);
            if(padY) vxReleaseScalar(&padY);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }
        }

        if(scaleIn) vxReleaseScalar(&scaleIn);
        if(scaleOut) vxReleaseScalar(&scaleOut);
        if(inZP) vxReleaseScalar(&inZP);
        if(outZP) vxReleaseScalar(&outZP);
    }
    else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {
        if (pad_left == 0 && pad_right == 0 && pad_top == 0 && pad_bottom == 0)
        {
            vx_reference parameters[6] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY,
                                          (vx_reference)stride_x, (vx_reference)stride_y, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_NoPadFP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_scalar padX = vxCreateScalar(context, VX_TYPE_INT32, &pad_left);
            vx_scalar padY = vxCreateScalar(context, VX_TYPE_INT32, &pad_top);
            vx_reference parameters[8] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY,
                                          (vx_reference)stride_x, (vx_reference)stride_y,
                                          (vx_reference)padX, (vx_reference)padY, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            if (!shaderExecutable)
            {
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
            if(padX) vxReleaseScalar(&padX);
            if(padY) vxReleaseScalar(&padY);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

    execution_parameters.globalWorkSize[0]   = out_width;
    execution_parameters.globalWorkSize[1]   = out_height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuLrn****************************************************/
vxnne_shader_executable vxnneGetGPUNormalizationShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               type_s,
    vx_scalar               norm_size_s,
    vx_scalar               alpha_s,
    vx_scalar               beta_s,
    vx_scalar               bias_s,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum         inputFormat   = TENSOR_DATA_TYPE(input);
    vx_enum         outputFormat  = TENSOR_DATA_TYPE(output);
    vx_uint32       width         = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32       height        = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32       channel       = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_enum         norm_type     = type_s->value->e;
    vx_float32      in_scale      = TENSOR_TF_SCALE(input);
    vx_float32      out_scale     = 1.0f/TENSOR_TF_SCALE(output);
    vx_int32        zpInValue     = TENSOR_TF_ZEROPOINT(input);
    vx_int32        zpOutValue    = TENSOR_TF_ZEROPOINT(output);
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_CONSTANT;
    if (inputFormat == VX_TYPE_INT8)
    {
        borderMode->constant_value.U8 = 0;
    }
    else
    {
        borderMode->constant_value.S16 = 0;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);
    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Normalization, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Normalization.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuNormalization", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (norm_type == VX_NN_NORMALIZATION_SAME_MAP)
    {
        vx_float32 orgAlphaValue = alpha_s->value->f32;
        vx_uint32 radiusValue = norm_size_s->value->u32;
        vx_float32 newAlphaValue = (orgAlphaValue/(radiusValue*radiusValue));
        vx_scalar alpha = vxCreateScalar(context, VX_TYPE_FLOAT32, &newAlphaValue);

        if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            vx_reference parameters[5] = {(vx_reference)input, (vx_reference)norm_size_s, (vx_reference)alpha, (vx_reference)beta_s, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SameMapsFP32", borderMode);
            if (!shaderExecutable)
            {
                if(alpha) vxReleaseScalar(&alpha);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if(alpha) vxReleaseScalar(&alpha);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_scalar       scaleIn    = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale);
            vx_scalar       scaleOut   = vxCreateScalar(context, VX_TYPE_FLOAT32, &out_scale);
            vx_scalar       zpIn       = vxCreateScalar(context, VX_TYPE_FLOAT32, &zpInValue);
            vx_scalar       zpOut      = vxCreateScalar(context, VX_TYPE_FLOAT32, &zpOutValue);
            vx_reference parameters[9] = {(vx_reference)input, (vx_reference)norm_size_s, (vx_reference)alpha, (vx_reference)beta_s,
                                           (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)zpIn, (vx_reference)zpOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SameMapsQuant8", borderMode);
            if (!shaderExecutable)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zpIn) vxReleaseScalar(&zpIn);
                if(zpOut) vxReleaseScalar(&zpOut);
                if(alpha) vxReleaseScalar(&alpha);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 9);
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(zpIn) vxReleaseScalar(&zpIn);
            if(zpOut) vxReleaseScalar(&zpOut);
            if(alpha) vxReleaseScalar(&alpha);
            if (status != VX_SUCCESS) goto OnError;
        }
    }
    else /*VX_NN_NORMALIZATION_ACROSS_MAPS*/
    {
        if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            vx_reference parameters[6] = {(vx_reference)input, (vx_reference)norm_size_s, (vx_reference)bias_s, (vx_reference)alpha_s, (vx_reference)beta_s, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AcrossMapsFP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_scalar       scaleIn    = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale);
            vx_scalar       scaleOut   = vxCreateScalar(context, VX_TYPE_FLOAT32, &out_scale);
            vx_scalar       zpIn       = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            vx_scalar       zpOut      = vxCreateScalar(context, VX_TYPE_INT32, &zpOutValue);
            vx_reference parameters[10] = {(vx_reference)input, (vx_reference)norm_size_s, (vx_reference)bias_s, (vx_reference)alpha_s, (vx_reference)beta_s,
                                           (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)zpIn, (vx_reference)zpOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AcrossMapsQuant8", borderMode);
            if (!shaderExecutable)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zpIn) vxReleaseScalar(&zpIn);
                if(zpOut) vxReleaseScalar(&zpOut);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(zpIn) vxReleaseScalar(&zpIn);
            if(zpOut) vxReleaseScalar(&zpOut);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = channel;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuActivation****************************************************/
vxnne_shader_executable vxnneGetGPUActivationShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_enum                 funcType,
    vx_tensor               input,
    vx_float32              minVal,
    vx_float32              maxVal,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum        inputFormat      = TENSOR_DATA_TYPE(input);
    vx_enum        outputFormat     = TENSOR_DATA_TYPE(output);
    vx_uint32      width            = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32      height           = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32      depth            = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32      dims             = TENSOR_DIM_NUM(input) == 1 ? 2 : TENSOR_DIM_NUM(input);
    vx_tensor      input_rs         = NULL;
    vx_tensor      output_rs        = NULL;
    vx_float32     scaleInValue     = TENSOR_TF_SCALE(input);
    vx_int32       zpInValue        = TENSOR_TF_ZEROPOINT(input);
    vx_float32     scaleOutValue    = 1.0f/TENSOR_TF_SCALE(output);
    vx_int32       zpOutValue       = TENSOR_TF_ZEROPOINT(output);
    vx_float32     logEValue        = (vx_float32)(log10(exp(1.0f)) / log10(2.0f));
    vx_bool        enalbe_relu      = (vx_bool)(funcType == VX_NN_ACTIVATION_RELU || funcType == VX_NN_ACTIVATION_RELU1 || funcType == VX_NN_ACTIVATION_RELU6);
    char           *programSources = NULL;
    vx_scalar      logE = VX_NULL;

    borderMode->mode = VX_BORDER_REPLICATE;
    if(TENSOR_DIM_NUM(input) == 1)
    {
        vx_int32 sizes[2] = {width, 1};

        input_rs  = vxoTensor_ReshapeTensor(input, sizes, dims);
        output_rs = vxoTensor_ReshapeTensor(output, sizes, dims);
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Activation, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Activation.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }

#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        vxmONERROR(vxBuildProgram(program, VX_NULL));

        kernel = vxnneAddKernelShadersInProgram(context, "gpuActivation", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (enalbe_relu)
    {
        char subName[128];

        if(funcType == VX_NN_ACTIVATION_RELU && inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            sprintf(subName, "_ReluFP32");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU && inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            sprintf(subName, "_ReluQuant8");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU1 && inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            sprintf(subName, "_Relu1FP32");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU1 && inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            sprintf(subName, "_Relu1Quant8");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU6 && inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            sprintf(subName, "_Relu6FP32");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU6 && inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            sprintf(subName, "_Relu6Quant8");
        }

        if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            vx_reference   parameters[2] = {(vx_reference)input, (vx_reference)output};

            if(TENSOR_DIM_NUM(input) == 1)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[1] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, subName, borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
            if (status != VX_SUCCESS) goto OnError;
        }
        else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
            vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
            vx_scalar zpOut = vxCreateScalar(context, VX_TYPE_INT32, &zpOutValue);
            vx_reference   parameters[6] = {(vx_reference)input, (vx_reference)scaleIn, (vx_reference)zpIn, (vx_reference)scaleOut, (vx_reference)zpOut, (vx_reference)output};

            if(TENSOR_DIM_NUM(input) == 1)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[5] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, subName, borderMode);
            if (!shaderExecutable)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(zpIn) vxReleaseScalar(&zpIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zpOut) vxReleaseScalar(&zpOut);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(zpIn) vxReleaseScalar(&zpIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(zpOut) vxReleaseScalar(&zpOut);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vxError("Not support data format at function %s line %d", __FUNCTION__, __LINE__);
        }
    }
    else if (funcType == VX_NN_ACTIVATION_LOGISTIC)
    {
        logEValue = -logEValue;
        logE = vxCreateScalar(context, VX_TYPE_FLOAT32, &logEValue);

        if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            vx_reference   parameters[3] = {(vx_reference)input, (vx_reference)logE, (vx_reference)output};

            if(TENSOR_DIM_NUM(input) == 1)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[2] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SigmoidFP32", borderMode);
            if (!shaderExecutable)
            {
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS)
            {
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
            vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            vx_reference   parameters[5] = {(vx_reference)input, (vx_reference)logE, (vx_reference)scaleIn, (vx_reference)zpIn, (vx_reference)output};

            if(TENSOR_DIM_NUM(input) == 1)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[4] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SigmoidQuant8", borderMode);
            if (!shaderExecutable)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(zpIn) vxReleaseScalar(&zpIn);
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(zpIn) vxReleaseScalar(&zpIn);
            if (status != VX_SUCCESS)
            {
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)
        {
            vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
            vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            vx_reference   parameters[5] = {(vx_reference)input, (vx_reference)logE, (vx_reference)scaleIn, (vx_reference)zpIn, (vx_reference)output};

            if(TENSOR_DIM_NUM(input) == 1)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[4] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SigmoidQ8toFP32", borderMode);
            if (!shaderExecutable)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(zpIn) vxReleaseScalar(&zpIn);
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(zpIn) vxReleaseScalar(&zpIn);
            if (status != VX_SUCCESS)
            {
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_UINT8)
        {
            vx_reference   parameters[3] = {(vx_reference)input, (vx_reference)logE, (vx_reference)output};

            if(TENSOR_DIM_NUM(input) == 1)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[2] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SigmoidFP32toQ8", borderMode);
            if (!shaderExecutable)
            {
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS)
            {
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }
        }

        if(logE) vxReleaseScalar(&logE);
    }
    else if(funcType == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
    {
        logEValue = 2*logEValue;
        logE = vxCreateScalar(context, VX_TYPE_FLOAT32, &logEValue);

        if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            vx_reference   parameters[3] = {(vx_reference)input, (vx_reference)logE, (vx_reference)output};

            if(TENSOR_DIM_NUM(input) == 1)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[2] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_TanhFP32", borderMode);
            if (!shaderExecutable)
            {
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS)
            {
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
            vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
            vx_scalar zpOut = vxCreateScalar(context, VX_TYPE_INT32, &zpOutValue);
            vx_reference   parameters[7] = {(vx_reference)input, (vx_reference)logE, (vx_reference)scaleIn, (vx_reference)zpIn, (vx_reference)scaleOut, (vx_reference)zpOut, (vx_reference)output};

            if(TENSOR_DIM_NUM(input) == 1)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[5] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_TanhQuant8", borderMode);
            if (!shaderExecutable)
            {
                if(logE) vxReleaseScalar(&logE);
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(zpIn) vxReleaseScalar(&zpIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(zpOut) vxReleaseScalar(&zpOut);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(zpIn) vxReleaseScalar(&zpIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(zpOut) vxReleaseScalar(&zpOut);
            if (status != VX_SUCCESS)
            {
                if(logE) vxReleaseScalar(&logE);
                goto OnError;
            }
        }
        if(logE) vxReleaseScalar(&logE);
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuMaxPooling****************************************************/
vxnne_shader_executable vxnneGetGPUMaxPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_x,
    vx_scalar               stride_y,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_uint32               pad_left,
    vx_uint32               pad_top,
    vx_uint32               pad_right,
    vx_uint32               pad_bottom,
    vx_scalar               rounding,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum      inputFormat        = TENSOR_DATA_TYPE(input);
    vx_enum      outputFormat       = TENSOR_DATA_TYPE(output);
    vx_uint32    in_width           = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    in_height          = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth              = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_int32     input_ZP           = TENSOR_TF_ZEROPOINT(input);
    vx_uint32    out_width          = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32    out_height         = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_int32     output_ZP          = TENSOR_TF_ZEROPOINT(output);
    vx_uint32    stride_w           = stride_x->value->u32;
    vx_uint32    stride_h           = stride_y->value->u32;
    vx_uint32    kernel_size_x      = poolSizeX->value->u32;
    vx_uint32    kernel_size_y      = poolSizeY->value->u32;
    vx_float32   scaleInValue       = TENSOR_TF_SCALE(input);
    vx_float32   scaleOutValue      = 1.0f/TENSOR_TF_SCALE(output);
    vx_uint32    height             = (out_height - 1) * stride_w + kernel_size_y - pad_top - pad_bottom;
    vx_uint32    width              = (out_width - 1) * stride_h + kernel_size_x - pad_left - pad_right;
    char *programSources = NULL;

    if (pad_left != 0 || pad_top != 0 ||
        pad_right != 0 || pad_bottom != 0 ||
        height != in_height || width != in_width)
    {
        borderMode->mode = VX_BORDER_CONSTANT;
        if (inputFormat == VX_TYPE_UINT8)
        {
            borderMode->constant_value.U8 = (vx_uint8)input_ZP;
        }
        else if (inputFormat == VX_TYPE_FLOAT16)
        {
            borderMode->constant_value.S16 = 0;
        }
    }
    else
    {
        borderMode->mode = VX_BORDER_REPLICATE;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, MaxPooling, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/MaxPooling.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuPooling_Max", program, 3, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_scalar inZP = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        vx_scalar outZP = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);

        if (pad_left == 0 && pad_right == 0 && pad_top == 0 && pad_bottom == 0)
        {
            vx_reference parameters[10] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY, (vx_reference)stride_x, (vx_reference)stride_y,
                                          (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)inZP, (vx_reference)outZP, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "NoPadQuant8", borderMode);
            if (!shaderExecutable)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }
        }
        else
        {
            vx_scalar padX = vxCreateScalar(context, VX_TYPE_INT32, &pad_left);
            vx_scalar padY = vxCreateScalar(context, VX_TYPE_INT32, &pad_top);
            vx_reference parameters[12] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY, (vx_reference)stride_x, (vx_reference)stride_y,
                                           (vx_reference)padX, (vx_reference)padY, (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)inZP, (vx_reference)outZP,(vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8", borderMode);
            if (!shaderExecutable)
            {
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 12);
            if(padX) vxReleaseScalar(&padX);
            if(padY) vxReleaseScalar(&padY);
            if (status != VX_SUCCESS)
            {
                if(scaleIn) vxReleaseScalar(&scaleIn);
                if(scaleOut) vxReleaseScalar(&scaleOut);
                if(inZP) vxReleaseScalar(&inZP);
                if(outZP) vxReleaseScalar(&outZP);
                goto OnError;
            }
        }

        if(scaleIn) vxReleaseScalar(&scaleIn);
        if(scaleOut) vxReleaseScalar(&scaleOut);
        if(inZP) vxReleaseScalar(&inZP);
        if(outZP) vxReleaseScalar(&outZP);
    }
    else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {
        if (pad_left == 0 && pad_right == 0 && pad_top == 0 && pad_bottom == 0)
        {
            vx_reference parameters[6] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY,
                                          (vx_reference)stride_x, (vx_reference)stride_y, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "NoPadFP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_scalar padX = vxCreateScalar(context, VX_TYPE_INT32, &pad_left);
            vx_scalar padY = vxCreateScalar(context, VX_TYPE_INT32, &pad_top);
            vx_reference parameters[8] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY,
                                          (vx_reference)stride_x, (vx_reference)stride_y,
                                          (vx_reference)padX, (vx_reference)padY, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32", borderMode);
            if (!shaderExecutable)
            {
                if(padX) vxReleaseScalar(&padX);
                if(padY) vxReleaseScalar(&padY);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
            if(padX) vxReleaseScalar(&padX);
            if(padY) vxReleaseScalar(&padY);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

    execution_parameters.globalWorkSize[0]   = out_width;
    execution_parameters.globalWorkSize[1]   = out_height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

vxnne_shader_executable vxnneGetGPUTensorScaleShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_enum                 type,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum      inputFormat        = TENSOR_DATA_TYPE(input);
    vx_enum      outputFormat       = TENSOR_DATA_TYPE(output);
    vx_uint32    in_width           = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    in_height          = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth              = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_int32     input_ZP           = TENSOR_TF_ZEROPOINT(input);
    vx_uint32    out_width          = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32    out_height         = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_int32     output_ZP          = TENSOR_TF_ZEROPOINT(output);
    vx_float32   scaleInValue       = TENSOR_TF_SCALE(input);
    vx_float32   scaleOutValue      = 1.0f/TENSOR_TF_SCALE(output);
    vx_float32   scaleXValue        = (vx_float32)in_width / (vx_float32)out_width;
    vx_float32   scaleYValue        = (vx_float32)in_height / (vx_float32)out_height;
    char        *programSources = NULL;
    vx_scalar    scaleX_s = VX_NULL, scaleY_s = VX_NULL;

    scaleX_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleXValue);
    scaleY_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleYValue);
    if(scaleX_s == VX_NULL || scaleY_s == VX_NULL)
    {
        vxError("Create scalar failed at function %s line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }

    borderMode->mode = VX_BORDER_REPLICATE;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorScale, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorScale.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorScale", program, 2, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {
        vx_reference parameters[4] = {(vx_reference)input, (vx_reference)scaleX_s, (vx_reference)scaleY_s, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_BilinearFP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_scalar inZP = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        vx_scalar outZP = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
        vx_reference parameters[8] = {(vx_reference)input, (vx_reference)scaleX_s, (vx_reference)scaleY_s, (vx_reference)scaleIn, (vx_reference)scaleOut,
                                        (vx_reference)inZP, (vx_reference)outZP, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_BilinearQuant8", borderMode);
        if (!shaderExecutable)
        {
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(inZP) vxReleaseScalar(&inZP);
            if(outZP) vxReleaseScalar(&outZP);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
        if(scaleIn) vxReleaseScalar(&scaleIn);
        if(scaleOut) vxReleaseScalar(&scaleOut);
        if(inZP) vxReleaseScalar(&inZP);
        if(outZP) vxReleaseScalar(&outZP);
        if (status != VX_SUCCESS) goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = out_width;
    execution_parameters.globalWorkSize[1]   = out_height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;
    if(scaleX_s) vxReleaseScalar(&scaleX_s);
    if(scaleY_s) vxReleaseScalar(&scaleY_s);
    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if(scaleX_s) vxReleaseScalar(&scaleX_s);
    if(scaleY_s) vxReleaseScalar(&scaleY_s);
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
    return VX_NULL;
}

vxnne_shader_executable vxnneGetGPUTensorMaxValueShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_array                output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_uint32 inDims = TENSOR_DIM_NUM(input);
    vx_kernel_execution_parameters_t execution_parameters = {inDims-1, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 1, 1}};
    vx_enum      inputFormat        = TENSOR_DATA_TYPE(input);
    vx_uint32    in_width           = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    in_height          = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth              = TENSOR_VIEW_SIZE_INDEX(input, 2);
    char        *programSources = NULL;

    borderMode->mode = VX_BORDER_REPLICATE;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorMaxValue, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorMaxValue.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorMaxValue", program, 2, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_FLOAT16)
    {
        vx_reference parameters[2] = {(vx_reference)input, (vx_reference)output};
        if(inDims == 4)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Dim2FP32", borderMode);
        }
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        vx_float32 scaleInValue = TENSOR_TF_SCALE(input);
        vx_float32 scaleOutValue = 1.0f/TENSOR_TF_SCALE(input);
        vx_uint32 zpInValue = TENSOR_TF_ZEROPOINT(input);
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
        vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_UINT32, &zpInValue);
        vx_reference parameters[5] = {(vx_reference)input, (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)zpIn, (vx_reference)output};

        if(inDims == 4)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Dim2Quant8", borderMode);
        }

        if (!shaderExecutable)
        {
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(zpIn) vxReleaseScalar(&zpIn);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
        if(scaleIn) vxReleaseScalar(&scaleIn);
        if(scaleOut) vxReleaseScalar(&scaleOut);
        if(zpIn) vxReleaseScalar(&zpIn);
        if (status != VX_SUCCESS) goto OnError;
    }

    if(inDims == 4)
    {
        execution_parameters.globalWorkSize[0]   = in_width;
        execution_parameters.globalWorkSize[1]   = in_height;
        execution_parameters.globalWorkSize[2]   = depth;
    }
    else
    {
        execution_parameters.globalWorkSize[0]   = in_width;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
    return VX_NULL;
}

vxnne_shader_executable vxnneGetGPUSoftmaxShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_scalar               beta,
    vx_tensor               input,
    vx_array                maxValue,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_uint32 inDims = TENSOR_DIM_NUM(input);
    vx_kernel_execution_parameters_t execution_parameters = {inDims-1, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 1, 1}};
    vx_enum      inputFormat        = TENSOR_DATA_TYPE(input);
    vx_uint32    in_width           = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    in_height          = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth              = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_float32   betaValue          = beta->value->f32;
    vx_float32   scaleValue         = (vx_float32)((log10(exp(1.0f)) / log10(2.0f))*betaValue);
    char        *programSources = NULL;
    vx_scalar    scale_s            = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);

    borderMode->mode = VX_BORDER_REPLICATE;
    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);
    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, SoftMax, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/SoftMax.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuSoftMax", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_FLOAT16)
    {
        vx_reference parameters[4] = {(vx_reference)input, (vx_reference)maxValue, (vx_reference)scale_s, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        vx_float32 scaleInValue = TENSOR_TF_SCALE(input);
        vx_int32 zpInValue = TENSOR_TF_ZEROPOINT(input);
        vx_scalar scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
        vx_scalar zpIn = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
        vx_reference parameters[6] = {(vx_reference)input, (vx_reference)maxValue, (vx_reference)scale_s,
                                        (vx_reference)scaleIn, (vx_reference)zpIn, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable)
        {
            if(scaleIn) vxReleaseScalar(&scaleIn);
            if(zpIn) vxReleaseScalar(&zpIn);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
        if(scaleIn) vxReleaseScalar(&scaleIn);
        if(zpIn) vxReleaseScalar(&zpIn);
        if (status != VX_SUCCESS) goto OnError;
    }

    if(inDims == 4)
    {
        execution_parameters.globalWorkSize[0]   = in_width;
        execution_parameters.globalWorkSize[1]   = in_height;
        execution_parameters.globalWorkSize[2]   = depth;
    }
    else
    {
        execution_parameters.globalWorkSize[0]   = in_width;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(scale_s) vxReleaseScalar(&scale_s);
    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if(scale_s) vxReleaseScalar(&scale_s);
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
    return VX_NULL;
}

vxnne_shader_executable vxnneGetGPUTensorReduceDivShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_uint32 inDims = TENSOR_DIM_NUM(input0);
    vx_reference parameters[2] = {(vx_reference)input0, (vx_reference)output};
    vx_kernel_execution_parameters_t execution_parameters = {inDims-1, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 1, 1}};
    vx_enum      inputFormat        = TENSOR_DATA_TYPE(input0);
    vx_uint32    in_width           = TENSOR_VIEW_SIZE_INDEX(input0, 0);
    vx_uint32    in_height          = TENSOR_VIEW_SIZE_INDEX(input0, 1);
    vx_uint32    depth              = TENSOR_VIEW_SIZE_INDEX(input0, 2);
    char        *programSources = NULL;

    borderMode->mode = VX_BORDER_REPLICATE;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorReduceDiv, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorReduceDiv.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorDiv", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_FLOAT16)
    {
        if(inDims == 4)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_ScalarDivFP32", borderMode);
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_ScalarDivFP32_Dim2", borderMode);
        }
        if (!shaderExecutable) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        if(inDims == 4)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_ScalarDivQuant8", borderMode);
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_ScalarDivQuant8_Dim2", borderMode);
        }
        if (!shaderExecutable) goto OnError;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
    if (status != VX_SUCCESS) goto OnError;

    if(inDims == 4)
    {
        execution_parameters.globalWorkSize[0]   = in_width;
        execution_parameters.globalWorkSize[1]   = in_height;
        execution_parameters.globalWorkSize[2]   = depth;
    }
    else
    {
        execution_parameters.globalWorkSize[0]   = in_width;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
    return VX_NULL;
}

/********gpuRnn****************************************/
vxnne_shader_executable vxnneGetGPURnnShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               bias,
    vx_tensor               weight,
    vx_tensor               hidden,
    vx_tensor               recurrent,
    vx_tensor               activation,
    vx_tensor               state_out,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum       inputFormat                = TENSOR_DATA_TYPE(input);
    vx_enum       outputFormat               = TENSOR_DATA_TYPE(output);
    vx_uint32     output_width               = TENSOR_VIEW_SIZE_INDEX(weight, 1);
    vx_uint32     output_height              = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_tensor     bias_rs                    = NULL;
    vx_int32      bs_sizes[4]                = {1, 1, 1, 1};
    vx_uint32     bs_dims                    = TENSOR_DIM_NUM(bias);
    vx_uint32     bs_dims_rs                 = TENSOR_DIM_NUM(bias) == 1 ? 2 : TENSOR_DIM_NUM(bias);
    vx_uint32     i                          = 0;
    vx_enum       act                        = VX_NN_ACTIVATION_NONE;
    char *programSources = NULL;

    borderMode->mode = VX_BORDER_CONSTANT;
    borderMode->constant_value.U8 = 0;
    borderMode->constant_value.S16 = 0;

    if(activation != VX_NULL)
    {
        switch(*(vx_int32_ptr)TENSOR_LOGICAL_ADDR(activation))
        {
        case 0:
            act = VX_NN_ACTIVATION_NONE;
            break;
        case 1:
            act = VX_NN_ACTIVATION_RELU;
            break;
        case 2:
            act = VX_NN_ACTIVATION_RELU1;
            break;
        case 3:
            act = VX_NN_ACTIVATION_RELU6;
            break;
        default:
            vxError("Invalid activation type at function %s line %d", __FUNCTION__, __LINE__);
            break;
        }
    }

    if (((inputFormat != VX_TYPE_FLOAT16) && (inputFormat != VX_TYPE_FLOAT32) && (inputFormat != VX_TYPE_UINT8)) ||
        ((outputFormat != VX_TYPE_FLOAT16) && (outputFormat != VX_TYPE_FLOAT32) && (outputFormat != VX_TYPE_UINT8))
        )
    {
        vxError("Invalid data type at function %s line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }

    if (bs_dims == 1)
    {
        bs_sizes[0] *= TENSOR_VIEW_SIZE_INDEX(bias, i);
        bias_rs = vxoTensor_ReshapeTensor(bias, bs_sizes, bs_dims_rs);
    }

    execution_parameters.globalWorkSize[0] = output_width;
    execution_parameters.globalWorkSize[1] = output_height;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Rnn, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Rnn.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuRnn", program, 8, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if ((inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32) || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16))
    {
        vx_reference parameters[7] = {(vx_reference)input, (vx_reference)weight, (vx_reference)recurrent, (vx_reference)bias_rs, (vx_reference)hidden, (vx_reference)state_out, (vx_reference)output};

        if(act == VX_NN_ACTIVATION_RELU)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32Relu", borderMode);
        }
        else if(act == VX_NN_ACTIVATION_RELU1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32Relu1", borderMode);
        }
        else if(act == VX_NN_ACTIVATION_RELU6)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32Relu6", borderMode);
        }
        else if(act == VX_NN_ACTIVATION_NONE)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        }
        else
        {
            vxError("Invalid activation type at function %s line %d", __FUNCTION__, __LINE__);
            goto OnError;
        }
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 7);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if ((inputFormat == VX_TYPE_UINT8) && (outputFormat == VX_TYPE_UINT8))
    {
        vx_int32   outputZPValue       = TENSOR_TF_ZEROPOINT(output);
        vx_int32   inputZPValue        = TENSOR_TF_ZEROPOINT(input);
        vx_int32   biasZPValue         = TENSOR_TF_ZEROPOINT(bias);
        vx_int32   weightZPValue       = TENSOR_TF_ZEROPOINT(weight);
        vx_int32   hiddenZPValue       = TENSOR_TF_ZEROPOINT(hidden);
        vx_int32   recurrentZPValue    = TENSOR_TF_ZEROPOINT(recurrent);
        vx_float32 inputScaleValue     = TENSOR_TF_SCALE(input);
        vx_float32 biasScaleValue      = TENSOR_TF_SCALE(bias);
        vx_float32 weightScaleValue    = TENSOR_TF_SCALE(weight);
        vx_float32 hiddenScaleValue    = TENSOR_TF_SCALE(hidden);
        vx_float32 recurrentScaleValue = TENSOR_TF_SCALE(recurrent);
        vx_float32 outputScaleValue    = 1.0f/TENSOR_TF_SCALE(output);
        vx_scalar  outputZP            = vxCreateScalar(context, VX_TYPE_UINT32, &outputZPValue);
        vx_scalar  inputZP             = vxCreateScalar(context, VX_TYPE_UINT32, &inputZPValue);
        vx_scalar  biasZP              = vxCreateScalar(context, VX_TYPE_UINT32, &biasZPValue);
        vx_scalar  weightZP            = vxCreateScalar(context, VX_TYPE_UINT32, &weightZPValue);
        vx_scalar  hiddenZP            = vxCreateScalar(context, VX_TYPE_UINT32, &hiddenZPValue);
        vx_scalar  recurrentZP         = vxCreateScalar(context, VX_TYPE_UINT32, &recurrentZPValue);
        vx_scalar  outputScale         = vxCreateScalar(context, VX_TYPE_UINT32, &outputScaleValue);
        vx_scalar  inputScale          = vxCreateScalar(context, VX_TYPE_UINT32, &inputScaleValue);
        vx_scalar  biasScale           = vxCreateScalar(context, VX_TYPE_UINT32, &biasScaleValue);
        vx_scalar  weightScale         = vxCreateScalar(context, VX_TYPE_UINT32, &weightScaleValue);
        vx_scalar  hiddenScale         = vxCreateScalar(context, VX_TYPE_UINT32, &hiddenScaleValue);
        vx_scalar  recurrentScale      = vxCreateScalar(context, VX_TYPE_UINT32, &recurrentScaleValue);
        vx_reference parameters[19]    = {(vx_reference)input, (vx_reference)inputScale, (vx_reference)inputZP, (vx_reference)weight, (vx_reference)weightScale, (vx_reference)weightZP,
                                          (vx_reference)recurrent, (vx_reference)recurrentScale, (vx_reference)recurrentZP, (vx_reference)bias_rs, (vx_reference)biasScale, (vx_reference)biasZP,
                                          (vx_reference)hidden, (vx_reference)hiddenScale, (vx_reference)hiddenZP, (vx_reference)state_out,
                                          (vx_reference)output, (vx_reference)outputScale, (vx_reference)outputZP};

        if(act == VX_NN_ACTIVATION_RELU)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8Relu", borderMode);
        }
        else if(act == VX_NN_ACTIVATION_RELU1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8Relu1", borderMode);
        }
        else if(act == VX_NN_ACTIVATION_RELU6)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8Relu6", borderMode);
        }
        else if(act == VX_NN_ACTIVATION_NONE)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        }
        else
        {
            vxError("Invalid activation type at function %s line %d", __FUNCTION__, __LINE__);
            goto OnError;
        }
        if (!shaderExecutable)
        {
            if(outputScale) vxReleaseScalar(&outputScale);
            if(inputScale) vxReleaseScalar(&inputScale);
            if(biasScale) vxReleaseScalar(&biasScale);
            if(recurrentScale) vxReleaseScalar(&recurrentScale);
            if(hiddenScale) vxReleaseScalar(&hiddenScale);
            if(weightScale) vxReleaseScalar(&weightScale);
            if(outputZP) vxReleaseScalar(&outputZP);
            if(inputZP) vxReleaseScalar(&inputZP);
            if(biasZP) vxReleaseScalar(&biasZP);
            if(weightZP) vxReleaseScalar(&weightZP);
            if(hiddenZP) vxReleaseScalar(&hiddenZP);
            if(recurrentZP) vxReleaseScalar(&recurrentZP);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 19);
        if(outputScale) vxReleaseScalar(&outputScale);
        if(inputScale) vxReleaseScalar(&inputScale);
        if(biasScale) vxReleaseScalar(&biasScale);
        if(recurrentScale) vxReleaseScalar(&recurrentScale);
        if(hiddenScale) vxReleaseScalar(&hiddenScale);
        if(weightScale) vxReleaseScalar(&weightScale);
        if(outputZP) vxReleaseScalar(&outputZP);
        if(inputZP) vxReleaseScalar(&inputZP);
        if(biasZP) vxReleaseScalar(&biasZP);
        if(weightZP) vxReleaseScalar(&weightZP);
        if(hiddenZP) vxReleaseScalar(&hiddenZP);
        if(recurrentZP) vxReleaseScalar(&recurrentZP);
        if (status != VX_SUCCESS) goto OnError;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);

    return shaderExecutable;

OnError:
    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}

/********gpuTensorAdd****************************************************/
vxnne_shader_executable vxnneGetGPUTensorElewiseShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_int32                activation,
    vx_enum                 operation,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[9]         = {(vx_reference)input0, (vx_reference)input1, (vx_reference)output, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL};
    vx_uint32     dims                  = TENSOR_DIM_NUM(output);
    vx_uint32     width                 = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32     height                = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(output, 1) : 1;
    vx_uint32     depth                 = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(output, 2) : 1;
    vx_enum       input0_format         = TENSOR_DATA_TYPE(input0);
    vx_enum       input1_format         = TENSOR_DATA_TYPE(input1);
    vx_tensor     src0                  = NULL;
    vx_tensor     src1                  = NULL;
    vx_tensor     dst                   = NULL;
    vx_float32    input_scale0          = TENSOR_TF_SCALE(input0);
    vx_float32    input_scale1          = TENSOR_TF_SCALE(input1);
    vx_float32    output_scale          = 1.0f/TENSOR_TF_SCALE(output);
    vx_int32      input_ZP0             = TENSOR_TF_ZEROPOINT(input0);
    vx_int32      input_ZP1             = TENSOR_TF_ZEROPOINT(input1);
    vx_int32      output_ZP             = TENSOR_TF_ZEROPOINT(output);
    vx_uint32     paramNum              = 3;
    vx_uint32     i                     = 0;

    char *programSources = NULL;

    /* parameter check for inputs, accordding to nn api, the two input tensorshould be of identical operancode and compatible dimensions. */
    for (i = 0; i < TENSOR_DIM_NUM(output); i++)
    {
        vx_uint32 size0 = TENSOR_DIM_NUM(input0) > i ? TENSOR_VIEW_SIZE_INDEX(input0, i) : 1;
        vx_uint32 size1 = TENSOR_DIM_NUM(input1) > i ? TENSOR_VIEW_SIZE_INDEX(input1, i) : 1;

        if (size0 != size1)
        {
            if((size0 != 1) && (size1 != 1))
            {
                goto OnError;
            }
        }
    }

    borderMode->mode = VX_BORDER_REPLICATE;

    if (TENSOR_DIM_NUM(input0) == 1)
    {
        vx_uint32 w         = TENSOR_VIEW_SIZE_INDEX(input0, 0);
        vx_int32  sizes[4]  = {w, 1, 1, 1};

        dims = 2;

        src0 = vxoTensor_ReshapeTensor(input0, sizes, dims);
        parameters[0] = (vx_reference)src0;
    }

    if (TENSOR_DIM_NUM(input1) == 1)
    {
        vx_uint32 w         = TENSOR_VIEW_SIZE_INDEX(input1, 0);
        vx_int32  sizes[4]  = {w, 1, 1, 1};

        dims = 2;

        src1 = vxoTensor_ReshapeTensor(input1, sizes, dims);
        parameters[1] = (vx_reference)src1;
    }

    if (TENSOR_DIM_NUM(output) == 1)
    {
        vx_uint32 w         = TENSOR_VIEW_SIZE_INDEX(output, 0);
        vx_int32  sizes[4]  = {w, 1, 1, 1};

        dims = 2;

        dst = vxoTensor_ReshapeTensor(output, sizes, dims);
        parameters[2] = (vx_reference)dst;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorElewise, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorElewise.vx", path));

        vxmONERROR_NULLPTR(programSources = loadSources(path, &programLength));

        vxmONERROR_NULLPTR(program = vxCreateProgramWithSource(context, 1, (const vx_char**)&programSources, &programLength));

        if (programSources)
        {
            vxFree(programSources);
            programSources = NULL;
        }
#endif /*gcdUSE_VXC_BINARY*/
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto OnError;

        status = vxBuildProgram(program, VX_NULL);
        if (status != VX_SUCCESS) goto OnError;

        if(operation == VX_TENSOR_OP_ADD)
        {
            kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorAdd", program, paramNum, kernelEnum);
        }
        else if(operation == VX_TENSOR_OP_SUB)
        {
            kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorSub", program, paramNum, kernelEnum);
        }
        else if (operation == VX_TENSOR_OP_MUL)
        {
            kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorMul", program, paramNum, kernelEnum);
        }
        else if (operation == VX_TENSOR_OP_DIV)
        {
            kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorDiv", program, paramNum, kernelEnum);
        }
        else
        {
            vxError("Invalid operation type at function %s line %d", __FUNCTION__, __LINE__);
            goto OnError;
        }
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if(input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramNum);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if(input0_format == VX_TYPE_UINT8 && input1_format == VX_TYPE_UINT8)
    {
        vx_scalar scaleIn0 = vxCreateScalar(context, VX_TYPE_FLOAT32, &input_scale0);
        vx_scalar scaleIn1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &input_scale1);
        vx_scalar scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &output_scale);
        vx_scalar zpIn0 = vxCreateScalar(context, VX_TYPE_UINT32, &input_ZP0);
        vx_scalar zpIn1 = vxCreateScalar(context, VX_TYPE_UINT32, &input_ZP1);
        vx_scalar zpOut = vxCreateScalar(context, VX_TYPE_UINT32, &output_ZP);

        parameters[paramNum++] = (vx_reference)scaleIn0;
        parameters[paramNum++] = (vx_reference)scaleIn1;
        parameters[paramNum++] = (vx_reference)scaleOut;
        parameters[paramNum++] = (vx_reference)zpIn0;
        parameters[paramNum++] = (vx_reference)zpIn1;
        parameters[paramNum++] = (vx_reference)zpOut;
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable)
        {
            if(scaleIn0) vxReleaseScalar(&scaleIn0);
            if(scaleIn1) vxReleaseScalar(&scaleIn1);
            if(scaleOut) vxReleaseScalar(&scaleOut);
            if(zpIn0) vxReleaseScalar(&zpIn0);
            if(zpIn1) vxReleaseScalar(&zpIn1);
            if(zpOut) vxReleaseScalar(&zpOut);
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramNum);
        if(scaleIn0) vxReleaseScalar(&scaleIn0);
        if(scaleIn1) vxReleaseScalar(&scaleIn1);
        if(scaleOut) vxReleaseScalar(&scaleOut);
        if(zpIn0) vxReleaseScalar(&zpIn0);
        if(zpIn1) vxReleaseScalar(&zpIn1);
        if(zpOut) vxReleaseScalar(&zpOut);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("Invalid data type at function %s line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (src0) vxoTensor_ReleaseTensor(&src0);
    if (src1) vxoTensor_ReleaseTensor(&src1);
    if (dst) vxoTensor_ReleaseTensor(&dst);

    return shaderExecutable;

OnError:
    if (src0) vxoTensor_ReleaseTensor(&src0);
    if (src1) vxoTensor_ReleaseTensor(&src1);
    if (dst) vxoTensor_ReleaseTensor(&dst);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }

    return VX_NULL;
}


