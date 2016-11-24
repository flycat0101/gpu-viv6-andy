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

VX_API_ENTRY vx_node VX_API_CALL vxColorConvertNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_COLOR_CONVERT, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxChannelExtractNode(vx_graph graph, vx_image input, vx_enum channelNum, vx_image output)
{
    vx_scalar       scalar;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)input,
        VX_NULL,
        (vx_reference)output
    };

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &channelNum);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS) return (vx_node)scalar;

    parameters[1] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_CHANNEL_EXTRACT, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxChannelCombineNode(
        vx_graph graph, vx_image plane0, vx_image plane1, vx_image plane2, vx_image plane3, vx_image output)
{
    vx_reference parameters[] = {
       (vx_reference)plane0,
       (vx_reference)plane1,
       (vx_reference)plane2,
       (vx_reference)plane3,
       (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_CHANNEL_COMBINE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxSobel3x3Node(vx_graph graph, vx_image input, vx_image output_x, vx_image output_y)
{
    vx_reference parameters[] = {
       (vx_reference)input,
       (vx_reference)output_x,
       (vx_reference)output_y
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_SOBEL_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxMagnitudeNode(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image mag)
{
    vx_reference parameters[] = {
       (vx_reference)grad_x,
       (vx_reference)grad_y,
       (vx_reference)mag
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_MAGNITUDE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxPhaseNode(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image orientation)
{
    vx_reference parameters[] = {
       (vx_reference)grad_x,
       (vx_reference)grad_y,
       (vx_reference)orientation
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_PHASE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxScaleImageNode(vx_graph graph, vx_image src, vx_image dst, vx_enum type)
{
    vx_scalar       scalar;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)src,
        (vx_reference)dst,
        VX_NULL
    };

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &type);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS) return (vx_node)scalar;

    parameters[2] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_SCALE_IMAGE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTableLookupNode(vx_graph graph, vx_image input, vx_lut lut, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)lut,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_TABLE_LOOKUP, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxHistogramNode(vx_graph graph, vx_image input, vx_distribution distribution)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)distribution
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_HISTOGRAM, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxEqualizeHistNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_EQUALIZE_HISTOGRAM, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxAbsDiffNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)out
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_ABSDIFF, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxMeanStdDevNode(vx_graph graph, vx_image input, vx_scalar mean, vx_scalar stddev)
{
    vx_reference parameters[] = {
       (vx_reference)input,
       (vx_reference)mean,
       (vx_reference)stddev
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_MEAN_STDDEV, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxThresholdNode(vx_graph graph, vx_image input, vx_threshold thesh, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)thesh,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_THRESHOLD, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxIntegralImageNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTEGRAL_IMAGE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxErode3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_ERODE_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxDilate3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_DILATE_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxMedian3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_MEDIAN_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxBox3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_BOX_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxGaussian3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_GAUSSIAN_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxConvolveNode(vx_graph graph, vx_image input, vx_convolution conv, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)conv,
        (vx_reference)output
    };

    vxReadConvolutionCoefficients(conv, NULL);

    return vxoNode_CreateSpecific(graph, VX_KERNEL_CUSTOM_CONVOLUTION, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxGaussianPyramidNode(vx_graph graph, vx_image input, vx_pyramid gaussian)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)gaussian
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_GAUSSIAN_PYRAMID, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateImageNode(vx_graph graph, vx_image input, vx_image accum)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)accum
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_ACCUMULATE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateWeightedImageNode(vx_graph graph, vx_image input, vx_scalar alpha, vx_image accum)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)alpha,
        (vx_reference)accum
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_ACCUMULATE_WEIGHTED, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateSquareImageNode(vx_graph graph, vx_image input, vx_scalar scalar, vx_image accum)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)scalar,
        (vx_reference)accum
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_ACCUMULATE_SQUARE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxMinMaxLocNode(vx_graph graph,
                        vx_image input,
                        vx_scalar minVal, vx_scalar maxVal,
                        vx_array minLoc, vx_array maxLoc,
                        vx_scalar minCount, vx_scalar maxCount)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)minVal,
        (vx_reference)maxVal,
        (vx_reference)minLoc,
        (vx_reference)maxLoc,
        (vx_reference)minCount,
        (vx_reference)maxCount
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_MINMAXLOC, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxConvertDepthNode(
        vx_graph graph, vx_image input, vx_image output, vx_enum policy, vx_scalar shift)
{
    vx_scalar       scalar;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)input,
        (vx_reference)output,
        VX_NULL,
        (vx_reference)shift
    };

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &policy);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS) return (vx_node)scalar;

    parameters[2] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_CONVERTDEPTH, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxCannyEdgeDetectorNode(
        vx_graph graph, vx_image input, vx_threshold hyst, vx_int32 gradient_size,
        vx_enum norm_type, vx_image output)
{
    vx_scalar       scalarGradientSize, scalarNormType;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)input,
        (vx_reference)hyst,
        VX_NULL,
        VX_NULL,
        (vx_reference)output
    };

    scalarGradientSize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &gradient_size);

    if (vxoReference_GetStatus((vx_reference)scalarGradientSize) != VX_SUCCESS) return (vx_node)scalarGradientSize;

    parameters[2] = (vx_reference)scalarGradientSize;

    scalarNormType = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &norm_type);

    if (vxoReference_GetStatus((vx_reference)scalarNormType) != VX_SUCCESS) return (vx_node)scalarNormType;

    parameters[3] = (vx_reference)scalarNormType;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_CANNY_EDGE_DETECTOR, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarGradientSize);

    vxReleaseScalar(&scalarNormType);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxAndNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)out
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_AND, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxOrNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)out
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_OR, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxXorNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)out
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_XOR, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxNotNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
       (vx_reference)input,
       (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_NOT, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxMultiplyNode(vx_graph graph, vx_image in1, vx_image in2, vx_scalar scale, vx_enum overflow_policy, vx_enum rounding_policy, vx_image out)
{
    vx_scalar       scalarOverflowPolicy, scalarRoundingPolicy;
    vx_node         node;
    vx_reference    parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)scale,
       VX_NULL,
       VX_NULL,
       (vx_reference)out
    };

    scalarOverflowPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &overflow_policy);

    if (vxoReference_GetStatus((vx_reference)scalarOverflowPolicy) != VX_SUCCESS) return (vx_node)scalarOverflowPolicy;

    parameters[3] = (vx_reference)scalarOverflowPolicy;

    scalarRoundingPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &rounding_policy);

    if (vxoReference_GetStatus((vx_reference)scalarRoundingPolicy) != VX_SUCCESS) return (vx_node)scalarRoundingPolicy;

    parameters[4] = (vx_reference)scalarRoundingPolicy;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_MULTIPLY, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarOverflowPolicy);

    vxReleaseScalar(&scalarRoundingPolicy);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxAddNode(vx_graph graph, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    vx_scalar       scalarPolicy;
    vx_node         node;
    vx_reference    parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       VX_NULL,
       (vx_reference)out
    };

    scalarPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &policy);

    if (vxoReference_GetStatus((vx_reference)scalarPolicy) != VX_SUCCESS) return (vx_node)scalarPolicy;

    parameters[2] = (vx_reference)scalarPolicy;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_ADD, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarPolicy);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxSubtractNode(vx_graph graph, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    vx_scalar       scalarPolicy;
    vx_node         node;
    vx_reference    parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       VX_NULL,
       (vx_reference)out
    };

    scalarPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &policy);

    if (vxoReference_GetStatus((vx_reference)scalarPolicy) != VX_SUCCESS) return (vx_node)scalarPolicy;

    parameters[2] = (vx_reference)scalarPolicy;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_SUBTRACT, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarPolicy);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxWarpAffineNode(
        vx_graph graph, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_scalar       scalar;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)input,
        (vx_reference)matrix,
        VX_NULL,
        (vx_reference)output
    };

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &type);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS) return (vx_node)scalar;

    parameters[2] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_WARP_AFFINE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxWarpPerspectiveNode(
        vx_graph graph, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_scalar       scalar;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)input,
        (vx_reference)matrix,
        VX_NULL,
        (vx_reference)output
    };

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &type);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS) return (vx_node)scalar;

    parameters[2] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_WARP_PERSPECTIVE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxHarrisCornersNode(
        vx_graph graph, vx_image input, vx_scalar strength_thresh, vx_scalar min_distance, vx_scalar sensitivity,
        vx_int32 gradient_size, vx_int32 block_size, vx_array corners, vx_scalar num_corners)
{
    vx_scalar       scalarGradientSize, scalarBlockSize;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)input,
        (vx_reference)strength_thresh,
        (vx_reference)min_distance,
        (vx_reference)sensitivity,
        VX_NULL,
        VX_NULL,
        (vx_reference)corners,
        (vx_reference)num_corners
    };

    scalarGradientSize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &gradient_size);

    if (vxoReference_GetStatus((vx_reference)scalarGradientSize) != VX_SUCCESS) return (vx_node)scalarGradientSize;

    parameters[4] = (vx_reference)scalarGradientSize;

    scalarBlockSize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &block_size);

    if (vxoReference_GetStatus((vx_reference)scalarBlockSize) != VX_SUCCESS) return (vx_node)scalarBlockSize;

    parameters[5] = (vx_reference)scalarBlockSize;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_HARRIS_CORNERS, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarGradientSize);

    vxReleaseScalar(&scalarBlockSize);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxFastCornersNode(
        vx_graph graph, vx_image input, vx_scalar strength_thresh,
        vx_bool nonmax_suppression, vx_array corners, vx_scalar num_corners)
{
    vx_scalar       scalarNonmaxSuppression;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)input,
        (vx_reference)strength_thresh,
        VX_NULL,
        (vx_reference)corners,
        (vx_reference)num_corners
    };

    scalarNonmaxSuppression = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_BOOL, &nonmax_suppression);

    if (vxoReference_GetStatus((vx_reference)scalarNonmaxSuppression) != VX_SUCCESS) return (vx_node)scalarNonmaxSuppression;

    parameters[2] = (vx_reference)scalarNonmaxSuppression;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_FAST_CORNERS, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarNonmaxSuppression);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxOpticalFlowPyrLKNode(
        vx_graph graph, vx_pyramid old_images, vx_pyramid new_images, vx_array old_points,
        vx_array new_points_estimates, vx_array new_points, vx_enum termination,
        vx_scalar epsilon, vx_scalar num_iterations, vx_scalar use_initial_estimate, vx_size window_dimension)
{
    vx_scalar       scalarTermination, scalarWindowDimension;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)old_images,
        (vx_reference)new_images,
        (vx_reference)old_points,
        (vx_reference)new_points_estimates,
        (vx_reference)new_points,
        VX_NULL,
        (vx_reference)epsilon,
        (vx_reference)num_iterations,
        (vx_reference)use_initial_estimate,
        VX_NULL
    };

    scalarTermination = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &termination);

    if (vxoReference_GetStatus((vx_reference)scalarTermination) != VX_SUCCESS) return (vx_node)scalarTermination;

    parameters[5] = (vx_reference)scalarTermination;

    scalarWindowDimension = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &window_dimension);

    if (vxoReference_GetStatus((vx_reference)scalarWindowDimension) != VX_SUCCESS) return (vx_node)scalarWindowDimension;

    parameters[9] = (vx_reference)scalarWindowDimension;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_OPTICAL_FLOW_PYR_LK, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarTermination);

    vxReleaseScalar(&scalarWindowDimension);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxRemapNode(vx_graph graph, vx_image input, vx_remap table, vx_enum policy, vx_image output)
{
    vx_scalar       scalarPolicy;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)input,
        (vx_reference)table,
        VX_NULL,
        (vx_reference)output
    };

    scalarPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &policy);

    if (vxoReference_GetStatus((vx_reference)scalarPolicy) != VX_SUCCESS) return (vx_node)scalarPolicy;

    parameters[2] = (vx_reference)scalarPolicy;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_REMAP, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarPolicy);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxHalfScaleGaussianNode(vx_graph graph, vx_image input, vx_image output, vx_int32 kernel_size)
{
    vx_scalar       scalarKernelSize;
    vx_node         node;
    vx_reference    parameters[] = {
        (vx_reference)input,
        (vx_reference)output,
        VX_NULL
    };

    scalarKernelSize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &kernel_size);

    if (vxoReference_GetStatus((vx_reference)scalarKernelSize) != VX_SUCCESS) return (vx_node)scalarKernelSize;

    parameters[2] = (vx_reference)scalarKernelSize;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_HALFSCALE_GAUSSIAN, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarKernelSize);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxSgmNode(vx_graph graph, vx_image right_img, vx_image left_img, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)right_img,
        (vx_reference)left_img,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM, parameters, vxmLENGTH_OF(parameters));
}

