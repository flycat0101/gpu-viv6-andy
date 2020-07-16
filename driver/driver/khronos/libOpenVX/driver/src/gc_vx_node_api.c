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


#include <gc_vx_common.h>
#include <float.h>

#define _GC_OBJ_ZONE            gcdZONE_VX_NODE

VX_API_ENTRY vx_node VX_API_CALL vxColorConvertNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxColorConvertNode: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input=%p, channelNum=0x%x output=%p", graph, input, channelNum, output);
    gcmDUMP_API("$VX vxChannelExtractNode: graph=%p, input=%p, channelNum=0x%x output=%p", graph, input, channelNum, output);

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &channelNum);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get Scale reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalar), "%s[%d]: Get Scale reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalar;
    }

    parameters[1] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_CHANNEL_EXTRACT, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, plane0=%p, plane1=%p, plane2=%p, plane3=%p, output=%p", graph, plane0, plane1, plane2, plane3, output);
    gcmDUMP_API("$VX vxChannelCombineNode: graph=%p, plane0=%p, plane1=%p, plane2=%p, plane3=%p, output=%p", graph, plane0, plane1, plane2, plane3, output);
    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_CHANNEL_COMBINE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxSobel3x3Node(vx_graph graph, vx_image input, vx_image output_x, vx_image output_y)
{
    vx_reference parameters[] = {
       (vx_reference)input,
       (vx_reference)output_x,
       (vx_reference)output_y
    };
    gcmHEADER_ARG("graph=%p, input=%p, output_x=%p, output_y=%p", graph, input, output_x, output_y);
    gcmDUMP_API("$VX vxSobel3x3Node: graph=%p, input=%p, output_x=%p, output_y=%p", graph, input, output_x, output_y);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_SOBEL_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxMagnitudeNode(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image mag)
{
    vx_reference parameters[] = {
       (vx_reference)grad_x,
       (vx_reference)grad_y,
       (vx_reference)mag
    };
    gcmHEADER_ARG(" graph=%p, grad_x=%p, grad_y=%p, mag=%p", graph, grad_x, grad_y, mag);
    gcmDUMP_API("$VX vxMagnitudeNode: graph=%p, grad_x=%p, grad_y=%p, mag=%p", graph, grad_x, grad_y, mag);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_MAGNITUDE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxPhaseNode(vx_graph graph, vx_image grad_x, vx_image grad_y, vx_image orientation)
{
    vx_reference parameters[] = {
       (vx_reference)grad_x,
       (vx_reference)grad_y,
       (vx_reference)orientation
    };
    gcmHEADER_ARG("graph=%p, grad_x=%p, grad_y=%p, orientation=%p", graph, grad_x, grad_y, orientation);
    gcmDUMP_API("$VX vxPhaseNode: graph=%p, grad_x=%p, grad_y=%p, orientation=%p", graph, grad_x, grad_y, orientation);

    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, src=%p, dst=%p, type=0x%x", graph, src, dst, type);
    gcmDUMP_API("$VX vxScaleImageNode: graph=%p, src=%p, dst=%p, type=0x%x", graph, src, dst, type);

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &type);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get ImageNode Scale reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalar), "%s[%d]: Get ImageNode Scale reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalar;
    }

    parameters[2] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_SCALE_IMAGE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTableLookupNode(vx_graph graph, vx_image input, vx_lut lut, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)lut,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, lut=%p, output=%p", graph, input, lut, output);
    gcmDUMP_API("$VX vxTableLookupNode: graph=%p, input=%p, lut=%p, output=%p", graph, input, lut, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_TABLE_LOOKUP, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxHistogramNode(vx_graph graph, vx_image input, vx_distribution distribution)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)distribution
    };
    gcmHEADER_ARG("graph=%p, input=%p, distribution=%p", graph, input, distribution);
    gcmDUMP_API("$VX vxHistogramNode: graph=%p, input=%p, distribution=%p", graph, input, distribution);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_HISTOGRAM, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxEqualizeHistNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxEqualizeHistNode: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_EQUALIZE_HISTOGRAM, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxAbsDiffNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)out
    };
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);
    gcmDUMP_API("$VX vxAbsDiffNode: graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_ABSDIFF, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxMeanStdDevNode(vx_graph graph, vx_image input, vx_scalar mean, vx_scalar stddev)
{
    vx_reference parameters[] = {
       (vx_reference)input,
       (vx_reference)mean,
       (vx_reference)stddev
    };
    gcmHEADER_ARG("graph=%p, input=%p, mean=%p, stddev=%p", graph, input, mean, stddev);
    gcmDUMP_API("$VX vxMeanStdDevNode: graph=%p, input=%p, mean=%p, stddev=%p", graph, input, mean, stddev);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_MEAN_STDDEV, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxThresholdNode(vx_graph graph, vx_image input, vx_threshold thesh, vx_image output)
{
    vx_node node = NULL;
    vx_scalar true_false_value, upper_lower_value, type;
    vx_uint32 value = 0;
    vx_int32 value1 = 0;
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)thesh,
        NULL,
        NULL,
        NULL,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, thesh=%p, output=%p", graph, input, thesh, output);
    gcmDUMP_API("$VX vxThresholdNode: graph=%p, input=%p, thesh=%p, output=%p", graph, input, thesh, output);

    true_false_value = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &value);
    if (vxoReference_GetStatus((vx_reference)true_false_value) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)true_false_value;
    }
    parameters[2] = (vx_reference)true_false_value;

    upper_lower_value = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &value);
    if (vxoReference_GetStatus((vx_reference)upper_lower_value) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)upper_lower_value;
    }
    parameters[3] = (vx_reference)upper_lower_value;

    type = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &value1);
    if (vxoReference_GetStatus((vx_reference)type) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get type reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&type->base, vxoReference_GetStatus((vx_reference)type), "%s[%d]: Get type reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)type;
    }
    parameters[4] = (vx_reference)type;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_THRESHOLD, parameters, vxmLENGTH_OF(parameters));
    vxReleaseScalar(&true_false_value);
    vxReleaseScalar(&upper_lower_value);
    vxReleaseScalar(&type);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxIntegralImageNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxIntegralImageNode: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTEGRAL_IMAGE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxErode3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxErode3x3Node: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_ERODE_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxDilate3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxDilate3x3Node: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_DILATE_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxMedian3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxMedian3x3Node: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_MEDIAN_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxBox3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxBox3x3Node: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_BOX_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxGaussian3x3Node(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxGaussian3x3Node: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_GAUSSIAN_3x3, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxNonLinearFilterNode(vx_graph graph, vx_enum function, vx_image input, vx_matrix mask, vx_image output)
{
    vx_node node;
    vx_scalar func = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &function);
    vx_reference parameters[] = {
        (vx_reference)func,
        (vx_reference)input,
        (vx_reference)mask,
        (vx_reference)output,
    };
    gcmHEADER_ARG("graph=%p, function=%p, input=%p, mask=%p, output=%p", graph, function, input, mask, output);
    gcmDUMP_API("$VX vxNonLinearFilterNode: graph=%p, function=%p, input=%p, mask=%p, output=%p", graph, function, input, mask, output);

    node = vxoNode_CreateSpecific(graph,
        VX_KERNEL_NON_LINEAR_FILTER,
        parameters,
        vxmLENGTH_OF(parameters));

    vxReleaseScalar(&func);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConvolveNode(vx_graph graph, vx_image input, vx_convolution conv, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)conv,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, conv=%p, output=%p", graph, input, conv, output);
    gcmDUMP_API("$VX vxConvolveNode: graph=%p, input=%p, conv=%p, output=%p", graph, input, conv, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_CUSTOM_CONVOLUTION, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxGaussianPyramidNode(vx_graph graph, vx_image input, vx_pyramid gaussian)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)gaussian
    };
    gcmHEADER_ARG("graph=%p, input=%p, gaussian=%p", graph, input, gaussian);
    gcmDUMP_API("$VX vxGaussianPyramidNode: graph=%p, input=%p, gaussian=%p", graph, input, gaussian);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_GAUSSIAN_PYRAMID, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateImageNode(vx_graph graph, vx_image input, vx_image accum)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)accum
    };
    gcmHEADER_ARG("graph=%p, input=%p, accum=%p", graph, input, accum);
    gcmDUMP_API("$VX vxAccumulateImageNode: graph=%p, input=%p, accum=%p", graph, input, accum);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_ACCUMULATE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateWeightedImageNode(vx_graph graph, vx_image input, vx_scalar alpha, vx_image accum)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)alpha,
        (vx_reference)accum
    };
    gcmHEADER_ARG("graph=%p, input=%p, alpha=%p, accum=%p", graph, input, alpha, accum);
    gcmDUMP_API("$VX vxAccumulateWeightedImageNode: graph=%p, input=%p, alpha=%p, accum=%p", graph, input, alpha, accum);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_ACCUMULATE_WEIGHTED, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxAccumulateSquareImageNode(vx_graph graph, vx_image input, vx_scalar scalar, vx_image accum)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)scalar,
        (vx_reference)accum
    };
    gcmHEADER_ARG("graph=%p, input=%p, scalar=%p, accum=%p", graph, input, scalar, accum);
    gcmDUMP_API("$VX vxAccumulateSquareImageNode: graph=%p, input=%p, scalar=%p, accum=%p", graph, input, scalar, accum);

    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input=%p, minVal=%p, maxVal=%p, minLoc=%p, maxLoc=%p, minCount=%p, maxCount=%p", graph, input, minVal, maxVal, minLoc, maxLoc, minCount, maxCount);
    gcmDUMP_API("$VX vxMinMaxLocNode: graph=%p, input=%p, minVal=%p, maxVal=%p, minLoc=%p, maxLoc=%p, minCount=%p, maxCount=%p", graph, input, minVal, maxVal, minLoc, maxLoc, minCount, maxCount);

    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input=%p, output=%p, policy=%p, shift=%p", graph, input, output, policy, shift);
    gcmDUMP_API("$VX vxConvertDepthNode: graph=%p, input=%p, output=%p, policy=%p, shift=%p", graph, input, output, policy, shift);

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &policy);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get Scale reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalar), "%s[%d]: Get Scale reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalar;
    }

    parameters[2] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_CONVERTDEPTH, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input=%p, hyst=%p, gradient_size=0x%x, norm_type=0x%x, output=%p", graph, input, hyst, gradient_size, norm_type, output);
    gcmDUMP_API("$VX vxCannyEdgeDetectorNode: graph=%p, input=%p, hyst=%p, gradient_size=0x%x, norm_type=0x%x, output=%p", graph, input, hyst, gradient_size, norm_type, output);

    scalarGradientSize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &gradient_size);

    if (vxoReference_GetStatus((vx_reference)scalarGradientSize) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarGradientSize reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarGradientSize), "%s[%d]: Get scalarGradientSize reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarGradientSize;
    }

    parameters[2] = (vx_reference)scalarGradientSize;

    scalarNormType = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &norm_type);

    if (vxoReference_GetStatus((vx_reference)scalarNormType) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarNormType reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarNormType), "%s[%d]: Get scalarNormType reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarNormType;
    }

    parameters[3] = (vx_reference)scalarNormType;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_CANNY_EDGE_DETECTOR, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarGradientSize);

    vxReleaseScalar(&scalarNormType);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxAndNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)out
    };
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);
    gcmDUMP_API("$VX vxAndNode: graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_AND, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxOrNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)out
    };
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);
    gcmDUMP_API("$VX vxOrNode: graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_OR, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxXorNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)out
    };
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);
    gcmDUMP_API("$VX vxXorNode: graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);
    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_XOR, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxNotNode(vx_graph graph, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
       (vx_reference)input,
       (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxNotNode: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, scale=%p, overflow_policy=0x%x, rounding_policy=0x%x, out=%p", graph, in1, in2, scale, overflow_policy, rounding_policy, out);
    gcmDUMP_API("$VX vxMultiplyNode: graph=%p, in1=%p, in2=%p, scale=%p, overflow_policy=0x%x, rounding_policy=0x%x, out=%p", graph, in1, in2, scale, overflow_policy, rounding_policy, out);

    scalarOverflowPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &overflow_policy);

    if (vxoReference_GetStatus((vx_reference)scalarOverflowPolicy) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarOverflowPolicy reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarOverflowPolicy), "%s[%d]: Get scalarOverflowPolicy reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarOverflowPolicy;
    }

    parameters[3] = (vx_reference)scalarOverflowPolicy;

    scalarRoundingPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &rounding_policy);

    if (vxoReference_GetStatus((vx_reference)scalarRoundingPolicy) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarRoundingPolicy reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarRoundingPolicy), "%s[%d]: Get scalarRoundingPolicy reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarRoundingPolicy;
    }

    parameters[4] = (vx_reference)scalarRoundingPolicy;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_MULTIPLY, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarOverflowPolicy);

    vxReleaseScalar(&scalarRoundingPolicy);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, policy=0x%x, out=%p", graph, in1, in2, policy, out);
    gcmDUMP_API("$VX vxAddNode: graph=%p, in1=%p, in2=%p, policy=0x%x, out=%p", graph, in1, in2, policy, out);

    scalarPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &policy);

    if (vxoReference_GetStatus((vx_reference)scalarPolicy) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarPolicy reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarPolicy), "%s[%d]: Get scalarPolicy reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarPolicy;
    }
    parameters[2] = (vx_reference)scalarPolicy;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_ADD, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarPolicy);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, policy=0x%x, out=%p", graph, in1, in2, policy, out);
    gcmDUMP_API("$VX vxSubtractNode: graph=%p, in1=%p, in2=%p, policy=0x%x, out=%p", graph, in1, in2, policy, out);

    scalarPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &policy);

    if (vxoReference_GetStatus((vx_reference)scalarPolicy) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarPolicy reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarPolicy), "%s[%d]: Get scalarPolicy reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarPolicy;
    }
    parameters[2] = (vx_reference)scalarPolicy;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_SUBTRACT, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarPolicy);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input=%p, matrix=%p, type=0x%x, output=%p", graph, input, matrix, type, output);
    gcmDUMP_API("$VX vxWarpAffineNode: graph=%p, input=%p, matrix=%p, type=0x%x, output=%p", graph, input, matrix, type, output);

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &type);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get Scale reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalar), "%s[%d]: Get Scale reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalar;
    }

    parameters[2] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_WARP_AFFINE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input=%p, matrix=%p, type=0x%x, output=%p", graph, input, matrix, type, output);
    gcmDUMP_API("$VX vxWarpPerspectiveNode: graph=%p, input=%p, matrix=%p, type=0x%x, output=%p", graph, input, matrix, type, output);

    scalar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &type);

    if (vxoReference_GetStatus((vx_reference)scalar) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get Scale reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalar), "%s[%d]: Get Scale reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalar;
    }

    parameters[2] = (vx_reference)scalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_WARP_PERSPECTIVE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalar);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input=%p, strength_thresh=%p, min_distance=%p, sensitivity=%p, gradient_size=0x%x, block_size=0x%x, corners=%p, num_corners=%p",
        graph, input, strength_thresh, min_distance, sensitivity, gradient_size, block_size, corners, num_corners);
    gcmDUMP_API("$VX vxHarrisCornersNode: graph=%p, input=%p, strength_thresh=%p, min_distance=%p, sensitivity=%p, gradient_size=0x%x, block_size=0x%x, corners=%p, num_corners=%p",
        graph, input, strength_thresh, min_distance, sensitivity, gradient_size, block_size, corners, num_corners);

    scalarGradientSize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &gradient_size);

    if (vxoReference_GetStatus((vx_reference)scalarGradientSize) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarGradientSize reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarGradientSize), "%s[%d]: Get scalarGradientSize reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarGradientSize;
    }

    parameters[4] = (vx_reference)scalarGradientSize;

    scalarBlockSize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &block_size);

    if (vxoReference_GetStatus((vx_reference)scalarBlockSize) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarBlockSize reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarBlockSize), "%s[%d]: Get scalarBlockSize reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarBlockSize;
    }

    parameters[5] = (vx_reference)scalarBlockSize;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_HARRIS_CORNERS, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarGradientSize);

    vxReleaseScalar(&scalarBlockSize);
    gcmFOOTER_NO();
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

    gcmHEADER_ARG("graph=%p, input=%p, strength_thresh=%p, nonmax_suppression=0x%x, corners=%p, num_corners=%p",
        graph, input, strength_thresh, nonmax_suppression, corners, num_corners);
    gcmDUMP_API("$VX vxFastCornersNode: graph=%p, input=%p, strength_thresh=%p, nonmax_suppression=0x%x, corners=%p, num_corners=%p",
        graph, input, strength_thresh, nonmax_suppression, corners, num_corners);

    scalarNonmaxSuppression = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_BOOL, &nonmax_suppression);

    if (vxoReference_GetStatus((vx_reference)scalarNonmaxSuppression) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarNonmaxSuppression reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarNonmaxSuppression), "%s[%d]: Get scalarNonmaxSuppression reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarNonmaxSuppression;
    }

    parameters[2] = (vx_reference)scalarNonmaxSuppression;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_FAST_CORNERS, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarNonmaxSuppression);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, old_images=%p, new_images=%p, old_points=%p, new_points_estimates=%p, new_points=%p, termination=0x%x, epsilon=%p,"\
        " num_iterations=%p, use_initial_estimate=%p, window_dimension=0x%lx", graph, old_images, new_images, old_points, new_points_estimates, new_points, termination,
        epsilon, num_iterations, use_initial_estimate, window_dimension);
    gcmDUMP_API("$VX vxOpticalFlowPyrLKNode: graph=%p, old_images=%p, new_images=%p, old_points=%p, new_points_estimates=%p, new_points=%p, termination=0x%x, epsilon=%p,"\
        " num_iterations=%p, use_initial_estimate=%p, window_dimension=0x%lx", graph, old_images, new_images, old_points, new_points_estimates, new_points, termination,
        epsilon, num_iterations, use_initial_estimate, window_dimension);

    scalarTermination = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &termination);

    if (vxoReference_GetStatus((vx_reference)scalarTermination) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalarTermination;
    }
    parameters[5] = (vx_reference)scalarTermination;

    scalarWindowDimension = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &window_dimension);

    if (vxoReference_GetStatus((vx_reference)scalarWindowDimension) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalarWindowDimension;
    }
    parameters[9] = (vx_reference)scalarWindowDimension;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_OPTICAL_FLOW_PYR_LK, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarTermination);

    vxReleaseScalar(&scalarWindowDimension);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input=%p, table=%p, policy=0x%x, out=%p", graph, input, table, policy, output);
    gcmDUMP_API("$VX vxRemapNode: graph=%p, input=%p, table=%p, policy=0x%x, out=%p", graph, input, table, policy, output);

    scalarPolicy = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &policy);

    if (vxoReference_GetStatus((vx_reference)scalarPolicy) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get scalarPolicy reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)scalarPolicy), "%s[%d]: Get scalarPolicy reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)scalarPolicy;
    }

    parameters[2] = (vx_reference)scalarPolicy;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_REMAP, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarPolicy);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input=%p, output=%p, kernel_size=0x%x", graph, input, output, kernel_size);
    gcmDUMP_API("$VX vxHalfScaleGaussianNode: graph=%p, input=%p, output=%p, kernel_size=0x%x", graph, input, output, kernel_size);

    scalarKernelSize = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &kernel_size);

    if (vxoReference_GetStatus((vx_reference)scalarKernelSize) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalarKernelSize;
    }
    parameters[2] = (vx_reference)scalarKernelSize;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_HALFSCALE_GAUSSIAN, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&scalarKernelSize);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxLaplacianPyramidNode(vx_graph graph, vx_image input, vx_pyramid laplacian, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)laplacian,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, laplacian=%p, output=%p", graph, input, laplacian, output);
    gcmDUMP_API("$VX vxLaplacianPyramidNode: graph=%p, input=%p, laplacian=%p, output=%p", graph, input, laplacian, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_LAPLACIAN_PYRAMID, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxLaplacianReconstructNode(vx_graph graph, vx_pyramid laplacian, vx_image input, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)laplacian,
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, laplacian=%p, input=%p, output=%p", graph, laplacian, input, output);
    gcmDUMP_API("$VX vxLaplacianReconstructNode: graph=%p, laplacian=%p, input=%p, output=%p", graph, laplacian, input, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_LAPLACIAN_RECONSTRUCT, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxSgmNode(vx_graph graph, vx_image right_img, vx_image left_img, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)right_img,
        (vx_reference)left_img,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, right_img=%p, left_img=%p, output=%p", graph, right_img, left_img, output);
    gcmDUMP_API("$VX vxSgmNode: graph=%p, right_img=%p, left_img=%p, output=%p", graph, right_img, left_img, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_SGM, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxCensus3x3Node(vx_graph graph, vx_image src, vx_image dst)
{
    vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)dst,
    };
    gcmHEADER_ARG("graph=%p, src=%p, dst=%p", graph, src, dst);
    gcmDUMP_API("$VX vxCensus3x3Node: graph=%p, src=%p, dst=%p", graph, src, dst);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_INTERNAL_CENSUS3x3, parameters, vxmLENGTH_OF(parameters));
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
    gcmHEADER_ARG("graph=%p, inputs=%p, weights_biases=%p, pad=0x%x, accumulator_bits=0x%x, overflow_policy=0x%x, rounding_policy=0x%x,"\
        " down_scale_size_rounding=0x%x, enable_relu=0x%x, outputs=%p", graph, inputs, weights_biases, pad, accumulator_bits, overflow_policy, rounding_policy, down_scale_size_rounding, enable_relu, outputs);
    gcmDUMP_API("$VX vxFullyConnectedReluLayer: graph=%p, inputs=%p, weights_biases=%p, pad=0x%x, accumulator_bits=0x%x, overflow_policy=0x%x, rounding_policy=0x%x,"\
        " down_scale_size_rounding=0x%x, enable_relu=0x%x, outputs=%p", graph, inputs, weights_biases, pad, accumulator_bits, overflow_policy, rounding_policy, down_scale_size_rounding, enable_relu, outputs);

    context = vxGetContext((vx_reference)graph);

    pad_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad);
    if (vxoReference_GetStatus((vx_reference)pad_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pad_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pad_s), "%s[%d]: Get pad_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pad_s;
    }

    accumulator_bits_s = vxCreateScalar(context, VX_TYPE_UINT32, &accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)accumulator_bits_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get accumulator_bits_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)accumulator_bits_s), "%s[%d]: Get accumulator_bits_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)accumulator_bits_s;
    }

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)overflow_policy_s), "%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)overflow_policy_s;
    }

    rounding_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)rounding_policy_s), "%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)rounding_policy_s;
    }

    down_scale_size_rounding_s = vxCreateScalar(context, VX_TYPE_UINT32, &down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get down_scale_size_rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s), "%s[%d]: Get down_scale_size_rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)down_scale_size_rounding_s;
    }

    enable_relu_s = vxCreateScalar(context, VX_TYPE_UINT32, &enable_relu);
    if (vxoReference_GetStatus((vx_reference)enable_relu_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get enable_relu_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)enable_relu_s), "%s[%d]: Get enable_relu_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)enable_relu_s;
    }

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
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxFullyConnectedLayer(
    vx_graph graph,
    vx_tensor inputs,
    vx_tensor weights,
    vx_tensor biases,
    vx_enum overflow_policy,
    vx_enum rounding_policy,
    vx_tensor outputs)
{
    vx_context context;
    vx_node    node;
    vx_scalar overflow_policy_s          = VX_NULL;
    vx_scalar rounding_policy_s          = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    (vx_reference)weights,
    (vx_reference)biases,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };
    gcmHEADER_ARG("graph=%p, inputs=%p, weights=%p, biases=0x%x, overflow_policy=0x%x, rounding_policy=0x%x, outputs=%p",
        graph, inputs, weights, biases, overflow_policy, rounding_policy, outputs);
    gcmDUMP_API("$VX vxFullyConnectedLayer: graph=%p, inputs=%p, weights=%p, biases=0x%x, overflow_policy=0x%x, rounding_policy=0x%x, outputs=%p",
        graph, inputs, weights, biases, overflow_policy, rounding_policy, outputs);

    vxmCHECK_PRECISION(biases, VX_TENSOR_PRECISION_HIGH);

    context = vxGetContext((vx_reference)graph);

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)overflow_policy_s), "%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)overflow_policy_s;
    }

    rounding_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)rounding_policy_s), "%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)rounding_policy_s;
    }

    parameters[3]  = (vx_reference)overflow_policy_s;
    parameters[4]  = (vx_reference)rounding_policy_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_FULLY_CONNECTED_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&overflow_policy_s);
    vxReleaseScalar(&rounding_policy_s);
    gcmFOOTER_NO();
    return node;
}

/* vxFullyConnectedLayer_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_node VX_API_CALL vxFullyConnectedLayer_11(
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
    gcmHEADER_ARG("graph=%p, inputs=%p, weights=%p, biases=0x%x, pad=0x%x, accumulator_bits=0x%x, overflow_policy=0x%x, rounding_policy=0x%x, down_scale_size_rounding=0x%x, outputs=%p",
        graph, inputs, weights, biases, pad, accumulator_bits, overflow_policy, rounding_policy, down_scale_size_rounding, outputs);
    gcmDUMP_API("$VX vxFullyConnectedLayer_11: graph=%p, inputs=%p, weights=%p, biases=0x%x, pad=0x%x, accumulator_bits=0x%x, overflow_policy=0x%x, rounding_policy=0x%x, down_scale_size_rounding=0x%x, outputs=%p",
        graph, inputs, weights, biases, pad, accumulator_bits, overflow_policy, rounding_policy, down_scale_size_rounding, outputs);

    vxmCHECK_PRECISION(biases, VX_TENSOR_PRECISION_HIGH);

    context = vxGetContext((vx_reference)graph);

    pad_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad);
    if (vxoReference_GetStatus((vx_reference)pad_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pad_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pad_s), "%s[%d]: Get pad_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pad_s;
    }

    accumulator_bits_s = vxCreateScalar(context, VX_TYPE_UINT32, &accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)accumulator_bits_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get accumulator_bits_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)accumulator_bits_s), "%s[%d]: Get accumulator_bits_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)accumulator_bits_s;
    }

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)overflow_policy_s), "%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)overflow_policy_s;
    }

    rounding_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)rounding_policy_s), "%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)rounding_policy_s;
    }

    down_scale_size_rounding_s = vxCreateScalar(context, VX_TYPE_UINT32, &down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get down_scale_size_rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s), "%s[%d]: Get down_scale_size_rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)down_scale_size_rounding_s;
    }

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
    gcmFOOTER_NO();
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

    gcmHEADER_ARG("graph=%p, inputs=%p, weights_biases=%p, pad_x=0x%x, pad_y=0x%x, accumulator_bits=0x%x, overflow_policy=0x%x, rounding_policy=0x%x,"\
        " down_scale_size_rounding=0x%x, enable_relu=0x%x, pool_type=0x%x, pool_size_x=0x%x, pool_size_y=0x%x, outputs=%p", graph, inputs, weights_biases, pad_x, pad_y, accumulator_bits,
        overflow_policy, rounding_policy, down_scale_size_rounding, enable_relu, pool_type, pool_size_x, pool_size_y, outputs);
    gcmDUMP_API("$VX vxConvolutionReluPoolingLayer: graph=%p, inputs=%p, weights_biases=%p, pad_x=0x%x, pad_y=0x%x, accumulator_bits=0x%x, overflow_policy=0x%x, rounding_policy=0x%x,"\
        " down_scale_size_rounding=0x%x, enable_relu=0x%x, pool_type=0x%x, pool_size_x=0x%x, pool_size_y=0x%x, outputs=%p", graph, inputs, weights_biases, pad_x, pad_y, accumulator_bits,
        overflow_policy, rounding_policy, down_scale_size_rounding, enable_relu, pool_type, pool_size_x, pool_size_y, outputs);

    context = vxGetContext((vx_reference)graph);

    pad_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_x);
    if (vxoReference_GetStatus((vx_reference)pad_x_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pad_x_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pad_x_s), "%s[%d]: Get pad_x_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pad_x_s;
    }

    pad_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_y);
    if (vxoReference_GetStatus((vx_reference)pad_y_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pad_y_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pad_y_s), "%s[%d]: Get pad_y_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pad_y_s;
    }

    accumulator_bits_s = vxCreateScalar(context, VX_TYPE_UINT32, &accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)accumulator_bits_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get accumulator_bits_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)accumulator_bits_s), "%s[%d]: Get accumulator_bits_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)accumulator_bits_s;
    }

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)overflow_policy_s), "%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)overflow_policy_s;
    }

    rounding_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)rounding_policy_s), "%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)rounding_policy_s;
    }

    down_scale_size_rounding_s = vxCreateScalar(context, VX_TYPE_UINT32, &down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get down_scale_size_rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s), "%s[%d]: Get down_scale_size_rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)down_scale_size_rounding_s;
    }

    enable_relu_s = vxCreateScalar(context, VX_TYPE_UINT32, &enable_relu);
    if (vxoReference_GetStatus((vx_reference)enable_relu_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get enable_relu_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)enable_relu_s), "%s[%d]: Get enable_relu_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)enable_relu_s;
    }

    pool_type_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_type);
    if (vxoReference_GetStatus((vx_reference)pool_type_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_type_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_type_s), "%s[%d]: Get pool_type_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_type_s;
    }

    pool_size_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_x);
    if (vxoReference_GetStatus((vx_reference)pool_size_x_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_size_x_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_size_x_s), "%s[%d]: Get pool_size_x_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_size_x_s;
    }

    pool_size_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_y);
    if (vxoReference_GetStatus((vx_reference)pool_size_y_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_size_y_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_size_y_s), "%s[%d]: Get pool_size_y_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_size_y_s;
    }

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
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, inputs=%p, weights_biases=%p, pad_x=0x%x, pad_y=0x%x, accumulator_bits=0x%x, overflow_policy=0x%x, rounding_policy=0x%x,"\
        " down_scale_size_rounding=0x%x, enable_relu=0x%x, outputs=%p", graph, inputs, weights_biases, pad_x, pad_y, accumulator_bits, overflow_policy, rounding_policy,
        down_scale_size_rounding, enable_relu, outputs);
    gcmDUMP_API("$VX vxConvolutionReluLayer: graph=%p, inputs=%p, weights_biases=%p, pad_x=0x%x, pad_y=0x%x, accumulator_bits=0x%x, overflow_policy=0x%x, rounding_policy=0x%x,"\
        " down_scale_size_rounding=0x%x, enable_relu=0x%x, outputs=%p", graph, inputs, weights_biases, pad_x, pad_y, accumulator_bits, overflow_policy, rounding_policy,
        down_scale_size_rounding, enable_relu, outputs);

    context = vxGetContext((vx_reference)graph);

    pad_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_x);
    if (vxoReference_GetStatus((vx_reference)pad_x_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pad_x_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pad_x_s), "%s[%d]: Get pad_x_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pad_x_s;
    }

    pad_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_y);
    if (vxoReference_GetStatus((vx_reference)pad_y_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pad_y_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pad_y_s), "%s[%d]: Get pad_y_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pad_y_s;
    }

    accumulator_bits_s = vxCreateScalar(context, VX_TYPE_UINT32, &accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)accumulator_bits_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get accumulator_bits_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)accumulator_bits_s), "%s[%d]: Get accumulator_bits_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)accumulator_bits_s;
    }

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)overflow_policy_s), "%s[%d]: Get overflow_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)overflow_policy_s;
    }

    rounding_policy_s = vxCreateScalar(context, VX_TYPE_UINT32, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_policy_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)rounding_policy_s), "%s[%d]: Get rounding_policy_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)rounding_policy_s;
    }

    down_scale_size_rounding_s = vxCreateScalar(context, VX_TYPE_UINT32, &down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get down_scale_size_rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)down_scale_size_rounding_s), "%s[%d]: Get down_scale_size_rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)down_scale_size_rounding_s;
    }

    enable_relu_s = vxCreateScalar(context, VX_TYPE_UINT32, &enable_relu);
    if (vxoReference_GetStatus((vx_reference)enable_relu_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get enable_relu_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)enable_relu_s), "%s[%d]: Get enable_relu_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)enable_relu_s;
    }

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
    gcmFOOTER_NO();
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
    vx_uint32 i = 0;
    vx_context context;
    vx_node    node;
    vx_uint32 stride_x = 1;
    vx_uint32 stride_y = 1;
    vx_int32 depth_mul = 0;
    vx_uint32 mergedNodeCount = 0;
    vx_int32 interZeroPoint[MERGED_NODE_COUNT_MAX] = {0};
    vx_float32 interScale[MERGED_NODE_COUNT_MAX] = {0.f};
    vx_enum interDataType[MERGED_NODE_COUNT_MAX] = {VX_TYPE_INVALID};
    vx_enum src_rank_mode = VX_TENSOR_RANK_WHCN;
    vx_enum convert_dst_format = VX_TYPE_CHAR;
    vx_int32 pad_const = TENSOR_TF_ZEROPOINT(inputs);

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

    gcmHEADER_ARG("graph=%p, inputs=%p, weights_biases=%p, convolution_relu_pooling_params=%p, size_of_convolution_relu_pooling_params=0x%lx, outputs=%p",
        graph, inputs, weights_biases, convolution_relu_pooling_params, size_of_convolution_relu_pooling_params, outputs);
    gcmDUMP_API("$VX vxConvolutionReluPoolingLayer2: graph=%p, inputs=%p, weights_biases=%p, convolution_relu_pooling_params=%p, size_of_convolution_relu_pooling_params=0x%lx, outputs=%p",
        graph, inputs, weights_biases, convolution_relu_pooling_params, size_of_convolution_relu_pooling_params, outputs);

    if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext_t))
    {
        vx_nn_convolution_relu_pooling_params_ext_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.stride_x;
        stride_y = conv_ext.stride_y;
    }
    else if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext2_t))
    {
        vx_nn_convolution_relu_pooling_params_ext2_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext2_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.ext.stride_x;
        stride_y = conv_ext.ext.stride_y;
        depth_mul =  conv_ext.depth_multiplier;
        src_rank_mode = conv_ext.src_rank_mode;
        convert_dst_format = conv_ext.convert_dst_format;
    }
    else if (size_of_convolution_relu_pooling_params == sizeof(vx_nn_convolution_relu_pooling_params_ext3_t))
    {
        vx_nn_convolution_relu_pooling_params_ext3_t conv_ext = *((vx_nn_convolution_relu_pooling_params_ext3_t*)(convolution_relu_pooling_params));
        stride_x = conv_ext.ext2.ext.stride_x;
        stride_y = conv_ext.ext2.ext.stride_y;
        depth_mul =  conv_ext.ext2.depth_multiplier;
        src_rank_mode = conv_ext.ext2.src_rank_mode;
        convert_dst_format = conv_ext.ext2.convert_dst_format;
        mergedNodeCount = conv_ext.mergedNodeCount;
        if(mergedNodeCount > 0)
        {
            gcoOS_MemCopy(interScale, conv_ext.interScale, sizeof(vx_float32) * mergedNodeCount);
            gcoOS_MemCopy(interZeroPoint, conv_ext.interZeroPoint, sizeof(vx_int32) * mergedNodeCount);
            gcoOS_MemCopy(interDataType, conv_ext.interDataType, sizeof(vx_enum) * mergedNodeCount);
            vxmASSERT(mergedNodeCount <= MERGED_NODE_COUNT_MAX);
        }
    }
    else if (size_of_convolution_relu_pooling_params != sizeof(vx_nn_convolution_relu_pooling_params_t))
    {
        vxError("Invalid parameter convolution_relu_pooling_params");
        gcmFOOTER_NO();
        return NULL;
    }

    context = vxGetContext((vx_reference)graph);

    parameters[2] = (vx_reference)vxCreateScalar(context, VX_TYPE_SIZE, &convolution_relu_pooling_params->dilation_x);
    if (vxoReference_GetStatus((vx_reference)parameters[2]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[2] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[2]), "%s[%d]: Get parameters[2] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[2];
    }

    parameters[3] = (vx_reference)vxCreateScalar(context, VX_TYPE_SIZE, &convolution_relu_pooling_params->dilation_y);
    if (vxoReference_GetStatus((vx_reference)parameters[3]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[3] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[3]), "%s[%d]: Get parameters[3] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[3];
    }

    parameters[4] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pad_x_left);
    if (vxoReference_GetStatus((vx_reference)parameters[4]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[4] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[4]), "%s[%d]: Get parameters[4] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[4];
    }

    parameters[5] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pad_x_right);
    if (vxoReference_GetStatus((vx_reference)parameters[5]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[5] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[5]), "%s[%d]: Get parameters[5] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[5];
    }

    parameters[6] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pad_y_top);
    if (vxoReference_GetStatus((vx_reference)parameters[6]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[6] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[6]), "%s[%d]: Get parameters[6] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[6];
    }

    parameters[7] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pad_y_bottom);
    if (vxoReference_GetStatus((vx_reference)parameters[7]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[7] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[7]), "%s[%d]: Get parameters[7] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[7];
    }

    parameters[8] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT8, &convolution_relu_pooling_params->accumulator_bits);
    if (vxoReference_GetStatus((vx_reference)parameters[8]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[8] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[8]), "%s[%d]: Get parameters[8] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[8];
    }

    parameters[9] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->overflow_policy);
    if (vxoReference_GetStatus((vx_reference)parameters[9]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[9] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[9]), "%s[%d]: Get parameters[9] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[9];
    }

    parameters[10] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->rounding_policy);
    if (vxoReference_GetStatus((vx_reference)parameters[10]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[10] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[10]), "%s[%d]: Get parameters[10] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[10];
    }

    parameters[11] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)parameters[11]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[11] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[11]), "%s[%d]: Get parameters[11] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[11];
    }

    parameters[12] = (vx_reference)vxCreateScalar(context, VX_TYPE_BOOL, &convolution_relu_pooling_params->enable_relu);
    if (vxoReference_GetStatus((vx_reference)parameters[12]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[12] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[12]), "%s[%d]: Get parameters[12] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[12];
    }

    parameters[13] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->pool_type);
    if (vxoReference_GetStatus((vx_reference)parameters[13]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[13] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[13]), "%s[%d]: Get parameters[13] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[13];
    }

    parameters[14] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pool_size_x);
    if (vxoReference_GetStatus((vx_reference)parameters[14]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[14] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[14]), "%s[%d]: Get parameters[14] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[14];
    }

    parameters[15] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &convolution_relu_pooling_params->pool_size_y);
    if (vxoReference_GetStatus((vx_reference)parameters[15]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[15] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[15]), "%s[%d]: Get parameters[15] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[15];
    }

    parameters[16] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convolution_relu_pooling_params->pad_mode);
    if (vxoReference_GetStatus((vx_reference)parameters[16]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[16] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[16]), "%s[%d]: Get parameters[16] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[16];
    }

    parameters[17] = convolution_relu_pooling_params->pad_const?(vx_reference)convolution_relu_pooling_params->pad_const: (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &pad_const);

    parameters[18] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &stride_x);
    if (vxoReference_GetStatus((vx_reference)parameters[18]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[18] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[18]), "%s[%d]: Get parameters[18] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[18];
    }

    parameters[19] = (vx_reference)vxCreateScalar(context, VX_TYPE_UINT32, &stride_y);
    if (vxoReference_GetStatus((vx_reference)parameters[19]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[19] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[19]), "%s[%d]: Get parameters[19] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[19];
    }

    parameters[20] = (vx_reference)vxCreateScalar(context, VX_TYPE_INT32, &depth_mul);
    if (vxoReference_GetStatus((vx_reference)parameters[20]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[20] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[20]), "%s[%d]: Get parameters[20] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[20];
    }

    parameters[21] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &src_rank_mode);
    if (vxoReference_GetStatus((vx_reference)parameters[21]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[21] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[21]), "%s[%d]: Get parameters[21] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[21];
    }

    parameters[22] = (vx_reference)vxCreateScalar(context, VX_TYPE_ENUM, &convert_dst_format);
    if (vxoReference_GetStatus((vx_reference)parameters[22]) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get parameters[22] reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)parameters[22]), "%s[%d]: Get parameters[22] reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)parameters[22];
    }
    if(mergedNodeCount > 0)
    {
        vx_array mergedScale = vxCreateArray(context, VX_TYPE_FLOAT32, mergedNodeCount);/*Scale*/
        vx_array mergedZeroPoint = vxCreateArray(context, VX_TYPE_INT32, mergedNodeCount);/*ZeroPoint*/
        vx_array mergedDataType = vxCreateArray(context, VX_TYPE_ENUM, mergedNodeCount);/*ZeroPoint*/

        if (!vxoArray_AllocateMemory(mergedScale))
        {
            gcmFOOTER_NO();
            return VX_NULL;
        }
        else
        {
            memcpy(mergedScale->memory.logicals[0], interScale, sizeof(vx_float32) * mergedNodeCount);
            parameters[23]  = (vx_reference)mergedScale;
        }

        if (!vxoArray_AllocateMemory(mergedZeroPoint))
        {
            gcmFOOTER_NO();
            return VX_NULL;
        }
        else
        {
            memcpy(mergedZeroPoint->memory.logicals[0], interZeroPoint, sizeof(vx_int32) * mergedNodeCount);
            parameters[24]  = (vx_reference)mergedZeroPoint;
        }
        if (!vxoArray_AllocateMemory(mergedDataType))
        {
            gcmFOOTER_NO();
            return VX_NULL;
        }
        else
        {
            memcpy(mergedDataType->memory.logicals[0], interDataType, sizeof(vx_enum) * mergedNodeCount);
            parameters[25]  = (vx_reference)mergedDataType;
        }
    }

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONVOLUTION_RELU_POOLING_LAYER2, parameters, vxmLENGTH_OF(parameters));

    for (i = 2; i < (gcmCOUNTOF(parameters) - 1); i ++)
    {
        if (i == 17)
        {
            if (convolution_relu_pooling_params->pad_const == VX_NULL)
            {
                vxReleaseScalar((vx_scalar*)&parameters[17]);
            }
            continue;
        }

        vxReleaseScalar((vx_scalar*)&parameters[i]);
    }
    gcmFOOTER_NO();
    return node;
}

/* vxPoolingLayer_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_node VX_API_CALL vxPoolingLayer_11(vx_graph graph,
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
    gcmHEADER_ARG("graph=%p, inputs=%p, pool_type=0x%x, pool_size_x=0x%x, pool_size_y=0x%x, pool_pad_x=0x%x, pool_pad_y=0x%x, rounding=0x%x, outputs=%p",
        graph, inputs, pool_type, pool_size_x, pool_size_y, pool_pad_x, pool_pad_y, rounding, outputs);
    gcmDUMP_API("$VX vxPoolingLayer_11: graph=%p, inputs=%p, pool_type=0x%x, pool_size_x=0x%x, pool_size_y=0x%x, pool_pad_x=0x%x, pool_pad_y=0x%x, rounding=0x%x, outputs=%p",
        graph, inputs, pool_type, pool_size_x, pool_size_y, pool_pad_x, pool_pad_y, rounding, outputs);

    context = vxGetContext((vx_reference)graph);

    pool_type_s = vxCreateScalar(context, VX_TYPE_ENUM, &pool_type);
    if (vxoReference_GetStatus((vx_reference)pool_type_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_type_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_type_s), "%s[%d]: Get pool_type_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_type_s;
    }

    pool_size_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_x);
    if (vxoReference_GetStatus((vx_reference)pool_size_x_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_size_x_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_size_x_s), "%s[%d]: Get pool_size_x_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_size_x_s;
    }

    pool_size_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_y);
    if (vxoReference_GetStatus((vx_reference)pool_size_y_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_size_y_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_size_y_s), "%s[%d]: Get pool_size_y_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_size_y_s;
    }

    pool_pad_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_pad_x);
    if (vxoReference_GetStatus((vx_reference)pool_pad_x_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_pad_x_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_pad_x_s), "%s[%d]: Get pool_pad_x_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_pad_x_s;
    }

    pool_pad_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_pad_y);
    if (vxoReference_GetStatus((vx_reference)pool_pad_y_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_pad_y_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_pad_y_s), "%s[%d]: Get pool_pad_y_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_pad_y_s;
    }

    rounding_s = vxCreateScalar(context, VX_TYPE_ENUM, &rounding);
    if (vxoReference_GetStatus((vx_reference)rounding_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)rounding_s), "%s[%d]: Get rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)rounding_s;
    }

    parameters[1]  = (vx_reference)pool_type_s;
    parameters[2]  = (vx_reference)pool_size_x_s;
    parameters[3]  = (vx_reference)pool_size_y_s;
    parameters[4]  = (vx_reference)pool_pad_x_s;
    parameters[5]  = (vx_reference)pool_pad_y_s;
    parameters[6]  = (vx_reference)rounding_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_POOLING_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&pool_type_s);
    vxReleaseScalar(&pool_size_x_s);
    vxReleaseScalar(&pool_size_y_s);
    vxReleaseScalar(&pool_pad_x_s);
    vxReleaseScalar(&pool_pad_y_s);
    vxReleaseScalar(&rounding_s);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxPoolingLayer(
    vx_graph graph,
    vx_tensor inputs,
    vx_enum pooling_type,
    vx_size pooling_size_x,
    vx_size pooling_size_y,
    vx_size pooling_padding_x,
    vx_size pooling_padding_y,
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

    vx_uint32 pool_size_x;
    vx_uint32 pool_size_y;
    vx_uint32 pool_pad_x;
    vx_uint32 pool_pad_y;

    gcmHEADER_ARG("graph=%p, inputs=%p, pooling_type=0x%x, pooling_size_x=0x%x, pooling_size_y=0x%x, pooling_padding_x=0x%x, pooling_padding_y=0x%x, rounding=0x%x, outputs=%p",
        graph, inputs, pooling_type, pooling_size_x, pooling_size_y, pooling_padding_x, pooling_padding_y, rounding, outputs);
    gcmDUMP_API("$VX vxPoolingLayer: graph=%p, inputs=%p, pooling_type=0x%x, pooling_size_x=0x%x, pooling_size_y=0x%x, pooling_padding_x=0x%x, pooling_padding_y=0x%x, rounding=0x%x, outputs=%p",
        graph, inputs, pooling_type, pooling_size_x, pooling_size_y, pooling_padding_x, pooling_padding_y, rounding, outputs);

    gcmSAFECASTSIZET(pool_size_x, pooling_size_x);
    gcmSAFECASTSIZET(pool_size_y, pooling_size_y);
    gcmSAFECASTSIZET(pool_pad_x, pooling_padding_x);
    gcmSAFECASTSIZET(pool_pad_y, pooling_padding_y);

    context = vxGetContext((vx_reference)graph);

    pool_type_s = vxCreateScalar(context, VX_TYPE_ENUM, &pooling_type);
    if (vxoReference_GetStatus((vx_reference)pool_type_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_type_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_type_s), "%s[%d]: Get pool_type_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_type_s;
    }

    pool_size_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_x);
    if (vxoReference_GetStatus((vx_reference)pool_size_x_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_size_x_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_size_x_s), "%s[%d]: Get pool_size_x_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_size_x_s;
    }

    pool_size_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_size_y);
    if (vxoReference_GetStatus((vx_reference)pool_size_y_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_size_y_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_size_y_s), "%s[%d]: Get pool_size_y_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_size_y_s;
    }

    pool_pad_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_pad_x);
    if (vxoReference_GetStatus((vx_reference)pool_pad_x_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_pad_x_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_pad_x_s), "%s[%d]: Get pool_pad_x_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_pad_x_s;
    }

    pool_pad_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &pool_pad_y);
    if (vxoReference_GetStatus((vx_reference)pool_pad_y_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_pad_y_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_pad_y_s), "%s[%d]: Get pool_pad_y_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_pad_y_s;
    }

    rounding_s = vxCreateScalar(context, VX_TYPE_ENUM, &rounding);
    if (vxoReference_GetStatus((vx_reference)rounding_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)rounding_s), "%s[%d]: Get rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)rounding_s;
    }

    parameters[1]  = (vx_reference)pool_type_s;
    parameters[2]  = (vx_reference)pool_size_x_s;
    parameters[3]  = (vx_reference)pool_size_y_s;
    parameters[4]  = (vx_reference)pool_pad_x_s;
    parameters[5]  = (vx_reference)pool_pad_y_s;
    parameters[6]  = (vx_reference)rounding_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_POOLING_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&pool_type_s);
    vxReleaseScalar(&pool_size_x_s);
    vxReleaseScalar(&pool_size_y_s);
    vxReleaseScalar(&pool_pad_x_s);
    vxReleaseScalar(&pool_pad_y_s);
    vxReleaseScalar(&rounding_s);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxPoolingLayer2(vx_graph graph,
                                                vx_tensor inputs,
                                                const vx_nn_pooling_params_t * pooling_params,
                                                vx_size                     size_of_pooling_params,
                                                vx_tensor outputs)
{
    vx_context context;
    vx_node    node;

    vx_scalar pool_type_s    = VX_NULL;
    vx_scalar pool_size_x_s  = VX_NULL;
    vx_scalar pool_size_y_s  = VX_NULL;
    vx_scalar pool_pad_x_left_s   = VX_NULL;
    vx_scalar pool_pad_x_right_s   = VX_NULL;
    vx_scalar pool_pad_y_top_s   = VX_NULL;
    vx_scalar pool_pad_y_bottom_s   = VX_NULL;
    vx_scalar rounding_s      = VX_NULL;
    vx_enum   type, rounding_policy;
    vx_uint32 size_x, size_y;
    vx_uint32 pad_x_left, pad_x_right, pad_y_top, pad_y_bottom;
    vx_uint32 stride_x, stride_y;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
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

    gcmHEADER_ARG("graph=%p, inputs=%p, pooling_params=%p, size_of_pooling_params=0x%lx, outputs=%p",
        graph, inputs, pooling_params, size_of_pooling_params, outputs);
    gcmDUMP_API("$VX vxPoolingLayer2: graph=%p, inputs=%p, pooling_params=%p, size_of_pooling_params=0x%lx, outputs=%p",
        graph, inputs, pooling_params, size_of_pooling_params, outputs);

    if (size_of_pooling_params == sizeof(vx_nn_pooling_params_t))
    {
        type = pooling_params->pool_type;
        size_x = pooling_params->pool_size_x;
        size_y = pooling_params->pool_size_y;
        pad_x_left = pooling_params->pool_pad_x_left;
        pad_x_right = pooling_params->pool_pad_x_right;
        pad_y_top = pooling_params->pool_pad_y_top;
        pad_y_bottom = pooling_params->pool_pad_y_bottom;
        rounding_policy = pooling_params->rounding;
    }
    else if (size_of_pooling_params == sizeof(vx_nn_pooling_params_ext_t))
    {
        vx_nn_pooling_params_ext_t *pooling_params_ext = (vx_nn_pooling_params_ext_t *)pooling_params;
        type = pooling_params_ext->base.pool_type;
        size_x = pooling_params_ext->base.pool_size_x;
        size_y = pooling_params_ext->base.pool_size_y;
        pad_x_left = pooling_params_ext->base.pool_pad_x_left;
        pad_x_right = pooling_params_ext->base.pool_pad_x_right;
        pad_y_top = pooling_params_ext->base.pool_pad_y_top;
        pad_y_bottom = pooling_params_ext->base.pool_pad_y_bottom;
        rounding_policy = pooling_params_ext->base.rounding;
        stride_x = pooling_params_ext->stride_x;
        stride_y = pooling_params_ext->stride_y;
    }
    else
    {
        vxError("Invalid parameter poolinglayer_params\n");
        gcmFOOTER_NO();
        return NULL;
    }

    context = vxGetContext((vx_reference)graph);

    pool_type_s = vxCreateScalar(context, VX_TYPE_ENUM, &type);
    if (vxoReference_GetStatus((vx_reference)pool_type_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_type_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_type_s), "%s[%d]: Get pool_type_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_type_s;
    }

    pool_size_x_s = vxCreateScalar(context, VX_TYPE_UINT32, &size_x);
    if (vxoReference_GetStatus((vx_reference)pool_size_x_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_size_x_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_size_x_s), "%s[%d]: Get pool_size_x_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_size_x_s;
    }

    pool_size_y_s = vxCreateScalar(context, VX_TYPE_UINT32, &size_y);
    if (vxoReference_GetStatus((vx_reference)pool_size_y_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_size_y_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_size_y_s), "%s[%d]: Get pool_size_y_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_size_y_s;
    }

    pool_pad_x_left_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_x_left);
    if (vxoReference_GetStatus((vx_reference)pool_pad_x_left_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_pad_x_left_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_pad_x_left_s), "%s[%d]: Get pool_pad_x_left_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_pad_x_left_s;
    }

    pool_pad_x_right_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_x_right);
    if (vxoReference_GetStatus((vx_reference)pool_pad_x_right_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_pad_x_right_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_pad_x_right_s), "%s[%d]: Get pool_pad_x_right_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_pad_x_right_s;
    }

    pool_pad_y_top_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_y_top);
    if (vxoReference_GetStatus((vx_reference)pool_pad_y_top_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_pad_y_top_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_pad_y_top_s), "%s[%d]: Get pool_pad_y_top_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_pad_y_top_s;
    }

    pool_pad_y_bottom_s = vxCreateScalar(context, VX_TYPE_UINT32, &pad_y_bottom);
    if (vxoReference_GetStatus((vx_reference)pool_pad_y_bottom_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get pool_pad_y_bottom_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)pool_pad_y_bottom_s), "%s[%d]: Get pool_pad_y_bottom_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)pool_pad_y_bottom_s;
    }

    rounding_s = vxCreateScalar(context, VX_TYPE_ENUM, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)rounding_s) != VX_SUCCESS)
    {
        vxError("%s[%d]: Get rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        vxAddLogEntry(&graph->base, vxoReference_GetStatus((vx_reference)rounding_s), "%s[%d]: Get rounding_s reference failed!\n", __FUNCTION__, __LINE__);
        gcmFOOTER_NO();
        return (vx_node)rounding_s;
    }

    parameters[1]  = (vx_reference)pool_type_s;
    parameters[2]  = (vx_reference)pool_size_x_s;
    parameters[3]  = (vx_reference)pool_size_y_s;
    parameters[4]  = (vx_reference)pool_pad_x_left_s;
    parameters[5]  = (vx_reference)pool_pad_x_right_s;
    parameters[6]  = (vx_reference)pool_pad_y_top_s;
    parameters[7]  = (vx_reference)pool_pad_y_bottom_s;
    parameters[8]  = (vx_reference)rounding_s;

    if (size_of_pooling_params == sizeof(vx_nn_pooling_params_ext_t))
    {
        parameters[9] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &stride_x);
        parameters[10] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &stride_y);
    }

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_POOLING_LAYER2, parameters, vxmLENGTH_OF(parameters));

    for (i = 1; i < (gcmCOUNTOF(parameters) - 1); i ++)
    {
        vxReleaseScalar((vx_scalar*)&parameters[i]);
    }
    gcmFOOTER_NO();
    return node;
}


VX_API_ENTRY vx_node VX_API_CALL vxSoftmaxLayer(vx_graph graph, vx_tensor inputs, vx_tensor outputs)
{
    vx_node    node;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    (vx_reference)outputs,
    };
    gcmHEADER_ARG("graph=%p, inputs=%p, outputs=%p", graph, inputs, outputs);
    gcmDUMP_API("$VX vxSoftmaxLayer: graph=%p, inputs=%p, outputs=%p", graph, inputs, outputs);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_SOFTMAX_LAYER, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

/* vxNormalizationLayer_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_node VX_API_CALL vxNormalizationLayer_11(vx_graph graph,
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
    gcmHEADER_ARG("graph=%p, inputs=%p, type=0x%x, norm_size=0x%x, alpha=%f, beta=%f, outputs=%p", graph, inputs, type, norm_size, alpha, beta, outputs);
    gcmDUMP_API("$VX vxNormalizationLayer_11: graph=%p, inputs=%p, type=0x%x, norm_size=0x%x, alpha=%f, beta=%f, outputs=%p", graph, inputs, type, norm_size, alpha, beta, outputs);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NORMALIZATION_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&type_s);
    vxReleaseScalar(&norm_size_s);
    vxReleaseScalar(&alpha_s);
    vxReleaseScalar(&beta_s);
    gcmFOOTER_NO();
    return node;

}

VX_API_ENTRY vx_node VX_API_CALL vxNormalizationLayer(
    vx_graph graph,
    vx_tensor inputs,
    vx_enum type,
    vx_size normalization_size,
    vx_float32 alpha,
    vx_float32 beta,
    vx_tensor outputs)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_node    node;
    vx_uint32 norm_size = 0;
    vx_scalar type_s = vxCreateScalar(context, VX_TYPE_ENUM, &type);
    vx_scalar norm_size_s = VX_NULL;
    vx_scalar alpha_s = vxCreateScalar(context, VX_TYPE_INT32, &alpha);
    vx_scalar beta_s = vxCreateScalar(context, VX_TYPE_INT32, &beta);
    vx_reference    parameters[] = {
         (vx_reference)inputs,
         (vx_reference)type_s,
         VX_NULL,
         (vx_reference)alpha_s,
         (vx_reference)beta_s,
         (vx_reference)outputs
    };
    gcmHEADER_ARG("graph=%p, inputs=%p, type=0x%x, normalization_size=0x%lx, alpha=%f, beta=%f, outputs=%p", graph, inputs, type, normalization_size, alpha, beta, outputs);
    gcmDUMP_API("$VX vxNormalizationLayer: graph=%p, inputs=%p, type=0x%x, normalization_size=0x%lx, alpha=%f, beta=%f, outputs=%p", graph, inputs, type, normalization_size, alpha, beta, outputs);

    gcmSAFECASTSIZET(norm_size, normalization_size);
    norm_size_s = vxCreateScalar(context, VX_TYPE_INT32, &norm_size);
    parameters[2] = (vx_reference)norm_size_s;
    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NORMALIZATION_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&type_s);
    vxReleaseScalar(&norm_size_s);
    vxReleaseScalar(&alpha_s);
    vxReleaseScalar(&beta_s);
    gcmFOOTER_NO();
    return node;

}

VX_API_ENTRY vx_node VX_API_CALL vxLocalResponseNormalizationLayer(
    vx_graph graph,
    vx_tensor inputs,
    vx_enum type,
    vx_size normalization_size,
    vx_float32 alpha,
    vx_float32 beta,
    vx_float32 bias,
    vx_tensor outputs)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_node    node;
    vx_uint32 norm_size = 0;
    vx_scalar type_s = vxCreateScalar(context, VX_TYPE_ENUM, &type);
    vx_scalar norm_size_s = VX_NULL;
    vx_scalar alpha_s = vxCreateScalar(context, VX_TYPE_INT32, &alpha);
    vx_scalar beta_s = vxCreateScalar(context, VX_TYPE_INT32, &beta);
    vx_scalar bias_s = vxCreateScalar(context, VX_TYPE_INT32, &bias);
    vx_reference    parameters[] = {
         (vx_reference)inputs,
         (vx_reference)type_s,
         VX_NULL,
         (vx_reference)alpha_s,
         (vx_reference)beta_s,
         (vx_reference)bias_s,
         (vx_reference)outputs
    };
    gcmHEADER_ARG("graph=%p, inputs=%p, type=0x%x, normalization_size=0x%lx, alpha=%f, beta=%f, outputs=%p", graph, inputs, type, normalization_size, alpha, beta, outputs);
    gcmDUMP_API("$VX vxNormalizationLayer: graph=%p, inputs=%p, type=0x%x, normalization_size=0x%lx, alpha=%f, beta=%f, outputs=%p", graph, inputs, type, normalization_size, alpha, beta, outputs);

    gcmSAFECASTSIZET(norm_size, normalization_size);
    norm_size_s = vxCreateScalar(context, VX_TYPE_INT32, &norm_size);
    parameters[2] = (vx_reference)norm_size_s;
    node = vxoNode_CreateSpecific(graph, VX_KERNEL_LOCAL_RESPONSE_NORMALIZATION_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&type_s);
    vxReleaseScalar(&norm_size_s);
    vxReleaseScalar(&alpha_s);
    vxReleaseScalar(&beta_s);
    vxReleaseScalar(&bias_s);
    gcmFOOTER_NO();
    return node;

}

VX_API_ENTRY vx_node VX_API_CALL vxActivationLayer(
    vx_graph graph,
    vx_tensor inputs,
    vx_enum function,
    vx_float32 a,
    vx_float32 b,
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
    gcmHEADER_ARG("graph=%p, inputs=%p, function=0x%x, a=%f, b=%f, outputs=%p", graph, inputs, function, a, b, outputs);
    gcmDUMP_API("$VX vxActivationLayer: graph=%p, inputs=%p, function=0x%x, a=%f, b=%f, outputs=%p", graph, inputs, function, a, b, outputs);

    context = vxGetContext((vx_reference)graph);

    func_s = vxCreateScalar(context, VX_TYPE_ENUM, &function);
    if (vxoReference_GetStatus((vx_reference)func_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)func_s;
    }

    a_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &a);
    if (vxoReference_GetStatus((vx_reference)a_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)a_s;
    }
    b_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &b);
    if (vxoReference_GetStatus((vx_reference)b_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)b_s;
    }
    parameters[1]  = (vx_reference)func_s;
    parameters[2]  = (vx_reference)a_s;
    parameters[3]  = (vx_reference)b_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_ACTIVATION_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&func_s);
    vxReleaseScalar(&a_s);
    vxReleaseScalar(&b_s);
    gcmFOOTER_NO();
    return node;
}

/* vxActivationLayer_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_node VX_API_CALL vxActivationLayer_11(
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
    gcmHEADER_ARG("graph=%p, inputs=%p, func=0x%x, a=0x%x, b=0x%x, outputs=%p", graph, inputs, func, a, b, outputs);
    gcmDUMP_API("$VX vxActivationLayer_11: graph=%p, inputs=%p, func=0x%x, a=0x%x, b=0x%x, outputs=%p", graph, inputs, func, a, b, outputs);

    context = vxGetContext((vx_reference)graph);

    func_s = vxCreateScalar(context, VX_TYPE_ENUM, &func);
    if (vxoReference_GetStatus((vx_reference)func_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)func_s;
    }
    a_s = vxCreateScalar(context, VX_TYPE_INT32, &a);
    if (vxoReference_GetStatus((vx_reference)a_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)a_s;
    }
    b_s = vxCreateScalar(context, VX_TYPE_INT32, &b);
    if (vxoReference_GetStatus((vx_reference)b_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)b_s;
    }
    parameters[1]  = (vx_reference)func_s;
    parameters[2]  = (vx_reference)a_s;
    parameters[3]  = (vx_reference)b_s;


    node = vxoNode_CreateSpecific(graph, VX_KERNEL_ACTIVATION_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&func_s);
    vxReleaseScalar(&a_s);
    vxReleaseScalar(&b_s);
    gcmFOOTER_NO();
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
    vx_scalar padXRightScalar              = VX_NULL;
    vx_scalar padYBottomScalar             = VX_NULL;
    vx_scalar padModeScalar                = VX_NULL;
    vx_scalar padValueScalar               = VX_NULL;
    vx_scalar dilationXScalar              = VX_NULL;
    vx_scalar dilationYScalar              = VX_NULL;
    vx_scalar strideXScalar                = VX_NULL;
    vx_scalar strideYScalar                = VX_NULL;
    vx_scalar depth_multiplier             = VX_NULL;
    vx_scalar downScaleSizeRoundingScalar  = VX_NULL;
    vx_scalar overflowPolicyScalar         = VX_NULL;
    vx_scalar roundingPolicyScalar         = VX_NULL;

    vx_int32 pad_x_right = (vx_int32)convolution_params->padding_x, pad_y_bottom = (vx_int32)convolution_params->padding_y;
    vx_uint32 padMode = VX_PAD_CONSTANT;
    vx_uint32 padConst = 0;
    vx_bool isNeedRelease = vx_false_e;
    vx_int32 stride_x = 1, stride_y = 1, depth_mul = 0;

    vx_reference    parameters[] = {
    (vx_reference)inputs,
    (vx_reference)weights,
    (vx_reference)biases,
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
    gcmHEADER_ARG("graph=%p, inputs=%p, weights=%p, biases=%p, convolution_params=%p, size_of_convolution_params=0x%lx, outputs=%p",
        graph, inputs, weights, biases, convolution_params, size_of_convolution_params, outputs);
    gcmDUMP_API("$VX vxConvolutionLayer: graph=%p, inputs=%p, weights=%p, biases=%p, convolution_params=%p, size_of_convolution_params=0x%lx, outputs=%p",
        graph, inputs, weights, biases, convolution_params, size_of_convolution_params, outputs);

    vxmCHECK_PRECISION(biases, VX_TENSOR_PRECISION_HIGH);

    context = vxGetContext((vx_reference)graph);

    padXScalar = vxCreateScalar(context, VX_TYPE_INT32, &convolution_params->padding_x);
    if (vxoReference_GetStatus((vx_reference)padXScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)padXScalar;
    }
    padYScalar = vxCreateScalar(context, VX_TYPE_INT32, &convolution_params->padding_y);
    if (vxoReference_GetStatus((vx_reference)padYScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)padYScalar;
    }
    dilationXScalar = vxCreateScalar(context, VX_TYPE_INT32, &convolution_params->dilation_x);
    if (vxoReference_GetStatus((vx_reference)dilationXScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)dilationXScalar;
    }
    dilationYScalar = vxCreateScalar(context, VX_TYPE_INT32, &convolution_params->dilation_y);
    if (vxoReference_GetStatus((vx_reference)dilationYScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)dilationYScalar;
    }
    if (size_of_convolution_params == sizeof(vx_nn_convolution_params_ext2_t))
    {
        vx_nn_convolution_params_ext2_t* param = (vx_nn_convolution_params_ext2_t*)convolution_params;

        pad_x_right = (vx_int32)param->ext.padding_x_right;
        pad_y_bottom = (vx_int32)param->ext.padding_y_bottom;
        stride_x = param->stride_x;
        stride_y = param->stride_y;
        depth_mul =  param->depth_multiplier;
    }
    else if (size_of_convolution_params == sizeof(vx_nn_convolution_params_ext_t))
    {
        vx_nn_convolution_params_ext_t* param = (vx_nn_convolution_params_ext_t*)convolution_params;

        pad_x_right = (vx_int32)param->padding_x_right;
        pad_y_bottom = (vx_int32)param->padding_y_bottom;

        padMode = (vx_uint32)param->pad_mode;
        padValueScalar = param->pad_const;
    }

    if (padValueScalar == VX_NULL)
    {
        padValueScalar = vxCreateScalar(context, VX_TYPE_UINT32, &padConst);
        isNeedRelease = vx_true_e;
    }

    padXRightScalar = vxCreateScalar(context, VX_TYPE_INT32, &pad_x_right);
    if (vxoReference_GetStatus((vx_reference)padXRightScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)padXRightScalar;
    }
    padYBottomScalar = vxCreateScalar(context, VX_TYPE_INT32, &pad_y_bottom);
    if (vxoReference_GetStatus((vx_reference)padYBottomScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)padYBottomScalar;
    }
    padModeScalar = vxCreateScalar(context, VX_TYPE_ENUM, &padMode);
    if (vxoReference_GetStatus((vx_reference)padModeScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)padModeScalar;
    }
    strideXScalar = vxCreateScalar(context, VX_TYPE_INT32, &stride_x);
    if (vxoReference_GetStatus((vx_reference)strideXScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)strideXScalar;
    }
    strideYScalar = vxCreateScalar(context, VX_TYPE_INT32, &stride_y);
    if (vxoReference_GetStatus((vx_reference)strideYScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)strideYScalar;
    }
    depth_multiplier = vxCreateScalar(context, VX_TYPE_INT32, &depth_mul);
    if (vxoReference_GetStatus((vx_reference)depth_multiplier) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)depth_multiplier;
    }
    downScaleSizeRoundingScalar = vxCreateScalar(context, VX_TYPE_INT32, &convolution_params->down_scale_size_rounding);
    if (vxoReference_GetStatus((vx_reference)downScaleSizeRoundingScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)downScaleSizeRoundingScalar;
    }
    overflowPolicyScalar = vxCreateScalar(context, VX_TYPE_UINT32, &convolution_params->overflow_policy);
    if (vxoReference_GetStatus((vx_reference)overflowPolicyScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)overflowPolicyScalar;
    }
    roundingPolicyScalar = vxCreateScalar(context, VX_TYPE_UINT32, &convolution_params->rounding_policy);
    if (vxoReference_GetStatus((vx_reference)roundingPolicyScalar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)roundingPolicyScalar;
    }
    parameters[3]  = (vx_reference)padXScalar;
    parameters[4]  = (vx_reference)padXRightScalar;
    parameters[5]  = (vx_reference)padYScalar;
    parameters[6]  = (vx_reference)padYBottomScalar;
    parameters[7]  = (vx_reference)padModeScalar;
    parameters[8]  = (vx_reference)padValueScalar;
    parameters[9]  = (vx_reference)dilationXScalar;
    parameters[10]  = (vx_reference)dilationYScalar;
    parameters[11]  = (vx_reference)strideXScalar;
    parameters[12]  = (vx_reference)strideYScalar;
    parameters[13]  = (vx_reference)depth_multiplier;
    parameters[14]  = (vx_reference)downScaleSizeRoundingScalar;
    parameters[15]  = (vx_reference)overflowPolicyScalar;
    parameters[16]  = (vx_reference)roundingPolicyScalar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_CONVOLUTION_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&padXScalar);
    vxReleaseScalar(&padXRightScalar);
    vxReleaseScalar(&padYScalar);
    vxReleaseScalar(&padYBottomScalar);
    vxReleaseScalar(&padModeScalar);
    vxReleaseScalar(&dilationXScalar);
    vxReleaseScalar(&dilationYScalar);
    vxReleaseScalar(&strideXScalar);
    vxReleaseScalar(&strideYScalar);
    vxReleaseScalar(&depth_multiplier);
    vxReleaseScalar(&downScaleSizeRoundingScalar);
    vxReleaseScalar(&overflowPolicyScalar);
    vxReleaseScalar(&roundingPolicyScalar);
    if (isNeedRelease)
    {
        vxReleaseScalar(&padValueScalar);
    }
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, in0=%p, in1=%p, out=%p", graph, in0, in1, out);
    gcmDUMP_API("$VX vxConcat2Layer: graph=%p, in0=%p, in1=%p, out=%p", graph, in0, in1, out);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONCAT2_LAYER, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConcatIndefiniteLayer(
    vx_graph graph,
    vx_object_array in,
    const vx_nn_concat_params_t* concat_params,
    vx_size size_of_concat_params,
    vx_tensor out
    )
{
    vx_node    node = VX_NULL;

    vx_reference    parameters[] = {
    (vx_reference)in,
    NULL,
    (vx_reference)out
    };

    vx_scalar axisSclar = VX_NULL;

    gcmHEADER_ARG("graph=%p, in=%p, concat_params=%p, size_of_concat_params=0x%lx, out=%p", graph, in, concat_params, size_of_concat_params, out);
    gcmDUMP_API("$VX vxConcatIndefiniteLayer: graph=%p, in=%p, concat_params=%p, size_of_concat_params=0x%lx, out=%p", graph, in, concat_params, size_of_concat_params, out);

    axisSclar = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &concat_params->axis);
    if (vxoReference_GetStatus((vx_reference)axisSclar) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)axisSclar;
    }
    parameters[1]  = (vx_reference)axisSclar;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONCATINDEFINITE_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&axisSclar);
    gcmFOOTER_NO();
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

    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, policy=0x%x, out=%p", graph, in1, in2, policy, out);
    gcmDUMP_API("$VX vxTensorAddNode: graph=%p, in1=%p, in2=%p, policy=0x%x, out=%p", graph, in1, in2, policy, out);

    context = vxGetContext((vx_reference)graph);

    policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &policy);
    if (vxoReference_GetStatus((vx_reference)policy_s) != VX_SUCCESS) {
        gcmFOOTER_NO();
        return (vx_node)policy_s;
    }
    parameters[2]  = (vx_reference)policy_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_ADD, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&policy_s);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, scale=%p, overflow_policy=0x%x, rounding_policy=0x%x, out=%p", graph, in1, in2, scale, overflow_policy, rounding_policy, out);
    gcmDUMP_API("$VX vxTensorMultiplyNode: graph=%p, in1=%p, in2=%p, scale=%p, overflow_policy=0x%x, rounding_policy=0x%x, out=%p", graph, in1, in2, scale, overflow_policy, rounding_policy, out);

    context = vxGetContext((vx_reference)graph);

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &overflow_policy);
    rounding_policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS || vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return NULL;
    }
    parameters[3]  = (vx_reference)overflow_policy_s;
    parameters[4]  = (vx_reference)rounding_policy_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_MULTIPLY, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&overflow_policy_s);
    vxReleaseScalar(&rounding_policy_s);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, scale=%p, overflow_policy=0x%x, rounding_policy=0x%x, out=%p", graph, in1, in2, scale, overflow_policy, rounding_policy, out);
    gcmDUMP_API("$VX vxTensorDivideNode: graph=%p, in1=%p, in2=%p, scale=%p, overflow_policy=0x%x, rounding_policy=0x%x, out=%p", graph, in1, in2, scale, overflow_policy, rounding_policy, out);

    context = vxGetContext((vx_reference)graph);

    overflow_policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &overflow_policy);
    rounding_policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &rounding_policy);
    if (vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS || vxoReference_GetStatus((vx_reference)overflow_policy_s) != VX_SUCCESS) {
        gcmFOOTER_NO();
        return NULL;
    }
    parameters[3]  = (vx_reference)overflow_policy_s;
    parameters[4]  = (vx_reference)rounding_policy_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_DIV, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&overflow_policy_s);
    vxReleaseScalar(&rounding_policy_s);
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, policy=0x%x, out=%p", graph, in1, in2, policy, out);
    gcmDUMP_API("$VX vxTensorSubtractNode: graph=%p, in1=%p, in2=%p, policy=0x%x, out=%p", graph, in1, in2, policy, out);


    context = vxGetContext((vx_reference)graph);

    policy_s = vxCreateScalar(context, VX_TYPE_ENUM, &policy);
    if (vxoReference_GetStatus((vx_reference)policy_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)policy_s;
    }
    parameters[2]  = (vx_reference)policy_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_SUBTRACT, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&policy_s);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorTableLookupNode(
    vx_graph graph,
    vx_tensor in1,
    vx_lut lut,
    vx_tensor out)
{
    vx_reference parameters[] = {
        (vx_reference)in1,
        (vx_reference)lut,
        (vx_reference)out
    };
    gcmHEADER_ARG("graph=%p, in1=%p, lut=%p, out=%p", graph, in1, lut, out);
    gcmDUMP_API("$VX vxTensorTableLookupNode: graph=%p, in1=%p, lut=%p, out=%p", graph, in1, lut, out);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_TABLE_LOOKUP, parameters, vxmLENGTH_OF(parameters));
}

/* vxTensorTransposeNode_11 is for back compatibility with spec 1.1*/
VX_API_ENTRY vx_node VX_API_CALL vxTensorTransposeNode_11(
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
    gcmHEADER_ARG("graph=%p, inputs=%p, outputs=%p, dim1=0x%x, dim2=0x%x", graph, inputs, outputs, dim1, dim2);
    gcmDUMP_API("$VX vxTensorTransposeNode_11: graph=%p, inputs=%p, outputs=%p, dim1=0x%x, dim2=0x%x", graph, inputs, outputs, dim1, dim2);

    context = vxGetContext((vx_reference)graph);

    if (dim1 == dim2)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    vxQueryTensor(inputs, VX_TENSOR_NUMBER_OF_DIMS, &nt, sizeof(nt));
    if (dim1 >= nt || dim2 >= nt)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    vxoTensor_GetTensorDimStride(inputs, &nt, indims, VX_NULL);
    vxoTensor_GetTensorDimStride(outputs, &nt, outdims, VX_NULL);
    if (indims[dim1] != outdims[dim2] || indims[dim2] != outdims[dim1])
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    perm = vxCreateArray(context, VX_TYPE_UINT32, nt);
    if (!vxoArray_AllocateMemory(perm))
    {
        gcmFOOTER_NO();
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
    if (vxoReference_GetStatus((vx_reference)pnum) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)pnum;
    }
    parameters[1]  = (vx_reference)perm;
    parameters[2]  = (vx_reference)pnum;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_TRANSPOSE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseArray(&perm);
    vxReleaseScalar(&pnum);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorTransposeNode(
    vx_graph graph,
    vx_tensor input,
    vx_tensor output,
    vx_size dimension1,
    vx_size dimension2)
{
    vx_context context;
    vx_node    node;
    vx_uint32  nt;
    vx_uint32  indims[VX_CONTEXT_TENSOR_MAX_DIMENSION], outdims[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vx_array perm = VX_NULL;
    vx_scalar pnum = VX_NULL;
    vx_uint32 dim1;
    vx_uint32 dim2;

    vx_reference parameters[] = {
    (vx_reference)input,
    VX_NULL,
    VX_NULL,
    (vx_reference)output
    };

    gcmHEADER_ARG("graph=%p, inputs=%p, outputs=%p, dim1=0x%lx, dim2=0x%lx", graph, input, output, dimension1, dimension2);
    gcmDUMP_API("$VX vxTensorTransposeNode: graph=%p, inputs=%p, outputs=%p, dim1=0x%lx, dim2=0x%lx", graph, input, output, dimension1, dimension2);

    gcmSAFECASTSIZET(dim1, dimension1);
    gcmSAFECASTSIZET(dim2, dimension2);

    context = vxGetContext((vx_reference)graph);

    vxQueryTensor(input, VX_TENSOR_NUMBER_OF_DIMS, &nt, sizeof(nt));
    if (dim1 >= nt || dim2 >= nt)
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    vxoTensor_GetTensorDimStride(input, &nt, indims, VX_NULL);
    vxoTensor_GetTensorDimStride(output, &nt, outdims, VX_NULL);
    if (indims[dim1] != outdims[dim2] || indims[dim2] != outdims[dim1])
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    if(nt == 2) nt = 3; /*fix the bug that shader implementation has error with dimensions=2*/
    perm = vxCreateArray(context, VX_TYPE_UINT32, nt);
    if (!vxoArray_AllocateMemory(perm))
    {
        gcmFOOTER_NO();
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
    if (vxoReference_GetStatus((vx_reference)pnum) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)pnum;
    }
    parameters[1]  = (vx_reference)perm;
    parameters[2]  = (vx_reference)pnum;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_TRANSPOSE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseArray(&perm);
    vxReleaseScalar(&pnum);
    gcmFOOTER_NO();
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
    vx_uint32  nt;
    vx_uint32  indims[VX_CONTEXT_TENSOR_MAX_DIMENSION], outdims[VX_CONTEXT_TENSOR_MAX_DIMENSION];

    vx_array perma = VX_NULL;
    vx_scalar pnum = VX_NULL;

    vx_reference parameters[] = {
    (vx_reference)inputs,
    VX_NULL,
    VX_NULL,
    (vx_reference)outputs
    };

    gcmHEADER_ARG("graph=%p, inputs=%p, outputs=%p, perm=%p, sizes_of_perm=0x%x", graph, inputs, outputs, perm, sizes_of_perm);
    gcmDUMP_API("$VX vxTensorPermuteNode: graph=%p, inputs=%p, outputs=%p, perm=%p, sizes_of_perm=0x%x", graph, inputs, outputs, perm, sizes_of_perm);

    context = vxGetContext((vx_reference)graph);

    vxQueryTensor(inputs, VX_TENSOR_NUMBER_OF_DIMS, &nt, sizeof(nt));

    if (perm == VX_NULL)
        sizes_of_perm = nt;
    else if (sizes_of_perm > nt)
    {
       gcmFOOTER_NO();
       return VX_NULL;
    }
    vxoTensor_GetTensorDimStride(inputs, &nt, indims, VX_NULL);
    vxoTensor_GetTensorDimStride(outputs, &nt, outdims, VX_NULL);

    perma = vxCreateArray(context, VX_TYPE_UINT32, nt);
    if (!vxoArray_AllocateMemory(perma))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    else
    {
        vx_uint32 i;
        vx_uint32* pos = (vx_uint32*)perma->memory.logicals[0];
        for (i=0; i<nt; i++)
        {
            if (perm == VX_NULL)
                pos[i] = nt - i - 1;
            else
                pos[i] = i < sizes_of_perm ? perm[i] : i;
        }
    }

    pnum = vxCreateScalar(context, VX_TYPE_UINT32, &sizes_of_perm);
    if (vxoReference_GetStatus((vx_reference)pnum) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)pnum;
    }
    parameters[1]  = (vx_reference)perma;
    parameters[2]  = (vx_reference)pnum;


    node = vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_TRANSPOSE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseArray(&perma);
    vxReleaseScalar(&pnum);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorReduceSumNode(
    vx_graph graph,
    vx_tensor in,
    vx_tensor out,
    vx_uint32* reduce_dim,
    vx_int32 dim_size,
    vx_bool keep_dim)
{
    vx_reference parameters[] = {
        (vx_reference)in,
        (vx_reference)out,
        VX_NULL,
        VX_NULL
    };

    vx_context context = vxGetContext((vx_reference)graph);
    vx_node node = VX_NULL;
    vx_scalar reduceDim = VX_NULL;
    vx_scalar keepDim = vxCreateScalar(context, VX_TYPE_BOOL, &keep_dim);

    gcmHEADER_ARG("graph=%p, in=%p, out=%p, reduce_dim=%p, dim_size=0x%x, keep_dim=0x%x", graph, in, out, reduce_dim, dim_size, keep_dim);
    gcmDUMP_API("$VX vxTensorReduceSumNode: graph=%p, in=%p, out=%p, reduce_dim=%p, dim_size=0x%x, keep_dim=0x%x", graph, in, out, reduce_dim, dim_size, keep_dim);

    if (reduce_dim)
    {
        vx_uint32 dimNum;
        vxQueryTensor(in, VX_TENSOR_NUMBER_OF_DIMS, &dimNum, sizeof(dimNum));
        if (*reduce_dim > dimNum)
        {
            vxError("Invalid parameter reduce_dim");
            gcmFOOTER_NO();
            return VX_NULL;
        }
        reduceDim = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, reduce_dim);
        parameters[2] = (vx_reference)reduceDim;
    }

    if (keep_dim)
    {
        vx_uint32 inDim, outDim;
        vxQueryTensor(in, VX_TENSOR_NUMBER_OF_DIMS, &inDim, sizeof(inDim));
        vxQueryTensor(out, VX_TENSOR_NUMBER_OF_DIMS, &outDim, sizeof(outDim));

        if (inDim != outDim)
        {
            vxError("Invalid output dimension num");
            gcmFOOTER_NO();
            return VX_NULL;
        }
    }

    parameters[3] = (vx_reference)keepDim;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_REDUCE_SUM, parameters, vxmLENGTH_OF(parameters));

    if (reduceDim)
        vxReleaseScalar(&reduceDim);

    vxReleaseScalar(&keepDim);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorPadNode(vx_graph graph,
    vx_tensor in,
    vx_tensor out,
    const vx_nn_pad_params pad_params,
    vx_size size_of_pad_params)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_node node = VX_NULL;
    vx_bool shSupport = vx_false_e;
    vx_scalar padConst = VX_NULL;

    gcmHEADER_ARG("graph=%p, in=%p, out=%p, pad_params=%p, size_of_pad_params=0x%lx", graph, in, out, pad_params, size_of_pad_params);
    gcmDUMP_API("$VX vxTensorPadNode: graph=%p, in=%p, out=%p, pad_params=%p, size_of_pad_params=0x%lx", graph, in, out, pad_params, size_of_pad_params);

    if(context->evisNoInst.supportEVIS == vx_false_e)
    {
       if(TENSOR_RANK(out) == VX_TENSOR_RANK_CWHN)
       {
            if((TENSOR_VIEW_SIZE_INDEX(out, 0) == 1) && (TENSOR_VIEW_SIZE_INDEX(out, 3) == 1))
                shSupport = vx_true_e;
       }
       else if(TENSOR_RANK(out) == VX_TENSOR_RANK_WHCN)
       {
            if((TENSOR_VIEW_SIZE_INDEX(out, 2) == 1) && (TENSOR_VIEW_SIZE_INDEX(out, 3) == 1))
                shSupport = vx_true_e;
       }
    }

    if ((pad_params->numViewDimensions == 2) || shSupport)
    {
        vx_reference parameters[] = {
            (vx_reference)in,
            (vx_reference)out,
            VX_NULL,
            VX_NULL,
            VX_NULL,
            VX_NULL,
            VX_NULL,
            (vx_reference)pad_params->pad_const
        };

        vx_scalar padLeft = vxCreateScalar(context, VX_TYPE_UINT32, &pad_params->pad_front_array[0]);
        vx_scalar padRight = vxCreateScalar(context, VX_TYPE_UINT32, &pad_params->pad_back_array[0]);
        vx_scalar padTop = vxCreateScalar(context, VX_TYPE_UINT32, &pad_params->pad_front_array[1]);
        vx_scalar padBottom = vxCreateScalar(context, VX_TYPE_UINT32, &pad_params->pad_back_array[1]);
        vx_scalar padMode = vxCreateScalar(context, VX_TYPE_ENUM, &pad_params->pad_mode);

        if(shSupport)
        {
            int value = 0;
            padConst = vxCreateScalar(context, VX_TYPE_INT32, &value);
            parameters[7] = (vx_reference)padConst;
        }

        if (size_of_pad_params != sizeof(vx_nn_pad_params_t))
        {
            vxError(" size_of_pad_params doesn't match");
            gcmFOOTER_NO();
            return NULL;
        }
        if(context->evisNoInst.supportEVIS) vxmASSERT(pad_params->numViewDimensions == 2);

        parameters[2] = (vx_reference)padLeft;
        parameters[3] = (vx_reference)padRight;
        parameters[4] = (vx_reference)padTop;
        parameters[5] = (vx_reference)padBottom;
        parameters[6] = (vx_reference)padMode;

        node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_PAD, parameters, vxmLENGTH_OF(parameters));

        vxReleaseScalar(&padLeft);
        vxReleaseScalar(&padRight);
        vxReleaseScalar(&padTop);
        vxReleaseScalar(&padBottom);
        vxReleaseScalar(&padMode);
    }
    else
    {
        vx_reference parameters[] = {
            (vx_reference)in,
            (vx_reference)out,
            VX_NULL,
            VX_NULL,
            (vx_reference)pad_params->pad_const
        };
        vx_uint32 sizes[] = { pad_params->numViewDimensions, 2 };
        vx_tensor_create_params_t param = { 2, VX_NULL, VX_TYPE_INT32 };

        vx_tensor pad_dims = VX_NULL;
        vx_int32_ptr base = VX_NULL, front_base = pad_params->pad_front_array, back_base = pad_params->pad_back_array;
        vx_scalar padMode = vxCreateScalar(context, VX_TYPE_ENUM, &pad_params->pad_mode);
        param.sizes = sizes;
        pad_dims = vxCreateTensor2(context, &param, sizeof(vx_tensor_create_params_t));

        vxoTensor_AllocateMemory(pad_dims);
        vxoTensor_GetTensorViewMemory(pad_dims, (gctPOINTER *)&base, VX_NULL);

        parameters[2] = (vx_reference)pad_dims;
        parameters[3] = (vx_reference)padMode;

        base[0] = front_base[0];
        base[1] = back_base[0];
        base[2] = front_base[1];
        base[3] = back_base[1];

        base[4] = front_base[2];
        base[5] = back_base[2];
        base[6] = front_base[3];
        base[7] = back_base[3];

        node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_PAD2, parameters, vxmLENGTH_OF(parameters));

        vxReleaseScalar(&padMode);
        vxReleaseTensor(&pad_dims);
    }
    gcmFOOTER_NO();
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

    gcmHEADER_ARG("graph=%p, inputs=%p, negative_slope=%f, outputs=%p", graph, inputs, negative_slope, outputs);
    gcmDUMP_API("$VX vxLeakyReluLayer: graph=%p, inputs=%p, negative_slope=%f, outputs=%p", graph, inputs, negative_slope, outputs);

    if (vxoReference_GetStatus((vx_reference)negative_slopes) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)negative_slopes;
    }
    parameters[1]  = (vx_reference)negative_slopes;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_LEAKY, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&negative_slopes);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxPReluLayer(
    vx_graph                    graph,
    vx_tensor                   inputs,
    vx_tensor                   alpha,
    vx_tensor                   outputs
    )
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        (vx_reference)alpha,
        (vx_reference)outputs
    };

    vx_node node = VX_NULL;

    gcmHEADER_ARG("graph=%p, inputs=%p, alpha=%p, outputs=%p", graph, inputs, alpha, outputs);
    gcmDUMP_API("$VX vxPReluLayer: graph=%p, inputs=%p, alpha=%p, outputs=%p", graph, inputs, alpha, outputs);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_PRELU, parameters, vxmLENGTH_OF(parameters));

    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, eps=%f, mean=%p, variance=%p, gamma=%p, beta=%p, input=%p, output=%p",
         graph, eps, mean, variance, gamma, beta, input, output);
    gcmDUMP_API("$VX vxBatchNormalizationLayer: graph=%p, eps=%f, mean=%p, variance=%p, gamma=%p, beta=%p, input=%p, output=%p",
         graph, eps, mean, variance, gamma, beta, input, output);

    context = vxGetContext((vx_reference)graph);

    eps_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &eps);
    if (vxoReference_GetStatus((vx_reference)eps_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)eps_s;
    }
    parameters[0]  = (vx_reference)eps_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_BATCH_NORM, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&eps_s);
    gcmFOOTER_NO();
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

    gcmHEADER_ARG("graph=%p, score=%p, bbox=%p, anchors=%p, img_info=%p, rpn_params=%p, size_of_rpn_params=0x%lx, roi_output=%p, score_output=%p",
        graph, score, bbox, anchors, img_info, rpn_params, size_of_rpn_params, roi_output, score_output);
    gcmDUMP_API("$VX vxRPNLayer: graph=%p, score=%p, bbox=%p, anchors=%p, img_info=%p, rpn_params=%p, size_of_rpn_params=0x%lx, roi_output=%p, score_output=%p",
        graph, score, bbox, anchors, img_info, rpn_params, size_of_rpn_params, roi_output, score_output);

    vxmCHECK_PRECISION(anchors, VX_TENSOR_PRECISION_HIGH);

    vxmCHECK_PRECISION(bbox, VX_TENSOR_PRECISION_HIGH);

    vxmCHECK_PRECISION(score, VX_TENSOR_PRECISION_HIGH);

    vxmCHECK_PRECISION(img_info, VX_TENSOR_PRECISION_HIGH);

    vxmCHECK_PRECISION(roi_output, VX_TENSOR_PRECISION_HIGH);

    vxmCHECK_PRECISION(score_output, VX_TENSOR_PRECISION_HIGH);

    context = vxGetContext((vx_reference)graph);

    feature_stride_s = vxCreateScalar(context, VX_TYPE_UINT32, &rpn_params->feature_stride);
    if (vxoReference_GetStatus((vx_reference)feature_stride_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)feature_stride_s;
    }
    min_size_s = vxCreateScalar(context, VX_TYPE_UINT32, &rpn_params->min_size);
    if (vxoReference_GetStatus((vx_reference)min_size_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)min_size_s;
    }
    pre_nms_topn_s = vxCreateScalar(context, VX_TYPE_UINT32, &rpn_params->pre_nms_topn);
    if (vxoReference_GetStatus((vx_reference)pre_nms_topn_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)pre_nms_topn_s;
    }
    post_nms_topn_s = vxCreateScalar(context, VX_TYPE_UINT32, &rpn_params->post_nms_topn);
    if (vxoReference_GetStatus((vx_reference)post_nms_topn_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)post_nms_topn_s;
    }
    nms_thresh_s = vxCreateScalar(context, VX_TYPE_FLOAT32, &rpn_params->nms_thresh);
    if (vxoReference_GetStatus((vx_reference)nms_thresh_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)nms_thresh_s;
    }
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
    gcmFOOTER_NO();
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
    gcmHEADER_ARG("graph=%p, input_data=%p, input_rois=%p, roi_pool_params=%p, size_of_roi_params=0x%lx, output_arr=%p",
        graph, input_data, input_rois, roi_pool_params, size_of_roi_params, output_arr);
    gcmDUMP_API("$VX vxROIPoolingLayer: graph=%p, input_data=%p, input_rois=%p, roi_pool_params=%p, size_of_roi_params=0x%lx, output_arr=%p",
        graph, input_data, input_rois, roi_pool_params, size_of_roi_params, output_arr);


    if (size_of_roi_params != sizeof(vx_nn_roi_pool_params_ext_t) && size_of_roi_params != sizeof(vx_nn_roi_pool_params_t))
    {
        gcmFOOTER_NO();
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
        vx_size pool_type, pool_height, pool_width;
        vx_float32 spatial_scale;

        if (size_of_roi_params == sizeof(vx_nn_roi_pool_params_t))
        {
            pool_type = roi_pool_params->pool_type;
            spatial_scale = 1.0;
            pool_height = output_arr->dims[1];
            pool_width = output_arr->dims[0];
        }
        else
        {
            vx_nn_roi_pool_params_ext_t * roi_pool_params_ext = (vx_nn_roi_pool_params_ext_t *)roi_pool_params;
            pool_type = roi_pool_params_ext->khr.pool_type;
            spatial_scale = roi_pool_params_ext->spatial_scale;
            pool_height = roi_pool_params_ext->pooled_height;
            pool_width = roi_pool_params_ext->pooled_width;
        }
        pool_types = vxCreateScalar(context, VX_TYPE_ENUM, &pool_type);
        if (vxoReference_GetStatus((vx_reference)pool_types) != VX_SUCCESS) {
            gcmFOOTER_NO();
            return (vx_node)pool_types;
        }
        spatial_scales = vxCreateScalar(context, VX_TYPE_FLOAT32, &spatial_scale);
        if (vxoReference_GetStatus((vx_reference)spatial_scales) != VX_SUCCESS) {
            gcmFOOTER_NO();
            return (vx_node)spatial_scales;
        }
        pooled_heights = vxCreateScalar(context, VX_TYPE_INT32, &pool_height);
        if (vxoReference_GetStatus((vx_reference)pooled_heights) != VX_SUCCESS) {
            gcmFOOTER_NO();
            return (vx_node)pooled_heights;
        }
        pooled_widths = vxCreateScalar(context, VX_TYPE_INT32, &pool_width);
        if (vxoReference_GetStatus((vx_reference)pooled_widths) != VX_SUCCESS) {
            gcmFOOTER_NO();
            return (vx_node)pooled_widths;
        }
        parameters[2]  = (vx_reference)pool_types;
        parameters[3]  = (vx_reference)spatial_scales;
        parameters[4]  = (vx_reference)pooled_heights;
        parameters[5]  = (vx_reference)pooled_widths;

        node = vxoNode_CreateSpecific(graph, VX_KERNEL_ROI_POOLING_LAYER, parameters, vxmLENGTH_OF(parameters));

        vxReleaseScalar(&pool_types);
        vxReleaseScalar(&spatial_scales);
        vxReleaseScalar(&pooled_heights);
        vxReleaseScalar(&pooled_widths);

        gcmFOOTER_NO();
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

    gcmHEADER_ARG("graph=%p, inputs=%p, stride=0x%x, outputs=%p", graph, inputs, stride, outputs);
    gcmDUMP_API("$VX vxReorgLayer: graph=%p, inputs=%p, stride=0x%x, outputs=%p", graph, inputs, stride, outputs);

    if (vxoReference_GetStatus((vx_reference)strides) != VX_SUCCESS) {
        gcmFOOTER_NO();
        return (vx_node)strides;
    }
    parameters[1]  = (vx_reference)strides;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_REORG_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&strides);
    gcmFOOTER_NO();
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

    gcmHEADER_ARG("graph=%p, inputs=%p, outputs=%p", graph, inputs, outputs);
    gcmDUMP_API("$VX vxL2NormalizeLayer: graph=%p, inputs=%p, outputs=%p", graph, inputs, outputs);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_L2NORMALIZE_LAYER, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxL2NormalizeLayer2(
    vx_graph                    graph,
    vx_tensor                   inputs,
    const vx_nn_l2norm_params_t *l2norm_params,
    vx_size size_of_l2norm_params,
    vx_tensor                   outputs
    )
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        NULL,
        (vx_reference)outputs
    };

    vx_node node = VX_NULL;
    vx_scalar axis_s = VX_NULL;
    vx_int32 axis;

    gcmHEADER_ARG("graph=%p, inputs=%p, l2norm_params=%p, size_of_l2norm_params=0x%lx, outputs=%p",
        graph, inputs, l2norm_params, size_of_l2norm_params, outputs);
    gcmDUMP_API("$VX vxL2NormalizeLayer2: graph=%p, inputs=%p, l2norm_params=%p, size_of_l2norm_params=0x%lx, outputs=%p",
        graph, inputs, l2norm_params, size_of_l2norm_params, outputs);

    axis = l2norm_params->axis;
    axis_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &axis);

    parameters[1] = (vx_reference)axis_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_L2NORMALIZE_LAYER2, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&axis_s);
    gcmFOOTER_NO();
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
    vx_node node = VX_NULL;
    vx_uint32 i = 0;
    vx_uint32 stride_x, stride_y;
    vx_int32 channel_group;
    vx_enum overflow_policy, rounding_policy, down_scale_size_rounding;
    vx_size pad_x, pad_x_right, pad_y, pad_y_bottom, a_x, a_y;

    vx_reference parameters[] = {
    (vx_reference)inputs,
    (vx_reference)weights,
    (vx_reference)biases,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)VX_NULL,
    (vx_reference)outputs,
    };

    gcmHEADER_ARG("graph=%p, inputs=%p, weights=%p, biases=%p, deconvolution_params=%p, size_of_deconv_params=0x%lx, outputs=%p",
        graph, inputs, weights, biases, deconvolution_params, size_of_deconv_params, outputs);
    gcmDUMP_API("$VX vxDeconvolutionLayer: graph=%p, inputs=%p, weights=%p, biases=%p, deconvolution_params=%p, size_of_deconv_params=0x%lx, outputs=%p",
        graph, inputs, weights, biases, deconvolution_params, size_of_deconv_params, outputs);

    if (size_of_deconv_params == sizeof(vx_nn_deconvolution_params_ext_t))
    {
        vx_nn_deconvolution_params_ext_t *deconvolution_params_ext = (vx_nn_deconvolution_params_ext_t *)deconvolution_params;
        pad_x = deconvolution_params_ext->khr.padding_x;
        pad_x_right = deconvolution_params_ext->padding_x_right;
        pad_y = deconvolution_params_ext->khr.padding_y;
        pad_y_bottom = deconvolution_params_ext->padding_y_bottom;
        overflow_policy = deconvolution_params_ext->khr.overflow_policy;
        rounding_policy = deconvolution_params_ext->khr.rounding_policy;
        a_x = deconvolution_params_ext->khr.a_x;
        a_y = deconvolution_params_ext->khr.a_y;
        channel_group = deconvolution_params_ext->channel_group;
    }
    else if (size_of_deconv_params == sizeof(vx_nn_deconvolution_params_ext2_t))
    {
        vx_nn_deconvolution_params_ext2_t *deconvolution_params_ext2 = (vx_nn_deconvolution_params_ext2_t *)deconvolution_params;
        pad_x = deconvolution_params_ext2->ext.khr.padding_x;
        pad_x_right = deconvolution_params_ext2->ext.padding_x_right;
        pad_y = deconvolution_params_ext2->ext.khr.padding_y;
        pad_y_bottom = deconvolution_params_ext2->ext.padding_y_bottom;
        overflow_policy = deconvolution_params_ext2->ext.khr.overflow_policy;
        rounding_policy = deconvolution_params_ext2->ext.khr.rounding_policy;
        a_x = deconvolution_params_ext2->ext.khr.a_x;
        a_y = deconvolution_params_ext2->ext.khr.a_y;
        channel_group = deconvolution_params_ext2->ext.channel_group;
        stride_x = deconvolution_params_ext2->stride_x;
        stride_y = deconvolution_params_ext2->stride_y;
        down_scale_size_rounding = deconvolution_params_ext2->down_scale_size_rounding;
    }
    else if (size_of_deconv_params == sizeof(vx_nn_deconvolution_params_t))
    {
        pad_x = deconvolution_params->padding_x;
        pad_x_right = deconvolution_params->padding_x;
        pad_y = deconvolution_params->padding_y;
        pad_y_bottom = deconvolution_params->padding_y;
        overflow_policy = deconvolution_params->overflow_policy;
        rounding_policy = deconvolution_params->rounding_policy;
        a_x = deconvolution_params->a_x;
        a_y = deconvolution_params->a_y;
        channel_group = 1;
    }
    else
    {
        vxError("Invalid parameter deconvolution_params\n");
        gcmFOOTER_NO();
        return NULL;
    }

    parameters[3] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &pad_x);
    parameters[4] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &pad_x_right);
    parameters[5] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &pad_y);
    parameters[6] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &pad_y_bottom);
    parameters[7] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &overflow_policy);
    parameters[8] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &rounding_policy);
    parameters[9] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &a_x);
    parameters[10] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_SIZE, &a_y);
    parameters[11] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &channel_group);

    if (size_of_deconv_params == sizeof(vx_nn_deconvolution_params_ext2_t))
    {
        parameters[12] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &stride_x);
        parameters[13] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &stride_y);
        parameters[14] = (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &down_scale_size_rounding);
    }

    vxmCHECK_PRECISION(biases, VX_TENSOR_PRECISION_HIGH);

    //vxmCHECK_LIFETIME(weights, VX_TENSOR_LIFE_TIME_STATIC);

    //vxmCHECK_LIFETIME(biases, VX_TENSOR_LIFE_TIME_STATIC);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_DECONVOLUTION_LAYER, parameters, vxmLENGTH_OF(parameters));

    for (i = 3; i < (gcmCOUNTOF(parameters) - 1); i ++)
    {
        if ((size_of_deconv_params == sizeof(vx_nn_deconvolution_params_ext_t)) &&
            ((i > 11) && (i < 15)))
        {
            continue;
        }
        vxReleaseScalar((vx_scalar*)&parameters[i]);
    }
    gcmFOOTER_NO();
    return node;
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
    gcmHEADER_ARG("graph=%p, src=%p, dst=%p", graph, src, dst);
    gcmDUMP_API("$VX vxTensorCopyNode: graph=%p, src=%p, dst=%p", graph, src, dst);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_COPY, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxLstmUnitLayer(
    vx_graph graph,
    vx_tensor input,
    vx_tensor output_state_in,
    vx_tensor cell_state_in,
    const vx_nn_lstm_params_t * lstm_params,
    vx_size size_of_lstm_params,
    vx_tensor scratch,
    vx_tensor output_state_out,
    vx_tensor cell_state_out,
    vx_tensor output)
{
    vx_node node = VX_NULL;
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)lstm_params->input2input_weight,
        (vx_reference)lstm_params->input2forget_weight,
        (vx_reference)lstm_params->input2cell_weight,
        (vx_reference)lstm_params->input2output_weight,

        (vx_reference)lstm_params->recurrent2input_weight,
        (vx_reference)lstm_params->recurrent2forget_weight,
        (vx_reference)lstm_params->recurrent2cell_weight,
        (vx_reference)lstm_params->recurrent2output_weight,

        (vx_reference)lstm_params->cell2input_weight,
        (vx_reference)lstm_params->cell2forget_weight,
        (vx_reference)lstm_params->cell2output_weight,

        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,

        (vx_reference)lstm_params->input_gate_bias,
        (vx_reference)lstm_params->forget_gate_bias,
        (vx_reference)lstm_params->cell_bias,
        (vx_reference)lstm_params->output_gate_bias,


        (vx_reference)lstm_params->projection_weight,
        (vx_reference)lstm_params->projection_bias,
        (vx_reference)output_state_in,
        (vx_reference)cell_state_in,

        (vx_reference)lstm_params->activation,
        VX_NULL,
        (vx_reference)lstm_params->cell_clip,
        (vx_reference)lstm_params->proj_clip,

        (vx_reference)scratch,
        (vx_reference)output_state_out,
        (vx_reference)cell_state_out,
        (vx_reference)output
    };

    gcmHEADER_ARG("graph=%p, input=%p, output_state_in=%p, cell_state_in=%p, lstm_params=%p, size_of_lstm_params=0x%lx, scratch=%p, output_state_out=%p, cell_state_out=%p, output=%p",
        graph, input, output_state_in, cell_state_in, lstm_params, size_of_lstm_params, scratch, output_state_out, cell_state_out, output);
    gcmDUMP_API("$VX vxLstmUnitLayer: graph=%p, input=%p, output_state_in=%p, cell_state_in=%p, lstm_params=%p, size_of_lstm_params=0x%lx, scratch=%p, output_state_out=%p, cell_state_out=%p, output=%p",
        graph, input, output_state_in, cell_state_in, lstm_params, size_of_lstm_params, scratch, output_state_out, cell_state_out, output);

    if (size_of_lstm_params == sizeof(vx_nn_lstm_params_t))
    {

    }
    else if (size_of_lstm_params == sizeof(vx_nn_lstm_params_ext_t))
    {
        vx_nn_lstm_params_ext_t * lstm = (vx_nn_lstm_params_ext_t *)lstm_params;
        parameters[12] = (vx_reference)lstm->layernorm2input_weight,
        parameters[13] = (vx_reference)lstm->layernorm2forget_weight,
        parameters[14] = (vx_reference)lstm->layernorm2cell_weight,
        parameters[15] = (vx_reference)lstm->layernorm2output_weight,

        parameters[25] = (vx_reference)lstm->forget_bias;
    }
    else
    {
        vxError(" size_of_lstm_params doesn't match");
        gcmFOOTER_NO();
        return NULL;
    }

    vxmCHECK_PRECISION(lstm_params->input_gate_bias, VX_TENSOR_PRECISION_HIGH);
    vxmCHECK_PRECISION(lstm_params->forget_gate_bias, VX_TENSOR_PRECISION_HIGH);
    vxmCHECK_PRECISION(lstm_params->cell_bias, VX_TENSOR_PRECISION_HIGH);
    vxmCHECK_PRECISION(lstm_params->output_gate_bias, VX_TENSOR_PRECISION_HIGH);
    vxmCHECK_PRECISION(lstm_params->projection_bias, VX_TENSOR_PRECISION_HIGH);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_LSTM_UNIT, parameters, vxmLENGTH_OF(parameters));

    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxLstmLayer(
    vx_graph graph,
    vx_tensor input,
    vx_tensor static_input,
    vx_tensor cont,
    const vx_nn_lstm_layer_params_t * lstm_layer_params,
    vx_size size_of_lstm_layer_params,
    vx_tensor output
    )
{

    vx_node node = VX_NULL;
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)static_input,
        (vx_reference)cont,
        (vx_reference)lstm_layer_params->lstm_param.input2input_weight,
        (vx_reference)lstm_layer_params->lstm_param.input2forget_weight,
        (vx_reference)lstm_layer_params->lstm_param.input2cell_weight,
        (vx_reference)lstm_layer_params->lstm_param.input2output_weight,

        (vx_reference)lstm_layer_params->lstm_param.recurrent2input_weight,
        (vx_reference)lstm_layer_params->lstm_param.recurrent2forget_weight,
        (vx_reference)lstm_layer_params->lstm_param.recurrent2cell_weight,
        (vx_reference)lstm_layer_params->lstm_param.recurrent2output_weight,

        (vx_reference)lstm_layer_params->lstm_param.cell2input_weight,
        (vx_reference)lstm_layer_params->lstm_param.cell2forget_weight,
        (vx_reference)lstm_layer_params->lstm_param.cell2output_weight,

        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,

        (vx_reference)lstm_layer_params->lstm_param.input_gate_bias,
        (vx_reference)lstm_layer_params->lstm_param.forget_gate_bias,
        (vx_reference)lstm_layer_params->lstm_param.cell_bias,
        (vx_reference)lstm_layer_params->lstm_param.output_gate_bias,


        (vx_reference)lstm_layer_params->lstm_param.projection_weight,
        (vx_reference)lstm_layer_params->lstm_param.projection_bias,

        (vx_reference)lstm_layer_params->lstm_param.activation,
        VX_NULL,
        (vx_reference)lstm_layer_params->lstm_param.cell_clip,
        (vx_reference)lstm_layer_params->lstm_param.proj_clip,
        (lstm_layer_params->lstm_layer_type > 0)?(vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &lstm_layer_params->lstm_layer_type):NULL,

        (vx_reference)output
    };

    gcmHEADER_ARG("graph=%p, input=%p, static_input=%p, cont=%p, lstm_layer_params=%p, size_of_lstm_layer_params=0x%lx, output=%p",
        graph, input, static_input, cont, lstm_layer_params, size_of_lstm_layer_params, output);
    gcmDUMP_API("$VX vxLstmUnitLayer: graph=%p, input=%p, static_input=%p, cont=%p, lstm_layer_params=%p, size_of_lstm_layer_params=0x%lx, output=%p",
        graph, input, static_input, cont, lstm_layer_params, size_of_lstm_layer_params, output);

    if (size_of_lstm_layer_params == sizeof(vx_nn_lstm_layer_params_t))
    {
    }
    else if (size_of_lstm_layer_params == sizeof(vx_nn_lstm_layer_params_ext_t))
    {
        vx_nn_lstm_layer_params_ext_t * lstm = (vx_nn_lstm_layer_params_ext_t *)&lstm_layer_params->lstm_param;
        parameters[14] = (vx_reference)lstm->lstm_param.layernorm2input_weight,
        parameters[15] = (vx_reference)lstm->lstm_param.layernorm2forget_weight,
        parameters[16] = (vx_reference)lstm->lstm_param.layernorm2cell_weight,
        parameters[17] = (vx_reference)lstm->lstm_param.layernorm2output_weight,

        parameters[25] = (vx_reference)lstm->lstm_param.forget_bias;
    }
    else
    {
        vxError(" size_of_lstm_params doesn't match");
        gcmFOOTER_NO();
        return NULL;
    }

    vxmCHECK_PRECISION(lstm_layer_params->lstm_param.input_gate_bias, VX_TENSOR_PRECISION_HIGH);
    vxmCHECK_PRECISION(lstm_layer_params->lstm_param.forget_gate_bias, VX_TENSOR_PRECISION_HIGH);
    vxmCHECK_PRECISION(lstm_layer_params->lstm_param.cell_bias, VX_TENSOR_PRECISION_HIGH);
    vxmCHECK_PRECISION(lstm_layer_params->lstm_param.output_gate_bias, VX_TENSOR_PRECISION_HIGH);
    vxmCHECK_PRECISION(lstm_layer_params->lstm_param.projection_bias, VX_TENSOR_PRECISION_HIGH);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_LSTM_LAYER, parameters, vxmLENGTH_OF(parameters));


    if (lstm_layer_params->lstm_layer_type > 0)
        vxReleaseScalar((vx_scalar*)&parameters[23]);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxReorgLayer2(
    vx_graph                    graph,
    vx_tensor                   input,
    const vx_nn_reorg_params    reorg_params,
    vx_size                     size_of_reorg_params,
    vx_tensor                   output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        (vx_reference)output,
        VX_NULL,
        VX_NULL
    };

    vx_node node = VX_NULL;
    vx_scalar block_size = NULL, type = NULL;
    vx_scalar num_group = NULL, axis = NULL;

    gcmHEADER_ARG("graph=%p, input=%p, reorg_params=%p, size_of_reorg_params=0x%lx, output=%p",
        graph, input, reorg_params, size_of_reorg_params, output);
    gcmDUMP_API("$VX vxReorgLayer2: graph=%p, input=%p, reorg_params=%p, size_of_reorg_params=0x%lx, output=%p",
        graph, input, reorg_params, size_of_reorg_params, output);

    type = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &reorg_params->type);
    if (vxoReference_GetStatus((vx_reference)type) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)type;
    }
    parameters[2]  = (vx_reference)type;

    if (sizeof(vx_nn_reorg_params_ext_t) == size_of_reorg_params)
    {
        vx_nn_reorg_params_ext_t * params = (vx_nn_reorg_params_ext_t *)reorg_params;
        parameters[1] = (vx_reference)params->base.block_size;
        parameters[3] = (vx_reference)params->pad;
    }
    else if (sizeof(vx_nn_reorg_params_t) == size_of_reorg_params)
    {
        parameters[1] = (vx_reference)reorg_params->block_size;
    }
    else if (sizeof(vx_nn_reorg_params_ext2_t) == size_of_reorg_params)
    {
        vx_nn_reorg_params_ext2_t * params = (vx_nn_reorg_params_ext2_t *)reorg_params;
        num_group = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, params->num_group);
        axis = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, params->axis);
        parameters[1] = (vx_reference)reorg_params->block_size;
        parameters[5] = (vx_reference)num_group;
        parameters[6] = (vx_reference)axis;
    }

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_REORG2_LAYER, parameters, vxmLENGTH_OF(parameters));

    if (sizeof(vx_nn_reorg_params_ext2_t) == size_of_reorg_params)
    {
        vxReleaseScalar(&num_group);
        vxReleaseScalar(&axis);
    }

    vxReleaseScalar(&block_size);
    vxReleaseScalar(&type);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorRoundingNode(
    vx_graph                       graph,
    vx_tensor                      input,
    const vx_nn_rounding_params    rounding_params,
    vx_size                        size_of_rounding_params,
    vx_tensor                      output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        VX_NULL,
        (vx_reference)output
    };

    vx_node node = VX_NULL;
    vx_scalar mode = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &rounding_params->mode);

    gcmHEADER_ARG("graph=%p, input=%p, rounding_params=%p, size_of_rounding_params=0x%lx, output=%p",
        graph, input, rounding_params, size_of_rounding_params, output);
    gcmDUMP_API("$VX vxTensorRoundingNode: graph=%p, input=%p, rounding_params=%p, size_of_rounding_params=0x%lx, output=%p",
        graph, input, rounding_params, size_of_rounding_params, output);

    if (vxoReference_GetStatus((vx_reference)mode) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)mode;
    }
    parameters[1]  = (vx_reference)mode;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_ROUNDING, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&mode);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxHashTableLookupLayer(
    vx_graph                    graph,
    vx_tensor                   input,
    const vx_nn_hashlut_params  hashlut_params,
    vx_size                     size_of_hashlut_params,
    vx_tensor                   hits,
    vx_tensor                   output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)hashlut_params->keys,
        (vx_reference)hashlut_params->values,
        (vx_reference)hits,
        (vx_reference)output
    };

    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, input=%p, hashlut_params=%p, size_of_hashlut_params=0x%lx, hits=%p, output=%p",
        graph, input, hashlut_params, size_of_hashlut_params, hits, output);
    gcmDUMP_API("$VX vxHashTableLookupLayer: graph=%p, input=%p, hashlut_params=%p, size_of_hashlut_params=0x%lx, hits=%p, output=%p",
        graph, input, hashlut_params, size_of_hashlut_params, hits, output);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_HASH_LUT_LAYER, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxLSHProjectionLayer(
    vx_graph                    graph,
    vx_tensor                   input,
    const vx_nn_lshproj_params  lshproj_params,
    vx_size                     size_of_lshproj_params,
    vx_tensor                   output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)lshproj_params->type,
        (vx_reference)lshproj_params->hash_func,
        (vx_reference)lshproj_params->weights,
        (vx_reference)output
    };

    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, input=%p, lshproj_params=%p, size_of_lshproj_params=0x%lx, output=%p",
        graph, input, lshproj_params, size_of_lshproj_params, output);
    gcmDUMP_API("$VX vxLSHProjectionLayer: graph=%p, input=%p, lshproj_params=%p, size_of_lshproj_params=0x%lx, output=%p",
        graph, input, lshproj_params, size_of_lshproj_params, output);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_LSH_PROJECTION_LAYER, parameters, vxmLENGTH_OF(parameters));

    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorReshapeNode(
    vx_graph                    graph,
    vx_tensor                   input,
    const vx_nn_reshape_params  reshape_params,
    vx_size                     size_of_reshape_params,
    vx_tensor                   output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)reshape_params->dims,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, reshape_params=%p, size_of_reshape_params=0x%lx, output=%p",
        graph, input, reshape_params, size_of_reshape_params, output);
    gcmDUMP_API("$VX vxTensorReshapeNode: graph=%p, input=%p, reshape_params=%p, size_of_reshape_params=0x%lx, output=%p",
        graph, input, reshape_params, size_of_reshape_params, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_RESHPE, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorScaleNode(
    vx_graph                    graph,
    vx_tensor                   input,
    const vx_nn_scale_params    scale_params,
    vx_size                     size_of_scale_params,
    vx_tensor                   output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        NULL,
        (vx_reference)output
    };

    vx_node node = VX_NULL;

    vx_scalar type = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &scale_params->type);
    gcmHEADER_ARG("graph=%p, input=%p, scale_params=%p, size_of_scale_params=0x%lx, output=%p",
        graph, input, scale_params, size_of_scale_params, output);
    gcmDUMP_API("$VX vxTensorScaleNode: graph=%p, input=%p, scale_params=%p, size_of_scale_params=0x%lx, output=%p",
        graph, input, scale_params, size_of_scale_params, output);

    if (vxoReference_GetStatus((vx_reference)type) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)type;
    }
    parameters[1]  = (vx_reference)type;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_SCALE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&type);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorReverse(
    vx_graph graph,
    vx_tensor inputs,
    const vx_nn_tensor_reverse_params_t * tensor_reverse_params,
    vx_size size_of_tensor_reverse_params,
    vx_tensor outputs)
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        (vx_reference)outputs
    };

    vx_node node = VX_NULL;
    vx_scalar numOfAxis = VX_NULL;
    vx_scalar axis[4] = {VX_NULL};
    vx_uint32 sizeOfAxis = 0;
    vx_uint32 i = 0;
    gcmHEADER_ARG("graph=%p, inputs=%p, tensor_reverse_params=%p, size_of_tensor_reverse_params=0x%lx, outputs=%p",
        graph, inputs, tensor_reverse_params, size_of_tensor_reverse_params, outputs);
    gcmDUMP_API("$VX vxTensorReverse: graph=%p, inputs=%p, tensor_reverse_params=%p, size_of_tensor_reverse_params=0x%lx, outputs=%p",
        graph, inputs, tensor_reverse_params, size_of_tensor_reverse_params, outputs);

    if (size_of_tensor_reverse_params != sizeof(vx_nn_tensor_reverse_params_t))
    {
        vxError("Invalid parameter convolution_relu_pooling_params");
        gcmFOOTER_NO();
        return NULL;
    }

    sizeOfAxis = tensor_reverse_params->numberOfAxis;

    if (sizeOfAxis > 4)
    {
        vxError("Invalid parameter numberOfAxis, can't larger than 4");
        gcmFOOTER_NO();
        return NULL;
    }

    numOfAxis = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &tensor_reverse_params->numberOfAxis);
    parameters[1] = (vx_reference)numOfAxis;

    for (i = 0; i < sizeOfAxis; i++)
    {
        if (&tensor_reverse_params->axis[i] != VX_NULL)
        {
            axis[i] = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &tensor_reverse_params->axis[i]);
            parameters[i + 2] = (vx_reference)axis[i];
        }
        else
        {
            vxError("Invalid parameter axis");
            gcmFOOTER_NO();
            return NULL;
        }
    }

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_REVERSE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&numOfAxis);
    for (i = 0; i < sizeOfAxis; i++)
    {
        vxReleaseScalar(&axis[i]);
    }
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxRNNLayer(
    vx_graph                    graph,
    vx_tensor                   input,
    const vx_nn_rnn_params      rnn_params,
    vx_size                     size_of_rnn_params,
    vx_tensor                   state_out,
    vx_tensor                   output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)rnn_params->weights,
        (vx_reference)rnn_params->recurrent_weights,
        (vx_reference)rnn_params->bias,
        (vx_reference)rnn_params->state_in,
        (vx_reference)rnn_params->activation,
        (vx_reference)state_out,
        (vx_reference)output
    };

    vx_node node = VX_NULL;

    gcmHEADER_ARG("graph=%p, input=%p, rnn_params=%p, size_of_rnn_params=0x%lx, output=%p",
        graph, input, rnn_params, size_of_rnn_params, output);
    gcmDUMP_API("$VX vxRNNLayer: graph=%p, input=%p, rnn_params=%p, size_of_rnn_params=0x%lx, output=%p",
        graph, input, rnn_params, size_of_rnn_params, output);

    vxmCHECK_PRECISION(rnn_params->bias, VX_TENSOR_PRECISION_HIGH);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_RNN_LAYER, parameters, vxmLENGTH_OF(parameters));

    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxSoftmaxLayer2(
    vx_graph                    graph,
    vx_tensor                   input,
    const vx_nn_softmax_params  softmax_params,
    vx_size                     size_of_softmax_params,
    vx_tensor                   output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        NULL,
        NULL,
        (vx_reference)output
    };

    vx_node node = VX_NULL;
    vx_scalar axis_s = VX_NULL, beta_s = VX_NULL;
    vx_float32 beta;
    vx_int32 axis;

    gcmHEADER_ARG("graph=%p, input=%p, softmax_params=%p, size_of_softmax_params=0x%lx, output=%p",
        graph, input, softmax_params, size_of_softmax_params, output);
    gcmDUMP_API("$VX vxSoftmaxLayer2: graph=%p, input=%p, softmax_params=%p, size_of_softmax_params=0x%lx, output=%p",
        graph, input, softmax_params, size_of_softmax_params, output);

    if (sizeof(vx_nn_softmax_params_ext_t) == size_of_softmax_params)
    {
        vx_nn_softmax_params_ext_t * params = (vx_nn_softmax_params_ext_t *)softmax_params;
        beta = params->base.beta;
        axis = params->axis;
    }
    else if (sizeof(vx_nn_softmax_params_t) == size_of_softmax_params)
    {
        beta = softmax_params->beta;
        axis = -1;
    }
    beta_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &beta);
    axis_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &axis);
    parameters[1] = (vx_reference)beta_s;
    parameters[2] = (vx_reference)axis_s;
    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_SOFTMAX2_LAYER, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&beta_s);
    vxReleaseScalar(&axis_s);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxSVDFLayer(
    vx_graph                    graph,
    vx_tensor                   input,
    const vx_nn_svdf_params     svdf_params,
    vx_size                     size_of_svdf_params,
    vx_tensor                   state_out,
    vx_tensor                   output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)svdf_params->weights_feature,
        (vx_reference)svdf_params->recurrent_time,
        (vx_reference)svdf_params->bias,
        (vx_reference)svdf_params->state_in,
        (vx_reference)svdf_params->rank,
        (vx_reference)svdf_params->activation,
        (vx_reference)state_out,
        (vx_reference)output
    };

    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, input=%p, svdf_params=%p, size_of_svdf_params=0x%lx, output=%p",
        graph, input, svdf_params, size_of_svdf_params, output);
    gcmDUMP_API("$VX vxSVDFLayer: graph=%p, input=%p, svdf_params=%p, size_of_svdf_params=0x%lx, output=%p",
        graph, input, svdf_params, size_of_svdf_params, output);

    vxmCHECK_PRECISION(svdf_params->bias, VX_TENSOR_PRECISION_HIGH);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_SVDF_LAYER, parameters, vxmLENGTH_OF(parameters));

    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorTableLookupNode2(
    vx_graph graph,
    vx_tensor in1,
    vx_tensor lut,
    vx_tensor out
    )
{
    vx_reference parameters[] = {
        (vx_reference)in1,
        (vx_reference)lut,
        (vx_reference)out
    };

    vx_node node = VX_NULL;

    gcmHEADER_ARG("graph=%p, input=%p, lut=%p, out=%p",
        graph, in1, lut, out);
    gcmDUMP_API("$VX vxTensorTableLookupNode2: graph=%p, input=%p, lut=%p, out=%p",
        graph, in1, lut, out);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_LUT2_LAYER, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxNormalizationLayer2(
    vx_graph graph,
    vx_tensor inputs,
    const vx_nn_normalization_params_t *normalization_params,
    vx_size size_of_normalization_param,
    vx_tensor outputs
    )
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        (vx_reference)outputs
    };

    vx_node node = VX_NULL;
    vx_scalar type_s = VX_NULL, norm_size_s = VX_NULL, alpha_s = VX_NULL, beta_s = VX_NULL, bias_s = VX_NULL, axis_s = VX_NULL;
    vx_uint32 norm_size;
    vx_float32 alpha, beta, bias;
    vx_int32 axis;
    vx_enum type;

    gcmHEADER_ARG("graph=%p, inputs=%p, normalization_params=%p, size_of_normalization_param=0x%lx, outputs=%p",
        graph, inputs, normalization_params, size_of_normalization_param, outputs);
    gcmDUMP_API("$VX vxNormalizationLayer2: graph=%p, inputs=%p, normalization_params=%p, size_of_normalization_param=0x%lx, outputs=%p",
        graph, inputs, normalization_params, size_of_normalization_param, outputs);

    if (size_of_normalization_param == sizeof(vx_nn_normalization_params_ext_t))
    {
        vx_nn_normalization_params_ext_t *normalization_params_ext = (vx_nn_normalization_params_ext_t *)normalization_params;
        type = normalization_params_ext->base.type;
        norm_size = normalization_params_ext->base.norm_size;
        alpha = normalization_params_ext->base.alpha;
        beta = normalization_params_ext->base.beta;
        bias = normalization_params_ext->base.bias;
        axis = normalization_params_ext->axis;
    }
    else if (size_of_normalization_param == sizeof(vx_nn_normalization_params_t))
    {
        type = normalization_params->type;
        norm_size = normalization_params->norm_size;
        alpha = normalization_params->alpha;
        beta = normalization_params->beta;
        bias = normalization_params->bias;
        axis = -1;
    }

    type_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &type);
    if (vxoReference_GetStatus((vx_reference)type_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)type_s;
    }
    norm_size_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_UINT32, &norm_size);
    if (vxoReference_GetStatus((vx_reference)norm_size_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)norm_size_s;
    }
    alpha_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &alpha);
    if (vxoReference_GetStatus((vx_reference)alpha_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)alpha_s;
    }
    beta_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &beta);
    if (vxoReference_GetStatus((vx_reference)beta_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)beta_s;
    }
    bias_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &bias);
    if (vxoReference_GetStatus((vx_reference)bias_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)bias_s;
    }
    axis_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &axis);

    parameters[1]  = (vx_reference)type_s;
    parameters[2]  = (vx_reference)norm_size_s;
    parameters[3]  = (vx_reference)alpha_s;
    parameters[4]  = (vx_reference)beta_s;
    parameters[5]  = (vx_reference)bias_s;
    parameters[6]  = (vx_reference)axis_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_NORMALIZATION_LAYER2, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&type_s);

    vxReleaseScalar(&norm_size_s);

    vxReleaseScalar(&alpha_s);

    vxReleaseScalar(&beta_s);

    vxReleaseScalar(&bias_s);

    vxReleaseScalar(&axis_s);

    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxYUV2RGBScaleNode(
    vx_graph                          graph,
    vx_image                          input,
    const vx_nn_yuv2rgb_scale_params  yuv2rgb_scale_params,
    vx_size                           size_of_yuv2rgb_scale_param,
    vx_tensor                         output
    )
{
    vx_reference parameters[] = {
        (vx_reference)input,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        (vx_reference)output
    };

    vx_array rect_a = VX_NULL;
    vx_scalar r_mean_s, g_mean_s, b_mean_s, rgb_scale_s, y_only_s, output_rgb_s;
    vx_node node = VX_NULL;
    vx_context context = vxGetContext((vx_reference)graph);

    gcmHEADER_ARG("graph=%p, input=%p, yuv2rgb_scale_params=%p, size_of_yuv2rgb_scale_param=0x%lx, output=%p",
        graph, input, yuv2rgb_scale_params, size_of_yuv2rgb_scale_param, output);
    gcmDUMP_API("$VX vxYUV2RGBScaleNode: graph=%p, input=%p, yuv2rgb_scale_params=%p, size_of_yuv2rgb_scale_param=0x%lx, output=%p",
        graph, input, yuv2rgb_scale_params, size_of_yuv2rgb_scale_param, output);

    rect_a = vxCreateArray(context, VX_TYPE_UINT32, 4);
    if (!vxoArray_AllocateMemory(rect_a))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    else
    {
        vx_uint32* pos = (vx_uint32*)rect_a->memory.logicals[0];
        pos[0] = yuv2rgb_scale_params->rect.start_x;
        pos[1] = yuv2rgb_scale_params->rect.start_y;
        pos[2] = yuv2rgb_scale_params->rect.end_x;
        pos[3] = yuv2rgb_scale_params->rect.end_y;
    }

    r_mean_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &yuv2rgb_scale_params->mean_r);
    if (vxoReference_GetStatus((vx_reference)r_mean_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)r_mean_s;
    }
    g_mean_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &yuv2rgb_scale_params->mean_g);
    if (vxoReference_GetStatus((vx_reference)g_mean_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)g_mean_s;
    }
    b_mean_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &yuv2rgb_scale_params->mean_b);
    if (vxoReference_GetStatus((vx_reference)b_mean_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)b_mean_s;
    }
    rgb_scale_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &yuv2rgb_scale_params->scale_rgb);
    if (vxoReference_GetStatus((vx_reference)rgb_scale_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)rgb_scale_s;
    }
    y_only_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_BOOL, &yuv2rgb_scale_params->y_only);
    if (vxoReference_GetStatus((vx_reference)y_only_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)y_only_s;
    }
    output_rgb_s = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_BOOL, &yuv2rgb_scale_params->output_rgb);
    if (vxoReference_GetStatus((vx_reference)output_rgb_s) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)output_rgb_s;
    }
    parameters[1]  = (vx_reference)rect_a;
    parameters[2]  = (vx_reference)r_mean_s;
    parameters[3]  = (vx_reference)g_mean_s;
    parameters[4]  = (vx_reference)b_mean_s;
    parameters[5]  = (vx_reference)rgb_scale_s;
    parameters[6]  = (vx_reference)y_only_s;
    parameters[7]  = (vx_reference)output_rgb_s;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_YUV2RGB_SCALE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseArray(&rect_a);
    vxReleaseScalar(&r_mean_s);
    vxReleaseScalar(&g_mean_s);
    vxReleaseScalar(&b_mean_s);
    vxReleaseScalar(&rgb_scale_s);
    vxReleaseScalar(&y_only_s);
    vxReleaseScalar(&output_rgb_s);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorTransposeNode2(
    vx_graph graph,
    vx_tensor inputs,
    const vx_nn_transpose_params_t *transpose_params,
    vx_size size_of_transpose_param,
    vx_tensor outputs)
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        NULL,
        NULL,
        (vx_reference)outputs
    };
    vx_node node = VX_NULL;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_array perm = vxCreateArray(context, VX_TYPE_UINT32, transpose_params->dims_num);
    vx_scalar pnum = VX_NULL;

    gcmHEADER_ARG("graph=%p, input=%p, transpose_params=%p, size_of_transpose_param=0x%lx, output=%p",
        graph, inputs, transpose_params, size_of_transpose_param, outputs);
    gcmDUMP_API("$VX vxTensorTransposeNode2: graph=%p, input=%p, transpose_params=%p, size_of_transpose_param=0x%lx, output=%p",
        graph, inputs, transpose_params, size_of_transpose_param, outputs);

    if (!vxoArray_AllocateMemory(perm))
    {
        gcmFOOTER_NO();
        return VX_NULL;
    }
    else
    {
        memcpy(perm->memory.logicals[0], transpose_params->dims, sizeof(vx_int32) * transpose_params->dims_num);
    }

    pnum = vxCreateScalar(context, VX_TYPE_UINT32, &transpose_params->dims_num);
    if (vxoReference_GetStatus((vx_reference)pnum) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)pnum;
    }
    parameters[1] = (vx_reference)perm;
    parameters[2] = (vx_reference)pnum;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_TRANSPOSE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseArray(&perm);
    vxReleaseScalar(&pnum);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorMeanNode(
    vx_graph graph,
    vx_tensor inputs,
    const vx_nn_mean_params_t *mean_params,
    vx_size size_of_mean_param,
    vx_tensor outputs)
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        (vx_reference)mean_params->axis,
        NULL,
        (vx_reference)outputs
    };
    vx_node node = VX_NULL;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar keep_dims = vxCreateScalar(context, VX_TYPE_UINT32, &mean_params->keep_dims);

    gcmHEADER_ARG("graph=%p, input=%p, mean_params=%p, size_of_mean_param=0x%lx, output=%p",
        graph, inputs, mean_params, size_of_mean_param, outputs);
    gcmDUMP_API("$VX vxTensorMeanNode: graph=%p, input=%p, mean_params=%p, size_of_mean_param=0x%lx, output=%p",
        graph, inputs, mean_params, size_of_mean_param, outputs);

    if (vxoReference_GetStatus((vx_reference)keep_dims) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)keep_dims;
    }
    parameters[2] = (vx_reference)keep_dims;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_MEAN, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&keep_dims);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorSqueezeNode(
    vx_graph graph,
    vx_tensor inputs,
    const vx_nn_squeeze_params_t *squeeze_params,
    vx_size size_of_squeeze_param,
    vx_tensor outputs)
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        (vx_reference)squeeze_params->squeeze_dims,
        (vx_reference)outputs
    };
    vx_node node = VX_NULL;

    gcmHEADER_ARG("graph=%p, inputs=%p, squeeze_params=%p, size_of_squeeze_param=0x%lx, outputs=%p",
        graph, inputs, squeeze_params, size_of_squeeze_param, outputs);
    gcmDUMP_API("$VX vxTensorSqueezeNode: graph=%p, inputs=%p, squeeze_params=%p, size_of_squeeze_param=0x%lx, outputs=%p",
        graph, inputs, squeeze_params, size_of_squeeze_param, outputs);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_SQUEEZE, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorStrideSliceNode(
    vx_graph graph,
    vx_tensor inputs,
    const vx_nn_stride_slice_params_t *stride_slice_params,
    vx_size size_of_stride_slice_param,
    vx_tensor outputs)
{
    vx_reference parameters[] = {
        (vx_reference)inputs,
        (vx_reference)stride_slice_params->begin_dims,
        (vx_reference)stride_slice_params->end_dims,
        (vx_reference)stride_slice_params->stride_dims,
        NULL,
        NULL,
        NULL,
        (vx_reference)outputs
    };
    vx_node node = VX_NULL;
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar begin_mask = VX_NULL, end_mask = VX_NULL, shrink_axis_mask = VX_NULL;
    gcmHEADER_ARG("graph=%p, inputs=%p, stride_slice_params=%p, size_of_stride_slice_param=0x%lx, outputs=%p",
        graph, inputs, stride_slice_params, size_of_stride_slice_param, outputs);
    gcmDUMP_API("$VX vxTensorStrideSliceNode: graph=%p, inputs=%p, stride_slice_params=%p, size_of_stride_slice_param=0x%lx, outputs=%p",
        graph, inputs, stride_slice_params, size_of_stride_slice_param, outputs);

    begin_mask = vxCreateScalar(context, VX_TYPE_UINT32, &stride_slice_params->begin_mask);
    if (vxoReference_GetStatus((vx_reference)begin_mask) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)begin_mask;
    }
    end_mask = vxCreateScalar(context, VX_TYPE_UINT32, &stride_slice_params->end_mask);
    if (vxoReference_GetStatus((vx_reference)end_mask) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)end_mask;
    }
    shrink_axis_mask = vxCreateScalar(context, VX_TYPE_UINT32, &stride_slice_params->shrink_axis_mask);
    if (vxoReference_GetStatus((vx_reference)shrink_axis_mask) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)shrink_axis_mask;
    }
    parameters[4] = (vx_reference)begin_mask;
    parameters[5] = (vx_reference)end_mask;
    parameters[6] = (vx_reference)shrink_axis_mask;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_TENSOR_STRIDE_SLICE, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&begin_mask);
    vxReleaseScalar(&end_mask);
    vxReleaseScalar(&shrink_axis_mask);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxGRUUnitLayer(
    vx_graph graph,
    vx_tensor input,
    vx_tensor h_prev,
    const vx_nn_gru_params_t * gru_params,
    vx_size size_of_gru_params,
    vx_tensor output)
{
    vx_node node = VX_NULL;
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)h_prev,
        (vx_reference)gru_params->reset2input_weights,
        (vx_reference)gru_params->update2input_weights,
        (vx_reference)gru_params->connection2input_weights,
        (vx_reference)gru_params->reset2recurrent_weights,
        (vx_reference)gru_params->update2recurrent_weights,
        (vx_reference)gru_params->connection2recurrent_weights,
        (vx_reference)gru_params->gate_input_bias,
        (vx_reference)gru_params->gate_recurrent_bias,
        (vx_reference)gru_params->connection_bias,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, inputs=%p, gru_params=%p, size_of_gru_params=0x%lx, outputs=%p",
        graph, input, gru_params, size_of_gru_params, output);
    gcmDUMP_API("$VX vxGRUUnitLayer: graph=%p, inputs=%p, gru_params=%p, size_of_gru_params=0x%lx, outputs=%p",
        graph, input, gru_params, size_of_gru_params, output);


    if (size_of_gru_params == sizeof(vx_nn_gru_params_t))
    {

    }
    else
    {
        vxError(" size_of_gru_params doesn't match");
        gcmFOOTER_NO();
        return NULL;
    }

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_GRU_UNIT_LAYER, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxGRULayer(
    vx_graph graph,
    vx_tensor input,
    vx_tensor h_prev,
    const vx_nn_gru_params_t * gru_layer_params,
    vx_size size_of_gru_layer_params,
    vx_tensor output
    )
{

    vx_node node = VX_NULL;
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)h_prev,
        (vx_reference)gru_layer_params->reset2input_weights,
        (vx_reference)gru_layer_params->update2input_weights,
        (vx_reference)gru_layer_params->connection2input_weights,
        (vx_reference)gru_layer_params->reset2recurrent_weights,
        (vx_reference)gru_layer_params->update2recurrent_weights,
        (vx_reference)gru_layer_params->connection2recurrent_weights,
        (vx_reference)gru_layer_params->gate_input_bias,
        (vx_reference)gru_layer_params->gate_recurrent_bias,
        (vx_reference)gru_layer_params->connection_bias,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, inputs=%p, gru_layer_params=%p, size_of_gru_params=0x%lx, outputs=%p",
        graph, input, gru_layer_params, size_of_gru_layer_params, output);
    gcmDUMP_API("$VX vxGRULayer: graph=%p, inputs=%p, gru_layer_params=%p, size_of_gru_params=0x%lx, outputs=%p",
        graph, input, gru_layer_params, size_of_gru_layer_params, output);

    if (size_of_gru_layer_params == sizeof(vx_nn_gru_params_t))
    {
    }
    else
    {
        vxError(" size_of_gru_layer_params doesn't match");
        gcmFOOTER_NO();
        return NULL;
    }

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_GRU_LAYER, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConvLSTMUnitLayer(
    vx_graph graph,
    vx_tensor input,
    vx_tensor output_state_in,
    vx_tensor cell_state_in,
    const vx_nn_convlstm_params_t * convlstm_params,
    vx_size size_of_convlstm_params,
    vx_tensor output_state_out,
    vx_tensor cell_state_out,
    vx_tensor output)
{
    vx_node node = VX_NULL;
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output_state_in,
        (vx_reference)cell_state_in,
        (vx_reference)convlstm_params->input2input_weight,
        (vx_reference)convlstm_params->input2cell_weight,
        (vx_reference)convlstm_params->input2forget_weight,
        (vx_reference)convlstm_params->input2output_weight,
        (vx_reference)convlstm_params->recurrent2input_weight,
        (vx_reference)convlstm_params->recurrent2cell_weight,
        (vx_reference)convlstm_params->recurrent2forget_weight,
        (vx_reference)convlstm_params->recurrent2output_weight,
        (vx_reference)convlstm_params->input_gate_bias,
        (vx_reference)convlstm_params->cell_bias,
        (vx_reference)convlstm_params->forget_gate_bias,
        (vx_reference)convlstm_params->output_gate_bias,
        (vx_reference)convlstm_params->activation,
        (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &convlstm_params->forget_bias),
        (vx_reference)output_state_out,
        (vx_reference)cell_state_out,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output_state_in=%p, cell_state_in=%p, convlstm_params=%p, size_of_convlstm_params=0x%lx, output_state_out=%p, cell_state_out=%p, output=%p",
            graph, input, output_state_in, cell_state_in, convlstm_params, size_of_convlstm_params, output_state_out, cell_state_out, output);
    gcmDUMP_API("$VX vxConvLSTMUnitLayer: graph=%p, input=%p, output_state_in=%p, cell_state_in=%p, convlstm_params=%p, size_of_convlstm_params=0x%lx, output_state_out=%p, cell_state_out=%p, output=%p",
            graph, input, output_state_in, cell_state_in, convlstm_params, size_of_convlstm_params, output_state_out, cell_state_out, output);

    if (size_of_convlstm_params == sizeof(vx_nn_convlstm_params_t))
    {

    }
    else
    {
        vxError(" size_of_lstm_params doesn't match");
        gcmFOOTER_NO();
        return NULL;
    }

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONV_LSTM_UNIT_LAYER, parameters, vxmLENGTH_OF(parameters));


    if (parameters[16] != VX_NULL)
        vxReleaseScalar((vx_scalar*)&parameters[16]);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxConvLSTMLayer(
    vx_graph graph,
    vx_tensor input,
    vx_tensor static_input,
    vx_tensor cont,
    const vx_nn_convlstm_layer_params_t * convlstm_layer_params,
    vx_size size_of_convlstm_layer_params,
    vx_tensor output
    )
{

    vx_node node = VX_NULL;
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)static_input,
        (vx_reference)convlstm_layer_params->convlstm_param.input2input_weight,
        (vx_reference)convlstm_layer_params->convlstm_param.input2cell_weight,
        (vx_reference)convlstm_layer_params->convlstm_param.input2forget_weight,
        (vx_reference)convlstm_layer_params->convlstm_param.input2output_weight,
        (vx_reference)convlstm_layer_params->convlstm_param.recurrent2input_weight,
        (vx_reference)convlstm_layer_params->convlstm_param.recurrent2cell_weight,
        (vx_reference)convlstm_layer_params->convlstm_param.recurrent2forget_weight,
        (vx_reference)convlstm_layer_params->convlstm_param.recurrent2output_weight,
        (vx_reference)convlstm_layer_params->convlstm_param.input_gate_bias,
        (vx_reference)convlstm_layer_params->convlstm_param.cell_bias,
        (vx_reference)convlstm_layer_params->convlstm_param.forget_gate_bias,
        (vx_reference)convlstm_layer_params->convlstm_param.output_gate_bias,
        (vx_reference)convlstm_layer_params->convlstm_param.activation,
        (vx_reference)vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &convlstm_layer_params->convlstm_param.forget_bias),
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, static_input=%p, cont=%p, convlstm_layer_params=%p, size_of_convlstm_layer_params=0x%lx, output=%p",
            graph, input, static_input, cont, convlstm_layer_params, size_of_convlstm_layer_params, output);
    gcmDUMP_API("$VX vxConvLSTMLayer: graph=%p, input=%p, static_input=%p, cont=%p, convlstm_layer_params=%p, size_of_convlstm_layer_params=0x%lx, output=%p",
            graph, input, static_input, cont, convlstm_layer_params, size_of_convlstm_layer_params, output);

    if (size_of_convlstm_layer_params == sizeof(vx_nn_convlstm_layer_params_t))
    {
    }
    else
    {
        vxError(" size_of_lstm_params doesn't match");
        gcmFOOTER_NO();
        return NULL;
    }

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NN_CONV_LSTM_LAYER, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxMaxNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
    vx_reference parameters[] = {
       (vx_reference)in1,
       (vx_reference)in2,
       (vx_reference)out
    };
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);
    gcmDUMP_API("$VX vxMaxNode: graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_MAX, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxMinNode(vx_graph graph, vx_image in1, vx_image in2, vx_image out)
{
vx_reference parameters[] = {
    (vx_reference)in1,
    (vx_reference)in2,
    (vx_reference)out
};
    gcmHEADER_ARG("graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);
    gcmDUMP_API("$VX vxMinNode: graph=%p, in1=%p, in2=%p, out=%p", graph, in1, in2, out);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_MIN, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxNonMaxSuppressionNode(vx_graph graph, vx_image input, vx_image mask, vx_int32 win_size, vx_image output)
{
    vx_scalar scalar_win_size;
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)mask,
        VX_NULL,
        (vx_reference)output
    };
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, input=%p, mask=%p, win_size=0x%x, output=%p", graph, input, mask, win_size, output);
    gcmDUMP_API("$VX vxNonMaxSuppressionNode: graph=%p, input=%p, mask=%p, win_size=0x%x, output=%p", graph, input, mask, win_size, output);

    scalar_win_size = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &win_size);
    if (vxoReference_GetStatus((vx_reference)scalar_win_size) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_win_size;
    }
    parameters[2] = (vx_reference)scalar_win_size;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_NON_MAX_SUPPRESSION, parameters, vxmLENGTH_OF(parameters));
    vxReleaseScalar(&scalar_win_size);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxScalarOperationNode(vx_graph graph, vx_enum operation, vx_scalar a, vx_scalar b, vx_scalar output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar op = vxCreateScalar(context, VX_TYPE_ENUM, &operation);
    vx_reference params[] = {
            (vx_reference)op,
            (vx_reference)a,
            (vx_reference)b,
            (vx_reference)output,
    };
    vx_node node = vxoNode_CreateSpecific(graph,
                                           VX_KERNEL_SCALAR_OPERATION,
                                           params,
                                           vxmLENGTH_OF(params));
    gcmHEADER_ARG("graph=%p, operation=0x%x, a=%p, b=%p, output=%p", graph, operation, a, b, output);
    gcmDUMP_API("$VX vxScalarOperationNode: graph=%p, operation=0x%x, a=%p, b=%p, output=%p", graph, operation, a, b, output);

    vxReleaseScalar(&op);
    gcmFOOTER_NO();
    return node;

}

VX_API_ENTRY vx_node VX_API_CALL vxSelectNode(vx_graph graph, vx_scalar condition, vx_reference true_value, vx_reference false_value, vx_reference output)
{
    vx_reference parameters[] = {
        (vx_reference)true_value,
        (vx_reference)output,
        (vx_reference)false_value,
        (vx_reference)condition
    };
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, condition=%p, true_value=%p, false_value=%p, output=%p", graph, condition, true_value, false_value, output);
    gcmDUMP_API("$VX vxSelectNode: graph=%p, condition=%p, true_value=%p, false_value=%p, output=%p", graph, condition, true_value, false_value, output);
    node = vxoNode_CreateSpecific(graph, VX_KERNEL_SELECT, parameters, vxmLENGTH_OF(parameters));
    gcmFOOTER_NO();
    return node;
}

 VX_API_ENTRY vx_node VX_API_CALL vxMatchTemplateNode(vx_graph graph, vx_image src, vx_image templateImage, vx_enum matchingMethod, vx_image output)
{
    vx_scalar scalar_matchingMethod;
    vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)templateImage,
        VX_NULL,
        (vx_reference)output
    };
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, src=%p, templateImage=%p, matchingMethod=0x%x, output=%p", graph, src, templateImage, matchingMethod, output);
    gcmDUMP_API("$VX vxMatchTemplateNode: graph=%p, src=%p, templateImage=%p, matchingMethod=0x%x, output=%p", graph, src, templateImage, matchingMethod, output);

    scalar_matchingMethod = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &matchingMethod);
    if (vxoReference_GetStatus((vx_reference)scalar_matchingMethod) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_matchingMethod;
    }
    parameters[2] = (vx_reference)scalar_matchingMethod;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_MATCH_TEMPLATE, parameters, vxmLENGTH_OF(parameters));
    vxReleaseScalar(&scalar_matchingMethod);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxLBPNode(vx_graph graph, vx_image in, vx_enum format, vx_int8 kernel_size, vx_image out)
{
    vx_scalar scalar_format, scalar_kernel_size;
    vx_reference parameters[] = {
        (vx_reference)in,
        VX_NULL,
        VX_NULL,
        (vx_reference)out
    };
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, in=%p, format=0x%x, kernel_size=0x%x, out=%p", graph, in, format, kernel_size, out);
    gcmDUMP_API("$VX vxLBPNode: graph=%p, in=%p, format=0x%x, kernel_size=0x%x, out=%p", graph, in, format, kernel_size, out);
    scalar_format = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &format);
    if (vxoReference_GetStatus((vx_reference)scalar_format) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_format;
    }
    parameters[1] = (vx_reference)scalar_format;

    scalar_kernel_size = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT8, &kernel_size);
    if (vxoReference_GetStatus((vx_reference)scalar_kernel_size) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_kernel_size;
    }
    parameters[2] = (vx_reference)scalar_kernel_size;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_LBP, parameters, vxmLENGTH_OF(parameters));
    vxReleaseScalar(&scalar_format);
    vxReleaseScalar(&scalar_kernel_size);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxHOGCellsNode(vx_graph graph, vx_image input, vx_int32 cell_width, vx_int32 cell_height, vx_int32 num_bins, vx_tensor magnitudes, vx_tensor bins)
{
    vx_scalar scalar_cell_width, scalar_cell_height, scalar_num_bins, scalar_data_type;
    vx_int32 type, temp;
    vx_reference parameters[] = {
        (vx_reference)input,
        VX_NULL,
        VX_NULL,
        VX_NULL,
        (vx_reference)magnitudes,
        (vx_reference)bins,
        VX_NULL
    };
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, input=%p, cell_width=0x%x, cell_height=0x%x, num_bins=0x%x, magnitudes=%p, bins=%p", graph, input, cell_width, cell_height, num_bins, magnitudes, bins);
    gcmDUMP_API("$VX vxHOGCellsNode: graph=%p, input=%p, cell_width=0x%x, cell_height=0x%x, num_bins=0x%x, magnitudes=%p, bins=%p", graph, input, cell_width, cell_height, num_bins, magnitudes, bins);

    scalar_cell_width = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &cell_width);
    if (vxoReference_GetStatus((vx_reference)scalar_cell_width) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_cell_width;
    }
    parameters[1] = (vx_reference)scalar_cell_width;

    scalar_cell_height = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &cell_height);
    if (vxoReference_GetStatus((vx_reference)scalar_cell_height) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_cell_height;
    }
    parameters[2] = (vx_reference)scalar_cell_height;

    scalar_num_bins = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &num_bins);
    if (vxoReference_GetStatus((vx_reference)scalar_num_bins) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_num_bins;
    }
    parameters[3] = (vx_reference)scalar_num_bins;

    temp = 0;
    type = 1;
    vxQueryTensor(bins, VX_TENSOR_DATA_TYPE, &type, sizeof(type));
    if(type == VX_TYPE_INT16){
        temp = 2;
    }else if(type == VX_TYPE_INT8){
        temp = 1;
    }else{
        gcmFOOTER_NO();
        return node;
    }
    scalar_data_type = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &temp);
    if (vxoReference_GetStatus((vx_reference)scalar_data_type) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_data_type;
    }
    parameters[6] = (vx_reference)scalar_data_type;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_HOG_CELLS, parameters, vxmLENGTH_OF(parameters));
    vxReleaseScalar(&scalar_cell_width);
    vxReleaseScalar(&scalar_cell_height);
    vxReleaseScalar(&scalar_num_bins);
    vxReleaseScalar(&scalar_data_type);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxHOGFeaturesNode(vx_graph graph, vx_image input, vx_tensor magnitudes, vx_tensor bins, const vx_hog_t *params, vx_size hog_param_size, vx_tensor features)
{
    vx_scalar scalar_hog_param_size, scalar_data_type;
    vx_int32 type, temp;
    vx_array hog_param;
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)magnitudes,
        (vx_reference)bins,
        VX_NULL,
        VX_NULL,
        (vx_reference)features,
        VX_NULL,
    };
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, input=%p, magnitudes=%p, bins=%p, params=%p, hog_param_size=0x%lx, features=%p", graph, input, magnitudes, bins, params, hog_param_size, features);
    gcmDUMP_API("$VX vxHOGFeaturesNode: graph=%p, input=%p, magnitudes=%p, bins=%p, params=%p, hog_param_size=0x%lx, features=%p", graph, input, magnitudes, bins, params, hog_param_size, features);
    hog_param = vxCreateArray(vxGetContext((vx_reference)graph), VX_TYPE_HOG_PARAMS, 1);
    vxAddArrayItems(hog_param, 1, params, hog_param_size*sizeof(vx_hog_t));
    parameters[3] = (vx_reference)hog_param;

    scalar_hog_param_size = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &hog_param_size);
    if (vxoReference_GetStatus((vx_reference)scalar_hog_param_size) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_hog_param_size;
    }
    parameters[4] = (vx_reference)scalar_hog_param_size;

    temp = 1;
    type = 0;
    vxQueryTensor(bins, VX_TENSOR_DATA_TYPE, &type, sizeof(type));
    if(type == VX_TYPE_INT16){
        temp = 2;
    }else if(type == VX_TYPE_INT8){
        temp = 1;
    }else{
        gcmFOOTER_NO();
        return node;
    }
    scalar_data_type = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &temp);
    if (vxoReference_GetStatus((vx_reference)scalar_data_type) != VX_SUCCESS)
    {
        gcmFOOTER_NO();
        return (vx_node)scalar_data_type;
    }
    parameters[6] = (vx_reference)scalar_data_type;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_HOG_FEATURES, parameters, vxmLENGTH_OF(parameters));
    vxReleaseScalar(&scalar_hog_param_size);
    vxReleaseScalar(&scalar_data_type);
    vxReleaseArray(&hog_param);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxHoughLinesPNode(vx_graph graph, vx_image input, const vx_hough_lines_p_t *params, vx_array lines_array, vx_scalar num_lines)
{
    vx_array params_hough_lines_array;
    vx_reference parameters[] = {
        (vx_reference)input,
        VX_NULL,
        (vx_reference)lines_array,
        (vx_reference)num_lines
    };
    vx_node node = VX_NULL;

    gcmHEADER_ARG("graph=%p, input=%p, params=%p, lines_array=%p, num_lines=%p", graph, input, params, lines_array, num_lines);
    gcmDUMP_API("$VX vxHoughLinesPNode: graph=%p, input=%p, params=%p, lines_array=%p, num_lines=%p", graph, input, params, lines_array, num_lines);

    params_hough_lines_array = vxCreateArray(vxGetContext((vx_reference)graph), VX_TYPE_HOUGH_LINES_PARAMS, 1);
    vxAddArrayItems(params_hough_lines_array, 1, params, sizeof(vx_hough_lines_p_t));
    parameters[1] = (vx_reference)params_hough_lines_array;

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_HOUGH_LINES_P, parameters, vxmLENGTH_OF(parameters));
    vxReleaseArray(&params_hough_lines_array);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxBilateralFilterNode(vx_graph graph, vx_tensor src, vx_int32 diameter, vx_float32 sigmaSpace, vx_float32 sigmaValues, vx_tensor dst)
{
    vx_scalar dia = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_INT32, &diameter);
    vx_scalar sSpa = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &sigmaSpace);
    vx_scalar sVal = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_FLOAT32, &sigmaValues);
    vx_reference parameters[] = {
        (vx_reference)src,
        (vx_reference)dia,
        (vx_reference)sSpa,
        (vx_reference)sVal,
        (vx_reference)dst,
    };
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, src=%p, diameter=0x%x, sigmaSpace=%f, sigmaValues=%f, dst=%p", graph, src, diameter, sigmaSpace, sigmaValues, dst);
    gcmDUMP_API("$VX vxBilateralFilterNode: graph=%p, src=%p, diameter=0x%x, sigmaSpace=%f, sigmaValues=%f, dst=%p", graph, src, diameter, sigmaSpace, sigmaValues, dst);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_BILATERAL_FILTER, parameters, vxmLENGTH_OF(parameters));
    vxReleaseScalar(&dia);
    vxReleaseScalar(&sSpa);
    vxReleaseScalar(&sVal);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorConvertDepthNode(vx_graph graph, vx_tensor input, vx_enum policy, vx_scalar norm, vx_scalar offset, vx_tensor output)
{
    vx_scalar overflow_policy_sc = vxCreateScalar(vxGetContext((vx_reference)graph), VX_TYPE_ENUM, &policy);
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)overflow_policy_sc,
        (vx_reference)norm,
        (vx_reference)offset,
        (vx_reference)output,
    };
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, input=%p, policy=0x%x, norm=%p, offset=%p, output=%p", graph, input, policy, norm, offset, output);
    gcmDUMP_API("$VX vxTensorConvertDepthNode: graph=%p, input=%p, policy=0x%x, norm=%p, offset=%p, output=%p", graph, input, policy, norm, offset, output);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_CONVERT_DEPTH, parameters, vxmLENGTH_OF(parameters));
    vxReleaseScalar(&overflow_policy_sc);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxTensorMatrixMultiplyNode(vx_graph graph, vx_tensor input1, vx_tensor input2, vx_tensor input3,
    const vx_tensor_matrix_multiply_params_t *matrix_multiply_params, vx_tensor output)
{
    vx_context context = vxGetContext((vx_reference)graph);
    vx_scalar transpose_src1 = vxCreateScalar(context, VX_TYPE_BOOL, &matrix_multiply_params->transpose_input1);
    vx_scalar transpose_src2 = vxCreateScalar(context, VX_TYPE_BOOL, &matrix_multiply_params->transpose_input2);
    vx_scalar transpose_src3 = vxCreateScalar(context, VX_TYPE_BOOL, &matrix_multiply_params->transpose_input3);

    vx_reference parameters[] = {
        (vx_reference)input1,
        (vx_reference)input2,
        (vx_reference)input3,
        (vx_reference)transpose_src1,
        (vx_reference)transpose_src2,
        (vx_reference)transpose_src3,
        (vx_reference)output,
    };
    vx_node node = VX_NULL;
    gcmHEADER_ARG("graph=%p, input1=%p, input2=%p, input3=%p, matrix_multiply_params=%p, output=%p", graph, input1, input2, input3, matrix_multiply_params, output);
    gcmDUMP_API("$VX vxTensorMatrixMultiplyNode: graph=%p, input1=%p, input2=%p, input3=%p, matrix_multiply_params=%p, output=%p", graph, input1, input2, input3, matrix_multiply_params, output);

    node = vxoNode_CreateSpecific(graph, VX_KERNEL_TENSOR_MATRIX_MULTIPLY, parameters, vxmLENGTH_OF(parameters));

    vxReleaseScalar(&transpose_src1);
    vxReleaseScalar(&transpose_src2);
    vxReleaseScalar(&transpose_src3);
    gcmFOOTER_NO();
    return node;
}

VX_API_ENTRY vx_node VX_API_CALL vxCopyNode(vx_graph graph, vx_reference input, vx_reference output)
{
    vx_reference parameters[] = {
        (vx_reference)input,
        (vx_reference)output
    };
    gcmHEADER_ARG("graph=%p, input=%p, output=%p", graph, input, output);
    gcmDUMP_API("$VX vxCopyNode: graph=%p, input=%p, output=%p", graph, input, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_COPY, parameters, vxmLENGTH_OF(parameters));
}

VX_API_ENTRY vx_node VX_API_CALL vxWeightedAverageNode(vx_graph graph, vx_image input0, vx_scalar alpha, vx_image input1, vx_image output)
{
    vx_reference parameters[] = {
        (vx_reference)input0,
        (vx_reference)alpha,
        (vx_reference)input1,
        (vx_reference)output
    };

    gcmHEADER_ARG("graph=%p, input0=%p, alpha=%p, input1=%p, output=%p", graph, input0, alpha, input1, output);
    gcmDUMP_API("$VX vxWeightedAverageKernelNode: graph=%p, input0=%p, alpha=%p, input1=%p, output=%p", graph, input0, alpha, input1, output);

    gcmFOOTER_NO();
    return vxoNode_CreateSpecific(graph, VX_KERNEL_WEIGHTED_AVERAGE, parameters, vxmLENGTH_OF(parameters));
}

