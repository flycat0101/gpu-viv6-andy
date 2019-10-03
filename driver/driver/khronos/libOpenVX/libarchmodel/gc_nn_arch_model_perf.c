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
#include <gc_vx_nn_util.h>
#include "gc_nn_arch_model.h"
#ifdef USE_LIB_NN_ARCH_PERF
#include "nnArchPerf.h"
#endif

#define AXI_BURST_SIZE 64
static vx_uint32 _kernel_size_in_pixel_by_arch_perf(
    vxnne_operation_target_e opTarget,
    vxnne_operator_e opType,
    vx_arch_perf perf,
    vx_bool full_chache_kernel_head_fix)
{
    vx_float64 coefCompressionRatio = perf->coefCompressRatio;
    vx_float64 margin_ratio = (1.25 - 1.05) * (1.0 - coefCompressionRatio) / (1.0 - 0.02) + 1.05;

    if (opType == VXNNE_OPERATOR_DEPTH_WISE_CONV)
    {
        if (full_chache_kernel_head_fix)
        {
            return (vx_uint32)(perf->info.kx
                      * perf->info.ky
                      * perf->info.outz
                      * coefCompressionRatio * margin_ratio + 0.5f);
        }
        else
        {
            return (vx_uint32)(perf->info.kx
                      * perf->info.ky
                      * ceilf((vx_float32)perf->info.outz / perf->info.nnCores) * perf->info.nnCores
                      * coefCompressionRatio * margin_ratio + 0.5f);
        }
    }

    if (opTarget != VXNNE_OPERATION_TARGET_TP || opType == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        if (coefCompressionRatio)
        {
            if (full_chache_kernel_head_fix)
            {
                return (vx_uint32)(perf->info.kx
                          * perf->info.ky
                          * perf->info.kz
                          * perf->info.outz
                          * coefCompressionRatio * margin_ratio * 1.05f + 0.5f);
            }
            else
            {
               return (vx_uint32)((vx_float32)perf->info.kx
                          * perf->info.ky
                          * perf->info.kz
                          * ceilf((vx_float32)perf->info.outz / perf->info.nnCores) * perf->info.nnCores
                          * coefCompressionRatio * margin_ratio * 1.05f + 0.5f);
            }
        }
    }
    return 0;
}

static vx_float64 _calcKernelCachePercentage(
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 z,
    vx_uint32 cores,
    vx_float64 coef_compress_ratio,
    vx_float64 cache_size_in_pixel,
    vx_bool full_cach_kernel_head_fix,
    vx_bool is_depth_wise);

static vx_float32 _calcImageIdealCacheInPixel(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 x, vx_uint32 y,
    vx_int32 xoffset, vx_int32 yoffset,
    vx_uint32 sub_x, vx_uint32 sub_y,
    vx_uint32 data_size,
    vx_bool image_not_packed_in_sram,
    vx_uint32 equivalent_vip_sram_width_in_byte);

static void _calcArchModelCacheMode(vx_context context, vx_arch_perf perf, vx_int32 *image_ideal_cache_size_in_pixel)
{
    if (perf->opTarget == VXNNE_OPERATION_TARGET_NN)
    {
        vx_int32 cacheSpaceLeftInPixel = 0;
        vx_float64 kernelCachePercentage = 0;
        vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling ? vx_true_e : vx_false_e;
        vx_int32 fullCacheSpaceSizeInPixel = vxmARCH_VIP_SRAM_SIZE;
        vx_int32 imageIdealCacheSizeInPixel = (vx_int32)_calcImageIdealCacheInPixel(
                perf->resultInfo.outImageTileXSize,
                perf->resultInfo.outImageTileXSize,
                perf->info.kx, perf->info.ky, perf->info.kz,
                perf->swTilingInfo.origInX,
                perf->swTilingInfo.origInY,
                perf->info.xOffSet, perf->info.yOffSet,
                perf->info.inx, perf->info.inx,
                perf->info.inputDataSize,
                context->nnConfig.unifiedFeature.imageNotPackedInSram,
                context->nnConfig.fixedFeature.equivalentVipsramWidthInByte);

        if (NULL != image_ideal_cache_size_in_pixel) *image_ideal_cache_size_in_pixel = imageIdealCacheSizeInPixel;
        if (perf->swTilingInfo.srcBuf == 0 && perf->swTilingInfo.dstBuf == 0) /*ddr -> ddr*/
        {
            if (imageIdealCacheSizeInPixel <= fullCacheSpaceSizeInPixel)
            {
                perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                cacheSpaceLeftInPixel = fullCacheSpaceSizeInPixel - imageIdealCacheSizeInPixel;
            }
            else
            {
                perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                cacheSpaceLeftInPixel = fullCacheSpaceSizeInPixel;
            }
            kernelCachePercentage = _calcKernelCachePercentage(perf->info.kx, perf->info.ky, perf->info.kz,
                                                            perf->swTilingInfo.origOutZ, perf->info.nnCores,
                                                            perf->coefCompressRatio, cacheSpaceLeftInPixel,
                                                            context->nnConfig.unifiedFeature.fullCacheKernelHeadFix,
                                                            (perf->opType == VXNNE_OPERATOR_DEPTH_WISE_CONV));
        }
        else if (perf->swTilingInfo.kernelBuf == 0) /*ab buffer*/
        {
            if (axiSramOnlySWTiling)
            {
                if (imageIdealCacheSizeInPixel <= fullCacheSpaceSizeInPixel)
                {
                    perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                    cacheSpaceLeftInPixel = fullCacheSpaceSizeInPixel - imageIdealCacheSizeInPixel;
                }
                else
                {
                    perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                    cacheSpaceLeftInPixel = fullCacheSpaceSizeInPixel;
                }
                kernelCachePercentage = _calcKernelCachePercentage(perf->info.kx, perf->info.ky, perf->info.kz,
                                                                perf->swTilingInfo.origOutZ, perf->info.nnCores,
                                                                perf->coefCompressRatio, cacheSpaceLeftInPixel,
                                                                context->nnConfig.unifiedFeature.fullCacheKernelHeadFix,
                                                                (perf->opType == VXNNE_OPERATOR_DEPTH_WISE_CONV));
            }
            else
            {
                cacheSpaceLeftInPixel = /*axiSramOnlySWTiling ? fullCacheSpaceSizeInPixel : */
                                       (vx_int32)(fullCacheSpaceSizeInPixel - perf->swTilingInfo.segTotalBufferSizeInPixel);
                if (perf->swTilingInfo.srcBuf == 0)
                {
                    if (imageIdealCacheSizeInPixel <= cacheSpaceLeftInPixel)
                    {
                        perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                        cacheSpaceLeftInPixel = cacheSpaceLeftInPixel - imageIdealCacheSizeInPixel;
                    }
                    else
                    {
                        perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                    }
                }
                else
                {
                    perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                }
                kernelCachePercentage = _calcKernelCachePercentage(perf->info.kx, perf->info.ky, perf->info.kz,
                                                            perf->swTilingInfo.origOutZ, perf->info.nnCores,
                                                            perf->coefCompressRatio, cacheSpaceLeftInPixel,
                                                            context->nnConfig.unifiedFeature.fullCacheKernelHeadFix,
                                                            (perf->opType == VXNNE_OPERATOR_DEPTH_WISE_CONV));
            }
        }
        else
        {
            vxmASSERT(perf->swTilingInfo.kernelBuf);

            if (axiSramOnlySWTiling)
            {
                vx_uint32 swTilingSegKernelBufSize = perf->swTilingInfo.swTilingSegKernelBufSizeInPixel;
                cacheSpaceLeftInPixel = (vx_int32)(fullCacheSpaceSizeInPixel - swTilingSegKernelBufSize);
                if (imageIdealCacheSizeInPixel <= cacheSpaceLeftInPixel)
                {
                    perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                }
                else
                {
                    perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                }
            }
            else
            {
                if (perf->swTilingInfo.srcBuf == 0)
                {
                    cacheSpaceLeftInPixel = (vx_int32)(fullCacheSpaceSizeInPixel - perf->swTilingInfo.segTotalBufferSizeInPixel);
                    if (imageIdealCacheSizeInPixel <= cacheSpaceLeftInPixel)
                    {
                        perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_FULL_CACHE;
                    }
                    else
                    {
                        perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                    }
                }
                else
                {
                    perf->swTilingInfo.imageCacheMode = VXNNE_SRAM_CACHE_MODE_NONE;
                }
            }
            kernelCachePercentage = 0;
        }
        perf->swTilingInfo.kernelCacheMode = (kernelCachePercentage == 1.0f) ? VXNNE_SRAM_CACHE_MODE_FULL_CACHE
                                             : (kernelCachePercentage > 0) ? VXNNE_SRAM_CACHE_MODE_PARTIAL_CACHE
                                             : (perf->swTilingInfo.kernelBuf) ? VXNNE_SRAM_CACHE_MODE_STREAM_CACHE : VXNNE_SRAM_CACHE_MODE_NONE;
    }

}


static vx_int8 gOrigShowType = -1;
static const vx_char *archModelVersion = "ARCHCTS@222120";
static const vx_char *SWTilingVersion = "ARCHCTS@222120";
vx_status showArchPerformance(
    vx_context context,
    vxnne_layer layer,
    vxnne_operation op,
    vx_arch_perf perf
    )
{
    vx_uint32 i;
    vx_uint32 profileMode = 0;
    vx_float32 mutiGpuFactor = (context->nnConfig.fixedFeature.vipCoreCount > 1) ? (0.7f * context->nnConfig.fixedFeature.vipCoreCount) : 1.0f;
    vx_int32 imageIdealCacheSizeInPixel = 0;
    if (gOrigShowType != (vx_int8)context->options.collectPerfType)
    {
        vxInfo("\nArchModelVersion: %s\nSWTilingVersion: %s\nProfileMode: %d\n"
               "NumNNCores:%d\nNumNNCoresInt8: %d\nNumNNCoresInt16: %d\nNumNNCoresFloat16: %d\n"
               "NumTPCores: %d\nNumTPLiteCores: %d\nMadPerCore: %d\nVIP7Version: %d\n"
               "InBuffDepth: %d\nAccumBufferDepth: %d\nDPAmount: %d\n"
               "XYDPX: %d\nXYDPY: %d\nZDP: %d\n"
               "AXISRAMSize: %d\nVIPSRAMSize: %d\nL2CacheWidth: %d\n"
               "USCCacheSize: %d\nBrickMode: %d\nSWTiling: %d\n"
               "SmallBatchEnable: %d\nSWTilingPhase1: %d\nTPWithFCLayer: %d\n"
               "TPCircularBufferSupport: %d\nKERNEL_HEADER_NOT_CACHED_FIX: %d\n"
               "NNFCNonPruneAccel: %d\nConv1x1HalfPerformance: %d\n"
               "DDRLatency: %d\nCacheLineModeDisabled: %d\n"
               "PER_3D_TILE_BUBBLE_FIX: %d\nSWConv1x1To1x2: %d\n"
               "TP_LOCALIZATION_REORDER_DISABLED_Fix: %d\nUSCCacheControllers: %d\n"
               "AsyncCopyPerfFix: %d\nZDP3NoCompressFix: %d\n"
               "ZXDP3KernelReadConflictFix: %d\n",
               archModelVersion,
               SWTilingVersion,
               (context->options.collectPerfType == COLLECT_PERF_ESTIMATE) ? 1 : profileMode,
               context->nnConfig.fixedFeature.nnCoreCount,
               context->nnConfig.fixedFeature.nnCoreCountInt8,
               context->nnConfig.fixedFeature.nnCoreCountInt16,
               context->nnConfig.fixedFeature.nnCoreCountFloat16,
               context->nnConfig.fixedFeature.tpCoreCount,
               context->nnConfig.fixedFeature.tpliteCoreCount,
               context->nnConfig.fixedFeature.nnMadPerCore,
               context->nnConfig.fixedFeature.vip7Version,
               context->nnConfig.fixedFeature.nnInputBufferDepth,
               context->nnConfig.fixedFeature.nnAccumBufferDepth,
               context->nnConfig.derivedFeature.nnDPAmount,
               context->nnConfig.derivedFeature.nnXYDPX,
               context->nnConfig.derivedFeature.nnXYDPY,
               context->nnConfig.derivedFeature.nnZDP,
               context->nnConfig.customizedFeature.axiSRAMSize,
               context->nnConfig.customizedFeature.vipSRAMSize,
               context->nnConfig.fixedFeature.equivalentVipsramWidthInByte,
               context->nnConfig.unifiedFeature.nnUSCCacheSize,
               context->nnConfig.fixedFeature.vipBrickMode,
               context->nnConfig.customizedFeature.vipSWTiling,
               context->nnConfig.unifiedFeature.smallBatchEnable, /*smallBatchEnable*/
               vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1) ? 1 : 0,
               1, /*TPWithFCLayer*/
               vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE2) ? 1 : 0,
               context->nnConfig.unifiedFeature.fullCacheKernelHeadFix,/*KERNEL_HEADER_NOT_CACHED_BUG1968*/
               0,  /*NNFCNonPruneAccel*/
               context->nnConfig.unifiedFeature.conv1x1HalfPerformance,
               (vx_uint32)context->nnConfig.customizedFeature.ddrLatency,
               context->nnConfig.unifiedFeature.cacheLineModeDisabled,
               context->nnConfig.unifiedFeature.per3DTileBubbleFix,
               0, /*swConv1x1To1x2*/
               context->nnConfig.unifiedFeature.tpReOrderFix,
               context->nnConfig.fixedFeature.uscCacheControllers,
               context->nnConfig.unifiedFeature.asyncCopyPerfFix,
               context->nnConfig.unifiedFeature.zdp3NoCompressFix,
               context->nnConfig.unifiedFeature.zxdp3KernelReadConflictFix
               );

        vxInfo("CoefDecodePerf: %d\nVectorPrune: %d\nEnableCacheDataFromSRAM: %d\n"
               "IMAGE_PARTIAL_CACHE_FIX: %d\nDDRReadBandWidthLimit: %.2f\n"
               "DDRWriteBandWidthLimit: %.2f\nDDRTotalBandWidthLimit: %.2f\n"
               "AXISRAMReadBandWidthLimit: %.2f\nAXISRAMWriteBandWidthLimit: %.2f\n"
               "AXISRAMTotalBandWidthLimit: %.2f\nAXIBusReadBandWidthLimit: %.2f\n"
               "AXIBusWriteBandWidthLimit: %.2f\nAXIBusTotalBandWidthLimit: %.2f\n\n",
               context->nnConfig.unifiedFeature.vipCoefDecodePerf,
               context->nnConfig.customizedFeature.vipVectorPrune,
               context->nnConfig.unifiedFeature.vipCachedReadFromSram,
               context->nnConfig.unifiedFeature.vipImagePartialCache,
               context->nnConfig.customizedFeature.ddrReadBWLimit,
               context->nnConfig.customizedFeature.ddrWriteBWLimit,
               context->nnConfig.customizedFeature.ddrTotalBWLimit,
               context->nnConfig.customizedFeature.axiSramReadBWLimit,
               context->nnConfig.customizedFeature.axiSramWriteBWLimit,
               context->nnConfig.customizedFeature.axiSramTotalBWLimit,
               context->nnConfig.customizedFeature.axiBusReadBWLimit,
               context->nnConfig.customizedFeature.axiBusWriteBWLimit,
               context->nnConfig.customizedFeature.axiBusTotalBWLimit);

        vxInfo("HANDLE_ABBUFFER: %d\nHANDLE_SUBIMAGE: %d\nHANDLE_BRANCH: %d\n\n",
               (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_ALL || context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_AB) ? 1 : 0,
               (context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_ALL || context->options.enableSwtilingPhase1 == VX_SWTILING_OPTION_TILING) ? 1 : 0,
               (context->options.collectPerfType == 1) ? 1 : context->options.enableHandleBranch);

        vxInfo("FreqInMHZ: %u\nAxiClockFreqInMHZ: %u\nOutstandingTransfer: %d\nInternalWriteBWLimit: %.2f\n\n",
               context->nnConfig.customizedFeature.freqInMHZ,
               context->nnConfig.customizedFeature.axiClockFreqInMHZ,
               context->nnConfig.fixedFeature.maxOTNumber,
               (vx_float32)context->nnConfig.fixedFeature.nnLanesPerOutCycle);

        vxInfo("LanesPerConv: %u\nMaxTileSize: %u\nAxiSramSlowedDownByAddr: %d\nSLOW_NN_REQ_ARBITRATION_FIX: %d\n\n",
               context->nnConfig.unifiedFeature.lanesPerConv,
               context->nnConfig.unifiedFeature.maxTileSize,
               context->nnConfig.unifiedFeature.axiSramSlowedDownByAddr,
               context->nnConfig.unifiedFeature.slowNNReqArbitrationFix);

        vxInfo("FLOAT_XYDP_X: %u\nFLOAT_XYDP_Y: %u\nFLOAT_ZDP: %d\n",
               context->nnConfig.fixedFeature.nnFP16XYDPX,
               context->nnConfig.fixedFeature.nnFP16XYDPY,
               context->nnConfig.fixedFeature.nnFP16ZDP);

        vxInfo("SINGLE_PORT_ACC_BUFFER: %d\nMAX_ZRL_BIT_WIDTH: %d\nMAX_SOC_OUT_STANDING_NUMBER: %d\n\n",
            context->nnConfig.unifiedFeature.singlePortAccBuffer,
            context->nnConfig.fixedFeature.zrlBits,
            context->nnConfig.customizedFeature.maxSocOTNumber
            );

        vxInfo("SWTilingPhase3: %d\nAXI_SRAM_ONLY_SW_TILING: %d\n",
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE3) ? 1 : 0,
            context->nnConfig.unifiedFeature.axiSramOnlySWTiling
            );

        vxInfo("VIP_CORE_COUNT: %d\n",
            context->nnConfig.fixedFeature.vipCoreCount
            );

        vxInfo("DEPTH_WISE_SUPPORT: %d\nNN_WRITE_WITHOUT_USC: %d\n",
            context->nnConfig.customizedFeature.depthWiseSupport,
            context->nnConfig.customizedFeature.nnWriteWithoutUSC
            );

        vxInfo("EQUIVALENT_VIP_SRAM_WIDTH_IN_BYTE: %d\n",
            context->nnConfig.fixedFeature.equivalentVipsramWidthInByte
            );

        vxInfo("IMAGE_NOT_PACKED_IN_SRAM: %d\n",
            context->nnConfig.unifiedFeature.imageNotPackedInSram
            );

        vxInfo("NN_COEF_COMPRESSION_ENHANCEMENT: %d\nTP_COMPRESSION_ENHANCEMENT: %d\n",
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_COEF_COMPRESSION_ENHANCEMENT),
            vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_TP_COMPRESSION_ENHANCEMENT)
            );

        vxInfo("COEF_DELTA_CORD_OVER_FLOW_ZRL_8BIT_FIX: %d\n",
            context->nnConfig.unifiedFeature.coefDeltaCordOverFlowZRL8BitFix
            );

        vxInfo("NumShaderCores: %d\n",
            context->nnConfig.fixedFeature.shaderCoreCount
            );

        vxInfo("KERNEL_PER_CORE_LESS_THAN_THIRD_COEF_BUFF_DEPTH_FIX: %d\n",
            context->nnConfig.unifiedFeature.kernelPerCoreLTOneThirdCoefFix
            );

        vxInfo("LOW_EFFICIENCY_OF_ID_WRITE_IMGBUF_FIX: %d\n",
            context->nnConfig.unifiedFeature.lowEfficiencyOfIDWriteImgBufFix
            );

        vxInfo("DR_JD_Diff_For_Cacheline_Mode_FIX: %d\n",
            context->nnConfig.unifiedFeature.diffConditionForCachelineModePreFix
            );

        vxInfo("CONVOUT_FIFO_DEPTH_FIX: %d\n",
            context->nnConfig.unifiedFeature.convOutFifoDepthFix
            );

        vxInfo("\n");
        gOrigShowType = (vx_int8)context->options.collectPerfType;
    }

    vxInfo("\n");
    vxInfo("===========================\n");
    vxInfo("**********Show Perf********\n");
    vxInfo("===========================\n");

    vxInfo("layer_id:%d layer_name:%s\noperation_id:%d operation_name:%s operation_target:%s\n",
             layer->node->id, layer->name,
             op->id, vxnneGetOperatorTypeName(op->operatorType), vxnneGetOperatorTargetName(op->target));

    vxInfo("abs_op_id:%d\n", op->absoluteOperationID);

    vxInfo("upstream_layer_num:%d upstream_opertaion_num:%d\n",
             op->parentLayerNum, op->parentOpNum);

    for (i = 0; i < op->parentOpNum; i++)
    {
        vxInfo("%d) upstream_operation_id:%d uptream_operation_name:%s (upstream_layer_id:%d upstream_layer_name:%s)\n",
                 i, op->parentOps[i]->id,
                 vxnneGetOperatorTypeName(op->parentOps[i]->operatorType),
                 op->parentOps[i]->layer->node->id, op->parentOps[i]->layer->name);
    }

    vxInfo("downstream_layer_num:%d downstream_opertaion_num:%d\n",
             op->childLayerNum, op->childOpNum);

    for (i = 0; i < op->childOpNum; i++)
    {
        vxInfo("%d) downstream_operation_id:%d downstream_operation_name:%s (downstream_layer_id:%d downstream_layer_name:%s)\n",
                 i, op->childOps[i]->id,
                 vxnneGetOperatorTypeName(op->childOps[i]->operatorType),
                 op->childOps[i]->layer->node->id, op->childOps[i]->layer->name);
    }

    if (op->target == VXNNE_OPERATION_TARGET_SH ||
        op->target == VXNNE_OPERATION_TARGET_SW ||
        op->target == VXNNE_OPERATION_TARGET_SC)
    {
        return VX_SUCCESS;
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

    if (perf->opTarget == VXNNE_OPERATION_TARGET_NN)
    {
        vxInfo("NumUsedNNCores: %d\nConvOutFIFODepth: %d\n\n", perf->info.nnCores, perf->info.convOutFifoDepth);
    }

    if (perf->opType == VXNNE_OPERATOR_CONVOLUTION
        || perf->opType == VXNNE_OPERATOR_DEPTH_WISE_CONV
        || (perf->opTarget == VXNNE_OPERATION_TARGET_NN && perf->opType == VXNNE_OPERATOR_FULLYCONNECTED))
    {
        vxInfo("OrigInImageX: %d\nOrigInImageY: %d\nOrigInImageZ: %d\nNNOutImageX: %d (sub: %d)\nNNOutImageY: %d (sub: %d)\nNNOutImageZ: %d (sub: %d)\nFinalOutImageX: %d\nFinalOutImageY: %d\nFinalOutImageZ: %d\n",
                    perf->info.oinx, perf->info.oiny, perf->info.oinz,
                    perf->swTilingInfo.origInX, perf->info.inx,
                    perf->swTilingInfo.origInY, perf->info.iny,
                    perf->swTilingInfo.origOutZ, perf->info.outz,
                    perf->swTilingInfo.origOutX,
                    perf->swTilingInfo.origOutY,
                    perf->swTilingInfo.origOutZ);
    }
    else if (perf->opType == VXNNE_OPERATOR_POOLING)
    {
        vxInfo("InImageX: %d (sub: %d)\nInImageY: %d (sub: %d)\nInImageZ: %d (sub: %d)\nOutImageX: %d\nOutImageY: %d\nOutImageZ: %d\n",
                    perf->swTilingInfo.origInX, perf->info.inx,
                    perf->swTilingInfo.origInY, perf->info.iny,
                    perf->info.oinz, perf->info.outz,
                    perf->swTilingInfo.origOutX,
                    perf->swTilingInfo.origOutY,
                    perf->swTilingInfo.origOutZ);
    }
    else
    {
        vxInfo("InImageX: %d\nInImageY: %d\nInImageZ: %d\nOutImageX: %d (sub: %d)\nOutImageY: %d (sub: %d)\nOutImageZ: %d (sub: %d)\n",
                    perf->swTilingInfo.origInX,
                    perf->swTilingInfo.origInY,
                    perf->info.oinz,
                    perf->swTilingInfo.origOutX, perf->info.outx,
                    perf->swTilingInfo.origOutY, perf->info.outy,
                    perf->swTilingInfo.origOutZ, perf->info.outz);
    }

    vxInfo("KernelX: %d\nKernelY: %d\nKernelZ: %d\nPoolingSize: %d\nPoolingStride: %d\nInputDataSize: %d\nOutputDataSize: %d\nFP16: %d\n",
            perf->info.kx, perf->info.ky, perf->info.kz,
            perf->info.poolingSize, perf->info.poolingStride,
            perf->info.inputDataSize, perf->info.outputDataSize, perf->info.inputDataFormat == VX_TYPE_FLOAT16 ? 1 : 0);

    vxInfo("archModel_kernelSize: %u\nkernelSize: %u\n",
        _kernel_size_in_pixel_by_arch_perf(perf->opTarget, perf->opType, perf, context->nnConfig.unifiedFeature.fullCacheKernelHeadFix ? vx_true_e : vx_false_e),
        perf->info.kernelSize
        );

    vxInfo("SrcBuf: %s\nDstBuf: %s\nKernelBuf: %s\n",
            !perf->swTilingInfo.srcBuf ? "DDR" : (perf->swTilingInfo.srcBuf == SW_TILING_FROM_AXI_SRAM) ? "AXI_SRAM" : "VIP_SRAM",
            !perf->swTilingInfo.dstBuf ? "DDR" : (perf->swTilingInfo.dstBuf == SW_TILING_FROM_AXI_SRAM) ? "AXI_SRAM" : "VIP_SRAM",
            !perf->swTilingInfo.kernelBuf ? "DDR" : (perf->swTilingInfo.kernelBuf == SW_TILING_FROM_AXI_SRAM) ? "AXI_SRAM" : "VIP_SRAM");

    if ((context->options.collectPerfType == COLLECT_PERF_ESTIMATE) || profileMode)
    {
         _calcArchModelCacheMode(context, perf, &imageIdealCacheSizeInPixel);
          vxInfo("imageIdealCacheSizeInPixel: %d\n", imageIdealCacheSizeInPixel);
    }

    vxInfo("KernelCacheMode=%s\nImageCacheMode=%s\n",
             vxnneGetCacheModeName(perf->swTilingInfo.kernelCacheMode),
             vxnneGetCacheModeName(perf->swTilingInfo.imageCacheMode));

    vxInfo("xOffset: %d, yOffset: %d\n", perf->info.xOffSet, perf->info.yOffSet);

    if (perf->opType == VXNNE_OPERATOR_CONVOLUTION ||
        perf->opType == VXNNE_OPERATOR_DEPTH_WISE_CONV ||
        perf->opType == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        vxInfo("coefNonZeroRatio: %.15f\ncoefCompression: %.15f\nimageCompression: %.15f\nimageNonZeroRatio: %.15f\n\n",
                 perf->coefNonZeroRatio,
                 perf->coefCompressRatio,
                 perf->imageCompressRatio,
                 perf->imageNonZeroRatio
                );

        vxInfo("coefNonZeroRatio__llu: %llu\ncoefCompression_llu: %llu\nimageCompression_llu: %llu\nimageNonZeroRatio_llu: %llu\n\n",
                 *(vx_uint64 *)&perf->coefNonZeroRatio,
                 *(vx_uint64 *)&perf->coefCompressRatio,
                 *(vx_uint64 *)&perf->imageCompressRatio,
                 *(vx_uint64 *)&perf->imageNonZeroRatio
                );

        if (perf->opType == VXNNE_OPERATOR_CONVOLUTION ||
            perf->opType == VXNNE_OPERATOR_DEPTH_WISE_CONV ||
            (perf->opTarget == VXNNE_OPERATION_TARGET_NN && perf->opType == VXNNE_OPERATOR_FULLYCONNECTED))
        {
            vxInfo("OutImageTileXSize: %d\nOutImageTileYSize: %d\nKernelsPerCore: %d\n\n",
                     perf->resultInfo.outImageTileXSize,
                     perf->resultInfo.outImageTileYSize,
                     perf->resultInfo.kernelsPerCore
                  );
        }
    }
    else
    {
        vxInfo("\n");
    }

    vxInfo("kernelDDRReadBW: %llu\nInImageDDrReadBW: %llu\n",
             (vx_uint64)(perf->resultInfo.perfKernelReadBandWidth + 0.5f),
             (vx_uint64)(perf->resultInfo.perfInImageReadBandWidth + 0.5f));

    vxInfo("ReadBW: %llu\nWriteBW: %llu\nCycleCount: %llu\n\n",
             (vx_uint64)(perf->resultInfo.perfReadBandWidth + 0.5f),
             (vx_uint64)(perf->resultInfo.perfWriteBandWidth + 0.5f),
             (vx_uint64)(perf->resultInfo.perfCycleCount / mutiGpuFactor + 0.5f));

    return VX_SUCCESS;
}

static vx_uint32 _calcInImageInterleaveMode(vx_uint32 x, vx_uint32 max_tile_size, vx_uint32 kernel_xy, vx_bool vip7_fp16, vx_bool interleave8)
{
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

static vx_uint32 _calcOutImageInterleaveMode(vx_uint32 x, vx_uint32 max_tile_size, vx_bool vip7_fp16, vx_bool interleave8)
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

static vx_uint32 _calcImageInterleaveMode(vx_uint32 x, vx_uint32 mad_per_core, vx_uint32 kxy, vx_bool vip7_fp16, vx_bool interleave8)
{
    /*mad_per_core = 64;*/
    return gcmMIN(_calcOutImageInterleaveMode(x, mad_per_core, vip7_fp16, interleave8),
                  _calcInImageInterleaveMode(x, mad_per_core, kxy, vip7_fp16, interleave8));
}

static vx_float64 _calcPartialAlignedBW(vx_uint32 size, vx_uint32 ppc, vx_uint32 inc, vx_uint32 line_length, vx_uint32 line_phases, vx_uint32 base_addr, vx_int32 xoffset)
{
    vx_uint32 lineCount = 0, stepCount = 0, i, j;
    vx_float64 partialAlignedBW;
    for (j = 0; j < line_phases; j++)
    {
        vx_uint32 accum = ((base_addr + (j * ppc / line_phases)) % ppc) + xoffset;
        for (i = 0; i < line_length; i += size)
        {
            stepCount++;
            if ((accum + size) > (2 * ppc))
            {
                lineCount += 3;
            }
            else if ((accum + size) > ppc)
            {
                lineCount += 2;
            }
            else
            {
                lineCount += 1;
            }
            accum = (accum + inc) % ppc;
        }
    }

    partialAlignedBW = ((vx_float64)lineCount / stepCount) * ppc;
    return partialAlignedBW;
}

static vx_float64 _calcUnalignedBW(vx_float64 size, vx_float64 ppc)
{
    vx_float64 ret = ((vx_int32)((size - 1.0f) / ppc)
        + 1 + ((vx_int32)(size - 1.0f) % (vx_uint32)ppc ) / ppc)
        * ppc;
    return ret;
}

static vx_uint32 _calcNumOfKernel(vx_uint32 tile_x, vx_uint32 tile_y, vx_uint32 z, vx_uint32 accu_buf_depth, vx_uint32 cores, vx_uint32 interleave_mode, vx_uint32 zdp, vx_uint32 kx, vx_uint32 ky, vx_bool isV8, vx_uint32 data_size, vx_uint32 lanes_per_conv, vx_bool isDepthWise, vx_bool kernel_per_core_lt_one_third_coef_fix, vx_uint32 pooling_stride)
{
    vx_uint32 numKernel, numOfVZGroup;

    numKernel = (vx_uint32)(accu_buf_depth * interleave_mode / tile_y);

    if (isV8)
    {
        if ((kx == 1) && (ky == 1))
        {
            numKernel = (vx_uint32)(1.0f * accu_buf_depth / ceilf(1.0f * tile_y / interleave_mode));
        }
        else
        {
            vx_float32 tileVecNum = ceilf(1.0f * tile_x * tile_y / pooling_stride / lanes_per_conv);
            numKernel = (vx_uint32)((vx_float32)accu_buf_depth / (tileVecNum * pooling_stride));
        }
    }

    numKernel = gcmMIN(127, (vx_uint32)gcmMIN(numKernel, ceilf((vx_float32)z / cores)));
    if ((kx == 1) && (ky == 1) && (zdp != 1) && !isV8)
    {
        numKernel = (vx_uint32)(gcmMIN(numKernel, accu_buf_depth / 3));
    }
    if (isV8 && !kernel_per_core_lt_one_third_coef_fix)
    {
#define ZDP_LOOP_COUNT 3
        numKernel = (vx_uint32)(gcmMIN(numKernel, (vx_uint32)((2 * accu_buf_depth * ZDP_LOOP_COUNT / 3) / 3)));
    }

    numOfVZGroup = (vx_uint32)ceilf(1.0f * z / (numKernel * cores));
    numKernel = (vx_uint32)ceilf(1.0f * z / (numOfVZGroup * cores));
    return (vx_uint32)numKernel;
}

static vx_float64 _calcKernel4DSingleReadRepeated(vx_uint32 tile_x, vx_uint32 tile_y, vx_uint32 x, vx_uint32 y)
{
    return ceilf((vx_float32)x / tile_x) * ceilf((vx_float32)y / tile_y);
}

static vx_float64 _calcKernel4DSingleReadBW(vx_uint32 kx, vx_uint32 ky, vx_uint32 kz, vx_uint32 z, vx_float64 coef_compress_ratio)
{
    return (vx_float64)kx * ky * kz * z * coef_compress_ratio;
}

static vx_float64 _calcTile3DImageSingleReadRepeated(vx_uint32 z, vx_uint32 kernel_per_core, vx_uint32 cores, vx_bool is_depth_wise)
{
    return is_depth_wise ? 1.0f : ceilf((vx_float32)z / (kernel_per_core * cores));
}

static vx_float64 _calcTile3DImageSingleReadBW(
    vx_uint32 tile_x, vx_uint32 tile_y,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 x, vx_uint32 y,
    vx_uint32 inx, vx_uint32 iny,
    vx_uint32 brick_mode,
    vx_uint32 data_size,
    vx_float64 image_compress_ratio,
    vx_bool cache_line_mode_disabled,
    vx_bool async_copy_perf_fix,
    vx_bool accurate_tile_bw,
    vx_uint32 in_image_stride,
    vx_uint32 in_image_slice,
    vx_uint32 zdp,
    vx_uint32 mem_acc_unit_size_in_byte)
{
    vx_uint32 intileX, intileY;
    vx_float64 ppc, tile3DImageSingleReadBW;

    intileX = tile_x + kx - 1;
    intileY = tile_y + ky - 1;

    intileX = gcmMIN(intileX, in_image_stride);
    intileY = gcmMIN(intileY, (vx_uint32)ceilf((vx_float32)in_image_slice / in_image_stride));

    ppc = mem_acc_unit_size_in_byte / ((vx_float32)data_size / 8);

    if ((
        ((zdp == 1) || (kx != 1) || (ky != 1))
        && (inx == in_image_stride) && (inx * iny == in_image_slice)
        && (inx <= intileX) && (iny <= intileY) && !cache_line_mode_disabled
        )
        || brick_mode == 1)
    {
        tile3DImageSingleReadBW = _calcUnalignedBW((vx_float64)intileX * intileY * kz, ppc) * image_compress_ratio;
    }
    else
    {
        if (
            ((((in_image_stride % tile_x) == 0) && (((vx_uint32)ppc % tile_x) == 0) && (kx == 1))
             || (((in_image_stride % (vx_uint32)ppc) == 0) && (ppc >= inx)))
            && ((in_image_slice % (vx_uint32)ppc) == 0))
        {
            tile3DImageSingleReadBW = ceilf(intileX / (vx_float32)ppc) * intileY * ppc * kz * image_compress_ratio;
        }
        else if (accurate_tile_bw && ((in_image_stride % (vx_uint32)ppc) == 0) && ((in_image_slice % in_image_stride) == 0))
        {
            tile3DImageSingleReadBW = _calcPartialAlignedBW(intileX, (vx_uint32)ppc, tile_x, inx, 1, 0, 0) * intileY * kz * image_compress_ratio;
        }
        else if (in_image_stride == intileX)
        {
            /*Async Copy can always merge requests when inimage_stride = tile_xsize*/
            if (((in_image_stride * intileY) % (vx_uint32)ppc) == 0 && (in_image_slice % (vx_uint32)ppc) == 0)
            {
                tile3DImageSingleReadBW = ceilf(intileX * intileY / (vx_float32)ppc) * ppc * kz * image_compress_ratio;
            }
            else
            {
                tile3DImageSingleReadBW = _calcUnalignedBW((vx_float32)intileX * intileY, ppc) * kz * image_compress_ratio;
            }
        }
        else if (accurate_tile_bw && ((in_image_stride % (vx_uint32)(ppc / 2)) == 0) && ((in_image_slice % in_image_stride) == 0))
        {
            tile3DImageSingleReadBW = _calcPartialAlignedBW(intileX, (vx_uint32)ppc, tile_x, inx, 2, 0, 0) * intileY * kz * image_compress_ratio;
        }
        else if (accurate_tile_bw && ((in_image_stride % (vx_uint32)(ppc / 4)) == 0) && ((in_image_slice % in_image_stride) == 0))
        {
             tile3DImageSingleReadBW = _calcPartialAlignedBW(intileX, (vx_uint32)ppc, tile_x, inx, 4, 0, 0) * intileY * kz * image_compress_ratio;
        }
        else
        {
            tile3DImageSingleReadBW = _calcUnalignedBW((vx_float32)intileX, ppc) * intileY * kz * image_compress_ratio;
        }

        if (async_copy_perf_fix)
        {
            if (((in_image_slice % (vx_uint32)ppc) == 0) && (((in_image_stride * intileY) % (vx_uint32)ppc) == 0))
            {
                tile3DImageSingleReadBW = gcmMIN(in_image_stride * intileY * kz * image_compress_ratio, tile3DImageSingleReadBW);
            }
            else
            {
                tile3DImageSingleReadBW = gcmMIN(_calcUnalignedBW((vx_float32)in_image_stride * intileY, ppc) * kz * image_compress_ratio, tile3DImageSingleReadBW);
            }
        }
    }
    return tile3DImageSingleReadBW;
}

static vx_float64 _calcKernelCachePercentage(
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 z,
    vx_uint32 cores,
    vx_float64 coef_compress_ratio,
    vx_float64 cache_size_in_pixel,
    vx_bool full_cach_kernel_head_fix,
    vx_bool is_depth_wise)
{
    vx_float64 kernelIdealCache, kernelNonIdealCache;
    vx_float64 zPerCore = ceilf((vx_float32)z/cores);
    vx_float64 adjCacheSizeInPixel, result = 1.0f;

    if (is_depth_wise)
    {
        kernelIdealCache = _calcKernel4DSingleReadBW(kx, ky, 1, z, 1);
        kernelNonIdealCache = ceilf((vx_float32)_calcKernel4DSingleReadBW(kx, ky, 1, (vx_uint32)zPerCore, 1)/AXI_BURST_SIZE) * AXI_BURST_SIZE * cores;
    }
    else
    {
        kernelIdealCache = _calcKernel4DSingleReadBW(kx, ky, kz, z, coef_compress_ratio);
        kernelNonIdealCache = ceilf((vx_float32)_calcKernel4DSingleReadBW(kx, ky, kz, (vx_uint32)zPerCore, coef_compress_ratio)/AXI_BURST_SIZE) * AXI_BURST_SIZE * cores;
    }

    if (full_cach_kernel_head_fix)
    {
        adjCacheSizeInPixel = cache_size_in_pixel;
    }
    else
    {
        adjCacheSizeInPixel = cache_size_in_pixel * kernelIdealCache / kernelNonIdealCache;
    }
    if (kernelIdealCache > adjCacheSizeInPixel)
    {
        result = (1.0f * gcmMIN(kernelIdealCache, adjCacheSizeInPixel) / kernelIdealCache);
    }

     return result;
}

static vx_float64 _calcKernelReadBandWidth(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kernel_per_core,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 x, vx_uint32 y, vx_uint32 z,
    vx_uint32 inx, vx_uint32 iny,
    vx_uint32 cores,
    vx_uint32 brick_mode,
    vx_uint32 data_size,
    vx_float64 coef_compress_ratio,
    vx_float64 image_compress_ratio,
    vx_float64 cache_size_in_pixel,
    vx_bool full_cach_kernel_head_fix,
    vx_bool is_depth_wise,
    vx_float64 *kernel_read_bw_tile0)
{
    vx_float64 kernelIdealCache, kernelRepeatRead, kernelReadBandWidth, kernelNonIdealCache, kernelReadBandWidthTile0;
    vx_float64 zPerCore = ceilf((vx_float32)z/cores);
    vx_float64 adjCacheSizeInPixel;
    vx_uint32 kernelHeaderReadBandWidth = 0;
    kernelRepeatRead = _calcKernel4DSingleReadRepeated(tile_x, tile_y, x, y);
    if (is_depth_wise)
    {
        kernelIdealCache = _calcKernel4DSingleReadBW(kx, ky, 1, z, 1);
        kernelNonIdealCache = ceilf((vx_float32)_calcKernel4DSingleReadBW(kx, ky, 1, (vx_uint32)zPerCore, 1)/AXI_BURST_SIZE) * AXI_BURST_SIZE * cores;
    }
    else
    {
        kernelIdealCache = _calcKernel4DSingleReadBW(kx, ky, kz, z, coef_compress_ratio);
        kernelNonIdealCache = ceilf((vx_float32)_calcKernel4DSingleReadBW(kx, ky, kz, (vx_uint32)zPerCore, coef_compress_ratio)/AXI_BURST_SIZE) * AXI_BURST_SIZE * cores;
    }
    if (full_cach_kernel_head_fix)
    {
        adjCacheSizeInPixel = cache_size_in_pixel;
        kernelHeaderReadBandWidth = 0;
    }
    else
    {
        adjCacheSizeInPixel = cache_size_in_pixel * kernelIdealCache / kernelNonIdealCache;
        kernelHeaderReadBandWidth = AXI_BURST_SIZE;
    }
    kernelReadBandWidthTile0 = (kernelIdealCache + kernelHeaderReadBandWidth) * (data_size / 8);
    kernelReadBandWidth = ((kernelIdealCache + kernelHeaderReadBandWidth) * kernelRepeatRead) - (gcmMIN(kernelIdealCache, adjCacheSizeInPixel) * (kernelRepeatRead - 1));
    kernelReadBandWidth *= (data_size / 8);
    if (NULL != kernel_read_bw_tile0) *kernel_read_bw_tile0 = kernelReadBandWidthTile0;

    return kernelReadBandWidth;
}

static vx_float64 _calcImageReadBandWidth(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kernel_per_core,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 x, vx_uint32 y, vx_uint32 z,
    vx_uint32 inx, vx_uint32 iny,
    vx_uint32 cores,
    vx_uint32 brick_mode,
    vx_uint32 data_size,
    vx_float64 coef_compress_ratio,
    vx_float64 image_compress_ratio,
    vx_float64 cache_size_in_pixel,
    vx_bool image_not_packed_in_sram,
    vx_bool cache_line_mode_disabled,
    vx_bool async_copy_perf_fix,
    vx_bool accurate_tile_bw,
    vx_bool is_depth_wise,
    vx_uint32 equivalent_vip_sram_width_in_byte,
    vx_uint32 in_image_stride,
    vx_uint32 in_image_slice,
    vx_uint32 zdp,
    vx_uint32 mem_acc_unit_size_in_byte,
    vx_bool image_partial_cache,
    vx_float64 *image_read_bw_vzgroup0)
{
    vx_float64 imageRepeatSingleRead, imageRepeatRead, imageRepeatCacheRead, imageIdealCache, imageReadBandWidth, imageTile3DBW, imageReadBandWidthVZGroup0;
    vx_float64 tmp;
    vx_uint32 intile_x = (tile_x + kx - 1);
    vx_uint32 intile_y = (tile_y + ky - 1);
    intile_x = gcmMIN(intile_x, inx);
    intile_y = gcmMIN(intile_y, iny);

    tmp = ((vx_float64)x / tile_x) * ((vx_float64)y / tile_y);
    imageRepeatSingleRead = _calcTile3DImageSingleReadRepeated(z, kernel_per_core, cores, is_depth_wise);
    imageRepeatRead = imageRepeatSingleRead * tmp;
    imageRepeatCacheRead = (imageRepeatSingleRead - 1.0f) * tmp;
    imageTile3DBW = _calcTile3DImageSingleReadBW(tile_x, tile_y, kx, ky, kz, x, y, inx, iny, brick_mode, data_size, image_compress_ratio,
                          cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, in_image_stride, in_image_slice, zdp, mem_acc_unit_size_in_byte);
    if (image_not_packed_in_sram)
    {
        imageIdealCache = ceilf(ceilf((vx_float32)intile_x * intile_y / 16) * 16 * kz / (equivalent_vip_sram_width_in_byte * 2))
                         * (equivalent_vip_sram_width_in_byte * 2) * data_size / 8;
    }
    else
    {
        imageIdealCache = (vx_float32)intile_x * intile_y * kz * data_size / 8;
    }
    if (!image_partial_cache && (imageIdealCache > cache_size_in_pixel))
    {
        imageReadBandWidthVZGroup0 = imageTile3DBW * (imageRepeatRead -imageRepeatCacheRead);
        imageReadBandWidth = imageTile3DBW * imageRepeatRead;
    }
    else
    {
        imageReadBandWidthVZGroup0 = imageTile3DBW * (imageRepeatRead -imageRepeatCacheRead);
        imageReadBandWidth = imageTile3DBW * (imageRepeatRead - (gcmMIN(imageIdealCache, cache_size_in_pixel) * imageRepeatCacheRead / imageIdealCache));
    }
    imageReadBandWidth = imageReadBandWidth * (data_size / 8);

    if (NULL != image_read_bw_vzgroup0) *image_read_bw_vzgroup0 = imageReadBandWidthVZGroup0;

    return imageReadBandWidth;
}

static vx_float32 _calcImageIdealCacheInPixel(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 x, vx_uint32 y,
    vx_int32 xoffset, vx_int32 yoffset,
    vx_uint32 sub_x, vx_uint32 sub_y,
    vx_uint32 data_size,
    vx_bool image_not_packed_in_sram,
    vx_uint32 equivalent_vip_sram_width_in_byte)
{
    vx_float32 imageIdealCache;
    vx_uint32 inx = x + kx - 1 + 2 * xoffset;
    vx_uint32 iny = y + ky - 1 + 2 * yoffset;
    vx_uint32 inSIX = gcmMIN(inx, sub_x + kx - 1);
    vx_uint32 inSIY = gcmMIN(iny, sub_y + ky - 1);

    vx_uint32 intile_x = (tile_x + kx - 1);
    vx_uint32 intile_y = (tile_y + ky - 1);
    intile_x = gcmMIN(intile_x, inSIX);
    intile_y = gcmMIN(intile_y, inSIY);

    if (image_not_packed_in_sram)
    {
        imageIdealCache = ceilf(ceilf((vx_float32)intile_x * intile_y / 16) * 16 * kz / (equivalent_vip_sram_width_in_byte * 2))
                         * (equivalent_vip_sram_width_in_byte * 2) * data_size / 8;
    }
    else
    {
        imageIdealCache = (vx_float32)intile_x * intile_y * kz * data_size / 8;
    }

    return imageIdealCache;
}

static vx_float64 _calcReadBandWidth(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kernel_per_core,
    vx_uint32 kx, vx_uint32 ky, vx_uint32 kz,
    vx_uint32 x, vx_uint32 y, vx_uint32 z,
    vx_uint32 inx, vx_uint32 iny,
    vx_uint32 cores,
    vx_uint32 brick_mode,
    vx_uint32 data_size,
    vx_float64 coef_compress_ratio,
    vx_float64 image_compress_ratio,
    vx_uint32 l2cache_size,
    vx_bool image_partial_cache,
    vx_uint32 nn_cmd_size,
    vx_bool image_not_packed_in_sram,
    vx_bool full_cach_kernel_head_fix,
    vx_bool cache_line_mode_disabled,
    vx_bool async_copy_perf_fix,
    vx_bool accurate_tile_bw,
    vx_bool is_depth_wise,
    vx_uint32 in_image_stride,
    vx_uint32 in_image_slice,
    vx_uint32 zdp,
    vx_uint32 mem_acc_unit_size_in_byte,
    vx_uint32 equivalent_vip_sram_width_in_byte,
    vx_float64 *ddrKernelReadBW,
    vx_float64 *ddrInImageReadBW,
    vx_float64 *kernel_read_bw_tile0,
    vx_float64 *image_read_bw_vzgroup0,
    vx_arch_model_bw_cost_s *bw_cost_detail)
{
    vx_float64 cacheSizeInPixel, kernelIdealCache, imageIdealCache, kernelReadBandWidthTile0, imageReadBandWidthVZGroup0;
    vx_float64 kernelRepeatRead, imageRepeatSingleRead, imageRepeatRead, readBandWidth = 0, kernelNonIdealCache, kernelStorage;
    vx_float64 kernelReadBW, inImageReadBW;
    vx_float32 tmp, zPerCore;
    vx_uint32 intile_x = (tile_x + kx - 1);
    vx_uint32 intile_y = (tile_y + ky - 1);
    intile_x = gcmMIN(intile_x, inx);
    intile_y = gcmMIN(intile_y, iny);

    tmp = ((vx_float32)x / tile_x) * ((vx_float32)y / tile_y);
    cacheSizeInPixel = l2cache_size / ((vx_float32)data_size / 8);

    kernelRepeatRead = _calcKernel4DSingleReadRepeated(tile_x, tile_y, x, y);
    imageRepeatSingleRead = _calcTile3DImageSingleReadRepeated(z, kernel_per_core, cores, is_depth_wise);
    imageRepeatRead = imageRepeatSingleRead * tmp;
    zPerCore = ceilf((vx_float32)z / cores);
    if (is_depth_wise)
    {
        kernelIdealCache = _calcKernel4DSingleReadBW(kx, ky, 1, z, 1);
        kernelNonIdealCache = ceilf((vx_float32)_calcKernel4DSingleReadBW(kx, ky, 1, (vx_uint32)zPerCore, 1) / AXI_BURST_SIZE) * AXI_BURST_SIZE * cores;
    }
    else
    {
        kernelIdealCache = _calcKernel4DSingleReadBW(kx, ky, kz, z, coef_compress_ratio);
        kernelNonIdealCache = ceilf((vx_float32)_calcKernel4DSingleReadBW(kx, ky, kz, (vx_uint32)zPerCore, coef_compress_ratio) / AXI_BURST_SIZE) * AXI_BURST_SIZE * cores;
    }
    kernelStorage = full_cach_kernel_head_fix ? kernelIdealCache : kernelNonIdealCache;
    if (image_not_packed_in_sram)
    {
        imageIdealCache = ceilf(ceilf((vx_float32)intile_x * intile_y / 16) * 16 * kz / (equivalent_vip_sram_width_in_byte * 2))
                         * (equivalent_vip_sram_width_in_byte * 2) * data_size / 8;
    }
    else
    {
        imageIdealCache = (vx_float32)intile_x * intile_y * kz * data_size / 8;
    }
    if (image_partial_cache)
    {
        if (kernelRepeatRead >= imageRepeatRead)
        {
            kernelReadBW = _calcKernelReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny, cores, brick_mode, data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, full_cach_kernel_head_fix, is_depth_wise, &kernelReadBandWidthTile0);
            cacheSizeInPixel = cacheSizeInPixel - gcmMIN(kernelStorage, cacheSizeInPixel);
            inImageReadBW = _calcImageReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, image_not_packed_in_sram,
                cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, zdp, mem_acc_unit_size_in_byte, image_partial_cache, &imageReadBandWidthVZGroup0);
        }
        else
        {

            inImageReadBW = _calcImageReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, image_not_packed_in_sram,
                cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, zdp, mem_acc_unit_size_in_byte, image_partial_cache, &imageReadBandWidthVZGroup0);
            cacheSizeInPixel = cacheSizeInPixel - gcmMIN(imageIdealCache, cacheSizeInPixel);
            kernelReadBW = _calcKernelReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, full_cach_kernel_head_fix, is_depth_wise, &kernelReadBandWidthTile0);
        }
    }
    else
    {
        if (imageIdealCache < cacheSizeInPixel)
        {
            inImageReadBW = _calcImageReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, image_not_packed_in_sram,
                cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, zdp, mem_acc_unit_size_in_byte, image_partial_cache, &imageReadBandWidthVZGroup0);
            cacheSizeInPixel = cacheSizeInPixel - gcmMIN(imageIdealCache, cacheSizeInPixel);
            kernelReadBW = _calcKernelReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny, cores, brick_mode, data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, full_cach_kernel_head_fix, is_depth_wise, &kernelReadBandWidthTile0);
        }
        else
        {
            inImageReadBW = _calcImageReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, data_size, coef_compress_ratio, image_compress_ratio, 0, image_not_packed_in_sram,
                cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, zdp, mem_acc_unit_size_in_byte, image_partial_cache, &imageReadBandWidthVZGroup0);
            kernelReadBW = _calcKernelReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, full_cach_kernel_head_fix, is_depth_wise, &kernelReadBandWidthTile0);
        }
    }
    if (ddrKernelReadBW) *ddrKernelReadBW = kernelReadBW;
    if (ddrInImageReadBW) *ddrInImageReadBW = inImageReadBW;
    readBandWidth = nn_cmd_size + kernelReadBW + inImageReadBW;
    if (NULL != bw_cost_detail)
    {
        bw_cost_detail->cost = readBandWidth;
        bw_cost_detail->tile0VZGroup0 = nn_cmd_size + kernelReadBandWidthTile0 * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) +  imageReadBandWidthVZGroup0 * (1.0f * tile_x / x) * (1.0f * tile_y / y);
        bw_cost_detail->tile0 = nn_cmd_size + kernelReadBandWidthTile0 + inImageReadBW * (1.0f * tile_x / x) * (1.0f * tile_y / y);
        bw_cost_detail->vzGroup0 = nn_cmd_size + kernelReadBW * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) + imageReadBandWidthVZGroup0;
    }
    if (NULL != kernel_read_bw_tile0) *kernel_read_bw_tile0 = kernelReadBandWidthTile0;
    if (NULL != image_read_bw_vzgroup0) *image_read_bw_vzgroup0 = imageReadBandWidthVZGroup0;
    return readBandWidth;
}

static vx_float64 _calcWriteBandWidth(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 x,
    vx_uint32 y,
    vx_uint32 z,
    vx_uint32 data_size,
    vx_float64 image_compress_ratio,
    vx_uint32 usc_cache_size,
    vx_uint32 pooling_stride,
    vx_uint32 out_image_stride,
    vx_uint32 out_image_slice,
    vx_bool is_nn_write_without_usc)
{
    vx_float32 ppc, poolX, poolY, poolTileX, poolTileY, hGap, vGap, hGapSubImg, vGapSubImg;
    vx_float64 cacheSizeInPixel;
    vx_float64 allTilesBW, rowTilesBW, tileBW, writeBW;

    cacheSizeInPixel = usc_cache_size * 1024 / ((vx_float64)data_size / 8);
    ppc = AXI_BURST_SIZE / ((vx_float32)data_size / 8);

    gcmASSERT(x >= pooling_stride);
    gcmASSERT(y >= pooling_stride);

    poolX = (vx_float32)x / pooling_stride;
    poolY = (vx_float32)y / pooling_stride;
    poolTileX = (vx_float32)tile_x / pooling_stride;
    poolTileY = (vx_float32)tile_y / pooling_stride;

    hGapSubImg = out_image_stride - poolX;
    vGapSubImg = out_image_slice - poolY * out_image_stride + hGapSubImg;
    if (vGapSubImg < ppc)
    {
        allTilesBW = _calcUnalignedBW((double)out_image_slice * z, ppc) * image_compress_ratio;
    }
    else if (hGapSubImg < ppc)
    {
        allTilesBW = _calcUnalignedBW((double)poolY * out_image_stride - hGapSubImg, ppc) * z * image_compress_ratio;
    }
    else
    {
        allTilesBW = _calcUnalignedBW((double)poolX, ppc) * poolY * z * image_compress_ratio;
    }
    if (( (((vx_uint32)(out_image_stride * poolTileY) % (vx_uint32)ppc) == 0) || (((vx_uint32)ppc % (vx_uint32)(out_image_stride * poolTileY)) == 0))
        && (poolTileX == poolX)
        && ((out_image_slice % (vx_uint32)ppc) == 0))
    {
        rowTilesBW = ceilf((vx_float32)out_image_stride * poolTileY / ppc) * ppc * z * image_compress_ratio;
    }
    else
    {
        rowTilesBW = _calcUnalignedBW((vx_float32)out_image_stride * poolTileY, ppc) * z * image_compress_ratio;
    }

    if (((((out_image_stride % (vx_uint32)poolTileX) == 0) && (((vx_uint32)ppc % (vx_uint32)poolTileX) == 0))
          || (((out_image_stride % (vx_uint32)ppc) == 0) && (ppc >= poolX)))
       && ((out_image_slice % (vx_uint32)ppc) == 0))
    {
        tileBW     = ceilf(poolTileX / ppc) * ppc * poolTileY * z * image_compress_ratio;
    }
    else
    {
        tileBW     = _calcUnalignedBW(poolTileX, ppc) * poolTileY * z * image_compress_ratio;
    }

    hGap = (vx_float32)out_image_stride - poolTileX;
    vGap = (vx_float32)out_image_slice - poolTileY * out_image_stride + hGap;

    if (!is_nn_write_without_usc && (tileBW < (cacheSizeInPixel / 2) || rowTilesBW < (cacheSizeInPixel / 2)))
    {
        writeBW = allTilesBW;
    }
    else if (!is_nn_write_without_usc && (tileBW < cacheSizeInPixel || rowTilesBW < cacheSizeInPixel))
    {
        writeBW = allTilesBW * 1.2f;
    }
    else if (vGap < ppc)
    {
        writeBW = allTilesBW * poolX * poolY / (poolTileX * poolTileY);
    }
    else if (hGap < ppc)
    {
        writeBW = rowTilesBW * poolX * poolY / (poolTileX * poolTileY);
    }
    else
    {
        writeBW = tileBW * poolX * poolY / (poolTileX * poolTileY);
    }

    writeBW = writeBW * (data_size / 8);
    return writeBW;
}

vx_float64 _calcComputeCycleCount(
    vx_uint32 tile_x,
    vx_uint32 tile_y,
    vx_uint32 kernel_per_core,
    vx_uint32 kx,
    vx_uint32 ky,
    vx_uint32 kz,
    vx_uint32 x,
    vx_uint32 y,
    vx_uint32 z,
    vx_float64 non_zero_ratio,
    vx_uint32 xydp_x,
    vx_uint32 xydp_y,
    vx_uint32 zdp,
    vx_uint32 float_xydp_x,
    vx_uint32 float_xydp_y,
    vx_uint32 float_zdp,
    vx_uint32 data_size,
    vx_uint32 vector_prune,
    vx_uint32 interleave_mode,
    vx_uint32 lanes_per_conv,
    vx_uint32 coef_decode_perf,
    vx_uint32 zrl_bits,
    vx_bool is_depth_wise,
    vx_bool vip_v7_16bit,
    vx_bool fp16,
    vx_bool conv1x1_half_performance,
    vx_bool zxdp3_kernel_read_conflict_fix,
    vx_bool single_port_acc_buffer,
    vx_float64 *refined_non_zero_ratio
    )
{
    vx_uint32 tmp, pipeLatency;
    /*vx_int32 dpAmount;*/
    vx_uint32 dpKX, dpKY, dpKZ;
    vx_float64 accumCycle, tile3DComputeCycle, bottomTile3DComputeCycle;
    vx_float64 dpNonZeroRatio = 1.0;
    vx_float64 computeCycle;
    vx_uint32 xydpVectorPruneAmount, zdpVectorPruneAmount, numOfPruneGroupsInDpn, numOfDpnInImgBuf;
    vx_uint32 selected_xydp_x, selected_xydp_y, selected_zdp;
    vx_uint32 vip7Version = 0, rotate_cell = 0, vip_v7_fc = 0, vip_v7_dp6 = 0, vip_v7_xdp3 = 0, vip_v7_zdp3 = 0;
    if (xydp_x == 0 || xydp_y == 0) /*zdp only arch*/
    {
        tile3DComputeCycle = ceilf(((vx_float32)tile_y * tile_x) / lanes_per_conv) * kernel_per_core;
        bottomTile3DComputeCycle = ceilf((1.0f * (y % tile_y) * tile_x) / lanes_per_conv) * kernel_per_core;

        if (vector_prune > 0)
        {
            dpNonZeroRatio = non_zero_ratio;
        }
        else
        {
            for (tmp = 0; tmp < zdp; tmp++)
            {
                dpNonZeroRatio *= (1.0f - non_zero_ratio);
            }
            dpNonZeroRatio = 1.0f - dpNonZeroRatio;
        }
        computeCycle = tile3DComputeCycle *(vx_uint32)(y / tile_y) + bottomTile3DComputeCycle;
        if (is_depth_wise)
        {
            computeCycle = computeCycle * ceilf((vx_float32)kx * ky / zdp) * z * ceilf((vx_float32)x / tile_x) / kernel_per_core;
        }
        else
        {
            computeCycle = computeCycle * ceilf((vx_float32)kx * ky * kz / zdp) * z * dpNonZeroRatio * ceilf((vx_float32)x / tile_x) / kernel_per_core;
        }

        if (data_size == 16) /*INT16 only; no FP16 support*/
        {
            computeCycle *= 4;
        }
        *refined_non_zero_ratio = non_zero_ratio;
        return computeCycle;
    }
    else
    {
        vip7Version = (vip_v7_16bit == 0 && fp16 == 1) ? 0 : 1;
    }

    vip_v7_fc = (vip7Version && (kx == 1) && (ky != 1)) ? 1 : 0;
    vip_v7_dp6 = (vip7Version && (xydp_x == 3) && (xydp_y == 2)) ? 1 : 0;
    vip_v7_xdp3 = (vip7Version && (xydp_x == 3) && (xydp_y == 1)) ? 1 : 0;
    vip_v7_zdp3 = (vip7Version && (zdp == 3) && (kx * ky == 1)) ? 1 : 0;
    if (vip_v7_fc || vip_v7_zdp3 || (vip_v7_dp6 && (kx == 3) && (ky == 3)) || (vip_v7_xdp3 && (kx == 2) && (ky == 2)))
    {
        rotate_cell = 2;
    }
    else if ((vip_v7_dp6 && (kx == 2) && (ky == 2)) || ((zdp == 1) && (kx * ky == 1)) || (xydp_x == 1))
    {
        rotate_cell = 1;
    }
    else
    {
        rotate_cell = 3;
    }

    pipeLatency = (rotate_cell == 1) ? 4 : 6;
    if (vip_v7_16bit)
    {
        pipeLatency += 2;
    }

    accumCycle = ceilf((vx_float32)tile_y / interleave_mode);
    if ((zdp == 1) && (xydp_x == 1) && (xydp_y == 1) && (accumCycle == 4))
    {
        tile3DComputeCycle = 4 * (vx_float32)kernel_per_core;
    }
    else if ((rotate_cell == 1) || (vip_v7_zdp3 == 1) || (vip7Version == 0))
    {
        tile3DComputeCycle = gcmMAX(ceilf((vx_float32)tile_y / interleave_mode) * kernel_per_core * rotate_cell, pipeLatency);
    }
    else
    {
        tile3DComputeCycle = gcmMAX(ceilf((vx_float32)tile_y / interleave_mode) * rotate_cell, pipeLatency) * kernel_per_core;
    }

    tmp = y % tile_y;
    if (tmp != 0)
    {
        accumCycle = ceilf((vx_float32)tmp / interleave_mode);
        if ((zdp == 1) && (xydp_x == 1) && (xydp_y == 1) && (accumCycle == 4))
        {
            bottomTile3DComputeCycle = 4 * (vx_float32)kernel_per_core;
        }
        else if ((rotate_cell == 1) || (vip_v7_zdp3 == 1) || (vip7Version == 0))
        {
            bottomTile3DComputeCycle = gcmMAX(ceilf((vx_float32)tmp / interleave_mode) * kernel_per_core * rotate_cell, pipeLatency);
        }
        else
        {
            bottomTile3DComputeCycle = gcmMAX(ceilf((vx_float32)tmp / interleave_mode) * rotate_cell, pipeLatency) * kernel_per_core;;
        }
    }
    else
    {
        bottomTile3DComputeCycle = 0;
    }

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
        dpKX = (vx_uint32)ceilf((vx_float32)kx / selected_xydp_x);
        dpKY = (vx_uint32)ceilf((vx_float32)ky / selected_xydp_y);
        dpKZ = kz;
        /*dpAmount = selected_xydp_x * selected_xydp_y;*/
        numOfPruneGroupsInDpn = (vx_uint32)ceilf((vx_float32)selected_xydp_x * selected_xydp_y / xydpVectorPruneAmount);
        numOfDpnInImgBuf = (vx_uint32)ceilf((vx_float32)kx / selected_xydp_x) * (vx_uint32)ceilf((vx_float32)ky / selected_xydp_y);

        if (vector_prune > 0)
        {
            non_zero_ratio = gcmMAX(non_zero_ratio, (vx_float64)((vx_float64)gcmMIN(selected_xydp_x, kx) * gcmMIN(selected_xydp_y, ky) / (pow(2, zrl_bits) - 1)));
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
                dpKZ = (vx_uint32)ceilf((vx_float32)kz / (2 * selected_zdp)) * 2;
                /*dpAmount = 2 * selected_zdp;*/
                numOfPruneGroupsInDpn = (vx_uint32)ceilf((vx_float32)2 * selected_zdp / zdpVectorPruneAmount);
                numOfDpnInImgBuf = 2;
            }
            else
            {
                dpKZ = (vx_uint32)ceilf((vx_float32)kz / selected_zdp);
                /*dpAmount = selected_zdp;*/
                numOfPruneGroupsInDpn = (vx_uint32)ceilf((vx_float32)selected_zdp / zdpVectorPruneAmount);
                numOfDpnInImgBuf = 1;
            }
        }
        else
        {
            dpKZ = kz;
            /*dpAmount = 1;*/
            numOfPruneGroupsInDpn = 1;
            numOfDpnInImgBuf = 1;
        }
        if (vector_prune > 0)
        {
            non_zero_ratio = gcmMAX(non_zero_ratio, (vx_float64)selected_zdp / ((vx_float64)pow(2, zrl_bits) - 1));
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
        vx_float64 probExactlyOneNonZeroDpn, tmpRatio = 1.0f;
        for (tmp = 0; tmp < (numOfDpnInImgBuf - 1); tmp++)
        {
            tmpRatio *= (1.0f - dpNonZeroRatio);
        }
        probExactlyOneNonZeroDpn = dpNonZeroRatio * tmpRatio;
        dpNonZeroRatio = dpNonZeroRatio + probExactlyOneNonZeroDpn;
    }

    if (vip_v7_16bit && (fp16 == 0))
    {
        {
            dpKY = ky;
            dpKZ = kz;
        }
    }

    computeCycle = tile3DComputeCycle * (vx_int32)(y / tile_y) + bottomTile3DComputeCycle;
    if (is_depth_wise)
    {
        computeCycle = computeCycle * ceilf((vx_float32)dpKX * dpKY / rotate_cell) * z * ceilf((vx_float32)x / tile_x) / kernel_per_core;
    }
    else
    {
        if ((vip7Version == 0) || (vip_v7_zdp3 == 1))
        {
            computeCycle = computeCycle * ceilf((vx_float32)dpKX * dpKY * dpKZ / rotate_cell) * z * dpNonZeroRatio * ceilf((vx_float32)x / tile_x) / kernel_per_core;
        }
        else
        {
            computeCycle = computeCycle * ceilf((vx_float32)dpKX * dpKY / rotate_cell) * dpKZ * z * dpNonZeroRatio * ceilf((vx_float32)x / tile_x) / kernel_per_core;
        }
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
    return computeCycle;
}


static vx_float64 _calcNNCycleCountBandWidth(
    vx_uint32  tile_x,
    vx_uint32  tile_y,
    vx_uint32  kernel_per_core,
    vx_uint32  x,
    vx_uint32  y,
    vx_uint32  z,
    vx_uint32  kx,
    vx_uint32  ky,
    vx_uint32  kz,
    vx_uint32  inx,
    vx_uint32  iny,
    vx_uint32  pooling_stride,
    vx_float64 non_zero_ratio,
    vx_float64 coef_compress_ratio,
    vx_float64 image_compress_ratio,
    vx_uint32  cores,
    vx_uint32  brick_mode,
    vx_uint32  input_data_size,
    vx_uint32  output_data_size,
    vx_uint32  l2cache_size,
    vx_uint32  l2cache_width,
    vx_uint32  xydp_x,
    vx_uint32  xydp_y,
    vx_uint32  zdp,
    vx_uint32  float_xydp_x,
    vx_uint32  float_xydp_y,
    vx_uint32  float_zdp,
    vx_uint32  usc_cache_size,
    vx_uint32  nn_cmd_size,
    vx_uint32  coef_decode_perf,
    vx_uint32  vector_prune,
    vx_uint32  image_partial_cache,
    vx_uint32  data_read_from_sram,
    vx_uint32  first_cmd,
    vx_uint32  src_buf,
    vx_uint32  dst_buf,
    vx_uint32  kernel_buf,
    vx_float32 axi_sram_read_bw_limit,
    vx_float32 axi_sram_write_bw_limit,
    vx_float32 axi_sram_total_bw_limit,
    vx_float32 axi_bus_read_bw_limit,
    vx_float32 axi_bus_write_bw_limit,
    vx_float32 axi_bus_total_bw_limit,
    vx_float32 internal_write_bw_limit,
    vx_uint32  interleave_mode,
    vx_uint32  lanes_per_conv,
    vx_uint32  outstanding_transfer,
    vx_uint32  zrl_bits,
    vx_uint32  equivalent_vip_sram_width_in_byte,
    vx_float32 ddr_latency,
    vx_float32 total_latency,
    vx_float32 ddr_read_bw_in_byte_per_cycle,
    vx_float32 ddr_write_bw_in_byte_per_cycle,
    vx_float32 ddr_total_bw_in_byte_per_cycle,
    vx_bool    image_not_packed_in_sram,
    vx_bool    vip_v7_16bit,
    vx_bool    fp16,
    vx_bool    kernel_head_not_cached_fix,
    vx_bool    conv1x1_half_performance,
    vx_bool    cache_line_mode_disabled,
    vx_bool    per_3d_tile_bubble_fix,
    vx_bool    zdp3_no_compress_fix,
    vx_bool    async_copy_perf_fix,
    vx_bool    zxdp3_kernel_read_conflict_fix,
    vx_bool    accurate_tile_bw,
    vx_bool    axi_sram_slowed_down_by_addr,
    vx_bool    slow_nn_req_arbitration_fix,
    vx_bool    single_port_acc_buffer,
    vx_bool    small_batch_enable,
    vx_bool    is_depth_wise,
    vx_bool    is_nn_write_without_usc,
    vx_uint32  in_image_stride,
    vx_uint32  in_image_slice,
    vx_uint32  out_image_stride,
    vx_uint32  out_image_slice,
    vx_bool    flush,
    vx_uint32  conv_out_fifo_depth,
    vx_float64* ddr_kernel_read_bandwidth,
    vx_float64* ddr_in_image_read_bandwidth,
    vx_float64* ddr_read_bandwidth,
    vx_float64* ddr_write_bandwidth,
    vx_float64* axi_read_bandwidth,
    vx_float64* axi_write_bandwidth,
    vx_bool *  is_compute_bottle_neck
    )
{
    vx_uint32 kernelFromDDR, kernelFromAXISram, sizeForAXISram;
    vx_float64 cacheSizeInPixel, kernelIdealCache, imageIdealCache;
    vx_float64 ddrReadBW, vipReadBW, axiReadBW, axiBusReadBW;
    vx_float64 ddrWriteBW, axiWriteBW, vipWriteBW, axiBusWriteBW;
    vx_float64 ddrTotalBW, axiTotalBW, axiBusTotalBW;
    vx_float64 ddrReadCycleCount, axiReadCycleCount, axiBusReadCycleCount;
    vx_float64 ddrWriteCycleCount, axiWriteCycleCount, axiBusWriteCycleCount, vipWriteCycleCount;
    vx_float64 ddrTotalCycleCount, axiTotalCycleCount, axiBusTotalCycleCount;
    vx_float64 computeCycleCount, vipReadCycleCount, kernelDecodeCycleCount;
    vx_float64 internalWriteCycleCount;
    vx_float32 adjustedAXISraReadBandWidthLimit = axi_sram_read_bw_limit;
    vx_float64 imageRepeatedSingleRead;
    vx_float64 arbCycleCount, xBarCycleCount = 0;
    vx_uint32 axiAccUnitSizeInByte = AXI_BURST_SIZE;
    vx_uint32 vipSramAccUnitSizeInByte = l2cache_width * 2; /* x2 because we are using half freq SRAM */
    vx_uint32 vipSramInImageStride, vipSramInImageSlice;
    vx_float64 refined_non_zero_ratio;
    vx_float64 kernelReadBWTile0, imageReadBWVZGroup0;
    vx_float64 ddrKernelReadBW = 0, ddrInImageReadBW = 0, vipKernelReadBW = 0, vipInImageReadBW = 0;
    vx_arch_model_bw_cost_s ddrTotalBWCost, axiTotalBWCost, axiBusTotalBWCost;
    vx_arch_model_cycle_cost_s computeCycleCost, internalWriteCycleCost, ddrTotalCycleCost, axiTotalCycleCost, axiBusTotalCycleCost;
    vx_arch_model_cycle_cost_s kernelDecodeCycleCost, arbCycleCost, xBarCycleCost, nnCycleCost;

    vx_arch_model_cost_s  nnCost[NUMBER_OF_NN_COST_TYPE];
    vx_arch_model_bw_cost_s *bwCost;
    vx_arch_model_cycle_cost_s *cycleCost;

    vx_uint32 tail_x = x % tile_x;
    vx_uint32 tail_y = y % tile_y;
    vx_uint32 tail_z = z % (kernel_per_core * cores);
    tail_x = (tail_x == 0) ? tile_x : tail_x;
    tail_y = (tail_y == 0) ? tile_y : tail_y;
    tail_z = (tail_z == 0) ? (kernel_per_core * cores) : tail_z;

    memset(nnCost, 0, sizeof(vx_arch_model_cost_s) * NUMBER_OF_NN_COST_TYPE);
    if (src_buf != SW_TILING_FROM_VIP_SRAM)
    {
        vipSramInImageStride = gcmMIN(tile_x + kx - 1, in_image_stride);
        vipSramInImageSlice = gcmMIN((vipSramInImageStride * (tile_y + ky - 1)), in_image_slice);
    }
    else
    {
        vipSramInImageStride = in_image_stride;
        vipSramInImageSlice = in_image_slice;
    }

    if (zdp > 1 && kx == 1 && ky == 1 && input_data_size == 8 && !zdp3_no_compress_fix)
    {
        gcmASSERT(non_zero_ratio == 1);
        coef_compress_ratio = gcmMAX(1, coef_compress_ratio);
        non_zero_ratio = 1;
    }

    non_zero_ratio = gcmMAX(non_zero_ratio, (vx_float64)1/((vx_float64)pow(2, zrl_bits) - 1)); /* for limitation of zrl_bit_width <= 5 */

    computeCycleCount = _calcComputeCycleCount(tile_x, tile_y, kernel_per_core,
                                               kx, ky, kz, x, y, z, non_zero_ratio,
                                               xydp_x, xydp_y, zdp,
                                               float_xydp_x, float_xydp_y, float_zdp,
                                               input_data_size, vector_prune,
                                               interleave_mode, lanes_per_conv, coef_decode_perf,
                                               zrl_bits, is_depth_wise,
                                               vip_v7_16bit, fp16, conv1x1_half_performance, zxdp3_kernel_read_conflict_fix, single_port_acc_buffer, &refined_non_zero_ratio)
                                               / cores;

    vipKernelReadBW = _calcKernelReadBandWidth(tile_x, tile_y, kernel_per_core,
        kx, ky, kz, x, y, z, x, y,
        cores, brick_mode, input_data_size,
        coef_compress_ratio, image_compress_ratio, 0, kernel_head_not_cached_fix, is_depth_wise, &kernelReadBWTile0);

    vipInImageReadBW = _calcImageReadBandWidth(tile_x, tile_y, kernel_per_core,
        kx, ky, kz, x, y, z, x, y,
        cores, brick_mode, input_data_size,
        coef_compress_ratio, 1, 0, image_not_packed_in_sram, cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte,
        vipSramInImageStride, vipSramInImageSlice, zdp, vipSramAccUnitSizeInByte, image_partial_cache, &imageReadBWVZGroup0);

    bwCost = &nnCost[ARCH_MODEL_VIP_SRAM_COST].readBW;
    cycleCost = &nnCost[ARCH_MODEL_VIP_SRAM_COST].readCycle;
    bwCost->cost = vipReadBW = vipKernelReadBW + vipInImageReadBW;
    bwCost->tile0VZGroup0 = kernelReadBWTile0 * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) + imageReadBWVZGroup0 * (1.0f * tile_x / x) * (1.0f * tile_y / y);
    bwCost->tile0 = kernelReadBWTile0 + vipInImageReadBW * (1.0f * tile_x / x) * (1.0f * tile_y / y);
    bwCost->vzGroup0 = vipKernelReadBW * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) + imageReadBWVZGroup0;
    vipReadCycleCount = vipReadBW / l2cache_width;
    cycleCost->cost = vipReadCycleCount;
    cycleCost->tile0VZGroup0 = bwCost->tile0VZGroup0 / l2cache_width;
    cycleCost->tile0ResetVZGroup = (bwCost->tile0 - bwCost->tile0VZGroup0) / l2cache_width;
    cycleCost->resetTileVZGroup0 = (bwCost->vzGroup0 - bwCost->tile0VZGroup0) / l2cache_width;
    cycleCost->resetTileResetVZGroup = (vipReadBW + bwCost->tile0VZGroup0 - bwCost->tile0 - bwCost->vzGroup0) / l2cache_width;

    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    ddrReadBW = (vx_float32)nn_cmd_size;
    bwCost = &nnCost[ARCH_MODEL_DDR_COST].readBW;
    memset(bwCost, 0, sizeof(vx_arch_model_bw_cost_s));
    bwCost->cost = ddrReadBW;
    bwCost->tile0VZGroup0 = (vx_float32)nn_cmd_size;
    bwCost->tile0 = (vx_float32)nn_cmd_size;
    bwCost->vzGroup0 = (vx_float32)nn_cmd_size;

    axiReadBW = 0;
    kernelFromDDR = kernel_buf == SW_TILING_FROM_DDR || (first_cmd && ((kernel_buf == SW_TILING_FROM_AXI_SRAM) || (kernel_buf == SW_TILING_FROM_VIP_SRAM)));
    kernelFromAXISram = (!first_cmd && (kernel_buf == SW_TILING_FROM_AXI_SRAM)) || (kernel_buf == SW_TILING_PERM_AXI_SRAM);

    cacheSizeInPixel = (vx_float32)l2cache_size / (input_data_size / 8);

    if (kernelFromDDR && src_buf == SW_TILING_FROM_DDR)
    {
        vx_float64 ddrKernelReadBWTile0, ddrInImageReadBWVZGroup0;
        ddrReadBW = _calcReadBandWidth(tile_x, tile_y, kernel_per_core,
            kx, ky, kz, x, y, z, inx, iny,
            cores, brick_mode, input_data_size, coef_compress_ratio, image_compress_ratio,
            l2cache_size, (vx_bool)image_partial_cache, nn_cmd_size, image_not_packed_in_sram,
            kernel_head_not_cached_fix, cache_line_mode_disabled,
            async_copy_perf_fix, accurate_tile_bw, is_depth_wise, in_image_stride, in_image_slice, zdp, axiAccUnitSizeInByte,
            equivalent_vip_sram_width_in_byte, &ddrKernelReadBW, &ddrInImageReadBW, &ddrKernelReadBWTile0, &ddrInImageReadBWVZGroup0, &nnCost[ARCH_MODEL_DDR_COST].readBW);

        bwCost = &nnCost[ARCH_MODEL_DDR_KERNEL_COST].readBW;
        bwCost->cost = ddrKernelReadBW;
        bwCost->tile0 = ddrKernelReadBWTile0;

        bwCost = &nnCost[ARCH_MODEL_DDR_IN_IMAGE_COST].readBW;
        bwCost->cost = ddrInImageReadBW;
        bwCost->vzGroup0 = ddrInImageReadBWVZGroup0;
    }
    else if (kernelFromDDR)
    {
        ddrKernelReadBW = _calcKernelReadBandWidth(tile_x, tile_y, kernel_per_core,
            kx, ky, kz, x, y, z, x, y,
            cores, brick_mode, input_data_size, coef_compress_ratio,
            image_compress_ratio, cacheSizeInPixel, kernel_head_not_cached_fix, is_depth_wise, &kernelReadBWTile0);

        bwCost = &nnCost[ARCH_MODEL_DDR_KERNEL_COST].readBW;
        bwCost->cost = ddrKernelReadBW;
        bwCost->tile0 = kernelReadBWTile0;

        bwCost = &nnCost[ARCH_MODEL_DDR_COST].readBW;
        ddrReadBW = nn_cmd_size + ddrKernelReadBW;
        bwCost->cost = ddrReadBW;
        bwCost->tile0VZGroup0 = nn_cmd_size + kernelReadBWTile0 * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);
        bwCost->tile0 = nn_cmd_size + kernelReadBWTile0;
        bwCost->vzGroup0 = nn_cmd_size + ddrKernelReadBW * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);

        /*empty nnCost[ARCH_MODEL_DDR_IN_IMAGE_COST].readBW */

        if (src_buf == SW_TILING_FROM_AXI_SRAM)
        {
            vx_uint32 zPerCore = (vx_uint32)ceilf((vx_float32)z / cores);
            vx_float64 kernelNonIdealCache, kernelStorage;
            if (is_depth_wise)
            {
                kernelIdealCache = _calcKernel4DSingleReadBW(kx, ky, 1, z, 1);
                kernelNonIdealCache = ceilf((vx_float32)_calcKernel4DSingleReadBW(kx, ky, 1, zPerCore, 1) / AXI_BURST_SIZE) * AXI_BURST_SIZE * cores;
            }
            else
            {
                kernelIdealCache = _calcKernel4DSingleReadBW(kx, ky, kz, z, coef_compress_ratio);
                kernelNonIdealCache = ceilf((vx_float32)_calcKernel4DSingleReadBW(kx, ky, kz, zPerCore, coef_compress_ratio) / AXI_BURST_SIZE) * AXI_BURST_SIZE * cores;
            }
            kernelStorage = kernel_head_not_cached_fix ? kernelIdealCache : kernelNonIdealCache;

            cacheSizeInPixel = (cacheSizeInPixel - gcmMIN(kernelStorage, cacheSizeInPixel)) * data_read_from_sram;
            axiReadBW = _calcImageReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
                cores, brick_mode, input_data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, image_not_packed_in_sram,
                cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, zdp, axiAccUnitSizeInByte, image_partial_cache, &imageReadBWVZGroup0);

            bwCost = &nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW;
            bwCost->cost = axiReadBW;
            bwCost->tile0VZGroup0 = imageReadBWVZGroup0 * (1.0f * tile_x / x) * (1.0f * tile_y / y);
            bwCost->tile0 = axiReadBW * (1.0f * tile_x / x) * (1.0f * tile_y / y);
            bwCost->vzGroup0 = imageReadBWVZGroup0;

            if (axi_sram_slowed_down_by_addr && (first_cmd || (cacheSizeInPixel != 0)))
            {
                vx_uint32 maxOutstandingCycle = outstanding_transfer * 4;
                if (total_latency > maxOutstandingCycle)
                {
                    vx_float32 bwLimitedByLatency = (vx_float32)(16.0 * maxOutstandingCycle) / total_latency;
                    adjustedAXISraReadBandWidthLimit = gcmMIN(axi_sram_read_bw_limit, bwLimitedByLatency);
                }
            }
        }
    }
    else if (src_buf == SW_TILING_FROM_DDR)
    {
        /*empty nnCost[ARCH_MODEL_DDR_KERNEL_COST].readBW */

        ddrInImageReadBW = _calcImageReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
            cores, brick_mode, input_data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, image_not_packed_in_sram,
            cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, zdp, axiAccUnitSizeInByte, image_partial_cache, &imageReadBWVZGroup0);

        bwCost = &nnCost[ARCH_MODEL_DDR_IN_IMAGE_COST].readBW;
        bwCost->cost = ddrInImageReadBW;
        bwCost->vzGroup0 = imageReadBWVZGroup0;

        ddrReadBW = nn_cmd_size + ddrInImageReadBW;
        bwCost = &nnCost[ARCH_MODEL_DDR_COST].readBW;
        bwCost->cost = ddrReadBW;
        bwCost->tile0VZGroup0 = nn_cmd_size + imageReadBWVZGroup0 * (1.0f * tile_x / x) * (1.0f * tile_y / y);
        bwCost->tile0 = nn_cmd_size + ddrInImageReadBW * (1.0f * tile_x / x) * (1.0f * tile_y / y);
        bwCost->vzGroup0 = nn_cmd_size + imageReadBWVZGroup0;


        if (kernelFromAXISram)
        {
            vx_uint32 intile_x = (tile_x + kx - 1);
            vx_uint32 intile_y = (tile_y + ky - 1);
            intile_x = gcmMIN(intile_x, inx);
            intile_y = gcmMIN(intile_y, iny);
            if (image_not_packed_in_sram)
            {
                imageIdealCache = ceilf(ceilf((vx_float32)intile_x * intile_y / 16) * 16 * kz / (equivalent_vip_sram_width_in_byte * 2))
                                  * (equivalent_vip_sram_width_in_byte * 2) * input_data_size / 8;
            }
            else
            {
                imageIdealCache = (vx_float32)intile_x * intile_y * kz * input_data_size / 8;
            }
            cacheSizeInPixel = (cacheSizeInPixel - gcmMIN(imageIdealCache, cacheSizeInPixel)) * data_read_from_sram;
            axiReadBW = _calcKernelReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, inx, iny, z, x, y,
                cores, brick_mode, input_data_size, coef_compress_ratio,
                image_compress_ratio, cacheSizeInPixel, kernel_head_not_cached_fix, is_depth_wise, &kernelReadBWTile0);

            bwCost = &nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW;
            bwCost->cost = axiReadBW;
            bwCost->tile0VZGroup0 = kernelReadBWTile0 * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);
            bwCost->tile0 = kernelReadBWTile0;
            bwCost->vzGroup0 = axiReadBW * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);

            if (axi_sram_slowed_down_by_addr)
            {
                vx_uint32 maxOutstandingCycle = outstanding_transfer * 4;
                if (total_latency > maxOutstandingCycle)
                {
                    vx_float32 bwLimitedByLatency = (vx_float32)(16.0 * maxOutstandingCycle) / total_latency;
                    adjustedAXISraReadBandWidthLimit = gcmMIN(axi_sram_read_bw_limit, bwLimitedByLatency);
                }
            }
        }
    }
    else if (kernelFromAXISram && src_buf == SW_TILING_FROM_AXI_SRAM)
    {
        sizeForAXISram = data_read_from_sram * l2cache_size;
        axiReadBW = _calcReadBandWidth(tile_x, tile_y, kernel_per_core,
            kx, ky, kz, x, y, z, inx, iny,
            cores, brick_mode, input_data_size, coef_compress_ratio, image_compress_ratio, sizeForAXISram,
            (vx_bool)image_partial_cache, nn_cmd_size, image_not_packed_in_sram, kernel_head_not_cached_fix, cache_line_mode_disabled,
            async_copy_perf_fix, accurate_tile_bw, is_depth_wise, in_image_stride, in_image_slice, zdp, axiAccUnitSizeInByte, equivalent_vip_sram_width_in_byte, NULL, NULL, &kernelReadBWTile0, &imageReadBWVZGroup0, &nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW);
    }
    else if (kernelFromAXISram)
    {
        cacheSizeInPixel = cacheSizeInPixel * data_read_from_sram;
        axiReadBW = _calcKernelReadBandWidth(tile_x, tile_y, kernel_per_core,
            kx, ky, kz, x, y, z, inx, iny,
            cores, brick_mode, input_data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel,
            kernel_head_not_cached_fix, is_depth_wise, &kernelReadBWTile0);

        bwCost = &nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW;
        bwCost->cost = axiReadBW;
        bwCost->tile0VZGroup0 = kernelReadBWTile0 * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);
        bwCost->tile0 = kernelReadBWTile0;
        bwCost->vzGroup0 = axiReadBW * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);
    }
    else if (src_buf == SW_TILING_FROM_AXI_SRAM)
    {
        cacheSizeInPixel = cacheSizeInPixel * data_read_from_sram;
        axiReadBW = _calcImageReadBandWidth(tile_x, tile_y, kernel_per_core, kx, ky, kz, x, y, z, inx, iny,
            cores, brick_mode, input_data_size, coef_compress_ratio, image_compress_ratio, cacheSizeInPixel, image_not_packed_in_sram,
            cache_line_mode_disabled, async_copy_perf_fix, accurate_tile_bw, is_depth_wise, equivalent_vip_sram_width_in_byte, in_image_stride, in_image_slice, zdp, axiAccUnitSizeInByte, image_partial_cache, &imageReadBWVZGroup0);

        bwCost = &nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW;
        bwCost->cost = axiReadBW;
        bwCost->tile0VZGroup0 = imageReadBWVZGroup0 * (1.0f * tile_x / x) * (1.0f * tile_y / y);
        bwCost->tile0 = axiReadBW * (1.0f * tile_x / x) * (1.0f * tile_y / y);
        bwCost->vzGroup0 = imageReadBWVZGroup0;
    }

    axiBusReadBW = ddrReadBW + axiReadBW;
    bwCost = &nnCost[ARCH_MODEL_AXI_BUS_COST].readBW;
    bwCost->cost = axiBusReadBW;
    bwCost->tile0VZGroup0 = nnCost[ARCH_MODEL_DDR_COST].readBW.tile0VZGroup0 + nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW.tile0VZGroup0;
    bwCost->tile0 = nnCost[ARCH_MODEL_DDR_COST].readBW.tile0 + nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW.tile0;
    bwCost->vzGroup0 = nnCost[ARCH_MODEL_DDR_COST].readBW.vzGroup0 + nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW.vzGroup0;

    ddrReadCycleCount = ddrReadBW / ddr_read_bw_in_byte_per_cycle;
    bwCost = &nnCost[ARCH_MODEL_DDR_COST].readBW;
    cycleCost = &nnCost[ARCH_MODEL_DDR_COST].readCycle;
    cycleCost->cost = ddrReadCycleCount;
    cycleCost->tile0VZGroup0 = bwCost->tile0VZGroup0 / ddr_read_bw_in_byte_per_cycle;
    cycleCost->tile0ResetVZGroup = (bwCost->tile0 - bwCost->tile0VZGroup0) / ddr_read_bw_in_byte_per_cycle;
    cycleCost->resetTileVZGroup0 = (bwCost->vzGroup0 - bwCost->tile0VZGroup0) / ddr_read_bw_in_byte_per_cycle;
    cycleCost->resetTileResetVZGroup = (bwCost->cost + bwCost->tile0VZGroup0 - bwCost->tile0 - bwCost->vzGroup0) / ddr_read_bw_in_byte_per_cycle;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));


    axiReadCycleCount = axiReadBW / adjustedAXISraReadBandWidthLimit;
    bwCost = &nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW;
    cycleCost = &nnCost[ARCH_MODEL_AXI_SRAM_COST].readCycle;
    cycleCost->cost = axiReadCycleCount;
    cycleCost->tile0VZGroup0 = bwCost->tile0VZGroup0 / adjustedAXISraReadBandWidthLimit;
    cycleCost->tile0ResetVZGroup = (bwCost->tile0 - bwCost->tile0VZGroup0) / adjustedAXISraReadBandWidthLimit;
    cycleCost->resetTileVZGroup0 = (bwCost->vzGroup0 - bwCost->tile0VZGroup0) / adjustedAXISraReadBandWidthLimit;
    cycleCost->resetTileResetVZGroup = (bwCost->cost + bwCost->tile0VZGroup0 - bwCost->tile0 - bwCost->vzGroup0) / adjustedAXISraReadBandWidthLimit;;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    axiBusReadCycleCount = axiBusReadBW / axi_bus_read_bw_limit;
    bwCost = &nnCost[ARCH_MODEL_AXI_BUS_COST].readBW;
    cycleCost = &nnCost[ARCH_MODEL_AXI_BUS_COST].readCycle;
    cycleCost->cost = axiBusReadCycleCount;
    cycleCost->tile0VZGroup0 = bwCost->tile0VZGroup0 / axi_bus_read_bw_limit;
    cycleCost->tile0ResetVZGroup = (bwCost->tile0 - bwCost->tile0VZGroup0) / axi_bus_read_bw_limit;
    cycleCost->resetTileVZGroup0 = (bwCost->vzGroup0 - bwCost->tile0VZGroup0) / axi_bus_read_bw_limit;
    cycleCost->resetTileResetVZGroup = (bwCost->cost + bwCost->tile0VZGroup0 - bwCost->tile0 - bwCost->vzGroup0) / axi_bus_read_bw_limit;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    ddrWriteBW = 0;
    axiWriteBW = 0;
    vipWriteBW = ddrReadBW;
    if (dst_buf == SW_TILING_FROM_DDR)
    {
        ddrWriteBW = _calcWriteBandWidth(tile_x, tile_y, x, y, z, output_data_size, image_compress_ratio, usc_cache_size, pooling_stride, out_image_stride, out_image_slice, is_nn_write_without_usc);
    }
    else if (dst_buf == SW_TILING_FROM_AXI_SRAM)
    {
        axiWriteBW = _calcWriteBandWidth(tile_x, tile_y, x, y, z, output_data_size, image_compress_ratio, usc_cache_size, pooling_stride, out_image_stride, out_image_slice, is_nn_write_without_usc);
    }
    else
    {
        vipWriteBW = vipWriteBW + _calcWriteBandWidth(tile_x, tile_y, x, y, z, output_data_size, 1, usc_cache_size, pooling_stride, out_image_stride, out_image_slice, is_nn_write_without_usc);
    }
    axiBusWriteBW = ddrWriteBW + axiWriteBW;

    bwCost = &nnCost[ARCH_MODEL_DDR_COST].writeBW;
    bwCost->cost = ddrWriteBW;
    bwCost->tile0VZGroup0 = 0;
    bwCost->tile0 = ddrWriteBW * (1.0f * tile_x / x) * (1.0f * tile_y / y) * (1 - 1.0f * tail_z / z);
    bwCost->vzGroup0 = ddrWriteBW * (1.0f * tail_z / z) * (1 - 1.0f * tail_x / x * tail_y / y);
    bwCost->residual = ddrWriteBW * tail_z / z * tail_x / x * tail_y / y;

    ddrWriteCycleCount = ddrWriteBW / ddr_write_bw_in_byte_per_cycle;
    cycleCost = &nnCost[ARCH_MODEL_DDR_COST].writeCycle;
    cycleCost->cost = ddrWriteCycleCount;
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = nnCost[ARCH_MODEL_DDR_COST].writeBW.tile0 / ddr_write_bw_in_byte_per_cycle;
    cycleCost->resetTileVZGroup0 = nnCost[ARCH_MODEL_DDR_COST].writeBW.vzGroup0 / ddr_write_bw_in_byte_per_cycle;
    cycleCost->resetTileResetVZGroup = (ddrWriteBW - nnCost[ARCH_MODEL_DDR_COST].writeBW.tile0 - nnCost[ARCH_MODEL_DDR_COST].writeBW.vzGroup0) / ddr_write_bw_in_byte_per_cycle;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));


    bwCost = &nnCost[ARCH_MODEL_AXI_SRAM_COST].writeBW;
    bwCost->cost = axiWriteBW;
    bwCost->tile0VZGroup0 = 0;
    bwCost->tile0 = axiWriteBW * (1.0f * tile_x / x) * (1.0f * tile_y / y) * (1 - 1.0f * tail_z / z);
    bwCost->vzGroup0 = axiWriteBW * (1.0f * tail_z / z) * (1 - 1.0f * tail_x / x * tail_y / y);
    bwCost->residual = axiWriteBW * tail_z / z * tail_x / x * tail_y / y;
    axiWriteCycleCount = axiWriteBW / axi_sram_write_bw_limit;
    cycleCost = &nnCost[ARCH_MODEL_AXI_SRAM_COST].writeCycle;
    cycleCost->cost = axiWriteCycleCount;
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = bwCost->tile0 / axi_sram_write_bw_limit;
    cycleCost->resetTileVZGroup0 = bwCost->vzGroup0 / axi_sram_write_bw_limit;
    cycleCost->resetTileResetVZGroup = (axiWriteBW - bwCost->tile0 - bwCost->vzGroup0) / axi_sram_write_bw_limit;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    bwCost = &nnCost[ARCH_MODEL_AXI_BUS_COST].writeBW;
    bwCost->cost = axiBusWriteBW;
    bwCost->tile0VZGroup0 = 0;
    bwCost->tile0 = axiBusWriteBW * (1.0f * tile_x / x) * (1.0f * tile_y / y) * (1 - 1.0f * tail_z / z);
    bwCost->vzGroup0 = axiBusWriteBW * (1.0f * tail_z / z) * (1 - 1.0f * tail_x / x * tail_y / y);
    bwCost->residual = axiBusWriteBW * tail_z / z * tail_x / x * tail_y / y;
    axiBusWriteCycleCount = axiBusWriteBW / axi_bus_write_bw_limit;
    cycleCost = &nnCost[ARCH_MODEL_AXI_BUS_COST].writeCycle;
    cycleCost->cost = axiBusWriteCycleCount;
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = bwCost->tile0 / axi_bus_write_bw_limit;
    cycleCost->resetTileVZGroup0 = bwCost->vzGroup0 / axi_bus_write_bw_limit;
    cycleCost->resetTileResetVZGroup = (axiBusWriteBW - bwCost->tile0 - bwCost->vzGroup0) / axi_bus_write_bw_limit;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    bwCost = &nnCost[ARCH_MODEL_VIP_SRAM_COST].writeBW;
    bwCost->cost = vipWriteBW;
    bwCost->tile0VZGroup0 = 0;
    bwCost->tile0 = vipWriteBW * (1.0f * tile_x / x) * (1.0f * tile_y / y) * (1 - 1.0f * tail_z / z);
    bwCost->vzGroup0 = vipWriteBW * (1.0f * tail_z / z) * (1 - 1.0f * tail_x / x * tail_y / y);
    bwCost->residual = vipWriteBW * tail_z / z * tail_x / x * tail_y / y;
    vipWriteCycleCount = vipWriteBW / l2cache_width;
    cycleCost = &nnCost[ARCH_MODEL_VIP_SRAM_COST].writeCycle;
    cycleCost->cost = vipWriteCycleCount;
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = bwCost->tile0 / l2cache_width;
    cycleCost->resetTileVZGroup0 = bwCost->vzGroup0 / l2cache_width;
    cycleCost->resetTileResetVZGroup = (vipWriteBW - bwCost->tile0 - bwCost->vzGroup0) / l2cache_width;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    {
        vx_bool isV8 = (xydp_x == 0 && xydp_y == 0) ? vx_true_e : vx_false_e;
        vx_float32 tmp_internal_write_bw_limit = internal_write_bw_limit;
        vx_uint32 zdpLoopCount = 3;
        if (isV8) /*for v8*/
        {
            vx_float32 slowInternalWriteBWLimit;
            vx_float64 slowInternalWriteCycleCount, slowCompCycleCount;
            if (input_data_size == 16 || ((kx * ky * kz) > 512))
            {
                tmp_internal_write_bw_limit = tmp_internal_write_bw_limit / 2;
            }

            slowInternalWriteBWLimit = gcmMIN(tmp_internal_write_bw_limit, (vx_float32)lanes_per_conv / zdpLoopCount);
            slowInternalWriteCycleCount = ceilf((vx_float32)tile_x * tile_y / slowInternalWriteBWLimit) * ceilf((vx_float32)x / tile_x) * ceilf((vx_float32)y / tile_y) * z * (input_data_size * input_data_size) / (8 * 8);
            slowCompCycleCount = computeCycleCount + ceilf((vx_float32)tile_x * tile_y / lanes_per_conv) * ceilf((vx_float32)x / tile_x) * ceilf((vx_float32)y / tile_y) * ceilf((vx_float32)z / cores) * (input_data_size * input_data_size) / (8 * 8);
            if (slowInternalWriteCycleCount > slowCompCycleCount)
            {
                computeCycleCount = slowCompCycleCount;
                internalWriteCycleCount = ceilf((vx_float32)tile_x * tile_y / tmp_internal_write_bw_limit) * ceilf((vx_float32)x / tile_x) * ceilf((vx_float32)y / tile_y) * z * (input_data_size * input_data_size) / (8 * 8);
            }
            else
            {
                internalWriteCycleCount = slowInternalWriteCycleCount;
            }
        }
        else
        {
            internalWriteCycleCount = ceilf(1.0f * tile_x * interleave_mode / tmp_internal_write_bw_limit) * ceilf(1.0f * x / tile_x) * ceilf(1.0f * tile_y / interleave_mode) * ceilf(1.0f * y / tile_y) * z * input_data_size / 8;
        }
    }

    cycleCost = &computeCycleCost;
    cycleCost->cost = computeCycleCount;
    cycleCost->tile0VZGroup0 = computeCycleCount * (1.0f * tile_x / x) * (1.0f * tile_y / y) * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);
    cycleCost->tile0ResetVZGroup = computeCycleCount * (1.0f * tile_x / x) * (1.0f * tile_y / y) - cycleCost->tile0VZGroup0;
    cycleCost->resetTileVZGroup0 = computeCycleCount * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) - cycleCost->tile0VZGroup0;
    cycleCost->resetTileResetVZGroup = computeCycleCount - cycleCost->tile0VZGroup0 - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    cycleCost = &internalWriteCycleCost;
    cycleCost->cost = internalWriteCycleCount;
    cycleCost->tile0VZGroup0 = 0;
    cycleCost->tile0ResetVZGroup = internalWriteCycleCount * (1.0f / ceilf(1.0f * x / tile_x)) * (1.0f / ceilf(1.0f * y / tile_y)) * (1 - 1 / ceilf(1.0f * z / (kernel_per_core * cores)));
    cycleCost->resetTileVZGroup0 = internalWriteCycleCount * (1.0f - 1 / (ceilf(1.0f * x / tile_x) * ceilf(1.0f * y / tile_y))) * (1 / ceilf(1.0f * z / (kernel_per_core * cores)));
    cycleCost->resetTileResetVZGroup = internalWriteCycleCount - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    ddrTotalBW = ddrReadBW + ddrWriteBW;
    bwCost = &ddrTotalBWCost;
    bwCost->cost = ddrTotalBW;
    bwCost->tile0VZGroup0 = nnCost[ARCH_MODEL_DDR_COST].readBW.tile0VZGroup0 + nnCost[ARCH_MODEL_DDR_COST].writeBW.tile0VZGroup0;
    bwCost->tile0 = nnCost[ARCH_MODEL_DDR_COST].readBW.tile0 + nnCost[ARCH_MODEL_DDR_COST].writeBW.tile0;
    bwCost->vzGroup0 = nnCost[ARCH_MODEL_DDR_COST].readBW.vzGroup0 + nnCost[ARCH_MODEL_DDR_COST].writeBW.vzGroup0;

    ddrTotalCycleCount = ddrTotalBW / ddr_total_bw_in_byte_per_cycle;
    cycleCost = &ddrTotalCycleCost;
    cycleCost->cost = ddrTotalCycleCount;
    cycleCost->tile0VZGroup0 = ddrTotalBWCost.tile0VZGroup0 / ddr_total_bw_in_byte_per_cycle;
    cycleCost->tile0ResetVZGroup = (ddrTotalBWCost.tile0 - ddrTotalBWCost.tile0VZGroup0) / ddr_total_bw_in_byte_per_cycle;
    cycleCost->resetTileVZGroup0 = (ddrTotalBWCost.vzGroup0 - ddrTotalBWCost.tile0VZGroup0) / ddr_total_bw_in_byte_per_cycle;
    cycleCost->resetTileResetVZGroup = (ddrTotalBW + ddrTotalBWCost.tile0VZGroup0 - ddrTotalBWCost.tile0 - ddrTotalBWCost.vzGroup0) / ddr_total_bw_in_byte_per_cycle;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    axiTotalBW = axiReadBW + axiWriteBW;
    bwCost = &axiTotalBWCost;
    bwCost->cost = axiTotalBW;
    bwCost->tile0VZGroup0 = nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW.tile0VZGroup0 + nnCost[ARCH_MODEL_AXI_SRAM_COST].writeBW.tile0VZGroup0;
    bwCost->tile0 = nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW.tile0 + nnCost[ARCH_MODEL_AXI_SRAM_COST].writeBW.tile0;
    bwCost->vzGroup0 = nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW.vzGroup0 + nnCost[ARCH_MODEL_AXI_SRAM_COST].writeBW.vzGroup0;

    cycleCost = &axiTotalCycleCost;
    axiTotalCycleCount = axiTotalBW / axi_sram_total_bw_limit;
    cycleCost->cost = axiTotalCycleCount;
    cycleCost->tile0VZGroup0 = axiTotalBWCost.tile0VZGroup0 / axi_sram_total_bw_limit;
    cycleCost->tile0ResetVZGroup = (axiTotalBWCost.tile0 - axiTotalBWCost.tile0VZGroup0) / axi_sram_total_bw_limit;
    cycleCost->resetTileVZGroup0 = (axiTotalBWCost.vzGroup0 - axiTotalBWCost.tile0VZGroup0) / axi_sram_total_bw_limit;
    cycleCost->resetTileResetVZGroup = (axiTotalBW + axiTotalBWCost.tile0VZGroup0 - axiTotalBWCost.tile0 - axiTotalBWCost.vzGroup0) / axi_sram_total_bw_limit;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    axiBusTotalBW = axiBusReadBW + axiBusWriteBW;
    bwCost = &axiBusTotalBWCost;
    bwCost->cost = axiBusTotalBW;
    bwCost->tile0VZGroup0 = nnCost[ARCH_MODEL_AXI_BUS_COST].readBW.tile0VZGroup0 + nnCost[ARCH_MODEL_AXI_BUS_COST].writeBW.tile0VZGroup0;
    bwCost->tile0 = nnCost[ARCH_MODEL_AXI_BUS_COST].readBW.tile0 + nnCost[ARCH_MODEL_AXI_BUS_COST].writeBW.tile0;
    bwCost->vzGroup0 = nnCost[ARCH_MODEL_AXI_BUS_COST].readBW.vzGroup0 + nnCost[ARCH_MODEL_AXI_BUS_COST].writeBW.vzGroup0;
    axiBusTotalCycleCount = axiBusTotalBW / axi_bus_total_bw_limit;

    cycleCost = &axiBusTotalCycleCost;
    cycleCost->cost = axiBusTotalCycleCount;
    cycleCost->tile0VZGroup0 = axiBusTotalBWCost.tile0VZGroup0 / axi_bus_total_bw_limit;
    cycleCost->tile0ResetVZGroup = (axiBusTotalBWCost.tile0 - axiBusTotalBWCost.tile0VZGroup0) / axi_bus_total_bw_limit;
    cycleCost->resetTileVZGroup0 = (axiBusTotalBWCost.vzGroup0 - axiBusTotalBWCost.tile0VZGroup0) / axi_bus_total_bw_limit;
    cycleCost->resetTileResetVZGroup = (axiBusTotalBW + axiBusTotalBWCost.tile0VZGroup0 - axiBusTotalBWCost.tile0 - axiBusTotalBWCost.vzGroup0) / axi_bus_total_bw_limit;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));
#define KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE  16
#define INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE 16

    bwCost = &nnCost[ARCH_MODEL_DDR_KERNEL_COST].readBW;
    cycleCost = &nnCost[ARCH_MODEL_DDR_KERNEL_COST].readCycle;
    cycleCost->cost = bwCost->cost / KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->tile0VZGroup0 = bwCost->tile0 * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) / KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->tile0ResetVZGroup = bwCost->tile0 / KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE - cycleCost->tile0VZGroup0;
    cycleCost->resetTileVZGroup0 = (bwCost->cost - bwCost->tile0) * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) / KERNEL_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->resetTileResetVZGroup = cycleCost->cost - cycleCost->tile0VZGroup0 - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    bwCost = &nnCost[ARCH_MODEL_DDR_IN_IMAGE_COST].readBW;
    cycleCost = &nnCost[ARCH_MODEL_DDR_IN_IMAGE_COST].readCycle;
    cycleCost->cost = bwCost->cost / INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->tile0VZGroup0 = bwCost->vzGroup0 * (1.0f * tile_x / x) * (1.0f * tile_y / y) / INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->tile0ResetVZGroup = (bwCost->cost - bwCost->vzGroup0) * (1.0f * tile_x / x) * (1.0f * tile_y / y) / INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE;
    cycleCost->resetTileVZGroup0 = bwCost->vzGroup0 / INIMAGE_DDR_READ_BW_IN_BYTE_PER_CYCLE - cycleCost->tile0VZGroup0;
    cycleCost->resetTileResetVZGroup = cycleCost->cost - cycleCost->tile0VZGroup0 - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    imageRepeatedSingleRead = _calcTile3DImageSingleReadRepeated(z, kernel_per_core, cores, is_depth_wise);
    if (is_depth_wise)
    {
        kernelDecodeCycleCount = refined_non_zero_ratio * kx * ky * z * _calcKernel4DSingleReadRepeated(tile_x, tile_y, x, y) / (cores * coef_decode_perf);
    }
    else
    {
        kernelDecodeCycleCount = refined_non_zero_ratio * kx * ky * kz * z * _calcKernel4DSingleReadRepeated(tile_x, tile_y, x, y) / (cores * coef_decode_perf);
    }
    cycleCost = &kernelDecodeCycleCost;
    cycleCost->cost = kernelDecodeCycleCount;
    cycleCost->tile0VZGroup0 = kernelDecodeCycleCount * (1.0f * tile_x / x) * (1.0f * tile_y / y) * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);
    cycleCost->tile0ResetVZGroup = kernelDecodeCycleCount * (1.0f * tile_x / x) * (1.0f * tile_y / y) - cycleCost->tile0VZGroup0;
    cycleCost->resetTileVZGroup0 = kernelDecodeCycleCount * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) - cycleCost->tile0VZGroup0;
    cycleCost->resetTileResetVZGroup = kernelDecodeCycleCount - cycleCost->tile0VZGroup0 - cycleCost->tile0ResetVZGroup - cycleCost->resetTileVZGroup0;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    if ((src_buf == SW_TILING_FROM_DDR) || (src_buf == SW_TILING_FROM_AXI_SRAM))
    {
        if (!slow_nn_req_arbitration_fix)
        {
            if ((zdp > 1) && (kx == 1) && (ky == 1))
            {
                arbCycleCount = imageRepeatedSingleRead * ceilf((vx_float32)kz / (2 * zdp)) * ceilf((vx_float32)x / tile_x) * y * (4 + 6);
            }
            else
            {
                arbCycleCount = imageRepeatedSingleRead * kz * ceilf((vx_float32)x / tile_x) * ceilf((vx_float32)y / tile_y) * (4 + 4);
            }
        }
        else
        {
            if ((zdp > 1) && (kx == 1) && (ky == 1))
            {
                arbCycleCount = imageRepeatedSingleRead * ceilf((vx_float32)kz / (2 * zdp)) * ceilf((vx_float32)x / tile_x) * y * 6;
            }
            else
            {
                arbCycleCount = imageRepeatedSingleRead * kz * ceilf((vx_float32)x / tile_x) * ceilf((vx_float32)y / tile_y) * 4;
            }
        }
    }
    else
    {
        arbCycleCount = 0;
    }
    cycleCost = &arbCycleCost;
    arbCycleCost.cost = arbCycleCount;
    arbCycleCost.tile0VZGroup0 = arbCycleCount * (1.0f * tile_x / x) * (1.0f * tile_y / y) * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);
    arbCycleCost.tile0ResetVZGroup = arbCycleCount * (1.0f * tile_x / x) * (1.0f * tile_y / y) - arbCycleCost.tile0VZGroup0;
    arbCycleCost.resetTileVZGroup0 = arbCycleCount * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) - arbCycleCost.tile0VZGroup0;
    arbCycleCost.resetTileResetVZGroup = arbCycleCount - arbCycleCost.tile0VZGroup0 - arbCycleCost.tile0ResetVZGroup - arbCycleCost.resetTileVZGroup0;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));

    if (xydp_x == 0 || xydp_y == 0)
    {
        vx_float64 regTile3DXBarCycleCount, bottomTile3DXBarCycleCount;
        vx_float64 regTile2DXBarCycleCount = gcmMAX(ceilf((vx_float32)tile_y * tile_x / lanes_per_conv), ceilf((vx_float32)tile_y / 8));
        vx_float64 bottomTile2DXBarCycleCount = gcmMAX(ceilf((1.0f * (y % tile_y) * tile_x) / lanes_per_conv), ceilf(1.0f * (y % tile_y) / 8));
        if (is_depth_wise)
        {
            regTile3DXBarCycleCount = regTile2DXBarCycleCount * ceilf((vx_float32)kx * ky / 3) * z;
            bottomTile3DXBarCycleCount = bottomTile2DXBarCycleCount * ceilf((vx_float32)kx * ky / 3) * z;
        }
        else
        {
            regTile3DXBarCycleCount = regTile2DXBarCycleCount * ceilf((vx_float32)kx * ky * kz / 3) * ceilf((vx_float32)z / (kernel_per_core * cores));
            bottomTile3DXBarCycleCount = bottomTile2DXBarCycleCount * ceilf((vx_float32)kx * ky * kz / 3) * ceilf((vx_float32)z / (kernel_per_core * cores));
        }
        xBarCycleCount = regTile3DXBarCycleCount * (vx_uint32)(1.0f * y / tile_y) + bottomTile3DXBarCycleCount;
        xBarCycleCount = xBarCycleCount * ceilf((vx_float32)x / tile_x);
    }
    cycleCost = &xBarCycleCost;
    xBarCycleCost.cost = xBarCycleCount;
    xBarCycleCost.tile0VZGroup0 = xBarCycleCount * (1.0f * tile_x / x) * (1.0f * tile_y / y) * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f);
    xBarCycleCost.tile0ResetVZGroup = xBarCycleCount * (1.0f * tile_x / x) * (1.0f * tile_y / y) - xBarCycleCost.tile0VZGroup0;
    xBarCycleCost.resetTileVZGroup0 = xBarCycleCount * gcmMIN((1.0f * kernel_per_core * cores / z), 1.0f) - xBarCycleCost.tile0VZGroup0;
    xBarCycleCost.resetTileResetVZGroup = xBarCycleCount - xBarCycleCost.tile0VZGroup0 - xBarCycleCost.tile0ResetVZGroup - xBarCycleCost.resetTileVZGroup0;
    vxmASSERT((cycleCost->tile0VZGroup0 >= -0.1) && (cycleCost->tile0ResetVZGroup >= -0.1) && (cycleCost->resetTileVZGroup0 >= -0.1) && (cycleCost->resetTileResetVZGroup >= -0.1));


#define CALC_MAX_COST(result, type) \
    { \
        result.type = gcmMAX(gcmMAX(gcmMAX(computeCycleCost.type, kernelDecodeCycleCost.type), arbCycleCost.type), xBarCycleCost.type); \
        result.type = gcmMAX(gcmMAX(gcmMAX(nnCost[ARCH_MODEL_DDR_KERNEL_COST].readCycle.type, nnCost[ARCH_MODEL_DDR_IN_IMAGE_COST].readCycle.type), nnCost[ARCH_MODEL_DDR_COST].readCycle.type), nnCycleCost.type); \
        result.type = gcmMAX(gcmMAX(gcmMAX(gcmMAX(nnCycleCost.type, nnCost[ARCH_MODEL_DDR_COST].readCycle.type), nnCost[ARCH_MODEL_AXI_SRAM_COST].readCycle.type), nnCost[ARCH_MODEL_AXI_BUS_COST].readCycle.type), nnCost[ARCH_MODEL_VIP_SRAM_COST].readCycle.type); \
        result.type = gcmMAX(gcmMAX(gcmMAX(gcmMAX(nnCycleCost.type, nnCost[ARCH_MODEL_DDR_COST].writeCycle.type), nnCost[ARCH_MODEL_AXI_SRAM_COST].writeCycle.type), nnCost[ARCH_MODEL_AXI_BUS_COST].writeCycle.type), nnCost[ARCH_MODEL_VIP_SRAM_COST].writeCycle.type); \
        result.type = gcmMAX(gcmMAX(gcmMAX(nnCycleCost.type, ddrTotalCycleCost.type), axiTotalCycleCost.type), axiBusTotalCycleCost.type); \
        result.type = gcmMAX(nnCycleCost.type, internalWriteCycleCost.type); \
    }

    CALC_MAX_COST(nnCycleCost, tile0VZGroup0);
    CALC_MAX_COST(nnCycleCost, tile0ResetVZGroup);
    CALC_MAX_COST(nnCycleCost, resetTileVZGroup0);
    CALC_MAX_COST(nnCycleCost, resetTileResetVZGroup);
    nnCycleCost.cost = nnCycleCost.tile0VZGroup0 + nnCycleCost.tile0ResetVZGroup + nnCycleCost.resetTileVZGroup0 + nnCycleCost.resetTileResetVZGroup;

    if (is_compute_bottle_neck)
    {
        *is_compute_bottle_neck = (nnCycleCost.cost == computeCycleCost.cost) ? vx_true_e : vx_false_e;
    }

    if (flush || !small_batch_enable)/*small_batch_en == 0 || flush_and_wait*/
    {
        nnCycleCost.cost += gcmMIN(ceilf((vx_float32)tile_x / 8) * interleave_mode * conv_out_fifo_depth,
                               gcmMIN(z, cores * kernel_per_core) * (ceilf((vx_float32)tile_x / 8) - 1) * tile_y); /*Emptying Conv Out FIFO*/

        nnCycleCost.cost += (1791 - 1407) + (cores - 1) * (560 - 557) + 1407 + ddr_latency;
    }

    {
        /*Each 3D Tile needs to wait if CONV_OUT_FIFO is not deep enough*/
        vx_float32 tileRow = gcmMIN(z, cores * kernel_per_core) * (vx_float32)tile_y / interleave_mode;
        vx_float32 tileRowSlow = tileRow - conv_out_fifo_depth * (1 + (vx_float32)8 / lanes_per_conv);
        vx_float32 tileOverhead = 0;
        vx_float64 imageRepeatedSingleRead = _calcTile3DImageSingleReadRepeated(z, kernel_per_core, cores, is_depth_wise);
        if (tileRowSlow > 0)
        {
            tileOverhead = tileRowSlow * ceilf((vx_float32)tile_x / 8) * interleave_mode;
        }

        if (!per_3d_tile_bubble_fix && imageRepeatedSingleRead != 1)
        {
            tileOverhead += ddr_latency + 150 + (cores - 1) * ceilf((vx_float32)tile_y / interleave_mode) * kernel_per_core;
        }

        nnCycleCost.cost += tileOverhead * ceilf((vx_float32)y / tile_y) * ceilf((vx_float32)x / tile_x);
    }
    *ddr_kernel_read_bandwidth = nnCost[ARCH_MODEL_DDR_KERNEL_COST].readBW.cost;
    *ddr_in_image_read_bandwidth = nnCost[ARCH_MODEL_DDR_IN_IMAGE_COST].readBW.cost;
    *ddr_read_bandwidth  = nnCost[ARCH_MODEL_DDR_COST].readBW.cost;
    *ddr_write_bandwidth = nnCost[ARCH_MODEL_DDR_COST].writeBW.cost;
    *axi_read_bandwidth = nnCost[ARCH_MODEL_AXI_SRAM_COST].readBW.cost;
    *axi_write_bandwidth = nnCost[ARCH_MODEL_AXI_SRAM_COST].writeBW.cost;
    return nnCycleCost.cost;
}


static vx_float64 _calcTPComputeCycleCount(vx_enum type, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 kz, vx_float64 coef_nonzero_ratio, vx_uint32 cores, vx_float64 image_nonzero_ratio)
{
    if (type == VXNNE_OPERATOR_NORMALIZATION || type == VXNNE_OPERATOR_ACTIVATION)
    {
        return (vx_float64)x * y * z / cores + 512;
    }
    else if (type == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        return (vx_float64)x * y * z * kz * coef_nonzero_ratio * image_nonzero_ratio / cores;
    }
    else
    {
        return (vx_float64)x * y * z / cores;
    }
}

static vx_float64 _calcTPCycleCountCore(vx_enum type, vx_uint32 x, vx_uint32 y, vx_uint32 z, vx_uint32 kz,
                           vx_float64 coef_nonzero_ratio, vx_float64 coef_compress_ratio,
                           vx_float64 image_nonzero_ratio, vx_uint32 cores, vx_uint32 tp_cmd_size,
                           vx_uint32 pooling_stride,
                           vx_float32 axi_sram_read_bw_limit, vx_float32 axi_sram_write_bw_limit, vx_float32 axi_sram_total_bw_limit,
                           vx_float32 axi_bus_read_bw_limit, vx_float32 axi_bus_write_bw_limit, vx_float32 axi_bus_total_bw_limit,
                           vx_uint32 l2cache_width,
                           vx_uint32 data_size,
                           vx_uint32 sw_tiling, vx_uint32 src_buf, vx_uint32 dst_buf, vx_uint32 kernel_buf,
                           vx_uint32 outstanding_transfer,
                           vx_float32 ddr_latency,
                           vx_float32 total_latency,
                           vx_float32 ddr_read_bw_in_byte_per_cycle,
                           vx_float32 ddr_write_bw_in_byte_per_cycle,
                           vx_float32 ddr_total_bw_in_byte_per_cycle,
                           vx_uint32 usc_cache_controllers,
                           vx_bool tp_reorder_fix,
                           vx_bool flush,
                           vx_bool small_batch_enable,
                           vx_bool axi_sram_slowed_down_by_ddr,
                           vx_float64 *ddr_kernel_read_bw, vx_float64 *ddr_in_image_read_bw,
                           vx_float64 *ddr_read_bw, vx_float64 *ddr_write_bw,
                           vx_float64 *axi_sram_read_bw, vx_float64 *axi_sram_write_bw
                           )
{
    vx_float64 ddrReadBandWidth = (vx_float64)tp_cmd_size;
    vx_float64 ddrWriteBandWidth = 0;
    vx_float64 ddrTotalBandWidth = 0;
    vx_float64 ddrReadCycleCount = 0;
    vx_float64 ddrWriteCycleCount = 0;
    vx_float64 ddrTotalCycleCount = 0;
    vx_float64 axiSRAMReadBandWidth = 0;
    vx_float64 axiSRAMWriteBandWidth = 0;
    vx_float64 axiSRAMReadCycleCount = 0;
    vx_float64 axiSRAMWriteCycleCount = 0;
    vx_float64 axiSRAMTotalCycleCount = 0;
    vx_float64 vipSRAMReadBandWidth = 0;
    vx_float64 vipSRAMWriteBandWidth = 0;
    vx_float64 vipSRAMReadCycleCount = 0;
    vx_float64 vipSRAMWriteCycleCount = 0;
    vx_float64 axiBusReadBandWidth = 0;
    vx_float64 axiBusWriteBandWidth = 0;
    vx_float64 axiBusReadCycleCount = 0;
    vx_float64 axiBusWriteCycleCount = 0;
    vx_float64 axiBusTotalCycleCount = 0;
    vx_float64 readBW, writeBW;
    vx_float64 compCycleCount = 0, tpCycleCountCore = 0, cacheControllerCycleCount = 0;
    vx_float64 adjustedAXISraReadBandWidthLimit = (vx_float64)axi_sram_read_bw_limit;
    vx_float64 ddrKernelReadBW = 0, ddrInImageReadBW = 0;
    vx_uint32 inz = z;
    if (type == VXNNE_OPERATOR_FULLYCONNECTED)
    {
        ddrKernelReadBW = x * y * z * kz * coef_compress_ratio * image_nonzero_ratio * data_size / 8; /*' always store kernel in DDR*/
        ddrReadBandWidth = ddrReadBandWidth + ddrKernelReadBW;
        inz = kz;
    }

    readBW = (vx_float64)(x * y * inz * data_size / 8);
    writeBW = (vx_float64)(x * y * z / (pooling_stride * pooling_stride));

    if (src_buf == SW_TILING_FROM_DDR)
    {
        ddrReadBandWidth = ddrReadBandWidth + readBW;
        ddrInImageReadBW = readBW;
    }
    else if (src_buf == SW_TILING_FROM_AXI_SRAM)
    {
        axiSRAMReadBandWidth = axiSRAMReadBandWidth + readBW;
        if ((type == VXNNE_OPERATOR_FULLYCONNECTED) && (kernel_buf == SW_TILING_FROM_DDR))
        {
            if (axi_sram_slowed_down_by_ddr)
            {
                vx_float64 maxOutstandingCycle = (vx_float64)outstanding_transfer * 4;
                if (total_latency > maxOutstandingCycle)
                {
                    vx_float64 bwLimitedByLatency = (16 * maxOutstandingCycle) / total_latency;
                    adjustedAXISraReadBandWidthLimit = gcmMIN(adjustedAXISraReadBandWidthLimit, bwLimitedByLatency);
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
    }
    else if (dst_buf == SW_TILING_FROM_AXI_SRAM)
    {
        axiSRAMWriteBandWidth = writeBW;
    }
    else if (dst_buf == SW_TILING_FROM_VIP_SRAM)
    {
        vipSRAMWriteBandWidth = writeBW;
    }


    axiBusReadBandWidth = ddrReadBandWidth + axiSRAMReadBandWidth;
    axiBusWriteBandWidth = ddrWriteBandWidth + axiSRAMWriteBandWidth;

    ddrTotalBandWidth = ddrReadBandWidth + ddrWriteBandWidth;

    ddrReadCycleCount = ddrReadBandWidth / ddr_read_bw_in_byte_per_cycle;
    ddrWriteCycleCount = ddrWriteBandWidth / ddr_write_bw_in_byte_per_cycle;
    ddrTotalCycleCount = ddrTotalBandWidth / ddr_total_bw_in_byte_per_cycle;

    axiSRAMReadCycleCount = axiSRAMReadBandWidth / adjustedAXISraReadBandWidthLimit;
    axiSRAMWriteCycleCount = axiSRAMWriteBandWidth / axi_sram_write_bw_limit;
    axiSRAMTotalCycleCount = (axiSRAMReadBandWidth + axiSRAMWriteBandWidth) / axi_sram_total_bw_limit;

    axiBusReadCycleCount = axiBusReadBandWidth / axi_bus_read_bw_limit;
    axiBusWriteCycleCount = axiBusWriteBandWidth / axi_bus_write_bw_limit;
    axiBusTotalCycleCount = (axiBusReadBandWidth + axiSRAMWriteBandWidth) / axi_bus_total_bw_limit;

    vipSRAMReadCycleCount = vipSRAMReadBandWidth / l2cache_width;
    vipSRAMWriteCycleCount = vipSRAMWriteBandWidth / 16;

    cacheControllerCycleCount = readBW / (4 * usc_cache_controllers); /* assume we do 4 byte per cache command */
    if (tp_reorder_fix ||
        ((type != VXNNE_OPERATOR_TENSOR_TRANS) && (type != VXNNE_OPERATOR_ROIPOOL)))
    {
        cacheControllerCycleCount += writeBW / (4 * usc_cache_controllers); /* assume we do 4 byte per cache command */
    }
    else
    {
        cacheControllerCycleCount += writeBW / usc_cache_controllers; /* assume we do 1 byte per cache command */
    }

    compCycleCount = _calcTPComputeCycleCount(type, x, y, z, kz, coef_nonzero_ratio, cores, image_nonzero_ratio);
    tpCycleCountCore = gcmMAX(ddrTotalCycleCount, gcmMAX(ddrWriteCycleCount, gcmMAX(ddrReadCycleCount, compCycleCount)));
    tpCycleCountCore = gcmMAX(axiSRAMTotalCycleCount, gcmMAX(axiSRAMWriteCycleCount, gcmMAX(axiSRAMReadCycleCount, tpCycleCountCore)));
    tpCycleCountCore = gcmMAX(axiBusTotalCycleCount, gcmMAX(axiBusWriteCycleCount, gcmMAX(axiBusReadCycleCount, tpCycleCountCore)));
    tpCycleCountCore = gcmMAX(vipSRAMWriteCycleCount, gcmMAX(vipSRAMReadCycleCount, tpCycleCountCore));
    tpCycleCountCore = gcmMAX(cacheControllerCycleCount, tpCycleCountCore);

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
    return tpCycleCountCore;
}


struct _archModelCost_u64
{
    vx_uint64 cycle;
    vx_uint64 bw;
};

vx_bool _cur_cost_u64_is_more_better(struct _archModelCost_u64 *cost, struct _archModelCost_u64 *cur)
{
    vx_float64 f = -(1.0f * (vx_int64)(cur->cycle - cost->cycle) / gcmMAX(cur->cycle, cost->cycle) * 20 + 1.0f * (vx_int64)(cur->bw - cost->bw) / gcmMAX(cur->bw, cost->bw) * 1);
    if (f > 0) return vx_true_e;
    return vx_false_e;
}

vx_status calculateArchPerf(
    vx_context context,
    vxnne_layer layer,
    vxnne_operation operation,
    vx_arch_perf perf,
    vx_weights_biases_parameter wb,
    vxnne_operation_target_e op_target,
    vxnne_operator_e op_type)
{
    /* version 0.29 - 0.50.5 */
    vx_uint32 numCores, tpCores, lanesPerConv, accuBuffDepth, adjustAccuBuffDepth, inputBuffDepth, inputBuffDepthForOneTile, l2CacheSize, l2CacheWidth, brickMode, swTiling, uscCacheSize, vip7Version, nnCmdSizeInBytes, tpCmdSizeInBytes;
    vx_uint32 inXSize, inYSize, outZSize, kernelXSize, kernelYSize, kernelZSize, poolingSize, poolingStride, inputDataSize, outputDataSize;
    vx_int32 xOffSet, yOffSet;
    vx_uint32 x, y, k;
    vx_uint32 tmpMaxOutTileYSize, tmpCovAccuMode, tmpCovMode, interleaveMode, cacheLineMode, maxInTileXSize, maxOutTileXSize, minOutTileXSize, minOutTileYSize;
    vx_bool initMinCost = vx_false_e;
    struct _archModelCost_u64 minCost, curCost;
    vx_float64 newCycleCount;
    vx_uint64  minCycleCount;
    vx_float64 newRDBandWidth, newNCRDBandWidth, newWTBandWidth;
    vx_uint64 minRDBandWidth, minNCRDBandWidth, minWTBandWidth;
    vx_float64 ddrKernelReadBandWidth, ddrInImageReadBandWidth, ddrRDBandWidth, ddrWTBandWidth,  axiRDBandWidth, axiWTBandWidth;
    vx_float64 coefNonZeroRatio=0.0f, coefCompressRatio=0.0f, imageCompressRatio=0.0f, imageNonZeroRatio=0.0f;
    vx_float32 axiSramReadBWLimit, axiSramWriteBWLimit, axiSramTotalBWLimit, axiBusReadBWLimit, axiBusWriteBWLimit, axiBusTotalBWLimit;
    vx_bool vip7_16bit, interleave8, fp16;
    vx_uint32 imagePartialCache, srcBuf, dstBuf, kernelBuf, cachedReadFromSram, vectorPrune, coefDecodePerf;
    vx_float32 ddrLatency = context->nnConfig.customizedFeature.ddrLatency;
    vx_uint32 convOutFifoDepth = 0;
    vx_uint32 uscCacheControllers = context->nnConfig.fixedFeature.uscCacheControllers;
    vx_bool tpReOrderFix = context->nnConfig.unifiedFeature.tpReOrderFix ? vx_true_e : vx_false_e;
    vx_bool fullCacheKernelHeadFix = context->nnConfig.unifiedFeature.fullCacheKernelHeadFix ? vx_true_e : vx_false_e;
    vx_bool conv1x1HalfPerformance = context->nnConfig.unifiedFeature.conv1x1HalfPerformance ? vx_true_e : vx_false_e;
    vx_bool cacheLineModeDisabled = context->nnConfig.unifiedFeature.cacheLineModeDisabled ? vx_true_e : vx_false_e;
    vx_bool per3DTileBubbleFix = context->nnConfig.unifiedFeature.per3DTileBubbleFix ? vx_true_e : vx_false_e;
    vx_bool zdp3NoCompressFix = context->nnConfig.unifiedFeature.zdp3NoCompressFix ? vx_true_e : vx_false_e;
    vx_bool asyncCopyPerfFix = context->nnConfig.unifiedFeature.asyncCopyPerfFix ? vx_true_e : vx_false_e;
    vx_bool zxdp3KernelReadConflictFix = context->nnConfig.unifiedFeature.zxdp3KernelReadConflictFix ? vx_true_e : vx_false_e;
    vx_bool accurateTileBW = context->nnConfig.unifiedFeature.accurateTileBW ? vx_true_e : vx_false_e;
    vx_bool isComputeBottleNeck = vx_false_e;
    vx_bool axiSramSlowedDownByAddr = context->nnConfig.unifiedFeature.axiSramSlowedDownByAddr ? vx_true_e : vx_false_e;
    vx_bool slowNNReqArbitrationFix = context->nnConfig.unifiedFeature.slowNNReqArbitrationFix ? vx_true_e : vx_false_e;
    vx_bool singlePortAccBuffer = context->nnConfig.unifiedFeature.singlePortAccBuffer ? vx_true_e : vx_false_e;
    vx_bool isDepthWise = (vx_bool)(context->nnConfig.customizedFeature.depthWiseSupport && (op_type == VXNNE_OPERATOR_DEPTH_WISE_CONV));
    vx_bool isNNWriteWithoutUSC = context->nnConfig.customizedFeature.nnWriteWithoutUSC ? vx_true_e : vx_false_e;
    vx_bool isV8 = (context->nnConfig.derivedFeature.nnXYDPX == 0 && context->nnConfig.derivedFeature.nnXYDPY == 0) ? vx_true_e : vx_false_e;
    vx_bool imageNotPackedInSram = context->nnConfig.unifiedFeature.imageNotPackedInSram ? vx_true_e : vx_false_e;
    vx_uint32 zrlBits = context->nnConfig.fixedFeature.zrlBits;
    vx_float32 totalLatency = context->nnConfig.derivedFeature.totalLatency;
    vx_uint32 outstandingTransfer = context->nnConfig.fixedFeature.maxOTNumber;
    vx_float32 ddrReadBWInBytePerCycle = 0, ddrWriteBWInBytePerCycle, ddrTotalBWInBytePerCycle, internalWriteBWLimit;
    vx_uint32 inSIXRefined = 0, inSIYRefined = 0;
    vx_uint32 inImageStride = 0, inImageSlice = 0;
    vx_uint32 outImageStride = 0, outImageSlice = 0;
    vx_uint32 axiAccUnitSizeInByte = 64;
    vx_uint32 vipSramAccUnitSizeInByte;
    vx_uint32 zdp = context->nnConfig.derivedFeature.nnZDP;
    vx_uint32 equivalentVipSramWidthInByte = context->nnConfig.fixedFeature.equivalentVipsramWidthInByte;
    vx_bool kernelPerCoreLTOneThirdCoefFix = context->nnConfig.unifiedFeature.kernelPerCoreLTOneThirdCoefFix ? vx_true_e : vx_false_e;
#ifdef USE_LIB_NN_ARCH_PERF
    APM_OUT_BW_T       outBandWidth = {0};
#endif
    perf->info.poolingStride = !perf->info.poolingSize ? 1 : gcmMAX(1, perf->info.poolingStride);
    perf->info.poolingSize = gcmMAX(1, perf->info.poolingSize);

    gcmASSERT(perf->info.kx && perf->info.inx && perf->info.inputDataSize);

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

        if (perf->info.inputDataFormat == VX_TYPE_FLOAT16)
            numCores       = context->nnConfig.fixedFeature.nnCoreCountFloat16;
        else if (perf->info.inputDataFormat == VX_TYPE_INT16)
            numCores       = context->nnConfig.fixedFeature.nnCoreCountInt16;
        else
            numCores       = context->nnConfig.fixedFeature.nnCoreCount;
        if (numCores == 0 && (op_target == VXNNE_OPERATION_TARGET_NN))
        {
            vxError("ERROR: not support input data format: %d\n", perf->info.inputDataFormat);
            vxmASSERT(0);
            return VX_FAILURE;
        }

        if (context->nnConfig.unifiedFeature.convOutFifoDepthFix)
        {
            convOutFifoDepth = (vx_uint32)ceilf(1.0f * context->nnConfig.fixedFeature.nnAccumBufferDepth * numCores * (64 - 8) / 64);
        }
        else
        {
            convOutFifoDepth = context->nnConfig.fixedFeature.nnAccumBufferDepth;
        }
        perf->info.convOutFifoDepth = convOutFifoDepth;
        perf->info.nnCores = numCores;
        tpCores = op_type != VXNNE_OPERATOR_FULLYCONNECTED ?
            context->nnConfig.fixedFeature.tpCoreCount : context->nnConfig.fixedFeature.tpCoreCount + context->nnConfig.fixedFeature.tpliteCoreCount;
        lanesPerConv   = context->nnConfig.unifiedFeature.lanesPerConv;
        maxInTileXSize = context->nnConfig.unifiedFeature.maxTileSize + 8;
        maxOutTileXSize = context->nnConfig.unifiedFeature.maxTileSize;
        inputBuffDepth = context->nnConfig.fixedFeature.nnInputBufferDepth;
        if ((context->nnConfig.derivedFeature.nnXYDPY >= 3) &&
            (inputDataSize == 8) &&
            (kernelXSize != 1 && kernelYSize != 1))
        {
            /* XYDP9. */
            inputBuffDepthForOneTile = inputBuffDepth / 2;
        }
        else if ((context->nnConfig.derivedFeature.nnZDP >= 6) &&
                 (kernelXSize == 1 && kernelYSize == 1))
        {
            /* ZDP6. */
            inputBuffDepthForOneTile = 32; /* A decent size. */
        }
        else
        {
            inputBuffDepthForOneTile = inputBuffDepth;
        }
        accuBuffDepth  = context->nnConfig.fixedFeature.nnAccumBufferDepth;
        gcmASSERT(perf->info.inputDataFormat != VX_TYPE_INVALID);
        if (perf->info.inputDataFormat == VX_TYPE_FLOAT16)
        {
            fp16 = vx_true_e;
        }
        else
        {
            fp16 = vx_false_e;
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

        vip7Version    = context->nnConfig.fixedFeature.vip7Version;

        uscCacheSize       = context->nnConfig.unifiedFeature.nnUSCCacheSize;
        l2CacheWidth       = context->nnConfig.fixedFeature.equivalentVipsramWidthInByte;
        vipSramAccUnitSizeInByte = l2CacheWidth * 2; /* x2 because we are using half freq SRAM */
        nnCmdSizeInBytes   = context->nnConfig.unifiedFeature.nnCmdSizeInBytes;
        tpCmdSizeInBytes   = context->nnConfig.unifiedFeature.tpCmdSizeInBytes;
        coefDecodePerf     = context->nnConfig.unifiedFeature.vipCoefDecodePerf;
        vectorPrune        = context->nnConfig.customizedFeature.vipVectorPrune;
        cachedReadFromSram = context->nnConfig.unifiedFeature.vipCachedReadFromSram;
        imagePartialCache  = context->nnConfig.unifiedFeature.vipImagePartialCache;
        brickMode          = context->nnConfig.fixedFeature.vipBrickMode;

        swTiling = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_SWTILING_PHASE1) ? vx_true_e : vx_false_e;

        vip7_16bit = (vip7Version && inputDataSize == 16) ? vx_true_e : vx_false_e;
        if (vip7_16bit)
        {
            maxOutTileXSize /= 2;
            maxInTileXSize /= 2;
        }

        interleave8 = vx_false_e; /*removed by arch perf revision 40*/
        context->nnConfig.fixedFeature.vipBrickMode = brickMode = (op_type != VXNNE_OPERATOR_CONVOLUTION && op_type != VXNNE_OPERATOR_DEPTH_WISE_CONV) ?
                            0 : vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_BRICK_MODE);

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

        ddrReadBWInBytePerCycle = context->nnConfig.derivedFeature.ddrReadBWInBytePerCycle;
        ddrWriteBWInBytePerCycle = context->nnConfig.derivedFeature.ddrWriteBWInBytePerCycle;
        ddrTotalBWInBytePerCycle = context->nnConfig.customizedFeature.ddrTotalBWLimit;
        internalWriteBWLimit = (vx_float32)context->nnConfig.fixedFeature.nnLanesPerOutCycle;
        axiSramReadBWLimit  = context->nnConfig.customizedFeature.axiSramReadBWLimit;
        axiSramWriteBWLimit = context->nnConfig.customizedFeature.axiSramWriteBWLimit;
        axiSramTotalBWLimit = context->nnConfig.customizedFeature.axiSramTotalBWLimit;
        axiBusReadBWLimit  = context->nnConfig.customizedFeature.axiBusReadBWLimit;
        axiBusWriteBWLimit = context->nnConfig.customizedFeature.axiBusWriteBWLimit;
        axiBusTotalBWLimit = context->nnConfig.customizedFeature.axiBusTotalBWLimit;

        l2CacheSize  = (perf->swTilingInfo.cacheSpace == -1) ?
            vxmARCH_VIP_SRAM_SIZE : perf->swTilingInfo.cacheSpace;

        outZSize = perf->info.outz;

        {
            if (wb != VX_NULL)
            {
                perf->maxPerCoreCompressionRatio = wb->max_per_core_compression_ratio;
                perf->coefNonZeroRatio  = WB_NON_ZERO_RATIO(wb);
                perf->coefCompressRatio = WB_COMPRESS_RATIO(wb);
                perf->imageCompressRatio = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_VIP_DEC400) ? 0.700000000000000f : 1.0f;
                if (perf->imageNonZeroRatio == 0)
                {
                    perf->imageNonZeroRatio  = 0.300000000000000;
                }
            }
            else
            {
                perf->coefNonZeroRatio   = 0.0f;
                perf->coefCompressRatio  = 0.0f;
                perf->imageCompressRatio = 0.0f;
                perf->imageNonZeroRatio  = 0.0f;
            }

            coefNonZeroRatio   = perf->coefNonZeroRatio;
            coefCompressRatio  = perf->coefCompressRatio;
            imageCompressRatio = perf->imageCompressRatio;
            imageNonZeroRatio  = perf->imageNonZeroRatio;

            if (op_target == VXNNE_OPERATION_TARGET_NN)
            {
                /* init to default */
                vx_uint32 tmpInX = perf->swTilingInfo.origInX + perf->info.kx - 1 + 2 * xOffSet;
                vx_uint32 tmpInY = perf->swTilingInfo.origInY + perf->info.ky - 1 + 2 * yOffSet;
                inSIXRefined = gcmMIN(tmpInX, perf->info.inx + perf->info.kx - 1);
                inSIYRefined = gcmMIN(tmpInY, perf->info.iny + perf->info.ky - 1);
                inImageStride = tmpInX;
                inImageSlice = tmpInX * tmpInY;
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
                        outImageStride = (vx_uint32)ceilf((vx_float32)(perf->info.inx - perf->info.p3) / perf->info.poolingStride);
                        outImageSlice = (vx_uint32)(outImageStride * ((vx_uint32)ceilf((vx_float32)(perf->info.iny - perf->info.p3) / perf->info.poolingStride) + perf->info.nextKY - 1));
                    }
                    else
                    {
                        outImageStride = perf->info.pix;
                        outImageSlice = perf->info.pix * perf->info.piy;
                    }
                }

                if (perf->resultInfo.calculated != HALF_DONE)
                {
                    perf->resultInfo.outImageTileXSize = 0;
                    perf->resultInfo.outImageTileYSize = 0;
                    perf->resultInfo.kernelsPerCore    = 0;
                    minCycleCount    = ~0ULL;
                    minRDBandWidth   = ~0ULL;
                    minNCRDBandWidth = ~0ULL;
                    minWTBandWidth   = ~0ULL;

                    maxOutTileXSize = gcmMIN(gcmMIN(inXSize, maxOutTileXSize), maxInTileXSize - kernelXSize + 1);
                    minOutTileXSize = gcmMAX((vx_int32)poolingSize, gcmMAX(-xOffSet - (vx_int32)kernelXSize + 1 + 1, 0));
                    minOutTileYSize = gcmMAX((vx_int32)poolingSize, gcmMAX(-yOffSet - (vx_int32)kernelYSize + 1 + 1, 0));
                    if (maxOutTileXSize < minOutTileXSize)
                    {
                        vxInfo("WARNING: maxOutTileXSize < minOutTileXSize\n");
                        goto errCalcArchPerf;
                    }

                    for (x = minOutTileXSize; x <= maxOutTileXSize; x++)
                    {
                        if ((poolingSize != 2 && poolingSize != 3) ||
                            (poolingSize == 2 && x % 2 == 0) ||
                            (poolingSize == 3 && x == inXSize))
                        {
                            interleaveMode = _calcImageInterleaveMode(
                                                  x,
                                                  context->nnConfig.unifiedFeature.maxTileSize,
                                                  kernelXSize,
                                                  vip7_16bit,
                                                  interleave8);

                            tmpCovMode = inputBuffDepthForOneTile * interleaveMode;
                            tmpCovAccuMode = adjustAccuBuffDepth * interleaveMode;

                            if (tmpCovMode < kernelYSize)
                            {
                                break;
                            }

                            tmpMaxOutTileYSize = gcmMIN(127, gcmMIN(tmpCovMode-kernelYSize+1, gcmMIN(tmpCovAccuMode, inYSize)));

                            if (tmpMaxOutTileYSize < minOutTileYSize)
                            {
                                if (poolingSize == 3)
                                {
                                    vxmASSERT("Hit NN Pooling Size = 3 limitation, either perform pooling in TP or split image into smaller vertical subimages\n" && 0);
                                }
                                continue;
                            }
                            for (y = minOutTileYSize; y <= tmpMaxOutTileYSize; y++)
                            {
                                if ((inXSize - xOffSet <= x + kernelXSize -1 &&
                                    inYSize - yOffSet <= y + kernelYSize -1) &&
                                    (y == inYSize && x == inXSize && context->nnConfig.unifiedFeature.diffConditionForCachelineModePreFix))
                                {
                                    cacheLineMode = 1;
                                }
                                else
                                {
                                    cacheLineMode = 0;
                                }
                                if ((brickMode || !cacheLineMode || (context->nnConfig.unifiedFeature.xyOffsetLimitationFix || (x == inXSize && y == inYSize))) &&
                                    ((poolingSize != 2 && poolingSize != 3) ||
                                     (poolingSize == 2 && y % 2 == 0) ||
                                     (poolingSize == 3 && (y != 3 || inYSize == 3))))
                                {
                                    vx_uint32 vipSramInimageStride = gcmMIN((x + kernelXSize - 1), inXSize);
                                    vx_uint32 vipSramInimageSlice = vipSramInimageStride * gcmMIN((y + kernelYSize - 1), inYSize);
                                    k = _calcNumOfKernel(x, y, outZSize, adjustAccuBuffDepth, numCores, interleaveMode,
                                        zdp, kernelXSize, kernelYSize, isV8, inputDataSize, lanesPerConv, isDepthWise, kernelPerCoreLTOneThirdCoefFix, poolingStride);
                                    if (!context->nnConfig.unifiedFeature.coefDeltaCordOverFlowZRL8BitFix &&
                                        ((kernelXSize == 2 && kernelYSize == 2) ||
                                         (kernelXSize == 1 && kernelYSize == 2) ||
                                         (kernelXSize == 1 && kernelYSize == 4) ||
                                         (kernelXSize == 1 && kernelYSize == 3)))
                                    {
                                        k = gcmMIN(63, k);
                                    }
                                    newRDBandWidth = _calcReadBandWidth(x, y, k, kernelXSize, kernelYSize, kernelZSize,
                                        inXSize, inYSize, outZSize, inSIXRefined, inSIYRefined,
                                        numCores, brickMode, inputDataSize, coefCompressRatio, imageCompressRatio,
                                        l2CacheSize, (vx_bool)imagePartialCache, nnCmdSizeInBytes, imageNotPackedInSram,
                                        fullCacheKernelHeadFix,
                                        context->nnConfig.unifiedFeature.cacheLineModeDisabled ? vx_true_e : vx_false_e,
                                        context->nnConfig.unifiedFeature.asyncCopyPerfFix ? vx_true_e : vx_false_e,
                                        context->nnConfig.unifiedFeature.accurateTileBW ? vx_true_e : vx_false_e,
                                        isDepthWise,
                                        inImageStride, inImageSlice, zdp, axiAccUnitSizeInByte, equivalentVipSramWidthInByte, NULL, NULL, NULL, NULL, NULL);
                                    newNCRDBandWidth = _calcReadBandWidth(x, y, k, kernelXSize, kernelYSize, kernelZSize,
                                        inXSize, inYSize, outZSize, inSIXRefined, inSIYRefined,
                                        numCores, brickMode, inputDataSize, coefCompressRatio, imageCompressRatio,
                                        0, (vx_bool)imagePartialCache, nnCmdSizeInBytes, imageNotPackedInSram,
                                        fullCacheKernelHeadFix,
                                        context->nnConfig.unifiedFeature.cacheLineModeDisabled ? vx_true_e : vx_false_e,
                                        context->nnConfig.unifiedFeature.asyncCopyPerfFix ? vx_true_e : vx_false_e,
                                        context->nnConfig.unifiedFeature.accurateTileBW ? vx_true_e : vx_false_e,
                                        isDepthWise,
                                        vipSramInimageStride, vipSramInimageSlice, zdp, vipSramAccUnitSizeInByte, equivalentVipSramWidthInByte, NULL, NULL, NULL, NULL, NULL);
                                    newWTBandWidth = _calcWriteBandWidth(x, y, inXSize, inYSize, outZSize,
                                        outputDataSize, imageCompressRatio, uscCacheSize, poolingStride, outImageStride, outImageSlice, isNNWriteWithoutUSC);

#ifdef USE_LIB_NN_ARCH_PERF
                                    if (context->apm)
                                    {
                                        APM_IN_PERF_PARAMS inPerfParams = {0};
                                        APM_COMM_INFO_T   *cmdInfo = &inPerfParams.cmdInfo;
                                        COMPRESSION_INFO_T *compInfo = &inPerfParams.compInfo;

                                        memset(&outBandWidth, 0, sizeof(outBandWidth));
                                        inPerfParams.bflush = perf->info.flush ? 1 : 0;
                                        inPerfParams.xydp_x = context->nnConfig.derivedFeature.nnXYDPX;
                                        inPerfParams.xydp_y = context->nnConfig.derivedFeature.nnXYDPY;
                                        inPerfParams.interleave_mode = interleaveMode;
                                        inPerfParams.vip_v7_16bit = vip7_16bit;
                                        inPerfParams.vector_prune = vectorPrune;
                                        inPerfParams.in_image_slice = inImageSlice;
                                        inPerfParams.out_image_slice = outImageSlice;
                                        inPerfParams.op_type = op_type;
                                        inPerfParams.inDataBitSize = inputDataSize;
                                        /*inPerfParams.outDataBitSize = outputDataSize;*/

                                        cmdInfo->outImageXsize  = inXSize;
                                        cmdInfo->outImageYsize  = inYSize;
                                        cmdInfo->outImageZsize = outZSize;
                                        cmdInfo->inSIXRefined  = inSIXRefined;
                                        cmdInfo->inSIYRefined  = inSIYRefined;
                                        cmdInfo->inImageFp16   = fp16;

                                        cmdInfo->u.nncmd.tile_x = x; /* tileXsize */
                                        cmdInfo->u.nncmd.tile_y = y; /* tileYsize */
                                        cmdInfo->u.nncmd.kernel_per_core = k; /* kernels per core */
                                        cmdInfo->kernelXsize = kernelXSize;
                                        cmdInfo->kernelYsize = kernelYSize;
                                        cmdInfo->kernelZsize = kernelZSize;
                                        cmdInfo->pooling_stride = poolingStride; /* can be NN command or tpcommand, refine me */
                                        cmdInfo->brick_mode = brickMode;
                                        cmdInfo->is_depth_wise = isDepthWise;
                                        cmdInfo->in_image_stride = inImageStride;
                                        cmdInfo->out_image_stride = outImageStride;

                                        cmdInfo->src_buf = srcBuf;    /* src buf in DDR or SRAM */
                                        cmdInfo->dst_buf = dstBuf;    /* dst buf in DDR or SRAM */
                                        cmdInfo->kernel_buf = kernelBuf; /*  kernel buf in DDR or SRAM */

                                        compInfo->coefNonZeroRatio = coefNonZeroRatio;
                                        compInfo->coefCompression = coefCompressRatio;
                                        compInfo->imageCompression = imageCompressRatio;
                                        compInfo->imageNonZeroRatio = imageNonZeroRatio;

                                        newCycleCount = APMCalcNNCycleCountBandWidth(context->apm, inPerfParams, &outBandWidth);
                                        ddrKernelReadBandWidth = outBandWidth.ddr_kernel_read_bandwidth;
                                        ddrInImageReadBandWidth = outBandWidth.ddr_in_image_read_bandwidth;
                                        ddrRDBandWidth = outBandWidth.ddr_read_bandwidth;
                                        ddrWTBandWidth = outBandWidth.ddr_write_bandwidth;
                                        axiRDBandWidth = outBandWidth.axi_read_bandwidth;
                                        axiWTBandWidth = outBandWidth.axi_write_bandwidth;
                                        isComputeBottleNeck = outBandWidth.is_compute_bottle_neck;
                                    }
                                    else
#endif
                                    {
                                        newCycleCount = _calcNNCycleCountBandWidth(
                                                                x, y, k,
                                                                inXSize, inYSize, outZSize,
                                                                kernelXSize, kernelYSize, kernelZSize,
                                                                inSIXRefined, inSIYRefined,
                                                                poolingStride,
                                                                coefNonZeroRatio, coefCompressRatio, imageCompressRatio,
                                                                numCores, brickMode, inputDataSize, outputDataSize,
                                                                l2CacheSize, l2CacheWidth,
                                                                context->nnConfig.derivedFeature.nnXYDPX,
                                                                context->nnConfig.derivedFeature.nnXYDPY,
                                                                zdp,
                                                                context->nnConfig.fixedFeature.nnFP16XYDPX,
                                                                context->nnConfig.fixedFeature.nnFP16XYDPY,
                                                                context->nnConfig.fixedFeature.nnFP16ZDP,
                                                                uscCacheSize,
                                                                nnCmdSizeInBytes,
                                                                coefDecodePerf,
                                                                vectorPrune,
                                                                imagePartialCache,
                                                                cachedReadFromSram,
                                                                1,
                                                                srcBuf,
                                                                dstBuf,
                                                                kernelBuf,
                                                                axiSramReadBWLimit,
                                                                axiSramWriteBWLimit,
                                                                axiSramTotalBWLimit,
                                                                axiBusReadBWLimit,
                                                                axiBusWriteBWLimit,
                                                                axiBusTotalBWLimit,
                                                                internalWriteBWLimit,
                                                                interleaveMode,
                                                                lanesPerConv,
                                                                outstandingTransfer,
                                                                zrlBits,
                                                                equivalentVipSramWidthInByte,
                                                                ddrLatency,
                                                                totalLatency,
                                                                ddrReadBWInBytePerCycle,
                                                                ddrWriteBWInBytePerCycle,
                                                                ddrTotalBWInBytePerCycle,
                                                                imageNotPackedInSram,
                                                                vip7_16bit,
                                                                fp16,
                                                                fullCacheKernelHeadFix,
                                                                conv1x1HalfPerformance,
                                                                cacheLineModeDisabled,
                                                                per3DTileBubbleFix,
                                                                zdp3NoCompressFix,
                                                                asyncCopyPerfFix,
                                                                zxdp3KernelReadConflictFix,
                                                                accurateTileBW,
                                                                axiSramSlowedDownByAddr,
                                                                slowNNReqArbitrationFix,
                                                                singlePortAccBuffer,
                                                                context->nnConfig.unifiedFeature.smallBatchEnable,
                                                                isDepthWise,
                                                                isNNWriteWithoutUSC,
                                                                inImageStride, inImageSlice,
                                                                outImageStride, outImageSlice,
                                                                perf->info.flush ? vx_true_e : vx_false_e,
                                                                convOutFifoDepth,
                                                                &ddrKernelReadBandWidth,
                                                                &ddrInImageReadBandWidth,
                                                                &ddrRDBandWidth,
                                                                &ddrWTBandWidth,
                                                                &axiRDBandWidth,
                                                                &axiWTBandWidth,
                                                                &isComputeBottleNeck);
                                    }
                                    curCost.cycle = (vx_uint64)(newCycleCount + 0.5f);
                                    curCost.bw = (vx_uint64)(newRDBandWidth + 0.5f) + (vx_uint64)(newWTBandWidth + 0.5f);
                                    if (!initMinCost || _cur_cost_u64_is_more_better(&minCost, &curCost))
                                    {
                                        initMinCost = vx_true_e;
                                        minCycleCount    = (vx_uint64)(newCycleCount + 0.5f);
                                        minRDBandWidth   = (vx_uint64)(newRDBandWidth + 0.5f);
                                        minNCRDBandWidth = (vx_uint64)(newNCRDBandWidth + 0.5f);
                                        minWTBandWidth   = (vx_uint64)(newWTBandWidth + 0.5f);
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
                                    }
                                }
                            }
                        }
                    }
                    vxmASSERT(perf->resultInfo.outImageTileXSize <= maxOutTileXSize);
                    vxmASSERT(perf->resultInfo.outImageTileXSize >= minOutTileXSize);
                    vxmASSERT(perf->resultInfo.outImageTileYSize >= minOutTileYSize);
                }
                else
                {
#ifdef USE_LIB_NN_ARCH_PERF
                    if (context->apm)
                    {
                        APM_IN_PERF_PARAMS inPerfParams = {0};
                        APM_COMM_INFO_T   *cmdInfo = &inPerfParams.cmdInfo;
                        COMPRESSION_INFO_T *compInfo = &inPerfParams.compInfo;

                        memset(&outBandWidth, 0, sizeof(outBandWidth));
                        inPerfParams.bflush = perf->info.flush ? 1 : 0;
                        inPerfParams.xydp_x = context->nnConfig.derivedFeature.nnXYDPX;
                        inPerfParams.xydp_y = context->nnConfig.derivedFeature.nnXYDPY;
                        inPerfParams.interleave_mode = perf->resultInfo.interleaveMode;
                        inPerfParams.vip_v7_16bit = vip7_16bit;
                        inPerfParams.vector_prune = vectorPrune;
                        inPerfParams.in_image_slice = inImageSlice;
                        inPerfParams.out_image_slice = outImageSlice;
                        inPerfParams.op_type = op_type;
                        inPerfParams.inDataBitSize = inputDataSize;
                        /*inPerfParams.outDataBitSize = outputDataSize;*/

                        cmdInfo->outImageXsize  = inXSize;
                        cmdInfo->outImageYsize  = inYSize;
                        cmdInfo->outImageZsize = outZSize;
                        cmdInfo->inSIXRefined  = inSIXRefined;
                        cmdInfo->inSIYRefined  = inSIYRefined;
                        cmdInfo->inImageFp16   = fp16;

                        cmdInfo->u.nncmd.tile_x = perf->resultInfo.outImageTileXSize; /* tileXsize */
                        cmdInfo->u.nncmd.tile_y = perf->resultInfo.outImageTileYSize; /* tileYsize */
                        cmdInfo->u.nncmd.kernel_per_core = perf->resultInfo.kernelsPerCore; /* kernels per core */
                        cmdInfo->kernelXsize = kernelXSize;
                        cmdInfo->kernelYsize = kernelYSize;
                        cmdInfo->kernelZsize = kernelZSize;
                        cmdInfo->pooling_stride = poolingStride; /* can be NN command or tpcommand, refine me */
                        cmdInfo->brick_mode = brickMode;
                        cmdInfo->is_depth_wise = isDepthWise;
                        cmdInfo->in_image_stride = inImageStride;
                        cmdInfo->out_image_stride = outImageStride;

                        cmdInfo->src_buf = srcBuf;    /* src buf in DDR or SRAM */
                        cmdInfo->dst_buf = dstBuf;    /* dst buf in DDR or SRAM */
                        cmdInfo->kernel_buf = kernelBuf; /*  kernel buf in DDR or SRAM */

                        compInfo->coefNonZeroRatio = coefNonZeroRatio;
                        compInfo->coefCompression = coefCompressRatio;
                        compInfo->imageCompression = imageCompressRatio;
                        compInfo->imageNonZeroRatio = imageNonZeroRatio;

                        perf->resultInfo.perfCycleCount = APMCalcNNCycleCountBandWidth(context->apm, inPerfParams, &outBandWidth);
                        ddrKernelReadBandWidth = outBandWidth.ddr_kernel_read_bandwidth;
                        ddrInImageReadBandWidth = outBandWidth.ddr_in_image_read_bandwidth;
                        ddrRDBandWidth = outBandWidth.ddr_read_bandwidth;
                        ddrWTBandWidth = outBandWidth.ddr_write_bandwidth;
                        axiRDBandWidth = outBandWidth.axi_read_bandwidth;
                        axiWTBandWidth = outBandWidth.axi_write_bandwidth;
                        isComputeBottleNeck = outBandWidth.is_compute_bottle_neck;
                    }
                    else
#endif
                    {
                        perf->resultInfo.perfCycleCount = _calcNNCycleCountBandWidth(
                                                        perf->resultInfo.outImageTileXSize,
                                                        perf->resultInfo.outImageTileYSize,
                                                        perf->resultInfo.kernelsPerCore,
                                                        inXSize, inYSize, outZSize,
                                                        kernelXSize, kernelYSize, kernelZSize,
                                                        inSIXRefined, inSIYRefined,
                                                        poolingStride,
                                                        coefNonZeroRatio, coefCompressRatio, imageCompressRatio,
                                                        numCores, brickMode, inputDataSize, outputDataSize,
                                                        l2CacheSize, l2CacheWidth,
                                                        context->nnConfig.derivedFeature.nnXYDPX,
                                                        context->nnConfig.derivedFeature.nnXYDPY,
                                                        context->nnConfig.derivedFeature.nnZDP,
                                                        context->nnConfig.fixedFeature.nnFP16XYDPX,
                                                        context->nnConfig.fixedFeature.nnFP16XYDPY,
                                                        context->nnConfig.fixedFeature.nnFP16ZDP,
                                                        uscCacheSize,
                                                        nnCmdSizeInBytes,
                                                        coefDecodePerf,
                                                        vectorPrune,
                                                        imagePartialCache,
                                                        cachedReadFromSram,
                                                        1,
                                                        srcBuf,
                                                        dstBuf,
                                                        kernelBuf,
                                                        axiSramReadBWLimit,
                                                        axiSramWriteBWLimit,
                                                        axiSramTotalBWLimit,
                                                        axiBusReadBWLimit,
                                                        axiBusWriteBWLimit,
                                                        axiBusTotalBWLimit,
                                                        internalWriteBWLimit,
                                                        perf->resultInfo.interleaveMode,
                                                        lanesPerConv,
                                                        outstandingTransfer,
                                                        zrlBits,
                                                        equivalentVipSramWidthInByte,
                                                        ddrLatency,
                                                        totalLatency,
                                                        ddrReadBWInBytePerCycle,
                                                        ddrWriteBWInBytePerCycle,
                                                        ddrTotalBWInBytePerCycle,
                                                        imageNotPackedInSram,
                                                        vip7_16bit,
                                                        fp16,
                                                        fullCacheKernelHeadFix,
                                                        conv1x1HalfPerformance,
                                                        cacheLineModeDisabled,
                                                        per3DTileBubbleFix,
                                                        zdp3NoCompressFix,
                                                        asyncCopyPerfFix,
                                                        zxdp3KernelReadConflictFix,
                                                        accurateTileBW,
                                                        axiSramSlowedDownByAddr,
                                                        slowNNReqArbitrationFix,
                                                        singlePortAccBuffer,
                                                        context->nnConfig.unifiedFeature.smallBatchEnable,
                                                        isDepthWise,
                                                        isNNWriteWithoutUSC,
                                                        inImageStride, inImageSlice,
                                                        outImageStride, outImageSlice,
                                                        perf->info.flush ? vx_true_e : vx_false_e,
                                                        convOutFifoDepth,
                                                        &ddrKernelReadBandWidth,
                                                        &ddrInImageReadBandWidth,
                                                        &ddrRDBandWidth,
                                                        &ddrWTBandWidth,
                                                        &axiRDBandWidth,
                                                        &axiWTBandWidth,
                                                        &isComputeBottleNeck);
                    }
                    perf->resultInfo.perfKernelReadBandWidth = ddrKernelReadBandWidth;
                    perf->resultInfo.perfInImageReadBandWidth = ddrInImageReadBandWidth;
                    perf->resultInfo.perfReadBandWidth = ddrRDBandWidth;
                    perf->resultInfo.perfWriteBandWidth = ddrWTBandWidth;
                    perf->resultInfo.perfAXIReadBandWidth = axiRDBandWidth;
                    perf->resultInfo.perfAXIWriteBandWidth = axiWTBandWidth;
                    perf->resultInfo.isFirstComputeBottleNeck = isComputeBottleNeck;
                }
            }
            else
            {
                perf->resultInfo.outImageTileXSize   = 0;
                perf->resultInfo.outImageTileYSize   = 0;
                perf->resultInfo.kernelsPerCore     = 0;

#ifdef USE_LIB_NN_ARCH_PERF
                if (context->apm)
                {
                    APM_IN_PERF_PARAMS inPerfParams = {0};
                    APM_COMM_INFO_T   *cmdInfo   = &inPerfParams.cmdInfo;
                    COMPRESSION_INFO_T *compInfo = &inPerfParams.compInfo;

                    memset(&outBandWidth, 0, sizeof(outBandWidth));
                    inPerfParams.bflush = perf->info.flush ? 1 : 0;
                    inPerfParams.xydp_x = context->nnConfig.derivedFeature.nnXYDPX;
                    inPerfParams.xydp_y = context->nnConfig.derivedFeature.nnXYDPY;
                    /*inPerfParams.interleave_mode = interleaveMode;*/
                    inPerfParams.vip_v7_16bit    = vip7_16bit;
                    inPerfParams.vector_prune    = vectorPrune;
                    inPerfParams.in_image_slice  = inImageSlice;
                    inPerfParams.out_image_slice = outImageSlice;
                    inPerfParams.op_type         = op_type;
                    inPerfParams.inDataBitSize   = inputDataSize;
                    /*inPerfParams.outDataBitSize   = outputDataSize;*/

                    cmdInfo->u.tpcmd.x   = inXSize;
                    cmdInfo->u.tpcmd.y   = inYSize;
                    cmdInfo->u.tpcmd.z   = outZSize;
                    cmdInfo->kernelZsize = kernelZSize;

                    cmdInfo->pooling_stride   = poolingStride; /* can be NN command or tpcommand, refine me */
                    cmdInfo->brick_mode       = brickMode;
                    cmdInfo->is_depth_wise    = isDepthWise;
                    cmdInfo->in_image_stride  = inImageStride;
                    cmdInfo->out_image_stride = outImageStride;

                    cmdInfo->src_buf = srcBuf;    /* src buf in DDR or SRAM */
                    cmdInfo->dst_buf = dstBuf;    /* dst buf in DDR or SRAM */
                    cmdInfo->kernel_buf = kernelBuf; /*  kernel buf in DDR or SRAM */

                    compInfo->coefNonZeroRatio = coefNonZeroRatio;
                    compInfo->coefCompression = coefCompressRatio;
                    compInfo->imageCompression = imageCompressRatio;
                    compInfo->imageNonZeroRatio = imageNonZeroRatio;

                    newCycleCount = APMCalcTPCycleCountCore(context->apm, inPerfParams, &outBandWidth);
                    ddrKernelReadBandWidth = outBandWidth.ddr_kernel_read_bandwidth;
                    ddrInImageReadBandWidth = outBandWidth.ddr_in_image_read_bandwidth;
                    ddrRDBandWidth = outBandWidth.ddr_read_bandwidth;
                    ddrWTBandWidth = outBandWidth.ddr_write_bandwidth;
                    axiRDBandWidth = outBandWidth.axi_read_bandwidth;
                    axiWTBandWidth = outBandWidth.axi_write_bandwidth;
                }
                else
#endif
                {
                    newCycleCount = _calcTPCycleCountCore(op_type, inXSize, inYSize, outZSize, kernelZSize,
                                                      coefNonZeroRatio, coefCompressRatio, imageNonZeroRatio,
                                                      tpCores, tpCmdSizeInBytes, poolingStride,
                                                      axiSramReadBWLimit, axiSramWriteBWLimit, axiSramTotalBWLimit,
                                                      axiBusReadBWLimit, axiBusWriteBWLimit, axiBusTotalBWLimit,
                                                      l2CacheWidth, inputDataSize, swTiling, srcBuf, dstBuf, kernelBuf,
                                                      outstandingTransfer,
                                                      ddrLatency,
                                                      totalLatency,
                                                      ddrReadBWInBytePerCycle,
                                                      ddrWriteBWInBytePerCycle,
                                                      ddrTotalBWInBytePerCycle,
                                                      uscCacheControllers,
                                                      tpReOrderFix,
                                                      (perf->info.flush ? vx_true_e : vx_false_e),
                                                      context->nnConfig.unifiedFeature.smallBatchEnable,
                                                      axiSramSlowedDownByAddr,
                                                      &ddrKernelReadBandWidth,
                                                      &ddrInImageReadBandWidth,
                                                      &ddrRDBandWidth,
                                                      &ddrWTBandWidth,
                                                      &axiRDBandWidth,
                                                      &axiWTBandWidth);
                }

                perf->resultInfo.perfCycleCount        = newCycleCount;
                perf->resultInfo.perfKernelReadBandWidth = ddrKernelReadBandWidth;
                perf->resultInfo.perfInImageReadBandWidth = ddrInImageReadBandWidth;
                perf->resultInfo.perfReadBandWidth     = ddrRDBandWidth;
                perf->resultInfo.perfWriteBandWidth    = ddrWTBandWidth;
                perf->resultInfo.perfAXIReadBandWidth  = axiRDBandWidth;
                perf->resultInfo.perfAXIWriteBandWidth = axiWTBandWidth;
            }
        }

        if (swTiling && op_target == VXNNE_OPERATION_TARGET_NN && perf->swTilingInfo.calcNonFirstCmd)
        {
            interleaveMode = _calcImageInterleaveMode(
                                  perf->resultInfo.outImageTileXSize,
                                  context->nnConfig.unifiedFeature.maxTileSize,
                                  kernelXSize,
                                  vip7_16bit,
                                  interleave8);
#ifdef USE_LIB_NN_ARCH_PERF
            if (context->apm)
            {
                APM_IN_PERF_PARAMS inPerfParams = {0};
                APM_COMM_INFO_T   *cmdInfo = &inPerfParams.cmdInfo;
                COMPRESSION_INFO_T *compInfo = &inPerfParams.compInfo;

                memset(&outBandWidth, 0, sizeof(outBandWidth));
                inPerfParams.bflush = perf->info.flush ? 1 : 0;
                inPerfParams.xydp_x = context->nnConfig.derivedFeature.nnXYDPX;
                inPerfParams.xydp_y = context->nnConfig.derivedFeature.nnXYDPY;
                inPerfParams.interleave_mode = interleaveMode;
                inPerfParams.vip_v7_16bit = vip7_16bit;
                inPerfParams.vector_prune = vectorPrune;
                inPerfParams.in_image_slice = inImageSlice;
                inPerfParams.out_image_slice = outImageSlice;
                inPerfParams.op_type = op_type;
                inPerfParams.inDataBitSize = inputDataSize;
                /*inPerfParams.outDataBitSize = outputDataSize;*/

                cmdInfo->outImageXsize  = inXSize;
                cmdInfo->outImageYsize  = inYSize;
                cmdInfo->outImageZsize = outZSize;
                cmdInfo->inSIXRefined  = inSIXRefined;
                cmdInfo->inSIYRefined  = inSIYRefined;
                cmdInfo->inImageFp16   = fp16;

                cmdInfo->u.nncmd.tile_x = perf->resultInfo.outImageTileXSize; /* tileXsize */
                cmdInfo->u.nncmd.tile_y = perf->resultInfo.outImageTileYSize; /* tileYsize */
                cmdInfo->u.nncmd.kernel_per_core = perf->resultInfo.kernelsPerCore; /* kernels per core */
                cmdInfo->kernelXsize = kernelXSize;
                cmdInfo->kernelYsize = kernelYSize;
                cmdInfo->kernelZsize = kernelZSize;
                cmdInfo->pooling_stride = poolingStride; /* can be NN command or tpcommand, refine me */
                cmdInfo->brick_mode = brickMode;
                cmdInfo->is_depth_wise = isDepthWise;
                cmdInfo->in_image_stride = inImageStride;
                cmdInfo->out_image_stride = outImageStride;
                cmdInfo->inSIXRefined = inSIXRefined;
                cmdInfo->inSIYRefined = inSIYRefined;

                cmdInfo->src_buf = srcBuf;    /* src buf in DDR or SRAM */
                cmdInfo->dst_buf = dstBuf;    /* dst buf in DDR or SRAM */
                cmdInfo->kernel_buf = kernelBuf; /*  kernel buf in DDR or SRAM */

                compInfo->coefNonZeroRatio = coefNonZeroRatio;
                compInfo->coefCompression = coefCompressRatio;
                compInfo->imageCompression = imageCompressRatio;
                compInfo->imageNonZeroRatio = imageNonZeroRatio;

                perf->swTilingInfo.perfNonFirstCycleCount = APMCalcNNCycleCountBandWidth(context->apm, inPerfParams, &outBandWidth);
                ddrKernelReadBandWidth = outBandWidth.ddr_kernel_read_bandwidth;
                ddrInImageReadBandWidth = outBandWidth.ddr_in_image_read_bandwidth;
                ddrRDBandWidth = outBandWidth.ddr_read_bandwidth;
                ddrWTBandWidth = outBandWidth.ddr_write_bandwidth;
                axiRDBandWidth = outBandWidth.axi_read_bandwidth;
                axiWTBandWidth = outBandWidth.axi_write_bandwidth;
                isComputeBottleNeck = outBandWidth.is_compute_bottle_neck;
            }
            else
#endif
            {
                perf->swTilingInfo.perfNonFirstCycleCount = _calcNNCycleCountBandWidth(
                                                           perf->resultInfo.outImageTileXSize,
                                                           perf->resultInfo.outImageTileYSize,
                                                           perf->resultInfo.kernelsPerCore,
                                                           inXSize, inYSize, outZSize,
                                                           kernelXSize, kernelYSize, kernelZSize,
                                                           inSIXRefined, inSIYRefined,
                                                           poolingStride,
                                                           coefNonZeroRatio, coefCompressRatio, imageCompressRatio,
                                                           numCores, brickMode, inputDataSize, outputDataSize,
                                                           l2CacheSize, l2CacheWidth,
                                                           context->nnConfig.derivedFeature.nnXYDPX,
                                                           context->nnConfig.derivedFeature.nnXYDPY,
                                                           zdp,
                                                           context->nnConfig.fixedFeature.nnFP16XYDPX,
                                                           context->nnConfig.fixedFeature.nnFP16XYDPY,
                                                           context->nnConfig.fixedFeature.nnFP16ZDP,
                                                           uscCacheSize,
                                                           nnCmdSizeInBytes,
                                                           coefDecodePerf,
                                                           vectorPrune,
                                                           imagePartialCache,
                                                           cachedReadFromSram,
                                                           0,
                                                           srcBuf,
                                                           dstBuf,
                                                           kernelBuf,
                                                           axiSramReadBWLimit,
                                                           axiSramWriteBWLimit,
                                                           axiSramTotalBWLimit,
                                                           axiBusReadBWLimit,
                                                           axiBusWriteBWLimit,
                                                           axiBusTotalBWLimit,
                                                           internalWriteBWLimit,
                                                           interleaveMode,
                                                           lanesPerConv,
                                                           outstandingTransfer,
                                                           zrlBits,
                                                           equivalentVipSramWidthInByte,
                                                           ddrLatency,
                                                           totalLatency,
                                                           ddrReadBWInBytePerCycle,
                                                           ddrWriteBWInBytePerCycle,
                                                           ddrTotalBWInBytePerCycle,
                                                           imageNotPackedInSram,
                                                           vip7_16bit,
                                                           fp16,
                                                           fullCacheKernelHeadFix,
                                                           conv1x1HalfPerformance,
                                                           cacheLineModeDisabled,
                                                           per3DTileBubbleFix,
                                                           zdp3NoCompressFix,
                                                           asyncCopyPerfFix,
                                                           zxdp3KernelReadConflictFix,
                                                           accurateTileBW,
                                                           axiSramSlowedDownByAddr,
                                                           slowNNReqArbitrationFix,
                                                           singlePortAccBuffer,
                                                           context->nnConfig.unifiedFeature.smallBatchEnable,
                                                           isDepthWise,
                                                           isNNWriteWithoutUSC,
                                                           inImageStride, inImageSlice,
                                                           outImageStride, outImageSlice,
                                                           perf->info.flush ? vx_true_e : vx_false_e,
                                                           convOutFifoDepth,
                                                           &ddrKernelReadBandWidth,
                                                           &ddrInImageReadBandWidth,
                                                           &ddrRDBandWidth,
                                                           &ddrWTBandWidth,
                                                           &axiRDBandWidth,
                                                           &axiWTBandWidth,
                                                           &isComputeBottleNeck);
            }
            perf->swTilingInfo.perfNonFirstKernelReadBandWidth = ddrKernelReadBandWidth;
            perf->swTilingInfo.perfNonFirstInImageReadBandWidth = ddrInImageReadBandWidth;
            perf->swTilingInfo.perfNonFirstReadBandWidth = ddrRDBandWidth;
            perf->swTilingInfo.perfNonFirstWriteBandWidth = ddrWTBandWidth;
            perf->swTilingInfo.perfNonFirstAXIReadBandWidth = axiRDBandWidth;
            perf->swTilingInfo.perfNonFirstAXIWriteBandWidth = axiWTBandWidth;
            perf->swTilingInfo.isNonFirstComputeBottleNeck = isComputeBottleNeck;
        }

        perf->calculated = vx_true_e;

        if ((wb != NULL) && (wb->weights_sizes[0] == 1 && wb->weights_sizes[1] == 1
            && (vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP3) || vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6))
            && (wb->wb_base->weights_data_format == VX_TYPE_INT8 || wb->wb_base->weights_data_format == VX_TYPE_UINT8)) && !isV8)
        {
            /*Per HW, there's a limition for HW now, arch perf's kernel per core should less equre than (accuBuffDepth / zdpNum) when zdp3 & zdp6*/
            vx_uint32 zdpNum = vxoContext_IsFeatureAvailable(context, VX_NN_FEATURE_ZDP6) ? 6 : 3;
            if (!(perf->resultInfo.kernelsPerCore <= (accuBuffDepth / zdpNum)))
            {
                vxError("Assert: perf->resultInfo.kernelsPerCore <= (accuBuffDepth / zdpNum) must be true: kernelPerCore: %d, accuBuffDepth: %d, zdpNum: %d\n", perf->resultInfo.kernelsPerCore, accuBuffDepth, zdpNum);
            }
            vxmASSERT(perf->resultInfo.kernelsPerCore <= (accuBuffDepth / zdpNum));
            zdpNum++; /* Make release build pass*/
        }
    }

    perf->opType = op_type;
    perf->opTarget = op_target;
    return VX_SUCCESS;

errCalcArchPerf:
    perf->calculated = vx_true_e;
    perf->resultInfo.outImageTileXSize   = 0;
    perf->resultInfo.outImageTileYSize   = 0;
    perf->resultInfo.kernelsPerCore     = 0;
    perf->opType = op_type;
    perf->opTarget = op_target;
    return VX_FAILURE;
}


VX_INTERNAL_API void calculateArchPerfFromTiling(
    vx_context context,
    vxnne_layer layer,
    vx_arch_perf perf,
    vxnne_tensor_info input_tiling,
    vxnne_tensor_info output_tiling,
    vx_tensor input,
    vx_tensor output,
    vx_weights_biases_parameter wb,
    vxnne_operation_command  op_command,
    vxnne_operation_target_e op_target,
    vxnne_operator_e op_type)
{
    vx_uint32 kernelXSize, kernelYSize, kernelZSize, poolingSize, poolingStride, strideX, strideY;
    vx_int32 inputDataSize, outputDataSize, xOffSet, yOffSet;
    vx_op_param conv_cmd = &op_command->parameter;
    /*vx_bool axiSramOnlySWTiling = context->nnConfig.unifiedFeature.axiSramOnlySWTiling ? vx_true_e : vx_false_e;*/

    kernelXSize = wb != VX_NULL && op_type != VXNNE_OPERATOR_RESHUFFLE ? WB_KERNEL_X(wb) : 1;
    kernelYSize = wb != VX_NULL && op_type != VXNNE_OPERATOR_RESHUFFLE ? WB_KERNEL_Y(wb) : 1;
    kernelZSize = wb != VX_NULL ? ((op_type == VXNNE_OPERATOR_DEPTH_WISE_CONV && wb->wb_base->hw_depth_wise) ? wb->weights_sizes[3] : WB_KERNEL_Z(wb))
                                : input_tiling->depth;

    xOffSet = (-1) * conv_cmd->pad_x_left;
    yOffSet = (-1) * conv_cmd->pad_y_top;

    poolingStride = !conv_cmd->pool_size_y ? 1 : gcmMAX(1, conv_cmd->pool_stride);
    poolingSize = gcmMAX(1, conv_cmd->pool_size_y);

    inputDataSize = 8 * TENSOR_DATA_SIZE(input);
    outputDataSize = 8 * TENSOR_DATA_SIZE(output);
    strideX = op_type == VXNNE_OPERATOR_RESHUFFLE ? WB_STRIDE_X(wb) : 1;
    strideY = op_type == VXNNE_OPERATOR_RESHUFFLE ? WB_STRIDE_Y(wb) : 1;

    /********** left perf structure configuration **********/
    perf->info.kx = kernelXSize;
    perf->info.ky = kernelYSize;
    perf->info.kz = kernelZSize;
    perf->info.oinx = TENSOR_VIEW_SIZE_INDEX(input, 0);
    perf->info.oiny = TENSOR_VIEW_SIZE_INDEX(input, 1);
    perf->info.oinz = TENSOR_VIEW_SIZE_INDEX(input, 2);
    perf->info.stridex = strideX;
    perf->info.stridey = strideY;
    perf->info.inputDataSize = inputDataSize;
    perf->info.outputDataSize = outputDataSize;
    perf->info.poolingSize = poolingSize;
    perf->info.poolingStride = poolingStride;
    perf->info.xOffSet = xOffSet;
    perf->info.yOffSet = yOffSet;
    perf->info.inputDataFormat = TENSOR_DATA_TYPE(input);
    perf->info.outputDataFormat = TENSOR_DATA_TYPE(output);
    perf->info.p3 = (poolingSize == 3) ? 1 : 0;

    perf->info.inx = op_type == VXNNE_OPERATOR_POOLING ? input_tiling->width : output_tiling->width;
    perf->info.iny = op_type == VXNNE_OPERATOR_POOLING ? input_tiling->height : output_tiling->height;
    perf->info.outz = output_tiling->depth;
    perf->info.pix = (vx_uint32)ceilf((vx_float32)(output_tiling->width - perf->info.p3) / poolingStride);
    perf->info.piy = (vx_uint32)ceilf((vx_float32)(output_tiling->height - perf->info.p3) / poolingStride);

    if ((op_target != VXNNE_OPERATION_TARGET_TP || op_type == VXNNE_OPERATOR_FULLYCONNECTED) && wb)
        perf->info.kernelSize = (vx_uint32)(gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(wb, 0), CACHE_ALIGNMENT_SIZE));
    perf->swTilingInfo.origOutX = TENSOR_VIEW_SIZE_INDEX(output, 0);
    perf->swTilingInfo.origOutY = TENSOR_VIEW_SIZE_INDEX(output, 1);
    perf->swTilingInfo.origOutZ = TENSOR_VIEW_SIZE_INDEX(output, 2);
    perf->swTilingInfo.cacheSpace = vxmARCH_VIP_SRAM_SIZE;
    if (op_type == VXNNE_OPERATOR_CONVOLUTION || op_type == VXNNE_OPERATOR_DEPTH_WISE_CONV)
    {
        /* original output tensor might be reshaped, cannot use original output tensor size at here */
        /* use reshaped sized to calc imageStride and imageSlice */
        perf->swTilingInfo.outImageStride = output_tiling->width;
        perf->swTilingInfo.outImageSlice = output_tiling->width * output_tiling->height;

        perf->swTilingInfo.origInX = output_tiling->width;
        perf->swTilingInfo.origInY = output_tiling->height;
    }
    else
    {
        perf->swTilingInfo.origInX = perf->info.oinx;
        perf->swTilingInfo.origInY = perf->info.oiny;
    }

    if (op_type == VXNNE_OPERATOR_FULLYCONNECTED && op_target == VXNNE_OPERATION_TARGET_TP)
    {
        /*convert TP FC input/output info for arch model analysis when dims<=2 */
        vx_uint32 inDims = input->dimCount;
        vx_uint32 outDims = output->dimCount;
        if ((inDims == 2) || (inDims == 1))
        {
            perf->info.inx = 1;
            perf->info.iny = 1;
            perf->info.pix = 1;
            perf->info.piy = 1;
            perf->info.oinx = 1;
            perf->info.oinx = 1;
            perf->swTilingInfo.origInX = 1;
            perf->swTilingInfo.origInY = 1;
            perf->info.inz = TENSOR_VIEW_SIZE_INDEX(input, 0) * TENSOR_VIEW_SIZE_INDEX(input, 1) * TENSOR_VIEW_SIZE_INDEX(input, 2);
            perf->info.oinz = perf->info.inz;
        }

        if (((outDims == 2) || (outDims == 1)) && (wb != VX_NULL))
        {
            perf->info.inx = 1;
            perf->info.iny = 1;
            perf->info.pix = 1;
            perf->info.piy = 1;
            perf->swTilingInfo.origOutX = 1;
            perf->swTilingInfo.origOutY = 1;
            perf->swTilingInfo.origOutZ = wb->weights_sizes[3];
            perf->info.outz = wb->weights_sizes[3];
            perf->swTilingInfo.origInX = perf->info.oinx;
            perf->swTilingInfo.origInY = perf->info.oiny;
        }
    }

    perf->swTilingInfo.srcBuf = (!input_tiling->sRAM) ? SW_TILING_FROM_DDR :
                                (input_tiling->sRAM == VXNNE_MEM_POOL_TYPE_AXI_SRAM) ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
    perf->swTilingInfo.dstBuf = (!output_tiling->sRAM) ? SW_TILING_FROM_DDR :
                                (output_tiling->sRAM == VXNNE_MEM_POOL_TYPE_AXI_SRAM) ? SW_TILING_FROM_AXI_SRAM : SW_TILING_FROM_VIP_SRAM;
    perf->swTilingInfo.kernelBuf = (op_command->cmdInfo.kernelCacheMode == VXNNE_SRAM_CACHE_MODE_STREAM_CACHE) ? SW_TILING_FROM_VIP_SRAM : SW_TILING_FROM_DDR;

    perf->info.flush = op_command->cmdInfo.flush;

    calculateArchPerf(context, layer, VX_NULL, perf, wb, op_target, op_type);
}

VX_INTERNAL_API void calculateArchPerfFromWB(
    vx_context context,
    vx_arch_perf perf,
    vx_weights_biases_parameter wb,
    vx_uint32 orig_input_dims[],
    vx_uint32 output_dims[],
    vx_enum output_format,
    vx_int32* offsets,
    vx_int32 flush,
    vx_uint8 src_buf,
    vx_uint8 dst_buf,
    vx_uint8 kernel_buf,
    vx_int32 cached_space,
    vxnne_operation_target_e op_target,
    vxnne_operator_e op_type
    )
{
    vx_uint32 outXSize, outYSize, kernelXSize, kernelYSize, kernelZSize, poolingSize, poolingStride;
    vx_int32 inputDataSize, outputDataSize, xOffSet, yOffSet;

    ComputeInputSize(
        output_dims[0],
        WB_KERNEL_X(wb),
        WB_PAD_LEFT(wb),
        WB_PAD_RIGHT(wb),
        WB_POOLING_SIZE_X(wb),
        WB_POOLING_STRIDE(wb),
        &outXSize,
        1);

    ComputeInputSize(
        output_dims[1],
        WB_KERNEL_Y(wb),
        WB_PAD_TOP(wb),
        WB_PAD_BOTTOM(wb),
        WB_POOLING_SIZE_Y(wb),
        WB_POOLING_STRIDE(wb),
        &outYSize,
        1);

    kernelXSize = WB_KERNEL_X(wb);
    kernelYSize = WB_KERNEL_Y(wb);
    kernelZSize = (op_type == VXNNE_OPERATOR_DEPTH_WISE_CONV && wb->wb_base->hw_depth_wise) ? wb->weights_sizes[3] : WB_KERNEL_Z(wb);
    poolingSize = gcmMAX(1, WB_POOLING_SIZE_X(wb));
    poolingStride = WB_POOLING_SIZE_X(wb) ? 2 : 1;
    xOffSet = offsets == VX_NULL ? WB_STRIDE_X(wb) > 1 ? 0 : ((-1) * WB_PAD_LEFT(wb)) : offsets[0];
    yOffSet = offsets == VX_NULL ? WB_STRIDE_Y(wb) > 1 ? 0 : ((-1) *  WB_PAD_TOP(wb)) : offsets[1];
    inputDataSize = 8 * (vx_uint32)vxDataType_GetSize((vx_type_e)WB_WEIGHT_DATA_FORMAT(wb));
    outputDataSize = 8 * (vx_uint32)vxDataType_GetSize((vx_type_e)output_format);

    /********** left perf structure configuration **********/
    perf->info.kx = kernelXSize;
    perf->info.ky = kernelYSize;
    perf->info.kz = kernelZSize;
    perf->info.oinx = orig_input_dims[0];
    perf->info.oiny = orig_input_dims[1];
    perf->info.oinz = orig_input_dims[2];
    if (op_type == VXNNE_OPERATOR_POOLING)
    {
        perf->info.inx = orig_input_dims[0];
        perf->info.iny = orig_input_dims[1];
        perf->info.inz = orig_input_dims[2];
    }
    else
    {
        perf->info.inx = outXSize;
        perf->info.iny = outYSize;
        perf->info.inz = kernelZSize;
    }
    perf->info.outx = output_dims[0];
    perf->info.outy = output_dims[1];
    perf->info.outz = output_dims[2];
    perf->info.stridex = WB_STRIDE_X(wb);
    perf->info.stridey = WB_STRIDE_Y(wb);
    perf->info.inputDataSize = inputDataSize;
    perf->info.outputDataSize = outputDataSize;
    perf->info.poolingSize = poolingSize;
    perf->info.poolingStride = poolingStride;
    perf->info.xOffSet = xOffSet;
    perf->info.yOffSet = yOffSet;

    if (op_type == VXNNE_OPERATOR_CONVOLUTION || op_type == VXNNE_OPERATOR_DEPTH_WISE_CONV)
    {
        perf->swTilingInfo.origInX = perf->info.inx;
        perf->swTilingInfo.origInY = perf->info.iny;
    }
    else
    {
        perf->swTilingInfo.origInX = perf->info.oinx;
        perf->swTilingInfo.origInY = perf->info.oiny;
    }
    perf->swTilingInfo.cacheSpace = cached_space;
    perf->swTilingInfo.srcBuf = src_buf;
    perf->swTilingInfo.dstBuf = dst_buf;
    perf->swTilingInfo.kernelBuf = kernel_buf;

    perf->info.p3 = (poolingSize == 3) ? 1: 0;
    perf->info.pix = (vx_uint32)ceilf((vx_float32)(outXSize - perf->info.p3) / poolingStride);
    perf->info.piy = (vx_uint32)ceilf((vx_float32)(outYSize - perf->info.p3) / poolingStride);
    if ((op_target != VXNNE_OPERATION_TARGET_TP || op_type == VXNNE_OPERATOR_FULLYCONNECTED) && wb)
      perf->info.kernelSize = (vx_uint32)(gcmALIGN_NP2(WB_STREAM_ALIGN_SIZE_INDEX(wb, 0), CACHE_ALIGNMENT_SIZE));
    /* use wb's format as input data format for arch model performance calculation */
    perf->info.inputDataFormat = WB_WEIGHT_DATA_FORMAT(wb);
    perf->info.flush = 1;
    calculateArchPerf(context, VX_NULL, VX_NULL, perf, wb, op_target, op_type);
}
