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


#include <gc_vx_common.h>
#include <layers/gc_vx_layer_tensor_transpose.h>

vx_status vxnneExecuteSWTensorTranspose(struct _vxnne_operation_s *operation)
{
    vxnne_tensor_trans_operation transOperation = (vxnne_tensor_trans_operation)operation;

    vx_tensor input  = (vx_tensor)transOperation->input;
    vx_tensor output = (vx_tensor)transOperation->output;

    vx_uint32_ptr perm = (vx_uint32_ptr)transOperation->perm->memory.logicals[0];
    vx_uint32 pnum = transOperation->pnum->value->u32;

    vx_uint8_ptr inaddr, outaddr;
    vx_uint32 dims[VX_CONTEXT_TENSOR_MAX_DIMENSION], strides[VX_CONTEXT_TENSOR_MAX_DIMENSION], tstrides[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vxoTensor_GetTensorViewMemory(input, (gctPOINTER*)&inaddr, VX_NULL);
    vxoTensor_GetTensorViewMemory(output, (gctPOINTER*)&outaddr, VX_NULL);

    vxoTensor_GetTensorDimStride(input, &pnum, dims, strides);
    vxoTensor_GetTensorDimStride(output, &pnum, VX_NULL, tstrides);

    if (pnum == 1)
    {
        memcpy(outaddr, inaddr, dims[0] * strides[0]);
    }
    else
    {
        _TransposeTensor(inaddr, outaddr,TENSOR_DATA_SIZE(input), dims, strides, tstrides, perm, pnum-1);
    }

    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_NNTensorTrans(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_ValidateInput(vx_node node, vx_uint32 index)
{
    return VX_SUCCESS;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr)
{
    return VX_SUCCESS;
}
#if REGISTER_FRAME

VX_PRIVATE_API vx_status vxoNNTensorTrans_SW_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_trans_layer  tensor_trans_layer = (vxnne_tensor_trans_layer)ops_layer;

    vx_tensor input = (vx_tensor)parameters[0];
    vx_array  perm = (vx_array)parameters[1];
    vx_scalar pnum = (vx_scalar)parameters[2];
    vx_tensor output = (vx_tensor)parameters[3];
    vx_uint32 batchCount = 1;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&tensor_trans_layer->tensor_trans_sw_operation.base,
        &tensor_trans_layer->base,
        VXNNE_OPERATION_TARGET_SW,
        VXNNE_OPERATOR_TENSOR_TRANS,
        vxnneExecuteSWTensorTranspose,
        VX_NULL,
        batchCount,
        0));

    vxmONERROR(vxnneLayer_SetOperation(
        &tensor_trans_layer->base,
        &tensor_trans_layer->tensor_trans_sw_operation.base,
        0));

    tensor_trans_layer->tensor_trans_sw_operation.input = input;
    tensor_trans_layer->tensor_trans_sw_operation.perm = perm;
    tensor_trans_layer->tensor_trans_sw_operation.pnum = pnum;
    tensor_trans_layer->tensor_trans_sw_operation.output = output;

    vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);
OnError:
    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorTrans_SH_EVIS_Support_Ext(vx_node node, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_tensor input = (vx_tensor)parameters[0];
    vx_array  perm = (vx_array)parameters[1];
    vx_scalar pnum = (vx_scalar)parameters[2];
    vx_tensor output = (vx_tensor)parameters[3];

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vx_uint32_ptr  pPerm = (vx_uint32_ptr)perm->memory.logicals[0];
    vx_uint32      num = pnum->value->u32;
    vx_uint32      depth = TENSOR_VIEW_SIZE_INDEX(input, 2);
    vx_uint32      batch = TENSOR_VIEW_SIZE_INDEX(input, 3);
    vx_bool        shExe_flag = vx_true_e;
    vx_bool        shExe_copy_flag = vx_true_e;
    vx_bool        enable_4Dtensor = vx_false_e;
    vx_bool        enable_batch_sh = vx_false_e;
    vx_bool        enable_4D_perm = vx_false_e;
    vx_bool        enable_dataFormat = vx_false_e;
    vx_enum        inputFormat = TENSOR_DATA_TYPE(input);
    vx_enum        outputFormat = TENSOR_DATA_TYPE(output);
    vx_uint32      i = 0;

    vxoLayer_VerificationHead(node, parameters, _num, reg_param);

    reg_param->flag = 0;

    if (evis)
    {
        enable_dataFormat = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
            (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16) ||
            (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8) ||
            (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));
    }
    else
    {
        enable_dataFormat = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
            (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32) ||
            (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));
    }

    enable_4Dtensor = (vx_bool)(enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 2 && pPerm[2] == 3 && pPerm[3] == 0 && num == 4 && batch == 1);
    enable_batch_sh = (vx_bool)(enable_dataFormat &&  pPerm[3] == 3 && num == 4 && TENSOR_DIM_NUM(input) == 4);

    if (evis == vx_false_e && enable_4Dtensor == vx_false_e && enable_4Dtensor == vx_false_e)
    {
        vx_bool support_format = (vx_bool)((inputFormat == VX_TYPE_INT32 && outputFormat == VX_TYPE_INT32) ||
                                            (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16) ||
                                            (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8));

        enable_4D_perm = (vx_bool)((enable_dataFormat || support_format) && num == 4 &&  pPerm[3] != 3 && TENSOR_DIM_NUM(input) == 4 && _IsSameQuantType(input, output));

        enable_4D_perm = enable_4D_perm && (depth * batch < IMG_MAX_WIDTH);
    }

    shExe_flag = (vx_bool)((enable_dataFormat && pPerm[0] == 2 && pPerm[1] == 0 && pPerm[2] == 1 && num == 3)
        || (enable_dataFormat && pPerm[0] == 2 && pPerm[1] == 1 && pPerm[2] == 0 && num == 3)
        || (enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 2 && pPerm[2] == 0 && num == 3)
        || (enable_dataFormat && pPerm[0] == 0 && pPerm[1] == 2 && pPerm[2] == 1 && num == 3)
        || (enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 0 && num <= 3 && num >= 2)
        || (enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 3 && pPerm[2] == 2 && pPerm[3] == 0 && num == 4)
        || enable_4Dtensor || enable_batch_sh || enable_4D_perm);

    for (i = 0; i < gcmMIN(TENSOR_DIM_NUM(input), num); i++)
    {
        shExe_copy_flag &= (pPerm[i] == i);
    }

    if (evis)
    {
        shExe_copy_flag &= (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
            || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)));
    }
    else
    {
        shExe_copy_flag &= (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
            || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT32)
            || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT32)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
            || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)));
    }

    support = support && (shExe_copy_flag || shExe_flag);

    if (support)
    {
        SETBIT(reg_param->flag, ((shExe_flag      == vx_true_e) ? 1 : 0), 0);
        SETBIT(reg_param->flag, ((shExe_copy_flag == vx_true_e) ? 1 : 0), 1);
        SETBIT(reg_param->flag, ((enable_4Dtensor == vx_true_e) ? 1 : 0), 2);
        SETBIT(reg_param->flag, ((enable_batch_sh == vx_true_e) ? 1 : 0), 3);
        SETBIT(reg_param->flag, ((enable_4D_perm  == vx_true_e) ? 1 : 0), 4);
    }

    vxoLayer_VerificationFoot(node, parameters, _num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_bool vxoNNTensorTrans_SH_EVIS_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    if (!support)return support;

    support = support && node->base.context->evisNoInst.supportEVIS;

    if (!support)return support;

    support = support && vxoNNTensorTrans_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_true_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorTrans_SH_Initialize_Ext(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 _num, vxnne_register_param reg_param, vx_bool evis)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_trans_layer  tensor_trans_layer = (vxnne_tensor_trans_layer)ops_layer;

    vx_tensor input = (vx_tensor)parameters[0];
    vx_array  perm = (vx_array)parameters[1];
    vx_scalar pnum = (vx_scalar)parameters[2];
    vx_tensor output = (vx_tensor)parameters[3];
    vx_uint32 batchCount = 1, batchCount2 = 1;

    vx_uint32_ptr  pPerm = (vx_uint32_ptr)perm->memory.logicals[0];
    vx_uint32      num = pnum->value->u32, num2 = 0;
    vx_uint32      batch = TENSOR_VIEW_SIZE_INDEX(input, 3);
    vx_bool        shExe_flag = GETBIT(reg_param->flag, 0);
    vx_bool        shExe_copy_flag = GETBIT(reg_param->flag, 1);
    vx_bool        enable_4Dtensor = GETBIT(reg_param->flag, 2);
    vx_bool        enable_batch_sh = GETBIT(reg_param->flag, 3);
    vx_bool        enable_4D_perm  = GETBIT(reg_param->flag, 4);

    vxoLayer_InitializeHead(ops_layer, parameters, _num, reg_param);

    if (shExe_copy_flag)
    {
        vxnne_shader_executable shaderExecutable = VX_NULL;
        vx_tensor src = NULL;
        vx_tensor dst = NULL;
        vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];
        vx_uint32 dims = 0;

        vxoElementOptimization_GetTensorShape(input, sizes, &dims);

        src = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, dims, VX_NULL);
        dst = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, dims, VX_NULL);

        tensor_trans_layer->base.temp_tensors[0] = src;
        tensor_trans_layer->base.temp_tensors[1] = dst;
        tensor_trans_layer->base.num_temp_tensors = 2;

        if (evis)
        {
            if (src && dst)
                shaderExecutable = vxnneTensorCopyShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_COPY, &ops_layer->node->kernelAttributes.borderMode, src, dst);
        }
        else
        {
            if (src && dst)
                shaderExecutable = vxnneGPUTensorCopyShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_COPY, &ops_layer->node->kernelAttributes.borderMode, src, dst);
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }
        vxmONERROR(vxnneShaderOperation_Initialize(&tensor_trans_layer->tensor_copy_sh_operation,
            &tensor_trans_layer->base,
            VXNNE_OPERATOR_CONVERT_FORMAT,
            1,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_copy_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_copy_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &tensor_trans_layer->base,
            &tensor_trans_layer->tensor_copy_sh_operation.base,
            0));
    }
    else if (shExe_flag)
    {
        vx_bool   is_per_channel_quant = (vx_bool)(TENSOR_QUANT_TYPE(output) == VX_QUANT_AFFINE_SCALE_PER_CHANNEL );
        vx_uint32 channelDim = TENSOR_TF_CHANNEL_DIMS(output);
        vx_uint32 *nChannelDim = NULL;
        vx_uint32 width = 0;
        vx_uint32 height = 0;
        vx_uint32 depth = 0;
        vx_int32  size[4] = { 0, 0, 0, 0 };
        vx_uint32 permArray[4] = { 1, 2, 0, 3 }, permArray2[4] = { 0, 2, 1, 3 };
        vx_uint32 dims = 3;
        vx_tensor src = NULL, src2 = NULL;
        vx_tensor dst = NULL, dst2 = NULL;
        vxnne_shader_executable shaderExecutable = VX_NULL;

        width = TENSOR_VIEW_SIZE_INDEX(input, 0);
        height = TENSOR_DIM_NUM(input) > 1 ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
        depth = TENSOR_DIM_NUM(input) > 2 ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
        batch = TENSOR_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

        batchCount = TENSOR_DIM_NUM(input) > 3 ? batch : 1;

        if (enable_batch_sh)
        {
            num = 3;
        }

        if (pPerm[0] == 2 && pPerm[1] == 0 && pPerm[2] == 1 && width * height < IMG_MAX_WIDTH && depth < IMG_MAX_WIDTH)
        {
            size[0] = width * height;
            size[1] = depth;
            size[2] = 1;
            size[3] = batch;
            dims = TENSOR_DIM_NUM(input);
            src = vxoTensor_ReshapeTensor(input, size, dims, VX_NULL);

            size[0] = depth;
            size[1] = width * height;
            size[2] = 1;
            size[3] = batch;
            dims = TENSOR_DIM_NUM(input);
            dst = vxoTensor_ReshapeTensor(output, size, dims, VX_NULL);

            num = 2;
        }
        else if (enable_4Dtensor)
        {
            size[0] = width;
            size[1] = height;
            size[2] = depth;
            dims = 3;
            src = vxoTensor_ReshapeTensor(input, size, dims, VX_NULL);

            size[0] = height;
            size[1] = depth;
            size[2] = width;
            dims = 3;
            dst = vxoTensor_ReshapeTensor(output, size, dims, VX_NULL);

            num = 3;
        }
        else if (pPerm[0] == 1 && pPerm[1] == 3 && pPerm[2] == 2 && pPerm[3] == 0 && width * height < IMG_MAX_WIDTH && depth < IMG_MAX_WIDTH)
        {
            size[0] = width;
            size[1] = height * depth * batch;
            size[2] = 1;
            dims = 3;
            src = vxoTensor_ReshapeTensor(input, size, dims, VX_NULL);

            size[0] = height * depth * batch;
            size[1] = width;
            size[2] = 1;
            dims = 3;

            {
                vx_tensor_create_params_t param = { dims, VX_NULL, TENSOR_DATA_TYPE(input), TENSOR_QUANT_TYPE(input), };
                param.sizes = (vx_uint32_ptr)size;

                if (TENSOR_QUANT_TYPE(input) == VX_QUANT_DYNAMIC_FIXED_POINT)
                    param.quant_data.dfp.fixed_point_pos = TENSOR_POS(input);
                else if (TENSOR_QUANT_TYPE(input) == VX_QUANT_AFFINE_SCALE)
                {
                    param.quant_data.affine.scale = TENSOR_TF_SCALE(input);
                    param.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input);
                }

                dst = vxoTensor_CreateTensor2(ops_layer->node->base.context, &param, sizeof(vx_tensor_create_params_t));
            }

            num = 3;
            permArray[0] = 1;
            permArray[1] = 0;
            permArray[2] = 2;

            size[0] = height;
            size[1] = depth;
            size[2] = batch;
            size[3] = width;
            dims = 4;
            src2 = vxoTensor_ReshapeTensor(dst, size, dims, VX_NULL);

            dst2 = output;
            batchCount2 = width;
            num2 = 3;
        }
        else if (pPerm[0] == 1 && pPerm[1] == 2 && pPerm[2] == 0 && width < IMG_MAX_WIDTH && height * depth < IMG_MAX_WIDTH
                 && (is_per_channel_quant && channelDim == 2))
        {
            size[0] = width;
            size[1] = height * depth;
            size[2] = 1;
            size[3] = batch;
            dims = TENSOR_DIM_NUM(input);
            src = vxoTensor_ReshapeTensor(input, size, dims, VX_NULL);

            size[0] = height * depth;
            size[1] = width;
            size[2] = 1;
            size[3] = batch;
            dims = TENSOR_DIM_NUM(input);
            if (is_per_channel_quant && channelDim == 2)
            {
                channelDim = 1;
                nChannelDim = &channelDim;
            }
            dst = vxoTensor_ReshapeTensor(output, size, dims, nChannelDim);

            num = 2;
        }
        else if (enable_4D_perm)
        {
            batchCount = 1;
        }

        if (src && dst)
        {
            if (evis)
            {
                shaderExecutable = vxnneTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, src, permArray, num, dst);
            }
            else
            {
                shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, src, permArray, num, dst);
            }

            vxoTensor_ReleaseTensor(&src);
            vxoTensor_ReleaseTensor(&dst);
        }
        else
        {
            if (evis)
            {
                shaderExecutable = vxnneTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, input, pPerm, num, output);
            }
            else
            {
                shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, input, pPerm, num, output);
            }
        }

        if (!shaderExecutable)
        {
            status = VX_FAILURE;
            goto OnError;
        }
        vxmONERROR(vxnneShaderOperation_Initialize(&tensor_trans_layer->tensor_trans_shader_operation,
            &tensor_trans_layer->base,
            VXNNE_OPERATOR_TENSOR_TRANS,
            batchCount,
            shaderExecutable));

        vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_shader_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
        vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_shader_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));

        vxmONERROR(vxnneLayer_SetOperation(
            &tensor_trans_layer->base,
            &tensor_trans_layer->tensor_trans_shader_operation.base,
            0));

        if (src2 && dst2)
        {
            vxnne_shader_executable shaderExecutable2 = VX_NULL;

            if (evis)
                shaderExecutable2 = vxnneTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, src2, permArray2, num2, dst2);
            else
                shaderExecutable2 = vxnneGPUTensorTransposeShaderExecutable(ops_layer->node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &ops_layer->node->kernelAttributes.borderMode, src2, permArray2, num2, dst2);

            vxmONERROR(vxnneShaderOperation_Initialize(&tensor_trans_layer->tensor_trans_shader_operation2,
                &tensor_trans_layer->base,
                VXNNE_OPERATOR_TENSOR_TRANS,
                batchCount2,
                shaderExecutable2));

            vxmONERROR(vxnneLayer_SetOperation(
                &tensor_trans_layer->base,
                &tensor_trans_layer->tensor_trans_shader_operation2.base,
                1));

            vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_shader_operation2.base, (vx_reference)src2, VXNNE_OPERATION_REFENRENCE_INPUT));
            vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_shader_operation2.base, (vx_reference)dst2, VXNNE_OPERATION_REFENRENCE_OUTPUT));

            vxmONERROR(vxoTensor_ReleaseTensor(&src2));
        }
    }
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, _num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNTensorTrans_SH_EVIS_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNTensorTrans_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_true_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}
VX_PRIVATE_API vx_bool vxoNNTensorTrans_SH_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_SHADER, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support = support && vxoNNTensorTrans_SH_EVIS_Support_Ext(node, parameters, num, reg_param, vx_false_e);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorTrans_SH_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    status = vxoNNTensorTrans_SH_Initialize_Ext(ops_layer, parameters, num, reg_param, vx_false_e);

    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_bool vxoNNTensorTrans_TP_Support(vx_node node, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_context context = vxGetContext((vx_reference)node);

    vx_tensor input = (vx_tensor)parameters[0];
    vx_scalar pnum = (vx_scalar)parameters[2];
    vx_tensor output = (vx_tensor)parameters[3];

    vx_bool support = vxoLayer_CheckSupport(node->base.context, VX_NN_QUERY_TP, VX_TYPE_INVALID, VX_NULL);

    vxoLayer_VerificationHead(node, parameters, num, reg_param);

    support =  support && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_TRANSPOSE) &&
                            vxnneIsTPSupportFormat(node->graph, input, VX_NULL, output) &&
                            (pnum->value->u32 > 1));

    support = support && IsTPSupport_CheckOutPixel(node->base.context, input, output);

    vxoLayer_VerificationFoot(node, parameters, num, reg_param, &support);

    return support;
}

VX_PRIVATE_API vx_status vxoNNTensorTrans_TP_Initialize(vxnne_layer ops_layer, const vx_reference parameters[], vx_uint32 num, vxnne_register_param reg_param)
{
    vx_status status = VX_SUCCESS;
    vxnne_tensor_trans_layer  tensor_trans_layer = (vxnne_tensor_trans_layer)ops_layer;

    vx_tensor input = (vx_tensor)parameters[0];
    vx_array  perm = (vx_array)parameters[1];
    vx_scalar pnum = (vx_scalar)parameters[2];
    vx_tensor output = (vx_tensor)parameters[3];
    vx_uint32 batchCount = 1;

    vx_op_param_s conv = { 0 };
    vx_uint32 dnum = pnum->value->u32;

    vxoLayer_InitializeHead(ops_layer, parameters, num, reg_param);

    vxmONERROR(vxnneOperation_Initialize(&tensor_trans_layer->tensor_trans_tp_operation.base,
        &tensor_trans_layer->base,
        VXNNE_OPERATION_TARGET_TP,
        VXNNE_OPERATOR_TENSOR_TRANS,
        VX_NULL,
        vxnneOperation_TP_Deinitialize,
        batchCount,
        0));

    conv.pad_x_left = 0;
    conv.pad_y_top = 0;
    conv.pool_size_x = 0;
    conv.pool_size_y = 0;
    conv.pool_stride = 1;
    conv.enable_relu = vx_false_e;
    conv.conv_rounding_type = 0;
    conv.pad_mode = VX_PAD_CONSTANT;
    conv.pad_const = 0;
    conv.tpType = TP_TRANSPOSE;
    conv.other_ref = (vx_reference)input;
    conv.data_buff = gcvNULL;
    conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
    conv.tp_value->u32[0] = dnum;
    conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
    vxMemCopy(conv.tp_value->p8[0], perm->memory.logicals[0], sizeof(vx_uint32) * dnum);

    vxMemCopy(&tensor_trans_layer->tensor_trans_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

    vxnneLayer_SetOperation(
        &tensor_trans_layer->base,
        &tensor_trans_layer->tensor_trans_tp_operation.base,
        0);

    tensor_trans_layer->tensor_trans_tp_operation.input = input;
    tensor_trans_layer->tensor_trans_tp_operation.output = output;

    vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT));
    vxmONERROR(vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_tp_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT));
OnError:
    vxoLayer_InitializeFoot(ops_layer, parameters, num, reg_param);

    return status;
}

VX_PRIVATE_API vx_status vxoNNLayer_GetOperations(vxnne_layer ops_layer, vx_uint32_ptr max_num_operations, vxnne_operation **operations)
{
    vx_status  status = VX_SUCCESS;
    vxnne_tensor_trans_layer  tensor_trans_layer = (vxnne_tensor_trans_layer)ops_layer;

    *max_num_operations = gcmCOUNTOF(tensor_trans_layer->operations);

    *operations = tensor_trans_layer->operations;

    return status;
}
#endif



VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num)
{
    vx_status status = VX_SUCCESS;
#if REGISTER_FRAME

    vxnne_layer_imp_s registerTensorTranss[] = {/* Please DON'T adjust the order, it's importent */
        { "TensorTrans NN", vxoNNCommon_NotSupport, vxoNNLayer_NotSupport_Initializer, VX_NULL },
        { "TensorTrans TP", vxoNNTensorTrans_TP_Support, vxoNNTensorTrans_TP_Initialize, VX_NULL },
        { "TensorTrans SH EVIS", vxoNNTensorTrans_SH_EVIS_Support, vxoNNTensorTrans_SH_EVIS_Initialize, VX_NULL },
        { "TensorTrans SH F32", vxoNNTensorTrans_SH_Support, vxoNNTensorTrans_SH_Initialize, VX_NULL },
        { "TensorTrans SW ", vxoNNCommon_Support, vxoNNTensorTrans_SW_Initialize, VX_NULL },
    };

    REGISTER_LAYERS(registerTensorTranss, vxnne_tensor_trans_layer_s, "TensorTranspose", vxoNNLayer_GetOperations);

OnError:
#else
    vx_context context = vxGetContext((vx_reference)node);

    vx_tensor input   = (vx_tensor)parameters[0];
    vx_array  perm    = (vx_array)parameters[1];
    vx_scalar pnum    = (vx_scalar)parameters[2];
    vx_tensor output  = (vx_tensor)parameters[3];
    vx_uint32 batchCount = 1, batchCount2 = 1;

    vxnne_tensor_trans_layer tensor_trans_layer = VX_NULL;

    /* destroy the existing layer */
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    gcoOS_Allocate(gcvNULL, sizeof(vxnne_tensor_trans_layer_s), (gctPOINTER*)&tensor_trans_layer);
    if (!tensor_trans_layer)
    {
        status = VX_ERROR_NO_MEMORY;
        vxError("allocate memory fail at function %s line %d", __FUNCTION__, __LINE__);
        goto exit;
    }

    gcoOS_ZeroMemory(tensor_trans_layer, sizeof(vxnne_tensor_trans_layer_s));

    vxnneLayer_Initialize(&tensor_trans_layer->base,
                          "TensorTranspose",
                          node,
                          vxmOPERATION_COUNT(tensor_trans_layer),
                          tensor_trans_layer->operations,
                          VX_NULL);

    if (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_TRANSPOSE) &&
        vxnneIsTPSupportFormat(node->graph, input, VX_NULL, output) &&
        (pnum->value->u32 > 1))
    {
        vx_op_param_s conv = {0};
        vx_uint32 dnum = pnum->value->u32;

        status = vxnneOperation_Initialize(&tensor_trans_layer->tensor_trans_tp_operation.base,
                                           &tensor_trans_layer->base,
                                           VXNNE_OPERATION_TARGET_TP,
                                           VXNNE_OPERATOR_TENSOR_TRANS,
                                           VX_NULL,
                                           vxnneOperation_TP_Deinitialize,
                                           batchCount,
                                           0);
        if (status != VX_SUCCESS) goto exit;

        conv.pad_x_left = 0;
        conv.pad_y_top = 0;
        conv.pool_size_x = 0;
        conv.pool_size_y = 0;
        conv.pool_stride = 1;
        conv.enable_relu = vx_false_e;
        conv.conv_rounding_type = 0;
        conv.pad_mode = VX_PAD_CONSTANT;
        conv.pad_const = 0;
        conv.tpType = TP_TRANSPOSE;
        conv.other_ref = (vx_reference)input;
        conv.data_buff = gcvNULL;
        conv.tp_value = (vx_tp_value_cmd)vxAllocateAndZeroMemory(sizeof(vx_tp_value_cmd_s));
        conv.tp_value->u32[0] = dnum;
        conv.tp_value->p8[0] = (vx_uint8_ptr)vxAllocateAndZeroMemory(sizeof(vx_uint32) * dnum);
        vxMemCopy(conv.tp_value->p8[0], perm->memory.logicals[0], sizeof(vx_uint32) * dnum);

        vxMemCopy(&tensor_trans_layer->tensor_trans_tp_operation.base.parameter, &conv, sizeof(vx_op_param_s));

        vxnneLayer_SetOperation(
            &tensor_trans_layer->base,
            &tensor_trans_layer->tensor_trans_tp_operation.base,
            0);

        tensor_trans_layer->tensor_trans_tp_operation.input  = input;
        tensor_trans_layer->tensor_trans_tp_operation.output = output;

        vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_tp_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
        vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_tp_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
    }
    else
    {
        vx_uint32_ptr  pPerm                   = (vx_uint32_ptr)perm->memory.logicals[0];
        vx_uint32      num                     = pnum->value->u32, num2 = 0;
        vx_uint32      batch                   = TENSOR_VIEW_SIZE_INDEX(input, 3);
        vx_bool        shExe_flag              = vx_true_e;
        vx_bool        shExe_copy_flag         = vx_true_e;
        vx_bool        enable_4Dtensor         = vx_false_e;
        vx_bool        enable_batch_sh         = vx_false_e;
        vx_bool        enable_dataFormat       = vx_false_e;
        vx_enum        inputFormat             = TENSOR_DATA_TYPE(input);
        vx_enum        outputFormat            = TENSOR_DATA_TYPE(output);
        vx_uint32      i                       = 0;

        if(context->evisNoInst.supportEVIS)
        {
            enable_dataFormat = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
                                          (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)     ||
                                          (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)     ||
                                          (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));
        }
        else
        {
            enable_dataFormat = (vx_bool)((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16) ||
                                          (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)     ||
                                          (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8));
        }

        enable_4Dtensor = (vx_bool)(enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 2 && pPerm[2] == 3 && pPerm[3] == 0 && num == 4 && batch == 1);
        enable_batch_sh = (vx_bool)(enable_dataFormat &&  pPerm[3] == 3 && num == 4 && TENSOR_DIM_NUM(input) == 4);

        shExe_flag    = (vx_bool)((enable_dataFormat && pPerm[0] == 2 && pPerm[1] == 0 && pPerm[2] == 1  && num == 3)
                                ||(enable_dataFormat && pPerm[0] == 2 && pPerm[1] == 1 && pPerm[2] == 0  && num == 3)
                                ||(enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 2 && pPerm[2] == 0  && num == 3)
                                ||(enable_dataFormat && pPerm[0] == 0 && pPerm[1] == 2 && pPerm[2] == 1  && num == 3)
                                ||(enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 0 && num <= 3 && num >= 2)
                                || (enable_dataFormat && pPerm[0] == 1 && pPerm[1] == 3 && pPerm[2] == 2 && pPerm[3] == 0 && num == 4)
                                || enable_4Dtensor || enable_batch_sh);

        for (i = 0; i < gcmMIN(TENSOR_DIM_NUM(input), num); i++)
        {
            shExe_copy_flag &= (pPerm[i] == i);
        }

        if(context->evisNoInst.supportEVIS)
        {
            shExe_copy_flag &= (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                                        || (inputFormat == VX_TYPE_INT16 && outputFormat == VX_TYPE_INT16)
                                        || (inputFormat == VX_TYPE_INT8 && outputFormat == VX_TYPE_INT8)
                                        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                        || (inputFormat == VX_TYPE_BFLOAT16 && outputFormat == VX_TYPE_BFLOAT16)
                                        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)));
        }
        else
        {
            shExe_copy_flag &= (vx_bool)(((inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT16)
                                        || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT32)
                                        || (inputFormat == VX_TYPE_FLOAT16 && outputFormat == VX_TYPE_FLOAT32)
                                        || (inputFormat == VX_TYPE_FLOAT32 && outputFormat == VX_TYPE_FLOAT16)
                                        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT32)
                                        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_UINT8)
                                        || (inputFormat == VX_TYPE_UINT8 && outputFormat == VX_TYPE_FLOAT16)));
        }

        if (shExe_copy_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vxnne_shader_executable shaderExecutable = VX_NULL;
            vx_tensor src          = NULL;
            vx_tensor dst          = NULL;
            vx_uint32 sizes[VX_CONTEXT_TENSOR_MAX_DIMENSION];
            vx_uint32 dims = 0;

            vxoElementOptimization_GetTensorShape(input, sizes, &dims);

            src     = vxoTensor_ReshapeTensor(input, (vx_int32*)sizes, dims, VX_NULL);
            dst     = vxoTensor_ReshapeTensor(output, (vx_int32*)sizes, dims, VX_NULL);

            tensor_trans_layer->base.temp_tensors[0] = src;
            tensor_trans_layer->base.temp_tensors[1] = dst;
            tensor_trans_layer->base.num_temp_tensors = 2;

            if(node->base.context->evisNoInst.supportEVIS)
            {
                if (src && dst)
                    shaderExecutable = vxnneTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_COPY, &node->kernelAttributes.borderMode, src, dst);
            }
            else
            {
                if (src && dst)
                    shaderExecutable = vxnneGPUTensorCopyShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_COPY, &node->kernelAttributes.borderMode, src, dst);
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&tensor_trans_layer->tensor_copy_sh_operation,
                &tensor_trans_layer->base,
                VXNNE_OPERATOR_CONVERT_FORMAT,
                1,
                shaderExecutable);

            if (status != VX_SUCCESS)
                goto exit;

            vxnneOperation_AddReference(&tensor_trans_layer->tensor_copy_sh_operation.base, (vx_reference)src, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tensor_trans_layer->tensor_copy_sh_operation.base, (vx_reference)dst, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &tensor_trans_layer->base,
                &tensor_trans_layer->tensor_copy_sh_operation.base,
                0);
        }
        else if(shExe_flag && (vxoContext_IsFeatureAvailable(node->base.context, VX_NN_FEATURE_SHADER)))
        {
            vx_uint32 width            = 0;
            vx_uint32 height           = 0;
            vx_uint32 depth            = 0;
            vx_int32  size[4]          = {0, 0, 0, 0};
            vx_uint32 permArray[4]     = {1, 2, 0, 3}, permArray2[4] = { 0, 2, 1, 3 };
            vx_uint32 dims             = 3;
            vx_tensor src = NULL, src2 = NULL;
            vx_tensor dst = NULL, dst2 = NULL;
            vxnne_shader_executable shaderExecutable = VX_NULL;

            width       = TENSOR_VIEW_SIZE_INDEX(input, 0);
            height      = TENSOR_DIM_NUM(input) > 1 ? TENSOR_VIEW_SIZE_INDEX(input, 1) : 1;
            depth       = TENSOR_DIM_NUM(input) > 2 ? TENSOR_VIEW_SIZE_INDEX(input, 2) : 1;
            batch       = TENSOR_DIM_NUM(input) > 3 ? TENSOR_VIEW_SIZE_INDEX(input, 3) : 1;

            batchCount = TENSOR_DIM_NUM(input) > 3  ? batch : 1;

            if (enable_batch_sh)
            {
                num = 3;
            }

            if (pPerm[0] == 2 && pPerm[1] == 0 && pPerm[2] == 1 && width * height < IMG_MAX_WIDTH && depth < IMG_MAX_WIDTH)
            {
                size[0] = width * height;
                size[1] = depth;
                size[2] = 1;
                size[3] = batch;
                dims    = TENSOR_DIM_NUM(input);
                src = vxoTensor_ReshapeTensor(input, size, dims, VX_NULL);

                size[0] = depth;
                size[1] = width * height;
                size[2] = 1;
                size[3] = batch;
                dims    = TENSOR_DIM_NUM(input);
                dst = vxoTensor_ReshapeTensor(output, size, dims, VX_NULL);

                num = 2;
            }
            else if (enable_4Dtensor)
            {
                size[0] = width;
                size[1] = height;
                size[2] = depth;
                dims    = 3;
                src = vxoTensor_ReshapeTensor(input, size, dims, VX_NULL);

                size[0] = height;
                size[1] = depth;
                size[2] = width;
                dims    = 3;
                dst = vxoTensor_ReshapeTensor(output, size, dims, VX_NULL);

                num = 3;
            }
            else if (pPerm[0] == 1 && pPerm[1] == 3 && pPerm[2] == 2 && pPerm[3] == 0 && width * height < IMG_MAX_WIDTH && depth < IMG_MAX_WIDTH)
            {
                size[0] = width;
                size[1] = height * depth * batch;
                size[2] = 1;
                dims = 3;
                src = vxoTensor_ReshapeTensor(input, size, dims, VX_NULL);

                size[0] = height * depth * batch;
                size[1] = width;
                size[2] = 1;
                dims = 3;

                {
                    vx_tensor_create_params_t param = {dims, VX_NULL, TENSOR_DATA_TYPE(input), TENSOR_QUANT_TYPE(input), };
                    param.sizes = (vx_uint32_ptr)size;

                    if (TENSOR_QUANT_TYPE(input) == VX_QUANT_DYNAMIC_FIXED_POINT)
                        param.quant_data.dfp.fixed_point_pos = TENSOR_POS(input);
                    else if (TENSOR_QUANT_TYPE(input) == VX_QUANT_AFFINE_SCALE)
                    {
                        param.quant_data.affine.scale = TENSOR_TF_SCALE(input);
                        param.quant_data.affine.zeroPoint = TENSOR_TF_ZEROPOINT(input);
                    }

                    dst = vxoTensor_CreateTensor2(node->base.context, &param, sizeof(vx_tensor_create_params_t));
                }

                num = 3;
                permArray[0] = 1;
                permArray[1] = 0;
                permArray[2] = 2;

                size[0] = height;
                size[1] = depth;
                size[2] = batch;
                size[3] = width;
                dims = 4;
                src2 = vxoTensor_ReshapeTensor(dst, size, dims, VX_NULL);

                dst2 = output;
                batchCount2 = width;
                num2 = 3;
            }
            else if (pPerm[0] == 1 && pPerm[1] == 2 && pPerm[2] == 0 && width < IMG_MAX_WIDTH && height * depth < IMG_MAX_WIDTH)
            {
                size[0] = width;
                size[1] = height * depth;
                size[2] = 1;
                size[3] = batch;
                dims    = TENSOR_DIM_NUM(input);
                src = vxoTensor_ReshapeTensor(input, size, dims, VX_NULL);

                size[0] = height * depth;
                size[1] = width;
                size[2] = 1;
                size[3] = batch;
                dims    = TENSOR_DIM_NUM(input);
                dst = vxoTensor_ReshapeTensor(output, size, dims, VX_NULL);

                num = 2;
            }

            if (src && dst)
            {
                if(node->base.context->evisNoInst.supportEVIS)
                {
                    shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, src, permArray, num, dst);
                }
                else
                {
                    shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, src, permArray, num, dst);
                }

                vxoTensor_ReleaseTensor(&src);
                vxoTensor_ReleaseTensor(&dst);
            }
            else
            {
                if(node->base.context->evisNoInst.supportEVIS)
                {
                    shaderExecutable = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, input, pPerm, num, output);
                }
                else
                {
                    shaderExecutable = vxnneGPUTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, input, pPerm, num, output);
                }
            }

            if (!shaderExecutable)
            {
                status = VX_FAILURE;
                goto exit;
            }
            status = vxnneShaderOperation_Initialize(&tensor_trans_layer->tensor_trans_shader_operation,
                &tensor_trans_layer->base,
                VXNNE_OPERATOR_TENSOR_TRANS,
                batchCount,
                shaderExecutable);

            if (status != VX_SUCCESS)
                goto exit;

            vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_shader_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_shader_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);

            vxnneLayer_SetOperation(
                &tensor_trans_layer->base,
                &tensor_trans_layer->tensor_trans_shader_operation.base,
                0);

            if (src2 && dst2)
            {
                vxnne_shader_executable shaderExecutable2 = VX_NULL;

                if (node->base.context->evisNoInst.supportEVIS)
                    shaderExecutable2 = vxnneTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, src2, permArray2, num2, dst2);
                else
                    shaderExecutable2 = vxnneGPUTensorTransposeShaderExecutable(node->base.context, VXNNE_KERNEL_GPU_TENSOR_TRANSPOSE, &node->kernelAttributes.borderMode, src2, permArray2, num2, dst2);

                status = vxnneShaderOperation_Initialize(&tensor_trans_layer->tensor_trans_shader_operation2,
                    &tensor_trans_layer->base,
                    VXNNE_OPERATOR_TENSOR_TRANS,
                    batchCount2,
                    shaderExecutable2);

                if (status != VX_SUCCESS)
                    goto exit;

                vxnneLayer_SetOperation(
                    &tensor_trans_layer->base,
                    &tensor_trans_layer->tensor_trans_shader_operation2.base,
                    1);

                vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_shader_operation2.base, (vx_reference)src2, VXNNE_OPERATION_REFENRENCE_INPUT);
                vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_shader_operation2.base, (vx_reference)dst2, VXNNE_OPERATION_REFENRENCE_OUTPUT);

                vxoTensor_ReleaseTensor(&src2);
            }
        }
        else
        {
            vxnneOperation_Initialize(&tensor_trans_layer->tensor_trans_sw_operation.base,
                                      &tensor_trans_layer->base,
                                      VXNNE_OPERATION_TARGET_SW,
                                      VXNNE_OPERATOR_TENSOR_TRANS,
                                      vxnneExecuteSWTensorTranspose,
                                      VX_NULL,
                                      batchCount,
                                      0);

            vxnneLayer_SetOperation(
                &tensor_trans_layer->base,
                &tensor_trans_layer->tensor_trans_sw_operation.base,
                0);

            tensor_trans_layer->tensor_trans_sw_operation.input   = input;
            tensor_trans_layer->tensor_trans_sw_operation.perm    = perm;
            tensor_trans_layer->tensor_trans_sw_operation.pnum    = pnum;
            tensor_trans_layer->tensor_trans_sw_operation.output  = output;

            vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_sw_operation.base, (vx_reference)input, VXNNE_OPERATION_REFENRENCE_INPUT);
            vxnneOperation_AddReference(&tensor_trans_layer->tensor_trans_sw_operation.base, (vx_reference)output, VXNNE_OPERATION_REFENRENCE_OUTPUT);
        }
    }

    node->layer = &tensor_trans_layer->base;
    return status;

exit:
    if (tensor_trans_layer)
        gcoOS_Free(NULL, (gctPOINTER)tensor_trans_layer);
#endif


    return status;
}

VX_PRIVATE_API vx_status VX_CALLBACK vxoNNTensorTrans_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num)
{
    if (node->layer)
    {
        vxnneLayer_Free(node->layer);
        node->layer = VX_NULL;
    }

    return VX_SUCCESS;
}

