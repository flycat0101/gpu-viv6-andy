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


#include <gc_vxk_common.h>

//#define COMPARE_TO_REF
//#define DUMP_NN

#if NN_LAYER_C
vx_status vxNNLayersC(vx_node node, vx_array cmd_buf, vx_weights_biases_parameter weights_biases,
                     vx_tensor inputs, vx_uint32 inputOffset,
                     vx_tensor outputs, vx_uint32 outputOffset);
#else

#ifdef COMPARE_TO_REF
static vx_float32 fp16_to_float32(const vx_int16 in) {
        vx_int32 t1;
        vx_int32 t2;
        vx_int32 t3;
        vx_float32 out;

        t1 = in & 0x7fff;                       // Non-sign bits
        t2 = in & 0x8000;                       // Sign bit
        t3 = in & 0x7c00;                       // Exponent

        t1 <<= 13;                              // Align mantissa on MSB
        t2 <<= 16;                              // Shift sign bit into position

        t1 += 0x38000000;                       // Adjust bias

        t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero

        t1 |= t2;                               // Re-insert sign bit

        *((uint32_t*)&out) = t1;

        return out;
    };
#endif

#endif

#ifdef DUMP_NN
FILE* dumpFILE = VX_NULL;
void init_nn_dump_file(char* prefix, gctUINT32 layer, gctUINT32 index)
{
    char dump_file[128];
    gctUINT offset = 0;

    gcmVERIFY_OK(gcoOS_PrintStrSafe(
        dump_file,
        gcmSIZEOF(dump_file),
        &offset,
        "dump_%s_pid-%d_%s_layer%02d_index%d.log",
        prefix,
        gcoOS_GetCurrentProcessID(),
        gcdDUMP_KEY,
        layer, index));

    dumpFILE = fopen(dump_file, "a+");
}

void deinit_nn_dump_file()
{
    if (dumpFILE != VX_NULL)
    {
        fclose(dumpFILE);
        dumpFILE = VX_NULL;
    }
}

void dump_trigger_command(
    gceVX_ACCELERATOR_TYPE Type,
    gctUINT32 Physical
    )
{
    static gctUINT32 cmd[] = {0x08010428, 0x0801042E};
    if (dumpFILE == VX_NULL) return;

    fprintf(dumpFILE, "@[command 0x00000001 0x00000030\n");
    fprintf(dumpFILE, "  0x%08X 0x%08X 0x08010429 0x00000000\n", cmd[Type], Physical);
    fprintf(dumpFILE, "  0x08010E03 0x00000C23 0x08010E03 0x00000C23\n");
    fprintf(dumpFILE, "  0x08010594 0x00000001 0x08010E03 0x00000C23\n");
    fprintf(dumpFILE, "] -- command\n");
    fprintf(dumpFILE, "@[commit]\n");
    fprintf(dumpFILE, "@[stall]\n");
}

void dump_buffer(
    gctSTRING Tag,
    gctUINT32 Physical,
    gctPOINTER Logical,
    gctSIZE_T Offset,
    gctSIZE_T Bytes
)
{
    gctUINT32_PTR ptr = (gctUINT32_PTR) Logical + (Offset >> 2);
    gctSIZE_T bytes   = gcmALIGN(Bytes + (Offset & 3), 4);

    if (dumpFILE == VX_NULL) return;

    fprintf(dumpFILE, "@[%s 0x%08X 0x%08X\n", Tag, Physical + (Offset & ~3), bytes);

    while (bytes >= 16)
    {
        fprintf(dumpFILE, "  0x%08X 0x%08X 0x%08X 0x%08X\n", ptr[0], ptr[1], ptr[2], ptr[3]);
        ptr   += 4;
        bytes -= 16;
    }

    switch (bytes)
    {
        case 12:
            fprintf(dumpFILE, "  0x%08X 0x%08X 0x%08X\n", ptr[0], ptr[1], ptr[2]);
            break;

        case 8:
            fprintf(dumpFILE, "  0x%08X 0x%08X\n", ptr[0], ptr[1]);
            break;

        case 4:
            fprintf(dumpFILE, "  0x%08X\n", ptr[0]);
            break;
    }

    fprintf(dumpFILE, "] -- %s\n", Tag);
}
#endif

static int layerIndex = 0;

vx_status vxNNExecute(vx_node node,
                      vx_array cmd_buf, vx_uint32 cmdOffset,
                      vx_weights_biases_parameter weights_biases, vx_uint32 wbOffset,
                      vx_tensor inputs, vx_uint32 inputOffset,
                      vx_tensor outputs, vx_uint32 outputOffset)
{

#if NN_LAYER_C
    return vxNNLayersC(node, cmd_buf, weights_biases, inputs, inputOffset, outputs, outputOffset);
#else

    vx_status status = VX_SUCCESS;

    vx_uint32 *nnCmdBufferPtr = NULL;

    gctUINT32 nnCmdBufferAddress = (gctUINT32)cmd_buf->memory.physicals[0] + cmdOffset;
    gctUINT32 inputKernelAddress = (gctUINT32)weights_biases->memory.physicals[0] + wbOffset;
    gctUINT32 inputBufferAddress = (gctUINT32)inputs->tensorBuffer->memory.physicals[0] + inputOffset;
    gctUINT32 outputBufferAddress = (gctUINT32)outputs->tensorBuffer->memory.physicals[0] + outputOffset;

#ifdef DUMP_NN
    vx_uint32 inputsSize;
    vx_uint32 outputsSizeDump;
    vx_uint32 weightBiasSize = 0;
#endif

#ifdef COMPARE_TO_REF
    FILE *outputFile = NULL;
    vx_float32 *pfNN_out, *pfRef_out;
    vx_float32 fDelta, fMax_delta, fPortion_BigDelta;
    vx_uint32 i, count_big_delta, offset_max_delta;
    vx_uint32 outputsElementCount, outputsSize;
    /**/
    vx_char *fileName[] = {"layer0//out1_1x96x27x27.dat",
                           "layer1//out1_1x256x13x13.dat",
                           "layer1//out1_1x256x13x13.dat",
                           "layer2//out1_1x384x13x13.dat",
                           "layer3//out1_1x384x13x13.dat",
                           "layer3//out1_1x384x13x13.dat",
                           "layer4//out1_1x256x6x6.dat",
                           "layer4//out1_1x256x6x6.dat",
                           "layer5//out1_1x4096.dat",
                           "layer6//out1_1x4096.dat",
                           "layer7//out1_1x1000.dat"};

    vx_char fullFileName[256] = {'\0'};
    gctBOOL doCompare[] = {gcvTRUE,
                           gcvFALSE,
                           gcvTRUE,
                           gcvTRUE,
                           gcvFALSE,
                           gcvTRUE,
                           gcvFALSE,
                           gcvTRUE,
                           gcvTRUE,
                           gcvTRUE,
                           gcvTRUE};
#endif
    static vx_uint32 index = 0;

    nnCmdBufferPtr = (vx_uint32*)(cmd_buf->memory.logicals[0] + cmdOffset);

    nnCmdBufferPtr += 5;

    if (gcoHAL_IsFeatureAvailable1(gcvNULL, gcvFEATURE_VIP_V7))
    {
        *nnCmdBufferPtr |= inputKernelAddress >> 6; /* high 6 bits are for other usage*/
        nnCmdBufferPtr++;
    }
    else
    {
        *nnCmdBufferPtr++ = inputKernelAddress >> 6; /* make sure high 6 bits are all 0 */
    }

    *nnCmdBufferPtr++ = inputBufferAddress;

    *nnCmdBufferPtr++ = outputBufferAddress;

#ifdef DUMP_NN
    init_nn_dump_file("NN", layerIndex, index);

    if (weights_biases->use_fc_accel)
    {
        vx_uint32 weightSize = (vx_uint32)vxDataType_GetSize(weights_biases->weights_data_format);
        vx_uint32 nnCores = node->base.context->nnConfig.nnCoreCount;
        vx_uint32_ptr header = (vx_uint32_ptr)(weights_biases->memory.logicals[0] + wbOffset);
        inputsSize = weights_biases->zgroup_array[index] * weights_biases->input_nonzero_count * weightSize;
        while (nnCores--)
        {
            weightBiasSize += *header++;
        };
        weightBiasSize += 64;
    }
    else
    {
        vxoTensor_GetTensorSize(inputs, &inputsSize);
        weightBiasSize = weights_biases->memory_size - weights_biases->memroy_head_offset;
    }

    gcoVX_Flush(gcvTRUE);

    dump_buffer("memory",
                cmd_buf->memory.physicals[0] + cmdOffset,
                (gctPOINTER)(cmd_buf->memory.logicals[0] + cmdOffset),
                0,
                cmd_buf->capacity * cmd_buf->itemSize);

    dump_buffer("memory",
                inputs->tensorBuffer->memory.physicals[0] + inputOffset,
                (gctPOINTER)(inputs->tensorBuffer->memory.logicals[0] + inputOffset),
                0,
                inputsSize);

    dump_buffer("memory",
                weights_biases->memory.physicals[0] + wbOffset,
                (gctPOINTER)(weights_biases->memory.logicals[0] + wbOffset),
                0,
                weightBiasSize);

    dump_trigger_command(gcvVX_ACCELERATOR_NN, (gctUINT32)nnCmdBufferAddress);

    /* close dump file first to avoid incomplete data when hang */
    deinit_nn_dump_file();
#endif

    status = gcfVX_Accel((gctUINT32)nnCmdBufferAddress, gcvVX_ACCELERATOR_NN, node->cnnTriggerEventID, node->forceWaitForEvent);

    if (node->base.context->options.enableCNNPerf)
    {
        gcoVX_Flush(gcvTRUE);
    }

#ifdef DUMP_NN
    gcoVX_Flush(gcvTRUE);

    /* reopen again */
    init_nn_dump_file("NN", layerIndex, index);

    vxoTensor_GetTensorSize(outputs, &outputsSizeDump);
    dump_buffer("verify",
                outputs->tensorBuffer->memory.physicals[0] + outputOffset,
                (gctPOINTER)(outputs->tensorBuffer->memory.logicals[0] + outputOffset),
                0,
                outputsSizeDump > 16 ?
                outputsSizeDump : 16);

    deinit_nn_dump_file();
#endif

#ifdef COMPARE_TO_REF
    if (doCompare[layerIndex])
    {
        outputsElementCount = (vx_uint32)vxoMemory_ComputeElementCount(&outputs->tensorBuffer->memory, 0);
        outputsSize = (vx_uint32)vxoMemory_ComputeSize(&outputs->tensorBuffer->memory, 0);

        pfNN_out =  (vx_float32*)malloc(outputsElementCount * sizeof(vx_float32));
        pfRef_out = (vx_float32*)malloc(outputsElementCount * sizeof(vx_float32));

        outputFile = fopen(fileName[layerIndex], "rb");            /* compare to ref output file of each layer */
        if (outputFile == NULL)
        {
            gcmPRINT("can't find file %s", fullFileName);
            return VX_ERROR_INVALID_PARAMETERS;
        }

        if(fread(pfRef_out, 1, outputsElementCount*sizeof(vx_float32), outputFile) != outputsElementCount*sizeof(vx_float32))
        {
            gcmPRINT("fread failure");
            return VX_ERROR_INVALID_PARAMETERS;
        }
        fclose(outputFile);

        fMax_delta =0.0f;
        count_big_delta = 0;
        offset_max_delta = 0;
        for (i = 0; i < outputsElementCount; i++)
        {
            pfNN_out[i] = fp16_to_float32(*((vx_int16*) outputs->tensorBuffer->memory.logicals[0] + i));
            if (pfRef_out[i] != 0.0f)
            {
                fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfRef_out[i]);
            }
            else if (pfNN_out[i] != 0.0f)
            {
                fDelta = (vx_float32)fabs((pfRef_out[i] - pfNN_out[i]) / pfNN_out[i]);
            }
            else
            {
                continue;
            }

            if (fDelta > 0.01f)
            {
                count_big_delta++;
            }
            if (fDelta > fMax_delta)
            {
                fMax_delta = fDelta;
                offset_max_delta = i;
            }
        }

        fPortion_BigDelta = (float)count_big_delta/(float)outputsElementCount;
        printf("Portion_BigDelta = %10.7f, Max delta = %10.6f, count_big_delta= %d \n", fPortion_BigDelta, fMax_delta, count_big_delta);
        free(pfRef_out);
        free(pfNN_out);
    }

#endif
    index++;
    if (index == weights_biases->zgroup_num)
    {
        index = 0;
        layerIndex++;
    }

    return status;

#endif
}

vx_status vxTPExecute(vx_node node, vx_uint32 op_num,
                      vx_array cmd_buf, vx_uint32 cmdOffset, vx_uint32 cmdSize,
                      vx_weights_biases_parameter weights_biases, vx_uint32 wbOffset, vx_uint32 wbSize,
                      vx_tensor inputs, vx_uint32 inputOffset, vx_uint32 inputSize,
                      vx_tensor outputs, vx_uint32 outputOffset, vx_uint32 outputSize,
                      vx_array buffer)
{
    vx_status status = VX_SUCCESS;
    gctUINT32 nnCmdBufferAddress = (gctUINT32)cmd_buf->memory.physicals[0] + cmdOffset;
    static gctUINT32 index = 0;

#ifdef DUMP_NN
    vx_uint32 outputsSizeDump;
    vx_uint32 inputsSize = vxoMemory_ComputeSize(&inputs->tensorBuffer->memory, 0);

    init_nn_dump_file("TP", layerIndex, 0);

    dump_buffer("memory",
                nnCmdBufferAddress,
                (gctPOINTER)(cmd_buf->memory.logicals[0] + cmdOffset),
                0,
                cmdSize);

    dump_buffer("memory",
                inputs->tensorBuffer->memory.physicals[0] + inputOffset,
                (gctPOINTER)(inputs->tensorBuffer->memory.logicals[0] + inputOffset),
                0,
                inputsSize);

    if (weights_biases != gcvNULL)
    {
        dump_buffer("memory",
                    weights_biases->memory.physicals[0] + wbOffset,
                    (gctPOINTER)(weights_biases->memory.logicals[0] + wbOffset),
                    0,
                    wbSize);
    }
    else if (buffer != gcvNULL)
    {
        dump_buffer("table",
                    buffer->memory.physicals[0],
                    (gctPOINTER)buffer->memory.logicals[0],
                    0,
                    buffer->itemCount * buffer->itemSize);
    }

    dump_trigger_command(gcvVX_ACCELERATOR_TP, (gctUINT32)nnCmdBufferAddress);

    /* close dump file first to avoid incomplete data when hang */
    deinit_nn_dump_file();
#endif

    status = gcfVX_Accel((gctUINT32)nnCmdBufferAddress, gcvVX_ACCELERATOR_TP, node->cnnTriggerEventID, node->forceWaitForEvent);

#ifdef DUMP_NN
    gcoVX_Flush(gcvTRUE);

    /* reopen again */
    init_nn_dump_file("TP", layerIndex, 0);

    vxoTensor_GetTensorSize(outputs, &outputsSizeDump);

    dump_buffer("verify",
                outputs->tensorBuffer->memory.physicals[0] + outputOffset,
                (gctPOINTER)(outputs->tensorBuffer->memory.logicals[0] + outputOffset),
                0,
                outputSize);

    deinit_nn_dump_file();
#endif

    if (++index == op_num)
    {
        layerIndex++;
        index = 0;
    }

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
        printf("Not support format :%d\n", format);
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
            printf("Not support format :%d\n", format);
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
        printf("Not support format :%d\n", format);
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

    *weight_buffer = (vx_uint8_ptr)malloc(size);
    *bias_buffer = (vx_uint8_ptr)malloc(depth_output * sizeof(vx_float32));

    memset(*weight_buffer, 0, size);

    memset(*bias_buffer, 0, depth_output * sizeof(vx_float32));

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

    *weights_buf = (vx_uint8_ptr)malloc(size);
    *bias_buf = (vx_uint8_ptr)malloc(depth_output * sizeof(vx_float32));

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

    vx_float32_ptr buffer = (vx_float32_ptr)malloc(sizeof(vx_float32) * depth_output * output_w * output_h);
    vx_float32_ptr buffer2 = (vx_float32_ptr)malloc(sizeof(vx_float32) * depth_output * output_w * output_h);
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
            printf("%s[%d] kernel per core(%d, ideal value = %d) error!\n", __FILE__, __LINE__, kernel_per_core, ideal_kernel_per_core);
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
    printf("    [REAL]tile_x = %d, tile_y = %d, interleave = %d, kernel_per_core = %d\n\n", tile_x_size, tile_y_size, interleave_mode, kernel_per_core);
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
    printf("debug_loop_count = %d\n", debug_loop_count);
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

    free(buffer);
    free(buffer2);

    if (weights_buffer)free(weights_buffer);
    if (bias_buffer)free(bias_buffer);

    return status;
}

#endif

