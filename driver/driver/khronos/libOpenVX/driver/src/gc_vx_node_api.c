/****************************************************************************
*
*    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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

VX_API_ENTRY vx_node VX_API_CALL vxNonLinearFilterNode(vx_graph graph, vx_enum function, vx_image input, vx_matrix mask, vx_image output)
{
    vx_scalar func = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &function);
    vx_reference parameters[] = {
        (vx_reference)func,
        (vx_reference)input,
        (vx_reference)mask,
        (vx_reference)output,
    };
    vx_node node = vxoNode_CreateSpecific(graph,
        VX_KERNEL_NON_LINEAR_FILTER,
        parameters,
        vxmLENGTH_OF(parameters));

    vxReleaseScalar(&func);
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConvolveNode(vx_graph graph, vx_image input, vx_convolution conv, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)conv,
        (vx_reference)output
    };


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

VX_API_ENTRY vx_node VX_API_CALL vxLaplacianPyramidNode(vx_graph graph, vx_image input, vx_pyramid laplacian, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)laplacian,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_LAPLACIAN_PYRAMID, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxLaplacianReconstructNode(vx_graph graph, vx_pyramid laplacian, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)laplacian,
        (vx_reference)input,
        (vx_reference)output
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_LAPLACIAN_RECONSTRUCT, parameters, vxmLENGTH_OF(parameters));
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

VX_API_ENTRY vx_node VX_API_CALL vxCensus3x3Node(vx_graph graph, vx_image src, vx_image dst)
{
    vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)dst,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CENSUS3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxCNNNode(vx_graph graph, vx_image inputImage, vx_array percentArray)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)percentArray,
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_CNN, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxFasterRCNNode(vx_graph graph, vx_image inputImage, vx_array percentArray, vx_array coordArray)
{
    vx_reference parameters[] = {
        (vx_reference)inputImage,
        (vx_reference)percentArray,
        (vx_reference)coordArray
    };

    return vxoNode_CreateSpecific(graph, VX_KERNEL_FASTER_RCNN, parameters, vxmLENGTH_OF(parameters));
}


VX_API_ENTRY vx_node VX_API_CALL vxFullyConnectedReluLayer(
    vx_graph  graph,
    vx_tensor inputs,
    vx_weights_biases_parameter weights_biases,
    vx_uint32 pad,
    vx_uint8  accumulator_bits,
    vx_enum   overflow_policy,
    vx_enum   rounding_policy,
    vx_enum   down_scale_size_rounding,
    vx_bool   enable_relu,
    vx_tensor outputs
    )
{
    vx_context context;
    vx_node    node;

    vx_scalar pad_s                      = VX_NULL;
    vx_scalar accumulator_bits_s         = VX_NULL;
    vx_scalar overflow_policy_s          = VX_NULL;
    vx_scalar rounding_policy_s          = VX_NULL;
    vx_scalar down_scale_size_rounding_s = VX_NULL;
    vx_scalar enable_relu_s              = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    (vx_reference)weights_biases,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    context = vxGetContext((vx_reference)graph);

    pad_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad);
    if (vxoReference_GetStatus((vx_reference)pad_s) != VX_SUCCESS) return (vx_node)pad_s;

    accumulator_bits_s = vxCreateScalar(context, VX_TYPE_UINT32, &accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)accumulator_bits_s) != VX_SUCCESS) return (vx_node)accumulator_bits_s;

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS) return (vx_node)overflow_policy_s;

    rounding_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_policy_s) != VX_SUCCESS) return (vx_node)rounding_policy_s;

    down_scale_size_rounding_s = vxCreateScalar(context, VX_TYPE_UINT32, &down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s) != VX_SUCCESS) return (vx_node)down_scale_size_rounding_s;

    enable_relu_s = vxCreateScalar(context, VX_TYPE_UINT32, &enable_relu);
    if (vxoReference_GetStatus((vx_reference)enable_relu_s) != VX_SUCCESS) return (vx_node)enable_relu_s;

    parameters[2]  = (vx_reference)pad_s;
    parameters[3]  = (vx_reference)accumulator_bits_s;
    parameters[4]  = (vx_reference)overflow_policy_s;
    parameters[5]  = (vx_reference)rounding_policy_s;
    parameters[6]  = (vx_reference)down_scale_size_rounding_s;
    parameters[7]  = (vx_reference)enable_relu_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_FULLY_CONNECTED_RELU_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&pad_s);
    vxReleaseScalar(&accumulator_bits_s);
    vxReleaseScalar(&overflow_policy_s);
    vxReleaseScalar(&rounding_policy_s);
    vxReleaseScalar(&down_scale_size_rounding_s);
    vxReleaseScalar(&enable_relu_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxFullyConnectedLayer(
    vx_graph graph,
    vx_tensor inputs,
    vx_tensor weights,
    vx_tensor biases,
    vx_uint32 pad,
    vx_uint8 accumulator_bits,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_enum down_scale_size_rounding,
    vx_tensor outputs)
{
    vx_context context;
    vx_node    node;

    vx_scalar pad_s                      = VX_NULL;
    vx_scalar accumulator_bits_s         = VX_NULL;
    vx_scalar overflow_policy_s          = VX_NULL;
    vx_scalar rounding_policy_s          = VX_NULL;
    vx_scalar down_scale_size_rounding_s = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    (vx_reference)weights,
    (vx_reference)biases,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    context = vxGetContext((vx_reference)graph);

    pad_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad);
    if (vxoReference_GetStatus((vx_reference)pad_s) != VX_SUCCESS) return (vx_node)pad_s;

    accumulator_bits_s = vxCreateScalar(context, VX_TYPE_UINT32, &accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)accumulator_bits_s) != VX_SUCCESS) return (vx_node)accumulator_bits_s;

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS) return (vx_node)overflow_policy_s;

    rounding_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_policy_s) != VX_SUCCESS) return (vx_node)rounding_policy_s;

    down_scale_size_rounding_s = vxCreateScalar(context, VX_TYPE_UINT32, &down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s) != VX_SUCCESS) return (vx_node)down_scale_size_rounding_s;

    parameters[3]  = (vx_reference)pad_s;
    parameters[4]  = (vx_reference)accumulator_bits_s;
    parameters[5]  = (vx_reference)overflow_policy_s;
    parameters[6]  = (vx_reference)rounding_policy_s;
    parameters[7]  = (vx_reference)down_scale_size_rounding_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_FULLY_CONNECTED_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&pad_s);
    vxReleaseScalar(&accumulator_bits_s);
    vxReleaseScalar(&overflow_policy_s);
    vxReleaseScalar(&rounding_policy_s);
    vxReleaseScalar(&down_scale_size_rounding_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConvolutionReluPoolingLayer(
    vx_graph                    graph,
    vx_tensor                   inputs,
    vx_weights_biases_parameter weights_biases,
    vx_uint32                   pad_x,
    vx_uint32                   pad_y,
    vx_uint8                    accumulator_bits,
    vx_enum                     overflow_policy,
    vx_enum                     rounding_policy,
    vx_enum                     down_scale_size_rounding,
    vx_bool                     enable_relu,
    vx_enum                     pool_type,
    vx_uint32                   pool_size_x,
    vx_uint32                   pool_size_y,
    vx_tensor                   outputs
    )
{
    vx_context context;
    vx_node    node;

    vx_scalar pad_x_s                    = VX_NULL;
    vx_scalar pad_y_s                    = VX_NULL;
    vx_scalar accumulator_bits_s         = VX_NULL;
    vx_scalar overflow_policy_s          = VX_NULL;
    vx_scalar rounding_policy_s          = VX_NULL;
    vx_scalar down_scale_size_rounding_s = VX_NULL;
    vx_scalar enable_relu_s              = VX_NULL;
    vx_scalar pool_type_s                = VX_NULL;
    vx_scalar pool_size_x_s              = VX_NULL;
    vx_scalar pool_size_y_s              = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    (vx_reference)weights_biases,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    context = vxGetContext((vx_reference)graph);

    pad_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_x);
    if (vxoReference_GetStatus((vx_reference)pad_x_s) != VX_SUCCESS) return (vx_node)pad_x_s;

    pad_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_y);
    if (vxoReference_GetStatus((vx_reference)pad_y_s) != VX_SUCCESS) return (vx_node)pad_y_s;

    accumulator_bits_s = vxCreateScalar(context, VX_TYPE_UINT32, &accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)accumulator_bits_s) != VX_SUCCESS) return (vx_node)accumulator_bits_s;

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS) return (vx_node)overflow_policy_s;

    rounding_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_policy_s) != VX_SUCCESS) return (vx_node)rounding_policy_s;

    down_scale_size_rounding_s = vxCreateScalar(context, VX_TYPE_UINT32, &down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s) != VX_SUCCESS) return (vx_node)down_scale_size_rounding_s;

    enable_relu_s = vxCreateScalar(context, VX_TYPE_UINT32, &enable_relu);
    if (vxoReference_GetStatus((vx_reference)enable_relu_s) != VX_SUCCESS) return (vx_node)enable_relu_s;

    pool_type_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_type);
    if (vxoReference_GetStatus((vx_reference)pool_type_s) != VX_SUCCESS) return (vx_node)pool_type_s;

    pool_size_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_x);
    if (vxoReference_GetStatus((vx_reference)pool_size_x_s) != VX_SUCCESS) return (vx_node)pool_size_x_s;

    pool_size_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_y);
    if (vxoReference_GetStatus((vx_reference)pool_size_y_s) != VX_SUCCESS) return (vx_node)pool_size_y_s;


    parameters[2]  = (vx_reference)pad_x_s;
    parameters[3]  = (vx_reference)pad_y_s;
    parameters[4]  = (vx_reference)accumulator_bits_s;
    parameters[5]  = (vx_reference)overflow_policy_s;
    parameters[6]  = (vx_reference)rounding_policy_s;
    parameters[7]  = (vx_reference)down_scale_size_rounding_s;
    parameters[8]  = (vx_reference)enable_relu_s;
    parameters[9]  = (vx_reference)pool_type_s;
    parameters[10] = (vx_reference)pool_size_x_s;
    parameters[11] = (vx_reference)pool_size_y_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&pad_x_s);
    vxReleaseScalar(&pad_y_s);
    vxReleaseScalar(&accumulator_bits_s);
    vxReleaseScalar(&overflow_policy_s);
    vxReleaseScalar(&rounding_policy_s);
    vxReleaseScalar(&down_scale_size_rounding_s);
    vxReleaseScalar(&enable_relu_s);
    vxReleaseScalar(&pool_type_s);
    vxReleaseScalar(&pool_size_x_s);
    vxReleaseScalar(&pool_size_y_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConvolutionReluLayer(
    vx_graph                    graph,
    vx_tensor                   inputs,
    vx_weights_biases_parameter weights_biases,
    vx_uint32                   pad_x,
    vx_uint32                   pad_y,
    vx_uint8                    accumulator_bits,
    vx_enum                     overflow_policy,
    vx_enum                     rounding_policy,
    vx_enum                     down_scale_size_rounding,
    vx_bool                     enable_relu,
    vx_tensor                   outputs
    )
{
    vx_context context;
    vx_node    node;

    vx_scalar pad_x_s                    = VX_NULL;
    vx_scalar pad_y_s                    = VX_NULL;
    vx_scalar accumulator_bits_s         = VX_NULL;
    vx_scalar overflow_policy_s          = VX_NULL;
    vx_scalar rounding_policy_s          = VX_NULL;
    vx_scalar down_scale_size_rounding_s = VX_NULL;
    vx_scalar enable_relu_s              = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    (vx_reference)weights_biases,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    context = vxGetContext((vx_reference)graph);

    pad_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_x);
    if (vxoReference_GetStatus((vx_reference)pad_x_s) != VX_SUCCESS) return (vx_node)pad_x_s;

    pad_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_y);
    if (vxoReference_GetStatus((vx_reference)pad_y_s) != VX_SUCCESS) return (vx_node)pad_y_s;

    accumulator_bits_s = vxCreateScalar(context, VX_TYPE_UINT32, &accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)accumulator_bits_s) != VX_SUCCESS) return (vx_node)accumulator_bits_s;

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS) return (vx_node)overflow_policy_s;

    rounding_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_policy_s) != VX_SUCCESS) return (vx_node)rounding_policy_s;

    down_scale_size_rounding_s = vxCreateScalar(context, VX_TYPE_UINT32, &down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s) != VX_SUCCESS) return (vx_node)down_scale_size_rounding_s;

    enable_relu_s = vxCreateScalar(context, VX_TYPE_UINT32, &enable_relu);
    if (vxoReference_GetStatus((vx_reference)enable_relu_s) != VX_SUCCESS) return (vx_node)enable_relu_s;

    parameters[2]  = (vx_reference)pad_x_s;
    parameters[3]  = (vx_reference)pad_y_s;
    parameters[4]  = (vx_reference)accumulator_bits_s;
    parameters[5]  = (vx_reference)overflow_policy_s;
    parameters[6]  = (vx_reference)rounding_policy_s;
    parameters[7]  = (vx_reference)down_scale_size_rounding_s;
    parameters[8]  = (vx_reference)enable_relu_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONVOLUTION_RELU_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&pad_x_s);
    vxReleaseScalar(&pad_y_s);
    vxReleaseScalar(&accumulator_bits_s);
    vxReleaseScalar(&overflow_policy_s);
    vxReleaseScalar(&rounding_policy_s);
    vxReleaseScalar(&down_scale_size_rounding_s);
    vxReleaseScalar(&enable_relu_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConvolutionReluPoolingLayer2(
    vx_graph                    graph,
    vx_tensor                   inputs,
    vx_weights_biases_parameter weights_biases,
    const vx_nn_convolution_relu_pooling_params_t * convolution_relu_pooling_params,
    vx_size                     size_of_convolution_relu_pooling_params,
    vx_tensor                   outputs
    )
{
    vx_context context;
    vx_node    node;

    vx_reference    parameters[] = {
        (vx_reference)inputs,
        (vx_reference)weights_biases,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        (vx_reference)outputs
    };
    vx_uint32 i = 0;

    if (size_of_convolution_relu_pooling_params != sizeof(vx_nn_convolution_relu_pooling_params_t))
    {
        return NULL;
    }

    context = vxGetContext((vx_reference)graph);

    parameters[2] = (vx_reference)vxCreateScalar(context, VX_TYPE_SIZE, &convolution_relu_pooling_params->dilation_x);
    if (vxoReference_GetStatus((vx_reference)parameters[2]) != VX_SUCCESS) return (vx_node)parameters[2];

    parameters[3] = (vx_reference)vxCreateScalar(context, VX_TYPE_SIZE, &convolution_relu_pooling_params->dilation_y);
    if (vxoReference_GetStatus((vx_reference)parameters[3]) != VX_SUCCESS) return (vx_node)parameters[3];

    parameters[4] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pad_x_left);
    if (vxoReference_GetStatus((vx_reference)parameters[4]) != VX_SUCCESS) return (vx_node)parameters[4];

    parameters[5] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pad_x_right);
    if (vxoReference_GetStatus((vx_reference)parameters[5]) != VX_SUCCESS) return (vx_node)parameters[5];

    parameters[6] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pad_y_top);
    if (vxoReference_GetStatus((vx_reference)parameters[6]) != VX_SUCCESS) return (vx_node)parameters[6];

    parameters[7] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pad_y_bottom);
    if (vxoReference_GetStatus((vx_reference)parameters[7]) != VX_SUCCESS) return (vx_node)parameters[7];

    parameters[8] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT8, &convolution_relu_pooling_params->accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)parameters[8]) != VX_SUCCESS) return (vx_node)parameters[8];

    parameters[9] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->overflow_policy);
    if (vxoReference_GetStatus((vx_reference)parameters[9]) != VX_SUCCESS) return (vx_node)parameters[9];

    parameters[10] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->rounding_policy);
    if (vxoReference_GetStatus((vx_reference)parameters[10]) != VX_SUCCESS) return (vx_node)parameters[10];

    parameters[11] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)parameters[11]) != VX_SUCCESS) return (vx_node)parameters[11];

    parameters[12] = (vx_reference)vxCreateScalar(context, VX_TYPE_BOOL, &convolution_relu_pooling_params->down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)parameters[12]) != VX_SUCCESS) return (vx_node)parameters[12];

    parameters[13] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->pool_type);
    if (vxoReference_GetStatus((vx_reference)parameters[13]) != VX_SUCCESS) return (vx_node)parameters[13];

    parameters[14] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pool_size_x);
    if (vxoReference_GetStatus((vx_reference)parameters[14]) != VX_SUCCESS) return (vx_node)parameters[14];

    parameters[15] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pool_size_y);
    if (vxoReference_GetStatus((vx_reference)parameters[15]) != VX_SUCCESS) return (vx_node)parameters[15];

    parameters[16] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->pad_mode);
    if (vxoReference_GetStatus((vx_reference)parameters[16]) != VX_SUCCESS) return (vx_node)parameters[16];

    parameters[17] = (vx_reference)convolution_relu_pooling_params->pad_const;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2, parameters, vxmLENGTH_OF(parameters));

    for (i = 2; i < (gcmCOUNTOF(parameters) - 2); i ++)
    {
        vxReleaseScalar((vx_scalar*)&parameters[i]);
    }

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxPoolingLayer(vx_graph graph,
                                                vx_tensor inputs,
                                                vx_enum pool_type,
                                                vx_uint32 pool_size_x,
                                                vx_uint32 pool_size_y,
                                                vx_uint32 pool_pad_x,
                                                vx_uint32 pool_pad_y,
                                                vx_enum rounding,
                                                vx_tensor outputs)
{
    vx_context context;
    vx_node    node;

    vx_scalar pool_type_s    = VX_NULL;
    vx_scalar pool_size_x_s  = VX_NULL;
    vx_scalar pool_size_y_s  = VX_NULL;
    vx_scalar pool_pad_x_s   = VX_NULL;
    vx_scalar pool_pad_y_s   = VX_NULL;
    vx_scalar rounding_s      = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    context = vxGetContext((vx_reference)graph);

    pool_type_s = vxCreateScalar(context, VX_TYPE_ENUM, &pool_type);
    if (vxoReference_GetStatus((vx_reference)pool_type_s) != VX_SUCCESS) return (vx_node)pool_type_s;

    pool_size_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_x);
    if (vxoReference_GetStatus((vx_reference)pool_size_x_s) != VX_SUCCESS) return (vx_node)pool_size_x_s;

    pool_size_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_y);
    if (vxoReference_GetStatus((vx_reference)pool_size_y_s) != VX_SUCCESS) return (vx_node)pool_size_y_s;

    pool_pad_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_pad_x);
    if (vxoReference_GetStatus((vx_reference)pool_pad_x_s) != VX_SUCCESS) return (vx_node)pool_pad_x_s;

    pool_pad_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_pad_y);
    if (vxoReference_GetStatus((vx_reference)pool_pad_y_s) != VX_SUCCESS) return (vx_node)pool_pad_y_s;

    rounding_s = vxCreateScalar(context, VX_TYPE_ENUM, &rounding);
    if (vxoReference_GetStatus((vx_reference)rounding_s) != VX_SUCCESS) return (vx_node)rounding_s;

    parameters[1]  = (vx_reference)pool_type_s;
    parameters[2]  = (vx_reference)pool_size_x_s;
    parameters[3]  = (vx_reference)pool_size_y_s;
    parameters[4]  = (vx_reference)pool_pad_x_s;
    parameters[5]  = (vx_reference)pool_pad_y_s;
    parameters[6]  = (vx_reference)rounding_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_POOLING_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&pool_type_s);
    vxReleaseScalar(&pool_size_x_s);
    vxReleaseScalar(&pool_size_y_s);
    vxReleaseScalar(&pool_pad_x_s);
    vxReleaseScalar(&pool_pad_y_s);
    vxReleaseScalar(&rounding_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxSoftmaxLayer(vx_graph graph, vx_tensor inputs, vx_tensor outputs)
{
    vx_node    node;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    (vx_reference)outputs,
    };

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_SOFTMAX_LAYER, parameters, vxmLENGTH_OF(parameters));

    return node;
}


VX_API_ENTRY vx_node VX_API_CALL vxNormalizationLayer(vx_graph graph,
                                                      vx_tensor inputs,
                                                      vx_enum type,
                                                      vx_uint32 norm_size,
                                                      vx_float32 alpha,
                                                      vx_float32 beta,
                                                      vx_tensor outputs)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_node    node;

    vx_scalar type_s = vxCreateScalar(context, VX_TYPE_ENUM, &type);
    vx_scalar norm_size_s = vxCreateScalar(context, VX_TYPE_INT32, &norm_size);
    vx_scalar alpha_s = vxCreateScalar(context, VX_TYPE_INT32, &alpha);
    vx_scalar beta_s = vxCreateScalar(context, VX_TYPE_INT32, &beta);
    vx_reference    parameters[] = {
         (vx_reference)inputs,
         (vx_reference)type_s,
         (vx_reference)norm_size_s,
         (vx_reference)alpha_s,
         (vx_reference)beta_s,
         (vx_reference)outputs
    };


    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_NORMALIZATION_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&type_s);
    vxReleaseScalar(&norm_size_s);
    vxReleaseScalar(&alpha_s);
    vxReleaseScalar(&beta_s);

    return node;

}


VX_API_ENTRY vx_node VX_API_CALL vxNormalizeImageLayer (vx_graph graph,
                                                       vx_tensor inputs,
                                                       vx_tensor outputs )
{
    vx_node    node;

    vx_reference    parameters[] = {
         (vx_reference)inputs,
         (vx_reference)outputs
    };

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_NORMALIZE_IMAGE_LAYER, parameters, vxmLENGTH_OF(parameters));

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxActivationLayer(
    vx_graph graph,
    vx_tensor inputs,
    vx_enum func,
    vx_int32 a,
    vx_int32 b,
    vx_tensor outputs)
{
    vx_context context;
    vx_node    node;

    vx_scalar func_s      = VX_NULL;
    vx_scalar a_s         = VX_NULL;
    vx_scalar b_s         = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    context = vxGetContext((vx_reference)graph);

    func_s = vxCreateScalar(context, VX_TYPE_ENUM, &func);
    if (vxoReference_GetStatus((vx_reference)func_s) != VX_SUCCESS) return (vx_node)func_s;

    a_s = vxCreateScalar(context, VX_TYPE_INT32, &a);
    if (vxoReference_GetStatus((vx_reference)a_s) != VX_SUCCESS) return (vx_node)a_s;

    b_s = vxCreateScalar(context, VX_TYPE_INT32, &b);
    if (vxoReference_GetStatus((vx_reference)b_s) != VX_SUCCESS) return (vx_node)b_s;

    parameters[1]  = (vx_reference)func_s;
    parameters[2]  = (vx_reference)a_s;
    parameters[3]  = (vx_reference)b_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_ACTIVATION_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&func_s);
    vxReleaseScalar(&a_s);
    vxReleaseScalar(&b_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConvolutionLayer(
    vx_graph graph,
    vx_tensor inputs,
    vx_tensor weights,
    vx_tensor biases,
    const vx_nn_convolution_params_t * convolution_params,
    vx_size size_of_convolution_params,
    vx_tensor outputs)
{
    vx_context context;
    vx_node    node;

    vx_scalar padXScalar                   = VX_NULL;
    vx_scalar padYScalar                   = VX_NULL;
    vx_scalar downScaleSizeRoundingScalar  = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    (vx_reference)weights,
    (vx_reference)biases,
    VX_NULL,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    context = vxGetContext((vx_reference)graph);

    padXScalar = vxCreateScalar(context, VX_TYPE_INT32, &convolution_params->padding_x);
    if (vxoReference_GetStatus((vx_reference)padXScalar) != VX_SUCCESS) return (vx_node)padXScalar;

    padYScalar = vxCreateScalar(context, VX_TYPE_INT32, &convolution_params->padding_y);
    if (vxoReference_GetStatus((vx_reference)padYScalar) != VX_SUCCESS) return (vx_node)padYScalar;

    downScaleSizeRoundingScalar = vxCreateScalar(context, VX_TYPE_INT32, &convolution_params->down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)downScaleSizeRoundingScalar) != VX_SUCCESS) return (vx_node)downScaleSizeRoundingScalar;

    parameters[3]  = (vx_reference)padXScalar;
    parameters[4]  = (vx_reference)padYScalar;
    parameters[5]  = (vx_reference)downScaleSizeRoundingScalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONVOLUTION_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&padXScalar);
    vxReleaseScalar(&padYScalar);
    vxReleaseScalar(&downScaleSizeRoundingScalar);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConcat2Layer(
    vx_graph graph,
    vx_tensor in0,
    vx_tensor in1,
    vx_tensor out)
{
    vx_node    node;

    vx_reference    parameters[] = {
    (vx_reference)in0,
    (vx_reference)in1,
    (vx_reference)out
    };

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONCAT2_LAYER, parameters, vxmLENGTH_OF(parameters));

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConcatIndefiniteLayer(
    vx_graph  graph,
    vx_tensor *in,
    vx_uint32 num,
    vx_uint32 axis,
    vx_tensor out)
{
    vx_context context;
    vx_node    node;

    vx_reference    parameters[] = {
    NULL,
    NULL,
    NULL,
    (vx_reference)out
    };

    vx_scalar inputScalar = VX_NULL;
    vx_scalar numScalar = VX_NULL;
    vx_scalar axisSclar = VX_NULL;

    vx_uint32 input = (vx_uint32)gcmPTR2INT32(in);

    context = vxGetContext((vx_reference)graph);

    inputScalar = vxCreateScalar(context, VX_TYPE_UINT32, &input);
    if (vxoReference_GetStatus((vx_reference)inputScalar) != VX_SUCCESS) return (vx_node)inputScalar;

    numScalar = vxCreateScalar(context, VX_TYPE_UINT32, &num);
    if (vxoReference_GetStatus((vx_reference)numScalar) != VX_SUCCESS) return (vx_node)numScalar;

    axisSclar = vxCreateScalar(context, VX_TYPE_UINT32, &axis);
    if (vxoReference_GetStatus((vx_reference)axisSclar) != VX_SUCCESS) return (vx_node)axisSclar;

    parameters[0]  = (vx_reference)inputScalar;
    parameters[1]  = (vx_reference)numScalar;
    parameters[2]  = (vx_reference)axisSclar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONCATINDEFINITE_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&numScalar);
    vxReleaseScalar(&axisSclar);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorAddNode(
    vx_graph  graph,
    vx_tensor in1,
    vx_tensor in2,
    vx_enum   policy,
    vx_tensor out)
{
    vx_context context;
    vx_node    node;

    vx_scalar policy_s      = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)in1,
    (vx_reference)in2,
    VX_NULL,
    (vx_reference)out
    };

    context = vxGetContext((vx_reference)graph);

    policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &policy);
    if (vxoReference_GetStatus((vx_reference)policy_s) != VX_SUCCESS) return (vx_node)policy_s;

    parameters[2]  = (vx_reference)policy_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_ADD, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&policy_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorMultiplyNode(
    vx_graph graph,
    vx_tensor in1,
    vx_tensor in2,
    vx_scalar scale,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_tensor out)
{
    vx_context context;
    vx_node    node;

    vx_scalar overflow_policy_s      = VX_NULL;
    vx_scalar rounding_policy_s      = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)in1,
    (vx_reference)in2,
    (vx_reference)scale,
    VX_NULL,
    VX_NULL,
    (vx_reference)out
    };

    context = vxGetContext((vx_reference)graph);

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &overflow_policy);
    rounding_policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS || vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS) return NULL;

    parameters[3]  = (vx_reference)overflow_policy_s;
    parameters[4]  = (vx_reference)rounding_policy_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_MUL, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&overflow_policy_s);
    vxReleaseScalar(&rounding_policy_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorDivideNode(
    vx_graph graph,
    vx_tensor in1,
    vx_tensor in2,
    vx_scalar scale,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_tensor out)
{
    vx_context context;
    vx_node    node;

    vx_scalar overflow_policy_s      = VX_NULL;
    vx_scalar rounding_policy_s      = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)in1,
    (vx_reference)in2,
    (vx_reference)scale,
    VX_NULL,
    VX_NULL,
    (vx_reference)out
    };

    context = vxGetContext((vx_reference)graph);

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &overflow_policy);
    rounding_policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS || vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS) return NULL;

    parameters[3]  = (vx_reference)overflow_policy_s;
    parameters[4]  = (vx_reference)rounding_policy_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_DIV, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&overflow_policy_s);
    vxReleaseScalar(&rounding_policy_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorSubtractNode(
    vx_graph graph,
    vx_tensor in1,
    vx_tensor in2,
    vx_enum policy,
    vx_tensor out)
{
    vx_context context;
    vx_node    node;

    vx_scalar policy_s      = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)in1,
    (vx_reference)in2,
    VX_NULL,
    (vx_reference)out
    };

    context = vxGetContext((vx_reference)graph);

    policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &policy);
    if (vxoReference_GetStatus((vx_reference)policy_s) != VX_SUCCESS) return (vx_node)policy_s;

    parameters[2]  = (vx_reference)policy_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_SUB, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&policy_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorTableLookupNode(
    vx_graph graph,
    vx_tensor in1,
    vx_lut lut,
    vx_tensor out)
{
    /* to do */
    return VX_NULL;
}


VX_API_ENTRY vx_node VX_API_CALL vxTensorTransposeNode(
    vx_graph graph,
    vx_tensor inputs,
    vx_tensor outputs,
    vx_uint32 dim1,
    vx_uint32 dim2)
{
    vx_context context;
    vx_node    node;
    vx_uint32  nt;
    vx_uint32  indims[VX_CONTEXT_TENSOR_MAX_DIMENSION], outdims[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vx_array perm = VX_NULL;
    vx_scalar pnum = VX_NULL;

    vx_reference parameters[] = {
    (vx_reference)inputs,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    context = vxGetContext((vx_reference)graph);

    if (dim1 == dim2)
        return VX_NULL;

    vxQueryTensor(inputs, VX_TENSOR_NUM_OF_DIMS, &nt, sizeof(nt));
    if (dim1 >= nt || dim2 >= nt) return VX_NULL;

    vxoTensor_GetTensorDimStride(inputs, &nt, indims, VX_NULL);
    vxoTensor_GetTensorDimStride(outputs, &nt, outdims, VX_NULL);
    if (indims[dim1] != outdims[dim2] || indims[dim2] != outdims[dim1]) return VX_NULL;

    perm = vxCreateArray(context, VX_TYPE_UINT32, nt);
    if (!vxoArray_AllocateMemory(perm))
    {
        return VX_NULL;
    }
    else
    {
        vx_uint32 i;
        vx_uint32* pos = (vx_uint32*)perm->memory.logicals[0];
        for (i=0; i<nt; i++) pos[i] = i;
        pos[dim1] = dim2;
        pos[dim2] = dim1;
    }

    pnum = vxCreateScalar(context, VX_TYPE_UINT32, &nt);
    if (vxoReference_GetStatus((vx_reference)pnum) != VX_SUCCESS) return (vx_node)pnum;

    parameters[1]  = (vx_reference)perm;
    parameters[2]  = (vx_reference)pnum;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_TRANS, parameters, vxmLENGTH_OF(parameters));

    vxReleaseArray(&perm);
    vxReleaseScalar(&pnum);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorPermuteNode(
    vx_graph        graph,
    vx_tensor       inputs,
    vx_tensor       outputs,
    vx_uint32*      perm,
    vx_uint32       sizes_of_perm
    )
{
    vx_context context;
    vx_node    node;
    vx_uint32  i, nt;
    vx_uint32  indims[VX_CONTEXT_TENSOR_MAX_DIMENSION], outdims[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vx_array perma = VX_NULL;
    vx_scalar pnum = VX_NULL;

    vx_reference parameters[] = {
    (vx_reference)inputs,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    context = vxGetContext((vx_reference)graph);

    vxQueryTensor(inputs, VX_TENSOR_NUM_OF_DIMS, &nt, sizeof(nt));

    if (perm == VX_NULL) sizes_of_perm = nt;
    else if (sizes_of_perm > nt) return VX_NULL;

    vxoTensor_GetTensorDimStride(inputs, &nt, indims, VX_NULL);
    vxoTensor_GetTensorDimStride(outputs, &nt, outdims, VX_NULL);
    for (i = 0; i < sizes_of_perm; i++)
    {
        if (indims[perm[i]] != outdims[i])
            return VX_NULL;
    }

    perma = vxCreateArray(context, VX_TYPE_UINT32, nt);
    if (!vxoArray_AllocateMemory(perma))
    {
        return VX_NULL;
    }
    else
    {
        vx_uint32 i;
        vx_uint32* pos = (vx_uint32*)perma->memory.logicals[0];
        for (i=0; i<nt; i++)
        {
            if (perm == VX_NULL) pos[i] = nt - i - 1;
            else pos[i] = i < sizes_of_perm ? perm[i] : i;
        }
    }

    pnum = vxCreateScalar(context, VX_TYPE_UINT32, &sizes_of_perm);
    if (vxoReference_GetStatus((vx_reference)pnum) != VX_SUCCESS) return (vx_node)pnum;

    parameters[1]  = (vx_reference)perma;
    parameters[2]  = (vx_reference)pnum;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_TRANS, parameters, vxmLENGTH_OF(parameters));

    vxReleaseArray(&perma);
    vxReleaseScalar(&pnum);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxLeakyReluLayer(
    vx_graph                    graph,
    vx_tensor                   inputs,
    vx_float32                  negative_slope,
    vx_tensor                   outputs
    )
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        VX_NULL,
        (vx_reference)outputs
    };

    vx_node node = VX_NULL;

    vx_scalar negative_slopes = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &negative_slope);
    if (vxoReference_GetStatus((vx_reference)negative_slopes) != VX_SUCCESS) return (vx_node)negative_slopes;

    parameters[1]  = (vx_reference)negative_slopes;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_LEAKY, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&negative_slopes);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxBatchNormalizationLayer(
    vx_graph                    graph,
    vx_float32                  eps,
    vx_tensor                   mean,
    vx_tensor                   variance,
    vx_tensor                   gamma,
    vx_tensor                   beta,
    vx_tensor                   input,
    vx_tensor                   output
    )
{
    vx_context context;
    vx_node    node;

    vx_scalar eps_s         = VX_NULL;

    vx_reference parameters[] = {
    VX_NULL,
    (vx_reference)mean,
    (vx_reference)variance,
    (vx_reference)gamma,
    (vx_reference)beta,
    (vx_reference)input,
    (vx_reference)output
    };

    context = vxGetContext((vx_reference)graph);

    eps_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps);
    if (vxoReference_GetStatus((vx_reference)eps_s) != VX_SUCCESS) return (vx_node)eps_s;

    parameters[0]  = (vx_reference)eps_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_BATCH_NORM, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&eps_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxRPNLayer(
    vx_graph                    graph,
    vx_tensor                   score,
    vx_tensor                   bbox,
    vx_tensor                   anchors,
    vx_tensor                   img_info,
    const vx_nn_rpn_params_t *  rpn_params,
    vx_size                     size_of_rpn_params,
    vx_tensor                   roi_output,
    vx_tensor                   score_output
    )
{
    vx_context  context;
    vx_node     node;

    vx_scalar   feature_stride_s    = VX_NULL;
    vx_scalar   min_size_s          = VX_NULL;
    vx_scalar   pre_nms_topn_s      = VX_NULL;
    vx_scalar   post_nms_topn_s     = VX_NULL;
    vx_scalar   nms_thresh_s        = VX_NULL;

    vx_reference parameters[] = {
        (vx_reference)score,
        (vx_reference)bbox,
        (vx_reference)anchors,
        (vx_reference)img_info,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        (vx_reference)roi_output,
        (vx_reference)score_output
    };

    context = vxGetContext((vx_reference)graph);

    feature_stride_s = vxCreateScalar(context, VX_TYPE_UINT32, &rpn_params->feature_stride);
    if (vxoReference_GetStatus((vx_reference)feature_stride_s) != VX_SUCCESS) return (vx_node)feature_stride_s;

    min_size_s = vxCreateScalar(context, VX_TYPE_UINT32, &rpn_params->min_size);
    if (vxoReference_GetStatus((vx_reference)min_size_s) != VX_SUCCESS) return (vx_node)min_size_s;

    pre_nms_topn_s = vxCreateScalar(context, VX_TYPE_UINT32, &rpn_params->pre_nms_topn);
    if (vxoReference_GetStatus((vx_reference)pre_nms_topn_s) != VX_SUCCESS) return (vx_node)pre_nms_topn_s;

    post_nms_topn_s = vxCreateScalar(context, VX_TYPE_UINT32, &rpn_params->post_nms_topn);
    if (vxoReference_GetStatus((vx_reference)post_nms_topn_s) != VX_SUCCESS) return (vx_node)post_nms_topn_s;

    nms_thresh_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &rpn_params->nms_thresh);
    if (vxoReference_GetStatus((vx_reference)nms_thresh_s) != VX_SUCCESS) return (vx_node)nms_thresh_s;

    parameters[4]  = (vx_reference)feature_stride_s;
    parameters[5]  = (vx_reference)min_size_s;
    parameters[6]  = (vx_reference)pre_nms_topn_s;
    parameters[7]  = (vx_reference)post_nms_topn_s;
    parameters[8]  = (vx_reference)nms_thresh_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_RPN, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&feature_stride_s);
    vxReleaseScalar(&min_size_s);
    vxReleaseScalar(&pre_nms_topn_s);
    vxReleaseScalar(&post_nms_topn_s);
    vxReleaseScalar(&nms_thresh_s);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxROIPoolingLayer(
    vx_graph  graph,
    vx_tensor input_data,
    vx_tensor input_rois,
    const vx_nn_roi_pool_params_t *roi_pool_params,
    vx_size size_of_roi_params,
    vx_tensor output_arr
    )
{
    if (size_of_roi_params != sizeof(vx_nn_roi_pool_params_ext_t))
    {
        return VX_NULL;
    }
    else
    {

        vx_reference parameters[] = {
            (vx_reference)input_data,
            (vx_reference)input_rois,
            NULL,
            NULL,
            NULL,
            NULL,
            (vx_reference)output_arr
        };

        vx_context context = vxGetContext((vx_reference)graph);
        vx_node node;

        vx_scalar pool_types = NULL;
        vx_scalar spatial_scales = NULL;
        vx_scalar pooled_heights = NULL;
        vx_scalar pooled_widths = NULL;

        vx_nn_roi_pool_params_ext_t * roi_pool_params_ext = (vx_nn_roi_pool_params_ext_t *)roi_pool_params;

        pool_types = vxCreateScalar(context, VX_TYPE_ENUM, &roi_pool_params_ext->khr.pool_type);
        if (vxoReference_GetStatus((vx_reference)pool_types) != VX_SUCCESS) return (vx_node)pool_types;

        spatial_scales = vxCreateScalar(context, VX_TYPE_FLOAT32, &roi_pool_params_ext->spatial_scale);
        if (vxoReference_GetStatus((vx_reference)spatial_scales) != VX_SUCCESS) return (vx_node)spatial_scales;

        pooled_heights = vxCreateScalar(context, VX_TYPE_INT32, &roi_pool_params_ext->pooled_height);
        if (vxoReference_GetStatus((vx_reference)pooled_heights) != VX_SUCCESS) return (vx_node)pooled_heights;

        pooled_widths = vxCreateScalar(context, VX_TYPE_INT32, &roi_pool_params_ext->pooled_width);
        if (vxoReference_GetStatus((vx_reference)pooled_widths) != VX_SUCCESS) return (vx_node)pooled_widths;

        parameters[2]  = (vx_reference)pool_types;
        parameters[3]  = (vx_reference)spatial_scales;
        parameters[4]  = (vx_reference)pooled_heights;
        parameters[5]  = (vx_reference)pooled_widths;

        node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_ROIPOOL, parameters, vxmLENGTH_OF(parameters));

        vxReleaseScalar(&pool_types);
        vxReleaseScalar(&spatial_scales);
        vxReleaseScalar(&pooled_heights);
        vxReleaseScalar(&pooled_widths);

        return node;
    }
}

VX_API_ENTRY vx_node VX_API_CALL vxReorgLayer(
    vx_graph                    graph,
    vx_tensor                   inputs,
    vx_uint32                   stride,
    vx_tensor                   outputs
    )
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        VX_NULL,
        (vx_reference)outputs
    };

    vx_node node = VX_NULL;

    vx_scalar strides = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &stride);
    if (vxoReference_GetStatus((vx_reference)strides) != VX_SUCCESS) return (vx_node)strides;

    parameters[1]  = (vx_reference)strides;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_REORG_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&strides);

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxL2NormalizeLayer(
    vx_graph                    graph,
    vx_tensor                   inputs,
    vx_tensor                   outputs
    )
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        (vx_reference)outputs
    };

    vx_node node = VX_NULL;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_L2NORMALIZE_LAYER, parameters, vxmLENGTH_OF(parameters));

    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxDeconvolutionLayer(
    vx_graph graph,
    vx_tensor inputs,
    vx_tensor weights,
    vx_tensor  biases,
    const vx_nn_deconvolution_params_t *deconvolution_params,
    vx_size size_of_deconv_params,
    vx_tensor outputs)
{
    if (size_of_deconv_params != sizeof(vx_nn_deconvolution_params_ext_t))
    {
        return NULL;
    }
    else
    {
        vx_node node = VX_NULL;
        vx_int32 i = 0;
        vx_nn_deconvolution_params_ext_t *deconvolution_params_ext = (vx_nn_deconvolution_params_ext_t *)deconvolution_params;
        vx_reference parameters[] = {
            (vx_reference)inputs,
            (vx_reference)weights,
            (vx_reference)biases,
            (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &deconvolution_params_ext->khr.padding_x),
            (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &deconvolution_params_ext->khr.padding_y),
            (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &deconvolution_params_ext->khr.overflow_policy),
            (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &deconvolution_params_ext->khr.rounding_policy),
            (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &deconvolution_params_ext->khr.a_x),
            (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &deconvolution_params_ext->khr.a_y),
            (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &deconvolution_params_ext->channel_group),
            (vx_reference)outputs,
        };

        node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_DECONVOLUTION_LAYER, parameters, vxmLENGTH_OF(parameters));

        for (i = 3; i < (gcmCOUNTOF(parameters) - 1); i ++)
            vxReleaseScalar((vx_scalar*)&parameters[i]);

        return node;
    }
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorCopyNode(
    vx_graph graph,
    vx_tensor src,
    vx_tensor dst)
{
    vx_node node = VX_NULL;
    vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)dst
    };

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_COPY, parameters, vxmLENGTH_OF(parameters));

    return node;
}

