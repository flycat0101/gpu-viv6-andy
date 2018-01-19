/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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

char VXC_VertMax3_Evis1[] =
{"\n\
#define VXC_VertMax3_Integer(dst, src0, src1, src2, info)\\\r\n\
    do\\\n\
    {\\\n\
        dst = max(src0, src1);\\\n\
        dst = max(src2, dst);\\\n\
    } while (0)\n\
#define VXC_VertMax3_Half(dst, src0, src1, src2, info)\\\r\n\
    do\\\n\
    {\\\n\
        vxc_short8 val0, val1, val2, minVal, maxVal;\\\n\
        _viv_asm(COPY, val0, src0, 16);\\\n\
        _viv_asm(COPY, val1, src1, 16);\\\n\
        _viv_asm(COPY, val2, src2, 16);\\\n\
        maxVal = max(val0, val1);\\\n\
        maxVal = max(val2, maxVal);\\\n\
        minVal = min(val0, val1);\\\n\
        minVal = min(val2, minVal);\\\n\
        maxVal = maxVal >= 0 ? maxVal : minVal;\\\n\
        _viv_asm(COPY, dst, maxVal, 16); \\\n\
    } while (0)\n"
};

char VXC_VertMax3_Evis2[] =
{"\n\
#define VXC_VertMax3_Integer(dst, src0, src1, src2, info)\\\r\n\
    do\\\n\
    {\\\n\
        VXC_VertMax3(dst, src0, src1, src2, info);\\\n\
    } while (0)\n\
#define VXC_VertMax3_Half(dst, src0, src1, src2, info)\\\r\n\
    do\\\n\
    {\\\n\
        VXC_VertMax3(dst, src0, src1, src2, info);\\\n\
    } while (0)\n"
};

char VXC_HorzMax3_Evis1[] =
{" \n\
#define VXC_HorzMax3_Integer(dst, src0, info)\\\r\n\
    do\\\n\
    {\\\n\
        int startBin     = (info & VXC_START_BIN_BITMASK) >> 12;\\\n\
        int endBin         = (info & VXC_END_BIN_BITMASK) >> 8;\\\n\
        int sourceBin     = (info & VXC_SOURCE_BIN_BITMASK) >> 4;\\\n\
        int clamp         = (info & VXC_CLAMP_BITMASK) >> 22;\\\n\
        int mod1 = VXC_MODIFIER_FILTER(startBin, endBin, sourceBin, VXC_FM_Max, clamp);\\\n\
        VXC_OP4(filter, dst, src0, src0, src0, mod1);\\\n\
    } while (0)\n\
#define VXC_HorzMax3_Half(dst, src0, info)\\\r\n\
    do\\\n\
    {\\\n\
        int startBin     = (info & VXC_START_BIN_BITMASK) >> 12;\\\n\
        int endBin         = (info & VXC_END_BIN_BITMASK) >> 8;\\\n\
        int sourceBin     = (info & VXC_SOURCE_BIN_BITMASK) >> 4;\\\n\
        int clamp         = (info & VXC_CLAMP_BITMASK) >> 22;\\\n\
        int mod1 = VXC_MODIFIER_FILTER(startBin, endBin, sourceBin, VXC_FM_Max, clamp);\\\n\
        int mod2 = VXC_MODIFIER_FILTER(startBin, endBin, sourceBin, VXC_FM_Min, clamp);\\\n\
        vxc_short8 val0, minVal, maxVal;\\\n\
        _viv_asm(COPY, val0, src0, 16);\\\n\
        VXC_OP4(filter, maxVal, val0, val0, val0, mod1);\\\n\
        VXC_OP4(filter, minVal, val0, val0, val0, mod2);\\\n\
        maxVal = maxVal >= 0 ? maxVal : minVal;\\\n\
        _viv_asm(COPY, dst, maxVal, 16);\\\n\
    } while (0)\n"
};

char VXC_HorzMax3_Evis2[] =
{" \n\
#define VXC_HorzMax3_Integer(dst, src0, info)\\\r\n\
    do\\\n\
    {\\\n\
        VXC_HorzMax3(dst, src0, info);\\\n\
    } while (0)\n\
#define VXC_HorzMax3_Half(dst, src0, info)\\\r\n\
    do\\\n\
    {\\\n\
        VXC_HorzMax3(dst, src0, info);\\\n\
    } while (0)\n"
};

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

/********vxcBatchNormalization****************************************************/
vxnne_shader_executable vxnneGetBatchNormShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               eps,
    vx_tensor               mean,
    vx_tensor               variance,
    vx_tensor               gamma,
    vx_tensor               beta,
    vx_tensor               output)
{
    vx_size    programLength = 0;
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_uint32 input_size[4] = {0, 0, 0, 0};
    vx_uint32 UniFP16SubtoFP32hi4_dp4x4[16] = {
        0x09090909, // TCfg
        0x04040404, // ASelt
        0x00050004, 0x00070006, // ABin
        0x0a0a0a0a, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000 // Constant
    };
    vx_uint32 UniFP16SubtoFP32lo4_dp4x4[16] = {
        0x09090909, // TCfg
        0x04040404, // ASelt
        0x00010000, 0x00030002, // ABin
        0x0a0a0a0a, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000 // Constant
    };
    vx_uint32 UniPackLow16bit_dp4x4[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00020000, 0x00060004, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference    parameters[7]              = {(vx_reference)input, (vx_reference)eps, NULL, NULL, NULL, NULL, (vx_reference)output};
    vx_enum         inputFormat                = input->tensorBuffer->dataFormat;
    vx_enum         outputFormat               = output->tensorBuffer->dataFormat;
    vx_image        imgmean                    = NULL;
    vx_image        imgvari                    = NULL;
    vx_image        imggamma                   = NULL;
    vx_image        imgbeta                    = NULL;

    if (inputFormat != VX_TYPE_FLOAT16 || outputFormat != VX_TYPE_FLOAT16)
    {
        gcmPRINT("input or output's format is not support");
        goto error;
    }
    status = vxQueryTensor(input, VX_TENSOR_DIMS, input_size, sizeof(input_size));
    if (status != VX_SUCCESS) goto error;

    imgmean  = vxoTensor_CreateImageFromTensor(mean, input_size[2], 1, VX_DF_IMAGE_F32);
    imgvari  = vxoTensor_CreateImageFromTensor(variance, input_size[2], 1, VX_DF_IMAGE_F32);
    imggamma = vxoTensor_CreateImageFromTensor(gamma, input_size[2], 1, VX_DF_IMAGE_F32);
    imgbeta  = vxoTensor_CreateImageFromTensor(beta, input_size[2], 1, VX_DF_IMAGE_F32);

    parameters[2] = (vx_reference)imgmean;
    parameters[3] = (vx_reference)imgvari;
    parameters[4] = (vx_reference)imggamma;
    parameters[5] = (vx_reference)imgbeta;

    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkOffset[2] = 0;
    execution_parameters.globalWorkScale[0]  = 8;
    execution_parameters.globalWorkScale[1]  = 1;
    execution_parameters.globalWorkScale[2]  = 1;
    execution_parameters.localWorkSize[0]    = 8;
    execution_parameters.localWorkSize[1]    = 1;
    execution_parameters.localWorkSize[2]    = 1;
    execution_parameters.globalWorkSize[0]   = gcmALIGN((input_size[0] + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
    execution_parameters.globalWorkSize[1]   = gcmALIGN((input_size[1] + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    execution_parameters.globalWorkSize[2]   = gcmALIGN((input_size[2] + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]);

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char * programSources[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            \n\
            _viv_uniform VXC_512Bits UniFP16SubtoFP32hi4_dp4x4;\n\
            _viv_uniform VXC_512Bits UniFP16SubtoFP32lo4_dp4x4;\n\
            _viv_uniform VXC_512Bits UniPackLow16bit_dp4x4;\n\
            __kernel void vxcBatchNorm_Fp16toFp16Tensor(\n\
                      __read_only image2d_array_t  input, \n\
                                            float  eps, \n\
                            __read_only image2d_t  mean, \n\
                            __read_only image2d_t  variance, \n\
                            __read_only image2d_t  gamma, \n\
                            __read_only image2d_t  beta, \n\
                     __write_only image2d_array_t  output)\n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                int2 coord_para = (int2)(coord.z, 0); \n\
                \n\
                vxc_short8 img1_s16; \n\
                vxc_half8 val1_fp16,val2_fp16; \n\
                float mean_fp32,vari_fp32,gamma_fp32,beta_fp32; \n\
                half mean_fp16; \n\
                float4 val1_fp32, val2_fp32; \n\
                half4 val_h4; \n\
                \n\
                VXC_ReadImage2DArray(img1_s16, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                mean_fp32 = read_imagef(mean, coord_para).x; \n\
                vari_fp32 = read_imagef(variance, coord_para).x; \n\
                gamma_fp32 = read_imagef(gamma, coord_para).x; \n\
                beta_fp32 = read_imagef(beta, coord_para).x; \n\
                \n\
                _viv_asm(CONV, mean_fp16, mean_fp32); \n\
                vari_fp32 += eps; \n\
                val1_fp32 = rsqrt(vari_fp32); \n\
                _viv_asm(COPY, val1_fp16, img1_s16, 16); \n\
                //low 4 elements \n\
                VXC_DP4x4(val2_fp32, val1_fp16, mean_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), UniFP16SubtoFP32lo4_dp4x4); \n\
                val2_fp32 = val2_fp32 * val1_fp32; \n\
                val2_fp32 = mad(val2_fp32, gamma_fp32, beta_fp32); \n\
                _viv_asm(CONV, val_h4, val2_fp32); \n\
                VXC_DP4x4(val2_fp16, val_h4, val_h4, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), UniPackLow16bit_dp4x4); \n\
                //high 4 elements \n\
                VXC_DP4x4(val2_fp32, val1_fp16, mean_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), UniFP16SubtoFP32hi4_dp4x4); \n\
                val2_fp32 = val2_fp32 * val1_fp32; \n\
                val2_fp32 = mad(val2_fp32, gamma_fp32, beta_fp32); \n\
                _viv_asm(CONV, val_h4, val2_fp32); \n\
                VXC_DP4x4(val2_fp16, val_h4, val_h4, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 1), UniPackLow16bit_dp4x4); \n\
                \n\
                _viv_asm(COPY, img1_s16, val2_fp16, 16); \n\
                VXC_WriteImage2DArray(output, coord, img1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n"
        };

        programLength = strlen(programSources[0]);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, &programLength);
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcBatchNorm", program, 7, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toFp16Tensor", borderMode);
    if (!shaderExecutable) goto error;

    status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFP16SubtoFP32hi4_dp4x4", 1, UniFP16SubtoFP32hi4_dp4x4);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFP16SubtoFP32lo4_dp4x4", 1, UniFP16SubtoFP32lo4_dp4x4);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniPackLow16bit_dp4x4", 1, UniPackLow16bit_dp4x4);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 7);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(imgmean)  vxReleaseImage(&imgmean);
    if(imgvari)  vxReleaseImage(&imgvari);
    if(imggamma)  vxReleaseImage(&imggamma);
    if(imgbeta)  vxReleaseImage(&imgbeta);

    return shaderExecutable;

error:
    if (program)  vxReleaseProgram(&program);
    if(imgmean)  vxReleaseImage(&imgmean);
    if(imgvari)  vxReleaseImage(&imgvari);
    if(imggamma)  vxReleaseImage(&imggamma);
    if(imgbeta)  vxReleaseImage(&imgbeta);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcReorg****************************************************/
vxnne_shader_executable vxnneGetReorgShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_scalar               stride,
    vx_scalar               outc,
    vx_tensor               output)
{
    vx_size    programLength = 0;
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[4]              = {(vx_reference)input, (vx_reference)stride, (vx_reference)outc, (vx_reference)output};
    vx_enum       inputFormat                = input->tensorBuffer->dataFormat;
    vx_enum       outputFormat               = output->tensorBuffer->dataFormat;
    vx_int8       input_fractionLengthValue  = input->tensorBuffer->fixedPointPos;
    vx_int8       output_fractionLengthValue = output->tensorBuffer->fixedPointPos;
    vx_uint32     input_width                = input->tensorBuffer->memory.dims[0][0];
    vx_uint32     input_height               = input->tensorBuffer->memory.dims[0][1];
    vx_uint32     input_depth                = input->tensorBuffer->memory.dims[0][2];
    vx_uint32     output_height              = output->tensorBuffer->memory.dims[0][1];
    vx_float32    heighto_scale              = 1.0f / (vx_float32)output_height;
    vx_int8       div_fractionLengthValue    = output_fractionLengthValue - input_fractionLengthValue;
    vx_float32    div_scale                  = 1.0f;

    if ((inputFormat != VX_TYPE_FLOAT16 && inputFormat != VX_TYPE_INT8) || (outputFormat != VX_TYPE_FLOAT16 && outputFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or output's format is not support");
        goto error;
    }
    if (div_fractionLengthValue >= 0)
    {
        div_scale = (vx_float32) (1 << div_fractionLengthValue);
    }
    else
    {
        div_scale = 1.0f / (vx_float32) (1 << -div_fractionLengthValue);
    }
    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkOffset[2] = 0;
    execution_parameters.globalWorkScale[0]  = 16;
    execution_parameters.globalWorkScale[1]  = 2;
    execution_parameters.globalWorkScale[2]  = 1;
    execution_parameters.localWorkSize[0]    = 1;
    execution_parameters.localWorkSize[1]    = 1;
    execution_parameters.localWorkSize[2]    = 8;
    execution_parameters.globalWorkSize[0]   = gcmALIGN((input_width  + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
    execution_parameters.globalWorkSize[1]   = gcmALIGN((input_height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    execution_parameters.globalWorkSize[2]   = gcmALIGN((input_depth  + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]);

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char * programSources[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            \n\
            _viv_uniform VXC_512Bits UniPackFP16even_2x8; \n\
            _viv_uniform VXC_512Bits UniPackFP16odd_2x8; \n\
            _viv_uniform VXC_512Bits UniPackS8even_dp2x8; \n\
            _viv_uniform VXC_512Bits UniPackS8odd_dp2x8; \n\
            _viv_uniform VXC_512Bits UniS8xFp16_dp2x8; \n\
            _viv_uniform VXC_512Bits UniFp16xFp16toS8_dp2x8; \n\
            _viv_uniform int height_in; \n\
            _viv_uniform int height_out; \n\
            _viv_uniform float heighto_scale; \n\
            _viv_uniform float div_scale; \n\
            __kernel void vxcReorg_for0str2Fp16toInt8(\n\
                      __read_only image2d_array_t   input, \n\
                                              int   stride, \n\
                                              int   outc, \n\
                     __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                vxc_short8 img1_s16,img2_s16,img3_s16,img4_s16;\n\
                vxc_short8 val_s16;\n\
                vxc_half8 val_fp16;\n\
                vxc_char8 val_s8;\n\
                half scale_fp16;\n\
                \n\
                VXC_ReadImage2DArray(img1_s16, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                VXC_ReadImage2DArray(img2_s16, input, coord, VXC_5BITOFFSET_XY(8,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                VXC_ReadImage2DArray(img3_s16, input, coord, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                VXC_ReadImage2DArray(img4_s16, input, coord, VXC_5BITOFFSET_XY(8,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                \n\
                int outx = coord.x >> 1;\n\
                int index_div = (coord.y + height_in * coord.z) >> 1;\n\
                int i = index_div & 1;\n\
                int offsetz = (i * outc) << 1;\n\
                int index_out = index_div - i;\n\
                int j = index_out * heighto_scale;\n\
                int outy = index_out - j * height_out;\n\
                int outz = offsetz + j;\n\
                int4 posout = (int4)(outx, outy, outz, 0);\n\
                \n\
                _viv_asm(CONV, scale_fp16, div_scale);\n\
                \n\
                VXC_DP2x8(val_s16, img1_s16, img2_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16even_2x8);\n\
                _viv_asm(COPY, val_fp16, val_s16, 16);\n\
                VXC_DP2x8(val_s8, val_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8);\n\
                VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                posout.z += outc;\n\
                VXC_DP2x8(val_s16, img1_s16, img2_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16odd_2x8);\n\
                _viv_asm(COPY, val_fp16, val_s16, 16);\n\
                VXC_DP2x8(val_s8, val_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8);\n\
                VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                int m = index_out + 1;\n\
                int k = m * heighto_scale;\n\
                posout.y = m - k * height_out;\n\
                posout.z = offsetz + k;\n\
                VXC_DP2x8(val_s16, img3_s16, img4_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16even_2x8);\n\
                _viv_asm(COPY, val_fp16, val_s16, 16);\n\
                VXC_DP2x8(val_s8, val_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8);\n\
                VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                posout.z += outc;\n\
                VXC_DP2x8(val_s16, img3_s16, img4_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16odd_2x8);\n\
                _viv_asm(COPY, val_fp16, val_s16, 16);\n\
                VXC_DP2x8(val_s8, val_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8);\n\
                VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            }\n\
            __kernel void vxcReorg_for0str2Int8toFp16(\n\
                      __read_only image2d_array_t   input, \n\
                                              int   stride, \n\
                                              int   outc, \n\
                     __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                vxc_char8 img1_s8,img2_s8,img3_s8,img4_s8;\n\
                vxc_char8 val_s8;\n\
                vxc_half8 val_fp16;\n\
                half scale_fp16;\n\
                vxc_short8 val_s16;\n\
                \n\
                VXC_ReadImage2DArray(img1_s8, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                VXC_ReadImage2DArray(img2_s8, input, coord, VXC_5BITOFFSET_XY(8,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                VXC_ReadImage2DArray(img3_s8, input, coord, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                VXC_ReadImage2DArray(img4_s8, input, coord, VXC_5BITOFFSET_XY(8,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                \n\
                int outx = coord.x >> 1;\n\
                int index_div = (coord.y + height_in * coord.z) >> 1;\n\
                int i = index_div & 1;\n\
                int offsetz = (i * outc) << 1;\n\
                int index_out = index_div - i;\n\
                int j = index_out * heighto_scale;\n\
                int outy = index_out - j * height_out;\n\
                int outz = offsetz + j;\n\
                int4 posout = (int4)(outx, outy, outz, 0);\n\
                \n\
                _viv_asm(CONV, scale_fp16, div_scale);\n\
                \n\
                VXC_DP2x8(val_s8, img1_s8, img2_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackS8even_dp2x8);\n\
                VXC_DP2x8(val_fp16, val_s8, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16_dp2x8);\n\
                _viv_asm(COPY, val_s16, val_fp16, 16);\n\
                VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                posout.z += outc;\n\
                VXC_DP2x8(val_s8, img1_s8, img2_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackS8odd_dp2x8);\n\
                VXC_DP2x8(val_fp16, val_s8, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16_dp2x8);\n\
                _viv_asm(COPY, val_s16, val_fp16, 16);\n\
                VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                int m = index_out + 1;\n\
                int k = m * heighto_scale;\n\
                posout.y = m - k * height_out;\n\
                posout.z = offsetz + k;\n\
                VXC_DP2x8(val_s8, img3_s8, img4_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackS8even_dp2x8);\n\
                VXC_DP2x8(val_fp16, val_s8, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16_dp2x8);\n\
                _viv_asm(COPY, val_s16, val_fp16, 16);\n\
                VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
                posout.z += outc;\n\
                VXC_DP2x8(val_s8, img3_s8, img4_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackS8odd_dp2x8);\n\
                VXC_DP2x8(val_fp16, val_s8, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16_dp2x8);\n\
                _viv_asm(COPY, val_s16, val_fp16, 16);\n\
                VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            }\n\
            __kernel void vxcReorg_for0str2Int8toInt8(\n\
                      __read_only image2d_array_t   input, \n\
                                              int   stride, \n\
                                              int   outc, \n\
                     __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                vxc_char8 img1_s16,img2_s16,img3_s16,img4_s16; \n\
                vxc_char8 val1_s16; \n\
                \n\
                VXC_ReadImage2DArray(img1_s16, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img2_s16, input, coord, VXC_5BITOFFSET_XY(8,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img3_s16, input, coord, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img4_s16, input, coord, VXC_5BITOFFSET_XY(8,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                \n\
                int outx = coord.x >> 1; \n\
                int index_div = (coord.y + height_in * coord.z) >> 1; \n\
                int i = index_div & 1; \n\
                int offsetz = (i * outc) << 1; \n\
                int index_out = index_div - i; \n\
                int j = index_out * heighto_scale; \n\
                int outy = index_out - j * height_out; \n\
                int outz = offsetz + j; \n\
                int4 posout = (int4)(outx, outy, outz, 0); \n\
                \n\
                VXC_DP2x8(val1_s16, img1_s16, img2_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackS8even_dp2x8); \n\
                VXC_WriteImage2DArray(output, posout, val1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                posout.z += outc; \n\
                VXC_DP2x8(val1_s16, img1_s16, img2_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackS8odd_dp2x8); \n\
                VXC_WriteImage2DArray(output, posout, val1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                int m = index_out + 1; \n\
                int k = m * heighto_scale; \n\
                posout.y = m - k * height_out; \n\
                posout.z = offsetz + k; \n\
                VXC_DP2x8(val1_s16, img3_s16, img4_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackS8even_dp2x8); \n\
                VXC_WriteImage2DArray(output, posout, val1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                posout.z += outc; \n\
                VXC_DP2x8(val1_s16, img3_s16, img4_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackS8odd_dp2x8); \n\
                VXC_WriteImage2DArray(output, posout, val1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n\
            __kernel void vxcReorg_for0str2Fp16toFp16(\n\
                      __read_only image2d_array_t   input, \n\
                                              int   stride, \n\
                                              int   outc, \n\
                     __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                vxc_short8 img1_s16,img2_s16,img3_s16,img4_s16; \n\
                vxc_short8 val1_s16; \n\
                \n\
                VXC_ReadImage2DArray(img1_s16, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img2_s16, input, coord, VXC_5BITOFFSET_XY(8,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img3_s16, input, coord, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img4_s16, input, coord, VXC_5BITOFFSET_XY(8,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                \n\
                int outx = coord.x >> 1; \n\
                int index_div = (coord.y + height_in * coord.z) >> 1; \n\
                int i = index_div & 1; \n\
                int offsetz = (i * outc) << 1; \n\
                int index_out = index_div - i; \n\
                int j = index_out * heighto_scale; \n\
                int outy = index_out - j * height_out; \n\
                int outz = offsetz + j; \n\
                int4 posout = (int4)(outx, outy, outz, 0); \n\
                \n\
                VXC_DP2x8(val1_s16, img1_s16, img2_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16even_2x8); \n\
                VXC_WriteImage2DArray(output, posout, val1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                posout.z += outc; \n\
                VXC_DP2x8(val1_s16, img1_s16, img2_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16odd_2x8); \n\
                VXC_WriteImage2DArray(output, posout, val1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                int m = index_out + 1; \n\
                int k = m * heighto_scale; \n\
                posout.y = m - k * height_out; \n\
                posout.z = offsetz + k; \n\
                VXC_DP2x8(val1_s16, img3_s16, img4_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16even_2x8); \n\
                VXC_WriteImage2DArray(output, posout, val1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                posout.z += outc; \n\
                VXC_DP2x8(val1_s16, img3_s16, img4_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16odd_2x8); \n\
                VXC_WriteImage2DArray(output, posout, val1_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n"
        };

        programLength = strlen(programSources[0]);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, &programLength);
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcReorg", program, 4, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }
    if (inputFormat == VX_TYPE_FLOAT16)
    {
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
        vx_uint32 UniFp16xFp16toS8_dp2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        if (outputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_for0str2Fp16toFp16", borderMode);
            if (!shaderExecutable) goto error;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_for0str2Fp16toInt8", borderMode);
            if (!shaderExecutable) goto error;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFp16xFp16toS8_dp2x8", 1, UniFp16xFp16toS8_dp2x8);
            if (status != VX_SUCCESS) goto error;
        }
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniPackFP16even_2x8", 1, UniPackFP16even_2x8);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniPackFP16odd_2x8", 1, UniPackFP16odd_2x8);
        if (status != VX_SUCCESS) goto error;
    }
    else
    {
        vx_uint32 UniPackS8even_dp2x8[16] = {
            0x33333333, // TCfg
            0x11110000, // ASelt
            0x06040200, 0x06040200, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 UniPackS8odd_dp2x8[16] = {
            0x33333333, // TCfg
            0x11110000, // ASelt
            0x07050301, 0x07050301, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 UniS8xFp16_dp2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        if (outputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_for0str2Int8toFp16", borderMode);
            if (!shaderExecutable) goto error;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniS8xFp16_dp2x8", 1, UniS8xFp16_dp2x8);
            if (status != VX_SUCCESS) goto error;
        }
        else
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_for0str2Int8toInt8", borderMode);
            if (!shaderExecutable) goto error;
        }
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniPackS8even_dp2x8", 1, UniPackS8even_dp2x8);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniPackS8odd_dp2x8", 1, UniPackS8odd_dp2x8);
        if (status != VX_SUCCESS) goto error;
    }
    status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "height_in", 1, &input_height);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "height_out", 1, &output_height);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "heighto_scale", 1, &heighto_scale);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "div_scale", 1, &div_scale);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    return shaderExecutable;

error:
    if (program)  vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcActivationRelu****************************************************/
vxnne_shader_executable vxnneGetActivationReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output)
{
    vx_size    programLength[2] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    vx_reference   parameters[2]    = {(vx_reference)input, (vx_reference)output};
    vx_enum        inputFormat      = input->tensorBuffer->dataFormat;
    vx_enum        outputFormat     = output->tensorBuffer->dataFormat;
    vx_uint32      in_width         = input->tensorBuffer->memory.dims[0][1];
    vx_uint32      in_height        = input->tensorBuffer->memory.dims[0][1];
    vx_uint32      depth            = input->tensorBuffer->memory.dims[0][2];
    vx_image       imgInput         = NULL;
    vx_image       imgOutput        = NULL;
    vx_int8        srcFixPointPos   = input->tensorBuffer->fixedPointPos;
    vx_int8        dstFixPointPos   = output->tensorBuffer->fixedPointPos;
    vx_float32     scaleOut         = 1.0f;
    vx_float32     scaleIn          = 1.0f;

    if (inputFormat == VX_TYPE_INT8)
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

    if (outputFormat == VX_TYPE_INT8)
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

    borderMode->mode = VX_BORDER_REPLICATE;
    if (inputFormat == VX_TYPE_INT8)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, in_width * in_height, depth, VX_DF_IMAGE_U8);
        borderMode->constant_value.U8 = 0;
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, in_width * in_height, depth, VX_DF_IMAGE_S16);
        borderMode->constant_value.S16 = 0;
    }

    if (outputFormat == VX_TYPE_INT8)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, in_width * in_height, depth, VX_DF_IMAGE_U8);
    }
    else if (outputFormat == VX_TYPE_FLOAT16)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, in_width * in_height, depth, VX_DF_IMAGE_S16);
    }

    parameters[0] = (vx_reference)imgInput;
    parameters[1] = (vx_reference)imgOutput;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[2] = {NULL, NULL};

        char programSources_ActivationRelu_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         \n\
         _viv_uniform VXC_512Bits uniS16MulS16toInt8_2x8; \n\
         _viv_uniform VXC_512Bits uniS8MulS16toFp16_2x8_lo; \n\
         _viv_uniform VXC_512Bits uniS8MulS16toFp16_2x8_hi; \n\
        _viv_uniform float scaleIn; \n\
         _viv_uniform float scaleOut; \n\
         \n\
         __kernel void vxcActivationRelu_Int8toFp16(\n\
         __read_only image2d_t   input, \n\
         __write_only image2d_t  output) \n\
        { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(1), get_global_id(1)); \n\
             \n\
            vxc_char16 img_val0, img_val1, img_val2, img_val3; \n\
            vxc_half8 h0, h1, h2, h3; \n\
            vxc_half scale; \n\
             \n\
            VXC_ReadImage(img_val0, input, coord.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val0, input, coord.xy, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(8, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val1, input, coord.xy, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val1, input, coord.xy, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(8, 15, 0, VXC_RM_TowardZero, 0)); \n\
            coord.zw += (int2)(1, 2); \n\
            _viv_asm(CONV, scale, scaleIn); \n\
            img_val0 = max(0, img_val0); \n\
            img_val1 = max(0, img_val1); \n\
            VXC_DP2x8(h0, img_val0, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), uniS8MulS16toFp16_2x8_lo); \n\
            VXC_DP2x8(h1, img_val0, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), uniS8MulS16toFp16_2x8_hi); \n\
            VXC_DP2x8(h2, img_val1, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), uniS8MulS16toFp16_2x8_lo); \n\
            VXC_DP2x8(h3, img_val1, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), uniS8MulS16toFp16_2x8_hi); \n\
            vxc_short8 dst; \n\
            _viv_asm(COPY, dst, h0, 16); \n\
            VXC_WriteImage(output, coord.xy, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord.y += 3; \n\
            _viv_asm(COPY, dst, h1, 16); \n\
            VXC_WriteImage(output, coord.xz, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, dst, h2, 16); \n\
            VXC_WriteImage(output, coord.xw, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, dst, h3, 16); \n\
            VXC_WriteImage(output, coord.xy, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
        } \n\
         __kernel void vxcActivationRelu_Fp16toFp16(\n\
         __read_only image2d_t   input, \n\
         __write_only image2d_t  output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(1), get_global_id(1)); \n\
            \n\
            vxc_short8 img_val0, img_val1, img_val2, img_val3; \n\
            \n\
            VXC_ReadImage(img_val0, input, coord.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val1, input, coord.xy, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val2, input, coord.xy, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val3, input, coord.xy, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord.zw += (int2)(1, 2); \n\
            img_val0 = max(0, img_val0); \n\
            img_val1 = max(0, img_val1); \n\
            img_val2 = max(0, img_val2); \n\
            img_val3 = max(0, img_val3); \n\
            VXC_WriteImage(output, coord.xy, img_val0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord.y += 3; \n\
            VXC_WriteImage(output, coord.xz, img_val1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord.xw, img_val2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord.xy, img_val3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
         }\n\
         __kernel void vxcActivationRelu_Fp16toInt8(\n\
         __read_only image2d_t   input, \n\
         __write_only image2d_t  output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(1), get_global_id(1)); \n\
            \n\
            vxc_short8 img_val0, img_val1, img_val2, img_val3; \n\
            vxc_half8 h0, h1, h2, h3; \n\
            vxc_char16 v0, v1; \n\
            vxc_half scale; \n\
            \n\
            VXC_ReadImage(img_val0, input, coord.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val1, input, coord.xy, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val2, input, coord.xy, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val3, input, coord.xy, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord.zw += (int2)(1, 2); \n\
            _viv_asm(CONV, scale, scaleOut); \n\
            img_val0 = max(0, img_val0); \n\
            img_val1 = max(0, img_val1); \n\
            img_val2 = max(0, img_val2); \n\
            img_val3 = max(0, img_val3); \n\
            _viv_asm(COPY, h0, img_val0, 16); \n\
            _viv_asm(COPY, h1, img_val1, 16); \n\
            _viv_asm(COPY, h2, img_val2, 16); \n\
            _viv_asm(COPY, h3, img_val3, 16); \n\
            VXC_DP2x8(v0, h0, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), uniS16MulS16toInt8_2x8); \n\
            VXC_DP2x8(v0, h1, scale, VXC_MODIFIER(8, 15, 0, VXC_RM_ToNearestEven, 1), uniS16MulS16toInt8_2x8); \n\
            VXC_DP2x8(v1, h2, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), uniS16MulS16toInt8_2x8); \n\
            VXC_DP2x8(v1, h3, scale, VXC_MODIFIER(8, 15, 0, VXC_RM_ToNearestEven, 1), uniS16MulS16toInt8_2x8); \n\
            VXC_WriteImage(output, coord.xy, v0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord.y += 3; \n\
            VXC_WriteImage(output, coord.xz, v0, VXC_MODIFIER(8, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord.xw, v1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord.xy, v1, VXC_MODIFIER(8, 15, 0, VXC_RM_TowardZero, 0)); \n\
         } \n\
         __kernel void vxcActivationRelu_Int8toInt8(\n\
         __read_only image2d_t   input, \n\
         __write_only image2d_t  output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(1), get_global_id(1)); \n\
            \n\
            vxc_char16 img_val0, img_val1, img_val2, img_val3; \n\
            \n\
            VXC_ReadImage(img_val0, input, coord.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val1, input, coord.xy, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val2, input, coord.xy, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(img_val3, input, coord.xy, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            coord.zw += (int2)(1, 2); \n\
            img_val0 = max(0, img_val0); \n\
            img_val1 = max(0, img_val1); \n\
            img_val2 = max(0, img_val2); \n\
            img_val3 = max(0, img_val3); \n\
            VXC_WriteImage(output, coord.xy, img_val0, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            coord.y += 3; \n\
            VXC_WriteImage(output, coord.xz, img_val1, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord.xw, img_val2, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord.xy, img_val3, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
         }\n"
        };

        programSources[0] = programSources_ActivationRelu_0;
        programSources[1] = programSources_ActivationRelu_0;
        programLength[0]  = strlen(programSources_ActivationRelu_0);
        programLength[1]  = strlen(programSources_ActivationRelu_0);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, programLength);

        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcActivationRelu", program, 2, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
    {

        execution_parameters.globalWorkScale[0]  = 16;
        execution_parameters.globalWorkScale[1]  = 4;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toInt8", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 4;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toFp16", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
    {
        vx_uint32 uniS16MulS16toInt8_2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 4;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toInt8", borderMode);
        if (!shaderExecutable) goto error;
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniS16MulS16toInt8_2x8", 1, uniS16MulS16toInt8_2x8);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleOut", 1, &scaleOut);
        if (status != VX_SUCCESS) goto error;
    }
    else if(inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
    {
        vx_uint32 uniS8MulS16toFp16_2x8_lo[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniS8MulS16toFp16_2x8_hi[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x0b0a0908, 0x0f0e0d0c, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 4;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toFp16", borderMode);
        if (!shaderExecutable) goto error;
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniS8MulS16toFp16_2x8_lo", 1, uniS8MulS16toFp16_2x8_lo);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniS8MulS16toFp16_2x8_hi", 1, uniS8MulS16toFp16_2x8_hi);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleIn", 1, &scaleIn);
        if (status != VX_SUCCESS) goto error;
    }

    if (in_width * in_height > execution_parameters.globalWorkScale[0] * 8)
    {
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
    }
    else if (in_width * in_height > execution_parameters.globalWorkScale[0] * 4)
    {
        execution_parameters.localWorkSize[0]    = 4;
        execution_parameters.localWorkSize[1]    = 2;
    }
    else
    {
        execution_parameters.localWorkSize[0]    = 1;
        execution_parameters.localWorkSize[1]    = 8;
    }
    execution_parameters.globalWorkSize[0]   = gcmALIGN((in_width * in_height + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
    execution_parameters.globalWorkSize[1]   = gcmALIGN((depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcActivationLogistic****************************************************/
vxnne_shader_executable vxnneGetActivationLogisticShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output)
{
    vx_size    programLength[2] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    vx_reference   parameters[2]    = {(vx_reference)input, (vx_reference)output};
    vx_enum        inputFormat      = input->tensorBuffer->dataFormat;
    vx_enum        outputFormat     = output->tensorBuffer->dataFormat;
    vx_uint32      in_width         = input->tensorBuffer->memory.dims[0][1];
    vx_uint32      in_height        = input->tensorBuffer->memory.dims[0][1];
    vx_uint32      depth            = input->tensorBuffer->memory.dims[0][2];
    vx_image       imgInput         = NULL;
    vx_image       imgOutput        = NULL;
    vx_int8        srcFixPointPos   = input->tensorBuffer->fixedPointPos;
    vx_int8        dstFixPointPos   = output->tensorBuffer->fixedPointPos;
    vx_float32     outputScale         = 1.0f;
    vx_float32     inputScale          = 1.0f;

    // uniforms
    vx_uint32 fp16ToFp32_high4[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00050004, 0x00070006, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
    vx_uint32 fp16ToFp32_low4[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00010000, 0x00030002, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
    vx_uint32 halfToVxcHalf_8[16] = {
        0x11111111, // TCfg
        0x11110000, // ASelt
        0x06040200, 0x06040200, // ABin
        0x22222222, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
    };
    vx_uint32 fp16MulFp16ToFp16_8x1[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 int8MulFp16ToFp16_8x1[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    if (inputFormat == VX_TYPE_INT8)
    {
        if (srcFixPointPos >= 0)
        {
            inputScale    = 1.0f / (vx_float32) (1 << srcFixPointPos);
        }
        else if (srcFixPointPos < 0)
        {
            inputScale    = (vx_float32)(1 << -srcFixPointPos);
        }
    }

    if (outputFormat == VX_TYPE_INT8)
    {
        if (dstFixPointPos >= 0)
        {
            outputScale = (vx_float32) (1 << dstFixPointPos);
        }
        else if (dstFixPointPos < 0)
        {
            outputScale = 1.0f / (vx_float32) (1 << -dstFixPointPos);
        }
    }

    borderMode->mode = VX_BORDER_REPLICATE;
    if (inputFormat == VX_TYPE_INT8)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, in_width * in_height, depth, VX_DF_IMAGE_U8);
        borderMode->constant_value.U8 = 0;
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, in_width * in_height, depth, VX_DF_IMAGE_S16);
        borderMode->constant_value.S16 = 0;
    }

    if (outputFormat == VX_TYPE_INT8)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, in_width * in_height, depth, VX_DF_IMAGE_U8);
    }
    else if (outputFormat == VX_TYPE_FLOAT16)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, in_width * in_height, depth, VX_DF_IMAGE_S16);
    }

    parameters[0] = (vx_reference)imgInput;
    parameters[1] = (vx_reference)imgOutput;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[2] = {NULL, NULL};

        char programSources_ActivationLogistic_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         \n\
         _viv_uniform VXC_512Bits fp16ToFp32_low4;\n\
        _viv_uniform VXC_512Bits fp16ToFp32_high4;\n\
        _viv_uniform VXC_512Bits halfToVxcHalf_8;\n\
        _viv_uniform float inputScale;\n\
        _viv_uniform float outputScale;\n\
        _viv_uniform VXC_512Bits int8MulFp16ToFp16_8x1;\n\
        _viv_uniform VXC_512Bits fp16MulFp16ToFp16_8x1;\n\
         \n\
         __kernel void vxcActivationLogistic_Int8toFp16(\n\
         __read_only image2d_t   din, \n\
         __write_only image2d_t  dout) \n\
        { \n\
            int2 coord = (int2)(get_global_id(0), get_global_id(1));\n\
            vxc_char8 dataIn;\n\
            vxc_short8 dataOut;\n\
            vxc_half8 dataInToHalf;\n\
            float4 dataInHalfToFloat0, dataInHalfToFloat1;\n\
            half4 res0, res1;\n\
            half scaleIn;\n\
            VXC_ReadImage(dataIn, din, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, scaleIn, inputScale);\n\
            VXC_DP2x8(dataInToHalf, dataIn, scaleIn, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);\n\
            VXC_DP4x4(dataInHalfToFloat0, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_low4);\n\
            dataInHalfToFloat0 = 1 / (1 + exp(-dataInHalfToFloat0));\n\
            _viv_asm(CONV, res0, dataInHalfToFloat0);\n\
            VXC_DP4x4(dataInHalfToFloat1, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_high4);\n\
            dataInHalfToFloat1 = 1 / (1 + exp(-dataInHalfToFloat1));\n\
            _viv_asm(CONV, res1, dataInHalfToFloat1);\n\
            VXC_DP2x8(dataInToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            _viv_asm(COPY, dataOut, dataInToHalf, 16);\n\
            VXC_WriteImage(dout, coord, dataOut, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
        } \n\
         __kernel void vxcActivationLogistic_Fp16toFp16(\n\
         __read_only image2d_t   din, \n\
         __write_only image2d_t  dout) \n\
         { \n\
            int2 coord = (int2)(get_global_id(0), get_global_id(1));\n\
            vxc_short8 dataIn;\n\
            vxc_half8 dataInToHalf;\n\
            float4 dataInHalfToFloat0, dataInHalfToFloat1;\n\
            half4 res0, res1;\n\
            VXC_ReadImage(dataIn, din, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(COPY, dataInToHalf, dataIn, 16);\n\
            VXC_DP4x4(dataInHalfToFloat0, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_low4);\n\
            dataInHalfToFloat0 = 1 / (1 + exp(-dataInHalfToFloat0));\n\
            _viv_asm(CONV, res0, dataInHalfToFloat0);\n\
            VXC_DP4x4(dataInHalfToFloat1, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_high4);\n\
            dataInHalfToFloat1 = 1 / (1 + exp(-dataInHalfToFloat1));\n\
            _viv_asm(CONV, res1, dataInHalfToFloat1);\n\
            VXC_DP2x8(dataInToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            _viv_asm(COPY, dataIn, dataInToHalf, 16);\n\
            VXC_WriteImage(dout, coord, dataIn, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         }\n\
         __kernel void vxcActivationLogistic_Fp16toInt8(\n\
         __read_only image2d_t   din, \n\
         __write_only image2d_t  dout) \n\
         { \n\
            int2 coord = (int2)(get_global_id(0), get_global_id(1));\n\
            vxc_char8 dataOut;\n\
            vxc_short8 dataIn;\n\
            vxc_half8 dataInToHalf;\n\
            float4 dataInHalfToFloat0, dataInHalfToFloat1;\n\
            half4 res0, res1;\n\
            half scaleOut;\n\
            VXC_ReadImage(dataIn, din, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(COPY, dataInToHalf, dataIn, 16);\n\
            _viv_asm(CONV, scaleOut, outputScale);\n\
            VXC_DP4x4(dataInHalfToFloat0, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_low4);\n\
            dataInHalfToFloat0 = 1 / (1 + exp(-dataInHalfToFloat0));\n\
            _viv_asm(CONV, res0, dataInHalfToFloat0);\n\
            VXC_DP4x4(dataInHalfToFloat1, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_high4);\n\
            dataInHalfToFloat1 = 1 / (1 + exp(-dataInHalfToFloat1));\n\
            _viv_asm(CONV, res1, dataInHalfToFloat1);\n\
            VXC_DP2x8(dataInToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(dataOut, dataInToHalf, scaleOut, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16MulFp16ToFp16_8x1);\n\
            VXC_WriteImage(dout, coord, dataOut, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcActivationLogistic_Int8toInt8(\n\
         __read_only image2d_t   din, \n\
         __write_only image2d_t  dout) \n\
         { \n\
            int2 coord = (int2)(get_global_id(0), get_global_id(1));\n\
            vxc_char8 dataIn;\n\
            vxc_half8 dataInToHalf;\n\
            float4 dataInHalfToFloat0, dataInHalfToFloat1;\n\
            half4 res0, res1;\n\
            half scaleIn, scaleOut;\n\
            VXC_ReadImage(dataIn, din, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, scaleIn, inputScale);\n\
            _viv_asm(CONV, scaleOut, outputScale);\n\
            VXC_DP2x8(dataInToHalf, dataIn, scaleIn, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);\n\
            VXC_DP4x4(dataInHalfToFloat0, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_low4);\n\
            dataInHalfToFloat0 = 1 / (1 + exp(-dataInHalfToFloat0));\n\
            _viv_asm(CONV, res0, dataInHalfToFloat0);\n\
            VXC_DP4x4(dataInHalfToFloat1, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_high4);\n\
            dataInHalfToFloat1 = 1 / (1 + exp(-dataInHalfToFloat1));\n\
            _viv_asm(CONV, res1, dataInHalfToFloat1);\n\
            VXC_DP2x8(dataInToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(dataIn, dataInToHalf, scaleOut, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16MulFp16ToFp16_8x1);\n\
            VXC_WriteImage(dout, coord, dataIn, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         }\n"
        };

        programSources[0] = programSources_ActivationLogistic_0;
        programSources[1] = programSources_ActivationLogistic_0;
        programLength[0]  = strlen(programSources_ActivationLogistic_0);
        programLength[1]  = strlen(programSources_ActivationLogistic_0);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, programLength);

        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcActivationLogistic", program, 2, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
    {

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toInt8", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toFp16", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
    {
        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toInt8", borderMode);
        if (!shaderExecutable) goto error;

        if (status != VX_SUCCESS) goto error;
    }
    else if(inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
    {
        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toFp16", borderMode);
        if (!shaderExecutable) goto error;

        if (status != VX_SUCCESS) goto error;
    }

    if (in_width * in_height > execution_parameters.globalWorkScale[0] * 8)
    {
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
    }
    else if (in_width * in_height > execution_parameters.globalWorkScale[0] * 4)
    {
        execution_parameters.localWorkSize[0]    = 4;
        execution_parameters.localWorkSize[1]    = 2;
    }
    else
    {
        execution_parameters.localWorkSize[0]    = 1;
        execution_parameters.localWorkSize[1]    = 8;
    }
    execution_parameters.globalWorkSize[0]   = gcmALIGN((in_width * in_height + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
    execution_parameters.globalWorkSize[1]   = gcmALIGN((depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);

    status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16ToFp32_low4", 1, fp16ToFp32_low4);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16ToFp32_high4", 1, fp16ToFp32_high4);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "halfToVxcHalf_8", 1, halfToVxcHalf_8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16MulFp16ToFp16_8x1", 1, fp16MulFp16ToFp16_8x1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8MulFp16ToFp16_8x1", 1, int8MulFp16ToFp16_8x1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "inputScale", 1, &inputScale);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outputScale", 1, &outputScale);

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcActivationSoftRelu****************************************************/
vxnne_shader_executable vxnneGetActivationSoftReluShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output)
{
    vx_size    programLength[2] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    vx_reference   parameters[2]    = {(vx_reference)input, (vx_reference)output};
    vx_enum        inputFormat      = input->tensorBuffer->dataFormat;
    vx_enum        outputFormat     = output->tensorBuffer->dataFormat;
    vx_uint32      in_width         = input->tensorBuffer->memory.dims[0][1];
    vx_uint32      in_height        = input->tensorBuffer->memory.dims[0][1];
    vx_uint32      depth            = input->tensorBuffer->memory.dims[0][2];
    vx_image       imgInput         = NULL;
    vx_image       imgOutput        = NULL;
    vx_int8        srcFixPointPos   = input->tensorBuffer->fixedPointPos;
    vx_int8        dstFixPointPos   = output->tensorBuffer->fixedPointPos;
    vx_float32     outputScale         = 1.0f;
    vx_float32     inputScale          = 1.0f;

    // uniforms
    vx_uint32 fp16ToFp32_high4[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00050004, 0x00070006, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
    vx_uint32 fp16ToFp32_low4[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00010000, 0x00030002, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
    vx_uint32 halfToVxcHalf_8[16] = {
        0x11111111, // TCfg
        0x11110000, // ASelt
        0x06040200, 0x06040200, // ABin
        0x22222222, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
    };
    vx_uint32 fp16MulFp16ToFp16_8x1[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 int8MulFp16ToFp16_8x1[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    if (inputFormat == VX_TYPE_INT8)
    {
        if (srcFixPointPos >= 0)
        {
            inputScale    = 1.0f / (vx_float32) (1 << srcFixPointPos);
        }
        else if (srcFixPointPos < 0)
        {
            inputScale    = (vx_float32)(1 << -srcFixPointPos);
        }
    }

    if (outputFormat == VX_TYPE_INT8)
    {
        if (dstFixPointPos >= 0)
        {
            outputScale = (vx_float32) (1 << dstFixPointPos);
        }
        else if (dstFixPointPos < 0)
        {
            outputScale = 1.0f / (vx_float32) (1 << -dstFixPointPos);
        }
    }

    borderMode->mode = VX_BORDER_REPLICATE;
    if (inputFormat == VX_TYPE_INT8)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, in_width * in_height, depth, VX_DF_IMAGE_U8);
        borderMode->constant_value.U8 = 0;
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, in_width * in_height, depth, VX_DF_IMAGE_S16);
        borderMode->constant_value.S16 = 0;
    }

    if (outputFormat == VX_TYPE_INT8)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, in_width * in_height, depth, VX_DF_IMAGE_U8);
    }
    else if (outputFormat == VX_TYPE_FLOAT16)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, in_width * in_height, depth, VX_DF_IMAGE_S16);
    }

    parameters[0] = (vx_reference)imgInput;
    parameters[1] = (vx_reference)imgOutput;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[2] = {NULL, NULL};

        char programSources_ActivationSoftRelu_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         \n\
         _viv_uniform VXC_512Bits fp16ToFp32_low4;\n\
        _viv_uniform VXC_512Bits fp16ToFp32_high4;\n\
        _viv_uniform VXC_512Bits halfToVxcHalf_8;\n\
        _viv_uniform float inputScale;\n\
        _viv_uniform float outputScale;\n\
        _viv_uniform VXC_512Bits int8MulFp16ToFp16_8x1;\n\
        _viv_uniform VXC_512Bits fp16MulFp16ToFp16_8x1;\n\
         \n\
         __kernel void vxcActivationSoftRelu_Int8toFp16(\n\
         __read_only image2d_t   din, \n\
         __write_only image2d_t  dout) \n\
        { \n\
            int2 coord = (int2)(get_global_id(0), get_global_id(1));\n\
            vxc_char8 dataIn;\n\
            vxc_short8 dataOut;\n\
            vxc_half8 dataInToHalf;\n\
            float4 dataInHalfToFloat0, dataInHalfToFloat1;\n\
            half4 res0, res1;\n\
            half scaleIn;\n\
            VXC_ReadImage(dataIn, din, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, scaleIn, inputScale);\n\
            VXC_DP2x8(dataInToHalf, dataIn, scaleIn, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);\n\
            VXC_DP4x4(dataInHalfToFloat0, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_low4);\n\
            dataInHalfToFloat0 = log1p(exp(dataInHalfToFloat0));\n\
            _viv_asm(CONV, res0, dataInHalfToFloat0);\n\
            VXC_DP4x4(dataInHalfToFloat1, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_high4);\n\
            dataInHalfToFloat1 = log1p(exp(dataInHalfToFloat1));\n\
            _viv_asm(CONV, res1, dataInHalfToFloat1);\n\
            VXC_DP2x8(dataInToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            _viv_asm(COPY, dataOut, dataInToHalf, 16);\n\
            VXC_WriteImage(dout, coord, dataOut, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
        } \n\
         __kernel void vxcActivationSoftRelu_Fp16toFp16(\n\
         __read_only image2d_t   din, \n\
         __write_only image2d_t  dout) \n\
         { \n\
            int2 coord = (int2)(get_global_id(0), get_global_id(1));\n\
            vxc_short8 dataIn;\n\
            vxc_half8 dataInToHalf;\n\
            float4 dataInHalfToFloat0, dataInHalfToFloat1;\n\
            half4 res0, res1;\n\
            VXC_ReadImage(dataIn, din, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(COPY, dataInToHalf, dataIn, 16);\n\
            VXC_DP4x4(dataInHalfToFloat0, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_low4);\n\
            dataInHalfToFloat0 = log1p(exp(dataInHalfToFloat0));\n\
            _viv_asm(CONV, res0, dataInHalfToFloat0);\n\
            VXC_DP4x4(dataInHalfToFloat1, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_high4);\n\
            dataInHalfToFloat1 = log1p(exp(dataInHalfToFloat1));\n\
            _viv_asm(CONV, res1, dataInHalfToFloat1);\n\
            VXC_DP2x8(dataInToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            _viv_asm(COPY, dataIn, dataInToHalf, 16);\n\
            VXC_WriteImage(dout, coord, dataIn, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         }\n\
         __kernel void vxcActivationSoftRelu_Fp16toInt8(\n\
         __read_only image2d_t   din, \n\
         __write_only image2d_t  dout) \n\
         { \n\
            int2 coord = (int2)(get_global_id(0), get_global_id(1));\n\
            vxc_char8 dataOut;\n\
            vxc_short8 dataIn;\n\
            vxc_half8 dataInToHalf;\n\
            float4 dataInHalfToFloat0, dataInHalfToFloat1;\n\
            half4 res0, res1;\n\
            half scaleOut;\n\
            VXC_ReadImage(dataIn, din, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(COPY, dataInToHalf, dataIn, 16);\n\
            _viv_asm(CONV, scaleOut, outputScale);\n\
            VXC_DP4x4(dataInHalfToFloat0, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_low4);\n\
            dataInHalfToFloat0 = log1p(exp(dataInHalfToFloat0));\n\
            _viv_asm(CONV, res0, dataInHalfToFloat0);\n\
            VXC_DP4x4(dataInHalfToFloat1, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_high4);\n\
            dataInHalfToFloat1 = log1p(exp(dataInHalfToFloat1));\n\
            _viv_asm(CONV, res1, dataInHalfToFloat1);\n\
            VXC_DP2x8(dataInToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(dataOut, dataInToHalf, scaleOut, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16MulFp16ToFp16_8x1);\n\
            VXC_WriteImage(dout, coord, dataOut, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcActivationSoftRelu_Int8toInt8(\n\
         __read_only image2d_t   din, \n\
         __write_only image2d_t  dout) \n\
         { \n\
            int2 coord = (int2)(get_global_id(0), get_global_id(1));\n\
            vxc_char8 dataIn;\n\
            vxc_half8 dataInToHalf;\n\
            float4 dataInHalfToFloat0, dataInHalfToFloat1;\n\
            half4 res0, res1;\n\
            half scaleIn, scaleOut;\n\
            VXC_ReadImage(dataIn, din, coord, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, scaleIn, inputScale);\n\
            _viv_asm(CONV, scaleOut, outputScale);\n\
            VXC_DP2x8(dataInToHalf, dataIn, scaleIn, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);\n\
            VXC_DP4x4(dataInHalfToFloat0, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_low4);\n\
            dataInHalfToFloat0 = log1p(exp(dataInHalfToFloat0));\n\
            _viv_asm(CONV, res0, dataInHalfToFloat0);\n\
            VXC_DP4x4(dataInHalfToFloat1, dataInToHalf, dataInToHalf, VXC_MODIFIER_BIN(0, 3, 0), fp16ToFp32_high4);\n\
            dataInHalfToFloat1 = log1p(exp(dataInHalfToFloat1));\n\
            _viv_asm(CONV, res1, dataInHalfToFloat1);\n\
            VXC_DP2x8(dataInToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(dataIn, dataInToHalf, scaleOut, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16MulFp16ToFp16_8x1);\n\
            VXC_WriteImage(dout, coord, dataIn, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         }\n"
        };

        programSources[0] = programSources_ActivationSoftRelu_0;
        programSources[1] = programSources_ActivationSoftRelu_0;
        programLength[0]  = strlen(programSources_ActivationSoftRelu_0);
        programLength[1]  = strlen(programSources_ActivationSoftRelu_0);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, programLength);

        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcActivationSoftRelu", program, 2, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
    {

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toInt8", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
    {

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toFp16", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
    {
        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toInt8", borderMode);
        if (!shaderExecutable) goto error;

        if (status != VX_SUCCESS) goto error;
    }
    else if(inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
    {
        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toFp16", borderMode);
        if (!shaderExecutable) goto error;

        if (status != VX_SUCCESS) goto error;
    }

    if (in_width * in_height > execution_parameters.globalWorkScale[0] * 8)
    {
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
    }
    else if (in_width * in_height > execution_parameters.globalWorkScale[0] * 4)
    {
        execution_parameters.localWorkSize[0]    = 4;
        execution_parameters.localWorkSize[1]    = 2;
    }
    else
    {
        execution_parameters.localWorkSize[0]    = 1;
        execution_parameters.localWorkSize[1]    = 8;
    }
    execution_parameters.globalWorkSize[0]   = gcmALIGN((in_width * in_height + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
    execution_parameters.globalWorkSize[1]   = gcmALIGN((depth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);

    status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16ToFp32_low4", 1, fp16ToFp32_low4);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16ToFp32_high4", 1, fp16ToFp32_high4);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "halfToVxcHalf_8", 1, halfToVxcHalf_8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16MulFp16ToFp16_8x1", 1, fp16MulFp16ToFp16_8x1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8MulFp16ToFp16_8x1", 1, int8MulFp16ToFp16_8x1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "inputScale", 1, &inputScale);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outputScale", 1, &outputScale);

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
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
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[3] = {(vx_reference)input0, (vx_reference)input1, (vx_reference)output};
    vx_enum       policyEnum    = convertPolicy->value->e;
    vx_uint32     width         = input0->tensorBuffer->memory.dims[0][0];
    vx_uint32     height        = input0->tensorBuffer->memory.dims[0][1];
    vx_uint32     depth         = input0->tensorBuffer->memory.dims[0][2];
    vx_enum       input0_format = input0->tensorBuffer->dataFormat;
    vx_enum       input1_format = input1->tensorBuffer->dataFormat;
    vx_enum       output_format = output->tensorBuffer->dataFormat;
    vx_float32    input_scale0  = 1.0f;
    vx_float32    input_scale1  = 1.0f;
    vx_float32    output_scale  = 1.0f;
    vx_float32    scaleIn0      = 1.0f;
    vx_float32    scaleIn1      = 1.0f;

    vx_uint32 UniFp16xFp16_dp2x8[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 UniFP16AddFP16_dp2x8[16] = {
        0x55555555, // TCfg
        0x44444444, // ASelt
        0x33221100, 0x77665544, // ABin
        0xaaaaaaaa, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00 // Constant
    };
    vx_uint32 UniS8xFp16toFp16_dp2x8[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };

    borderMode->mode = VX_BORDER_CONSTANT;

    if(input0_format == VX_TYPE_INT8)
    {
        vx_int8   input0_fixedPointPos    = input0->tensorBuffer->fixedPointPos;
        if (input0_fixedPointPos >= 0)
        {
            input_scale0 = 1.0f / (vx_float32) (1 << input0_fixedPointPos);
        }
        else if (input0_fixedPointPos < 0)
        {
            input_scale0 = (vx_float32) (1 << -input0_fixedPointPos);
        }
    }

    if(input1_format == VX_TYPE_INT8)
    {
        vx_int8   input1_fixedPointPos    = input1->tensorBuffer->fixedPointPos;
        if (input1_fixedPointPos >= 0)
        {
            input_scale1 = 1.0f / (vx_float32) (1 << input1_fixedPointPos);
        }
        else if (input1_fixedPointPos < 0)
        {
            input_scale1 = (vx_float32) (1 << -input1_fixedPointPos);
        }
    }

    if(output_format == VX_TYPE_INT8)
    {
        vx_int8   output_fixedPointPos    = output->tensorBuffer->fixedPointPos;
        if (output_fixedPointPos >= 0)
        {
            output_scale = (vx_float32) (1 << output_fixedPointPos);
        }
        else if (output_fixedPointPos < 0)
        {
            output_scale = 1.0f / (vx_float32) (1 << -output_fixedPointPos);
        }
    }

    if (input0_format == VX_TYPE_INT8 &&  input1_format ==  VX_TYPE_INT8)
    {
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
         _viv_uniform VXC_512Bits UniS8xFp16toFp16_dp2x8;\n\
         _viv_uniform VXC_512Bits UniFp16xFp16_dp2x8;\n\
         _viv_uniform float input_scale0;\n\
         _viv_uniform float input_scale1;\n\
         _viv_uniform float output_scale;\n\
         _viv_uniform float scaleIn0;\n\
         _viv_uniform float scaleIn1;\n\
         __kernel void vxcTensorAdd_Fp16AddInt8toInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
             int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
             \n\
             vxc_short8 src0; \n\
             vxc_half8 vec0, vec1; \n\
             vxc_char8 src1; \n\
             half scale1_fp16; \n\
             half scaleOut_fp16; \n\
             VXC_ReadImage2DArray(src0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             VXC_ReadImage2DArray(src1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec0, src0, 16); \n\
             _viv_asm(CONV, scale1_fp16, input_scale1); \n\
             _viv_asm(CONV, scaleOut_fp16, output_scale); \n\
             VXC_DP2x8(vec1, src1, scale1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniS8xFp16toFp16_dp2x8); \n\
             VXC_DP2x8(vec0, vec0, vec1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 1), UniFP16AddFP16_dp2x8); \n\
             VXC_DP2x8(src1, vec0, scaleOut_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16_dp2x8); \n\
             \n\
             VXC_WriteImage2DArray(output, coord, src1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
         } \n\
         __kernel void vxcTensorAdd_Fp16AddInt8toInt8Warp(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
             \n\
            vxc_short8 src0; \n\
            vxc_half8 vec0, vec1; \n\
            vxc_char8 src1; \n\
            half scale1_fp16; \n\
            half scaleOut_fp16; \n\
            VXC_ReadImage2DArray(src0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(src1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, vec0, src0, 16); \n\
            _viv_asm(CONV, scale1_fp16, input_scale1); \n\
            _viv_asm(CONV, scaleOut_fp16, output_scale); \n\
            VXC_DP2x8(vec1, src1, scale1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniS8xFp16toFp16_dp2x8); \n\
            VXC_DP2x8(vec0, vec0, vec1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 1), UniFP16AddFP16_dp2x8); \n\
            VXC_DP2x8(src1, vec0, scaleOut_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16_dp2x8); \n\
             \n\
            VXC_WriteImage2DArray(output, coord, src1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
        } \n\
         \n\
         __kernel void vxcTensorAdd_Int8AddInt8toInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
        {\n\
           int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
           \n\
           vxc_char8 in0, in1; \n\
           vxc_char8 dst; \n\
           half scale0_fp16, scale1_fp16; \n\
           vxc_half8 val0_fp16, val1_fp16; \n\
           VXC_ReadImage2DArray(in0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
           VXC_ReadImage2DArray(in1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
           \n\
           _viv_asm(CONV, scale0_fp16, scaleIn0); \n\
           _viv_asm(CONV, scale1_fp16, scaleIn1); \n\
           VXC_DP2x8(val0_fp16, in0, scale0_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
           VXC_DP2x8(val1_fp16, in1, scale1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
           VXC_DP2x8(dst, val0_fp16, val1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFP16AddFP16_dp2x8); \n\
           VXC_WriteImage2DArray(output, coord, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
        }\n\
        __kernel void vxcTensorAdd_Int8AddInt8toInt8Wrap(\n\
        __read_only  image2d_array_t input0, \n\
        __read_only  image2d_array_t input1, \n\
        __write_only image2d_array_t output) \n\
        {\n\
           int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
           \n\
           vxc_char8 in0, in1; \n\
           vxc_char8 dst; \n\
           half scale0_fp16, scale1_fp16; \n\
           vxc_half8 val0_fp16, val1_fp16; \n\
           VXC_ReadImage2DArray(in0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
           VXC_ReadImage2DArray(in1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
           \n\
           _viv_asm(CONV, scale0_fp16, input_scale0); \n\
           _viv_asm(CONV, scale1_fp16, input_scale1); \n\
           VXC_DP2x8(val0_fp16, in0, scale0_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
           VXC_DP2x8(val1_fp16, in1, scale1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
           VXC_DP2x8(dst, val0_fp16, val1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 0), UniFP16AddFP16_dp2x8); \n\
           VXC_WriteImage2DArray(output, coord, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
        }\n\
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
           vxc_char8 in0, in1; \n\
           half scale0_fp16, scale1_fp16; \n\
           vxc_half8 val0_fp16, val1_fp16; \n\
           vxc_short8 val_s16; \n\
           VXC_ReadImage2DArray(in0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
           VXC_ReadImage2DArray(in1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
           \n\
           _viv_asm(CONV, scale0_fp16, input_scale0); \n\
           _viv_asm(CONV, scale1_fp16, input_scale1); \n\
           VXC_DP2x8(val0_fp16, in0, scale0_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
           VXC_DP2x8(val1_fp16, in1, scale1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
           VXC_DP2x8(val0_fp16, val0_fp16, val1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniFP16AddFP16_dp2x8); \n\
           _viv_asm(COPY, val_s16, val0_fp16, 16); \n\
           VXC_WriteImage2DArray(output, coord, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
        }\n\
        __kernel void vxcTensorAdd_Int8Wrap(\n\
        __read_only  image2d_array_t input0, \n\
        __read_only  image2d_array_t input1, \n\
        __write_only image2d_array_t output) \n\
        {\n\
           int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
           \n\
           vxc_char8 in0, in1; \n\
           half scale0_fp16, scale1_fp16; \n\
           vxc_half8 val0_fp16, val1_fp16; \n\
           vxc_short8 val_s16; \n\
           VXC_ReadImage2DArray(in0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
           VXC_ReadImage2DArray(in1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
           \n\
           _viv_asm(CONV, scale0_fp16, input_scale0); \n\
           _viv_asm(CONV, scale1_fp16, input_scale1); \n\
           VXC_DP2x8(val0_fp16, in0, scale0_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
           VXC_DP2x8(val1_fp16, in1, scale1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
           VXC_DP2x8(val0_fp16, val0_fp16, val1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16AddFP16_dp2x8); \n\
           _viv_asm(COPY, val_s16, val0_fp16, 16); \n\
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
    else if(policyEnum == VX_CONVERT_POLICY_WRAP && input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8AddInt8toInt8Wrap", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (policyEnum == VX_CONVERT_POLICY_SATURATE && input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8AddInt8toInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if(policyEnum == VX_CONVERT_POLICY_WRAP && input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16AddInt8toInt8Wrap", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (policyEnum == VX_CONVERT_POLICY_SATURATE && input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16AddInt8toInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }

    scaleIn0 = input_scale0 * output_scale;
    scaleIn1 = input_scale1 * output_scale;

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto error;

    status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFP16AddFP16_dp2x8", 1, UniFP16AddFP16_dp2x8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "input_scale0", 1, &input_scale0);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "input_scale1", 1, &input_scale1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "output_scale", 1, &output_scale);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleIn0", 1, &scaleIn0);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleIn1", 1, &scaleIn1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniS8xFp16toFp16_dp2x8", 1, UniS8xFp16toFp16_dp2x8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFp16xFp16_dp2x8", 1, UniFp16xFp16_dp2x8);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcTensorSub****************************************************/
vxnne_shader_executable vxnneGetTensorSubShaderExecutable(
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
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[3] = {(vx_reference)input0, (vx_reference)input1, (vx_reference)output};
    vx_enum       policyEnum    = convertPolicy->value->e;
    vx_uint32     width         = input0->tensorBuffer->memory.dims[0][0];
    vx_uint32     height        = input0->tensorBuffer->memory.dims[0][1];
    vx_uint32     depth         = input0->tensorBuffer->memory.dims[0][2];
    vx_enum       input0_format = input0->tensorBuffer->dataFormat;
    vx_enum       input1_format = input1->tensorBuffer->dataFormat;
    vx_enum       output_format = output->tensorBuffer->dataFormat;
    vx_float32    input_scale0  = 1.0f;
    vx_float32    input_scale1  = 1.0f;
    vx_float32    output_scale  = 1.0f;

    vx_uint32 dp_fp16SubFp16[16] = {
        0x55555555, // TCfg
        0x44444444, // ASelt
        0x33221100, 0x77665544, // ABin
        0xaaaaaaaa, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00003400, // AccumType, ConstantType, and PostShift
        0xffff0001, 0xffff0001, 0xffff0001, 0xffff0001, 0xffff0001, 0xffff0001, 0xffff0001, 0xffff0001 // Constant
    };

    vx_uint32 dp_fp16MulFp16[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 dp_int8MulFp16[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 dp_fp16ToInt8 [16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x22222222, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
    };
    vx_uint32 fp16ToInt8_8[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x22222222, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
    };
    borderMode->mode = VX_BORDER_CONSTANT;

    if(input0_format == VX_TYPE_INT8)//2^(-fixedPointPos)
    {
        vx_int8   input0_fixedPointPos    = input0->tensorBuffer->fixedPointPos;
        if (input0_fixedPointPos >= 0)
        {
            input_scale0 = 1.0f / (vx_float32) (1 << input0_fixedPointPos);
        }
        else if (input0_fixedPointPos < 0)
        {
            input_scale0 = (vx_float32) (1 << -input0_fixedPointPos);
        }
    }
    else
        input_scale0 = 1.0f;

    if(input1_format == VX_TYPE_INT8)
    {
        vx_int8   input1_fixedPointPos    = input1->tensorBuffer->fixedPointPos;
        if (input1_fixedPointPos >= 0)
        {
            input_scale1 = 1.0f / (vx_float32) (1 << input1_fixedPointPos);
        }
        else if (input1_fixedPointPos < 0)
        {
            input_scale1 = (vx_float32) (1 << -input1_fixedPointPos);
        }
    }
    else
        input_scale1 = 1.0f;

    if(output_format == VX_TYPE_INT8)//2^(fixedPointPos)
    {
        vx_int8   output_fixedPointPos    = output->tensorBuffer->fixedPointPos;
        if (output_fixedPointPos >= 0)
        {
            output_scale = (vx_float32) (1 << output_fixedPointPos);
        }
        else if (output_fixedPointPos < 0)
        {
            output_scale = 1.0f / (vx_float32) (1 << -output_fixedPointPos);
        }
    }
    else
        output_scale = 1.0f;

    if (input0_format == VX_TYPE_INT8 &&  input1_format ==  VX_TYPE_INT8)
    {
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
         _viv_uniform VXC_512Bits dp_fp16SubFp16;\n\
         _viv_uniform VXC_512Bits dp_int8MulFp16;\n\
         _viv_uniform VXC_512Bits dp_fp16MulFp16;\n\
         _viv_uniform VXC_512Bits dp_fp16ToInt8;\n\
         _viv_uniform VXC_512Bits fp16ToInt8_8;\n\
         _viv_uniform float input_scale0;\n\
         _viv_uniform float input_scale1;\n\
         _viv_uniform float output_scale;\n\
         __kernel void vxcTensorSub_fp16SubFp16ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_short8 din0, din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_fp16SubFp16);\n\
            _viv_asm(COPY, din0, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorSub_fp16SubFp16ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_short8 din0, din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_half coeff2;\n\
            vxc_char8 halfToChar;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_fp16SubFp16);\n\
            VXC_DP2x8(halfToChar, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16ToInt8_8);\n\
            VXC_DP2x8(halfToChar, halfToChar, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), dp_int8MulFp16);\n\
            VXC_WriteImage2DArray(output, coord, halfToChar, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorSub_int8SubFp16ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0;\n\
            vxc_short8 din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_half coeff0;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_fp16SubFp16);\n\
            _viv_asm(COPY, din1, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, din1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorSub_int8SubFp16ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0;\n\
            vxc_short8 din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_half coeff0, coeff2;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_fp16SubFp16);\n\
            VXC_DP2x8(din0, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16ToInt8_8);\n\
            VXC_DP2x8(din0, din0, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), dp_int8MulFp16);\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorSub_fp16SubInt8ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din1;\n\
            vxc_short8 din0;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_half coeff1;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            VXC_DP2x8(din0ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_fp16SubFp16);\n\
            _viv_asm(COPY, din0, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorSub_fp16SubInt8ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din1;\n\
            vxc_short8 din0;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_half coeff1, coeff2;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_fp16SubFp16);\n\
            VXC_DP2x8(din1, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16ToInt8_8);\n\
            VXC_DP2x8(din1, din1, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), dp_int8MulFp16);\n\
            VXC_WriteImage2DArray(output, coord, din1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorSub_int8SubInt8ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0, din1;\n\
            vxc_half coeff0, coeff1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_short8 halfToShort;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_fp16SubFp16);\n\
            _viv_asm(COPY, halfToShort, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, halfToShort, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorSub_int8SubInt8ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0, din1;\n\
            vxc_half coeff0, coeff1, coeff2;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_fp16SubFp16);\n\
            VXC_DP2x8(din0, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), dp_fp16MulFp16);\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorSub_int8SubInt8ToInt8Wrap(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0, din1;\n\
            vxc_half coeff0, coeff1, coeff2;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), dp_int8MulFp16);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), dp_fp16SubFp16);\n\
            VXC_DP2x8(din0, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), dp_fp16ToInt8);\n\
            VXC_DP2x8(din0, din0, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 0), dp_int8MulFp16);\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n"
        };

        programLength = strlen(programSources[0]);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, &programLength);
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcTensorSub", program, 3, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16 && policyEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16SubFp16ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_INT8 && policyEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16SubFp16ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16 && policyEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8SubFp16ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_INT8 && policyEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8SubFp16ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_FLOAT16 && policyEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16SubInt8ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8 && policyEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16SubInt8ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_FLOAT16 && policyEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8SubInt8ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8 && policyEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8SubInt8ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (policyEnum == VX_CONVERT_POLICY_WRAP && input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8SubInt8ToInt8Wrap", borderMode);
        if (!shaderExecutable) goto error;
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "input_scale0", 1, &input_scale0);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "input_scale1", 1, &input_scale1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "output_scale", 1, &output_scale);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "dp_fp16SubFp16", 1, dp_fp16SubFp16);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "dp_int8MulFp16", 1, dp_int8MulFp16);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "dp_fp16MulFp16", 1, dp_fp16MulFp16);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "dp_fp16ToInt8", 1, dp_fp16ToInt8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16ToInt8_8", 1, fp16ToInt8_8);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcTensorMul****************************************************/
vxnne_shader_executable vxnneGetTensorMulShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               scale,
    vx_scalar               overflow,
    vx_scalar               rounding,
    vx_tensor               output)
{
    vx_size    programLength = 0;
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[3] = {(vx_reference)input0, (vx_reference)input1, (vx_reference)output};
    vx_enum       overflowEnum    = overflow->value->e;
    float scaleValue = scale->value->f32;
    //vx_enum       roundingEnum = rounding->value->e;
    vx_uint32     width         = input0->tensorBuffer->memory.dims[0][0];
    vx_uint32     height        = input0->tensorBuffer->memory.dims[0][1];
    vx_uint32     depth         = input0->tensorBuffer->memory.dims[0][2];
    vx_enum       input0_format = input0->tensorBuffer->dataFormat;
    vx_enum       input1_format = input1->tensorBuffer->dataFormat;
    vx_enum       output_format = output->tensorBuffer->dataFormat;
    vx_float32    input_scale0  = 1.0f;
    vx_float32    input_scale1  = 1.0f;
    vx_float32    output_scale  = 1.0f;

    vx_uint32 int8MulFp16ToFp16_8x1[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };

     vx_uint32 fp16MulFp16ToFp16_8x8[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x03020100, 0x07060504, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };

     vx_uint32 fp16MulFp16ToFp16_8x1[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 fp16ToInt8_8[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x22222222, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
    };

    borderMode->mode = VX_BORDER_CONSTANT;

    if(input0_format == VX_TYPE_INT8)//2^(-fixedPointPos)
    {
        vx_int8 input0_fixedPointPos;
        input0_fixedPointPos = input0->tensorBuffer->fixedPointPos;

        if (input0_fixedPointPos >= 0)
            input_scale0 = 1.0f / (vx_float32) (1 << input0_fixedPointPos);
        else
            input_scale0 = (vx_float32) (1 << -input0_fixedPointPos);
    }
    else
        input_scale0 = 1.0f;

    if(input1_format == VX_TYPE_INT8)
    {
        vx_int8 input1_fixedPointPos;
        input1_fixedPointPos = input1->tensorBuffer->fixedPointPos;

        if (input1_fixedPointPos >= 0)
            input_scale1 = 1.0f / (vx_float32) (1 << input1_fixedPointPos);
        else
            input_scale1 = (vx_float32) (1 << -input1_fixedPointPos);
    }
    else
        input_scale1 = 1.0f;

    if(output_format == VX_TYPE_INT8)//2^(fixedPointPos)
    {
        vx_int8   output_fixedPointPos    = output->tensorBuffer->fixedPointPos;
        if (output_fixedPointPos >= 0)
        {
            output_scale = (vx_float32) (1 << output_fixedPointPos);
        }
        else
        {
            output_scale = 1.0f / (vx_float32) (1 << -output_fixedPointPos);
        }
    }
    else
        output_scale = 1.0f;

    if (input0_format == VX_TYPE_INT8 &&  input1_format ==  VX_TYPE_INT8)
    {
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
         _viv_uniform float input_scale0;\n\
         _viv_uniform float input_scale1;\n\
         _viv_uniform float output_scale;\n\
         _viv_uniform float scaleValue;\n\
         _viv_uniform VXC_512Bits int8MulFp16ToFp16_8x1;\n\
         _viv_uniform VXC_512Bits fp16MulFp16ToFp16_8x8;\n\
         _viv_uniform VXC_512Bits fp16MulFp16ToFp16_8x1;\n\
         _viv_uniform VXC_512Bits fp16ToInt8_8;\n\
         __kernel void vxcTensorMul_fp16MulFp16ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_short8 din0, din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_half scale;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            _viv_asm(COPY, din0, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorMul_fp16MulFp16ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_short8 din0, din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_half coeff2, scale;\n\
            vxc_char8 halfToChar;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            VXC_DP2x8(halfToChar, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16ToInt8_8);//overflow\n\
            VXC_DP2x8(halfToChar, halfToChar, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), int8MulFp16ToFp16_8x1);//round\n\
            VXC_WriteImage2DArray(output, coord, halfToChar, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorMul_int8MulFp16ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0;\n\
            vxc_short8 din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_half coeff0, scale;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            _viv_asm(COPY, din1, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, din1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorMul_int8MulFp16ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0;\n\
            vxc_short8 din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_half coeff0, coeff2, scale;\n\
            vxc_char8 halfToChar;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            VXC_DP2x8(halfToChar, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16ToInt8_8);//overflow\n\
            VXC_DP2x8(halfToChar, halfToChar, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), int8MulFp16ToFp16_8x1);//round\n\
            VXC_WriteImage2DArray(output, coord, halfToChar, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorMul_int8MulInt8ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0, din1;\n\
            vxc_half scale;\n\
            vxc_half coeff0, coeff1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            vxc_short8 halfToShort;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);//mul\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            _viv_asm(COPY, halfToShort, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, halfToShort, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorMul_int8MulInt8ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0, din1;\n\
            vxc_half scale;\n\
            vxc_half coeff0, coeff1, coeff2;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);//mul\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            VXC_DP2x8(din0, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16ToInt8_8);//overflow\n\
            VXC_DP2x8(din0, din0, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), int8MulFp16ToFp16_8x1);//scale\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorMul_int8MulInt8ToInt8Wrap(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0, din1;\n\
            vxc_half scale;\n\
            vxc_half coeff0, coeff1, coeff2;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16^^\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16^^\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);//mul^^\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale^^\n\
            VXC_DP2x8(din0, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 0), fp16ToInt8_8);//round, overflow\n\
            VXC_DP2x8(din0, din0, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), int8MulFp16ToFp16_8x1);//scale^^\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n"
        };

        programLength = strlen(programSources[0]);
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, &programLength);
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcTensorMul", program, 3, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16MulFp16ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_INT8 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16MulFp16ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8MulFp16ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_INT8 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8MulFp16ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_FLOAT16 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8MulInt8ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8MulInt8ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (overflowEnum == VX_CONVERT_POLICY_WRAP && input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8MulInt8ToInt8Wrap", borderMode);
        if (!shaderExecutable) goto error;
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "input_scale0", 1, &input_scale0);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "input_scale1", 1, &input_scale1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "output_scale", 1, &output_scale);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleValue", 1, &scaleValue);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8MulFp16ToFp16_8x1", 1, int8MulFp16ToFp16_8x1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16MulFp16ToFp16_8x8", 1, fp16MulFp16ToFp16_8x8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16MulFp16ToFp16_8x1", 1, fp16MulFp16ToFp16_8x1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16ToInt8_8", 1, fp16ToInt8_8);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}
/********vxcTensorDiv****************************************************/
vxnne_shader_executable vxnneGetTensorDivShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input0,
    vx_tensor               input1,
    vx_scalar               scale,
    vx_scalar               overflow,
    vx_scalar               rounding,
    vx_tensor               output)
{
    vx_size    programLength[2] = {0, 0};
    char *programSource[2] = {NULL, NULL};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference  parameters[3] = {(vx_reference)input0, (vx_reference)input1, (vx_reference)output};
    vx_enum       overflowEnum    = overflow->value->e;
    float scaleValue = scale->value->f32;
    //vx_enum       roundingEnum = rounding->value->e;
    vx_uint32     width         = input0->tensorBuffer->memory.dims[0][0];
    vx_uint32     height        = input0->tensorBuffer->memory.dims[0][1];
    vx_uint32     depth         = input0->tensorBuffer->memory.dims[0][2];
    vx_enum       input0_format = input0->tensorBuffer->dataFormat;
    vx_enum       input1_format = input1->tensorBuffer->dataFormat;
    vx_enum       output_format = output->tensorBuffer->dataFormat;
    vx_float32    input_scale0  = 1.0f;
    vx_float32    input_scale1  = 1.0f;
    vx_float32    output_scale  = 1.0f;

    vx_uint32 int8MulFp16ToFp16_8x1[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };

    vx_uint32 fp16MulFp16ToFp16_8x1[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x11111111, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000400, // AccumType, ConstantType, and PostShift
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
    };
    vx_uint32 fp16ToInt8_8[16] = {
        0x11111111, // TCfg
        0x00000000, // ASelt
        0x03020100, 0x07060504, // ABin
        0x22222222, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
    };
    vx_uint32 fp16ToFp32_high4[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00050004, 0x00070006, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
    vx_uint32 fp16ToFp32_low4[16] = {
        0x01010101, // TCfg
        0x00000000, // ASelt
        0x00010000, 0x00030002, // ABin
        0x02020202, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
    };
    vx_uint32 halfToVxcHalf_8[16] = {
        0x11111111, // TCfg
        0x11110000, // ASelt
        0x06040200, 0x06040200, // ABin
        0x22222222, // BSelt
        0x00000000, 0x00000000, // BBin
        0x00000100, // AccumType, ConstantType, and PostShift
        0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
    };
    borderMode->mode = VX_BORDER_CONSTANT;

    if(input0_format == VX_TYPE_INT8)//2^(-fixedPointPos)
    {
        vx_int8 input0_fixedPointPos;
        input0_fixedPointPos = input0->tensorBuffer->fixedPointPos;

        if (input0_fixedPointPos >= 0)
            input_scale0 = 1.0f / (vx_float32) (1 << input0_fixedPointPos);
        else
            input_scale0 = (vx_float32) (1 << -input0_fixedPointPos);
    }
    else
        input_scale0 = 1.0f;

    if(input1_format == VX_TYPE_INT8)
    {
        vx_int8 input1_fixedPointPos;
        input1_fixedPointPos = input1->tensorBuffer->fixedPointPos;

        if (input1_fixedPointPos >= 0)
            input_scale1 = 1.0f / (vx_float32) (1 << input1_fixedPointPos);
        else
            input_scale1 = (vx_float32) (1 << -input1_fixedPointPos);
    }
    else
        input_scale1 = 1.0f;

    if(output_format == VX_TYPE_INT8)//2^(fixedPointPos)
    {
        vx_int8   output_fixedPointPos    = output->tensorBuffer->fixedPointPos;
        if (output_fixedPointPos >= 0)
        {
            output_scale = (vx_float32) (1 << output_fixedPointPos);
        }
        else
        {
            output_scale = 1.0f / (vx_float32) (1 << -output_fixedPointPos);
        }
    }
    else
        output_scale = 1.0f;

    if (input0_format == VX_TYPE_INT8 &&  input1_format ==  VX_TYPE_INT8)
    {
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
        char *programSource0 =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         \n\
         _viv_uniform float input_scale0;\n\
         _viv_uniform float input_scale1;\n\
         _viv_uniform float output_scale;\n\
         _viv_uniform float scaleValue;\n\
         _viv_uniform VXC_512Bits int8MulFp16ToFp16_8x1;\n\
         _viv_uniform VXC_512Bits fp16MulFp16ToFp16_8x1;\n\
         _viv_uniform VXC_512Bits fp16ToInt8_8;\n\
         _viv_uniform VXC_512Bits fp16ToFp32_low4;\n\
         _viv_uniform VXC_512Bits fp16ToFp32_high4;\n\
         _viv_uniform VXC_512Bits halfToVxcHalf_8;\n\
         __kernel void vxcTensorDiv_fp16DivFp16ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_short8 din0, din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            float4 halfToFloat0, halfToFloat1;\n\
            half4 res0, res1;\n\
            vxc_half scale;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            //VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res0, halfToFloat0);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res1, halfToFloat0);\n\
            VXC_DP2x8(din0ToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            _viv_asm(COPY, din0, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorDiv_fp16DivFp16ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_short8 din0, din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            float4 halfToFloat0, halfToFloat1;\n\
            half4 res0, res1;\n\
            vxc_half coeff2, scale;\n\
            vxc_char8 halfToChar;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            //VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res0, halfToFloat0);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res1, halfToFloat0);\n\
            VXC_DP2x8(din0ToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            VXC_DP2x8(halfToChar, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16ToInt8_8);//overflow\n\
            VXC_DP2x8(halfToChar, halfToChar, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), int8MulFp16ToFp16_8x1);//round\n\
            VXC_WriteImage2DArray(output, coord, halfToChar, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorDiv_fp16DivInt8ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din1;\n\
            vxc_short8 din0;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            float4 halfToFloat0, halfToFloat1;\n\
            half4 res0, res1;\n\
            vxc_half coeff1, scale;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            //VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res0, halfToFloat0);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res1, halfToFloat0);\n\
            VXC_DP2x8(din0ToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            _viv_asm(COPY, din0, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorDiv_fp16DivInt8ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din1;\n\
            vxc_short8 din0;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            float4 halfToFloat0, halfToFloat1;\n\
            half4 res0, res1;\n\
            vxc_half coeff1, coeff2, scale;\n\
            vxc_char8 halfToChar;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            _viv_asm(COPY, din0ToHalf, din0, 16);\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            //VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res0, halfToFloat0);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res1, halfToFloat0);\n\
            VXC_DP2x8(din0ToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            VXC_DP2x8(halfToChar, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16ToInt8_8);//overflow\n\
            VXC_DP2x8(halfToChar, halfToChar, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), int8MulFp16ToFp16_8x1);//round\n\
            VXC_WriteImage2DArray(output, coord, halfToChar, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n"
         };
         char *programSource1 = {"\n\
         __kernel void vxcTensorDiv_int8DivFp16ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0;\n\
            vxc_short8 din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            float4 halfToFloat0, halfToFloat1;\n\
            half4 res0, res1;\n\
            vxc_half coeff0, scale;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            //VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res0, halfToFloat0);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res1, halfToFloat0);\n\
            VXC_DP2x8(din0ToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            _viv_asm(COPY, din1, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, din1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorDiv_int8DivFp16ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0;\n\
            vxc_short8 din1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            float4 halfToFloat0, halfToFloat1;\n\
            half4 res0, res1;\n\
            vxc_half coeff0, coeff2, scale;\n\
            vxc_char8 halfToChar;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            _viv_asm(COPY, din1ToHalf, din1, 16);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            //VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res0, halfToFloat0);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res1, halfToFloat0);\n\
            VXC_DP2x8(din0ToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            VXC_DP2x8(halfToChar, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16ToInt8_8);//overflow\n\
            VXC_DP2x8(halfToChar, halfToChar, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), int8MulFp16ToFp16_8x1);//round\n\
            VXC_WriteImage2DArray(output, coord, halfToChar, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorDiv_int8DivInt8ToFp16Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0, din1;\n\
            vxc_half scale;\n\
            vxc_half coeff0, coeff1;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            float4 halfToFloat0, halfToFloat1;\n\
            half4 res0, res1;\n\
            vxc_short8 halfToShort;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            //VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);//mul\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res0, halfToFloat0);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res1, halfToFloat0);\n\
            VXC_DP2x8(din0ToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            _viv_asm(COPY, halfToShort, din0ToHalf, 16);\n\
            VXC_WriteImage2DArray(output, coord, halfToShort, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n\
         __kernel void vxcTensorDiv_int8DivInt8ToInt8Saturate(\n\
         __read_only  image2d_array_t input0, \n\
         __read_only  image2d_array_t input1, \n\
         __write_only image2d_array_t output) \n\
         { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);\n\
            vxc_char8 din0, din1;\n\
            vxc_half scale;\n\
            vxc_half coeff0, coeff1, coeff2;\n\
            vxc_half8 din0ToHalf, din1ToHalf;\n\
            float4 halfToFloat0, halfToFloat1;\n\
            half4 res0, res1;\n\
            VXC_ReadImage2DArray(din0, input0, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            VXC_ReadImage2DArray(din1, input1, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
            _viv_asm(CONV, coeff0, input_scale0);\n\
            _viv_asm(CONV, coeff1, input_scale1);\n\
            _viv_asm(CONV, coeff2, output_scale);\n\
            _viv_asm(CONV, scale, scaleValue);\n\
            VXC_DP2x8(din0ToHalf, din0, coeff0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            VXC_DP2x8(din1ToHalf, din1, coeff1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), int8MulFp16ToFp16_8x1);//in8->fp16\n\
            //VXC_DP2x8(din0ToHalf, din0ToHalf, din1ToHalf, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x8);//mul\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_low4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res0, halfToFloat0);\n\
            VXC_DP4x4(halfToFloat0, din0ToHalf, din0ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            VXC_DP4x4(halfToFloat1, din1ToHalf, din1ToHalf, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), fp16ToFp32_high4);\n\
            halfToFloat0 = halfToFloat0 / halfToFloat1;\n\
            _viv_asm(CONV, res1, halfToFloat0);\n\
            VXC_DP2x8(din0ToHalf, res0, res1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), halfToVxcHalf_8);\n\
            VXC_DP2x8(din0ToHalf, din0ToHalf, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), fp16MulFp16ToFp16_8x1);//scale\n\
            VXC_DP2x8(din0, din0ToHalf, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), fp16ToInt8_8);//overflow\n\
            VXC_DP2x8(din0, din0, coeff2, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), int8MulFp16ToFp16_8x1);//rounding\n\
            VXC_WriteImage2DArray(output, coord, din0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));\n\
         } \n"
        };

        programSource[0] = programSource0;
        programSource[1] = programSource1;
        programLength[0] = strlen(programSource0);
        programLength[1] = strlen(programSource1);
        program = vxCreateProgramWithSource(context, 2, (const vx_char**)programSource, programLength);
        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcTensorDiv", program, 3, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16DivFp16ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_INT8 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16DivFp16ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_FLOAT16 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8DivFp16ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_FLOAT16 && output_format == VX_TYPE_INT8 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8DivFp16ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_FLOAT16 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16DivInt8ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_FLOAT16 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_fp16DivInt8ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_FLOAT16 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8DivInt8ToFp16Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8 && overflowEnum == VX_CONVERT_POLICY_SATURATE)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8DivInt8ToInt8Saturate", borderMode);
        if (!shaderExecutable) goto error;
    }
    else if (overflowEnum == VX_CONVERT_POLICY_WRAP && input0_format == VX_TYPE_INT8 && input1_format == VX_TYPE_INT8 && output_format == VX_TYPE_INT8)
    {
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_int8DivInt8ToInt8Wrap", borderMode);
        if (!shaderExecutable) goto error;
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetUniform(shaderExecutable, "input_scale0", 1, &input_scale0);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "input_scale1", 1, &input_scale1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "output_scale", 1, &output_scale);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleValue", 1, &scaleValue);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8MulFp16ToFp16_8x1", 1, int8MulFp16ToFp16_8x1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16MulFp16ToFp16_8x1", 1, fp16MulFp16ToFp16_8x1);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16ToInt8_8", 1, fp16ToInt8_8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16ToFp32_low4", 1, fp16ToFp32_low4);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16ToFp32_high4", 1, fp16ToFp32_high4);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "halfToVxcHalf_8", 1, halfToVxcHalf_8);
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
    vx_uint32       UniS8xFp16toFp16_dp2x8[16] = {
                        0x11111111, // TCfg
                        0x00000000, // ASelt
                        0x03020100, 0x07060504, // ABin
                        0x11111111, // BSelt
                        0x00000000, 0x00000000, // BBin
                        0x00000400, // AccumType, ConstantType, and PostShift
                        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
                    };
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference    parameters[3]              = {(vx_reference)input, (vx_reference)alpha, (vx_reference)output};
    vx_enum         inputFormat                = input->tensorBuffer->dataFormat;
    vx_enum         outputFormat               = output->tensorBuffer->dataFormat;
    vx_int8         input_fractionLengthValue  = input->tensorBuffer->fixedPointPos;
    vx_int8         output_fractionLengthValue = output->tensorBuffer->fixedPointPos;
    vx_uint32       elementCount1              = 0;
    vx_uint32       elementCount2              = 0;
    vx_image        imgInput                   = NULL;
    vx_image        imgOutput                  = NULL;
    vx_float32      in_scale                   = 1.0f;
    vx_float32      out_scale                  = 1.0f;

    if ((inputFormat != VX_TYPE_FLOAT16 && inputFormat != VX_TYPE_INT8) || (outputFormat != VX_TYPE_FLOAT16 && outputFormat != VX_TYPE_INT8))
    {
        gcmPRINT("input or output's format is not support");
        goto error;
    }
    status = vxQueryTensor(input, VX_TENSOR_DIMS, input_size, sizeof(input_size));
    if (status != VX_SUCCESS) goto error;
    elementCount1 = input_size[0] * input_size[1];
    elementCount2 = input_size[0] * input_size[1] * input_size[2];

    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkOffset[2] = 0;
    execution_parameters.globalWorkScale[0]  = 8;
    execution_parameters.globalWorkScale[1]  = 1;
    execution_parameters.globalWorkScale[2]  = 1;
    if (elementCount2 < IMG_MAX_WIDTH)
    {
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.localWorkSize[2]    = 1;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((elementCount2 + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = 1;
        execution_parameters.globalWorkSize[2]   = 1;
    }
    else if ((elementCount2 >= IMG_MAX_WIDTH) && (elementCount1 < IMG_MAX_WIDTH))
    {
        execution_parameters.localWorkSize[0]    = 2;
        execution_parameters.localWorkSize[1]    = 4;
        execution_parameters.localWorkSize[2]    = 1;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((elementCount1 + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((input_size[2] + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
        execution_parameters.globalWorkSize[2]   = 1;
    }
    else
    {
        execution_parameters.localWorkSize[0]    = 2;
        execution_parameters.localWorkSize[1]    = 4;
        execution_parameters.localWorkSize[2]    = 1;
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
            __kernel void vxcLeakyRelu_Fp16toFp16Tensor(\n\
                      __read_only image2d_array_t   input, \n\
                                            float   alpha, \n\
                     __write_only image2d_array_t   output)\n\
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
                _viv_asm(COPY, img_s16, img_fp16, 16); \n\
                VXC_WriteImage2DArray(output, coord, img_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            } \n\
            __kernel void vxcLeakyRelu_Fp16toInt8Tensor(\n\
                      __read_only image2d_array_t   input, \n\
                                            float   alpha, \n\
                     __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                half alpha_fp16, scale_fp16; \n\
                vxc_short8 img_s16; \n\
                vxc_half8 img_fp16, val_fp16; \n\
                vxc_char8 val_s8; \n\
                \n\
                VXC_ReadImage2DArray(img_s16, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(CONV, alpha_fp16, alpha); \n\
                _viv_asm(COPY, img_fp16, img_s16, 16); \n\
                VXC_DP2x8(val_fp16, img_fp16, alpha_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16Mul_dp2x8); \n\
                VXC_Clamp(img_fp16, img_fp16, val_fp16, img_fp16, VXC_MODIFIER_CLAMP(0, 7, 0, 0)); \n\
                _viv_asm(CONV, scale_fp16, out_scale); \n\
                VXC_DP2x8(val_s8, img_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFP16Mul_dp2x8); \n\
                VXC_WriteImage2DArray(output, coord, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            } \n\
            __kernel void vxcLeakyRelu_Fp16toFp16Image(\n\
                     __read_only image2d_t   input, \n\
                                     float   alpha, \n\
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
                _viv_asm(COPY, img_s16, img_fp16, 16); \n\
                VXC_WriteImage(output, coord, img_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n\
            __kernel void vxcLeakyRelu_Fp16toInt8Image(\n\
                     __read_only image2d_t   input, \n\
                                     float   alpha, \n\
                    __write_only image2d_t   output)\n\
            {\n\
                int2 coord = (int2)(get_global_id(0), get_global_id(1)); \n\
                \n\
                half alpha_fp16,scale_fp16; \n\
                vxc_short8 img_s16; \n\
                vxc_half8 img_fp16, val_fp16; \n\
                vxc_char8 val_s8; \n\
                \n\
                VXC_ReadImage(img_s16, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(CONV, alpha_fp16, alpha); \n\
                _viv_asm(COPY, img_fp16, img_s16, 16); \n\
                VXC_DP2x8(val_fp16, img_fp16, alpha_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16Mul_dp2x8); \n\
                VXC_Clamp(img_fp16, img_fp16, val_fp16, img_fp16, VXC_MODIFIER_CLAMP(0, 7, 0, 0)); \n\
                _viv_asm(CONV, scale_fp16, out_scale); \n\
                VXC_DP2x8(val_s8, img_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFP16Mul_dp2x8); \n\
                VXC_WriteImage(output, coord, val_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n\
            __kernel void vxcLeakyRelu_Int8toFp16Tensor(\n\
                      __read_only image2d_array_t   input, \n\
                                            float   alpha, \n\
                     __write_only image2d_array_t   output)\n\
            {\n\
                int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
                \n\
                vxc_char8 img_s8; \n\
                vxc_half8 img_fp16, val_fp16; \n\
                half scale_fp16, alpha_fp16; \n\
                vxc_short8 val_s16; \n\
                \n\
                VXC_ReadImage2DArray(img_s8, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(CONV, scale_fp16, in_scale); \n\
                _viv_asm(CONV, alpha_fp16, alpha); \n\
                \n\
                VXC_DP2x8(img_fp16, img_s8, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                VXC_DP2x8(val_fp16, img_fp16, alpha_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16Mul_dp2x8); \n\
                VXC_Clamp(img_fp16, img_fp16, val_fp16, img_fp16, VXC_MODIFIER_CLAMP(0, 7, 0, 0)); \n\
                _viv_asm(COPY, val_s16, img_fp16, 16); \n\
                VXC_WriteImage2DArray(output, coord, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            } \n\
            __kernel void vxcLeakyRelu_Int8toInt8Tensor(\n\
                      __read_only image2d_array_t   input, \n\
                                            float   alpha, \n\
                     __write_only image2d_array_t   output)\n\
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
                _viv_asm(CONV, scale_fp16, out_scale); \n\
                VXC_DP2x8(img_s8, img_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFP16Mul_dp2x8); \n\
                VXC_WriteImage2DArray(output, coord, img_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            } \n\
            __kernel void vxcLeakyRelu_Int8toFp16Image(\n\
                     __read_only image2d_t    input, \n\
                                     float    alpha, \n\
                    __write_only image2d_t   output)\n\
            {\n\
                int2 coord = (int2)(get_global_id(0), get_global_id(1)); \n\
                \n\
                vxc_char8 img_s8; \n\
                vxc_half8 img_fp16, val_fp16; \n\
                half scale_fp16, alpha_fp16; \n\
                vxc_short8 val_s16; \n\
                \n\
                VXC_ReadImage(img_s8, input, coord, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(CONV, scale_fp16, in_scale); \n\
                _viv_asm(CONV, alpha_fp16, alpha); \n\
                \n\
                VXC_DP2x8(img_fp16, img_s8, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16toFp16_dp2x8); \n\
                VXC_DP2x8(val_fp16, img_fp16, alpha_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniFP16Mul_dp2x8); \n\
                VXC_Clamp(img_fp16, img_fp16, val_fp16, img_fp16, VXC_MODIFIER_CLAMP(0, 7, 0, 0)); \n\
                _viv_asm(COPY, val_s16, img_fp16, 16); \n\
                VXC_WriteImage(output, coord, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            }\n\
            __kernel void vxcLeakyRelu_Int8toInt8Image(\n\
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
                _viv_asm(CONV, scale_fp16, out_scale); \n\
                VXC_DP2x8(img_s8, img_fp16, scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFP16Mul_dp2x8); \n\
                VXC_WriteImage(output, coord, img_s8, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
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

    if ((elementCount1 < IMG_MAX_WIDTH) || (elementCount2 < IMG_MAX_WIDTH))
    {
        if (elementCount2 < IMG_MAX_WIDTH)
        {
            if (inputFormat == VX_TYPE_FLOAT16)
                imgInput = vxoTensor_CreateImageFromTensor(input, elementCount2, 1, VX_DF_IMAGE_S16);
            else
                imgInput = vxoTensor_CreateImageFromTensor(input, elementCount2, 1, VX_DF_IMAGE_U8);
            if (outputFormat == VX_TYPE_FLOAT16)
                imgOutput = vxoTensor_CreateImageFromTensor(output, elementCount2, 1, VX_DF_IMAGE_S16);
            else
                imgOutput = vxoTensor_CreateImageFromTensor(output, elementCount2, 1, VX_DF_IMAGE_U8);
        }
        else
        {
            if (inputFormat == VX_TYPE_FLOAT16)
                imgInput = vxoTensor_CreateImageFromTensor(input, elementCount1, input_size[2], VX_DF_IMAGE_S16);
            else
                imgInput = vxoTensor_CreateImageFromTensor(input, elementCount1, input_size[2], VX_DF_IMAGE_U8);
            if (outputFormat == VX_TYPE_FLOAT16)
                imgOutput = vxoTensor_CreateImageFromTensor(output, elementCount1, input_size[2], VX_DF_IMAGE_S16);
            else
                imgOutput = vxoTensor_CreateImageFromTensor(output, elementCount1, input_size[2], VX_DF_IMAGE_U8);
        }
        parameters[0] = (vx_reference)imgInput;
        parameters[2] = (vx_reference)imgOutput;

        if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toFp16Image", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toInt8Image", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toFp16Image", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toInt8Image", borderMode);
            if (!shaderExecutable) goto error;
        }
        else
        {
            gcmPRINT("input or output's format is not support");
            goto error;
        }
    }
    else
    {
        if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toFp16Tensor", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toInt8Tensor", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toFp16Tensor", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8toInt8Tensor", borderMode);
            if (!shaderExecutable) goto error;
        }
        else
        {
            gcmPRINT("input or output's format is not support");
            goto error;
        }
    }

    status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFP16Mul_dp2x8", 1, UniFP16Mul_dp2x8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniS8xFp16toFp16_dp2x8", 1, UniS8xFp16toFp16_dp2x8);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "in_scale", 1, &in_scale);
    status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "out_scale", 1, &out_scale);
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

    vx_scalar     dRelu_s        = NULL;
    vx_int32      dRelu          = 0;
    vx_enum       output_format  = VX_TYPE_FLOAT16;
    vx_enum       input_format   = VX_TYPE_FLOAT16;
    vx_enum       weights_format = VX_TYPE_FLOAT16;
    vx_enum       bias_format    = VX_TYPE_FLOAT32;
    vx_uint32     input_size[4]  = {0, 0, 0, 0};
    vx_uint32     weight_size[4] = {0, 0, 0, 0};
    vx_uint32     bias_size[4]   = {0, 0, 0, 0};
    vx_uint32     output_size[4] = {0, 0, 0, 0};
    vx_uint32     input_width    = 0;

    vx_uint32 uniMulAcc[16] = {
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
    vx_image   imgInput   = NULL;
    vx_image   imgWeights = NULL;
    vx_image   imgBias    = NULL;
    vx_float32 in_scale   = 0;

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
        vx_int8   output_fixedPointPos    = output->tensorBuffer->fixedPointPos;
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
            multiplicator = multiplicator + output_fixedPointPos;
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
                char val = convert_char_sat_rte(dst); \n\
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
    vx_size    programLength[4] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    vx_reference parameters[3]      = {(vx_reference)input, VX_NULL, (vx_reference)output};
    vx_enum      inputFormat        = input->tensorBuffer->dataFormat;
    vx_enum      outputFormat       = output->tensorBuffer->dataFormat;
    vx_uint32    in_width           = input->tensorBuffer->memory.dims[0][1];
    vx_uint32    in_height          = input->tensorBuffer->memory.dims[0][1];
    vx_uint32    depth              = input->tensorBuffer->memory.dims[0][2];
    vx_uint32    out_width          = output->tensorBuffer->memory.dims[0][0];
    vx_uint32    out_height         = output->tensorBuffer->memory.dims[0][1];
    vx_uint32    stride_v           = stride_s->value->u32;
    vx_uint32    kernel_v           = poolSizeX->value->u32;
    vx_uint32    pad_v              = poolPadX->value->u32;
    vx_scalar    in_heights         = vxCreateScalar(context, VX_TYPE_INT32, &in_height);
    vx_image     imgInput           = NULL;
    vx_image     imgOutput          = NULL;
    vx_int8      srcFixPointPos     = input->tensorBuffer->fixedPointPos;
    vx_int8      dstFixPointPos     = output->tensorBuffer->fixedPointPos;
    vx_bool      globalPooling_flag = (vx_bool)(stride_v == 1 && pad_v == 0 && out_width == 1 && out_height == 1 && kernel_v == in_width && kernel_v == in_height);
    vx_float32   div_scale          = 1.0f;

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
    else if (pad_v != 0)
    {
        borderMode->mode = VX_BORDER_CONSTANT;
        if (inputFormat == VX_TYPE_INT8)
        {
            borderMode->constant_value.U8 = 0;
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
        char *programSources[4] = {NULL, NULL, NULL, NULL};
        char programSources_AvgPoolSrc0[] =
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
                VXC_DP2x8(img_val1, sum, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), uniDotScale_2x8); \n\
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
                VXC_DP4x4(sum, sum, scale, VXC_MODIFIER(0, 3, 0, VXC_RM_ToNearestEven, 1), uniDotScale_4x4); \n\
                vxc_short4 vec; \n\
                _viv_asm(COPY, vec, sum, 8); \n\
                VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0,VXC_RM_TowardZero, 0)); \n\
                coord_out.y ++; \n\
                 \n\
                VXC_DP2x8(h5, h5, h6, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16Swap); \n\
            } while (coord_in.y < height); \n\
         }\n"
        };

        char programSources_AvgPoolSrc1[] =
        {"\n\
         \n\
         _viv_uniform VXC_512Bits uniInt8AddInt8Kernel6Lo_8x4; \n\
         _viv_uniform VXC_512Bits uniInt8AddInt8Kernel6Hi_8x4; \n\
         _viv_uniform VXC_512Bits uniS16AddS16Kernel6_2x8; \n\
         _viv_uniform VXC_512Bits uniS16DotFP16_2x8; \n\
         _viv_uniform VXC_512Bits uniInt8AddInt8_4x8; \n\
         _viv_uniform VXC_512Bits uniS16MulS16toInt8_2x8; \n\
        _viv_uniform float scaleInt8_Int8_3_1_1; \n\
         _viv_uniform float scaleInt8_FP16_6_1_0; \n\
         __kernel void vxcPooling_AvgInt8toInt8ker3str1pad1 (\n\
         __read_only image2d_array_t   input, \n\
         int height, \n\
         __write_only image2d_array_t  output) \n\
         { \n\
            int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            int4 coord_out = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
             \n\
            vxc_char16 lineA, lineB, lineC, lineD; \n\
            vxc_short8    sumA, sumB, sumC, sumD; \n\
            half scale; \n\
            _viv_asm(CONV, scale, scaleInt8_Int8_3_1_1); \n\
            \n\
            VXC_ReadImage2DArray(lineA, input, coord_in, VXC_5BITOFFSET_XY(-1, -1), VXC_MODIFIER(0, 9, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(lineB, input, coord_in, VXC_5BITOFFSET_XY(-1, 0), VXC_MODIFIER(0, 9, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(lineC, input, coord_in, VXC_5BITOFFSET_XY(-1, 1), VXC_MODIFIER(0, 9, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(lineD, input, coord_in, VXC_5BITOFFSET_XY(-1, 2), VXC_MODIFIER(0, 9, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP4x8(sumA, lineA, lineA, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_4x8); \n\
            VXC_DP4x8(sumB, lineB, lineB, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_4x8); \n\
            VXC_DP4x8(sumC, lineC, lineC, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_4x8); \n\
            coord_out.y --; \n\
            do \n\
            { \n\
                sumA = sumA + sumB; \n\
                sumA = sumA + sumC; \n\
                VXC_DP2x8(lineA, sumA, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), uniS16MulS16toInt8_2x8); \n\
                coord_out.y ++; \n\
                VXC_WriteImage2DArray(output, coord_out, lineA, VXC_MODIFIER(0, 7, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_DP4x8(sumD, lineD, lineD, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_4x8); \n\
                 \n\
                coord_in.y ++; \n\
                VXC_ReadImage2DArray(lineA, input, coord_in, VXC_5BITOFFSET_XY(-1, 2), VXC_MODIFIER(0, 9, 0, VXC_RM_TowardZero, 0)); \n\
                sumB = sumB + sumC; \n\
                sumB = sumB + sumD; \n\
                VXC_DP2x8(lineB, sumB, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), uniS16MulS16toInt8_2x8); \n\
                coord_out.y ++; \n\
                VXC_WriteImage2DArray(output, coord_out, lineB, VXC_MODIFIER(0, 7, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_DP4x8(sumA, lineA, lineA, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_4x8); \n\
                 \n\
                coord_in.y ++; \n\
                VXC_ReadImage2DArray(lineB, input, coord_in, VXC_5BITOFFSET_XY(-1, 2), VXC_MODIFIER(0, 9, 0, VXC_RM_TowardZero, 0)); \n\
                sumC = sumC + sumD; \n\
                sumC = sumC + sumA; \n\
                VXC_DP2x8(lineC, sumC, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), uniS16MulS16toInt8_2x8); \n\
                coord_out.y ++; \n\
                VXC_WriteImage2DArray(output, coord_out, lineC, VXC_MODIFIER(0, 7, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_DP4x8(sumB, lineB, lineB, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_4x8); \n\
                 \n\
                coord_in.y ++; \n\
                VXC_ReadImage2DArray(lineC, input, coord_in, VXC_5BITOFFSET_XY(-1, 2), VXC_MODIFIER(0, 9, 0, VXC_RM_TowardZero, 0)); \n\
                sumD = sumD + sumA; \n\
                sumD = sumD + sumB; \n\
                VXC_DP2x8(lineD, sumD, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), uniS16MulS16toInt8_2x8); \n\
                coord_out.y ++; \n\
                VXC_WriteImage2DArray(output, coord_out, lineD, VXC_MODIFIER(0, 7, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_DP4x8(sumC, lineC, lineC, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniInt8AddInt8_4x8); \n\
                 \n\
                coord_in.y ++; \n\
                VXC_ReadImage2DArray(lineD, input, coord_in, VXC_5BITOFFSET_XY(-1, 2), VXC_MODIFIER(0, 9, 0, VXC_RM_TowardZero, 0)); \n\
                 \n\
            } while (coord_out.y < height); \n\
         } \n\
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
         VXC_DP2x8(dst, sum, scale, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), uniS16DotFP16_2x8); \n\
         \n\
         vxc_short8 vec; \n\
         _viv_asm(COPY, vec, dst, 16); \n\
         VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0,VXC_RM_TowardZero, 0)); \n\
         coord_out.y ++; \n\
         } while (coord_in.y < height); \n\
         }\n"
        };

        char programSources_AvgPoolSrc2[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         \n\
         _viv_uniform VXC_512Bits uniInt8AddInt8_32x1; \n\
         _viv_uniform VXC_512Bits uni2S16Dot1FP16_KernelLE8_16x1; \n\
         _viv_uniform VXC_512Bits uni6S16Dot1FP16_KernelLE8_16x1; \n\
         _viv_uniform VXC_512Bits uniS16AddS16_16x1; \n\
         _viv_uniform VXC_512Bits uniFp16AddFp16_16x1; \n\
         _viv_uniform float scale_globalPool; \n\
         _viv_uniform VXC_512Bits uniFp16AddFp16_4x4; \n\
         _viv_uniform VXC_512Bits uniPackedHalf4_2x8; \n\
        _viv_uniform float         scale_fp16tofp16; \n\
        __kernel void vxcPooling_AvgFp16toFp16ker3str1pad1 (\n\
        __read_only image2d_array_t   input, \n\
        int height, \n\
        __write_only image2d_array_t  output) \n\
        { \n\
            int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            int4 coord_out = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
             \n\
            vxc_ushort8 vec0, vec1, vec2, vec3; \n\
            vxc_half8 lineA, lineB, lineC, lineD; \n\
            float4 sumA, sumB, sumC, sumD, sum; \n\
             \n\
            VXC_ReadImage2DArray(vec0, input, coord_in, VXC_5BITOFFSET_XY(-1, -1), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, lineA, vec0, 16); \n\
            VXC_ReadImage2DArray(vec1, input, coord_in, VXC_5BITOFFSET_XY(-1, 0), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, lineB, vec1, 16); \n\
            VXC_ReadImage2DArray(vec2, input, coord_in, VXC_5BITOFFSET_XY(-1, 1), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, lineC, vec2, 16); \n\
            VXC_ReadImage2DArray(vec3, input, coord_in, VXC_5BITOFFSET_XY(-1, 2), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, lineD, vec3, 16); \n\
             \n\
            VXC_DP4x4(sumA, lineA, lineA, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_4x4); \n\
            VXC_DP4x4(sumB, lineB, lineB, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_4x4); \n\
            VXC_DP4x4(sumC, lineC, lineC, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_4x4); \n\
            VXC_DP4x4(sumD, lineD, lineD, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_4x4); \n\
            do  \n\
            { \n\
                VXC_ReadImage2DArray(vec0, input, coord_in, VXC_5BITOFFSET_XY(-1, 3), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, lineA, vec0, 16); \n\
                coord_in.y ++; \n\
                VXC_ReadImage2DArray(vec1, input, coord_in, VXC_5BITOFFSET_XY(-1, 3), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, lineB, vec1, 16); \n\
                coord_in.y ++; \n\
                 \n\
                sum  = sumB + sumC; \n\
                sumA = sumA + sum; \n\
                sumB = sumD + sum; \n\
                sumA = sumA * scale_fp16tofp16; \n\
                sumB = sumB * scale_fp16tofp16; \n\
                half4 tmp0, tmp1; \n\
                _viv_asm(CONV, tmp0, sumA); \n\
                _viv_asm(CONV, tmp1, sumB); \n\
                 \n\
                VXC_DP2x8(lineC, tmp0, tmp1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniPackedHalf4_2x8); \n\
                vxc_short8 dst0; \n\
                _viv_asm(COPY, dst0, lineC, 16); \n\
                VXC_WriteImage2DArray(output, coord_out, dst0, VXC_MODIFIER(0, 3, 0,VXC_RM_TowardZero, 0)); \n\
                coord_out.y ++; \n\
                VXC_WriteImage2DArray(output, coord_out, dst0, VXC_MODIFIER(4, 7, 0,VXC_RM_TowardZero, 0)); \n\
                coord_out.y ++; \n\
                 \n\
                VXC_ReadImage2DArray(vec2, input, coord_in, VXC_5BITOFFSET_XY(-1, 3), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, lineC, vec2, 16); \n\
                coord_in.y ++; \n\
                VXC_ReadImage2DArray(vec3, input, coord_in, VXC_5BITOFFSET_XY(-1, 3), VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, lineD, vec3, 16); \n\
                coord_in.y ++; \n\
                 \n\
                VXC_DP4x4(sumA, lineA, lineA, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_4x4); \n\
                VXC_DP4x4(sumB, lineB, lineB, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_4x4); \n\
                 \n\
                sum  = sumD + sumA; \n\
                sumC = sumC + sum; \n\
                sumD = sumB + sum; \n\
                sumC = sumC * scale_fp16tofp16; \n\
                sumD = sumD * scale_fp16tofp16; \n\
                _viv_asm(CONV, tmp0, sumC); \n\
                _viv_asm(CONV, tmp1, sumD); \n\
                 \n\
                VXC_DP2x8(lineA, tmp0, tmp1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniPackedHalf4_2x8); \n\
                vxc_short8 dst1; \n\
                _viv_asm(COPY, dst1, lineA, 16); \n\
                VXC_WriteImage2DArray(output, coord_out, dst1, VXC_MODIFIER(0, 3, 0,VXC_RM_TowardZero, 0)); \n\
                coord_out.y ++; \n\
                VXC_WriteImage2DArray(output, coord_out, dst1, VXC_MODIFIER(4, 7, 0,VXC_RM_TowardZero, 0)); \n\
                coord_out.y ++; \n\
                 \n\
                VXC_DP4x4(sumC, lineC, lineC, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_4x4); \n\
                VXC_DP4x4(sumD, lineD, lineD, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_4x4); \n\
                 \n\
            } while (coord_out.y < height); \n\
        } \n\
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
            VXC_ReadImage(img_val7, input, coord.wy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, vec7, img_val7, 16); \n\
            \n\
            VXC_DP16x1(sum, vec0, vec1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniS16AddS16_16x1); \n\
            VXC_DP16x1(sum, vec2, vec3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniS16AddS16_16x1); \n\
            VXC_DP16x1(sum, vec4, vec5, VXC_MODIFIER(2, 2, 0, VXC_RM_TowardZero, 0), uniS16AddS16_16x1); \n\
            VXC_DP16x1(sum, vec6, vec7, VXC_MODIFIER(3, 3, 0, VXC_RM_TowardZero, 0), uniS16AddS16_16x1); \n\
            vxc_float4 scale = {scale_globalPool, scale_globalPool, scale_globalPool, scale_globalPool}; \n\
            sum.x = dot(sum, scale); \n\
            char val = convert_char_sat_rte(sum.x); \n\
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
            VXC_DP16x1(scale, sum, scale, VXC_MODIFIER(0, 0, 0, VXC_RM_ToNearestEven, 1), uni2S16Dot1FP16_KernelLE8_16x1); \n\
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
            VXC_DP16x1(scale, sum, scale, VXC_MODIFIER(0, 0, 0, VXC_RM_ToNearestEven, 1), uni6S16Dot1FP16_KernelLE8_16x1); \n\
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
            VXC_DP16x1(img_val0, sum, scale, VXC_MODIFIER(0, 0, 0, VXC_RM_ToNearestEven, 1), uni2S16Dot1FP16_KernelLE8_16x1); \n\
            VXC_WriteImage(output, coord, img_val0, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0)); \n\
         }\n"
        };

        char programSources_AvgPoolSrc3[] =
        {" \n\
         __kernel void vxcPooling_GlobalAvgFp16toFp16KernelLE8 (\n\
         __read_only image2d_t   input, \n\
         int height, \n\
         __write_only image2d_t  output) \n\
         { \n\
             int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(0) + 16, get_global_id(0) + 48); \n\
             \n\
             vxc_short8 img_val0, img_val1, img_val2, img_val3, img_val4, img_val5, img_val6, img_val7; \n\
             vxc_half8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7; \n\
             vxc_float4  sum; \n\
             float dst; \n\
             const float4 one4 = (float4)(1.0, 1.0, 1.0, 1.0); \n\
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
             VXC_ReadImage(img_val7, input, coord.wy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec7, img_val7, 16); \n\
             coord.zw += 64; \n\
             VXC_DP16x1(sum, vec0, vec1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             VXC_DP16x1(sum, vec2, vec3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             VXC_DP16x1(sum, vec4, vec5, VXC_MODIFIER(2, 2, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             VXC_DP16x1(sum, vec6, vec7, VXC_MODIFIER(3, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             dst = dot(sum, one4); \n\
             dst *= scale_globalPool; \n\
             \n\
             half vect; \n\
             _viv_asm(CONV, vect, dst); \n\
             vxc_ushort4 dstVec; \n\
             _viv_asm(COPY, dstVec, vect, 2); \n\
             VXC_WriteImage(output, coord.xy, dstVec, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0)); \n\
         } \n\
         __kernel void vxcPooling_GlobalAvgFp16toFp16KernelLE13 (\n\
         __read_only image2d_t   input, \n\
         int height, \n\
         __write_only image2d_t  output) \n\
         { \n\
             int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(0) + 16, get_global_id(0) + 48); \n\
             \n\
             vxc_short8 img_val0, img_val1, img_val2, img_val3, img_val4, img_val5, img_val6, img_val7; \n\
             vxc_half8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7; \n\
             vxc_float4  sum; \n\
             float dst; \n\
             const float4 one4 = (float4)(1.0, 1.0, 1.0, 1.0); \n\
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
             VXC_ReadImage(img_val7, input, coord.wy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec7, img_val7, 16); \n\
             coord.zw += 64; \n\
             VXC_DP16x1(sum, vec0, vec1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             VXC_DP16x1(sum, vec2, vec3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             \n\
             VXC_ReadImage(img_val0, input, coord.zy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec0, img_val0, 16); \n\
             VXC_ReadImage(img_val1, input, coord.zy, VXC_5BITOFFSET_XY(-8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec1, img_val1, 16); \n\
             VXC_ReadImage(img_val2, input, coord.zy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec2, img_val2, 16); \n\
             VXC_ReadImage(img_val3, input, coord.zy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec3, img_val3, 16); \n\
             VXC_DP16x1(sum, vec4, vec5, VXC_MODIFIER(2, 2, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             VXC_DP16x1(sum, vec6, vec7, VXC_MODIFIER(3, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             dst = dot(sum, one4); \n\
             VXC_ReadImage(img_val4, input, coord.wy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec4, img_val4, 16); \n\
             VXC_ReadImage(img_val5, input, coord.wy, VXC_5BITOFFSET_XY(-8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec5, img_val5, 16); \n\
             VXC_ReadImage(img_val6, input, coord.wy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec6, img_val6, 16); \n\
             VXC_ReadImage(img_val7, input, coord.wy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec7, img_val7, 16); \n\
             coord.zw += 64; \n\
             VXC_DP16x1(sum, vec0, vec1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             VXC_DP16x1(sum, vec2, vec3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             \n\
             VXC_ReadImage(img_val0, input, coord.zy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec0, img_val0, 16); \n\
             VXC_ReadImage(img_val1, input, coord.zy, VXC_5BITOFFSET_XY(-8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec1, img_val1, 16); \n\
             VXC_ReadImage(img_val2, input, coord.zy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec2, img_val2, 16); \n\
             VXC_ReadImage(img_val3, input, coord.zy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec3, img_val3, 16); \n\
             VXC_DP16x1(sum, vec4, vec5, VXC_MODIFIER(2, 2, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             VXC_DP16x1(sum, vec6, vec7, VXC_MODIFIER(3, 3, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             dst += dot(sum, one4); \n\
             VXC_ReadImage(img_val4, input, coord.wy, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec4, img_val4, 16); \n\
             VXC_ReadImage(img_val5, input, coord.wy, VXC_5BITOFFSET_XY(-8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             _viv_asm(COPY, vec5, img_val5, 16); \n\
             \n\
             VXC_DP16x1(sum, vec0, vec1, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             VXC_DP16x1(sum, vec2, vec3, VXC_MODIFIER(1, 1, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             sum.w = 0; \n\
             VXC_DP16x1(sum, vec4, vec5, VXC_MODIFIER(2, 2, 0, VXC_RM_TowardZero, 0), uniFp16AddFp16_16x1); \n\
             dst += dot(sum, one4); \n\
             dst *= scale_globalPool; \n\
             \n\
             half vect; \n\
             _viv_asm(CONV, vect, dst); \n\
             vxc_ushort4 dstVec; \n\
             _viv_asm(COPY, dstVec, vect, 2); \n\
             VXC_WriteImage(output, coord.xy, dstVec, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0)); \n\
         } \n"
        };

        if ((inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16))
        {
            programSources[0] = programSources_AvgPoolSrc0;
            programSources[1] = programSources_AvgPoolSrc1;
            programSources[2] = programSources_AvgPoolSrc2;
            programSources[3] = programSources_AvgPoolSrc3;
            programLength[0] = strlen(programSources_AvgPoolSrc0);
            programLength[1] = strlen(programSources_AvgPoolSrc1);
            programLength[2] = strlen(programSources_AvgPoolSrc2);
            programLength[3] = strlen(programSources_AvgPoolSrc3);
            program = vxCreateProgramWithSource(context, 4, (const vx_char**)programSources, programLength);
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
        vx_uint32 uniFp16AddFp16_16x1[16] = {
            0x55555555, // TCfg
            0x55550000, // ASelt
            0x76543210, 0x76543210, // ABin
            0xaaaaaaaa, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00 // Constant
        };

        if(inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16 && kernel_v <= 8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_GlobalAvgInt8toFp16KernelLE8", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if(inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_FLOAT16 && kernel_v <= 13)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_GlobalAvgInt8toFp16KernelLE13", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && kernel_v <= 8)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_GlobalAvgFp16toFp16KernelLE8", borderMode);
            if (!shaderExecutable) goto error;
        }
        else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && kernel_v <= 13)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_GlobalAvgFp16toFp16KernelLE13", borderMode);
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
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniFp16AddFp16_16x1", 1, uniFp16AddFp16_16x1);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scale_globalPool", 1, &scale_globalPool);
        if (status != VX_SUCCESS) goto error;

    }
    else if (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && kernel_v == 3 && stride_v == 1 &&  pad_v == 1)
    {
        vx_float32    scale_fp16tofp16 = 1 / (float)(kernel_v * kernel_v);
        vx_uint32 uniFp16AddFp16_4x4[16] = {
            0x15151515, // TCfg
            0x00000000, // ASelt
            0x03210210, 0x05430432, // ABin
            0x2a2a2a2a, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00010001, 0x00000001, 0x00010001, 0x00000001, 0x00010001, 0x00000001, 0x00010001, 0x00000001 // Constant
        };
        vx_uint32 uniPackedHalf4_2x8[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x06040200, 0x06040200, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001 // Constant
        };

        execution_parameters.globalWorkScale[0]  = 4;
        execution_parameters.globalWorkScale[1]  = out_height;
        execution_parameters.globalWorkScale[2]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgFp16toFp16ker3str1pad1", borderMode);
        if (!shaderExecutable) goto error;
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniFp16AddFp16_4x4", 1, uniFp16AddFp16_4x4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniPackedHalf4_2x8", 1, uniPackedHalf4_2x8);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scale_fp16tofp16", 1, &scale_fp16tofp16);
        if (status != VX_SUCCESS) goto error;
    }
    else if (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8 && kernel_v == 3 && stride_v == 1 &&  pad_v == 1)
    {
        vx_float32    scaleInt8_Int8_3_1_1 = div_scale * (1 / (float)(kernel_v * kernel_v));
        vx_uint32 uniS16MulS16toInt8_2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniInt8AddInt8_4x8[16] = {
            0x3f3f3f3f, 0x3f3f3f3f, // TCfg
            0xc4100820, 0x30106200, 0x18a40148, 0xe601cc50, 0x02507020, // BinSelect
            0x00004400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = out_height;
        execution_parameters.globalWorkScale[2]  = 1;

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_AvgInt8toInt8ker3str1pad1", borderMode);
        if (!shaderExecutable) goto error;
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniS16MulS16toInt8_2x8", 1, uniS16MulS16toInt8_2x8);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniInt8AddInt8_4x8", 1, uniInt8AddInt8_4x8);
        if (status != VX_SUCCESS) goto error;

        status = vxnneShaderExecutable_SetUniform(shaderExecutable, "scaleInt8_Int8_3_1_1", 1, &scaleInt8_Int8_3_1_1);
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
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(in_heights) vxReleaseScalar(&in_heights);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);
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
    vx_size    programLength[5] = {0};
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

    vx_int8    input_fractionLengthValue  = input->tensorBuffer->fixedPointPos;
    vx_int8    output_fractionLengthValue = output->tensorBuffer->fixedPointPos;
    vx_int8    div_fractionLengthValue    = input_fractionLengthValue - output_fractionLengthValue;
    vx_float32 out_scale                  = 1.0f;
    vx_float32 div_scale                  = 1.0f;

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
    execution_parameters.globalWorkScale[0]  = ((8 - kernel_v + stride_v) /2 ) * 2;
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
        char *programSources_MaxPool[5] = {NULL, NULL, NULL, NULL, NULL};
        char programSources1_MaxPoolFp16In[] =
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
                VXC_VertMax3_Half(fp16_val1, img_reg2, img_reg3, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Half(fp16_val1, fp16_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_VertMax3_Half(fp16_val2, img_reg5, img_reg6, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Half(fp16_val2, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0), UniPackMaxPool2x8_fp16); \n\
                if (outputFormat1 == 15) \n\
                {\n\
                    _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                    VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniFP16Mul_dp2x8); \n\
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
                    VXC_VertMax3_Half(fp16_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3_Half(fp16_val1, fp16_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3_Half(fp16_val2, img_reg4, img_reg5, img_reg6, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3_Half(fp16_val2, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0), UniPackMaxPool2x8_fp16); \n\
                    VXC_VertMax3_Half(fp16_val1, fp16_val1, fp16_val1, val_min, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat1 == 15) \n\
                    {\n\
                        _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                        VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniFP16Mul_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                } \n\
                //the last 1 row \n\
                posout.y += 1; \n\
                VXC_HorzMax3_Half(fp16_val1, img_reg3, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Half(fp16_val2, img_reg6, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0), UniPackMaxPool2x8_fp16); \n\
                VXC_VertMax3_Half(fp16_val1, fp16_val1, fp16_val1, val_min, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat1 == 15) \n\
                {\n\
                    _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                    VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniFP16Mul_dp2x8); \n\
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
                    VXC_VertMax3_Half(fp16_val1, img_reg1, img_reg2, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3_Half(fp16_val2, img_reg3, img_reg4, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg1, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(8,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg4, input, posin, VXC_5BITOFFSET_XY(8,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_DP2x8(fp16_val3, fp16_val1, fp16_val2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16even_2x8); \n\
                    VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), UniPackFP16odd_2x8); \n\
                    VXC_VertMax3_Half(fp16_val1, fp16_val1, fp16_val1, fp16_val3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat1 == 15) \n\
                    {\n\
                        _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                        VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFP16Mul_dp2x8); \n\
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
                    VXC_VertMax3_Half(fp16_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3_Half(fp16_val2, img_reg4, img_reg5, img_reg6, VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    img_reg1 = img_reg3; \n\
                    img_reg4 = img_reg6; \n\
                    posin.y += 2; \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,2), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg5, input, posin, VXC_5BITOFFSET_XY(6,1), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg6, input, posin, VXC_5BITOFFSET_XY(6,2), VXC_MODIFIER(0, 6, 0, VXC_RM_TowardZero, 0)); \n\
                    \n\
                    VXC_HorzMax3_Half(fp16_val1, fp16_val1, VXC_MODIFIER(0, 4, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3_Half(fp16_val2, fp16_val2, VXC_MODIFIER(0, 4, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_DP2x8(fp16_val1, fp16_val1, fp16_val2, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0), UniPackMaxPool2x8_fp16); \n\
                    VXC_VertMax3_Half(fp16_val1, fp16_val1, fp16_val1, val_min, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat1 == 15) \n\
                    {\n\
                        _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                        VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniFP16Mul_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    posout.y += 1; \n\
                } \n\
            }\n"
        };

        char programSources2_MaxPoolInt8In[] =
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
                    VXC_VertMax3_Integer(s8_val1, img_reg1, img_reg2, val_min, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg1, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0));\n\
                    \n\
                    if (outputFormat2 == 15) \n\
                    {\n\
                        VXC_DP2x8(val1_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16Packeven_dp2x8); \n\
                        VXC_DP2x8(val2_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), UniS8xFp16Packodd_dp2x8); \n\
                        VXC_VertMax3_Half(val1_fp16, val1_fp16, val2_fp16, val1_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                        _viv_asm(COPY, val_s16, val1_fp16, 16); \n\
                        VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(s8_val2, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniS8xFp16Packeven_dp2x8); \n\
                        VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniS8xFp16Packodd_dp2x8); \n\
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
                    VXC_VertMax3_Integer(s8_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    img_reg1 = img_reg3; \n\
                    posin.y += 2; \n\
                    VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,2), VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    \n\
                    VXC_VertMax3_Integer(s8_val1, s8_val1, s8_val1, val_min, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3_Integer(s8_val1, s8_val1, VXC_MODIFIER(0, 10, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat2 == 15) \n\
                    {\n\
                        VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                        _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                        VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniMaxPoolS8xFp16_dp2x8); \n\
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
                VXC_VertMax3_Integer(s8_val1, img_reg2, img_reg3, val_min, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Integer(s8_val1, s8_val1, VXC_MODIFIER(0, 10, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat2 == 15) \n\
                {\n\
                    VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniMaxPoolS8xFp16_dp2x8); \n\
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
                    VXC_VertMax3_Integer(s8_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3_Integer(s8_val1, s8_val1, s8_val1, val_min, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3_Integer(s8_val1, s8_val1, VXC_MODIFIER(0, 10, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat2 == 15) \n\
                    {\n\
                        VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                        _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                        VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                }\n\
                //the last 1 row \n\
                posout.y += 1; \n\
                VXC_VertMax3_Integer(s8_val1, img_reg3, val_min, img_reg3, VXC_MODIFIER(0, 12, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Integer(s8_val1, s8_val1, VXC_MODIFIER(0, 10, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat2 == 15) \n\
                {\n\
                    VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniMaxPoolS8xFp16_dp2x8); \n\
                    VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            }\n"
        };

        char programSources3_MaxPool[] =
        {"\n\
            #include \"cl_viv_vx_ext.h\" \n\
            \n\
            _viv_uniform VXC_512Bits UniFP16MulSrc3_dp2x8; \n\
            _viv_uniform VXC_512Bits UniS8xFp16_dp2x8; \n\
            _viv_uniform float outSrc3_scale; \n\
            _viv_uniform float divSrc3_scale; \n\
            _viv_uniform int outputFormat3; \n\
            __kernel void vxcPooling_maxfp16ker3str1pad1(\n\
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
                vxc_short8 img_reg1,img_reg2,img_reg3; \n\
                vxc_short8 s16_val0; \n\
                vxc_half8 fp16_val1; \n\
                half out_scale_fp16; \n\
                vxc_char8 val_s8; \n\
                \n\
                VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0));//head 2 row \n\
                VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                int4 posout = (int4)(coord_in.x, coord_in.y, coord_in.z, 0); \n\
                vxc_short8 val_min = {0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff,0xfbff}; \n\
                _viv_asm(CONV, out_scale_fp16, outSrc3_scale); \n\
                VXC_VertMax3_Half(fp16_val1, img_reg2, img_reg3, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Half(fp16_val1, fp16_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat3 == 15) \n\
                {\n\
                    _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                    VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniFP16MulSrc3_dp2x8); \n\
                    VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                int loop_count = out_height- 1; \n\
                for (int i = 1; i < loop_count; i++) \n\
                {\n\
                    img_reg1 = img_reg2; \n\
                    img_reg2 = img_reg3; \n\
                    posin.y += 1; \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    posout.y += 1; \n\
                    VXC_VertMax3_Half(fp16_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3_Half(fp16_val1, fp16_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3_Half(fp16_val1, fp16_val1, fp16_val1, val_min, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat3 == 15)\n\
                    {\n\
                        _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                        VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniFP16MulSrc3_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                }\n\
                //the last 1 row \n\
                posout.y += 1; \n\
                VXC_VertMax3_Half(fp16_val1, img_reg2, img_reg3, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Half(fp16_val1, fp16_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat3 == 15) \n\
                {\n\
                    _viv_asm(COPY, s16_val0, fp16_val1, 16); \n\
                    VXC_WriteImage2DArray(output, posout, s16_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(val_s8, fp16_val1, out_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniFP16MulSrc3_dp2x8); \n\
                    VXC_WriteImage2DArray(output, posout, val_s8, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            }\n\
            __kernel void vxcPooling_MaxInt8ker3str1pad1(\n\
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
                vxc_char8 img_reg1,img_reg2,img_reg3; \n\
                half div_scale_fp16; \n\
                vxc_char8 s8_val1; \n\
                vxc_half8 val_fp16; \n\
                vxc_short8 val_s16; \n\
                \n\
                VXC_ReadImage2DArray(img_reg2, input, posin, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                int4 posout = (int4)(coord_in.x, coord_in.y, coord_in.z, 0); \n\
                _viv_asm(CONV, div_scale_fp16, divSrc3_scale); \n\
                vxc_char8 val_min = {0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80}; \n\
                \n\
                VXC_VertMax3_Integer(s8_val1, img_reg2, img_reg3, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Integer(s8_val1, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat3 == 15)\n\
                {\n\
                    VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniS8xFp16_dp2x8); \n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else \n\
                {\n\
                    VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniS8xFp16_dp2x8); \n\
                    VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                int loop_count = out_height- 1; \n\
                for (int i = 1; i < loop_count; i++)\n\
                {\n\
                    img_reg1 = img_reg2; \n\
                    img_reg2 = img_reg3; \n\
                    posin.y += 1; \n\
                    VXC_ReadImage2DArray(img_reg3, input, posin, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    posout.y += 1; \n\
                    VXC_VertMax3_Integer(s8_val1, img_reg1, img_reg2, img_reg3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_HorzMax3_Integer(s8_val1, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    VXC_VertMax3_Integer(s8_val1, s8_val1, s8_val1, val_min, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    if (outputFormat3 == 15)\n\
                    {\n\
                        VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniS8xFp16_dp2x8); \n\
                        _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                        VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                    else \n\
                    {\n\
                        VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniS8xFp16_dp2x8); \n\
                        VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                    }\n\
                }\n\
                //the last 1 row \n\
                posout.y += 1; \n\
                VXC_VertMax3_Integer(s8_val1, img_reg2, img_reg3, val_min, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Integer(s8_val1, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                if (outputFormat3 == 15)\n\
                {\n\
                    VXC_DP2x8(val_fp16, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 1), UniS8xFp16_dp2x8); \n\
                    _viv_asm(COPY, val_s16, val_fp16, 16); \n\
                    VXC_WriteImage2DArray(output, posout, val_s16, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
                else\n\
                {\n\
                    VXC_DP2x8(s8_val1, s8_val1, div_scale_fp16, VXC_MODIFIER(0, 5, 0, VXC_RM_ToNearestEven, 1), UniS8xFp16_dp2x8); \n\
                    VXC_WriteImage2DArray(output, posout, s8_val1, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                }\n\
            }\n"
        };

        if (context->evisNoInst.isVX2)
        {
            programSources_MaxPool[0] = VXC_VertMax3_Evis2;
            programSources_MaxPool[1] = VXC_HorzMax3_Evis2;
            programSources_MaxPool[2] = programSources1_MaxPoolFp16In;
            programSources_MaxPool[3] = programSources2_MaxPoolInt8In;
            programSources_MaxPool[4] = programSources3_MaxPool;

            programLength[0]          = strlen(VXC_VertMax3_Evis2);
            programLength[1]          = strlen(VXC_HorzMax3_Evis2);
            programLength[2]          = strlen(programSources1_MaxPoolFp16In);
            programLength[3]          = strlen(programSources2_MaxPoolInt8In);
            programLength[4]          = strlen(programSources3_MaxPool);
        }
        else
        {
            programSources_MaxPool[0] = VXC_VertMax3_Evis1;
            programSources_MaxPool[1] = VXC_HorzMax3_Evis1;
            programSources_MaxPool[2] = programSources1_MaxPoolFp16In;
            programSources_MaxPool[3] = programSources2_MaxPoolInt8In;
            programSources_MaxPool[4] = programSources3_MaxPool;

            programLength[0]          = strlen(VXC_VertMax3_Evis1);
            programLength[1]          = strlen(VXC_HorzMax3_Evis1);
            programLength[2]          = strlen(programSources1_MaxPoolFp16In);
            programLength[3]          = strlen(programSources2_MaxPoolInt8In);
            programLength[4]          = strlen(programSources3_MaxPool);
        }
        program = vxCreateProgramWithSource(context, 5, (const vx_char**)programSources_MaxPool, programLength);

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
        else if (kernel_v == 3 && stride_v == 1 &&  pad_v == 1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_maxfp16ker3str1pad1", borderMode);
            if (!shaderExecutable) goto error;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniFP16MulSrc3_dp2x8", 1, UniFP16Mul_dp2x8);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outSrc3_scale", 1, &out_scale);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outputFormat3", 1, &outputFormat);
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
        vx_uint32 UniS8xFp16_dp2x8[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
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
        else if (kernel_v == 3 && stride_v == 1 &&  pad_v == 1)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_MaxInt8ker3str1pad1", borderMode);
            if (!shaderExecutable) goto error;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniS8xFp16_dp2x8", 1, UniS8xFp16_dp2x8);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "divSrc3_scale", 1, &div_scale);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "outputFormat3", 1, &outputFormat);
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
    vx_reference    parameters[9]              = {(vx_reference)input, VX_NULL, VX_NULL, VX_NULL, (vx_reference)type_s, (vx_reference)norm_size_s, (vx_reference)alpha_s, (vx_reference)beta_s, (vx_reference)output};
    vx_enum         inputFormat                = input->tensorBuffer->dataFormat;
    vx_enum         outputFormat               = output->tensorBuffer->dataFormat;
    vx_uint32       width                      = input->tensorBuffer->memory.dims[0][0];
    vx_uint32       height                     = input->tensorBuffer->memory.dims[0][1];
    vx_uint32       channel                    = input->tensorBuffer->memory.dims[0][2];
    vx_enum         norm_type                  = type_s->value->e;
    vx_scalar       width_s                    = vxCreateScalar(context, VX_TYPE_UINT32, &width);
    vx_scalar       height_s                   = vxCreateScalar(context, VX_TYPE_UINT32, &height);
    vx_scalar       channel_s                  = vxCreateScalar(context, VX_TYPE_UINT32, &channel);
    vx_float32      in_scale                   = 1.0f;
    vx_float32      out_scale                  = 1.0f;
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
                        VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
                    VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
                    VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
                        VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
                    VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
                    VXC_DP2x8(val_s8, val_fp16, out_scale_fp16, VXC_MODIFIER(0, 3, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
                    VXC_DP2x8(val_s8, out_h, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
                        VXC_DP2x8(val_s8, out_h, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
                    VXC_DP2x8(val5, out_h, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
                        VXC_DP2x8(val5, out_h, out_scale_fp16, VXC_MODIFIER(0, 7, 0, VXC_RM_ToNearestEven, 1), UniFp16xFp16toS8_dp2x8); \n\
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
        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni4x4_SquareSubLo4", 1, Uni4x4_SquareSubLo4);
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
   if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

   return VX_NULL;
}

/********vxcSoftmax****************************************************/
vxnne_shader_executable vxnneGetSoftmaxShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               output)
{
    vx_size    programLength[5] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[2]     = {(vx_reference)input, (vx_reference)output};
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_enum      outputFormat      = output->tensorBuffer->dataFormat;
    vx_uint32    width             = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    height            = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth             = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32    batch             = TENSOR_VIEW_SIZE_INDEX(input, 3);
	vx_uint32    dims              = TENSOR_DIM_NUM(input);
    vx_image     imgInput          = NULL;
    vx_image     imgOutput         = NULL;
    vx_int8      srcFixPointPos    = input->tensorBuffer->fixedPointPos;
    vx_int8      dstFixPointPos    = output->tensorBuffer->fixedPointPos;
    vx_float32   scaleOut          = 1.0f;
    vx_float32   scaleIn           = 1.0f;
    vx_uint32    inputWidth        = depth / 4 * 4;
    vx_uint32    inputWidthRemain4 = depth % 4;
    vx_uint32    itemCount         = 0;
    vx_uint32    itemDepth         = 0;
    vx_int32     int8_isFp16       = 0;
    vx_int32     fp16_isFp16       = 0;

    if(batch == 0)
        batch = 1;

    itemCount = width * height;
    itemDepth = depth * batch;
    if (inputFormat == VX_TYPE_INT8)
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

    if (outputFormat == VX_TYPE_INT8)
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

    borderMode->mode = VX_BORDER_REPLICATE;
    if (inputFormat == VX_TYPE_INT8)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, depth, batch, VX_DF_IMAGE_U8);
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && width == 2 && dims == 2)
        {
            imgInput = vxoTensor_CreateImageFromTensor(input, itemCount, 1, VX_DF_IMAGE_S16);
        }
        else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && depth == 2)
        {
            imgInput = vxoTensor_CreateImageFromTensor(input, itemCount, itemDepth, VX_DF_IMAGE_S16);
        }
        else if(itemCount == 1)
        {
            imgInput = vxoTensor_CreateImageFromTensor(input, depth, batch, VX_DF_IMAGE_S16);
        }
        else
        {
            imgInput = vxoTensor_CreateImageFromTensor(input, itemCount, itemDepth, VX_DF_IMAGE_S16);
        }
    }

    if (outputFormat == VX_TYPE_INT8)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, depth, batch, VX_DF_IMAGE_U8);
    }
    else if (outputFormat == VX_TYPE_FLOAT16)
    {
        if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && width == 2 && dims == 2)
        {
            imgOutput = vxoTensor_CreateImageFromTensor(output, itemCount, itemDepth, VX_DF_IMAGE_S16);
        }
        else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && depth == 2)
        {
            imgOutput = vxoTensor_CreateImageFromTensor(output, itemCount, itemDepth, VX_DF_IMAGE_S16);
        }
        else if(itemCount == 1)
        {
            imgOutput = vxoTensor_CreateImageFromTensor(output, depth, batch, VX_DF_IMAGE_S16);
        }
        else
        {
            imgOutput = vxoTensor_CreateImageFromTensor(output, itemCount, itemDepth, VX_DF_IMAGE_S16);
        }
    }
    else if (outputFormat == VX_TYPE_FLOAT32)
    {
        if(itemCount == 1)
        {
            imgOutput = vxoTensor_CreateImageFromTensor(output, depth, batch, VX_DF_IMAGE_F32);
        }
        else
        {
            imgOutput = vxoTensor_CreateImageFromTensor(output, itemCount, itemDepth, VX_DF_IMAGE_F32);
        }
    }
    parameters[0] = (vx_reference)imgInput;
    parameters[1] = (vx_reference)imgOutput;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[5] = {NULL, NULL, NULL, NULL, NULL};

        char programSources_Softmax_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         _viv_uniform VXC_512Bits int8_uniPackMaxData_2x8; \n\
         _viv_uniform VXC_512Bits int8_uniPackMaxAndScale_2x8; \n\
         _viv_uniform VXC_512Bits int8_uniGetSubData0to3_4x4; \n\
         _viv_uniform VXC_512Bits int8_uniExtractHalf4_4x4; \n\
        _viv_uniform float int8_scaleIn; \n\
        _viv_uniform int int8_inputSize; \n\
        _viv_uniform int int8_inputWidth; \n\
        _viv_uniform int int8_inputWidthRemain4; \n\
        _viv_uniform int int8_isFp16; \n\
         \n\
         __kernel void vxcSoftmax_Int8(\n\
         __read_only image2d_t   input, \n\
         __write_only image2d_t  output) \n\
         { \n\
             int4 coord = (int4)(16, 0, 0, 0); \n\
            vxc_half8 scale; \n\
            float ProbFP32[4096]; \n\
            \n\
            vxc_char16 img_val0, img_val1; \n\
            vxc_char16 val = (vxc_char16)(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); \n\
            \n\
            do \n\
            { \n\
                VXC_ReadImage(img_val0, input, coord.xw, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(img_val1, input, coord.xw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                coord.x += 32; \n\
                \n\
                VXC_VertMax3_Integer(val, img_val0, img_val1, val, VXC_MODIFIER(0, 15, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Integer(val, val, VXC_MODIFIER(0, 13, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(val, val, img_val0, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0), int8_uniPackMaxData_2x8); \n\
                VXC_HorzMax3_Integer(val, val, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Integer(val, val, VXC_MODIFIER(0, 2, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Integer(val, val, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0)); \n\
            } \n\
            while(coord.x < int8_inputSize); \n\
            \n\
            half scaleIn_half; \n\
            _viv_asm(CONV, scaleIn_half, int8_scaleIn); \n\
            \n\
            VXC_DP2x8(scale, val, scaleIn_half, VXC_MODIFIER(0, 1, 0, VXC_RM_TowardZero, 0), int8_uniPackMaxAndScale_2x8); \n\
            \n\
            vxc_float4 prob; \n\
            float fProbSum = 0; \n\
            const float4 one4 = (float4)(1.0, 1.0, 1.0, 1.0); \n\
            \n\
            coord.xyz = 0; \n\
            \n\
            int idx = 0; \n\
            for (coord.x = 0; coord.x < int8_inputWidth; idx ++) \n\
            { \n\
                VXC_ReadImage(img_val0, input, coord.xw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP4x4(prob, img_val0, scale, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), int8_uniGetSubData0to3_4x4); \n\
                prob.x = exp(prob.x); \n\
                prob.y = exp(prob.y); \n\
                prob.z = exp(prob.z); \n\
                prob.w = exp(prob.w); \n\
                fProbSum += dot(prob, one4); \n\
                \n\
                vstore4(prob, idx, ProbFP32); \n\
                coord.x += 4; \n\
            } \n\
            VXC_ReadImage(img_val0, input, coord.xw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_DP4x4(prob, img_val0, scale, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), int8_uniGetSubData0to3_4x4); \n\
            if(int8_inputWidthRemain4 == 1) \n\
            { \n\
                prob.x = exp(prob.x); \n\
                prob.yzw = 0; \n\
                fProbSum += dot(prob, one4); \n\
                vstore4(prob, idx, ProbFP32); \n\
            } \n\
            else if(int8_inputWidthRemain4 == 2) \n\
            { \n\
                prob.x = exp(prob.x); \n\
                prob.y = exp(prob.y); \n\
                prob.zw = 0; \n\
                fProbSum += dot(prob, one4); \n\
                vstore4(prob, idx, ProbFP32); \n\
            } \n\
            else if(int8_inputWidthRemain4 == 3) \n\
            { \n\
                prob.x = exp(prob.x); \n\
                prob.y = exp(prob.y); \n\
                prob.z = exp(prob.z); \n\
                prob.w = 0; \n\
                fProbSum += dot(prob, one4); \n\
                vstore4(prob, idx, ProbFP32); \n\
            } \n\
            \n\
            vxc_float4 probSum_rcp; \n\
            probSum_rcp.x = 1 / fProbSum; \n\
            idx = 0; \n\
             if(int8_isFp16)\n\
             {\n\
                for (coord.x = 0; coord.x < int8_inputSize; idx ++) \n\
                { \n\
                    prob = vload4(idx, ProbFP32); \n\
                     \n\
                    prob = prob.xyzw * probSum_rcp.xxxx; \n\
                    half4 vec; \n\
                    vxc_half4 tmp; \n\
                    vxc_short4 dst; \n\
                    _viv_asm(CONV, vec, prob); \n\
                    VXC_DP4x4(tmp, vec, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), int8_uniExtractHalf4_4x4); \n\
                    _viv_asm(COPY, dst, tmp, 8); \n\
                    VXC_WriteImage(output, coord.xw, dst, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                     \n\
                    coord.x += 4; \n\
                } \n\
             }\n\
             else\n\
             {\n\
                for (coord.x = 0; coord.x < int8_inputSize; idx ++) \n\
                { \n\
                    prob = vload4(idx, ProbFP32); \n\
                    \n\
                    prob = prob.xyzw * probSum_rcp.xxxx; \n\
                    write_imagef(output, coord.xw, prob); \n\
                    coord.x += 4; \n\
                } \n\
             }\n\
         }\n"
        };

        char programSources_Softmax_1[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         _viv_uniform VXC_512Bits uniPackMaxData_2x8; \n\
         _viv_uniform VXC_512Bits uniGetSubData0to3_4x4; \n\
         _viv_uniform VXC_512Bits uniExtractHalf4_4x4; \n\
         _viv_uniform int inputSize; \n\
         _viv_uniform int inputWidth; \n\
         _viv_uniform int inputWidthRemain4; \n\
         _viv_uniform int fp16_isFp16; \n\
         __kernel void vxcSoftmax_Fp16(\n\
         __read_only image2d_t   input, \n\
         __write_only image2d_t  output) \n\
        { \n\
            int4 coord = (int4)(16, 0, 0, 0); \n\
            float ProbFP32[4096]; \n\
             \n\
            vxc_half8 img_val0, img_val1, img_val2, img_val3; \n\
            vxc_short8 val0, val1, val2, val3 = (vxc_short8)(0, 0, 0, 0, 0, 0, 0, 0); \n\
            vxc_half8 val; \n\
            _viv_asm(COPY, val, val3, 16); \n\
            do \n\
            { \n\
                VXC_ReadImage(val0, input, coord.xw, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, img_val0, val0, 16); \n\
                VXC_ReadImage(val1, input, coord.xw, VXC_5BITOFFSET_XY(-8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, img_val1, val1, 16); \n\
                VXC_ReadImage(val2, input, coord.xw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, img_val2, val2, 16); \n\
                VXC_ReadImage(val3, input, coord.xw, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, img_val3, val3, 16); \n\
                coord.x += 32; \n\
                 \n\
                VXC_VertMax3_Half(val, img_val0, img_val1, val, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_VertMax3_Half(val, img_val2, img_val3, val, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_HorzMax3_Half(val, val, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(val, val, val, VXC_MODIFIER(0, 2, 0, VXC_RM_TowardZero, 0), uniPackMaxData_2x8); \n\
                VXC_HorzMax3_Half(val, val, VXC_MODIFIER(0, 0, 0, VXC_RM_TowardZero, 0)); \n\
            } \n\
            while(coord.x < inputSize); \n\
             \n\
            vxc_float4 prob; \n\
            float fProbSum = 0; \n\
            const float4 one4 = (float4)(1.0, 1.0, 1.0, 1.0); \n\
            coord.xyzw = 0; \n\
             \n\
            int idx = 0; \n\
            for (coord.x = 0; coord.x < inputWidth; idx ++) \n\
            { \n\
                VXC_ReadImage(val0, input, coord.xw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                _viv_asm(COPY, img_val0, val0, 16); \n\
                VXC_DP4x4(prob, img_val0, val, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniGetSubData0to3_4x4); \n\
                prob.x = exp(prob.x); \n\
                prob.y = exp(prob.y); \n\
                prob.z = exp(prob.z); \n\
                prob.w = exp(prob.w); \n\
                fProbSum += dot(prob, one4); \n\
                 \n\
                vstore4(prob, idx, ProbFP32); \n\
                coord.x += 4; \n\
            } \n\
            VXC_ReadImage(val0, input, coord.xw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, img_val0, val0, 16); \n\
            VXC_DP4x4(prob, img_val0, val, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniGetSubData0to3_4x4); \n\
            if(inputWidthRemain4 == 1) \n\
            { \n\
                prob.x = exp(prob.x); \n\
                prob.yzw = 0; \n\
                fProbSum += dot(prob, one4); \n\
                vstore4(prob, idx, ProbFP32); \n\
            } \n\
            else if(inputWidthRemain4 == 2) \n\
            { \n\
                prob.x = exp(prob.x); \n\
                prob.y = exp(prob.y); \n\
                prob.zw = 0; \n\
                fProbSum += dot(prob, one4); \n\
                vstore4(prob, idx, ProbFP32); \n\
            } \n\
            else if(inputWidthRemain4 == 3) \n\
            { \n\
                prob.x = exp(prob.x); \n\
                prob.y = exp(prob.y); \n\
                prob.z = exp(prob.z); \n\
                prob.w = 0; \n\
                fProbSum += dot(prob, one4); \n\
                vstore4(prob, idx, ProbFP32); \n\
            } \n\
             \n\
            vxc_float4 probSum_rcp; \n\
            probSum_rcp.x = 1 / fProbSum; \n\
            idx = 0; \n\
            if(fp16_isFp16)\n\
            {\n\
                for (coord.x = 0; coord.x < inputSize; idx ++) \n\
                { \n\
                    prob = vload4(idx, ProbFP32); \n\
                     \n\
                    prob = prob.xyzw * probSum_rcp.xxxx; \n\
                    half4 vec; \n\
                    vxc_half4 tmp; \n\
                    vxc_short4 dst; \n\
                    _viv_asm(CONV, vec, prob); \n\
                    VXC_DP4x4(tmp, vec, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniExtractHalf4_4x4); \n\
                    _viv_asm(COPY, dst, tmp, 8); \n\
                    VXC_WriteImage(output, coord.xw, dst, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                     \n\
                    coord.x += 4; \n\
                } \n\
            }\n\
            else\n\
            {\n\
                for (coord.x = 0; coord.x < inputSize; idx ++) \n\
                { \n\
                    prob = vload4(idx, ProbFP32); \n\
                    \n\
                    prob = prob.xyzw * probSum_rcp.xxxx; \n\
                    write_imagef(output, coord.xw, prob); \n\
                    coord.x += 4; \n\
                } \n\
            }\n\
        }\n"
        };

        char programSources_Softmax_2[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         _viv_uniform VXC_512Bits uniSubMax2FP32_Lo; \n\
         _viv_uniform VXC_512Bits uniSubMax2FP32_Hi; \n\
        _viv_uniform VXC_512Bits uniExtractHalf8_2x8; \n\
        __kernel void vxcSoftmax_Fp16toFp16_channel2(\n\
            __read_only image2d_t   input, \n\
            __write_only image2d_t  output) \n\
        { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(0), get_global_id(1)); \n\
            vxc_short8 in0, in1; \n\
            vxc_half8 vec0, vec1, max; \n\
            vxc_float4 data0, data1, fProbSum; \n\
             \n\
            VXC_ReadImage(in0, input, coord.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, vec0, in0, 16); \n\
            VXC_ReadImage(in1, input, coord.xy, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, vec1, in1, 16); \n\
             \n\
            coord.zw += (int2)(4, 1); \n\
            VXC_VertMax3_Half(max, vec0, vec0, vec1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_DP4x4(data0, vec0, max, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniSubMax2FP32_Lo); \n\
            VXC_DP4x4(data1, vec1, max, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniSubMax2FP32_Lo); \n\
            data0.x = exp(data0.x); \n\
            data0.y = exp(data0.y); \n\
            data0.z = exp(data0.z); \n\
            data0.w = exp(data0.w); \n\
            data1.x = exp(data1.x); \n\
            data1.y = exp(data1.y); \n\
            data1.z = exp(data1.z); \n\
            data1.w = exp(data1.w); \n\
            fProbSum = data0 + data1; \n\
            fProbSum = 1 / fProbSum; \n\
            data0 *= fProbSum; \n\
            data1 *= fProbSum; \n\
             \n\
            half4 vect0, vect1; \n\
            vxc_half8 tmp; \n\
            vxc_short8 dst; \n\
            _viv_asm(CONV, vect0, data0); \n\
            _viv_asm(CONV, vect1, data1); \n\
             \n\
            VXC_DP2x8(tmp, vect0, vect1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniExtractHalf8_2x8); \n\
            _viv_asm(COPY, dst, tmp, 16); \n\
            VXC_WriteImage(output, coord.xy, dst, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord.xw, dst, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP4x4(data0, vec0, max, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniSubMax2FP32_Hi); \n\
            VXC_DP4x4(data1, vec1, max, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniSubMax2FP32_Hi); \n\
            data0.x = exp(data0.x); \n\
            data0.y = exp(data0.y); \n\
            data0.z = exp(data0.z); \n\
            data0.w = exp(data0.w); \n\
            data1.x = exp(data1.x); \n\
            data1.y = exp(data1.y); \n\
            data1.z = exp(data1.z); \n\
            data1.w = exp(data1.w); \n\
            fProbSum = data0 + data1; \n\
            fProbSum = 1 / fProbSum; \n\
            data0 *= fProbSum; \n\
            data1 *= fProbSum; \n\
             \n\
            _viv_asm(CONV, vect0, data0); \n\
            _viv_asm(CONV, vect1, data1); \n\
             \n\
            VXC_DP2x8(tmp, vect0, vect1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniExtractHalf8_2x8); \n\
            _viv_asm(COPY, dst, tmp, 16); \n\
            VXC_WriteImage(output, coord.zy, dst, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord.zw, dst, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
        }\n\
        _viv_uniform VXC_512Bits uniExtract2Half_2x8; \n\
        _viv_uniform VXC_512Bits uniPackedEvenData_2x8; \n\
        _viv_uniform VXC_512Bits uniPackedOddData_2x8; \n\
        _viv_uniform VXC_512Bits uniDataLoSubMaxLo_4x4; \n\
        _viv_uniform VXC_512Bits uniDataHiSubMaxLo_4x4; \n\
        _viv_uniform VXC_512Bits uniDataLoSubMaxHi_4x4; \n\
        _viv_uniform VXC_512Bits uniDataHiSubMaxHi_4x4; \n\
        __kernel void vxcSoftmax_Fp16toFp16_D2C2(\n\
            __read_only image2d_t   input, \n\
            __write_only image2d_t  output) \n\
        { \n\
            int4 coord = (int4)(get_global_id(0), get_global_id(1), get_global_id(0), get_global_id(1)); \n\
            vxc_short8 in0, in1; \n\
            vxc_half8 vec0, vec1, vec2, vec3, max; \n\
            vxc_float4 data0, data1, fProbSum; \n\
             \n\
            VXC_ReadImage(in0, input, coord.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, vec0, in0, 16); \n\
            VXC_ReadImage(in1, input, coord.xy, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, vec1, in1, 16); \n\
             \n\
            coord.zw += (int2)(8, 0); \n\
            VXC_DP2x8(vec2, vec0, vec1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniPackedEvenData_2x8); \n\
            VXC_DP2x8(vec3, vec0, vec1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniPackedOddData_2x8); \n\
            VXC_VertMax3_Half(max, vec2, vec2, vec3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP4x4(data0, vec0, max, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniDataLoSubMaxLo_4x4); \n\
            VXC_DP4x4(data1, vec0, max, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniDataHiSubMaxLo_4x4); \n\
            data0.x = exp(data0.x); \n\
            data0.y = exp(data0.y); \n\
            data0.z = exp(data0.z); \n\
            data0.w = exp(data0.w); \n\
            data1.x = exp(data1.x); \n\
            data1.y = exp(data1.y); \n\
            data1.z = exp(data1.z); \n\
            data1.w = exp(data1.w); \n\
            fProbSum.xy = data0.xz + data0.yw; \n\
            fProbSum.zw = data1.xz + data1.yw; \n\
            fProbSum = 1 / fProbSum; \n\
            data0 *= fProbSum.xxyy; \n\
            data1 *= fProbSum.zzww; \n\
             \n\
            half4 vect0, vect1; \n\
            vxc_half8 tmp; \n\
            vxc_short8 dst; \n\
            _viv_asm(CONV, vect0, data0); \n\
            _viv_asm(CONV, vect1, data1); \n\
             \n\
            VXC_DP2x8(tmp, vect0, vect1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniExtract2Half_2x8); \n\
            _viv_asm(COPY, dst, tmp, 16); \n\
            VXC_WriteImage(output, coord.xy, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP4x4(data0, vec1, max, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniDataLoSubMaxHi_4x4); \n\
            VXC_DP4x4(data1, vec1, max, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniDataHiSubMaxHi_4x4); \n\
            data0.x = exp(data0.x); \n\
            data0.y = exp(data0.y); \n\
            data0.z = exp(data0.z); \n\
            data0.w = exp(data0.w); \n\
            data1.x = exp(data1.x); \n\
            data1.y = exp(data1.y); \n\
            data1.z = exp(data1.z); \n\
            data1.w = exp(data1.w); \n\
            fProbSum.xy = data0.xz + data0.yw; \n\
            fProbSum.zw = data1.xz + data1.yw; \n\
            fProbSum = 1 / fProbSum; \n\
            data0 *= fProbSum.xxyy; \n\
            data1 *= fProbSum.zzww; \n\
             \n\
            _viv_asm(CONV, vect0, data0); \n\
            _viv_asm(CONV, vect1, data1); \n\
            VXC_DP2x8(tmp, vect0, vect1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniExtract2Half_2x8); \n\
            _viv_asm(COPY, dst, tmp, 16); \n\
            VXC_WriteImage(output, coord.zy, dst, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
        }\n"
        };

        if (context->evisNoInst.isVX2)
        {
            programSources[0] = VXC_VertMax3_Evis2;
            programLength[0] = strlen(VXC_VertMax3_Evis2);
            programSources[1] = VXC_HorzMax3_Evis2;
            programLength[1] = strlen(VXC_HorzMax3_Evis2);
            programSources[2] = programSources_Softmax_0;
            programLength[2] = strlen(programSources_Softmax_0);
            programSources[3] = programSources_Softmax_1;
            programLength[3] = strlen(programSources_Softmax_1);
            programSources[4] = programSources_Softmax_2;
            programLength[4] = strlen(programSources_Softmax_2);
        }
        else
        {
            programSources[0] = VXC_VertMax3_Evis1;
            programLength[0] = strlen(VXC_VertMax3_Evis1);
            programSources[1] = VXC_HorzMax3_Evis1;
            programLength[1] = strlen(VXC_HorzMax3_Evis1);
            programSources[2] = programSources_Softmax_0;
            programLength[2] = strlen(programSources_Softmax_0);
            programSources[3] = programSources_Softmax_1;
            programLength[3] = strlen(programSources_Softmax_1);
            programSources[4] = programSources_Softmax_2;
            programLength[4] = strlen(programSources_Softmax_2);
        }

        program = vxCreateProgramWithSource(context, 5, (const vx_char**)programSources, programLength);


        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcSoftmax", program, 2, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && width == 2 && dims == 2)
    {
        vx_uint32 uniExtract2Half_2x8[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x06040200, 0x06040200, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001 // Constant
        };
        vx_uint32 uniPackedEvenData_2x8[16] = {
            0x11112111, // TCfg
            0x11110000, // ASelt
            0x06040200, 0x06040200, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
        };
        vx_uint32 uniPackedOddData_2x8[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x07050301, 0x07050301, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
        };
        vx_uint32 uniDataLoSubMaxLo_4x4[16] = {
            0x09090909, // TCfg
            0x04040404, // ASelt
            0x00010000, 0x00130012, // ABin
            0x0a0a0a0a, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000 // Constant
        };
        vx_uint32 uniDataHiSubMaxLo_4x4[16] = {
            0x09090909, // TCfg
            0x04040404, // ASelt
            0x00250024, 0x00370036, // ABin
            0x0a0a0a0a, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000 // Constant
        };
        vx_uint32 uniDataLoSubMaxHi_4x4[16] = {
            0x09090909, // TCfg
            0x04040404, // ASelt
            0x00410040, 0x00530052, // ABin
            0x0a0a0a0a, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000 // Constant
        };
        vx_uint32 uniDataHiSubMaxHi_4x4[16] = {
            0x09090909, // TCfg
            0x04040404, // ASelt
            0x00650064, 0x00770076, // ABin
            0x0a0a0a0a, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000, 0x3c003c00, 0x00000000 // Constant
        };

        if(outputFormat == VX_TYPE_FLOAT16)
        {
            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toFp16_D2C2", borderMode);
            if (!shaderExecutable) goto error;
        }

        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniPackedOddData_2x8", 1, uniPackedOddData_2x8);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniDataLoSubMaxLo_4x4", 1, uniDataLoSubMaxLo_4x4);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniDataHiSubMaxLo_4x4", 1, uniDataHiSubMaxLo_4x4);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniDataLoSubMaxHi_4x4", 1, uniDataLoSubMaxHi_4x4);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniDataHiSubMaxHi_4x4", 1, uniDataHiSubMaxHi_4x4);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniPackedEvenData_2x8", 1, uniPackedEvenData_2x8);
        status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniExtract2Half_2x8", 1, uniExtract2Half_2x8);
        if (status != VX_SUCCESS) goto error;

        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 1;
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((itemCount + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = 1;
    }
    else if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16 && depth == 2)
    {
        if(inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
        {
            vx_uint32 uniSubMax2FP32_Lo[16] = {
                0x05050505, // TCfg
                0x04040404, // ASelt
                0x00110000, 0x00330022, // ABin
                0x0a0a0a0a, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0xffff0001, 0x00000000, 0xffff0001, 0x00000000, 0xffff0001, 0x00000000, 0xffff0001, 0x00000000 // Constant
            };
            vx_uint32 uniSubMax2FP32_Hi[16] = {
                0x05050505, // TCfg
                0x04040404, // ASelt
                0x00550044, 0x00770066, // ABin
                0x0a0a0a0a, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0xffff0001, 0x00000000, 0xffff0001, 0x00000000, 0xffff0001, 0x00000000, 0xffff0001, 0x00000000 // Constant
            };
            vx_uint32 uniExtractHalf8_2x8[16] = {
                0x11111111, // TCfg
                0x11110000, // ASelt
                0x06040200, 0x06040200, // ABin
                0x22222222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001, 0x00000001 // Constant
            };

            if(outputFormat == VX_TYPE_FLOAT16)
            {
                shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16toFp16_channel2", borderMode);
                if (!shaderExecutable) goto error;
            }

            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniSubMax2FP32_Lo", 1, uniSubMax2FP32_Lo);
            status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniSubMax2FP32_Hi", 1, uniSubMax2FP32_Hi);
            status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniExtractHalf8_2x8", 1, uniExtractHalf8_2x8);
            if (status != VX_SUCCESS) goto error;

        }
        execution_parameters.globalWorkScale[0]  = 8;
        execution_parameters.globalWorkScale[1]  = 2;
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((itemCount + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((itemDepth + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    }
    else if(itemCount == 1)
    {
        if(inputFormat == VX_TYPE_INT8 && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32))
        {
            vx_uint32 uniPackMaxData_2x8[16] = {
                0x00333333, // TCfg
                0x00000000, // ASelt
                0x09060300, 0x00000d0c, // ABin
                0x00000000, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00004400, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 uniPackMaxAndScale_2x8[16] = {
                0x00000013, // TCfg
                0x00000010, // ASelt
                0x00000000, 0x00000000, // ABin
                0x00000020, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00004400, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 uniGetSubData0to3_4x4[16] = {
                0x09090909, // TCfg
                0x04040404, // ASelt
                0x00010000, 0x00030002, // ABin
                0x05050505, // BSelt
                0x00110011, 0x00110011, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 uniExtractHalf4_4x4[16] = {
                0x01010101, // TCfg
                0x00000000, // ASelt
                0x00020000, 0x00060004, // ABin
                0x02020202, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000000 // Constant
            };

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Int8", borderMode);
            if (!shaderExecutable) goto error;
            if(outputFormat == VX_TYPE_FLOAT16)
                int8_isFp16 = 1;
            else if(outputFormat == VX_TYPE_FLOAT32)
                int8_isFp16 = 0;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "int8_isFp16", 1, &int8_isFp16);
            status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8_uniExtractHalf4_4x4", 1, uniExtractHalf4_4x4);
            status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8_uniPackMaxAndScale_2x8", 1, uniPackMaxAndScale_2x8);
            status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8_uniGetSubData0to3_4x4", 1, uniGetSubData0to3_4x4);
            status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8_uniPackMaxData_2x8", 1, uniPackMaxData_2x8);
            if (status != VX_SUCCESS) goto error;

            status = vxnneShaderExecutable_SetUniform(shaderExecutable, "int8_scaleIn", 1, &scaleIn);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8_inputWidth", 1, &inputWidth);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8_inputWidthRemain4", 1, &inputWidthRemain4);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "int8_inputSize", 1, &depth);
            if (status != VX_SUCCESS) goto error;
        }
        else if(inputFormat == VX_TYPE_FLOAT16 && (outputFormat == VX_TYPE_FLOAT16 || outputFormat == VX_TYPE_FLOAT32))
        {
            vx_uint32 uniPackMaxData_2x8[16] = {
                0x00000111, // TCfg
                0x00000000, // ASelt
                0x00050300, 0x00000000, // ABin
                0x00000222, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00004400, // AccumType, ConstantType, and PostShift
                0x00000001, 0x00000001, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
            };
            vx_uint32 uniGetSubData0to3_4x4[16] = {
                0x09090909, // TCfg
                0x04040404, // ASelt
                0x00010000, 0x00030002, // ABin
                0x0a0a0a0a, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0x00010001, 0x00000000, 0x00010001, 0x00000000, 0x00010001, 0x00000000, 0x00010001, 0x00000000 // Constant
            };
            vx_uint32 uniExtractHalf4_4x4[16] = {
                0x01010101, // TCfg
                0x00000000, // ASelt
                0x00020000, 0x00060004, // ABin
                0x02020202, // BSelt
                0x00000000, 0x00000000, // BBin
                0x00000400, // AccumType, ConstantType, and PostShift
                0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000000, 0x00000001, 0x00000000 // Constant
            };

            shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16", borderMode);
            if (!shaderExecutable) goto error;
            if(outputFormat == VX_TYPE_FLOAT16)
                fp16_isFp16 = 1;
            else if(outputFormat == VX_TYPE_FLOAT32)
                fp16_isFp16 = 0;
            status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "fp16_isFp16", 1, &fp16_isFp16);
            status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniExtractHalf4_4x4", 1, uniExtractHalf4_4x4);
            status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniGetSubData0to3_4x4", 1, uniGetSubData0to3_4x4);
            status  |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniPackMaxData_2x8", 1, uniPackMaxData_2x8);
            if (status != VX_SUCCESS) goto error;

            status = vxnneShaderExecutable_SetUniform(shaderExecutable, "inputWidth", 1, &inputWidth);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "inputWidthRemain4", 1, &inputWidthRemain4);
            status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "inputSize", 1, &depth);
            if (status != VX_SUCCESS) goto error;
        }

        execution_parameters.globalWorkScale[0]  = 1;
        execution_parameters.globalWorkScale[1]  = 1;
        execution_parameters.localWorkSize[0]    = 1;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.globalWorkSize[0]   = 1;
        execution_parameters.globalWorkSize[1]   = 1;
    }

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}

/********vxcVertMaxPool****************************************************/
vxnne_shader_executable vxnneVertMaxPoolShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32                pool_width,
    vx_uint32                pool_height,
    vx_tensor               output)
{
    vx_size    programLength[3] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[6]     = {(vx_reference)input, NULL, NULL, NULL, NULL, (vx_reference)output};
    vx_uint32    width             = input->tensorBuffer->memory.dims[0][0];
    vx_uint32    height            = input->tensorBuffer->memory.dims[0][1];
    vx_uint32    depth             = input->tensorBuffer->memory.dims[0][2];
    vx_image     imgInput          = NULL;
    vx_image     imgOutput         = NULL;
    vx_scalar    width_s           = NULL;
    vx_scalar    outWidth_s        = NULL;
    vx_scalar    height_s          = NULL;
    vx_scalar    depth_s           = NULL;
    vx_uint32    outputWidth       = width * height;
    vx_uint32    outputheight      = output->tensorBuffer->memory.dims[0][2];
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_enum      outputFormat      = output->tensorBuffer->dataFormat;

    borderMode->mode = VX_BORDER_REPLICATE;
    if (inputFormat == VX_TYPE_INT8)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, width, height * depth, VX_DF_IMAGE_U8);
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, width, height * depth, VX_DF_IMAGE_U16);
    }

    if (outputFormat == VX_TYPE_INT8)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, outputWidth, outputheight, VX_DF_IMAGE_U8);
    }
    else if (outputFormat == VX_TYPE_FLOAT16)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, outputWidth, outputheight, VX_DF_IMAGE_U16);
    }

    width_s       = vxCreateScalar(context, VX_TYPE_INT32, &width);
    outWidth_s    = vxCreateScalar(context, VX_TYPE_INT32, &outputWidth);
    height_s      = vxCreateScalar(context, VX_TYPE_INT32, &height);
    depth_s       = vxCreateScalar(context, VX_TYPE_INT32, &depth);
    parameters[0] = (vx_reference)imgInput;
    parameters[1] = (vx_reference)width_s;
    parameters[2] = (vx_reference)outWidth_s;
    parameters[3] = (vx_reference)height_s;
    parameters[4] = (vx_reference)depth_s;
    parameters[5] = (vx_reference)imgOutput;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[3] = {NULL, NULL, NULL};

        char programSources_vertMaxPool[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         #define VERT_IMAGE_NUM        (4) \n\
         #define VERT_REMAIN        (3) \n\
         __kernel void vertMaxPool_20x16_6x6(\n\
         __read_only image2d_t    input, \n\
         int        width, \n\
         int        width_stride, \n\
         int        height, \n\
         int        depth, \n\
         __write_only image2d_t    output) \n\
        { \n\
            vxc_short8 lineA; \n\
            vxc_short8 lineB; \n\
            vxc_short8 lineC; \n\
            vxc_short8 lineD; \n\
            vxc_short8 maxLine; \n\
            int2 coord_in = (int2)(get_global_id(0), get_global_id(1)); \n\
            vxc_int4 coord_out; \n\
            coord_out.x = coord_in.x << 0; \n\
            coord_out.y = coord_in.y << 2; \n\
            coord_out.z = coord_in.y << 2; \n\
            coord_out.w = coord_in.y << 2; \n\
            coord_in.y *= height; \n\
             \n\
            coord_out += (int4)(0, 0, 1, 0); \n\
             \n\
            VXC_ReadImage(lineA, input, coord_in.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 4, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(lineB, input, coord_in.xy, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 4, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage(lineC, input, coord_in.xy, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 4, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            int loopCnt = 0; \n\
            do \n\
            { \n\
                VXC_ReadImage(lineD, input, coord_in.xy, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 4, 0, VXC_RM_TowardZero, 0)); \n\
                //0 \n\
                VXC_WriteImage(output, coord_out.xy, lineA, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
                //1 \n\
                VXC_VertMax3_Integer(maxLine, lineA, lineB, lineA, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_WriteImage(output, coord_out.xz, maxLine, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
                coord_out.yz += 2; \n\
                //2 \n\
                VXC_VertMax3_Integer(maxLine, maxLine, maxLine, lineC, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_WriteImage(output, coord_out.xy, maxLine, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
                //3 \n\
                VXC_VertMax3_Integer(maxLine, maxLine, maxLine, lineD, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_WriteImage(output, coord_out.xz, maxLine, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
                 \n\
                lineA = lineB; \n\
                lineB = lineC; \n\
                lineC = lineD; \n\
                 \n\
                coord_out.yz -= 2; \n\
                coord_out.x += width; \n\
                coord_in.y ++; \n\
                loopCnt ++; \n\
            } while (loopCnt < height - VERT_REMAIN); \n\
             \n\
            //height = -2 \n\
            //1 \n\
            VXC_WriteImage(output, coord_out.xy, lineA, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            //2 \n\
            VXC_VertMax3_Integer(maxLine, lineA, lineB, lineA, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord_out.xz, maxLine, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            coord_out.yz += 2; \n\
            VXC_VertMax3_Integer(maxLine, maxLine, maxLine, lineC, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            //3 \n\
            VXC_WriteImage(output, coord_out.xy, maxLine, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord_out.xz, maxLine, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            coord_out.yz -= 2; \n\
            coord_out.x += width; \n\
            //height = -1 \n\
            //1 \n\
            VXC_WriteImage(output, coord_out.xy, lineB, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            //2 \n\
            VXC_VertMax3_Integer(maxLine, lineB, lineC, lineB, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord_out.xz, maxLine, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            coord_out.yz += 2; \n\
            VXC_WriteImage(output, coord_out.xy, maxLine, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord_out.xz, maxLine, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            coord_out.yz -= 2; \n\
            coord_out.x += width; \n\
            //height = 0 \n\
            //1 \n\
            VXC_WriteImage(output, coord_out.xy, lineC, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord_out.xz, lineC, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            coord_out.yz += 2; \n\
            VXC_WriteImage(output, coord_out.xy, lineC, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
            VXC_WriteImage(output, coord_out.xz, lineC, VXC_MODIFIER(0, 4, 0,VXC_RM_TowardZero, 0)); \n\
        }\n"
        };

        if(inputFormat == VX_TYPE_FLOAT16)
        {
            if (context->evisNoInst.isVX2)
            {
                programSources[0] = VXC_VertMax3_Evis2;
                programSources[1] = VXC_HorzMax3_Evis2;
                programSources[2] = programSources_vertMaxPool;

                programLength[0]  = strlen(VXC_VertMax3_Evis2);
                programLength[1]  = strlen(VXC_HorzMax3_Evis2);
                programLength[2]  = strlen(programSources_vertMaxPool);
            }
            else
            {
                programSources[0] = VXC_VertMax3_Evis1;
                programSources[1] = VXC_HorzMax3_Evis1;
                programSources[2] = programSources_vertMaxPool;

                programLength[0]  = strlen(VXC_VertMax3_Evis1);
                programLength[1]  = strlen(VXC_HorzMax3_Evis1);
                programLength[2]  = strlen(programSources_vertMaxPool);
            }
        }
        program = vxCreateProgramWithSource(context, 3, (const vx_char**)programSources, programLength);


        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vertMaxPool", program, 6, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if(inputFormat == VX_TYPE_FLOAT16 && width == 20 && height == 16 /*&& pool_width == 6 && pool_height == 6*/)
    {

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_20x16_6x6", borderMode);
        if (!shaderExecutable) goto error;

        execution_parameters.globalWorkScale[0]  = 5;
        execution_parameters.globalWorkScale[1]  = 1;
        execution_parameters.localWorkSize[0]    = 4;
        execution_parameters.localWorkSize[1]    = 2;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((depth  + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 6);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);
    if(width_s)vxReleaseScalar(&width_s);
    if(outWidth_s)vxReleaseScalar(&outWidth_s);
    if(height_s)vxReleaseScalar(&height_s);
    if(depth_s)vxReleaseScalar(&depth_s);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);
    if(width_s)vxReleaseScalar(&width_s);
    if(outWidth_s)vxReleaseScalar(&outWidth_s);
    if(height_s)vxReleaseScalar(&height_s);
    if(depth_s)vxReleaseScalar(&depth_s);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}

/********vxcpreTreatedRect****************************************************/
vxnne_shader_executable vxnnePreTreatedRectShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32                rois_stride,
    vx_uint32                rois_num,
    vx_uint32               imgWid,
    vx_uint32               imgHeight,
    vx_float32              spatial_scale,
    vx_tensor               output)
{
    vx_size    programLength[2] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[5]     = {(vx_reference)input, NULL, NULL, NULL, (vx_reference)output};
    vx_image     imgInput          = NULL;
    vx_image     imgOutput         = NULL;
    vx_scalar     width_s          = NULL;
    vx_scalar     height_s         = NULL;
    vx_scalar     spatial_s        = NULL;
    vx_uint32    outputWidth       = output->tensorBuffer->memory.dims[0][0];
    vx_uint32    outputheight      = output->tensorBuffer->memory.dims[0][1];
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_enum      outputFormat      = output->tensorBuffer->dataFormat;

    borderMode->mode = VX_BORDER_REPLICATE;
    if (inputFormat == VX_TYPE_INT8)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, rois_stride, rois_num, VX_DF_IMAGE_U8);
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, rois_stride, rois_num, VX_DF_IMAGE_U16);
    }

    if (outputFormat == VX_TYPE_INT8)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, outputWidth, outputheight, VX_DF_IMAGE_U8);
    }
    else if (outputFormat == VX_TYPE_FLOAT16)
    {
        imgOutput = vxoTensor_CreateImageFromTensor(output, outputWidth, outputheight, VX_DF_IMAGE_S16);
    }

    width_s          = vxCreateScalar(context, VX_TYPE_INT32, &imgWid);
    height_s      = vxCreateScalar(context, VX_TYPE_INT32, &imgHeight);
    spatial_s      = vxCreateScalar(context, VX_TYPE_FLOAT32, &spatial_scale);
    parameters[0] = (vx_reference)imgInput;
    parameters[1] = (vx_reference)width_s;
    parameters[2] = (vx_reference)height_s;
    parameters[3] = (vx_reference)spatial_s;
    parameters[4] = (vx_reference)imgOutput;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[2] = {NULL, NULL};

        char programSources_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         _viv_uniform VXC_512Bits uniGetRoiRectPos; \n\
        #define EPS        (0.000002f); \n\
        __kernel void preTreatedRect_6x6_fp16toS16(\n\
            __read_only image2d_t    rois, \n\
            int        width, \n\
            int        height, \n\
            float   spatial_scale, \n\
            __write_only image2d_t    output) \n\
        { \n\
            short wstart_arr[6]; \n\
            short hstart_arr[6]; \n\
            short hLen_arr[6]; \n\
             \n\
            int2 coord = (int2)(get_global_id(0), get_global_id(1)); \n\
             \n\
            vxc_half8 rect_roi_coord; \n\
            vxc_float4 roi_pos_fl32; \n\
            vxc_int4 roi_pos; \n\
            float roi_width; \n\
            float roi_height; \n\
            float bin_size_h = 0, bin_size_w = 0; \n\
            float pooled_width = 6.0; \n\
            float pooled_height = 6.0; \n\
            int pool_width = 6; \n\
            int pool_height = 6; \n\
            half scale; \n\
             \n\
            vxc_ushort8 tmp; \n\
            VXC_ReadImage(tmp, rois, coord.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 4, 0, VXC_RM_TowardZero, 0)); \n\
            _viv_asm(COPY, rect_roi_coord, tmp, 16); \n\
            _viv_asm(CONV, scale, spatial_scale); \n\
             \n\
            VXC_DP4x4(roi_pos_fl32, rect_roi_coord, scale, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniGetRoiRectPos); \n\
             \n\
            roi_pos_fl32 += 0.5f; \n\
            roi_pos_fl32 = floor(roi_pos_fl32); \n\
            \n\
            roi_pos        = convert_int4_rtz(roi_pos_fl32); \n\
             \n\
            roi_width    = roi_pos_fl32.z - roi_pos_fl32.x + 1; \n\
            roi_height    = roi_pos_fl32.w - roi_pos_fl32.y + 1; \n\
             \n\
            bin_size_h = roi_height / pooled_width; \n\
            bin_size_w = roi_width  / pooled_height; \n\
             \n\
            int2 coord_out =  coord; \n\
             \n\
            float x = 0; \n\
            for (int idx = 0; idx < pool_width; idx ++) \n\
            { \n\
                float wstart0    = x * bin_size_w; \n\
                float hstart0    = x * bin_size_h; \n\
                float wend0        = (x + 1) * bin_size_w - EPS; \n\
                float hend0        = (x + 1) * bin_size_h - EPS; \n\
                 \n\
                int wstart    = floor(wstart0) + roi_pos.x; \n\
                int wend    = ceil(wend0) + roi_pos.x; \n\
                int hstart    = floor(hstart0) + roi_pos.y; \n\
                int hend    = ceil(hend0) + roi_pos.y; \n\
                 \n\
                wstart    = max(wstart, 0); \n\
                wstart    = min(wstart, width); \n\
                wend    = max(wend, 0); \n\
                wend    = min(wend, width); \n\
                hstart    = max(hstart, 0); \n\
                hstart    = min(hstart, height); \n\
                hend    = max(hend, 0); \n\
                hend    = min(hend, height); \n\
                 \n\
                int len = wend - wstart; \n\
                 \n\
                wstart_arr[idx] = wstart; \n\
                hstart_arr[idx] = hstart; \n\
                hLen_arr[idx] = hend - hstart; \n\
                 \n\
                coord_out.x = idx; \n\
                if(len == 0) \n\
                { \n\
                    short mask = 0x00; \n\
                    VXC_WriteImage(output, coord_out.xy, mask, VXC_MODIFIER(0, 0, 0,VXC_RM_TowardZero, 0)); \n\
                } \n\
                else if(len == 1) \n\
                { \n\
                    short mask = 0x01; \n\
                    VXC_WriteImage(output, coord_out.xy, mask, VXC_MODIFIER(0, 0, 0,VXC_RM_TowardZero, 0)); \n\
                } \n\
                else if(len == 2) \n\
                { \n\
                    short mask = 0x03; \n\
                    VXC_WriteImage(output, coord_out.xy, mask, VXC_MODIFIER(0, 0, 0,VXC_RM_TowardZero, 0)); \n\
                } \n\
                else if(len == 3) \n\
                { \n\
                    short mask = 0x07; \n\
                    VXC_WriteImage(output, coord_out.xy, mask, VXC_MODIFIER(0, 0, 0,VXC_RM_TowardZero, 0)); \n\
                } \n\
                else if(len == 4) \n\
                { \n\
                    short mask = 0x0F; \n\
                    VXC_WriteImage(output, coord_out.xy, mask, VXC_MODIFIER(0, 0, 0,VXC_RM_TowardZero, 0)); \n\
                } \n\
                 \n\
                coord_out.x = 6 + idx; \n\
                short szx = (short)len; \n\
                VXC_WriteImage(output, coord_out.xy, szx, VXC_MODIFIER(0, 0, 0,VXC_RM_TowardZero, 0)); \n\
                 \n\
                x += 1.0; \n\
            } \n\
             \n\
            for (int y = 0; y < pool_height; y++) \n\
            { \n\
                for (int x = 0; x < pool_width; x++) \n\
                { \n\
                    vxc_short2 posXY; \n\
                    posXY.y = hLen_arr[y]; \n\
                    short x0 =  wstart_arr[x]; \n\
                    short y0 =  hstart_arr[y]; \n\
                     \n\
                    if(posXY.y == 0) \n\
                        posXY.x = -16; \n\
                    else \n\
                        posXY.x = y0 * width + x0; \n\
                     \n\
                    posXY.y -= 1; \n\
                     \n\
                    coord_out.x = 12 + ((y * pool_width + x) << 1); \n\
                    VXC_WriteImage(output, coord_out.xy, posXY, VXC_MODIFIER(0, 1, 0,VXC_RM_TowardZero, 0)); \n\
                } \n\
            } \n\
        }\n"
        };

        if(inputFormat == VX_TYPE_FLOAT16)
        {
            programSources[0] = programSources_0;
            programLength[0] = strlen(programSources_0);
        }
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, programLength);

        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "preTreatedRect", program, 5, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if(inputFormat == VX_TYPE_FLOAT16)
    {
        vx_uint32 uniGetRoiRectPos[16] = {
            0x01010101, // TCfg
            0x00000000, // ASelt
            0x00020001, 0x00040003, // ABin
            0x01010101, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_6x6_fp16toS16", borderMode);
        if (!shaderExecutable) goto error;

        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniGetRoiRectPos", 1, uniGetRoiRectPos);
        if (status != VX_SUCCESS) goto error;

        execution_parameters.globalWorkScale[0]  = 1;
        execution_parameters.globalWorkScale[1]  = 1;
        execution_parameters.localWorkSize[0]    = 1;
        execution_parameters.localWorkSize[1]    = 8;
        execution_parameters.globalWorkSize[0]   = 1;
        execution_parameters.globalWorkSize[1]   = gcmALIGN((rois_num  + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 5);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);
    if(width_s)vxReleaseScalar(&width_s);
    if(height_s)vxReleaseScalar(&height_s);
    if(spatial_s)vxReleaseScalar(&spatial_s);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgOutput)vxReleaseImage(&imgOutput);
    if(width_s)vxReleaseScalar(&width_s);
    if(height_s)vxReleaseScalar(&height_s);
    if(spatial_s)vxReleaseScalar(&spatial_s);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}


/********vxcHorzMaxPoolRect****************************************************/
vxnne_shader_executable vxnneHorzMaxPoolShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_tensor               rois,
    vx_tensor               output
    )
{
    vx_size    programLength[4] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[3]     = {(vx_reference)input, (vx_reference)rois, (vx_reference)output};
    vx_uint32    output_size[4];
    vx_uint32    width             = input->tensorBuffer->memory.dims[0][0];
    vx_uint32    height            = input->tensorBuffer->memory.dims[0][1];
    vx_uint32    depth             = input->tensorBuffer->memory.dims[0][2];
    vx_uint32    output_channel    = 0;
    vx_uint32    output_rois       = 0;
    vx_uint32    rois_width        = rois->tensorBuffer->memory.dims[0][0];
    vx_uint32    rois_height       = rois->tensorBuffer->memory.dims[0][1];
    vx_image     imgInput          = NULL;
    vx_image     imgRois           = NULL;
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_enum      roisFormat        = rois->tensorBuffer->dataFormat;

    status = vxQueryTensor(output, VX_TENSOR_DIMS, output_size, sizeof(output_size));
    output_channel = output_size[1];
    output_rois    = output_size[2];
    borderMode->mode = VX_BORDER_UNDEFINED;
    if (inputFormat == VX_TYPE_INT8)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, width * height, depth, VX_DF_IMAGE_U8);
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        imgInput = vxoTensor_CreateImageFromTensor(input, width * height, depth, VX_DF_IMAGE_U16);
    }

    if (roisFormat == VX_TYPE_INT8)
    {
        imgRois = vxoTensor_CreateImageFromTensor(rois, rois_width, rois_height, VX_DF_IMAGE_U8);
    }
    else if (roisFormat == VX_TYPE_FLOAT16)
    {
        imgRois = vxoTensor_CreateImageFromTensor(rois, rois_width, rois_height, VX_DF_IMAGE_S16);
    }

    parameters[0] = (vx_reference)imgInput;
    parameters[1] = (vx_reference)imgRois;
    parameters[2] = (vx_reference)output;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[4] = {NULL, NULL, NULL, NULL};

        char programSources_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         _viv_uniform VXC_512Bits uniGetMaxPoolPos; \n\
         _viv_uniform VXC_512Bits uniRePackData0; \n\
         _viv_uniform VXC_512Bits uniRePackData1; \n\
         _viv_uniform VXC_512Bits uniMaskData; \n\
         \n\
         __kernel void horzMaxPool_20x16_fp16tofp16(\n\
         __read_only image2d_t    input, \n\
         __read_only image2d_t    rois, \n\
         __write_only image2d_array_t  output) \n\
         { \n\
             int rois_idx    = get_global_id(0); \n\
             int channel        = get_global_id(1); \n\
             \n\
             int2 coord_rect = (int2)(16, rois_idx); \n\
             \n\
             vxc_ushort8 mask0to5; \n\
             vxc_ushort4    pos01, pos23, pos45; \n\
             vxc_ushort8 vect0, vect1, vect2; \n\
             vxc_int4 horz_len0, horz_len1; \n\
             \n\
             VXC_ReadImage(pos01, rois, coord_rect, VXC_5BITOFFSET_XY(-4, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
             VXC_ReadImage(mask0to5, rois, coord_rect, VXC_5BITOFFSET_XY(-16, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             VXC_ReadImage(pos23, rois, coord_rect, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
             VXC_ReadImage(pos45, rois, coord_rect, VXC_5BITOFFSET_XY(4, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
             int loopCnt = 0; \n\
             int channelNum = channel << 2; \n\
             \n\
             vxc_int4 coord_in; \n\
             VXC_DP4x4(coord_in, pos01, channelNum, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniGetMaxPoolPos); \n\
             VXC_ReadImage(vect0, input, coord_in.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
             VXC_ReadImage(vect0, input, coord_in.zw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
             vxc_int4 coord_out = (vxc_int4)(0, channel, rois_idx, 0); \n\
             coord_rect.x = 24; \n\
             do \n\
             { \n\
                vxc_ushort8 dst; \n\
                vxc_ushort8 mask; \n\
                vxc_ushort8 config; \n\
                 \n\
                VXC_DP4x4(coord_in, pos23, channelNum, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniGetMaxPoolPos); \n\
                VXC_ReadImage(vect1, input, coord_in.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(vect1, input, coord_in.zw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(pos01, rois, coord_rect, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(pos23, rois, coord_rect, VXC_5BITOFFSET_XY(4, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                 \n\
                VXC_DP4x4(coord_in, pos45, channelNum, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniGetMaxPoolPos); \n\
                VXC_ReadImage(vect2, input, coord_in.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(vect2, input, coord_in.zw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(pos45, rois, coord_rect, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                 \n\
                // 0 1 \n\
                vxc_uint4 config0 = (vxc_uint4)(0x03020100, 0x13121110, 0x01010101, 0x01010101); \n\
                VXC_BitExtract(mask, mask0to5, mask0to5, config0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(vect0, vect0, mask, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniMaskData); \n\
                VXC_HorzMax3_Integer(vect0, vect0, VXC_MODIFIER(0, 5, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(dst, vect0, vect0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniRePackData1); \n\
                VXC_DP2x8(vect0, vect0, vect0, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniRePackData0); \n\
                VXC_VertMax3_Integer(dst, vect0, dst, dst, VXC_MODIFIER(0, 1, 0, VXC_RM_TowardZero, 0)); \n\
                 \n\
                // 2 3 \n\
                vxc_uint4 config2 = (vxc_uint4)(0x23222120, 0x33323130, 0x01010101, 0x01010101); \n\
                VXC_BitExtract(mask, mask0to5, mask0to5, config2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(vect1, vect1, mask, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniMaskData); \n\
                VXC_HorzMax3_Integer(vect1, vect1, VXC_MODIFIER(0, 5, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(vect0, vect1, vect1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniRePackData1); \n\
                VXC_DP2x8(vect1, vect1, vect1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniRePackData0); \n\
                VXC_VertMax3_Integer(dst, vect1, vect0, vect0, VXC_MODIFIER(2, 3, 0, VXC_RM_TowardZero, 0)); \n\
                 \n\
                VXC_DP4x4(coord_in, pos01, channelNum, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0), uniGetMaxPoolPos); \n\
                VXC_ReadImage(vect0, input, coord_in.xy, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage(vect0, input, coord_in.zw, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
                 \n\
                // 4 5 \n\
                vxc_uint4 config4 = (vxc_uint4)(0x43424140, 0x53525150, 0x01010101, 0x01010101); \n\
                VXC_BitExtract(mask, mask0to5, mask0to5, config4, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(vect2, vect2, mask, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniMaskData); \n\
                VXC_HorzMax3_Integer(vect2, vect2, VXC_MODIFIER(0, 5, 0,VXC_RM_TowardZero, 0)); \n\
                VXC_DP2x8(vect1, vect2, vect2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniRePackData1); \n\
                VXC_DP2x8(vect2, vect2, vect2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0), uniRePackData0); \n\
                VXC_VertMax3_Integer(dst, vect2, vect1, vect1, VXC_MODIFIER(4, 5, 0, VXC_RM_TowardZero, 0)); \n\
                 \n\
                VXC_WriteImage2DArray(output, coord_out, dst, VXC_MODIFIER(0, 5, 0, VXC_RM_TowardZero, 0)); \n\
                 \n\
                coord_out.x += 6; \n\
                coord_rect.x += 12; \n\
                loopCnt ++; \n\
            } while (loopCnt < 6); \n\
         }\n"
        };

        if(inputFormat == VX_TYPE_FLOAT16)
        {
            if (context->evisNoInst.isVX2)
            {
                programSources[0] = VXC_VertMax3_Evis2;
                programSources[1] = VXC_HorzMax3_Evis2;
                programSources[2] = programSources_0;

                programLength[0]  = strlen(VXC_VertMax3_Evis2);
                programLength[1]  = strlen(VXC_HorzMax3_Evis2);
                programLength[2]  = strlen(programSources_0);
            }
            else
            {
                programSources[0] = VXC_VertMax3_Evis1;
                programSources[1] = VXC_HorzMax3_Evis1;
                programSources[2] = programSources_0;

                programLength[0]  = strlen(VXC_VertMax3_Evis1);
                programLength[1]  = strlen(VXC_HorzMax3_Evis1);
                programLength[2]  = strlen(programSources_0);
            }
        }
        program = vxCreateProgramWithSource(context, 3, (const vx_char**)programSources, programLength);


        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "horzMaxPool", program, 3, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if(inputFormat == VX_TYPE_FLOAT16)
    {
        vx_uint32 uniGetMaxPoolPos[16] = {
            0x0f030f03, // TCfg
            0x04000400, // ASelt
            0x00010000, 0x00030002, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00006400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        vx_uint32 uniMaskData[16] = {
            0x11111111, // TCfg
            0x00000000, // ASelt
            0x03020100, 0x07060504, // ABin
            0x11111111, // BSelt
            0x03020100, 0x07060504, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniRePackData0[16] = {
            0x00000033, // TCfg
            0x00000000, // ASelt
            0x00000400, 0x00000000, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00006400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniRePackData1[16] = {
            0x00000033, // TCfg
            0x00000000, // ASelt
            0x00000501, 0x00000000, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00006400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_20x16_fp16tofp16", borderMode);
        if (!shaderExecutable) goto error;

        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniGetMaxPoolPos", 1, uniGetMaxPoolPos);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniRePackData0", 1, uniRePackData0);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniRePackData1", 1, uniRePackData1);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniMaskData", 1, uniMaskData);
        if (status != VX_SUCCESS) goto error;

        execution_parameters.globalWorkScale[0]  = 1;
        execution_parameters.globalWorkScale[1]  = 1;
        execution_parameters.localWorkSize[0]    = 2;
        execution_parameters.localWorkSize[1]    = 4;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((output_rois + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((output_channel  + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(imgInput) vxReleaseImage(&imgInput);
    if(imgRois)vxReleaseImage(&imgRois);


    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if(imgInput) vxReleaseImage(&imgInput);
    if(imgRois)vxReleaseImage(&imgRois);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}

/********DeConvolution****************************************************/
vxnne_shader_executable vxnneDeConvolutionShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor  inputs,
    vx_tensor  weights,
    vx_tensor  bias,
    vx_scalar  padding_x,
    vx_scalar  padding_y,
    vx_scalar  overflow_policy,
    vx_scalar  rounding_policy,
    vx_scalar  a_x,
    vx_scalar  a_y,
    vx_scalar  group,
    vx_tensor  outputs
    )
{
    vx_size    programLength[2] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable = VX_NULL;
    vxnne_kernel_shaders        kernel;

    vx_kernel_execution_parameters_t execution_parameters = {2, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};

    vx_reference parameters[3]     = {(vx_reference)inputs, (vx_reference)weights, (vx_reference)outputs};

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);

    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[2] = {NULL, NULL};

        char programSources_0[] =
        {"\n\
             #include \"cl_viv_vx_ext.h\"\n\
            _viv_uniform VXC_512Bits Uni2x8_Conv12_r;\n\
            _viv_uniform VXC_512Bits Uni2x8_Conv4_r;\n\
            _viv_uniform VXC_512Bits Uni2x8_Conv8_r;\n\
            _viv_uniform VXC_512Bits Uni2x8_Conv0_r;\n\
            \n\
            _viv_uniform VXC_512Bits  Uni2x8_Add;\n\
            __kernel void DeConvVXC \n\
                (\n\
                image2d_array_t input, \n\
                image2d_array_t weight, \n\
                image2d_array_t output \n\
                ) \n\
            { \n\
               int z = get_global_id(0)+get_global_id(1)*256;// channel + batch*depth \n\
               int4 coord_in  = (int4)(0,0,z,0); \n\
               int4 coord_out = (int4)(0,0,z,0); \n\
               int4 coord_wei = (int4)(0,0,get_global_id(0),0); \n\
               vxc_short8 w0,w1; \n\
               VXC_ReadImage2DArray(w0, weight, coord_wei, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
               VXC_ReadImage2DArray(w1, weight, coord_wei, VXC_5BITOFFSET_XY(0,1), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
               VXC_ReadImage2DArray(w0, weight, coord_wei, VXC_5BITOFFSET_XY(0,2), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
               VXC_ReadImage2DArray(w1, weight, coord_wei, VXC_5BITOFFSET_XY(0,3), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
               vxc_half8 w0_h,w1_h; \n\
               _viv_asm(COPY,w0_h, w0, 16);//0,1,2,3,8,9,10,11 \n\
               _viv_asm(COPY,w1_h, w1, 16);//4,5,6,7,12,13,14,15 \n\
                \n\
                   vxc_short8 line0_0={0,0,0,0,0,0,0,0},line0_1={0,0,0,0,0,0,0,0},line0_2={0,0,0,0,0,0,0,0}; \n\
                vxc_short8 line1_0={0,0,0,0,0,0,0,0},line1_1={0,0,0,0,0,0,0,0},line1_2={0,0,0,0,0,0,0,0}; \n\
                 \n\
                vxc_half8 line0_0h,line0_1h,line0_2h; \n\
                vxc_half8 line1_0h,line1_1h,line1_2h; \n\
                 \n\
                _viv_asm(COPY, line0_0h,line0_0, 16); \n\
                _viv_asm(COPY, line0_1h,line0_1, 16); \n\
                _viv_asm(COPY, line0_2h,line0_2, 16); \n\
                 \n\
                vxc_short8 r; \n\
                vxc_half8 r_h,r0_h,r1_h; \n\
                 \n\
                VXC_ReadImage2DArray(line1_0, input, coord_in, VXC_5BITOFFSET_XY(0,0), VXC_MODIFIER(1,7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(line1_1, input, coord_in, VXC_5BITOFFSET_XY(3,0), VXC_MODIFIER(0,7, 0, VXC_RM_TowardZero, 0)); \n\
                VXC_ReadImage2DArray(line1_2, input, coord_in, VXC_5BITOFFSET_XY(7,0), VXC_MODIFIER(0,7, 0, VXC_RM_TowardZero, 0)); \n\
                 _viv_asm(COPY, line1_0h,line1_0, 16); \n\
                 _viv_asm(COPY, line1_1h,line1_1, 16); \n\
                 _viv_asm(COPY, line1_2h,line1_2, 16); \n\
               for(int i = 0; i < 8;i++) \n\
               { \n\
            //line 0 \n\
             \n\
                   VXC_DP2x8(r0_h, line0_0h, w0_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv0_r); \n\
                   VXC_DP2x8(r1_h, line1_0h, w0_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv8_r); \n\
                   VXC_DP2x8(r_h, r0_h, r1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Add); \n\
                   if(i==0) \n\
                      _viv_asm(COPY, r,r1_h, 16); \n\
                   else  \n\
                     _viv_asm(COPY, r,r_h, 16); \n\
                   coord_out.x = 0; \n\
                   VXC_WriteImage2DArray(output, coord_out, r, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    \n\
                  VXC_DP2x8(r0_h, line0_1h, w0_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv0_r); \n\
                  VXC_DP2x8(r1_h, line1_1h, w0_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv8_r); \n\
                  VXC_DP2x8(r_h, r0_h, r1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Add); \n\
                       if(i==0) \n\
                      _viv_asm(COPY, r,r1_h, 16); \n\
                   else  \n\
                     _viv_asm(COPY, r,r_h, 16); \n\
                  coord_out.x = 8; \n\
                  VXC_WriteImage2DArray(output, coord_out, r, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                   \n\
                  VXC_DP2x8(r0_h, line0_2h, w0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv0_r); \n\
                  VXC_DP2x8(r1_h, line1_2h, w0_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv8_r); \n\
                  VXC_DP2x8(r_h, r0_h, r1_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), Uni2x8_Add); \n\
                       if(i==0) \n\
                      _viv_asm(COPY, r,r1_h, 16); \n\
                   else  \n\
                     _viv_asm(COPY, r,r_h, 16); \n\
                  coord_out.x = 16; \n\
                  VXC_WriteImage2DArray(output, coord_out, r, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                   \n\
                    \n\
            //line 1 \n\
                  line0_0h = line1_0h; \n\
                  line0_1h = line1_1h; \n\
                  line0_2h = line1_2h; \n\
                  coord_in.y++; \n\
                  coord_out.y++; \n\
                   VXC_ReadImage2DArray(line1_0, input, coord_in, VXC_5BITOFFSET_XY(-1,0), VXC_MODIFIER(0,7, 0, VXC_RM_TowardZero, 0)); \n\
                   VXC_ReadImage2DArray(line1_1, input, coord_in, VXC_5BITOFFSET_XY(3,0), VXC_MODIFIER(0,7, 0, VXC_RM_TowardZero, 0)); \n\
                   VXC_ReadImage2DArray(line1_2, input, coord_in, VXC_5BITOFFSET_XY(7,0), VXC_MODIFIER(0,7, 0, VXC_RM_TowardZero, 0)); \n\
                    \n\
                   _viv_asm(COPY, line1_0h,line1_0, 16); \n\
                   _viv_asm(COPY, line1_1h,line1_1, 16); \n\
                   _viv_asm(COPY, line1_2h,line1_2, 16); \n\
                    \n\
                   VXC_DP2x8(r0_h, line0_0h, w1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv4_r); \n\
                   VXC_DP2x8(r1_h, line1_0h, w1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv12_r); \n\
                   VXC_DP2x8(r_h, r0_h, r1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Add); \n\
                   _viv_asm(COPY, r,r_h, 16); \n\
                   coord_out.x = 0; \n\
                   VXC_WriteImage2DArray(output, coord_out, r, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                    \n\
                  VXC_DP2x8(r0_h, line0_1h, w1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv4_r); \n\
                  VXC_DP2x8(r1_h, line1_1h, w1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv12_r); \n\
                  VXC_DP2x8(r_h, r0_h, r1_h, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 1), Uni2x8_Add); \n\
                  _viv_asm(COPY, r,r_h, 16); \n\
                  coord_out.x = 8; \n\
                  VXC_WriteImage2DArray(output, coord_out, r, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                   \n\
                  VXC_DP2x8(r0_h, line0_2h, w1_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv4_r); \n\
                  VXC_DP2x8(r1_h, line1_2h, w1_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), Uni2x8_Conv12_r); \n\
                  VXC_DP2x8(r_h, r0_h, r1_h, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 1), Uni2x8_Add); \n\
                  _viv_asm(COPY, r,r_h, 16); \n\
                  coord_out.x = 16; \n\
                  VXC_WriteImage2DArray(output, coord_out, r, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
                  coord_out.y++; \n\
                   \n\
               } \n\
              \n\
            }\n"
        };


        programSources[0] = programSources_0;
        programLength[0] = strlen(programSources_0);

        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, programLength);

        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "DeConv", program, 3, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }


    {
           vx_uint32 Uni2x8_Conv12_r[16] = {
            0x55555555, // TCfg
            0x00000000, // ASelt
            0x32212110, 0x54434332, // ABin
            0x55555555, // BSelt
            0x75647564, 0x75647564, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 Uni2x8_Conv4_r[16] = {
            0x55555555, // TCfg
            0x00000000, // ASelt
            0x32212110, 0x54434332, // ABin
            0x55555555, // BSelt
            0x31203120, 0x31203120, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 Uni2x8_Conv8_r[16] = {
            0x55555555, // TCfg
            0x00000000, // ASelt
            0x32212110, 0x54434332, // ABin
            0x55555555, // BSelt
            0x75647564, 0x75647564, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 Uni2x8_Conv0_r[16] = {
            0x55555555, // TCfg
            0x00000000, // ASelt
            0x32212110, 0x54434332, // ABin
            0x55555555, // BSelt
            0x31203120, 0x31203120, // BBin
            0x00000400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };

        vx_uint32 Uni2x8_Add[16] = {
            0x55555555, // TCfg
            0x44444444, // ASelt
            0x33221100, 0x77665544, // ABin
            0xaaaaaaaa, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00, 0x3c003c00 // Constant
        };
        int input_size[6];
        int depth ;
        int batch ;
        status = vxQueryTensor((vx_tensor)inputs, VX_TENSOR_DIMS, input_size, sizeof(input_size));
        depth = input_size[2];
        batch = input_size[3];
        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "VXC", borderMode);
        if (!shaderExecutable) goto error;

        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni2x8_Add", 1, Uni2x8_Add);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni2x8_Conv12_r", 1, Uni2x8_Conv12_r);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni2x8_Conv4_r", 1, Uni2x8_Conv4_r);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni2x8_Conv8_r", 1, Uni2x8_Conv8_r);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "Uni2x8_Conv0_r", 1, Uni2x8_Conv0_r);

        if (status != VX_SUCCESS) goto error;

        execution_parameters.globalWorkScale[0]  = 1;
        execution_parameters.globalWorkScale[1]  = 1;
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((depth + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((batch  + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 3);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;




    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}


/********vxcReshuffle****************************************************/
vxnne_shader_executable vxnneReshuffleShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               stride_x,
    vx_uint32               stride_y,
    vx_enum                 padMode,
    vx_uint32               padConstValue,
    vx_uint32               padXLeft,
    vx_uint32               padXRight,
    vx_uint32               padYTop,
    vx_uint32               padYBottom,
    vx_tensor               output
    )
{
    vx_size    programLength[2] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable   = VX_NULL;
    vxnne_kernel_shaders    kernel             = VX_NULL;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[4]     = {(vx_reference)input, NULL, NULL, (vx_reference)output};
    vx_uint32    width             = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    height            = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth             = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;
    vx_scalar    padXLeft_s        = VX_NULL;
    vx_scalar    padYTOP_s         = VX_NULL;

    padXLeft_s   = vxCreateScalar(context, VX_TYPE_INT32, &padXLeft);
    padYTOP_s    = vxCreateScalar(context, VX_TYPE_INT32, &padYTop);

    borderMode->mode = VX_BORDER_REPLICATE;
    if(padMode == VX_PAD_CONSTANT)
        borderMode->mode = VX_BORDER_CONSTANT;

    parameters[1] = (vx_reference)padXLeft_s;
    parameters[2] = (vx_reference)padYTOP_s;
    if (inputFormat == VX_TYPE_INT8)
    {
        borderMode->constant_value.U8 = (vx_uint8)padConstValue;
    }
    else if (inputFormat == VX_TYPE_FLOAT16)
    {
        borderMode->constant_value.U16 = (vx_uint16)padConstValue;
    }

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);
    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[2] = {NULL, NULL};

        char programSources_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         _viv_uniform VXC_512Bits UniReshuffle_dual_0_2x8; \n\
         _viv_uniform VXC_512Bits UniReshuffle_dual_1_2x8; \n\
         _viv_uniform VXC_512Bits UniReshuffle_quad_0_4x4; \n\
         _viv_uniform VXC_512Bits UniReshuffle_quad_1_4x4; \n\
         _viv_uniform VXC_512Bits UniReshuffle_quad_2_4x4; \n\
         _viv_uniform VXC_512Bits UniReshuffle_quad_3_4x4; \n\
         __kernel void vxcReshuffle_Fp16Stride4 \n\
            (\n\
            __read_only image2d_array_t    input, \n\
            int    padXLeft, \n\
            int    padYTop, \n\
            __write_only image2d_array_t   output \n\
            ) \n\
        { \n\
            int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            int4 coord_out; \n\
            coord_out.xy = coord_in.xy >> 2; \n\
            coord_out.z  = coord_in.z << 4; \n\
             \n\
            coord_in.xy -= (int2)(padXLeft, padYTop); \n\
            vxc_ushort8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec; \n\
            VXC_ReadImage2DArray(vec0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec2, input, coord_in, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec1, input, coord_in, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec3, input, coord_in, VXC_5BITOFFSET_XY(8, 1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec4, input, coord_in, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec6, input, coord_in, VXC_5BITOFFSET_XY(8, 2), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec5, input, coord_in, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec7, input, coord_in, VXC_5BITOFFSET_XY(8, 3), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP4x4(vec, vec0, vec2, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_0_4x4); \n\
            VXC_DP4x4(vec, vec0, vec2, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_1_4x4); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_DP4x4(vec, vec0, vec2, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_2_4x4); \n\
            VXC_DP4x4(vec, vec0, vec2, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_3_4x4); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
             \n\
            VXC_DP4x4(vec, vec1, vec3, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_0_4x4); \n\
            VXC_DP4x4(vec, vec1, vec3, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_1_4x4); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_DP4x4(vec, vec1, vec3, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_2_4x4); \n\
            VXC_DP4x4(vec, vec1, vec3, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_3_4x4); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
             \n\
            VXC_DP4x4(vec, vec4, vec6, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_0_4x4); \n\
            VXC_DP4x4(vec, vec4, vec6, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_1_4x4); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_DP4x4(vec, vec4, vec6, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_2_4x4); \n\
            VXC_DP4x4(vec, vec4, vec6, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_3_4x4); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
             \n\
            VXC_DP4x4(vec, vec5, vec7, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_0_4x4); \n\
            VXC_DP4x4(vec, vec5, vec7, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_1_4x4); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_DP4x4(vec, vec5, vec7, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_2_4x4); \n\
            VXC_DP4x4(vec, vec5, vec7, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_quad_3_4x4); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
        } \n\
        __kernel void vxcReshuffle_Fp16Stride2 \n\
            (\n\
            __read_only image2d_array_t    input, \n\
            int    padXLeft, \n\
            int    padYTop, \n\
            __write_only image2d_array_t   output \n\
            ) \n\
        { \n\
            int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            int4 coord_out; \n\
             \n\
            coord_out.xy = coord_in.xy >> 1; \n\
            coord_out.z  = coord_in.z << 2; \n\
             \n\
            coord_in.xy -= (int2)(padXLeft, padYTop); \n\
            vxc_ushort8 vec0, vec1, vec2, vec3, vec4, vec5, vec6, vec7, vec; \n\
            VXC_ReadImage2DArray(vec0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec2, input, coord_in, VXC_5BITOFFSET_XY(8, 0), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec4, input, coord_in, VXC_5BITOFFSET_XY(0, 2), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec6, input, coord_in, VXC_5BITOFFSET_XY(8, 2), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec1, input, coord_in, VXC_5BITOFFSET_XY(0, 1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec3, input, coord_in, VXC_5BITOFFSET_XY(8, 1), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec5, input, coord_in, VXC_5BITOFFSET_XY(0, 3), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            VXC_ReadImage2DArray(vec7, input, coord_in, VXC_5BITOFFSET_XY(8, 3), VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP2x8(vec, vec0, vec2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_dual_0_2x8); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_DP2x8(vec, vec0, vec2, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_dual_1_2x8); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            coord_out.yz += (int2)(1, -1); \n\
            VXC_DP2x8(vec, vec4, vec6, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_dual_0_2x8); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_DP2x8(vec, vec4, vec6, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_dual_1_2x8); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            coord_out.yz += (int2)(-1, 1); \n\
            VXC_DP2x8(vec, vec1, vec3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_dual_0_2x8); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_DP2x8(vec, vec1, vec3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_dual_1_2x8); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            coord_out.yz += (int2)(1, -1); \n\
            VXC_DP2x8(vec, vec5, vec7, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_dual_0_2x8); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.z ++; \n\
            VXC_DP2x8(vec, vec5, vec7, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), UniReshuffle_dual_1_2x8); \n\
            VXC_WriteImage2DArray(output, coord_out, vec, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardZero, 0)); \n\
        }\n"
        };

        if(inputFormat == VX_TYPE_FLOAT16)
        {
            programSources[0] = programSources_0;
            programLength[0] = strlen(programSources_0);
        }
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, programLength);

        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxcReshuffle", program, 4, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if(inputFormat == VX_TYPE_FLOAT16 && stride_x == 2 && stride_y == 2)
    {
        vx_uint32 UniReshuffle_dual_0_2x8[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x06040200, 0x06040200, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
        };
        vx_uint32 UniReshuffle_dual_1_2x8[16] = {
            0x11111111, // TCfg
            0x11110000, // ASelt
            0x07050301, 0x07050301, // ABin
            0x22222222, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00, 0x00003c00 // Constant
        };


        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16Stride2", borderMode);
        if (!shaderExecutable) goto error;

        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniReshuffle_dual_0_2x8", 1, UniReshuffle_dual_0_2x8);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniReshuffle_dual_1_2x8", 1, UniReshuffle_dual_1_2x8);
        if (status != VX_SUCCESS) goto error;

        execution_parameters.globalWorkScale[0]  = 16;
        execution_parameters.globalWorkScale[1]  = 4;
        execution_parameters.globalWorkScale[2]  = 1;
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.localWorkSize[2]    = 1;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((width + padXLeft + padXRight + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((height + padYTop + padYBottom + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
        execution_parameters.globalWorkSize[2]   = gcmALIGN((depth + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]);
    }
    else if(inputFormat == VX_TYPE_FLOAT16 && stride_x == 4 && stride_y == 4)
    {
        vx_uint32 UniReshuffle_quad_0_4x4[16] = {
            0x01010101, // TCfg
            0x01010000, // ASelt
            0x00040000, 0x00040000, // ABin
            0x02020202, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
        };
        vx_uint32 UniReshuffle_quad_1_4x4[16] = {
            0x01010101, // TCfg
            0x01010000, // ASelt
            0x00050001, 0x00050001, // ABin
            0x02020202, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
        };
        vx_uint32 UniReshuffle_quad_2_4x4[16] = {
            0x01010101, // TCfg
            0x01010000, // ASelt
            0x00060002, 0x00060002, // ABin
            0x02020202, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
        };
        vx_uint32 UniReshuffle_quad_3_4x4[16] = {
            0x01010101, // TCfg
            0x01010000, // ASelt
            0x00070003, 0x00070003, // ABin
            0x02020202, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00000100, // AccumType, ConstantType, and PostShift
            0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000, 0x00003c00, 0x00000000 // Constant
        };


        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_Fp16Stride4", borderMode);
        if (!shaderExecutable) goto error;

        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "UniReshuffle_quad_0_4x4", 1, UniReshuffle_quad_0_4x4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniReshuffle_quad_1_4x4", 1, UniReshuffle_quad_1_4x4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniReshuffle_quad_2_4x4", 1, UniReshuffle_quad_2_4x4);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "UniReshuffle_quad_3_4x4", 1, UniReshuffle_quad_3_4x4);
        if (status != VX_SUCCESS) goto error;

        execution_parameters.globalWorkScale[0]  = 16;
        execution_parameters.globalWorkScale[1]  = 4;
        execution_parameters.globalWorkScale[2]  = 1;
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.localWorkSize[2]    = 1;
        execution_parameters.globalWorkSize[0]   = gcmALIGN((width + padXLeft + padXRight + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
        execution_parameters.globalWorkSize[1]   = gcmALIGN((height + padYTop + padYBottom + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
        execution_parameters.globalWorkSize[2]   = gcmALIGN((depth + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]);
    }


    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 4);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    if(padXLeft_s) vxReleaseScalar(&padXLeft_s);
    if(padYTOP_s) vxReleaseScalar(&padYTOP_s);

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if (padXLeft_s) vxReleaseScalar(&padXLeft_s);
    if (padYTOP_s) vxReleaseScalar(&padYTOP_s);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}

/************************vxcTensorTranspose************************************/
vxnne_shader_executable vxnneTensorTransposeShaderExecutable(
    vx_context              context,
    vx_enum                 kernelEnum,
    vx_border_mode_t        *borderMode,
    vx_tensor               input,
    vx_uint32               *perm,
    vx_uint32               pnum,
    vx_tensor               output
    )
{
    vx_size    programLength[2] = {0};
    vx_program program = VX_NULL;
    vx_status  status = VX_FAILURE;
    vxnne_shader_executable shaderExecutable   = VX_NULL;
    vxnne_kernel_shaders    kernel             = VX_NULL;
    vx_kernel_execution_parameters_t execution_parameters = {3, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}};
    vx_reference parameters[2]     = {(vx_reference)input, (vx_reference)output};
    vx_uint32    width             = TENSOR_VIEW_SIZE_INDEX(input, 0);
    vx_uint32    height            = TENSOR_VIEW_SIZE_INDEX(input, 1);
    vx_uint32    depth             = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_enum      inputFormat       = input->tensorBuffer->dataFormat;


    borderMode->mode = VX_BORDER_REPLICATE;

    kernel = vxnneGetKernelShadersByEnum(context, kernelEnum);
    if (!kernel)
    {
        /* register an shader kernel */
        char *programSources[2] = {NULL, NULL};

        char programSources_0[] =
        {"\n\
         #include \"cl_viv_vx_ext.h\" \n\
         _viv_uniform VXC_512Bits uniPackDataABCD_01; \n\
         _viv_uniform VXC_512Bits uniPackDataABCD_23; \n\
        __kernel void vxTensorTranspose_WHC2CWH \n\
            (\n\
            image2d_array_t input, \n\
            image2d_array_t output \n\
            ) \n\
        { \n\
            int4 coord_in = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0); \n\
            int4 coord_out = (int4)(get_global_id(2), get_global_id(0), get_global_id(1), 0); \n\
             \n\
            vxc_ushort8 vec0, vec1, vec2, vec3, vec01, vec23; \n\
             \n\
            VXC_ReadImage2DArray(vec0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_in.z ++; \n\
            VXC_ReadImage2DArray(vec0, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_in.z ++; \n\
            VXC_ReadImage2DArray(vec1, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_in.z ++; \n\
            VXC_ReadImage2DArray(vec1, input, coord_in, VXC_5BITOFFSET_XY(0, 0), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            coord_in.z -= 3; \n\
            VXC_ReadImage2DArray(vec2, input, coord_in, VXC_5BITOFFSET_XY(4, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_in.z ++; \n\
            VXC_ReadImage2DArray(vec2, input, coord_in, VXC_5BITOFFSET_XY(4, 0), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_in.z ++; \n\
            VXC_ReadImage2DArray(vec3, input, coord_in, VXC_5BITOFFSET_XY(4, 0), VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_in.z ++; \n\
            VXC_ReadImage2DArray(vec3, input, coord_in, VXC_5BITOFFSET_XY(4, 0), VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
             \n\
            VXC_DP2x8(vec01, vec0, vec1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), uniPackDataABCD_01); \n\
            VXC_DP2x8(vec23, vec0, vec1, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), uniPackDataABCD_23); \n\
             \n\
            VXC_WriteImage2DArray(output, coord_out, vec01, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.y ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec01, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.y ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec23, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.y ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec23, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.y ++; \n\
             \n\
            VXC_DP2x8(vec01, vec2, vec3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), uniPackDataABCD_01); \n\
            VXC_DP2x8(vec23, vec2, vec3, VXC_MODIFIER(0, 7, 0, VXC_RM_TowardInf, 0), uniPackDataABCD_23); \n\
             \n\
            VXC_WriteImage2DArray(output, coord_out, vec01, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.y ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec01, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.y ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec23, VXC_MODIFIER(0, 3, 0, VXC_RM_TowardZero, 0)); \n\
            coord_out.y ++; \n\
            VXC_WriteImage2DArray(output, coord_out, vec23, VXC_MODIFIER(4, 7, 0, VXC_RM_TowardZero, 0)); \n\
        }\n"
        };

        if(inputFormat == VX_TYPE_FLOAT16)
        {
            programSources[0] = programSources_0;
            programLength[0] = strlen(programSources_0);
        }
        program = vxCreateProgramWithSource(context, 1, (const vx_char**)programSources, programLength);

        status = vxGetStatus((vx_reference)program);
        if (status != VX_SUCCESS) goto error;

        status = vxBuildProgram(program, "-cl-viv-vx-extension");
        if (status != VX_SUCCESS) goto error;

        kernel = vxnneAddKernelShadersInProgram(context, "vxTensorTranspose", program, 2, kernelEnum);
        if (!kernel) goto error;

        vxReleaseProgram(&program);
    }

    if(inputFormat == VX_TYPE_FLOAT16 && perm[0] == 2 && perm[1] == 0 && perm[2] == 1 && pnum == 3)
    {
        vx_uint32 uniPackDataABCD_01[16] = {
            0x33333333, // TCfg
            0x11001100, // ASelt
            0x04000400, 0x05010501, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00006400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };
        vx_uint32 uniPackDataABCD_23[16] = {
            0x33333333, // TCfg
            0x11001100, // ASelt
            0x06020602, 0x07030703, // ABin
            0x00000000, // BSelt
            0x00000000, 0x00000000, // BBin
            0x00006400, // AccumType, ConstantType, and PostShift
            0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000 // Constant
        };


        shaderExecutable = vxnneKernelShaders_CreateShaderExecutable(kernel, "_WHC2CWH", borderMode);
        if (!shaderExecutable) goto error;

        status  = vxnneShaderExecutable_SetUniform(shaderExecutable, "uniPackDataABCD_01", 1, uniPackDataABCD_01);
        status |= vxnneShaderExecutable_SetUniform(shaderExecutable, "uniPackDataABCD_23", 1, uniPackDataABCD_23);
        if (status != VX_SUCCESS) goto error;
    }

    execution_parameters.globalWorkOffset[0] = 0;
    execution_parameters.globalWorkOffset[1] = 0;
    execution_parameters.globalWorkOffset[2] = 0;
    execution_parameters.globalWorkScale[0]    = 8;
    execution_parameters.globalWorkScale[1]    = 1;
    execution_parameters.globalWorkScale[2]    = 4;

    if (width > execution_parameters.globalWorkScale[0] * 8)
    {
        execution_parameters.localWorkSize[0]    = 8;
        execution_parameters.localWorkSize[1]    = 1;
        execution_parameters.localWorkSize[2]    = 1;
    }
    else if (width > execution_parameters.globalWorkScale[0] * 4)
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

    execution_parameters.globalWorkSize[0]    = gcmALIGN((width + execution_parameters.globalWorkScale[0] - 1) / execution_parameters.globalWorkScale[0], execution_parameters.localWorkSize[0]);
    execution_parameters.globalWorkSize[1]    = gcmALIGN((height + execution_parameters.globalWorkScale[1] - 1) / execution_parameters.globalWorkScale[1], execution_parameters.localWorkSize[1]);
    execution_parameters.globalWorkSize[2]    = gcmALIGN((depth + execution_parameters.globalWorkScale[2] - 1) / execution_parameters.globalWorkScale[2], execution_parameters.localWorkSize[2]);

    status = vxnneShaderExecutable_SetParameters(shaderExecutable, parameters, 2);
    if (status != VX_SUCCESS) goto error;

    status = vxnneShaderExecutable_SetExecutionParameters(shaderExecutable, &execution_parameters);
    if (status != VX_SUCCESS) goto error;

    return shaderExecutable;

error:
    if (program) vxReleaseProgram(&program);
    if (shaderExecutable) vxnneShaderExecutable_Destroy(shaderExecutable);

    return VX_NULL;
}

