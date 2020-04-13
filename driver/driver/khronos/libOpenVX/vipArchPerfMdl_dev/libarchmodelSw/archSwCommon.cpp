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


/***********************************************************************************
* Copyright:    Verisilicon
* FileName:        archSwCommon.c
* Author:        JinboHuang
* Data:            2019-05-27
* Version:        0.5.00
* Description:    Definition common function for Arch Model Library
*
***************************************************************************************/
#include "archSwPerf.h"
#include "archSwCommon.h"


/*******************************************MARCO definition***************************************************/
#define NUMBER_OF_NN_COST_TYPE 6
#define AXI_BURST_SIZE 64

#define NN_DDR_BURST_SIZE   256             /* get 256 from CTS xlsm file, may need to adjust according to the driver */
enum
{
    ARCHNNE_SRAM_CACHE_MODE_NONE = 0,
    ARCHNNE_SRAM_CACHE_MODE_PARTIAL_CACHE,
    ARCHNNE_SRAM_CACHE_MODE_FULL_CACHE,
    ARCHNNE_SRAM_CACHE_MODE_STREAM_CACHE
};

static arch_int32 archDebugLevel = ARCH_DEBUG_LEVEL_NONE;
#define ARCH_PRINT_BUFFER_COUNT      2048

/* Some times the ceilf can not work on ARM platform, need to re-declare it */
/*arch_float32 ceilf(arch_float32 x);*/

/*******************************************Global var definition*****************************************************/
static arch_int8 gOrigShowType = -1;
static const arch_char *archModelVersion = "ARCHCTS@235166";
static const arch_char *SWTilingVersion = "ARCHCTS@235166";
/* Merge from CL 212817 */

extern arch_uint32 gAlign64BExceted;

#ifndef ceilf
#define ceilf ceil
#endif

/****************************************** Local function declaration ***********************************************/


/****************************************** Local function declaration done ***********************************************/


/* debug print related definition */
void archSetDebugLevel(arch_int32 level)
{
    archDebugLevel = level;
}

void archPRINT(arch_uint32 level, const char *msg, ...)
{
    va_list args;
    int size;
    char buffer[ARCH_PRINT_BUFFER_COUNT];
    if (level == ARCH_DEBUG_LEVEL_NONE)
    {
        return;
    }
    va_start(args, msg);
    size = vsnprintf(buffer, VX_PRINT_BUFFER_COUNT - 1, msg, args);
    buffer[size] = '\0';
    va_end(args);
    fprintf(stdout, "%s", buffer);
    fflush(stdout);
}


/***********************************************************************************
* Function:     archAllocateMemory
* Description:  Allocate memory
* Input:        :
*               :
* Ouput:
***************************************************************************************/
ARCH_INTERNAL_API archSTATUS archAllocateMemory(size_t size, archPOINTER *pMemory)
{
    archSTATUS status = archSTATUS_OK;
    archPOINTER pTemp = NULL;

    pTemp = malloc(size);

    if (pTemp == NULL)
    {
        archError("Failed to allocate enough memory");
        status = archSTATUS_OUT_OF_RESOURCES;
    }
    else
    {
        *pMemory = pTemp;
    }
    return status;
}


/***********************************************************************************
* Function:     archFreeMemory
* Description:  Free memory
* Input:        :
*               :
* Ouput:
***************************************************************************************/
ARCH_INTERNAL_API void archFreeMemory(archPOINTER pMemory)
{
    free(pMemory);
    pMemory = NULL;
}


/***********************************************************************************
* Function:     archAllocateAndZeroMemory
* Description:  Allocate memory and set to zero
* Input:        :
*               :
* Ouput:
***************************************************************************************/
ARCH_INTERNAL_API archPOINTER archAllocateAndZeroMemory(size_t size)
{
    archSTATUS status = archSTATUS_OK;
    archPOINTER pMemory = NULL;
    status = archAllocateMemory(size, &pMemory);
    if(status != archSTATUS_OK)
    {
        return NULL;
    }
    memset(pMemory,0,size);
    return pMemory;
}

/***********************************************************************************
* Function:     archnneGetOperatorTypeName
* Description:  Get the Operator Name in string via the operation type
* Input:        :
*               :
* Ouput:
***************************************************************************************/
static const arch_char* archnneGetOperatorTypeName(archnne_operator_e operationType)
{
    switch (operationType)
    {
    case ARCHNNE_OPERATOR_CONVOLUTION:
        return "VXNNE_OPERATOR_CONVOLUTION";

    case ARCHNNE_OPERATOR_RESHUFFLE:
        return "VXNNE_OPERATOR_RESHUFFLE";

    case ARCHNNE_OPERATOR_FULLYCONNECTED:
        return "VXNNE_OPERATOR_FULLYCONNECTED";

    case ARCHNNE_OPERATOR_ACTIVATION:
        return "VXNNE_OPERATOR_ACTIVATION";

    case ARCHNNE_OPERATOR_POOLING:
        return "VXNNE_OPERATOR_POOLING";

    case ARCHNNE_OPERATOR_RESIZE:
        return "VXNNE_OPERATOR_RESIZE";

    case ARCHNNE_OPERATOR_TENSOR_ADD:
        return "VXNNE_OPERATOR_TENSOR_ADD";

    case ARCHNNE_OPERATOR_TENSOR_TRANS:
        return "VXNNE_OPERATOR_TENSOR_TRANS";

    case ARCHNNE_OPERATOR_SOFTMAX:
        return "VXNNE_OPERATOR_SOFTMAX";

    case ARCHNNE_OPERATOR_NORMALIZATION:
        return "VXNNE_OPERATOR_NORMALIZATION";

    case ARCHNNE_OPERATOR_BATCHNORM:
        return "VXNNE_OPERATOR_BATCHNORM";

    case ARCHNNE_OPERATOR_INPUT2WEIGHT:
        return "VXNNE_OPERATOR_INPUT2WEIGHT";

    case ARCHNNE_OPERATOR_RPN:
        return "VXNNE_OPERATOR_RPN";

    case ARCHNNE_OPERATOR_ROIPOOL:
        return "VXNNE_OPERATOR_ROIPOOL";

    case ARCHNNE_OPERATOR_CONCAT2:
        return "VXNNE_OPERATOR_CONCAT2";

    case ARCHNNE_OPERATOR_DILATION_RESHUFFLE:
        return "VXNNE_OPERATOR_DILATION_RESHUFFLE";

    case ARCHNNE_OPERATOR_DILATION_UPSAMPLE:
        return "VXNNE_OPERATOR_DILATION_UPSAMPLE";

    case ARCHNNE_OPERATOR_DILATION_UPSAMPLE2:
        return "VXNNE_OPERATOR_DILATION_UPSAMPLE2";

    case ARCHNNE_OPERATOR_DEPTH_WISE_CONV:
        return "VXNNE_OPERATOR_DEPTH_WISE_CONV";

    case ARCHNNE_OPERATOR_CONCATINDEFINITE:
        return "VXNNE_OPERATOR_CONCAT_INDEFINITE";

    case ARCHNNE_OPERATOR_TENSOR_RESHAPE:
        return "VXNNE_OPERATOR_TENSOR_RESHAPE";
    default:
        return "unkown operation type";
    }
}

/***********************************************************************************
* Function:     archnneGetOperatorTargetName
* Description:  Get the Operator Target Name in string via the operation target
* Input:        :
*               :
* Ouput:
***************************************************************************************/
static const arch_char* archnneGetOperatorTargetName(archnne_operation_target_e operationTarget)
{
    switch (operationTarget)
    {
    case ARCHNNE_OPERATION_TARGET_SW:
        return "VXNNE_OPERATION_TARGET_SW";

    case ARCHNNE_OPERATION_TARGET_NN:
        return "VXNNE_OPERATION_TARGET_NN";

    case ARCHNNE_OPERATION_TARGET_SH:
        return "VXNNE_OPERATION_TARGET_SH";

    case ARCHNNE_OPERATION_TARGET_TP:
        return "VXNNE_OPERATION_TARGET_TP";

    case ARCHNNE_OPERATION_TARGET_SC:
        return "VXNNE_OPERATION_TARGET_SC";

    default:
        return "unkown operation target";
    }
}

/***********************************************************************************
* Function:     archnneGetCacheModeName
* Description:  Get the Operator Cache Mode Name in string via the operation cacheMode
* Input:        :
*               :
* Ouput:
***************************************************************************************/
static const arch_char* archnneGetCacheModeName(arch_int32 cacheMode)
{
    switch (cacheMode)
    {
    case ARCHNNE_SRAM_CACHE_MODE_NONE:
        return "VXNNE_SRAM_CACHE_MODE_NONE";

    case ARCHNNE_SRAM_CACHE_MODE_PARTIAL_CACHE:
        return "VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE";

    case ARCHNNE_SRAM_CACHE_MODE_FULL_CACHE:
        return "VXNNE_SRAM_CACHE_MODE_FULL_CACHE";

    case ARCHNNE_SRAM_CACHE_MODE_STREAM_CACHE:
        return "VXNNE_SRAM_CACHE_MODE_STREAM_CACHE";

    default:
        return "unkown cache mode";
    }
}

/* return WARType */
static arch_uint32 WAR_1X1_Reshape_To_ZDPOpt(arch_uint32* x, arch_uint32* y, arch_uint32* ky, arch_uint32* kz,
                                             arch_uint32* inx, arch_uint32* iny, arch_uint32* instride, arch_uint32* inslice,
                                             arch_uint32* outstride, arch_uint32* outslice, arch_char *warType)
{
    arch_uint32 tempX = 0;
    arch_uint32 tempY = 0, tempKy = 0, tempKz = 0, tempInx = 0, tempIny = 0;
    arch_uint32 tempInstride = 0, tempInslice = 0, tempOutstride = 0;
    tempX = (*x) * (*y);
    tempY = 1;
    tempKy = 2;
    tempKz = (*kz) / 2;
    tempInx = tempX;
    tempIny = tempY * 2;

    tempInstride = *inslice;;
    tempInslice = tempInstride * 2;
    tempOutstride = *outslice;

    /* set back value */
    *x = tempX;
    *y = tempY;
    *ky = tempKy;
    *kz = tempKz;
    *inx = tempInx;
    *iny = tempIny;
    *instride = tempInstride;
    *inslice = tempInslice;
    *outstride = tempOutstride;

    memcpy(warType, "ZDPOpt", 8);
    return 0;
}


static arch_uint32 WAR_1X1_Reshape_To_Nx1(
    arch_uint32* x,
    arch_uint32* y,
    arch_uint32* ky,
    arch_uint32* inx,
    arch_uint32* iny,
    arch_uint32* instride,
    arch_uint32* inslice,
    arch_uint32* outstride,
    arch_uint32* outslice,
    arch_char  *warType
    )
{
    arch_uint32 tempX = 0;
    arch_uint32 tempY = 0, tempKy = 0, tempInx = 0, tempIny = 0;
    arch_uint32 tempInstride = 0, tempOutstride = 0;
    tempX = (*x) * (*y);
    tempY = 1;
    tempKy = 1;
    tempInx = tempX;
    tempIny = tempY;

    tempInstride = *inslice;;
    tempOutstride = *outslice;

    /* set back value */
    *x = tempX;
    *y = tempY;
    *ky = tempKy;
    *inx = tempInx;
    *iny = tempIny;
    *instride = tempInstride;
    *outstride = tempOutstride;

    memcpy(warType, "Nx1", 8);
    return 0;
}

static void WAR_1X1_Reshape_To_MxN(
    arch_uint32 align,
    arch_uint32* x,
    arch_uint32* y,
    arch_uint32* ky,
    arch_uint32* kz,
    arch_uint32* inx,
    arch_uint32* iny,
    arch_uint32* instride,
    arch_uint32* outstride,
    arch_char *warType
    )
{
    arch_uint32 x_adj = 0, y_adj = 0;
    x_adj = align;
    y_adj = (*x) * (*y) / x_adj;

    if(y_adj <= 8192)       /* if y_adj > 8192, don't do reshape */
    {
        *x = x_adj;
        *y = y_adj;
        *ky = 1;
        *kz = *kz;
        *inx = x_adj;
        *iny = y_adj;
        *instride = x_adj;
        *outstride = x_adj;

        sprintf(warType, "%dx%d", *x, *y);
    }
}

/*
*/
/************************************************** for Show performance *************************************/
/* process function druing predict or show performance */
static arch_uint32 _kernel_size_in_pixel_by_vx_arch_perf(
    archnne_operation_target_e opTarget,
    archnne_operator_e opType,
    arch_perf perf,
    arch_bool fullCacheIntervalFix)
{
    arch_float64 coefCompressionRatio = perf->coefCompressRatio;
    arch_float64 marginRatio = (1.25 - 1.05) * (1 - archMIN(1, coefCompressionRatio)) / (1 - 0.02) + 1.05;
    if (opTarget != ARCHNNE_OPERATION_TARGET_TP || opType == ARCHNNE_OPERATOR_FULLYCONNECTED)
    {
        if (coefCompressionRatio)
        {
            if (fullCacheIntervalFix)        /* bug 2033 */
            {
                if(perf->opType == ARCHNNE_OPERATOR_DEPTH_WISE_CONV)
                {
                    return (arch_uint32)(perf->info.kx
                          * perf->info.ky
                          * perf->info.outz
                          * coefCompressionRatio * marginRatio);
                }
                else
                {
                    return (arch_uint32)(perf->info.kx
                          * perf->info.ky
                          * perf->info.kz
                          * perf->info.outz
                          * coefCompressionRatio * marginRatio);
                }
            }
            else
            {
               if(perf->opType == ARCHNNE_OPERATOR_DEPTH_WISE_CONV)
               {
                    return (arch_uint32)((arch_float32)perf->info.kx
                          * perf->info.ky
                          * ceilf((arch_float32)perf->info.outz / perf->info.nnCores) * perf->info.nnCores
                          * coefCompressionRatio * marginRatio);
               }
               else
               {
                    return (arch_uint32)((arch_float32)perf->info.kx
                          * perf->info.ky
                          * perf->info.kz
                          * ceilf((arch_float32)perf->info.outz / perf->info.nnCores) * perf->info.nnCores
                          * coefCompressionRatio * marginRatio);
               }
            }
        }
    }
    return 0;
}


void _calcArchModelCacheMode(APMHandle apm,
                             arch_nn_config *pArchNnConfig,
                             arch_perf perf,
                             arch_int32 *image_ideal_cache_size_in_pixel
                             )
{
    if (perf->opTarget == ARCHNNE_OPERATION_TARGET_NN)
    {
        arch_int32 cacheSpaceLeftInPixel = 0;
        arch_float64 kernelCachePercentage = 0, kernelSizeInPixel = 0, kernelIdalCache = 0;
        arch_bool axiSramOnlySWTiling = pArchNnConfig->unifiedFeature.axiSramOnlySWTiling ? arch_true_e : arch_false_e;
        arch_int32 fullCacheSpaceSizeInPixel = ARCH_VIP_SRAM_SIZE;
        arch_int32 imageIdealCacheSizeInPixel = 0;

        if(apm)
        {
            imageIdealCacheSizeInPixel = (arch_int32)APMCalcImageIdealCacheInPixel(
                perf->resultInfo.outImageTileXSize,
                perf->resultInfo.outImageTileYSize,
                perf->info.kx, perf->info.ky, perf->info.kz,
                perf->swTilingInfo.origInX,
                perf->swTilingInfo.origInY,
                perf->info.xOffSet, perf->info.yOffSet,
                perf->info.inx, perf->info.inx,
                perf->info.inputDataSize,
                pArchNnConfig->unifiedFeature.imageNotPackedInSram,
                pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte);
        }


        if (NULL != image_ideal_cache_size_in_pixel) *image_ideal_cache_size_in_pixel = imageIdealCacheSizeInPixel;
        if (perf->swTilingInfo.srcBuf == 0 && perf->swTilingInfo.dstBuf == 0) /*ddr -> ddr*/
        {
            if (imageIdealCacheSizeInPixel <= fullCacheSpaceSizeInPixel)
            {
                perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_FULL_CACHE;
                cacheSpaceLeftInPixel = fullCacheSpaceSizeInPixel - imageIdealCacheSizeInPixel;
            }
            else
            {
                perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_NONE;
                cacheSpaceLeftInPixel = fullCacheSpaceSizeInPixel;
            }

            if(apm)
            {
                kernelCachePercentage = APMCalcKernelCachePercentage(perf->info.kx, perf->info.ky, perf->info.kz,
                                                            perf->swTilingInfo.origOutZ, perf->info.nnCores,
                                                            perf->coefCompressRatio, cacheSpaceLeftInPixel,
                                                            &kernelSizeInPixel, &kernelIdalCache/*,
                                                            pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix,
                                                            (perf->opType == ARCHNNE_OPERATOR_DEPTH_WISE_CONV) */ );
            }

        }
        else if (perf->swTilingInfo.kernelBuf == 0) /*ab buffer*/
        {
            if (axiSramOnlySWTiling)
            {
                if (imageIdealCacheSizeInPixel <= fullCacheSpaceSizeInPixel)
                {
                    perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_FULL_CACHE;
                    cacheSpaceLeftInPixel = fullCacheSpaceSizeInPixel - imageIdealCacheSizeInPixel;
                }
                else
                {
                    perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_NONE;
                    cacheSpaceLeftInPixel = fullCacheSpaceSizeInPixel;
                }

                if(apm)
                {
                    kernelCachePercentage = APMCalcKernelCachePercentage(perf->info.kx, perf->info.ky, perf->info.kz,
                                                                perf->swTilingInfo.origOutZ, perf->info.nnCores,
                                                                perf->coefCompressRatio, cacheSpaceLeftInPixel,
                                                                &kernelSizeInPixel, &kernelIdalCache /*,
                                                                pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix,
                                                                (perf->opType == ARCHNNE_OPERATOR_DEPTH_WISE_CONV) */);
                }
            }
            else
            {
                cacheSpaceLeftInPixel = axiSramOnlySWTiling ? fullCacheSpaceSizeInPixel
                                                        : (arch_int32)(fullCacheSpaceSizeInPixel - perf->swTilingInfo.segTotalBufferSizeInPixel);
                if (perf->swTilingInfo.srcBuf == 0)
                {
                    if (imageIdealCacheSizeInPixel <= cacheSpaceLeftInPixel)
                    {
                        perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_FULL_CACHE;
                        cacheSpaceLeftInPixel = cacheSpaceLeftInPixel - imageIdealCacheSizeInPixel;
                    }
                    else
                    {
                        perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_NONE;
                    }
                }
                else
                {
                    perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_NONE;
                }

                if(apm)
                {
                    kernelCachePercentage = APMCalcKernelCachePercentage(perf->info.kx, perf->info.ky, perf->info.kz,
                                                            perf->swTilingInfo.origOutZ, perf->info.nnCores,
                                                            perf->coefCompressRatio, cacheSpaceLeftInPixel,
                                                            &kernelSizeInPixel, &kernelIdalCache /*,
                                                            pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix,
                                                            (perf->opType == ARCHNNE_OPERATOR_DEPTH_WISE_CONV) */);
                }

            }
        }
        else
        {
            assert(perf->swTilingInfo.kernelBuf);

            if (axiSramOnlySWTiling)
            {
                arch_uint32 swTilingSegKernelBufSize = perf->swTilingInfo.swTilingSegKernelBufSizeInPixel;
                cacheSpaceLeftInPixel = (arch_int32)(fullCacheSpaceSizeInPixel - swTilingSegKernelBufSize);
                if (imageIdealCacheSizeInPixel <= cacheSpaceLeftInPixel)
                {
                    perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_FULL_CACHE;
                }
                else
                {
                    perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_NONE;
                }
            }
            else
            {
                if (perf->swTilingInfo.srcBuf == 0)
                {
                    cacheSpaceLeftInPixel = (arch_int32)(fullCacheSpaceSizeInPixel - perf->swTilingInfo.segTotalBufferSizeInPixel);
                    if (imageIdealCacheSizeInPixel <= cacheSpaceLeftInPixel)
                    {
                        perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_FULL_CACHE;
                    }
                    else
                    {
                        perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_NONE;
                    }
                }
                else
                {
                    perf->swTilingInfo.imageCacheMode = ARCHNNE_SRAM_CACHE_MODE_NONE;
                }
            }
            kernelCachePercentage = 0;
        }
        perf->swTilingInfo.kernelCacheMode = (kernelCachePercentage == 1.0f) ? ARCHNNE_SRAM_CACHE_MODE_FULL_CACHE
                                             : (kernelCachePercentage > 0) ? ARCHNNE_SRAM_CACHE_MODE_PARTIAL_CACHE
                                             : (perf->swTilingInfo.kernelBuf) ? ARCHNNE_SRAM_CACHE_MODE_STREAM_CACHE : ARCHNNE_SRAM_CACHE_MODE_NONE;
        /* save kernel cache percentage  */
        perf->swTilingInfo.kernelSizeInPixel = kernelSizeInPixel;
        perf->swTilingInfo.kernelIdalCache = kernelIdalCache;
        perf->swTilingInfo.kernelCachePercentage = kernelCachePercentage;
    }

}

#define DUMP_DETAIL
#ifdef DUMP_DETAIL
void printArchDetail(arch_perf perf)
{
    archInfo("Arch Detail is:\n");
    archInfo("computeCC is %f.\n",perf->archPerfDetailResult.computeCC);
    archInfo("ddrRdCC is %f.\n",perf->archPerfDetailResult.ddrRdCC);
    archInfo("ddrWrCC is %f.\n",perf->archPerfDetailResult.ddrWrCC);
    archInfo("axiSramRdCC is %f.\n",perf->archPerfDetailResult.axiSramRdCC);
    archInfo("axiSramWrCC is %f.\n",perf->archPerfDetailResult.axiSramWrCC);
    archInfo("axiBusRdCC is %f.\n",perf->archPerfDetailResult.axiBusRdCC);
    archInfo("axiBusWrCC is %f.\n",perf->archPerfDetailResult.axiBusWrCC);
    archInfo("vipSramRdCC is %f.\n",perf->archPerfDetailResult.vipSramRdCC);
    archInfo("vipSramWrCC is %f.\n",perf->archPerfDetailResult.vipSramWrCC);
    archInfo("slowInternalWrCC is %f.\n",perf->archPerfDetailResult.slowInternalWrCC);
    archInfo("slowCompCC is %f.\n",perf->archPerfDetailResult.slowCompCC);
    archInfo("internalWrCC is %f.\n",perf->archPerfDetailResult.internalWrCC);
    archInfo("dWOutCC is %f.\n",perf->archPerfDetailResult.dWOutCC);
    archInfo("kernelDdrRdCC is %f.\n",perf->archPerfDetailResult.kernelDdrRdCC);
    archInfo("inImageDdrRdCC is %f.\n",perf->archPerfDetailResult.inImageDdrRdCC);
    archInfo("kernelDecodeCC is %f.\n",perf->archPerfDetailResult.kernelDecodeCC);
    archInfo("dqArbCC is %f.\n",perf->archPerfDetailResult.dqArbCC);
    archInfo("regTile2DxBarCC is %f.\n",perf->archPerfDetailResult.regTile2DxBarCC);
    archInfo("bottomTile2DXBarCC is %f.\n",perf->archPerfDetailResult.bottomTile2DXBarCC);
    archInfo("xBarCC is %f.\n",perf->archPerfDetailResult.xBarCC);
    archInfo("cacheControllerCC is %f.\n",perf->archPerfDetailResult.cacheControllerCC);
    archInfo("overHeadsCC is %f.\n",perf->archPerfDetailResult.overHeadsCC);
    archInfo("overallCC is %f.\n",perf->archPerfDetailResult.overAllCC);
    archInfo("cyclesTile0Vzgroup0 is %f.\n",perf->archPerfDetailResult.cyclesTile0Vzgroup0);
    archInfo("cyclesTile0RestVzgroup0 is %f.\n",perf->archPerfDetailResult.cyclesTile0RestVzgroup0);
    archInfo("cyclesRestTileVzgroup0 is %f.\n",perf->archPerfDetailResult.cyclesRestTileVzgroup0);
    archInfo("cyclesRestTileRestVzgroup0 is %f.\n",perf->archPerfDetailResult.cyclesRestTileRestVzgroup0);
    archInfo("BottleneckTile0Vzgroup0 is %f.\n",perf->archPerfDetailResult.BottleneckTile0Vzgroup0);
    archInfo("BottleneckTile0RestVzgroup0 is %f.\n",perf->archPerfDetailResult.BottleneckTile0RestVzgroup0);
    archInfo("BottleneckRestTileVzgroup0 is %f.\n",perf->archPerfDetailResult.BottleneckRestTileVzgroup0);
    archInfo("BottleneckRestTileRestVzgroup0 is %f.\n",perf->archPerfDetailResult.BottleneckRestTileRestVzgroup0);
}
#endif

/***********************************************************************************
* Function:     showArchPerformanceLib
* Description:  Show the performance result after predict
* Input:        :
*               :
* Ouput:
***************************************************************************************/
arch_status showArchPerformanceLib(
    archHAL_CHIPIDENTITY *pChipDentity,
    arch_nn_config *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archNN_DATABASE_FEATURE *pArchDataFeature,
    archModelOpInfo **opInfo,
    arch_uint32 index,
    arch_perf perf
    )
{
    arch_uint32 i;
    arch_uint32 profileMode = 0;
    arch_float32 mutiGpuFactor = (pArchNnConfig->fixedFeature.vipCoreCount > 1) ? (0.7f * pArchNnConfig->fixedFeature.vipCoreCount) : 1.0f;

    if (gOrigShowType != (arch_int8)pArchOptions->collectPerfType)
    {
        archInfo("\nArchModelVersion: %s\nSWTilingVersion: %s\nProfileMode: %d\n"
                "chipModel: 0x%x\nchipRevision: 0x%x\nproductID: 0x%x\ncustomerID: 0x%x\necoID: 0x%x\n"
               "NumNNCores:%d\nNumNNCoresInt8: %d\nNumNNCoresInt16: %d\nNumNNCoresFloat16: %d\n"
               "NumTPCores: %d\nNumTPLiteCores: %d\nMadPerCore: %d\nVIP7Version: %d\n"
               "InBuffDepth: %d\nAccumBufferDepth: %d\nDPAmount: %d\n"
               "XYDPX: %d\nXYDPY: %d\nZDP: %d\n"
               "ZDP3Enable: %d\nZDP6Enable: %d\n"
               "AXISRAMSize: %d\nVIPSRAMSize: %d\nL2CacheWidth: %d\n"
               "USCCacheSize: %d\nBrickMode: %d\nSWTiling: %d\n"
               "SmallBatchEnable: %d\nSWTilingPhase1: %d\nTPWithFCLayer: %d\n"
               "TPCircularBufferSupport: %d\nKERNEL_HEADER_NOT_CACHED_FIX: %d\n"
               "NNFCNonPruneAccel: %d\nConv1x1HalfPerformance: %d\n"
               "DDRLatency: %d\nCacheLineModeDisabled: %d\n"
               "PER_3D_TILE_BUBBLE_FIX: %d\nSWConv1x1To1x2: %d\n"
               "TP_LOCALIZATION_REORDER_DISABLED_Fix: %d\nUSCCacheControllers: %d\n"
               "AsyncCopyPerfFix: %d\nZDP3NoCompressFix: %d\n"
               "ZXDP3KernelReadConflictFix: %d\nxyOffsetLimitationFix: %d\n",
               archModelVersion,
               SWTilingVersion,
               (pArchOptions->collectPerfType == COLLECT_PERF_ESTIMATE) ? 1 : profileMode,
               pChipDentity->chipModel,
               pChipDentity->chipRevision,
               pChipDentity->productID,
               pChipDentity->customerID,
               pChipDentity->ecoID,
               pArchNnConfig->fixedFeature.nnCoreCount,
               pArchNnConfig->fixedFeature.nnCoreCountInt8,
               pArchNnConfig->fixedFeature.nnCoreCountInt16,
               pArchNnConfig->fixedFeature.nnCoreCountFloat16,
               pArchNnConfig->fixedFeature.tpCoreCount,
               pArchNnConfig->fixedFeature.tpliteCoreCount,
               pArchNnConfig->fixedFeature.nnMadPerCore,
               pArchNnConfig->fixedFeature.vip7Version,
               pArchNnConfig->fixedFeature.nnInputBufferDepth,
               pArchNnConfig->fixedFeature.nnAccumBufferDepth,
               pArchNnConfig->derivedFeature.nnDPAmount,
               pArchNnConfig->derivedFeature.nnXYDPX,
               pArchNnConfig->derivedFeature.nnXYDPY,
               pArchNnConfig->derivedFeature.nnZDP,
               archIsFeatureAvailable(pArchNnConfig,pArchOptions, pArchDataFeature,ARCH_NN_FEATURE_ZDP3),
               archIsFeatureAvailable(pArchNnConfig,pArchOptions, pArchDataFeature,ARCH_NN_FEATURE_ZDP6),
               pArchNnConfig->customizedFeature.axiSRAMSize,
               pArchNnConfig->customizedFeature.vipSRAMSize,
               pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte,
               pArchNnConfig->unifiedFeature.nnUSCCacheSize,
               pArchNnConfig->fixedFeature.vipBrickMode,
               pArchNnConfig->customizedFeature.vipSWTiling,
               pArchNnConfig->unifiedFeature.smallBatchEnable, /*smallBatchEnable*/
               archIsFeatureAvailable(pArchNnConfig,pArchOptions,pArchDataFeature, ARCH_NN_FEATURE_SWTILING_PHASE1) ? 1 : 0,
               1, /*TPWithFCLayer*/
               archIsFeatureAvailable(pArchNnConfig,pArchOptions, pArchDataFeature,ARCH_NN_FEATURE_SWTILING_PHASE2) ? 1 : 0,
               pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix,/*KERNEL_HEADER_NOT_CACHED_BUG1968*/
               0,  /*NNFCNonPruneAccel*/
               pArchNnConfig->unifiedFeature.conv1x1HalfPerformance,
               (arch_uint32)pArchNnConfig->customizedFeature.ddrLatency,
               pArchNnConfig->unifiedFeature.cacheLineModeDisabled,
               pArchNnConfig->unifiedFeature.per3DTileBubbleFix,
               0, /*swConv1x1To1x2*/
               pArchNnConfig->unifiedFeature.tpReOrderFix,
               pArchNnConfig->fixedFeature.uscCacheControllers,
               pArchNnConfig->unifiedFeature.asyncCopyPerfFix,
               pArchNnConfig->unifiedFeature.zdp3NoCompressFix,
               pArchNnConfig->unifiedFeature.zxdp3KernelReadConflictFix,
               pArchNnConfig->unifiedFeature.xyOffsetLimitationFix
               );

        archInfo("CoefDecodePerf: %d\nVectorPrune: %d\nEnableCacheDataFromSRAM: %d\n"
               "IMAGE_PARTIAL_CACHE_FIX: %d\nDDRReadBandWidthLimit: %.2f\n"
               "DDRWriteBandWidthLimit: %.2f\nDDRTotalBandWidthLimit: %.2f\n"
               "AXISRAMReadBandWidthLimit: %.2f\nAXISRAMWriteBandWidthLimit: %.2f\n"
               "AXISRAMTotalBandWidthLimit: %.2f\nAXIBusReadBandWidthLimit: %.2f\n"
               "AXIBusWriteBandWidthLimit: %.2f\nAXIBusTotalBandWidthLimit: %.2f\n\n",
               pArchNnConfig->unifiedFeature.vipCoefDecodePerf,
               pArchNnConfig->customizedFeature.vipVectorPrune,
               pArchNnConfig->unifiedFeature.vipCachedReadFromSram,
               pArchNnConfig->unifiedFeature.vipImagePartialCache,
               pArchNnConfig->customizedFeature.ddrReadBWLimit,
               pArchNnConfig->customizedFeature.ddrWriteBWLimit,
               pArchNnConfig->customizedFeature.ddrTotalBWLimit,
               pArchNnConfig->customizedFeature.axiSramReadBWLimit,
               pArchNnConfig->customizedFeature.axiSramWriteBWLimit,
               pArchNnConfig->customizedFeature.axiSramTotalBWLimit,
               pArchNnConfig->customizedFeature.axiBusReadBWLimit,
               pArchNnConfig->customizedFeature.axiBusWriteBWLimit,
               pArchNnConfig->customizedFeature.axiBusTotalBWLimit);

        archInfo("HANDLE_ABBUFFER: %d\nHANDLE_SUBIMAGE: %d\nHANDLE_BRANCH: %d\n\n",
               (pArchOptions->enableSwtilingPhase1 == ARCH_SWTILING_OPTION_ALL || pArchOptions->enableSwtilingPhase1 == ARCH_SWTILING_OPTION_AB) ? 1 : 0,
               (pArchOptions->enableSwtilingPhase1 == ARCH_SWTILING_OPTION_ALL || pArchOptions->enableSwtilingPhase1 == ARCH_SWTILING_OPTION_TILING) ? 1 : 0,
               (pArchOptions->collectPerfType == 1) ? 1 : pArchOptions->enableHandleBranch);

        archInfo("FreqInMHZ: %u\nAxiClockFreqInMHZ: %u\nOutstandingTransfer: %d\nInternalWriteBWLimit: %.2f\n\n",
               pArchNnConfig->customizedFeature.freqInMHZ,
               pArchNnConfig->customizedFeature.axiClockFreqInMHZ,
               pArchNnConfig->fixedFeature.maxOTNumber,
               (arch_float32)pArchNnConfig->fixedFeature.nnLanesPerOutCycle);

        archInfo("LanesPerConv: %u\nMaxTileSize: %u\nAxiSramSlowedDownByAddr: %d\nSLOW_NN_REQ_ARBITRATION_FIX: %d\n\n",
               pArchNnConfig->unifiedFeature.lanesPerConv,
               pArchNnConfig->unifiedFeature.maxTileSize,
               pArchNnConfig->unifiedFeature.axiSramSlowedDownByAddr,
               pArchNnConfig->unifiedFeature.slowNNReqArbitrationFix);

        archInfo("FLOAT_XYDP_X: %u\nFLOAT_XYDP_Y: %u\nFLOAT_ZDP: %d\n",
               pArchNnConfig->fixedFeature.nnFP16XYDPX,
               pArchNnConfig->fixedFeature.nnFP16XYDPY,
               pArchNnConfig->fixedFeature.nnFP16ZDP);

        archInfo("SINGLE_PORT_ACC_BUFFER: %d\nMAX_ZRL_BIT_WIDTH: %d\nMAX_SOC_OUT_STANDING_NUMBER: %d\n\n",
            pArchNnConfig->unifiedFeature.singlePortAccBuffer,
            pArchNnConfig->fixedFeature.zrlBits,
            pArchNnConfig->customizedFeature.maxSocOTNumber
            );

        archInfo("SWTilingPhase3: %d\nAXI_SRAM_ONLY_SW_TILING: %d\n",
            archIsFeatureAvailable(pArchNnConfig,pArchOptions,pArchDataFeature, ARCH_NN_FEATURE_SWTILING_PHASE3) ? 1 : 0,
            pArchNnConfig->unifiedFeature.axiSramOnlySWTiling
            );

        archInfo("VIP_CORE_COUNT: %d\n",
            pArchNnConfig->fixedFeature.vipCoreCount
            );

        archInfo("DEPTH_WISE_SUPPORT: %d\nDEPTH_WISE_MERGE_SUPPORT: %d\nNN_WRITE_WITHOUT_USC: %d\n",
            pArchNnConfig->customizedFeature.depthWiseSupport,
            pArchDataFeature->depthWiseMergeSupport,
            pArchNnConfig->customizedFeature.nnWriteWithoutUSC
            );

        /* Add log output */
        archInfo("DDR_ALIGN: %d\nIN_LINES_PER_CYCLE: %d\n",
            pArchDataFeature->ddrAlign,
            pArchDataFeature->inlinesPerCycle
            );

        archInfo("NN_SLOW_OUTPUT: %d\nNO_NARROW_POST_PROCESS_PIPE: %d\nNN_SMALLBATCH_PHASE1: %d\n",
            pArchDataFeature->nnSlowOutput,
            pArchDataFeature->noNarrowPostProcessPipe,
            pArchDataFeature->prefetchNNCommandKernelHeader
            );

        archInfo("EQUIVALENT_VIP_SRAM_WIDTH_IN_BYTE: %d\n",
            pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte
            );

        archInfo("IMAGE_NOT_PACKED_IN_SRAM: %d\n",
            pArchNnConfig->unifiedFeature.imageNotPackedInSram
            );

        archInfo("NN_COEF_COMPRESSION_ENHANCEMENT: %d\nTP_COMPRESSION_ENHANCEMENT: %d\n",
            archIsFeatureAvailable(pArchNnConfig,pArchOptions, pArchDataFeature,ARCH_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT),
            archIsFeatureAvailable(pArchNnConfig,pArchOptions, pArchDataFeature,ARCH_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT)
            );

        archInfo("COEF_DELTA_CORD_OVER_FLOW_ZRL_8BIT_FIX: %d\n",
            pArchNnConfig->unifiedFeature.coefDeltaCordOverFlowZRL8BitFix
            );

        archInfo("NumShaderCores: %d\n",
            pArchNnConfig->fixedFeature.shaderCoreCount
            );

        archInfo("KERNEL_PER_CORE_LESS_THAN_THIRD_COEF_BUFF_DEPTH_FIX: %d\n",
            pArchNnConfig->unifiedFeature.kernelPerCoreLTOneThirdCoefFix
            );

        archInfo("LOW_EFFICIENCY_OF_ID_WRITE_IMGBUF_FIX: %d\n",
            pArchNnConfig->unifiedFeature.lowEfficiencyOfIDWriteImgBufFix
            );

        /* Add log output for fix */
        archInfo("NN_KERNEL_SIZE_WASTE_IN_PARTIAL_MODE_FIX: %d\n",
            pArchDataFeature->partialKernelCacheInternalFix
            );

        archInfo("KERNEL_VIP_SRAM_READ_BW_LIMITATION_FIX: %d\n",
            pArchDataFeature->internalKernelReadBottleneckFix
            );

        archInfo("IMG_POP_PIPELINE_PAUSE_FIX: %d\n",
            pArchDataFeature->ImgPopPipelinePauseFix
            );

        archInfo("FULL_CACHE_INTERVAL_FIX: %d\n",
            pArchDataFeature->fullCacheIntervalFix
            );

        archInfo("NN_Transpose: %d\nSPECIFIED_DDR_BW_LIMIT_BY_BURST: %d\n",
            pArchDataFeature->nnTranspose, pArchDataFeature->specificDDRLimitByBurst
            );

        archInfo("DR_JD_Diff_For_Cacheline_Mode_Fix: %d\n",
            pArchDataFeature->drJdDiffConditionForCacheLineModePreFix
            );

        /* DDR Read Sustained BW Burst */
        archInfo("DDR_READ_SUSTAINED_BW_64B_BURST: %f\nDDR_READ_SUSTAINED_BW_128B_BURST: %f\nDDR_READ_SUSTAINED_BW_256B_BURST: %f\n",
            pArchDataFeature->ddrReadSustainedBw64BBurst,
            pArchDataFeature->ddrReadSustainedBw128BBurst,
            pArchDataFeature->ddrReadSustainedBw256BBurst
            );
        archInfo("DDR_WRITE_SUSTAINED_BW_64B_MASK_BURST: %f\nDDR_WRITE_SUSTAINED_BW_128B_MASK_BURST: %f\nDDR_WRITE_SUSTAINED_BW_256B_MASK_BURST: %f\n",
            pArchDataFeature->ddrMaskWriteSustainedBw64BBurst,
            pArchDataFeature->ddrMaskWriteSustainedBw128BBurst,
            pArchDataFeature->ddrMaskWriteSustainedBw256BBurst
            );
        archInfo("DDR_WRITE_SUSTAINED_BW_64B_NONMASK_BURST: %f\nDDR_WRITE_SUSTAINED_BW_128B_NONMASK_BURST: %f\nDDR_WRITE_SUSTAINED_BW_256B_NONMASK_BURST: %f\n",
            pArchDataFeature->ddrNonMaskWriteSustainedBw64BBurst,
            pArchDataFeature->ddrNonMaskWriteSustainedBw128BBurst,
            pArchDataFeature->ddrNonMaskWriteSustainedBw256BBurst
            );

        /* new configration */
        archInfo("VIPSRAM_ASYNC_FIFO: %d\nreadReturnArbiterBubbleFix: %d\nnerghborImageDataTransferNotEfficientFix: %d\ntpVipSramOt1Fix: %d\n",
            pArchDataFeature->vipSramAsyncFifo,
            pArchDataFeature->readReturnArbiterBubbleFix,          /* 2038 */
            pArchDataFeature->nerghborImageDataTransferNotEfficientFix,          /* 2045 */
            pArchDataFeature->tpVipSramOt1Fix           /* 2050 */
            );

        /* Burst size */
        archInfo("NN_DDR_BURST_SIZE: %d\nNN_LARGE_BURST_SIZE: %d\nTP_COMP_2PIXEL_PER_CYCLE: %d\n",
            pArchDataFeature->nnDDRBurstSize,
            pArchDataFeature->nnLargeBurstSize,
            pArchDataFeature->tpComp2pixelPerCycle
            );

        archInfo("\n");
        gOrigShowType = (arch_int8)pArchOptions->collectPerfType;
    }

    archInfo("\n");
    archInfo("===========================\n");
    archInfo("**********Show Perf********\n");
    archInfo("===========================\n");

    archInfo("layer_id:%d layer_name:%s\noperation_id:%d operation_name:%s operation_target:%s\n",
             opInfo[index]->layerId, opInfo[index]->layerName,
             opInfo[index]->operationId, archnneGetOperatorTypeName(opInfo[index]->op), archnneGetOperatorTargetName(opInfo[index]->target));

    archInfo("abs_op_id:%d\n", opInfo[index]->absoluteOperationID);

    archInfo("upstream_layer_num:%d upstream_opertaion_num:%d\n",
        opInfo[index]->upLayerCount, opInfo[index]->upOpCount);

    for (i = 0; i < opInfo[index]->upOpCount; i++)
    {
        archInfo("%d) upstream_operation_id:%d uptream_operation_name:%s (upstream_layer_id:%d upstream_layer_name:%s)\n",
            i, opInfo[index]->parentOpId[i],
            archnneGetOperatorTypeName((archnne_operator_e)opInfo[index]->parentOpType[i]),
                opInfo[index]->parentLayer[i], archGetLayerName(opInfo[index]->parentLayerType[i]));
    }

    archInfo("downstream_layer_num:%d downstream_opertaion_num:%d\n",
        opInfo[index]->downLayerCount, opInfo[index]->downOpCount);

    for (i = 0; i < opInfo[index]->downOpCount; i++)
    {
        archInfo("%d) downstream_operation_id:%d downstream_operation_name:%s (downstream_layer_id:%d downstream_layer_name:%s)\n",
                i, opInfo[index]->childOpId[i],
                archnneGetOperatorTypeName((archnne_operator_e)opInfo[index]->childOpType[i]),
                opInfo[index]->childLayer[i], archGetLayerName((archnne_operator_e)opInfo[index]->childLayerType[i]));
    }

    if (opInfo[index]->target == ARCHNNE_OPERATION_TARGET_SH ||
        opInfo[index]->target == ARCHNNE_OPERATION_TARGET_SW)
    {
        return ARCH_SUCCESS;
    }

    if (!perf->swTilingInfo.origOutX || !perf->swTilingInfo.origOutY || !perf->swTilingInfo.origOutZ)
    {
        perf->swTilingInfo.origOutX = perf->info.poolingSize > 1 ? (perf->swTilingInfo.origInX - perf->info.poolingSize + perf->info.poolingStride - 1) / perf->info.poolingStride + 1 : perf->swTilingInfo.origInX;
        perf->swTilingInfo.origOutY = perf->info.poolingSize > 1 ? (perf->swTilingInfo.origInY - perf->info.poolingSize + perf->info.poolingStride - 1) / perf->info.poolingStride + 1 : perf->swTilingInfo.origInY;
        perf->swTilingInfo.origOutZ = perf->info.outz;
    }

    if (!perf->info.outx || !perf->info.outy)
    {
        perf->info.outx = perf->info.pix;
        perf->info.outy = perf->info.piy;
    }

    if (perf->opTarget == ARCHNNE_OPERATION_TARGET_NN)
    {
        archInfo("NumUsedNNCores: %d\nConvOutFIFODepth: %d\n\n", perf->info.nnCores, perf->info.convOutFifoDepth);
    }


    if (perf->opType == ARCHNNE_OPERATOR_TENSOR_ADD
        || perf->opTarget == ARCHNNE_OPERATION_TARGET_NN)
    {
        archInfo("OrigInImageX: %d\nOrigInImageY: %d\nOrigInImageZ: %d\nOutImageX: %d (sub: %d)\nOutImageY: %d (sub: %d)\nOutImageZ: %d (sub: %d)\nFinalOutImageX: %d\nFinalOutImageY: %d\nFinalOutImageZ: %d\n",
                    perf->info.oinx, perf->info.oiny, perf->info.oinz,
                    perf->swTilingInfo.origInX, perf->info.inx,
                    perf->swTilingInfo.origInY, perf->info.iny,
                    perf->swTilingInfo.origOutZ, perf->info.outz,
                    perf->swTilingInfo.origOutX,
                    perf->swTilingInfo.origOutY,
                    perf->swTilingInfo.origOutZ);
    }
    else if (perf->opType == ARCHNNE_OPERATOR_POOLING)
    {
        archInfo("OrigInImageX: %d (sub: %d)\nOrigInImageY: %d (sub: %d)\nOrigInImageZ: %d (sub: %d)\nOutImageX: %d\nOutImageY: %d\nOutImageZ: %d\n",
                    perf->swTilingInfo.origInX, perf->info.inx,
                    perf->swTilingInfo.origInY, perf->info.iny,
                    perf->info.oinz, perf->info.outz,
                    perf->swTilingInfo.origOutX,
                    perf->swTilingInfo.origOutY,
                    perf->swTilingInfo.origOutZ);
    }
    else
    {
        archInfo("OrigInImageX: %d\nOrigInImageY: %d\nOrigInImageZ: %d\nOutImageX: %d (sub: %d)\nOutImageY: %d (sub: %d)\nOutImageZ: %d (sub: %d)\n",
                    perf->swTilingInfo.origInX,
                    perf->swTilingInfo.origInY,
                    perf->info.oinz,
                    perf->swTilingInfo.origOutX, perf->info.outx,
                    perf->swTilingInfo.origOutY, perf->info.outy,
                    perf->swTilingInfo.origOutZ, perf->info.outz);
    }

    archInfo("KernelX: %d\nKernelY: %d\nKernelZ: %d\nPoolingSize: %d\nPoolingStride: %d\ninputDataSize: %d\noutputDataSize: %d\nFP16: %d\nstridex: %d\nstridey: %d\n",
            perf->info.kx, perf->info.ky, perf->info.kz,
            perf->info.poolingSize, perf->info.poolingStride,
            perf->info.inputDataSize, perf->info.outputDataSize,perf->info.inputDataFormat == ARCH_TYPE_FLOAT16 ? 1 : 0,
            perf->info.stridex, perf->info.stridey);

    perf->swTilingInfo.archModelKernelSize = _kernel_size_in_pixel_by_vx_arch_perf(perf->opTarget, perf->opType, perf, 0 /*  ID 2033 not done */);
    archInfo("archModel_kernelSize: %u\nkernelSize: %u\n",
        perf->swTilingInfo.archModelKernelSize,
        perf->info.kernelSize
        );

    archInfo("SrcBuf: %s\nDstBuf: %s\nKernelBuf: %s\n",
            !perf->swTilingInfo.srcBuf ? "DDR" : (perf->swTilingInfo.srcBuf == SW_TILING_FROM_AXI_SRAM) ? "AXI_SRAM" : "VIP_SRAM",
            !perf->swTilingInfo.dstBuf ? "DDR" : (perf->swTilingInfo.dstBuf == SW_TILING_FROM_AXI_SRAM) ? "AXI_SRAM" : "VIP_SRAM",
            !perf->swTilingInfo.kernelBuf ? "DDR" : (perf->swTilingInfo.kernelBuf == SW_TILING_FROM_AXI_SRAM) ? "AXI_SRAM" : "VIP_SRAM");

    archInfo("NN_Transpose_Channel_In: %d\nNN_Transpose_Channel_out: %d\n",
            perf->swTilingInfo.trspIvLayerChsIn, perf->swTilingInfo.trspIvLayerChsOut
            );

    if ((pArchOptions->collectPerfType == COLLECT_PERF_ESTIMATE) || profileMode)
    {
         /*_calcArchModelCacheMode(apm,pArchNnConfig, perf, &imageIdealCacheSizeInPixel);*/
         archInfo("imageIdealCacheSizeInPixel: %d\n", perf->swTilingInfo.imageIdealCacheSizeInPixel);
    }

    archInfo("KernelCacheMode=%s\nImageCacheMode=%s\n",
             archnneGetCacheModeName(perf->swTilingInfo.kernelCacheMode),
             archnneGetCacheModeName(perf->swTilingInfo.imageCacheMode));

    archInfo("xOffset: %d, yOffset: %d\n", perf->info.xOffSet, perf->info.yOffSet);

    if (perf->opType == ARCHNNE_OPERATOR_FULLYCONNECTED ||
        perf->opType == ARCHNNE_OPERATOR_TENSOR_ADD ||
        perf->opTarget == ARCHNNE_OPERATION_TARGET_NN)
    {
        /*archInfo("maxPerCoreCompressionRatio: %.15f\n", perf->maxPerCoreCompressionRatio);*/
        archInfo("coefNonZeroRatio: %.15f\ncoefCompression: %.15f\nimageCompression: %.15f\nimageNonZeroRatio: %.15f\n\n",
                 perf->coefNonZeroRatio,
                 perf->coefCompressRatio,
                 perf->imageCompressRatio,
                 perf->imageNonZeroRatio
                );
        /*archInfo("maxPerCoreCompressionRatio__llu: %llu\n", *(arch_uint64 *)&perf->maxPerCoreCompressionRatio);*/
        archInfo("coefNonZeroRatio__llu: %llu\ncoefCompression_llu: %llu\nimageCompression_llu: %llu\nimageNonZeroRatio_llu: %llu\n\n",
                 *(arch_uint64 *)&perf->coefNonZeroRatio,
                 *(arch_uint64 *)&perf->coefCompressRatio,
                 *(arch_uint64 *)&perf->imageCompressRatio,
                 *(arch_uint64 *)&perf->imageNonZeroRatio
                );

        if (perf->opTarget == ARCHNNE_OPERATION_TARGET_NN
            ||perf->opType == ARCHNNE_OPERATOR_TENSOR_ADD)
        {
            archInfo("OutImageTileXSize: %d\nOutImageTileYSize: %d\nKernelsPerCore: %d\n\n",
                     perf->resultInfo.outImageTileXSize,
                     perf->resultInfo.outImageTileYSize,
                     perf->resultInfo.kernelsPerCore
                  );
        }
    }
    else
    {
        archInfo("\n");
    }

    archInfo("kernelDDRReadBW: %llu\nInImageDDrReadBW: %llu\n",
             (arch_uint64)(perf->resultInfo.perfKernelReadBandWidth + 0.5f),
             (arch_uint64)(perf->resultInfo.perfInImageReadBandWidth + 0.5f));

    archInfo("ReadBW: %llu\nWriteBW: %llu\nCycleCount: %llu\n\n",
             (arch_uint64)(perf->resultInfo.perfReadBandWidth + 0.5f),
             (arch_uint64)(perf->resultInfo.perfWriteBandWidth + 0.5f),
             (arch_uint64)(perf->resultInfo.perfCycleCount / mutiGpuFactor + 0.5f));
#ifdef DUMP_DETAIL
    printArchDetail(perf);
#endif
    return ARCH_SUCCESS;
}


/***********************************************************************************
* Function:        _cur_cost_u64_is_more_better
* Description:    Check if the new cost result is better
* Input:        :
*                :
* Ouput:
***************************************************************************************/
static arch_bool _cur_cost_u64_is_more_better(struct _archModelCost_u64 *cost, struct _archModelCost_u64 *cur)
{
    arch_float64 f = -(1.0f * (arch_int64)(cur->cycle - cost->cycle) / archMAX(cur->cycle, cost->cycle) * 20 + 1.0f * (arch_int64)(cur->bw - cost->bw) / archMAX(cur->bw, cost->bw) * 1);
    if (f > 0) return arch_true_e;
    return arch_false_e;
}

/***********************************************************************************
* Function:       calculateArchPerf
* Description:    Claculate the Arch Model Performance for one Op. Will Loop all OutTileX/OutTileY/KernelsPerCore
* Input:        :
*               :
* Ouput:
***************************************************************************************/
arch_status archCalculateArchPerf(
    APMHandle        apm,
    arch_nn_config  *pArchNnConfig,
    arch_drv_option *pArchOptions,
    archNN_DATABASE_FEATURE *pArchDataFeature,
    arch_perf        perf,
    archnne_operation_target_e op_target,
    archnne_operator_e op_type
    )
{
    /* version 0.29 - 0.50.5 */
    arch_uint32 numCores, lanesPerConv, accuBuffDepth, adjustAccuBuffDepth, inputBuffDepth, inputBuffDepthForOneTile, l2CacheSize, brickMode, swTiling, vip7Version;
    arch_uint32 inXSize, inYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, poolingSize, poolingStride, inputDataSize, outputDataSize;
    arch_int32 xOffSet, yOffSet;
    arch_uint32 x = 0, y = 0, k = 0;
    arch_uint32 tmpMaxOutTileYSize, tmpCovAccuMode, tmpCovMode = 0, interleaveMode = 0, cacheLineMode, maxInTileXSize, maxOutTileXSize, minOutTileXSize, minOutTileYSize;
    arch_bool initMinCost = arch_false_e;
    struct _archModelCost_u64 minCost = {0}, curCost = {0};
    arch_float64 newCycleCount = 0;
    arch_uint64  minCycleCount;
    arch_float64 newRDBandWidth = 0, newWTBandWidth = 0;
    arch_uint64 minRDBandWidth, minWTBandWidth;
    /*arch_uint64 minNCRDBandWidth = 0,newNCRDBandWidth = 0; */
    arch_float64 ddrKernelReadBandWidth = 0, ddrInImageReadBandWidth = 0, ddrRDBandWidth = 0, ddrWTBandWidth = 0,  axiRDBandWidth = 0, axiWTBandWidth = 0;
    arch_float64 coefNonZeroRatio=0.0f, coefCompressRatio=0.0f, imageCompressRatio=0.0f, imageNonZeroRatio=0.0f;
    arch_bool vip7_16bit, interleave8, fp16;
    arch_uint32 srcBuf, dstBuf, kernelBuf, vectorPrune;
    arch_uint32 convOutFifoDepth = 0;
    arch_bool isComputeBottleNeck = arch_false_e;

    /*
    arch_uint32 tpCores,l2CacheWidth,uscCacheSize,nnCmdSizeInBytes, tpCmdSizeInBytes;
    arch_float32 axiSramReadBWLimit, axiSramWriteBWLimit, axiSramTotalBWLimit, axiBusReadBWLimit, axiBusWriteBWLimit, axiBusTotalBWLimit;
    arch_uint32 imagePartialCache, srcBuf, dstBuf, kernelBuf, cachedReadFromSram, vectorPrune, coefDecodePerf;
    arch_bool per3DTileBubbleFix = pArchNnConfig->unifiedFeature.per3DTileBubbleFix ? arch_true_e : arch_false_e;
    arch_bool low_efficiency_of_id_write_imagebuf_fix = pArchNnConfig->unifiedFeature.lowEfficiencyOfIDWriteImgBufFix ? arch_true_e : arch_false_e;
    arch_bool accurateTileBW = pArchNnConfig->unifiedFeature.accurateTileBW ? arch_true_e : arch_false_e;
    arch_bool zdp3NoCompressFix = pArchNnConfig->unifiedFeature.zdp3NoCompressFix ? arch_true_e : arch_false_e;
    arch_float32 ddrLatency = pArchNnConfig->customizedFeature.ddrLatency;
    arch_bool cacheLineModeDisabled = pArchNnConfig->unifiedFeature.cacheLineModeDisabled ? arch_true_e : arch_false_e;
    arch_bool fullCacheKernelHeadFix = pArchNnConfig->unifiedFeature.fullCacheKernelHeadFix ? arch_true_e : arch_false_e;
    arch_bool conv1x1HalfPerformance = pArchNnConfig->unifiedFeature.conv1x1HalfPerformance ? arch_true_e : arch_false_e;
    arch_bool asyncCopyPerfFix = pArchNnConfig->unifiedFeature.asyncCopyPerfFix ? arch_true_e : arch_false_e;
    arch_bool zxdp3KernelReadConflictFix = pArchNnConfig->unifiedFeature.zxdp3KernelReadConflictFix ? arch_true_e : arch_false_e;
    arch_uint32 uscCacheControllers = pArchNnConfig->fixedFeature.uscCacheControllers;
    arch_bool axiSramSlowedDownByAddr = pArchNnConfig->unifiedFeature.axiSramSlowedDownByAddr ? arch_true_e : arch_false_e;
    arch_uint32 zrlBits = pArchNnConfig->fixedFeature.zrlBits;
    arch_bool tpReOrderFix = pArchNnConfig->unifiedFeature.tpReOrderFix ? arch_true_e : arch_false_e;
    arch_bool slowNNReqArbitrationFix = pArchNnConfig->unifiedFeature.slowNNReqArbitrationFix ? arch_true_e : arch_false_e;
    arch_uint32 axiAccUnitSizeInByte = 64;
    arch_uint32 outstandingTransfer = pArchNnConfig->fixedFeature.maxOTNumber;
    arch_bool singlePortAccBuffer = pArchNnConfig->unifiedFeature.singlePortAccBuffer ? arch_true_e : arch_false_e;
    arch_float32 totalLatency = pArchNnConfig->derivedFeature.totalLatency;
    arch_bool imageNotPackedInSram = pArchNnConfig->unifiedFeature.imageNotPackedInSram ? arch_true_e : arch_false_e;
    arch_bool isNNWriteWithoutUSC = pArchNnConfig->customizedFeature.nnWriteWithoutUSC ? arch_true_e : arch_false_e;
    arch_float32 ddrReadBWInBytePerCycle = 0, ddrWriteBWInBytePerCycle, ddrTotalBWInBytePerCycle, internalWriteBWLimit;
    */

    arch_bool   isDepthWise = (arch_bool)(pArchNnConfig->customizedFeature.depthWiseSupport && (op_type == ARCHNNE_OPERATOR_DEPTH_WISE_CONV));
    arch_bool   isDepthWiseMerge = (arch_bool)(pArchDataFeature->depthWiseMergeSupport);
    arch_bool   asymmetricQuantization = (perf->info.inputDataFormat == ARCH_TYPE_UINT8)?1:0;
    arch_bool   isV8 = (pArchNnConfig->derivedFeature.nnXYDPX == 0 && pArchNnConfig->derivedFeature.nnXYDPX == 0) ? arch_true_e : arch_false_e;
    arch_uint32 inSIXRefined   = 0, inSIYRefined  = 0;
    arch_uint32 inImageStride  = 0, inImageSlice  = 0;
    arch_uint32 outImageStride = 0, outImageSlice = 0;

    /*arch_uint32 vipSramAccUnitSizeInByte;*/
    arch_uint32 zdp = pArchNnConfig->derivedFeature.nnZDP;
    arch_uint32 equivalentVipSramWidthInByte = pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte;
    arch_bool   kernelPerCoreLTOneThirdCoefFix = pArchNnConfig->unifiedFeature.kernelPerCoreLTOneThirdCoefFix ? arch_true_e : arch_false_e;

    APM_IN_PERF_PARAMS inPerfParams;
    APM_OUT_BW_T       outBandWidth = {0};
    APM_OUT_RESULT_T   outResult = {0};

    perf->info.poolingStride = !perf->info.poolingSize ? 1 : archMAX(1, perf->info.poolingStride);
    perf->info.poolingSize = archMAX(1, perf->info.poolingSize);
    assert(apm != NULL);
    assert(perf->info.kx && perf->info.inx && perf->info.inputDataSize);

    unsigned int flush_and_wait_first_cmd = 0;
    unsigned int flush_and_wait_nonfirst_cmd = 0;

    if (perf->info.flush == 0)
    {
        flush_and_wait_first_cmd = 0;
        flush_and_wait_nonfirst_cmd = 0;
    }
    else if (perf->info.flush == 1)
    {
        flush_and_wait_first_cmd = 1;
        flush_and_wait_nonfirst_cmd = 1;
    }
    else if (perf->info.flush == 2)
    {
        flush_and_wait_first_cmd = 1;
        flush_and_wait_nonfirst_cmd = 0;
    }
    else
    {
        assert (0 && " 'flush_and_wait can only be values of (0, 1, 2)");
    }

    if (!perf->calculated)
    {
        /********** input/output/wb configuration **********/
        kernelXSize   = perf->info.kx;
        kernelYSize   = perf->info.ky;
        kernelZSize   = perf->info.kz;
        inXSize       = perf->info.inx;
        inYSize       = perf->info.iny;
        inputDataSize = perf->info.inputDataSize;
        outputDataSize= perf->info.outputDataSize;
        poolingSize   = perf->info.poolingSize;
        poolingStride = perf->info.poolingStride;
        xOffSet       = perf->info.xOffSet;
        yOffSet       = perf->info.yOffSet;

        /********** HW configuration **********/
        if (perf->info.inputDataFormat == ARCH_TYPE_FLOAT16)
        {
            numCores = pArchNnConfig->fixedFeature.nnCoreCountFloat16;
        }
        else if (perf->info.inputDataFormat == ARCH_TYPE_INT16)
        {
            numCores = pArchNnConfig->fixedFeature.nnCoreCountInt16;
        }
        else
        {
            numCores = pArchNnConfig->fixedFeature.nnCoreCount;
        }
        if (numCores == 0 && (op_target == ARCHNNE_OPERATION_TARGET_NN))
        {
            archError("ERROR: not support input data format: %d\n", perf->info.inputDataFormat);
            assert(0);
            return ARCH_FAILURE;
        }

        if (pArchNnConfig->unifiedFeature.convOutFifoDepthFix)
        {
            convOutFifoDepth = (arch_uint32)ceilf(1.0f * pArchNnConfig->fixedFeature.nnAccumBufferDepth * numCores * (64 - 8) / 64);
        }
        else
        {
            convOutFifoDepth = pArchNnConfig->fixedFeature.nnAccumBufferDepth;
        }
        perf->info.convOutFifoDepth = convOutFifoDepth;
        perf->info.nnCores = numCores;
        /*
        tpCores = op_type != ARCHNNE_OPERATOR_FULLYCONNECTED ?
            pArchNnConfig->fixedFeature.tpCoreCount : pArchNnConfig->fixedFeature.tpCoreCount + pArchNnConfig->fixedFeature.tpliteCoreCount;
        */
        lanesPerConv    = pArchNnConfig->unifiedFeature.lanesPerConv;
        maxInTileXSize  = pArchNnConfig->unifiedFeature.maxTileSize + 8;
        maxOutTileXSize = pArchNnConfig->unifiedFeature.maxTileSize;
        inputBuffDepth  = pArchNnConfig->fixedFeature.nnInputBufferDepth;
        if ((pArchNnConfig->derivedFeature.nnXYDPY >= 3) &&
            (inputDataSize == 8) &&
            (kernelXSize != 1 && kernelYSize != 1))
        {
            /* XYDP9. */
            inputBuffDepthForOneTile = inputBuffDepth / 2;
        }
        else if ((pArchNnConfig->derivedFeature.nnZDP >= 6) &&
                 (kernelXSize == 1 && kernelYSize == 1))
        {
            /* ZDP6. */
            inputBuffDepthForOneTile = 32; /* A decent size. */
        }
        else
        {
            inputBuffDepthForOneTile = inputBuffDepth;
        }
        accuBuffDepth  = pArchNnConfig->fixedFeature.nnAccumBufferDepth;
        assert(perf->info.inputDataFormat != ARCH_TYPE_INVALID);
        if (perf->info.inputDataFormat == ARCH_TYPE_FLOAT16)
        {
            fp16 = arch_true_e;
        }
        else
        {
            fp16 = arch_false_e;
        }

        if (fp16 && isV8)
        {
            adjustAccuBuffDepth = accuBuffDepth / 16;
        }
        else if (!fp16 && inputDataSize == 16)
        {
            adjustAccuBuffDepth = isV8 ? (accuBuffDepth / 4) : (accuBuffDepth / 2);
        }
        else
        {
            adjustAccuBuffDepth = accuBuffDepth;
        }

        /* fix me, when runing 0xae, vip7Version is 1 */
        vip7Version    = pArchNnConfig->fixedFeature.vip7Version;
        /*
        uscCacheSize       = pArchNnConfig->unifiedFeature.nnUSCCacheSize;
        l2CacheWidth       = pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte;
        */
        /*vipSramAccUnitSizeInByte = l2CacheWidth * 2;*/ /* x2 because we are using half freq SRAM */
        /*
        nnCmdSizeInBytes   = pArchNnConfig->unifiedFeature.nnCmdSizeInBytes;
        tpCmdSizeInBytes   = pArchNnConfig->unifiedFeature.tpCmdSizeInBytes;
        coefDecodePerf     = pArchNnConfig->unifiedFeature.vipCoefDecodePerf;
        cachedReadFromSram = pArchNnConfig->unifiedFeature.vipCachedReadFromSram;
        imagePartialCache  = pArchNnConfig->unifiedFeature.vipImagePartialCache;
        */
        vectorPrune        = pArchNnConfig->customizedFeature.vipVectorPrune;
        brickMode          = pArchNnConfig->fixedFeature.vipBrickMode;

        swTiling = archIsFeatureAvailable(pArchNnConfig,pArchOptions, pArchDataFeature,ARCH_NN_FEATURE_SWTILING_PHASE1) ? arch_true_e : arch_false_e;

        vip7_16bit = (vip7Version && inputDataSize == 16) ? arch_true_e : arch_false_e;
        if (vip7_16bit)
        {
            maxOutTileXSize /= 2;
            maxInTileXSize  /= 2;
        }

        interleave8 = arch_false_e; /*removed by arch perf revision 40*/
        /* refine me: break mode now always 0 */
        pArchNnConfig->fixedFeature.vipBrickMode = brickMode = (op_type != ARCHNNE_OPERATOR_CONVOLUTION && op_type != ARCHNNE_OPERATOR_DEPTH_WISE_CONV) ?
                            0 : archIsFeatureAvailable(pArchNnConfig,pArchOptions,pArchDataFeature, ARCH_NN_FEATURE_BRICK_MODE);

        if (swTiling)
        {
            srcBuf = perf->swTilingInfo.srcBuf;
            dstBuf = perf->swTilingInfo.dstBuf;
            kernelBuf = perf->swTilingInfo.kernelBuf;
        }
        else
        {
            srcBuf = SW_TILING_FROM_DDR;
            dstBuf = SW_TILING_FROM_DDR;
            kernelBuf = SW_TILING_FROM_DDR;
        }

        /*
        ddrReadBWInBytePerCycle = pArchNnConfig->derivedFeature.ddrReadBWInBytePerCycle;
        ddrWriteBWInBytePerCycle = pArchNnConfig->derivedFeature.ddrWriteBWInBytePerCycle;
        ddrTotalBWInBytePerCycle = pArchNnConfig->customizedFeature.ddrTotalBWLimit;
        internalWriteBWLimit = (arch_float32)pArchNnConfig->fixedFeature.nnLanesPerOutCycle;
        axiSramReadBWLimit  = pArchNnConfig->customizedFeature.axiSramReadBWLimit;
        axiSramWriteBWLimit = pArchNnConfig->customizedFeature.axiSramWriteBWLimit;
        axiSramTotalBWLimit = pArchNnConfig->customizedFeature.axiSramTotalBWLimit;
        axiBusReadBWLimit  = pArchNnConfig->customizedFeature.axiBusReadBWLimit;
        axiBusWriteBWLimit = pArchNnConfig->customizedFeature.axiBusWriteBWLimit;
        axiBusTotalBWLimit = pArchNnConfig->customizedFeature.axiBusTotalBWLimit;
        */

        l2CacheSize  = (perf->swTilingInfo.cacheSpace == -1) ?
                       ARCH_VIP_SRAM_SIZE : perf->swTilingInfo.cacheSpace;

        outZSize = perf->info.outz;
        {
            coefNonZeroRatio   = perf->coefNonZeroRatio;
            coefCompressRatio  = perf->coefCompressRatio;
            imageCompressRatio = perf->imageCompressRatio;
            imageNonZeroRatio  = perf->imageNonZeroRatio;

            if (op_target == ARCHNNE_OPERATION_TARGET_NN)
            {
                /* init to default */
                arch_uint32 tmpInX = perf->swTilingInfo.origInX + perf->info.kx - 1 + 2 * xOffSet;
                arch_uint32 tmpInY = perf->swTilingInfo.origInY + perf->info.ky - 1 + 2 * yOffSet;
                tmpInY = (tmpInY == 0) ? -yOffSet : tmpInY;
                inSIXRefined = archMIN(tmpInX, perf->info.inx + perf->info.kx - 1);
                inSIYRefined = archMIN(tmpInY, perf->info.iny + perf->info.ky - 1);
                inImageStride = tmpInX;
                inImageSlice = tmpInX * tmpInY;

                /* Merge from CL 213845 */
                if(srcBuf == SW_TILING_FROM_DDR)
                {
                    /* Merge from CL 213817 */
                    if(pArchDataFeature->ddrAlign == 1
                        && (perf->upStreamLayerCount == 0 || perf->allSibling1x1 == 1)
                        && (perf->info.kx == 1)
                        && (perf->info.ky == 1)
                        && (perf->trspIvLayerChsIn == 1))
                    {
                        while(inImageSlice % NN_DDR_BURST_SIZE != 0)
                        {
                            inImageSlice = inImageSlice + inImageStride;
                        }
                    }
                }
                else
                {
#ifdef ALIGNMENT_64B
                    if(gAlign64BExceted == 1)
                    {
                        if(srcBuf == SW_TILING_FROM_VIP_SRAM && perf->inputAlignment == 1)
                        {
                            perf->vipSramAccUnitSizeInByte = equivalentVipSramWidthInByte * 2;
                            if (pArchNnConfig->unifiedFeature.HW_Alignment)
                            {
                                inImageSlice = (arch_uint32)ceilf((float)inImageSlice / perf->vipSramAccUnitSizeInByte) * perf->vipSramAccUnitSizeInByte;
                            }
                            else if (pArchNnConfig->unifiedFeature.SW_Alignment)
                            {
                                while(inImageSlice % perf->vipSramAccUnitSizeInByte != 0)
                                {
                                    inImageSlice = inImageSlice + inImageStride;
                                }
                            }
                        }
                    }
#endif
                }
                if (srcBuf != SW_TILING_FROM_DDR)
                {
                    inImageStride = inSIXRefined;
                    inImageSlice = inSIXRefined * inSIYRefined;
                }
                if (swTiling && perf->swTilingInfo.outImageStride != 0)
                {
                    outImageStride =  perf->swTilingInfo.outImageStride;
                    outImageSlice = perf->swTilingInfo.outImageSlice;
                }
                else
                {
                    if (dstBuf != SW_TILING_FROM_DDR)
                    {
                        outImageStride = (arch_uint32)ceilf((arch_float32)(perf->info.inx - perf->info.p3) / perf->info.poolingStride);
                        outImageSlice = (arch_uint32)(outImageStride * ((arch_uint32)ceilf((arch_float32)(perf->info.iny - perf->info.p3) / perf->info.poolingStride) + perf->info.nextKY - 1));

                        /* Merge from CL 213845 for 64B alignment */
#ifdef ALIGNMENT_64B
                        if (gAlign64BExceted == 1)
                        {
                            if(dstBuf == SW_TILING_FROM_VIP_SRAM && perf->outputAlignment == 1)
                            {
                                perf->vipSramAccUnitSizeInByte = equivalentVipSramWidthInByte * 2;
                                if (pArchNnConfig->unifiedFeature.HW_Alignment)
                                {
                                    outImageSlice = (arch_uint32)ceilf((float)outImageSlice / perf->vipSramAccUnitSizeInByte) * perf->vipSramAccUnitSizeInByte;
                                }
                                else if (pArchNnConfig->unifiedFeature.SW_Alignment)
                                {
                                    while(outImageSlice % perf->vipSramAccUnitSizeInByte != 0)
                                    {
                                        outImageSlice = outImageSlice + outImageStride;
                                    }
                                }
                            }
                        }
#endif
                    }
                    else
                    {
                        outImageStride = perf->info.pix;
                        outImageSlice = perf->info.pix * perf->info.piy;

                        /* Merge from CL 213817 */
                        if(pArchDataFeature->ddrAlign == 1 && perf->downStreamLayerCount != 0 && perf->trspIvLayerChsIn == 1)
                        {
                            if(perf->firstChildAllSibling1x1 == 1)
                            {
                                while(outImageSlice % NN_DDR_BURST_SIZE != 0)
                                    outImageSlice += outImageStride;
                            }
                        }

                    }
                }
                /* Decide 1x1 WAR */
                //{
                arch_uint32 x_adj = 0, y_adj = 0, ky_adj = 0, kz_adj = 0, align = 0;
                arch_uint32 HWBUG_TMP = 0;
                arch_char warType[8];
                x_adj = inSIXRefined;        /* need to double check if it is */
                y_adj = inSIYRefined;
                ky_adj = perf->info.kx;
                kz_adj = perf->info.kz;
                /* SW_CONV1x1_TO_1x2 need to read from config, set to 0 now */
#define SW_CONV1x1_TO_1x2   0
                if ((op_target != ARCHNNE_OPERATION_TARGET_TP)
                    && (perf->info.kx == 1) && (perf->info.ky == 1) && ((perf->info.kz % 2) == 0)
                    && (perf->info.xOffSet == 0) && (perf->info.yOffSet == 0) && (perf->info.poolingStride == 1)
                    && (SW_CONV1x1_TO_1x2)
                    )
                {
                    align = 63;
                    while(((x_adj * y_adj) % align != 0) && (align >= 9))
                    {
                        align--;
                    }
                    HWBUG_TMP = 0;
                    if(vip7Version == 1 && HWBUG_TMP == 1)
                    {
                        WAR_1X1_Reshape_To_ZDPOpt(&x_adj, &y_adj, &ky_adj, &kz_adj, &inSIXRefined, &inSIYRefined, &inImageStride, &inImageSlice, &outImageStride, &outImageSlice, warType);
                    }
                    else if((x_adj * y_adj) % 64 == 0)
                    {
                        WAR_1X1_Reshape_To_MxN(64, &x_adj, &y_adj, &ky_adj, &kz_adj, &inSIXRefined, &inSIYRefined, &inImageStride, /*&inImageSlice,*/ &outImageStride, /*&outImageSlice,*/ warType);
                    }
                    else if((x_adj * y_adj) <= 64)
                    {
                        WAR_1X1_Reshape_To_Nx1(&x_adj, &y_adj, &ky_adj, &inSIXRefined, &inSIYRefined, &inImageStride, &inImageSlice, &outImageStride, &outImageSlice, warType);
                    }
                    else if (((x_adj * y_adj) % 16 == 0) /*&& (BUG_2035 = 0)*/)
                    {
                        WAR_1X1_Reshape_To_MxN(16, &x_adj, &y_adj, &ky_adj, &kz_adj, &inSIXRefined, &inSIYRefined, &inImageStride, /*&inImageSlice,*/ &outImageStride, /*&outImageSlice,*/ warType);
                    }
                    else if(align >= 9 && align <= 63)
                    {
                        WAR_1X1_Reshape_To_MxN(align, &x_adj, &y_adj, &ky_adj, &kz_adj, &inSIXRefined, &inSIYRefined, &inImageStride, /*&inImageSlice,*/ &outImageStride, /*&outImageSlice,*/ warType);
                    }
                    else
                    {
                        WAR_1X1_Reshape_To_Nx1(&x_adj, &y_adj, &ky_adj, &inSIXRefined, &inSIYRefined, &inImageStride, &inImageSlice, &outImageStride, &outImageSlice, warType);
                    }
                }
                //}

                /* port CL#213873 */ /* refined by CL#221777*/
                //reshapeImageTo16xN(pArchNnConfig, srcBuf, inImageSlice, &inImageStride, &outImageStride, &inSIXRefined, &inSIYRefined);

                if (perf->resultInfo.calculated != HALF_DONE)
                {
                    perf->resultInfo.outImageTileXSize = 0;
                    perf->resultInfo.outImageTileYSize = 0;
                    perf->resultInfo.kernelsPerCore    = 0;
                    minCycleCount    = ~0ULL;
                    minRDBandWidth   = ~0ULL;
                    /*minNCRDBandWidth = ~0ULL;*/
                    minWTBandWidth   = ~0ULL;

                    maxOutTileXSize = archMIN(archMIN(inXSize, maxOutTileXSize), maxInTileXSize - kernelXSize + 1);
                    minOutTileXSize = archMAX((arch_int32)poolingSize, archMAX(-xOffSet - (arch_int32)kernelXSize + 1 + 1, 0));
                    minOutTileYSize = archMAX((arch_int32)poolingSize, archMAX(-yOffSet - (arch_int32)kernelYSize + 1 + 1, 0));

                    if (maxOutTileXSize < minOutTileXSize)
                    {
                        archInfo("WARNING: maxOutTileXSize < minOutTileXSize\n");
                        goto errCalcArchPerf;
                    }

                    for (x = minOutTileXSize; x <= maxOutTileXSize; x++)
                    {
                        if ((poolingSize != 2 && poolingSize != 3) ||
                            (poolingSize == 2 && x % 2 == 0) ||
                            (poolingSize == 3 && x == inXSize))
                        {
                            interleaveMode = APMCalcImageInterleaveMode(
                                x,
                                pArchNnConfig->unifiedFeature.maxTileSize,
                                kernelXSize,
                                vip7_16bit,
                                interleave8
                                );

                            tmpCovMode = inputBuffDepthForOneTile * interleaveMode;
                            tmpCovAccuMode = adjustAccuBuffDepth * interleaveMode;


                            //if ( !isV8 && (tmpCovMode < kernelYSize))
                            //{
                            //    // V6 or V7 has this limitation,V8, V9 and later version do not have
                            //    break;
                            //}

                            //if (!isV8 || ((kernelXSize == 1) && (kernelYSize == 1)))
                            //{
                            //    tmpMaxOutTileYSize = archMIN(127, archMIN(tmpCovMode-kernelYSize+1, archMIN(tmpCovAccuMode, inYSize)));
                            //}
                            //else
                            //{
                            //    tmpMaxOutTileYSize = archMIN(127, inYSize);
                            //}

                            if (tmpCovMode < kernelYSize)
                            {
                                break;
                            }

                            tmpMaxOutTileYSize = archMIN(127, archMIN(tmpCovMode-kernelYSize+1, archMIN(tmpCovAccuMode, inYSize)));

                            if (tmpMaxOutTileYSize < minOutTileYSize)
                            {
                                if (poolingSize == 3)
                                {
                                    assert("Hit NN Pooling Size = 3 limitation, either perform pooling in TP or split image into smaller vertical subimages\n" && 0);
                                }
                                continue;
                            }
                            for (y = minOutTileYSize; y <= tmpMaxOutTileYSize; y++)
                            {
                                if (inXSize - xOffSet <= x + kernelXSize -1 &&
                                    inYSize - yOffSet <= y + kernelYSize -1 &&
                                    (!pArchDataFeature->drJdDiffConditionForCacheLineModePreFix ||
                                    (pArchDataFeature->drJdDiffConditionForCacheLineModePreFix && y == inYSize && x == inXSize)))
                                {
                                    cacheLineMode = 1;
                                }
                                else
                                {
                                    cacheLineMode = 0;
                                }
                                if ((brickMode || !cacheLineMode || (x == inXSize && y == inYSize)) &&
                                    ((poolingSize != 2 && poolingSize != 3) ||
                                     (poolingSize == 2 && y % 2 == 0) ||
                                     (poolingSize == 3 && (y != 3 || inYSize == 3))))
                                {
                                    /*arch_uint32 vipSramInimageStride = archMIN((x + kernelXSize - 1), inXSize);
                                    arch_uint32 vipSramInimageSlice = vipSramInimageStride * archMIN((y + kernelYSize - 1), inYSize);*/
                                    k = APMCalcNumOfKernel(x, y, outZSize, adjustAccuBuffDepth, numCores, interleaveMode,
                                                           zdp, kernelXSize, kernelYSize, pArchNnConfig->derivedFeature.nnXYDPX, isV8,
                                                           /*inputDataSize,*/ lanesPerConv, poolingStride,isDepthWise, isDepthWiseMerge, kernelPerCoreLTOneThirdCoefFix,asymmetricQuantization);
                                    //if ( isV8 && (k==0) )
                                    //{
                                    //    break;
                                    //}

                                    if (!pArchNnConfig->unifiedFeature.coefDeltaCordOverFlowZRL8BitFix &&
                                        ((kernelXSize == 2 && kernelYSize == 2) ||
                                         (kernelXSize == 1 && kernelYSize == 2) ||
                                         (kernelXSize == 1 && kernelYSize == 4) ||
                                         (kernelXSize == 1 && kernelYSize == 3)))
                                    {
                                        k = archMIN(63, k);
                                    }

                                    memset(&inPerfParams,0,sizeof(APM_IN_PERF_PARAMS));
                                    memset(&outBandWidth, 0, sizeof(outBandWidth));
                                    inPerfParams.bflush = flush_and_wait_first_cmd; // first command
                                    inPerfParams.xydp_x = pArchNnConfig->derivedFeature.nnXYDPX;
                                    inPerfParams.xydp_y = pArchNnConfig->derivedFeature.nnXYDPY;
                                    inPerfParams.interleave_mode = interleaveMode;
                                    inPerfParams.vip_v7_16bit = vip7_16bit;
                                    inPerfParams.vector_prune = vectorPrune;
                                    inPerfParams.in_image_slice = inImageSlice;
                                    inPerfParams.out_image_slice = outImageSlice;
                                    inPerfParams.op_type = op_type;
                                    inPerfParams.inDataBitSize = inputDataSize;
                                    inPerfParams.outDataBitSize = outputDataSize;

                                    inPerfParams.cmdInfo.outImageXsize  = inXSize;
                                    inPerfParams.cmdInfo.outImageYsize  = inYSize;
                                    inPerfParams.cmdInfo.outImageZsize = outZSize;
                                    inPerfParams.cmdInfo.inSIXRefined  = inSIXRefined;
                                    inPerfParams.cmdInfo.inSIYRefined  = inSIYRefined;
                                    inPerfParams.cmdInfo.inImageFp16   = fp16;

                                    inPerfParams.cmdInfo.u.nncmd.tile_xsize = x; /* tileXsize */
                                    inPerfParams.cmdInfo.u.nncmd.tile_ysize = y; /* tileYsize */
                                    inPerfParams.cmdInfo.u.nncmd.kernel_per_core = k; /* kernels per core */
                                    inPerfParams.cmdInfo.kernelXsize = kernelXSize;
                                    inPerfParams.cmdInfo.kernelYsize = kernelYSize;
                                    inPerfParams.cmdInfo.kernelZsize = kernelZSize;
                                    inPerfParams.cmdInfo.pooling_stride = poolingStride; /* can be NN command or tpcommand, refine me */
                                    inPerfParams.cmdInfo.brick_mode = brickMode;
                                    inPerfParams.cmdInfo.is_depth_wise = isDepthWise;
                                    inPerfParams.cmdInfo.isTA_MERGE = (op_type == ARCHNNE_OPERATOR_TENSOR_ADD_MERGE);
                                    inPerfParams.cmdInfo.in_image_stride = inImageStride;
                                    inPerfParams.cmdInfo.out_image_stride = outImageStride;

                                    inPerfParams.cmdInfo.src_buf = srcBuf;    /* src buf in DDR or SRAM */
                                    inPerfParams.cmdInfo.dst_buf = dstBuf;    /* dst buf in DDR or SRAM */
                                    inPerfParams.cmdInfo.kernel_buf = kernelBuf; /*  kernel buf in DDR or SRAM */

                                    inPerfParams.compInfo.coefNonZeroRatio = coefNonZeroRatio;
                                    inPerfParams.compInfo.coefCompression = coefCompressRatio;
                                    inPerfParams.compInfo.imageCompression = imageCompressRatio;
                                    inPerfParams.compInfo.imageNonZeroRatio = imageNonZeroRatio;

                                    /* NN Transpose */
                                    inPerfParams.cmdInfo.TrspInterleaveCh_in = perf->swTilingInfo.trspIvLayerChsIn;
                                    inPerfParams.cmdInfo.TrspInterleaveCh_out = perf->swTilingInfo.trspIvLayerChsOut;

                                    /* add for first command */
                                    inPerfParams.first_cmd = 1;
                                    inPerfParams.l2cache_size = l2CacheSize;

                                    // refine me
                                    //APMSetManualParams();

                                    memset(&outResult,0,sizeof(APM_OUT_RESULT_T));
                                    newCycleCount = APMCalcNNCycleCountBandWidth(apm, inPerfParams, &outBandWidth,&outResult);
                                    ddrKernelReadBandWidth = outBandWidth.ddr_kernel_read_bandwidth;
                                    ddrInImageReadBandWidth = outBandWidth.ddr_in_image_read_bandwidth;
                                    ddrRDBandWidth = outBandWidth.ddr_read_bandwidth;
                                    ddrWTBandWidth = outBandWidth.ddr_write_bandwidth;
                                    axiRDBandWidth = outBandWidth.axi_read_bandwidth;
                                    axiWTBandWidth = outBandWidth.axi_write_bandwidth;
                                    isComputeBottleNeck = outBandWidth.is_compute_bottle_neck;

                                    newRDBandWidth = outBandWidth.ddr_read_bandwidth;
                                    newWTBandWidth = outBandWidth.ddr_write_bandwidth;

                                    curCost.cycle = (arch_uint64)(newCycleCount + 0.5f);
                                    curCost.bw = (arch_uint64)(newRDBandWidth + 0.5f) + (arch_uint64)(newWTBandWidth + 0.5f);

                                    if (!initMinCost || _cur_cost_u64_is_more_better(&minCost, &curCost))
                                    {
                                        initMinCost = arch_true_e;
                                        minCycleCount    = (arch_uint64)(newCycleCount + 0.5f);
                                        minRDBandWidth   = (arch_uint64)(newRDBandWidth + 0.5f);
                                        /*minNCRDBandWidth = (arch_uint64)(newNCRDBandWidth + 0.5f);*/
                                        minWTBandWidth   = (arch_uint64)(newWTBandWidth + 0.5f);
                                        minCost.cycle = minCycleCount;
                                        minCost.bw = minRDBandWidth + minWTBandWidth;
                                        perf->resultInfo.outImageTileXSize     = x;
                                        perf->resultInfo.outImageTileYSize     = y;
                                        perf->resultInfo.kernelsPerCore        = k;
                                        perf->resultInfo.interleaveMode        = interleaveMode;
                                        perf->resultInfo.nnCoreCount           = numCores;
                                        perf->resultInfo.perfCycleCount        = newCycleCount;
                                        perf->resultInfo.perfKernelReadBandWidth = ddrKernelReadBandWidth;
                                        perf->resultInfo.perfInImageReadBandWidth = ddrInImageReadBandWidth;
                                        perf->resultInfo.perfReadBandWidth     = ddrRDBandWidth;
                                        perf->resultInfo.perfWriteBandWidth    = ddrWTBandWidth;
                                        perf->resultInfo.perfAXIReadBandWidth  = axiRDBandWidth;
                                        perf->resultInfo.perfAXIWriteBandWidth = axiWTBandWidth;
                                        perf->resultInfo.isFirstComputeBottleNeck = isComputeBottleNeck;

                                        /* set detail result */
                                        memcpy(&(perf->archPerfDetailResult),&outResult,sizeof(APM_OUT_RESULT_T));
                                        /* set first/nonfirst cmd */
                                        perf->swTilingInfo.firstCmdBottleNeck = outResult.BN_BottleNeck_e;
                                    }
                                }
                            }
                        }
                    }
                    assert(perf->resultInfo.outImageTileXSize <= maxOutTileXSize);
                    assert(perf->resultInfo.outImageTileXSize >= minOutTileXSize);
                    assert(perf->resultInfo.outImageTileYSize >= minOutTileYSize);
                }
                else
                {
                    memset(&inPerfParams,0,sizeof(APM_IN_PERF_PARAMS));
                    memset(&outBandWidth, 0, sizeof(outBandWidth));
                    inPerfParams.bflush = flush_and_wait_first_cmd; // first command
                    inPerfParams.xydp_x = pArchNnConfig->derivedFeature.nnXYDPX;
                    inPerfParams.xydp_y = pArchNnConfig->derivedFeature.nnXYDPY;
                    inPerfParams.interleave_mode = perf->resultInfo.interleaveMode;
                    inPerfParams.vip_v7_16bit = vip7_16bit;
                    inPerfParams.vector_prune = vectorPrune;
                    inPerfParams.in_image_slice = inImageSlice;
                    inPerfParams.out_image_slice = outImageSlice;
                    inPerfParams.op_type = op_type;
                    inPerfParams.inDataBitSize = inputDataSize;
                    inPerfParams.outDataBitSize = outputDataSize;

                    inPerfParams.cmdInfo.outImageXsize  = inXSize;
                    inPerfParams.cmdInfo.outImageYsize  = inYSize;
                    inPerfParams.cmdInfo.outImageZsize = outZSize;
                    inPerfParams.cmdInfo.inSIXRefined  = inSIXRefined;
                    inPerfParams.cmdInfo.inSIYRefined  = inSIYRefined;
                    inPerfParams.cmdInfo.inImageFp16   = fp16;

                    inPerfParams.cmdInfo.u.nncmd.tile_xsize = perf->resultInfo.outImageTileXSize; /* tileXsize */
                    inPerfParams.cmdInfo.u.nncmd.tile_ysize = perf->resultInfo.outImageTileYSize; /* tileYsize */
                    inPerfParams.cmdInfo.u.nncmd.kernel_per_core = perf->resultInfo.kernelsPerCore; /* kernels per core */
                    inPerfParams.cmdInfo.kernelXsize = kernelXSize;
                    inPerfParams.cmdInfo.kernelYsize = kernelYSize;
                    inPerfParams.cmdInfo.kernelZsize = kernelZSize;
                    inPerfParams.cmdInfo.pooling_stride = poolingStride; /* can be NN command or tpcommand, refine me */
                    inPerfParams.cmdInfo.brick_mode = brickMode;
                    inPerfParams.cmdInfo.is_depth_wise = isDepthWise;
                    inPerfParams.cmdInfo.in_image_stride = inImageStride;
                    inPerfParams.cmdInfo.out_image_stride = outImageStride;

                    inPerfParams.cmdInfo.src_buf = srcBuf;    /* src buf in DDR or SRAM */
                    inPerfParams.cmdInfo.dst_buf = dstBuf;    /* dst buf in DDR or SRAM */
                    inPerfParams.cmdInfo.kernel_buf = kernelBuf; /*  kernel buf in DDR or SRAM */

                    inPerfParams.compInfo.coefNonZeroRatio = coefNonZeroRatio;
                    inPerfParams.compInfo.coefCompression = coefCompressRatio;
                    inPerfParams.compInfo.imageCompression = imageCompressRatio;
                    inPerfParams.compInfo.imageNonZeroRatio = imageNonZeroRatio;

                    /* NN Transpose */
                    inPerfParams.cmdInfo.TrspInterleaveCh_in = perf->swTilingInfo.trspIvLayerChsIn;
                    inPerfParams.cmdInfo.TrspInterleaveCh_out = perf->swTilingInfo.trspIvLayerChsOut;

                    /* for first command */
                    inPerfParams.first_cmd  = 1;
                    inPerfParams.l2cache_size = l2CacheSize;

                    // refine me
                    //APMSetManualParams();
                    memset(&outResult,0,sizeof(APM_OUT_RESULT_T));
                    perf->resultInfo.perfCycleCount = APMCalcNNCycleCountBandWidth(apm, inPerfParams, &outBandWidth,&outResult);
                    ddrKernelReadBandWidth = outBandWidth.ddr_kernel_read_bandwidth;
                    ddrInImageReadBandWidth = outBandWidth.ddr_in_image_read_bandwidth;
                    ddrRDBandWidth = outBandWidth.ddr_read_bandwidth;
                    ddrWTBandWidth = outBandWidth.ddr_write_bandwidth;
                    axiRDBandWidth = outBandWidth.axi_read_bandwidth;
                    axiWTBandWidth = outBandWidth.axi_write_bandwidth;
                    isComputeBottleNeck = outBandWidth.is_compute_bottle_neck;

                    perf->resultInfo.perfKernelReadBandWidth = ddrKernelReadBandWidth;
                    perf->resultInfo.perfInImageReadBandWidth = ddrInImageReadBandWidth;
                    perf->resultInfo.perfReadBandWidth = ddrRDBandWidth;
                    perf->resultInfo.perfWriteBandWidth = ddrWTBandWidth;
                    perf->resultInfo.perfAXIReadBandWidth = axiRDBandWidth;
                    perf->resultInfo.perfAXIWriteBandWidth = axiWTBandWidth;
                    perf->resultInfo.isFirstComputeBottleNeck = isComputeBottleNeck;

                    /* set detail result */
                    memcpy(&(perf->archPerfDetailResult),&outResult,sizeof(APM_OUT_RESULT_T));
                    /* set first/nonfirst cmd */
                    perf->swTilingInfo.firstCmdBottleNeck = outResult.BN_BottleNeck_e;
                }
            }
            else
            {
                perf->resultInfo.outImageTileXSize   = 0;
                perf->resultInfo.outImageTileYSize   = 0;
                perf->resultInfo.kernelsPerCore     = 0;

                memset(&inPerfParams,0,sizeof(APM_IN_PERF_PARAMS));
                memset(&outBandWidth, 0, sizeof(outBandWidth));
                inPerfParams.bflush = flush_and_wait_nonfirst_cmd;
                inPerfParams.xydp_x = pArchNnConfig->derivedFeature.nnXYDPX;
                inPerfParams.xydp_y = pArchNnConfig->derivedFeature.nnXYDPY;
                /*inPerfParams.interleave_mode = interleaveMode;*/
                inPerfParams.vip_v7_16bit    = vip7_16bit;
                inPerfParams.vector_prune    = vectorPrune;
                inPerfParams.in_image_slice  = inImageSlice;
                inPerfParams.out_image_slice = outImageSlice;
                inPerfParams.op_type         = op_type;
                inPerfParams.inDataBitSize   = inputDataSize;
                inPerfParams.outDataBitSize  = outputDataSize;

                inPerfParams.cmdInfo.u.tpcmd.x   = inXSize;
                inPerfParams.cmdInfo.u.tpcmd.y   = inYSize;
                inPerfParams.cmdInfo.u.tpcmd.z   = outZSize;
                inPerfParams.cmdInfo.kernelZsize = kernelZSize;

                inPerfParams.cmdInfo.pooling_stride   = poolingStride; /* can be NN command or tpcommand, refine me */
                inPerfParams.cmdInfo.brick_mode       = brickMode;
                inPerfParams.cmdInfo.is_depth_wise    = isDepthWise;
                inPerfParams.cmdInfo.in_image_stride  = inImageStride;
                inPerfParams.cmdInfo.out_image_stride = outImageStride;

                inPerfParams.cmdInfo.src_buf = srcBuf;    /* src buf in DDR or SRAM */
                inPerfParams.cmdInfo.dst_buf = dstBuf;    /* dst buf in DDR or SRAM */
                inPerfParams.cmdInfo.kernel_buf = kernelBuf; /*  kernel buf in DDR or SRAM */

                inPerfParams.compInfo.coefNonZeroRatio = coefNonZeroRatio;
                inPerfParams.compInfo.coefCompression = coefCompressRatio;
                inPerfParams.compInfo.imageCompression = imageCompressRatio;
                inPerfParams.compInfo.imageNonZeroRatio = imageNonZeroRatio;

                inPerfParams.l2cache_size = l2CacheSize;
                /* NN Transpose */
                inPerfParams.cmdInfo.TrspInterleaveCh_in = perf->swTilingInfo.trspIvLayerChsIn;
                inPerfParams.cmdInfo.TrspInterleaveCh_out = perf->swTilingInfo.trspIvLayerChsOut;
                APM_OUT_RESULT_T outResult;
                memset(&outResult, 0, sizeof(APM_OUT_RESULT_T));

                newCycleCount = APMCalcTPCycleCountCore(apm, inPerfParams, &outBandWidth, &outResult);

                ddrKernelReadBandWidth = outBandWidth.ddr_kernel_read_bandwidth;
                ddrInImageReadBandWidth = outBandWidth.ddr_in_image_read_bandwidth;
                ddrRDBandWidth = outBandWidth.ddr_read_bandwidth;
                ddrWTBandWidth = outBandWidth.ddr_write_bandwidth;
                axiRDBandWidth = outBandWidth.axi_read_bandwidth;
                axiWTBandWidth = outBandWidth.axi_write_bandwidth;

                perf->resultInfo.perfCycleCount        = newCycleCount;
                perf->resultInfo.perfKernelReadBandWidth = ddrKernelReadBandWidth;
                perf->resultInfo.perfInImageReadBandWidth = ddrInImageReadBandWidth;
                perf->resultInfo.perfReadBandWidth     = ddrRDBandWidth;
                perf->resultInfo.perfWriteBandWidth    = ddrWTBandWidth;
                perf->resultInfo.perfAXIReadBandWidth  = axiRDBandWidth;
                perf->resultInfo.perfAXIWriteBandWidth = axiWTBandWidth;

                memcpy(&(perf->archPerfDetailResult), &outResult, sizeof(APM_OUT_RESULT_T));
                /* set first/nonfirst cmd: TP always first cmd, so first and nonfirst are the same */
                perf->swTilingInfo.firstCmdBottleNeck = outResult.BN_BottleNeck_e;
                perf->swTilingInfo.nonFirstCmdBottleNeck = outResult.BN_BottleNeck_e;
            }
        }

        if (swTiling && op_target == ARCHNNE_OPERATION_TARGET_NN && perf->swTilingInfo.calcNonFirstCmd)
        {
            interleaveMode = APMCalcImageInterleaveMode(
                perf->resultInfo.outImageTileXSize,
                pArchNnConfig->unifiedFeature.maxTileSize,
                kernelXSize,
                vip7_16bit,
                interleave8);

            memset(&inPerfParams,0,sizeof(APM_IN_PERF_PARAMS));
            memset(&outBandWidth, 0, sizeof(outBandWidth));
            inPerfParams.bflush = flush_and_wait_nonfirst_cmd; // non first command
            inPerfParams.xydp_x = pArchNnConfig->derivedFeature.nnXYDPX;
            inPerfParams.xydp_y = pArchNnConfig->derivedFeature.nnXYDPY;
            inPerfParams.interleave_mode = interleaveMode;
            inPerfParams.vip_v7_16bit = vip7_16bit;
            inPerfParams.vector_prune = vectorPrune;
            inPerfParams.in_image_slice = inImageSlice;
            inPerfParams.out_image_slice = outImageSlice;
            inPerfParams.op_type = op_type;
            inPerfParams.inDataBitSize = inputDataSize;
            inPerfParams.outDataBitSize  = outputDataSize;

            inPerfParams.cmdInfo.outImageXsize  = inXSize;
            inPerfParams.cmdInfo.outImageYsize  = inYSize;
            inPerfParams.cmdInfo.outImageZsize = outZSize;
            inPerfParams.cmdInfo.inSIXRefined  = inSIXRefined;
            inPerfParams.cmdInfo.inSIYRefined  = inSIYRefined;
            inPerfParams.cmdInfo.inImageFp16   = fp16;

            inPerfParams.cmdInfo.u.nncmd.tile_xsize = perf->resultInfo.outImageTileXSize; /* tileXsize */
            inPerfParams.cmdInfo.u.nncmd.tile_ysize = perf->resultInfo.outImageTileYSize; /* tileYsize */
            inPerfParams.cmdInfo.u.nncmd.kernel_per_core = perf->resultInfo.kernelsPerCore; /* kernels per core */
            inPerfParams.cmdInfo.kernelXsize = kernelXSize;
            inPerfParams.cmdInfo.kernelYsize = kernelYSize;
            inPerfParams.cmdInfo.kernelZsize = kernelZSize;
            inPerfParams.cmdInfo.pooling_stride = poolingStride; /* can be NN command or tpcommand, refine me */
            inPerfParams.cmdInfo.brick_mode = brickMode;
            inPerfParams.cmdInfo.is_depth_wise = isDepthWise;
            inPerfParams.cmdInfo.isTA_MERGE = (op_type == ARCHNNE_OPERATOR_TENSOR_ADD_MERGE);
            inPerfParams.cmdInfo.in_image_stride = inImageStride;
            inPerfParams.cmdInfo.out_image_stride = outImageStride;
            inPerfParams.cmdInfo.inSIXRefined = inSIXRefined;
            inPerfParams.cmdInfo.inSIYRefined = inSIYRefined;

            inPerfParams.cmdInfo.src_buf = srcBuf;    /* src buf in DDR or SRAM */
            inPerfParams.cmdInfo.dst_buf = dstBuf;    /* dst buf in DDR or SRAM */
            inPerfParams.cmdInfo.kernel_buf = kernelBuf; /*  kernel buf in DDR or SRAM */

            inPerfParams.compInfo.coefNonZeroRatio = coefNonZeroRatio;
            inPerfParams.compInfo.coefCompression = coefCompressRatio;
            inPerfParams.compInfo.imageCompression = imageCompressRatio;
            inPerfParams.compInfo.imageNonZeroRatio = imageNonZeroRatio;

            /* NN Transpose */
            inPerfParams.cmdInfo.TrspInterleaveCh_in = perf->swTilingInfo.trspIvLayerChsIn;
            inPerfParams.cmdInfo.TrspInterleaveCh_out = perf->swTilingInfo.trspIvLayerChsOut;

            /* for first command */
            inPerfParams.first_cmd = 0;
            inPerfParams.l2cache_size = l2CacheSize;

            // refine me
            //APMSetManualParams();
            memset(&outResult,0,sizeof(APM_OUT_RESULT_T));
            perf->swTilingInfo.perfNonFirstCycleCount = APMCalcNNCycleCountBandWidth(apm, inPerfParams, &outBandWidth,&outResult);
            ddrKernelReadBandWidth = outBandWidth.ddr_kernel_read_bandwidth;
            ddrInImageReadBandWidth = outBandWidth.ddr_in_image_read_bandwidth;
            ddrRDBandWidth = outBandWidth.ddr_read_bandwidth;
            ddrWTBandWidth = outBandWidth.ddr_write_bandwidth;
            axiRDBandWidth = outBandWidth.axi_read_bandwidth;
            axiWTBandWidth = outBandWidth.axi_write_bandwidth;
            isComputeBottleNeck = outBandWidth.is_compute_bottle_neck;

            perf->swTilingInfo.perfNonFirstKernelReadBandWidth = ddrKernelReadBandWidth;
            perf->swTilingInfo.perfNonFirstInImageReadBandWidth = ddrInImageReadBandWidth;
            perf->swTilingInfo.perfNonFirstReadBandWidth = ddrRDBandWidth;
            perf->swTilingInfo.perfNonFirstWriteBandWidth = ddrWTBandWidth;
            perf->swTilingInfo.perfNonFirstAXIReadBandWidth = axiRDBandWidth;
            perf->swTilingInfo.perfNonFirstAXIWriteBandWidth = axiWTBandWidth;
            perf->swTilingInfo.isNonFirstComputeBottleNeck = isComputeBottleNeck;
            /* set detail result */
            /* save first command bottlleneck */

            memcpy(&(perf->archPerfDetailResult),&outResult,sizeof(APM_OUT_RESULT_T));
            /* set first/nonfirst cmd */
            perf->swTilingInfo.nonFirstCmdBottleNeck = outResult.BN_BottleNeck_e;
        }

        perf->calculated = arch_true_e;

        /* weight_size equal to kx and ky, need to verify */
        if ((perf->info.kx == 1 && perf->info.ky == 1
            && (archIsFeatureAvailable(pArchNnConfig,pArchOptions, pArchDataFeature,ARCH_NN_FEATURE_ZDP3) || archIsFeatureAvailable(pArchNnConfig,pArchOptions, pArchDataFeature,ARCH_NN_FEATURE_ZDP6))
            && (perf->info.inputDataFormat == ARCH_TYPE_INT8 || perf->info.inputDataFormat == ARCH_TYPE_UINT8)) && !isV8)
        {
            /*Per HW, there's a limition for HW now, arch perf's kernel per core should less equre than (accuBuffDepth / zdpNum) when zdp3 & zdp6*/
            arch_uint32 zdpNum = archIsFeatureAvailable(pArchNnConfig,pArchOptions,pArchDataFeature, ARCH_NN_FEATURE_ZDP6) ? 6 : 3;
            if (!(perf->resultInfo.kernelsPerCore <= (accuBuffDepth / zdpNum)))
            {
                archError("Assert: perf->resultInfo.kernelsPerCore <= (accuBuffDepth / zdpNum) must be true: kernelPerCore: %d, accuBuffDepth: %d, zdpNum: %d\n", perf->resultInfo.kernelsPerCore, accuBuffDepth, zdpNum);
            }
            assert(perf->resultInfo.kernelsPerCore <= (accuBuffDepth / zdpNum));
            zdpNum++; /* Make release build pass*/
        }
    }

    perf->opType   = op_type;
    perf->opTarget = op_target;
    return ARCH_SUCCESS;

errCalcArchPerf:
    perf->calculated = arch_true_e;
    perf->resultInfo.outImageTileXSize   = 0;
    perf->resultInfo.outImageTileYSize   = 0;
    perf->resultInfo.kernelsPerCore     = 0;
    perf->opType = op_type;
    perf->opTarget = op_target;
    return ARCH_FAILURE;
}


void reshapeImageTo16xN(
    arch_nn_config  *pArchNnConfig,
    /*arch_drv_option *pArchOptions,*/
    arch_uint32      src_buf,
    arch_uint32    inimageSlice,
    arch_uint32    *inImageStride,
    arch_uint32    *outImageStride,
    /*unsigned int    &ky,
    unsigned int    &kz,*/
    arch_uint32    *SIX,
    arch_uint32    *SIY
    )
{
    unsigned int PPC = 64;
    unsigned int adjusted_SIX = *SIX;
    unsigned int adjusted_SIY = *SIY;

    if (src_buf == SW_TILING_FROM_DDR || src_buf == SW_TILING_FROM_AXI_SRAM)
    {
        PPC = 64;
    }
    else if (src_buf == SW_TILING_PERM_VIP_SRAM)
    {
        PPC = pArchNnConfig->fixedFeature.equivalentVipsramWidthInByte * 2;
    }

    if ((inimageSlice % PPC == 0) && ((*inImageStride) == (*SIX)))
    {
        adjusted_SIX = 16;
        adjusted_SIX = (unsigned int)ceilf((float)(*SIX) * (*SIY) / adjusted_SIX);
        while (adjusted_SIY >= 8192 && adjusted_SIX <= 128)
        {
            adjusted_SIX = adjusted_SIX * 2;
            adjusted_SIY = (unsigned int)ceilf((float)(*SIX) * (*SIY) / adjusted_SIX);
        }

        /*ky = 1;
        kz = kz;*/
        *SIX = adjusted_SIX;
        *SIY = adjusted_SIY;

        *inImageStride = *outImageStride = adjusted_SIX;
    }
}


