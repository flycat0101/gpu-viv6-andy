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
#include <gc_vx_internal_node_api.h>

VX_INTERNAL_API vx_node vxSobelMxNNode(vx_graph graph, vx_image input, vx_scalar ws, vx_image gx, vx_image gy)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)ws,
        (vx_reference)gx,
        (vx_reference)gy
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SOBEL_MxN, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxHarrisScoreNode(
        vx_graph graph, vx_image gx, vx_image gy, vx_scalar sensitivity, vx_scalar winSize, vx_scalar blockSize, vx_image score)
{
    vx_reference parameters[] = {
        (vx_reference)gx,
        (vx_reference)gy,
        (vx_reference)sensitivity,
        (vx_reference)winSize,
        (vx_reference)blockSize,
        (vx_reference)score
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_HARRIS_SCORE, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEuclideanNonMaxNode(
        vx_graph graph, vx_image input, vx_scalar strengthThresh, vx_scalar minDistance, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)strengthThresh,
        (vx_reference)minDistance,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(
                graph, VX_KERNEL_INTERNAL_EUCLIDEAN_NONMAXSUPPRESSION, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxImageListerNode(vx_graph graph, vx_image input, vx_array arr, vx_scalar numPoints)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)arr,
        (vx_reference)numPoints
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_IMAGE_LISTER, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxElementwiseNormNode(
        vx_graph graph, vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input_x,
        (vx_reference)input_y,
        (vx_reference)norm_type,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_ELEMENTWISE_NORM, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxNonMaxSuppressionNode(vx_graph graph, vx_image mag, vx_image phase, vx_image edge)
{
    vx_reference parameters[] = {
        (vx_reference)mag,
        (vx_reference)phase,
        (vx_reference)edge
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_NONMAXSUPPRESSION, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEdgeTraceNode(vx_graph graph, vx_image norm, vx_threshold threshold, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)norm,
        (vx_reference)threshold,
        (vx_reference)output,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EDGE_TRACE, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCopyImageNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_COPY_IMAGE, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxFast9CornersStrengthNode(vx_graph graph, vx_image input, vx_scalar t, vx_scalar do_nonmax, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)t,
        (vx_reference)do_nonmax,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_FAST9CORNERS_STRENGTH, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxFast9CornersNonMaxNode(vx_graph graph, vx_image input, vx_scalar t, vx_scalar do_nonmax, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)t,
        (vx_reference)do_nonmax,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_FAST9CORNERS_NONMAX, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCreateListerNode(vx_graph graph, vx_image input, vx_image countImg, vx_array array)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)countImg,
        (vx_reference)array
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CREATE_LISTER, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxPackArraysNode(vx_graph graph, vx_image inputImage, vx_array inputArray, vx_scalar widthScalar, vx_scalar heightScalar, vx_array dstArray, vx_scalar numScalar)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)inputArray,
        (vx_reference)widthScalar,
        (vx_reference)heightScalar,
        (vx_reference)dstArray,
        (vx_reference)numScalar
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_PACK_ARRAYS, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxMinMaxLocFilterNode(vx_graph graph, vx_image inputImage, vx_scalar filterMin, vx_scalar filterMax)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)filterMin,
        (vx_reference)filterMax
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_MINMAXLOC_FILTER, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxGetLocationNode(vx_graph graph, vx_image inputImage, vx_scalar filterMinValue, vx_scalar filterMaxValue,
                  vx_image minImage, vx_image maxImage, vx_array minArray, vx_array maxArray, vx_scalar filterMinCount, vx_scalar filterMaxCount)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)filterMinValue,
        (vx_reference)filterMaxValue,
        (vx_reference)minImage,
        (vx_reference)maxImage,
        (vx_reference)minArray,
        (vx_reference)maxArray,
        (vx_reference)filterMinCount,
        (vx_reference)filterMaxCount
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_GET_LOCATION, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxMinMaxLocPackArrayNode(vx_graph graph, vx_image inputImage, vx_array inputArray, vx_scalar widthScalar, vx_scalar heightScalar, vx_scalar countScalar, vx_array dstArray)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)inputArray,
        (vx_reference)widthScalar,
        (vx_reference)heightScalar,
        (vx_reference)countScalar,
        (vx_reference)dstArray
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_MIN_MAX_LOC_PACK_ARRAYS, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEdgeTraceThresholdNode(vx_graph graph, vx_image inputImage, vx_threshold threshold, vx_image outputImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)threshold,
        (vx_reference)outputImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EDGE_TRACE_THRESHOLD, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEdgeTraceHysteresisNode(vx_graph graph, vx_image image, vx_scalar flag)
{
    vx_reference parameters[] = {
        (vx_reference)image,
        (vx_reference)flag
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EDGE_TRACE_HYSTERESIS, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEdgeTraceClampNode(vx_graph graph, vx_image inputImage, vx_image outputImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)outputImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EDGE_TRACE_CLAMP, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxIntegralImageStepNode(vx_graph graph, vx_image inputImage, vx_scalar stepScalar, vx_image outputImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)stepScalar,
        (vx_reference)outputImage,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_INTEGRAL_IMAGE_STEP, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxScharr3x3Node(vx_graph graph, vx_image inputImage, vx_image gradXImage, vx_image gradYImage)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)gradXImage,
        (vx_reference)gradYImage,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SCHARR3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxVLKTrackerNode(vx_graph graph, vx_pyramid oldPyramid, vx_pyramid newPyramid, vx_pyramid gradXPyramid, vx_pyramid gradYPyramid,
                                         vx_array prevPts, vx_array estimatedPts, vx_array nextPts,
                                         vx_scalar criteriaScalar, vx_scalar epsilonScalar, vx_scalar numIterationsScalar, vx_scalar isUseInitialEstimateScalar,
                                         vx_scalar winSizeScalar)
{
    vx_reference parameters[] = {
        (vx_reference)oldPyramid,
        (vx_reference)newPyramid,
        (vx_reference)gradXPyramid,
        (vx_reference)gradYPyramid,
        (vx_reference)prevPts,
        (vx_reference)estimatedPts,
        (vx_reference)nextPts,
        (vx_reference)criteriaScalar,
        (vx_reference)epsilonScalar,
        (vx_reference)numIterationsScalar,
        (vx_reference)isUseInitialEstimateScalar,
        (vx_reference)winSizeScalar
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_VLK_TRACKER, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEqualizeHistHistNode(vx_graph graph, vx_image srcImage, vx_image histImage, vx_scalar minIndexScalar)
{
    vx_reference parameters[] = {
        (vx_reference)srcImage,
        (vx_reference)histImage,
        (vx_reference)minIndexScalar
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_HIST, parameters, vxmLENGTH_OF(parameters));
}


VX_INTERNAL_API vx_node vxEqualizeHistGcdfNode(vx_graph graph, vx_image histImage, vx_scalar minIndexScalar, vx_image cdfImage, vx_scalar minValueScalar)
{
    vx_reference parameters[] = {
        (vx_reference)histImage,
        (vx_reference)minIndexScalar,
        (vx_reference)cdfImage,
        (vx_reference)minValueScalar
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_GCDF, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEqualizeHistCdfNode(vx_graph graph, vx_image srcImage, vx_image cdfImage, vx_scalar minValueScalar, vx_image histImage)
{
    vx_reference parameters[] = {
        (vx_reference)srcImage,
        (vx_reference)cdfImage,
        (vx_reference)minValueScalar,
        (vx_reference)histImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_CDF, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxEqualizeHistLutNode(vx_graph graph, vx_image srcImage, vx_image histImage, vx_image dstImage)
{
    vx_reference parameters[] = {
        (vx_reference)srcImage,
        (vx_reference)histImage,
        (vx_reference)dstImage
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_LUT, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostNode(vx_graph graph, vx_image right_img, vx_image left_img, vx_scalar range, vx_image cost)
{
    vx_reference parameters[] = {
        (vx_reference)right_img,
        (vx_reference)left_img,
        (vx_reference)range,
        (vx_reference)cost
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_COST, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostPath90Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path)
{
    vx_reference parameters[] = {
        (vx_reference)cost,
        (vx_reference)range,
        (vx_reference)path
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_PATH90, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostPath45Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path)
{
    vx_reference parameters[] = {
        (vx_reference)cost,
        (vx_reference)range,
        (vx_reference)path
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_PATH45, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostPath135Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path)
{
    vx_reference parameters[] = {
        (vx_reference)cost,
        (vx_reference)range,
        (vx_reference)path
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_PATH135, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmCostPath0Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path)
{
    vx_reference parameters[] = {
        (vx_reference)cost,
        (vx_reference)range,
        (vx_reference)path
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_PATH0, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxSgmGetDispNode(vx_graph graph, vx_image path, vx_scalar range, vx_image depth)
{
    vx_reference parameters[] = {
        (vx_reference)path,
        (vx_reference)range,
        (vx_reference)depth
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM_DISP, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxLaplacian3x3Node(vx_graph graph, vx_image src, vx_image dst)
{
    vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_LAPLACIAN3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCensus3x3Node(vx_graph graph, vx_image src, vx_image dst)
{
    vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CENSUS3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCnnSoftMaxNode(vx_graph graph, vx_array src, vx_scalar batchSize, vx_scalar networkType, vx_scalar hasInterleave, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)batchSize,
        (vx_reference)networkType,
        (vx_reference)hasInterleave,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CNN_SOFTMAX, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCnnInterleaveBuffersNode(vx_graph graph, vx_array src, vx_scalar itemSize, vx_scalar batchSize, vx_scalar networkType, vx_scalar setEvent, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)itemSize,
        (vx_reference)batchSize,
        (vx_reference)networkType,
        (vx_reference)setEvent,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CNN_INTERLEAVE_BUFFERS, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCnnLayerNode(vx_graph graph, vx_array src, vx_scalar levelScalar,vx_array kernelBuffer, vx_array nnCmdBuffer, vx_scalar repeatScalar, vx_scalar batchSize, vx_scalar networkType, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)levelScalar,
        (vx_reference)kernelBuffer,
        (vx_reference)nnCmdBuffer,
        (vx_reference)repeatScalar,
        (vx_reference)batchSize,
        (vx_reference)networkType,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CNN_LAYER, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCnnDataConvertNode(vx_graph graph, vx_image src, vx_scalar batchSize, vx_scalar networkType, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)batchSize,
        (vx_reference)networkType,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CNN_DATACONVERT, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxCnnReshuffleImageNode(vx_graph graph, vx_image src, vx_array mean, vx_scalar levelScalar, vx_scalar padScalar, vx_scalar strideX, vx_scalar strideY, vx_scalar batchSize, vx_scalar networkType, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)mean,
        (vx_reference)levelScalar,
        (vx_reference)padScalar,
        (vx_reference)strideX,
        (vx_reference)strideY,
        (vx_reference)batchSize,
        (vx_reference)networkType,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CNN_RESHUFFLE_IMAGE, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxFasterRCNNSoftMaxNode(vx_graph graph, vx_array src, vx_array src_bbox, vx_scalar batchSize, vx_scalar networkType, vx_array percentArray, vx_array coordArray)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)src_bbox,
        (vx_reference)batchSize,
        (vx_reference)networkType,
        (vx_reference)percentArray,
        (vx_reference)coordArray
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_FASTERRCNN_SOFTMAX, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxFasterRCNNReshuffleDataNode(vx_graph graph, vx_array src, vx_scalar levelScalar, vx_scalar strideX, vx_scalar strideY, vx_scalar batchSize, vx_scalar networkType, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)levelScalar,
        (vx_reference)strideX,
        (vx_reference)strideY,
        (vx_reference)batchSize,
        (vx_reference)networkType,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_FASTERRCNN_RESHUFFLE_DATA, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxFasterRCNNConvertNode(vx_graph graph, vx_array src, vx_scalar src_format, vx_scalar dst_format, vx_scalar set_event, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)src_format,
        (vx_reference)dst_format,
        (vx_reference)set_event,
        (vx_reference)dst
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CONVERT, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxLRNNode(vx_graph graph, vx_array src, vx_scalar w, vx_scalar h, vx_scalar d, vx_scalar batch, vx_scalar type, vx_scalar kernel, vx_scalar stride, vx_scalar pad, vx_scalar shift, vx_scalar alpha, vx_scalar beta, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)w,
        (vx_reference)h,
        (vx_reference)d,
        (vx_reference)batch,
        (vx_reference)type,
        (vx_reference)kernel,
        (vx_reference)stride,
        (vx_reference)pad,
        (vx_reference)shift,
        (vx_reference)alpha,
        (vx_reference)beta,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_LRN, parameters, vxmLENGTH_OF(parameters));
}


VX_INTERNAL_API vx_node vxMaxPoolNode(vx_graph graph, vx_array src, vx_scalar format, vx_scalar w, vx_scalar h, vx_scalar d, vx_scalar batch, vx_scalar w2, vx_scalar h2, vx_scalar kernel, vx_scalar stride, vx_scalar pad, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)format,
        (vx_reference)w,
        (vx_reference)h,
        (vx_reference)d,
        (vx_reference)batch,
        (vx_reference)w2,
        (vx_reference)h2,
        (vx_reference)kernel,
        (vx_reference)stride,
        (vx_reference)pad,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_MAX_POOL3x3, parameters, vxmLENGTH_OF(parameters));
}


VX_INTERNAL_API vx_node vxROIPoolNode(vx_graph graph, vx_array input1, vx_array input2, vx_scalar kernel, vx_scalar stride, vx_scalar pad, vx_array dst)
{
     vx_reference parameters[] = {
        (vx_reference)input1,
        (vx_reference)input2,
        (vx_reference)kernel,
        (vx_reference)stride,
        (vx_reference)pad,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_ROI_POOLING, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxRCNNWaitNode(vx_graph graph, vx_graph graph_id, vx_scalar event_id)
{
     vx_reference parameters[] = {
        (vx_reference)graph_id,
        (vx_reference)event_id
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_FASTERRCNN_WAIT, parameters, vxmLENGTH_OF(parameters));
}

VX_INTERNAL_API vx_node vxRPNNode(vx_graph graph, vx_array input, vx_array dst0, vx_array dst1)
{
     vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)dst0,
        (vx_reference)dst1,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_RPN, parameters, vxmLENGTH_OF(parameters));
}

