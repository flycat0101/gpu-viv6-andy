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


#include <gc_vxk_common.h>

/* Float version. */
/* If xsize * ysize is not multiple of 4, split the task into 2 task.
    execute the second one first, and then the first one. */

/* Half version. */
/* Use EVIS image_load/image_store to handle out-of-bound pixels. */

char * vxgLRNGPUSource =
"__kernel void lrn_n5(\n\
    read_only image2d_t input,\n\
    write_only image2d_t output,\n\
    float k,\n\
    float alpha,\n\
    float beta,\n\
    uint width,\n\
    uint depthMiuns2\n\
    )\n\
{\n\
    int2 coord;\n\
    float4 inData0, inData1, inData2, inData3, inData4;\n\
    float4 outData;\n\
    float4 sum;\n\
    coord.x = get_global_id(0);\n\
    if (coord.x < width) {\n\
        const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE\n\
                                | CLK_ADDRESS_BORDER\n\
                                | CLK_FILTER_NEAREST;\n\
        inData0 = 0;\n\
        inData1 = 0;\n\
        coord.y = 0;\n\
        inData2 = read_imagef(input, sampler, coord);\n\
        outData = inData0 * inData0;\n\
        sum = mad(outData, alpha, k);\n\
        coord.y = 1;\n\
        inData3 = read_imagef(input, sampler, coord);\n\
        outData = inData1 * inData1;\n\
        sum = mad(square, alpha, sum);\n\
        for (coord.y = 0; coord.y < depthMinus2; coord.y++)\n\
        {\n\
            inData4 = read_imagef(input, sampler, coord + (0, 2));\n\
            outData = inData4 * inData4;\n\
            sum = mad(outData, alpha, sum);\n\
            outData = inData2 * native_powr(sum, -beta);\n\
            write_imagef(output, coord, outData);\n\
            outData = inData0 * inData0;\n\
            sum = mad(outData, -alpha, sum);\n\
            inData0 = inData1;\n\
            inData1 = inData2;\n\
            inData2 = inData3;\n\
            inData3 = inData4;\n\
        }\n\
        outData = inData2 * native_powr(sum, -beta);\n\
        write_imagef(output, coord, outData);\n\
        outData = inData0 * inData0;\n\
        sum = mad(outData, -alpha, sum);\n\
        outData = inData1 * native_powr(sum, -beta);\n\
        coord.y++;\n\
        write_imagef(output, coord, outData);\n\
    }\n\
}\n";

vx_status vxLocalResponseNormalization(
    vx_node node,
    vx_array input,
    vx_uint32 kernelSize,
    vx_float32 k,
    vx_float32 alpha,
    vx_float32 beta,
    vx_uint32 xSize,
    vx_uint32 ySize,
    vx_uint32 zSize,
    vx_uint8 dataType,
    vx_array output)
{
    vx_status status = VX_SUCCESS;

    return status;
}

