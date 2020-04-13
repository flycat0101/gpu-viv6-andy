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


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "nnArchPerf.h"
#include "nnArchPerfMisc.h"

// fix me
static unsigned int g_xydp_x;
static unsigned int g_xydp_y;

static unsigned int Merge2DTileEnabled = 0;
static unsigned int SpecifiedBurstSize = false;

#define MAX_INTERLEAVE_CH      1

#define NOT_USE_NEW_VB
/* temp definition */
#define SPECIFIED_DDR_BW_LIMIT_BY_BURST 0
#define  DDR_READ_BW_IN_BYTE_PER_CYCLE_64B              16
#define  DDR_READ_BW_IN_BYTE_PER_CYCLE_128B             16
#define  DDR_READ_BW_IN_BYTE_PER_CYCLE_256B             16
#define  DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_64B         16
#define  DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_128B        16
#define  DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_256B        16
#define  DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_64B      16
#define  DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_128B     16
#define  DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_256B     16

static unsigned int SeparateBurstcountBySize(double *BurstCount_64B, double *BurstCount_128B, /*double BurstCount_256B,*/
                                             unsigned int hgap, unsigned int vgap, int tile_x, int tile_y,
                                             unsigned int x, unsigned int y,unsigned int z,
                                             int stride,int slice, unsigned int zdp, double factor,unsigned int NonTransposed_1x1,
                                             double *output_BurstCount_64B_Standalone, double *output_BurstCount_128B_Standalone, double *output_BurstCount_256B_Standalone);

static double WriteBandWidth(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int x,
    unsigned int y,
    unsigned int outZ,
    unsigned int data_size,
    double image_compress_ratio,
    unsigned int usc_cache_size,
    unsigned int pooling_stride,
    unsigned int outimage_stride,
    unsigned int outimage_slice,
    unsigned int is_nn_write_without_usc,
    unsigned int dst_buf,
    unsigned int burst_size
    );

static double Kernel4DSingleReadBW(
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int z,
    double       coef_compress_ratio
    );

static unsigned int _calcInImageInterleaveMode(
    unsigned int x,
    unsigned int max_tile_size,
    unsigned int kernel_xy,
    unsigned int         vip7_fp16,
    unsigned int         interleave8
    )
{
    // todo: vip v8 kernelX and kernelYsize are not the same
    if (vip7_fp16)
    {
        if ((max_tile_size + 8) / 4 < (x + kernel_xy - 1))
        {
            return 1;
        }
        else if ((max_tile_size + 8) / 8 < (x + kernel_xy - 1) || !interleave8)
        {
            return 2;
        }
        return 4;
    }
    else
    {
        if ((max_tile_size + 8) / 2 < (x + kernel_xy - 1))
        {
            return 1;
        }
        else if ((max_tile_size + 8) / 4 < (x + kernel_xy - 1))
        {
            return 2;
        }
        else if ((max_tile_size + 8) / 8 < (x + kernel_xy - 1) || !interleave8)
        {
            return 4;
        }
        return 8;
    }
}

static unsigned int _calcOutImageInterleaveMode(
    unsigned int x,
    unsigned int max_tile_size,
    unsigned int         vip7_fp16,
    unsigned int         interleave8
    )
{
    if (vip7_fp16)
    {
        return (x > (max_tile_size / 4)) ? 1
            : ((x > (max_tile_size / 8)) || !interleave8) ? 2
            : 4;
    }
    else
    {
        return (x > (max_tile_size / 2)) ? 1
            : (x > (max_tile_size / 4)) ? 2
            : ((x > (max_tile_size / 8)) || !interleave8) ? 4
            : 8;
    }
}

unsigned int _calcImageInterleaveMode(
    unsigned int x,
    unsigned int mad_per_core,
    unsigned int kxy,
    unsigned int         vip7_fp16,
    unsigned int         interleave8)
{
    /*mad_per_core = 64;*/
    return min(_calcOutImageInterleaveMode(x, mad_per_core, vip7_fp16, interleave8),
               _calcInImageInterleaveMode(x, mad_per_core, kxy, vip7_fp16, interleave8));
}
static double _calcPartialAlignedBW(
    unsigned int size,
    unsigned int PPC,
    unsigned int inc,
    unsigned int line_length,
    unsigned int line_phases,
    unsigned int base_addr,
    int          xoffset
    )
{
    unsigned int lineCount = 0, stepCount = 0, i, j;
    double partialAlignedBW;
    for (j = 0; j < line_phases; j++)
    {
        unsigned int accum = ((base_addr + (j * PPC / line_phases)) % PPC) + xoffset;
        for (i = 0; i < line_length - size + inc - 1; i += inc)
        {
            unsigned int Adj_size = min(size, line_length - i);
            stepCount++;

            unsigned int trsp_interleave_ch = pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_in;
            if (trsp_interleave_ch == 1)
            {
                if ((accum + Adj_size) > (2 * PPC))
                {
                    lineCount += 3;
                }
                else if ((accum + Adj_size) > PPC)
                {
                    lineCount += 2;
                }
                else
                {
                    lineCount += 1;
                }
            }
            else
            {
                lineCount += (unsigned int)ceilf( (float) (accum + Adj_size) / PPC);
            }

            accum = (accum + inc) % PPC;
        }
    }

    partialAlignedBW = ((double)lineCount / stepCount) * PPC;
    return partialAlignedBW;
}

static double UnalignedBW(
    double size,
    double PPC
    )
{
    if (size == 0)
    {
         return 0;
    }

    double ret = ((unsigned int)((size - 1) / PPC) + 1 +
        (((unsigned int)size - 1) % (unsigned int)PPC ) / (double)PPC) * PPC;

    return ret;
}

unsigned int gcd(unsigned int a, unsigned int b)
{
    unsigned int M = max(a, b);
    unsigned int N = min(a, b);
    unsigned int r = 0;
    do
    {
        r = M % N;
        M = N;
        N = r;
    }while (r > 0);

    return M;
}

unsigned int gcd_multi(unsigned int *arr, unsigned int arrSize)
{
    unsigned int res = arr[0];
    for (unsigned int i = 0; i!= arrSize-1; i++)
    {
        res = gcd(res, arr[i+1]);
    }

    return res;
}

float macUtility(
    unsigned int MAC_PER_LAYER,
    /*unsigned int EFFECTIVE_MAC_PER_LAYER,*/
    unsigned int cycleCount,
    bool bCoefInt16
    )
{
    unsigned int MacPerCycleNN = pContext->p_hwInfo->INT8_MAC_PER_CYCLE_NN;

    if ((pContext->pInPerfParams->xydp_x == 0 && pContext->pInPerfParams->xydp_y == 0) && bCoefInt16)
    {
        MacPerCycleNN = MacPerCycleNN / 4;
    }

    unsigned int x = pContext->pInPerfParams->cmdInfo.outImageXsize;
    unsigned int y = pContext->pInPerfParams->cmdInfo.outImageYsize;
    unsigned int z = pContext->pInPerfParams->cmdInfo.outImageZsize;
    unsigned int kx   = pContext->pInPerfParams->cmdInfo.kernelXsize;
    unsigned int ky   = pContext->pInPerfParams->cmdInfo.kernelYsize;
    unsigned int kz   = pContext->pInPerfParams->cmdInfo.kernelZsize;

    bool isDW         = pContext->pInPerfParams->cmdInfo.is_depth_wise;
    bool isDWMerge    = pContext->pInPerfParams->cmdInfo.is_depth_wise_merge;

    if (isDW)
    {
        MAC_PER_LAYER = x * y * z * kx * ky;
    }
    else if (isDWMerge)
    {
        MAC_PER_LAYER = x * y * z * kz + x * y * kz * kx * ky;
    }
    else
    {
        MAC_PER_LAYER = x * y * z * kx * ky * kz;
    }

    float ROW_MAD_UTIL = (float)MAC_PER_LAYER / (cycleCount * MacPerCycleNN);
    //float ROW_EFFECTIVE_MAC_UTIL = ;

    // to do: model mac unitlization
    return ROW_MAD_UTIL;
}

static double UnalignedBW2(unsigned int size, unsigned int stride, unsigned int slice, unsigned int PPC)
{
    unsigned int div = 0;
    unsigned int arr[4] = {0};
    if(size == 0)
        return 0;

    arr[0] = size;
    arr[1] = stride;
    arr[2] = slice;
    arr[3] = PPC;
    div = gcd_multi(arr,4);

    assert(div > 0);

    size /= div;
    stride /= div;
    PPC /= div;

    return UnalignedBW(size, PPC) * div;
}

/* UnalignedNMBC for Burst Size, change name in shelve CL221226 */

static double UnalignedNMBC_stride_aligned(unsigned int size, unsigned int stride, unsigned int PPC, unsigned int ext)
{
    unsigned int res = 0, i = 0;
    unsigned int stride_act = 0;
    unsigned int addr = 0, num_of_events = 0;
    stride_act = stride - (stride % size);
    num_of_events = (unsigned int)ceilf((float)stride_act / PPC);

    while(i < num_of_events)
    {
        if ((addr % size) <= (size + ext - PPC))
            res = res + 1;

        addr = addr + PPC;
        i++;
    }

    return (double)res / (stride / size);
}

/* return should be int cause it may return -1 */
static double UnalignedNMBC(unsigned int size, unsigned int stride, unsigned int slice, unsigned int PPC, int ext)
{
    double UnalignedNMBC = 0;
    int div = 0;
    unsigned int arr[8] = {0};
    float ext_perc = ((float)ext + 1) / PPC;
    if((size + ext) < PPC)
    {
        UnalignedNMBC = 0;
    }
    else
    {
        arr[0] = size;
        arr[1] = stride;
        arr[2] = slice;
        arr[3] = PPC;
        div = gcd_multi(arr,4);
        size = size / div;
        PPC = PPC / div;
        stride = stride / div;
        slice = slice / div;
        if(ext > 0)
            ext = max((int)(PPC * ext_perc - 1), 0);

        if(stride % PPC == 0 && (slice % PPC == 0))
            UnalignedNMBC = UnalignedNMBC_stride_aligned(size, stride, PPC, ext);
        else
            UnalignedNMBC = (int)((size + ext + 1) / PPC) + ((size + ext + 1) % PPC) /(double)PPC -1;
    }
    return UnalignedNMBC;
}

static double UnalgiendNMBC_Extended(int size, unsigned int PPC, /*unsigned int hgap,*/ unsigned int vgap,
                                           unsigned int tile_x, unsigned int tile_y, unsigned int x, unsigned int y, unsigned int z,
                                           int image_stride, int image_slice)
{
    double res = 0, tile_y_tail = 0, res_tail = 0, tile_x_tail = 0;
    int ext = (int)(PPC / 2 - 1);
    if(size >= (image_slice -ext))
        res = UnalignedNMBC(image_slice * z, image_slice * z, image_slice * z, PPC, 0);
    else if((image_slice - ext) > size && size >= image_stride)
    {
        tile_y_tail = y % tile_y;
        res_tail = UnalignedNMBC((unsigned int)tile_y_tail * image_stride, image_slice, image_slice, PPC, ext) * z;
        res = UnalignedNMBC(image_slice - vgap, image_slice, image_slice, PPC, ext) * (y / tile_y) * z + res_tail;
    }
    else if(image_stride > size && size >= (image_stride - ext))
    {
        tile_y_tail = y % tile_y;
        res_tail = UnalignedNMBC((unsigned int)tile_y_tail * image_stride, image_slice, image_slice, PPC, 0) * z;
        res = UnalignedNMBC(tile_y * image_stride, image_slice, image_slice, PPC,0) * (y / tile_y) * z + res_tail;
    }
    else
    {
        tile_x_tail = x % tile_x;
        res_tail = UnalignedNMBC((unsigned int)tile_x_tail, image_stride, image_slice, PPC, ext) * y * z;
        res = UnalignedNMBC(size, image_stride, image_slice, PPC, ext) * (x / tile_x) * y * z + res_tail;
    }

    return res;
}

static unsigned int Cal_StandAloneMaskWrites(double *S64, double *S128, double *S256,
                                             unsigned int hgap, unsigned int vgap, int tile_x, int tile_y,
                                             unsigned int outx, unsigned int outy, unsigned int z, int stride, int slice,
                                             double image_compress_ratio,
                                             APM_COLLECTION_T *nonmask, APM_COLLECTION_T *mask)
{
    double NMW_BC64 = 0, NMW_BC128 = 0, NMW_BC256 = 0, NMWE_BC128 = 0, NMWE_BC256 = 0;
    double tile_y_tail = 0,NMW_BC64_taily = 0,NMW_BC128_taily = 0,NMW_BC256_taily = 0,num_2dtiles = 0;
    double NMW_BC64_tailx = 0,NMW_BC128_tailx = 0,NMW_BC256_tailx = 0;
    double tile_x_tail = 0;
    double NMW_S64 = 0, NMW_S128 = 0, NMW_S256 = 0, MW_S64 = 0, MW_S128 = 0, MW_S256 = 0;
    if(vgap == 0)
    {
        NMW_BC64 = UnalignedNMBC(slice * z, slice * z, slice * z, 64, 0) * image_compress_ratio;
        NMW_BC128 = UnalignedNMBC(slice * z, slice * z, slice * z, 128, 0) * image_compress_ratio;
        NMW_BC256 = UnalignedNMBC(slice * z, slice * z, slice * z, 256, 0) * image_compress_ratio;
        NMWE_BC128 = UnalgiendNMBC_Extended(slice * z, 128, /*hgap,*/ vgap, tile_x, tile_y, outx, outy, z, stride, slice) * image_compress_ratio;
        NMWE_BC256 = UnalgiendNMBC_Extended(slice * z, 256, /*hgap,*/ vgap, tile_x, tile_y, outx, outy, z, stride, slice) * image_compress_ratio;
    }
    else if(hgap == 0)
    {
        tile_y_tail = outy % tile_y;
        NMW_BC64_taily = UnalignedNMBC((unsigned int)tile_y_tail * stride, slice, slice, 64, 0) * z * image_compress_ratio;
        NMW_BC128_taily = UnalignedNMBC((unsigned int)tile_y_tail * stride, slice, slice, 128, 0) * z * image_compress_ratio;
        NMW_BC256_taily = UnalignedNMBC((unsigned int)tile_y_tail * stride, slice, slice, 256, 0) * z * image_compress_ratio;
        num_2dtiles = outy / tile_y;
        NMW_BC64 = UnalignedNMBC(slice - vgap, slice, slice, 64, 0) * z * num_2dtiles * image_compress_ratio + NMW_BC64_taily;
        NMW_BC128 = UnalignedNMBC(slice - vgap, slice, slice, 128, 0) * z * num_2dtiles * image_compress_ratio + NMW_BC128_taily;
        NMW_BC256 = UnalignedNMBC(slice - vgap, slice, slice, 256, 0) * z * num_2dtiles * image_compress_ratio + NMW_BC256_taily;
        NMWE_BC128 = UnalgiendNMBC_Extended(slice - vgap, 128, /*hgap,*/ vgap, tile_x, tile_y, outx, outy, z, stride, slice) * image_compress_ratio;
        NMWE_BC256 = UnalgiendNMBC_Extended(slice - vgap, 256, /*hgap,*/ vgap, tile_x, tile_y, outx, outy, z, stride, slice) * image_compress_ratio;
    }
    else
    {
        tile_x_tail = outx % tile_x;
        num_2dtiles = (outx / tile_x) * ((double)outy / tile_y);
        NMW_BC64_tailx = UnalignedNMBC((unsigned int)tile_x_tail, stride, slice, 64, 0) * outy * z * image_compress_ratio;
        NMW_BC128_tailx = UnalignedNMBC((unsigned int)tile_x_tail, stride, slice, 128, 0) * outy * z * image_compress_ratio;
        NMW_BC256_tailx = UnalignedNMBC((unsigned int)tile_x_tail, stride, slice, 256, 0) * outy * z * image_compress_ratio;

        NMW_BC64 = UnalignedNMBC(tile_x, stride, slice, 64, 0) * tile_y * z * num_2dtiles * image_compress_ratio + NMW_BC64_tailx;
        NMW_BC128 = UnalignedNMBC(tile_x, stride, slice, 128, 0) * tile_y * z * num_2dtiles * image_compress_ratio + NMW_BC128_tailx;
        NMW_BC256 = UnalignedNMBC(tile_x, stride, slice, 256, 0) * tile_y * z * num_2dtiles * image_compress_ratio + NMW_BC256_tailx;
        NMWE_BC128 = UnalgiendNMBC_Extended(tile_x, 128, /*hgap,*/ vgap, tile_x, tile_y, outx, outy, z, stride, slice) * image_compress_ratio;
        NMWE_BC256 = UnalgiendNMBC_Extended(tile_x, 256, /*hgap,*/ vgap, tile_x, tile_y, outx, outy, z, stride, slice) * image_compress_ratio;
    }

    /* below equations only work when hgap >= 64 */
    if(tile_x > 255 && false)
    {
        NMW_S256 = NMW_BC256;
        NMW_S128 = NMW_BC128 - *S256 - NMW_BC256;
        NMW_S64 = NMW_BC64 - 2 * (*S256) - (*S128) - NMW_BC128;
    }
    else
    {
        NMW_S64 = max(NMW_BC64 - NMW_BC128 - NMWE_BC128, 0);
        NMW_S128 = max(NMW_BC128 - NMW_BC256 - NMWE_BC256, 0);
        NMW_S256 = NMW_BC256;
    }

    MW_S64 = *S64 - NMW_S64;
    MW_S128 = *S128 - NMW_S128;
    MW_S256 = *S256 - NMW_S256;

    /* As we cannot get the equations to be accurate enough */
    if(MW_S64 < 0 && MW_S64 > -100)
    {
        *S64 = *S64 - MW_S64;
        MW_S64 = 0;
    }

    if(MW_S128 < 0 && MW_S128 > -100)
    {
        *S128 = *S128 - MW_S128;
        MW_S128 = 0;
    }

    if(MW_S256 < 0 && MW_S256 > -100)
    {
        *S256 = *S256 - MW_S256;
        MW_S256 = 0;
    }

    /*assert(NMW_S64 >= 0 && NMW_S128 >= 0 && NMW_S256 >= 0
        && MW_S64 >= 0 && MW_S128 >= 0 && MW_S256 >= 0);*/

    /* set result for nonmask and mask */
    nonmask->NMW_S64 = NMW_S64;
    nonmask->NMW_S128 = NMW_S128;
    nonmask->NMW_S256 = NMW_S256;

    mask->NMW_S64 = MW_S64;
    mask->NMW_S128 = MW_S128;
    mask->NMW_S256 = MW_S256;
    return 0;
}

static double Cal_StandAlone256BC(unsigned int hgap, unsigned int vgap, unsigned int intile_x, unsigned int intile_y,
                                        unsigned int inx, unsigned int iny, unsigned int kz, unsigned int stride, unsigned int slice,
                                        unsigned int zdp)
{
    double BubbleHgapCount = 0, BubbleVgapCount = 0, BubbleCount = 0, ZDP_amount = 0;
    double SA256 = 0, Cal_StandAlone256BC = 0;
    double num_2dtile = 0;
    double tile_count_h = 0,tile_count_v = 0,intile_x_tail = 0,intile_y_tail = 0;
    double vgap_tailx = 0,hgap_tailx = 0, vgap_taily = 0, hgap_taily = 0, SA256_tailx = 0, SA256_taily = 0;
    double BubbleHgapCount_tailx = 0,BubbleVgapCount_tailx = 0, BubbleCount_tailx = 0;
    double BubbleHgapCount_taily = 0, BubbleVgapCount_taily = 0, BubbleCount_taily = 0;
    double BubbleHgapCount_corner = 0, BubbleVgapCount_corner = 0, BubbleCount_corner = 0;
    double hgap_corner = 0,vgap_corner = 0, SA256_corner = 0;

    if (g_xydp_x == 0 && g_xydp_x == 0 )
        ZDP_amount = ZDP_LOOP_COUNT * zdp;
    else
        ZDP_amount = 2 * zdp;

    tile_count_h = inx / intile_x;
    tile_count_v = iny / intile_y;
    num_2dtile = (inx / intile_x) * ((double)iny / intile_y);

    /* calculate tail_x */
    intile_x_tail = inx % intile_x;
    if(intile_x_tail > 0)
    {
        vgap_tailx = vgap + (intile_x - intile_x_tail);
        hgap_tailx = hgap + (intile_x - intile_x_tail);
        BubbleHgapCount_tailx = min(UnalignedNMBC((unsigned int)hgap_tailx, stride, slice, 64, 0), 1) * (intile_y - 1) * kz;
        BubbleVgapCount_tailx = min(UnalignedNMBC((unsigned int)vgap_tailx, slice, slice, 64, 0), 1) * kz;
        BubbleCount_tailx = BubbleHgapCount_tailx + BubbleVgapCount_tailx;
        if(vgap_tailx < 128 && Merge2DTileEnabled)
            SA256_tailx = (UnalignedNMBC(slice * kz - (unsigned int)vgap_tailx, slice * kz, slice * kz, 256, 63) - BubbleCount_tailx) * tile_count_v;
        else if(hgap_tailx < 128)
            SA256_tailx = (UnalignedNMBC(slice - (unsigned int)vgap_tailx, stride * intile_y, slice, 256, 63) * kz - BubbleHgapCount_tailx) * tile_count_v;
        else
            SA256_tailx = UnalignedNMBC((unsigned int)intile_x_tail, stride, slice, 256, 63) * intile_y * tile_count_v * kz;
    }
    else
    {
        SA256_tailx = 0;
    }

    /* calculate tail_y */
    intile_y_tail = iny % intile_y;
    if (intile_y_tail > 0)
    {
        vgap_taily = vgap + (intile_y - intile_y_tail) * stride;
        hgap_taily = hgap;
        BubbleHgapCount_taily = min(UnalignedNMBC((unsigned int)hgap_taily, stride, slice, 64, 0), 1) * (intile_y_tail - 1) * kz;
        BubbleVgapCount_taily = min(UnalignedNMBC((unsigned int)vgap_taily, slice, slice, 64, 0), 1) * kz;
        BubbleCount_taily = BubbleHgapCount_taily + BubbleVgapCount_taily;
        if( vgap_taily < 128 && Merge2DTileEnabled)
            SA256_taily = (UnalignedNMBC(slice * kz - (unsigned int)vgap_taily, slice * kz, slice * kz, 256, 63) - BubbleCount_taily) * tile_count_h;
        else if (hgap_taily < 128)
            SA256_taily = (UnalignedNMBC(slice - (unsigned int)vgap_taily, stride * (unsigned int)intile_y_tail, slice, 256, 63) * kz - BubbleHgapCount_taily) * tile_count_h;
        else
            SA256_taily = UnalignedNMBC(intile_x, stride, slice, 256, 63) * intile_y_tail * tile_count_h * kz;

    }
    else
    {
        SA256_taily = 0;
    }

    /* calculate corner */
    if( intile_x_tail > 0 && intile_y_tail > 0)
    {
        hgap_corner = hgap_tailx;
        vgap_corner = (iny / intile_y) * intile_y * stride + hgap_corner;
        BubbleHgapCount_corner = min(UnalignedNMBC((unsigned int)hgap_corner, stride, slice, 64, 0), 1) * (intile_y_tail - 1) * kz;
        BubbleVgapCount_corner = min(UnalignedNMBC((unsigned int)vgap_corner, slice, slice, 64, 0), 1) * kz;
        BubbleCount_corner = BubbleHgapCount_corner + BubbleVgapCount_corner;
        if( vgap_corner < 128 && Merge2DTileEnabled)
            SA256_corner = (UnalignedNMBC(slice * kz - (unsigned int)vgap_corner, slice * kz, slice * kz, 256, 63) - BubbleCount_corner);
        else if (hgap_corner < 128)
            SA256_corner = (UnalignedNMBC(slice - (unsigned int)vgap_corner, slice, slice, 256, 63) * kz - BubbleHgapCount_corner);
        else
            SA256_corner = UnalignedNMBC((unsigned int)intile_x_tail, stride, slice, 256, 63) * intile_y_tail * kz;
    }
    else
    {
        SA256_corner = 0;
    }

    /* calculate total */
    BubbleHgapCount = min(UnalignedNMBC(hgap, stride,slice, 64, 0), 1) * (intile_y - 1) * kz;
    BubbleVgapCount = min(UnalignedNMBC(vgap, slice,slice, 64, 0), 1) * kz;
    BubbleCount = BubbleHgapCount + BubbleVgapCount;

    /*num_2dtile = ceilf((float)inx / intile_x) * ((float)iny / intile_y);*/
    if( vgap < 128 && Merge2DTileEnabled)
        SA256 = (UnalignedNMBC(slice * kz - vgap, slice * kz, slice * kz, 256, 63) - BubbleCount) * tile_count_h * tile_count_v + SA256_tailx + SA256_taily + SA256_corner;
    else if( hgap < 128)
        SA256 = (UnalignedNMBC(slice - vgap, stride * intile_y, slice, 256, 63) * kz - BubbleHgapCount) * tile_count_h * tile_count_v + SA256_tailx + SA256_taily + SA256_corner;
    else
        SA256 = UnalignedNMBC(intile_x, stride, slice, 256, 63) * intile_y * kz * tile_count_h * tile_count_v + SA256_tailx + SA256_taily + SA256_corner;

    Cal_StandAlone256BC = max(SA256, 0);
    return Cal_StandAlone256BC;
}


static double Cal_StandAlone256BC_NonTransposed_1x1conv(unsigned int hgap, unsigned int vgap, unsigned int intile_x, unsigned int intile_y,
                                        unsigned int inx, unsigned int iny, unsigned int kz, unsigned stride, unsigned slice,unsigned int zdp)
{
    double BubbleHgapCount = 0, BubbleVgapCount = 0, BubbleCount = 0;
    double SA256 = 0, Cal_StandAlone256BC_NonTransposed_1x1conv = 0;
    unsigned int ZDP_amount = 0;
    float num_2dtile = ((float)inx / intile_x) * ((float)iny / intile_y);
    if(g_xydp_x == 0 && g_xydp_x == 0)
    {
        ZDP_amount = ZDP_LOOP_COUNT * zdp;
    }
    else
    {
        ZDP_amount = 2 * zdp;
    }

    BubbleHgapCount = min(UnalignedNMBC(hgap, stride,slice, 64, 0), 1) * (intile_y - 1) * kz;
    BubbleVgapCount = min(UnalignedNMBC(vgap, slice,slice, 64, 0), 1) * kz;
    BubbleCount = BubbleHgapCount + BubbleVgapCount;

    if (vgap < 256)
        SA256 = UnalignedNMBC(slice * ZDP_amount, slice * ZDP_amount, slice * ZDP_amount, 256, 0) * kz / ZDP_amount;
    else if (hgap < 256)
        SA256 = UnalignedNMBC(slice - vgap, slice, slice, 256, 0) * kz - BubbleHgapCount;
    else
        SA256 = UnalignedNMBC(intile_x, stride, slice, 256, 0) * intile_y * kz;

    SA256 = SA256 * num_2dtile;
    Cal_StandAlone256BC_NonTransposed_1x1conv = max(SA256, 0);

    return Cal_StandAlone256BC_NonTransposed_1x1conv;
}


static unsigned int WriteBandWidth_By_BurstSize(unsigned int tile_xsize, unsigned int tile_ysize, unsigned int x, unsigned int y, unsigned int z,
                                                unsigned int ker_per_cores, unsigned int cores,
                                                unsigned int dst_buf, unsigned int data_size, double image_compress_ratio,
                                                unsigned int USCCacheSize, unsigned int pooling_stride, unsigned int zdp,
                                                unsigned int outimage_stride, unsigned int outimage_slice, unsigned int is_nn_write_without_usc,
                                                APM_WR_BW_BYBURST_T * WriteBandWidth_By_BurstSize)
{
    double BW_64B = 0, BW_128B = 0, BW_256B = 0;
    double BurstCount_64B = 0, BurstCount_128B = 0,BurstCount_256B = 0;
    double BurstCount_64B_Standalone = 0, BurstCount_128B_Standalone = 0, BurstCount_256B_Standalone = 0;
    int z_IL = 0,x_IL = 0;
    double factor = 0;
    int tile_xsize_IL = 0, outimage_stride_IL = 0, outimage_slice_IL = 0;
    int pooledX = 0, pooledY = 0, PooledTileX = 0, PooledTileY = 0, hgap = 0, vgap = 0;
    /*APM_WR_BW_BYBURST_T writebw_byburst_type = {0};*/
    APM_COLLECTION_T mask, nonmask;
    memset(&mask, 0, sizeof(mask)); memset(&nonmask, 0, sizeof(nonmask));

    unsigned int TrspInterleaveCh_out = pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_out;
    BW_64B = WriteBandWidth(tile_xsize, tile_ysize, x, y, z, data_size, image_compress_ratio,  USCCacheSize, pooling_stride, outimage_stride, outimage_slice, is_nn_write_without_usc, dst_buf, 64);
    BW_128B = WriteBandWidth(tile_xsize, tile_ysize, x, y, z, data_size, image_compress_ratio,  USCCacheSize, pooling_stride, outimage_stride, outimage_slice, is_nn_write_without_usc, dst_buf, 128);
    BW_256B = WriteBandWidth(tile_xsize, tile_ysize, x, y, z, data_size, image_compress_ratio,  USCCacheSize, pooling_stride, outimage_stride, outimage_slice, is_nn_write_without_usc, dst_buf, 256);

    BurstCount_64B = BW_64B / 64;
    BurstCount_128B = BW_128B / 128;
    BurstCount_256B = BW_256B / 256;

    TrspInterleaveCh_out = max(TrspInterleaveCh_out, 1);

    /*
    '  If (TrspInterleaveCh_in >= 1) And ((src_buf = DDR) Or (src_buf = AXI_SRAM)) Then 'transpose when src from DDR
    ' Assume in tileX expand TRSP_MAX_INTERLEAVE_CH times, before calling Tile3DImageSingleReadBW should adjust kz */
    z_IL = (int)ceilf((float)z / TrspInterleaveCh_out);
    x_IL = x * TrspInterleaveCh_out;
    tile_xsize_IL = tile_xsize * TrspInterleaveCh_out;
    outimage_stride_IL = outimage_stride * TrspInterleaveCh_out;
    outimage_slice_IL = outimage_slice * TrspInterleaveCh_out;

    pooledX = (unsigned int)(x_IL / pooling_stride);
    pooledY = y / pooling_stride;
    PooledTileX = tile_xsize_IL / pooling_stride;
    PooledTileY = tile_ysize / pooling_stride;

    hgap = outimage_stride_IL - PooledTileX;
    vgap = outimage_slice_IL - PooledTileY * outimage_stride_IL + hgap;

    /* in arch model VB, k is empty, so the factor should be 0 */
    ker_per_cores = 0;
    factor = min(ker_per_cores * cores / z, 1) * image_compress_ratio;
    Merge2DTileEnabled = true;
    SeparateBurstcountBySize(&BurstCount_64B, &BurstCount_128B, /*BurstCount_256B,*/
                                hgap, vgap, PooledTileX, PooledTileY, pooledX, pooledY, z_IL,
                                outimage_stride_IL, outimage_slice_IL,zdp,
                                image_compress_ratio, false,
                                &BurstCount_64B_Standalone, &BurstCount_128B_Standalone, &BurstCount_256B_Standalone);

    Cal_StandAloneMaskWrites(&BurstCount_64B_Standalone, &BurstCount_128B_Standalone, &BurstCount_256B_Standalone,
                                hgap, vgap, PooledTileX, PooledTileY, pooledX, pooledY, z_IL, outimage_stride_IL, outimage_slice_IL,
                                image_compress_ratio,&nonmask, &mask);

    /* output */
    WriteBandWidth_By_BurstSize->mask.BW_64B.cost = mask.NMW_S64 * 64;
    WriteBandWidth_By_BurstSize->mask.BW_128B.cost = mask.NMW_S128 * 128;
    WriteBandWidth_By_BurstSize->mask.BW_256B.cost = mask.NMW_S256 * 256;
    WriteBandWidth_By_BurstSize->nonmask.BW_64B.cost = nonmask.NMW_S64 * 64;
    WriteBandWidth_By_BurstSize->nonmask.BW_128B.cost = nonmask.NMW_S128 * 128;
    WriteBandWidth_By_BurstSize->nonmask.BW_256B.cost = nonmask.NMW_S256 * 256;
    return 0;
}

unsigned int _calcNumOfKernel(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int z,
    unsigned int accu_buf_depth,
    unsigned int cores,
    unsigned int interleave_mode,
    unsigned int zdp,
    unsigned int kx,
    unsigned int ky,
    unsigned int xdpx,
    unsigned int isV8,
    /*unsigned int data_size, */
    unsigned int lanes_per_conv,
    unsigned int pooling_stride,
    bool isDepthWise,
    bool isDepthWiseSupport,
    bool kernel_per_core_lt_one_third_coef_fix, /* bug2000 */
    bool asymmetricQuantization
    )
{
    unsigned int numKernel;

    // In INT16 mode, accum_buffer_depth needs to be adjusted before calling NumOfKernel()
    numKernel = (unsigned int)(accu_buf_depth * interleave_mode / tile_ysize);
    numKernel = min(127, (unsigned int)min(numKernel, ceilf((float)z / cores)));

    if (isV8)
    {
        if ((kx == 1) && (ky == 1))
        {
            numKernel = (unsigned int)((float)accu_buf_depth / (ceilf( (float) tile_ysize / interleave_mode)));
        }
        else
        {
            float tileVecNum = ceilf(tile_xsize * ((float)tile_ysize/pooling_stride) / lanes_per_conv);
            float ram_depth_for_each_kid = tileVecNum * pooling_stride;
            numKernel = (unsigned int)((float)accu_buf_depth / ram_depth_for_each_kid);
        }
    }

    if ((kx == 1) && (ky == 1) && (zdp != 1) && !isV8)
    {
        numKernel = (unsigned int)(min(numKernel, accu_buf_depth / 3));
    }

    bool bDWMerge = isDepthWiseSupport;

    if ( isV8 && (pContext->bf.COEF_ZERO_POINT_AREA_OPTIMIZATION) && (asymmetricQuantization))
    {
        float SumIReserved = ceilf((float)tile_xsize * tile_ysize / (cores * lanes_per_conv));// ' reserve a portion of accum buffer to store SUM1 result
        numKernel = min(numKernel, (unsigned int)((accu_buf_depth - SumIReserved) * lanes_per_conv / (tile_xsize * tile_ysize)));
        if (isDepthWise)
        {
            numKernel = (unsigned int)min(min(numKernel, ceilf((float)z / (cores - 1))), 127);
        }
        else
        {
            numKernel = (unsigned int)min(min(numKernel, ceilf((float)z / cores)), 127);
        }
    }
    else
    {
        if (bDWMerge)
        {
            numKernel = (unsigned int)min(min(numKernel, ceilf((float)z / (cores - 1))), 127);
        }
        else
        {
            numKernel = (unsigned int)min(min(numKernel, ceilf((float)z / cores)), 127);
        }

        if ((kx == 1) && (ky == 1) && (zdp != 1) && (xdpx != 0))
        {
            numKernel = int(min(numKernel, accu_buf_depth / 3));
        }

        if (isV8 && !kernel_per_core_lt_one_third_coef_fix)
        {
            unsigned int coef_buf_depth = 2 * accu_buf_depth * ZDP_LOOP_COUNT / 3;
            numKernel = (unsigned int)(min(numKernel, (unsigned int)(coef_buf_depth / 3)));
        }
    }

    //if (numKernel == 0)
    //{
    //    return (unsigned int)0;
    //}

    int Num_of_VZGroup = (int)ceilf((float)z / (numKernel * cores));
    numKernel = (unsigned int)ceilf((float)z / (Num_of_VZGroup * cores));

    if(numKernel == 0)
        numKernel = 0;

    return (unsigned int)numKernel;
}

// to do: export me
double _calcKernelCachePercentage(
    unsigned int kx, unsigned int ky, unsigned int kz,
    unsigned int z,
    unsigned int cores,
    double coef_compress_ratio,
    unsigned int cache_size_in_pixel,
    OUT double *adj_cache_size_in_pixel_out,
    OUT double *KernelIdealCache_out
    /*,
    bool full_cach_kernel_head_fix,
    bool is_depth_wise*/
    )
{
    bool KERNEL_HEADER_NOT_CACHED_BUG1968 = !pContext->p_hwInfo->pFeatures->FULLCACHE_KERNELHEAD_FIX;
    bool PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007 = pContext->bf.PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007;
    bool Full_KERNEL_CACHE_INTERLEAVE_BUG_2033 = pContext->bf.Full_KERNEL_CACHE_INTERLEAVE_BUG_2033; // have not fix and add feature yet. 7/14/2019

    double KernelIdealCache = ComputeKernelIdealCache(kx, ky, kz, z, coef_compress_ratio);
    double KernelNonIdealCache = ComputeKernelNonIdealCache(kx, ky, kz, z, coef_compress_ratio, cores);

    double KernelHeaderReadBandWidth = 0;
    if (KERNEL_HEADER_NOT_CACHED_BUG1968 == 0)
    {
        KernelHeaderReadBandWidth = 0;
    }
    else
    {
        KernelHeaderReadBandWidth = AXI_BURST_SIZE; // fix me not use??
    }


    double KernelStorage = ComputeKernelStorage(KernelIdealCache, KernelNonIdealCache, PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007, Full_KERNEL_CACHE_INTERLEAVE_BUG_2033, cache_size_in_pixel);
    double adj_cache_size_in_pixel = cache_size_in_pixel * KernelIdealCache / KernelStorage;

    double result = min(1, adj_cache_size_in_pixel / KernelIdealCache);

    *adj_cache_size_in_pixel_out = adj_cache_size_in_pixel;
    *KernelIdealCache_out = KernelIdealCache;

    return result;
}

// to do: export me
float ImageIdealCacheInPixel(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int kx, unsigned int ky, unsigned int kz,
    unsigned int x, unsigned int y,
    int xoffset, int yoffset,
    unsigned int sub_x, unsigned int sub_y,
    unsigned int data_size,
    bool image_not_packed_in_sram,
    unsigned int equivalent_vip_sram_width_in_byte)
{
    float imageIdealCache;
    unsigned int inx = x + kx - 1 + 2 * xoffset;
    unsigned int iny = ((y + ky - 1 + 2 * yoffset) == 0) ? -yoffset : (y + ky - 1 + 2 * yoffset);
    unsigned int inSIX = min(inx, sub_x + kx - 1);
    unsigned int inSIY = min(iny, sub_y + ky - 1);

    unsigned int intile_xsize = (tile_xsize + kx - 1);
    unsigned int intile_ysize = (tile_ysize + ky - 1);
    intile_xsize = min(intile_xsize, inSIX);
    intile_ysize = min(intile_ysize, inSIY);

    if (image_not_packed_in_sram)
    {
        imageIdealCache = ceilf(ceilf((float)intile_xsize * intile_ysize / 16) * 16 * kz / (equivalent_vip_sram_width_in_byte * 2))
                         * (equivalent_vip_sram_width_in_byte * 2) * data_size / 8;
    }
    else
    {
        imageIdealCache = (float)intile_xsize * intile_ysize * kz * data_size / 8;
    }

    return imageIdealCache;
}

static double _calcKernel4DSingleReadRepeated(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int x,
    unsigned int y
    )
{
    return ceilf((float)x / tile_xsize) * ceilf((float)y / tile_ysize);
}

static double Kernel4DSingleReadBW(
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int z,
    double       coef_compress_ratio
    )
{
    double Kernel4DSingleReadBW = (double)kx * ky * kz * z * coef_compress_ratio;
    /* Delete according to CL215304 CTS */
    /*Kernel4DSingleReadBW = ceil(Kernel4DSingleReadBW / 64) * 64;*/
    return Kernel4DSingleReadBW;
}

static double Tile3DImageSingleReadRepeated(
    unsigned int z,
    unsigned int kernel_per_core,
    unsigned int cores
    )
{
    bool is_depth_wise = pContext->pInPerfParams->cmdInfo.is_depth_wise;
    bool is_depth_wise_merge = pContext->pInPerfParams->cmdInfo.is_depth_wise_merge;

    if (is_depth_wise)
    {
        return 1.0f;
    }
    else if (is_depth_wise_merge)
    {
        // depth wise merge for mobile net
        return ceilf((float)z / (kernel_per_core * (cores - 1)));
    }
    else
    {
        // number of vz group
        return ceilf((float)z / (kernel_per_core * cores));
    }
}

static double Tile3DImageSingleReadBW(
    unsigned int tile_xsize, unsigned int tile_ysize,
    unsigned int kx, unsigned int ky, unsigned int kz,
    unsigned int x,
    /*unsigned int y,*/
    unsigned int inx, unsigned int iny,
    unsigned int brick_mode,
    unsigned int data_size,
    double       image_compress_ratio,
    unsigned int cache_line_mode_disabled,
    unsigned int async_copy_perf_fix,
    unsigned int accurate_tile_bw,
    unsigned int inimage_stride,
    unsigned int inimage_slice,
    unsigned int zdp,
    unsigned int mem_acc_unit_size_in_byte,
    /*bool         low_efficiency_jd_wr_imgbuf_bug1992_fix,  when streaming or caching from DDR */
    unsigned int vipsram_stream_or_cache_read,
    unsigned int src_buf
    )
{
    unsigned int intile_xsize, intile_ysize;
    double Tile3DImageSingleReadBW;
    int hgap = 0, vgap = 0;

    // fix me
    bool ASYNC_COPY_BUG1928 = !async_copy_perf_fix;
    intile_xsize = tile_xsize + kx - 1;
    intile_ysize = tile_ysize + ky - 1;

    intile_xsize = min(intile_xsize, inimage_stride);
    intile_ysize = min(intile_ysize, (unsigned int)ceilf((float)inimage_slice / inimage_stride));

    unsigned int TrspInterleaveCh_in = pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_in;
    if (TrspInterleaveCh_in > 1 && (src_buf == SW_TILING_FROM_DDR || src_buf == SW_TILING_FROM_AXI_SRAM))
    {
        intile_xsize = intile_xsize * TrspInterleaveCh_in;
        inx            = inx * TrspInterleaveCh_in;
        inimage_stride = inimage_stride * TrspInterleaveCh_in;
        inimage_slice  = inimage_slice * TrspInterleaveCh_in;
        tile_xsize     = tile_xsize * TrspInterleaveCh_in; // for incSize, it's strange
        kz =  (unsigned int)ceilf((float)kz / TrspInterleaveCh_in);
    }

    unsigned int DO_ACCURATE_PROFILE = (accurate_tile_bw == 1) && (TrspInterleaveCh_in == 1);

    double PPC = mem_acc_unit_size_in_byte / ((float)data_size / 8);

    if ((
        ((zdp == 1) || (kx != 1) || (ky != 1))
        && (inx == inimage_stride) && (inx * iny == inimage_slice)
        && (inx <= intile_xsize) && (iny <= intile_ysize) && !cache_line_mode_disabled
        )
        || brick_mode == 1)
    {
        Tile3DImageSingleReadBW = UnalignedBW2(intile_xsize * intile_ysize * kz, intile_xsize * intile_ysize * kz, inimage_slice * kz, (unsigned int)PPC) * image_compress_ratio;
    }
    else if((kx != 1) || (ky != 1) || (zdp == 1) || (TrspInterleaveCh_in == 9)) // non-1x1 convolution or Non_Tranposed 1x1 convolution
    {
        if (
            ((((inimage_stride % tile_xsize) == 0) && (((unsigned int)PPC % tile_xsize) == 0) && (kx == 1))
             || (((inimage_stride % (unsigned int)PPC) == 0) && (PPC >= inx)))
            && ((inimage_slice % (unsigned int)PPC) == 0))
        {
            Tile3DImageSingleReadBW = ceilf(intile_xsize / (float)PPC) * intile_ysize * PPC * kz * image_compress_ratio;
        }
        else if (DO_ACCURATE_PROFILE && ((inimage_stride % (unsigned int)PPC) == 0) && ((inimage_slice % inimage_stride) == 0))
        {
            Tile3DImageSingleReadBW = _calcPartialAlignedBW(intile_xsize, (unsigned int)PPC, tile_xsize, inx, 1, 0, 0) * intile_ysize * kz * image_compress_ratio;
        }
        else if (inimage_stride == intile_xsize)
        {
            /*Async Copy can always merge requests when inimage_stride = tile_xsize for non-1x1 convolution*/
            if (((inimage_stride * intile_ysize) % (unsigned int)PPC) == 0 && (inimage_slice % (unsigned int)PPC) == 0)
            {
                Tile3DImageSingleReadBW = ceilf(intile_xsize * intile_ysize / (float)PPC) * PPC * kz * image_compress_ratio;
            }
            else
            {
                Tile3DImageSingleReadBW = UnalignedBW2(intile_xsize * intile_ysize, intile_xsize * intile_ysize, inimage_slice, (unsigned int)PPC) * kz * image_compress_ratio;
            }
        }
        else if (DO_ACCURATE_PROFILE && ((inimage_stride % (unsigned int)(PPC / 2)) == 0) && ((inimage_slice % inimage_stride) == 0))
        {
            Tile3DImageSingleReadBW = _calcPartialAlignedBW(intile_xsize, (unsigned int)PPC, tile_xsize, inx, 2, 0, 0) * intile_ysize * kz * image_compress_ratio;
        }
        else if (DO_ACCURATE_PROFILE && ((inimage_stride % (unsigned int)(PPC / 4)) == 0) && ((inimage_slice % inimage_stride) == 0))
        {
             Tile3DImageSingleReadBW = _calcPartialAlignedBW(intile_xsize, (unsigned int)PPC, tile_xsize, inx, 4, 0, 0) * intile_ysize * kz * image_compress_ratio;
        }
        else
        {
            // debug
            Tile3DImageSingleReadBW = UnalignedBW2(intile_xsize, inimage_stride, inimage_slice, (unsigned int)PPC) * intile_ysize * kz * image_compress_ratio;
        }

        if (async_copy_perf_fix && (src_buf == SW_TILING_FROM_DDR || src_buf == SW_TILING_FROM_AXI_SRAM) && vipsram_stream_or_cache_read == 0) // 1928
        {
            if (((inimage_slice % (unsigned int)PPC) == 0) && (((inimage_stride * intile_ysize) % (unsigned int)PPC) == 0))
            {
                Tile3DImageSingleReadBW = min(inimage_stride * intile_ysize * kz * image_compress_ratio, Tile3DImageSingleReadBW);
            }
            else
            {
                Tile3DImageSingleReadBW = min(UnalignedBW2(inimage_stride * intile_ysize,  inimage_stride * intile_ysize, inimage_slice, (unsigned int)PPC) * kz * image_compress_ratio, Tile3DImageSingleReadBW);
            }

            hgap = inimage_stride - intile_xsize;
            vgap = inimage_slice - (intile_ysize * inimage_stride - hgap);

            if(hgap < PPC)
            {
                Tile3DImageSingleReadBW = UnalignedBW2(intile_ysize * inimage_stride - hgap, intile_ysize * inimage_stride, inimage_slice, (unsigned int)PPC) * kz * image_compress_ratio;
            }
            else
            {
                Tile3DImageSingleReadBW = UnalignedBW2(intile_xsize, inimage_stride, inimage_slice, (unsigned int)PPC) * intile_ysize * kz * image_compress_ratio;
            }

            unsigned int lastTileX = (x % tile_xsize) == 0 ? tile_xsize : (x % tile_xsize);
            unsigned int lastInTileX = lastTileX + kx - 1;
            unsigned int tileXNum = (x + tile_xsize - 1) / tile_xsize;
            //unsigned int tileYNum = (y + tile_ysize - 1) / tile_ysize;
            hgap = inimage_stride - lastInTileX;
            double tile3DImageSingleReadBW_right = 0;
            if(hgap < PPC)
            {
                tile3DImageSingleReadBW_right = UnalignedBW2(intile_ysize * inimage_stride - hgap, intile_ysize * inimage_stride, inimage_slice, (unsigned int)PPC) * kz * image_compress_ratio;
            }
            else
            {
                tile3DImageSingleReadBW_right = UnalignedBW2(lastInTileX, inimage_stride, inimage_slice, (unsigned int)PPC) * intile_ysize * kz * image_compress_ratio;
            }

            Tile3DImageSingleReadBW = (Tile3DImageSingleReadBW * (tileXNum - 1) + tile3DImageSingleReadBW_right) / tileXNum;

        }
    }
    else // 1x1 convolution
    {
        unsigned int ZDP_amount = 0;
        bool isV8 = (g_xydp_x == 0) && (g_xydp_y == 0);
        if (g_xydp_x == 0 && g_xydp_y == 0)
        {
            ZDP_amount = ZDP_LOOP_COUNT * zdp;
        }
        else
        {
            ZDP_amount = 2 * zdp;
        }

        unsigned int interleave_mode = 1;
        if (intile_xsize > 32)
        {
            interleave_mode = 1;
        }
        else if (intile_xsize > 16)
        {
            interleave_mode = 2;
        }
        else
        {
            interleave_mode = 4;
        }

        hgap = inimage_stride - intile_xsize;
        vgap = (int)inimage_slice - (interleave_mode * inimage_stride - hgap);


        if (isV8
            && ((inimage_slice % (unsigned int)PPC == 0) && ((inimage_stride == intile_xsize) || (intile_xsize % (unsigned int)PPC == 0)))
            )
        {
            // v8 1x1 convolute and 64B align
            Tile3DImageSingleReadBW = PPC * ceilf(intile_xsize * intile_ysize / (float)PPC) * kz * image_compress_ratio;
        }

        //if (((inimage_slice % (unsigned int)PPC) == 0) &&
        //    (((((interleave_mode * inimage_stride) % (unsigned int)PPC) == 0) && (inimage_stride == intile_xsize)) || ((intile_xsize % (unsigned int)PPC) == 0))
        //    )  // perfectly aligned

        if ( ((inimage_slice % (unsigned int)PPC) == 0) && (((interleave_mode * inimage_stride) % (unsigned int)PPC) == 0) && ((intile_ysize % interleave_mode) == 0) &&
           ((inimage_stride == intile_xsize) || ((intile_xsize == PPC) && (interleave_mode == 1))) ) // perfectly aligned

        {
            Tile3DImageSingleReadBW = interleave_mode * intile_xsize * ceilf((float)intile_ysize / interleave_mode) * kz * image_compress_ratio;
        }
        else if ((vgap < PPC) && (ASYNC_COPY_BUG1928 == 0) && (src_buf == SW_TILING_FROM_DDR || src_buf == SW_TILING_FROM_AXI_SRAM) && vipsram_stream_or_cache_read == 0)
        {
            Tile3DImageSingleReadBW = UnalignedBW2(inimage_slice * ZDP_amount, inimage_slice * ZDP_amount, inimage_slice *ZDP_amount, (unsigned int)PPC) * ceilf((float)intile_ysize / interleave_mode) * ((float)kz / ZDP_amount) * image_compress_ratio;
        }
        else if ((hgap < PPC) && (ASYNC_COPY_BUG1928 == 0) && (src_buf == SW_TILING_FROM_DDR || src_buf == SW_TILING_FROM_AXI_SRAM) && vipsram_stream_or_cache_read == 0)
        {
            Tile3DImageSingleReadBW = UnalignedBW2(interleave_mode * inimage_stride - hgap,  interleave_mode * inimage_stride, inimage_slice, (unsigned int)PPC) * ((float)intile_ysize / interleave_mode) * kz * image_compress_ratio;
        }
        else
        {
            /* unsigned int interleaveMode = pContext->pInPerfParams->interleave_mode; */
            Tile3DImageSingleReadBW = UnalignedBW2(intile_xsize,  inimage_stride, inimage_slice, (unsigned int)PPC) * intile_ysize * kz * image_compress_ratio;
        }
    }

    if (pContext->bf.LOW_EFFICIENCY_JD_WR_IMGBUF_BUG1992 && vipsram_stream_or_cache_read)
    {
        Tile3DImageSingleReadBW = max(UnalignedBW2(intile_xsize, inimage_stride, inimage_slice, (unsigned int)PPC) * intile_ysize * kz, Tile3DImageSingleReadBW);
    }

    Tile3DImageSingleReadBW = ceil(Tile3DImageSingleReadBW / PPC) * PPC;

    return Tile3DImageSingleReadBW;
}

double ComputeKernelIdealCache(
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int z,
    double coef_compress_ratio
    )
{
    bool isDepthWise = pContext->pInPerfParams->cmdInfo.is_depth_wise;
    bool isDepthWiseMerge = pContext->pInPerfParams->cmdInfo.is_depth_wise_merge;

    double kernelIdealCache = 0;
    if (isDepthWise)
    {
       kernelIdealCache = Kernel4DSingleReadBW(kx, ky, 1, z, 1);
    }
    else if(isDepthWiseMerge)
    {
        kernelIdealCache = Kernel4DSingleReadBW(kx, ky, 1, kz, 1) + \
                           Kernel4DSingleReadBW(1, 1, kz, z, coef_compress_ratio);
    }
    else
    {
       kernelIdealCache = Kernel4DSingleReadBW(kx, ky, kz, z, coef_compress_ratio);
    }

    return kernelIdealCache;
}

/* if we got each core's bit stream size, this function can be improved*/
double ComputeKernelNonIdealCache(
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int z,
    double coef_compress_ratio,
    unsigned int cores
    )
{
    bool isDepthWise = pContext->pInPerfParams->cmdInfo.is_depth_wise;
    bool isDepthWiseMerge = pContext->pInPerfParams->cmdInfo.is_depth_wise_merge;

    double kernelNonIdealCache;
    unsigned int burst_size = pContext->p_hwInfo->pFeatures->EQUIVALENT_VIP_SRAM_WIDTH_INBYTE * 2;
    double zPerCore = ceilf((float)z/cores);
    if (isDepthWise)
    {
        /* add ceil for the accuracy issue */
        kernelNonIdealCache = ceilf((float)(ceil(Kernel4DSingleReadBW(kx, ky, 1, (unsigned int)zPerCore, 1)) / burst_size)) * burst_size * cores;
    }
    else if(isDepthWiseMerge)
    {
        /* add ceil for the accuracy issue */
        kernelNonIdealCache = ceilf((float)(ceil(Kernel4DSingleReadBW(kx, ky, 1, kz, 1)) / burst_size)) * burst_size;
        zPerCore = ceilf((float)z / (cores - 1));
        kernelNonIdealCache += ceilf((float)(ceil(Kernel4DSingleReadBW(1, 1, kz, (unsigned int)zPerCore, coef_compress_ratio)) / burst_size)) * burst_size * (cores - 1);
    }
    else
    {
        /* add ceil for the accuracy issue */
        kernelNonIdealCache = ceilf((float)(ceil(Kernel4DSingleReadBW(kx, ky, kz, (unsigned int)zPerCore, coef_compress_ratio)) / burst_size)) * burst_size * cores;
    }

    /*  what the hell of this magic number. */
    double margin_ratio = (1.25 - 1.05) * (1 - min(coef_compress_ratio, 1)) / (1 - 0.02) + 1.05;

    return kernelNonIdealCache * margin_ratio;
}

double ComputeKernelStorage(
    double IdealCache,
    double NonIdealCache,
    int    Bug2007,
    int    Bug2033,
    int    cache_space
    )
{
    //'Bug2007 - PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007
    //'Bug2033 - Full_KERNEL_CACHE_INTERLEAVE_BUG_2033

    /* Comment it since V7 And V8 are opposite */
    //assert((Bug2007 >= Bug2033) && "bug2033 is fixed before bug2007!");
    double KernelStorage = 0;
    if (Bug2007 == 0)
    {
        KernelStorage = IdealCache;
    }
    else
    {
        if (Bug2033 == 1)
        {
            KernelStorage = NonIdealCache;
        }
        else
        {
            if (IdealCache <= cache_space)
            {
                KernelStorage = IdealCache;
            }
            else
            {
                KernelStorage = NonIdealCache;
            }
        }
    }

    return KernelStorage;
}

// KernelReadBandWidth is in bytes
static double KernelReadBandWidth(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    /*unsigned int kernel_per_core,*/
    unsigned int kx, unsigned int ky, unsigned int kz,
    unsigned int x, unsigned int y, unsigned int z,
    unsigned int kernel_buf,
    /*unsigned int inx, unsigned int iny,*/
    unsigned int cores,
    /*unsigned int brick_mode,*/
    unsigned int data_size,
    double       coef_compress_ratio,
    /*double       image_compress_ratio,*/
    double       cache_size_in_pixel,
    unsigned int full_cach_kernel_head_fix,
    unsigned int actual_burst_size,
    /*unsigned int is_depth_wise,*/
    double *kernel_read_bw_tile0,
    arch_model_cache_type *cacheStrategy
    )
{
    double KernelIdealCache, kernelRepeatRead, kernelReadBandWidth, KernelNonIdealCache, kernelReadBandWidthTile0;
    bool PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007 = pContext->bf.PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007;
    bool Full_KERNEL_CACHE_INTERLEAVE_BUG_2033    = pContext->bf.Full_KERNEL_CACHE_INTERLEAVE_BUG_2033;
    /*double zPerCore = ceilf((float)z/cores);*/
    double adj_cache_size_in_pixel;
    unsigned int kernelHeaderReadBandWidth = 0;
    unsigned int kernel_burst_size = 0;
    if ((kernel_buf == SW_TILING_FROM_DDR) && (pContext->bf.NN_LARGE_BURST_SIZE == 0))
    {
        kernel_burst_size = 64;
    }
    else
    {
        kernel_burst_size = actual_burst_size;
    }

    kernelRepeatRead = _calcKernel4DSingleReadRepeated(tile_xsize, tile_ysize, x, y); // refine my name
    KernelIdealCache = ComputeKernelIdealCache(kx, ky, kz, z, coef_compress_ratio);
    KernelNonIdealCache = ComputeKernelNonIdealCache(kx, ky, kz, z, coef_compress_ratio, cores);

    if (full_cach_kernel_head_fix)
    {
        // bug 1968
        kernelHeaderReadBandWidth = 0;
    }
    else
    {
        // fix me, kernel header readBandwidth may not always 64
        kernelHeaderReadBandWidth = AXI_BURST_SIZE;
    }

    bool kernelFullCahce = (KernelIdealCache <= cache_size_in_pixel)? 1: 0;

    double KernelStorage = ComputeKernelStorage(KernelIdealCache, KernelNonIdealCache, PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007, Full_KERNEL_CACHE_INTERLEAVE_BUG_2033, (int)cache_size_in_pixel);
    adj_cache_size_in_pixel = cache_size_in_pixel * KernelIdealCache / KernelStorage;

    if (pContext->bf.PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007 && !kernelFullCahce)
    {
        adj_cache_size_in_pixel = cache_size_in_pixel * KernelIdealCache / KernelNonIdealCache;
    }
    else
    {
        adj_cache_size_in_pixel = cache_size_in_pixel;
    }

    /* Add cache strategy */
    cacheStrategy->sizeCached = min(KernelStorage, adj_cache_size_in_pixel);
    cacheStrategy->percentage = cacheStrategy->sizeCached / KernelStorage;
    cacheStrategy->sizeNeeded = KernelStorage;
    /* if only one HW tile, output NO kernel cache */
    if(tile_xsize >= x && tile_ysize >= y)
    {
        cacheStrategy->sizeCached = 0;
        cacheStrategy->percentage = 0;
    }

    kernelReadBandWidthTile0 = (KernelIdealCache) * (data_size / 8) + kernelHeaderReadBandWidth;

    /* fix me, when partial interleave cache, use nonIdeal cache is better? */
    kernelReadBandWidth = ((KernelIdealCache + kernelHeaderReadBandWidth) * kernelRepeatRead) - (min(KernelIdealCache, adj_cache_size_in_pixel) * (kernelRepeatRead - 1));
    kernelReadBandWidth *= (data_size / 8);
    /* align Kernel BW to 64B */

    kernelReadBandWidthTile0 = ceilf((float)kernelReadBandWidthTile0 / kernel_burst_size) * actual_burst_size;
    kernelReadBandWidth = ceilf((float)kernelReadBandWidth / kernel_burst_size) * actual_burst_size;

    if (NULL != kernel_read_bw_tile0)
    {
        *kernel_read_bw_tile0 = kernelReadBandWidthTile0;
    }

    return kernelReadBandWidth;
}


static unsigned int KernelReadBandWidth_By_BurstSize(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int kernel_per_core,
    unsigned int kx, unsigned int ky, unsigned int kz,
    unsigned int x, unsigned int y, unsigned int z,
    unsigned int kernel_buf,
    /*unsigned int inx, unsigned int iny,*/
    unsigned int cores,
    /*unsigned int brick_mode,*/
    unsigned int data_size,
    double       coef_compress_ratio,
    /*double       image_compress_ratio,*/
    double       cache_size_in_pixel,
    /*unsigned int is_depth_wise,*/
    arch_model_cache_type   *cache_strategy,
    unsigned int full_cach_kernel_head_fix,
    arch_model_bw_byburst_type *bw_byburst_type
    )
{
    double BW_64B = 0, BW_128B = 0, BW_256B = 0;
    double BW_64B_tile0 = 0, BW_128B_tile0 = 0,BW_256B_tile0 = 0;
    BW_64B = KernelReadBandWidth(tile_xsize, tile_ysize, /*kernel_per_core,*/ kx, ky, kz, x, y, z, kernel_buf, /*inx, iny,*/ cores, /*brick_mode,*/ data_size, coef_compress_ratio, /*image_compress_ratio,*/ cache_size_in_pixel, full_cach_kernel_head_fix, 64, /*is_depth_wise,*/&BW_64B_tile0, cache_strategy);
    BW_128B = KernelReadBandWidth(tile_xsize, tile_ysize, /*kernel_per_core,*/ kx, ky, kz, x, y, z, kernel_buf, /*inx, iny,*/ cores, /*brick_mode,*/ data_size, coef_compress_ratio, /*image_compress_ratio,*/ cache_size_in_pixel, full_cach_kernel_head_fix, 128, /*is_depth_wise,*/ &BW_128B_tile0, cache_strategy);
    BW_256B = KernelReadBandWidth(tile_xsize, tile_ysize, /*kernel_per_core,*/ kx, ky, kz, x, y, z, kernel_buf, /*inx, iny,*/ cores, /*brick_mode,*/ data_size, coef_compress_ratio, /*image_compress_ratio,*/ cache_size_in_pixel, full_cach_kernel_head_fix, 256, /*is_depth_wise,*/ &BW_256B_tile0, cache_strategy);

    bw_byburst_type->BW_256B.cost = BW_256B;
    bw_byburst_type->BW_256B.tile0 = BW_256B_tile0;
    bw_byburst_type->BW_256B.tile0VZGroup0 = BW_256B_tile0 * min((float)kernel_per_core * cores / z, 1);

    return 0;
}

//' ImageReadBandWidth is in byte
static double ImageReadBandWidth(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int kernel_per_core,
    unsigned int kx, unsigned int ky, unsigned int kz,
    unsigned int x, unsigned int y, unsigned int z,
    unsigned int inx, unsigned int iny,
    unsigned int cores,
    unsigned int brick_mode,
    unsigned int data_size,
    /*double coef_compress_ratio,*/
    double image_compress_ratio,
    double cache_size_in_pixel,
    bool image_not_packed_in_sram,
    bool cache_line_mode_disabled,
    bool async_copy_perf_fix,
    bool accurate_tile_bw,
    bool is_depth_wise,
    unsigned int equivalent_vip_sram_width_in_byte,
    unsigned int in_image_stride,
    unsigned int in_image_slice,
    unsigned int mem_acc_unit_size_in_byte,
    unsigned int zdp,
    bool image_partial_cache,
    bool low_efficiency_jd_wr_imgbuf_bug1992_fix,
    bool vipsram_stream_or_cache_read,
    unsigned int src_buf,
    double *image_read_bw_vzgroup0,
    arch_model_cache_type *cacheStrategy
    )
{
    // fix me, hard code here
    low_efficiency_jd_wr_imgbuf_bug1992_fix = !pContext->bf.LOW_EFFICIENCY_JD_WR_IMGBUF_BUG1992;
    double temp = 0,temp1 = 0;
    double imageRepeatSingleRead, imageRepeatRead, imageRepeatCacheRead, imageIdealCache, imageReadBandWidth, imageTile3DBW, result_bw_vzgroup0;
    double tmp;
    unsigned int intile_xsize = (tile_xsize + kx - 1);
    unsigned int intile_ysize = (tile_ysize + ky - 1);
    intile_xsize = min(intile_xsize, inx);
    intile_ysize = min(intile_ysize, iny);

    tmp = ceil((double)(inx + (ceil((double)x / tile_xsize) - 1) * (kx - 1)) / intile_xsize) *
                      (iny + (ceil((double)y / tile_ysize) - 1) * (ky - 1)) / intile_ysize;
    imageRepeatSingleRead = Tile3DImageSingleReadRepeated(z, kernel_per_core, cores);
    imageRepeatRead = imageRepeatSingleRead * tmp;
    imageRepeatCacheRead = (imageRepeatSingleRead - 1.0f) * tmp;
    unsigned int TrspInterleaveCh_in = pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_in;
    if (TrspInterleaveCh_in > 1 && (src_buf == SW_TILING_FROM_DDR || SW_TILING_FROM_AXI_SRAM))
    {
        if ((kx == 1) && (ky == 1) && (zdp == 3))
        {
            assert(TrspInterleaveCh_in == 9 && "1x1 kernel always interleave 9!\n");
        }
    }

    imageTile3DBW = Tile3DImageSingleReadBW(tile_xsize, tile_ysize, kx, ky, kz, x, /*y,*/ inx, iny, brick_mode, data_size, image_compress_ratio,
        cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, in_image_stride, in_image_slice, zdp, mem_acc_unit_size_in_byte,
        vipsram_stream_or_cache_read, src_buf);

    if (image_not_packed_in_sram) // bug 1980
    {
        imageIdealCache = ceilf(ceilf((float)intile_xsize * intile_ysize / 16) * 16 * kz / (equivalent_vip_sram_width_in_byte * 2))
                         * (equivalent_vip_sram_width_in_byte * 2) * data_size / 8;
    }
    else
    {
        imageIdealCache = (float)intile_xsize * intile_ysize * kz * data_size / 8;
    }

    if (!image_partial_cache && (imageIdealCache > cache_size_in_pixel))
    {
        result_bw_vzgroup0 = imageTile3DBW * (imageRepeatRead - imageRepeatCacheRead);
        //imageReadBandWidth = imageTile3DBW * (imageRepeatRead - (min(imageIdealCache, cache_size_in_pixel) * imageRepeatCacheRead / imageIdealCache));
        imageReadBandWidth = imageTile3DBW * imageRepeatRead;
        cacheStrategy->sizeCached = 0;
        cacheStrategy->percentage = 0;
        cacheStrategy->sizeNeeded = imageIdealCache;
    }
    else
    {
        result_bw_vzgroup0 = imageTile3DBW * (imageRepeatRead - imageRepeatCacheRead);
        temp = min(imageIdealCache, cache_size_in_pixel) * imageRepeatCacheRead / imageIdealCache;
        temp1 = imageRepeatRead - (min(imageIdealCache, cache_size_in_pixel) * imageRepeatCacheRead / imageIdealCache);
        imageReadBandWidth = imageTile3DBW * temp1;
        cacheStrategy->sizeCached = min(imageIdealCache, cache_size_in_pixel);
        cacheStrategy->percentage = cacheStrategy->sizeCached / imageIdealCache;
        cacheStrategy->sizeNeeded = imageIdealCache;
    }

    /* if only one vzgroup, output No image cache */
    if(kernel_per_core * cores >= z)
    {
        cacheStrategy->sizeCached = 0;
        cacheStrategy->percentage = 0;
    }

    // adjust for depthwise and depthwise merge
    if (is_depth_wise)
    {
        // depth wise vz group0 will read part of input 3D tile
        result_bw_vzgroup0 *= min((float)kernel_per_core * cores / z, 1);
    }

    imageReadBandWidth = ceilf((float)imageReadBandWidth * (data_size / 8)/ mem_acc_unit_size_in_byte) * mem_acc_unit_size_in_byte;
    result_bw_vzgroup0 = ceilf((float)result_bw_vzgroup0 * (data_size / 8) / mem_acc_unit_size_in_byte) * mem_acc_unit_size_in_byte;

    bool isTA_MERGE = pContext->pInPerfParams->cmdInfo.isTA_MERGE;
    if (isTA_MERGE)
    {
        double TA_TileBW = Tile3DImageSingleReadBW(tile_xsize, tile_ysize, 1, 1, 1, x, /* y,*/ x, y, /* fix me, should inx, iny */
                                                   brick_mode, data_size, image_compress_ratio,
                                                   cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, in_image_stride, in_image_slice, zdp, mem_acc_unit_size_in_byte,
                                                   vipsram_stream_or_cache_read, src_buf);

        double TA_BW = TA_TileBW * ceilf((inx + (ceilf((float)x / tile_xsize) - 1) * (kx - 1)) / intile_xsize) * (iny + (ceilf((float)y / tile_ysize) - 1) * (ky - 1)) / intile_ysize;
        result_bw_vzgroup0 = result_bw_vzgroup0 + TA_BW * min(z, kernel_per_core * cores);
        imageReadBandWidth = imageReadBandWidth + TA_BW * z;
    }

    if (imageReadBandWidth < result_bw_vzgroup0 )
    {
        result_bw_vzgroup0 = imageReadBandWidth;
    }

    if (NULL != image_read_bw_vzgroup0)
    {
        *image_read_bw_vzgroup0 = result_bw_vzgroup0;
    }

    return imageReadBandWidth;
}

// ' ReadBandWidth is in bytes
static double ReadBandWidth(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int kernel_per_core,
    unsigned int kx, unsigned int ky, unsigned int kz,
    unsigned int x, unsigned int y, unsigned int z,
    unsigned int kernel_buf,
    unsigned int inx, unsigned int iny,
    unsigned int cores,
    unsigned int brick_mode,
    unsigned int data_size,
    double coef_compress_ratio,
    double image_compress_ratio,
    unsigned int l2cache_size,
    unsigned int image_partial_cache,
    unsigned int nn_cmd_size,
    unsigned int image_not_packed_in_sram,
    unsigned int full_cach_kernel_head_fix,
    unsigned int cache_line_mode_disabled,
    unsigned int async_copy_perf_fix,
    unsigned int accurate_tile_bw,
    unsigned int is_depth_wise,
    unsigned int inimage_stride,
    unsigned int inimage_slice,
    unsigned int zdp,
    unsigned int mem_acc_unit_size_in_byte,
    unsigned int equivalent_vip_sram_width_in_byte,
    double *ddrKernelReadBW,
    double *ddrInImageReadBW,
    double *kernel_read_bw_tile0,
    double *image_read_bw_vzgroup0,
    APM_BW_CYCLE_COST_T *bw_cost_detail,
    Outputs_Type *outPuts
    )
{
    //bool vipsram_stream_or_cache_read = pContext->pInPerfParams->cmdInfo.src_buf != SW_TILING_FROM_VIP_SRAM;
    bool low_efficiency_jd_wr_imgbuf_bug1992_fix = !pContext->bf.LOW_EFFICIENCY_JD_WR_IMGBUF_BUG1992;

    double cache_size_in_pixel, KernelIdealCache, ImageIdealCache, kernelReadBandWidthTile0, imageReadBandWidthVZGroup0;
    double kernelRepeatRead, imageRepeatSingleRead, imageRepeatRead, readBandWidth = 0, KernelNonIdealCache, KernelStorage;
    double kernel_readbw, inimage_readbw;
    double tmp;
    /* zPerCore; */
    unsigned int intile_xsize = (tile_xsize + kx - 1);
    unsigned int intile_ysize = (tile_ysize + ky - 1);
    intile_xsize = min(intile_xsize, inx);
    intile_ysize = min(intile_ysize, iny);

    tmp = ceil((double)(inx + (ceil((double)x / tile_xsize) - 1) * (kx - 1)) / intile_xsize) *
                      (iny + (ceil((double)y / tile_ysize) - 1) * (ky - 1)) / intile_ysize;
    cache_size_in_pixel = l2cache_size / ((float)data_size / 8);

    kernelRepeatRead = _calcKernel4DSingleReadRepeated(tile_xsize, tile_ysize, x, y);
    imageRepeatSingleRead = Tile3DImageSingleReadRepeated(z, kernel_per_core, cores);
    /* According to the VB, here need to use  (imageRepeatSingleRead -1)*/
    imageRepeatRead = (imageRepeatSingleRead) * tmp;
    //imageRepeatedCacheRead = ;
    KernelIdealCache = ComputeKernelIdealCache(kx, ky, kz, z, coef_compress_ratio);
    KernelNonIdealCache = ComputeKernelNonIdealCache(kx, ky, kz, z, coef_compress_ratio, cores);

    KernelStorage = pContext->bf.PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007? KernelIdealCache : KernelNonIdealCache;
    if (image_not_packed_in_sram)
    {
        // cache bug 1980
        ImageIdealCache = ceilf(ceilf((float)intile_xsize * intile_ysize / 16) * 16 * kz / (equivalent_vip_sram_width_in_byte * 2))
                         * (equivalent_vip_sram_width_in_byte * 2) * data_size / 8;
    }
    else
    {
        ImageIdealCache = (float)intile_xsize * intile_ysize * kz * data_size / 8;
    }
    {
          //'Try cache image first
        double cache_space_for_image = cache_size_in_pixel;
        double cache_space_for_kernel = cache_size_in_pixel - ImageIdealCache * ((ImageIdealCache <= cache_size_in_pixel) ? 1 : 0);
        inimage_readbw = ImageReadBandWidth(tile_xsize, tile_ysize, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, data_size, /*coef_compress_ratio,*/ image_compress_ratio, /* cache_space_for_image */ cache_size_in_pixel, image_not_packed_in_sram,
                cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, inimage_stride, inimage_slice, mem_acc_unit_size_in_byte, zdp, image_partial_cache,
                low_efficiency_jd_wr_imgbuf_bug1992_fix, 0 /*vipsram_stream_or_cache_read*/, SW_TILING_FROM_DDR, &imageReadBandWidthVZGroup0, &(outPuts->imageCacheType)); // fix me

        kernel_readbw = KernelReadBandWidth(tile_xsize, tile_ysize, /*kernel_per_core,*/ kx, ky, kz, x, y, z, kernel_buf, /*inx, iny, */
             cores, /*brick_mode,*/ data_size, coef_compress_ratio, /*image_compress_ratio,*/ cache_space_for_kernel,
             full_cach_kernel_head_fix, mem_acc_unit_size_in_byte, /*is_depth_wise,*/ &kernelReadBandWidthTile0,&(outPuts->kernelCacheType));

        //'Try cache Kernel first
        double cache_space_for_kernel_try = cache_size_in_pixel;
        KernelStorage = ComputeKernelStorage(KernelIdealCache, KernelNonIdealCache, pContext->bf.PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007, pContext->bf.Full_KERNEL_CACHE_INTERLEAVE_BUG_2033, (int)cache_space_for_kernel);
        double cache_space_for_image_try = max(0, cache_size_in_pixel - KernelStorage);
        double inimage_readbw_try = ImageReadBandWidth(tile_xsize, tile_ysize, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, data_size, /*coef_compress_ratio,*/ image_compress_ratio, cache_size_in_pixel, image_not_packed_in_sram,
                cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, inimage_stride, inimage_slice, mem_acc_unit_size_in_byte, zdp, image_partial_cache,
                low_efficiency_jd_wr_imgbuf_bug1992_fix, 0 /*vipsram_stream_or_cache_read*/, SW_TILING_FROM_DDR, &imageReadBandWidthVZGroup0, &(outPuts->imageCacheType)); // fix me

        double kernel_readbw_try = KernelReadBandWidth(tile_xsize, tile_ysize, /*kernel_per_core,*/ kx, ky, kz, x, y, z, kernel_buf, /*inx, iny, */
             cores, /*brick_mode,*/ data_size, coef_compress_ratio, /*image_compress_ratio,*/ cache_space_for_kernel,
             full_cach_kernel_head_fix, mem_acc_unit_size_in_byte, /*is_depth_wise,*/ &kernelReadBandWidthTile0,&(outPuts->kernelCacheType));

        if ((inimage_readbw_try + kernel_readbw_try) < (inimage_readbw + kernel_readbw))
        {
            inimage_readbw = inimage_readbw_try;
            kernel_readbw = kernel_readbw_try;
            cache_space_for_image = cache_space_for_image_try;
            cache_space_for_kernel = cache_space_for_kernel_try;
        }
    }
    if (ddrKernelReadBW) *ddrKernelReadBW = kernel_readbw;
    if (ddrInImageReadBW) *ddrInImageReadBW = inimage_readbw;

    readBandWidth = nn_cmd_size + kernel_readbw + inimage_readbw;
    if (NULL != bw_cost_detail)
    {
        bw_cost_detail->cost = readBandWidth;
        bw_cost_detail->tile0VZGroup0 = nn_cmd_size + kernelReadBandWidthTile0 * min((1.0f * kernel_per_core * cores / z), 1.0f) +  imageReadBandWidthVZGroup0 * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        bw_cost_detail->tile0 = nn_cmd_size + kernelReadBandWidthTile0 + inimage_readbw * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        bw_cost_detail->vzGroup0 = nn_cmd_size + kernel_readbw * min((1.0f * kernel_per_core * cores / z), 1.0f) + imageReadBandWidthVZGroup0;
    }
    if (NULL != kernel_read_bw_tile0)
    {
        *kernel_read_bw_tile0 = kernelReadBandWidthTile0;
    }
    if (NULL != image_read_bw_vzgroup0)
    {
        *image_read_bw_vzgroup0 = imageReadBandWidthVZGroup0;
    }

    return readBandWidth;
}


static unsigned int SeparateBurstcountBySize(double *BurstCount_64B, double *BurstCount_128B, /*double BurstCount_256B,*/
                                             unsigned int hgap, unsigned int vgap, int tile_x, int tile_y,
                                             unsigned int x, unsigned int y,unsigned int z,
                                             int stride,int slice, unsigned int zdp, double factor,unsigned int NonTransposed_1x1,
                                             double *output_BurstCount_64B_Standalone, double *output_BurstCount_128B_Standalone, double *output_BurstCount_256B_Standalone)
{
    double t_output_BurstCount_64B_Standalone = 0, t_output_BurstCount_128B_Standalone = 0, t_output_BurstCount_256B_Standalone = 0;
    t_output_BurstCount_64B_Standalone = 2 * (*BurstCount_128B) - (*BurstCount_64B);
    if(NonTransposed_1x1 == 1)
    {
        t_output_BurstCount_256B_Standalone = Cal_StandAlone256BC_NonTransposed_1x1conv(hgap, vgap, tile_x, tile_y, x, y, z, stride, slice, zdp) * factor;
    }
    else
    {
        t_output_BurstCount_256B_Standalone = Cal_StandAlone256BC(hgap, vgap, tile_x, tile_y, x, y, z, stride, slice, zdp) * factor;
    }

    /* output_BurstCount_256B_Standalone = output_BurstCount_256B_Standalone * Ceil(x / tile_x) * (y / tile_y) */
    t_output_BurstCount_128B_Standalone = *BurstCount_128B - t_output_BurstCount_64B_Standalone - 2 * t_output_BurstCount_256B_Standalone;
    if (t_output_BurstCount_128B_Standalone < 0)
    {
        *BurstCount_128B = *BurstCount_128B - t_output_BurstCount_128B_Standalone;
        t_output_BurstCount_128B_Standalone = 0;
    }
    assert(t_output_BurstCount_64B_Standalone >= 0);
    /* assert(t_output_BurstCount_128B_Standalone >= -1); */
    assert(t_output_BurstCount_256B_Standalone >= 0);

    *output_BurstCount_64B_Standalone = t_output_BurstCount_64B_Standalone;
    *output_BurstCount_128B_Standalone = t_output_BurstCount_128B_Standalone;
    *output_BurstCount_256B_Standalone = t_output_BurstCount_256B_Standalone;

    return 0;
}


/* add new function for NN Transpose and Burst Size */
static unsigned int ImageReadBandWidth_By_BurstSize(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int kernel_per_core,
    unsigned int kx, unsigned int ky, unsigned int kz,
    unsigned int x, unsigned int y, unsigned int z,
    unsigned int inx, unsigned int iny,
    unsigned int cores,
    unsigned int brick_mode,
    unsigned int data_size,
    /*double       coef_compress_ratio,*/
    double       image_compress_ratio,
    double       cache_size_in_pixel,
    unsigned int image_not_packed_in_sram,
    unsigned int cache_line_mode_disabled,
    unsigned int async_copy_perf_fix,
    unsigned int accurate_tile_bw,
    unsigned int is_depth_wise,
    unsigned int equivalent_vip_sram_width_in_byte,
    unsigned int inimage_stride,
    unsigned int inimage_slice,
    /*unsigned int mem_acc_unit_size_in_byte,*/
    unsigned int zdp,
    unsigned int image_partial_cache,
    bool low_efficiency_jd_wr_imgbuf_bug1992_fix,
    bool vipsram_stream_or_cache_read,
    arch_model_cache_type  *cache_strategy,
    /*unsigned int src_buf,*/
    unsigned int trspInterleaveCh_in,
    arch_model_bw_byburst_type *bw_byburst_type
    )
{
    //BW_By_Burst_Type
    double BW_64B = 0, BW_128B = 0, BW_256B = 0;
    double BW_64B_tile0 = 0, BW_128B_tile0 = 0,BW_256B_tile0 = 0;
    double BurstCount_64B = 0,BurstCount_64B_vzgroup0 = 0,BurstCount_128B = 0,BurstCount_128B_vzgroup0 = 0,BurstCount_256B = 0,BurstCount_256B_vzgroup0 = 0;
    double BurstCount_64B_Standalone = 0, BurstCount_64B_Standalone_vzgroup0 = 0,
            BurstCount_128B_Standalone = 0,BurstCount_128B_Standalone_vzgroup0 = 0,
            BurstCount_256B_Standalone = 0, BurstCount_256B_Standalone_vzgroup0 = 0;
    unsigned int intile_xsize = 0,intile_ysize = 0;
    unsigned int intile_xsize_IL = 0,inx_IL = 0,inimage_stride_IL = 0, inimage_slice_IL = 0,tile_xsize_IL = 0,kz_IL = 0;
    unsigned int TrspInterleaveCh_in = trspInterleaveCh_in;
    unsigned int NonTransposed_1x1 = 0,interleave_mode = 0;
    unsigned int ZDP_amount = 0, vgap = 0, hgap = 0;
    double factor = 0;

    BW_64B = ImageReadBandWidth(tile_xsize, tile_ysize, kernel_per_core, kx, ky, kz, x, y, z, inx, iny, cores, brick_mode, data_size, /*coef_compress_ratio,*/ image_compress_ratio, cache_size_in_pixel,
        image_not_packed_in_sram,cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, inimage_stride, inimage_slice, 64, zdp,
        image_partial_cache, low_efficiency_jd_wr_imgbuf_bug1992_fix, vipsram_stream_or_cache_read, SW_TILING_FROM_DDR, &BW_64B_tile0, cache_strategy);
    BW_128B = ImageReadBandWidth(tile_xsize, tile_ysize, kernel_per_core, kx, ky, kz, x, y, z, inx, iny, cores, brick_mode, data_size, /*coef_compress_ratio,*/ image_compress_ratio, cache_size_in_pixel,
        image_not_packed_in_sram,cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, inimage_stride, inimage_slice, 128, zdp,
        image_partial_cache, low_efficiency_jd_wr_imgbuf_bug1992_fix, vipsram_stream_or_cache_read, SW_TILING_FROM_DDR, &BW_128B_tile0, cache_strategy);
    BW_256B = ImageReadBandWidth(tile_xsize, tile_ysize, kernel_per_core, kx, ky, kz, x, y, z, inx, iny, cores, brick_mode, data_size, /*coef_compress_ratio,*/ image_compress_ratio, cache_size_in_pixel,
        image_not_packed_in_sram,cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, inimage_stride, inimage_slice, 256, zdp,
        image_partial_cache, low_efficiency_jd_wr_imgbuf_bug1992_fix, vipsram_stream_or_cache_read, SW_TILING_FROM_DDR, &BW_256B_tile0, cache_strategy);

    BurstCount_64B = BW_64B / 64;
    BurstCount_128B = BW_128B / 128;
    BurstCount_256B = BW_256B / 256;
    BurstCount_64B_vzgroup0 = BW_64B_tile0 / 64;
    BurstCount_128B_vzgroup0 = BW_128B_tile0 / 128;
    BurstCount_256B_vzgroup0 = BW_256B_tile0 / 256;


    intile_xsize = (tile_xsize + kx - 1);
    intile_ysize = (tile_ysize + ky - 1);
    intile_xsize = min(intile_xsize, inx);
    intile_ysize = min(intile_ysize, iny);
    TrspInterleaveCh_in = max(TrspInterleaveCh_in, 1);

    intile_xsize_IL = intile_xsize * TrspInterleaveCh_in;
    inx_IL = inx * TrspInterleaveCh_in;
    inimage_stride_IL = inimage_stride * TrspInterleaveCh_in;
    inimage_slice_IL = inimage_slice * TrspInterleaveCh_in;
    tile_xsize_IL = tile_xsize * TrspInterleaveCh_in;       /*  for incSize  */
    kz_IL = (unsigned int)ceilf((float)kz / TrspInterleaveCh_in);

    NonTransposed_1x1 = (kx == 1 && ky == 1) && (TrspInterleaveCh_in == 1);

    if(!NonTransposed_1x1)          /* non_1x1 conv or tranposed conv */
    {
        hgap = inimage_stride_IL - intile_xsize_IL;
        vgap = inimage_slice_IL - (intile_ysize * inimage_stride_IL - hgap);
    }
    else                            /* 1x1 non_transposed */
    {
        if(g_xydp_x == 0 && g_xydp_y == 0)
        {
            ZDP_amount = ZDP_LOOP_COUNT * zdp;
        }
        else
        {
            ZDP_amount = 2 * zdp;
        }

        if(intile_xsize_IL * (data_size / 8) > 32)
            interleave_mode = 1;
        else if (intile_xsize_IL * (data_size / 8) > 16)
            interleave_mode = 2;
        else
            interleave_mode = 4;

        hgap = inimage_stride_IL - intile_xsize_IL;
        vgap = inimage_slice_IL - (min(interleave_mode, intile_ysize) * inimage_stride_IL - hgap);
    }



    factor = min((double)kernel_per_core * cores / z, 1) * image_compress_ratio;
    Merge2DTileEnabled = false;
    SeparateBurstcountBySize(&BurstCount_64B, &BurstCount_128B, /*BurstCount_256B,*/
                                hgap, vgap, intile_xsize_IL, intile_ysize, inx_IL, iny, kz_IL, inimage_stride_IL, inimage_slice_IL,
                                zdp, image_compress_ratio, NonTransposed_1x1,
                                &BurstCount_64B_Standalone, &BurstCount_128B_Standalone, &BurstCount_256B_Standalone);
    SeparateBurstcountBySize(&BurstCount_64B_vzgroup0, &BurstCount_128B_vzgroup0, /*BurstCount_256B_vzgroup0,*/
                                hgap, vgap, intile_xsize_IL, intile_ysize, inx_IL, iny, kz_IL, inimage_stride_IL, inimage_slice_IL,
                                zdp, factor, NonTransposed_1x1,
                                &BurstCount_64B_Standalone_vzgroup0, &BurstCount_128B_Standalone_vzgroup0, &BurstCount_256B_Standalone_vzgroup0);

    bw_byburst_type->BW_64B.cost = BurstCount_64B_Standalone * 64;
    bw_byburst_type->BW_64B.vzGroup0 = BurstCount_64B_Standalone_vzgroup0 * 64;
    bw_byburst_type->BW_64B.tile0VZGroup0 = bw_byburst_type->BW_64B.vzGroup0 * ((double)tile_xsize / x) * ((double)tile_ysize / y);

    bw_byburst_type->BW_128B.cost = BurstCount_128B_Standalone * 128;
    bw_byburst_type->BW_128B.vzGroup0 = BurstCount_128B_Standalone_vzgroup0 * 128;
    bw_byburst_type->BW_128B.tile0VZGroup0 = bw_byburst_type->BW_128B.vzGroup0 * ((double)tile_xsize / x) * ((double)tile_ysize / y);

    bw_byburst_type->BW_256B.cost = BurstCount_256B_Standalone * 256;
    bw_byburst_type->BW_256B.vzGroup0 = BurstCount_256B_Standalone_vzgroup0 * 256;
    bw_byburst_type->BW_256B.tile0VZGroup0 = bw_byburst_type->BW_256B.vzGroup0 * ((double)tile_xsize / x) * ((double)tile_ysize / y);

    return 0;
}


static unsigned int SplitIntoFourRegions_Read(APM_BW_CYCLE_COST_T *ByRegion_Type, unsigned int tile_xsize, unsigned int tile_ysize,
                                              unsigned int kernel_per_cores,unsigned int x, unsigned int y, unsigned z, unsigned int cores)
{
    /* fill struction */
    if(ByRegion_Type->vzGroup0 != 0)    /* Solely for DDR ImageReadBW because image need to be cached when processing vzgroup0 */
    {
        ByRegion_Type->tile0 = ByRegion_Type->cost * ((float)tile_xsize / x) * ((float)tile_ysize / y);
        ByRegion_Type->tile0VZGroup0 = ByRegion_Type->vzGroup0 * ((float)tile_xsize / x) * ((float)tile_ysize / y);
    }
    else if(ByRegion_Type->tile0 != 0)  /* Solely for DDR KernelReadBW because kernel need to be cached when processing tile 0 */
    {
        ByRegion_Type->vzGroup0 = ByRegion_Type->cost * min((float)kernel_per_cores * cores / z, 1);
        ByRegion_Type->tile0VZGroup0 = ByRegion_Type->tile0 * min((float)kernel_per_cores * cores / z, 1);
    }
    else  /* mainly for cyclecount */
    {
        ByRegion_Type->tile0 = ByRegion_Type->cost * ((float)tile_xsize / x) * ((float)tile_ysize / y);
        ByRegion_Type->vzGroup0 = ByRegion_Type->cost * min((float)kernel_per_cores * cores / z, 1);
        ByRegion_Type->tile0VZGroup0 = ByRegion_Type->cost * ((float)tile_xsize / x) * ((float)tile_ysize / y) * min((float)kernel_per_cores * cores / z, 1);
    }

    ByRegion_Type->tile0ResetVZGroup = ByRegion_Type->tile0 - ByRegion_Type->tile0VZGroup0;
    ByRegion_Type->resetTileVZGroup0 = ByRegion_Type->vzGroup0 - ByRegion_Type->tile0VZGroup0;
    ByRegion_Type->resetTileResetVZGroup = ByRegion_Type->cost - ByRegion_Type->tile0 - ByRegion_Type->vzGroup0 + ByRegion_Type->tile0VZGroup0;

    return 0;
}

static unsigned int SplitIntoFourRegions_Write(APM_BW_CYCLE_COST_T *ByRegion_Type, unsigned int tile_xsize, unsigned int tile_ysize,
                                              unsigned int kernel_per_cores,unsigned int x, unsigned int y, unsigned z, unsigned int cores)
{
    double tail_x = 0, tail_y = 0,tail_z = 0;

    tail_x = x % tile_xsize;
    tail_y = y % tile_ysize;
    tail_z = z % (kernel_per_cores * cores);
    if(tail_x == 0)
    {
       tail_x = tile_xsize;
    }

    if(tail_y == 0)
    {
        tail_y = tile_ysize;
    }

    if(tail_z == 0)
    {
        tail_z = kernel_per_cores * cores;
    }

    /* fill struct */
    ByRegion_Type->tile0 = ByRegion_Type->cost * ((float)tile_xsize / x) * ((float)tile_ysize / y) * (1 - (double)tail_z / z);
    ByRegion_Type->vzGroup0 = ByRegion_Type->cost * (tail_z / z) * (1 - (tail_x) / x * (tail_y) / y);
    ByRegion_Type->residual = ByRegion_Type->cost * (tail_z) / z * (tail_x) / x * (tail_y) / y;
    ByRegion_Type->tile0VZGroup0 = 0;
    ByRegion_Type->tile0ResetVZGroup = ByRegion_Type->tile0;
    ByRegion_Type->resetTileVZGroup0 = ByRegion_Type->vzGroup0;
    ByRegion_Type->resetTileResetVZGroup = ByRegion_Type->cost - ByRegion_Type->tile0 - ByRegion_Type->vzGroup0;

    return 0;
}

static int assertCheck = 0;

unsigned int CheckFourRegions(APM_BW_CYCLE_COST_T *ByRegion_Type)
{
    if (assertCheck)
    {
        assert((ByRegion_Type->cost - (ByRegion_Type->tile0VZGroup0 + ByRegion_Type->tile0ResetVZGroup + ByRegion_Type->resetTileVZGroup0 + ByRegion_Type->resetTileResetVZGroup)) < 1
            && (ByRegion_Type->cost - (ByRegion_Type->tile0VZGroup0 + ByRegion_Type->tile0ResetVZGroup + ByRegion_Type->resetTileVZGroup0 + ByRegion_Type->resetTileResetVZGroup)) > -1);
        assert(ByRegion_Type->tile0VZGroup0 >= -0.1 && ByRegion_Type->tile0ResetVZGroup >= -0.1 && ByRegion_Type->resetTileVZGroup0 >= -0.1 && ByRegion_Type->resetTileResetVZGroup >= -0.1);
    }
    //ByRegion_Type = ByRegion_Type;
    return 0;
}

static APM_BW_CYCLE_COST_T Type_ByRegion_Add(
    APM_BW_CYCLE_COST_T t1,
    APM_BW_CYCLE_COST_T t2 /*,
    APM_BW_CYCLE_COST_T *result*/
    )
{
    APM_BW_CYCLE_COST_T result = {0};

    result.cost = t1.cost + t2.cost;
    result.tile0 = t1.tile0 + t2.tile0;
    result.vzGroup0 = t1.vzGroup0 + t2.vzGroup0;
    result.tile0VZGroup0 = t1.tile0VZGroup0 + t2.tile0VZGroup0;
    result.tile0ResetVZGroup = t1.tile0ResetVZGroup + t2.tile0ResetVZGroup;
    result.resetTileVZGroup0 = t1.resetTileVZGroup0 + t2.resetTileVZGroup0;
    result.resetTileResetVZGroup = t1.resetTileResetVZGroup + t2.resetTileResetVZGroup;

    return result;
}

// refine me
static APM_BW_CYCLE_COST_T Type_ByRegion_Multi(
    APM_BW_CYCLE_COST_T t1,
    double num
    /*, APM_BW_CYCLE_COST_T * result*/
    )
{
    APM_BW_CYCLE_COST_T result = {0};
    result.cost = t1.cost *num;
    result.tile0 = t1.tile0 * num;
    result.vzGroup0 = t1.vzGroup0 * num;
    result.tile0VZGroup0 = t1.tile0VZGroup0 * num;
    result.tile0ResetVZGroup = t1.tile0ResetVZGroup * num;
    result.resetTileVZGroup0 = t1.resetTileVZGroup0 * num;
    result.resetTileResetVZGroup = t1.resetTileResetVZGroup * num;

    return result;
}

/*
 * WriteBandWidth is in bytes
 */
static double WriteBandWidth(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int x,
    unsigned int y,
    unsigned int outZ,
    unsigned int data_size,
    double image_compress_ratio,
    unsigned int usc_cache_size,
    unsigned int pooling_stride,
    unsigned int outimage_stride,
    unsigned int outimage_slice,
    unsigned int is_nn_write_without_usc,
    unsigned int dst_buf,
    unsigned int burst_size
    )
{
    unsigned int PooledX, PooledY, PooledTileX, PooledTileY, HGap, VGap;
    double cacheSizeInPixel;
    double AllTilesBW, rowTilesBW, tileBW, WriteBandWidth;
    unsigned int pooledTileX_tail = 0,pooledTileY_tail = 0,rowOftilesBW_tail = 0;
    unsigned int WriteBandWidth_tailx = 0, WriteBandWidth_taily = 0, WriteBandWidth_corner = 0;
    bool isV8 = (g_xydp_x == 0 && g_xydp_y == 0) ? true : false;

    cacheSizeInPixel = usc_cache_size * 1024 / ((double)data_size / 8);

    if(burst_size == 0)
    {
        if (dst_buf == SW_TILING_FROM_DDR)
        {
            if (pContext->bf.NN_LARGE_BURST_SIZE == 1)
            {
                burst_size = pContext->pInParams->NN_DDR_BURST_SIZE;
            }
            else
            {
                burst_size = 64;
            }
        }
        else if (dst_buf == SW_TILING_FROM_AXI_SRAM)
        {
            burst_size = AXI_BURST_SIZE;
        }
        else
        {
            burst_size = pContext->p_hwInfo->pFeatures->EQUIVALENT_VIP_SRAM_WIDTH_INBYTE * 2;
        }
        SpecifiedBurstSize = false;
    }
    else
        SpecifiedBurstSize = true;
    float PPC = burst_size / ((float)data_size / 8);

    assert(x >= pooling_stride);
    assert(y >= pooling_stride);

    unsigned int z = outZ;
    unsigned int outTrspIvch = pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_out;
    if (outTrspIvch > 1 && ((dst_buf == SW_TILING_FROM_DDR) || (dst_buf == SW_TILING_FROM_AXI_SRAM)))
    {
        z = (unsigned int)ceil((float)z / outTrspIvch); // use ceil to model it
        x *= outTrspIvch;
        tile_xsize *= outTrspIvch;
        outimage_stride *= outTrspIvch;
        outimage_slice *= outTrspIvch;
    }

    PooledX = x / pooling_stride;
    PooledY = y / pooling_stride;
    PooledTileX = tile_xsize / pooling_stride;
    PooledTileY = tile_ysize / pooling_stride;

    unsigned int SI_hgap = outimage_stride - PooledX;
    unsigned int SI_vgap = outimage_slice  - PooledY * outimage_stride + SI_hgap;
    if (SI_vgap < PPC)
    {
        AllTilesBW = UnalignedBW2(outimage_slice * z, outimage_slice * z, outimage_slice * z, (unsigned int)PPC) * image_compress_ratio;
    }
    else if (SI_hgap < PPC)
    {
        AllTilesBW = UnalignedBW2(PooledY * outimage_stride - SI_hgap, outimage_slice, outimage_slice, (unsigned int)PPC) * z * image_compress_ratio;
    }
    else
    {
        AllTilesBW = UnalignedBW2(PooledX, outimage_stride, outimage_slice, (unsigned int)PPC) * PooledY * z * image_compress_ratio;
    }

    if (( (((unsigned int)(outimage_stride * PooledTileY) % (unsigned int)PPC) == 0) || (((unsigned int)PPC % (unsigned int)(outimage_stride * PooledTileY)) == 0))
        && (PooledTileX == PooledX)
        && ((outimage_slice % (unsigned int)PPC) == 0))
    {
        rowTilesBW = ceilf(((float)outimage_stride * (PooledTileY - 1) + PooledTileX) / PPC) * PPC * z * image_compress_ratio;
        pooledTileY_tail = PooledY % PooledTileY;
        rowOftilesBW_tail = (unsigned int)(ceilf((outimage_stride * pooledTileY_tail) / PPC) * PPC * z * image_compress_ratio);

    }
    else
    {
        rowTilesBW = UnalignedBW2((outimage_stride * (PooledTileY - 1) + PooledTileX), outimage_stride * PooledTileY, outimage_slice, (unsigned int)PPC) * z * image_compress_ratio;
        pooledTileY_tail = PooledY % PooledTileY;
        rowOftilesBW_tail = (unsigned int)(UnalignedBW2((outimage_stride * pooledTileY_tail), outimage_stride * PooledTileY, outimage_slice, (unsigned int)PPC) * z * image_compress_ratio);

    }

    if (((((outimage_stride % (unsigned int)PooledTileX) == 0) && (((unsigned int)PPC % (unsigned int)PooledTileX) == 0))
          || (((outimage_stride % (unsigned int)PPC) == 0) && (PPC >= PooledX)))
       && ((outimage_slice % (unsigned int)PPC) == 0))
    {
        tileBW     = ceilf(PooledTileX / PPC) * PPC * PooledTileY * z * image_compress_ratio;
    }
    else
    {
        tileBW     = UnalignedBW2(PooledTileX, outimage_stride, outimage_slice, (unsigned int)PPC) * PooledTileY * z * image_compress_ratio;
    }

    HGap = outimage_stride - PooledTileX;
    VGap = outimage_slice - PooledTileY * outimage_stride + HGap;
    if (!isV8)
    {
        if (!is_nn_write_without_usc && (tileBW < (cacheSizeInPixel / 2) || rowTilesBW < (cacheSizeInPixel / 2)))
        {
            WriteBandWidth = AllTilesBW;
        }
        else if (!is_nn_write_without_usc && (tileBW < cacheSizeInPixel || rowTilesBW < cacheSizeInPixel))
        {
            WriteBandWidth = AllTilesBW * 1.2f;
        }
        else if (VGap < PPC)
        {
            WriteBandWidth = AllTilesBW * ceilf((float)PooledX / PooledTileX) * ((float)PooledY / PooledTileY);
        }
        else if (HGap < PPC)
        {
            WriteBandWidth = rowTilesBW * ceilf((float)PooledX / PooledTileX) * ((float)PooledY / PooledTileY);
        }
        else
        {
            WriteBandWidth = tileBW * ceilf((float)PooledX / PooledTileX) * ((float)PooledY / PooledTileY);
        }
    }
    else /* VIP V8 */
    {
        bool ACCURATE_PROFILE = 0;
        if (ACCURATE_PROFILE) //may slow down modeling process
        {
            WriteBandWidth = 0;
            int outimage_addr  = 0; // refine me: this should be offset
            int tileXnum = PooledX / PooledTileX;
            int tileYnum = PooledY / PooledTileY;
            for (int TileYIdx = 0; TileYIdx != tileYnum; TileYIdx++)
            {
                for (int TileXIdx = 0; TileXIdx != tileXnum; TileXIdx++)
                {
                    int curPooledTileX = min(PooledX - TileXIdx * PooledTileX, PooledTileX);
                    int curPooledTileY = min(PooledY - TileYIdx * PooledTileY, PooledTileY);
                    HGap = outimage_stride - min(outimage_stride - TileXIdx * PooledTileX, PooledTileX);
                    VGap = outimage_slice - min(outimage_slice - TileYIdx * PooledTileY * outimage_stride, PooledTileY * outimage_stride) + HGap;
                    if (VGap < PPC)
                    {
                        WriteBandWidth = WriteBandWidth + ceilf((((outimage_addr + TileYIdx * PooledTileY * outimage_stride + TileXIdx * PooledTileX) % (unsigned int)PPC) + (curPooledTileY * outimage_stride - HGap) + outimage_slice * (z - 1)) / PPC) * PPC;// ' Contiguous Write in 3D tile, so the whole image is written contiguously
                        //'Write request size is first_slice_write_size+last_slice_write_size+rest_slice_write_size = (outimage_slice-TileYIdx*PooledTileY*outimage_stride-TileXIdx*PooledTileX) + (TileYIdx*PooledTileY*outimage_stride+TileXIdx*PooledTileX + curPooledTileY*outimage_stride-HGap) + outimage_slice*(z-2)
                    }
                    else if (HGap < PPC)
                    {
                        for (unsigned int ZIdx = 0; ZIdx != z; ZIdx++)
                        {
                            WriteBandWidth = WriteBandWidth + ceilf((((outimage_addr + ZIdx * outimage_slice + TileYIdx * PooledTileY * outimage_stride + TileXIdx * PooledTileX) % (unsigned int)PPC) + curPooledTileY * outimage_stride - HGap) / PPC) * PPC; // ' Contiguous Write in 2D tile
                        }
                    }
                    else
                    {
                        for (unsigned int ZIdx = 0; ZIdx != z; ZIdx++)
                        {
                            for (int YIdx = 0; YIdx != curPooledTileY; YIdx++)
                            {
                                WriteBandWidth = WriteBandWidth + ceilf((((outimage_addr + ZIdx * outimage_slice + (TileYIdx * PooledTileY + YIdx) * outimage_stride + TileXIdx * PooledTileX) % (unsigned int)PPC) + curPooledTileX) / PPC) * PPC; //  ' all lines in 2D tile is not merged
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // original model
            unsigned int tile_count_h = 0,tile_count_v = 0, vgap_tailx= 0, hgap_tailx = 0, vgap_taily = 0, hgap_taily = 0, hgap_corner = 0, vgap_corner = 0;
            pooledTileX_tail = PooledX % PooledTileX;
            pooledTileY_tail = PooledY % PooledTileY;
            tile_count_h = PooledX / PooledTileX;
            tile_count_v = PooledY / PooledTileY;
            /* calculate tail_x */
            if(pooledTileX_tail > 0)
            {
                vgap_tailx = VGap + (PooledTileX - pooledTileX_tail);
                hgap_tailx = HGap + (PooledTileX - pooledTileX_tail);

                if( vgap_tailx < PPC)
                {
                    WriteBandWidth_tailx = (unsigned int)(UnalignedBW2(outimage_slice * z - vgap_tailx, outimage_slice * z, outimage_slice * z, (unsigned int)PPC)) * tile_count_v;
                }
                else if (hgap_tailx < PPC)
                {
                    WriteBandWidth_tailx = (unsigned int)(UnalignedBW2(outimage_slice - vgap_tailx, outimage_stride * PooledTileY, outimage_slice, (unsigned int)PPC) * z) * tile_count_v;
                }
                else
                {
                    WriteBandWidth_tailx = (unsigned int)UnalignedBW2(pooledTileX_tail, outimage_stride, outimage_slice, (unsigned int)PPC) * PooledTileY * tile_count_v * z;
                }
            }
            else
            {
                WriteBandWidth_tailx = 0;
            }

            /* calculate tail_y */
            if (pooledTileY_tail > 0)
            {
                vgap_taily = VGap + (PooledTileY - pooledTileY_tail) * outimage_stride;
                hgap_taily = HGap;

                if (vgap_taily < PPC)
                {
                    WriteBandWidth_taily = (unsigned int)(UnalignedBW2(outimage_slice * z - vgap_taily, outimage_slice * z, outimage_slice * z, (unsigned int)PPC)) * tile_count_h;
                }
                else if (hgap_taily < PPC)
                {
                    WriteBandWidth_taily = (unsigned int)(UnalignedBW2(outimage_slice - vgap_taily, outimage_stride * pooledTileY_tail, outimage_slice, (unsigned int)PPC) * z) * tile_count_h;
                }
                else
                {
                    WriteBandWidth_taily = (unsigned int)UnalignedBW2(PooledTileX, outimage_stride, outimage_slice, (unsigned int)PPC) * pooledTileY_tail * tile_count_h * z;
                }
            }
            else
            {
                WriteBandWidth_taily = 0;
            }

            /* calculate corner */
            if (pooledTileX_tail > 0 && pooledTileY_tail > 0)
            {
                hgap_corner = hgap_tailx;
                vgap_corner = (PooledY / PooledTileY) * PooledTileY * outimage_stride + hgap_corner;

                if (vgap_corner < PPC)
                {
                    WriteBandWidth_corner = (unsigned int)(UnalignedBW2(outimage_slice * z - vgap_corner, outimage_slice * z, outimage_slice * z, (unsigned int)PPC));
                }
                else if (hgap_corner < PPC)
                {
                    WriteBandWidth_corner = (unsigned int)(UnalignedBW2(outimage_slice - vgap_corner, outimage_stride * PooledTileY, outimage_slice, (unsigned int)PPC) * z);
                }
                else
                {
                    WriteBandWidth_corner = (unsigned int)UnalignedBW2(pooledTileX_tail, outimage_stride, outimage_slice, (unsigned int)PPC) * pooledTileY_tail * z;
                }
            }
            else
            {
                WriteBandWidth_corner = 0;
            }

            /* calculate total */
            if (VGap < PPC)
            {
                WriteBandWidth = UnalignedBW2(outimage_slice * z - VGap, outimage_slice * z, outimage_slice * z, (unsigned int)PPC) * tile_count_h * tile_count_v + WriteBandWidth_tailx + WriteBandWidth_taily + WriteBandWidth_corner;
            }
            else if (HGap < PPC)
            {
                WriteBandWidth = UnalignedBW2(outimage_slice - VGap, outimage_stride * PooledTileY, outimage_slice, (unsigned int)PPC) * z * tile_count_h * tile_count_v + WriteBandWidth_tailx + WriteBandWidth_taily + WriteBandWidth_corner;
            }
            else
            {
                WriteBandWidth = UnalignedBW2(PooledTileX, outimage_stride, outimage_slice, (unsigned int)PPC) * PooledTileY * z * tile_count_h * tile_count_v + WriteBandWidth_tailx + WriteBandWidth_taily + WriteBandWidth_corner;
            }
        }
    }

    WriteBandWidth = WriteBandWidth * (data_size / 8);

    if(dst_buf == SW_TILING_FROM_DDR && SpecifiedBurstSize == false)
    {
        WriteBandWidth = WriteBandWidth * pContext->pInParams->NN_DDR_BURST_SIZE / burst_size;
    }

    return WriteBandWidth;
}

// model bug read write accumulation bubble
// bug ID:
double rw_accum_bubble(
    unsigned int kx,
    unsigned int ky,
    /*unsigned int kz,*/
    unsigned int tile_ysize,
    unsigned int kernel_per_core,
    unsigned int xydp_x,
    /*unsigned int zdp,*/
    unsigned int interleave_mode,
    double non_zero_ratio,
    double org3DCC // roginal 3D tile Compute Cycle

    )
{
    double new3DCCWithBubble = org3DCC;

    int i_rotate_num = 0;
    if (xydp_x != 1)
    {
        i_rotate_num = (int)ceilf( (float)kx / xydp_x) * ky;
    }

    double rotate_num = i_rotate_num * non_zero_ratio;

    double cycle_per_input_2dtile     =  ceilf((float)tile_ysize / interleave_mode) * rotate_num * kernel_per_core;
    double bubbles_per_rotatedGroup = max(3 + rotate_num - tile_ysize * rotate_num, 0);
    double bubble_per_input_2dtile = bubbles_per_rotatedGroup * kernel_per_core;
    new3DCCWithBubble = org3DCC * (1 + ((double)bubble_per_input_2dtile / cycle_per_input_2dtile));


    return new3DCCWithBubble;
}

double ComputeCycleCount(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int kernel_per_core,
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int x,
    unsigned int y,
    unsigned int z,
    double non_zero_ratio,
    unsigned int xydp_x,
    unsigned int xydp_y,
    unsigned int zdp,
    unsigned int float_xydp_x,
    unsigned int float_xydp_y,
    unsigned int float_zdp,
    unsigned int data_size,
    unsigned int vector_prune,
    unsigned int interleave_mode,
    unsigned int lanes_per_conv,
    /*unsigned int coef_decode_perf,*/
    unsigned int zrl_bits,
    unsigned int is_depth_wise,
    unsigned int vip_v7_16bit,
    unsigned int fp16,
    unsigned int conv1x1_half_performance,
    unsigned int zxdp3_kernel_read_conflict_fix,
    unsigned int single_port_acc_buffer,
    double *refined_non_zero_ratio
    )
{
    unsigned int tmp, pipeLatency;
    int dpAmount;
    unsigned int dpKX, dpKY, dpKZ;
    double accumCycle, tile3DComputeCycle, bottomTile3DComputeCycle;
    double dpNonZeroRatio = 1.0;
    double computeCycle;
    unsigned int xydpVectorPruneAmount, zdpVectorPruneAmount, numOfPruneGroupsInDpn, numOfDpnInImgBuf;
    unsigned int selected_xydp_x, selected_xydp_y, selected_zdp;

    bool IMG_POP_PIPELINE_PAUSE_BUG2029 = pContext->bf.IMG_POP_PIPELINE_PAUSE_BUG2029;
    if (xydp_x == 0 || xydp_y == 0) /*zdp only arch*/
    {
        bool Non_Pooling_Pack_1x1   = pContext->bf.Non_Pooling_Pack_1x1;
        bool MxN_interleave_pooling = pContext->bf.MxN_interleave_pooling;

        if (   ((kx * ky == 1) && (pContext->pooling_stride >  1))
            || ((kx * ky == 1) && (pContext->pooling_stride == 1) && (Non_Pooling_Pack_1x1 == 0))
            || ((kx * ky >  1) && (pContext->pooling_stride >  1) && (MxN_interleave_pooling == 1))
            )
        {
            /* Merge from VB but it seems may not correct, need to double check  */
            tile3DComputeCycle = ceilf((float)tile_ysize / interleave_mode) * (kernel_per_core + (float)IMG_POP_PIPELINE_PAUSE_BUG2029 / ZDP_LOOP_COUNT);
            bottomTile3DComputeCycle = ceilf(1.0f * (y % tile_ysize) / interleave_mode) * (kernel_per_core + (float)IMG_POP_PIPELINE_PAUSE_BUG2029 / ZDP_LOOP_COUNT);
        }
        else
        {
            tile3DComputeCycle = ceilf(((float)tile_ysize * tile_xsize) / (pContext->pooling_stride * lanes_per_conv)) * pContext->pooling_stride * \
                                 (kernel_per_core + (float)IMG_POP_PIPELINE_PAUSE_BUG2029/ZDP_LOOP_COUNT);
            bottomTile3DComputeCycle = ceilf((1.0f * (y % tile_ysize) * tile_xsize ) / (pContext->pooling_stride * lanes_per_conv)) * pContext->pooling_stride * \
                                       (kernel_per_core + (float)IMG_POP_PIPELINE_PAUSE_BUG2029/ZDP_LOOP_COUNT);
        }

        dpNonZeroRatio = 1 - pow((double)1-non_zero_ratio, (int)zdp);

        if (vector_prune > 0)
        {
            dpNonZeroRatio = non_zero_ratio;
        }
        computeCycle = tile3DComputeCycle *(unsigned int)(y / tile_ysize) + bottomTile3DComputeCycle;

        unsigned int adj_z = z;
        if(pContext->bf.COEF_ZERO_POINT_AREA_OPTIMIZATION && pContext->pInPerfParams->misc.asymmetric_quantization)
        {
            adj_z += 1; // for sum1 channal
        }

        bool isDepthWiseMerge = pContext->pInPerfParams->cmdInfo.is_depth_wise_merge;
        if (is_depth_wise)
        {
            computeCycle = computeCycle * ceilf((float)kx * ky / zdp) * adj_z * ceilf((float)x / tile_xsize) / kernel_per_core;
        }
        else if (isDepthWiseMerge)
        {
            computeCycle = computeCycle * ceilf((float)kz / zdp) * adj_z * dpNonZeroRatio * ceil((float)x / tile_xsize) / kernel_per_core;
        }
        else
        {
            computeCycle = computeCycle * ceilf((float)kx * ky * kz / zdp) * adj_z * dpNonZeroRatio * ceilf((float)x / tile_xsize) / kernel_per_core;
        }

        if (data_size == 16) /*INT16; FP16 support added*/
        {
            computeCycle *= (fp16 == 1) ? 2 : 4;
        }

        *refined_non_zero_ratio = non_zero_ratio;
        return computeCycle;
    }
    bool vip_v7 = (xydp_x != 0) && (xydp_y != 0);
    bool vip_v7_fc = (vip_v7 && kx == 1 && ky != 1);
    bool vip_v7_dp6 = (vip_v7 && xydp_x == 3 && xydp_y == 2);
    bool vip_v7_xdp3 = (vip_v7 && xydp_x == 3 && xydp_y == 1);
    bool vip_v7_zdp3 = (vip_v7 && zdp == 3 && kx * ky == 1);
    unsigned int rotate_cell;
    if (fp16 == 1)
    {
        selected_xydp_x = float_xydp_x;
        selected_xydp_y = float_xydp_y;
        selected_zdp = float_zdp;
    }
    else
    {
        selected_xydp_x = xydp_x;
        selected_xydp_y = xydp_y;
        selected_zdp = zdp;
    }

    if (vector_prune > 0)
    {
        xydpVectorPruneAmount = selected_xydp_x * selected_xydp_y;
        zdpVectorPruneAmount = 2 * selected_zdp;
    }
    else
    {
        xydpVectorPruneAmount = 1;
        zdpVectorPruneAmount = 1;
    }

    if (kx != 1 || ky != 1)
    {
        dpKX = (unsigned int)ceilf((float)kx / selected_xydp_x);
        dpKY = (unsigned int)ceilf((float)ky / selected_xydp_y);
        dpKZ = kz;
        dpAmount = selected_xydp_x * selected_xydp_y;
        numOfPruneGroupsInDpn = (unsigned int)ceilf((float)selected_xydp_x * selected_xydp_y / xydpVectorPruneAmount);
        numOfDpnInImgBuf = (unsigned int)ceilf((float)kx / selected_xydp_x) * (unsigned int)ceilf((float)ky / selected_xydp_y);

        if (vector_prune > 0)
        {
            non_zero_ratio = max(non_zero_ratio, (double)((double)min(selected_xydp_x, kx) * min(selected_xydp_y, ky) / (pow(2, (float)zrl_bits) - 1)));
        }
    }
    else
    {
        dpKX = kx;
        dpKY = ky;
        if (selected_zdp > 1)
        {
            if (single_port_acc_buffer)
            {
                dpKZ = (unsigned int)ceilf((float)kz / (2 * selected_zdp)) * 2;
                dpAmount = 2 * selected_zdp;
                numOfPruneGroupsInDpn = (unsigned int)ceilf((float)2 * selected_zdp / zdpVectorPruneAmount);
                numOfDpnInImgBuf = 2;
            }
            else
            {
                dpKZ = (unsigned int)ceilf((float)kz / selected_zdp);
                dpAmount = selected_zdp;
                numOfPruneGroupsInDpn = (unsigned int)ceilf((float)selected_zdp / zdpVectorPruneAmount);
                numOfDpnInImgBuf = 1;
            }
        }
        else
        {
            dpKZ = kz;
            dpAmount = 1;
            numOfPruneGroupsInDpn = 1;
            numOfDpnInImgBuf = 1;
        }
        if (vector_prune > 0)
        {
            non_zero_ratio = max(non_zero_ratio, (double)selected_zdp / ((double)pow(2, (float)zrl_bits) - 1));
        }
    }
    for (tmp = 0; tmp < numOfPruneGroupsInDpn; tmp++)
    {
        dpNonZeroRatio *= (1.0f - non_zero_ratio);
    }
    dpNonZeroRatio = 1.0f - dpNonZeroRatio;

    if (single_port_acc_buffer)
    {
        /* find the probably of exactly 1 non-zero DPN operation in a 2D kernel or in a 2*ZDP region
        the cycle count of this 1 non-zero DPN operation needs to be doubled because of single port accum buffer RW conflict*/
        double probExactlyOneNonZeroDpn, tmpRatio = 1.0f;
        for (tmp = 0; tmp < (numOfDpnInImgBuf - 1); tmp++)
        {
            tmpRatio *= (1.0f - dpNonZeroRatio);
        }
        probExactlyOneNonZeroDpn = dpNonZeroRatio * tmpRatio;
        dpNonZeroRatio = dpNonZeroRatio + probExactlyOneNonZeroDpn;
    }

    if (vip_v7_16bit && (fp16 == 0))
    {
        unsigned int int16OutSideConvCore = false;
        if (int16OutSideConvCore == 1)
        {
            dpKZ = (kz * 2);
        }
        else
        {
            dpKY = ky;
            dpKZ = kz;
        }
    }

    if (vip_v7_fc ||
        vip_v7_zdp3 ||
        (vip_v7_dp6 && (kx == 3) && (ky == 3)) ||
        (vip_v7_xdp3 && (kx == 2) && (ky == 2)))
    {
        rotate_cell = 2;
    }
    else if ((vip_v7_dp6 && (kx == 2) && (kx == 2)) ||
        (zdp == 1 && kx * ky == 1) ||
        xydp_x == 1)
    {
        rotate_cell = 1;
    }
    else
    {
        rotate_cell = 3;
    }

    pipeLatency = (rotate_cell == 1) ? 4 : 6;

    int rotate_group_num = (int)ceilf((float)dpKX * dpKY / rotate_cell);
    if (vip_v7_16bit)
        pipeLatency += 2;

    accumCycle = ceilf((float)tile_ysize / interleave_mode);

    if (zdp == 1 && xydp_x == 1 && xydp_y == 1 && accumCycle == 4)
    {
        tile3DComputeCycle = 4 * (float)kernel_per_core;
    }
    else if (rotate_cell == 1 || vip_v7_zdp3 || !vip_v7)
    {
        tile3DComputeCycle = max(ceilf((float)tile_ysize / interleave_mode) * kernel_per_core * rotate_cell, pipeLatency);
    }
    else
    {
        if (ceilf((float)tile_ysize / interleave_mode) * rotate_cell > pipeLatency)
        {
            tile3DComputeCycle = ceilf((float)tile_ysize / interleave_mode) * rotate_cell * rotate_group_num * kernel_per_core;
        }
        else
        {
            tile3DComputeCycle = (pipeLatency * (rotate_group_num - 1) + ceilf((float)tile_ysize / interleave_mode) * rotate_cell) * kernel_per_core;
        }
    }

    //tile3DComputeCycle = rw_accum_bubble(kx, ky, tile_ysize, kernel_per_core, xydp_x, /*zdp,*/ interleave_mode, non_zero_ratio, tile3DComputeCycle);

    tmp = y % tile_ysize;
    if (tmp != 0)
    {
        accumCycle = ceilf((float)tmp / interleave_mode);

        if (zdp == 1 && xydp_x == 1 && xydp_y == 1 && accumCycle == 4)
        {
            bottomTile3DComputeCycle = 4 * (float)kernel_per_core;
        }
        else if (rotate_cell == 1 || vip_v7_zdp3 || !vip_v7)
        {
            bottomTile3DComputeCycle = max(ceilf((float)tmp / interleave_mode) * kernel_per_core *rotate_cell, pipeLatency);
        }
        else
        {
            bottomTile3DComputeCycle = max(ceilf((float)tmp / interleave_mode) * rotate_cell, pipeLatency) * kernel_per_core;
        }

        // rw_accum_sync_pause bubble for bottom 3D tile, timeYsize is tmp
        //bottomTile3DComputeCycle = rw_accum_bubble(kx, ky, tmp, kernel_per_core, xydp_x, interleave_mode, non_zero_ratio, tile3DComputeCycle);
    }
    else
    {
        bottomTile3DComputeCycle = 0;
    }


    computeCycle = tile3DComputeCycle * (int)(y / tile_ysize) + bottomTile3DComputeCycle;
    if (is_depth_wise)
    {
        computeCycle = computeCycle * ceilf((float)dpKX * dpKY / rotate_cell) * z * ceilf((float)x / tile_xsize) / kernel_per_core;
    }
    else if (!vip_v7 || vip_v7_zdp3)
    {
        computeCycle = computeCycle * ceilf((float)dpKX * dpKY * dpKZ / rotate_cell) * z * dpNonZeroRatio * ceilf((float)x / tile_xsize) / kernel_per_core;
    }
    else
    {
        computeCycle = computeCycle * dpKZ * z * dpNonZeroRatio * ceilf((float)x / tile_xsize) / kernel_per_core;
    }
    if (kx == 1 && ky == 1 && conv1x1_half_performance && selected_zdp == 1)
    {
        computeCycle = computeCycle * 2;
    }
    else if (!zxdp3_kernel_read_conflict_fix && single_port_acc_buffer && kx <=2 && ky <= 2)
    {
        computeCycle = (computeCycle * 12/10);
    }
    *refined_non_zero_ratio = non_zero_ratio;

    //if ((data_size == 16) && (fp16 == 0) )
    //{
    //    //' INT16 only; no FP16 support
    //  computeCycle = computeCycle * 4;
    //}
    //else if (fp16 == 1)
    //{
    //   computeCycle = computeCycle * 2;
    //}

    return computeCycle;
}

void outputBandwidthInfo (
    APM_OUT_BW_T * outBandWidth,
    APM_COST_T nnCost[]
    )
{
    outBandWidth->ddr_kernel_read_bandwidth = nnCost[APM_DDR_KERNEL_COST].readBW.cost;
    outBandWidth->ddr_in_image_read_bandwidth = nnCost[APM_DDR_IN_IMAGE_COST].readBW.cost;
    outBandWidth->ddr_read_bandwidth  = nnCost[APM_DDR_COST].readBW.cost;
    outBandWidth->ddr_write_bandwidth = nnCost[APM_DDR_COST].writeBW.cost;
    outBandWidth->axi_read_bandwidth = nnCost[APM_AXI_SRAM_COST].readBW.cost;
    outBandWidth->axi_write_bandwidth = nnCost[APM_AXI_SRAM_COST].writeBW.cost;
}

/* refine me, this function just pass CycleCounts to APM_BW_CYCLE_COST_T */
void outputCycleInfo(
    APM_OUT_RESULT_T * outResult,
    CycleCounts Cycles,
    APM_BW_CYCLE_COST_T nnCycleCost
    )
{
    ////outResult->warType     = ;
    outResult->computeCC   = Cycles.Compute;
    outResult->ddrRdCC     = Cycles.DDRRead;
    outResult->ddrWrCC     = Cycles.DDRWrite;
    outResult->axiSramRdCC = Cycles.AXISRAMRead;
    outResult->axiSramWrCC = Cycles.AXISRAMWrite;
    outResult->axiBusRdCC  = Cycles.AXIBUSRead;
    outResult->axiBusWrCC  = Cycles.AXIBUSWrite;
    outResult->axiBusTotalCC = Cycles.AXIBUSTotal;
    outResult->vipSramRdCC = Cycles.VIPSRAMRead;
    outResult->vipSramWrCC = Cycles.VIPSRAMWrite;
    outResult->slowInternalWrCC = Cycles.Slow_InternalWrite;
    outResult->slowCompCC       = Cycles.Slow_Comp;
    outResult->internalWrCC     = Cycles.InternalWrite; // same as slowInternalWrCC?
    outResult->dWOutCC          = Cycles.DWOut;
    outResult->kernelDdrRdCC    = Cycles.KernelDDRRead;
    outResult->inImageDdrRdCC   = Cycles.InImageDDRRead;
    outResult->kernelDecodeCC   = Cycles.KernelDecodeBW;
    outResult->dqArbCC          = Cycles.DQArb;
    outResult->regTile2DxBarCC  = Cycles.RegTile2DXBar;
    outResult->bottomTile2DXBarCC = Cycles.BottomTile2DXbar;
    outResult->xBarCC             = Cycles.XBAR;
    outResult->cacheControllerCC  = Cycles.CacheController;
    outResult->overHeadsCC        = Cycles.Overheads;
    outResult->overAllCC          = Cycles.Overall;
    ///* region cycles/Bottleneck */

    outResult->cyclesTile0Vzgroup0     = nnCycleCost.tile0VZGroup0;
    outResult->cyclesTile0RestVzgroup0 = nnCycleCost.tile0ResetVZGroup;
    outResult->cyclesRestTileVzgroup0  = nnCycleCost.resetTileVZGroup0;
    outResult->cyclesRestTileRestVzgroup0  = nnCycleCost.resetTileResetVZGroup;

    //memcpy(&outResult->BottleneckTile0Vzgroup0, Cycles.BottleNeck, sizeof(Cycles.BottleNeck));

    // is following correct??
    outResult->BN_BottleNeck_e = Cycles.BottleNeck_e;
    //outResult->BN_Tile0Vzgroup0_e = Cycles.BottleNeck_e;
    //outResult->BN_Tile0RestVzgroup0_e = Cycles.BottleNeck_e;
    //outResult->BN_RestTileVzgroup0_e = Cycles.BottleNeck_e;
    //outResult->BN_RestTileRestVzgroup0_e = Cycles.BottleNeck_e;
    // to do: outResult->BN_Tile0RestVzgroup0_e =

    //outResult->BottleneckTile0Vzgroup0     = nnCycleCost.;
    //outResult->BottleneckTile0RestVzgroup0 = Cycles.BottleNeck;
    //outResult->BottleneckRestTileVzgroup0  = ;
    //outResult->BottleneckRestTileRestVzgroup0 = ;
    outResult->DDRRead_Combined_Bursts  = Cycles.DDRRead_Combined_Bursts;
    outResult->DDRWrite_Combined_Bursts = Cycles.DDRWrite_Combined_Bursts;
}

void calcBottleNeckNNCycleCount(
    CycleCounts & Cycles
    )
{
    double BottleNeckNNCycleCount    = 0;
    double CompCycleCount            = Cycles.Compute;
    double KernelDecodeBWCycleCount  = Cycles.KernelDecodeBW;
    double DQArbCycleCount           = Cycles.DQArb; // arbCycleCount
    double XBarCycleCount            = Cycles.XBAR;  //XBarCycleCount
    double DDRReadCycleCount         = Cycles.DDRRead;
    double KernelDDRReadCycleCount   = Cycles.KernelDDRRead;
    double InImageDDRReadCycleCount  = Cycles.InImageDDRRead;
    double AXISRAMReadCycleCount     = Cycles.AXISRAMRead;
    double AXIBusReadCycleCount      = Cycles.AXIBUSRead;
    double VIPSRAMReadCycleCount     = Cycles.VIPSRAMRead;
    double DDRWriteCycleCount        = Cycles.DDRWrite;
    double AXISRAMWriteCycleCount    = Cycles.AXISRAMWrite;
    double AXIBusWriteCycleCount     = Cycles.AXIBUSWrite;
    double VIPSRAMWriteCycleCount    = Cycles.VIPSRAMWrite;
    double DDRTotalCycleCount        = Cycles.DDRTotal;
    double AXISRAMTotalCycleCount    = Cycles.AXISRAMTotal;
    double AXIBusTotalCycleCount     = Cycles.AXIBUSTotal;
    double InternalWriteCycleCount   = Cycles.InternalWrite;
    double RdReturnArbiterCycleCount = Cycles.RdReturnArbiter;
    double InternalKernelReadCycleCount = Cycles.InternalKernelRead;

    BottleNeckNNCycleCount = max(max(max(max(CompCycleCount, KernelDecodeBWCycleCount), DQArbCycleCount), XBarCycleCount), InternalKernelReadCycleCount);
    BottleNeckNNCycleCount = max(max(max(KernelDDRReadCycleCount, InImageDDRReadCycleCount), DDRReadCycleCount), BottleNeckNNCycleCount);
    BottleNeckNNCycleCount = max(max(max(max(BottleNeckNNCycleCount, DDRReadCycleCount), AXISRAMReadCycleCount), AXIBusReadCycleCount), VIPSRAMReadCycleCount);
    BottleNeckNNCycleCount = max(max(max(max(BottleNeckNNCycleCount, DDRWriteCycleCount), AXISRAMWriteCycleCount), AXIBusWriteCycleCount), VIPSRAMWriteCycleCount);
    BottleNeckNNCycleCount = max(max(max(BottleNeckNNCycleCount, DDRTotalCycleCount), AXISRAMTotalCycleCount), AXIBusTotalCycleCount);
    BottleNeckNNCycleCount = max(max(BottleNeckNNCycleCount, InternalWriteCycleCount), RdReturnArbiterCycleCount);
    // todo: to add InternalKernelReadCycleCount in Cycles, it can be bottle net

    char BottleNeck[64] = "";
    // todo refine to show more details? Such as AXI_BUS have more
    if (BottleNeckNNCycleCount == CompCycleCount)
    {
        memcpy(BottleNeck, "COMPUTE", sizeof("COMPUTE"));
        Cycles.BottleNeck_e = COMPUTE;
    }
    else if (BottleNeckNNCycleCount == KernelDecodeBWCycleCount)
    {
        memcpy(BottleNeck, "COEF_DECODE", sizeof("COEF_DECODE"));
        Cycles.BottleNeck_e = COEF_DECODE;
    }
    else if (BottleNeckNNCycleCount == DQArbCycleCount)
    {
        memcpy(BottleNeck, "DQ_ARB", sizeof("DQ_ARB"));
        Cycles.BottleNeck_e = DQ_ARB;
    }
    else if (BottleNeckNNCycleCount == XBarCycleCount)
    {
        memcpy(BottleNeck, "XBAR", sizeof("XBAR"));
        Cycles.BottleNeck_e = XBAR;
    }
    else if (BottleNeckNNCycleCount == VIPSRAMReadCycleCount)
    {
        memcpy(BottleNeck, "VIP_SRAM_RD", sizeof("VIP_SRAM_RD"));
        Cycles.BottleNeck_e = VIP_SRAM_RD;
    }
    else if (BottleNeckNNCycleCount == VIPSRAMWriteCycleCount)
    {
        memcpy(BottleNeck, "VIP_SRAM_WR", sizeof("VIP_SRAM_WR"));
        Cycles.BottleNeck_e = VIP_SRAM_WR;
    }
    else if (BottleNeckNNCycleCount == AXIBusReadCycleCount)
    {
        memcpy(BottleNeck, "AXI_BUS", sizeof("AXI_BUS"));
        Cycles.BottleNeck_e = AXI_BUS;
    }
    else if (BottleNeckNNCycleCount == AXIBusWriteCycleCount)
    {
        memcpy(BottleNeck, "AXI_BUS", sizeof("AXI_BUS"));
        Cycles.BottleNeck_e = AXI_BUS;
    }
    else if (BottleNeckNNCycleCount == AXIBusTotalCycleCount)
    {
        memcpy(BottleNeck, "AXI_BUS", sizeof("AXI_BUS"));
        Cycles.BottleNeck_e = AXI_BUS;
    }
    else if (BottleNeckNNCycleCount == AXISRAMReadCycleCount)
    {
        memcpy(BottleNeck, "AXI_SRAM", sizeof("AXI_SRAM"));
        Cycles.BottleNeck_e = AXI_SRAM;
    }
    else if (BottleNeckNNCycleCount == AXISRAMWriteCycleCount)
    {
        memcpy(BottleNeck, "AXI_SRAM", sizeof("AXI_SRAM"));
        Cycles.BottleNeck_e = AXI_SRAM;
    }
    else if (BottleNeckNNCycleCount == AXISRAMTotalCycleCount)
    {
        memcpy(BottleNeck, "AXI_SRAM", sizeof("AXI_SRAM"));
        Cycles.BottleNeck_e = AXI_SRAM;
    }
    else if (BottleNeckNNCycleCount == DDRReadCycleCount)
    {
        memcpy(BottleNeck, "DDRRead", sizeof("DDRRead"));
        Cycles.BottleNeck_e = DDRRead;
    }
    else if (BottleNeckNNCycleCount == DDRWriteCycleCount)
    {
        memcpy(BottleNeck, "DDRWrite", sizeof("DDRWrite"));
        Cycles.BottleNeck_e = DDRWrite;
    }
    else if (BottleNeckNNCycleCount == DDRTotalCycleCount)
    {
        memcpy(BottleNeck, "DDR", sizeof("DDR"));
        Cycles.BottleNeck_e = DDR;
    }
    else if (BottleNeckNNCycleCount == InternalWriteCycleCount)
    {
        memcpy(BottleNeck, "INTERNAL_WRITE", sizeof("INTERNAL_WRITE"));
        Cycles.BottleNeck_e = INTERNAL_WRITE;
    }
    else if (BottleNeckNNCycleCount == KernelDDRReadCycleCount)
    {
        memcpy(BottleNeck, "KERNEL_READ", sizeof("KERNEL_READ"));
        Cycles.BottleNeck_e = KERNEL_READ;
    }
    else if (BottleNeckNNCycleCount == InImageDDRReadCycleCount)
    {
        memcpy(BottleNeck, "IMAGE_READ", sizeof("IMAGE_READ"));
        Cycles.BottleNeck_e = IMAGE_READ;
    }
    else if (BottleNeckNNCycleCount == InternalKernelReadCycleCount)
    {
        memcpy(BottleNeck, "INTERNAL_KERNEL_READ", sizeof("INTERNAL_KERNEL_READ"));
        Cycles.BottleNeck_e = INTERNAL_KERNEL_READ;
    }
    else if (BottleNeckNNCycleCount == RdReturnArbiterCycleCount)
    {
        memcpy(BottleNeck, "Rd_Arb", sizeof("Rd_Arb"));
        Cycles.BottleNeck_e = Rd_Arb;
    }
    else
    {
        assert(0 && "Should never be here!");
    }

    memcpy(Cycles.BottleNeck, BottleNeck, sizeof(BottleNeck));
}


void calcBottleNeckTPCycleCount(
    CycleCounts & Cycles
    )
{
    double DDRTotalCycleCount = Cycles.DDRTotal;
    double DDRWriteCycleCount = Cycles.DDRWrite;
    double DDRReadCycleCount  = Cycles.DDRRead;
    double CompCycleCount     = Cycles.Compute;
    double AXISRAMTotalCycleCount = Cycles.AXISRAMTotal;
    double AXISRAMWriteCycleCount = Cycles.AXISRAMWrite;
    double AXISRAMReadCycleCount  = Cycles.AXISRAMRead;
    double AXIBusTotalCycleCount  = Cycles.AXIBUSTotal;
    double AXIBusWriteCycleCount  = Cycles.AXIBUSWrite;
    double AXIBusReadCycleCount   = Cycles.AXIBUSRead;
    double VIPSRAMWriteCycleCount = Cycles.VIPSRAMWrite;
    double VIPSRAMReadCycleCount  = Cycles.VIPSRAMRead;
    double CacheControllerCycleCount = Cycles.CacheController;
    double DDRReadCycleCount_Combined_Bursts = Cycles.DDRRead_Combined_Bursts;
    double DDRWriteCycleCount_Combined_Bursts = Cycles.DDRWrite_Combined_Bursts;

    double OverallCycleCount = max(DDRTotalCycleCount, max(DDRWriteCycleCount, max(DDRReadCycleCount, CompCycleCount)));
    OverallCycleCount = max(AXISRAMTotalCycleCount, max(AXISRAMWriteCycleCount, max(AXISRAMReadCycleCount, OverallCycleCount)));
    OverallCycleCount = max(AXIBusTotalCycleCount, max(AXIBusWriteCycleCount, max(AXIBusReadCycleCount, OverallCycleCount)));
    OverallCycleCount = max(VIPSRAMWriteCycleCount, max(VIPSRAMReadCycleCount, OverallCycleCount));
    OverallCycleCount = max(max(max(CacheControllerCycleCount, OverallCycleCount), DDRReadCycleCount_Combined_Bursts), DDRWriteCycleCount_Combined_Bursts);
    Cycles.Overall = OverallCycleCount;

    char BottleNeck[64] = "";
    if (OverallCycleCount == CompCycleCount)
    {
        memcpy(BottleNeck, "COMPUTE", sizeof("COMPUTE"));
        Cycles.BottleNeck_e = COMPUTE;
    }
    else if (OverallCycleCount == VIPSRAMReadCycleCount)
    {
        memcpy(BottleNeck, "VIP_SRAM_RD", sizeof("VIP_SRAM_RD"));
        Cycles.BottleNeck_e = VIP_SRAM_RD;
    }
    else if (OverallCycleCount == VIPSRAMWriteCycleCount)
    {
        memcpy(BottleNeck, "VIP_SRAM_WR", sizeof("VIP_SRAM_WR"));
        Cycles.BottleNeck_e = VIP_SRAM_WR;
    }
    else if (OverallCycleCount == AXIBusReadCycleCount)
    {
        memcpy(BottleNeck, "AXI_BUS", sizeof("AXI_BUS"));
        Cycles.BottleNeck_e = AXI_BUS;
    }
    else if (OverallCycleCount == AXIBusWriteCycleCount)
    {
        memcpy(BottleNeck, "AXI_BUS", sizeof("AXI_BUS"));
        Cycles.BottleNeck_e = AXI_BUS;
    }
    else if (OverallCycleCount == AXIBusTotalCycleCount)
    {
        memcpy(BottleNeck, "AXI_BUS", sizeof("AXI_BUS"));
        Cycles.BottleNeck_e = AXI_BUS;
    }
    else if (OverallCycleCount == AXISRAMReadCycleCount)
    {
        memcpy(BottleNeck, "AXI_BUS", sizeof("AXI_BUS"));
        Cycles.BottleNeck_e = AXI_BUS;
    }
    else if (OverallCycleCount == AXISRAMWriteCycleCount)
    {
        memcpy(BottleNeck, "AXI_SRAM", sizeof("AXI_SRAM"));
        Cycles.BottleNeck_e = AXI_SRAM;
    }
    else if (OverallCycleCount == AXISRAMTotalCycleCount)
    {
        memcpy(BottleNeck, "AXI_SRAM", sizeof("AXI_SRAM"));
        Cycles.BottleNeck_e = AXI_SRAM;
    }
    else if (OverallCycleCount == DDRReadCycleCount)
    {
        memcpy(BottleNeck, "DDR", sizeof("DDR"));
        Cycles.BottleNeck_e = DDR;
    }
    else if (OverallCycleCount == DDRWriteCycleCount)
    {
        memcpy(BottleNeck, "DDR", sizeof("DDR"));
        Cycles.BottleNeck_e = DDR;
    }
    else if (OverallCycleCount == DDRTotalCycleCount)
    {
        memcpy(BottleNeck, "DDR", sizeof("DDR"));
        Cycles.BottleNeck_e = DDR;
    }
    else if (OverallCycleCount == CacheControllerCycleCount)
    {
        memcpy(BottleNeck, "USC_CONTROLLER", sizeof("USC_CONTROLLER"));
        Cycles.BottleNeck_e = USC_CONTROLLER;
    }
    else if (OverallCycleCount == DDRWriteCycleCount_Combined_Bursts)
    {
        memcpy(BottleNeck, "LP_WRITE", sizeof("LP_WRITE"));
        Cycles.BottleNeck_e = LP_WRITE;
    }
    else if (OverallCycleCount == DDRReadCycleCount_Combined_Bursts)
    {
        memcpy(BottleNeck, "LP_READ", sizeof("LP_READ"));
        Cycles.BottleNeck_e = LP_READ;
    }
    else
    {
        assert(0 && "should not be here !\n");
    }
}

// refine me
void Cycles_Bottleneck(CycleCounts & cycles1)
{
    cycles1.Overall = cycles1.Compute;
    //cycles1.BottleNeck = "COMPUTE";
    cycles1.BottleNeck_e = COMPUTE;

    if (cycles1.DDRRead > cycles1.Overall)
    {
        cycles1.Overall = cycles1.DDRRead;
        //cycles1.BottleNeck = "DDRRead";
        cycles1.BottleNeck_e = DDRRead;
    }

    if (cycles1.DDRWrite > cycles1.Overall )
    {
        cycles1.Overall = cycles1.DDRWrite;
        //cycles1.BottleNeck = "DDRWrite";
        cycles1.BottleNeck_e = DDRWrite;
    }

    if (cycles1.DDRTotal > cycles1.Overall)
    {
        cycles1.Overall = cycles1.DDRTotal;
        //cycles1.BottleNeck = "DDR";
        cycles1.BottleNeck_e = DDR;
    }
    if (cycles1.AXISRAMRead > cycles1.Overall)
    {
        cycles1.Overall = cycles1.AXISRAMRead;
        //cycles1.BottleNeck = "AXI_SRAM";
        cycles1.BottleNeck_e = AXI_SRAM;
    }

    if (cycles1.AXISRAMWrite > cycles1.Overall)
    {
        cycles1.Overall = cycles1.AXISRAMWrite;
        //cycles1.BottleNeck = "AXI_SRAM";
        cycles1.BottleNeck_e = AXI_SRAM;
    }
    if ( cycles1.AXISRAMTotal > cycles1.Overall )
    {
        cycles1.Overall = cycles1.AXISRAMTotal;
        //cycles1.BottleNeck = "AXI_SRAM";
        cycles1.BottleNeck_e = AXI_SRAM;
    }
    if ( cycles1.AXIBUSRead > cycles1.Overall )
    {
        cycles1.Overall = cycles1.AXIBUSRead;
        //cycles1.BottleNeck = "AXI_BUS";
        cycles1.BottleNeck_e = AXI_BUS;
    }
    if ( cycles1.AXIBUSWrite > cycles1.Overall )
    {
        cycles1.Overall = cycles1.AXIBUSWrite;
        //cycles1.BottleNeck = "AXI_BUS";
        cycles1.BottleNeck_e = AXI_BUS;
    }

    if ( cycles1.AXIBUSTotal > cycles1.Overall )
    {
        cycles1.Overall = cycles1.AXIBUSTotal;
        //cycles1.BottleNeck = "AXI_BUS";
        cycles1.BottleNeck_e = AXI_BUS;
    }

    if ( cycles1.VIPSRAMRead > cycles1.Overall )
    {
        cycles1.Overall = cycles1.VIPSRAMRead;
        //cycles1.BottleNeck = "VIP_SRAM_RD";
        cycles1.BottleNeck_e = VIP_SRAM_RD;
    }

    if ( cycles1.VIPSRAMWrite > cycles1.Overall )
    {
        cycles1.Overall = cycles1.VIPSRAMWrite;
        //cycles1.BottleNeck = "VIP_SRAM_WR";
        cycles1.BottleNeck_e = VIP_SRAM_WR;
    }

    if ( cycles1.InternalKernelRead > cycles1.Overall )
    {
        cycles1.Overall = cycles1.InternalKernelRead;
        //cycles1.BottleNeck = "INTERNAL_Kernel_Read";
        cycles1.BottleNeck_e = INTERNAL_KERNEL_READ;
    }

    if (cycles1.InternalWrite > cycles1.Overall )
    {  cycles1.Overall = cycles1.InternalWrite;
    //cycles1.BottleNeck = "INTERNAL_WRITE";
    cycles1.BottleNeck_e = INTERNAL_WRITE;
    }
    if (cycles1.KernelDDRRead > cycles1.Overall )
    {
        cycles1.Overall = cycles1.KernelDDRRead;
        //cycles1.BottleNeck = "KERNEL_READ";
        cycles1.BottleNeck_e = KERNEL_READ;
    }

    if ( cycles1.InImageDDRRead > cycles1.Overall )
    {
        cycles1.Overall = cycles1.InImageDDRRead;
        //cycles1.BottleNeck = "IMAGE_READ";
        cycles1.BottleNeck_e = IMAGE_READ;
    }

    if ( cycles1.DWOut > cycles1.Overall )
    {
        cycles1.Overall = cycles1.DWOut;
        //cycles1.BottleNeck = "DWOut";
    }

    if ( cycles1.DQArb > cycles1.Overall )
    {
        cycles1.Overall = cycles1.DQArb;
        //cycles1.BottleNeck = "DQ_ARB";
        //cycles1.BottleNeck_e = DQ_ARB; // never enable yet
    }

    if ( cycles1.XBAR > cycles1.Overall )
    {  cycles1.Overall = cycles1.XBAR;
    //cycles1.BottleNeck = "XBAR";
    }

    if ( cycles1.KernelDecodeBW > cycles1.Overall )
    {
        cycles1.Overall = cycles1.KernelDecodeBW;
        //cycles1.BottleNeck = "COEF_DECODE";
        cycles1.BottleNeck_e = COEF_DECODE;
    }

    if ( cycles1.CacheController > cycles1.Overall )
    {
        cycles1.Overall = cycles1.CacheController;
        //cycles1.BottleNeck = "USC_CONTROLLER";
        cycles1.BottleNeck_e = USC_CONTROLLER;
    }

    if ( cycles1.RdReturnArbiter > cycles1.Overall )
    {
        cycles1.Overall = cycles1.RdReturnArbiter;
        //cycles1.BottleNeck = "Rd_Arb";
        cycles1.BottleNeck_e = Rd_Arb;
    }

    if ( cycles1.DDRRead_Combined_Bursts > cycles1.Overall )
    {
        cycles1.Overall = cycles1.DDRRead_Combined_Bursts;
        //cycles1.BottleNeck = "LP_READ";
        cycles1.BottleNeck_e = LP_READ;
    }

    if ( cycles1.DDRWrite_Combined_Bursts > cycles1.Overall )
    {
        cycles1.Overall = cycles1.DDRWrite_Combined_Bursts;
        //cycles1.BottleNeck = "LP_WRITE";
        cycles1.BottleNeck_e = LP_WRITE;
    }

}

void Cycles_Create(CycleCounts &cycleCountsType, double Compute, double DDRRead, double DDRWrite, double DDRTotal,
              double AXISRAMRead, double AXISRAMWrite, double AXISRAMTotal, double AXIBUSRead, double AXIBUSWrite,
              double AXIBUSTotal, double VIPSRAMRead, double VIPSRAMWrite, double Slow_InternalWrite, double Slow_Comp,
              double InternalWrite, double InternalKernelRead, double KernelDDRRead, double InImageDDRRead,
              double KernelDecodeBW, double DDRRead_Combined_Burst, double DDRWrite_Combined_Bursts, double DWOut,
              double DRArb, double RegTile2DXBar, double BottomTile2DXbar, double XBAR, double CacheController,
              double RdReturnArbiter
              )
{
    cycleCountsType.Compute = Compute;
    cycleCountsType.DDRRead = DDRRead;
    cycleCountsType.DDRWrite = DDRWrite;
    cycleCountsType.DDRTotal = DDRTotal;
    cycleCountsType.AXISRAMRead = AXISRAMRead;
    cycleCountsType.AXISRAMWrite = AXISRAMWrite;
    cycleCountsType.AXISRAMTotal = AXISRAMTotal;
    cycleCountsType.AXIBUSRead = AXIBUSRead;
    cycleCountsType.AXIBUSWrite = AXIBUSWrite;
    cycleCountsType.AXIBUSTotal = AXIBUSTotal;
    cycleCountsType.VIPSRAMRead = VIPSRAMRead;
    cycleCountsType.VIPSRAMWrite = VIPSRAMWrite;
    cycleCountsType.DDRRead_Combined_Bursts = DDRRead_Combined_Burst;
    cycleCountsType.DDRWrite_Combined_Bursts = DDRWrite_Combined_Bursts;
    cycleCountsType.Slow_InternalWrite = Slow_InternalWrite;
    cycleCountsType.Slow_Comp = Slow_Comp;
    cycleCountsType.InternalWrite = InternalWrite;
    cycleCountsType.InternalKernelRead = InternalKernelRead;
    cycleCountsType.KernelDDRRead = KernelDDRRead;
    cycleCountsType.InImageDDRRead = InImageDDRRead;
    cycleCountsType.KernelDecodeBW = KernelDecodeBW;
    cycleCountsType.DWOut = DWOut;
    cycleCountsType.DQArb = DRArb; // DR or DQ?
    cycleCountsType.RegTile2DXBar = RegTile2DXBar;
    cycleCountsType.BottomTile2DXbar = BottomTile2DXbar;
    cycleCountsType.XBAR = XBAR;
    cycleCountsType.CacheController = CacheController;
    cycleCountsType.RdReturnArbiter = RdReturnArbiter;

    Cycles_Bottleneck (cycleCountsType);
}

static double NNCycleCountCore(
    unsigned int  tile_xsize,
    unsigned int  tile_ysize,
    unsigned int  k, //kernel_per_core
    unsigned int  x,
    unsigned int  y,
    unsigned int  z,
    unsigned int  kx,
    unsigned int  ky,
    unsigned int  kz,
    unsigned int  inx,
    unsigned int  iny,
    unsigned int  pooling_stride,
    double        non_zero_ratio,
    double        coef_compress_ratio,
    double        image_compress_ratio,
    unsigned int  cores,
    unsigned int  brick_mode,
    unsigned int  input_data_size,
    unsigned int  output_data_size,
    unsigned int  l2cache_size,
    unsigned int  l2cache_width,
    unsigned int  xydp_x,
    unsigned int  xydp_y,
    unsigned int  zdp,
    unsigned int  float_xydp_x,
    unsigned int  float_xydp_y,
    unsigned int  float_zdp,
    unsigned int  usc_cache_size,
    unsigned int  NN_cmd_size_in_byte,
    unsigned int  coef_decode_perf,
    unsigned int  vector_prune,
    unsigned int  image_partial_cache,
    unsigned int  data_read_from_sram,
    unsigned int  first_cmd,
    unsigned int  src_buf,
    unsigned int  dst_buf,
    unsigned int  kernel_buf,
    float         axi_sram_read_bw_limit,
    float         axi_sram_write_bw_limit,
    float         axi_sram_total_bw_limit,
    float         axi_bus_read_bw_limit,
    float         axi_bus_write_bw_limit,
    float         axi_bus_total_bw_limit,
    float         internal_write_bw_limit,
    unsigned int  interleave_mode,
    unsigned int  lanes_per_conv,
    unsigned int  outstanding_transfer,
    unsigned int  zrl_bits,
    unsigned int  equivalent_vip_sram_width_in_byte,
    float   ddr_latency,
    float   total_latency,
    float   ddr_read_bw_in_byte_per_cycle,
    float   ddr_write_bw_in_byte_per_cycle,
    float   ddr_total_bw_in_byte_per_cycle,
    bool    image_not_packed_in_sram,
    bool    vip_v7_16bit,
    bool    input_fp16,
    bool    kernel_head_not_cached_fix,
    bool    conv1x1_half_performance,
    bool    cache_line_mode_disabled,
    bool    per_3d_tile_bubble_fix,
    bool    zdp3_no_compress_fix,
    bool    async_copy_perf_fix,
    bool    zxdp3_kernel_read_conflict_fix,
    bool    accurate_tile_bw,
    bool    axi_sram_slowed_down_by_addr,
    bool    slow_nn_req_arbitration_fix,
    bool    single_port_acc_buffer,
    bool    small_batch_en,
    bool    is_depth_wise,
    bool    nn_slow_output_feature,
    bool    is_nn_write_without_usc,
    unsigned int  in_image_stride,
    unsigned int  in_image_slice,
    unsigned int  out_image_stride,
    unsigned int  out_image_slice,
    bool    flush_and_wait,
    unsigned int  conv_out_fifo_depth,
    unsigned int specified_ddr_bw_limit_by_burst,
    OUT APM_OUT_BW_T *     outBandWidth,
    OUT APM_OUT_RESULT_T * outResult /* more info about cycle count bandwidth */
    )
{
    CycleCounts Cycles; memset(&Cycles, 0, sizeof(CycleCounts));
    CycleCounts NNCycleCounts_tile0_vzgroup0; memset(&NNCycleCounts_tile0_vzgroup0, 0, sizeof(CycleCounts));
    Outputs_Type * outputs = &outResult->outputs;
    memset(outputs, 0, sizeof(Outputs_Type));
    arch_model_cache_type cacheStrategy; memset(&cacheStrategy, 0, sizeof(arch_model_cache_type));    /* For VIP Sram BW calc */
    /* fix me, Internal_Kernel_Read_BYTE_PER_CYCLE should be a parameter */
    double Internal_Kernel_Read_BYTE_PER_CYCLE = pContext->bf.Internal_Kernel_Read_BYTE_PER_CYCLE;
    bool vipsram_stream_or_cache_read = (src_buf != SW_TILING_FROM_VIP_SRAM);
    bool low_efficiency_jd_wr_imgbuf_bug1992_fix = !pContext->bf.LOW_EFFICIENCY_JD_WR_IMGBUF_BUG1992;

    unsigned int trspInterleaveCh_in = pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_in;

    unsigned int kernel_from_ddr, kernel_from_axi_sram, sizeForAXISram;
    double cache_size_in_pixel, imageIdealCache;
    double ddrReadBW, vipReadBW, axiReadBW, AXIBusReadBandWidth;
    double ddrWriteBW, axiWriteBW, vipWriteBW, axiBusWriteBW;
    double ddrTotalBW, axiTotalBW, axiBusTotalBW;
    double ddrReadCycleCount, axiReadCycleCount, axiBusReadCycleCount;
    double ddrWriteCycleCount, axiWriteCycleCount, axiBusWriteCycleCount, vipWriteCycleCount;
    double ddrTotalCycleCount, axiTotalCycleCount, axiBusTotalCycleCount;
    double CompCycleCount, vipReadCycleCount, kernelDecodeCycleCount;
    double internalWriteCycleCount;
    float adjustedAXISramReadBandWidthLimit = axi_sram_read_bw_limit;
    double imageRepeatedSingleRead;
    double DQArbCycleCount, XBarCycleCount = 0;
    unsigned int axi_acc_unit_size_in_byte = AXI_BURST_SIZE;
    /* add for vip sram stride and slice */
    unsigned vipSramInImageStride = 0, vipSramInImageSlice = 0;

    // fix me, not all project use half freq of SRAM
    unsigned int vip_sram_acc_unit_size_in_byte = l2cache_width * 2; /* x2 because we are using half freq SRAM */

    //unsigned int vipSramInImageStride, vipSramInImageSlice;
    double refined_non_zero_ratio;
    double kernelReadBWTile0, imageReadBWVZGroup0;
    double ddrKernelReadBW = 0, ddrInImageReadBW = 0, vipKernelReadBW = 0, vipInImageReadBW = 0;
    APM_BW_CYCLE_COST_T ddrTotalBWCost, axiTotalBWCost, axiBusTotalBWCost;
    APM_BW_CYCLE_COST_T computeCycleCost, internalWriteCycleCost, ddrTotalCycleCost, axiTotalCycleCost, axiBusTotalCycleCost, InternalKernelReadCycleCount;
    APM_BW_CYCLE_COST_T kernelDecodeCycleCost, arbCycleCost, xBarCycleCost, nnCycleCost , RdReturnArbiterCycleCount;

    APM_COST_T  nnCost[APM_TOTAL_NN_COST_TYPE];
    APM_BW_CYCLE_COST_T *bwCost;
    APM_BW_CYCLE_COST_T *cycleCost;

    arch_model_bw_byburst_type DDRReadBandWidth_By_Burst; memset(&DDRReadBandWidth_By_Burst, 0, sizeof(DDRReadBandWidth_By_Burst));
    //arch_model_cycle_byburst_type    DDRReadCycle_By_Burst;
    arch_model_bw_byburst_type DDRKernelReadBandWidth_By_Burst; memset(&DDRKernelReadBandWidth_By_Burst, 0, sizeof(DDRKernelReadBandWidth_By_Burst));
    //arch_model_cycle_byburst_type    DDRKernelReadCycle_By_Burst;
    arch_model_bw_byburst_type DDRImageReadBandWidth_By_Burst; memset(&DDRImageReadBandWidth_By_Burst, 0, sizeof(DDRImageReadBandWidth_By_Burst));
    //arch_model_cycle_byburst_type    DDRImageReadCycle_By_Burst = {{0}};
    arch_model_cache_type empty_Cache_Type; memset(&empty_Cache_Type, 0, sizeof(empty_Cache_Type));
    arch_model_bw_byburst_type DDRWriteBandWidth_NonMask; memset(&DDRWriteBandWidth_NonMask, 0, sizeof(arch_model_bw_byburst_type));
    arch_model_bw_byburst_type DDRWriteBandWidth_Mask; memset(&DDRWriteBandWidth_Mask, 0, sizeof(arch_model_bw_byburst_type));

    APM_BW_CYCLE_COST_T DDRReadCycleCount_64B; memset(&DDRReadCycleCount_64B, 0, sizeof(APM_BW_CYCLE_COST_T));
    APM_BW_CYCLE_COST_T DDRReadCycleCount_128B; memset(&DDRReadCycleCount_128B, 0, sizeof(APM_BW_CYCLE_COST_T));
    APM_BW_CYCLE_COST_T DDRReadCycleCount_256B; memset(&DDRReadCycleCount_256B, 0, sizeof(APM_BW_CYCLE_COST_T));
    APM_BW_CYCLE_COST_T DDRReadCycleCount_Combined_Bursts; memset(&DDRReadCycleCount_Combined_Bursts, 0, sizeof(APM_BW_CYCLE_COST_T));

    APM_BW_CYCLE_COST_T DDRWriteCycleCount_64B; memset(&DDRWriteCycleCount_64B, 0, sizeof(APM_BW_CYCLE_COST_T));
    APM_BW_CYCLE_COST_T DDRWriteCycleCount_128B; memset(&DDRWriteCycleCount_128B, 0, sizeof(APM_BW_CYCLE_COST_T));
    APM_BW_CYCLE_COST_T DDRWriteCycleCount_256B; memset(&DDRWriteCycleCount_256B, 0, sizeof(APM_BW_CYCLE_COST_T));
    APM_BW_CYCLE_COST_T DDRWriteCycleCount_Combined_Bursts; memset(&DDRWriteCycleCount_Combined_Bursts, 0, sizeof(APM_BW_CYCLE_COST_T));

    //cores = (z < cores)? z: cores;

    unsigned int tail_x = x % tile_xsize;
    unsigned int tail_y = y % tile_ysize;
    unsigned int tail_z = z % (k * cores);
    tail_x = (tail_x == 0) ? tile_xsize : tail_x;
    tail_y = (tail_y == 0) ? tile_ysize : tail_y;
    tail_z = (tail_z == 0) ? (k * cores) : tail_z;

    memset(nnCost, 0, sizeof(APM_COST_T) * APM_TOTAL_NN_COST_TYPE);

    if (zdp > 1 && kx == 1 && ky == 1 && input_data_size == 8 && !zdp3_no_compress_fix)
    {
        //assert(non_zero_ratio == 1);
        coef_compress_ratio = max(1, coef_compress_ratio);
        non_zero_ratio = 1;
    }

    non_zero_ratio = max(non_zero_ratio, (double)1/((double)pow(2, (float)zrl_bits) - 1)); /* for limitation of zrl_bit_width <= 5 */
    CompCycleCount = ComputeCycleCount(tile_xsize, tile_ysize, k,
                                               kx, ky, kz, x, y, z, non_zero_ratio,
                                               xydp_x, xydp_y, zdp,
                                               float_xydp_x, float_xydp_y, float_zdp,
                                               input_data_size, vector_prune,
                                               interleave_mode, lanes_per_conv, /*coef_decode_perf,*/
                                               zrl_bits, is_depth_wise,
                                               vip_v7_16bit, input_fp16, conv1x1_half_performance, zxdp3_kernel_read_conflict_fix, single_port_acc_buffer, &refined_non_zero_ratio);

    CompCycleCount = (pContext->pInPerfParams->cmdInfo.is_depth_wise_merge)? CompCycleCount / (cores - 1): CompCycleCount / cores;
    CompCycleCount = CompCycleCount + (pContext->bf.new_feature ? 1 : 0); //Shuangbei: example of using new feature
    /* add process of stride and slice */
    if(src_buf != SW_TILING_FROM_VIP_SRAM)
    {
        vipSramInImageStride = min((tile_xsize + kx - 1), in_image_stride);
        vipSramInImageSlice = min((vipSramInImageStride*(tile_ysize + ky - 1)), in_image_slice);
    }
    else
    {
        vipSramInImageStride = in_image_stride;
        vipSramInImageSlice = in_image_slice;
    }

    vipKernelReadBW = KernelReadBandWidth(tile_xsize, tile_ysize, /*kernel_per_core,*/
        kx, ky, kz, x, y, z, SW_TILING_FROM_VIP_SRAM, /*x, y,*/
        cores, /*brick_mode,*/ input_data_size,
        coef_compress_ratio, /*image_compress_ratio,*/ 0, kernel_head_not_cached_fix,
        vip_sram_acc_unit_size_in_byte, /*is_depth_wise,*/ &kernelReadBWTile0, &cacheStrategy);

    vipInImageReadBW = ImageReadBandWidth(tile_xsize, tile_ysize, k,
        kx, ky, kz, x, y, z, x, y,
        cores, brick_mode, input_data_size,
        /*coef_compress_ratio,*/ 1, 0, image_not_packed_in_sram, cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte,
        vipSramInImageStride, vipSramInImageSlice, vip_sram_acc_unit_size_in_byte, zdp, image_partial_cache,
        low_efficiency_jd_wr_imgbuf_bug1992_fix, vipsram_stream_or_cache_read, SW_TILING_FROM_VIP_SRAM, &imageReadBWVZGroup0, &cacheStrategy);

    bwCost = &nnCost[APM_VIP_SRAM_COST].readBW;
    cycleCost = &nnCost[APM_VIP_SRAM_COST].readCycle;
    bwCost->cost = vipReadBW = vipKernelReadBW + vipInImageReadBW;
    bwCost->tile0VZGroup0 = kernelReadBWTile0 * min((1.0f * k * cores / z), 1.0f) + imageReadBWVZGroup0 * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
    bwCost->tile0 = kernelReadBWTile0 + vipInImageReadBW * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
    bwCost->vzGroup0 = vipKernelReadBW * min((1.0f * k * cores / z), 1.0f) + imageReadBWVZGroup0;

    // DDR count or SRAM count
    vipReadCycleCount = vipReadBW / l2cache_width;
    cycleCost->cost = vipReadCycleCount;
    cycleCost->tile0VZGroup0 = bwCost->tile0VZGroup0 / l2cache_width;
    cycleCost->tile0ResetVZGroup = (bwCost->tile0 - bwCost->tile0VZGroup0) / l2cache_width;
    cycleCost->resetTileVZGroup0 = (bwCost->vzGroup0 - bwCost->tile0VZGroup0) / l2cache_width;
    cycleCost->resetTileResetVZGroup = (vipReadBW + bwCost->tile0VZGroup0 - bwCost->tile0 - bwCost->vzGroup0) / l2cache_width;

    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.VIPSRAMRead = vipReadCycleCount;

    // step0: VIPSRAM VIPSRAM-kernel, VIPSRAM-inimage
    ddrReadBW = (float)NN_cmd_size_in_byte;
    bwCost = &nnCost[APM_DDR_COST].readBW;
    memset(bwCost, 0, sizeof(APM_BW_CYCLE_COST_T));
    bwCost->cost = ddrReadBW;
    bwCost->tile0VZGroup0 = (float)NN_cmd_size_in_byte;
    bwCost->tile0 = (float)NN_cmd_size_in_byte;
    bwCost->vzGroup0 = (float)NN_cmd_size_in_byte;

    axiReadBW = 0;

    double KernelVIPSRAMReadBandWidth       = vipKernelReadBW;
    double KernelVIPSRAMReadBandWidth_tile0 = kernelReadBWTile0;
    double KernelAXISRAMReadBandWidth       = 0;
    double KernelAXISRAMReadBandWidth_tile0 = 0;

    kernel_from_ddr = (kernel_buf == SW_TILING_FROM_DDR) || (first_cmd && ((kernel_buf == SW_TILING_FROM_AXI_SRAM) || (kernel_buf == SW_TILING_FROM_VIP_SRAM)));
    kernel_from_axi_sram = (!first_cmd && (kernel_buf == SW_TILING_FROM_AXI_SRAM)) || (kernel_buf == SW_TILING_PERM_AXI_SRAM);

    cache_size_in_pixel = (float)l2cache_size / (input_data_size / 8);

    double ddrKernelReadBWTile0 = 0, ddrInImageReadBWVZGroup0 = 0;
    if (kernel_from_ddr && src_buf == SW_TILING_FROM_DDR)
    {
        ddrReadBW = ReadBandWidth(tile_xsize, tile_ysize, k,
            kx, ky, kz, x, y, z, 0, inx, iny,
            cores, brick_mode, input_data_size, coef_compress_ratio, image_compress_ratio,
            l2cache_size, (bool)image_partial_cache, NN_cmd_size_in_byte, image_not_packed_in_sram,
            kernel_head_not_cached_fix, cache_line_mode_disabled,
            async_copy_perf_fix, accurate_tile_bw, is_depth_wise, in_image_stride, in_image_slice, zdp, axi_acc_unit_size_in_byte,
            equivalent_vip_sram_width_in_byte, &ddrKernelReadBW, &ddrInImageReadBW, &ddrKernelReadBWTile0, &ddrInImageReadBWVZGroup0, &nnCost[APM_DDR_COST].readBW, outputs);
        // ' In above line we use 0 for kernel_buf instead of passing kernel_buf, because for the first cmd of subimage tiling, kernel_buf will be VIP/AXI SRAM while kernel is actually from DDR

        bwCost = &nnCost[APM_DDR_KERNEL_COST].readBW;
        bwCost->cost = ddrKernelReadBW;
        bwCost->tile0 = ddrKernelReadBWTile0;

        bwCost = &nnCost[APM_DDR_IN_IMAGE_COST].readBW;
        bwCost->cost = ddrInImageReadBW;
        bwCost->vzGroup0 = ddrInImageReadBWVZGroup0;

        // 'Calculate Burst BW
        double KernelDDRReadBurst, InImageDDRReadBurst, KernelDDRReadBurst_tile0, InImageDDRReadBurst_vzgroup0;
        ReadBandWidth(tile_xsize, tile_ysize, k,
            kx, ky, kz, x, y, z, 0, inx, iny,
            cores, brick_mode, input_data_size, coef_compress_ratio, image_compress_ratio,
            l2cache_size, (bool)image_partial_cache, NN_cmd_size_in_byte, image_not_packed_in_sram,
            kernel_head_not_cached_fix, cache_line_mode_disabled,
            async_copy_perf_fix, accurate_tile_bw, is_depth_wise, in_image_stride, in_image_slice, zdp, 256,
            equivalent_vip_sram_width_in_byte, &KernelDDRReadBurst, &InImageDDRReadBurst, &KernelDDRReadBurst_tile0,
            &InImageDDRReadBurst_vzgroup0, &nnCost[APM_DDR_READ_BURST_COST].readBW, outputs);

        //// ' In above line we use 0 for kernel_buf instead of passing kernel_buf, because for the first cmd of subimage tiling, kernel_buf will be VIP/AXI SRAM while kernel is actually from DDR

        if(specified_ddr_bw_limit_by_burst)
        {
            /*Calculate BW by Burst Sizes*/
            KernelReadBandWidth_By_BurstSize(tile_xsize, tile_ysize, k, kx, ky, kz, x, y, z, 0, /*inx, iny,*/ cores, /*brick_mode,*/ input_data_size, coef_compress_ratio, /*image_compress_ratio,*/ outputs->kernelCacheType.sizeCached,/*is_depth_wise,*/ &empty_Cache_Type, kernel_head_not_cached_fix, &DDRKernelReadBandWidth_By_Burst);
            ImageReadBandWidth_By_BurstSize(tile_xsize, tile_ysize, k, kx, ky, kz, x, y, z, inx, iny, cores, brick_mode, input_data_size, /*coef_compress_ratio,*/ image_compress_ratio, outputs->imageCacheType.sizeCached,
                image_not_packed_in_sram, cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, /*axi_acc_unit_size_in_byte,*/ zdp,
                image_partial_cache, low_efficiency_jd_wr_imgbuf_bug1992_fix, 0, &empty_Cache_Type, /*SW_TILING_FROM_DDR,*/ trspInterleaveCh_in, &DDRImageReadBandWidth_By_Burst);

            SplitIntoFourRegions_Read(&(DDRKernelReadBandWidth_By_Burst.BW_64B), tile_xsize, tile_ysize, k, x, y, z, cores);
            SplitIntoFourRegions_Read(&(DDRKernelReadBandWidth_By_Burst.BW_128B), tile_xsize, tile_ysize, k, x, y, z, cores);
            SplitIntoFourRegions_Read(&(DDRKernelReadBandWidth_By_Burst.BW_256B), tile_xsize, tile_ysize, k, x, y, z, cores);

            SplitIntoFourRegions_Read(&(DDRImageReadBandWidth_By_Burst.BW_64B), tile_xsize, tile_ysize, k, x, y, z, cores);
            SplitIntoFourRegions_Read(&(DDRImageReadBandWidth_By_Burst.BW_128B), tile_xsize, tile_ysize, k, x, y, z, cores);
            SplitIntoFourRegions_Read(&(DDRImageReadBandWidth_By_Burst.BW_256B), tile_xsize, tile_ysize, k, x, y, z, cores);

            DDRReadBandWidth_By_Burst.BW_64B = Type_ByRegion_Add(DDRKernelReadBandWidth_By_Burst.BW_64B, DDRImageReadBandWidth_By_Burst.BW_64B);
            DDRReadBandWidth_By_Burst.BW_128B = Type_ByRegion_Add(DDRKernelReadBandWidth_By_Burst.BW_128B, DDRImageReadBandWidth_By_Burst.BW_128B);
            DDRReadBandWidth_By_Burst.BW_256B =Type_ByRegion_Add(DDRKernelReadBandWidth_By_Burst.BW_256B, DDRImageReadBandWidth_By_Burst.BW_256B);

        }
    }
    else if (kernel_from_ddr)
    {
        ddrKernelReadBW = KernelReadBandWidth(tile_xsize, tile_ysize, /*kernel_per_core, */
            kx, ky, kz, x, y, z, 0, /*x, y,*/
            cores, /*brick_mode,*/ input_data_size, coef_compress_ratio,
            /*image_compress_ratio,*/ cache_size_in_pixel, kernel_head_not_cached_fix, pContext->pInParams->NN_DDR_BURST_SIZE, /*is_depth_wise,*/ &kernelReadBWTile0, &(outputs->kernelCacheType));

        bwCost = &nnCost[APM_DDR_KERNEL_COST].readBW;
        bwCost->cost = ddrKernelReadBW;
        bwCost->tile0 = kernelReadBWTile0;

        bwCost = &nnCost[APM_DDR_COST].readBW;
        ddrReadBW = NN_cmd_size_in_byte + ddrKernelReadBW;
        bwCost->cost = ddrReadBW;
        bwCost->tile0VZGroup0 = NN_cmd_size_in_byte + kernelReadBWTile0 * min((1.0f * k * cores / z), 1.0f);
        bwCost->tile0 = NN_cmd_size_in_byte + kernelReadBWTile0;
        bwCost->vzGroup0 = NN_cmd_size_in_byte + ddrKernelReadBW * min((1.0f * k * cores / z), 1.0f);

        // 'Calculate Burst BW
        double KernelDDRReadBurst_tile0 = 0;
        double KernelDDRReadBurst = KernelReadBandWidth(tile_xsize, tile_ysize, /*kernel_per_core, */
            kx, ky, kz, x, y, z, 0, /*x, y,*/
            cores, /*brick_mode,*/ input_data_size, coef_compress_ratio,
            /*image_compress_ratio,*/ cache_size_in_pixel, kernel_head_not_cached_fix, 256, /*is_depth_wise,*/ &KernelDDRReadBurst_tile0, &(outputs->kernelCacheType));

        bwCost = &nnCost[APM_DDR_KERNEL_READ_BURST_COST].readBW;
        bwCost->cost = KernelDDRReadBurst;
        bwCost->tile0 = KernelDDRReadBurst_tile0;

        bwCost = &nnCost[APM_DDR_READ_BURST_COST].readBW;
        double DDRReadBurst = NN_cmd_size_in_byte + KernelDDRReadBurst;
        bwCost->cost = DDRReadBurst;
        bwCost->tile0VZGroup0 = NN_cmd_size_in_byte + KernelDDRReadBurst_tile0 * min(1.0f * k * cores / z, 1.0f);
        bwCost->tile0 = NN_cmd_size_in_byte + KernelDDRReadBurst_tile0;
        bwCost->vzGroup0 = NN_cmd_size_in_byte + KernelDDRReadBurst * min((1.0f * k * cores / z), 1.0f);

        /* Calculate BW by Burst Sizes */
        if (specified_ddr_bw_limit_by_burst == 1)
        {
            KernelReadBandWidth_By_BurstSize(tile_xsize, tile_ysize, k, kx, ky, kz, x, y, z, 0, /*inx, iny,*/ cores, /*brick_mode,*/ input_data_size, coef_compress_ratio, /*image_compress_ratio,*/ outputs->kernelCacheType.sizeCached, /*is_depth_wise,*/ &empty_Cache_Type, kernel_head_not_cached_fix, &DDRReadBandWidth_By_Burst);

            SplitIntoFourRegions_Read(&(DDRReadBandWidth_By_Burst.BW_64B), tile_xsize, tile_ysize, k, x, y, z, cores);
            SplitIntoFourRegions_Read(&(DDRReadBandWidth_By_Burst.BW_128B), tile_xsize, tile_ysize, k, x, y, z, cores);
            SplitIntoFourRegions_Read(&(DDRReadBandWidth_By_Burst.BW_256B), tile_xsize, tile_ysize, k, x, y, z, cores);
        }
        if (src_buf == SW_TILING_FROM_AXI_SRAM)
        {
            //unsigned int zPerCore = (unsigned int)ceilf((float)z / cores);
            double KernelIdealCache, KernelNonIdealCache, KernelStorage;
            KernelIdealCache = ComputeKernelIdealCache(kx, ky, kz, z, coef_compress_ratio);
            KernelNonIdealCache = ComputeKernelNonIdealCache(kx, ky, kz, z, coef_compress_ratio, cores);
            KernelStorage =  ComputeKernelStorage(KernelIdealCache, KernelNonIdealCache, pContext->bf.PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007, pContext->bf.Full_KERNEL_CACHE_INTERLEAVE_BUG_2033, (int)cache_size_in_pixel);

            cache_size_in_pixel = (cache_size_in_pixel - min(KernelStorage, cache_size_in_pixel)) * data_read_from_sram;
            axiReadBW = ImageReadBandWidth(tile_xsize, tile_ysize, k, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, input_data_size, /*coef_compress_ratio,*/ image_compress_ratio, cache_size_in_pixel, image_not_packed_in_sram,
                cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, axi_acc_unit_size_in_byte, zdp, image_partial_cache,
                low_efficiency_jd_wr_imgbuf_bug1992_fix, 0 /*vipsram_stream_or_cache_read always 0*/, SW_TILING_FROM_AXI_SRAM, &imageReadBWVZGroup0, &(outputs->imageCacheType));

            bwCost = &nnCost[APM_AXI_SRAM_COST].readBW;
            bwCost->cost = axiReadBW;
            bwCost->tile0VZGroup0 = imageReadBWVZGroup0 * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
            bwCost->tile0 = axiReadBW * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
            bwCost->vzGroup0 = imageReadBWVZGroup0;

            if (axi_sram_slowed_down_by_addr && (first_cmd || (cache_size_in_pixel != 0)))
            {
                unsigned int maxOutstandingCycle = outstanding_transfer * 4;
                if (total_latency > maxOutstandingCycle)
                {
                    float bwLimitedByLatency = (float)(16.0 * maxOutstandingCycle) / total_latency;
                    adjustedAXISramReadBandWidthLimit = min(axi_sram_read_bw_limit, bwLimitedByLatency);
                }
            }
        }
    }
    else if (src_buf == SW_TILING_FROM_DDR)
    {
        ddrInImageReadBW = ImageReadBandWidth(tile_xsize, tile_ysize, k, kx, ky, kz, x, y, z, inx, iny,
            cores, brick_mode, input_data_size, /*coef_compress_ratio,*/ image_compress_ratio, cache_size_in_pixel, image_not_packed_in_sram,
            cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice,
            axi_acc_unit_size_in_byte, zdp, image_partial_cache,
            low_efficiency_jd_wr_imgbuf_bug1992_fix, 0 /*vipsram_stream_or_cache_read always 0*/, SW_TILING_FROM_DDR, &imageReadBWVZGroup0, &(outputs->imageCacheType));

        bwCost = &nnCost[APM_DDR_IN_IMAGE_COST].readBW;
        bwCost->cost = ddrInImageReadBW;
        bwCost->vzGroup0 = imageReadBWVZGroup0;

        ddrReadBW = NN_cmd_size_in_byte + ddrInImageReadBW;
        bwCost = &nnCost[APM_DDR_COST].readBW;
        bwCost->cost = ddrReadBW;
        bwCost->tile0VZGroup0 = NN_cmd_size_in_byte + imageReadBWVZGroup0 * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        bwCost->tile0 = NN_cmd_size_in_byte + ddrInImageReadBW * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        bwCost->vzGroup0 = NN_cmd_size_in_byte + imageReadBWVZGroup0;

        // ' Calculate Burst BW
        double InImageDDRReadBurst_vzgroup0;
        double InImageDDRReadBurst = ImageReadBandWidth(tile_xsize, tile_ysize, k, kx, ky, kz, x, y, z, inx, iny,
            cores, brick_mode, input_data_size, /*coef_compress_ratio,*/ image_compress_ratio, cache_size_in_pixel, image_not_packed_in_sram,
            cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice,
            256, zdp, image_partial_cache,
            low_efficiency_jd_wr_imgbuf_bug1992_fix, 0 /*vipsram_stream_or_cache_read always 0*/, SW_TILING_FROM_DDR, &InImageDDRReadBurst_vzgroup0, &(outputs->imageCacheType));

        bwCost = &nnCost[APM_DDR_IN_IMAGE_READ_BURST_COST].readBW;
        bwCost->cost = InImageDDRReadBurst;
        bwCost->vzGroup0 = InImageDDRReadBurst_vzgroup0;

        double DDRReadBurst = NN_cmd_size_in_byte + InImageDDRReadBurst;
        bwCost = &nnCost[APM_DDR_READ_BURST_COST].readBW;
        bwCost->cost = DDRReadBurst;
        bwCost->tile0VZGroup0 = NN_cmd_size_in_byte + InImageDDRReadBurst_vzgroup0 * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        bwCost->tile0 = NN_cmd_size_in_byte + InImageDDRReadBurst * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        bwCost->vzGroup0 = NN_cmd_size_in_byte + InImageDDRReadBurst_vzgroup0;

        /* Calculate BW by Burst Sizes */
        if(specified_ddr_bw_limit_by_burst) //If BRCM_DDR_BURST_ISSUE = 1
        {
            ImageReadBandWidth_By_BurstSize(tile_xsize, tile_ysize, k, kx, ky, kz, x, y, z, inx, iny, cores, brick_mode, input_data_size, /*coef_compress_ratio,*/ image_compress_ratio, outputs->imageCacheType.sizeCached,
                image_not_packed_in_sram, cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, /*axi_acc_unit_size_in_byte,*/ zdp,
                image_partial_cache, low_efficiency_jd_wr_imgbuf_bug1992_fix, 0, &empty_Cache_Type, /*SW_TILING_FROM_DDR,*/ trspInterleaveCh_in, &DDRReadBandWidth_By_Burst);

            SplitIntoFourRegions_Read(&(DDRReadBandWidth_By_Burst.BW_64B), tile_xsize, tile_ysize, k, x, y, z, cores);
            SplitIntoFourRegions_Read(&(DDRReadBandWidth_By_Burst.BW_128B), tile_xsize, tile_ysize, k, x, y, z, cores);
            SplitIntoFourRegions_Read(&(DDRReadBandWidth_By_Burst.BW_256B), tile_xsize, tile_ysize, k, x, y, z, cores);
        }

        if (kernel_from_axi_sram)
        {
            unsigned int intile_xsize = (tile_xsize + kx - 1);
            unsigned int intile_ysize = (tile_ysize + ky - 1);
            intile_xsize = min(intile_xsize, inx);
            intile_ysize = min(intile_ysize, iny);
            if (image_not_packed_in_sram)
            {
                imageIdealCache = ceilf(ceilf((float)intile_xsize * intile_ysize / 16) * 16 * kz / (equivalent_vip_sram_width_in_byte * 2))
                                  * (equivalent_vip_sram_width_in_byte * 2) * input_data_size / 8;
            }
            else
            {
                imageIdealCache = (float)intile_xsize * intile_ysize * kz * input_data_size / 8;
            }
            cache_size_in_pixel = (cache_size_in_pixel - min(imageIdealCache, cache_size_in_pixel)) * data_read_from_sram;
            axiReadBW = KernelReadBandWidth(tile_xsize, tile_ysize, /*k,*/ kx, ky, kz, kernel_buf, inx, iny, z, /*x, y,*/
                cores, /*brick_mode,*/ input_data_size, coef_compress_ratio,
                /*image_compress_ratio,*/ cache_size_in_pixel, kernel_head_not_cached_fix, axi_acc_unit_size_in_byte, /*is_depth_wise,*/ &kernelReadBWTile0, &(outputs->kernelCacheType));

            bwCost = &nnCost[APM_AXI_SRAM_COST].readBW;
            bwCost->cost = axiReadBW;
            bwCost->tile0VZGroup0 = kernelReadBWTile0 * min((1.0f * k * cores / z), 1.0f);
            bwCost->tile0 = kernelReadBWTile0;
            bwCost->vzGroup0 = axiReadBW * min((1.0f * k * cores / z), 1.0f);

            KernelAXISRAMReadBandWidth = axiReadBW;
            KernelAXISRAMReadBandWidth_tile0 = bwCost->tile0VZGroup0;

            if (axi_sram_slowed_down_by_addr)
            {
                unsigned int maxOutstandingCycle = outstanding_transfer * 4;
                if (total_latency > maxOutstandingCycle)
                {
                    float bwLimitedByLatency = (float)(16.0 * maxOutstandingCycle) / total_latency;
                    adjustedAXISramReadBandWidthLimit = min(axi_sram_read_bw_limit, bwLimitedByLatency);
                }
            }
        }
    }
    else if (kernel_from_axi_sram && src_buf == SW_TILING_FROM_AXI_SRAM)
    {
        sizeForAXISram = data_read_from_sram * l2cache_size;
        axiReadBW = ReadBandWidth(tile_xsize, tile_ysize, k,
            kx, ky, kz, x, y, z, kernel_buf, inx, iny,
            cores, brick_mode, input_data_size, coef_compress_ratio, image_compress_ratio, sizeForAXISram,
            (bool)image_partial_cache, NN_cmd_size_in_byte, image_not_packed_in_sram, kernel_head_not_cached_fix, cache_line_mode_disabled,
            async_copy_perf_fix, accurate_tile_bw, is_depth_wise, in_image_stride, in_image_slice, zdp, axi_acc_unit_size_in_byte,
            equivalent_vip_sram_width_in_byte, NULL, NULL, &kernelReadBWTile0, &imageReadBWVZGroup0, &nnCost[APM_AXI_SRAM_COST].readBW, outputs);

        KernelAXISRAMReadBandWidth       = axiReadBW;
        KernelAXISRAMReadBandWidth_tile0 = kernelReadBWTile0;
    }
    else if (kernel_from_axi_sram)
    {
        cache_size_in_pixel = cache_size_in_pixel * data_read_from_sram;
        axiReadBW = KernelReadBandWidth(tile_xsize, tile_ysize, /*k,*/
            kx, ky, kz, x, y, z, kernel_buf, /*inx, iny,*/
            cores, /*brick_mode,*/ input_data_size, coef_compress_ratio, /*image_compress_ratio,*/ cache_size_in_pixel,
            kernel_head_not_cached_fix, axi_acc_unit_size_in_byte, /*is_depth_wise,*/ &kernelReadBWTile0, &(outputs->kernelCacheType));

        bwCost = &nnCost[APM_AXI_SRAM_COST].readBW;
        bwCost->cost = axiReadBW;
        bwCost->tile0VZGroup0 = kernelReadBWTile0 * min((1.0f * k * cores / z), 1.0f);
        bwCost->tile0 = kernelReadBWTile0;
        bwCost->vzGroup0 = axiReadBW * min((1.0f * k * cores / z), 1.0f);

        KernelAXISRAMReadBandWidth       = axiReadBW;
        KernelAXISRAMReadBandWidth_tile0 = kernelReadBWTile0;
    }
    else if (src_buf == SW_TILING_FROM_AXI_SRAM)
    {
        cache_size_in_pixel = cache_size_in_pixel * data_read_from_sram;
        axiReadBW = ImageReadBandWidth(tile_xsize, tile_ysize, k, kx, ky, kz, x, y, z, inx, iny,
            cores, brick_mode, input_data_size, /*coef_compress_ratio,*/ image_compress_ratio, cache_size_in_pixel, image_not_packed_in_sram,
            cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, axi_acc_unit_size_in_byte, zdp, image_partial_cache,
            low_efficiency_jd_wr_imgbuf_bug1992_fix, 0 /*vipsram_stream_or_cache_read always 0*/, SW_TILING_FROM_AXI_SRAM, &imageReadBWVZGroup0, &(outputs->imageCacheType));

        bwCost = &nnCost[APM_AXI_SRAM_COST].readBW;
        bwCost->cost = axiReadBW;
        bwCost->tile0VZGroup0 = imageReadBWVZGroup0 * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        bwCost->tile0 = axiReadBW * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        bwCost->vzGroup0 = imageReadBWVZGroup0;
    }

    // step1: AXIBUS read bandwidh / cycle count
    // refine me
    bool BUG_2035 = 0; //Merged_Data_Split_Bottleneck_FIX( fix?)
    if ( (BUG_2035 == 1) && (src_buf != SW_TILING_FROM_VIP_SRAM) )
    {
        ByRegion_Type ImageReadBandWidth_Unmerged;
        ByRegion_Type KernelReadBandWidth_Unmerged;
        unsigned int ppc_tmp_i = 0;
        if (src_buf == SW_TILING_FROM_DDR)
        {
            ppc_tmp_i = pContext->pInParams->NN_DDR_BURST_SIZE;
        }
        else if (src_buf == SW_TILING_FROM_AXI_SRAM)
        {
            ppc_tmp_i = axi_acc_unit_size_in_byte;
        }
        else
        {
            assert(0);
        }

        bool unmerged_mode = true;
        axiReadBW = ImageReadBandWidth(tile_xsize, tile_ysize, k, kx, ky, kz, x, y, z, inx, iny,
                                      cores, brick_mode, input_data_size, /*coef_compress_ratio,*/ image_compress_ratio, cache_size_in_pixel, image_not_packed_in_sram,
                                      cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, ppc_tmp_i, zdp, image_partial_cache,
                                      low_efficiency_jd_wr_imgbuf_bug1992_fix, vipsram_stream_or_cache_read, SW_TILING_FROM_AXI_SRAM, &imageReadBWVZGroup0, &(outputs->imageCacheType));

        ImageReadBandWidth_Unmerged.cost = axiReadBW; // Total
        ImageReadBandWidth_Unmerged.vzGroup0 = imageReadBWVZGroup0; //vzGroup0

        // fix me, VB looks not fill tile0VZGroup0, and tile0
        //bwCost->tile0VZGroup0 = ImageReadBandWidth_Unmerged.vzgroup0 * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        //bwCost->tile0 = ImageReadBandWidth_Unmerged.Total * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y);
        SplitIntoFourRegions_Read(&ImageReadBandWidth_Unmerged, tile_xsize, tile_ysize, k, x, y, z, cores);

        unmerged_mode = false;
        if (kernel_from_ddr)
        {
            unsigned int ppc_tmp_k = pContext->pInParams->NN_DDR_BURST_SIZE;
            double ddrReadBandwith = KernelReadBandWidth(tile_xsize, tile_ysize, /*k,*/ kx, ky, kz, kernel_buf, inx, iny, z, /*x, y,*/
                                                  cores, /*brick_mode,*/ input_data_size, coef_compress_ratio,
                                                  /*image_compress_ratio,*/ cache_size_in_pixel, kernel_head_not_cached_fix, ppc_tmp_k, &kernelReadBWTile0, &(outputs->kernelCacheType));

            KernelReadBandWidth_Unmerged.cost = ddrReadBandwith; //KernelReadBandWidth_Unmerged_array(0); // this is ddrReadBW (refine me, should be kernelDDRRdBandwidth)
            KernelReadBandWidth_Unmerged.tile0 = kernelReadBWTile0; //KernelReadBandWidth_Unmerged_array(1); // this is kernelreadBWTile0
            SplitIntoFourRegions_Read(&KernelReadBandWidth_Unmerged, tile_xsize, tile_ysize, k, x, y, z, cores);
        }
        else
        {
            KernelReadBandWidth_Unmerged = Type_ByRegion_Multi(KernelReadBandWidth_Unmerged, 0);
        }

        AXIBusReadBandWidth = NN_cmd_size_in_byte + ImageReadBandWidth_Unmerged.cost + KernelReadBandWidth_Unmerged.cost;
        double AXIBusReadBandWidth_tile0_vzgroup0 = NN_cmd_size_in_byte + ImageReadBandWidth_Unmerged.tile0VZGroup0 + KernelReadBandWidth_Unmerged.tile0VZGroup0; // fix me, this may be zero!
        double AXIBusReadBandWidth_tile0 = NN_cmd_size_in_byte + ImageReadBandWidth_Unmerged.tile0 + KernelReadBandWidth_Unmerged.tile0;
        double AXIBusReadBandWidth_vzgroup0 = NN_cmd_size_in_byte + ImageReadBandWidth_Unmerged.vzGroup0 + KernelReadBandWidth_Unmerged.vzGroup0; // fix me, this may be zero!

        bwCost = &nnCost[APM_AXI_BUS_COST].readBW;
        bwCost->cost = AXIBusReadBandWidth;
        bwCost->tile0VZGroup0 = AXIBusReadBandWidth_tile0_vzgroup0;
        bwCost->tile0 = AXIBusReadBandWidth_tile0;
        bwCost->vzGroup0 = AXIBusReadBandWidth_vzgroup0;
    }
    else
    {
        AXIBusReadBandWidth = ddrReadBW + axiReadBW;
        bwCost = &nnCost[APM_AXI_BUS_COST].readBW;
        bwCost->cost = AXIBusReadBandWidth;
        bwCost->tile0VZGroup0 = nnCost[APM_DDR_COST].readBW.tile0VZGroup0 + nnCost[APM_AXI_SRAM_COST].readBW.tile0VZGroup0;
        bwCost->tile0 = nnCost[APM_DDR_COST].readBW.tile0 + nnCost[APM_AXI_SRAM_COST].readBW.tile0;
        bwCost->vzGroup0 = nnCost[APM_DDR_COST].readBW.vzGroup0 + nnCost[APM_AXI_SRAM_COST].readBW.vzGroup0;
    }

    // step2: DDR Read cycle
    ddrReadCycleCount = ddrReadBW / ddr_read_bw_in_byte_per_cycle;
    bwCost = &nnCost[APM_DDR_COST].readBW;
    cycleCost = &nnCost[APM_DDR_COST].readCycle;
    cycleCost->cost = ddrReadCycleCount;
    cycleCost->tile0VZGroup0 = bwCost->tile0VZGroup0 / ddr_read_bw_in_byte_per_cycle;
    cycleCost->tile0ResetVZGroup = (bwCost->tile0 - bwCost->tile0VZGroup0) / ddr_read_bw_in_byte_per_cycle;
    cycleCost->resetTileVZGroup0 = (bwCost->vzGroup0 - bwCost->tile0VZGroup0) / ddr_read_bw_in_byte_per_cycle;
    cycleCost->resetTileResetVZGroup = (bwCost->cost + bwCost->tile0VZGroup0 - bwCost->tile0 - bwCost->vzGroup0) / ddr_read_bw_in_byte_per_cycle;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.DDRRead = ddrReadCycleCount; // out put ddr Read cycle count

    /* Calculate DDRReadCycleCount_Combined_Bursts */
    if(specified_ddr_bw_limit_by_burst)
    {
        cycleCost = &nnCost[APM_DDR_READ_BURST_COMBINE_COST].readCycle;
        // refine me
        outputs->DDRReadBW_64B.cost = DDRReadBandWidth_By_Burst.BW_64B.cost;
        outputs->DDRReadBW_128B.cost = DDRReadBandWidth_By_Burst.BW_128B.cost;
        outputs->DDRReadBW_256B.cost = DDRReadBandWidth_By_Burst.BW_256B.cost;
        DDRReadCycleCount_64B.cost = DDRReadBandWidth_By_Burst.BW_64B.cost / DDR_READ_BW_IN_BYTE_PER_CYCLE_64B;
        DDRReadCycleCount_64B.tile0VZGroup0 = DDRReadBandWidth_By_Burst.BW_64B.tile0VZGroup0 / DDR_READ_BW_IN_BYTE_PER_CYCLE_64B;
        DDRReadCycleCount_64B.tile0ResetVZGroup = DDRReadBandWidth_By_Burst.BW_64B.tile0ResetVZGroup / DDR_READ_BW_IN_BYTE_PER_CYCLE_64B;
        DDRReadCycleCount_64B.resetTileVZGroup0 = DDRReadBandWidth_By_Burst.BW_64B.resetTileVZGroup0 / DDR_READ_BW_IN_BYTE_PER_CYCLE_64B;
        DDRReadCycleCount_64B.resetTileResetVZGroup = DDRReadBandWidth_By_Burst.BW_64B.resetTileResetVZGroup / DDR_READ_BW_IN_BYTE_PER_CYCLE_64B;

        DDRReadCycleCount_128B.cost = DDRReadBandWidth_By_Burst.BW_128B.cost / DDR_READ_BW_IN_BYTE_PER_CYCLE_128B;
        DDRReadCycleCount_128B.tile0VZGroup0 = DDRReadBandWidth_By_Burst.BW_128B.tile0VZGroup0 / DDR_READ_BW_IN_BYTE_PER_CYCLE_128B;
        DDRReadCycleCount_128B.tile0ResetVZGroup = DDRReadBandWidth_By_Burst.BW_128B.tile0ResetVZGroup / DDR_READ_BW_IN_BYTE_PER_CYCLE_128B;
        DDRReadCycleCount_128B.resetTileVZGroup0 = DDRReadBandWidth_By_Burst.BW_128B.resetTileVZGroup0 / DDR_READ_BW_IN_BYTE_PER_CYCLE_128B;
        DDRReadCycleCount_128B.resetTileResetVZGroup = DDRReadBandWidth_By_Burst.BW_128B.resetTileResetVZGroup / DDR_READ_BW_IN_BYTE_PER_CYCLE_128B;

        DDRReadCycleCount_256B.cost = DDRReadBandWidth_By_Burst.BW_256B.cost / DDR_READ_BW_IN_BYTE_PER_CYCLE_256B;
        DDRReadCycleCount_256B.tile0VZGroup0 = DDRReadBandWidth_By_Burst.BW_256B.tile0VZGroup0 / DDR_READ_BW_IN_BYTE_PER_CYCLE_256B;
        DDRReadCycleCount_256B.tile0ResetVZGroup = DDRReadBandWidth_By_Burst.BW_256B.tile0ResetVZGroup / DDR_READ_BW_IN_BYTE_PER_CYCLE_256B;
        DDRReadCycleCount_256B.resetTileVZGroup0 = DDRReadBandWidth_By_Burst.BW_256B.resetTileVZGroup0 / DDR_READ_BW_IN_BYTE_PER_CYCLE_256B;
        DDRReadCycleCount_256B.resetTileResetVZGroup = DDRReadBandWidth_By_Burst.BW_256B.resetTileResetVZGroup / DDR_READ_BW_IN_BYTE_PER_CYCLE_256B;

        cycleCost->cost = DDRReadCycleCount_64B.cost + DDRReadCycleCount_128B.cost + DDRReadCycleCount_256B.cost;
        cycleCost->tile0VZGroup0 = DDRReadCycleCount_64B.tile0VZGroup0 + DDRReadCycleCount_128B.tile0VZGroup0 + DDRReadCycleCount_256B.tile0VZGroup0;
        cycleCost->tile0ResetVZGroup = DDRReadCycleCount_64B.tile0ResetVZGroup + DDRReadCycleCount_128B.tile0ResetVZGroup + DDRReadCycleCount_256B.tile0ResetVZGroup;
        cycleCost->resetTileVZGroup0 = DDRReadCycleCount_64B.resetTileVZGroup0 + DDRReadCycleCount_128B.resetTileVZGroup0 + DDRReadCycleCount_256B.resetTileVZGroup0;
        cycleCost->resetTileResetVZGroup = DDRReadCycleCount_64B.resetTileResetVZGroup + DDRReadCycleCount_128B.resetTileResetVZGroup + DDRReadCycleCount_256B.resetTileResetVZGroup;


        DDRReadCycleCount_64B = Type_ByRegion_Multi(DDRReadBandWidth_By_Burst.BW_64B, (double)1 / DDR_READ_BW_IN_BYTE_PER_CYCLE_64B);
        DDRReadCycleCount_128B = Type_ByRegion_Multi(DDRReadBandWidth_By_Burst.BW_128B, (double)1 / DDR_READ_BW_IN_BYTE_PER_CYCLE_128B);
        DDRReadCycleCount_256B = Type_ByRegion_Multi(DDRReadBandWidth_By_Burst.BW_256B, (double)1 / DDR_READ_BW_IN_BYTE_PER_CYCLE_256B);
        CheckFourRegions(&DDRReadCycleCount_64B);
        CheckFourRegions(&DDRReadCycleCount_128B);
        CheckFourRegions(&DDRReadCycleCount_256B);
        DDRReadCycleCount_Combined_Bursts = Type_ByRegion_Add(DDRReadCycleCount_64B, DDRReadCycleCount_128B);
        DDRReadCycleCount_Combined_Bursts = Type_ByRegion_Add(DDRReadCycleCount_Combined_Bursts, DDRReadCycleCount_256B);


        Cycles.DDRRead_Combined_Bursts = DDRReadCycleCount_Combined_Bursts.cost;
        /* Call CheckFourRegions(DDRReadCycleCount_Combined_Bursts) */
    }

    memset(&RdReturnArbiterCycleCount, 0, sizeof(RdReturnArbiterCycleCount)); // not enable yet
    /* Calculate RdReturnArbiterCycleCount Bottleneck */
    /*
#define RD_RETURN_ARBITER_BUBBLE_BUG2038 0
    if (RD_RETURN_ARBITER_BUBBLE_BUG2038) //RD_RETURN_ARBITER_BUBBLE_BUG2038 == 0
    {
        bwCost = &nnCost[APM_DDR_READ_BURST_COST].readBW;
        double DDRReadBurst = bwCost->cost;
        double DDRReadBurst_tile0_vzgroup0 = bwCost->tile0VZGroup0;
        double DDRReadBurst_tile0 = bwCost->tile0;
        double DDRReadBurst_vzgroup0 = bwCost->vzGroup0;

        double DDRReadBurstCount = ceilf((float)DDRReadBurst / 256);
        double DDRReadBurstCount_tile0_vzgroup0 = ceilf((float)DDRReadBurst_tile0_vzgroup0 / 256);
        double DDRReadBurstCount_tile0 = ceilf((float)DDRReadBurst_tile0 / 256);
        double DDRReadBurstCount_vzgroup0 = ceilf((float)DDRReadBurst_vzgroup0 / 256);
        double DDRReadBurstCount_tile0_rest_vzgroup = DDRReadBurstCount_tile0 - DDRReadBurstCount_tile0_vzgroup0;
        double DDRReadBurstCount_rest_tile_vzgroup0 = DDRReadBurstCount_vzgroup0 - DDRReadBurstCount_tile0_vzgroup0;
        double DDRReadBurstCount_rest_tile_rest_vzgroup = DDRReadBurstCount + DDRReadBurstCount_tile0_vzgroup0 - DDRReadBurstCount_tile0 - DDRReadBurstCount_vzgroup0;

        // fill cycleCost structure
        cycleCost = &nnCost[APM_DDR_READ_BURST_COST].readCycle;
        cycleCost->cost = DDRReadBurstCount;
        cycleCost->tile0VZGroup0 = DDRReadBurstCount_tile0_vzgroup0;
        cycleCost->tile0ResetVZGroup = DDRReadBurstCount_tile0_rest_vzgroup;
        cycleCost->resetTileVZGroup0 = DDRReadBurstCount_rest_tile_vzgroup0;
        cycleCost->resetTileResetVZGroup = DDRReadBurstCount_rest_tile_rest_vzgroup;

        //Cycles.DDRRead = ddrReadCycleCount; // out put ddr Read cycle count

        //Debug.Assert (Round(DDRReadBurstCount - (DDRReadBurstCount_tile0_vzgroup0 + DDRReadBurstCount_tile0_rest_vzgroup + DDRReadBurstCount_rest_tile_vzgroup0 + DDRReadBurstCount_rest_tile_rest_vzgroup)) = 0)
        //Debug.Assert (DDRReadBurstCount_tile0_vzgroup0 >= -0.1)
        //Debug.Assert (DDRReadBurstCount_tile0_rest_vzgroup >= -0.1)
        //Debug.Assert (DDRReadBurstCount_rest_tile_vzgroup0 >= -0.1)
        //Debug.Assert (DDRReadBurstCount_rest_tile_rest_vzgroup >= -1) 'used Ceil() to get integer result but it may cause certain BurstCount to be -1

        bwCost = &nnCost[APM_DDR_COST].readBW;
        double DDRReadBandWidth= bwCost->cost;
        double DDRReadBandWidth_tile0_vzgroup0 = bwCost->tile0VZGroup0;
        double DDRReadBandWidth_tile0 = bwCost->tile0;
        double DDRReadBandWidth_vzgroup0 = bwCost->vzGroup0;

        double RdReturnArbiterCycleCount = DDRReadBandWidth / 16;
        double RdReturnArbiterCycleCount_tile0_vzgroup0 = DDRReadBandWidth_tile0_vzgroup0 / 16;
        double RdReturnArbiterCycleCount_tile0_rest_vzgroup = (DDRReadBandWidth_tile0 - DDRReadBandWidth_tile0_vzgroup0) / 16;
        double RdReturnArbiterCycleCount_rest_tile_vzgroup0 = (DDRReadBandWidth_vzgroup0 - DDRReadBandWidth_tile0_vzgroup0) / 16;
        double RdReturnArbiterCycleCount_rest_tile_rest_vzgroup = (DDRReadBandWidth + DDRReadBandWidth_tile0_vzgroup0 - DDRReadBandWidth_tile0 - DDRReadBandWidth_vzgroup0) / 16;

        //Debug.Assert (Round(RdReturnArbiterCycleCount - (RdReturnArbiterCycleCount_tile0_vzgroup0 + RdReturnArbiterCycleCount_tile0_rest_vzgroup + RdReturnArbiterCycleCount_rest_tile_vzgroup0 + RdReturnArbiterCycleCount_rest_tile_rest_vzgroup)) = 0)
        //Debug.Assert (RdReturnArbiterCycleCount_tile0_vzgroup0 >= -0.1)
        //Debug.Assert (RdReturnArbiterCycleCount_tile0_rest_vzgroup >= -0.1)
        //Debug.Assert (RdReturnArbiterCycleCount_rest_tile_vzgroup0 >= -0.1)
        //Debug.Assert (RdReturnArbiterCycleCount_rest_tile_rest_vzgroup >= -0.1)

        RdReturnArbiterCycleCount = RdReturnArbiterCycleCount + DDRReadBurstCount;
        RdReturnArbiterCycleCount_tile0_vzgroup0 = RdReturnArbiterCycleCount_tile0_vzgroup0 + DDRReadBurstCount_tile0_vzgroup0;
        RdReturnArbiterCycleCount_tile0_rest_vzgroup = RdReturnArbiterCycleCount_tile0_rest_vzgroup + DDRReadBurstCount_tile0_rest_vzgroup;
        RdReturnArbiterCycleCount_rest_tile_vzgroup0 = RdReturnArbiterCycleCount_rest_tile_vzgroup0 + DDRReadBurstCount_rest_tile_vzgroup0;
        RdReturnArbiterCycleCount_rest_tile_rest_vzgroup = RdReturnArbiterCycleCount_rest_tile_rest_vzgroup + DDRReadBurstCount_rest_tile_rest_vzgroup;

        cycleCost = &nnCost[APM_RD_RETURN_ARB_COST].readCycle;
        cycleCost->cost = RdReturnArbiterCycleCount;
        cycleCost->tile0VZGroup0 = RdReturnArbiterCycleCount_tile0_vzgroup0;
        cycleCost->tile0ResetVZGroup = RdReturnArbiterCycleCount_tile0_rest_vzgroup;
        cycleCost->resetTileVZGroup0 = RdReturnArbiterCycleCount_rest_tile_vzgroup0;
        cycleCost->resetTileResetVZGroup = RdReturnArbiterCycleCount_rest_tile_rest_vzgroup;

        //Debug.Assert (Round(RdReturnArbiterCycleCount - (RdReturnArbiterCycleCount_tile0_vzgroup0 + RdReturnArbiterCycleCount_tile0_rest_vzgroup + RdReturnArbiterCycleCount_rest_tile_vzgroup0 + RdReturnArbiterCycleCount_rest_tile_rest_vzgroup)) = 0)
        //Debug.Assert (RdReturnArbiterCycleCount_tile0_vzgroup0 >= -0.1)
        //Debug.Assert (RdReturnArbiterCycleCount_tile0_rest_vzgroup >= -0.1)
        //Debug.Assert (RdReturnArbiterCycleCount_rest_tile_vzgroup0 >= -0.1)
        //Debug.Assert (RdReturnArbiterCycleCount_rest_tile_rest_vzgroup >= -0.1)
        Cycles.RdReturnArbiter = RdReturnArbiterCycleCount;
    }
    */
    // step3: AXISRAM read cycle
    axiReadCycleCount = axiReadBW / adjustedAXISramReadBandWidthLimit;
    bwCost = &nnCost[APM_AXI_SRAM_COST].readBW;
    cycleCost = &nnCost[APM_AXI_SRAM_COST].readCycle;
    cycleCost->cost = axiReadCycleCount;
    cycleCost->tile0VZGroup0 = bwCost->tile0VZGroup0 / adjustedAXISramReadBandWidthLimit;
    cycleCost->tile0ResetVZGroup = (bwCost->tile0 - bwCost->tile0VZGroup0) / adjustedAXISramReadBandWidthLimit;
    cycleCost->resetTileVZGroup0 = (bwCost->vzGroup0 - bwCost->tile0VZGroup0) / adjustedAXISramReadBandWidthLimit;
    cycleCost->resetTileResetVZGroup = (bwCost->cost + bwCost->tile0VZGroup0 - bwCost->tile0 - bwCost->vzGroup0) / adjustedAXISramReadBandWidthLimit;;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.AXISRAMRead = axiReadCycleCount; /* AXI SRAM READ CYCLE COUNT */

    // step4: AXIBus Read cycle count
    axiBusReadCycleCount = AXIBusReadBandWidth / axi_bus_read_bw_limit;
    bwCost = &nnCost[APM_AXI_BUS_COST].readBW;
    cycleCost = &nnCost[APM_AXI_BUS_COST].readCycle;
    cycleCost->cost = axiBusReadCycleCount;
    cycleCost->tile0VZGroup0 = bwCost->tile0VZGroup0 / axi_bus_read_bw_limit;
    cycleCost->tile0ResetVZGroup = (bwCost->tile0 - bwCost->tile0VZGroup0) / axi_bus_read_bw_limit;
    cycleCost->resetTileVZGroup0 = (bwCost->vzGroup0 - bwCost->tile0VZGroup0) / axi_bus_read_bw_limit;
    cycleCost->resetTileResetVZGroup = (bwCost->cost + bwCost->tile0VZGroup0 - bwCost->tile0 - bwCost->vzGroup0) / axi_bus_read_bw_limit;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.AXIBUSRead = axiBusReadCycleCount; /* AXI BUS READ CYCLE COUNT */

    // step5: interlalKernelRead bandwidth cycle count
    double InternalKernelReadBW = max(KernelVIPSRAMReadBandWidth, max(KernelAXISRAMReadBandWidth, ddrKernelReadBW));
    double InternalKernelReadBW_tile0 = max(KernelVIPSRAMReadBandWidth_tile0, max(KernelAXISRAMReadBandWidth_tile0, ddrKernelReadBWTile0));
    double InternalKernelReadBW_vzgroup0 = InternalKernelReadBW * min((float)k * cores / z, 1);
    double InternalKernelReadBW_tile0_vzgroup0 = InternalKernelReadBW_tile0 * min((float)k * cores / z, 1);

    InternalKernelReadCycleCount.cost = (double)InternalKernelReadBW / Internal_Kernel_Read_BYTE_PER_CYCLE;
    InternalKernelReadCycleCount.tile0VZGroup0 = (double)InternalKernelReadBW_tile0_vzgroup0 / Internal_Kernel_Read_BYTE_PER_CYCLE;
    InternalKernelReadCycleCount.tile0ResetVZGroup = (double)(InternalKernelReadBW_tile0 - InternalKernelReadBW_tile0_vzgroup0) / Internal_Kernel_Read_BYTE_PER_CYCLE;
    InternalKernelReadCycleCount.resetTileVZGroup0 = (double)(InternalKernelReadBW_vzgroup0 - InternalKernelReadBW_tile0_vzgroup0) / Internal_Kernel_Read_BYTE_PER_CYCLE;
    InternalKernelReadCycleCount.resetTileResetVZGroup = (double)(InternalKernelReadBW + InternalKernelReadBW_tile0_vzgroup0 - InternalKernelReadBW_tile0 - InternalKernelReadBW_vzgroup0) / Internal_Kernel_Read_BYTE_PER_CYCLE;
    Cycles.InternalKernelRead = InternalKernelReadCycleCount.cost;

    // step6: DDR write bandwidth
    ddrWriteBW = 0;
    axiWriteBW = 0;
    vipWriteBW = ddrReadBW;
    if (dst_buf == SW_TILING_FROM_DDR)
    {
        ddrWriteBW = WriteBandWidth(tile_xsize, tile_ysize, x, y, z, output_data_size, image_compress_ratio, usc_cache_size, pooling_stride, out_image_stride, out_image_slice, is_nn_write_without_usc, SW_TILING_FROM_DDR, 0);    /* burst size set to 0 for not used */
        /* Calculate BW by bursts and by (non)mask */
        if(specified_ddr_bw_limit_by_burst)
        {
            APM_WR_BW_BYBURST_T result_tmp; memset(&result_tmp, 0, sizeof(result_tmp));

            WriteBandWidth_By_BurstSize(tile_xsize, tile_ysize, x, y, z, k, cores, dst_buf, output_data_size, image_compress_ratio, usc_cache_size, pooling_stride, zdp, out_image_stride, out_image_slice,is_nn_write_without_usc, &result_tmp);
            memcpy(&DDRWriteBandWidth_NonMask, &(result_tmp.nonmask), sizeof(arch_model_bw_byburst_type));
            memcpy(&DDRWriteBandWidth_Mask, &(result_tmp.mask), sizeof(arch_model_bw_byburst_type));
        }
    }
    else if (dst_buf == SW_TILING_FROM_AXI_SRAM)
    {
        axiWriteBW = WriteBandWidth(tile_xsize, tile_ysize, x, y, z, output_data_size, image_compress_ratio, usc_cache_size, pooling_stride, out_image_stride, out_image_slice, is_nn_write_without_usc, SW_TILING_FROM_AXI_SRAM, 0);   /* burst size set to 0 for not used */
    }
    else
    {
        vipWriteBW = vipWriteBW + WriteBandWidth(tile_xsize, tile_ysize, x, y, z, output_data_size, 1, usc_cache_size, pooling_stride, out_image_stride, out_image_slice, is_nn_write_without_usc, SW_TILING_FROM_VIP_SRAM, 0); /* burst size set to 0 for not used */
    }
    /* when output and transpose enable, write to VIP SRAM first */
    if(pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_out > 1)
    {
        if(dst_buf == SW_TILING_FROM_DDR)
        {
            vipWriteBW = vipWriteBW + ddrWriteBW;
        }
        else if(dst_buf == SW_TILING_FROM_AXI_SRAM)
        {
            vipWriteBW = vipWriteBW + axiWriteBW;
        }
    }

    axiBusWriteBW = ddrWriteBW + axiWriteBW;

    bwCost = &nnCost[APM_DDR_COST].writeBW;
    bwCost->cost = ddrWriteBW;
    bwCost->tile0VZGroup0 = 0;
    bwCost->tile0 = ddrWriteBW * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) * (1 - 1.0f * tail_z / z);
    bwCost->vzGroup0 = ddrWriteBW * (1.0f * tail_z / z) * (1 - 1.0f * tail_x / x * tail_y / y);
    bwCost->residual = ddrWriteBW * tail_z / z * tail_x / x * tail_y / y;

    ddrWriteCycleCount = ddrWriteBW / ddr_write_bw_in_byte_per_cycle;
    cycleCost = &nnCost[APM_DDR_COST].writeCycle;
    cycleCost->cost = ddrWriteCycleCount;
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = nnCost[APM_DDR_COST].writeBW.tile0 / ddr_write_bw_in_byte_per_cycle;
    cycleCost->resetTileVZGroup0 = nnCost[APM_DDR_COST].writeBW.vzGroup0 / ddr_write_bw_in_byte_per_cycle;
    cycleCost->resetTileResetVZGroup = (ddrWriteBW - nnCost[APM_DDR_COST].writeBW.tile0 - nnCost[APM_DDR_COST].writeBW.vzGroup0) / ddr_write_bw_in_byte_per_cycle;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    /* Calculate DDRWriteCycleCount_By_Burst */
    if(specified_ddr_bw_limit_by_burst)
    {

        cycleCost = &nnCost[APM_DDR_READ_BURST_COMBINE_COST].writeCycle;

        /* non mask */
        SplitIntoFourRegions_Write(&(DDRWriteBandWidth_NonMask.BW_64B), tile_xsize, tile_ysize, k, x, y, z, cores);
        SplitIntoFourRegions_Write(&(DDRWriteBandWidth_NonMask.BW_128B), tile_xsize, tile_ysize, k, x, y, z, cores);
        SplitIntoFourRegions_Write(&(DDRWriteBandWidth_NonMask.BW_256B), tile_xsize, tile_ysize, k, x, y, z, cores);
        /* mask */
        SplitIntoFourRegions_Write(&(DDRWriteBandWidth_Mask.BW_64B), tile_xsize, tile_ysize, k, x, y, z, cores);
        SplitIntoFourRegions_Write(&(DDRWriteBandWidth_Mask.BW_128B), tile_xsize, tile_ysize, k, x, y, z, cores);
        SplitIntoFourRegions_Write(&(DDRWriteBandWidth_Mask.BW_256B), tile_xsize, tile_ysize, k, x, y, z, cores);

        DDRWriteCycleCount_64B = Type_ByRegion_Multi(DDRWriteBandWidth_NonMask.BW_64B, (double)1 / DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_64B);
        DDRWriteCycleCount_64B = Type_ByRegion_Add(DDRWriteCycleCount_64B, Type_ByRegion_Multi(DDRWriteBandWidth_Mask.BW_64B, (double)1 / DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_64B));

        DDRWriteCycleCount_128B = Type_ByRegion_Multi(DDRWriteBandWidth_NonMask.BW_128B, (double)1 / DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_128B);
        DDRWriteCycleCount_128B = Type_ByRegion_Add(DDRWriteCycleCount_128B, Type_ByRegion_Multi(DDRWriteBandWidth_Mask.BW_128B, (double)1 / DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_128B));

        DDRWriteCycleCount_256B = Type_ByRegion_Multi(DDRWriteBandWidth_NonMask.BW_256B, (double)1 / DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_256B);
        DDRWriteCycleCount_256B = Type_ByRegion_Add((DDRWriteCycleCount_256B),Type_ByRegion_Multi((DDRWriteBandWidth_Mask.BW_256B), (double)1 / DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_256B));

        DDRWriteCycleCount_Combined_Bursts = Type_ByRegion_Add(DDRWriteCycleCount_64B, DDRWriteCycleCount_128B);
        DDRWriteCycleCount_Combined_Bursts = Type_ByRegion_Add(DDRWriteCycleCount_Combined_Bursts, DDRWriteCycleCount_256B);
    //With DDRWriteBandWidth_NonMask
      outputs->DDRNonMaskWriteBW_64B.cost = DDRWriteBandWidth_NonMask.BW_64B.cost;
      outputs->DDRNonMaskWriteBW_128B.cost = DDRWriteBandWidth_NonMask.BW_128B.cost;
      outputs->DDRNonMaskWriteBW_256B.cost = DDRWriteBandWidth_NonMask.BW_256B.cost;
    //End With
    //With DDRWriteBandWidth_Mask
      outputs->DDRMaskWriteBW_64B.cost = DDRWriteBandWidth_Mask.BW_64B.cost;
      outputs->DDRMaskWriteBW_128B.cost = DDRWriteBandWidth_Mask.BW_128B.cost;
      outputs->DDRMaskWriteBW_256B.cost = DDRWriteBandWidth_Mask.BW_256B.cost;
    //End With
      Cycles.DDRWrite_Combined_Bursts = DDRWriteCycleCount_Combined_Bursts.cost;

        CheckFourRegions(&DDRWriteCycleCount_Combined_Bursts);
    }
    /* update VIP SRAM bandwith because of transpose write */
    if((pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_out > 1)
        && (dst_buf == SW_TILING_FROM_DDR || dst_buf == SW_TILING_FROM_AXI_SRAM))
    {
        APM_BW_CYCLE_COST_T *bwDdrReadCost = &nnCost[APM_DDR_COST].writeBW;
        bwCost = &nnCost[APM_VIP_SRAM_COST].readBW;
        bwCost->cost = bwCost->cost + bwDdrReadCost->cost;
        bwCost->tile0VZGroup0 = bwCost->tile0VZGroup0 + bwDdrReadCost->tile0VZGroup0;
        bwCost->tile0 = bwCost->tile0 + bwDdrReadCost->tile0;
        bwCost->vzGroup0 = bwCost->vzGroup0 + bwDdrReadCost->vzGroup0;

        cycleCost = &nnCost[APM_VIP_SRAM_COST].readCycle;
        cycleCost->cost = bwCost->cost / l2cache_width;
        cycleCost->tile0VZGroup0 = bwCost->tile0VZGroup0 / l2cache_width;
        cycleCost->tile0ResetVZGroup = (bwCost->tile0 - bwCost->tile0VZGroup0)/ l2cache_width;
        cycleCost->resetTileVZGroup0 = (bwCost->vzGroup0 - bwCost->tile0VZGroup0) / l2cache_width;
        cycleCost->resetTileResetVZGroup = (bwCost->cost + bwCost->tile0VZGroup0 - bwCost->tile0 - bwCost->vzGroup0) /l2cache_width;
        assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    }

    // step7: AXISRAM write bandwidth cycle count
    bwCost = &nnCost[APM_AXI_SRAM_COST].writeBW;
    bwCost->cost = axiWriteBW;
    bwCost->tile0VZGroup0 = 0;
    bwCost->tile0 = axiWriteBW * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) * (1 - 1.0f * tail_z / z);
    bwCost->vzGroup0 = axiWriteBW * (1.0f * tail_z / z) * (1 - 1.0f * tail_x / x * tail_y / y);
    bwCost->residual = axiWriteBW * tail_z / z * tail_x / x * tail_y / y;
    axiWriteCycleCount = axiWriteBW / axi_sram_write_bw_limit;
    cycleCost = &nnCost[APM_AXI_SRAM_COST].writeCycle;
    cycleCost->cost = axiWriteCycleCount;
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = bwCost->tile0 / axi_sram_write_bw_limit;
    cycleCost->resetTileVZGroup0 = bwCost->vzGroup0 / axi_sram_write_bw_limit;
    cycleCost->resetTileResetVZGroup = (axiWriteBW - bwCost->tile0 - bwCost->vzGroup0) / axi_sram_write_bw_limit;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    // step8: AXI_BUS write bandwidth cycle count
    bwCost = &nnCost[APM_AXI_BUS_COST].writeBW;
    bwCost->cost = axiBusWriteBW;       /* ?? */
    bwCost->tile0VZGroup0 = 0;
    bwCost->tile0 = axiBusWriteBW * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) * (1 - 1.0f * tail_z / z);
    bwCost->vzGroup0 = axiBusWriteBW * (1.0f * tail_z / z) * (1 - 1.0f * tail_x / x * tail_y / y);
    bwCost->residual = axiBusWriteBW * tail_z / z * tail_x / x * tail_y / y;
    axiBusWriteCycleCount = axiBusWriteBW / axi_bus_write_bw_limit;
    cycleCost = &nnCost[APM_AXI_BUS_COST].writeCycle;
    cycleCost->cost = axiBusWriteCycleCount;
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = bwCost->tile0 / axi_bus_write_bw_limit;
    cycleCost->resetTileVZGroup0 = bwCost->vzGroup0 / axi_bus_write_bw_limit;
    cycleCost->resetTileResetVZGroup = (axiBusWriteBW - bwCost->tile0 - bwCost->vzGroup0) / axi_bus_write_bw_limit;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    // step9: VIPSRAM write bandwidth
    bwCost = &nnCost[APM_VIP_SRAM_COST].writeBW;
    bwCost->cost = vipWriteBW;
    bwCost->tile0VZGroup0 = 0;
    bwCost->tile0 = vipWriteBW * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) * (1 - 1.0f * tail_z / z);
    bwCost->vzGroup0 = vipWriteBW * (1.0f * tail_z / z) * (1 - 1.0f * tail_x / x * tail_y / y);
    bwCost->residual = vipWriteBW * tail_z / z * tail_x / x * tail_y / y;
    vipWriteCycleCount = vipWriteBW / l2cache_width;
    cycleCost = &nnCost[APM_VIP_SRAM_COST].writeCycle;
    cycleCost->cost = vipWriteCycleCount;
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = bwCost->tile0 / l2cache_width;
    cycleCost->resetTileVZGroup0 = bwCost->vzGroup0 / l2cache_width;
    cycleCost->resetTileResetVZGroup = (vipWriteBW - bwCost->tile0 - bwCost->vzGroup0) / l2cache_width;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    Cycles.DDRWrite = ddrWriteCycleCount;
    Cycles.AXISRAMWrite = axiWriteCycleCount; //AXISRAMWriteCycleCount;
    Cycles.AXIBUSWrite  = axiBusWriteCycleCount; //AXIBusWriteCycleCount
    Cycles.VIPSRAMWrite = vipWriteCycleCount; //VIPSRAMWriteCycleCount;

    double slowInternalWriteCycleCount = 0, slowCompCycleCount = 0, DWOutCycleCount = 0;
    bool isV8 = (xydp_x == 0 && xydp_y == 0) ? true : false;
    float tmp_internal_write_bw_limit = internal_write_bw_limit;
    unsigned int zdpLoopCount = 3;
    // step10: internal write, slow write
    if (isV8) /*for v8*/
    {
        float slowInternalWriteBWLimit;

        bool NO_NARROW_POST_PROCESS_PIPE = pContext->bf.NO_NARROW_POST_PROCESS_PIPE;
        NO_NARROW_POST_PROCESS_PIPE = (pContext->manualFeatureSetted) ?
                                      pContext->pManualFeatures->no_narrow_post_process_pipe:
                                      NO_NARROW_POST_PROCESS_PIPE;

        if ((input_data_size == 16 || ((kx * ky * kz) > 512)) && (NO_NARROW_POST_PROCESS_PIPE == 0))
        {   //' pick the narrow post process pipe
            tmp_internal_write_bw_limit = tmp_internal_write_bw_limit / 2;
        }

        bool isTA_MERGE = pContext->pInPerfParams->cmdInfo.isTA_MERGE;
        if (isTA_MERGE) //Then ' half of the cycles are used to do the tensor add
        {
            tmp_internal_write_bw_limit = tmp_internal_write_bw_limit / 2;
        }
        if (kx == 1 && ky == 1)
        {
            slowInternalWriteBWLimit = min(tmp_internal_write_bw_limit, (float)lanes_per_conv / zdpLoopCount);
            slowInternalWriteCycleCount = ceilf((float)tile_xsize * interleave_mode / slowInternalWriteBWLimit) * ceilf((float)x / tile_xsize) * ceilf((float)tile_ysize / interleave_mode) * ceilf((float)y / tile_ysize) * z * (input_data_size * input_data_size) / (8 * 8);
            slowCompCycleCount = CompCycleCount + ceilf((float)tile_xsize * interleave_mode / lanes_per_conv) * ceilf((float)x / tile_xsize) * ceilf((float)tile_ysize / interleave_mode) * ceilf((float)y / tile_ysize) * ceilf((float)z / cores) * (input_data_size * input_data_size) / (8 * 8);
            if (pContext->pInPerfParams->cmdInfo.is_depth_wise_merge)
            {
                //DW_MERGE always takes the slow compute path and gives higher pirority to output
                slowCompCycleCount = CompCycleCount + ceilf((float)tile_xsize * interleave_mode / lanes_per_conv) * ceilf((float)x / tile_xsize) * ceilf((float)tile_ysize / interleave_mode) * ceilf((float)y / tile_ysize) * ceilf((float)z / (cores - 1)) * (input_data_size * input_data_size) / (8 * 8); // ' 2 data_size here as we do 4X INT8 results for INT16
                internalWriteCycleCount = ceilf((float)tile_xsize * interleave_mode / tmp_internal_write_bw_limit) * ceilf((float)x / tile_xsize) * ceilf((float)tile_ysize / interleave_mode) * ceilf((float)y / tile_ysize) * (kz + z) * (input_data_size * input_data_size) / (8 * 8); // ' 2 data_size here as we do 4X INT8 results for INT16
                CompCycleCount = slowCompCycleCount;
            }
            else if ((nn_slow_output_feature == 0) || (slowInternalWriteCycleCount > slowCompCycleCount))
            {
                CompCycleCount = slowCompCycleCount;
                internalWriteCycleCount = ceilf((float)tile_xsize * interleave_mode / tmp_internal_write_bw_limit) * ceilf((float)x / tile_xsize) * ceilf((float)tile_ysize / interleave_mode) * ceilf((float)y / tile_ysize) * z * (input_data_size * input_data_size) / (8 * 8);
            }
            else
            {
                internalWriteCycleCount = slowInternalWriteCycleCount;
            }
        }
        else
        {
            slowInternalWriteBWLimit = min(tmp_internal_write_bw_limit, (float)lanes_per_conv / zdpLoopCount);
            slowInternalWriteCycleCount = ceilf((float)tile_xsize * (tile_ysize / pooling_stride) / slowInternalWriteBWLimit) * pooling_stride * ceilf((float)x / tile_xsize) * ceilf((float)y / tile_ysize) * z * (input_data_size * input_data_size) / (8 * 8);
            slowCompCycleCount = CompCycleCount + ceilf((float)tile_xsize * (tile_ysize / pooling_stride) / lanes_per_conv) * pooling_stride * ceilf((float)x / tile_xsize) * ceilf((float)y / tile_ysize) * ceilf((float)z / cores) * (input_data_size * input_data_size) / (8 * 8);
            if (pContext->pInPerfParams->cmdInfo.is_depth_wise_merge)
            {
                //DW_MERGE always takes the slow compute path and gives higher pirority to output
                slowCompCycleCount = CompCycleCount + ceilf((float)tile_xsize * (tile_ysize / pooling_stride) / lanes_per_conv) * pooling_stride * ceilf((float)x / tile_xsize) * ceilf((float)y / tile_ysize) * ceilf((float)z / (cores - 1)) * (input_data_size * input_data_size) / (8 * 8); // ' 2 data_size here as we do 4X INT8 results for INT16
                internalWriteCycleCount = ceilf((float)tile_xsize * (tile_ysize / pooling_stride) / tmp_internal_write_bw_limit) * pooling_stride * ceilf((float)x / tile_xsize) * ceilf((float)y / tile_ysize) * (kz + z) * (input_data_size * input_data_size) / (8 * 8); // ' 2 data_size here as we do 4X INT8 results for INT16
                CompCycleCount = slowCompCycleCount;
            }
            else if ((nn_slow_output_feature == 0) || (slowInternalWriteCycleCount > slowCompCycleCount))
            {
                CompCycleCount = slowCompCycleCount;
                internalWriteCycleCount = ceilf((float)tile_xsize * (tile_ysize / pooling_stride) / tmp_internal_write_bw_limit) * pooling_stride * ceilf((float)x / tile_xsize) * ceilf((float)y / tile_ysize) * z * (input_data_size * input_data_size) / (8 * 8);
            }
            else
            {
                internalWriteCycleCount = slowInternalWriteCycleCount;
            }
        }
    }
    else
    {
        internalWriteCycleCount = ceilf(1.0f * tile_xsize * interleave_mode / tmp_internal_write_bw_limit) * ceilf(1.0f * x / tile_xsize) * ceilf(1.0f * tile_ysize / interleave_mode) * ceilf(1.0f * y / tile_ysize) * z * input_data_size / 8;
    }
    // step11: computer cycle count t0vz0
    cycleCost = &computeCycleCost;
    cycleCost->cost = CompCycleCount; /* ?? */
    cycleCost->tile0VZGroup0 = CompCycleCount * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) * min((1.0f * k * cores / z), 1.0f);
    cycleCost->tile0ResetVZGroup = CompCycleCount * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) - cycleCost->tile0VZGroup0;
    cycleCost->resetTileVZGroup0 = CompCycleCount * min((1.0f * k * cores / z), 1.0f) - cycleCost->tile0VZGroup0;
    cycleCost->resetTileResetVZGroup = CompCycleCount - cycleCost->tile0VZGroup0 - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    // step12: depthwise merge out cycle count t0vz0 ??

    // step13: internal write cycle count t0 vz0
    cycleCost = &internalWriteCycleCost;
    cycleCost->cost = internalWriteCycleCount;  /* ?? */
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = internalWriteCycleCount * (1.0f / ceilf(1.0f * x / tile_xsize)) * (1.0f / ceilf(1.0f * y / tile_ysize)) * (1 - 1 / ceilf(1.0f * z / (k * cores)));
    cycleCost->resetTileVZGroup0 = internalWriteCycleCount * (1.0f - 1 / (ceilf(1.0f * x / tile_xsize) * ceilf(1.0f * y / tile_ysize))) * (1 / ceilf(1.0f * z / (k * cores)));
    cycleCost->resetTileResetVZGroup = internalWriteCycleCount - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    Cycles.Compute = CompCycleCount;
    Cycles.Slow_Comp = slowCompCycleCount; // ' no need to output this
    Cycles.Slow_InternalWrite = slowInternalWriteCycleCount; //' no need to output this
    Cycles.InternalWrite = internalWriteCycleCount;
    Cycles.DWOut = DWOutCycleCount;

    // step14: ddr total bandwidth
    ddrTotalBW = ddrReadBW + ddrWriteBW;
    bwCost = &ddrTotalBWCost;
    bwCost->cost = ddrTotalBW;
    bwCost->tile0VZGroup0 = nnCost[APM_DDR_COST].readBW.tile0VZGroup0 + nnCost[APM_DDR_COST].writeBW.tile0VZGroup0;
    bwCost->tile0 = nnCost[APM_DDR_COST].readBW.tile0 + nnCost[APM_DDR_COST].writeBW.tile0;
    bwCost->vzGroup0 = nnCost[APM_DDR_COST].readBW.vzGroup0 + nnCost[APM_DDR_COST].writeBW.vzGroup0;

    ddrTotalCycleCount = ddrTotalBW / ddr_total_bw_in_byte_per_cycle;
    cycleCost = &ddrTotalCycleCost;
    cycleCost->cost = ddrTotalCycleCount;
    cycleCost->tile0VZGroup0 = ddrTotalBWCost.tile0VZGroup0 / ddr_total_bw_in_byte_per_cycle;
    cycleCost->tile0ResetVZGroup = (ddrTotalBWCost.tile0 - ddrTotalBWCost.tile0VZGroup0) / ddr_total_bw_in_byte_per_cycle;
    cycleCost->resetTileVZGroup0 = (ddrTotalBWCost.vzGroup0 - ddrTotalBWCost.tile0VZGroup0) / ddr_total_bw_in_byte_per_cycle;
    cycleCost->resetTileResetVZGroup = (ddrTotalBW + ddrTotalBWCost.tile0VZGroup0 - ddrTotalBWCost.tile0 - ddrTotalBWCost.vzGroup0) / ddr_total_bw_in_byte_per_cycle;

    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.DDRTotal = cycleCost->cost; /* DDRTotal cycle cost */

    // step15: AXISRAM total
    axiTotalBW = axiReadBW + axiWriteBW;
    bwCost = &axiTotalBWCost;
    bwCost->cost = axiTotalBW;
    bwCost->tile0VZGroup0 = nnCost[APM_AXI_SRAM_COST].readBW.tile0VZGroup0 + nnCost[APM_AXI_SRAM_COST].writeBW.tile0VZGroup0;
    bwCost->tile0 = nnCost[APM_AXI_SRAM_COST].readBW.tile0 + nnCost[APM_AXI_SRAM_COST].writeBW.tile0;
    bwCost->vzGroup0 = nnCost[APM_AXI_SRAM_COST].readBW.vzGroup0 + nnCost[APM_AXI_SRAM_COST].writeBW.vzGroup0;

    cycleCost = &axiTotalCycleCost;
    axiTotalCycleCount = axiTotalBW / axi_sram_total_bw_limit;
    cycleCost->cost = axiTotalCycleCount;
    cycleCost->tile0VZGroup0 = axiTotalBWCost.tile0VZGroup0 / axi_sram_total_bw_limit;
    cycleCost->tile0ResetVZGroup = (axiTotalBWCost.tile0 - axiTotalBWCost.tile0VZGroup0) / axi_sram_total_bw_limit;
    cycleCost->resetTileVZGroup0 = (axiTotalBWCost.vzGroup0 - axiTotalBWCost.tile0VZGroup0) / axi_sram_total_bw_limit;
    cycleCost->resetTileResetVZGroup = (axiTotalBW + axiTotalBWCost.tile0VZGroup0 - axiTotalBWCost.tile0 - axiTotalBWCost.vzGroup0) / axi_sram_total_bw_limit;

    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.AXISRAMTotal = cycleCost->cost; /* AXISRAMTotal cycle cost */
    // step16: AXI bug total
    axiBusTotalBW = AXIBusReadBandWidth + axiBusWriteBW;
    bwCost = &axiBusTotalBWCost;
    bwCost->cost = axiBusTotalBW;
    bwCost->tile0VZGroup0 = nnCost[APM_AXI_BUS_COST].readBW.tile0VZGroup0 + nnCost[APM_AXI_BUS_COST].writeBW.tile0VZGroup0;
    bwCost->tile0 = nnCost[APM_AXI_BUS_COST].readBW.tile0 + nnCost[APM_AXI_BUS_COST].writeBW.tile0;
    bwCost->vzGroup0 = nnCost[APM_AXI_BUS_COST].readBW.vzGroup0 + nnCost[APM_AXI_BUS_COST].writeBW.vzGroup0;
    axiBusTotalCycleCount = axiBusTotalBW / axi_bus_total_bw_limit;

    cycleCost = &axiBusTotalCycleCost;
    cycleCost->cost = axiBusTotalCycleCount;
    cycleCost->tile0VZGroup0 = axiBusTotalBWCost.tile0VZGroup0 / axi_bus_total_bw_limit;
    cycleCost->tile0ResetVZGroup = (axiBusTotalBWCost.tile0 - axiBusTotalBWCost.tile0VZGroup0) / axi_bus_total_bw_limit;
    cycleCost->resetTileVZGroup0 = (axiBusTotalBWCost.vzGroup0 - axiBusTotalBWCost.tile0VZGroup0) / axi_bus_total_bw_limit;
    cycleCost->resetTileResetVZGroup = (axiBusTotalBW + axiBusTotalBWCost.tile0VZGroup0 - axiBusTotalBWCost.tile0 - axiBusTotalBWCost.vzGroup0) / axi_bus_total_bw_limit;

    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.AXIBUSTotal = cycleCost->cost; /* AXIBUSTotal cycle cost */

#define KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE  16
#define INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE 16
    // stpep17: kernel DDR read cycle
    bwCost = &nnCost[APM_DDR_KERNEL_COST].readBW;
    cycleCost = &nnCost[APM_DDR_KERNEL_COST].readCycle;
    cycleCost->cost = bwCost->cost / KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->tile0VZGroup0 = bwCost->tile0 * min((1.0f * k * cores / z), 1.0f) / KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->tile0ResetVZGroup = bwCost->tile0 / KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE - cycleCost->tile0VZGroup0;
    cycleCost->resetTileVZGroup0 = (bwCost->cost - bwCost->tile0) * min((1.0f * k * cores / z), 1.0f) / KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->resetTileResetVZGroup = cycleCost->cost - cycleCost->tile0VZGroup0 - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.KernelDDRRead = cycleCost->cost;

    // step18: inimage DDR read cycle
    bwCost = &nnCost[APM_DDR_IN_IMAGE_COST].readBW;
    cycleCost = &nnCost[APM_DDR_IN_IMAGE_COST].readCycle;
    cycleCost->cost = bwCost->cost / INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->tile0VZGroup0 = bwCost->vzGroup0 * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) / INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->tile0ResetVZGroup = (bwCost->cost - bwCost->vzGroup0) * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) / INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->resetTileVZGroup0 = bwCost->vzGroup0 / INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE - cycleCost->tile0VZGroup0;
    cycleCost->resetTileResetVZGroup = cycleCost->cost - cycleCost->tile0VZGroup0 - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0,1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.InImageDDRRead = cycleCost->cost;

    double KernelRepeatedRead = _calcKernel4DSingleReadRepeated(tile_xsize, tile_ysize, x, y);
    imageRepeatedSingleRead = Tile3DImageSingleReadRepeated(z, k, cores);

    if (pContext->pInPerfParams->cmdInfo.is_depth_wise)
    {
        kernelDecodeCycleCount = kx * ky * z * refined_non_zero_ratio * KernelRepeatedRead / (cores * coef_decode_perf);
    }
    else if (pContext->pInPerfParams->cmdInfo.is_depth_wise_merge)
    {
        kernelDecodeCycleCount = max(kx * ky * kz * KernelRepeatedRead / coef_decode_perf, \
                                     kz * z * refined_non_zero_ratio * KernelRepeatedRead / ((cores - 1) * coef_decode_perf));
    }
    else
    {
        kernelDecodeCycleCount = refined_non_zero_ratio * kx * ky * kz * z * KernelRepeatedRead / (cores * coef_decode_perf);
    }
    // step19: kernel Decode Bandwidth
    cycleCost = &kernelDecodeCycleCost;
    cycleCost->cost = kernelDecodeCycleCount;
    cycleCost->tile0VZGroup0 = kernelDecodeCycleCount * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) * min((1.0f * k * cores / z), 1.0f);
    cycleCost->tile0ResetVZGroup = kernelDecodeCycleCount * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) - cycleCost->tile0VZGroup0;
    cycleCost->resetTileVZGroup0 = kernelDecodeCycleCount * min((1.0f * k * cores / z), 1.0f) - cycleCost->tile0VZGroup0;
    cycleCost->resetTileResetVZGroup = kernelDecodeCycleCount - cycleCost->tile0VZGroup0 - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    Cycles.KernelDecodeBW = cycleCost->cost; /* KernelDecodeBW cycle count */

    // step20: DR arb
    if ((src_buf == SW_TILING_FROM_DDR) || (src_buf == SW_TILING_FROM_AXI_SRAM))
    {
        unsigned int myZDP_LOOP_COUNT = 0;
        if (xydp_x == 0 && xydp_y == 0)
        {
            myZDP_LOOP_COUNT = 3;
        }
        else
        {
            myZDP_LOOP_COUNT = 2;
        }

        if (!slow_nn_req_arbitration_fix)
        {
            if ((zdp > 1) && (kx == 1) && (ky == 1) && pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_in <= 1)
            {
                //' this should never be the bottleneck as (2 * ZDP) means 6x64bytes for ZDP3 which takes 24 cycles to finish > 10 cycle needed
                DQArbCycleCount = imageRepeatedSingleRead * ceilf((float)kz / (myZDP_LOOP_COUNT * zdp)) * ceilf((float)x / tile_xsize) * y * (4 + 6);
            }
            else
            {
                DQArbCycleCount = imageRepeatedSingleRead * kz * ceilf((float)x / tile_xsize) * ceilf((float)y / tile_ysize) * (4 + 4);
            }
        }
        else
        {
            if ((zdp > 1) && (kx == 1) && (ky == 1) && pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_in <= 1)
            {
                DQArbCycleCount = imageRepeatedSingleRead * ceilf((float)kz / (myZDP_LOOP_COUNT * zdp)) * ceilf((float)x / tile_xsize) * y * 6;
            }
            else
            {
                DQArbCycleCount = imageRepeatedSingleRead * kz * ceilf((float)x / tile_xsize) * ceilf((float)y / tile_ysize) * 4;
            }
        }
    }
    else
    {
        DQArbCycleCount = 0;
    }
    // step20: DrArb
    cycleCost = &arbCycleCost;
    arbCycleCost.cost = DQArbCycleCount;
    arbCycleCost.tile0VZGroup0 = DQArbCycleCount * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) * min((1.0f * k * cores / z), 1.0f);
    arbCycleCost.tile0ResetVZGroup = DQArbCycleCount * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) - arbCycleCost.tile0VZGroup0;
    arbCycleCost.resetTileVZGroup0 = DQArbCycleCount * min((1.0f * k * cores / z), 1.0f) - arbCycleCost.tile0VZGroup0;
    arbCycleCost.resetTileResetVZGroup = DQArbCycleCount - arbCycleCost.tile0VZGroup0 - arbCycleCost.tile0ResetVZGroup - arbCycleCost.resetTileVZGroup0;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
    Cycles.DQArb = DQArbCycleCount; /* DQArb cycle count */

    // ZDP only architecture Cluster Crossbar bottleneck
    // Cluster Crossbar

    unsigned int in_lines_per_cycle = 3;
    in_lines_per_cycle = pContext->manualFeatureSetted ? pContext->pManualFeatures->in_lines_per_cycle : in_lines_per_cycle;
    assert(in_lines_per_cycle == 1 || in_lines_per_cycle == 2 || in_lines_per_cycle == 3);

    if (xydp_x == 0 || xydp_y == 0)
    {
        double regTile3DXBarCycleCount, bottomTile3DXBarCycleCount;
        double regTile2DXBarCycleCount = max(ceilf((float)tile_ysize * tile_xsize / lanes_per_conv), ceilf((float)tile_ysize / 8));
        double bottomTile2DXBarCycleCount = max(ceilf((1.0f * (y % tile_ysize) * tile_xsize) / lanes_per_conv), ceilf(1.0f * (y % tile_ysize) / 8));
        bool is_depth_wise_merge = pContext->pInPerfParams->cmdInfo.is_depth_wise_merge;
        float loopCount = 0;
        if(kx * ky == 1)
        {
            regTile3DXBarCycleCount = 0;
            bottomTile3DXBarCycleCount = 0;
        }
        else
        {
            bool MxN_interleave_pooling = pContext->bf.MxN_interleave_pooling;
            if (pooling_stride > 1 && MxN_interleave_pooling)
            {
                regTile3DXBarCycleCount = ceilf(tile_ysize / (float)interleave_mode);
                bottomTile3DXBarCycleCount = ceilf((y % tile_ysize) / (float)interleave_mode);
            }
            else
            {
                regTile3DXBarCycleCount = max(ceilf((tile_ysize * (float)tile_xsize) / lanes_per_conv), ceilf((float)tile_ysize / 8));  /* max segment size is 8*/
                bottomTile3DXBarCycleCount = max(ceilf(((y % tile_ysize) * (float)tile_xsize) / lanes_per_conv), ceilf((float)(y % tile_ysize) / 8));   /* max segment size is 8*/
            }

            if (is_depth_wise)
            {
                loopCount = ceilf((float)kx * ky / (zdp * ZDP_LOOP_COUNT)) * ceilf((float)zdp * ZDP_LOOP_COUNT / in_lines_per_cycle) * z;
                regTile3DXBarCycleCount = regTile2DXBarCycleCount * loopCount;

                loopCount = ceilf((float)kx * ky / (zdp * ZDP_LOOP_COUNT)) * ceilf((float)zdp * ZDP_LOOP_COUNT / in_lines_per_cycle) * z;
                bottomTile3DXBarCycleCount = bottomTile2DXBarCycleCount * loopCount;
            }
            else if (is_depth_wise_merge)
            {
                // 3 is IN_LINES_PER_CYCLE
                loopCount = ceilf((float)kx * ky / (zdp * ZDP_LOOP_COUNT)) * ceilf((float)zdp * ZDP_LOOP_COUNT / in_lines_per_cycle) * kz * ceilf((float)z / (k * (cores - 1)));
                regTile3DXBarCycleCount = regTile2DXBarCycleCount * loopCount;
                loopCount = ceilf((float)kx * ky / (zdp * ZDP_LOOP_COUNT)) * ceilf((float)zdp * ZDP_LOOP_COUNT / in_lines_per_cycle) * kz * ceilf((float)z / (k * (cores - 1)));
                bottomTile3DXBarCycleCount = bottomTile2DXBarCycleCount * loopCount;
            }
            else
            {
                loopCount = ceilf((float)kx * ky * kz / (zdp * ZDP_LOOP_COUNT)) * ceilf((float)zdp * ZDP_LOOP_COUNT / in_lines_per_cycle) * ceilf((float)z / (k * cores));
                regTile3DXBarCycleCount = regTile2DXBarCycleCount * loopCount;
                loopCount = ceilf((float)kx * ky * kz / (zdp * ZDP_LOOP_COUNT)) * ceilf((float)zdp * ZDP_LOOP_COUNT / in_lines_per_cycle) * ceilf((float)z / (k * cores));
                bottomTile3DXBarCycleCount = bottomTile2DXBarCycleCount * loopCount;
            }
        }

        XBarCycleCount = regTile3DXBarCycleCount * (unsigned int)(1.0f * y / tile_ysize) + bottomTile3DXBarCycleCount;
        XBarCycleCount = XBarCycleCount * ceilf((float)x / tile_xsize);

        if ((pContext->bf.NEIGHBOR_IMG_TRAN_NOT_EFFICIENT_BUG2045 == 1) && is_depth_wise)
        {
            float tile_vec_num = ceilf((float)tile_xsize * ((float)tile_ysize / pooling_stride) / lanes_per_conv);
            if (tile_vec_num == 1)
            {
                XBarCycleCount = XBarCycleCount * 2.43;
            }
            else
            {
                XBarCycleCount = XBarCycleCount * 1.89;
            }
        }

        Cycles.RegTile2DXBar = regTile2DXBarCycleCount; /* RegTile2DXBar cycle count */
        Cycles.BottomTile2DXbar = bottomTile2DXBarCycleCount; /* BottomTile2DXbar cycle count */
        Cycles.XBAR = XBarCycleCount; /* XBAR cycle count */
    }
    /* step21: xBar */
    cycleCost = &xBarCycleCost;
    xBarCycleCost.cost = XBarCycleCount;
    xBarCycleCost.tile0VZGroup0 = XBarCycleCount * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) * min((1.0f * k * cores / z), 1.0f);
    xBarCycleCost.tile0ResetVZGroup = XBarCycleCount * (1.0f * tile_xsize / x) * (1.0f * tile_ysize / y) - xBarCycleCost.tile0VZGroup0;
    xBarCycleCost.resetTileVZGroup0 = XBarCycleCount * min((1.0f * k * cores / z), 1.0f) - xBarCycleCost.tile0VZGroup0;
    xBarCycleCost.resetTileResetVZGroup = XBarCycleCount - xBarCycleCost.tile0VZGroup0 - xBarCycleCost.tile0ResetVZGroup - xBarCycleCost.resetTileVZGroup0;
    assert((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    // BottleNeckNNCycleCount is just for Bottle Neck determination

    // step22: calculate bottlenet
    calcBottleNeckNNCycleCount(Cycles);

#define CALC_MAX_COST(result, type) \
    { \
        result.type = max(max(max(computeCycleCost.type, kernelDecodeCycleCost.type), arbCycleCost.type), xBarCycleCost.type); \
        result.type = max(max(max(nnCost[APM_DDR_KERNEL_COST].readCycle.type, nnCost[APM_DDR_IN_IMAGE_COST].readCycle.type), nnCost[APM_DDR_COST].readCycle.type), nnCycleCost.type); \
        result.type = max(max(max(max(nnCycleCost.type, nnCost[APM_DDR_COST].readCycle.type), nnCost[APM_AXI_SRAM_COST].readCycle.type), nnCost[APM_AXI_BUS_COST].readCycle.type), nnCost[APM_VIP_SRAM_COST].readCycle.type); \
        result.type = max(max(max(max(nnCycleCost.type, nnCost[APM_DDR_COST].writeCycle.type), nnCost[APM_AXI_SRAM_COST].writeCycle.type), nnCost[APM_AXI_BUS_COST].writeCycle.type), nnCost[APM_VIP_SRAM_COST].writeCycle.type); \
        result.type = max(max(max(nnCycleCost.type, ddrTotalCycleCost.type), axiTotalCycleCost.type), axiBusTotalCycleCost.type); \
        result.type = max(max(max(max(nnCycleCost.type, internalWriteCycleCost.type), InternalKernelReadCycleCount.type),DDRReadCycleCount_Combined_Bursts.type),DDRWriteCycleCount_Combined_Bursts.type); \
    }
    CALC_MAX_COST(nnCycleCost, tile0VZGroup0);
    CALC_MAX_COST(nnCycleCost, tile0ResetVZGroup);
    CALC_MAX_COST(nnCycleCost, resetTileVZGroup0);
    CALC_MAX_COST(nnCycleCost, resetTileResetVZGroup);
    nnCycleCost.cost = nnCycleCost.tile0VZGroup0 + nnCycleCost.tile0ResetVZGroup + nnCycleCost.resetTileVZGroup0 + nnCycleCost.resetTileResetVZGroup;
    // tile0 vz group0
    Cycles_Create(NNCycleCounts_tile0_vzgroup0,
                  computeCycleCost.tile0VZGroup0, nnCost[APM_DDR_COST].readCycle.tile0VZGroup0, nnCost[APM_DDR_COST].writeCycle.tile0VZGroup0, ddrTotalCycleCost.tile0VZGroup0,
                  nnCost[APM_AXI_SRAM_COST].readCycle.tile0VZGroup0, nnCost[APM_AXI_SRAM_COST].writeCycle.tile0VZGroup0, axiTotalCycleCost.tile0VZGroup0,
                  nnCost[APM_AXI_BUS_COST].readCycle.tile0VZGroup0, nnCost[APM_AXI_BUS_COST].writeCycle.tile0VZGroup0, axiBusTotalCycleCost.tile0VZGroup0,
                  nnCost[APM_VIP_SRAM_COST].readCycle.tile0VZGroup0, nnCost[APM_VIP_SRAM_COST].writeCycle.tile0VZGroup0,  Cycles.Slow_InternalWrite, Cycles.Slow_Comp, internalWriteCycleCost.tile0VZGroup0,
                  InternalKernelReadCycleCount.tile0VZGroup0, nnCost[APM_DDR_KERNEL_COST].readCycle.tile0VZGroup0, nnCost[APM_DDR_IN_IMAGE_COST].readCycle.tile0VZGroup0,
                  kernelDecodeCycleCost.tile0VZGroup0, DDRReadCycleCount_Combined_Bursts.tile0VZGroup0, DDRWriteCycleCount_Combined_Bursts.tile0VZGroup0,
                  /*DWOutCycleCount_tile0_vzgroup0*/ 0, arbCycleCost.tile0VZGroup0, 0, 0, xBarCycleCost.tile0VZGroup0, 0, RdReturnArbiterCycleCount.tile0VZGroup0);
    outputs->tile0_vzgroup0 = NNCycleCounts_tile0_vzgroup0;
    outResult->BN_Tile0Vzgroup0_e = NNCycleCounts_tile0_vzgroup0.BottleNeck_e;
    // tile0 reset vz group0
    CycleCounts NNCycleCounts_tile0_rest_vzgroup; memset(&NNCycleCounts_tile0_rest_vzgroup, 0, sizeof(CycleCounts));
    Cycles_Create(NNCycleCounts_tile0_rest_vzgroup,
                  computeCycleCost.tile0ResetVZGroup, nnCost[APM_DDR_COST].readCycle.tile0ResetVZGroup, nnCost[APM_DDR_COST].writeCycle.tile0ResetVZGroup, ddrTotalCycleCost.tile0ResetVZGroup,
                  nnCost[APM_AXI_SRAM_COST].readCycle.tile0ResetVZGroup, nnCost[APM_AXI_SRAM_COST].writeCycle.tile0ResetVZGroup, axiTotalCycleCost.tile0ResetVZGroup,
                  nnCost[APM_AXI_BUS_COST].readCycle.tile0ResetVZGroup, nnCost[APM_AXI_BUS_COST].writeCycle.tile0ResetVZGroup, axiBusTotalCycleCost.tile0ResetVZGroup,
                  nnCost[APM_VIP_SRAM_COST].readCycle.tile0ResetVZGroup, nnCost[APM_VIP_SRAM_COST].writeCycle.tile0ResetVZGroup,  Cycles.Slow_InternalWrite, Cycles.Slow_Comp, internalWriteCycleCost.tile0ResetVZGroup,
                  InternalKernelReadCycleCount.tile0ResetVZGroup, nnCost[APM_DDR_KERNEL_COST].readCycle.tile0ResetVZGroup, nnCost[APM_DDR_IN_IMAGE_COST].readCycle.tile0ResetVZGroup,
                  kernelDecodeCycleCost.tile0ResetVZGroup, DDRReadCycleCount_Combined_Bursts.tile0ResetVZGroup, DDRWriteCycleCount_Combined_Bursts.tile0ResetVZGroup,
                  /*DWOutCycleCount_tile0_vzgroup0*/ 0, arbCycleCost.tile0ResetVZGroup, 0, 0, xBarCycleCost.tile0ResetVZGroup, 0, RdReturnArbiterCycleCount.tile0ResetVZGroup);
    outputs->tile0_rest_vzgroup = NNCycleCounts_tile0_rest_vzgroup;
    outResult->BN_Tile0RestVzgroup0_e = NNCycleCounts_tile0_rest_vzgroup.BottleNeck_e;

    // reset tile vzgroup0
    CycleCounts NNCycleCounts_rest_tile_vzgroup0; memset(&NNCycleCounts_rest_tile_vzgroup0, 0, sizeof(CycleCounts));
    Cycles_Create(NNCycleCounts_rest_tile_vzgroup0,
                  computeCycleCost.resetTileVZGroup0, nnCost[APM_DDR_COST].readCycle.resetTileVZGroup0, nnCost[APM_DDR_COST].writeCycle.resetTileVZGroup0, ddrTotalCycleCost.resetTileVZGroup0,
                  nnCost[APM_AXI_SRAM_COST].readCycle.resetTileVZGroup0, nnCost[APM_AXI_SRAM_COST].writeCycle.resetTileVZGroup0, axiTotalCycleCost.resetTileVZGroup0,
                  nnCost[APM_AXI_BUS_COST].readCycle.resetTileVZGroup0, nnCost[APM_AXI_BUS_COST].writeCycle.resetTileVZGroup0, axiBusTotalCycleCost.resetTileVZGroup0,
                  nnCost[APM_VIP_SRAM_COST].readCycle.resetTileVZGroup0, nnCost[APM_VIP_SRAM_COST].writeCycle.resetTileVZGroup0,  Cycles.Slow_InternalWrite, Cycles.Slow_Comp, internalWriteCycleCost.resetTileVZGroup0,
                  InternalKernelReadCycleCount.resetTileVZGroup0, nnCost[APM_DDR_KERNEL_COST].readCycle.resetTileVZGroup0, nnCost[APM_DDR_IN_IMAGE_COST].readCycle.resetTileVZGroup0,
                  kernelDecodeCycleCost.resetTileVZGroup0, DDRReadCycleCount_Combined_Bursts.resetTileVZGroup0, DDRWriteCycleCount_Combined_Bursts.resetTileVZGroup0,
                  /*DWOutCycleCount_tile0_vzgroup0*/ 0, arbCycleCost.resetTileVZGroup0, 0, 0, xBarCycleCost.resetTileVZGroup0, 0, RdReturnArbiterCycleCount.resetTileVZGroup0);
    outputs->rest_tile_vzgroup0 = NNCycleCounts_rest_tile_vzgroup0;
    outResult->BN_RestTileVzgroup0_e = NNCycleCounts_rest_tile_vzgroup0.BottleNeck_e;

    // reset tile0 reset vzgroup0
    CycleCounts NNCycleCounts_rest_tile_rest_vzgroup; memset(&NNCycleCounts_rest_tile_rest_vzgroup, 0, sizeof(CycleCounts));
    Cycles_Create(NNCycleCounts_rest_tile_rest_vzgroup,
                  computeCycleCost.resetTileResetVZGroup, nnCost[APM_DDR_COST].readCycle.resetTileResetVZGroup, nnCost[APM_DDR_COST].writeCycle.resetTileResetVZGroup, ddrTotalCycleCost.resetTileResetVZGroup,
                  nnCost[APM_AXI_SRAM_COST].readCycle.resetTileResetVZGroup, nnCost[APM_AXI_SRAM_COST].writeCycle.resetTileResetVZGroup, axiTotalCycleCost.resetTileResetVZGroup,
                  nnCost[APM_AXI_BUS_COST].readCycle.resetTileResetVZGroup, nnCost[APM_AXI_BUS_COST].writeCycle.resetTileResetVZGroup, axiBusTotalCycleCost.resetTileResetVZGroup,
                  nnCost[APM_VIP_SRAM_COST].readCycle.resetTileResetVZGroup, nnCost[APM_VIP_SRAM_COST].writeCycle.resetTileResetVZGroup,  Cycles.Slow_InternalWrite, Cycles.Slow_Comp, internalWriteCycleCost.resetTileResetVZGroup,
                  InternalKernelReadCycleCount.resetTileResetVZGroup, nnCost[APM_DDR_KERNEL_COST].readCycle.resetTileResetVZGroup, nnCost[APM_DDR_IN_IMAGE_COST].readCycle.resetTileResetVZGroup,
                  kernelDecodeCycleCost.resetTileResetVZGroup, DDRReadCycleCount_Combined_Bursts.resetTileResetVZGroup, DDRWriteCycleCount_Combined_Bursts.resetTileResetVZGroup,
                  /*DWOutCycleCount_tile0_vzgroup0*/ 0, arbCycleCost.resetTileResetVZGroup, 0, 0, xBarCycleCost.resetTileResetVZGroup, 0, RdReturnArbiterCycleCount.resetTileResetVZGroup);
    outputs->rest_tile_rest_vzgroup = NNCycleCounts_rest_tile_rest_vzgroup;
    outResult->BN_RestTileRestVzgroup0_e = NNCycleCounts_rest_tile_rest_vzgroup. BottleNeck_e;


    //NNCycleCounts_tile0_vzgroup0.Compute  = computeCycleCost.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.DDRRead  = nnCost[APM_DDR_COST].readCycle.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.DDRWrite = nnCost[APM_DDR_COST].writeCycle.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.DDRTotal = ddrTotalCycleCost.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.AXISRAMRead  = nnCost[APM_AXI_SRAM_COST].readCycle.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.AXISRAMWrite = nnCost[APM_AXI_SRAM_COST].writeCycle.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.AXISRAMTotal = axiTotalCycleCost.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.AXIBUSRead   = nnCost[APM_AXI_BUS_COST].readCycle.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.AXIBUSWrite  = nnCost[APM_AXI_BUS_COST].writeCycle.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.AXIBUSTotal  = axiBusTotalCycleCost.tile0VZGroup0;;
    //NNCycleCounts_tile0_vzgroup0.VIPSRAMRead  = nnCost[APM_VIP_SRAM_COST].readCycle.tile0VZGroup0;
    //NNCycleCounts_tile0_vzgroup0.VIPSRAMWrite = nnCost[APM_VIP_SRAM_COST].writeCycle.tile0VZGroup0;
    //NNCycleCounts_tile0_vzgroup0.Slow_InternalWrite = Cycles.Slow_InternalWrite;
    //NNCycleCounts_tile0_vzgroup0.Slow_Comp        = Cycles.Slow_Comp;
    //NNCycleCounts_tile0_vzgroup0.InternalWrite    = Cycles.InternalWrite;
    //NNCycleCounts_tile0_vzgroup0.InternalKernelRead = InternalKernelReadCycleCount.tile0VZGroup0;
    //NNCycleCounts_tile0_vzgroup0.KernelDDRRead    = nnCost[APM_DDR_KERNEL_COST].readCycle.tile0VZGroup0;
    //NNCycleCounts_tile0_vzgroup0.InImageDDRRead   = nnCost[APM_DDR_IN_IMAGE_COST].readCycle.tile0VZGroup0;
    //NNCycleCounts_tile0_vzgroup0.KernelDecodeBW   = kernelDecodeCycleCost.tile0VZGroup0;
    //NNCycleCounts_tile0_vzgroup0.DWOut            = 0; // depthwise merge, do not enable yet
    //NNCycleCounts_tile0_vzgroup0.DQArb            = arbCycleCost.tile0VZGroup0;
    //NNCycleCounts_tile0_vzgroup0.RegTile2DXBar    = 0; // fix me
    //NNCycleCounts_tile0_vzgroup0.BottomTile2DXbar = 0;
    //NNCycleCounts_tile0_vzgroup0.XBAR             = 0;
    ////NNCycleCounts_tile0_vzgroup0.CacheController = ;
    ////NNCycleCounts_tile0_vzgroup0.RdReturnArbiter;
    ////NNCycleCounts_tile0_vzgroup0.Overheads;
    ////NNCycleCounts_tile0_vzgroup0.Overall;
    ////NNCycleCounts_tile0_vzgroup0.BottleNeck[64];

    NNCycleCounts_tile0_vzgroup0.Compute = computeCycleCost.tile0VZGroup0;
    NNCycleCounts_tile0_vzgroup0.KernelDecodeBW = kernelDecodeCycleCost.tile0VZGroup0;

    // should refine
    outBandWidth->is_compute_bottle_neck = (nnCycleCost.cost == computeCycleCost.cost) ? true : false;

    if (small_batch_en == 0 || flush_and_wait)
    {
        if (!((xydp_x == 0) && (xydp_y == 0)))
        {
            nnCycleCost.cost += min(ceilf((float)tile_xsize / 8) * interleave_mode * conv_out_fifo_depth,
                                (z % (cores * k)) * (ceilf((float)tile_xsize / 8) - 1) * (y % tile_ysize)); /*Emptying Conv Out FIFO*/

            nnCycleCost.cost += (1791 - 1407) + (cores - 1) * (560 - 557) + 1407 + ddr_latency;
        }
        else
        {
            unsigned int lastTileXsize = (x % tile_xsize) == 0 ? tile_xsize : (x % tile_xsize);
            unsigned int lastTileYsize = (y % tile_ysize) == 0 ? tile_ysize : (y % tile_ysize);


            unsigned int AdjCore = (pContext->pInPerfParams->cmdInfo.is_depth_wise_merge)? cores - 1: cores;
            double OverheadsCycleCount;
            if (kx * ky == 1)
            {
                OverheadsCycleCount = (z % (AdjCore * k)) * ceilf(ceilf((float)lastTileXsize * interleave_mode / 64) * (64 / interleave_mode) * lastTileYsize / LANES_PER_OUT_CYCLE); // ' Emptying Accum Buffer
            }
            else
            {
                OverheadsCycleCount = (z % (AdjCore * k)) * ceilf((float)lastTileXsize * lastTileYsize / LANES_PER_OUT_CYCLE);
            }

            bool PREFETCH_NN_COMMAND_KERNEL_HEADER = pContext->bf.PREFETCH_NN_COMMAND_KERNEL_HEADER;
            PREFETCH_NN_COMMAND_KERNEL_HEADER = pContext->manualFeatureSetted ? pContext->pManualFeatures->prefetchNNComandKernelHeader : PREFETCH_NN_COMMAND_KERNEL_HEADER;
            if (PREFETCH_NN_COMMAND_KERNEL_HEADER == 0) // small batch phase 1
            {
                OverheadsCycleCount = OverheadsCycleCount + (81 + ddr_latency) * 2; // NN Command and Kernel Header read
            }

            double ImageToCoreCycleCount = 0;
            // Image read
            if (src_buf == SW_TILING_FROM_DDR)
            {
                ImageToCoreCycleCount = 81 + ddr_latency;
            }
            else if (src_buf == SW_TILING_FROM_AXI_SRAM)
            {
                ImageToCoreCycleCount = 81 + min(20, ddr_latency); // assume 20 cycle AXI-SRAM latency
            }
            else
            {
                ImageToCoreCycleCount = 20; // assume 20 cycle VIP-SRAM latency
            }

            // JD to image buffer
            ImageToCoreCycleCount = ImageToCoreCycleCount + 24;

            // preproces
            if (kx == 1 && ky == 1)
            {
                ImageToCoreCycleCount = ImageToCoreCycleCount + 2;
            }
            else
            {
                ImageToCoreCycleCount = ImageToCoreCycleCount + 12 + 5;
            }

            // Kernel read
            double KernelToCoreCycleCount = 0;
            if (kernel_from_ddr)
            {
                KernelToCoreCycleCount = 81 + ddr_latency;
            }
            else if (kernel_from_axi_sram)
            {
                KernelToCoreCycleCount = 81 + min(20, ddr_latency); //assume 20 cycle AXI-SRAM latency
            }
            else
            {   // from VIP-SRAM
                KernelToCoreCycleCount = 20; //
            }

            //Kernel Decode + Zero Skip Sequencer
            KernelToCoreCycleCount = KernelToCoreCycleCount + 20; // assume 20 cycles
            OverheadsCycleCount = OverheadsCycleCount + max(ImageToCoreCycleCount, KernelToCoreCycleCount);

            //' Write out including Post Process
            if (dst_buf == SW_TILING_FROM_DDR)
            {
                OverheadsCycleCount = OverheadsCycleCount + 57 + ddr_latency;
            }
            else if (dst_buf == SW_TILING_FROM_AXI_SRAM)
            {
                OverheadsCycleCount = OverheadsCycleCount + 57 + min(20, ddr_latency); // ' assume 20 cycle AXI-SRAM latency
            }
            else
            {
                OverheadsCycleCount = OverheadsCycleCount + 45;
            }

            nnCycleCost.cost += OverheadsCycleCount;

            Cycles.Overheads = Cycles.Overheads + OverheadsCycleCount;

        }
    }

    /*Each 3D Tile needs to wait if CONV_OUT_FIFO is not deep enough*/
    float tileRow = min(z, cores * k) * (float)tile_ysize / interleave_mode;
    float tileRowSlow = tileRow - conv_out_fifo_depth * (1 + (float)8 / lanes_per_conv);
    float tileOverhead = 0;
    imageRepeatedSingleRead = Tile3DImageSingleReadRepeated(z, k, cores);
    if (tileRowSlow > 0)
    {
        tileOverhead = tileRowSlow * ceilf((float)tile_xsize / 8) * interleave_mode;
    }

    if (!per_3d_tile_bubble_fix && imageRepeatedSingleRead != 1)
    {
        tileOverhead += ddr_latency + 150 + (cores - 1) * ceilf((float)tile_ysize / interleave_mode) * k;
    }

    double OverheadsCycleCount = tileOverhead * ceilf((float)y / tile_ysize) * ceilf((float)x / tile_xsize);
    nnCycleCost.cost += OverheadsCycleCount;

    Cycles.Overheads = Cycles.Overheads + OverheadsCycleCount;
    Cycles.Overall = nnCycleCost.cost;

    outputBandwidthInfo(outBandWidth, nnCost);

    outputCycleInfo(outResult, Cycles, nnCycleCost);

    return nnCycleCost.cost;
}


static double _calcTPComputeCycleCount(
    unsigned int type,
    unsigned int x,
    unsigned int y,
    unsigned int z,
    unsigned int kz,
    double coef_nonzero_ratio,
    unsigned int cores,
    double image_nonzero_ratio
    )
{
    if (type == VXNNE_OPERATOR_NORMALIZATION || type == VXNNE_OPERATOR_ACTIVATION)
    {
        return (double)x * y * z / cores + 512;
    }
    else if (type == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        return (double)x * y * z * kz * coef_nonzero_ratio * image_nonzero_ratio / cores;
    }
    else
    {
        return (double)x * y * z / cores;
    }
}

static double TPCycleCountCore(
    unsigned int type, unsigned int x, unsigned int y, unsigned int z, unsigned int kz,
    double coef_nonzero_ratio, double coef_compress_ratio,
    double image_nonzero_ratio, unsigned int cores, unsigned int tp_cmd_size,
    unsigned int pooling_stride,
    float axi_sram_read_bw_limit, float axi_sram_write_bw_limit, float axi_sram_total_bw_limit,
    float axi_bus_read_bw_limit, float axi_bus_write_bw_limit, float axi_bus_total_bw_limit,
    unsigned int l2cache_width,
    unsigned int data_size,
    /*unsigned int sw_tiling,*/
    unsigned int src_buf, unsigned int dst_buf, unsigned int kernel_buf,
    unsigned int outstanding_transfer,
    float ddr_latency,
    float total_latency,
    float ddr_read_bw_in_byte_per_cycle,
    float ddr_write_bw_in_byte_per_cycle,
    float ddr_total_bw_in_byte_per_cycle,
    unsigned int usc_cache_controllers,
    unsigned int tp_reorder_fix,
    unsigned int flush,
    unsigned int small_batch_enable,
    unsigned int axi_sram_slowed_down_by_ddr,
    unsigned int specified_ddr_bw_limit_by_burst,
    double *ddr_kernel_read_bw, double *ddr_in_image_read_bw,
    double *ddr_read_bw, double *ddr_write_bw,
    double *axi_sram_read_bw, double *axi_sram_write_bw,
    APM_OUT_RESULT_T *outResult
    )
{
    double ddrReadBandWidth       = (double)tp_cmd_size;
    double ddrWriteBandWidth      = 0;
    double ddrTotalBandWidth      = 0;
    //double ddrReadCycleCount      = 0;
    //double DDRWriteCycleCount     = 0;
    double DDRTotalCycleCount     = 0;
    double axiSRAMReadBandWidth   = 0;
    double axiSRAMWriteBandWidth  = 0;
    //double axiSRAMReadCycleCount  = 0;
    //double axiSRAMWriteCycleCount = 0;
    double vipSRAMReadBandWidth   = 0;
    double vipSRAMWriteBandWidth  = 0;
    //double vipSRAMReadCycleCount  = 0;
    //double vipSRAMWriteCycleCount = 0;
    double axiBusReadBandWidth    = 0;
    double axiBusWriteBandWidth   = 0;
    //double axiBusReadCycleCount   = 0;
    //double axiBusWriteCycleCount  = 0;
    //double axiBusTotalCycleCount  = 0;
    double readBW, writeBW;
    double CompCycleCount = 0, tpCycleCountCore = 0;
    double adjustedAXISraReadBandWidthLimit = (double)axi_sram_read_bw_limit;
    double ddrKernelReadBW = 0, ddrInImageReadBW = 0;
    unsigned int inz = z;

    double DDRReadBandWidth_64B = TP_COMMAND_SIZE;
    double DDRWriteBandWidth_NonMask_64B = 0;
    double DDRWriteBandWidth_Mask_64B = 0;
    double TP_MISS_RATIO = 1;
    double DDRReadCycleCount_Combined_Bursts = 0,DDRWriteCycleCount_Combined_Bursts = 0;
    bool   TP_VIPSRAM_OT1_BUG2050 = pContext->bf.TP_VIPSRAM_OT1_BUG2050;
    bool   VIPSRAM_ASYNC_FIFO = pContext->bf.VIPSRAM_ASYNC_FIFO;
    double vipsram_width_avail = l2cache_width;

    // refine me, can be parameters of this function?
    CycleCounts Cycles;
    memset(&Cycles, 0, sizeof(CycleCounts));

    if (TP_VIPSRAM_OT1_BUG2050)
    {
        vipsram_width_avail /= (VIPSRAM_ASYNC_FIFO) ? 13 : 5;
    }

    // data size is bit size
    assert (data_size == 8 || data_size == 16 || data_size == 32);
    if (data_size == 32)
    {
        printf("CArch warning: data_size is 32 bits, should support FP32!\n");
    }
    if (type == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        ddrKernelReadBW = x * y * z * kz * coef_compress_ratio * image_nonzero_ratio * data_size / 8 * TP_MISS_RATIO; /*' always store kernel in DDR*/
        ddrReadBandWidth = (ddrReadBandWidth + ddrKernelReadBW) * TP_MISS_RATIO;
        inz = kz;
    }

    readBW = (double)(x * y * inz * data_size / 8 * TP_MISS_RATIO);
    writeBW = (double)(x * y * z / (pooling_stride * pooling_stride) * TP_MISS_RATIO);

    if (src_buf == SW_TILING_FROM_DDR)
    {
        ddrReadBandWidth = ddrReadBandWidth + readBW;
        ddrInImageReadBW = readBW;
        DDRReadBandWidth_64B = ddrReadBandWidth;
        outResult->outputs.DDRReadBW_64B.cost = DDRReadBandWidth_64B;
    }
    else if (src_buf == SW_TILING_FROM_AXI_SRAM)
    {
        axiSRAMReadBandWidth = axiSRAMReadBandWidth + readBW;
        if ((type == VXNNE_OPERATOR_FULLYCONNECTED) && (kernel_buf == SW_TILING_FROM_DDR))
        {
            if (axi_sram_slowed_down_by_ddr)
            {
                double maxOutstandingCycle = (double)outstanding_transfer * 4;
                if (total_latency > maxOutstandingCycle)
                {
                    double bwLimitedByLatency = (16 * maxOutstandingCycle) / total_latency;
                    adjustedAXISraReadBandWidthLimit = min(adjustedAXISraReadBandWidthLimit, bwLimitedByLatency);
                }
            }
        }
    }
    else if (src_buf == SW_TILING_FROM_VIP_SRAM)
    {
        vipSRAMReadBandWidth = vipSRAMReadBandWidth + readBW;
    }

    if (dst_buf == SW_TILING_FROM_DDR)
    {
        ddrWriteBandWidth = writeBW;
        DDRWriteBandWidth_NonMask_64B = (2 - TP_MISS_RATIO) * ddrWriteBandWidth;
        DDRWriteBandWidth_Mask_64B = (TP_MISS_RATIO - 1) * ddrWriteBandWidth;
        outResult->outputs.DDRMaskWriteBW_64B.cost = DDRWriteBandWidth_Mask_64B;
        outResult->outputs.DDRNonMaskWriteBW_64B.cost = DDRWriteBandWidth_NonMask_64B;
    }
    else if (dst_buf == SW_TILING_FROM_AXI_SRAM)
    {
        axiSRAMWriteBandWidth = writeBW;
    }
    else if (dst_buf == SW_TILING_FROM_VIP_SRAM)
    {
        vipSRAMWriteBandWidth = writeBW;
    }

    axiBusReadBandWidth    = ddrReadBandWidth + axiSRAMReadBandWidth;
    axiBusWriteBandWidth   = ddrWriteBandWidth + axiSRAMWriteBandWidth;
    ddrTotalBandWidth      = ddrReadBandWidth + ddrWriteBandWidth;

    double DDRReadCycleCount      = ddrReadBandWidth / ddr_read_bw_in_byte_per_cycle;
    double DDRWriteCycleCount     = ddrWriteBandWidth / ddr_write_bw_in_byte_per_cycle;
    DDRTotalCycleCount     = ddrTotalBandWidth / ddr_total_bw_in_byte_per_cycle;

    Cycles.DDRRead  = DDRReadCycleCount;
    Cycles.DDRWrite = DDRWriteCycleCount;
    Cycles.DDRTotal = DDRTotalCycleCount;

    double AXISRAMReadCycleCount  = axiSRAMReadBandWidth / adjustedAXISraReadBandWidthLimit;
    double AXISRAMWriteCycleCount = axiSRAMWriteBandWidth / axi_sram_write_bw_limit;
    double AXISRAMTotalCycleCount = (axiSRAMReadBandWidth + axiSRAMWriteBandWidth) / axi_sram_total_bw_limit;

    Cycles.AXISRAMRead = AXISRAMReadCycleCount;
    Cycles.AXISRAMWrite = AXISRAMWriteCycleCount;
    Cycles.AXISRAMTotal = AXISRAMTotalCycleCount;

    double AXIBusReadCycleCount   = axiBusReadBandWidth / axi_bus_read_bw_limit;
    double AXIBusWriteCycleCount  = axiBusWriteBandWidth / axi_bus_write_bw_limit;
    double AXIBusTotalCycleCount  = (axiBusReadBandWidth + axiSRAMWriteBandWidth) / axi_bus_total_bw_limit;

    Cycles.AXIBUSRead  = AXIBusReadCycleCount;
    Cycles.AXIBUSWrite = AXIBusWriteCycleCount;
    Cycles.AXIBUSTotal = AXIBusTotalCycleCount;

    double VIPSRAMReadCycleCount  = vipSRAMReadBandWidth / vipsram_width_avail;
    double VIPSRAMWriteCycleCount = vipSRAMWriteBandWidth / min(16, l2cache_width);

    Cycles.VIPSRAMRead = VIPSRAMReadCycleCount;
    Cycles.VIPSRAMWrite = VIPSRAMWriteCycleCount;

    double CacheControllerCycleCount = readBW / (4 * usc_cache_controllers); /* assume we do 4 byte per cache command */
    if (tp_reorder_fix ||
        ((type != VXNNE_OPERATOR_TENSOR_TRANS) && (type != VXNNE_OPERATOR_ROIPOOL)))
    {
        CacheControllerCycleCount += writeBW / (4 * usc_cache_controllers); /* assume we do 4 byte per cache command */
    }
    else
    {
        CacheControllerCycleCount += writeBW / usc_cache_controllers; /* assume we do 1 byte per cache command */
    }
    Cycles.CacheController = CacheControllerCycleCount;

    CompCycleCount = _calcTPComputeCycleCount(type, x, y, z, kz, coef_nonzero_ratio, cores, image_nonzero_ratio);
    Cycles.Compute = CompCycleCount;

    if (specified_ddr_bw_limit_by_burst)
    {
        DDRReadCycleCount_Combined_Bursts = DDRReadBandWidth_64B / DDR_READ_BW_IN_BYTE_PER_CYCLE_64B;
        DDRWriteCycleCount_Combined_Bursts = DDRWriteBandWidth_Mask_64B / DDR_MASKWRITE_BW_IN_BYTE_PER_CYCLE_64B + DDRWriteBandWidth_NonMask_64B / DDR_NONMASKWRITE_BW_IN_BYTE_PER_CYCLE_64B;
        Cycles.DDRRead_Combined_Bursts = DDRReadCycleCount_Combined_Bursts;
        Cycles.DDRWrite_Combined_Bursts = DDRWriteCycleCount_Combined_Bursts;
    }

    calcBottleNeckTPCycleCount(Cycles);
    tpCycleCountCore = Cycles.Overall;

    if (tp_reorder_fix)
    {
        if (type == VXNNE_OPERATOR_TENSOR_TRANS || type == VXNNE_OPERATOR_ROIPOOL)
        {
            tpCycleCountCore += 256; /*half of 512 in average*/
        }
    }

    if (flush || !small_batch_enable)/*small_batch_en = 0 || (flush_and_wait == 1) */
    {
        tpCycleCountCore += 1000 + 1407 + ddr_latency;
    }

    *ddr_kernel_read_bw = ddrKernelReadBW;
    *ddr_in_image_read_bw = ddrInImageReadBW;
    *ddr_read_bw = ddrReadBandWidth;
    *ddr_write_bw = ddrWriteBandWidth;
    *axi_sram_read_bw = axiSRAMReadBandWidth;
    *axi_sram_write_bw = axiSRAMWriteBandWidth;

    Cycles.Overall = tpCycleCountCore;
    //Cycles.Overheads = 0; // TP should calculate, to match VB, does not impliment yet. Refine me
    //Cycles.BottleNeck = BottleNeck; // calculated in Overheads

    APM_BW_CYCLE_COST_T tpCycleCost = {0}; // just pass a parameter
    outputCycleInfo(outResult, Cycles, tpCycleCost);

    return tpCycleCountCore;
}

void APMSetManualParams(
    IN APM_MANUAL_PARAMS_T manualParams
    )
{
    pContext->pManualParams = (APM_MANUAL_PARAMS_T*)malloc(sizeof(APM_MANUAL_PARAMS_T));
    memcpy(pContext->pManualParams, &manualParams, sizeof(APM_MANUAL_PARAMS_T));
}

/*
  Manually set features
 */
void APMSetManualFeatures(
    IN APM_MANUAL_FEATURE_T manualFeatures
    )
{
    pContext->pManualFeatures = (APM_MANUAL_FEATURE_T *) malloc(sizeof(APM_MANUAL_FEATURE_T));
    memcpy(pContext->pManualFeatures, &manualFeatures, sizeof(APM_MANUAL_FEATURE_T));
    pContext->manualFeatureSetted = true;
}

void UpdateManualParams(
    unsigned int &tile_xsize,
    unsigned int &tile_ysize,
    unsigned int &k,
    unsigned int &inx,
    unsigned int &iny
    )
{
    if(pContext->pManualParams != NULL)
    {
        tile_xsize = pContext->pManualParams->outTileXsize;
        tile_ysize = pContext->pManualParams->outTileYsize;
        k          = pContext->pManualParams->k;
        inx        = pContext->pManualParams->inX;
        iny        = pContext->pManualParams->inY;

        free(pContext->pManualParams);
        pContext->pManualParams = NULL;
    }
}

void UpdateManualFeaturesNN(
    unsigned int & small_batch_enable,
    unsigned int & lans_per_conv
    )
{
    if (pContext->manualFeatureSetted)
    {
        small_batch_enable = pContext->pManualFeatures->small_bach_enable;
        lans_per_conv = pContext->pManualFeatures->lanes_per_conv;
        pContext->bf.new_feature = pContext->pManualFeatures->new_feature; //Shuangbei: initialized value will be modified here if manual feature is ON..
    }
}

void UpdateManualFeaturesTP(
    unsigned int & small_batch_enable
)
{
    if (pContext->manualFeatureSetted)
    {
        small_batch_enable = pContext->pManualFeatures->small_bach_enable;
    }

}

// Arch Perf Model get calculate NN cycle count.
// refine the interface, we have three output
double APMCalcNNCycleCountBandWidth (
    IN  APMHandle          apmHandle,
    IN  APM_IN_PERF_PARAMS inPerfParams,
    OUT APM_OUT_BW_T       *outBandWidth,
    OUT APM_OUT_RESULT_T   *outResult /* more info beside total bandwidth*/
    )
{
    APM_HW_INFO_T *pHwInfo = (APM_HW_INFO_T *)apmHandle;
    pContext->pInPerfParams = &inPerfParams;

    unsigned int  tile_xsize = inPerfParams.cmdInfo.u.nncmd.tile_xsize;
    unsigned int  tile_ysize = inPerfParams.cmdInfo.u.nncmd.tile_ysize;
    unsigned int  kernel_per_core = inPerfParams.cmdInfo.u.nncmd.kernel_per_core;

    unsigned int  x   = inPerfParams.cmdInfo.outImageXsize;   // inImageXsize
    unsigned int  y   = inPerfParams.cmdInfo.outImageYsize;   // inImageYsize
    unsigned int  z   = inPerfParams.cmdInfo.outImageZsize;   // outImageZsize
    unsigned int  kx  = inPerfParams.cmdInfo.kernelXsize;
    unsigned int  ky  = inPerfParams.cmdInfo.kernelYsize;
    unsigned int  kz  = inPerfParams.cmdInfo.kernelZsize;
    unsigned int  inx = inPerfParams.cmdInfo.inSIXRefined; // subImageXsize
    unsigned int  iny = inPerfParams.cmdInfo.inSIYRefined; // subImageYsize

    pContext->pooling_stride    = inPerfParams.cmdInfo.pooling_stride;
    double non_zero_ratio       = inPerfParams.compInfo.coefNonZeroRatio;
    double coef_compress_ratio  = inPerfParams.compInfo.coefCompression;
    double image_compress_ratio = inPerfParams.compInfo.imageCompression;
    // cores better to set by interface, no need to calculate. todo
    unsigned int  cores            = pHwInfo->pFeatures->NNCoreCount;
    unsigned int  brick_mode       = inPerfParams.cmdInfo.brick_mode;
    unsigned int  input_data_size  = inPerfParams.inDataBitSize;
    unsigned int  output_data_size = inPerfParams.outDataBitSize;

    // fix me, VIP_SRAM_SIZE use KB or B
    unsigned int  l2cache_size = pHwInfo->pFeatures->VIP_SRAM_SIZE;
    l2cache_size = inPerfParams.l2cache_size;
    unsigned int  l2cache_width = pHwInfo->pFeatures->EQUIVALENT_VIP_SRAM_WIDTH_INBYTE;
    unsigned int  xydp_x = inPerfParams.xydp_x;
    unsigned int  xydp_y = inPerfParams.xydp_y;
    g_xydp_x = xydp_x;
    g_xydp_y = xydp_y;
    // ZDP6 always 0, until now
    unsigned int  zdp = (pHwInfo->pFeatures->NN_ZDP3)? 3:1;
    unsigned int  float_xydp_x = pHwInfo->pFeatures->NNFP16_XYDP_X;
    unsigned int  float_xydp_y = pHwInfo->pFeatures->NNFP16_XYDP_Y;
    unsigned int  float_zdp    = pHwInfo->pFeatures->NNFP16_ZDP;
    unsigned int  usc_cache_size = USC_CACHE_SIZE;
    unsigned int  nn_cmd_size    = NNE_COMMAND_SIZE; // size in Byte
    unsigned int  coef_decode_perf = pHwInfo->pFeatures->NN_COEF_DECOMPRESS_PERF2X ? 2: 1;
    unsigned int  vector_prune = inPerfParams.vector_prune;
    unsigned int  image_partial_cache = pHwInfo->pFeatures->IMAGE_PARTIAL_CACHE;
    assert( image_partial_cache == 0 );
    unsigned int  data_read_from_sram = CACHED_DATA_READ_FROM_SRAM;
    unsigned int  first_cmd  = pContext->pInPerfParams->first_cmd;
    unsigned int  src_buf    = inPerfParams.cmdInfo.src_buf;
    unsigned int  dst_buf    = inPerfParams.cmdInfo.dst_buf;
    unsigned int  kernel_buf = inPerfParams.cmdInfo.kernel_buf;
    float axi_sram_read_bw_limit  = pHwInfo->bwl.axiSramReadBWLimit;
    float axi_sram_write_bw_limit = pHwInfo->bwl.axi_sram_write_bw_limit;
    float axi_sram_total_bw_limit = pHwInfo->bwl.axi_sram_total_bw_limit;
    float axi_bus_read_bw_limit   = pHwInfo->bwl.axi_bus_read_bw_limit;
    float axi_bus_write_bw_limit  = pHwInfo->bwl.axi_bus_write_bw_limit;
    float axi_bus_total_bw_limit  = pHwInfo->bwl.axi_bus_total_bw_limit;
    float internal_write_bw_limit = pHwInfo->bwl.internal_write_bw_limit;
    unsigned int  interleave_mode = inPerfParams.interleave_mode;
    unsigned int  lanes_per_conv  = 64; // default is 64, can manually set to 32 or 128
    unsigned int  outstanding_transfer = pHwInfo->pFeatures->MAX_OT_NUMBER;
    // refine me, should just use number for zrl_bits
    unsigned int  zrl_bits = pHwInfo->pFeatures->ZRL_8BIT ? 8 : (pHwInfo->pFeatures->ZRL_7BIT ? 7 : 5);
    unsigned int  equivalent_vip_sram_width_in_byte = pHwInfo->pFeatures->EQUIVALENT_VIP_SRAM_WIDTH_INBYTE;
    float ddr_latency   = pHwInfo->bwl.ddr_latency;
    float total_latency = pHwInfo->bwl.total_latency;
    float ddr_read_bw_in_byte_per_cycle  = pHwInfo->bwl.ddr_read_bw_in_byte_per_cycle;
    float ddr_write_bw_in_byte_per_cycle = pHwInfo->bwl.ddr_write_bw_in_byte_per_cycle;
    float ddr_total_bw_in_byte_per_cycle = pHwInfo->bwl.ddr_total_bw_in_byte_per_cycle;
    unsigned int  image_not_packed_in_sram = !pHwInfo->pFeatures->IMAGE_NOT_PACKED_IN_SRAM_FIX;

    // following two can be get from data type
    unsigned int    vip_v7_16bit = inPerfParams.vip_v7_16bit;
    unsigned int    fp16 = inPerfParams.cmdInfo.inImageFp16;
    unsigned int    kernel_head_not_cached_fix = pHwInfo->pFeatures->FULLCACHE_KERNELHEAD_FIX;
    //assert(kernel_head_not_cached_fix == 0);
    unsigned int    conv1x1_half_performance = pHwInfo->pFeatures->NN_CONV1x1_PERF_FIX ? 0: 1;
    unsigned int    cache_line_mode_disabled = pHwInfo->pFeatures->NN_CACHELINE_MODE_PERF_FIX? 0:1;
    unsigned int    per_3d_tile_bubble_fix   = pHwInfo->pFeatures->NN_PER3DTILE_BUBBLE_FIX;
    unsigned int    zdp3_no_compress_fix = ((pHwInfo->pFeatures->NN_ZDP3 || pHwInfo->pFeatures->NN_ZDP6)
                                    && pHwInfo->pFeatures->NN_ZDP3_NO_COMPRESS_FIX) ? 1 : 0;
    unsigned int    async_copy_perf_fix = pHwInfo->pFeatures->NN_ASYNC_COPY_PERF_FIX;
    unsigned int    zxdp3_kernel_read_conflict_fix = pHwInfo->pFeatures->NN_ZXDP3_KERNEL_READ_CONFLICT_FIX;
    unsigned int    accurate_tile_bw = ACCURATE_TILE_BW;
    unsigned int    axi_sram_slowed_down_by_addr = AXI_SRAM_SLOWED_DOWN_BY_DDR;
    unsigned int    slow_nn_req_arbitration_fix = pHwInfo->pFeatures->NN_REQ_SLOWARBITRATION_FIX;
    unsigned int    single_port_acc_buffer = pHwInfo->pFeatures->NN_SINGLEPORT_ACCUMBUFFER;
    unsigned int    small_batch_enable = pHwInfo->pFeatures->NN_SMALLBATCH;
    unsigned int    is_depth_wise = inPerfParams.cmdInfo.is_depth_wise;
    unsigned int    is_nn_write_without_usc = (xydp_x == 0 && xydp_y == 0) ? 1: 0;
    unsigned int    in_image_stride = inPerfParams.cmdInfo.in_image_stride;
    unsigned int    in_image_slice  = inPerfParams.in_image_slice;

    if (inPerfParams.out_image_slice == 0)
    {
#ifdef WARNING_PRINT
        printf ("error, fix me!\n");
#endif
        inPerfParams.out_image_slice = x * y;
    }

    unsigned int  out_image_stride = inPerfParams.cmdInfo.out_image_stride;
    unsigned int  out_image_slice  = inPerfParams.out_image_slice;
    unsigned int  flush_and_wait = inPerfParams.bflush;
    unsigned int  conv_out_fifo_depth = 0; //pHwInfo->pFeatures->NN_CONVOUT_FIFO_DEPTH_FIX;
    bool          nn_slow_output_feature = (xydp_x == 0) && (xydp_y == 0); // vip_v8 always has feature nn_slow_output
    if (pHwInfo->pFeatures->NN_CONVOUT_FIFO_DEPTH_FIX)
    {
        conv_out_fifo_depth = (unsigned int)ceilf(1.0f * pHwInfo->pFeatures->NNAccumBufferDepth * cores * (64 - 8) / 64);
    }
    else
    {
        conv_out_fifo_depth = pHwInfo->pFeatures->NNAccumBufferDepth;
    }
    pContext->pInPerfParams->misc.asymmetric_quantization = 0; // fix me

    UpdateManualParams(tile_xsize, tile_ysize, kernel_per_core, inx, iny);
    UpdateManualFeaturesNN(small_batch_enable, lanes_per_conv);

    sanityCheck();

    outBandWidth->cycle_count = NNCycleCountCore(
        tile_xsize,
        tile_ysize,
        kernel_per_core,
        x, // inImageXsize
        y, // inImageYsize
        z,
        kx,
        ky,
        kz,
        inx,
        iny,
        pContext->pooling_stride,
        non_zero_ratio,
        coef_compress_ratio,
        image_compress_ratio,
        cores,
        brick_mode,
        input_data_size,
        output_data_size,
        l2cache_size,
        l2cache_width,
        xydp_x,
        xydp_y,
        zdp,
        float_xydp_x,
        float_xydp_y,
        float_zdp,
        usc_cache_size,
        nn_cmd_size,
        coef_decode_perf,
        vector_prune,
        image_partial_cache,
        data_read_from_sram,
        first_cmd,
        src_buf,
        dst_buf,
        kernel_buf,
        axi_sram_read_bw_limit,
        axi_sram_write_bw_limit,
        axi_sram_total_bw_limit,
        axi_bus_read_bw_limit,
        axi_bus_write_bw_limit,
        axi_bus_total_bw_limit,
        internal_write_bw_limit,
        interleave_mode,
        lanes_per_conv,
        outstanding_transfer,
        zrl_bits,
        equivalent_vip_sram_width_in_byte,
        ddr_latency,
        total_latency,
        ddr_read_bw_in_byte_per_cycle,
        ddr_write_bw_in_byte_per_cycle,
        ddr_total_bw_in_byte_per_cycle,
        image_not_packed_in_sram,
        vip_v7_16bit,
        fp16,
        kernel_head_not_cached_fix,
        conv1x1_half_performance,
        cache_line_mode_disabled,
        per_3d_tile_bubble_fix,
        zdp3_no_compress_fix,
        async_copy_perf_fix,
        zxdp3_kernel_read_conflict_fix,
        accurate_tile_bw,
        axi_sram_slowed_down_by_addr,
        slow_nn_req_arbitration_fix,
        single_port_acc_buffer,
        small_batch_enable,
        is_depth_wise,
        nn_slow_output_feature,
        is_nn_write_without_usc,
        in_image_stride,
        in_image_slice,
        out_image_stride,
        out_image_slice,
        flush_and_wait,
        conv_out_fifo_depth,
        pContext->pInParams->specified_ddr_bw_limit_by_burst,
        outBandWidth,
        outResult
        );
    return outBandWidth->cycle_count;
}

double APMCalcTPCycleCountCore(
    IN  APMHandle          apmHandle,
    IN  APM_IN_PERF_PARAMS inPerfParams,
    OUT APM_OUT_BW_T       *outBandWidth,
    OUT APM_OUT_RESULT_T   *outResult
    )
{
    APM_HW_INFO_T *pHwInfo = (APM_HW_INFO_T *)apmHandle;

    unsigned int type = inPerfParams.op_type;
    unsigned int x = inPerfParams.cmdInfo.u.tpcmd.x;
    unsigned int y = inPerfParams.cmdInfo.u.tpcmd.y;
    unsigned int z = inPerfParams.cmdInfo.u.tpcmd.z;
    unsigned int kz = inPerfParams.cmdInfo.kernelZsize;

    double coef_nonzero_ratio  = inPerfParams.compInfo.coefNonZeroRatio;
    double coef_compress_ratio = inPerfParams.compInfo.coefCompression;
    double image_nonzero_ratio = inPerfParams.compInfo.imageNonZeroRatio;
    unsigned int tpCores       = inPerfParams.op_type != VXNNE_OPERATOR_FULLYCONNECTED ?
            pHwInfo->pFeatures->TPEngine_CoreCount : pHwInfo->pFeatures->TPEngine_CoreCount + pHwInfo->pFeatures->TPLite_CoreCount;
    unsigned int tp_cmd_size      = TP_COMMAND_SIZE;
    unsigned int pooling_stride   = inPerfParams.cmdInfo.pooling_stride;
    float axi_sram_read_bw_limit  = pHwInfo->bwl.axiSramReadBWLimit;
    float axi_sram_write_bw_limit = pHwInfo->bwl.axi_sram_write_bw_limit;
    float axi_sram_total_bw_limit = pHwInfo->bwl.axi_sram_total_bw_limit;
    float axi_bus_read_bw_limit   = pHwInfo->bwl.axi_bus_read_bw_limit;
    float axi_bus_write_bw_limit  = pHwInfo->bwl.axi_bus_write_bw_limit;
    float axi_bus_total_bw_limit  = pHwInfo->bwl.axi_bus_total_bw_limit;
    unsigned int l2cache_width = pHwInfo->pFeatures->EQUIVALENT_VIP_SRAM_WIDTH_INBYTE;
    unsigned int input_data_size  = inPerfParams.inDataBitSize; // input data size in bit
    //unsigned int output_data_size  = inPerfParams.outDataBitSize; // output data size in bit
    /*unsigned int sw_tiling  = pHwInfo->pFeatures->SWTILING_PHASE1; // refine me, why driver is so difficault */
    unsigned int src_buf    = inPerfParams.cmdInfo.src_buf;
    unsigned int dst_buf    = inPerfParams.cmdInfo.dst_buf;
    unsigned int kernel_buf = inPerfParams.cmdInfo.kernel_buf;
    unsigned int outstanding_transfer = pHwInfo->pFeatures->MAX_OT_NUMBER;
    float ddr_latency   = pHwInfo->bwl.ddr_latency;
    float total_latency = pHwInfo->bwl.total_latency;
    float ddr_read_bw_in_byte_per_cycle  = pHwInfo->bwl.ddr_read_bw_in_byte_per_cycle;
    float ddr_write_bw_in_byte_per_cycle = pHwInfo->bwl.ddr_write_bw_in_byte_per_cycle;
    float ddr_total_bw_in_byte_per_cycle = pHwInfo->bwl.ddr_total_bw_in_byte_per_cycle;
    unsigned int usc_cache_controllers = pHwInfo->pFeatures->USC_CACHE_CONTROLLERS;
    unsigned int tp_reorder_fix = pHwInfo->pFeatures->TP_REORDER_FIX; // todo
    unsigned int flush = inPerfParams.bflush;
    unsigned int small_batch_enable = pHwInfo->pFeatures->NN_SMALLBATCH;
    unsigned int axi_sram_slowed_down_by_ddr = AXI_SRAM_SLOWED_DOWN_BY_DDR; // refine me, why hardcode in driver
    double *ddr_kernel_read_bw   = &outBandWidth->ddr_kernel_read_bandwidth;
    double *ddr_in_image_read_bw = &outBandWidth->ddr_in_image_read_bandwidth;
    double *ddr_read_bw  = &outBandWidth->ddr_read_bandwidth;
    double *ddr_write_bw = &outBandWidth->ddr_write_bandwidth;
    double *axi_sram_read_bw  = &outBandWidth->axi_sram_read_bw;
    double *axi_sram_write_bw = &outBandWidth->axi_sram_write_bw;

    UpdateManualFeaturesTP(small_batch_enable);

    outBandWidth->cycle_count = TPCycleCountCore(type, x, y, z, kz,
        coef_nonzero_ratio, coef_compress_ratio,
        image_nonzero_ratio, tpCores, tp_cmd_size,
        pooling_stride,
        axi_sram_read_bw_limit, axi_sram_write_bw_limit, axi_sram_total_bw_limit,
        axi_bus_read_bw_limit, axi_bus_write_bw_limit, axi_bus_total_bw_limit,
        l2cache_width,
        input_data_size,
        /*sw_tiling, */
        src_buf, dst_buf, kernel_buf,
        outstanding_transfer,
        ddr_latency,
        total_latency,
        ddr_read_bw_in_byte_per_cycle,
        ddr_write_bw_in_byte_per_cycle,
        ddr_total_bw_in_byte_per_cycle,
        usc_cache_controllers,
        tp_reorder_fix,
        flush,
        small_batch_enable,
        axi_sram_slowed_down_by_ddr,
        pContext->pInParams->specified_ddr_bw_limit_by_burst,
        ddr_kernel_read_bw, ddr_in_image_read_bw,
        ddr_read_bw, ddr_write_bw,
        axi_sram_read_bw, axi_sram_write_bw,
        outResult
        );

    return outBandWidth->cycle_count;
}

APMHandle CreateAPModel (
    IN APM_IN_PARAM_T inParam
    )
{
    APM_HW_INFO_T *p_hwInfo = (APM_HW_INFO_T *) malloc(sizeof(APM_HW_INFO_T));
    memset(p_hwInfo, 0, sizeof(APM_HW_INFO_T));
    APM_IN_PARAM_T *p_inParam = (APM_IN_PARAM_T *)malloc(sizeof(APM_IN_PARAM_T));
    memset(p_inParam,0,sizeof(APM_IN_PARAM_T));

    // query features from feature DB, according to chip defination
    p_hwInfo->pFeatures = gcQueryFeatureDB(inParam.chipDef.ChipID, inParam.chipDef.ChipVersion,
                                           inParam.chipDef.ProductID, inParam.chipDef.EcoID,
                                           inParam.chipDef.CustomerID);
    if (p_hwInfo->pFeatures == NULL)
    {
        printf("Error, not found chip def: ChipID:0x%x, ChipVersion:0x%x, ProductID:0x%x, EcoID:0x%x, \
               CustomerID:0x%x!\n", inParam.chipDef.ChipID, inParam.chipDef.ChipVersion, \
               inParam.chipDef.ProductID, inParam.chipDef.EcoID, inParam.chipDef.CustomerID);
        assert(0);
    }

    InitHWModeling(inParam.bwl);

    // copy fpga info from inParam
    memcpy(&p_hwInfo->fpagInfo, &inParam.fpagInfo, sizeof(FPGA_INFO_T));

    // copy bandwidth latency information from inParam
    memcpy(&p_hwInfo->bwl, &inParam.bwl, sizeof(BWL_T));
    p_hwInfo->INT8_MAC_PER_CYCLE_NN = inParam.INT8_MAC_PER_CYCLE_NN;
    memset (&context, 0, sizeof(apm_context_type));
    pContext = &context;
    pContext->p_hwInfo = p_hwInfo;
    memcpy(p_inParam,&inParam, sizeof(APM_IN_PARAM_T));
    pContext->pInParams = p_inParam;

    initBugStatus(&pContext->bf);
    initFeatureStatus(&pContext->bf);


    return (APMHandle)(p_hwInfo);
}

void InitHWModeling(
    BWL_T &bwl
    )
{
    float totalLatency = bwl.total_latency; // total_latency is generate in archApmInit
    float OUTSTANDING_TRANSFER = bwl.maxSocOTNumber;

    if (OUTSTANDING_TRANSFER == 0)
    {
        printf("CArch SOC Out Standing Number is 0!, set 32 as default"); // //
        OUTSTANDING_TRANSFER = 32;
    }

    float MaxOutstandingCycle = OUTSTANDING_TRANSFER * 4; // 4 cycles one request, latency hiding at full axi bw
    if (totalLatency > MaxOutstandingCycle)
    {
        float DDR_BW_limited_by_latency    = (16 * MaxOutstandingCycle) / totalLatency;
        bwl.ddr_read_bw_in_byte_per_cycle  = min(bwl.ddr_read_bw_in_byte_per_cycle, DDR_BW_limited_by_latency);
        bwl.ddr_write_bw_in_byte_per_cycle = min(bwl.ddr_write_bw_in_byte_per_cycle, DDR_BW_limited_by_latency);
    }
}

void DestroyAPModel(
    IN APMHandle apmHandle
    )
{
    if (pContext->manualFeatureSetted)
    {
        free(pContext->pManualFeatures);
        pContext->pManualFeatures = NULL;
    }

    if (pContext->pInParams != NULL)
    {
        free(pContext->pInParams);
        pContext->pInParams = NULL;
    }

    if (apmHandle != NULL)
    {
        free(apmHandle);
        apmHandle = NULL;
    }
}

unsigned int APMCalcImageInterleaveMode(
    unsigned int x,
    unsigned int mad_per_core,
    unsigned int kxy,
    unsigned int vip7_fp16,
    unsigned int interleave8)
{
    return _calcImageInterleaveMode(x, mad_per_core, kxy, vip7_fp16, interleave8);
}

unsigned int APMCalcNumOfKernel(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int z,
    unsigned int accu_buf_depth,
    unsigned int cores,
    unsigned int interleave_mode,
    unsigned int zdp,
    unsigned int kx,
    unsigned int ky,
    unsigned int xdpx,
    unsigned int isV8,
    /*unsigned int data_size, */
    unsigned int lanes_per_conv,
    unsigned int pooling_stride,
    bool         isDepthWise,
    bool         isDepthWiseMerge,
    bool         kernel_per_core_lt_one_third_coef_fix /* bug2000 */,
    bool         asymmetricQuantization
    )
{
    if (pContext->manualFeatureSetted)
    {
        lanes_per_conv = pContext->pManualFeatures->lanes_per_conv;
    }

    return _calcNumOfKernel(tile_xsize, tile_ysize, z, accu_buf_depth, cores, interleave_mode, zdp, \
                            kx, ky, xdpx, isV8, /*data_size,*/ lanes_per_conv, pooling_stride, \
                            isDepthWise, isDepthWiseMerge, kernel_per_core_lt_one_third_coef_fix, /* bug2000 */ \
                            asymmetricQuantization
                            );
}

ARCH_PERF_MODEL_API double APMCalcKernelCachePercentage(
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int z,
    unsigned int cores,
    double       coef_compress_ratio,
    unsigned int cache_size_in_pixel,
    OUT double *adj_cache_size_in_pixel,
    OUT double *KernelIdealCache
    /*,
    bool         full_cach_kernel_head_fix ,
    bool         is_depth_wise*/
    )
{
    return _calcKernelCachePercentage(kx, ky, kz, z, cores, coef_compress_ratio, \
                                        cache_size_in_pixel,
                                        adj_cache_size_in_pixel,
                                        KernelIdealCache
                                        /*, full_cach_kernel_head_fix,\
                                        is_depth_wise*/
                                        );
}

ARCH_PERF_MODEL_API float APMCalcImageIdealCacheInPixel(
    unsigned int tile_xsize,
    unsigned int tile_ysize,
    unsigned int kx,
    unsigned int ky,
    unsigned int kz,
    unsigned int x,
    unsigned int y,
    int          xoffset,
    int          yoffset,
    unsigned int sub_x,
    unsigned int sub_y,
    unsigned int data_size,
    bool         image_not_packed_in_sram,
    unsigned int equivalent_vip_sram_width_in_byte
    )
{
    return ImageIdealCacheInPixel(tile_xsize, tile_ysize, kx, ky, kz, x, y, xoffset, \
                                         yoffset, sub_x, sub_y, data_size, \
                                         image_not_packed_in_sram, equivalent_vip_sram_width_in_byte
                                         );
}

void initBugStatus(APM_BUG_FEATURE_TYPE * pBf)
{
    pBf->IMG_POP_PIPELINE_PAUSE_BUG2029      = !pContext->p_hwInfo->pFeatures->IMG_POP_PIPELINE_PAUSE_FIX; // get value from feature database
    pBf->PARTIAL_KERNEL_CACHE_INTERLEAVE_BUG_2007 = !pContext->p_hwInfo->pFeatures->KERNEL_SIZE_WASTE_IN_PARTIAL_MODE_FIX;
    pBf->LOW_EFFICIENCY_JD_WR_IMGBUF_BUG1992 = 1; // bug has not, use feature DB for it?
    pBf->COEF_ZERO_POINT_AREA_OPTIMIZATION   = 0;
    pBf->INTERNAL_KERNEL_READ_BOTTLENECK_BUG1998 = !pContext->p_hwInfo->pFeatures->KERNEL_VIP_SRAM_READ_BW_LIMITATION_FIX;

    if (pBf->INTERNAL_KERNEL_READ_BOTTLENECK_BUG1998)
    {
        // refine me
        //int i = pContext->pInPerfParams->misc.asymmetric_quantization;
        pBf->Internal_Kernel_Read_BYTE_PER_CYCLE = 16;
    }
    else
    {
        // fix me, should be EQUIVALENT_VIP_SRAM_WIDTH_IN_BYTE
        pBf->Internal_Kernel_Read_BYTE_PER_CYCLE = 32;
    }
    // small batch phase 1
    pBf->PREFETCH_NN_COMMAND_KERNEL_HEADER = pContext->p_hwInfo->pFeatures->SMALLBATCH;

    pBf->NO_NARROW_POST_PROCESS_PIPE = 0;
    /* have not fix and add feature yet. 7/14/2019 */
    /* refine me */
    pBf->Full_KERNEL_CACHE_INTERLEAVE_BUG_2033 = !pContext->p_hwInfo->pFeatures->FULLCACHE_KERNEL_INTERLEAVE_FIX;
    pBf->NEIGHBOR_IMG_TRAN_NOT_EFFICIENT_BUG2045 = 0; //set to 0 until it can be passed from featureDB

    /* initialize in below until actual value can be passed down from FeatureTable*/
    pBf->TP_VIPSRAM_OT1_BUG2050 = 0;
}

void initFeatureStatus(
    APM_BUG_FEATURE_TYPE *pBf)
{
    /* initialize in below until actual value can be passed down from FeatureTable*/
    pBf->MxN_interleave_pooling = 0; // not impliment yet
    pBf->VIPSRAM_ASYNC_FIFO = 0;

    /* initialize newly added features */
    pBf->new_feature = 0; //Shuangbei: initialized here as global variable. Its value will be modified in UpdateManualFeaturesNN() or UpdateManualFeaturesTP().
}

/* sanity checks before call arch hardware model */
void sanityCheck()
{
    if (pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_in == 0)
    {
#ifdef WARNING_PRINT
        printf("warning: transpose interleave channel in is 0, default change to 1!\n");
#endif
        pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_in = 1;
    }

    if (pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_out == 0)
    {
#ifdef WARNING_PRINT
        printf("warning: transpose interleave channel in is 0, default change to 1!\n");
#endif
        pContext->pInPerfParams->cmdInfo.TrspInterleaveCh_out = 1;
    }

    if (pContext->pInParams->NN_DDR_BURST_SIZE == 0)
    {
#ifdef WARNING_PRINT
        printf("warning: NN_DDR_BURST_SIZE is 0, set to default value 64!\n");
#endif
        pContext->pInParams->NN_DDR_BURST_SIZE = 64;
    }

    if (pContext->bf.NN_LARGE_BURST_SIZE == 0)
    {
#ifdef WARNING_PRINT
        printf("warning: NN_DDR_BURST_SIZE is a feature switch, set to default value 0!\n");
#endif
        pContext->bf.NN_LARGE_BURST_SIZE = 0;
    }
}



