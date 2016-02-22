/****************************************************************************
*
*    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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
        vx_graph graph, vx_image gx, vx_image gy, vx_scalar sensitivity, vx_scalar blockSize, vx_image score)
{
    vx_reference parameters[] = {
        (vx_reference)gx,
        (vx_reference)gy,
        (vx_reference)sensitivity,
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