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

#define IMG_MAX_WIDTH (65536)
#define SHADER_THREAD_COUNT  (4)
#define _vxcFILENAME_MAX 1024
#define ALIGN_SIZE4 (4)
#define _GC_OBJ_ZONE            gcdZONE_VX_GPULAYER

#if gcdUSE_VXC_BINARY
static void * getGPUKernelInfo(vx_context context, nngpu_kernel_enum type, vx_uint32_ptr len)
{
    gceSTATUS status = gcvSTATUS_OK;

    GetBinaryPtr_FUNC funcHandle = VX_NULL;
    status = gcoOS_GetProcAddress(gcvNULL, context->globalData->libNNGPUKernelHandle, "GetBinaryPtr", (gctPOINTER *)&funcHandle);
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
    char *                           programSources = NULL;
#endif
    vx_program                       program = VX_NULL;
    vx_status                        status = VX_FAILURE;
    vxnne_shader_executable          shaderExecutable = VX_NULL;
    vxnne_kernel_shaders             kernel;
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
    vx_scalar    scaleIn            = VX_NULL;
    vx_scalar    scaleOut           = VX_NULL;
    vx_scalar    zeroPointIn        = VX_NULL;
    vx_scalar    zeroPointOut       = VX_NULL;
    vx_scalar    padX               = VX_NULL;
    vx_scalar    padY               = VX_NULL;
    vx_int32     res                = filterValue[0] % 4;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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
        void * ptr = getGPUKernelInfo(context, AvgPooling, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        if ((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
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
            vx_float32 outScaleValue = (vx_float32)1.0/TENSOR_TF_SCALE(output);
            vx_reference parameters[10] = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, (vx_reference)output};

            if(filterValue[0] == 1)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadQuant8", borderMode);
            else if(filterValue[0] == 7 && filterValue[1] == 7)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadQuant8_7x7", borderMode);
            else if(res == 0)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadQuant8_opt", borderMode);
            else if(res == 1)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadQuant8_res1", borderMode);
            else if(res == 2)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadQuant8_res2", borderMode);
            else if(res == 3)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadQuant8_res3", borderMode);
            if (!shaderExecutable) goto OnError;

            if(filterValue[0] != 1)
            {
                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;
            }

            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &input->scale);
            scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outScaleValue);
            zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &input->zeroPoint);
            zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &output->zeroPoint);

            parameters[1] = (vx_reference)poolSizeX;
            parameters[2] = (vx_reference)poolSizeY;
            parameters[3] = (vx_reference)stride_x;
            parameters[4] = (vx_reference)stride_y;
            parameters[5] = (vx_reference)scaleIn;
            parameters[6] = (vx_reference)scaleOut;
            parameters[7] = (vx_reference)zeroPointIn;
            parameters[8] = (vx_reference)zeroPointOut;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else if((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
                (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        {
            vx_reference parameters[6] = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, VX_NULL, (vx_reference)output};

            if(filterValue[0] == 1)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadFP32", borderMode);
            else if(res == 0)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadFP32_opt", borderMode);
            else if(res == 1)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadFP32_res1", borderMode);
            else if(res == 2)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadFP32_res2", borderMode);
            else if(res == 3)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgNoPadFP32_res3", borderMode);
            if (!shaderExecutable) goto OnError;

            if(filterValue[0] != 1)
            {
                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;
            }

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
            vx_float32 outScaleValue = (vx_float32)1.0/TENSOR_TF_SCALE(output);
            vx_reference parameters[12] = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, (vx_reference)output};

            if(filterValue[0] < 4)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgPadQuant8", borderMode);
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgPadQuant8_opt", borderMode);
            if (!shaderExecutable) goto OnError;

            if(filterValue[0] >= 4)
            {
                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;
            }

            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &input->scale);
            scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outScaleValue);
            zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &input->zeroPoint);
            zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &output->zeroPoint);
            padX = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_x_left);
            padY = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_y_top);

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
            if (status != VX_SUCCESS) goto OnError;
        }
        else if((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
                (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        {
            vx_reference parameters[8] = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, (vx_reference)output};

            if(filterValue[0] < 4)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgPadFP32", borderMode);
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgPadFP32_opt", borderMode);
            if (!shaderExecutable) goto OnError;

            if(filterValue[0] >= 4)
            {
                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;
            }

            padX = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_x_left);
            padY = vxCreateScalar(context, VX_TYPE_INT32, &pool_pad_y_top);

            parameters[1] = (vx_reference)poolSizeX;
            parameters[2] = (vx_reference)poolSizeY;
            parameters[3] = (vx_reference)stride_x;
            parameters[4] = (vx_reference)stride_y;
            parameters[5] = (vx_reference)padX;
            parameters[6] = (vx_reference)padY;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

    status = vxnneShaderExecutable_GetMaxWorkGroupSize(shaderExecutable, &maxWorkGroupSize);
    if (status != VX_SUCCESS) goto OnError;

    execution_parameters.globalWorkSize[0]   = out_width;
    execution_parameters.globalWorkSize[1]   = out_height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
    if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
    if(padX) vxReleaseScalar(&padX);
    if(padY) vxReleaseScalar(&padY);
    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
    if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
    if(padX) vxReleaseScalar(&padX);
    if(padY) vxReleaseScalar(&padY);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar scale = NULL;
    vx_scalar zp = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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

    if((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT16) &&
        (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32))
    {
        vx_reference parameters[2] = {(vx_reference)input_rs, (vx_reference)output_rs};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        if (_IsSameType(input, output))
        {
            vx_reference parameters[2] = {(vx_reference)input_rs, (vx_reference)output_rs};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_8Bto8B", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_float32 scale_in  = TENSOR_TF_SCALE(input);
            vx_float32  zp_in    = (vx_float32)TENSOR_TF_ZEROPOINT(input);
            vx_float32 scale_out = TENSOR_TF_SCALE(output);
            vx_float32  zp_out   = (vx_float32)TENSOR_TF_ZEROPOINT(output);
            vx_float32 uint8_scale = scale_in / scale_out;
            vx_float32 tail = zp_out - zp_in * uint8_scale + 0.5f;
            vx_scalar scale_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &uint8_scale);
            vx_scalar tail_s  = vxCreateScalar(context, VX_TYPE_FLOAT32, &tail);
            vx_reference parameters[4] = {(vx_reference)input_rs, (vx_reference)output_rs, (vx_reference)scale_s, (vx_reference)tail_s};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
            if (status != VX_SUCCESS) goto OnError;
        }
    }
    else if (inputFormat == VX_TYPE_UINT8 && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32))
    {
        vx_float32 scaleValue = TENSOR_TF_SCALE(input);
        vx_int32 zpValue = TENSOR_TF_ZEROPOINT(input);
        vx_reference parameters[4] = {(vx_reference)input_rs, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output_rs};

        scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);
        zp = vxCreateScalar(context, VX_TYPE_INT32, &zpValue);
        parameters[1] = (vx_reference)scale;
        parameters[2] = (vx_reference)zp;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Q8toFP32", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if ((inputFormat == VX_TYPE_FLOAT32 || inputFormat == VX_TYPE_FLOAT16) && outputFormat == VX_TYPE_UINT8)
    {
        vx_float32 scaleValue = 1.0f / TENSOR_TF_SCALE(output);
        vx_float32 zpValue = (vx_float32)TENSOR_TF_ZEROPOINT(output) + 0.5f;
        vx_reference parameters[4] = {(vx_reference)input_rs, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output_rs};

        scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);
        zp = vxCreateScalar(context, VX_TYPE_FLOAT32, &zpValue);
        parameters[1] = (vx_reference)scale;
        parameters[2] = (vx_reference)zp;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32toQ8", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if(zp) vxReleaseScalar(&zp);
    if(scale) vxReleaseScalar(&scale);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (zp) vxReleaseScalar(&zp);
    if (scale) vxReleaseScalar(&scale);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable   = VX_NULL;
    vxnne_kernel_shaders    kernel             = VX_NULL;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[2]     = {(vx_reference)input, (vx_reference)output};
    vx_uint32    dims              = TENSOR_DIM_NUM(input);
    vx_uint32    width             = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    height            = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth             = (dims > 2) ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_uint32    tmp               = 0;
    vx_bool      optFlg            = vx_false_e;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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

    execution_parameters.globalWorkScale[0]  = 1;
    execution_parameters.globalWorkScale[1]  = 1;
    execution_parameters.globalWorkScale[2]  = 1;

    if(pnum == 3)
    {
        if((inputFormat == VX_TYPE_FLOAT16) || (inputFormat == VX_TYPE_FLOAT32))
        {
            if (perm[0] == 2 && perm[1] == 0 && perm[2] == 1)
            {
                if(width % 4 == 0 && height % 4 == 0 && depth % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2CWH_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 1;
                    execution_parameters.globalWorkScale[2]  = 4;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2CWH", borderMode);
            }
            else if (perm[0] == 2 && perm[1] == 1 && perm[2] == 0)
            {
                if(width % 4 == 0 && depth % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2CHW_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 1;
                    execution_parameters.globalWorkScale[2]  = 4;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2CHW", borderMode);
            }
            else if (perm[0] == 0 && perm[1] == 2 && perm[2] == 1)
            {
                if(width % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2WCH_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 1;
                    execution_parameters.globalWorkScale[2]  = 1;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2WCH", borderMode);
            }
            else if (perm[0] == 1 && perm[1] == 2 && perm[2] == 0)
            {
                if(width % 4 == 0 && height % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2HCW_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 4;
                    execution_parameters.globalWorkScale[2]  = 1;
                    optFlg = vx_true_e;
                }
                else if(width == 3 && height % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_3HC2HC3_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 4;
                    execution_parameters.globalWorkScale[2]  = 1;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_WHC2HCW", borderMode);
            }
            else if (perm[0] == 1 && perm[1] == 0 && perm[2] == 2)
            {
                tmp = width;
                width = height;
                height = tmp;

                if(width % 4 == 0 && height % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_CHW2CWH_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 1;
                    execution_parameters.globalWorkScale[2]  = 1;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_CHW2CWH", borderMode);
            }
        }
        else if(inputFormat == VX_TYPE_UINT8)
        {
            if (perm[0] == 2 && perm[1] == 0 && perm[2] == 1)
            {
                if(width % 4 == 0 && height % 4 == 0 && depth % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2CWH_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 1;
                    execution_parameters.globalWorkScale[2]  = 4;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2CWH", borderMode);
            }
            else if (perm[0] == 2 && perm[1] == 1 && perm[2] == 0)
            {
                if(width % 4 == 0 && depth % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2CHW_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 1;
                    execution_parameters.globalWorkScale[2]  = 4;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2CHW", borderMode);
            }
            else if (perm[0] == 0 && perm[1] == 2 && perm[2] == 1)
            {
                if(width % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2WCH_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 1;
                    execution_parameters.globalWorkScale[2]  = 1;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2WCH", borderMode);
            }
            else if (perm[0] == 1 && perm[1] == 2 && perm[2] == 0)
            {
                if(width % 4 == 0 && height % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2HCW_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 4;
                    execution_parameters.globalWorkScale[2]  = 1;
                    optFlg = vx_true_e;
                }
                else if(width == 3 && height % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_3HC2HC3_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 4;
                    execution_parameters.globalWorkScale[2]  = 1;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_WHC2HCW", borderMode);
            }
            else if (perm[0] == 1 && perm[1] == 0 && perm[2] == 2)
            {
                tmp = width;
                width = height;
                height = tmp;

                if(width % 4 == 0 && height % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_CHW2CWH_opt", borderMode);
                    if (!shaderExecutable) goto OnError;

                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;

                    execution_parameters.globalWorkScale[0]  = 4;
                    execution_parameters.globalWorkScale[1]  = 1;
                    execution_parameters.globalWorkScale[2]  = 1;
                    optFlg = vx_true_e;
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_CHW2CWH", borderMode);
            }
        }

        execution_parameters.globalWorkSize[0]    = width;
        execution_parameters.globalWorkSize[1]    = height;
        execution_parameters.globalWorkSize[2]    = depth;

        if(optFlg)
        {
            execution_parameters.globalWorkSize[0]   = (width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0];
            execution_parameters.globalWorkSize[1]   = (height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1];
            execution_parameters.globalWorkSize[2]   = (depth + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2];
        }
    }
    else if(pnum == 2)
    {
        if((inputFormat == VX_TYPE_FLOAT16) || (inputFormat == VX_TYPE_FLOAT32))
        {
            if(width % 4 == 0 && height % 4 == 0)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_2D_opt", borderMode);
                if (!shaderExecutable) goto OnError;

                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;

                execution_parameters.globalWorkScale[0]  = 4;
                execution_parameters.globalWorkScale[1]  = 4;
                execution_parameters.globalWorkScale[2]  = 1;
                optFlg = vx_true_e;
            }
            else if(width == 3 && height % 4 == 0)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_2D_3h", borderMode);
                if (!shaderExecutable) goto OnError;

                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;

                execution_parameters.globalWorkScale[0]  = 4;
                execution_parameters.globalWorkScale[1]  = 4;
                execution_parameters.globalWorkScale[2]  = 1;
                optFlg = vx_true_e;
            }
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "FP32_2D", borderMode);
        }
        else if(inputFormat == VX_TYPE_UINT8)
        {
            if(width % 4 == 0 && height % 4 == 0)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_2D_opt", borderMode);
                if (!shaderExecutable) goto OnError;

                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;

                execution_parameters.globalWorkScale[0]  = 4;
                execution_parameters.globalWorkScale[1]  = 4;
                execution_parameters.globalWorkScale[2]  = 1;
                optFlg = vx_true_e;
            }
            else if(width == 3 && height % 4 == 0)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_2D_3h", borderMode);
                if (!shaderExecutable) goto OnError;

                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;

                execution_parameters.globalWorkScale[0]  = 4;
                execution_parameters.globalWorkScale[1]  = 4;
                execution_parameters.globalWorkScale[2]  = 1;
                optFlg = vx_true_e;
            }
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "Quant8_2D", borderMode);
        }
        else
        {
            vxError("Not supported data format at line %d of function %s.\n", __LINE__, __FUNCTION__);
        }

        execution_parameters.workDim = 2;
        execution_parameters.globalWorkSize[0]    = width;
        execution_parameters.globalWorkSize[1]    = height;

        if(optFlg)
        {
            execution_parameters.globalWorkSize[0]   = (width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0];
            execution_parameters.globalWorkSize[1]   = (height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1];
        }
    }
    else
    {
        vxError("Invalid parameter at line %d of function %s.\n", __LINE__, __FUNCTION__);
    }

    if (!shaderExecutable) goto OnError;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar zeroPointIn =NULL;
    vx_scalar zeroPointOut = NULL;
    vx_scalar scale = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    if (!((valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        || (valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        || (valueFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
        ))
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    borderMode->mode = VX_BORDER_CONSTANT;
    if (valueFormat == VX_TYPE_UINT8)
    {
        borderMode->constant_value.U8 = 0;
    }
    else if (valueFormat == VX_TYPE_FLOAT32)
    {
        borderMode->constant_value.S32 = 0;
    }
    else if (valueFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
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
        vx_reference  parameters[6] = {(vx_reference)input_rs, (vx_reference)value, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointInValue);
        zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointOutValue);
        scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);
        parameters[2] = (vx_reference)scale;
        parameters[3] = (vx_reference)zeroPointIn;
        parameters[4] = (vx_reference)zeroPointOut;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_quant8", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if((valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
            (valueFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
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
    if(scale) vxReleaseScalar(&scale);
    if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
    if(zeroPointOut) vxReleaseScalar(&zeroPointOut);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if(scale) vxReleaseScalar(&scale);
    if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
    if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
#endif
    vx_program program          = VX_NULL;
    vx_status  status           = VX_FAILURE;
    vx_uint32  dims             = TENSOR_DIM_NUM(input);
    vx_uint32  width            = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32  height           = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32  channels         = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32  out_width        = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32  batch            = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;
    vx_uint32  inputZp          = (vx_uint32)TENSOR_TF_ZEROPOINT(input);
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {1, 1, 1}, {0, 0, 0}, {0, 0, 0}};
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_scalar strideX = NULL;
    vx_scalar strideY = NULL;
    vx_scalar padX = NULL;
    vx_scalar padY = NULL;
    vx_scalar kernelX = NULL;
    vx_scalar kernelY = NULL;
    vx_scalar dilateX = NULL;
    vx_scalar dilateY = NULL;
    vx_scalar output_width = NULL;
    vx_scalar input_width  = NULL;
    vx_scalar input_zp     = NULL;
    vx_bool    enable_K1S1      = vx_false_e;
    vx_tensor  src              = VX_NULL;
    vx_tensor  dst              = VX_NULL;
    vx_bool    useImage2DFlag   = (vx_bool)((outputWidth * outputHeight < IMG_MAX_WIDTH) && (out_width < IMG_MAX_WIDTH));
    vx_bool    is_castformat    = vx_false_e;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    borderMode->mode = VX_BORDER_CONSTANT;
    if (inputFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        borderMode->constant_value.U8 = (vx_uint8)TENSOR_TF_ZEROPOINT(input);
    }
    else if (inputFormat == VX_TYPE_FLOAT32)
    {
        borderMode->constant_value.S32 = 0;
    }

    if (kernelSize_x == 1 && kernelSize_y == 1 && stride_x == 1 && stride_y == 1
     && padding_x == 0 && padding_y == 0 && dilation_x == 1 && dilation_y == 1
     && useImage2DFlag && (inputFormat == VX_TYPE_INT8 || inputFormat == VX_TYPE_UINT8))
    {
        vx_int32 sizes[4] = {width * height, channels, 1, batch};

        src = vxoTensor_ReshapeTensor(input, sizes, dims);

        sizes[0] = out_width;
        sizes[1] = outputWidth * outputHeight;

        dst = vxoTensor_ReshapeTensor(output, sizes, dims);

        enable_K1S1 = vx_true_e;

        borderMode->mode = VX_BORDER_REPLICATE;
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

    if (enable_K1S1)
    {
        vx_reference parameters[2] = {(vx_reference)src, (vx_reference)dst};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_8Bits_K1S1", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);

        if (status != VX_SUCCESS)
        {
            goto OnError;
        }
    }
    else
    {
        vx_reference parameters[10] = {(vx_reference)input, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL,
            (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        strideX = vxCreateScalar(context, VX_TYPE_INT32, &stride_x);
        strideY = vxCreateScalar(context, VX_TYPE_INT32, &stride_y);
        padX = vxCreateScalar(context, VX_TYPE_INT32, &padding_x);
        padY = vxCreateScalar(context, VX_TYPE_INT32, &padding_y);
        kernelX = vxCreateScalar(context, VX_TYPE_INT32, &kernelSize_x);
        kernelY = vxCreateScalar(context, VX_TYPE_INT32, &kernelSize_y);
        dilateX = vxCreateScalar(context, VX_TYPE_INT32, &dilation_x);
        dilateY = vxCreateScalar(context, VX_TYPE_INT32, &dilation_y);
        parameters[1] = (vx_reference)strideX;
        parameters[2] = (vx_reference)strideY;
        parameters[3] = (vx_reference)padX;
        parameters[4] = (vx_reference)padY;
        parameters[5] = (vx_reference)kernelX;
        parameters[6] = (vx_reference)kernelY;
        parameters[7] = (vx_reference)dilateX;
        parameters[8] = (vx_reference)dilateY;

        if (inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);

            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_UINT8)
        {
            if (kernelSize_x == 3 && kernelSize_y == 3 && stride_x == 2
             && padding_x == 0 && dilation_x == 1 && dilation_y == 1
             && useImage2DFlag )
            {
                if (useImage2DFlag)
                {
                    vx_int32 sizes[4] = {out_width, outputWidth * outputHeight, 1, batch};
                    dst = vxoTensor_ReshapeTensor(output, sizes, dims);
                }
                if (width % 16 == 0)
                {
                    src = vxoTensor_ReformatTensor(input, VX_TYPE_UINT32);
                    parameters[0] = (vx_reference)src;
                    is_castformat = vx_true_e;
                    borderMode->constant_value.U32 = ((inputZp << 24) | (inputZp << 16) | (inputZp << 8) | inputZp);
                }
                output_width = vxCreateScalar(context, VX_TYPE_INT32, &outputWidth);
                input_width  = vxCreateScalar(context, VX_TYPE_INT32, &width);
                input_zp     = vxCreateScalar(context, VX_TYPE_UINT32, &inputZp);
                parameters[6] = (vx_reference)input_zp;
                parameters[7] = (vx_reference)input_width;
                parameters[8] = (vx_reference)output_width;
                parameters[9] = (vx_reference)dst;
                if (is_castformat)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Q32_3x3_s2_dil1_Pad0_x8", borderMode);
                    execution_parameters.globalWorkScale[0] = 8;
                }
                else
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_3x3_s2_dil1_Pad0", borderMode);
                }
                if (!shaderExecutable)
                {
                    goto OnError;
                }
                status  = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 9, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS);
                if (status != VX_SUCCESS)
                {
                    goto OnError;
                }
            }
            else
            {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
                }
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);

            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_INT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant16", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);

            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else
        {
            vxError("input or output's format is not support");
            goto OnError;
        }
    }

    if (enable_K1S1)
    {
        execution_parameters.workDim = 2;

        execution_parameters.globalWorkSize[0]   = width * height;
        execution_parameters.globalWorkSize[1]   = channels;
    }
    else
    {
        execution_parameters.globalWorkSize[0]   = (outputWidth + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0];
        execution_parameters.globalWorkSize[1]   = outputHeight;
        execution_parameters.globalWorkSize[2]   = channels;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(strideX) vxReleaseScalar(&strideX);
    if(strideY) vxReleaseScalar(&strideY);
    if(padX) vxReleaseScalar(&padX);
    if(padY) vxReleaseScalar(&padY);
    if(kernelX) vxReleaseScalar(&kernelX);
    if(kernelY) vxReleaseScalar(&kernelY);
    if(dilateX) vxReleaseScalar(&dilateX);
    if(dilateY) vxReleaseScalar(&dilateY);
    if (output_width) vxReleaseScalar(&output_width);
    if (input_width) vxReleaseScalar(&input_width);
    if (input_zp) vxReleaseScalar(&input_zp);
    if (src) vxoTensor_ReleaseTensor(&src);
    if (dst) vxoTensor_ReleaseTensor(&dst);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(strideX) vxReleaseScalar(&strideX);
    if(strideY) vxReleaseScalar(&strideY);
    if(padX) vxReleaseScalar(&padX);
    if(padY) vxReleaseScalar(&padY);
    if(kernelX) vxReleaseScalar(&kernelX);
    if(kernelY) vxReleaseScalar(&kernelY);
    if(dilateX) vxReleaseScalar(&dilateX);
    if(dilateY) vxReleaseScalar(&dilateY);
    if (output_width) vxReleaseScalar(&output_width);
    if (input_width) vxReleaseScalar(&input_width);
    if (input_zp) vxReleaseScalar(&input_zp);
    if (src) vxoTensor_ReleaseTensor(&src);
    if (dst) vxoTensor_ReleaseTensor(&dst);

    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar cycle = NULL;
    vx_scalar scaleIn = NULL;
    vx_scalar scaleWeight = NULL;
    vx_scalar scaleOut = NULL;
    vx_scalar zpIn = NULL;
    vx_scalar zpWeight = NULL;
    vx_scalar zpOut = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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
    else if (inputFormat == VX_TYPE_FLOAT32)
    {
        borderMode->constant_value.S32 = 0;
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

    if (inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32)
    {
        vx_reference parameters[4]     = {(vx_reference)input, (vx_reference)weights, (vx_reference)cycle, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        vx_reference parameters[10]     = {(vx_reference)input, (vx_reference)weights, (vx_reference)cycle, (vx_reference)NULL,
                                            (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL,
                                            (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &input_scale);
        scaleWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weight_scale);
        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &output_scale);
        zpIn = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        zpWeight = vxCreateScalar(context, VX_TYPE_INT32, &weight_ZP);
        zpOut = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        parameters[3] = (vx_reference)scaleIn;
        parameters[4] = (vx_reference)scaleWeight;
        parameters[5] = (vx_reference)scaleOut;
        parameters[6] = (vx_reference)zpIn;
        parameters[7] = (vx_reference)zpWeight;
        parameters[8] = (vx_reference)zpOut;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (cycle) vxReleaseScalar(&cycle);
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (scaleWeight) vxReleaseScalar(&scaleWeight);
    if (scaleOut) vxReleaseScalar(&scaleOut);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (zpWeight) vxReleaseScalar(&zpWeight);
    if (zpOut) vxReleaseScalar(&zpOut);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (scaleWeight) vxReleaseScalar(&scaleWeight);
    if (scaleOut) vxReleaseScalar(&scaleOut);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (zpWeight) vxReleaseScalar(&zpWeight);
    if (zpOut) vxReleaseScalar(&zpOut);
    if (program) vxReleaseProgram(&program);
    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (cycle) vxReleaseScalar(&cycle);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/********gpuGEMM**********************************************/
vxnne_shader_executable vxnneGPUGemmShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_bool                 enable_reorgWeights,
    vx_tensor               input,
    vx_tensor               weight,
    vx_tensor               bias,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char        *programSources          = NULL;
#endif
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {1, 1, 1}, {0, 0, 0}, {0, 0, 0}};
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders    kernel;
    vx_program  program                  = VX_NULL;
    vx_status   status                   = VX_FAILURE;
    vx_uint32   inputSize                = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32   width                    = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32   height                   = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_uint32   depth                    = TENSOR_VIEW_SIZE_INDEX(output, 2);
    vx_uint32   dims                     = TENSOR_DIM_NUM(output) == 1 ? 2 : TENSOR_DIM_NUM(output);
    vx_uint32   kernel_x                 = TENSOR_VIEW_SIZE_INDEX(weight, 0);
    vx_uint32   kernel_y                 = TENSOR_VIEW_SIZE_INDEX(weight, 1);
    vx_uint32   ifm                      = TENSOR_VIEW_SIZE_INDEX(weight, 2);
    vx_uint32   ofm                      = TENSOR_VIEW_SIZE_INDEX(weight, 3);
    vx_tensor   weights                  = NULL;
    vx_tensor   biases                   = NULL;
    vx_scalar   cycle                    = NULL;
    vx_uint32   input_ZP                 = TENSOR_TF_ZEROPOINT(input);
    vx_uint32   weight_ZP                = TENSOR_TF_ZEROPOINT(weight);
    vx_uint32   output_ZP                = TENSOR_TF_ZEROPOINT(output);
    vx_float32  input_scale              = TENSOR_TF_SCALE(input);
    vx_float32  weight_scale             = TENSOR_TF_SCALE(weight);
    vx_float32  output_scale             = (vx_float32)1.0/TENSOR_TF_SCALE(output);

    vx_enum     inputFormat              = input->tensorBuffer->dataFormat;
    vx_int32    size[]                   = {1, 1, 1, 1};
    vx_scalar   scaleIn                  = NULL;
    vx_scalar   zpIn                     = NULL;
    vx_scalar   zpWeight                 = NULL;
    vx_scalar   zpOut                    = NULL;
    vx_tensor   inputs                   = NULL;
    vx_tensor   outputs                  = NULL;
    vx_int32    output_width             = width;
    vx_int32    output_depth             = depth;
    vx_bool     enable_four_pixel        = vx_false_e;
    vx_bool     is_static_weights_biases = vx_false_e;
    vx_bool     enable_adjust_biases     = vx_false_e;
    vx_bool     enable_2d_img            = vx_false_e;
    vx_bool     enable_small_kernel      = vx_false_e;
    vx_uint32   element_cnt_input        = 0;
    vx_uint32   element_cnt_kernel       = 0;
    vx_float32  radio                    = 0;


    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    status  = vxoTensor_GetTensorElementCount(input, &element_cnt_input);
    status |= vxoTensor_GetTensorElementCount(weight, &element_cnt_kernel);
    if (status != VX_SUCCESS) goto OnError;

    radio = (vx_float32)element_cnt_kernel / (vx_float32)element_cnt_input;
    enable_small_kernel = radio < 0.5 ? vx_true_e : vx_false_e;

    is_static_weights_biases = (vx_bool)(TENSOR_DATA_LIFETIME(weight) == VX_TENSOR_LIFE_TIME_STATIC && TENSOR_DATA_LIFETIME(bias) == VX_TENSOR_LIFE_TIME_STATIC);
    enable_adjust_biases     = is_static_weights_biases && TENSOR_QUANT_TYPE(weight) == VX_QUANT_AFFINE_SCALE && TENSOR_QUANT_TYPE(bias);

    size[0] = kernel_x * kernel_y * ifm;
    size[1] = ofm;
    dims    = TENSOR_DIM_NUM(weight) == 1 ? 2 : TENSOR_DIM_NUM(weight);
    weights = vxoTensor_ReshapeTensor(weight, size, dims);

    size[0] = TENSOR_VIEW_SIZE_INDEX(bias, 0);
    size[1] = 1;
    dims    = TENSOR_DIM_NUM(bias) == 1 ? 2 : TENSOR_DIM_NUM(bias);
    biases  = vxoTensor_ReshapeTensor(bias, size, dims);

    enable_2d_img = (vx_bool)((width * height < IMG_MAX_WIDTH) && depth < IMG_MAX_WIDTH
        && is_static_weights_biases);

    enable_small_kernel = enable_small_kernel && enable_2d_img && (depth > 4);

    borderMode->mode = VX_BORDER_REPLICATE;

    if (kernel_x * kernel_y * ifm == inputSize * 4 )
        enable_reorgWeights = vx_true_e;

    if (enable_2d_img)
    {
        output_width = width * height;

        size[0] = inputSize;
        size[1] = width * height;
        dims    = TENSOR_DIM_NUM(input);
        inputs  = vxoTensor_ReshapeTensor(input, size, dims);

        size[0] = width * height;
        size[1] = depth;
        dims    = TENSOR_DIM_NUM(input);
        outputs = vxoTensor_ReshapeTensor(output, size, dims);
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

    if (enable_reorgWeights)
    {
        vx_reference parameters[4] = {(vx_reference)input, (vx_reference)weights, (vx_reference)biases, (vx_reference)output};
        vx_float32 uint8Scale  = input_scale * weight_scale * output_scale;
        vx_uint32 weightsSize = kernel_x * kernel_y * ifm;
        vx_float32 outputZP = (vx_float32)output_ZP + 0.5f;
        vx_float32 weightZP = (vx_float32)weight_ZP;


        if (enable_2d_img)
        {
            parameters[0]  = (vx_reference)inputs;
            parameters[3]  = (vx_reference)outputs;
        }

        if (enable_2d_img)
        {
            execution_parameters.globalWorkScale[0] = 4;
            execution_parameters.globalWorkScale[1] = 16;
            if (inputSize % 4 == 0)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_ReorgWeights_2D_4x", borderMode);
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_ReorgWeights_2D_4s", borderMode);
        }

        if (!shaderExecutable) goto OnError;

        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uint8Scale", 1, &uint8Scale);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "inputSize", 1, &inputSize);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "weightsSize", 1, &weightsSize);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "weight_ZP", 1, &weightZP);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outputZP", 1, &outputZP);
        if (status != VX_SUCCESS) goto OnError;

        status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
        if (inputSize % 4 == 0)
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 3, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);

        status |= vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32)
    {
        vx_reference parameters[5]     = {(vx_reference)input, (vx_reference)weights, (vx_reference)biases, (vx_reference)cycle, (vx_reference)output};

        if (enable_2d_img)
        {
            parameters[0]  = (vx_reference)inputs;
            parameters[4]  = (vx_reference)outputs;
        }

        if (is_static_weights_biases)
        {
            if ((output_width % ALIGN_SIZE4 == 0) && enable_small_kernel == vx_false_e)
                enable_four_pixel = vx_true_e;

            if (enable_2d_img)
            {
                if (enable_small_kernel)
                {
                    execution_parameters.globalWorkScale[1] = 4;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32_2D_4S", borderMode);
                }
                else if (enable_four_pixel)
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32_2D_4X", borderMode);
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32_2D", borderMode);
                if (!shaderExecutable) goto OnError;
            }
            else
            {
                if (enable_four_pixel)
                {
                    execution_parameters.globalWorkScale[1] = 4;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32_4X", borderMode);
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
                if (!shaderExecutable) goto OnError;
            }

            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (enable_small_kernel)
            {
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            }
            if (enable_four_pixel)
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 4, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32_non_static", borderMode);
            if (!shaderExecutable) goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        vx_float32 outZP        = (vx_float32)output_ZP;
        vx_float32 uint8_scale  = input_scale * weight_scale * output_scale;

        vx_reference parameters[9] = {(vx_reference)input, (vx_reference)weights, (vx_reference)biases, (vx_reference)cycle,
                     (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &uint8_scale);
        zpIn = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        zpWeight = vxCreateScalar(context, VX_TYPE_INT32, &weight_ZP);
        zpOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outZP);
        parameters[4] = (vx_reference)scaleIn;
        parameters[5] = (vx_reference)zpIn;
        parameters[6] = (vx_reference)zpWeight;
        parameters[7] = (vx_reference)zpOut;

        if (enable_2d_img)
        {
            parameters[0]  = (vx_reference)inputs;
            parameters[8]  = (vx_reference)outputs;
        }

        if (enable_adjust_biases)
        {
            if ((output_width % ALIGN_SIZE4 == 0) && enable_small_kernel == vx_false_e)
                enable_four_pixel = vx_true_e;

            if (enable_2d_img)
            {
                if (enable_small_kernel)
                {
                    execution_parameters.globalWorkScale[1] = 4;
                    if (output_width % ALIGN_SIZE4 == 0)
                    {
                        execution_parameters.globalWorkScale[0] = 4;
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_2D_4XS", borderMode);
                        if (!shaderExecutable) goto OnError;

                        status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 8, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                        if (status != VX_SUCCESS) goto OnError;
                    }
                    else
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_2D_4S", borderMode);
                }
                else if (enable_four_pixel)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_2D_4X", borderMode);
                    if (!shaderExecutable) goto OnError;
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 8, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                }
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_2D", borderMode);
            }
            else
            {
                if (enable_four_pixel)
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_4X", borderMode);
                else
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            }

            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (enable_small_kernel)
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);

            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_non_static", borderMode);
            if (!shaderExecutable) goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 9);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if(inputFormat == VX_TYPE_INT16)
    {
        vx_int8   srcFixedPointPos  = TENSOR_POS(input);
        vx_int8   weiFixedPointPos  = TENSOR_POS(weights);
        vx_int8   dstFixedPointPos  = TENSOR_POS(output);
        vx_int32  postshift         = 0;
        vx_float32   in_scale       = 0;

        vx_reference parameters[6] = {(vx_reference)input, (vx_reference)weights, (vx_reference)biases, (vx_reference)cycle,
                     (vx_reference)NULL, (vx_reference)output};

        postshift = postshift - srcFixedPointPos;
        postshift = postshift - weiFixedPointPos;
        postshift = postshift + dstFixedPointPos;

        if (postshift < 0)
        {
            in_scale = 1.0f / (vx_float32) (1 << -postshift);
        }
        else
        {
            in_scale = (vx_float32) (1 << postshift);
        }

        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale);
        parameters[4] = (vx_reference)scaleIn;

        if (enable_adjust_biases)
        {
            if (enable_2d_img)
            {
                parameters[0]  = (vx_reference)inputs;
                parameters[5]  = (vx_reference)outputs;

                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant16_2D", borderMode);
            }
            else
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant16", borderMode);
            }
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant16_non_static", borderMode);
        }
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
        if (status != VX_SUCCESS) goto OnError;
    }

    if (enable_2d_img)
    {
        execution_parameters.workDim = 2;
        if (enable_four_pixel)
            execution_parameters.globalWorkScale[0] = 4;

        if (enable_small_kernel && enable_2d_img)
            output_depth = depth;


        execution_parameters.localWorkSize[0]  = 1;
        execution_parameters.localWorkSize[1]  = 8;

        execution_parameters.globalWorkSize[0] = gcmALIGN((output_width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1] = gcmALIGN((output_depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    }
    else
    {
        if (enable_four_pixel)
            execution_parameters.globalWorkScale[0] = 4;

        execution_parameters.localWorkSize[0]  = SHADER_THREAD_COUNT;
        execution_parameters.localWorkSize[1]  = context->nnConfig.fixedFeature.shaderCoreCount;
        execution_parameters.localWorkSize[2]  =1;
        execution_parameters.globalWorkSize[0] = gcmALIGN((output_width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1] = gcmALIGN((height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
        execution_parameters.globalWorkSize[2] = depth;
    }

    if (!shaderExecutable) goto OnError;
    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (inputs) vxoTensor_ReleaseTensor(&inputs);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (biases) vxoTensor_ReleaseTensor(&biases);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (cycle) vxReleaseScalar(&cycle);
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (zpWeight) vxReleaseScalar(&zpWeight);
    if (zpOut) vxReleaseScalar(&zpOut);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (inputs) vxoTensor_ReleaseTensor(&inputs);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (zpWeight) vxReleaseScalar(&zpWeight);
    if (zpOut) vxReleaseScalar(&zpOut);
    if (program) vxReleaseProgram(&program);
    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (biases) vxoTensor_ReleaseTensor(&biases);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (cycle) vxReleaseScalar(&cycle);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    vx_int32                strideXvalue,
    vx_int32                strideYvalue,
    vx_tensor               outputs)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
#endif
    vx_program                       program              = VX_NULL;
    vx_status                        status               = VX_FAILURE;
    vxnne_shader_executable          shaderExecutable     = VX_NULL;
    vxnne_kernel_shaders             kernel               = NULL;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {1, 1, 1}, {0, 0, 0}, {0, 0, 0}};

    vx_uint32     dimCount                   = TENSOR_VIEW_DIM_NUM(inputs);
    vx_uint32     width                      = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32     height                     = (dimCount > 1) ? TENSOR_VIEW_SIZE_INDEX(inputs, 1) : 1;
    vx_uint32     depth                      = (dimCount > 2) ? TENSOR_VIEW_SIZE_INDEX(inputs, 2) : 1;
    vx_uint32     output_dimCount            = TENSOR_VIEW_DIM_NUM(outputs);
    vx_uint32     output_width               = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    vx_uint32     output_height              = (output_dimCount > 1) ? TENSOR_VIEW_SIZE_INDEX(outputs, 1) : 1;
    vx_uint32     output_depth               = (output_dimCount > 2) ? TENSOR_VIEW_SIZE_INDEX(outputs, 2) : 1;
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
    vx_uint32     biasZP                     = biases ? TENSOR_TF_ZEROPOINT(biases) : 0;
    vx_float32    biasScale                  = biases ? TENSOR_TF_SCALE(biases) : 0;
    vx_uint32     outputZP                   = TENSOR_TF_ZEROPOINT(outputs);
    vx_float32    outputScale                = (vx_float32)1.0/TENSOR_TF_SCALE(outputs);
    vx_scalar     strideX                    = VX_NULL;
    vx_scalar     strideY                    = VX_NULL;
    vx_scalar     kernelX                     = vxCreateScalar(context, VX_TYPE_INT32, &kernel_width);
    vx_scalar     kernelY                    = vxCreateScalar(context, VX_TYPE_INT32, &kernel_height);
    vx_tensor     reBiases                   = VX_NULL;
    vx_tensor     reWeights                  = VX_NULL;
    vx_bool       is_static_weights_biases   = vx_false_e;
    vx_bool       enable_adjust_biases       = vx_false_e;
    vx_bool       enable_2d_img              = vx_false_e;
    vx_bool       is_use_2d_fun              = vx_false_e;
    vx_scalar scaleIn = NULL;
    vx_scalar scaleWeight = NULL;
    vx_scalar scaleOut = NULL;
    vx_scalar zpIn = NULL;
    vx_scalar zpIn_int = NULL;
    vx_scalar zpWeight = NULL;
    vx_scalar zpOut = NULL;
    vx_uint32 dims              = 0;
    vx_int32  sizes[4]                       = {1, 1, 1, 1};
    vx_bool   is_write_2data = vx_false_e, is_write_4data = vx_false_e;
    vx_bool   is_write_q32_8data = vx_false_e, is_write_q32_16data = vx_false_e;
    vx_bool   is_write_q32_7data = vx_false_e, is_write_q32_14data = vx_false_e;
    vx_bool   is_no_pad = vx_false_e;
    vx_bool   enable_in_cast_format   = vx_false_e;
    vx_tensor    inputs_rs            = NULL;
    vx_tensor    outputs_rs           = NULL;
    vx_scalar    outputHeight         = NULL;
    vx_scalar    heightDiff           = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, inputs=%p, outputs=%p", context, kernelEnum, inputs, outputs);

    is_no_pad = ((0 == padLeftv) && (0 == padRightv) && (0 == padTopv) && (0 == padBottomv));
    if (width % 4 == 0)
    {
        enable_in_cast_format = vx_true_e;
    }
    if (is_no_pad)
    {
        enable_2d_img = (vx_bool)(((width  < IMG_MAX_WIDTH) && (height * depth < IMG_MAX_WIDTH))
                                    && ((output_width < IMG_MAX_WIDTH) && (output_depth * output_height < IMG_MAX_WIDTH)));
        if (enable_2d_img)
        {
            sizes[0] = width;
            sizes[1] = depth * height;
            dims    = TENSOR_DIM_NUM(inputs);
            inputs_rs  = vxoTensor_ReshapeTensor(inputs, sizes, dims);

            sizes[0] = output_width;
            sizes[1] = output_depth * output_height;
            dims    = TENSOR_DIM_NUM(outputs);
            outputs_rs = vxoTensor_ReshapeTensor(outputs, sizes, dims);
        }
    }
    if (biases)
    {
        if (fabs(inputScale*weightScale - biasScale) > 0.000001f || biasZP !=0)
        {
            vxError("biase's scale must equal to input's scale multiply weight's scale, and biase's zero point must equal to 0! failed at function %s line %d", __FUNCTION__, __LINE__);
            goto OnError;
        }
    }

    if (biases != VX_NULL)
    {
        is_static_weights_biases = (vx_bool)((TENSOR_DATA_LIFETIME(weights) == VX_TENSOR_LIFE_TIME_STATIC) && (TENSOR_DATA_LIFETIME(biases) == VX_TENSOR_LIFE_TIME_STATIC));
        enable_adjust_biases     = (is_static_weights_biases && (TENSOR_QUANT_TYPE(weights) == VX_QUANT_AFFINE_SCALE) && (TENSOR_QUANT_TYPE(biases) == VX_QUANT_AFFINE_SCALE));
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



    sizes[0] = kernel_width * kernel_height;
    sizes[1] = TENSOR_VIEW_SIZE_INDEX(weights, 2);
    reWeights = vxoTensor_ReshapeTensor(weights, sizes, 2);

    if (is_no_pad)
    {
        borderMode->mode = VX_BORDER_REPLICATE;
    }
    else
    {
        borderMode->mode = VX_BORDER_CONSTANT;
        if (inputFormat == VX_TYPE_FLOAT16)
            borderMode->constant_value.S16 = 0;
        else if (inputFormat == VX_TYPE_UINT8)
            borderMode->constant_value.U8 = (vx_uint8)inputZP;
        else if (inputFormat == VX_TYPE_FLOAT32)
            borderMode->constant_value.S32 = 0;
    }

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

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto OnError;

        kernel = vxnneAddKernelShadersInProgram(context, "gpuDepthwiseConv", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    {
        if((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32) && biases != VX_NULL)
        {
            vx_reference  parameters[11] = {(vx_reference)inputs, (vx_reference)reWeights, (vx_reference)reBiases, (vx_reference)channel_multiplier,
                                            (vx_reference)kernelX, (vx_reference)kernelY, (vx_reference)strideX, (vx_reference)strideY,
                                            (vx_reference)padXLeft, (vx_reference)padYTop, (vx_reference)outputs};
            if ((3 == kernel_width) && (3 == kernel_height))
            {
                if (inputFormat == VX_TYPE_FLOAT16)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_FP16", borderMode);
                }
                else
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_FP32", borderMode);
                }
                if (!shaderExecutable)
                {
                    goto OnError;
                }
                status  = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;
            }
            else
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
                if (!shaderExecutable) goto OnError;
            }
            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 11);
            if (status != VX_SUCCESS) goto OnError;
        }
        else if (inputFormat == VX_TYPE_UINT8 && biases != VX_NULL)
        {

            if ((3 == kernel_width) && (3 == kernel_height) && enable_adjust_biases)
            {
                if (1 == channel_multiplier->value->n32)
                {
                    if ((output_width == 7) && is_no_pad && enable_2d_img && enable_in_cast_format && (1 == strideXvalue || 2 == strideXvalue))
                    {
                        is_write_q32_7data = vx_true_e;
                        inputs_rs = vxoTensor_ReformatTensor(inputs_rs, VX_TYPE_UINT32);
                        if (1 == strideXvalue)
                        {
                            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_NoPad_Q32toQ8_w7_2D", borderMode);
                        }
                        else if (2 == strideXvalue)
                        {
                            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s2_NoPad_Q32toQ8_w7_2D", borderMode);
                        }
                        is_use_2d_fun   = vx_true_e;
                    }
                    else if ((output_width == 14) && is_no_pad && enable_2d_img && enable_in_cast_format && (1 == strideXvalue || 2 == strideXvalue))
                    {
                        is_write_q32_14data = vx_true_e;
                        inputs_rs = vxoTensor_ReformatTensor(inputs_rs, VX_TYPE_UINT32);
                        if (1 == strideXvalue)
                        {
                            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_NoPad_Q32toQ8_w14_2D", borderMode);
                        }
                        else if (2 == strideXvalue)
                        {
                            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s2_NoPad_Q32toQ8_w14_2D", borderMode);
                        }
                        is_use_2d_fun   = vx_true_e;
                    }
                    else if ((output_width % 16 == 0) && is_no_pad && enable_2d_img && enable_in_cast_format && (1 == strideXvalue))
                    {
                        is_write_q32_16data = vx_true_e;
                        inputs_rs = vxoTensor_ReformatTensor(inputs_rs, VX_TYPE_UINT32);
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_NoPad_Q32_x16_2D", borderMode);
                        is_use_2d_fun   = vx_true_e;
                    }
                    else if ((output_width % 8 == 0) && is_no_pad && enable_2d_img && enable_in_cast_format  && (1 == strideXvalue || 2 == strideXvalue))
                    {
                        is_write_q32_8data = vx_true_e;
                        inputs_rs = vxoTensor_ReformatTensor(inputs_rs, VX_TYPE_UINT32);
                        if (1 == strideXvalue)
                        {
                            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_NoPad_Q32_x8_2D", borderMode);
                        }
                        else if (2 == strideXvalue)
                        {
                            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s2_NoPad_Q32_x8_2D", borderMode);
                        }
                        is_use_2d_fun   = vx_true_e;
                    }
                    else if ((output_width % 4 == 0) && (padRightv <= 1) && (padLeftv <= 1) && (1 == strideXvalue || 2 == strideXvalue))
                    {
                        is_write_4data = vx_true_e;
                        if (1 == strideXvalue)
                        {
                            if (is_no_pad)
                            {
                                if (enable_in_cast_format && enable_2d_img)
                                {
                                    inputs_rs = vxoTensor_ReformatTensor(inputs_rs, VX_TYPE_UINT32);
                                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_NoPad_Q32toQ8_x4_2D", borderMode);
                                    is_use_2d_fun   = vx_true_e;
                                }
                                else if (enable_2d_img)
                                {
                                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_NoPad_Quant8_x4_2D", borderMode);
                                    is_use_2d_fun   = vx_true_e;
                                }
                                else
                                {
                                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_NoPad_Quant8_x4", borderMode);
                                }

                            }
                            else
                            {
                                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_Quant8_x4", borderMode);
                            }

                        }
                        else if (2 == strideXvalue)
                        {
                            if (enable_in_cast_format && enable_2d_img)
                            {
                                inputs_rs = vxoTensor_ReformatTensor(inputs_rs, VX_TYPE_UINT32);
                                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s2_NoPad_Q32toQ8_x4_2D", borderMode);
                                is_use_2d_fun   = vx_true_e;
                            }
                            else if (is_no_pad)
                            {
                                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s2_NoPad_Quant8_x4", borderMode);
                            }
                            else
                            {
                                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s2_Quant8_x4", borderMode);
                            }
                        }
                    }
                    else if ((output_width % 2 == 0) && (1 == strideXvalue))
                    {
                        is_write_2data = vx_true_e;
                        if (is_no_pad && enable_2d_img)
                        {
                            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_NoPad_Quant8_x2_2D", borderMode);
                            is_use_2d_fun   = vx_true_e;
                        }
                        else
                        {
                            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s1_Quant8_x2", borderMode);
                        }
                    }
                    else if ((output_width % 2 == 0) && (padRightv <= 1) && (padLeftv <= 1) && (2 == strideXvalue))
                    {
                        is_write_2data = vx_true_e;
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_s2_Quant8_x2", borderMode);
                    }
                    else
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_c1_Quant8", borderMode);
                    }
                }
                else
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_3x3_Quant8", borderMode);
                }
                if (!shaderExecutable)
                {
                    goto OnError;
                }

                if (is_write_q32_7data)
                {
                    status  = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                }
                else if (is_write_q32_14data)
                {
                    status  = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (is_use_2d_fun)
                    {
                        status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 10, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_TWO_COMPONENTS);
                    }
                    else
                    {
                        status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 15, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_TWO_COMPONENTS);
                    }
                }
                else if (is_write_4data || is_write_q32_16data || is_write_q32_8data)
                {
                    status  = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (is_use_2d_fun)
                    {
                        status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 10, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    }
                    else
                    {
                        status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 15, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    }
                }
                else if (is_write_2data)
                {
                    status  = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (is_use_2d_fun)
                    {
                        status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 10, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_TWO_COMPONENTS);
                    }
                    else
                    {
                        status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 15, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_TWO_COMPONENTS);
                    }
                }
                else
                {
                    status  = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS);
                }
                if (status != VX_SUCCESS) goto OnError;
            }
            else
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
                if (!shaderExecutable)
                {
                    goto OnError;
                }
            }

            if (is_use_2d_fun)
            {
                vx_reference  parameters[11] = {(vx_reference)inputs, (vx_reference)reWeights, (vx_reference)reBiases,
                        (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL,
                        (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)outputs};
                vx_float32  scale_out_value;
                vx_float32  inputZP_f   = (vx_float32)inputZP;
                vx_float32  weightZP_f  = (vx_float32)weightZP;
                vx_float32  outputZP_f  = (vx_float32)outputZP + 0.5f;
                vx_int32    height_diff = height - output_height;

                outputHeight = vxCreateScalar(context, VX_TYPE_INT32, &output_height);
                heightDiff   = vxCreateScalar(context, VX_TYPE_INT32, &height_diff);
                scale_out_value = inputScale * weightScale * outputScale;
                scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_out_value);
                zpIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputZP_f);
                zpWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weightZP_f);
                zpOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputZP_f);
                zpIn_int  = vxCreateScalar(context, VX_TYPE_INT32, &inputZP);
                parameters[3] = (vx_reference)outputHeight;
                parameters[4] = (vx_reference)heightDiff;
                parameters[5] = (vx_reference)scaleOut;
                parameters[6] = (vx_reference)zpIn;
                parameters[7] = (vx_reference)zpWeight;
                parameters[8] = (vx_reference)zpOut;
                parameters[9] = (vx_reference)zpIn_int;
                parameters[0] = (vx_reference)inputs_rs;
                parameters[10] = (vx_reference)outputs_rs;
                status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 11);
            }
            else
            {
                vx_reference  parameters[16] = {(vx_reference)inputs, (vx_reference)reWeights, (vx_reference)reBiases, (vx_reference)channel_multiplier,
                        (vx_reference)kernelX, (vx_reference)kernelY, (vx_reference)strideX, (vx_reference)strideY,
                        (vx_reference)padXLeft, (vx_reference)padYTop, (vx_reference)NULL, (vx_reference)NULL,
                        (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)outputs};
                vx_float32  scale_out_value;
                vx_float32  inputZP_f   = (vx_float32)inputZP;
                vx_float32  weightZP_f  = (vx_float32)weightZP;
                vx_float32  outputZP_f  = (vx_float32)outputZP + 0.5f;
                scale_out_value = inputScale * weightScale * outputScale;
                scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale_out_value);
                zpIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputZP_f);
                zpWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weightZP_f);
                zpOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputZP_f);
                zpIn_int  = vxCreateScalar(context, VX_TYPE_INT32, &inputZP);
                parameters[10] = (vx_reference)scaleOut;
                parameters[11] = (vx_reference)zpIn;
                parameters[12] = (vx_reference)zpWeight;
                parameters[13] = (vx_reference)zpOut;
                parameters[14] = (vx_reference)zpIn_int;
                status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 16);
            }


            if (status != VX_SUCCESS) goto OnError;
        }
        else if((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32) && biases == VX_NULL)
        {
            vx_reference  parameters[10] = {(vx_reference)inputs, (vx_reference)reWeights, (vx_reference)channel_multiplier,
                                (vx_reference)kernelX, (vx_reference)kernelY, (vx_reference)strideX, (vx_reference)strideY,
                                (vx_reference)padXLeft, (vx_reference)padYTop, (vx_reference)outputs};
            if ((3 == kernel_width) && (3 == kernel_height))
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "NoBias_3x3_FP32", borderMode);
                if (!shaderExecutable)
                {
                    goto OnError;
                }
                status  = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_THREE_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;
            }
            else
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "NoBias_FP32", borderMode);
                if (!shaderExecutable) goto OnError;
            }
            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS) goto OnError;
        }
        else if (inputFormat == VX_TYPE_UINT8 && biases == VX_NULL)
        {
            vx_reference  parameters[16] = {(vx_reference)inputs, (vx_reference)reWeights, (vx_reference)channel_multiplier,
                                (vx_reference)kernelX, (vx_reference)kernelY, (vx_reference)strideX, (vx_reference)strideY,
                                (vx_reference)padXLeft, (vx_reference)padYTop, (vx_reference)NULL, (vx_reference)NULL,
                                (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL,(vx_reference)outputs};


            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScale);
            scaleWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weightScale);
            scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScale);
            zpIn = vxCreateScalar(context, VX_TYPE_INT32, &inputZP);
            zpWeight = vxCreateScalar(context, VX_TYPE_INT32, &weightZP);
            zpOut = vxCreateScalar(context, VX_TYPE_INT32, &outputZP);
            parameters[9] = (vx_reference)scaleIn;
            parameters[10] = (vx_reference)scaleWeight;
            parameters[11] = (vx_reference)scaleOut;
            parameters[12] = (vx_reference)zpIn;
            parameters[13] = (vx_reference)zpWeight;
            parameters[14] = (vx_reference)zpOut;

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "NoBias_Quant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 16);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

    if (is_write_q32_7data)
    {
        execution_parameters.globalWorkScale[0]  = 7;
    }
    else if (is_write_q32_14data)
    {
        execution_parameters.globalWorkScale[0]  = 14;
    }
    else if (is_write_q32_16data)
    {
        execution_parameters.globalWorkScale[0]  = 16;
    }
    else if (is_write_q32_8data)
    {
        execution_parameters.globalWorkScale[0]  = 8;
    }
    else if (is_write_4data)
    {
        execution_parameters.globalWorkScale[0]  = 4;
    }
    else if (is_write_2data)
    {
        execution_parameters.globalWorkScale[0]  = 2;
    }

    if (is_use_2d_fun)
    {
        execution_parameters.workDim = 2;
        execution_parameters.localWorkSize[0]  = 1;
        execution_parameters.localWorkSize[1]  = 16;
        if (execution_parameters.localWorkSize[0])
        {
            execution_parameters.globalWorkSize[0] = gcmALIGN((output_width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
            execution_parameters.globalWorkSize[1] = gcmALIGN((output_height * output_depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
        }
        else
        {
            execution_parameters.globalWorkSize[0] = (output_width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0];
            execution_parameters.globalWorkSize[1] = (output_height * output_depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1];
        }
    }
    else
    {
        execution_parameters.globalWorkSize[0]   = (output_width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0];
        execution_parameters.globalWorkSize[1]   = (output_height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1];
        execution_parameters.globalWorkSize[2]   = output_depth;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (reBiases) vxoTensor_ReleaseTensor(&reBiases);
    if (reWeights) vxoTensor_ReleaseTensor(&reWeights);
    if (kernelX) vxReleaseScalar(&kernelX);
    if (kernelY) vxReleaseScalar(&kernelY);
    if (strideX) vxReleaseScalar(&strideX);
    if (strideY) vxReleaseScalar(&strideY);
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (scaleWeight) vxReleaseScalar(&scaleWeight);
    if (scaleOut) vxReleaseScalar(&scaleOut);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (zpWeight) vxReleaseScalar(&zpWeight);
    if (zpOut) vxReleaseScalar(&zpOut);
    if (inputs_rs) vxoTensor_ReleaseTensor(&inputs_rs);
    if (outputs_rs) vxoTensor_ReleaseTensor(&outputs_rs);
    if (outputHeight) vxReleaseScalar(&outputHeight);
    if (heightDiff) vxReleaseScalar(&heightDiff);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (scaleWeight) vxReleaseScalar(&scaleWeight);
    if (scaleOut) vxReleaseScalar(&scaleOut);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (zpWeight) vxReleaseScalar(&zpWeight);
    if (zpOut) vxReleaseScalar(&zpOut);
    if (program)  vxReleaseProgram(&program);
    if (reBiases) vxoTensor_ReleaseTensor(&reBiases);
    if (kernelX) vxReleaseScalar(&kernelX);
    if (kernelY) vxReleaseScalar(&kernelY);
    if (strideX) vxReleaseScalar(&strideX);
    if (strideY) vxReleaseScalar(&strideY);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if (inputs_rs) vxoTensor_ReleaseTensor(&inputs_rs);
    if (outputs_rs) vxoTensor_ReleaseTensor(&outputs_rs);
    if (outputHeight) vxReleaseScalar(&outputHeight);
    if (heightDiff) vxReleaseScalar(&heightDiff);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar zeroPointIn = NULL;
    vx_scalar zeroPointOut = NULL;
    vx_scalar scale = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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

        zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointInValue);
        zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointOutValue);
        scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);

        if(block_sizes->value->u32 == 2)
        {
            vx_reference  parameters[5] = {(vx_reference)input, (vx_reference)scale, (vx_reference)zeroPointIn, (vx_reference)zeroPointOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Depth2SpaceQuant8Block2", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else
        {
            vx_reference  parameters[6] = {(vx_reference)input, (vx_reference)block_sizes, (vx_reference)scale, (vx_reference)zeroPointIn, (vx_reference)zeroPointOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Depth2SpaceQuant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }
    else if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
             (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
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
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = output_width;
    execution_parameters.globalWorkSize[1]   = output_height;
    execution_parameters.globalWorkSize[2]   = output_depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(scale) vxReleaseScalar(&scale);
    if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
    if(zeroPointOut) vxReleaseScalar(&zeroPointOut);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scale) vxReleaseScalar(&scale);
    if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
    if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar zeroPointIn = NULL;
    vx_scalar zeroPointOut = NULL;
    vx_scalar scale = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    if (!((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
       || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
       || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
       || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
       || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        )
    {
        vxError("input or output's format is not support(space to depth)");
        goto OnError;
    }

    borderMode->mode = VX_BORDER_CONSTANT;
    if (inputFormat == VX_TYPE_UINT8)
    {
        borderMode->constant_value.U8 = 0;
    }
    else if (inputFormat == VX_TYPE_FLOAT32)
    {
        borderMode->constant_value.S32 = 0;
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
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

        zeroPointIn = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointInValue);
        zeroPointOut = vxCreateScalar(context, VX_TYPE_INT32, &zeroPointOutValue);
        scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);

        if (stride->value->u32 == 2)
        {
            vx_reference  parameters[5] = {(vx_reference)input, (vx_reference)scale, (vx_reference)zeroPointIn, (vx_reference)zeroPointOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Space2DepthQuant8Block2", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else
        {
            vx_reference  parameters[6] = {(vx_reference)input, (vx_reference)stride, (vx_reference)scale, (vx_reference)zeroPointIn, (vx_reference)zeroPointOut, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Space2DepthQuant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }
    else if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
             (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
    {
        if (stride->value->u32 == 2)
        {
            vx_reference  parameters[2] = {(vx_reference)input, (vx_reference)output};

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
    else if ((inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16))
    {
        vx_int8   srcFixedPointPos  = TENSOR_POS(input);
        vx_int8   dstFixedPointPos  = TENSOR_POS(output);
        vx_int32  postshift         = 0;
        vx_float32   in_scale       = 0;

        postshift = postshift - srcFixedPointPos;
        postshift = postshift + dstFixedPointPos;

        if (postshift < 0)
        {
            in_scale = 1.0f / (vx_float32) (1 << -postshift);
        }
        else
        {
            in_scale = (vx_float32) (1 << postshift);
        }
        scale = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale);

        if (stride->value->u32 == 2)
        {
            vx_reference  parameters[3] = {(vx_reference)input, (vx_reference)scale, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Space2DepthQuant16Block2", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else
        {
            vx_reference  parameters[4] = {(vx_reference)input, (vx_reference)stride, (vx_reference)scale, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Space2DepthQuant16", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(scale) vxReleaseScalar(&scale);
    if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
    if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scale) vxReleaseScalar(&scale);
    if(zeroPointIn) vxReleaseScalar(&zeroPointIn);
    if(zeroPointOut) vxReleaseScalar(&zeroPointOut);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    if (!((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
          (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
         ))
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    borderMode->mode = VX_BORDER_CONSTANT;
    if (inputFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }
    else if (inputFormat == VX_TYPE_FLOAT32)
    {
        borderMode->constant_value.S32 = 0;
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

    if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
        (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
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

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/********gpuFullyConnected****************************************************/
vxnne_shader_executable vxnneGetGPUFullyConnectedShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_bool                 enable_cast_format,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               bias,
    vx_int32                activation,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
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
    vx_scalar scaleIn = NULL;
    vx_scalar     scaleWeight      = NULL;
    vx_scalar     scaleOut         = NULL;
    vx_scalar     inZP             = NULL;
    vx_scalar     weightZP         = NULL;
    vx_scalar     outZP            = NULL;
    vx_scalar     cycle            = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    if (TENSOR_QUANT_TYPE(output) == VX_QUANT_DYNAMIC_FIXED_POINT)
    {
        vx_int8 dstFixPointPos    = TENSOR_POS(output);
        if (dstFixPointPos >= 0)
        {
            outputScaleValue = (vx_float32) (1 << dstFixPointPos);
        }
        else if (dstFixPointPos < 0)
        {
            outputScaleValue = 1.0f / (vx_float32) (1 << -dstFixPointPos);
        }
    }
    else if (TENSOR_QUANT_TYPE(output) == VX_QUANT_AFFINE_SCALE)
    {
        outputScaleValue   = 1.0f / TENSOR_TF_SCALE(output);
        outputZPValue = TENSOR_TF_ZEROPOINT(output);
    }

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

    borderMode->mode = VX_BORDER_CONSTANT;
    if (input_format == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }
    else if (input_format == VX_TYPE_UINT8)
    {
        borderMode->constant_value.U8 = (vx_uint8)inputZPValue;
    }
    else if (input_format == VX_TYPE_FLOAT32)
    {
        borderMode->constant_value.S32 = 0;
    }

    if (enable_cast_format)
        borderMode->mode = VX_BORDER_REPLICATE;

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

    if (enable_cast_format)
    {
        vx_float32 oZP          = (vx_float32)outputZPValue;
        vx_float32 iZP          = (vx_float32)inputZPValue;
        vx_float32 wZP          = (vx_float32)weightZPValue;
        vx_float32 uint8_scale  = inputScaleValue * weightScaleValue * outputScaleValue;

        vx_reference  parameters[9] = {(vx_reference)input_rs, (vx_reference)weight_rs, (vx_reference)bias_rs,
            (vx_reference)scaleIn, (vx_reference)inZP, (vx_reference)weightZP, (vx_reference)outZP,
            (vx_reference)cycle, (vx_reference)output_rs};

        cycle = vxCreateScalar(context, VX_TYPE_INT32, &inputSize);
        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &uint8_scale);
        inZP = vxCreateScalar(context, VX_TYPE_FLOAT32, &iZP);
        weightZP = vxCreateScalar(context, VX_TYPE_FLOAT32, &wZP);
        outZP = vxCreateScalar(context, VX_TYPE_FLOAT32, &oZP);

        parameters[3] = (vx_reference)scaleIn;
        parameters[4] = (vx_reference)inZP;
        parameters[5] = (vx_reference)weightZP;
        parameters[6] = (vx_reference)outZP;
        parameters[7] = (vx_reference)cycle;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant32_2D", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
        status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
        if (status != VX_SUCCESS) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 9);
        if (status != VX_SUCCESS)
        {
            goto OnError;
        }
    }
    else if (input_format == VX_TYPE_UINT8 &&  weights_format ==  VX_TYPE_UINT8 &&
            ((output_format == VX_TYPE_UINT8) || (output_format == VX_TYPE_INT32) || (output_format == VX_TYPE_INT16) ||
            (output_format == VX_TYPE_FLOAT32) || (output_format == VX_TYPE_FLOAT16)))
    {
        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScaleValue);
        scaleWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &weightScaleValue);
        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScaleValue);
        inZP = vxCreateScalar(context, VX_TYPE_INT32, &inputZPValue);
        weightZP = vxCreateScalar(context, VX_TYPE_INT32, &weightZPValue);
        outZP = vxCreateScalar(context, VX_TYPE_INT32, &outputZPValue);

        if (enable_bias)
        {
            vx_reference  parameters[10] = {(vx_reference)input_rs, (vx_reference)weight_rs, (vx_reference)bias_rs,
                                            (vx_reference)scaleIn, (vx_reference)scaleWeight, (vx_reference)scaleOut,
                                            (vx_reference)inZP, (vx_reference)weightZP, (vx_reference)outZP,
                                            (vx_reference)output_rs};

            vx_enum bias_format = TENSOR_DATA_TYPE(bias_rs);
            if (bias_format != VX_TYPE_INT32)
            {
                vxError("FC OCL not support bias format: %d\n", bias_format);
                goto OnError;
            }

            if ((output_format == VX_TYPE_FLOAT16) || (output_format == VX_TYPE_FLOAT32))
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AffineToFP", borderMode);

                if (!shaderExecutable)
                {
                    goto OnError;
                }
            }
            else if ((output_format == VX_TYPE_INT16) || (output_format == VX_TYPE_INT32))
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AffineToDFP", borderMode);

                if (!shaderExecutable)
                {
                    goto OnError;
                }
            }
            else if (output_format == VX_TYPE_UINT8)
            {
                if(inputSize % 4 == 0)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_VecQuant8", borderMode);
                }
                else
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
                }

                if (!shaderExecutable)
                {
                    goto OnError;
                }

                if(inputSize % 4 == 0)
                {
                    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                    if (status != VX_SUCCESS) goto OnError;
                }
            }
            else
            {
                vxError("FC OCL path doesn't support this output data type: %d\n", output_format);
                vxmASSERT(0);
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else
        {
            vx_reference  parameters[9] = {(vx_reference)input_rs, (vx_reference)weight_rs, (vx_reference)scaleIn,
                                            (vx_reference)scaleWeight, (vx_reference)scaleOut, (vx_reference)inZP,
                                            (vx_reference)weightZP, (vx_reference)outZP, (vx_reference)output_rs};

            if(inputSize % 4 == 0)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_VecQuant8Nobias", borderMode);
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8Nobias", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            if(inputSize % 4 == 0)
            {
                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 9);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }
    else if ((input_format == VX_TYPE_FLOAT16 && weights_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16) ||
            (input_format == VX_TYPE_FLOAT32 && weights_format == VX_TYPE_FLOAT32 && output_format == VX_TYPE_FLOAT32))
    {
        if (enable_bias)
        {
            vx_reference  parameters[4] = {(vx_reference)input_rs, (vx_reference)weight_rs, (vx_reference)bias_rs, (vx_reference)output_rs};

            if(inputSize % 4 == 0)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_VecFP32", borderMode);
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            if (!shaderExecutable) goto OnError;

            if(inputSize % 4 == 0)
            {
                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_reference  parameters[3] = {(vx_reference)input_rs, (vx_reference)weight_rs, (vx_reference)output_rs};

            if(inputSize % 4 == 0)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_VecFP32Nobias", borderMode);
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32Nobias", borderMode);
            if (!shaderExecutable) goto OnError;

            if(inputSize % 4 == 0)
            {
                status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
                if (status != VX_SUCCESS) goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS) goto OnError;
        }
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkScale[0] = 1;
    execution_parameters.globalWorkScale[1] = 1;
    execution_parameters.localWorkSize[0]  = 8;
    execution_parameters.localWorkSize[1]  = 1;
    execution_parameters.globalWorkSize[0] = gcmALIGN((num_units + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
    execution_parameters.globalWorkSize[1] = gcmALIGN((batch + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);


    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (weight_rs) vxoTensor_ReleaseTensor(&weight_rs);
    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleWeight) vxReleaseScalar(&scaleWeight);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(weightZP) vxReleaseScalar(&weightZP);
    if(outZP) vxReleaseScalar(&outZP);
    if(cycle) vxReleaseScalar(&cycle);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleWeight) vxReleaseScalar(&scaleWeight);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(weightZP) vxReleaseScalar(&weightZP);
    if(outZP) vxReleaseScalar(&outZP);
    if(cycle) vxReleaseScalar(&cycle);
    if (program) vxReleaseProgram(&program);
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (weight_rs) vxoTensor_ReleaseTensor(&weight_rs);
    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    if (!((valueFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        || (valueFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
        || (valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)))
    {
        gcmPRINT("input or output's format is not support");
        goto OnError;
    }

    borderMode->mode = VX_BORDER_CONSTANT;
    if (valueFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }
    else if (valueFormat == VX_TYPE_UINT8)
    {
        borderMode->constant_value.U8 = 0;
    }
    else if (valueFormat == VX_TYPE_FLOAT32)
    {
        borderMode->constant_value.S32 = 0;
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
    else if ((valueFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
             (valueFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
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

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (key_rs) vxoTensor_ReleaseTensor(&key_rs);
    if (value_rs) vxoTensor_ReleaseTensor(&value_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (hit_rs) vxoTensor_ReleaseTensor(&hit_rs);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar scaleIn = NULL;
    vx_scalar scaleOut = NULL;
    vx_scalar zp = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    borderMode->mode = VX_BORDER_CONSTANT;

    if (srcFormat == VX_TYPE_UINT8)
        borderMode->constant_value.U8 = (vx_uint8)TENSOR_TF_ZEROPOINT(input);
    else if (srcFormat == VX_TYPE_FLOAT16)
        borderMode->constant_value.S16 = 0;
    else
        borderMode->constant_value.S32 = 0;

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
        vx_float32 outputScale = (vx_float32)1.0/inputScale;
        vx_reference  parameters[5] = {(vx_reference)input, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScale);
        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScale);
        zp = vxCreateScalar(context, VX_TYPE_INT32, &inputZP);
        parameters[1] = (vx_reference)scaleIn;
        parameters[2] = (vx_reference)scaleOut;
        parameters[3] = (vx_reference)zp;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SumRsqrtQuant8", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (srcFormat == VX_TYPE_FLOAT16 || srcFormat == VX_TYPE_FLOAT32)
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

    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zp) vxReleaseScalar(&zp);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zp) vxReleaseScalar(&zp);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar scaleIn = NULL;
    vx_scalar scaleOut = NULL;
    vx_scalar inZP = NULL;
    vx_scalar outZP = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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

    if ((srcFormat == VX_TYPE_FLOAT16 && dstFormat == VX_TYPE_FLOAT16) ||
        (srcFormat == VX_TYPE_FLOAT32 && dstFormat == VX_TYPE_FLOAT32))
    {
        vx_reference  parameters[3] = {(vx_reference)input, (vx_reference)sumTmp, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_MulScaleFP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
        if (status != VX_SUCCESS) goto OnError;

    }
    else if (srcFormat == VX_TYPE_UINT8 && dstFormat == VX_TYPE_UINT8)
    {
        vx_reference  parameters[7] = {(vx_reference)input, (vx_reference)sumTmp, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScale);
        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScale);
        inZP = vxCreateScalar(context, VX_TYPE_INT32, &inputZP);
        outZP = vxCreateScalar(context, VX_TYPE_INT32, &outputZP);
        parameters[2] = (vx_reference)scaleIn;
        parameters[3] = (vx_reference)scaleOut;
        parameters[4] = (vx_reference)inZP;
        parameters[5] = (vx_reference)outZP;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_MulScaleQuant8", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 7);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar inZP = NULL;
    vx_scalar outZP = NULL;
    vx_scalar scaleIn = NULL;
    vx_scalar scaleOut = NULL;
    vx_scalar padX = vxCreateScalar(context, VX_TYPE_INT32, &pad_left);
    vx_scalar padY = vxCreateScalar(context, VX_TYPE_INT32, &pad_top);

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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
        else
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
        inZP = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        outZP = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        scaleIn = vxCreateScalar(context, VX_TYPE_INT32, &scaleInValue);
        scaleOut = vxCreateScalar(context, VX_TYPE_INT32, &scaleOutValue);

        if (pad_left == 0 && pad_right == 0 && pad_top == 0 && pad_bottom == 0)
        {
            vx_reference parameters[10] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY, (vx_reference)stride_x, (vx_reference)stride_y,
                                          (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)inZP, (vx_reference)outZP, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_NoPadQuant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else
        {
            vx_reference parameters[12] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY, (vx_reference)stride_x, (vx_reference)stride_y,
                                           (vx_reference)padX, (vx_reference)padY, (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)inZP, (vx_reference)outZP,(vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 12);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }
    else if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
             (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
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
            vx_reference parameters[8] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY,
                                          (vx_reference)stride_x, (vx_reference)stride_y,
                                          (vx_reference)padX, (vx_reference)padY, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
            if (status != VX_SUCCESS) goto OnError;
        }
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = out_width;
    execution_parameters.globalWorkSize[1]   = out_height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;


    if(padX) vxReleaseScalar(&padX);
    if(padY) vxReleaseScalar(&padY);
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(padX) vxReleaseScalar(&padX);
    if(padY) vxReleaseScalar(&padY);
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar alpha = NULL;
    vx_scalar scaleIn  = NULL;
    vx_scalar scaleOut = NULL;
    vx_scalar zpIn     = NULL;
    vx_scalar zpOut    = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    borderMode->mode = VX_BORDER_CONSTANT;
    if (inputFormat == VX_TYPE_INT8)
    {
        borderMode->constant_value.U8 = 0;
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }
    else
    {
        borderMode->constant_value.S32 = 0;
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

        alpha = vxCreateScalar(context, VX_TYPE_FLOAT32, &newAlphaValue);

        if((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
           (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        {
            vx_reference parameters[5] = {(vx_reference)input, (vx_reference)norm_size_s, (vx_reference)alpha, (vx_reference)beta_s, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SameMapsFP32", borderMode);
            if (!shaderExecutable)
            {
                if(alpha) vxReleaseScalar(&alpha);
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_reference parameters[9] = {(vx_reference)input, (vx_reference)norm_size_s, (vx_reference)alpha, (vx_reference)beta_s,
                                           (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

            scaleIn    = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale);
            scaleOut   = vxCreateScalar(context, VX_TYPE_FLOAT32, &out_scale);
            zpIn       = vxCreateScalar(context, VX_TYPE_FLOAT32, &zpInValue);
            zpOut      = vxCreateScalar(context, VX_TYPE_FLOAT32, &zpOutValue);
            parameters[4] = (vx_reference)scaleIn;
            parameters[5] = (vx_reference)scaleOut;
            parameters[6] = (vx_reference)zpIn;
            parameters[7] = (vx_reference)zpOut;

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SameMapsQuant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 9);
            if (status != VX_SUCCESS) goto OnError;
        }
    }
    else /*VX_NN_NORMALIZATION_ACROSS_MAPS*/
    {
        if((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
           (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        {
            vx_reference parameters[6] = {(vx_reference)input, (vx_reference)norm_size_s, (vx_reference)bias_s, (vx_reference)alpha_s, (vx_reference)beta_s, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AcrossMapsFP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            vx_reference parameters[10] = {(vx_reference)input, (vx_reference)norm_size_s, (vx_reference)bias_s, (vx_reference)alpha_s, (vx_reference)beta_s,
                                           (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

            scaleIn    = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale);
            scaleOut   = vxCreateScalar(context, VX_TYPE_FLOAT32, &out_scale);
            zpIn       = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            zpOut      = vxCreateScalar(context, VX_TYPE_INT32, &zpOutValue);
            parameters[5] = (vx_reference)scaleIn;
            parameters[6] = (vx_reference)scaleOut;
            parameters[7] = (vx_reference)zpIn;
            parameters[8] = (vx_reference)zpOut;

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AcrossMapsQuant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS) goto OnError;
        }
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = channel;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zpIn) vxReleaseScalar(&zpIn);
    if(zpOut) vxReleaseScalar(&zpOut);
    if(alpha) vxReleaseScalar(&alpha);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zpIn) vxReleaseScalar(&zpIn);
    if(zpOut) vxReleaseScalar(&zpOut);
    if(alpha) vxReleaseScalar(&alpha);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char           *programSources  = NULL;
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
    vx_uint32      batch            = TENSOR_VIEW_SIZE_INDEX(input, 3);
    vx_uint32      dims             = TENSOR_DIM_NUM(input) == 1 ? 2 : TENSOR_DIM_NUM(input);
    vx_tensor      input_rs         = NULL;
    vx_tensor      output_rs        = NULL;
    vx_float32     scaleInValue     = TENSOR_TF_SCALE(input);
    vx_int32       zpInValue        = TENSOR_TF_ZEROPOINT(input);
    vx_float32     scaleOutValue    = 1.0f/TENSOR_TF_SCALE(output);
    vx_int32       zpOutValue       = TENSOR_TF_ZEROPOINT(output);
    vx_float32     logEValue        = (vx_float32)(log10(exp(1.0f)) / log10(2.0f));
    vx_bool        enalbe_relu      = (vx_bool)(funcType == VX_NN_ACTIVATION_RELU || funcType == VX_NN_ACTIVATION_RELU1 || funcType == VX_NN_ACTIVATION_RELU6);
    vx_scalar      logE             = VX_NULL;
    vx_bool        paramChanged     = vx_false_e;
    vx_scalar      scaleIn          = VX_NULL;
    vx_scalar      zpIn             = VX_NULL;
    vx_scalar      scaleOut         = VX_NULL;
    vx_scalar      zpOut            = VX_NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    borderMode->mode = VX_BORDER_REPLICATE;
    if(TENSOR_DIM_NUM(input) == 1)
    {
        vx_int32 sizes[2] = {width, 1};

        input_rs  = vxoTensor_ReshapeTensor(input, sizes, dims);
        output_rs = vxoTensor_ReshapeTensor(output, sizes, dims);
        paramChanged = vx_true_e;
    }

    if(width == 1 && height ==1 && batch != 1)
    {
        vx_int32 sizes[2] = {depth, batch};

        width = depth;
        height = batch;
        execution_parameters.workDim = 2;
        input_rs  = vxoTensor_ReshapeTensor(input, sizes, 2);
        output_rs = vxoTensor_ReshapeTensor(output, sizes, 2);
        paramChanged = vx_true_e;
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

        if(funcType == VX_NN_ACTIVATION_RELU && ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)))
        {
            sprintf(subName, "_ReluFP32");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU && inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            sprintf(subName, "_ReluQuant8");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU1 && ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)))
        {
            sprintf(subName, "_Relu1FP32");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU1 && inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            sprintf(subName, "_Relu1Quant8");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU6 && ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)))
        {
            sprintf(subName, "_Relu6FP32");
        }
        else if(funcType == VX_NN_ACTIVATION_RELU6 && inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            sprintf(subName, "_Relu6Quant8");
        }

        if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
            (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        {
            vx_reference   parameters[2] = {(vx_reference)input, (vx_reference)output};

            if(paramChanged)
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

            vx_reference   parameters[6] = {(vx_reference)input, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
            zpIn = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
            zpOut = vxCreateScalar(context, VX_TYPE_INT32, &zpOutValue);
            parameters[1] = (vx_reference)scaleIn;
            parameters[2] = (vx_reference)zpIn;
            parameters[3] = (vx_reference)scaleOut;
            parameters[4] = (vx_reference)zpOut;

            if(paramChanged)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[5] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, subName, borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
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

        if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
            (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        {
            vx_reference   parameters[3] = {(vx_reference)input, (vx_reference)logE, (vx_reference)output};

            if(paramChanged)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[2] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SigmoidFP32", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            vx_reference   parameters[5] = {(vx_reference)input, (vx_reference)logE, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
            zpIn = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            parameters[2] = (vx_reference)scaleIn;
            parameters[3] = (vx_reference)zpIn;

            if(paramChanged)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[4] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SigmoidQuant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_UINT8 && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32))
        {
            vx_reference   parameters[5] = {(vx_reference)input, (vx_reference)logE, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
            zpIn = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            parameters[2] = (vx_reference)scaleIn;
            parameters[3] = (vx_reference)zpIn;

            if(paramChanged)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[4] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SigmoidQ8toFP32", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else if ((inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32) && outputFormat == VX_TYPE_UINT8)
        {
            vx_reference   parameters[3] = {(vx_reference)input, (vx_reference)logE, (vx_reference)output};

            if(paramChanged)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[2] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SigmoidFP32toQ8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }
    else if(funcType == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
    {
        logEValue = 2*logEValue;
        logE = vxCreateScalar(context, VX_TYPE_FLOAT32, &logEValue);

        if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
            (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        {
            vx_reference   parameters[3] = {(vx_reference)input, (vx_reference)logE, (vx_reference)output};

            if(paramChanged)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[2] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_TanhFP32", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            vx_reference   parameters[7] = {(vx_reference)input, (vx_reference)logE, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
            zpIn = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
            scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
            zpOut = vxCreateScalar(context, VX_TYPE_INT32, &zpOutValue);
            parameters[2] = (vx_reference)scaleIn;
            parameters[3] = (vx_reference)zpIn;
            parameters[4] = (vx_reference)scaleOut;
            parameters[5] = (vx_reference)zpOut;

            if(paramChanged)
            {
                parameters[0] = (vx_reference)input_rs;
                parameters[5] = (vx_reference)output_rs;
            }

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_TanhQuant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }
    else
    {
        vx_reference   parameters[6] = {(vx_reference)input, (vx_reference)output, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL};

        if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            vx_float32 inputZP = (vx_float32)zpInValue;
            vx_float32 outputZP = (vx_float32)zpOutValue + 0.5f;
            scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
            zpIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputZP);
            scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
            zpOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputZP);

            parameters[2] = (vx_reference)scaleIn;
            parameters[3] = (vx_reference)zpIn;
            parameters[4] = (vx_reference)scaleOut;
            parameters[5] = (vx_reference)zpOut;
        }

        if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
        {
            if (funcType == VX_NN_ACTIVATION_RSQRT)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_RSqrtQuant8", borderMode);
            }
            else if (funcType == VX_NN_ACTIVATION_SQRT)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SqrtQuant8", borderMode);
            }
            else if (funcType == VX_NN_ACTIVATION_ABS)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AbsQuant8", borderMode);
            }

            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else
        {
            if (funcType == VX_NN_ACTIVATION_RSQRT)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_RSqrtFP32", borderMode);
            }
            else if (funcType == VX_NN_ACTIVATION_SQRT)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SqrtFP32", borderMode);
            }
            else if (funcType == VX_NN_ACTIVATION_ABS)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AbsFP32", borderMode);
            }

            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }
    if (!shaderExecutable) goto OnError;

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (scaleOut) vxReleaseScalar(&scaleOut);
    if (zpOut) vxReleaseScalar(&zpOut);
    if (logE) vxReleaseScalar(&logE);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (logE) vxReleaseScalar(&logE);
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (scaleOut) vxReleaseScalar(&scaleOut);
    if (zpOut) vxReleaseScalar(&zpOut);
    if (program) vxReleaseProgram(&program);
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar inZP = NULL;
    vx_scalar outZP = NULL;
    vx_scalar scaleIn = NULL;
    vx_scalar scaleOut = NULL;
    vx_scalar padX = vxCreateScalar(context, VX_TYPE_INT32, &pad_left);
    vx_scalar padY = vxCreateScalar(context, VX_TYPE_INT32, &pad_top);

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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
        inZP = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        outZP = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);

        if (pad_left == 0 && pad_right == 0 && pad_top == 0 && pad_bottom == 0)
        {
            vx_reference parameters[10] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY, (vx_reference)stride_x, (vx_reference)stride_y,
                                          (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)inZP, (vx_reference)outZP, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "NoPadQuant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
        else
        {
            vx_reference parameters[12] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY, (vx_reference)stride_x, (vx_reference)stride_y,
                                           (vx_reference)padX, (vx_reference)padY, (vx_reference)scaleIn, (vx_reference)scaleOut, (vx_reference)inZP, (vx_reference)outZP,(vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "PadQuant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 12);
            if (status != VX_SUCCESS)
            {
                goto OnError;
            }
        }
    }
    else if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
             (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
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
            vx_reference parameters[8] = {(vx_reference)input, (vx_reference)poolSizeX, (vx_reference)poolSizeY,
                                          (vx_reference)stride_x, (vx_reference)stride_y,
                                          (vx_reference)padX, (vx_reference)padY, (vx_reference)output};

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "PadFP32", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
            if (status != VX_SUCCESS) goto OnError;
        }
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = out_width;
    execution_parameters.globalWorkSize[1]   = out_height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(padX) vxReleaseScalar(&padX);
    if(padY) vxReleaseScalar(&padY);
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(padX) vxReleaseScalar(&padX);
    if(padY) vxReleaseScalar(&padY);
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char        *programSources = NULL;
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
    vx_scalar    scaleX_s = VX_NULL, scaleY_s = VX_NULL;
    vx_scalar inZP = NULL;
    vx_scalar outZP = NULL;
    vx_scalar scaleIn = NULL;
    vx_scalar scaleOut = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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

    if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
        (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
    {
        vx_reference parameters[4] = {(vx_reference)input, (vx_reference)scaleX_s, (vx_reference)scaleY_s, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_BilinearFP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_reference parameters[8] = {(vx_reference)input, (vx_reference)scaleX_s, (vx_reference)scaleY_s, (vx_reference)NULL, (vx_reference)NULL,
                                        (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        inZP = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        outZP = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
        parameters[3] = (vx_reference)scaleIn;
        parameters[4] = (vx_reference)scaleOut;
        parameters[5] = (vx_reference)inZP;
        parameters[6] = (vx_reference)outZP;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_BilinearQuant8", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = out_width;
    execution_parameters.globalWorkSize[1]   = out_height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(scaleX_s) vxReleaseScalar(&scaleX_s);
    if(scaleY_s) vxReleaseScalar(&scaleY_s);
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if(scaleX_s) vxReleaseScalar(&scaleX_s);
    if(scaleY_s) vxReleaseScalar(&scaleY_s);
#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

vxnne_shader_executable vxnneGetGPUResizeNearestNeighborShaderExecutable(
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
    char        *programSources = NULL;
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
    vx_scalar    scaleX_s = VX_NULL, scaleY_s = VX_NULL;
    vx_scalar inZP = NULL;
    vx_scalar outZP = NULL;
    vx_scalar scaleIn = NULL;
    vx_scalar scaleOut = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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
        void * ptr = getGPUKernelInfo(context, ResizeNearestNeighbor, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/ResizeNearestNeighbor.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuResizeNearNb", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
        (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
    {
        vx_reference parameters[4] = {(vx_reference)input, (vx_reference)scaleX_s, (vx_reference)scaleY_s, (vx_reference)output};

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_reference parameters[8] = {(vx_reference)input, (vx_reference)scaleX_s, (vx_reference)scaleY_s, (vx_reference)NULL, (vx_reference)NULL,
                                        (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        inZP = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        outZP = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
        parameters[3] = (vx_reference)scaleIn;
        parameters[4] = (vx_reference)scaleOut;
        parameters[5] = (vx_reference)inZP;
        parameters[6] = (vx_reference)outZP;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = out_width;
    execution_parameters.globalWorkSize[1]   = out_height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(scaleX_s) vxReleaseScalar(&scaleX_s);
    if(scaleY_s) vxReleaseScalar(&scaleY_s);
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(inZP) vxReleaseScalar(&inZP);
    if(outZP) vxReleaseScalar(&outZP);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if(scaleX_s) vxReleaseScalar(&scaleX_s);
    if(scaleY_s) vxReleaseScalar(&scaleY_s);
#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char        *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_uint32 inDims = TENSOR_DIM_NUM(input);
    vx_kernel_execution_parameters_t execution_parameters = {inDims-1, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {1, 1, 1}};
    vx_enum      inputFormat        = TENSOR_DATA_TYPE(input);
    vx_scalar scaleIn = NULL;
    vx_scalar scaleOut = NULL;
    vx_scalar zpIn = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

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

    if (inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32)
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
        vx_reference parameters[5] = {(vx_reference)input, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleInValue);
        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
        zpIn = vxCreateScalar(context, VX_TYPE_UINT32, &zpInValue);
        parameters[1] = (vx_reference)scaleIn;
        parameters[2] = (vx_reference)scaleOut;
        parameters[3] = (vx_reference)zpIn;

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
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zpIn) vxReleaseScalar(&zpIn);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zpIn) vxReleaseScalar(&zpIn);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

vxnne_shader_executable vxnneGetGPUSoftmaxShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_scalar               beta,
    vx_tensor               input,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char        *programSources     = NULL;
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
    vx_enum      outputFormat       = TENSOR_DATA_TYPE(output);
    vx_float32   betaValue          = beta->value->f32;
    vx_float32   scaleValue         = (vx_float32)((log10(exp(1.0f)) / log10(2.0f))*betaValue);
    vx_enum      output_qnt_type    = TENSOR_QUANT_TYPE(output);
    vx_float32   scaleOutValue      = 1.0f;
    vx_float32   zpOutValue         = 0.0f;
    vx_scalar    scale_s            = NULL;
    vx_scalar    scaleout           = NULL;
    vx_scalar    zpOut              = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    if (output_qnt_type == VX_QUANT_AFFINE_SCALE)
    {
        scaleOutValue = 1.0f / TENSOR_TF_SCALE(output);
        zpOutValue    = (vx_float32)TENSOR_TF_ZEROPOINT(output);
    }

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

    if (inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32)
    {
        vx_reference parameters[3] = {(vx_reference)input, (vx_reference)NULL, (vx_reference)output};

        scale_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);
        parameters[1] = (vx_reference)scale_s;

        if(inDims == 2)
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Dim2FP32", borderMode);
        else
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        vx_float32 scaleInValue = TENSOR_TF_SCALE(input);
        vx_reference parameters[5] = {(vx_reference)input, (vx_reference)NULL,
                                        (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        scaleValue *= scaleInValue;
        scale_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleValue);
        parameters[1] = (vx_reference)scale_s;

        scaleout = vxCreateScalar(context, VX_TYPE_FLOAT32, &scaleOutValue);
        zpOut = vxCreateScalar(context, VX_TYPE_INT32, &zpOutValue);
        parameters[2] = (vx_reference)scaleout;
        parameters[3] = (vx_reference)zpOut;

        if(inDims == 2)
        {
            if (outputFormat == VX_TYPE_UINT8)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Dim2Quant8", borderMode);
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Dim2Quant8toFloat", borderMode);
        }
        else
        {
            if (outputFormat == VX_TYPE_UINT8)
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            else
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8toFloat", borderMode);
        }

        if (!shaderExecutable)
        {
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    if(inDims > 2)
    {
        execution_parameters.globalWorkSize[0]   = in_width;
        execution_parameters.globalWorkSize[1]   = in_height;
        execution_parameters.globalWorkSize[2]   = 1;
    }
    else
    {
        execution_parameters.globalWorkSize[0]   = in_height;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (scale_s) vxReleaseScalar(&scale_s);
    if (scaleout) vxReleaseScalar(&scaleout);
    if (zpOut) vxReleaseScalar(&zpOut);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (scaleout) vxReleaseScalar(&scaleout);
    if (zpOut) vxReleaseScalar(&zpOut);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if (scale_s) vxReleaseScalar(&scale_s);
#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
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
    char        *programSources = NULL;
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

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input0=%p, output=%p", context, kernelEnum, input0, output);

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

    if (inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32)
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
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
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

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif

    gcmFOOTER_NO();
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
    char *programSources = NULL;
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
    vx_scalar  outputZP       = NULL;
    vx_scalar  inputZP        = NULL;
    vx_scalar  biasZP         = NULL;
    vx_scalar  weightZP       = NULL;
    vx_scalar  hiddenZP       = NULL;
    vx_scalar  recurrentZP    = NULL;
    vx_scalar  outputScale    = NULL;
    vx_scalar  inputScale     = NULL;
    vx_scalar  biasScale      = NULL;
    vx_scalar  weightScale    = NULL;
    vx_scalar  hiddenScale    = NULL;
    vx_scalar  recurrentScale = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    borderMode->mode = VX_BORDER_CONSTANT;
    if(inputFormat == VX_TYPE_UINT8)
        borderMode->constant_value.U8 = 0;
    else if(inputFormat == VX_TYPE_FLOAT16)
        borderMode->constant_value.S16 = 0;
    else
        borderMode->constant_value.S32 = 0;

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
        vx_reference parameters[19]    = {(vx_reference)input, (vx_reference)inputScale, (vx_reference)inputZP, (vx_reference)weight, (vx_reference)weightScale, (vx_reference)weightZP,
                                          (vx_reference)recurrent, (vx_reference)recurrentScale, (vx_reference)recurrentZP, (vx_reference)bias_rs, (vx_reference)biasScale, (vx_reference)biasZP,
                                          (vx_reference)hidden, (vx_reference)hiddenScale, (vx_reference)hiddenZP, (vx_reference)state_out,
                                          (vx_reference)output, (vx_reference)outputScale, (vx_reference)outputZP};

        outputZP       = vxCreateScalar(context, VX_TYPE_UINT32, &outputZPValue);
        inputZP        = vxCreateScalar(context, VX_TYPE_UINT32, &inputZPValue);
        biasZP         = vxCreateScalar(context, VX_TYPE_UINT32, &biasZPValue);
        weightZP       = vxCreateScalar(context, VX_TYPE_UINT32, &weightZPValue);
        hiddenZP       = vxCreateScalar(context, VX_TYPE_UINT32, &hiddenZPValue);
        recurrentZP    = vxCreateScalar(context, VX_TYPE_UINT32, &recurrentZPValue);
        outputScale    = vxCreateScalar(context, VX_TYPE_UINT32, &outputScaleValue);
        inputScale     = vxCreateScalar(context, VX_TYPE_UINT32, &inputScaleValue);
        biasScale      = vxCreateScalar(context, VX_TYPE_UINT32, &biasScaleValue);
        weightScale    = vxCreateScalar(context, VX_TYPE_UINT32, &weightScaleValue);
        hiddenScale    = vxCreateScalar(context, VX_TYPE_UINT32, &hiddenScaleValue);
        recurrentScale = vxCreateScalar(context, VX_TYPE_UINT32, &recurrentScaleValue);
        parameters[1] = (vx_reference)inputScale;
        parameters[2] = (vx_reference)inputZP;
        parameters[4] = (vx_reference)weightScale;
        parameters[5] = (vx_reference)weightZP;
        parameters[7] = (vx_reference)recurrentScale;
        parameters[8] = (vx_reference)recurrentZP;
        parameters[10] = (vx_reference)biasScale;
        parameters[11] = (vx_reference)biasZP;
        parameters[13] = (vx_reference)hiddenScale;
        parameters[14] = (vx_reference)hiddenZP;
        parameters[17] = (vx_reference)outputScale;
        parameters[18] = (vx_reference)outputZP;

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
            goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 19);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if(bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
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

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if(bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
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
    if(program) vxReleaseProgram(&program);
    if(shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/********gpuTensorAdd****************************************************/
vxnne_shader_executable vxnneGetGPUTensorEltwiseShaderExecutable(
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
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {1, 1, 1}, {0, 0, 0}, {0, 0, 0}};
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
    vx_bool       useImage2DFlag        = vx_false_e;

    vx_scalar scaleIn0 = NULL;
    vx_scalar scaleIn1 = NULL;
    vx_scalar scaleOut = NULL;
    vx_scalar zpIn0 = NULL;
    vx_scalar zpIn1 = NULL;
    vx_scalar zpOut = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input0=%p, input1=%p, output=%p", context, kernelEnum, input0, input1, output);

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

    if (depth == 1 && TENSOR_VIEW_SIZE_INDEX(input0, 0) == TENSOR_VIEW_SIZE_INDEX(input1, 0)
        && (width % 4 == 0)
        && operation == VX_TENSOR_OP_ADD)
    {
        useImage2DFlag = vx_true_e;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorEltwise, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorEltwise.vx", path));

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

    if((input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16) ||
       (input0_format == VX_TYPE_FLOAT32 && input1_format == VX_TYPE_FLOAT32))
    {
        if (useImage2DFlag)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32_2D_4X", borderMode);

            if (!shaderExecutable) goto OnError;
            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (status != VX_SUCCESS) goto OnError;

            execution_parameters.globalWorkScale[0] = 4;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            if (!shaderExecutable) goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramNum);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (input0_format == VX_TYPE_UINT8 && input1_format == VX_TYPE_UINT8)
    {
        vx_float32 zp0 = (vx_float32)input_ZP0 * input_scale0 * output_scale;
        vx_float32 zp1 = (vx_float32)input_ZP1 * input_scale1 * output_scale;
        vx_float32 zp2 = (vx_float32)output_ZP;
        vx_float32 s0 = input_scale0 * output_scale;
        vx_float32 s1 = input_scale1 * output_scale;

        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &output_scale);
        if (useImage2DFlag)
        {
            scaleIn0 = vxCreateScalar(context, VX_TYPE_FLOAT32, &s0);
            scaleIn1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &s1);

            zp0 = zp0 - zp2 - 0.5f + zp1;
            zpIn0 = vxCreateScalar(context, VX_TYPE_FLOAT32, &zp0);
            zpIn1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &zp1);
            zpOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &zp2);
        }
        else
        {
            scaleIn0 = vxCreateScalar(context, VX_TYPE_FLOAT32, &input_scale0);
            scaleIn1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &input_scale1);

            zpIn0 = vxCreateScalar(context, VX_TYPE_UINT32, &input_ZP0);
            zpIn1 = vxCreateScalar(context, VX_TYPE_UINT32, &input_ZP1);
            zpOut = vxCreateScalar(context, VX_TYPE_UINT32, &output_ZP);
        }

        parameters[paramNum++] = (vx_reference)scaleIn0;
        parameters[paramNum++] = (vx_reference)scaleIn1;
        parameters[paramNum++] = (vx_reference)scaleOut;
        parameters[paramNum++] = (vx_reference)zpIn0;
        parameters[paramNum++] = (vx_reference)zpIn1;
        parameters[paramNum++] = (vx_reference)zpOut;

        if (useImage2DFlag)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_2D_4X", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (status != VX_SUCCESS) goto OnError;

            execution_parameters.globalWorkScale[0] = 4;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);

            if (!shaderExecutable)
            {
                goto OnError;
            }
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramNum);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if ((input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8)
            ||(input0_format == VX_TYPE_INT16 && input1_format == VX_TYPE_INT16))
    {
        vx_int8   srcFixedPointPos0 = TENSOR_POS(input0);
        vx_int8   srcFixedPointPos1 = TENSOR_POS(input1);
        vx_int8   dstFixedPointPos  = TENSOR_POS(output);
        vx_float32   in_scale0      = 0;
        vx_float32   in_scale1      = 0;
        vx_float32   out_scale      = 0;

        if (srcFixedPointPos0 >= 0)
        {
            in_scale0    = 1.0f / (vx_float32) (1 << srcFixedPointPos0);
        }
        else if (srcFixedPointPos0 < 0)
        {
            in_scale0    = (vx_float32)(1 << -srcFixedPointPos0);
        }

        if (srcFixedPointPos1 >= 0)
        {
            in_scale1    = 1.0f / (vx_float32) (1 << srcFixedPointPos1);
        }
        else if (srcFixedPointPos1 < 0)
        {
            in_scale1    = (vx_float32)(1 << -srcFixedPointPos1);
        }

        if (dstFixedPointPos >= 0)
        {
            out_scale = (vx_float32) (1 << dstFixedPointPos);
        }
        else if (dstFixedPointPos < 0)
        {
            out_scale = 1.0f / (vx_float32) (1 << -dstFixedPointPos);
        }

        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &out_scale);
        scaleIn0 = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale0);
        scaleIn1 = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale1);

        parameters[paramNum++] = (vx_reference)scaleIn0;
        parameters[paramNum++] = (vx_reference)scaleIn1;
        parameters[paramNum++] = (vx_reference)scaleOut;
        if (useImage2DFlag)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant16_2D_4X", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (status != VX_SUCCESS) goto OnError;

            execution_parameters.globalWorkScale[0] = 4;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant16", borderMode);

            if (!shaderExecutable)
            {
                goto OnError;
            }
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramNum);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if ((input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_FLOAT16)
            ||(input0_format == VX_TYPE_INT16 && input1_format == VX_TYPE_FLOAT16))
    {
        vx_int8   srcFixedPointPos0 = TENSOR_POS(input0);
        vx_int8   dstFixedPointPos  = TENSOR_POS(output);
        vx_float32   in_scale0      = 0;
        vx_float32   out_scale      = 0;

        if (srcFixedPointPos0 >= 0)
        {
            in_scale0    = 1.0f / (vx_float32) (1 << srcFixedPointPos0);
        }
        else if (srcFixedPointPos0 < 0)
        {
            in_scale0    = (vx_float32)(1 << -srcFixedPointPos0);
        }

        if (dstFixedPointPos >= 0)
        {
            out_scale = (vx_float32) (1 << dstFixedPointPos);
        }
        else if (dstFixedPointPos < 0)
        {
            out_scale = 1.0f / (vx_float32) (1 << -dstFixedPointPos);
        }

        scaleIn0 = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale0);
        scaleOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &out_scale);

        parameters[paramNum++] = (vx_reference)scaleIn0;
        parameters[paramNum++] = (vx_reference)scaleOut;
        if (useImage2DFlag)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant16Fp16_2D_4X", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }

            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (status != VX_SUCCESS) goto OnError;

            execution_parameters.globalWorkScale[0] = 4;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant16Fp16", borderMode);

            if (!shaderExecutable)
            {
                goto OnError;
            }
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramNum);
        if (status != VX_SUCCESS) goto OnError;
    }
    else
    {
        vxError("Invalid data type at function %s line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }

    if (useImage2DFlag)
    {
        execution_parameters.workDim = 2;

        execution_parameters.globalWorkSize[0]   =  gcmALIGN((width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], SHADER_THREAD_COUNT);
        execution_parameters.globalWorkSize[1]   = height;
    }
    else
    {
        execution_parameters.globalWorkSize[0]   = width;
        execution_parameters.globalWorkSize[1]   = height;
        execution_parameters.globalWorkSize[2]   = depth;
    }

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (src0) vxoTensor_ReleaseTensor(&src0);
    if (src1) vxoTensor_ReleaseTensor(&src1);
    if (dst) vxoTensor_ReleaseTensor(&dst);
    if (scaleIn0) vxReleaseScalar(&scaleIn0);
    if (scaleIn1) vxReleaseScalar(&scaleIn1);
    if (scaleOut) vxReleaseScalar(&scaleOut);
    if (zpIn0) vxReleaseScalar(&zpIn0);
    if (zpIn1) vxReleaseScalar(&zpIn1);
    if (zpOut) vxReleaseScalar(&zpOut);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (src0) vxoTensor_ReleaseTensor(&src0);
    if (src1) vxoTensor_ReleaseTensor(&src1);
    if (dst) vxoTensor_ReleaseTensor(&dst);
    if (scaleIn0) vxReleaseScalar(&scaleIn0);
    if (scaleIn1) vxReleaseScalar(&scaleIn1);
    if (scaleOut) vxReleaseScalar(&scaleOut);
    if (zpIn0) vxReleaseScalar(&zpIn0);
    if (zpIn1) vxReleaseScalar(&zpIn1);
    if (zpOut) vxReleaseScalar(&zpOut);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

vxnne_shader_executable vxnneGPULSTMUnitShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               w_h,
    vx_tensor               h_state_in,
    vx_tensor               c_state,
    vx_float32              cellClipValue,
    vx_bool                 enable_cifg,
    vx_bool                 enable_peephole,
    vx_bool                 enable_projection,
    vx_tensor               cell2input_weight,
    vx_tensor               cell2forget_weight,
    vx_tensor               cell2output_weight,
    vx_tensor               c_state_out,
    vx_tensor               h_state_out,
    vx_enum                 activation,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[13]         = {(vx_reference)input, (vx_reference)w_h, (vx_reference)h_state_in, (vx_reference)c_state, (vx_reference)c_state_out, (vx_reference)h_state_out, (vx_reference)output, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL};

    vx_uint32    dims                   = TENSOR_DIM_NUM(input);
    vx_uint32    output_dims            = TENSOR_DIM_NUM(output);
    vx_uint32    batch                  = (output_dims > 1) ? TENSOR_VIEW_SIZE_INDEX(output, 1) : 1;
    vx_uint32    num_units              = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_tensor    input_rs               = NULL;
    vx_tensor    cell2input_weight_rs   = NULL;
    vx_tensor    cell2forget_weight_rs  = NULL;
    vx_tensor    cell2output_weight_rs  = NULL;
    vx_int32     sizes[]                = {1,1,1,1};
    vx_uint32    inputCount             = 0;
    vx_uint32    paramt_num             = 7;
    vx_scalar    cell_clip              = NULL;
    vx_float32   logEValue              = (vx_float32)(log10(exp(1.0f)) / log10(2.0f));
    vx_float32   twoLogEValue           = 2 * logEValue;
    vx_scalar    logE                   = vxCreateScalar(context, VX_TYPE_FLOAT32, &logEValue);
    vx_scalar    twoLogE                = vxCreateScalar(context, VX_TYPE_FLOAT32, &twoLogEValue);

    vxmASSERT(TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT16 || TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT32);

    if(cellClipValue != 0.0)
    {
        cell_clip                = vxCreateScalar(context, VX_TYPE_FLOAT32, &cellClipValue);
        parameters[paramt_num++] = (vx_reference)cell_clip;
    }

    vxoTensor_GetTensorElementCount(input, &inputCount);
    sizes[0]        = inputCount / batch;
    sizes[1]        = batch;
    dims            = 2;
    input_rs        = vxoTensor_ReshapeTensor(input, sizes, dims);
    parameters[0]   = (vx_reference)input_rs;

    borderMode->mode = VX_BORDER_CONSTANT;
    if(TENSOR_DATA_TYPE(input) == VX_TYPE_FLOAT16)
        borderMode->constant_value.S16 = 0;
    else
        borderMode->constant_value.S32 = 0;

    if (enable_peephole)
    {
        vx_uint32     w             = TENSOR_VIEW_SIZE_INDEX(cell2forget_weight, 0);
        vx_int32 sizes[4]           = {w, 1, 1, 1};

        dims = 2;
        cell2forget_weight_rs       = vxoTensor_ReshapeTensor(cell2forget_weight, sizes, dims);
        cell2output_weight_rs       = vxoTensor_ReshapeTensor(cell2output_weight, sizes, dims);
        if (!enable_cifg)
        {
            cell2input_weight_rs    = vxoTensor_ReshapeTensor(cell2input_weight, sizes, dims);

            parameters[paramt_num++]           = (vx_reference)cell2input_weight_rs;
            parameters[paramt_num++]           = (vx_reference)cell2forget_weight_rs;
            parameters[paramt_num++]          = (vx_reference)cell2output_weight_rs;
        }
        else
        {
            parameters[paramt_num++]           = (vx_reference)cell2forget_weight_rs;
            parameters[paramt_num++]           = (vx_reference)cell2output_weight_rs;
        }

    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, LSTMUnit, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/LSTMUnit.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuLSTMUnit", program, paramt_num, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    parameters[paramt_num++] = (vx_reference)logE;

    if (!enable_peephole)
    {
        if (enable_cifg)
        {
            if(cell_clip == NULL)
            {
                if (activation == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
                {
                    parameters[paramt_num++] = (vx_reference)twoLogE;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_tanh", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_LOGISTIC)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_logistic", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_relu", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU6)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_relu6", borderMode);
                }
                if (!shaderExecutable) goto OnError;
            }
            else
            {
                if (activation == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
                {
                    parameters[paramt_num++] = (vx_reference)twoLogE;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_tanh_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_LOGISTIC)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_logistic_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_relu_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU6)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_relu6_cellclip", borderMode);
                }
                if (!shaderExecutable) goto OnError;
            }

        }
        else
        {
            if(cell_clip == NULL)
            {
                if (activation == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
                {
                    parameters[paramt_num++] = (vx_reference)twoLogE;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_non_CIFG_tanh", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_LOGISTIC)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_non_CIFG_logistic", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_non_CIFG_relu", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU6)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_non_CIFG_relu6", borderMode);
                }
                if (!shaderExecutable) goto OnError;
            }
            else
            {
                if (activation == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
                {
                    parameters[paramt_num++] = (vx_reference)twoLogE;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_non_CIFG_tanh_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_LOGISTIC)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_non_CIFG_logistic_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_non_CIFG_relu_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU6)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_non_peephole_non_CIFG_relu6_cellclip", borderMode);
                }
                if (!shaderExecutable) goto OnError;
            }

        }
    }
    else
    {
        if (enable_cifg)
        {
            if(cell_clip == NULL)
            {
                if (activation == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
                {
                    parameters[paramt_num++] = (vx_reference)twoLogE;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_tanh", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_LOGISTIC)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_logistic", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_relu", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU6)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_relu6", borderMode);
                }
                if (!shaderExecutable) goto OnError;
            }
            else
            {
                if (activation == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
                {
                    parameters[paramt_num++] = (vx_reference)twoLogE;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_tanh_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_LOGISTIC)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_logistic_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_relu_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU6)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_relu6_cellclip", borderMode);
                }
                if (!shaderExecutable) goto OnError;
            }
        }
        else
        {
            if(cell_clip == NULL)
            {
                if (activation == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
                {
                    parameters[paramt_num++] = (vx_reference)twoLogE;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_non_CIFG_tanh", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_LOGISTIC)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_non_CIFG_logistic", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_non_CIFG_relu", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU6)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_non_CIFG_relu6", borderMode);
                }
                if (!shaderExecutable) goto OnError;
            }
            else
            {
                if (activation == VX_NN_ACTIVATION_HYPERBOLIC_TAN)
                {
                    parameters[paramt_num++] = (vx_reference)twoLogE;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_non_CIFG_tanh_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_LOGISTIC)
                {
                    parameters[paramt_num++] = (vx_reference)logE;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_non_CIFG_logistic_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_non_CIFG_relu_cellclip", borderMode);
                }
                else if(activation == VX_NN_ACTIVATION_RELU6)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_peephole_non_CIFG_relu6_cellclip", borderMode);
                }
                if (!shaderExecutable) goto OnError;
            }
        }
    }

    execution_parameters.workDim             = 2;
    execution_parameters.globalWorkSize[0]   = num_units;
    execution_parameters.globalWorkSize[1]   = batch;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramt_num);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (cell2input_weight_rs) vxoTensor_ReleaseTensor(&cell2input_weight_rs);
    if (cell2forget_weight_rs) vxoTensor_ReleaseTensor(&cell2forget_weight_rs);
    if (cell2output_weight_rs) vxoTensor_ReleaseTensor(&cell2output_weight_rs);
    if (cell_clip) vxReleaseScalar(&cell_clip);
    if (logE) vxReleaseScalar(&logE);
    if (twoLogE) vxReleaseScalar(&twoLogE);

    return shaderExecutable;

OnError:
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (cell2input_weight_rs) vxoTensor_ReleaseTensor(&cell2input_weight_rs);
    if (cell2forget_weight_rs) vxoTensor_ReleaseTensor(&cell2forget_weight_rs);
    if (cell2output_weight_rs) vxoTensor_ReleaseTensor(&cell2output_weight_rs);
    if (cell_clip) vxReleaseScalar(&cell_clip);
    if (logE) vxReleaseScalar(&logE);
    if (twoLogE) vxReleaseScalar(&twoLogE);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    return VX_NULL;
}

vxnne_shader_executable vxnneGetGPULSTMUnitProjectionShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               bias,
    vx_float32              projClipValue,
    vx_tensor               output_state_out,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[6]   = {(vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL};
    vx_bool       enable_bias    = (bias == NULL) ? vx_false_e : vx_true_e;
    vx_enum       output_format  = TENSOR_DATA_TYPE(output);
    vx_enum       input_format   = TENSOR_DATA_TYPE(input);
    vx_enum       weights_format = TENSOR_DATA_TYPE(weights);
    vx_uint32     bias_dims      = enable_bias ? (TENSOR_DIM_NUM(bias) == 1 ? 2 : TENSOR_DIM_NUM(bias)) : 0;
    vx_uint32     output_dims    = TENSOR_DIM_NUM(output);
    vx_uint32     num_units      = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32     batch_size     = (output_dims > 1) ? TENSOR_VIEW_SIZE_INDEX(output, 1) : 1;
    vx_tensor     bias_rs        = NULL;
    vx_scalar     proj_clip      = NULL;
    vx_int32      bias_sizes[4]  = {num_units, 1, 1, 1};
    vx_uint32     paramNum       = 0;

    parameters[paramNum++] = (vx_reference)input;
    parameters[paramNum++] = (vx_reference)weights;
    if (enable_bias)
    {
        bias_rs       = vxoTensor_ReshapeTensor(bias, bias_sizes, bias_dims);
        parameters[paramNum++] = (vx_reference)bias_rs;
    }
    if(projClipValue != 0.0f)
    {
        proj_clip      = vxCreateScalar(context, VX_TYPE_FLOAT32, &projClipValue);
        parameters[paramNum++] = (vx_reference)proj_clip;
    }
    parameters[paramNum++] = (vx_reference)output_state_out;
    parameters[paramNum++] = (vx_reference)output;

    borderMode->mode = VX_BORDER_CONSTANT;
    if (input_format == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }
    else
    {
        borderMode->constant_value.S32 = 0;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, LSTMUnitProjection, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/LSTMUnitProjection.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuLSTMUnitProjection", program, paramNum, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if ((input_format == VX_TYPE_FLOAT16 && weights_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16) ||
        (input_format == VX_TYPE_FLOAT32 && weights_format == VX_TYPE_FLOAT32 && output_format == VX_TYPE_FLOAT32))
    {
        if (enable_bias)
        {
            if(proj_clip == NULL)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            }
            else
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32ProjClip", borderMode);
            }
            if (!shaderExecutable) goto OnError;
        }
        else
        {
            if(proj_clip == NULL)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32NoBias", borderMode);
            }
            else
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32NoBiasProjClip", borderMode);
            }
            if (!shaderExecutable) goto OnError;
        }
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramNum);
    if (status != VX_SUCCESS) goto OnError;

    execution_parameters.globalWorkSize[0]   = num_units;
    execution_parameters.globalWorkSize[1]   = batch_size;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
    if (proj_clip) vxReleaseScalar(&proj_clip);

    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
    if (proj_clip) vxReleaseScalar(&proj_clip);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    return VX_NULL;
}

/********gpuReorg2 Batch2Space****************************************/
vxnne_shader_executable vxnneGetGPUBatch2SpaceShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               stride,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[5]              = {(vx_reference)input, (vx_reference)VX_NULL, (vx_reference)VX_NULL, (vx_reference) VX_NULL, (vx_reference)output};
    vx_enum       inputFormat                = TENSOR_DATA_TYPE(input);
    vx_enum       outputFormat               = TENSOR_DATA_TYPE(output);
    vx_uint32     input_width                = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     input_height               = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32     input_depth                = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32     input_batch                = TENSOR_VIEW_SIZE_INDEX(input, 3);
    vx_uint32     input_dim                  = TENSOR_DIM_NUM(input);
    vx_uint32     output_dim                 = TENSOR_DIM_NUM(output);
    vx_uint32     output_width               = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32     output_height              = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_uint32     output_depth               = TENSOR_VIEW_SIZE_INDEX(output, 2);
    vx_uint32     output_batch               = TENSOR_VIEW_SIZE_INDEX(output, 3);
    vx_uint32     input_dimz                 = 0;
    vx_int32      sizes[4]                   = {output_width, output_height, output_depth * output_batch, 1};
    vx_tensor     input_rs                   = NULL;
    vx_tensor     output_rs                  = NULL;
    vx_scalar     inputDepth                 = vxCreateScalar(context, VX_TYPE_UINT32, &input_depth);
    vx_int32_ptr  block_size                 = VX_NULL;
    vx_int32      block_w                    = 0;
    vx_int32      block_h                    = 0;
    vx_scalar     blockw                     = NULL;
    vx_scalar     blockh                     = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, input=%p, output=%p",
         context, kernelEnum, borderMode, input, output);

    vxoTensor_GetTensorViewMemory(stride, (gctPOINTER*)&block_size, VX_NULL);
    block_w = block_size[0];
    block_h = block_size[1];
    blockw = vxCreateScalar(context, VX_TYPE_INT32, &block_w);
    blockh = vxCreateScalar(context, VX_TYPE_INT32, &block_h);
    parameters[1] = (vx_reference)blockw;
    parameters[2] = (vx_reference)blockh;

    if (!((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
       || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
       || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        )
    {
        vxError("input or output's format is not support(space to depth)");
        goto OnError;
    }

    borderMode->mode = VX_BORDER_CONSTANT;
    if(inputFormat == VX_TYPE_UINT8)
        borderMode->constant_value.U8 = 0;
    else if(inputFormat == VX_TYPE_FLOAT16)
        borderMode->constant_value.S16 = 0;
    else
        borderMode->constant_value.S32 = 0;

    input_batch = (input_batch == 0) ? 1 : input_batch;
    input_dimz = input_batch * input_depth;
    parameters[3] = (vx_reference)inputDepth;

    if (output_dim == 4)
    {
        output_rs        = vxoTensor_ReshapeTensor(output, sizes, 3);
        parameters[4]    = (vx_reference)output_rs;
    }

    if (input_dim == 4)
    {
        sizes[0] = input_width;
        sizes[1] = input_height;
        sizes[2] = input_depth * input_batch;
        sizes[3] = 1;
        input_rs        = vxoTensor_ReshapeTensor(input, sizes, 3);
        parameters[0]    = (vx_reference)input_rs;
    }

    execution_parameters.globalWorkSize[0]   = input_width;
    execution_parameters.globalWorkSize[1]   = input_height;
    execution_parameters.globalWorkSize[2]   = input_dimz;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Batch2Space, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Batch2Space.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuBatch2Space", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable) goto OnError;

    }
    else if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
             (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (inputDepth) vxReleaseScalar(&inputDepth);
    if (blockw) vxReleaseScalar(&blockw);
    if (blockh) vxReleaseScalar(&blockh);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (inputDepth) vxReleaseScalar(&inputDepth);
    if (blockw) vxReleaseScalar(&blockw);
    if (blockh) vxReleaseScalar(&blockh);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/********gpuReorg2 Space2Batch****************************************/
vxnne_shader_executable vxnneGetGPUSpace2BatchShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               stride,
    vx_scalar               outc,
    vx_tensor               output,
    vx_uint32*              padList)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[7]              = {(vx_reference)input, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output, (vx_reference)outc, (vx_reference)NULL, (vx_reference)NULL};
    vx_enum       inputFormat                = TENSOR_DATA_TYPE(input);
    vx_enum       outputFormat               = TENSOR_DATA_TYPE(output);
    vx_uint32     input_width                = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     input_height               = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32     input_depth                = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32     input_batch                = TENSOR_VIEW_SIZE_INDEX(input, 3);
    vx_uint32     input_dim                  = TENSOR_DIM_NUM(input);
    vx_uint32     output_dim                 = TENSOR_DIM_NUM(output);
    vx_uint32     output_width               = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32     output_height              = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_uint32     output_depth               = TENSOR_VIEW_SIZE_INDEX(output, 2);
    vx_uint32     output_batch               = TENSOR_VIEW_SIZE_INDEX(output, 3);
    vx_uint32     input_dimz                 = 0;
    vx_int32      sizes[4]                   = {output_width, output_height, output_depth * output_batch, 1};
    vx_tensor     output_rs                  = NULL;
    vx_tensor     input_rs                   = NULL;
    vx_int32_ptr  block_size                 = VX_NULL;
    vx_int32      block_w                    = 0;
    vx_int32      block_h                    = 0;
    vx_scalar     blockw                     = NULL;
    vx_scalar     blockh                     = NULL;
    vx_scalar     padX                       = NULL;
    vx_scalar     padY                       = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, input=%p, output=%p",
         context, kernelEnum, borderMode, input, output);

    if (!((inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
       || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
       || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
        )
    {
        vxError("input or output's format is not support(space to depth)");
        goto OnError;
    }

    vxoTensor_GetTensorViewMemory(stride, (gctPOINTER*)&block_size, VX_NULL);
    block_w = block_size[0];
    block_h = block_size[1];
    blockw = vxCreateScalar(context, VX_TYPE_INT32, &block_w);
    blockh = vxCreateScalar(context, VX_TYPE_INT32, &block_h);
    parameters[1] = (vx_reference)blockw;
    parameters[2] = (vx_reference)blockh;

    padX = vxCreateScalar(context, VX_TYPE_INT32, &padList[0]);
    padY = vxCreateScalar(context, VX_TYPE_INT32, &padList[2]);
    parameters[5] = (vx_reference)padX;
    parameters[6] = (vx_reference)padY;

    borderMode->mode = VX_BORDER_CONSTANT;
    if(inputFormat == VX_TYPE_UINT8)
        borderMode->constant_value.U8 = 0;
    else if(inputFormat == VX_TYPE_FLOAT16)
        borderMode->constant_value.S16 = 0;
    else
        borderMode->constant_value.S32 = 0;

    input_batch = (input_batch == 0) ? 1 : input_batch;
    input_dimz = input_batch * input_depth;

    if (output_dim == 4)
    {
        output_rs        = vxoTensor_ReshapeTensor(output, sizes, 3);
        parameters[3]    = (vx_reference)output_rs;
    }

    if (input_dim == 4)
    {
        sizes[0] = input_width;
        sizes[1] = input_height;
        sizes[2] = input_depth * input_batch;
        sizes[3] = 1;
        input_rs        = vxoTensor_ReshapeTensor(input, sizes, 3);
        parameters[0]    = (vx_reference)input_rs;
    }

    execution_parameters.globalWorkSize[0]   = input_width+padList[0]+padList[1];
    execution_parameters.globalWorkSize[1]   = input_height+padList[2]+padList[3];
    execution_parameters.globalWorkSize[2]   = input_dimz;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Space2Batch, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Space2Batch.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuSpace2Batch", program, 7, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 7);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (blockw) vxReleaseScalar(&blockw);
    if (blockh) vxReleaseScalar(&blockh);
    if (padX) vxReleaseScalar(&padX);
    if (padY) vxReleaseScalar(&padY);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (blockw) vxReleaseScalar(&blockw);
    if (blockh) vxReleaseScalar(&blockh);
    if (padX) vxReleaseScalar(&padX);
    if (padY) vxReleaseScalar(&padY);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/********Tensor Mean on x-axis****************************************************/
vxnne_shader_executable vxnneGetGPUTensorMeanAxisShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_float32              axis_coef,
    vx_tensor               input,
    vx_tensor               output,
    vx_uint32               axis)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
    char *programSources   = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_uint32     dims                  = TENSOR_DIM_NUM(input);
    vx_uint32     width                 = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     height                = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
    vx_uint32     depth                 = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_enum       input_format          = TENSOR_DATA_TYPE(input);
    vx_enum       output_format         = TENSOR_DATA_TYPE(output);
    vx_scalar     axisScale             = vxCreateScalar(context, VX_TYPE_FLOAT32, &axis_coef);
    vx_reference  parameters[8]         = {(vx_reference)input, (vx_reference)axisScale, (vx_reference)output, (vx_reference)VX_NULL, (vx_reference)VX_NULL, (vx_reference)VX_NULL, (vx_reference)VX_NULL, (vx_reference)VX_NULL};
    vx_uint32     paramnum              = 3;
    vx_scalar     zpIn     = NULL;
    vx_scalar     zpOut    = NULL;
    vx_scalar     scaleIn  = NULL;
    vx_scalar     scaleOut = NULL;
    vx_scalar     sCount   = NULL;
    char          subKernelName[32];
    vx_uint32     offset = 0;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, input=%p, output=%p",
         context, kernelEnum, borderMode, input, output);

    borderMode->mode = VX_BORDER_CONSTANT;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorMeanAxis0, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorMeanAxis.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorMeanAxis", program, 2, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (0 == axis)
    {
        sCount                 = vxCreateScalar(context, VX_TYPE_INT32, &width);
        parameters[paramnum++] = (vx_reference)sCount;
    }
    else if (1 == axis)
    {
        sCount                 = vxCreateScalar(context, VX_TYPE_INT32, &height);
        parameters[paramnum++] = (vx_reference)sCount;
    }
    else if (2 == axis)
    {
        sCount                 = vxCreateScalar(context, VX_TYPE_INT32, &depth);
        parameters[paramnum++] = (vx_reference)sCount;
    }

    gcoOS_PrintStrSafe(subKernelName, sizeof(subKernelName), &offset, "%d_", axis);

    if ((input_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16) ||
        (input_format == VX_TYPE_FLOAT32 && output_format == VX_TYPE_FLOAT32))
    {
        gcoOS_PrintStrSafe(subKernelName, sizeof(subKernelName), &offset, "FP32");
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, subKernelName, borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if (input_format == VX_TYPE_UINT8 && output_format == VX_TYPE_UINT8)
    {
        vx_float32    inputScaleValue       = TENSOR_TF_SCALE(input);
        vx_float32    outputScaleValue      = 1.0f/TENSOR_TF_SCALE(output);
        vx_int32      inputZPValue          = TENSOR_TF_ZEROPOINT(input);
        vx_int32      outputZPValue         = TENSOR_TF_ZEROPOINT(output);

        zpIn                  = vxCreateScalar(context, VX_TYPE_INT32, &inputZPValue);
        zpOut                 = vxCreateScalar(context, VX_TYPE_INT32, &outputZPValue);
        scaleIn               = vxCreateScalar(context, VX_TYPE_FLOAT32, &inputScaleValue);
        scaleOut              = vxCreateScalar(context, VX_TYPE_FLOAT32, &outputScaleValue);

        parameters[paramnum++] = (vx_reference)scaleIn;
        parameters[paramnum++] = (vx_reference)zpIn;
        parameters[paramnum++] = (vx_reference)scaleOut;
        parameters[paramnum++] = (vx_reference)zpOut;

        gcoOS_PrintStrSafe(subKernelName, sizeof(subKernelName), &offset, "Quant8");
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, subKernelName, borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    if (0 == axis)
    {
        execution_parameters.globalWorkSize[0]   = height;
        execution_parameters.globalWorkSize[1]   = depth;
    }
    else if (1 == axis)
    {
        execution_parameters.globalWorkSize[0]   = width;
        execution_parameters.globalWorkSize[1]   = depth;
    }
    else if (2 == axis)
    {
        execution_parameters.globalWorkSize[0]   = width;
        execution_parameters.globalWorkSize[1]   = height;
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramnum);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (axisScale) vxReleaseScalar(&axisScale);
    if(zpIn) vxReleaseScalar(&zpIn);
    if(zpOut) vxReleaseScalar(&zpOut);
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(sCount)   vxReleaseScalar(&sCount);
    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (axisScale) vxReleaseScalar(&axisScale);
    if(zpIn) vxReleaseScalar(&zpIn);
    if(zpOut) vxReleaseScalar(&zpOut);
    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(sCount)   vxReleaseScalar(&sCount);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/********gpuSvdf****************************************/
vxnne_shader_executable vxnneGetGPUSvdfShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               bias,
    vx_tensor               weight,
    vx_tensor               recurrent,
    vx_enum                 activation,
    vx_int32                rankValue,
    vx_tensor               state_in,
    vx_tensor               state_out,
    vx_tensor               output
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[8]              = {(vx_reference)input, (vx_reference)bias, (vx_reference)weight, (vx_reference)recurrent,
                                                (vx_reference)NULL, (vx_reference)state_in, (vx_reference)state_out, (vx_reference)output};
    vx_enum       inputFormat                = TENSOR_DATA_TYPE(input);
    vx_enum       outputFormat               = TENSOR_DATA_TYPE(output);
    vx_uint32     output_height              = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_tensor     bias_rs                    = NULL;
    vx_int32      sizes[4]                   = {1, 1, 1, 1};
    vx_uint32     bs_dims                    = TENSOR_DIM_NUM(bias);
    vx_uint32     output_width               = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_scalar     rank                       = vxCreateScalar(context, VX_TYPE_INT32, &rankValue);

   gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, input=%p, bias=%p, weight=%p, output=%p",
         context, kernelEnum, borderMode, input, bias, weight, output);

    parameters[4] = (vx_reference)rank;

    if (!(((inputFormat == VX_TYPE_FLOAT16) && (outputFormat == VX_TYPE_FLOAT16)) ||
           ((inputFormat == VX_TYPE_FLOAT32) && (outputFormat == VX_TYPE_FLOAT32))) )
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    borderMode->mode = VX_BORDER_CONSTANT;
    if(inputFormat == VX_TYPE_FLOAT16)
        borderMode->constant_value.S16 = 0;
    else
        borderMode->constant_value.S32 = 0;

    if (bs_dims == 1)
    {
        sizes[0] = TENSOR_VIEW_SIZE_INDEX(bias, 0);
        bias_rs = vxoTensor_ReshapeTensor(bias, sizes, 2);
        parameters[1] = (vx_reference)bias_rs;
    }

    execution_parameters.globalWorkSize[0]   = output_width;
    execution_parameters.globalWorkSize[1]   = output_height;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Svdf, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Svdf.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuSvdf", program, 8, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if(activation == VX_NN_ACTIVATION_NONE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if(activation == VX_NN_ACTIVATION_RELU)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32Relu", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if(activation == VX_NN_ACTIVATION_RELU1)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32Relu1", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if(activation == VX_NN_ACTIVATION_RELU6)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32Relu6", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else
    {
        vxError("activation type is not support");
        goto OnError;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
    if (rank) vxReleaseScalar(&rank);
    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (bias_rs) vxoTensor_ReleaseTensor(&bias_rs);
    if (rank) vxReleaseScalar(&rank);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/************************gpuTensorPad*****************************************************************/
vxnne_shader_executable vxnneGetGPUTensorPadShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               inputs,
    vx_scalar               padLeft,
    vx_scalar               padRight,
    vx_scalar               padTop,
    vx_scalar               padBottom,
    vx_scalar               padMode,
    vx_scalar               padConst,
    vx_tensor               outputs)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
#endif
    vx_program                       program              = VX_NULL;
    vx_status                        status               = VX_FAILURE;
    vxnne_shader_executable          shaderExecutable     = VX_NULL;
    vxnne_kernel_shaders             kernel               = NULL;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[4]             = {(vx_reference)inputs, (vx_reference)padLeft, (vx_reference)padTop, (vx_reference)outputs};
    vx_uint32     input_width                = TENSOR_VIEW_SIZE_INDEX(inputs, 0);
    vx_uint32     input_height               = TENSOR_VIEW_SIZE_INDEX(inputs, 1);
    vx_uint32     input_depth                = TENSOR_VIEW_SIZE_INDEX(inputs, 2);
    vx_uint32     output_width               = TENSOR_VIEW_SIZE_INDEX(outputs, 0);
    vx_uint32     output_height              = TENSOR_VIEW_SIZE_INDEX(outputs, 1);
    vx_uint32     output_depth               = TENSOR_VIEW_SIZE_INDEX(outputs, 2);
    vx_enum       inputFormat                = TENSOR_DATA_TYPE(inputs);
    vx_enum       padModev                   = padMode->value->e;
    vx_int32      padConstv                  = padConst->value->n32;
    vx_uint32     padLeftv                   = padLeft->value->u32;
    vx_uint32     padRightv                  = padRight->value->u32;
    vx_uint32     padTopv                    = padTop->value->u32;
    vx_uint32     padBottomv                 = padBottom->value->u32;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, inputs=%p, outputs=%p",
         context, kernelEnum, borderMode, inputs, outputs);

    if (input_width + padLeftv + padRightv != output_width ||
        input_height + padTopv + padBottomv != output_height || input_depth != output_depth)
    {
        vxError("The input size not match with the output size! failed at function %s line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }

    if (padModev == VX_PAD_CONSTANT)
    {
        borderMode->mode = VX_BORDER_CONSTANT;
        if (inputFormat == VX_TYPE_FLOAT16)
            borderMode->constant_value.S16 = (vx_int16)padConstv;
        else if (inputFormat == VX_TYPE_UINT8)
            borderMode->constant_value.U8 = (vx_uint8)padConstv;
        else if (inputFormat == VX_TYPE_FLOAT32)
            borderMode->constant_value.S32 = (vx_int32)padConstv;
    }
    else if (padModev == VX_PAD_REPLICATE)
    {
        borderMode->mode = VX_BORDER_REPLICATE;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorPad, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorPad.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorPad", program, 4, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = output_width;
    execution_parameters.globalWorkSize[1]   = output_height;
    execution_parameters.globalWorkSize[2]   = output_depth;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/********Tensor crop****************************************************/
vxnne_shader_executable vxnneGetGPUTensorCropShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_int32                start[4],
    vx_int32                stop[4],
    vx_tensor               input,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[2]         = {(vx_reference)input, (vx_reference)output};
    vx_enum       input_format          = TENSOR_DATA_TYPE(input);
    vx_enum       output_format         = TENSOR_DATA_TYPE(output);
    vx_uint32     dims                  = TENSOR_DIM_NUM(output);
    vx_uint32     width                 = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32     height                = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(output, 1) : 1;
    vx_uint32     depth                 = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(output, 2) : 1;
    vx_tensor     src                   = NULL;
    vx_tensor     dst                   = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, input=%p, output=%p",
         context, kernelEnum, borderMode, input, output);

    borderMode->mode = VX_BORDER_REPLICATE;

    if (TENSOR_DIM_NUM(input) == 1)
    {
        vx_uint32 sizes[4] = {1, 1, 1, 1};

        sizes[0]    = TENSOR_VIEW_SIZE_INDEX(input, 0);
        src         = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, 2);
        parameters[0] = (vx_reference)src;
    }

    if (TENSOR_DIM_NUM(output) == 1)
    {
        vx_uint32 sizes[4] = {1, 1, 1, 1};

        sizes[0]    = TENSOR_VIEW_SIZE_INDEX(output, 0);
        dst         = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, 2);
        parameters[1] = (vx_reference)dst;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorCrop, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorCrop.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorCrop", program, 2, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if ((input_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16) ||
        (input_format == VX_TYPE_FLOAT32 && output_format == VX_TYPE_FLOAT32))
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if (input_format == VX_TYPE_UINT8 && output_format == VX_TYPE_UINT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else
    {
        vxError("Invalid data format at %s line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }


    execution_parameters.globalWorkOffset[0] = start[0];
    execution_parameters.globalWorkOffset[1] = start[1];
    execution_parameters.globalWorkOffset[2] = start[2];
    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (src) vxoTensor_ReleaseTensor(&src);
    if (dst) vxoTensor_ReleaseTensor(&dst);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (src) vxoTensor_ReleaseTensor(&src);
    if (dst) vxoTensor_ReleaseTensor(&dst);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif

    gcmFOOTER_NO();
    return VX_NULL;
}

/********Tensor strided slice****************************************************/
vxnne_shader_executable vxnneGetGPUTensorStridedSliceShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_int32                start[4],
    vx_int32                stop[4],
    vx_int32                stride[4],
    vx_tensor               input,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum       input_format          = TENSOR_DATA_TYPE(input);
    vx_enum       output_format         = TENSOR_DATA_TYPE(output);
    vx_tensor     src                   = NULL;
    vx_tensor     dst                   = NULL;
    vx_scalar     offsetX               = vxCreateScalar(context, VX_TYPE_INT32, &start[0]);
    vx_scalar     offsetY               = vxCreateScalar(context, VX_TYPE_INT32, &start[1]);
    vx_scalar     offsetZ               = vxCreateScalar(context, VX_TYPE_INT32, &start[2]);
    vx_scalar     strideX               = vxCreateScalar(context, VX_TYPE_INT32, &stride[0]);
    vx_scalar     strideY               = vxCreateScalar(context, VX_TYPE_INT32, &stride[1]);
    vx_scalar     strideZ               = vxCreateScalar(context, VX_TYPE_INT32, &stride[2]);
    vx_uint32     dims                  = TENSOR_DIM_NUM(output);
    vx_uint32     width                 = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32     height                = dims > 1 ? TENSOR_VIEW_SIZE_INDEX(output, 1) : 1;
    vx_uint32     depth                 = dims > 2 ? TENSOR_VIEW_SIZE_INDEX(output, 2) : 1;
    vx_reference  parameters[8]         = {(vx_reference)input, (vx_reference)output, (vx_reference)offsetX, (vx_reference)offsetY, (vx_reference)offsetZ, (vx_reference)strideX, (vx_reference)strideY, (vx_reference)strideZ};

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, input=%p, output=%p",
         context, kernelEnum, borderMode, input, output);

    borderMode->mode = VX_BORDER_REPLICATE;

    if (TENSOR_DIM_NUM(input) == 1)
    {
        vx_uint32 sizes[4] = {1, 1, 1, 1};

        sizes[0]    = TENSOR_VIEW_SIZE_INDEX(input, 0);
        src         = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, 2);
        parameters[0] = (vx_reference)src;
    }

    if (TENSOR_DIM_NUM(output) == 1)
    {
        vx_uint32 sizes[4] = {1, 1, 1, 1};

        sizes[0]    = TENSOR_VIEW_SIZE_INDEX(output, 0);
        dst         = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, 2);
        parameters[1] = (vx_reference)dst;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorStridedSlice, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorStridedSlice.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuStridedSlice", program, 8, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if ((input_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16) ||
        (input_format == VX_TYPE_FLOAT32 && output_format == VX_TYPE_FLOAT32))
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if (input_format == VX_TYPE_UINT8 && output_format == VX_TYPE_UINT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else
    {
        vxError("Invalid data format at %s line %d", __FUNCTION__, __LINE__);
        goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = depth;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 8);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (src) vxoTensor_ReleaseTensor(&src);
    if (dst) vxoTensor_ReleaseTensor(&dst);
    if (offsetX) vxReleaseScalar(&offsetX);
    if (offsetY) vxReleaseScalar(&offsetY);
    if (offsetZ) vxReleaseScalar(&offsetZ);
    if (strideX) vxReleaseScalar(&strideX);
    if (strideY) vxReleaseScalar(&strideY);
    if (strideZ) vxReleaseScalar(&strideZ);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (src) vxoTensor_ReleaseTensor(&src);
    if (dst) vxoTensor_ReleaseTensor(&dst);
    if (offsetX) vxReleaseScalar(&offsetX);
    if (offsetY) vxReleaseScalar(&offsetY);
    if (offsetZ) vxReleaseScalar(&offsetZ);
    if (strideX) vxReleaseScalar(&strideX);
    if (strideY) vxReleaseScalar(&strideY);
    if (strideZ) vxReleaseScalar(&strideZ);
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/********gpuReverse****************************************/
vxnne_shader_executable vxnneGetGPUReverseShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output,
    vx_uint32               axsisNum,
    vx_uint32*              axsis
    )
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[2]              = {(vx_reference)input, (vx_reference)output};
    vx_enum       inputFormat                = TENSOR_DATA_TYPE(input);
    vx_enum       outputFormat               = TENSOR_DATA_TYPE(output);
    vx_uint32     dims                       = TENSOR_DIM_NUM(input);
    vx_uint32     channel                    = dims < 3 ? 1 : TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32     height                     = dims < 2 ? 1 : TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32     width                      = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_tensor     input_rs                   = NULL;
    vx_tensor     output_rs                  = NULL;
    vx_int32      rs_sizes[4]                = {1, 1, 1, 1};

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, input=%p, output=%p",
         context, kernelEnum, borderMode, input, output);
    borderMode->mode = VX_BORDER_REPLICATE;

    if (dims == 1)
    {
        rs_sizes[0]   = width;
        input_rs      = vxoTensor_ReshapeTensor(input, rs_sizes, 2);
        output_rs     = vxoTensor_ReshapeTensor(output, rs_sizes, 2);
        parameters[0] = (vx_reference)input_rs;
        parameters[1] = (vx_reference)output_rs;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Reverse, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Reverse.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorReverse", program, 2, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
    {
        if (axsisNum == 1)
        {
            if (axsis[0] == 0)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis0_FP32", borderMode);
            }
            else if (axsis[0] == 1)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis1_FP32", borderMode);
            }
            else if (axsis[0] == 2)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis2_FP32", borderMode);
            }
            if (!shaderExecutable) goto OnError;
        }
        else if (axsisNum == 2)
        {
            if (axsis[0] == 0 && axsis[1] == 1)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis01_FP32", borderMode);
            }
            else if (axsis[0] == 0 && axsis[1] == 2)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis02_FP32", borderMode);
            }
            else if (axsis[0] == 1 && axsis[1] == 2)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis12_FP32", borderMode);
            }
            if (!shaderExecutable) goto OnError;
        }
        else if (axsisNum == 3)
        {
            if (axsis[0] == 0 && axsis[1] == 1 && axsis[2] == 2)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis012_FP32", borderMode);
            }
            if (!shaderExecutable) goto OnError;
        }
    }
    else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        if (axsisNum == 1)
        {
            if (axsis[0] == 0)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis0_Quant8", borderMode);
            }
            else if (axsis[0] == 1)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis1_Quant8", borderMode);
            }
            else if (axsis[0] == 2)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis2_Quant8", borderMode);
            }
            if (!shaderExecutable) goto OnError;
        }
        else if (axsisNum == 2)
        {
            if (axsis[0] == 0 && axsis[1] == 1)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis01_Quant8", borderMode);
            }
            else if (axsis[0] == 0 && axsis[1] == 2)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis02_Quant8", borderMode);
            }
            else if (axsis[0] == 1 && axsis[1] == 2)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis12_Quant8", borderMode);
            }
            if (!shaderExecutable) goto OnError;
        }
        else if (axsisNum == 3)
        {
            if (axsis[0] == 0 && axsis[1] == 1 && axsis[2] == 2)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_axis012_Quant8", borderMode);
            }
            if (!shaderExecutable) goto OnError;
        }
    }

    execution_parameters.globalWorkSize[0]   = width;
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = channel;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif
    gcmFOOTER_NO();
    return VX_NULL;
}

/********gpuConv2D**********************************************/
vxnne_shader_executable vxnneGPUConv2D_1x1ShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_bool                 enable_cast_format,
    vx_bool                 enable_packed_weights,
    vx_tensor               input,
    vx_tensor               weight,
    vx_tensor               bias,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char        *programSources          = NULL;
#endif
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {1, 1, 1}, {0, 0, 0}, {0, 0, 0}};
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders    kernel;
    vx_program  program                  = VX_NULL;
    vx_status   status                   = VX_FAILURE;
    vx_uint32   width                    = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32   height                   = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32   inputSize                = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32   depth                    = TENSOR_VIEW_SIZE_INDEX(output, 2);
    vx_uint32   dims                     = TENSOR_DIM_NUM(output) == 1 ? 2 : TENSOR_DIM_NUM(output);
    vx_uint32   kernel_x                 = TENSOR_VIEW_SIZE_INDEX(weight, 0);
    vx_uint32   kernel_y                 = TENSOR_VIEW_SIZE_INDEX(weight, 1);
    vx_uint32   ifm                      = TENSOR_VIEW_SIZE_INDEX(weight, 2);
    vx_uint32   ofm                      = TENSOR_VIEW_SIZE_INDEX(weight, 3);
    vx_tensor   weights                  = NULL;
    vx_tensor   biases                   = NULL;
    vx_scalar   cycle                    = NULL;
    vx_uint32   input_ZP                 = TENSOR_TF_ZEROPOINT(input);
    vx_uint32   weight_ZP                = TENSOR_TF_ZEROPOINT(weight);
    vx_uint32   output_ZP                = TENSOR_TF_ZEROPOINT(output);
    vx_float32  input_scale              = TENSOR_TF_SCALE(input);
    vx_float32  weight_scale             = TENSOR_TF_SCALE(weight);
    vx_float32  output_scale             = (vx_float32)1.0/TENSOR_TF_SCALE(output);

    vx_enum     inputFormat              = TENSOR_DATA_TYPE(input);
    vx_int32    size[]                   = {1, 1, 1, 1};
    vx_scalar   scaleIn                  = NULL;
    vx_scalar   zpIn                     = NULL;
    vx_scalar   zpWeight                 = NULL;
    vx_scalar   zpOut                    = NULL;
    vx_tensor   inputs                   = NULL;
    vx_tensor   outputs                  = NULL;
    vx_uint32   input_width              = width;
    vx_uint32   output_width             = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_bool     enable_four_pixel        = vx_false_e;
    vx_bool     enable_adjust_biases     = vx_false_e;
    vx_bool     enable_2d_img            = vx_false_e;
    vx_uint32   element_cnt_input        = 0;
    vx_uint32   element_cnt_kernel       = 0;
    vx_float32  radio                    = 0;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    status  = vxoTensor_GetTensorElementCount(input, &element_cnt_input);
    status |= vxoTensor_GetTensorElementCount(weight, &element_cnt_kernel);
    if (status != VX_SUCCESS) goto OnError;

    radio = (vx_float32)element_cnt_kernel / (vx_float32)element_cnt_input;

    enable_adjust_biases     = TENSOR_QUANT_TYPE(weight) == VX_QUANT_AFFINE_SCALE && TENSOR_QUANT_TYPE(bias);

    size[0] = kernel_x * kernel_y * ifm;
    size[1] = ofm;
    dims    = TENSOR_DIM_NUM(weight) == 1 ? 2 : TENSOR_DIM_NUM(weight);
    weights = vxoTensor_ReshapeTensor(weight, size, dims);

    size[0] = depth;
    size[1] = 1;
    dims    = TENSOR_DIM_NUM(bias) == 1 ? 2 : TENSOR_DIM_NUM(bias);
    biases  = vxoTensor_ReshapeTensor(bias, size, dims);

    enable_2d_img = (vx_bool)((width * height < IMG_MAX_WIDTH) && depth < IMG_MAX_WIDTH );

    if (inputSize % ALIGN_SIZE4 == 0)
        borderMode->mode = VX_BORDER_REPLICATE;
    else
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
        else if (inputFormat == VX_TYPE_FLOAT16)
        {
            borderMode->constant_value.S16 = 0;
        }
    }

    if (enable_2d_img)
    {
        input_width = width * height;

        size[0] = width * height;
        size[1] = inputSize;
        dims    = TENSOR_DIM_NUM(input);
        inputs  = vxoTensor_ReshapeTensor(input, size, dims);

        output_width = TENSOR_VIEW_SIZE_INDEX(output, 0) * TENSOR_VIEW_SIZE_INDEX(output, 1);
        size[0] = output_width;
        size[1] = depth;
        dims    = TENSOR_DIM_NUM(output);
        outputs = vxoTensor_ReshapeTensor(output, size, dims);
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Conv2D_1x1, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Conv2D_1x1.vx", path));

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

     status = vxBuildProgram(program, "-cl-viv-vx-extension");
     if (status != VX_SUCCESS) goto OnError;

     kernel = vxnneAddKernelShadersInProgram(context, "gpuConv2D_1x1", program, 0, kernelEnum);
     if (!kernel) goto OnError;

     vxReleaseProgram(&program);
    }

    cycle = vxCreateScalar(context, VX_TYPE_INT32, &inputSize);

    if (enable_cast_format)
    {
        vx_float32 outZP        = (vx_float32)output_ZP + 0.5f;
        vx_float32 wZP        = (vx_float32)weight_ZP;
        vx_float32 uint8_scale  = input_scale * weight_scale * output_scale;

        vx_reference parameters[9] = {(vx_reference)input, (vx_reference)weights, (vx_reference)biases, (vx_reference)cycle,
                     (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &uint8_scale);
        zpIn = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        zpWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &wZP);
        zpOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outZP);
        parameters[4] = (vx_reference)scaleIn;
        parameters[5] = (vx_reference)zpIn;
        parameters[6] = (vx_reference)zpWeight;
        parameters[7] = (vx_reference)zpOut;

        if (enable_2d_img)
        {
            parameters[0]  = (vx_reference)inputs;
            parameters[8]  = (vx_reference)outputs;
        }

        if (enable_adjust_biases)
        {
            if ((output_width % ALIGN_SIZE4 == 0))
                enable_four_pixel = vx_true_e;

            if (enable_2d_img)
            {
                execution_parameters.globalWorkScale[0] = 4;
                if (enable_four_pixel)
                {
                    if (enable_packed_weights)
                    {
                        execution_parameters.globalWorkScale[1] = 4;
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant32_2D_4X_Wpacked", borderMode);
                    }
                    else
                    {
                        execution_parameters.globalWorkScale[1] = 4;
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant32_2D_4X", borderMode);
                    }
                    if (!shaderExecutable) goto OnError;
                }
                else if (input_width != output_width)
                {
                    execution_parameters.globalWorkScale[0] = 4;
                    execution_parameters.globalWorkScale[1] = 8;

                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant32_8x8_4x8_2D", borderMode);
                    if (!shaderExecutable) goto OnError;
                }
                else
                {
                    execution_parameters.globalWorkScale[1] = 2;

                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant32_2D", borderMode);
                    if (!shaderExecutable) goto OnError;
                }
            }

            if (!shaderExecutable) goto OnError;

            status  = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (enable_four_pixel)
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 8, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);

            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_non_static", borderMode);
            if (!shaderExecutable) goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 9);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_FLOAT16 || inputFormat == VX_TYPE_FLOAT32)
    {
        vx_reference parameters[5]     = {(vx_reference)input, (vx_reference)weights, (vx_reference)biases, (vx_reference)cycle, (vx_reference)output};

        if (enable_2d_img)
        {
            parameters[0]  = (vx_reference)inputs;
            parameters[4]  = (vx_reference)outputs;
        }

        {
            if (output_width % ALIGN_SIZE4 == 0)
                enable_four_pixel = vx_true_e;

            if (enable_2d_img)
            {
                execution_parameters.globalWorkScale[0] = 4;
                if (enable_four_pixel)
                {
                    execution_parameters.globalWorkScale[1] = 4;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32_2D_4X", borderMode);
                }
                else
                {
                    execution_parameters.globalWorkScale[1] = 4;
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32_2D", borderMode);
                }
                if (!shaderExecutable) goto OnError;
            }

            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (enable_four_pixel)
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 4, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (status != VX_SUCCESS) goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        vx_float32 outZP        = (vx_float32)output_ZP + 0.5f;
        vx_float32 wZP        = (vx_float32)weight_ZP;
        vx_float32 uint8_scale  = input_scale * weight_scale * output_scale;

        vx_reference parameters[9] = {(vx_reference)input, (vx_reference)weights, (vx_reference)biases, (vx_reference)cycle,
                     (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};

        scaleIn = vxCreateScalar(context, VX_TYPE_FLOAT32, &uint8_scale);
        zpIn = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        zpWeight = vxCreateScalar(context, VX_TYPE_FLOAT32, &wZP);
        zpOut = vxCreateScalar(context, VX_TYPE_FLOAT32, &outZP);
        parameters[4] = (vx_reference)scaleIn;
        parameters[5] = (vx_reference)zpIn;
        parameters[6] = (vx_reference)zpWeight;
        parameters[7] = (vx_reference)zpOut;

        if (enable_2d_img)
        {
            parameters[0]  = (vx_reference)inputs;
            parameters[8]  = (vx_reference)outputs;
        }

        if (enable_adjust_biases)
        {
            if ((output_width % ALIGN_SIZE4 == 0))
                enable_four_pixel = vx_true_e;

            if (enable_2d_img)
            {
                execution_parameters.globalWorkScale[0] = 4;
                if (enable_four_pixel)
                {
                    if (enable_packed_weights)
                    {
                        execution_parameters.globalWorkScale[1] = 16;
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_Wpacked_4x16_2D_4X", borderMode);
                    }
                    else
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_2D_4X", borderMode);
                    }
                }
                else if ((width * height < depth) || enable_packed_weights)
                {
                    if (enable_packed_weights)
                    {
                        execution_parameters.globalWorkScale[1] = 16;
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_Wpacked_4x16_2D_4S", borderMode);
                    }
                    else
                    {
                        execution_parameters.globalWorkScale[1] = 8;
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant32_4x8_2D_4S", borderMode);
                    }
                }
                else
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_2D", borderMode);
                }
            }

            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (enable_four_pixel)
                status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 8, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);

            if (status != VX_SUCCESS) goto OnError;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_non_static", borderMode);
            if (!shaderExecutable) goto OnError;
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 9);
        if (status != VX_SUCCESS) goto OnError;
    }

    if (enable_2d_img)
    {
        execution_parameters.workDim = 2;

        if (input_width != output_width)
        {
            if (context->nnConfig.fixedFeature.shaderCoreCount > 4)
            {
                execution_parameters.localWorkSize[0]  = 2;
                execution_parameters.localWorkSize[1]  = 16;
            }
            else if (context->nnConfig.fixedFeature.shaderCoreCount > 2)
            {
                execution_parameters.localWorkSize[0]  = 1;
                execution_parameters.localWorkSize[1]  = 2;
            }
            else
            {
                execution_parameters.localWorkSize[0]  = 2;
                execution_parameters.localWorkSize[1]  = 4;
            }
        }
        else if (enable_cast_format )
        {
            if (context->nnConfig.fixedFeature.shaderCoreCount > 4)
            {
                execution_parameters.localWorkSize[0]  = 2;
                execution_parameters.localWorkSize[1]  = 16;
            }
            else if (context->nnConfig.fixedFeature.shaderCoreCount > 2)
            {
                execution_parameters.localWorkSize[0]  = 1;
                execution_parameters.localWorkSize[1]  = 8;
            }
            else
            {
                if (enable_packed_weights)
                {
                    execution_parameters.localWorkSize[0]  = 1;
                    execution_parameters.localWorkSize[1]  = 8;
                }
                else
                {
                    execution_parameters.localWorkSize[0]  = 2;
                    execution_parameters.localWorkSize[1]  = 4;
                }
            }
        }
        else
        {
            if (width * height < depth)
            {
                if (context->nnConfig.fixedFeature.shaderCoreCount > 4)
                {
                    execution_parameters.localWorkSize[0]  = 2;
                    execution_parameters.localWorkSize[1]  = 16;
                }
                else if (context->nnConfig.fixedFeature.shaderCoreCount > 2)
                {
                    execution_parameters.localWorkSize[0]  = 1;
                    execution_parameters.localWorkSize[1]  = 2;
                }
                else
                {
                    if (enable_packed_weights)
                    {
                        execution_parameters.localWorkSize[0]  = 1;
                        execution_parameters.localWorkSize[1]  = 8;
                    }
                    else
                    {
                        execution_parameters.localWorkSize[0]  = 2;
                        execution_parameters.localWorkSize[1]  = 4;
                    }
                }
            }
            else
            {
                execution_parameters.localWorkSize[0]  = 2;
                execution_parameters.localWorkSize[1]  = 4;
            }
        }

        execution_parameters.globalWorkSize[0] = gcmALIGN((input_width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1] = gcmALIGN((depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    }
    else
    {
        output_width = output_width >> 2;

        execution_parameters.localWorkSize[0]  = SHADER_THREAD_COUNT;
        execution_parameters.localWorkSize[1]  = context->nnConfig.fixedFeature.shaderCoreCount;
        execution_parameters.localWorkSize[2]  =1;
        execution_parameters.globalWorkSize[0] = gcmALIGN(output_width, execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1] = gcmALIGN(height, execution_parameters.localWorkSize[1]);
        execution_parameters.globalWorkSize[2] = 1;
    }

    if (!shaderExecutable) goto OnError;
    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (inputs) vxoTensor_ReleaseTensor(&inputs);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (biases) vxoTensor_ReleaseTensor(&biases);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (cycle) vxReleaseScalar(&cycle);
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (zpWeight) vxReleaseScalar(&zpWeight);
    if (zpOut) vxReleaseScalar(&zpOut);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (inputs) vxoTensor_ReleaseTensor(&inputs);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (scaleIn) vxReleaseScalar(&scaleIn);
    if (zpIn) vxReleaseScalar(&zpIn);
    if (zpWeight) vxReleaseScalar(&zpWeight);
    if (zpOut) vxReleaseScalar(&zpOut);
    if (program) vxReleaseProgram(&program);
    if (weights) vxoTensor_ReleaseTensor(&weights);
    if (biases) vxoTensor_ReleaseTensor(&biases);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (cycle) vxReleaseScalar(&cycle);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif

    gcmFOOTER_NO();
    return VX_NULL;
}

/********gpuPrelu****************************************/
vxnne_shader_executable vxnneGetGPUPReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               alpha,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_enum       inputFormat                = TENSOR_DATA_TYPE(input);
    vx_enum       outputFormat               = TENSOR_DATA_TYPE(output);
    vx_uint32     dims                       = TENSOR_DIM_NUM(input);
    vx_uint32     aDims                      = TENSOR_DIM_NUM(alpha);
    vx_uint32     oDims                      = TENSOR_DIM_NUM(output);
    vx_uint32     channel                    = dims < 3 ? 1 : TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32     height                     = dims < 2 ? 1 : TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32     width                      = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32     aWidth                     = TENSOR_VIEW_SIZE_INDEX(alpha, 0);
    vx_uint32     aheight                    = aDims < 2 ? 1 : TENSOR_VIEW_SIZE_INDEX(alpha, 1);
    vx_uint32     achannel                   = aDims < 3 ? 1 : TENSOR_VIEW_SIZE_INDEX(alpha, 2);
    vx_uint32     aSize                      = aWidth * aheight * achannel;
    vx_float32    in_scale                   = TENSOR_TF_SCALE(input);
    vx_float32    out_scale                  = 1.0f/TENSOR_TF_SCALE(output);
    vx_int32      zpInValue                  = TENSOR_TF_ZEROPOINT(input);
    vx_int32      zpOutValue                 = TENSOR_TF_ZEROPOINT(output);
    vx_tensor     input_rs                   = NULL;
    vx_tensor     alpha_rs                   = NULL;
    vx_tensor     output_rs                  = NULL;
    vx_int32      rs_sizes[4]                = {1, 1, 1, 1};
    vx_scalar     scaleIn                    = NULL;
    vx_scalar     scaleOut                   = NULL;
    vx_scalar     zpIn                       = NULL;
    vx_scalar     zpOut                      = NULL;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, input=%p, output=%p",
         context, kernelEnum, borderMode, input, output);
    borderMode->mode = VX_BORDER_REPLICATE;

    if (dims == 1)
    {
        rs_sizes[0]   = width;
        input_rs      = vxoTensor_ReshapeTensor(input, rs_sizes, 2);
    }

    if (oDims == 1)
    {
        rs_sizes[0]   = width;
        output_rs     = vxoTensor_ReshapeTensor(output, rs_sizes, 2);
    }

    if(aSize != aWidth)
    {
        rs_sizes[0] = aSize;
        alpha_rs     = vxoTensor_ReshapeTensor(alpha, rs_sizes, 2);
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, Prelu, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/Prelu.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuPrelu", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    execution_parameters.globalWorkScale[0]  = 1;
    execution_parameters.globalWorkScale[1]  = 1;
    execution_parameters.globalWorkScale[2]  = 1;

    if ((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32))
    {
        vx_reference parameters[3] = {(vx_reference)input, (vx_reference)alpha, (vx_reference)output};
        if(input_rs)
            parameters[0] = (vx_reference)input_rs;
        if(alpha_rs)
            parameters[1] = (vx_reference)alpha_rs;
        if(output_rs)
            parameters[2] = (vx_reference)output_rs;

        if(width % 4 == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_VecFP32", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 2, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (status != VX_SUCCESS) goto OnError;

            execution_parameters.globalWorkScale[0]  = 4;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
        if (status != VX_SUCCESS) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        vx_reference parameters[7] = {(vx_reference)input, (vx_reference)alpha,
                        (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, (vx_reference)output};
        if(input_rs)
            parameters[0] = (vx_reference)input_rs;
        if(alpha_rs)
            parameters[1] = (vx_reference)alpha_rs;
        if(output_rs)
            parameters[6] = (vx_reference)output_rs;
        scaleIn    = vxCreateScalar(context, VX_TYPE_FLOAT32, &in_scale);
        scaleOut   = vxCreateScalar(context, VX_TYPE_FLOAT32, &out_scale);
        zpIn       = vxCreateScalar(context, VX_TYPE_INT32, &zpInValue);
        zpOut      = vxCreateScalar(context, VX_TYPE_INT32, &zpOutValue);
        parameters[2] = (vx_reference)scaleIn;
        parameters[3] = (vx_reference)scaleOut;
        parameters[4] = (vx_reference)zpIn;
        parameters[5] = (vx_reference)zpOut;

        if(width % 4 == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_VecQuant8", borderMode);
            if (!shaderExecutable) goto OnError;

            status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 6, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            if (status != VX_SUCCESS) goto OnError;

            execution_parameters.globalWorkScale[0]  = 4;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            if (!shaderExecutable)
            {
                goto OnError;
            }
        }

        status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 7);
        if (status != VX_SUCCESS) goto OnError;
    }

    execution_parameters.globalWorkSize[0]   = (width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0];
    execution_parameters.globalWorkSize[1]   = height;
    execution_parameters.globalWorkSize[2]   = channel;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (alpha_rs) vxoTensor_ReleaseTensor(&alpha_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);

    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zpIn) vxReleaseScalar(&zpIn);
    if(zpOut) vxReleaseScalar(&zpOut);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (input_rs) vxoTensor_ReleaseTensor(&input_rs);
    if (alpha_rs) vxoTensor_ReleaseTensor(&alpha_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);

    if(scaleIn) vxReleaseScalar(&scaleIn);
    if(scaleOut) vxReleaseScalar(&scaleOut);
    if(zpIn) vxReleaseScalar(&zpIn);
    if(zpOut) vxReleaseScalar(&zpOut);

    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif

    gcmFOOTER_NO();
    return VX_NULL;
}

vxnne_shader_executable vxnneGPUTensorCopyROIShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_scalar               inputXOffset,
    vx_scalar               inputYOffset,
    vx_scalar               outputXOffset,
    vx_scalar               outputYOffset,
    vx_tensor               input,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength = 0;
    char *programSources = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {1, 1, 1}, {0, 0, 0}, {0, 0, 0}};
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_enum      outputFormat      = output->tensorBuffer->dataFormat;
    vx_uint32    dimCount          = TENSOR_VIEW_DIM_NUM(input);
    vx_uint32    width             = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    height            = (dimCount > 1) ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
    vx_uint32    depth             = (dimCount > 2) ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
    vx_uint32    output_dimCount   = TENSOR_VIEW_DIM_NUM(output);
    vx_uint32    output_width      = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32    output_height     = (output_dimCount > 1) ? TENSOR_VIEW_SIZE_INDEX(output, 1) : 1;
    vx_uint32    output_depth      = (output_dimCount > 2) ? TENSOR_VIEW_SIZE_INDEX(output, 2) : 1;
    vx_int32     inputXOffsetV     = inputXOffset->value->n32;
    vx_int32     inputYOffsetV     = inputYOffset->value->n32;
    vx_int32     outputXOffsetV    = outputXOffset->value->n32;
    vx_int32     outputYOffsetV    = outputYOffset->value->n32;
    vx_bool      enable_2d_img     = vx_false_e;
    vx_bool      is_use_2d_fun     = vx_false_e;
    vx_bool      is_pad1           = vx_false_e;
    vx_bool      is_no_pad         = vx_false_e;
    vx_bool      enable_pad1_x7    = vx_false_e;
    vx_bool      enable_pad1_x14   = vx_false_e;
    vx_tensor    inputs            = NULL;
    vx_tensor    outputs           = NULL;
    vx_uint32    dims              = 0;
    vx_int32     size[]            = {1, 1, 1, 1};
    vx_scalar    inputHeight       = NULL;
    vx_scalar    heightDiff        = NULL;


    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, input=%p, output=%p", context, kernelEnum, input, output);

    borderMode->mode = VX_BORDER_REPLICATE;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, TensorCopyROI, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/TensorCopyROI.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "gpuTensorCopyROI", program, 0, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
    {
        is_pad1 = ((inputXOffsetV == 0) && (inputYOffsetV == 0) && (outputXOffsetV == 1) && (outputYOffsetV == 1));
        is_no_pad = ((inputXOffsetV == 0) && (inputYOffsetV == 0) && (outputXOffsetV == 0) && (outputYOffsetV == 0));
        if (is_pad1 || is_no_pad)
        {
            enable_2d_img = (vx_bool)(((width  < IMG_MAX_WIDTH) && (height * depth < IMG_MAX_WIDTH))
                                     && ((output_width < IMG_MAX_WIDTH) && (output_depth * output_height < IMG_MAX_WIDTH)));
            if (enable_2d_img)
            {
                size[0] = width;
                size[1] = depth * height;
                dims    = TENSOR_DIM_NUM(input);
                inputs  = vxoTensor_ReshapeTensor(input, size, dims);

                size[0] = output_width;
                size[1] = output_depth * output_height;
                dims    = TENSOR_DIM_NUM(output);
                outputs = vxoTensor_ReshapeTensor(output, size, dims);
            }
        }

        if (is_pad1 || is_no_pad)
        {
            enable_pad1_x7 =  ((width % 7 == 0) && (width % 4 != 0));
            enable_pad1_x14 = ((width % 14 == 0) && (width % 4 != 0));
             if (width % 16 == 0 && enable_2d_img)
            {
                if (is_pad1)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_Pad1_2D_x16", borderMode);
                }
                else if (is_no_pad)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_NoPad_2D_x16", borderMode);
                }
                is_use_2d_fun    = vx_true_e;
            }
            else if (width % 8 == 0 && enable_2d_img)
            {
                if (is_pad1)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_Pad1_2D_x8", borderMode);
                }
                else if (is_no_pad)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_NoPad_2D_x8", borderMode);
                }
                is_use_2d_fun    = vx_true_e;
            }
            else if (enable_pad1_x14 && enable_2d_img)
            {
                if (is_pad1)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_Pad1_2D_x14", borderMode);
                }
                else if (is_no_pad)
                {
                    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_NoPad_2D_x14", borderMode);
                }
                is_use_2d_fun    = vx_true_e;
            }
            else if (enable_pad1_x7)
            {

                if (enable_2d_img)
                {
                    if (is_pad1)
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_Pad1_2D_x7", borderMode);
                    }
                    else if (is_no_pad)
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_NoPad_2D_x7", borderMode);
                    }
                    is_use_2d_fun    = vx_true_e;
                }
                else
                {
                    if (is_pad1)
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_Pad1_x7", borderMode);
                    }
                    else if (is_no_pad)
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_NoPad_x7", borderMode);
                    }
                }

            }
            else
            {
                if (enable_2d_img)
                {
                    if (is_pad1)
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_Pad1_2D", borderMode);
                    }
                    else if (is_no_pad)
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_NoPad_2D", borderMode);
                    }
                    is_use_2d_fun    = vx_true_e;
                }
                else
                {
                    if (is_pad1)
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_Pad1", borderMode);
                    }
                    else if (is_no_pad)
                    {
                        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8_NoPad", borderMode);
                    }
                }
            }
            if (!shaderExecutable) goto OnError;
            if (is_use_2d_fun)
            {
                vx_reference parameters[4] = {(vx_reference)inputs, (vx_reference)outputs, VX_NULL, VX_NULL};
                vx_int32     height_diff = output_height - height;
                inputHeight = vxCreateScalar(context, VX_TYPE_INT32, &height);
                heightDiff  = vxCreateScalar(context, VX_TYPE_INT32, &height_diff);
                parameters[2] = (vx_reference)inputHeight;
                parameters[3] = (vx_reference)heightDiff;
                status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
            }
            else
            {
                vx_reference parameters[2] = {(vx_reference)input, (vx_reference)output};
                status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
            }

        }
        else
        {
            vx_reference parameters[6] = {(vx_reference)input, (vx_reference)output, (vx_reference)inputXOffset, (vx_reference)inputYOffset,
            (vx_reference)outputXOffset, (vx_reference)outputYOffset};
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
            if (!shaderExecutable) goto OnError;
            status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
        }

        if (width % 16 == 0 && (is_pad1 || is_no_pad) && is_use_2d_fun)
        {
            execution_parameters.globalWorkScale[0] = 16;
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
        }
        else if (width % 8 == 0 && (is_pad1 || is_no_pad) && is_use_2d_fun)
        {
            execution_parameters.globalWorkScale[0] = 8;
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
        }
        else if (enable_pad1_x14)
        {
            execution_parameters.globalWorkScale[0] = 14;
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_TWO_COMPONENTS);
        }
        else if (width % 4 == 0)
        {
            execution_parameters.globalWorkScale[0] = 4;
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
        }
        else if (enable_pad1_x7)
        {
            execution_parameters.globalWorkScale[0] = 7;
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
        }
        else if (width % 2 == 0)
        {
            execution_parameters.globalWorkScale[0] = 2;
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 0, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_TWO_COMPONENTS);
            status |= vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_TWO_COMPONENTS);
        }


        if (status != VX_SUCCESS) goto OnError;
    }

    if (is_use_2d_fun)
    {
        execution_parameters.workDim = 2;
        execution_parameters.localWorkSize[0]  = 1;
        execution_parameters.localWorkSize[1]  = 16;
        if (execution_parameters.localWorkSize[0])
        {
            execution_parameters.globalWorkSize[0] = gcmALIGN((width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
            execution_parameters.globalWorkSize[1] = gcmALIGN((height * depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
        }
        else
        {
             execution_parameters.globalWorkSize[0] = (width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0];
             execution_parameters.globalWorkSize[1] = height * depth;
        }
    }
    else
    {
        execution_parameters.globalWorkSize[0]   = (width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0];
        execution_parameters.globalWorkSize[1]   = height;
        execution_parameters.globalWorkSize[2]   = depth;
    }


    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;
    if (inputs) vxoTensor_ReleaseTensor(&inputs);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (inputHeight) vxReleaseScalar(&inputHeight);
    if (heightDiff) vxReleaseScalar(&heightDiff);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);
    if (inputs) vxoTensor_ReleaseTensor(&inputs);
    if (outputs) vxoTensor_ReleaseTensor(&outputs);
    if (inputHeight) vxReleaseScalar(&inputHeight);
    if (heightDiff) vxReleaseScalar(&heightDiff);

#if !gcdUSE_VXC_BINARY
    if(programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif

    gcmFOOTER_NO();
    return VX_NULL;
}

/********gpuROIPooling****************************************************/
vxnne_shader_executable vxnneGPUROIPoolShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               input_rois,
    vx_uint32               pool_width,
    vx_uint32               pool_height,
    vx_float32              spatial_scale,
    vx_bool                 enable_relu,
    vx_tensor               output)
{
#if !gcdUSE_VXC_BINARY
    vx_size    programLength    = 0;
    char *programSources        = NULL;
#endif
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[15]     = {(vx_reference)input, (vx_reference)input_rois, NULL, NULL, NULL, (vx_reference)output};
    vx_uint32    width             = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    height            = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    rois_depth        = TENSOR_VIEW_SIZE_INDEX(input_rois, 2);
    vx_uint32    rois_batch        = TENSOR_VIEW_SIZE_INDEX(input_rois, 3);
    vx_uint32    rois_dims         = TENSOR_DIM_NUM(input_rois) == 1 ? 2 : TENSOR_DIM_NUM(input_rois);
    vx_scalar    offset_s          = NULL;
    vx_scalar    pool_width_s      = NULL;
    vx_scalar    pool_height_s     = NULL;
    vx_scalar    spatial_scale_s   = NULL;
    vx_scalar    q_pool_width_s    = NULL;
    vx_scalar    q_pool_height_s   = NULL;
    vx_scalar    input_width_s     = NULL;
    vx_scalar    input_height_s    = NULL;
    vx_scalar    enable_relu_s     = NULL;
    vx_scalar    minVal_s          = NULL;
    vx_scalar    input_ZP_s        = NULL;
    vx_scalar    output_ZP_s       = NULL;
    vx_scalar    uint8Scale_s      = NULL;
    vx_uint32    output_width      = TENSOR_VIEW_SIZE_INDEX(output, 0);
    vx_uint32    output_height     = TENSOR_VIEW_SIZE_INDEX(output, 1);
    vx_uint32    output_depth      = TENSOR_VIEW_SIZE_INDEX(output, 2);
    vx_uint32    output_batch      = TENSOR_VIEW_SIZE_INDEX(output, 3);
    vx_uint32    dst_dims          = TENSOR_DIM_NUM(output) == 1 ? 2 : TENSOR_DIM_NUM(output);
    vx_int8      srcFixPointPos    = TENSOR_POS(input);
    vx_int8      dstFixPointPos    = TENSOR_POS(output);
    vx_tensor    input_rois_rs     = NULL;
    vx_tensor    output_rs         = NULL;
    vx_float32   scaleIn           = 1.0;
    vx_float32   scaleOut          = 1.0;
    vx_int32     inputZP           = 0;
    vx_int32     outputZP          = 0;
    vx_enum      inputFormat       = TENSOR_DATA_TYPE(input);
    vx_enum      roisFormat        = TENSOR_DATA_TYPE(input_rois);
    vx_enum      outputFormat      = TENSOR_DATA_TYPE(output);
    vx_int32     rois_sizes[4]     = {rois_depth, rois_batch, 1, 1};
    vx_int32     dst_sizes[4]      = {output_width * output_height, output_depth, output_batch, 1};
    vx_uint32    minVal            = 0;
    vx_uint32    paramnum          = 6;
    vx_int32     offset            = rois_depth == 5? 1 : 0;
    float        q_pool_width      = 1 / (vx_float32)pool_width;
    float        q_pool_height     = 1 / (vx_float32)pool_height;

    gcmHEADER_ARG("context=%p, kernelEnum=0x%x, borderMode=%p, input=%p, output=%p",
         context, kernelEnum, borderMode, input, output);

    if (inputFormat == VX_TYPE_INT8 || inputFormat == VX_TYPE_INT16)
    {
        if (srcFixPointPos >= 0)
        {
            scaleIn    = 1.0f / (vx_float32) (1 << srcFixPointPos);
        }
        else if (srcFixPointPos < 0)
        {
            scaleIn    = (vx_float32)(1 << -srcFixPointPos);
        }
    }
    else if (inputFormat == VX_TYPE_UINT8)
    {
        scaleIn = TENSOR_TF_SCALE(input);
        inputZP = TENSOR_TF_ZEROPOINT(input);
    }

    if (outputFormat == VX_TYPE_INT8 || outputFormat == VX_TYPE_INT16)
    {
        if (dstFixPointPos >= 0)
        {
            scaleOut = (vx_float32) (1 << dstFixPointPos);
        }
        else if (dstFixPointPos < 0)
        {
            scaleOut = 1.0f / (vx_float32) (1 << -dstFixPointPos);
        }
    }
    else if (outputFormat == VX_TYPE_UINT8)
    {
        scaleOut = TENSOR_TF_SCALE(output);
        outputZP = TENSOR_TF_ZEROPOINT(output);
    }

    borderMode->mode   = VX_BORDER_REPLICATE;
    input_rois_rs      = vxoTensor_ReshapeTensor(input_rois, rois_sizes, rois_dims);
    output_rs          = vxoTensor_ReshapeTensor(output, dst_sizes, dst_dims);
    pool_width_s       = vxCreateScalar(context, VX_TYPE_INT32, &pool_width);
    pool_height_s      = vxCreateScalar(context, VX_TYPE_INT32, &pool_height);
    spatial_scale_s    = vxCreateScalar(context, VX_TYPE_FLOAT32, &spatial_scale);
    offset_s           = vxCreateScalar(context, VX_TYPE_INT32, &offset);
    q_pool_width_s     = vxCreateScalar(context, VX_TYPE_FLOAT32, &q_pool_width);
    q_pool_height_s    = vxCreateScalar(context, VX_TYPE_FLOAT32, &q_pool_height);
    input_width_s      = vxCreateScalar(context, VX_TYPE_INT32, &width);
    input_height_s     = vxCreateScalar(context, VX_TYPE_INT32, &height);
    parameters[1]      = (vx_reference)input_rois_rs;
    parameters[2]      = (vx_reference)pool_width_s;
    parameters[3]      = (vx_reference)pool_height_s;
    parameters[4]      = (vx_reference)spatial_scale_s;
    parameters[5]      = (vx_reference)output_rs;
    parameters[6]      = (vx_reference)offset_s;
    parameters[7]      = (vx_reference)q_pool_width_s;
    parameters[8]      = (vx_reference)q_pool_height_s;
    parameters[9]      = (vx_reference)input_width_s;
    parameters[10]     = (vx_reference)input_height_s;
    paramnum           = 11;

    if (inputFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.U16 = 0;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */

#if gcdUSE_VXC_BINARY
        vx_uint32 len;
        void * ptr = getGPUKernelInfo(context, ROIPool, &len);
        program = vxCreateProgramWithBinary(context, ptr, len);
#else
        char path[_vxcFILENAME_MAX];

        vxmONERROR(getFilePath("nngpu_kernels/ROIPool.vx", path));

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

        kernel = vxnneAddKernelShadersInProgram(context, "roiPooling", program, 6, kernelEnum);
        if (!kernel) goto OnError;

        vxReleaseProgram(&program);
    }

    if (enable_relu)
    {
        if (outputFormat == VX_TYPE_UINT8 && TENSOR_QUANT_TYPE(output) == VX_QUANT_AFFINE_SCALE)
        {
            minVal = TENSOR_TF_ZEROPOINT(output);
        }
        else
        {
            minVal = 0;
        }
    }
    else
    {
        if (inputFormat == VX_TYPE_FLOAT16)
        {
            minVal = 0xFC00;
        }
        else if (inputFormat == VX_TYPE_INT8)
        {
            minVal = 0x80;
        }
        else if (inputFormat == VX_TYPE_UINT8)
        {
            minVal = 0;
        }
        else if (inputFormat == VX_TYPE_INT16)
        {
            minVal = 0x8000;
        }
    }

    if ((inputFormat == VX_TYPE_FLOAT16   || inputFormat == VX_TYPE_FLOAT32)
       && (roisFormat == VX_TYPE_FLOAT16   || roisFormat == VX_TYPE_FLOAT32)
       && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32))
    {
        enable_relu_s      = vxCreateScalar(context, VX_TYPE_INT32, &enable_relu);
        parameters[paramnum++]     = (vx_reference)enable_relu_s;
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_FP32", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else if (inputFormat == VX_TYPE_UINT8 && (roisFormat == VX_TYPE_FLOAT16 || roisFormat == VX_TYPE_FLOAT32) && outputFormat == VX_TYPE_UINT8)
    {
        vx_float32 uint8Scale   = scaleIn / scaleOut;
        vx_float32 input_ZP     = (vx_float32)inputZP;
        vx_float32 output_ZP    = (vx_float32)outputZP;

        minVal_s      = vxCreateScalar(context, VX_TYPE_INT32, &minVal);
        input_ZP_s    = vxCreateScalar(context, VX_TYPE_INT32, &input_ZP);
        output_ZP_s   = vxCreateScalar(context, VX_TYPE_INT32, &output_ZP);
        uint8Scale_s  = vxCreateScalar(context, VX_TYPE_INT32, &uint8Scale);
        parameters[paramnum++]     = (vx_reference)minVal_s;
        parameters[paramnum++]     = (vx_reference)input_ZP_s;
        parameters[paramnum++]     = (vx_reference)output_ZP_s;
        parameters[paramnum++]     = (vx_reference)uint8Scale_s;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Quant8", borderMode);
        if (!shaderExecutable) goto OnError;
    }
    else
    {
        vxError("input or output's format is not support");
        goto OnError;
    }

    execution_parameters.workDim             = 2;
    execution_parameters.globalWorkScale[0]  = 1;
    execution_parameters.globalWorkScale[1]  = 1;
    execution_parameters.globalWorkSize[0]   = (output_batch + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0];
    execution_parameters.globalWorkSize[1]   = gcmALIGN((output_depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], SHADER_THREAD_COUNT);

    status = vxnneShaderExecutable_SetParametersAttribute(shaderExecutable, 1, VXNNE_SHADER_PARAMETERS_ATTRIBUTE_FOUR_COMPONENTS);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, paramnum);
    if (status != VX_SUCCESS) goto OnError;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto OnError;

    if (input_rois_rs) vxoTensor_ReleaseTensor(&input_rois_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (pool_width_s) vxReleaseScalar(&pool_width_s);
    if (pool_height_s) vxReleaseScalar(&pool_height_s);
    if (spatial_scale_s) vxReleaseScalar(&spatial_scale_s);
    if (offset_s) vxReleaseScalar(&offset_s);
    if (q_pool_width_s) vxReleaseScalar(&q_pool_width_s);
    if (q_pool_height_s) vxReleaseScalar(&q_pool_height_s);
    if (input_width_s) vxReleaseScalar(&input_width_s);
    if (input_height_s) vxReleaseScalar(&input_height_s);
    if (enable_relu_s)  vxReleaseScalar(&enable_relu_s);
    if (minVal_s)  vxReleaseScalar(&minVal_s);
    if (input_ZP_s)  vxReleaseScalar(&input_ZP_s);
    if (output_ZP_s)  vxReleaseScalar(&output_ZP_s);
    if (uint8Scale_s)  vxReleaseScalar(&uint8Scale_s);

    gcmFOOTER_ARG("%p", shaderExecutable);
    return shaderExecutable;

OnError:
    if (program) vxReleaseProgram(&program);
    if (input_rois_rs) vxoTensor_ReleaseTensor(&input_rois_rs);
    if (output_rs) vxoTensor_ReleaseTensor(&output_rs);
    if (pool_width_s) vxReleaseScalar(&pool_width_s);
    if (pool_height_s) vxReleaseScalar(&pool_height_s);
    if (spatial_scale_s) vxReleaseScalar(&spatial_scale_s);
    if (offset_s) vxReleaseScalar(&offset_s);
    if (q_pool_width_s) vxReleaseScalar(&q_pool_width_s);
    if (q_pool_height_s) vxReleaseScalar(&q_pool_height_s);
    if (input_width_s) vxReleaseScalar(&input_width_s);
    if (input_height_s) vxReleaseScalar(&input_height_s);
    if (enable_relu_s)  vxReleaseScalar(&enable_relu_s);
    if (minVal_s)  vxReleaseScalar(&minVal_s);
    if (input_ZP_s)  vxReleaseScalar(&input_ZP_s);
    if (output_ZP_s)  vxReleaseScalar(&output_ZP_s);
    if (uint8Scale_s)  vxReleaseScalar(&uint8Scale_s);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

#if !gcdUSE_VXC_BINARY
    if (programSources)
    {
        vxFree(programSources);
        programSources = NULL;
    }
#endif

    gcmFOOTER_NO();
    return VX_NULL;
}

