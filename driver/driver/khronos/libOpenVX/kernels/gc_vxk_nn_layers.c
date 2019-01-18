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


#include <gc_vxk_common.h>
#include <gc_vx_nn_util.h>

#if NN_LAYER_C
vx_status vxNNLayersC(vx_node node, vx_array cmd_buf, vx_weights_biases_parameter weights_biases,
                     vx_tensor inputs, vx_uint32 inputOffset,
                     vx_tensor outputs, vx_uint32 outputOffset);
#endif

/*
 * A dump block refers to:
 * 1. Multiple operations belong to the same SW tiling block.
 * 2. Single operation which is NOT in any SW tiling block.
 *
 * For each block, the followings will be dumped.
 * 1. Input(s).
 * 2. Output(s).
 * 3. Commands.
 * 4. Weights and bias (if any).
 *
 * The dump format is as below.
 * 1. Driver adds the tag for each dump section as below.
 *    @[dump id=xx]
 *      ...
 *    @[dump_end id=xx, layerid=y1, y2, ..., y, operationid=z1, z2, ..., z]
 * 2. Parser replaces NN and TP trigger commands in each section if the dump id
 *    doesn’t match the requested one.
 */
vx_status vxOpCommandDump(
    vxnne_operation_command opCommand,
    vxnne_operation operation,
    vx_enum dumpStage
    )
{
    enum vx_status_e status = VX_SUCCESS;

#if gcdDUMP
    static vx_uint32 dumpId = 0;
    vxnne_command_buffer commandBuffer;
    vx_bool dumpHeader, dumpInput, dumpOutput, dumpFooter;
    vxnne_operation_target_e target = VXNNE_OPERATION_TARGET_NONE;
    vxnne_convolution_relu_pooling_operation convOperation = VX_NULL;
    vxnne_tp_operation tpOperation = VX_NULL;
    vx_tensor inputs = VX_NULL, outputs = VX_NULL;
    gctPOINTER logical = VX_NULL;
    vx_uint32 physical = ~0U;
    vx_uint32 size = 0;

    if (!opCommand)
    {
        return VX_ERROR_INVALID_PARAMETERS;
    }

    target = operation->target;

    switch(target)
    {
    case VXNNE_OPERATION_TARGET_NN:
        convOperation = (vxnne_convolution_relu_pooling_operation)operation;
        inputs = (vx_tensor)convOperation->inputs;
        outputs = (vx_tensor)convOperation->outputs;

        break;

    case VXNNE_OPERATION_TARGET_TP:
        tpOperation = (vxnne_tp_operation)operation;
        inputs = tpOperation->input;
        outputs = tpOperation->output;

        break;

#if gcdDUMP_PER_OPERATION
    case VXNNE_OPERATION_TARGET_SH:
        /*
         * Just trigger PPU to make sure PPU commands is not
         * saved in TP/NN dump block. Sync-flush to flush
         * PPU's outputs so that it dumps the correct inputs
         * for TP/NN.
         */
        gcoVX_Flush(gcvTRUE);

        return VX_SUCCESS;
#endif

    default:
        return VX_SUCCESS;
    }

    commandBuffer = &opCommand->commandBuffer;

    dumpHeader = dumpInput = dumpOutput = dumpFooter = vx_false_e;

    switch (dumpStage)
    {
    case VXNNE_DUMP_PRE_EXECUTION:

#if gcdDUMP_PER_OPERATION
        switch (opCommand->blockFlag)
        {
        case VXNNE_BLOCK_FLAG_NONE:
        case VXNNE_BLOCK_FLAG_START:
            dumpHeader = vx_true_e;
            dumpInput = vx_true_e;

            break;

        case VXNNE_BLOCK_FLAG_INTERNAL:
        case VXNNE_BLOCK_FLAG_END:
            break;

        default:
            status = VX_ERROR_INVALID_TYPE;
            break;
        }
#endif

        break;

    case VXNNE_DUMP_POST_EXECUTION:
#if gcdDUMP_PER_OPERATION
        switch (opCommand->blockFlag)
        {
        case VXNNE_BLOCK_FLAG_NONE:
        case VXNNE_BLOCK_FLAG_END:
            dumpOutput = vx_true_e;
            dumpFooter = vx_true_e;

            break;

        case VXNNE_BLOCK_FLAG_START:
        case VXNNE_BLOCK_FLAG_INTERNAL:
            break;

        default:
            status = VX_ERROR_INVALID_TYPE;
            break;
        }
#endif

        break;

    default:
        status = VX_ERROR_INVALID_PARAMETERS;
        break;
    }


    if (dumpHeader)
    {
        gcmDUMP(gcvNULL, "@[dump id=%u]\n", dumpId);
    }

    if (dumpInput)
    {
        vxoTensor_GetTensorBatchArrayViewMemory(inputs, 0, &logical, &physical);
        vxoTensor_GetTensorSize(inputs, &size);

        gcmDUMP(gcvNULL, "#[input]\n");

        gcmDUMP_BUFFER(gcvNULL,
                       gcvDUMP_BUFFER_MEMORY,
                       physical,
                       logical,
                       0,
                       size);
    }

    if (dumpOutput)
    {
        vxoTensor_GetTensorBatchArrayViewMemory(outputs, 0, &logical, &physical);
        vxoTensor_GetTensorSize(outputs, &size);

        vxnneMultiChannel_SetCurrentChannel(VXNNE_OPERATION_TARGET_SH);
        gcoVX_Flush(gcvTRUE);
        vxnneMultiChannel_SetCurrentChannel(target);

        gcmDUMP(gcvNULL, "#[output]\n");

        gcmDUMP_BUFFER(gcvNULL,
                       gcvDUMP_BUFFER_VERIFY,
                       physical,
                       logical,
                       0,
                       size);
    }

    if (dumpFooter)
    {
        gcmDUMP(gcvNULL, "@[dumpend id=%u, layerid=0, operationid=0]\n", dumpId++);
    }
#endif

    return status;
}

#if NN_LAYER_C

#define NN_DEBUG 0

#if NN_DEBUG
#define NN_DEBUG_INFO 0
#define NN_DEBUG_CHECK_KERNEL_PER_CORE 1
#define NN_DEBUG_LOCAL_COUNT 0
#define NN_DEBUG_DUMP 0
#else
#define NN_DEBUG_INFO 0
#define NN_DEBUG_CHECK_KERNEL_PER_CORE 0
#define NN_DEBUG_LOCAL_COUNT 0
#define NN_DEBUG_DUMP 0
#endif

#define NN_KERNEL_ZRL 1

#define gcmALIGN_NP3(x, y) gcmALIGN_NP2((x), (y))/(y)

extern vx_float32 Fp16toFp32(vx_int16 in);

extern vx_uint16 Fp32toFp16(vx_float32 in);

typedef struct _gc_nn_inst
{
    /* Word0*/
    vx_uint32     layerType               :  1;   /* [0]*/
    vx_uint32     _pad0                   :  1;   /* [1]*/
    vx_uint32     kernelXYsize            :  4;   /* [5:2]*/
    vx_uint32     kernelZsize             : 14;   /* [19:6]*/
    vx_uint32     kernelsPerCore          :  7;   /* [26:20]*/
    vx_uint32     pooling                 :  2;   /* [28:27]*/
    vx_uint32     poolingXYsize           :  1;   /* [29]*/
    vx_uint32     _pad3                   :  1;   /* [30]*/
    vx_uint32     lastLayer               :  1;   /* [31]*/
    /* Word1*/
    vx_uint32     kernelDataSizeMinus1    :  1;   /* [0]*/
    vx_uint32     _pad4                   :  1;   /* [1]*/
    vx_uint32     inImageDataSizeMinus1   :  1;   /* [2]*/
    vx_uint32     _pad5                   :  1;   /* [3]*/
    vx_uint32     outImageDataSizeMinus1  :  1;   /* [4]*/
    vx_uint32     _pad6                   :  1;   /* [5]*/
    vx_uint32     inImageXsize            : 13;   /* [18:6]*/
    vx_uint32     inImageYsize            : 13;   /* [31:19]*/
    /* Word2*/
    vx_int32     inImageXoffset          :  3;   /* [2:0]*/
    vx_int32     inImageYoffset          :  3;   /* [5:3]*/
    vx_uint32     inImageSigned           :  1;   /* [6]*/
    vx_uint32     brickMode               :  1;   /* [7]*/
    vx_uint32     brickDistance           : 16;   /* [23:8]*/
    vx_uint32     relu                    :  1;   /* [24]*/
    vx_uint32     _pad11                  :  1;   /* [25]*/
    vx_uint32     postMultiplier          :  1;   /* [26:26]*/
    vx_uint32     postShift               :  5;   /* [31:27]*/
    /* Word3*/
    vx_uint32     oneTile                 :  1;   /* [0]*/
    vx_uint32     noOutput                :  1;   /* [1]*/
    vx_uint32     noBias                  :  1;   /* [2]*/
    vx_uint32     noFlush                 :  1;   /* [3]*/
    vx_uint32     _pad12                  :  2;   /* [5:4]*/
    vx_uint32     outImageXsize           : 13;   /* [18:6]*/
    vx_uint32     outImageYsize           : 13;   /* [31:19]*/
    /* Word4*/
    vx_uint32     outImageZsize           : 14;   /* [13:0]*/
    vx_uint32     _pad13                  :  4;   /* [19:14]*/
    vx_uint32     outImageTileXsize       :  7;   /* [25:20]*/
    vx_uint32     outImageTileYsize       :  7;   /* [31:26]*/
    /* Word5*/
    vx_uint32     kernelBaseAddress       : 26;   /* [25:0]*/
    vx_uint32     _pad14                  :  6;   /* [31:26]*/
    /* Word6*/
    vx_uint32     inputBaseAddress;               /* [31:0]*/
    /* Word7*/
    vx_uint32     outputBaseAddress;              /* [31:0]*/
}
gc_nn_inst;

VX_INTERNAL_API vx_int32 vx_nnu_get_type_size(vx_type_e format)
{
    vx_int32 size = 0;
    switch(format)
    {
    case VX_TYPE_INT8:
        size = sizeof(vx_int8);
        break;
    case VX_TYPE_FLOAT16:
        size = sizeof(vx_int16);
        break;
    case VX_TYPE_FLOAT32:
        size = sizeof(vx_float32);
        break;
    default:
        vxError("Not support format :%d\n", format);
        break;
    }

    return size;
}

VX_INTERNAL_API vx_float32 vx_nnu_get_date(vx_type_e format, vx_int32 index, vx_uint8_ptr data)
{
    vx_float32 value = 0;

    if (index >= 0)
    {
        switch(format)
        {
        case VX_TYPE_INT8:
            value = (vx_int8)data[index];
            break;
        case VX_TYPE_FLOAT16:
            {
                vx_int16_ptr data_ptr = (vx_int16_ptr)data;
                value = Fp16toFp32(data_ptr[index]);
            }
            break;
        case VX_TYPE_FLOAT32:
            {
                vx_float32_ptr data_ptr = (vx_float32_ptr)data;
                value = data_ptr[index];
            }
            break;
        default:
            vxError("Not support format :%d\n", format);
            break;
        }
    }
    return value;
}

VX_INTERNAL_API vx_status vx_nnu_save_date(vx_type_e format, vx_int32 index, vx_float32 data, vx_ptr dst_data)
{
    switch(format)
    {
    case VX_TYPE_INT8:
        {
            vx_int8* dst_data_p = (vx_int8*)dst_data;
            dst_data_p[index] = (vx_int8)(data);
        }
        break;
    case VX_TYPE_FLOAT16:
        {
            vx_int16* dst_data_p = (vx_int16*)dst_data;
            dst_data_p[index] = Fp32toFp16(data);
        }
        break;
    case VX_TYPE_FLOAT32:
        {
            vx_float32_ptr dst_data_p = (vx_float32_ptr)dst_data;
            dst_data_p[index] = (vx_float32)(data);
        }
        break;
    default:
        vxError("Not support format :%d\n", format);
        break;
    }

    return VX_SUCCESS;
}

void Conv_MAC(vx_int32 interleave_mode, vx_int32 coef_base, vx_uint8_ptr weights_data, vx_int32 w, vx_int32 h, vx_type_e format, vx_uint8_ptr input_data,
              vx_int32 tx, vx_int32 kernel_x_group_size, vx_int32 pad, vx_float32_ptr accum_buf, vx_int32 k, vx_int32 kernel_per_core, vx_int32 ki,
              vx_int32 y, vx_int32 z, vx_int32 input_w, vx_int32 input_h, vx_int32 output_w, vx_int32 output_h, vx_int32 kernel_x
#if NN_DEBUG_LOCAL_COUNT
, vx_int32_ptr debug_loop_count
#endif
              )
{
    vx_int32 tyi = 0, x = 0;
    vx_int32 coef_index = coef_base + w + h * kernel_x;

    vx_float32 data = 0, coef_data = vx_nnu_get_date(format, coef_index, (vx_uint8_ptr)weights_data);

    for (tyi = y; tyi < y + interleave_mode; tyi += interleave_mode)
    {
        for (x = tx; x < tx + kernel_x_group_size; x ++)

        {

            vx_int32 _x = x + w - pad, _y = y + h - pad;

            vx_int32 data_index = z * input_w * input_h + _y * input_w + _x;
            vx_int32 buff_index = x + output_w * y + (k * kernel_per_core + ki) * output_w * output_h;


            if ((_y < 0) || (_x < 0) || (((_y >= output_h) || (_x >= output_w)) && (pad > 0)))
                data = 0;
            else
                data = vx_nnu_get_date(format, data_index, (vx_uint8_ptr)input_data);


            accum_buf[buff_index] += data * coef_data;

#if NN_DEBUG_LOCAL_COUNT
            (*debug_loop_count)++;
#endif
        }
    }
}

void Tile2DConv(vx_int32 interleave_mode, vx_int32 ty, vx_int32 tile_y_group, vx_int32 tile_y_size, vx_int32 depth_input, vx_int32 output_w, vx_int32 output_h, vx_int32 input_w, vx_int32 input_h,
                vx_float32_ptr accum_buf, vx_uint8_ptr bias_data, vx_uint8_ptr weights_data, vx_type_e format, vx_uint8_ptr input_data,
                vx_int32 kernel_x, vx_int32 kernel_y, vx_int32 z, vx_int32 k, vx_int32 kernel_per_core, vx_int32 ki, vx_int32 tx, vx_int32 kernel_x_group_size, vx_int32 pad
#if NN_DEBUG_LOCAL_COUNT
, vx_int32_ptr debug_loop_count
#endif
              )
{
    /* Tile2DConv() */
    vx_int32 y = 0, w = 0, h = 0, q = 0;
    vx_int32 current_tile_y_size = (ty < (tile_y_group - 1))?tile_y_size:(output_h - (tile_y_group - 1)*tile_y_size);/*4 or 55%4=3*/
    vx_int32 coef_base = z * kernel_x * kernel_y + (k * kernel_per_core + ki) * depth_input * kernel_x * kernel_y;

    for (y = ty * tile_y_size; y < ty * tile_y_size + current_tile_y_size; y ++)
    {
        if (z == 0)
        {
            for (q = 0; q < output_w; q ++)
            {
                vx_float32 bias = vx_nnu_get_date(VX_TYPE_FLOAT32, k * kernel_per_core + ki, (vx_uint8_ptr)bias_data);
                vx_int32 index = q + output_w * y + (k * kernel_per_core + ki) * output_w * output_h;
                accum_buf[index] = bias;
            }
        }

        for (h = 0; h < kernel_y; ++ h)
        {
            for (w = 0; w < kernel_x; ++ w)
            {
                Conv_MAC(interleave_mode, coef_base, weights_data, w, h, format, input_data, tx, kernel_x_group_size, pad, accum_buf, k, kernel_per_core, ki,
                        y, z, input_w, input_h, output_w, output_h, kernel_x
#if NN_DEBUG_LOCAL_COUNT
, debug_loop_count
#endif
                  );
            }
        }
    }
}

void ConvJobs(vx_int32 interleave_mode, vx_int32 kernel_z_group_size, vx_int32 k, vx_int32 kernel_group, vx_int32 kernel_per_core, vx_int32 depth_output, vx_int32 ty, vx_int32 tile_y_group,
              vx_float32_ptr accum_buf, vx_uint8_ptr bias_data, vx_uint8_ptr weights_data, vx_type_e format, vx_uint8_ptr input_data,
              vx_int32 tile_y_size, vx_int32 depth_input, vx_int32 output_w, vx_int32 output_h, vx_int32 input_w, vx_int32 input_h, vx_int32 kernel_x,
              vx_int32 kernel_y, vx_int32 z, vx_int32 tx, vx_int32 kernel_x_group_size, vx_int32 pad
#if NN_DEBUG_LOCAL_COUNT
, vx_int32_ptr debug_loop_count
#endif
              )
{
    vx_int32 ki = 0;

    /* Tile2DConvJobs() */
    /* ConvJobs() */
    kernel_z_group_size = (k < (kernel_group - 1))?kernel_per_core:(depth_output - kernel_per_core * k);/*14 or 12*/
    for (ki = 0; ki < kernel_z_group_size; ki ++)
    {
        /* Tile2DConv() */
        Tile2DConv(interleave_mode, ty, tile_y_group, tile_y_size, depth_input, output_w, output_h, input_w, input_h,
                        accum_buf, bias_data, weights_data, format, input_data,
                        kernel_x, kernel_y, z, k, kernel_per_core, ki,
                        tx, kernel_x_group_size, pad
#if NN_DEBUG_LOCAL_COUNT
, debug_loop_count
#endif
                  );

    }
}

void Tile2DConvJobs(vx_int32 interleave_mode, vx_int32 kernel_z_group_size, vx_int32 k, vx_int32 kernel_group, vx_int32 kernel_per_core, vx_int32 depth_output, vx_int32 ty, vx_int32 tile_y_group,
              vx_float32_ptr accum_buf, vx_uint8_ptr bias_data, vx_uint8_ptr weights_data, vx_type_e format, vx_uint8_ptr input_data,
              vx_int32 tile_y_size, vx_int32 depth_input, vx_int32 output_w, vx_int32 output_h, vx_int32 input_w, vx_int32 input_h, vx_int32 kernel_x,
              vx_int32 kernel_y, vx_int32 z, vx_int32 tx, vx_int32 kernel_x_group_size, vx_int32 pad
#if NN_DEBUG_LOCAL_COUNT
, vx_int32_ptr debug_loop_count
#endif
              )
{
    ConvJobs(interleave_mode, kernel_z_group_size, k, kernel_group, kernel_per_core, depth_output, ty, tile_y_group,
              accum_buf, bias_data, weights_data, format, input_data,
              tile_y_size, depth_input, output_w, output_h, input_w, input_h, kernel_x,
              kernel_y, z, tx, kernel_x_group_size, pad
#if NN_DEBUG_LOCAL_COUNT
, debug_loop_count
#endif
                  );
}

void Tile3DConvJobs(vx_int32 interleave_mode, vx_int32 k, vx_int32 kernel_group, vx_int32 kernel_per_core, vx_int32 depth_output, vx_int32 ty, vx_int32 tile_y_group,
              vx_float32_ptr accum_buf, vx_uint8_ptr bias_data, vx_uint8_ptr weights_data, vx_type_e format, vx_uint8_ptr input_data,
              vx_int32 tile_y_size, vx_int32 depth_input, vx_int32 output_w, vx_int32 output_h, vx_int32 input_w, vx_int32 input_h, vx_int32 kernel_x,
              vx_int32 kernel_y, vx_int32 tx, vx_int32 kernel_x_group_size, vx_int32 pad
#if NN_DEBUG_LOCAL_COUNT
, vx_int32_ptr debug_loop_count
#endif
              )
{
    vx_int32 z = 0;
    vx_int32 kernel_z_group_size = 0;

    for (z = 0; z < depth_input; z ++)
    {
        Tile2DConvJobs(interleave_mode, kernel_z_group_size, k, kernel_group, kernel_per_core, depth_output, ty, tile_y_group,
                  accum_buf, bias_data, weights_data, format, input_data,
                  tile_y_size, depth_input, output_w, output_h, input_w, input_h, kernel_x,
                  kernel_y, z, tx, kernel_x_group_size, pad
#if NN_DEBUG_LOCAL_COUNT
, debug_loop_count
#endif
                  );
    }
}
void Image3DConvJobs(vx_int32 interleave_mode, vx_int32 kernel_per_core, vx_int32 depth_output, vx_int32 tile_y_group,
              vx_float32_ptr accum_buf, vx_uint8_ptr bias_data, vx_uint8_ptr weights_data, vx_type_e format, vx_uint8_ptr input_data,
              vx_int32 tile_y_size, vx_int32 depth_input, vx_int32 output_w, vx_int32 output_h, vx_int32 input_w, vx_int32 input_h, vx_int32 kernel_x,
              vx_int32 kernel_y, vx_int32 pad, vx_int32 tile_x_group, vx_int32 tile_x_size
#if NN_DEBUG_LOCAL_COUNT
, vx_int32_ptr debug_loop_count
#endif
              )
{
    vx_int32 tx = 0, ty = 0, k = 0;
    /* Image3DConvJobs() */
    for (ty = 0; ty < tile_y_group; ty ++)
    {
        for (tx = 0; tx < tile_x_group; tx ++)
        {
            vx_int32 kernel_x_group_size = (tx < (tile_x_group - 1))?tile_x_size:output_w - tx * tile_x_size;
            vx_int32 kernel_group = gcmALIGN_NP3(depth_output, kernel_per_core);

            for (k = 0; k < kernel_group; k ++)
            {
                /* Tile3DConvJobs() */
                Tile3DConvJobs(interleave_mode, k, kernel_group, kernel_per_core, depth_output, ty, tile_y_group,
                  accum_buf, bias_data, weights_data, format, input_data,
                  tile_y_size, depth_input, output_w, output_h, input_w, input_h, kernel_x,
                  kernel_y, tx, kernel_x_group_size, pad
#if NN_DEBUG_LOCAL_COUNT
, debug_loop_count
#endif
                  );
            }

        }
    }
}

vx_status vx_nnk_pool_avg(vx_uint8_ptr src, vx_type_e format, vx_int32 input_width, vx_int32 input_height, vx_int32 depth, vx_int32_ptr output_width, vx_int32_ptr output_height, vx_int32 stride, vx_int32 kernel_size, vx_int32 pad, vx_uint8_ptr dst)
{
    vx_uint8_ptr data = 0;
    vx_uint8_ptr data_d = 0;
    vx_int32 i = 0, j = 0, p = 0;

    vx_int32 width = input_width;
    vx_int32 height = input_height;
    vx_int32 depth_v = depth;
    vx_int32 stride_v = stride;
    vx_int32 kernel_v = kernel_size;
    vx_int32 pad_v = pad;
    vx_int32 width_o = (vx_uint32)((ceilf((width + 2.0f * pad_v - kernel_v)/stride_v)) + 1);
    vx_int32 height_o = (vx_uint32)((ceilf((height + 2.0f * pad_v - kernel_v)/stride_v)) + 1);

    *output_width = width_o;
    *output_height = height_o;

    data = src;
    data_d = dst;

    for (p = 0; p < depth_v; p ++)
    {
        for (j = 0; j < height_o; j ++)
        {
            for (i = 0; i < width_o; i ++)
            {
                vx_int32 pad_h = pad_v, pad_w = pad_v;
                vx_int32 hstart = j * stride_v - pad_h;
                vx_int32 wstart = i * stride_v - pad_w;
                vx_int32 hend = gcmMIN(hstart + kernel_v, height);
                vx_int32 wend = gcmMIN(wstart + kernel_v, width);
                vx_int32 pool_index = 0;
                vx_int32 h, w = 0;
                vx_float32 sum = 0;

                hstart = gcmMAX(hstart, 0);
                wstart = gcmMAX(wstart, 0);

                pool_index = j * (width_o) + i;

                for (h = hstart; h < hend; ++ h)
                {
                    for (w = wstart; w < wend; ++ w)
                    {
                        const vx_int32 index = h * (width) + w;
                        sum += vx_nnu_get_date(format, index, (vx_uint8_ptr)data);
                    }
                }

                vx_nnu_save_date(format, pool_index, sum/((hend - hstart) * (wend - wstart)), data_d);
            }

        }

        data += width * height * vx_nnu_get_type_size(format);
        data_d += width_o * height_o * vx_nnu_get_type_size(format);
    }

    return VX_SUCCESS;
}

#define FP32_MAX 3.402823466e+38F

vx_status vx_nnk_pool_max(vx_uint8_ptr src, vx_type_e format, vx_int32 input_width, vx_int32 input_height, vx_int32 depth, vx_int32_ptr output_width, vx_int32_ptr output_height, vx_int32 stride, vx_int32 kernel_size, vx_int32 pad, vx_uint8_ptr dst)
{
    vx_uint8_ptr data = 0;
    vx_uint8_ptr data_d = 0;
    vx_int32 i = 0, j = 0, p = 0;

    vx_int32 width = input_width;
    vx_int32 height = input_height;
    vx_int32 depth_v = depth;
    vx_int32 stride_v = stride;
    vx_int32 kernel_v = kernel_size;
    vx_int32 pad_v = pad;
    vx_int32 width_o = (vx_uint32)((ceilf((width + 2.0f * pad_v - kernel_v)/stride_v)) + 1);
    vx_int32 height_o = (vx_uint32)((ceilf((height + 2.0f * pad_v - kernel_v)/stride_v)) + 1);

    *output_width = width_o;
    *output_height = height_o;

    data = src;
    data_d = dst;

    for (p = 0; p < depth_v; p ++)
    {
        for (j = 0; j < height_o; j ++)
        {
            for (i = 0; i < width_o; i ++)
            {
                vx_int32 pad_h = pad_v, pad_w = pad_v;
                vx_int32 hstart = j * stride_v - pad_h;
                vx_int32 wstart = i * stride_v - pad_w;
                vx_int32 hend = gcmMIN(hstart + kernel_v, height);
                vx_int32 wend = gcmMIN(wstart + kernel_v, width);
                vx_int32 pool_index = 0;
                vx_int32 h, w = 0;
                vx_float32 d_f32 = -FP32_MAX;

                hstart = gcmMAX(hstart, 0);
                wstart = gcmMAX(wstart, 0);

                pool_index = j * (width_o) + i;

                for (h = hstart; h < hend; ++ h)
                {
                    for (w = wstart; w < wend; ++ w)
                    {
                        const vx_int32 index = h * (width) + w;

                        vx_float32 d = vx_nnu_get_date(format, index, (vx_uint8_ptr)data);

                        if (d > d_f32)
                            d_f32 = d;

                    }
                }

                vx_nnu_save_date(format, pool_index, d_f32, data_d);
            }
        }

        data += width * height * vx_nnu_get_type_size(format);
        data_d += width_o * height_o * vx_nnu_get_type_size(format);
    }

    return VX_SUCCESS;
}

vx_status vx_nnk_pool_nn_layer_cpu(vx_uint8_ptr src, vx_int32 type, vx_type_e format, vx_int32 input_width, vx_int32 input_height, vx_int32 depth,
                                vx_int32_ptr output_width, vx_int32_ptr output_height, vx_int32 stride, vx_int32 kernel_size, vx_int32 pad, vx_uint8_ptr dst)
{
    switch (type)
    {
        case 1:
            vx_nnk_pool_max(src, format, input_width, input_height, depth, output_width, output_height, stride, kernel_size, pad, dst);
        break;
        case 2:
            vx_nnk_pool_avg(src, format, input_width, input_height, depth, output_width, output_height, stride, kernel_size, pad, dst);
        break;
    }

    return VX_SUCCESS;
}
vx_status vx_nnk_relu_NN_LAYER_Cpu(vx_uint8_ptr input_data, vx_type_e dst_format, vx_uint32 count, vx_uint8_ptr output_data)
{
    vx_uint32 i = 0;

    for (i = 0; i < count; i ++)
        vx_nnu_save_date(dst_format, i, gcmMAX(0.0f, vx_nnu_get_date(dst_format, i, input_data)), output_data);


    return VX_SUCCESS;
}

vx_int32 vx_nnu_get_interleave(vx_int32 mad_per_core, vx_int32 tile_x_size)
{
    vx_int32 half_mad_per_core = mad_per_core / 2, quarter_mad_per_core = mad_per_core / 4;
    vx_int32 interleave_mode = 0, in_interleave_mode = 0, out_interleave_mode = (tile_x_size > half_mad_per_core)?1:((tile_x_size > quarter_mad_per_core)?2:4);

    mad_per_core +=  8;
    half_mad_per_core = mad_per_core / 2;
    quarter_mad_per_core = mad_per_core / 4;
    in_interleave_mode = (tile_x_size > half_mad_per_core)?1:((tile_x_size > quarter_mad_per_core)?2:4);
    interleave_mode = gcoMATH_MIN(in_interleave_mode, out_interleave_mode);

    return interleave_mode;
}

vx_int32 vx_nnu_get_kernel_per_core(vx_int32 mad_per_core, vx_int32 interleave_mode, vx_int32 tile_y_size, vx_int32 out_z_size)
{
    vx_int32 kernel_per_core = gcmMIN((mad_per_core - 8), ((mad_per_core - 8) * interleave_mode)/tile_y_size);

    return gcmMIN(out_z_size, kernel_per_core);
}

vx_int32 vx_nnu_get_tile_x_size(vx_int32 accum_buf_size, vx_int32 input_width, vx_int32 output_width)
{
    return gcmMIN(gcmMIN(accum_buf_size, output_width), input_width);
}

vx_int32 vx_nnu_get_tile_y_size(vx_int32 accum_buf_size, vx_int32 tile_x_size, vx_int32 output_height)
{
    return gcmMIN(accum_buf_size/tile_x_size, output_height);
}

#if NN_KERNEL_ZRL
vx_uint32 gc_vx_read_bit(vx_uint32_ptr ptr, vx_int32_ptr bit_offset, vx_uint32 bit_length)
{
    vx_uint32 byte = *bit_offset / 32;
    vx_uint32 bit = *bit_offset % 32;
    vx_uint32_ptr p = ptr + byte;
    vx_uint32 data = 0;

    gcmASSERT(bit_length <= 32);

    if (bit + bit_length < 32)
    {
        /* (data << bit) & ((1 << (32 - (bit + bit_length))) - 1);
         * p__bit____bit+len___________________
         * |   |////////|    |                 |
         * |___|////////|____|_________________|
         */
        data = *p >> bit;
        data &= (1 << bit_length) - 1;
    }
    else
    {
        /*
         * p__bit_________bit+len______________
         * |   |/////////////|//|              |
         * |___|/////////////|//|______________|
         */
        vx_uint32_ptr low_p = p;
        vx_uint32_ptr high_p = p + 1;

        vx_uint32 data_low = (*low_p) >> bit;
        vx_uint32 data_high = (*high_p) & ((1 << (bit + bit_length - 32)) - 1);

        data = data_low | (data_high << (32 - bit));

    }

    *bit_offset += bit_length;

    return data;
}

vx_status vx_nnk_coef_decompress_zrl(vx_uint8_ptr src, vx_type_e format, vx_int32 kernel_per_core, vx_int32 width,
                                     vx_int32 output_w, vx_int32 output_h, vx_int32 depth_output, vx_int32 kernel_x, vx_int32 depth_input, vx_uint8_ptr* weight_buffer, vx_uint8_ptr* bias_buffer)
{
    vx_uint8_ptr weights_data = src;
    vx_int32 zrlw = 0, zrl = 0, bit_offset = 0, total = 0, z_size = 0;
    vx_int32 kx = 0, ky = 0;
    vx_int32 group_count    = gcmALIGN_NP3(depth_output, kernel_per_core);
    vx_uint8_ptr weights    = VX_NULL;
    vx_float32* bias        = VX_NULL;
    vx_uint32 offset_size   = 3;       /*offsetSize is 3 bytes*/
    vx_int32 g = 0, k = 0, s = 0;
    vx_int32 size = depth_input * kernel_x * kernel_x * depth_output * vx_nnu_get_type_size(format);

    *weight_buffer = (vx_uint8_ptr)vxAllocateAndZeroMemory(size);
    *bias_buffer = (vx_uint8_ptr)vxAllocateAndZeroMemory(depth_output * sizeof(vx_float32));

    group_count = gcmALIGN_NP3(depth_output, kernel_per_core);


    total  = gc_vx_read_bit((vx_uint32_ptr)weights_data, &bit_offset, sizeof(vx_uint32) * 8);
    bit_offset = 64 * 8;
    zrlw   = gc_vx_read_bit((vx_uint32_ptr)weights_data, &bit_offset, sizeof(vx_uint8) * 8);
    z_size = gc_vx_read_bit((vx_uint32_ptr)weights_data, &bit_offset, sizeof(vx_int16) * 8);

    zrl  = gc_vx_read_bit((vx_uint32_ptr)weights_data, &bit_offset, zrlw);

    /* fill weight value and bias for every group */
    for (g = 0; g < group_count; g++)
    {
        for (s = 0; s < depth_input; s++)
        {
            vx_int32 group_size = (g == group_count - 1) && ((depth_output % kernel_per_core) != 0) ? (depth_output % kernel_per_core) : kernel_per_core;

            /*add the first slice (slice 0) of every filter and bias value for this filter*/
            for (k = 0; k < group_size; k++)
            {
                vx_bool read_bias = (s == 0)?vx_true_e:vx_false_e;
                weights = (vx_uint8_ptr)*weight_buffer + (g * kernel_per_core + k) * kernel_x * kernel_x * depth_input * vx_nnu_get_type_size(format);
                bias = (vx_float32_ptr)*bias_buffer + g * kernel_per_core + k;

                for (ky = 0; ky < kernel_x; ky ++)
                {
                    for (kx = 0; kx < kernel_x; kx ++)
                    {
                        if (zrl == 0)
                        {
                            vx_int32 index = kx + kernel_x * (ky + kernel_x * s);

                            vx_int16 weight = (vx_int16)gc_vx_read_bit((vx_uint32_ptr)weights_data, &bit_offset, vx_nnu_get_type_size(format) * 8);

                            memcpy(weights + index * vx_nnu_get_type_size(format), &weight, vx_nnu_get_type_size(format));

                            if (read_bias)
                            {
                                vx_int32 bias_d = gc_vx_read_bit((vx_uint32_ptr)weights_data, &bit_offset, sizeof(vx_float32) * 8);

                                memcpy(bias, &bias_d, sizeof(vx_float32));

                                read_bias = vx_false_e;
                            }

                            if (
                                (kx == (kernel_x - 1))
                                && (ky == (kernel_x -1))
                                && (s == (depth_input - 1))
                                )
                            {
                                bit_offset += offset_size * 8;
                            }

                            if (zrlw)

                            zrl  = gc_vx_read_bit((vx_uint32_ptr)weights_data, &bit_offset, zrlw);
                        }
                        else
                            zrl --;

                        /* zrl not 0, skip*/
                    }
                }

            }
        }/*depth*/
    }

#if NN_DEBUG_DUMP
    {
        static gctUINT32 level = 0;
        vx_char name[1024] = {0};
        FILE* fp = NULL;
        size = depth_input * kernel_x * kernel_x * depth_output * vx_nnu_get_type_size(format);
        sprintf(name, "weight_zrl_decompress_%d_%d_%d_%d_%d.dat", level, depth_input, kernel_x, kernel_x, depth_output);
        fp = fopen(name, "wb");
        fwrite(*weight_buffer, 1, size, fp);
        fclose(fp);

        sprintf(name, "bias_zrl_decompress_%d_%d.dat", level++, depth_output);
        fp = fopen(name, "wb");
        fwrite(*bias_buffer, 1, depth_output * sizeof(vx_float32), fp);
        fclose(fp);
    }
#endif
    return VX_SUCCESS;
}
#else
vx_status vx_nnk_coef_decompress(vx_uint8_ptr src, vx_uint32 type, vx_type_e format, vx_int32 kernel_per_core, vx_int32 width, vx_int32 height, vx_int32 depth_input,
                                 vx_int32 depth_output, vx_int32 kernel_x, vx_int32 output_w, vx_int32 output_h, vx_uint8_ptr* weights_buf, vx_uint8_ptr* bias_buf)
{
    vx_int32 kernel_y = kernel_x;
    vx_uint8_ptr weights_data = src;

    vx_int32 group_count    = gcmALIGN_NP3(depth_output, kernel_per_core);
    vx_uint8_ptr weights    = VX_NULL;
    vx_float32* bias        = VX_NULL;
    vx_uint32 offset_size   = 3;       /*offsetSize is 3 bytes*/
    vx_int32 g = 0, k = 0, s = 0;
    vx_int32 size = depth_input * kernel_x * kernel_y * depth_output * vx_nnu_get_type_size(format);

    *weights_buf = (vx_uint8_ptr)vxAllocateAndZeroMemory(size);
    *bias_buf = (vx_uint8_ptr)vxAllocateAndZeroMemory(depth_output * sizeof(vx_float32));

    group_count = gcmALIGN_NP3(depth_output, kernel_per_core);

    weights_data += 64 + sizeof(vx_uint8) + sizeof(vx_int16);

    /* fill weight value and bias for every group */
    for (g = 0; g < group_count; g++)
    {
        vx_int32 group_size = (g == group_count - 1) && ((depth_output % kernel_per_core) != 0) ? (depth_output % kernel_per_core) : kernel_per_core;

        /*add the first slice (slice 0) of every filter and bias value for this filter*/
        for (k = 0; k < group_size; k++)
        {
            weights = (vx_uint8_ptr)*weights_buf + (g * kernel_per_core + k) * kernel_x * kernel_y * depth_input * vx_nnu_get_type_size(format);
            bias = (vx_float32_ptr)*bias_buf + g * kernel_per_core + k;

            /*add the first point data of the first slice of filter No: k*/
            memcpy(weights, weights_data, vx_nnu_get_type_size(format));
            weights_data += vx_nnu_get_type_size(format);

            /* add bias data, 4bytes/bians */
            memcpy(bias, weights_data, sizeof(vx_float32));
            weights_data += sizeof(vx_float32);

            /* add other point in the first slice(slice 0)*/
            size = (kernel_x * kernel_y - 1) * vx_nnu_get_type_size(format);
            memcpy(weights + vx_nnu_get_type_size(format), weights_data, size);
            weights_data += size;
        }

        for (s = 1; s < depth_input; s++)
        {
            /* add other slices of every filter*/
            for (k = 0; k < group_size; k++)
            {
                /* add one slice data every filter */
                weights = (vx_uint8_ptr)*weights_buf + (g * kernel_per_core + k) * kernel_x * kernel_y * depth_input * vx_nnu_get_type_size(format) + kernel_x * kernel_y * vx_nnu_get_type_size(format) * s;
                memcpy(weights, weights_data, kernel_x * kernel_y * vx_nnu_get_type_size(format));
                weights_data += kernel_x * kernel_y * vx_nnu_get_type_size(format);

                /* add offet behind the last point of the last slice of each filter */
                if (s == depth_input - 1)
                {
                    weights_data += offset_size;
                }
            }
        }
    }


    return VX_SUCCESS;
}
#endif

vx_status vxNNLayersC(vx_node node, vx_array cmd_buf, vx_weights_biases_parameter weights_biases,
                     vx_tensor inputs, vx_uint32 inputOffset,
                     vx_tensor outputs, vx_uint32 outputOffset)
{
    vx_status status = VX_SUCCESS;

    const vx_int32 max_accum_depth = 224;
    gc_nn_inst* insts = (gc_nn_inst*)(cmd_buf->memory.logicals[0]);
    vx_uint8_ptr weights_data = weights_biases->memory.logicals[0];
    vx_uint8_ptr input_data = inputs->tensorBuffer->memory.logicals[0] + inputOffset;
    vx_uint8_ptr output_data = outputs->tensorBuffer->memory.logicals[0] + outputOffset;

    vx_int32 input_w = insts->inImageXsize, input_h = insts->inImageYsize;
    vx_int32 output_w = insts->outImageXsize, output_h = insts->outImageYsize;
    vx_int32 kernel_x = insts->kernelXYsize, kernel_y = kernel_x;
    vx_int32 depth_output = insts->outImageZsize, depth_input = insts->kernelZsize;

    vx_int32 q = 0;

    vx_int32 kernel_per_core = insts->kernelsPerCore;/*14;*/
    vx_type_e format = VX_TYPE_FLOAT16;
    vx_int32 tile_x_size = vx_nnu_get_tile_x_size(max_accum_depth, input_w, output_w), tile_y_size = vx_nnu_get_tile_y_size(max_accum_depth, tile_x_size, output_h);
    vx_int32 tile_x_group = gcmALIGN_NP3(output_w, tile_x_size), tile_y_group = gcmALIGN_NP3(output_h, tile_y_size);

    vx_int32 pad = (insts->inImageXoffset != 0)?-(vx_int32)(0xfffffff8 | insts->inImageXoffset):0;
    vx_int32 pool_output_w = output_w, pool_output_h = output_h;
#if NN_DEBUG_LOCAL_COUNT
    vx_int32 debug_loop_count = 0;
#endif

    vx_float32_ptr buffer = (vx_float32_ptr)vxAllocateAndZeroMemory(sizeof(vx_float32) * depth_output * output_w * output_h);
    vx_float32_ptr buffer2 = (vx_float32_ptr)vxAllocateAndZeroMemory(sizeof(vx_float32) * depth_output * output_w * output_h);
    vx_float32_ptr accum_buf = (vx_float32_ptr)buffer;
    vx_float32_ptr weights_buffer, bias_buffer;
    vx_int32 interleave_mode = vx_nnu_get_interleave(64, tile_x_size);

    if (weights_biases->use_fc_accel)
    {
        input_data = weights_biases->memory.logicals[0];
        weights_data = inputs->tensorBuffer->memory.logicals[0] + inputOffset;
    }

#if NN_DEBUG_CHECK_KERNEL_PER_CORE
    {
        vx_int32 ideal_kernel_per_core = vx_nnu_get_kernel_per_core(64, interleave_mode, tile_y_size, depth_output);

        if (kernel_per_core != ideal_kernel_per_core)
            gcmPRINT("%s[%d] kernel per core(%d, ideal value = %d) error!\n", __FILE__, __LINE__, kernel_per_core, ideal_kernel_per_core);
    }
#endif

    tile_x_group = gcmALIGN_NP3(output_w, tile_x_size);
    tile_y_group = gcmALIGN_NP3(output_h, tile_y_size);


    if (insts->layerType == 1)/*fc*/
    {
        kernel_x = input_w;
        kernel_y = input_h;
        memset(buffer, 0, sizeof(vx_float32) * depth_output * output_w * output_h);
    }
    else
    {
        memset(buffer, 0, sizeof(vx_float32) * depth_output * output_w * output_h);
    }

#if NN_KERNEL_ZRL
    vx_nnk_coef_decompress_zrl(weights_data, format, kernel_per_core, input_w, output_w, output_h, depth_output, kernel_x, depth_input, (vx_uint8_ptr*)&weights_buffer, (vx_uint8_ptr*)&bias_buffer);
#else
    vx_nnk_coef_decompress(weights_data, 0, format, kernel_per_core, input_w, input_h, depth_input, depth_output, kernel_x, output_w, output_h, (vx_uint8_ptr*)&weights_buffer, (vx_uint8_ptr*)&bias_buffer);
#endif

#if NN_DEBUG_INFO
    gcmPRINT("    [REAL]tile_x = %d, tile_y = %d, interleave = %d, kernel_per_core = %d\n\n", tile_x_size, tile_y_size, interleave_mode, kernel_per_core);
#endif

    Image3DConvJobs(interleave_mode, kernel_per_core, depth_output, tile_y_group,
              accum_buf, (vx_uint8_ptr)bias_buffer, (vx_uint8_ptr)weights_buffer, format, input_data,
              tile_y_size, depth_input, output_w, output_h, input_w, input_h, kernel_x,
              kernel_y, pad, tile_x_group, tile_x_size
#if NN_DEBUG_LOCAL_COUNT
, &debug_loop_count
#endif
              );
#if NN_DEBUG_DUMP
    {
        FILE * fp = NULL;
        char name[1024] = {0};
        static int index = 0;
        sprintf(name, "tile_conv_input_f16_%d_%d_%d_%d.dat", index, depth_input, input_w, input_h);
        fp = fopen(name, "wb");
        fwrite(input_data, vx_nnu_get_type_size(format), depth_input * input_w * input_h, fp);
        fclose(fp);

        sprintf(name, "tile_conv_output_f32_%d_%d_%d_%d.dat", index, depth_output, output_w, output_h);
        fp = fopen(name, "wb");
        fwrite(accum_buf, vx_nnu_get_type_size(VX_TYPE_FLOAT32), depth_output * output_w * output_h, fp);
        fclose(fp);

        index++;
    }
#endif
    for (q = 0; q < depth_output * output_w * output_h; q++)
    {
        vx_nnu_save_date(format, q, vx_nnu_get_date(VX_TYPE_FLOAT32, q, (vx_uint8_ptr)accum_buf), buffer2);
    }
#if NN_DEBUG_LOCAL_COUNT
    gcmPRINT("debug_loop_count = %d\n", debug_loop_count);
#endif

    if (insts->relu)
        vx_nnk_relu_NN_LAYER_Cpu((vx_uint8_ptr)buffer2, format, depth_output * output_w * output_h, (vx_uint8_ptr)buffer);
    else
        memcpy((vx_uint8_ptr)buffer, (vx_uint8_ptr)buffer2, depth_output * output_w * output_h * vx_nnu_get_type_size(format));

#if NN_DEBUG_DUMP
    {
        FILE * fp = NULL;
        char name[1024] = {0};
        static int index4 = 0;
        sprintf(name, "layer_output_relu_f16_%d_%d_%d_%d.dat", index4, depth_output, output_w, output_h);
        fp = fopen(name, "wb");
        fwrite(buffer, vx_nnu_get_type_size(format), depth_output * output_w * output_h, fp);
        fclose(fp);

        index4++;
    }
#endif

    if (insts->pooling == 0)
    {
        for (q = 0; q < depth_output * output_w * output_h; q++)
        {
            vx_nnu_save_date(format, q, vx_nnu_get_date(format, q, (vx_uint8_ptr)buffer), output_data);
        }
    }
    else
    {
        vx_nnk_pool_nn_layer_cpu((vx_uint8_ptr)buffer, insts->pooling, format, output_w, output_h, depth_output, &pool_output_w, &pool_output_h, insts->pooling?2:1, insts->poolingXYsize?3:2, 0, (vx_uint8_ptr)buffer2);

        for (q = 0; q < depth_output * pool_output_w * pool_output_h; q++)
        {
            vx_nnu_save_date(format, q, vx_nnu_get_date(format, q, (vx_uint8_ptr)buffer2), output_data);
        }
    }

#if NN_DEBUG_DUMP
    {
        FILE * fp = NULL;
        char name[1024] = {0};
        static int index5 = 0;

        sprintf(name, "layer_output_f16_%d_%d_%d_%d.dat", index5, depth_output, pool_output_w, pool_output_h);
        fp = fopen(name, "wb");
        fwrite(output_data, vx_nnu_get_type_size(format), depth_output * pool_output_w * pool_output_h, fp);
        fclose(fp);

        index5++;
    }
#endif

    vxFree(buffer);
    vxFree(buffer2);

    if (weights_buffer) vxFree(weights_buffer);
    if (bias_buffer) vxFree(bias_buffer);

    return status;
}

#endif

