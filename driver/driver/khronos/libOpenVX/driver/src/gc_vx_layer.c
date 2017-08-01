/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#include <gc_vx_common.h>
#include <gc_vx_layer.h>

#define IMG_MAX_WIDTH 65536

static vx_uint16 Fp32toFp16(vx_float32 in)
{
    vx_uint32 fp32 = *((vx_uint32 *) &in);
    vx_uint32 t1 = (fp32 & 0x80000000u) >> 16;  /* sign bit. */
    vx_uint32 t2 = (fp32 & 0x7F800000u) >> 13;  /* Exponent bits */
    vx_uint32 t3 = (fp32 & 0x007FE000u) >> 13;  /* Mantissa bits, no rounding */
    vx_uint32 fp16 = 0u;

    if (t2 >= 0x023c00u)
    {
        fp16 = t1 | 0x7BFF;     /* Don't round to infinity. */
    }
    else if (t2 <= 0x01c000u)
    {
        fp16 = t1;
    }
    else
    {
        t2 -= 0x01c000u;
        fp16 = t1 | t2 | t3;
    }

    return (vx_uint16) fp16;
}

/********vxcTensorAdd****************************************************/
vxnne_shader_executable vxnneGetTensorAddShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               convertPolicy,
    vx_tensor               output)
{
    vx_size    programLength = 0;
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vx_enum         input0_format    = VX_TYPE_FLOAT16;
    vx_enum         input1_format    = VX_TYPE_FLOAT16;
    vx_enum         output_format    = VX_TYPE_FLOAT16;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_uint32 input_scale = 0;

    vx_enum         policyEnum = convertPolicy->value->e;
    vx_uint32       width      = input0->tensorBuffer->memory.dims[0][0];
    vx_uint32       height     = input0->tensorBuffer->memory.dims[0][1];
    vx_uint32       depth      = input0->tensorBuffer->memory.dims[0][2];
    vx_reference    parameters[3] = {(vx_reference)input0, (vx_reference)input1, (vx_reference)output};

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_uint32 UniFP16AddFP16_dp2x8[16] = {
        0x55555555, // TCfg
        0x44444444, // ASelt
        0x33221100, 0x77665544, // ABin
        0xaaaaaaaa, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00 // Constant
    };
    vx_uint32 uniINT8ADDINT8_FP16[16] = {
        0x55555555, // TCfg
        0x00000000, // ASelt
        0xb3a29180, 0xf7e6d5c4, // ABin
        0x55555555, // BSelt
        0x10101010, 0x10101010, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };

    status = vxQueryTensor(input0, VX_TENSOR_DATA_TYPE, &input0_format, sizeof(input0_format));
    status |= vxQueryTensor(input1, VX_TENSOR_DATA_TYPE, &input1_format, sizeof(input1_format));
    status |= vxQueryTensor(output, VX_TENSOR_DATA_TYPE, &output_format, sizeof(output_format));

    if (input0_format == VX_TYPE_INT8 &&  input1_format ==  VX_TYPE_INT8 && output_format == VX_TYPE_FLOAT16)
    {
        vx_int8   input0_fixedPointPos    = input0->tensorBuffer->fixedPointPos;
        vx_int8   input1_fixedPointPos    = input1->tensorBuffer->fixedPointPos;
        vx_float32    div_scale0 = 0;
        vx_float32    div_scale1 = 0;

        if (input0_fixedPointPos >= 0)
        {
            div_scale0 = 1.0f / (vx_float32) (1 << input0_fixedPointPos);
        }
        else if (input0_fixedPointPos < 0)
        {
            div_scale0 = (vx_float32) (1 << -input0_fixedPointPos);
        }

        if (input1_fixedPointPos >= 0)
        {
            div_scale1 = 1.0f / (vx_float32) (1 << input1_fixedPointPos);
        }
        else if (input1_fixedPointPos < 0)
        {
            div_scale1 = (vx_float32) (1 << -input1_fixedPointPos);
        }

        input_scale = ((Fp32toFp16(div_scale1) << 16) & 0xFFFF0000) | (Fp32toFp16(div_scale0));

        borderMode->constant_value.U8 = 0;
    }
    else
    {
        borderMode->constant_value.S16 = 0;
    }


    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkOffset[2] = 0;
    execution_parameters.globalWorkScale[0]  = 8;
    execution_parameters.globalWorkScale[1]  = 1;
    execution_parameters.globalWorkScale[2]  = 1;
    execution_parameters.localWorkSize[0]    = 4;
    execution_parameters.localWorkSize[1]    = 2;
    execution_parameters.localWorkSize[2]    = 1;
    execution_parameters.globalWorkSize[0]   = gcmALIGN((width  + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
    execution_parameters.globalWorkSize[1]   = gcmALIGN((height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    execution_parameters.globalWorkSize[2]   = gcmALIGN((depth  + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]);

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char * programSources[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            \n\
            _viv_uniform VXC_512Bits UniFP16AddFP16_dp2x8;\n\
            _viv_uniform VXC_512Bits uniINT8ADDINT8_FP16;\n\
            _viv_uniform unsigned int input_scale;\n\
            \n\
            __kernel void vxcTensorAdd_fp16Saturate(\n\
                __read_only  image2d_array_t input0, \n\
                __read_only  image2d_array_t input1, \n\
                __write_only image2d_array_t output) \n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                vxc_short8 img1_s16, img2_s16; \n\
                vxc_half8 val1_fp16, val2_fp16; \n\
                \n\
                VXC_ReadImage2DArray(img1_s16, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img2_s16, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                \n\
                _viv_asm(COPY, val1_fp16, img1_s16, 16); \n\
                _viv_asm(COPY, val2_fp16, img2_s16, 16); \n\
                VXC_DP2x8(val1_fp16, val1_fp16, val2_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFP16AddFP16_dp2x8); \n\
                _viv_asm(COPY, img1_s16, val1_fp16, 16); \n\
                VXC_WriteImage2DArray(output, coord, img1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n\
            __kernel void vxcTensorAdd_Int8Saturate(\n\
            __read_only  image2d_array_t input0, \n\
            __read_only  image2d_array_t input1, \n\
            __write_only image2d_array_t output) \n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                vxc_char16 in0; \n\
                vxc_half2 scale_h2; \n\
                vxc_half8 v; \n\
                vxc_short8 val_s16; \n\
                VXC_ReadImage2DArray(in0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(in0, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(8, 15, 0, VXC_RM_TowardZero, 0)); \n\
                \n\
                _viv_asm(COPY, scale_h2, input_scale, 4); \n\
                VXC_DP2x8(v, in0, scale_h2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), uniINT8ADDINT8_FP16); \n\
                _viv_asm(COPY, val_s16, v, 16); \n\
                VXC_WriteImage2DArray(output, coord, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n\
            __kernel void vxcTensorAdd_Int8Wrap(\n\
            __read_only  image2d_array_t input0, \n\
            __read_only  image2d_array_t input1, \n\
            __write_only image2d_array_t output) \n\
            {\n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            \n\
            vxc_char16 in0; \n\
            vxc_half2 scale_h2; \n\
            vxc_half8 v; \n\
            vxc_short8 val_s16; \n\
            VXC_ReadImage2DArray(in0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(in0, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(8, 15, 0, VXC_RM_TowardZero, 0)); \n\
            \n\
            _viv_asm(COPY, scale_h2, input_scale, 4); \n\
            VXC_DP2x8(v, in0, scale_h2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniINT8ADDINT8_FP16); \n\
            _viv_asm(COPY, val_s16, v, 16); \n\
            VXC_WriteImage2DArray(output, coord, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n\
            __kernel void vxcTensorAdd_fp16Wrap(\n\
                __read_only  image2d_array_t input0, \n\
                __read_only  image2d_array_t input1, \n\
                __write_only image2d_array_t output) \n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                vxc_short8 img1_s16, img2_s16; \n\
                vxc_half8 val1_fp16, val2_fp16; \n\
                \n\
                VXC_ReadImage2DArray(img1_s16, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img2_s16, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                \n\
                _viv_asm(COPY, val1_fp16, img1_s16, 16); \n\
                _viv_asm(COPY, val2_fp16, img2_s16, 16); \n\
                VXC_DP2x8(val1_fp16, val1_fp16, val2_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16AddFP16_dp2x8); \n\
                _viv_asm(COPY, img1_s16, val1_fp16, 16); \n\
                VXC_WriteImage2DArray(output, coord, img1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n"
        };

        programLength = strlen(programSources[0]);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, &programLength);
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcTensorAdd", program, 3, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (policyEnum == VX_CONVERT_POLICY_SATURATE && input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16)
    {
       shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16Saturate", borderMode);
       if (!shaderExecutable) goto error;
    }
    else if(policyEnum == VX_CONVERT_POLICY_WRAP && input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16Wrap", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (policyEnum == VX_CONVERT_POLICY_SATURATE && input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_FLOAT16)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if(policyEnum == VX_CONVERT_POLICY_WRAP && input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_FLOAT16)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8Wrap", borderMode);
        if (!shaderExecutable) goto error;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFP16AddFP16_dp2x8", 1, UniFP16AddFP16_dp2x8);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniINT8ADDINT8_FP16", 1, uniINT8ADDINT8_FP16);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "input_scale", 1, &input_scale);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcLeakyRelu****************************************************/
vxnne_shader_executable vxnneGetLeakyReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               alpha,
    vx_tensor               output)
{
    vx_size    programLength = 0;
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_uint32       input_size[4] = {0, 0, 0, 0};
    vx_uint32       UniFP16Mul_dp2x8[16] = {
                        0x11111111, // TCfg
                        0x00000000, // ASelt
                        0x03020100, 0x07060504, // ABin
                        0x11111111, // BSelt
                        0x00000000, 0x00000000, // BBin
                        0x00000100, // AccumType, ConstantType, and PostShift
                        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
                    };
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[3]              = {(vx_reference)input, (vx_reference)alpha, (vx_reference)output};
    vx_enum       inputFormat                = input->tensorBuffer->dataFormat;
    vx_enum       outputFormat               = output->tensorBuffer->dataFormat;
    vx_int8       input_fractionLengthValue  = input->tensorBuffer->fixedPointPos;
    vx_int8       output_fractionLengthValue = output->tensorBuffer->fixedPointPos;
    vx_uint32     elementCount = 0;
    vx_image      imgInput     = NULL;
    vx_image      imgOutput    = NULL;
    vx_float32    in_scale     = 1.0f;
    vx_float32    out_scale    = 1.0f;

    if ((inputFormat != VX_TYPE_FLOAT16 && inputFormat != VX_TYPE_INT8) || (outputFormat != VX_TYPE_FLOAT16 && outputFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or output's format is not support");
        goto error;
    }
    status = vxQueryTensor(input, VX_TENSOR_DIMS, input_size, sizeof(input_size));
    if (status != VX_SUCCESS) goto error;
    elementCount = input_size[0] * input_size[1] * input_size[2];

    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkOffset[2] = 0;
    execution_parameters.globalWorkScale[0]  = 8;
    execution_parameters.globalWorkScale[1]  = 1;
    execution_parameters.globalWorkScale[2]  = 1;
    execution_parameters.localWorkSize[0]    = 8;
    execution_parameters.localWorkSize[1]    = 1;
    execution_parameters.localWorkSize[2]    = 1;
    if (elementCount < IMG_MAX_WIDTH)
    {
        execution_parameters.globalWorkSize[0]   = gcmALIGN((elementCount + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = 1;
        execution_parameters.globalWorkSize[2]   = 1;
    }
    else
    {
        execution_parameters.globalWorkSize[0]   = gcmALIGN((input_size[0] + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((input_size[1] + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
        execution_parameters.globalWorkSize[2]   = gcmALIGN((input_size[2] + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]);
    }

    if (input_fractionLengthValue >= 0)
    {
        in_scale = 1.0f / (vx_float32) (1 << input_fractionLengthValue);
    }
    else if (input_fractionLengthValue < 0)
    {
        in_scale = (vx_float32) (1 << -input_fractionLengthValue);
    }
    if (output_fractionLengthValue >= 0)
    {
        out_scale = (vx_float32) (1 << output_fractionLengthValue);
    }
    else if (output_fractionLengthValue < 0)
    {
        out_scale = 1.0f / (vx_float32) (1 << -output_fractionLengthValue);
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char * programSources[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            \n\
            _viv_uniform VXC_512Bits UniFP16Mul_dp2x8; \n\
            _viv_uniform VXC_512Bits UniS8xFp16toFp16_dp2x8; \n\
            _viv_uniform float in_scale; \n\
            _viv_uniform float out_scale; \n\
            _viv_uniform int outputFormat; \n\
            __kernel void vxcLeakyRelu_Fp16Tensor(\n\
                     image2d_array_t   input, \n\
                               float   alpha, \n\
                     image2d_array_t   output)\n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                half alpha_fp16; \n\
                vxc_short8 img_s16; \n\
                vxc_half8 img_fp16, val_fp16; \n\
                \n\
                VXC_ReadImage2DArray(img_s16, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(CONV, alpha_fp16, alpha); \n\
                _viv_asm(COPY, img_fp16, img_s16, 16); \n\
                VXC_DP2x8(val_fp16, img_fp16, alpha_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16Mul_dp2x8); \n\
                VXC_Clamp(img_fp16, img_fp16, val_fp16, img_fp16, VXC_MODIFIER_CLAMP(0, 7, 0, 0)); \n\
                if (outputFormat == 15) \n\
                {\n\
                    _viv_asm(COPY, img_s16, img_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, coord, img_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else if (outputFormat == 2) \n\
                {\n\
                    half scale_fp16; \n\
                    vxc_char8 val_s8; \n\
                    _viv_asm(CONV, scale_fp16, out_scale); \n\
                    VXC_DP2x8(val_s8, img_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFP16Mul_dp2x8); \n\
                    VXC_WriteImage2DArray(output, coord, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            } \n\
            __kernel void vxcLeakyRelu_Fp16Image(\n\
                    __read_only image2d_t    input, \n\
                                    float    alpha, \n\
                    __write_only image2d_t   output)\n\
            {\n\
                int2 coord = (int2)(get_global_id(0), get_global_id(1)); \n\
                \n\
                half alpha_fp16; \n\
                vxc_short8 img_s16; \n\
                vxc_half8 img_fp16, val_fp16; \n\
                \n\
                VXC_ReadImage(img_s16, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(CONV, alpha_fp16, alpha); \n\
                _viv_asm(COPY, img_fp16, img_s16, 16); \n\
                VXC_DP2x8(val_fp16, img_fp16, alpha_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16Mul_dp2x8); \n\
                VXC_Clamp(img_fp16, img_fp16, val_fp16, img_fp16, VXC_MODIFIER_CLAMP(0, 7, 0, 0)); \n\
                if (outputFormat == 15) \n\
                {\n\
                    _viv_asm(COPY, img_s16, img_fp16, 16); \n\
                    VXC_WriteImage(output, coord, img_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else if (outputFormat == 2) \n\
                {\n\
                    half scale_fp16; \n\
                    vxc_char8 val_s8; \n\
                    _viv_asm(CONV, scale_fp16, out_scale); \n\
                    VXC_DP2x8(val_s8, img_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFP16Mul_dp2x8); \n\
                    VXC_WriteImage(output, coord, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            }\n\
            __kernel void vxcLeakyRelu_Int8Tensor(\n\
                     image2d_array_t   input, \n\
                     float             alpha, \n\
                     image2d_array_t   output)\n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                vxc_char8 img_s8; \n\
                vxc_half8 img_fp16, val_fp16; \n\
                half scale_fp16, alpha_fp16; \n\
                \n\
                VXC_ReadImage2DArray(img_s8, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(CONV, scale_fp16, in_scale); \n\
                _viv_asm(CONV, alpha_fp16, alpha); \n\
                \n\
                VXC_DP2x8(img_fp16, img_s8, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                VXC_DP2x8(val_fp16, img_fp16, alpha_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16Mul_dp2x8); \n\
                VXC_Clamp(img_fp16, img_fp16, val_fp16, img_fp16, VXC_MODIFIER_CLAMP(0, 7, 0, 0)); \n\
                if (outputFormat == 15) \n\
                {\n\
                    vxc_short8 val_s16; \n\
                    _viv_asm(COPY, val_s16, img_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, coord, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else if (outputFormat == 2) \n\
                {\n\
                    _viv_asm(CONV, scale_fp16, out_scale); \n\
                    VXC_DP2x8(img_s8, img_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFP16Mul_dp2x8); \n\
                    VXC_WriteImage2DArray(output, coord, img_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            } \n\
            __kernel void vxcLeakyRelu_Int8Image(\n\
                    __read_only image2d_t    input, \n\
                                    float    alpha, \n\
                    __write_only image2d_t   output)\n\
            {\n\
                int2 coord = (int2)(get_global_id(0), get_global_id(1)); \n\
                \n\
                vxc_char8 img_s8; \n\
                vxc_half8 img_fp16, val_fp16; \n\
                half scale_fp16, alpha_fp16; \n\
                \n\
                VXC_ReadImage(img_s8, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(CONV, scale_fp16, in_scale); \n\
                _viv_asm(CONV, alpha_fp16, alpha); \n\
                \n\
                VXC_DP2x8(img_fp16, img_s8, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                VXC_DP2x8(val_fp16, img_fp16, alpha_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16Mul_dp2x8); \n\
                VXC_Clamp(img_fp16, img_fp16, val_fp16, img_fp16, VXC_MODIFIER_CLAMP(0, 7, 0, 0)); \n\
                if (outputFormat == 15) \n\
                {\n\
                    vxc_short8 val_s16; \n\
                    _viv_asm(COPY, val_s16, img_fp16, 16); \n\
                    VXC_WriteImage(output, coord, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else if (outputFormat == 2) \n\
                {\n\
                    _viv_asm(CONV, scale_fp16, out_scale); \n\
                    VXC_DP2x8(img_s8, img_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFP16Mul_dp2x8); \n\
                    VXC_WriteImage(output, coord, img_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            }\n"
        };

        programLength = strlen(programSources[0]);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, &programLength);
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcLeakyRelu", program, 3, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_FLOAT16)
    {
        if (elementCount < IMG_MAX_WIDTH)
        {
            imgInput = vxoTensor_CreateImageFromTensor(input, elementCount, 1, VX_DF_IMAGE_S16);
            if (outputFormat == VX_TYPE_FLOAT16)
            {
                imgOutput = vxoTensor_CreateImageFromTensor(output, elementCount, 1, VX_DF_IMAGE_S16);
            }
            else
            {
                imgOutput = vxoTensor_CreateImageFromTensor(output, elementCount, 1, VX_DF_IMAGE_U8);
            }
            parameters[0] = (vx_reference)imgInput;
            parameters[2] = (vx_reference)imgOutput;
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16Image", borderMode);
            if (!shaderExecutable) goto error;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16Tensor", borderMode);
            if (!shaderExecutable) goto error;
        }
    }
    else
    {
        vx_uint32 UniS8xFp16toFp16_dp2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        if (elementCount < IMG_MAX_WIDTH)
        {
            imgInput = vxoTensor_CreateImageFromTensor(input, elementCount, 1, VX_DF_IMAGE_U8);
            if (outputFormat == VX_TYPE_FLOAT16)
            {
                imgOutput = vxoTensor_CreateImageFromTensor(output, elementCount, 1, VX_DF_IMAGE_S16);
            }
            else
            {
                imgOutput = vxoTensor_CreateImageFromTensor(output, elementCount, 1, VX_DF_IMAGE_U8);
            }
            parameters[0] = (vx_reference)imgInput;
            parameters[2] = (vx_reference)imgOutput;
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8Image", borderMode);
            if (!shaderExecutable) goto error;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8Tensor", borderMode);
            if (!shaderExecutable) goto error;
        }
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniS8xFp16toFp16_dp2x8", 1, UniS8xFp16toFp16_dp2x8);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "in_scale", 1, &in_scale);
        if (status != VX_SUCCESS) goto error;
    }
    status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFP16Mul_dp2x8", 1, UniFP16Mul_dp2x8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "out_scale", 1, &out_scale);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outputFormat", 1, &outputFormat);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(imgInput)  vxReleaseImage(&imgInput);
    if(imgOutput) vxReleaseImage(&imgOutput);

    return shaderExecutable;

error:
    if (program)  vxReleaseProgram(&program);
    if(imgInput)  vxReleaseImage(&imgInput);
    if(imgOutput) vxReleaseImage(&imgOutput);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcFullyConnected****************************************************/
vxnne_shader_executable vxnneGetFullyConnectedShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               weights,
    vx_tensor               bias,
    vx_tensor               output)
{
    vx_size    programLength = 0;
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_scalar  dRelu_s = NULL;
    vx_int32    dRelu = 0;

    vx_enum         output_format    = VX_TYPE_FLOAT16;
    vx_enum         input_format    = VX_TYPE_FLOAT16;
    vx_enum         weights_format    = VX_TYPE_FLOAT16;
    vx_enum         bias_format        = VX_TYPE_FLOAT32;
    vx_uint32       input_size[4] = {0, 0, 0, 0};
    vx_uint32       weight_size[4] = {0, 0, 0, 0};
    vx_uint32       bias_size[4] = {0, 0, 0, 0};
    vx_uint32       output_size[4] = {0, 0, 0, 0};
    vx_uint32       input_width = 0;
    vx_uint32       uniMulAcc[16] = {
                    0x00005555, // TCfg
                    0x00000000, // ASelt
                    0x76543210, 0x00000000, // ABin
                    0x00005555, // BSelt
                    0x76543210, 0x00000000, // BBin
                    0x00000400, // AccumType, ConstantType, and PostShift
                    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 uniMulAcc_Int8[16] = {
        0x55555555, // TCfg
        0x00000000, // ASelt
        0x76543210, 0xfedcba98, // ABin
        0x55555555, // BSelt
        0x76543210, 0xfedcba98, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_image imgInput = NULL;
    vx_image imgWeights = NULL;
    vx_image imgBias = NULL;
    vx_float32 in_scale = 0;

    vx_reference    parameters[5] = {(vx_reference)NULL, (vx_reference)NULL, (vx_reference)NULL, NULL, (vx_reference)output};

    status    = vxQueryTensor(input, VX_TENSOR_DIMS, input_size, sizeof(input_size));
    status    |= vxQueryTensor(weights, VX_TENSOR_DIMS, weight_size, sizeof(weight_size));
    status    |= vxQueryTensor(bias, VX_TENSOR_DIMS, bias_size, sizeof(bias_size));
    status    |= vxQueryTensor(output, VX_TENSOR_DIMS, output_size, sizeof(input_size));
    if (status != VX_SUCCESS) goto error;

    status = vxQueryTensor(output, VX_TENSOR_DATA_TYPE, &output_format, sizeof(output_format));
    status |= vxQueryTensor(input, VX_TENSOR_DATA_TYPE, &input_format, sizeof(input_format));
    status |= vxQueryTensor(weights, VX_TENSOR_DATA_TYPE, &weights_format, sizeof(weights_format));
    status |= vxQueryTensor(bias, VX_TENSOR_DATA_TYPE, &bias_format, sizeof(bias_format));

    if (input_format == VX_TYPE_FLOAT16 && weights_format == VX_TYPE_FLOAT16 && bias_format == VX_TYPE_FLOAT32  && output_format == VX_TYPE_FLOAT16)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, input_size[0] * input_size[1] * input_size[2], 1, VX_DF_IMAGE_U16);
        imgWeights = vxoTensor_CreateImageFromTensor(weights, weight_size[0] * weight_size[1] * weight_size[2], weight_size[3], VX_DF_IMAGE_U16);
        imgBias = vxoTensor_CreateImageFromTensor(bias, bias_size[0], 1, VX_DF_IMAGE_F32);
    }
    else if (input_format == VX_TYPE_INT8 &&  weights_format ==  VX_TYPE_INT8 && bias_format == VX_TYPE_INT32  && output_format == VX_TYPE_INT8)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, input_size[0] * input_size[1] * input_size[2], 1, VX_DF_IMAGE_U8);
        imgWeights = vxoTensor_CreateImageFromTensor(weights, weight_size[0] * weight_size[1] * weight_size[2], weight_size[3], VX_DF_IMAGE_U8);
        imgBias = vxoTensor_CreateImageFromTensor(bias, bias_size[0], 1, VX_DF_IMAGE_S32);
    }
    else
    {
        gcmPRINT("input or outputs format is not support");
        goto error;
    }

    parameters[0] = (vx_reference)imgInput;
    parameters[1] = (vx_reference)imgWeights;
    parameters[2] = (vx_reference)imgBias;

    dRelu_s = vxCreateScalar(context, VX_TYPE_INT32, &dRelu);
    parameters[3] = (vx_reference)dRelu_s;

    if (status != VX_SUCCESS) goto error;

    if (input_format == VX_TYPE_INT8 &&  weights_format ==  VX_TYPE_INT8 && bias_format == VX_TYPE_INT32  && output_format == VX_TYPE_INT8)
    {
        vx_int8   input_fixedPointPos    = input->tensorBuffer->fixedPointPos;
        vx_int8   weights_fixedPointPos = weights->tensorBuffer->fixedPointPos;
        //vx_int8   bias_fixedPointPos    = bias->tensorBuffer->fixedPointPos;
        vx_int8      output_fixedPointPos    = output->tensorBuffer->fixedPointPos;
        vx_int32  multiplicator            = 0;
        if (input_fixedPointPos >= 0)
        {
            multiplicator = multiplicator - input_fixedPointPos;
        }
        else
        {
            multiplicator = multiplicator - input_fixedPointPos;
        }

        if (weights_fixedPointPos >= 0)
        {
            multiplicator = multiplicator - weights_fixedPointPos;
        }
        else
        {
            multiplicator = multiplicator - weights_fixedPointPos;
        }

        if (output_fixedPointPos >= 0)
        {
            multiplicator = multiplicator - output_fixedPointPos;
        }
        else
        {
            multiplicator = multiplicator + output_fixedPointPos;
        }

        if (multiplicator < 0)
        {
            in_scale = 1.0f / (vx_float32) (1 << -multiplicator);
        }
        else
        {
            in_scale = (vx_float32) (1 << multiplicator);
        }

        borderMode->mode = VX_BORDER_CONSTANT;
        borderMode->constant_value.U8 = 0;
    }
    else if (input_format == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.S16 = 0;
    }

    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkScale[0]  = 1;
    execution_parameters.globalWorkScale[1]  = 1;
    execution_parameters.localWorkSize[0]    = 1;
    execution_parameters.localWorkSize[1]    = 8;
    execution_parameters.globalWorkSize[0]   = 1;
    execution_parameters.globalWorkSize[1]   = gcmALIGN((output_size[2] + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char * programSources[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            _viv_uniform int Cycles; \n\
            _viv_uniform float in_scale; \n\
            _viv_uniform VXC_512Bits uniMulAcc; \n\
            _viv_uniform VXC_512Bits uniMulAcc_Int8; \n\
            __kernel void vxcFullyConnected_NoRelu(\n\
                 __read_only image2d_t            input, \n\
                 __read_only image2d_t            weights, \n\
                 __read_only image2d_t            bias, \n\
                                   int            dRelu, \n\
                 __write_only image2d_array_t     output) \n\
            { \n\
                int4 coord_in    = (int4)(16, get_global_id(1), 0, 0); \n\
                int4 coord_out   = (int4)(get_global_id(0), 0, get_global_id(1), 0); \n\
                \n\
                vxc_short8 v0, v1, v2, v3, v4, v5, v6, v7; \n\
                vxc_half8 i0, i1, i2, i3; \n\
                vxc_half8 w0, w1, w2, w3; \n\
                float4 sum = 0; \n\
                float dst = 0; \n\
                dst = read_imagef(bias, coord_out.zw).x; \n\
                do \n\
                { \n\
                    VXC_ReadImage(v0, input, coord_in.xz, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    _viv_asm(COPY, i0, v0, 16); \n\
                    VXC_ReadImage(v1, weights, coord_in.xy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    _viv_asm(COPY, w0, v1, 16); \n\
                    VXC_ReadImage(v2, input, coord_in.xz, VXC_5BITOFFSET_XY(-8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    _viv_asm(COPY, i1, v2, 16); \n\
                    VXC_ReadImage(v3, weights, coord_in.xy, VXC_5BITOFFSET_XY(-8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    _viv_asm(COPY, w1, v3, 16); \n\
                    VXC_ReadImage(v4, input, coord_in.xz, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    _viv_asm(COPY, i2, v4, 16); \n\
                    VXC_ReadImage(v5, weights, coord_in.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    _viv_asm(COPY, w2, v5, 16); \n\
                    VXC_ReadImage(v6, input, coord_in.xz, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    _viv_asm(COPY, i3, v6, 16); \n\
                    VXC_ReadImage(v7, weights, coord_in.xy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    _viv_asm(COPY, w3, v7, 16); \n\
                    \n\
                    coord_in.x += 32; \n\
                    \n\
                    VXC_DP16x1(sum, i0, w0, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniMulAcc); \n\
                    VXC_DP16x1(sum, i1, w1, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniMulAcc); \n\
                    VXC_DP16x1(sum, i2, w2, VXC_MODIFIER(2, 2, 0, VXC_RM_TowardZero, 0), uniMulAcc); \n\
                    VXC_DP16x1(sum, i3, w3, VXC_MODIFIER(3, 3, 0, VXC_RM_TowardZero, 0), uniMulAcc); \n\
                    \n\
                    float4 tmp = {1, 1, 1, 1}; \n\
                    dst = dst + dot(sum, tmp); \n\
                    \n\
                } while (coord_in.x < Cycles); \n\
                vxc_half v; \n\
                _viv_asm(CONV, v, dst); \n\
                _viv_asm(COPY, v0, v, 16); \n\
                VXC_WriteImage2DArray(output, coord_out, v0, VXC_MODIFIER(0, 0, 0,VXC_RM_TowardZero, 0)); \n\
            } \n\
            __kernel void vxcFullyConnected_NoReluInt8(\n\
            __read_only image2d_t            input, \n\
            __read_only image2d_t            weights, \n\
            __read_only image2d_t            bias, \n\
            int            dRelu, \n\
            __write_only image2d_array_t     output) \n\
            { \n\
                int4 coord_in    = (int4)(16, get_global_id(1), 0, 0); \n\
                int4 coord_out   = (int4)(get_global_id(0), 0, get_global_id(1), 0); \n\
                \n\
                vxc_char16 v0, v1, v2, v3, v4, v5, v6, v7; \n\
                float4 sum = 0; \n\
                int temp = 0; \n\
                float dst; \n\
                temp = read_imagei(bias, coord_out.zw).x; \n\
                dst = convert_float(temp); \n\
                do \n\
                { \n\
                VXC_ReadImage(v0, input, coord_in.xz, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(v1, weights, coord_in.xy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(v2, input, coord_in.xz, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(v3, weights, coord_in.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                coord_in.x += 32; \n\
                VXC_ReadImage(v4, input, coord_in.xz, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(v5, weights, coord_in.xy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(v6, input, coord_in.xz, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(v7, weights, coord_in.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                coord_in.x += 32; \n\
                \n\
                VXC_DP16x1(sum, v0, v1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniMulAcc_Int8); \n\
                VXC_DP16x1(sum, v2, v3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniMulAcc_Int8); \n\
                VXC_DP16x1(sum, v4, v5, VXC_MODIFIER(2, 2, 0, VXC_RM_TowardZero, 0), uniMulAcc_Int8); \n\
                VXC_DP16x1(sum, v6, v7, VXC_MODIFIER(3, 3, 0, VXC_RM_TowardZero, 0), uniMulAcc_Int8); \n\
                \n\
                float4 tmp = {1, 1, 1, 1}; \n\
                dst = dst + dot(sum, tmp); \n\
                \n\
                } while (coord_in.x < Cycles); \n\
                dst = in_scale * dst; \n\
                dst = round(dst); \n\
                char val = convert_char_sat(dst); \n\
                VXC_WriteImage2DArray(output, coord_out, val, VXC_MODIFIER(0, 0, 0,VXC_RM_TowardZero, 0)); \n\
            }\n"
        };

        programLength = strlen(programSources[0]);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, &programLength);
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcFullyConnected", program, 5, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (input_format == VX_TYPE_FLOAT16 && weights_format == VX_TYPE_FLOAT16 && bias_format == VX_TYPE_FLOAT32  && output_format == VX_TYPE_FLOAT16)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_NoRelu", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input_format == VX_TYPE_INT8 &&  weights_format ==  VX_TYPE_INT8 && bias_format == VX_TYPE_INT32  && output_format == VX_TYPE_INT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_NoReluInt8", borderMode);
        if (!shaderExecutable) goto error;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniMulAcc", 1, uniMulAcc);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniMulAcc_Int8", 1, uniMulAcc_Int8);
    if (status != VX_SUCCESS) goto error;

    input_width = input_size[0] * input_size[1] * input_size[2] + 16;
    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "Cycles", 1, &input_width);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "in_scale", 1, &in_scale);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(dRelu_s) vxReleaseScalar(&dRelu_s);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgWeights) vxReleaseImage(&imgWeights);
    if(imgBias) vxReleaseImage(&imgBias);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(dRelu_s) vxReleaseScalar(&dRelu_s);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgWeights) vxReleaseImage(&imgWeights);
    if(imgBias) vxReleaseImage(&imgBias);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcAvgPooling****************************************************/
vxnne_shader_executable vxnneGetAvgPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_scalar               poolPadX,
    vx_scalar               poolPadY,
    vx_scalar               rounding,
    vx_tensor               output)
{
    vx_size    programLength[2] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[3] = {(vx_reference)input, VX_NULL, (vx_reference)output};
    vx_enum   inputFormat  = input->tensorBuffer->dataFormat;
    vx_enum   outputFormat = output->tensorBuffer->dataFormat;
    vx_uint32 in_width            = input->tensorBuffer->memory.dims[0][1];
    vx_uint32 in_height         = input->tensorBuffer->memory.dims[0][1];
    vx_uint32 depth             = input->tensorBuffer->memory.dims[0][2];
    vx_uint32 out_width         = output->tensorBuffer->memory.dims[0][0];
    vx_uint32 out_height        = output->tensorBuffer->memory.dims[0][1];
    vx_uint32 stride_v          = stride_s->value->u32;
    vx_uint32 kernel_v          = poolSizeX->value->u32;
    vx_uint32 pad_v             = poolPadX->value->u32;
    vx_scalar in_heights        = vxCreateScalar(context, VX_TYPE_INT32, &in_height);
    vx_image imgInput = NULL;
    vx_image imgOutput = NULL;

    vx_int8   srcFixPointPos  = input->tensorBuffer->fixedPointPos;
    vx_int8   dstFixPointPos  = output->tensorBuffer->fixedPointPos;
    vx_bool      globalPooling_flag = (vx_bool)(stride_v == 1 && pad_v == 0 && out_width == 1 && out_height == 1 && kernel_v == in_width && kernel_v == in_height);
    vx_float32 div_scale = 1.0f;

    parameters[1] = (vx_reference)in_heights;

    if (inputFormat == VX_TYPE_INT8)
    {
        if (srcFixPointPos >= 0)
        {
            div_scale *= 1.0f / (vx_float32) (1 << srcFixPointPos);
        }
        else if (srcFixPointPos < 0)
        {
            div_scale *= (vx_float32)(1 << -srcFixPointPos);
        }
    }

    if (outputFormat == VX_TYPE_INT8)
    {
        if (dstFixPointPos >= 0)
        {
            div_scale *= (vx_float32) (1 << dstFixPointPos);
        }
        else if (dstFixPointPos < 0)
        {
            div_scale *= 1.0f / (vx_float32) (1 << -dstFixPointPos);
        }
    }

    if (globalPooling_flag)
    {

        borderMode->mode = VX_BORDER_CONSTANT;
        if (inputFormat == VX_TYPE_INT8)
        {
            imgInput = vxoTensor_CreateImageFromTensor(input, in_width * in_height, depth, VX_DF_IMAGE_U8);
            borderMode->constant_value.U8 = 0;
        }
        else if (inputFormat == VX_TYPE_FLOAT16)
        {
            imgInput = vxoTensor_CreateImageFromTensor(input, in_width * in_height, depth, VX_DF_IMAGE_U16);
            borderMode->constant_value.S16 = 0;
        }

        if (outputFormat == VX_TYPE_INT8)
        {
            imgOutput = vxoTensor_CreateImageFromTensor(output, 1, depth, VX_DF_IMAGE_U8);
        }
        else if (outputFormat == VX_TYPE_FLOAT16)
        {
            imgOutput = vxoTensor_CreateImageFromTensor(output, 1, depth, VX_DF_IMAGE_U16);
        }

        parameters[0] = (vx_reference)imgInput;
        parameters[2] = (vx_reference)imgOutput;
    }
    else
    {
        borderMode->mode = VX_BORDER_REPLICATE;
    }


    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[2] = {NULL, NULL};
        char programSources_AvgPoolInt8_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         \n\
         _viv_uniform VXC_512Bits uniInt8AddInt8_16x1; \n\
         _viv_uniform VXC_512Bits uniFp16AddInt8_4x4; \n\
        _viv_uniform VXC_512Bits uniFp16AddInt8Hi_4x4; \n\
        _viv_uniform VXC_512Bits uniFp16Sum4Line_4x4; \n\
        _viv_uniform VXC_512Bits uniFp16Sum2Line_4x4; \n\
        _viv_uniform VXC_512Bits uniFp16Swap; \n\
        _viv_uniform VXC_512Bits uniFp16Sum1Line_4x4; \n\
        _viv_uniform VXC_512Bits uniDotScale_4x4; \n\
        _viv_uniform float scaleIn_kernel13; \n\
        _viv_uniform VXC_512Bits uniFp16AddFp16_8x2; \n\
        _viv_uniform float scale7x7_FP16_INT8; \n\
        _viv_uniform VXC_512Bits uniInt8AddInt8Lo_8x4; \n\
        _viv_uniform VXC_512Bits uniInt8AddInt8Hi_8x4; \n\
        _viv_uniform VXC_512Bits uniS16AddS16_2x8; \n\
        _viv_uniform VXC_512Bits uniDotScale_2x8; \n\
        _viv_uniform float scaleInt8_Int8_7_1_0; \n\
         \n\
         __kernel void vxcPooling_AvgInt8toInt8ker7str1pad0 (\n\
         __read_only image2d_array_t   input, \n\
         int height, \n\
         __write_only image2d_array_t  output) \n\
        { \n\
            int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            int4 coord_out = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
             \n\
            vxc_char16 img_val0, img_val1, img_val2, img_val3; \n\
            vxc_short8    h0, h1, h2, h3, h4, h5; \n\
            vxc_short8  sum; \n\
            half scale; \n\
            _viv_asm(CONV, scale, scaleInt8_Int8_7_1_0); \n\
             \n\
            VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val1, input, coord_in, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val2, input, coord_in, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val3, input, coord_in, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP8x4(h0, img_val0, img_val0, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Lo_8x4); \n\
            VXC_DP8x4(h0, img_val0, img_val0, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Hi_8x4); \n\
            VXC_DP8x4(h1, img_val1, img_val1, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Lo_8x4); \n\
            VXC_DP8x4(h1, img_val1, img_val1, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Hi_8x4); \n\
             \n\
            VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 4), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val1, input, coord_in, VXC_5BITOFFSET_XY(0, 5), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP8x4(h2, img_val2, img_val2, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Lo_8x4); \n\
            VXC_DP8x4(h2, img_val2, img_val2, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Hi_8x4); \n\
            VXC_DP8x4(h3, img_val3, img_val3, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Lo_8x4); \n\
            VXC_DP8x4(h3, img_val3, img_val3, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Hi_8x4); \n\
             \n\
            coord_in.y += 6; \n\
            VXC_DP8x4(h4, img_val0, img_val0, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Lo_8x4); \n\
            VXC_DP8x4(h4, img_val0, img_val0, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Hi_8x4); \n\
            VXC_DP8x4(h5, img_val1, img_val1, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Lo_8x4); \n\
            VXC_DP8x4(h5, img_val1, img_val1, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Hi_8x4); \n\
            do \n\
            { \n\
                VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                coord_in.y++; \n\
                 \n\
                VXC_DP2x8(sum, h0, h1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16_2x8); \n\
                VXC_DP2x8(sum, sum, h2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16_2x8); \n\
                VXC_DP2x8(sum, sum, h3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16_2x8); \n\
                VXC_DP2x8(sum, sum, h4, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16_2x8); \n\
                VXC_DP2x8(sum, sum, h5, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16_2x8); \n\
                _viv_asm(COPY, h0, h1, 16); \n\
                _viv_asm(COPY, h1, h2, 16); \n\
                _viv_asm(COPY, h2, h3, 16); \n\
                _viv_asm(COPY, h3, h4, 16); \n\
                _viv_asm(COPY, h4, h5, 16); \n\
                VXC_DP8x4(h5, img_val0, img_val0, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Lo_8x4); \n\
                VXC_DP8x4(h5, img_val0, img_val0, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Hi_8x4); \n\
                VXC_DP2x8(sum, sum, h5, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16_2x8); \n\
                VXC_DP2x8(img_val1, sum, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 1), uniDotScale_2x8); \n\
                VXC_WriteImage2DArray(output, coord_out, img_val1, VXC_MODIFIER(0, 7, 0,VXC_RM_TowardZero, 0)); \n\
                coord_out.y ++; \n\
            } while (coord_in.y < height); \n\
        } \n\
        __kernel void vxcPooling_AvgFp16toInt8ker7str1pad0 (\n\
             __read_only image2d_array_t   input, \n\
                                           int height, \n\
             __write_only image2d_array_t  output) \n\
        { \n\
            int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            int4 coord_out = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
             \n\
            vxc_short8 img_val0, img_val1, img_val2, img_val3; \n\
            vxc_half8    h0, h1, h2, h3, h4, h5, h6; \n\
            vxc_float4   sum0, sum1, sum2, sum; \n\
            \n\
            VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, h0, img_val0, 16); \n\
            VXC_ReadImage2DArray(img_val1, input, coord_in, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, h1, img_val1, 16); \n\
            VXC_ReadImage2DArray(img_val2, input, coord_in, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, h2, img_val2, 16); \n\
            VXC_ReadImage2DArray(img_val3, input, coord_in, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, h3, img_val3, 16); \n\
             \n\
            VXC_DP8x2(sum0, h0, h0, VXC_MODIFIER(0, 1, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_8x2); \n\
            VXC_DP8x2(sum0, h1, h1, VXC_MODIFIER(2, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_8x2); \n\
             \n\
            VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 4), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, h0, img_val0, 16); \n\
            VXC_ReadImage2DArray(img_val1, input, coord_in, VXC_5BITOFFSET_XY(0, 5), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, h1, img_val1, 16); \n\
             \n\
            VXC_DP8x2(sum1, h2, h2, VXC_MODIFIER(0, 1, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_8x2); \n\
            VXC_DP8x2(sum1, h3, h3, VXC_MODIFIER(2, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_8x2); \n\
             \n\
            coord_in.y += 6; \n\
            VXC_DP8x2(sum2, h0, h0, VXC_MODIFIER(0, 1, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_8x2); \n\
            VXC_DP8x2(sum2, h1, h1, VXC_MODIFIER(2, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_8x2); \n\
             \n\
            do \n\
            { \n\
                VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, h0, img_val0, 16); \n\
                coord_in.y++; \n\
                sum.xy = sum0.xy + sum0.zw + sum1.xy + sum1.zw + sum2.xy + sum2.zw; \n\
                sum0.xy = sum0.zw; \n\
                sum0.zw = sum1.xy; \n\
                sum1.xy = sum1.zw; \n\
                sum1.zw = sum2.xy; \n\
                sum2.xy = sum2.zw; \n\
                VXC_DP8x2(sum2, h0, h0, VXC_MODIFIER(2, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_8x2); \n\
                sum.xy += sum2.zw; \n\
                sum.xy *= scale7x7_FP16_INT8; \n\
                sum.xy = round(sum.xy); \n\
                vxc_char4 val = convert_char4_sat(sum); \n\
                VXC_WriteImage2DArray(output, coord_out, val, VXC_MODIFIER(0, 1, 0,VXC_RM_TowardZero, 0)); \n\
                coord_out.y ++; \n\
            } while (coord_in.y < height); \n\
        } \n\
         \n\
         __kernel void vxcPooling_AvgInt8toFp16ker13str1pad0(\n\
         __read_only image2d_array_t   input, \n\
         int height, \n\
         __write_only image2d_array_t  output) \n\
         {\n\
            int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            int4 coord_out = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            \n\
            vxc_char16 img_val0, img_val1, img_val2, img_val3; \n\
            vxc_half8    h0, h1, h2, h3, h4, h5, h6; \n\
            vxc_half4   sum; \n\
            half scale; \n\
            _viv_asm(CONV, scale, scaleIn_kernel13); \n\
             \n\
            VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val1, input, coord_in, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val2, input, coord_in, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val3, input, coord_in, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP16x1(h0, img_val0, img_val0, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h0, h0, img_val0, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8_4x4); \n\
            VXC_DP16x1(h0, img_val1, img_val1, VXC_MODIFIER(4, 4, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h0, h0, img_val1, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8Hi_4x4); \n\
             \n\
            VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 4), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val1, input, coord_in, VXC_5BITOFFSET_XY(0, 5), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP16x1(h1, img_val2, img_val2, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h1, h1, img_val2, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8_4x4); \n\
            VXC_DP16x1(h1, img_val3, img_val3, VXC_MODIFIER(4, 4, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h1, h1, img_val3, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8Hi_4x4); \n\
             \n\
            VXC_ReadImage2DArray(img_val2, input, coord_in, VXC_5BITOFFSET_XY(0, 6), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val3, input, coord_in, VXC_5BITOFFSET_XY(0, 7), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP16x1(h2, img_val0, img_val0, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h2, h2, img_val0, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8_4x4); \n\
            VXC_DP16x1(h2, img_val1, img_val1, VXC_MODIFIER(4, 4, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h2, h2, img_val1, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8Hi_4x4); \n\
             \n\
            VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 8), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val1, input, coord_in, VXC_5BITOFFSET_XY(0, 9), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP16x1(h3, img_val2, img_val2, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h3, h3, img_val2, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8_4x4); \n\
            VXC_DP16x1(h3, img_val3, img_val3, VXC_MODIFIER(4, 4, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h3, h3, img_val3, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8Hi_4x4); \n\
             \n\
            VXC_ReadImage2DArray(img_val2, input, coord_in, VXC_5BITOFFSET_XY(0, 10), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(img_val3, input, coord_in, VXC_5BITOFFSET_XY(0, 11), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            \n\
            VXC_DP16x1(h4, img_val0, img_val0, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h4, h4, img_val0, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8_4x4); \n\
            VXC_DP16x1(h4, img_val1, img_val1, VXC_MODIFIER(4, 4, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h4, h4, img_val1, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8Hi_4x4); \n\
             \n\
            VXC_DP16x1(h5, img_val2, img_val2, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h5, h5, img_val2, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8_4x4); \n\
            VXC_DP16x1(h5, img_val3, img_val3, VXC_MODIFIER(4, 4, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
            VXC_DP4x4(h5, h5, img_val3, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8Hi_4x4); \n\
             \n\
            coord_in.y += 12; \n\
            do \n\
            { \n\
                VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                coord_in.y++; \n\
                 \n\
                VXC_DP4x4(sum, h0, h1, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Sum4Line_4x4); \n\
                VXC_DP4x4(sum, sum, h2, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Sum2Line_4x4); \n\
                VXC_DP4x4(sum, sum, h3, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Sum2Line_4x4); \n\
                VXC_DP4x4(sum, sum, h4, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Sum2Line_4x4); \n\
                VXC_DP4x4(sum, sum, h5, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Sum2Line_4x4); \n\
                 \n\
                VXC_DP2x8(h0, h0, h1, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Swap); \n\
                VXC_DP2x8(h1, h1, h2, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Swap); \n\
                VXC_DP2x8(h2, h2, h3, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Swap); \n\
                VXC_DP2x8(h3, h3, h4, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Swap); \n\
                VXC_DP2x8(h4, h4, h5, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Swap); \n\
                 \n\
                VXC_DP16x1(h6, img_val0, img_val0, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_16x1); \n\
                VXC_DP4x4(h6, h6, img_val0, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddInt8_4x4); \n\
                 \n\
                VXC_DP4x4(sum, sum, h6, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Sum1Line_4x4); \n\
                VXC_DP4x4(sum, sum, scale, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardInf, 1), uniDotScale_4x4); \n\
                vxc_short4 vec; \n\
                _viv_asm(COPY, vec, sum, 8); \n\
                VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0,VXC_RM_TowardZero, 0)); \n\
                coord_out.y ++; \n\
                 \n\
                VXC_DP2x8(h5, h5, h6, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Swap); \n\
            } while (coord_in.y < height); \n\
         }\n"
        };

        char programSources_AvgPoolInt8_1[] =
        {"\n\
         \n\
         _viv_uniform VXC_512Bits uniInt8AddInt8Kernel6Lo_8x4; \n\
         _viv_uniform VXC_512Bits uniInt8AddInt8Kernel6Hi_8x4; \n\
         _viv_uniform VXC_512Bits uniS16AddS16Kernel6_2x8; \n\
         _viv_uniform VXC_512Bits uniS16DotFP16_2x8; \n\
         _viv_uniform float scaleInt8_FP16_6_1_0; \n\
         __kernel void vxcPooling_AvgInt8toFp16ker6str1pad0 (\n\
         __read_only image2d_array_t   input, \n\
         int height, \n\
         __write_only image2d_array_t  output) \n\
         { \n\
         int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
         int4 coord_out = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
         \n\
         vxc_char16 img_val0, img_val1, img_val2, img_val3; \n\
         vxc_short8 h0, h1, h2, h3, h4; \n\
         vxc_short8 sum; \n\
         half scale; \n\
         _viv_asm(CONV, scale, scaleInt8_FP16_6_1_0); \n\
         \n\
         VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
         VXC_ReadImage2DArray(img_val1, input, coord_in, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
         VXC_ReadImage2DArray(img_val2, input, coord_in, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
         VXC_ReadImage2DArray(img_val3, input, coord_in, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
         \n\
         VXC_DP8x4(h0, img_val0, img_val0, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Lo_8x4); \n\
         VXC_DP8x4(h0, img_val0, img_val0, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Hi_8x4); \n\
         VXC_ReadImage2DArray(img_val0, input, coord_in, VXC_5BITOFFSET_XY(0, 4), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
         coord_in.y += 5; \n\
         VXC_DP8x4(h1, img_val1, img_val1, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Lo_8x4); \n\
         VXC_DP8x4(h1, img_val1, img_val1, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Hi_8x4); \n\
         VXC_DP8x4(h2, img_val2, img_val2, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Lo_8x4); \n\
         VXC_DP8x4(h2, img_val2, img_val2, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Hi_8x4); \n\
         VXC_DP8x4(h3, img_val3, img_val3, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Lo_8x4); \n\
         VXC_DP8x4(h3, img_val3, img_val3, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Hi_8x4); \n\
         VXC_DP8x4(h4, img_val0, img_val0, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Lo_8x4); \n\
         VXC_DP8x4(h4, img_val0, img_val0, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Hi_8x4); \n\
         do \n\
         { \n\
         VXC_ReadImage2DArray(img_val1, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
         coord_in.y++; \n\
         \n\
         VXC_DP2x8(sum, h0, h1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16Kernel6_2x8); \n\
         VXC_DP2x8(sum, sum, h2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16Kernel6_2x8); \n\
         VXC_DP2x8(sum, sum, h3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16Kernel6_2x8); \n\
         VXC_DP2x8(sum, sum, h4, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16Kernel6_2x8); \n\
         \n\
         _viv_asm(COPY, h0, h1, 16); \n\
         _viv_asm(COPY, h1, h2, 16); \n\
         _viv_asm(COPY, h2, h3, 16); \n\
         _viv_asm(COPY, h3, h4, 16); \n\
         \n\
         VXC_DP8x4(h4, img_val1, img_val1, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Lo_8x4); \n\
         VXC_DP8x4(h4, img_val1, img_val1, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8Kernel6Hi_8x4); \n\
         \n\
         vxc_half8 dst; \n\
         VXC_DP2x8(sum, sum, h4, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniS16AddS16Kernel6_2x8);  \n\
         VXC_DP2x8(dst, sum, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 1), uniS16DotFP16_2x8); \n\
         \n\
         vxc_short8 vec; \n\
         _viv_asm(COPY, vec, dst, 16); \n\
         VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0,VXC_RM_TowardZero, 0)); \n\
         coord_out.y ++; \n\
         } while (coord_in.y < height); \n\
         }\n"
        };


        char programSources_GlobalAvgPoolInt8_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         \n\
         _viv_uniform VXC_512Bits uniInt8AddInt8_32x1; \n\
         _viv_uniform VXC_512Bits uni2S16Dot1FP16_KernelLE8_16x1; \n\
         _viv_uniform VXC_512Bits uni6S16Dot1FP16_KernelLE8_16x1; \n\
         _viv_uniform VXC_512Bits uniS16AddS16_16x1; \n\
         _viv_uniform float scale_globalPool; \n\
          \n\
          __kernel void vxcPooling_GlobalAvgFp16toInt8KernelLE8 (\n\
          __read_only image2d_t   input, \n\
          int height, \n\
          __write_only image2d_t  output) \n\
          { \n\
          int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(0) + 16, get_global_id(0) + 48); \n\
          \n\
          vxc_short8 img_val0, img_val1, img_val2, img_val3; \n\
          vxc_short8 img_val4, img_val5, img_val6, img_val7; \n\
          vxc_half8  vec0, vec1, vec2, vec3; \n\
          vxc_half8  vec4, vec5, vec6, vec7; \n\
          vxc_float4  sum; \n\
          \n\
          VXC_ReadImage(img_val0, input, coord.zy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
          _viv_asm(COPY, vec0, img_val0, 16); \n\
          VXC_ReadImage(img_val1, input, coord.zy, VXC_5BITOFFSET_XY(-8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
          _viv_asm(COPY, vec1, img_val1, 16); \n\
          VXC_ReadImage(img_val2, input, coord.zy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
          _viv_asm(COPY, vec2, img_val2, 16); \n\
          VXC_ReadImage(img_val3, input, coord.zy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
          _viv_asm(COPY, vec3, img_val3, 16); \n\
          VXC_ReadImage(img_val4, input, coord.wy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
          _viv_asm(COPY, vec4, img_val4, 16); \n\
          VXC_ReadImage(img_val5, input, coord.wy, VXC_5BITOFFSET_XY(-8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
          _viv_asm(COPY, vec5, img_val5, 16); \n\
          VXC_ReadImage(img_val6, input, coord.wy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
          _viv_asm(COPY, vec6, img_val6, 16); \n\
          VXC_ReadImage(img_val3, input, coord.wy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
          _viv_asm(COPY, vec7, img_val7, 16); \n\
          \n\
          VXC_DP16x1(sum, vec0, vec1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniS16AddS16_16x1); \n\
          VXC_DP16x1(sum, vec2, vec3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniS16AddS16_16x1); \n\
          VXC_DP16x1(sum, vec4, vec5, VXC_MODIFIER(2, 2, 0, VXC_RM_TowardZero, 0), uniS16AddS16_16x1); \n\
          VXC_DP16x1(sum, vec6, vec7, VXC_MODIFIER(3, 3, 0, VXC_RM_TowardZero, 0), uniS16AddS16_16x1); \n\
          vxc_float4 scale = {scale_globalPool, scale_globalPool, scale_globalPool, scale_globalPool}; \n\
          sum.x = dot(sum, scale); \n\
          sum.x = round(sum.x); \n\
          char val = convert_char_sat(sum); \n\
          VXC_WriteImage(output, coord.xy, val, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0)); \n\
          } \n\
         __kernel void vxcPooling_GlobalAvgInt8toFp16KernelLE8 (\n\
         __read_only image2d_t   input, \n\
         int height, \n\
         __write_only image2d_t  output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(0) + 16, get_global_id(0) + 48); \n\
            \n\
            vxc_char16 img_val0, img_val1, img_val2, img_val3; \n\
            vxc_short8  sum; \n\
            half scale; \n\
            _viv_asm(CONV, scale, scale_globalPool); \n\
            \n\
            VXC_ReadImage(img_val0, input, coord.zy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val1, input, coord.zy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val2, input, coord.wy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val3, input, coord.wy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            \n\
            VXC_DP32x1(sum, img_val0, img_val1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_DP32x1(sum, img_val2, img_val3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_DP16x1(scale, sum, scale, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardInf, 1), uni2S16Dot1FP16_KernelLE8_16x1); \n\
            _viv_asm(COPY, sum, scale, 2); \n\
            VXC_WriteImage(output, coord.xy, sum, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0)); \n\
         } \n\
         __kernel void vxcPooling_GlobalAvgInt8toFp16KernelLE13 (\n\
         __read_only image2d_t   input, \n\
         int height, \n\
         __write_only image2d_t  output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(0) + 16, get_global_id(0) + 48); \n\
            \n\
            vxc_char16 img_val0, img_val1, img_val2, img_val3, img_val4, img_val5, img_val6, img_val7; \n\
            vxc_short8  sum; \n\
            half scale; \n\
            _viv_asm(CONV, scale, scale_globalPool); \n\
            \n\
            VXC_ReadImage(img_val0, input, coord.zy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val1, input, coord.zy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val2, input, coord.wy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val3, input, coord.wy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            coord.zw += 64; \n\
            VXC_ReadImage(img_val4, input, coord.zy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val5, input, coord.zy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val6, input, coord.wy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val7, input, coord.wy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            coord.zw += 64; \n\
            \n\
            VXC_DP32x1(sum, img_val0, img_val1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_DP32x1(sum, img_val2, img_val3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_ReadImage(img_val0, input, coord.zy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val1, input, coord.zy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val2, input, coord.wy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_DP32x1(sum, img_val4, img_val5, VXC_MODIFIER(2, 2, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_DP32x1(sum, img_val6, img_val7, VXC_MODIFIER(3, 3, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_DP32x1(sum, img_val0, img_val1, VXC_MODIFIER(4, 4, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_DP32x1(sum, img_val2, 0, VXC_MODIFIER(5, 5, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_DP16x1(scale, sum, scale, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardInf, 1), uni6S16Dot1FP16_KernelLE8_16x1); \n\
            _viv_asm(COPY, sum, scale, 2); \n\
            VXC_WriteImage(output, coord.xy, sum, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0)); \n\
         } \n\
         \n\
         __kernel void vxcPooling_GlobalAvgInt8toInt8KernelLE8 (\n\
         __read_only image2d_t   input, \n\
         int height, \n\
         __write_only image2d_t  output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(0) + 16, get_global_id(0) + 48); \n\
            \n\
            vxc_char16 img_val0, img_val1, img_val2, img_val3; \n\
            vxc_short8  sum; \n\
            half scale; \n\
            _viv_asm(CONV, scale, scale_globalPool); \n\
            \n\
            VXC_ReadImage(img_val0, input, coord.zy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val1, input, coord.zy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val2, input, coord.wy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val3, input, coord.wy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            \n\
            VXC_DP32x1(sum, img_val0, img_val1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_DP32x1(sum, img_val2, img_val3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_32x1); \n\
            VXC_DP16x1(img_val0, sum, scale, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardInf, 1), uni2S16Dot1FP16_KernelLE8_16x1); \n\
            VXC_WriteImage(output, coord, img_val0, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0)); \n\
         }\n"
        };

        if (((inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8))
            && globalPooling_flag)
        {
            programSources[0] = programSources_GlobalAvgPoolInt8_0;
            programSources[1] = programSources_GlobalAvgPoolInt8_0;
            programLength[0] = strlen(programSources_GlobalAvgPoolInt8_0);
            programLength[1] = strlen(programSources_GlobalAvgPoolInt8_0);
            program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, programLength);
        }
        else if ((inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8))
        {
            programSources[0] = programSources_AvgPoolInt8_0;
            programSources[1] = programSources_AvgPoolInt8_1;
            programLength[0] = strlen(programSources_AvgPoolInt8_0);
            programLength[1] = strlen(programSources_AvgPoolInt8_1);
            program = vxCreateProgramWithSource(context, 2, (const vx_char**)programSources, programLength);
        }
        else
        {
            gcmPRINT("data format not support %s line %d", __FUNCTION__, __LINE__);
            goto error;
        }
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcPooling", program, 3, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (globalPooling_flag && ((outputFormat == VX_TYPE_FLOAT16 && kernel_v <= 13) || kernel_v <= 8) )
    {
        vx_float32    scale_globalPool = div_scale * (1 / (float)(kernel_v * kernel_v));
        vx_uint32 uniInt8AddInt8_32x1[16] = {
            0xffffffff, 0xffffffff, // TCfg
            0x8a418820, 0xc5a92839, 0xca307b9a, 0x38bdab49, 0xffbbcdeb, // BinSelect
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uni2S16Dot1FP16_KernelLE8_16x1[16] = {
            0x00000005, // TCfg
            0x00000000, // ASelt
            0x00000010, 0x00000000, // ABin
            0x00000005, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uni6S16Dot1FP16_KernelLE8_16x1[16] = {
            0x00000555, // TCfg
            0x00000000, // ASelt
            0x00543210, 0x00000000, // ABin
            0x00000555, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniS16AddS16_16x1[16] = {
            0x55555555, // TCfg
            0x55550000, // ASelt
            0x76543210, 0x76543210, // ABin
            0xaaaaaaaa, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001 // Constant
        };

        if(outputFormat == VX_TYPE_FLOAT16 && kernel_v <= 8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_GlobalAvgInt8toFp16KernelLE8", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if(outputFormat == VX_TYPE_FLOAT16 && kernel_v <= 13)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_GlobalAvgInt8toFp16KernelLE13", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_GlobalAvgInt8toInt8KernelLE8", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_GlobalAvgFp16toInt8KernelLE8", borderMode);
            if (!shaderExecutable) goto error;
        }

        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniInt8AddInt8_32x1", 1, uniInt8AddInt8_32x1);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uni2S16Dot1FP16_KernelLE8_16x1", 1, uni2S16Dot1FP16_KernelLE8_16x1);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uni6S16Dot1FP16_KernelLE8_16x1", 1, uni6S16Dot1FP16_KernelLE8_16x1);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniS16AddS16_16x1", 1, uniS16AddS16_16x1);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scale_globalPool", 1, &scale_globalPool);
        if (status != VX_SUCCESS) goto error;

    }
    else if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16 && kernel_v == 13 && stride_v == 1 &&  pad_v == 0)
    {
        vx_float32    scaleIn_kernel13 = div_scale * (1 / (float)(13 * 13));
        vx_uint32 uniInt8AddInt8_16x1[16] = {
            0x0fffffff, // TCfg
            0x00000000, // ASelt
            0x87654321, 0x00edcba9, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniFp16AddInt8_4x4[16] = {
            0xe90909ad, // TCfg
            0x54040454, // ASelt
            0x00e0ed00, 0xf2100010, // ABin
            0x2a0a0aa2, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000001, 0x00010001, 0x00010001, 0x00000000, 0x00010001, 0x00000000, 0x00010001, 0x00000001 // Constant
        };
        vx_uint32 uniFp16Sum4Line_4x4[16] = {
            0x55555555, // TCfg
            0x50505050, // ASelt
            0x51514040, 0x73736262, // ABin
            0xaaaaaaaa, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001 // Constant
        };
        vx_uint32 uniFp16Sum2Line_4x4[16] = {
            0x15151515, // TCfg
            0x14141414, // ASelt
            0x05110400, 0x07330622, // ABin
            0x2a2a2a2a, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00010001, 0x00000001, 0x00010001, 0x00000001, 0x00010001, 0x00000001, 0x00010001, 0x00000001 // Constant
        };
        vx_uint32 uniFp16Swap[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x07060504, 0x03020100, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001 // Constant
        };
        vx_uint32 uniFp16Sum1Line_4x4[16] = {
            0x05050505, // TCfg
            0x04040404, // ASelt
            0x00110000, 0x00330022, // ABin
            0x0a0a0a0a, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00010001, 0x00000000, 0x00010001, 0x00000000, 0x00010001, 0x00000000, 0x00010001, 0x00000000 // Constant
        };
        vx_uint32 uniDotScale_4x4[16] = {
            0x01010101, // TCfg
            0x00000000, // ASelt
            0x00010000, 0x00030002, // ABin
            0x01010101, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniFp16AddInt8Hi_4x4[16] = {
            0xe90909ad, // TCfg
            0x54040454, // ASelt
            0x00e4ed04, 0xf2140014, // ABin
            0x2a0a0aa2, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000001, 0x00010001, 0x00010001, 0x00000000, 0x00010001, 0x00000000, 0x00010001, 0x00000001 // Constant
        };

        execution_parameters.globalWorkScale[0]  = 4;
        execution_parameters.globalWorkScale[1]  = out_height;
        execution_parameters.globalWorkScale[2]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgInt8toFp16ker13str1pad0", borderMode);
        if (!shaderExecutable) goto error;
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniInt8AddInt8_16x1", 1, uniInt8AddInt8_16x1);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniFp16AddInt8_4x4", 1, uniFp16AddInt8_4x4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniFp16Sum4Line_4x4", 1, uniFp16Sum4Line_4x4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniFp16Sum2Line_4x4", 1, uniFp16Sum2Line_4x4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniFp16Swap", 1, uniFp16Swap);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniFp16Sum1Line_4x4", 1, uniFp16Sum1Line_4x4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniDotScale_4x4", 1, uniDotScale_4x4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniFp16AddInt8Hi_4x4", 1, uniFp16AddInt8Hi_4x4);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleIn_kernel13", 1, &scaleIn_kernel13);
        if (status != VX_SUCCESS) goto error;
    }
    else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8 && kernel_v == 7 && stride_v == 1 &&  pad_v == 0 )
    {
        vx_float32    scale7x7_FP16_INT8 = div_scale * (1 / (float)(7 * 7));
        vx_uint32 uniFp16AddFp16_8x2[16] = {
            0x15551555, // TCfg
            0x00000000, // ASelt
            0x06543210, 0x07654321, // ABin
            0x2aaa2aaa, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00010001, 0x00010001, 0x00010001, 0x00000001, 0x00010001, 0x00010001, 0x00010001, 0x00000001 // Constant
        };

        execution_parameters.globalWorkScale[0]  = 2;
        execution_parameters.globalWorkScale[1]  = out_height;
        execution_parameters.globalWorkScale[2]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgFp16toInt8ker7str1pad0", borderMode);
        if (!shaderExecutable) goto error;
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniFp16AddFp16_8x2", 1, uniFp16AddFp16_8x2);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scale7x7_FP16_INT8", 1, &scale7x7_FP16_INT8);
        if (status != VX_SUCCESS) goto error;
    }
    else if(inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8 && kernel_v == 7 && stride_v == 1 &&  pad_v == 0 )
    {
        vx_float32    scale7x7_INT8_INT8 = div_scale * (1 / (float)(7 * 7));
        vx_uint32 uniInt8AddInt8Lo_8x4[16] = {
            0x3fff3fff, 0x3fff3fff, // TCfg
            0x8a418820, 0x520c4101, 0x906201cc, 0x83020e62, 0x02507314, // BinSelect
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniInt8AddInt8Hi_8x4[16] = {
            0x3fff3fff, 0x3fff3fff, // TCfg
            0x8a452507, 0x55a92801, 0x2d4901cc, 0x6a020e66, 0x025076b1, // BinSelect
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniS16AddS16_2x8[16] = {
            0x55555555, // TCfg
            0x44444444, // ASelt
            0x33221100, 0x77665544, // ABin
            0xaaaaaaaa, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001 // Constant
        };
        vx_uint32 uniDotScale_2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = out_height;
        execution_parameters.globalWorkScale[2]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgInt8toInt8ker7str1pad0", borderMode);
        if (!shaderExecutable) goto error;
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniInt8AddInt8Lo_8x4", 1, uniInt8AddInt8Lo_8x4);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniInt8AddInt8Hi_8x4", 1, uniInt8AddInt8Hi_8x4);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniS16AddS16_2x8", 1, uniS16AddS16_2x8);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniDotScale_2x8", 1, uniDotScale_2x8);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleInt8_Int8_7_1_0", 1, &scale7x7_INT8_INT8);
        if (status != VX_SUCCESS) goto error;
    }
    else if(inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16 && kernel_v == 6 && stride_v == 1 &&  pad_v == 0 )
    {
        vx_float32    scaleInt8_FP16_6_1_0 = div_scale * (1 / (float)(6 * 6));
        vx_uint32 uniS16AddS16Kernel6_2x8[16] = {
            0x55555555, // TCfg
            0x44444444, // ASelt
            0x33221100, 0x77665544, // ABin
            0xaaaaaaaa, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001, 0x00010001 // Constant
        };
        vx_uint32 uniInt8AddInt8Kernel6Lo_8x4[16] = {
            0x0fff0fff, 0x0fff0fff, // TCfg
            0x0a418820, 0x520c4100, 0x9062000c, 0x83000e62, 0x00107314, // BinSelect
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniInt8AddInt8Kernel6Hi_8x4[16] = {
            0x0fff0fff, 0x0fff0fff, // TCfg
            0x128398a4, 0x941cc500, 0xa0e60014, 0x070016a4, 0x0018b525, // BinSelect
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniS16DotFP16_2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };


        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = out_height;
        execution_parameters.globalWorkScale[2]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgInt8toFp16ker6str1pad0", borderMode);
        if (!shaderExecutable) goto error;
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniS16AddS16Kernel6_2x8", 1, uniS16AddS16Kernel6_2x8);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniInt8AddInt8Kernel6Lo_8x4", 1, uniInt8AddInt8Kernel6Lo_8x4);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniInt8AddInt8Kernel6Hi_8x4", 1, uniInt8AddInt8Kernel6Hi_8x4);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniS16DotFP16_2x8", 1, uniS16DotFP16_2x8);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleInt8_FP16_6_1_0", 1, &scaleInt8_FP16_6_1_0);
        if (status != VX_SUCCESS) goto error;
    }

    if(globalPooling_flag)
    {
        execution_parameters.workDim             = 2;
        execution_parameters.globalWorkScale[0]  = 1;
        execution_parameters.globalWorkScale[1]  = 1;
        execution_parameters.globalWorkScale[2]  = 0;
        execution_parameters.globalWorkOffset[0] = 0;
        execution_parameters.globalWorkOffset[1] = 0;
        execution_parameters.localWorkSize[0]    = 1;
        execution_parameters.localWorkSize[1]    = 8;
        execution_parameters.globalWorkSize[0]   = 1;
        execution_parameters.globalWorkSize[1]   = gcmALIGN((depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    }
    else
    {
        execution_parameters.workDim = 3;
        execution_parameters.globalWorkOffset[0] = 0;
        execution_parameters.globalWorkOffset[1] = 0;
        execution_parameters.globalWorkOffset[2] = 0;
        if (out_width > execution_parameters.globalWorkScale[0] * 8)
        {
            execution_parameters.localWorkSize[0]    = 8;
            execution_parameters.localWorkSize[1]    = 1;
            execution_parameters.localWorkSize[2]    = 1;
        }
        else if (out_width > execution_parameters.globalWorkScale[0] * 4)
        {
            execution_parameters.localWorkSize[0]    = 4;
            execution_parameters.localWorkSize[1]    = 1;
            execution_parameters.localWorkSize[2]    = 2;
        }
        else
        {
            execution_parameters.localWorkSize[0]    = 1;
            execution_parameters.localWorkSize[1]    = 1;
            execution_parameters.localWorkSize[2]    = 8;
        }
        execution_parameters.globalWorkSize[0]   = gcmALIGN((out_width  + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = 1;
        execution_parameters.globalWorkSize[2]   = gcmALIGN((depth      + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]);
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(in_heights) vxReleaseScalar(&in_heights);


    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(in_heights) vxReleaseScalar(&in_heights);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcMaxPooling****************************************************/
vxnne_shader_executable vxnneGetMaxPoolingShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               poolType,
    vx_scalar               stride_s,
    vx_scalar               poolSizeX,
    vx_scalar               poolSizeY,
    vx_scalar               poolPadX,
    vx_scalar               poolPadY,
    vx_scalar               rounding,
    vx_tensor               output)
{
    vx_size    programLength[2] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[10] = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, VX_NULL, VX_NULL, (vx_reference)stride_s, (vx_reference)poolSizeX, (vx_reference)poolPadX, (vx_reference)output};
    vx_enum   inputFormat       = input->tensorBuffer->dataFormat;
    vx_enum   outputFormat      = output->tensorBuffer->dataFormat;
    vx_uint32 in_width          = input->tensorBuffer->memory.dims[0][0];
    vx_uint32 in_height         = input->tensorBuffer->memory.dims[0][1];
    vx_uint32 depth             = input->tensorBuffer->memory.dims[0][2];
    vx_uint32 out_width         = output->tensorBuffer->memory.dims[0][0];
    vx_uint32 out_height        = output->tensorBuffer->memory.dims[0][1];
    vx_enum   poolType_v        = poolType->value->e;
    vx_uint32 stride_v          = stride_s->value->u32;
    vx_uint32 kernel_v          = poolSizeX->value->u32;
    vx_uint32 pad_v             = poolPadX->value->u32;
    vx_scalar in_widths         = vxCreateScalar(context, VX_TYPE_UINT32, &in_width);
    vx_scalar in_heights        = vxCreateScalar(context, VX_TYPE_UINT32, &in_height);
    vx_scalar depth_s           = vxCreateScalar(context, VX_TYPE_UINT32, &depth);
    vx_scalar out_widths        = vxCreateScalar(context, VX_TYPE_UINT32, &out_width);
    vx_scalar out_heights       = vxCreateScalar(context, VX_TYPE_UINT32, &out_height);

    vx_int8   input_fractionLengthValue  = input->tensorBuffer->fixedPointPos;
    vx_int8   output_fractionLengthValue = output->tensorBuffer->fixedPointPos;
    vx_int8   div_fractionLengthValue    = input_fractionLengthValue - output_fractionLengthValue;
    vx_float32 out_scale = 1.0f;
    vx_float32 div_scale = 1.0f;

    if ((inputFormat != VX_TYPE_FLOAT16 && inputFormat != VX_TYPE_INT8) || (outputFormat != VX_TYPE_FLOAT16 && outputFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or outputs format is not support");
        goto error;
    }
    if (poolType_v != VX_CONVOLUTIONAL_NETWORK_POOLING_MAX)
    {
        gcmPRINT("pool type not support");
        goto error;
    }

    parameters[1] = (vx_reference)in_widths;
    parameters[2] = (vx_reference)in_heights;
    parameters[3] = (vx_reference)depth_s;
    parameters[4] = (vx_reference)out_widths;
    parameters[5] = (vx_reference)out_heights;

    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkOffset[2] = 0;
    execution_parameters.globalWorkScale[0]  = ((8 - kernel_v + 2) / stride_v) * 2;
    execution_parameters.globalWorkScale[1]  = out_height;
    execution_parameters.globalWorkScale[2]  = 1;
    if ((2 * out_width) > execution_parameters.globalWorkScale[0] * 8)
    {
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.localWorkSize[2]    = 1;
    }
    else
    {
        execution_parameters.localWorkSize[0]    = 1;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.localWorkSize[2]    = 8;
    }
    execution_parameters.globalWorkSize[0]   = gcmALIGN((out_width  + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
    execution_parameters.globalWorkSize[1]   = gcmALIGN((out_height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    execution_parameters.globalWorkSize[2]   = gcmALIGN((depth      + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]);

    borderMode->mode = VX_BORDER_REPLICATE;
    if (output_fractionLengthValue >= 0)
    {
        out_scale = (vx_float32) (1 << output_fractionLengthValue);
    }
    else if (output_fractionLengthValue < 0)
    {
        out_scale = 1.0f / (vx_float32) (1 << -output_fractionLengthValue);
    }
    if (div_fractionLengthValue >= 0)
    {
        div_scale = 1.0f / (vx_float32) (1 << div_fractionLengthValue);
    }
    else if (div_fractionLengthValue < 0)
    {
        div_scale = (vx_float32) (1 << -div_fractionLengthValue);
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources_MaxPool[2] = {NULL, NULL};
        char programSources_MaxPoolFp16In[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            \n\
            _viv_uniform VXC_512Bits UniPackMaxPool2x8_fp16; \n\
            _viv_uniform VXC_512Bits UniPackFP16even_2x8; \n\
            _viv_uniform VXC_512Bits UniPackFP16odd_2x8; \n\
            _viv_uniform VXC_512Bits UniFP16Mul_dp2x8; \n\
            _viv_uniform float out_scale; \n\
            _viv_uniform int outputFormat1; \n\
            __kernel void vxcPooling_maxfp16ker3str2pad1(\n\
                     image2d_array_t   input, \n\
                     int               in_width, \n\
                     int               in_height, \n\
                     int               depth, \n\
                     int               out_width, \n\
                     int               out_height, \n\
                     int               stride_v, \n\
                     int               kernel_v, \n\
                     int               pad_v, \n\
                     image2d_array_t   output)\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                int offset_x = coord_in.x * stride_v - pad_v; \n\
                int4 posin = (int4)(offset_x, coord_in.y, coord_in.z, 0); \n\
                vxc_short8 img_reg1,img_reg2,img_reg3,img_reg4,img_reg5,img_reg6; \n\
                vxc_short8 s16_val0; \n\
                vxc_half8 fp16_val1, fp16_val2; \n\
                half out_scale_fp16; \n\
                vxc_char8 val_s8; \n\
                \n\
                VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));//head 2 row \n\
                VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg5, input, posin, VXC_5BITOFFSET_XY(6,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg6, input, posin, VXC_5BITOFFSET_XY(6,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                \n\
                int4 posout = (int4)(coord_in.x, coord_in.y, coord_in.z, 0); \n\
                vxc_short8 val_min = {0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff}; \n\
                _viv_asm(CONV, out_scale_fp16, out_scale); \n\
                \n\
                VXC_VertMax3(fp16_val1, img_reg2, img_reg3, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3(fp16_val1, fp16_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_VertMax3(fp16_val2, img_reg5, img_reg6, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3(fp16_val2, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0), UniPackMaxPool2x8_fp16); \n\
                if (outputFormat1 == 15) \n\
                {\n\
                    _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                    VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniFP16Mul_dp2x8); \n\
                    VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                int loop_count = out_height- 1; \n\
                for (int i = 1; i < loop_count; i++) \n\
                { \n\
                    img_reg1 = img_reg3; \n\
                    img_reg4 = img_reg6; \n\
                    posin.y += 2; \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg5, input, posin, VXC_5BITOFFSET_XY(6,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg6, input, posin, VXC_5BITOFFSET_XY(6,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    posout.y += 1; \n\
                    VXC_VertMax3(fp16_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3(fp16_val1, fp16_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3(fp16_val2, img_reg4, img_reg5, img_reg6, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3(fp16_val2, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0), UniPackMaxPool2x8_fp16); \n\
                    VXC_VertMax3(fp16_val1, fp16_val1, fp16_val1, val_min, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat1 == 15) \n\
                    {\n\
                        _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                        VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniFP16Mul_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                } \n\
                //the last 1 row \n\
                posout.y += 1; \n\
                VXC_HorzMax3(fp16_val1, img_reg3, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3(fp16_val2, img_reg6, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0), UniPackMaxPool2x8_fp16); \n\
                VXC_VertMax3(fp16_val1, fp16_val1, fp16_val1, val_min, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat1 == 15) \n\
                {\n\
                    _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                    VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniFP16Mul_dp2x8); \n\
                    VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            }\n\
            __kernel void vxcPooling_maxfp16ker2str2pad0 \n\
                (\n\
                     image2d_array_t   input, \n\
                     int               in_width, \n\
                     int               in_height, \n\
                     int               depth, \n\
                     int               out_width, \n\
                     int               out_height, \n\
                     int               stride_v, \n\
                     int               kernel_v, \n\
                     int               pad_v, \n\
                     image2d_array_t   output \n\
                )\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                int offset_x = coord_in.x * stride_v; \n\
                int4 posin = (int4)(offset_x, coord_in.y, coord_in.z, 0); \n\
                int4 posout = (int4)(coord_in.x, coord_in.y, coord_in.z, 0); \n\
                vxc_short8 img_reg1,img_reg2,img_reg3,img_reg4; \n\
                vxc_short8 s16_val0; \n\
                vxc_half8 fp16_val1, fp16_val2, fp16_val3; \n\
                half out_scale_fp16; \n\
                vxc_char8 val_s8; \n\
                \n\
                VXC_ReadImage2DArray(img_reg1, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));//head 2 row \n\
                VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(8,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg4, input, posin, VXC_5BITOFFSET_XY(8,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                vxc_short8 val_min = {0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff}; \n\
                _viv_asm(CONV, out_scale_fp16, out_scale); \n\
                for (int i = 0; i < out_height; i++) \n\
                {\n\
                    posin.y += 2; \n\
                    VXC_VertMax3(fp16_val1, img_reg1, img_reg2, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3(fp16_val2, img_reg3, img_reg4, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg1, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(8,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg4, input, posin, VXC_5BITOFFSET_XY(8,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_DP2x8(fp16_val3, fp16_val1, fp16_val2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16even_2x8); \n\
                    VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16odd_2x8); \n\
                    VXC_VertMax3(fp16_val1, fp16_val1, fp16_val1, fp16_val3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat1 == 15) \n\
                    {\n\
                        _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                        VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFP16Mul_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    posout.y += 1; \n\
                } \n\
            }\n\
            __kernel void vxcPooling_maxfp16ker3str2pad0 \n\
                (\n\
                     image2d_array_t   input, \n\
                     int               in_width, \n\
                     int               in_height, \n\
                     int               depth, \n\
                     int               out_width, \n\
                     int               out_height, \n\
                     int               stride_v, \n\
                     int               kernel_v, \n\
                     int               pad_v, \n\
                     image2d_array_t   output \n\
                )\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                int offset_x = coord_in.x * stride_v; \n\
                int4 posin = (int4)(offset_x, coord_in.y, coord_in.z, 0); \n\
                vxc_short8 img_reg1,img_reg2,img_reg3,img_reg4,img_reg5,img_reg6; \n\
                vxc_short8 s16_val0; \n\
                vxc_half8 fp16_val1, fp16_val2; \n\
                half out_scale_fp16; \n\
                vxc_char8 val_s8; \n\
                \n\
                VXC_ReadImage2DArray(img_reg1, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,2), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg4, input, posin, VXC_5BITOFFSET_XY(6,0), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg5, input, posin, VXC_5BITOFFSET_XY(6,1), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg6, input, posin, VXC_5BITOFFSET_XY(6,2), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                \n\
                int4 posout = (int4)(coord_in.x, coord_in.y, coord_in.z, 0); \n\
                vxc_short8 val_min = {0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff}; \n\
                _viv_asm(CONV, out_scale_fp16, out_scale); \n\
                \n\
                for (int i = 0; i < out_height; i++) \n\
                {\n\
                    VXC_VertMax3(fp16_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3(fp16_val2, img_reg4, img_reg5, img_reg6, VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    img_reg1 = img_reg3; \n\
                    img_reg4 = img_reg6; \n\
                    posin.y += 2; \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,2), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg5, input, posin, VXC_5BITOFFSET_XY(6,1), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg6, input, posin, VXC_5BITOFFSET_XY(6,2), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    \n\
                    VXC_HorzMax3(fp16_val1, fp16_val1, VXC_MODIFIER(0, 4, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3(fp16_val2, fp16_val2, VXC_MODIFIER(0, 4, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0), UniPackMaxPool2x8_fp16); \n\
                    VXC_VertMax3(fp16_val1, fp16_val1, fp16_val1, val_min, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat1 == 15) \n\
                    {\n\
                        _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                        VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniFP16Mul_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    posout.y += 1; \n\
                } \n\
            }\n"
        };

        char programSources_MaxPoolInt8In[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            \n\
            _viv_uniform VXC_512Bits UniMaxPoolS8xFp16_dp2x8; \n\
            _viv_uniform VXC_512Bits UniS8xFp16Packeven_dp2x8; \n\
            _viv_uniform VXC_512Bits UniS8xFp16Packodd_dp2x8; \n\
            _viv_uniform float div_scale; \n\
            _viv_uniform int outputFormat2; \n\
            __kernel void vxcPooling_MaxInt8ker2str2pad0(\n\
                     image2d_array_t   input, \n\
                     int               in_width, \n\
                     int               in_height, \n\
                     int               depth, \n\
                     int               out_width, \n\
                     int               out_height, \n\
                     int               stride_v, \n\
                     int               kernel_v, \n\
                     int               pad_v, \n\
                     image2d_array_t   output )\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                int offset_x = coord_in.x * stride_v; \n\
                int4 posin = (int4)(offset_x, coord_in.y, coord_in.z, 0); \n\
                int4 posout = (int4)(coord_in.x, coord_in.y, coord_in.z, 0); \n\
                vxc_char16 img_reg1,img_reg2; \n\
                vxc_char16 s8_val1, s8_val2; \n\
                half div_scale_fp16; \n\
                vxc_half8 val1_fp16, val2_fp16; \n\
                vxc_short8 val_s16; \n\
                \n\
                VXC_ReadImage2DArray(img_reg1, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                vxc_char16 val_min = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80}; \n\
                _viv_asm(CONV, div_scale_fp16, div_scale); \n\
                \n\
                for (int i = 0; i < out_height; i++) \n\
                {\n\
                    posin.y += 2; \n\
                    VXC_VertMax3(s8_val1, img_reg1, img_reg2, val_min, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg1, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0));\n\
                    \n\
                    if (outputFormat2 == 15) \n\
                    {\n\
                        VXC_DP2x8(val1_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16Packeven_dp2x8); \n\
                        VXC_DP2x8(val2_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16Packodd_dp2x8); \n\
                        VXC_VertMax3(val1_fp16, val1_fp16, val2_fp16, val1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                        _viv_asm(COPY, val_s16, val1_fp16, 16); \n\
                        VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(s8_val2, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16Packeven_dp2x8); \n\
                        VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16Packodd_dp2x8); \n\
                        s8_val1 = max(s8_val1, s8_val2); \n\
                        VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    } \n\
                    posout.y += 1; \n\
                }\n\
            }\n\
            __kernel void vxcPooling_MaxInt8ker3str2pad0(\n\
                     image2d_array_t   input, \n\
                     int               in_width, \n\
                     int               in_height, \n\
                     int               depth, \n\
                     int               out_width, \n\
                     int               out_height, \n\
                     int               stride_v, \n\
                     int               kernel_v, \n\
                     int               pad_v, \n\
                     image2d_array_t   output )\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                int offset_x = coord_in.x * stride_v; \n\
                int4 posin = (int4)(offset_x, coord_in.y, coord_in.z, 0); \n\
                vxc_char16 img_reg1,img_reg2,img_reg3; \n\
                half div_scale_fp16; \n\
                vxc_char16 s8_val1; \n\
                vxc_half8 val_fp16; \n\
                vxc_short8 val_s16; \n\
                \n\
                VXC_ReadImage2DArray(img_reg1, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,2), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                \n\
                int4 posout = (int4)(coord_in.x, coord_in.y, coord_in.z, 0); \n\
                \n\
                _viv_asm(CONV, div_scale_fp16, div_scale); \n\
                vxc_char16 val_min = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80}; \n\
                \n\
                for (int i = 0; i < out_height; i++) \n\
                {\n\
                    VXC_VertMax3(s8_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    img_reg1 = img_reg3; \n\
                    posin.y += 2; \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,2), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    \n\
                    VXC_VertMax3(s8_val1, s8_val1, s8_val1, val_min, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3(s8_val1, s8_val1, VXC_MODIFIER(0, 10, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat2 == 15) \n\
                    {\n\
                        VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                        _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                        VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    posout.y += 1; \n\
                }\n\
            }\n\
            __kernel void vxcPooling_MaxInt8ker3str2pad1(\n\
                     image2d_array_t   input, \n\
                     int               in_width, \n\
                     int               in_height, \n\
                     int               depth, \n\
                     int               out_width, \n\
                     int               out_height, \n\
                     int               stride_v, \n\
                     int               kernel_v, \n\
                     int               pad_v, \n\
                     image2d_array_t   output )\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                int offset_x = coord_in.x * stride_v - pad_v; \n\
                int4 posin = (int4)(offset_x, coord_in.y, coord_in.z, 0); \n\
                vxc_char16 img_reg1,img_reg2,img_reg3; \n\
                half div_scale_fp16; \n\
                vxc_char16 s8_val1; \n\
                vxc_half8 val_fp16; \n\
                vxc_short8 val_s16; \n\
                \n\
                VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                int4 posout = (int4)(coord_in.x, coord_in.y, coord_in.z, 0); \n\
                _viv_asm(CONV, div_scale_fp16, div_scale); \n\
                vxc_char16 val_min = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80}; \n\
                \n\
                VXC_VertMax3(s8_val1, img_reg2, img_reg3, val_min, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3(s8_val1, s8_val1, VXC_MODIFIER(0, 10, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat2 == 15) \n\
                {\n\
                    VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                    VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                \n\
                int loop_count = out_height- 1; \n\
                for (int i = 1; i < loop_count; i++) \n\
                {\n\
                    img_reg1 = img_reg3; \n\
                    posin.y += 2; \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    posout.y += 1; \n\
                    VXC_VertMax3(s8_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3(s8_val1, s8_val1, s8_val1, val_min, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3(s8_val1, s8_val1, VXC_MODIFIER(0, 10, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat2 == 15) \n\
                    {\n\
                        VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                        _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                        VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                }\n\
                //the last 1 row \n\
                posout.y += 1; \n\
                VXC_VertMax3(s8_val1, img_reg3, val_min, img_reg3, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3(s8_val1, s8_val1, VXC_MODIFIER(0, 10, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat2 == 15) \n\
                {\n\
                    VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                    VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            }\n"
        };

        programSources_MaxPool[0] = programSources_MaxPoolFp16In;
        programSources_MaxPool[1] = programSources_MaxPoolInt8In;
        programLength[0]          = strlen(programSources_MaxPoolFp16In);
        programLength[1]          = strlen(programSources_MaxPoolInt8In);
        program = vxCreateProgramWithSource(context, 2, (const vx_char**)programSources_MaxPool, programLength);
        status  = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcPooling", program, 10, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_FLOAT16)
    {
        vx_uint32 UniPackMaxPool2x8_fp16[16] = {
            0x00111111, // TCfg
            0x00111000, // ASelt
            0x00040200, 0x00000402, // ABin
            0x00222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 UniPackFP16even_2x8[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x06040200, 0x06040200, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
        };
        vx_uint32 UniPackFP16odd_2x8[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x07050301, 0x07050301, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
        };
        vx_uint32 UniFP16Mul_dp2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        if (kernel_v == 3 && stride_v == 2 &&  pad_v == 1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_maxfp16ker3str2pad1", borderMode);
            if (!shaderExecutable) goto error;
            status = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniPackMaxPool2x8_fp16", 1, UniPackMaxPool2x8_fp16);
            if (status != VX_SUCCESS) goto error;
        }
        else if (kernel_v == 2 && stride_v == 2 &&  pad_v == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_maxfp16ker2str2pad0", borderMode);
            if (!shaderExecutable) goto error;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniPackFP16even_2x8", 1, UniPackFP16even_2x8);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniPackFP16odd_2x8", 1, UniPackFP16odd_2x8);
            if (status != VX_SUCCESS) goto error;
        }
        else if (kernel_v == 3 && stride_v == 2 &&  pad_v == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_maxfp16ker3str2pad0", borderMode);
            if (!shaderExecutable) goto error;
            status = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniPackMaxPool2x8_fp16", 1, UniPackMaxPool2x8_fp16);
            if (status != VX_SUCCESS) goto error;
        }
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFP16Mul_dp2x8", 1, UniFP16Mul_dp2x8);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "out_scale", 1, &out_scale);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outputFormat1", 1, &outputFormat);
        if (status != VX_SUCCESS) goto error;
    }
    else
    {
        vx_uint32 UniMaxPoolS8xFp16_dp2x8[16] = {
            0x00111111, // TCfg
            0x00000000, // ASelt
            0x06040200, 0x00000a08, // ABin
            0x00111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 UniS8xFp16Packeven_dp2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x06040200, 0x0e0c0a08, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 UniS8xFp16Packodd_dp2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x07050301, 0x0f0d0b09, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        if (kernel_v == 2 && stride_v == 2 &&  pad_v == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_MaxInt8ker2str2pad0", borderMode);
            if (!shaderExecutable) goto error;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniS8xFp16Packeven_dp2x8", 1, UniS8xFp16Packeven_dp2x8);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniS8xFp16Packodd_dp2x8", 1, UniS8xFp16Packodd_dp2x8);
            if (status != VX_SUCCESS) goto error;
        }
        else if (kernel_v == 3 && stride_v == 2 &&  pad_v == 0)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_MaxInt8ker3str2pad0", borderMode);
            if (!shaderExecutable) goto error;
            status = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniMaxPoolS8xFp16_dp2x8", 1, UniMaxPoolS8xFp16_dp2x8);
            if (status != VX_SUCCESS) goto error;
        }
        else if (kernel_v == 3 && stride_v == 2 &&  pad_v == 1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_MaxInt8ker3str2pad1", borderMode);
            if (!shaderExecutable) goto error;
            status = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniMaxPoolS8xFp16_dp2x8", 1, UniMaxPoolS8xFp16_dp2x8);
            if (status != VX_SUCCESS) goto error;
        }
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "div_scale", 1, &div_scale);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outputFormat2", 1, &outputFormat);
        if (status != VX_SUCCESS) goto error;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 10);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(in_widths) vxReleaseScalar(&in_widths);
    if(in_heights) vxReleaseScalar(&in_heights);
    if(depth_s) vxReleaseScalar(&depth_s);
    if(out_widths) vxReleaseScalar(&out_widths);
    if(out_heights) vxReleaseScalar(&out_heights);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(in_widths) vxReleaseScalar(&in_widths);
    if(in_heights) vxReleaseScalar(&in_heights);
    if(depth_s) vxReleaseScalar(&depth_s);
    if(out_widths) vxReleaseScalar(&out_widths);
    if(out_heights) vxReleaseScalar(&out_heights);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}


static vx_char * LoadSources(vx_char *filename, vx_size *programSize)
{
    FILE *pFile = NULL;
    vx_char *programSource = NULL;

    if (!programSize) return NULL;

    pFile = fopen(filename, "rb");

    if (pFile)
    {
        vx_int32 size = 0;
        /* obtain file size:*/
        fseek(pFile, 0, SEEK_END);
        *programSize = ftell(pFile);
        rewind(pFile);

        size = (int)(*programSize + 1);
        programSource = (char*)malloc(sizeof(char)*(size));
        if (programSource)
        {
            fread(programSource, sizeof(char), *programSize, pFile);
            programSource[*programSize] = '\0';
        }

        fclose(pFile);
    }

    return programSource;
}
/********vxcLrn****************************************************/
vxnne_shader_executable vxnneGetNormalizationShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               type_s,
    vx_scalar               norm_size_s,
    vx_scalar               alpha_s,
    vx_scalar               beta_s,
    vx_tensor               output)
{
    vx_size    programLength = 0;
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;

    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference    parameters[9]       = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, (vx_reference)type_s, (vx_reference)norm_size_s, (vx_reference)alpha_s, (vx_reference)beta_s, (vx_reference)output};
    vx_enum         inputFormat         = input->tensorBuffer->dataFormat;
    vx_enum         outputFormat        = output->tensorBuffer->dataFormat;
    vx_uint32       width               = input->tensorBuffer->memory.dims[0][0];
    vx_uint32       height              = input->tensorBuffer->memory.dims[0][1];
    vx_uint32       channel             = input->tensorBuffer->memory.dims[0][2];
    vx_enum         norm_type           = type_s->value->e;
    vx_scalar       width_s             = vxCreateScalar(context, VX_TYPE_UINT32, &width);
    vx_scalar       height_s            = vxCreateScalar(context, VX_TYPE_UINT32, &height);
    vx_scalar       channel_s           = vxCreateScalar(context, VX_TYPE_UINT32, &channel);
    vx_float32      in_scale            = 1.0f;
    vx_float32      out_scale           = 1.0f;
    vx_int8         input_fractionLengthValue  = input->tensorBuffer->fixedPointPos;
    vx_int8         output_fractionLengthValue = output->tensorBuffer->fixedPointPos;
    vx_uint32 UniS8xFp16toFp16_dp2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
    vx_uint32 UniFp16xFp16toS8_dp2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

    if ((inputFormat != VX_TYPE_FLOAT16 && inputFormat != VX_TYPE_INT8) || (outputFormat != VX_TYPE_FLOAT16 && outputFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or output's format is not support");
        goto error;
    }
    if ((norm_type != VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP) && (norm_type != VX_CONVOLUTIONAL_NETWORK_NORM_ACROSS_MAPS))
    {
        gcmPRINT("the normalization type is not support");
        goto error;
    }

    parameters[1] = (vx_reference)width_s;
    parameters[2] = (vx_reference)height_s;
    parameters[3] = (vx_reference)channel_s;

    borderMode->mode = VX_BORDER_CONSTANT;

    if (inputFormat == VX_TYPE_INT8)
    {
        borderMode->constant_value.U8 = 0;
    }
    else
    {
        borderMode->constant_value.S16 = 0;
    }
    if (input_fractionLengthValue >= 0)
    {
        in_scale = 1.0f / (vx_float32) (1 << input_fractionLengthValue);
    }
    else
    {
        in_scale = (vx_float32) (1 << -input_fractionLengthValue);
    }
    if (output_fractionLengthValue >= 0)
    {
        out_scale = (vx_float32) (1 << output_fractionLengthValue);
    }
    else
    {
        out_scale = 1.0f / (vx_float32) (1 << -output_fractionLengthValue);
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);
    if (!kernel)
    {
        /* register an shader kernel */
        char * LrnSameMap_programSources[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            \n\
            _viv_uniform VXC_512Bits dp_fp16_1;\n\
            _viv_uniform VXC_512Bits UniFp16xFp16PackLo4_dp4x4;\n\
            _viv_uniform VXC_512Bits UniS8xFp16toFp16_dp2x8;\n\
            _viv_uniform VXC_512Bits UniFp16xFp16toS8_dp2x8;\n\
            _viv_uniform float in_scale;\n\
            _viv_uniform float out_scale;\n\
            _viv_uniform int outputFormat; \n\
            \n\
            __kernel void vxcNormalization_SameMapFp16In(\n\
                __read_only  image2d_array_t   input, \n\
                             int               width, \n\
                             int               height, \n\
                             int               channel, \n\
                             int               type, \n\
                             int               norm_size, \n\
                             float             alpha, \n\
                             float             beta, \n\
                __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                vxc_short8 val1; \n\
                vxc_short8 val2; \n\
                vxc_short8 val_s16; \n\
                vxc_char8  val_s8; \n\
                vxc_float4 sum0; \n\
                vxc_float4 sum1; \n\
                vxc_float4 sum2; \n\
                vxc_float4 sum; \n\
                float4 alphadiv9; \n\
                float4 shift4 = (float)1; \n\
                \n\
                vxc_half8  val0_h; \n\
                vxc_half8  val1_h; \n\
                vxc_half8  val2_h; \n\
                vxc_half8  val_fp16; \n\
                half4 convfp16; \n\
                half out_scale_fp16; \n\
                int loop_height; \n\
                \n\
                VXC_ReadImage2DArray(val1, input, coord_in, VXC_5BITOFFSET_XY(-1,0), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0));//first row  \n\
                VXC_ReadImage2DArray(val2, input, coord_in, VXC_5BITOFFSET_XY(-1,1), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0));//second row \n\
                loop_height = height-2; \n\
                alphadiv9 = (float4)alpha/9; \n\
                _viv_asm(CONV, out_scale_fp16, out_scale); \n\
                \n\
                _viv_asm(COPY, val1_h, val1, 16); \n\
                _viv_asm(COPY, val2_h, val2, 16); \n\
                VXC_DP4x4(sum1, val1_h, val1_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), dp_fp16_1); \n\
                VXC_DP4x4(sum2, val2_h, val2_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), dp_fp16_1); \n\
                \n\
                sum = sum1 + sum2; \n\
                sum = mad(sum, alphadiv9, shift4); \n\
                sum = sum * sum * sum; \n\
                sum = sqrt(sum); \n\
                sum = rsqrt(sum); \n\
                \n\
                do \n\
                { \n\
                    _viv_asm(CONV, convfp16, sum); \n\
                    VXC_DP4x4(val_fp16, val1_h, convfp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), UniFp16xFp16PackLo4_dp4x4); \n\
                    _viv_asm(COPY, val1_h, val2, 16); \n\
                    VXC_ReadImage2DArray(val2, input, coord_in, VXC_5BITOFFSET_XY(-1,2), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat == 15) \n\
                    {\n\
                        _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                        VXC_WriteImage2DArray(output, coord_in, val_s16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                        VXC_WriteImage2DArray(output, coord_in, val_s8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    \n\
                    sum0 = sum1; \n\
                    sum1 = sum2; \n\
                    coord_in.y = coord_in.y + 1; \n\
                    \n\
                    _viv_asm(COPY, val2_h, val2, 16); \n\
                    VXC_DP4x4(sum2, val2_h, val2_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), dp_fp16_1); \n\
                    sum = sum0 + sum1 + sum2; \n\
                    sum = mad(sum, alphadiv9, shift4); \n\
                    sum = sum * sum * sum; \n\
                    sum = sqrt(sum); \n\
                    sum = rsqrt(sum); \n\
                    loop_height--; \n\
                }while(loop_height > 0); \n\
                \n\
                _viv_asm(CONV, convfp16, sum); \n\
                VXC_DP4x4(val_fp16, val1_h, convfp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), UniFp16xFp16PackLo4_dp4x4); \n\
                if (outputFormat == 15)\n\
                {\n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, coord_in, val_s16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                    VXC_WriteImage2DArray(output, coord_in, val_s8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                \n\
                // do last line \n\
                coord_in.y = coord_in.y + 1; \n\
                sum = sum1 + sum2; \n\
                sum = mad(sum, alphadiv9, shift4); \n\
                sum = sum * sum * sum; \n\
                sum = sqrt(sum); \n\
                sum = rsqrt(sum); \n\
                _viv_asm(CONV, convfp16, sum); \n\
                VXC_DP4x4(val_fp16, val2_h, convfp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), UniFp16xFp16PackLo4_dp4x4); \n\
                if (outputFormat == 15) \n\
                {\n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, coord_in, val_s16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                    VXC_WriteImage2DArray(output, coord_in, val_s8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            } \n\
           __kernel void vxcNormalization_SameMapInt8In(\n\
                __read_only  image2d_array_t   input, \n\
                             int               width, \n\
                             int               height, \n\
                             int               channel, \n\
                             int               type, \n\
                             int               norm_size, \n\
                             float             alpha, \n\
                             float             beta, \n\
                __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                vxc_char8 val1; \n\
                vxc_char8 val2; \n\
                vxc_char8 val_s8; \n\
                vxc_short8 val_s16; \n\
                vxc_float4 sum0; \n\
                vxc_float4 sum1; \n\
                vxc_float4 sum2; \n\
                vxc_float4 sum; \n\
                float4 alphadiv9; \n\
                float4 shift4 = (float)1; \n\
                \n\
                vxc_half8  val0_h; \n\
                vxc_half8  val1_h; \n\
                vxc_half8  val2_h; \n\
                vxc_half8  val_fp16; \n\
                half4 convfp16; \n\
                half in_scale_fp16; \n\
                half out_scale_fp16; \n\
                int loop_height; \n\
                \n\
                VXC_ReadImage2DArray(val1, input, coord_in, VXC_5BITOFFSET_XY(-1,0), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0));//first row \n\
                VXC_ReadImage2DArray(val2, input, coord_in, VXC_5BITOFFSET_XY(-1,1), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0));//second row \n\
                loop_height = height-2; \n\
                alphadiv9 = (float4)alpha/9; \n\
                \n\
                _viv_asm(CONV, in_scale_fp16, in_scale); \n\
                _viv_asm(CONV, out_scale_fp16, out_scale); \n\
                VXC_DP2x8(val1_h, val1, in_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                VXC_DP2x8(val2_h, val2, in_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                VXC_DP4x4(sum1, val1_h, val1_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), dp_fp16_1); \n\
                VXC_DP4x4(sum2, val2_h, val2_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), dp_fp16_1); \n\
                \n\
                sum = sum1 + sum2; \n\
                sum = mad(sum, alphadiv9, shift4); \n\
                sum = sum * sum * sum; \n\
                sum = sqrt(sum); \n\
                sum = rsqrt(sum); \n\
                \n\
                do \n\
                {\n\
                    _viv_asm(CONV, convfp16, sum); \n\
                    VXC_DP4x4(val_fp16, val1_h, convfp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), UniFp16xFp16PackLo4_dp4x4); \n\
                    VXC_DP2x8(val1_h, val2, in_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                    VXC_ReadImage2DArray(val2, input, coord_in, VXC_5BITOFFSET_XY(-1,2), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat == 15) \n\
                    {\n\
                        _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                        VXC_WriteImage2DArray(output, coord_in, val_s16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                        VXC_WriteImage2DArray(output, coord_in, val_s8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    \n\
                    sum0 = sum1; \n\
                    sum1 = sum2; \n\
                    coord_in.y = coord_in.y + 1; \n\
                    \n\
                    VXC_DP2x8(val2_h, val2, in_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                    VXC_DP4x4(sum2, val2_h, val2_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), dp_fp16_1); \n\
                    sum = sum0 + sum1 + sum2; \n\
                    sum = mad(sum, alphadiv9, shift4); \n\
                    sum = sum * sum * sum; \n\
                    sum = sqrt(sum); \n\
                    sum = rsqrt(sum);\n\
                    loop_height--; \n\
                }while(loop_height > 0); \n\
                \n\
                _viv_asm(CONV, convfp16, sum); \n\
                VXC_DP4x4(val_fp16, val1_h, convfp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), UniFp16xFp16PackLo4_dp4x4); \n\
                if (outputFormat == 15) \n\
                {\n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, coord_in, val_s16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                    VXC_WriteImage2DArray(output, coord_in, val_s8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                \n\
                // do last line \n\
                coord_in.y = coord_in.y + 1; \n\
                sum = sum1 + sum2; \n\
                sum = mad(sum, alphadiv9, shift4); \n\
                sum = sum * sum * sum; \n\
                sum = sqrt(sum); \n\
                sum = rsqrt(sum); \n\
                _viv_asm(CONV, convfp16, sum); \n\
                VXC_DP4x4(val_fp16, val2_h, convfp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), UniFp16xFp16PackLo4_dp4x4); \n\
                if (outputFormat == 15) \n\
                {\n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, coord_in, val_s16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                    VXC_WriteImage2DArray(output, coord_in, val_s8, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            }\n"
        };
        char * LrnAcrossMaps_programSources[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            \n\
            _viv_uniform VXC_512Bits Uni4x4_SquareSubLo4;\n\
            _viv_uniform VXC_512Bits Uni4x4_SquareSubHi4;\n\
            _viv_uniform VXC_512Bits Uni4x4_Fp16xFp16UnpackLo4;\n\
            _viv_uniform VXC_512Bits Uni4x4_Fp16xFp16UnpackHi4;\n\
            _viv_uniform VXC_512Bits Uni2x8_CopyHalf8;\n\
            _viv_uniform VXC_512Bits UniS8xFp16toFp16_dp2x8;\n\
            _viv_uniform VXC_512Bits UniFp16xFp16toS8_dp2x8;\n\
            _viv_uniform float in_scale;\n\
            _viv_uniform float out_scale;\n\
            _viv_uniform int outputFormat;\n\
            \n\
            __kernel void vxcNormalization_AcrossMapsFp16In(\n\
                __read_only  image2d_array_t   input,\n\
                             int               width,\n\
                             int               height,\n\
                             int               channel,\n\
                             int               type,\n\
                             int               norm_size,\n\
                             float             alpha,\n\
                             float             beta,\n\
                __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
                int4 coord_out = coord_in;\n\
                vxc_short8 val0 = {0};\n\
                vxc_short8 val1 = {0};\n\
                vxc_short8 val2;\n\
                vxc_short8 val3;\n\
                vxc_short8 val4;\n\
                vxc_short8 val5;\n\
                vxc_char8 val_s8;\n\
                \n\
                vxc_float4 sum1; \n\
                vxc_float4 sum2;\n\
                vxc_float4 sum_lo;\n\
                vxc_float4 sum_hi;\n\
                float4 alpha_div_nsz = (float4)(alpha/norm_size); \n\
                float4 shift4  = (float4)1;\n\
                \n\
                vxc_half8  val_sub_h;\n\
                vxc_half8  val0_h;\n\
                vxc_half8  val1_h; \n\
                vxc_half8  val2_h ;\n\
                vxc_half8  val3_h; \n\
                vxc_half8  val4_h;\n\
                vxc_half8  out_h;\n\
                half4 sum2_fp16;\n\
                half out_scale_fp16;\n\
                int loops = channel-1;\n\
                \n\
                coord_in.z = 0;\n\
                VXC_ReadImage2DArray(val2, input, coord_in, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                coord_in.z = 1;\n\
                VXC_ReadImage2DArray(val3, input, coord_in, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                coord_in.z = 2;\n\
                VXC_ReadImage2DArray(val4, input, coord_in, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                _viv_asm(COPY,val0_h,val0,16);\n\
                _viv_asm(COPY,val1_h,val1,16);\n\
                _viv_asm(COPY,val2_h,val2,16);\n\
                _viv_asm(COPY,val3_h,val3,16);\n\
                _viv_asm(COPY,val4_h,val4,16);\n\
                _viv_asm(CONV, out_scale_fp16, out_scale); \n\
                //low 4 elements\n\
                VXC_DP4x4(sum1,val2_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubLo4);\n\
                sum_lo+=sum1;\n\
                VXC_DP4x4(sum1,val3_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubLo4);\n\
                sum_lo+=sum1;\n\
                VXC_DP4x4(sum1,val4_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubLo4);\n\
                sum_lo+=sum1;\n\
                sum2 = mad(sum_lo,alpha_div_nsz,shift4);\n\
                sum2 = sum2 * sum2 * sum2;\n\
                sum2 = sqrt(sum2);\n\
                sum2 = rsqrt(sum2);\n\
                _viv_asm(CONV,sum2_fp16,sum2);\n\
                VXC_DP4x4(out_h,val2_h,sum2_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_Fp16xFp16UnpackLo4);\n\
                \n\
                //high4 elements\n\
                VXC_DP4x4(sum1,val2_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubHi4);\n\
                sum_hi+=sum1;\n\
                VXC_DP4x4(sum1,val3_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubHi4);\n\
                sum_hi+=sum1;\n\
                VXC_DP4x4(sum1,val4_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubHi4);\n\
                sum_hi+=sum1;\n\
                sum2 = mad(sum_hi,alpha_div_nsz,shift4);\n\
                sum2 = sum2 * sum2 * sum2;\n\
                sum2 = sqrt(sum2);\n\
                sum2 = rsqrt(sum2);\n\
                _viv_asm(CONV,sum2_fp16,sum2);    \n\
                VXC_DP4x4(out_h,val2_h,sum2_fp16, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 1),Uni4x4_Fp16xFp16UnpackHi4);\n\
                if (outputFormat == 15) \n\
                {\n\
                    _viv_asm(COPY, val5, out_h, 16); \n\
                    VXC_WriteImage2DArray(output, coord_out, val5, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, out_h, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                    VXC_WriteImage2DArray(output, coord_out, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                do\n\
                {\n\
                    coord_out.z++;\n\
                    coord_in.z++;\n\
                    if(coord_in.z<channel) \n\
                       VXC_ReadImage2DArray(val4, input, coord_in, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                    else \n\
                       val4 = 0;\n\
                    VXC_DP2x8(val_sub_h,val0_h,val0_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8);\n\
                    VXC_DP2x8(val0_h,val1_h,val1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8);\n\
                    VXC_DP2x8(val1_h,val2_h,val2_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8);\n\
                    VXC_DP2x8(val2_h,val3_h,val3_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8);\n\
                    VXC_DP2x8(val3_h,val4_h,val4_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8);\n\
                    \n\
                    _viv_asm(COPY,val4_h,val4,16);\n\
                    //low 4 elements\n\
                    VXC_DP4x4(sum1,val4_h,val_sub_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubLo4);\n\
                    sum_lo+=sum1;\n\
                    sum2 = mad(sum_lo,alpha_div_nsz,shift4);\n\
                    sum2 = sum2 * sum2 * sum2;\n\
                    sum2 = sqrt(sum2);\n\
                    sum2 = rsqrt(sum2);\n\
                    _viv_asm(CONV,sum2_fp16,sum2);\n\
                    VXC_DP4x4(out_h,val2_h,sum2_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0),Uni4x4_Fp16xFp16UnpackLo4);\n\
                    //high 4 elements\n\
                    VXC_DP4x4(sum1,val4_h,val_sub_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubHi4);\n\
                    sum_hi+=sum1;\n\
                    sum2 = mad(sum_hi,alpha_div_nsz,shift4);\n\
                    sum2 = sum2 * sum2 * sum2;\n\
                    sum2 = sqrt(sum2);\n\
                    sum2 = rsqrt(sum2);\n\
                    _viv_asm(CONV,sum2_fp16,sum2);\n\
                    VXC_DP4x4(out_h,val2_h,sum2_fp16, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 1),Uni4x4_Fp16xFp16UnpackHi4);\n\
                    if (outputFormat == 15) \n\
                    {\n\
                        _viv_asm(COPY, val5, out_h, 16); \n\
                        VXC_WriteImage2DArray(output, coord_out, val5, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, out_h, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                        VXC_WriteImage2DArray(output, coord_out, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                }while(coord_out.z < loops) ;\n\
            } \n\
            __kernel void vxcNormalization_AcrossMapsInt8In(\n\
                __read_only  image2d_array_t   input,\n\
                             int               width,\n\
                             int               height,\n\
                             int               channel,\n\
                             int               type,\n\
                             int               norm_size,\n\
                             float             alpha,\n\
                             float             beta,\n\
                __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                int4 coord_out = coord_in; \n\
                vxc_char8 val0 = {0}; \n\
                vxc_char8 val1 = {0}; \n\
                vxc_char8 val2; \n\
                vxc_char8 val3; \n\
                vxc_char8 val4; \n\
                vxc_char8 val5; \n\
                vxc_short8 val_s16; \n\
                \n\
                vxc_float4 sum1; \n\
                vxc_float4 sum2; \n\
                vxc_float4 sum_lo; \n\
                vxc_float4 sum_hi; \n\
                float4 alpha_div_nsz = (float4)(alpha/norm_size); \n\
                float4 shift4  = (float4)1; \n\
                \n\
                vxc_half8  val_sub_h; \n\
                vxc_half8  val0_h; \n\
                vxc_half8  val1_h; \n\
                vxc_half8  val2_h; \n\
                vxc_half8  val3_h; \n\
                vxc_half8  val4_h; \n\
                vxc_half8  out_h; \n\
                half4 sum2_fp16; \n\
                half in_scale_fp16; \n\
                half out_scale_fp16; \n\
                int loops = channel-1; \n\
                \n\
                coord_in.z = 0; \n\
                VXC_ReadImage2DArray(val2, input, coord_in, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                coord_in.z = 1; \n\
                VXC_ReadImage2DArray(val3, input, coord_in, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                coord_in.z = 2; \n\
                VXC_ReadImage2DArray(val4, input, coord_in, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(CONV, in_scale_fp16, in_scale); \n\
                _viv_asm(CONV, out_scale_fp16, out_scale); \n\
                _viv_asm(COPY, val0_h, val0, 16); \n\
                _viv_asm(COPY, val1_h, val1, 16); \n\
                VXC_DP2x8(val2_h, val2, in_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                VXC_DP2x8(val3_h, val3, in_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                VXC_DP2x8(val4_h, val4, in_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                \n\
                //low 4 elements \n\
                VXC_DP4x4(sum1,val2_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubLo4); \n\
                sum_lo+=sum1; \n\
                VXC_DP4x4(sum1,val3_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubLo4); \n\
                sum_lo+=sum1; \n\
                VXC_DP4x4(sum1,val4_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubLo4); \n\
                sum_lo+=sum1; \n\
                sum2 = mad(sum_lo,alpha_div_nsz,shift4); \n\
                sum2 = sum2 * sum2 * sum2; \n\
                sum2 = sqrt(sum2); \n\
                sum2 = rsqrt(sum2); \n\
                _viv_asm(CONV,sum2_fp16,sum2); \n\
                VXC_DP4x4(out_h,val2_h,sum2_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_Fp16xFp16UnpackLo4); \n\
                \n\
                //high4 elements \n\
                VXC_DP4x4(sum1,val2_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubHi4); \n\
                sum_hi+=sum1; \n\
                VXC_DP4x4(sum1,val3_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubHi4); \n\
                sum_hi+=sum1; \n\
                VXC_DP4x4(sum1,val4_h,val0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubHi4); \n\
                sum_hi+=sum1; \n\
                sum2 = mad(sum_hi,alpha_div_nsz,shift4); \n\
                sum2 = sum2 * sum2 * sum2; \n\
                sum2 = sqrt(sum2); \n\
                sum2 = rsqrt(sum2); \n\
                _viv_asm(CONV,sum2_fp16,sum2); \n\
                VXC_DP4x4(out_h,val2_h,sum2_fp16, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 1),Uni4x4_Fp16xFp16UnpackHi4); \n\
                if (outputFormat == 15) \n\
                {\n\
                    _viv_asm(COPY, val_s16, out_h, 16); \n\
                    VXC_WriteImage2DArray(output, coord_out, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val5, out_h, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                    VXC_WriteImage2DArray(output, coord_out, val5, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                do \n\
                {\n\
                    coord_out.z++; \n\
                    coord_in.z++; \n\
                    if(coord_in.z<channel) \n\
                       VXC_ReadImage2DArray(val4, input, coord_in, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    else \n\
                       val4 = 0; \n\
                    VXC_DP2x8(val_sub_h,val0_h,val0_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8); \n\
                    VXC_DP2x8(val0_h,val1_h,val1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8); \n\
                    VXC_DP2x8(val1_h,val2_h,val2_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8); \n\
                    VXC_DP2x8(val2_h,val3_h,val3_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8); \n\
                    VXC_DP2x8(val3_h,val4_h,val4_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0),Uni2x8_CopyHalf8); \n\
                    VXC_DP2x8(val4_h, val4, in_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                    \n\
                    //low 4 elements \n\
                    VXC_DP4x4(sum1,val4_h,val_sub_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubLo4); \n\
                    sum_lo+=sum1; \n\
                    sum2 = mad(sum_lo,alpha_div_nsz,shift4); \n\
                    sum2 = sum2 * sum2 * sum2; \n\
                    sum2 = sqrt(sum2); \n\
                    sum2 = rsqrt(sum2); \n\
                    _viv_asm(CONV,sum2_fp16,sum2); \n\
                    VXC_DP4x4(out_h,val2_h,sum2_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0),Uni4x4_Fp16xFp16UnpackLo4); \n\
                    //high 4 elements \n\
                    VXC_DP4x4(sum1,val4_h,val_sub_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1),Uni4x4_SquareSubHi4); \n\
                    sum_hi+=sum1; \n\
                    sum2 = mad(sum_hi,alpha_div_nsz,shift4); \n\
                    sum2 = sum2 * sum2 * sum2; \n\
                    sum2 = sqrt(sum2); \n\
                    sum2 = rsqrt(sum2); \n\
                    _viv_asm(CONV,sum2_fp16,sum2); \n\
                    VXC_DP4x4(out_h,val2_h,sum2_fp16, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 1),Uni4x4_Fp16xFp16UnpackHi4); \n\
                    if (outputFormat == 15) \n\
                    {\n\
                        _viv_asm(COPY, val_s16, out_h, 16); \n\
                        VXC_WriteImage2DArray(output, coord_out, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val5, out_h, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFp16xFp16toS8_dp2x8); \n\
                        VXC_WriteImage2DArray(output, coord_out, val5, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                }while(coord_out.z < loops) ; \n\
            }\n"
        };
        if (norm_type == VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP)
        {
            programLength = strlen(LrnSameMap_programSources[0]);
            program = vxCreateProgramWithSource(context, 1, (const vx_char**)LrnSameMap_programSources, &programLength);
        }
        else if (norm_type == VX_CONVOLUTIONAL_NETWORK_NORM_ACROSS_MAPS)
        {
            programLength = strlen(LrnAcrossMaps_programSources[0]);
            program = vxCreateProgramWithSource(context, 1, (const vx_char**)LrnAcrossMaps_programSources, &programLength);
        }
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcNormalization", program, 9, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }
    if(norm_type == VX_CONVOLUTIONAL_NETWORK_NORM_SAME_MAP)
    {

        vx_uint32 dp_fp16_1[16] = {
            0x15151515, // TCfg
            0x00000000, // ASelt
            0x03210210, 0x05430432, // ABin
            0x15151515, // BSelt
            0x03210210, 0x05430432, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 UniFp16xFp16PackLo4_dp4x4[16] = {
            0x01010101, // TCfg
            0x00000000, // ASelt
            0x00020001, 0x00040003, // ABin
            0x01010101, // BSelt
            0x00020000, 0x00060004, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        execution_parameters.globalWorkOffset[0] = 0;
        execution_parameters.globalWorkOffset[1] = 0;
        execution_parameters.globalWorkOffset[2] = 0;
        execution_parameters.globalWorkScale[0]  = 4;
        execution_parameters.globalWorkScale[1]  = height;
        execution_parameters.globalWorkScale[2]  = 1;
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.localWorkSize[2]    = 1;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((width  + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]); //1
        execution_parameters.globalWorkSize[2]   = gcmALIGN((channel+ execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]); //channel

        if (inputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SameMapFp16In", borderMode);
            if (!shaderExecutable) goto error;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_SameMapInt8In", borderMode);
            if (!shaderExecutable) goto error;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniS8xFp16toFp16_dp2x8", 1, UniS8xFp16toFp16_dp2x8);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "in_scale", 1, &in_scale);
            if (status != VX_SUCCESS) goto error;
        }
        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "dp_fp16_1", 1, dp_fp16_1);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFp16xFp16PackLo4_dp4x4", 1, UniFp16xFp16PackLo4_dp4x4);
        if (status != VX_SUCCESS) goto error;
    }
    else //VX_CONVOLUTIONAL_NETWORK_NORM_ACROSS_MAPS
    {
        vx_uint32 Uni4x4_SquareSubLo4[16] = {
            0x09090909, // TCfg
            0x04040404, // ASelt
            0x00110000, 0x00330022, // ABin
            0x04040404, // BSelt
            0x00110000, 0x00330022, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 Uni4x4_SquareSubHi4[16] = {
            0x09090909, // TCfg
            0x04040404, // ASelt
            0x00550044, 0x00770066, // ABin
            0x04040404, // BSelt
            0x00550044, 0x00770066, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 Uni4x4_Fp16xFp16UnpackLo4[16] = {
            0x01010101, // TCfg
            0x00000000, // ASelt
            0x00010000, 0x00030002, // ABin
            0x01010101, // BSelt
            0x00020000, 0x00060004, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 Uni4x4_Fp16xFp16UnpackHi4[16] = {
            0x01010101, // TCfg
            0x00000000, // ASelt
            0x00050004, 0x00070006, // ABin
            0x01010101, // BSelt
            0x00020000, 0x00060004, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        vx_uint32 Uni2x8_CopyHalf8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
        };

        execution_parameters.globalWorkOffset[0] = 0;
        execution_parameters.globalWorkOffset[1] = 0;
        execution_parameters.globalWorkOffset[2] = 0;
        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;
        execution_parameters.globalWorkScale[2]  = 1;
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.localWorkSize[2]    = 1;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((width  + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]); //1
        execution_parameters.globalWorkSize[2]   = 1; //gcmALIGN((channel+ execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]); //channel

        if (inputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AcrossMapsFp16In", borderMode);
            if (!shaderExecutable) goto error;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AcrossMapsInt8In", borderMode);
            if (!shaderExecutable) goto error;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniS8xFp16toFp16_dp2x8", 1, UniS8xFp16toFp16_dp2x8);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "in_scale", 1, &in_scale);
            if (status != VX_SUCCESS) goto error;
        }
        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni4x4_SquareSubLo4", 1, Uni4x4_SquareSubLo4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni4x4_SquareSubHi4", 1, Uni4x4_SquareSubHi4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni4x4_Fp16xFp16UnpackLo4", 1, Uni4x4_Fp16xFp16UnpackLo4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni4x4_Fp16xFp16UnpackHi4", 1, Uni4x4_Fp16xFp16UnpackHi4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni2x8_CopyHalf8", 1, Uni2x8_CopyHalf8);
        if (status != VX_SUCCESS) goto error;
   }
   status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFp16xFp16toS8_dp2x8", 1, UniFp16xFp16toS8_dp2x8);
   status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "out_scale", 1, &out_scale);
   status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outputFormat", 1, &outputFormat);
   if (status != VX_SUCCESS) goto error;

   status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 9);
   if (status != VX_SUCCESS) goto error;

   status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
   if (status != VX_SUCCESS) goto error;

   if(width_s) vxReleaseScalar(&width_s);
   if(height_s) vxReleaseScalar(&height_s);
   if(channel_s) vxReleaseScalar(&channel_s);

   return shaderExecutable;

error:
   if (program) vxReleaseProgram(&program);
   if(width_s) vxReleaseScalar(&width_s);
   if(height_s) vxReleaseScalar(&height_s);
   if(channel_s) vxReleaseScalar(&channel_s);
   if (shaderExecutable) gcoOS_Free(gcvNULL, (gctPOINTER)shaderExecutable);

   return VX_NULL;
}

