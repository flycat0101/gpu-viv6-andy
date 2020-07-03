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


#ifndef __GC_VX_INTERNAL_NODE_API_H__
#define __GC_VX_INTERNAL_NODE_API_H__

EXTERN_C_BEGIN

#include <gc_vx_common.h>
#include <VX/vx_lib_debug.h>

VX_INTERNAL_API vx_node vxSobelMxNNode(vx_graph graph, vx_image input, vx_scalar ws, vx_image gx, vx_image gy);

VX_INTERNAL_API vx_node vxSobelMxNF16Node(vx_graph graph, vx_image input, vx_scalar ws, vx_scalar shift, vx_image gx, vx_image gy);

VX_INTERNAL_API vx_node vxHarrisScoreNode(
        vx_graph graph, vx_image gx, vx_image gy, vx_scalar sensitivity, vx_scalar winSize, vx_scalar blockSize, vx_scalar shiftSize, vx_image score);

VX_INTERNAL_API vx_node vxEuclideanNonMaxNode(
        vx_graph graph, vx_image input, vx_scalar strengthThresh, vx_scalar minDistance, vx_image output);

VX_INTERNAL_API vx_node vxImageListerNode(vx_graph graph, vx_image input, vx_array arr, vx_scalar numPoints);

VX_INTERNAL_API vx_node vxElementwiseNormNode(
        vx_graph graph, vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output);

VX_INTERNAL_API vx_node vxElementwiseNormF16Node(
        vx_graph graph, vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output);

VX_INTERNAL_API vx_node vxPhaseF16Node(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image orientation);

VX_INTERNAL_API vx_node vxNonMaxSuppressionCannyNode(vx_graph graph, vx_image mag, vx_image phase, vx_image edge);

VX_INTERNAL_API vx_node vxEdgeTraceNode(vx_graph graph, vx_image norm, vx_threshold threshold, vx_image output);

VX_INTERNAL_API vx_node vxEdgeTraceThresholdNode(vx_graph graph, vx_image inputImage, vx_threshold threshold, vx_image outputImage);

VX_INTERNAL_API vx_node vxEdgeTraceHysteresisNode(vx_graph graph, vx_image image, vx_scalar flag);

VX_INTERNAL_API vx_node vxEdgeTraceClampNode(vx_graph graph, vx_image inputImage, vx_image outputImage);

VX_INTERNAL_API vx_node vxFast9CornersStrengthNode(vx_graph graph, vx_image input, vx_scalar t, vx_scalar do_nonmax, vx_image output);

VX_INTERNAL_API vx_node vxFast9CornersNonMaxNode(vx_graph graph, vx_image input, vx_scalar t, vx_scalar do_nonmax, vx_image output);

VX_INTERNAL_API vx_node vxCreateListerNode(vx_graph graph, vx_image input, vx_image countImg, vx_array array);

VX_INTERNAL_API vx_node vxPackArraysNode(vx_graph graph, vx_image inputImage, vx_array inputArray, vx_scalar widthScalar, vx_scalar heightScalar, vx_array dstArray, vx_scalar numScalar);

VX_INTERNAL_API vx_node vxMinMaxLocFilterNode(vx_graph graph, vx_image inputImage, vx_scalar filterMin, vx_scalar filterMax);

VX_INTERNAL_API vx_node vxGetLocationNode(vx_graph graph, vx_image inputImage, vx_scalar filterMinValue, vx_scalar filterMaxValue,
        vx_image minImage, vx_image maxImage, vx_array minArray, vx_array maxArray, vx_scalar filterMinCount, vx_scalar filterMaxCount);

VX_INTERNAL_API vx_node vxMinMaxLocPackArrayNode(vx_graph graph, vx_image inputImage, vx_array inputArray, vx_scalar widthScalar, vx_scalar heightScalar, vx_scalar countScalar, vx_array dstArray);

VX_INTERNAL_API vx_node vxIntegralImageStepNode(vx_graph graph, vx_image inputImage, vx_scalar stepScalar, vx_image outputImage);

VX_INTERNAL_API vx_node vxScharr3x3Node(vx_graph graph, vx_image inputImage, vx_image gradXImage, vx_image gradYImage);

VX_INTERNAL_API vx_node vxVLKTrackerNode(vx_graph graph, vx_pyramid oldPyramid, vx_pyramid newPyramid, vx_pyramid gradXPyramid, vx_pyramid gradYPyramid,
                                         vx_array prevPts, vx_array estimatedPts, vx_array nextPts,
                                         vx_scalar criteriaScalar, vx_scalar epsilonScalar, vx_scalar numIterationsScalar, vx_scalar isUseInitialEstimateScalar,
                                         vx_scalar winSizeScalar);

VX_INTERNAL_API vx_node vxEqualizeHistHistNode(vx_graph graph, vx_image srcImage, vx_image histImage, vx_scalar minIndexScalar);
VX_INTERNAL_API vx_node vxEqualizeHistGcdfNode(vx_graph graph, vx_image histImage, vx_scalar minIndexScalar, vx_image cdfImage, vx_scalar minValueScalar);
VX_INTERNAL_API vx_node vxEqualizeHistCdfNode(vx_graph graph, vx_image srcImage, vx_image cdfImage, vx_scalar minValueScalar, vx_image histImage);
VX_INTERNAL_API vx_node vxEqualizeHistLutNode(vx_graph graph, vx_image srcImage, vx_image histImage, vx_image dstImage);

VX_INTERNAL_API vx_node vxSgmCostNode(vx_graph graph, vx_image right_img, vx_image left_img, vx_scalar range, vx_image cost);
VX_INTERNAL_API vx_node vxSgmCostPath90Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path);
VX_INTERNAL_API vx_node vxSgmCostPath45Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path);
VX_INTERNAL_API vx_node vxSgmCostPath135Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path);
VX_INTERNAL_API vx_node vxSgmCostPath0Node(vx_graph graph, vx_image cost, vx_scalar range, vx_image path);
VX_INTERNAL_API vx_node vxSgmGetDispNode(vx_graph graph, vx_image path, vx_scalar range, vx_image depth);
VX_INTERNAL_API vx_node vxLaplacian3x3Node(vx_graph graph, vx_image src, vx_image dst);

VX_INTERNAL_API vx_node vxUpSamplePaddingNode(vx_graph graph, vx_image inputImage, vx_image outputImage);
VX_INTERNAL_API vx_node vxUpSampleConvertNode(vx_graph graph, vx_image inputImage, vx_image outputImage);
VX_INTERNAL_API vx_node vxPyramidCopyImageNode(vx_graph graph, vx_image inputImage, vx_image outputImage);

VX_INTERNAL_API vx_node vxTransPose2DTensorNode(vx_graph graph, vx_tensor inputTensor, vx_tensor outputTensor);
VX_INTERNAL_API vx_node vxMultiply2DMatrixesNode(vx_graph graph, vx_tensor inputTensor0, vx_tensor inputTensor1, vx_tensor inputTensor2, vx_scalar enable_tensorC, vx_tensor outputTensor);

EXTERN_C_END

#endif /* __GC_VX_INTERNAL_NODE_API_H__ */

