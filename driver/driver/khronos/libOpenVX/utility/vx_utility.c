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


#include <VX/vx.h>
#include <VX/vxu.h>

vx_status vxuColorConvert(vx_context context, vx_image src, vx_image dst)
{
    vx_status status = VX_FAILURE;
    vx_node   node = NULL;
    vx_graph  graph = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxColorConvertNode(graph, src, dst);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuChannelExtract(vx_context context, vx_image src, vx_enum channel, vx_image dst)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxChannelExtractNode(graph, src, channel, dst);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuChannelCombine(vx_context context,
                            vx_image plane0,
                            vx_image plane1,
                            vx_image plane2,
                            vx_image plane3,
                            vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxChannelCombineNode(graph, plane0, plane1, plane2, plane3, output);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

static vx_status vx_useImmediateBorderMode(vx_context context, vx_node node)
{
    vx_border_mode_t border;
    vx_status        status = VX_FAILURE;

    if (vxQueryContext(context, VX_CONTEXT_ATTRIBUTE_IMMEDIATE_BORDER_MODE, &border, sizeof(border)) == VX_SUCCESS)
    {
        status = vxSetNodeAttribute(node, VX_NODE_ATTRIBUTE_BORDER_MODE, &border, sizeof(border));
    }

    return status;
}

vx_status vxuSobel3x3(vx_context context, vx_image src, vx_image output_x, vx_image output_y)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxSobel3x3Node(graph, src, output_x, output_y);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);
    if (status == VX_SUCCESS)
    {
        status = vxVerifyGraph(graph);
    }

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuMagnitude(vx_context context, vx_image grad_x, vx_image grad_y, vx_image dst)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxMagnitudeNode(graph, grad_x, grad_y, dst);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuPhase(vx_context context, vx_image grad_x, vx_image grad_y, vx_image dst)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxPhaseNode(graph, grad_x, grad_y, dst);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuScaleImage(vx_context context, vx_image src, vx_image dst, vx_enum type)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxScaleImageNode(graph, src, dst, type);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);
    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuTableLookup(vx_context context, vx_image input, vx_lut lut, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxTableLookupNode(graph, input, lut, output);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuHistogram(vx_context context, vx_image input, vx_distribution distribution)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxHistogramNode(graph, input, distribution);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuEqualizeHist(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxEqualizeHistNode(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuAbsDiff(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxAbsDiffNode(graph, in1, in2, out);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuMeanStdDev(vx_context context, vx_image input, vx_float32 *mean, vx_float32 *stddev)
{
    vx_status status   = VX_FAILURE;
    vx_graph  graph    = NULL;
    vx_node   node     = NULL;
    vx_scalar s_mean   = NULL;
    vx_scalar s_stddev = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    s_mean = vxCreateScalar(context, VX_TYPE_FLOAT32, NULL);
    s_stddev = vxCreateScalar(context, VX_TYPE_FLOAT32, NULL);
    node = vxMeanStdDevNode(graph, input, s_mean, s_stddev);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
        vxAccessScalarValue(s_mean, mean);
        vxAccessScalarValue(s_stddev, stddev);
    }

    vxReleaseNode(&node);
    vxReleaseScalar(&s_mean);
    vxReleaseScalar(&s_stddev);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuThreshold(vx_context context, vx_image input, vx_threshold thresh, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxThresholdNode(graph, input, thresh, output);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuIntegralImage(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxIntegralImageNode(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuErode3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxErode3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);

    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuDilate3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxDilate3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);
    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuMedian3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxMedian3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);
    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }
    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuBox3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxBox3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);

    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuGaussian3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxGaussian3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);

    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuConvolve(vx_context context, vx_image input, vx_convolution conv, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxConvolveNode(graph, input, conv, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);
    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuGaussianPyramid(vx_context context, vx_image input, vx_pyramid gaussian)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

   graph = vxCreateGraph(context);
   if (graph == NULL) return VX_FAILURE;

    node = vxGaussianPyramidNode(graph, input, gaussian);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);

    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuAccumulateImage(vx_context context, vx_image input, vx_image accum)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxAccumulateImageNode(graph, input, accum);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuAccumulateWeightedImage(vx_context context, vx_image input, vx_scalar scale, vx_image accum)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxAccumulateWeightedImageNode(graph, input, scale, accum);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuAccumulateSquareImage(vx_context context, vx_image input, vx_scalar scale, vx_image accum)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxAccumulateSquareImageNode(graph, input, scale, accum);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuMinMaxLoc(vx_context context, vx_image input,
                        vx_scalar minVal, vx_scalar maxVal,
                        vx_array minLoc, vx_array maxLoc,
                        vx_scalar minCount, vx_scalar maxCount)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxMinMaxLocNode(graph, input, minVal, maxVal, minLoc, maxLoc, minCount, maxCount);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuConvertDepth(vx_context context, vx_image input, vx_image output, vx_enum policy, vx_int32 shift)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;
    vx_scalar sshift = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    sshift = vxCreateScalar(context, VX_TYPE_INT32, &shift);

    node = vxConvertDepthNode(graph, input, output, policy, sshift);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);
    vxReleaseScalar(&sshift);
    return status;
}

vx_status vxuCannyEdgeDetector(vx_context context, vx_image input, vx_threshold hyst,
                               vx_int32 gradient_size, vx_enum norm_type,
                               vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxCannyEdgeDetectorNode(graph, input, hyst, gradient_size, norm_type, output);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }
    vxReleaseNode(&node);

    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuHalfScaleGaussian(vx_context context, vx_image input, vx_image output, vx_int32 kernel_size)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxHalfScaleGaussianNode(graph, input, output, kernel_size);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);

    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuAnd(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxAndNode(graph, in1, in2, out);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuOr(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxOrNode(graph, in1, in2, out);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuXor(vx_context context, vx_image in1, vx_image in2, vx_image out)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxXorNode(graph, in1, in2, out);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuNot(vx_context context, vx_image input, vx_image out)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxNotNode(graph, input, out);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuMultiply(vx_context context, vx_image in1, vx_image in2, vx_float32 scale, vx_enum overflow_policy, vx_enum rounding_policy, vx_image out)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;
    vx_scalar sscale = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    sscale = vxCreateScalar(context, VX_TYPE_FLOAT32, &scale);

    node = vxMultiplyNode(graph, in1, in2, sscale, overflow_policy, rounding_policy, out);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);
    vxReleaseScalar(&sscale);

    return status;
}

vx_status vxuAdd(vx_context context, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxAddNode(graph, in1, in2, policy, out);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuSubtract(vx_context context, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxSubtractNode(graph, in1, in2, policy, out);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuWarpAffine(vx_context context, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxWarpAffineNode(graph, input, matrix, type, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);

    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuWarpPerspective(vx_context context, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxWarpPerspectiveNode(graph, input, matrix, type, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);

    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuHarrisCorners(vx_context context, vx_image input,
        vx_scalar strength_thresh,
        vx_scalar min_distance,
        vx_scalar sensitivity,
        vx_int32 gradient_size,
        vx_int32 block_size,
        vx_array corners,
        vx_scalar num_corners)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxHarrisCornersNode(graph, input, strength_thresh, min_distance, sensitivity, gradient_size, block_size, corners, num_corners);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuFastCorners(vx_context context, vx_image input, vx_scalar sens, vx_bool nonmax, vx_array corners, vx_scalar num_corners)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxFastCornersNode(graph, input, sens, nonmax, corners, num_corners);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuOpticalFlowPyrLK(vx_context context, vx_pyramid old_images,
                              vx_pyramid new_images,
                              vx_array old_points,
                              vx_array new_points_estimates,
                              vx_array new_points,
                              vx_enum termination,
                              vx_scalar epsilon,
                              vx_scalar num_iterations,
                              vx_scalar use_initial_estimate,
                              vx_size window_dimension)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxOpticalFlowPyrLKNode(graph, old_images, new_images, old_points,new_points_estimates, new_points,
                termination,epsilon,num_iterations,use_initial_estimate,window_dimension);
    if (node == NULL) return VX_FAILURE;

    status = vxVerifyGraph(graph);
    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}

vx_status vxuRemap(vx_context context, vx_image input, vx_remap table, vx_enum policy, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxRemapNode(graph, input, table, policy, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node);

    if (status == VX_SUCCESS)
        status = vxVerifyGraph(graph);

    if (status == VX_SUCCESS)
    {
        status = vxProcessGraph(graph);
    }

    vxReleaseNode(&node);
    vxReleaseGraph(&graph);

    return status;
}
