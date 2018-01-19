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


#include <VX/vx.h>
#include <VX/vxu.h>

#ifndef gcmCOUNTOF
#define gcmCOUNTOF(array)                 (sizeof(array) / sizeof((array)[0]))
#endif

VX_API_ENTRY vx_status VX_API_CALL vxuColorConvert(vx_context context, vx_image src, vx_image dst)
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

VX_API_ENTRY vx_status VX_API_CALL vxuChannelExtract(vx_context context, vx_image src, vx_enum channel, vx_image dst)
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

VX_API_ENTRY vx_status VX_API_CALL vxuChannelCombine(vx_context context,
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

static const vx_enum vx_border_modes_2_e[] =
{
    VX_BORDER_UNDEFINED,
    VX_BORDER_CONSTANT
};

static const vx_enum vx_border_modes_3_e[] =
{
    VX_BORDER_UNDEFINED,
    VX_BORDER_CONSTANT,
    VX_BORDER_REPLICATE
};

static vx_status vxIsBorderModeSupported(vx_enum borderMode, const vx_enum supportedModes[], vx_size modeCount)
{
    vx_uint32 index = 0;

    for ( index = 0; index < modeCount; index++)
    {
        if (borderMode == supportedModes[index]) return VX_SUCCESS;
    }
    return VX_ERROR_NOT_SUPPORTED;
}

static vx_status vx_useImmediateBorderMode(vx_context context, vx_node node, const vx_enum supportedModes[], vx_size modeCount)
{
    vx_border_t border;
    vx_status        status = VX_FAILURE;

    status = vxQueryContext(context, VX_CONTEXT_IMMEDIATE_BORDER, &border, sizeof(border));

    if (status != VX_SUCCESS) return status;

    status = vxIsBorderModeSupported(border.mode, supportedModes, modeCount);

    if (status == VX_ERROR_NOT_SUPPORTED)
    {
        vx_enum policy;

        status = vxQueryContext(context, VX_CONTEXT_IMMEDIATE_BORDER_POLICY, &policy, sizeof(policy));

        if (status != VX_SUCCESS) return status;

        switch (policy)
        {
            case VX_BORDER_POLICY_DEFAULT_TO_UNDEFINED:
                border.mode = VX_BORDER_UNDEFINED;
                status = VX_SUCCESS;
                break;
            case VX_BORDER_POLICY_RETURN_ERROR:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
            default:
                status = VX_ERROR_NOT_SUPPORTED;
                break;
        }
    }

    return status == VX_SUCCESS ? vxSetNodeAttribute(node, VX_NODE_BORDER, &border, sizeof(border)) : status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuSobel3x3(vx_context context, vx_image src, vx_image output_x, vx_image output_y)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxSobel3x3Node(graph, src, output_x, output_y);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_3_e, gcmCOUNTOF(vx_border_modes_3_e));
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

VX_API_ENTRY vx_status VX_API_CALL vxuMagnitude(vx_context context, vx_image grad_x, vx_image grad_y, vx_image dst)
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

VX_API_ENTRY vx_status VX_API_CALL vxuPhase(vx_context context, vx_image grad_x, vx_image grad_y, vx_image dst)
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

VX_API_ENTRY vx_status VX_API_CALL vxuScaleImage(vx_context context, vx_image src, vx_image dst, vx_enum type)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxScaleImageNode(graph, src, dst, type);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_3_e, gcmCOUNTOF(vx_border_modes_3_e));
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

VX_API_ENTRY vx_status VX_API_CALL vxuTableLookup(vx_context context, vx_image input, vx_lut lut, vx_image output)
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

VX_API_ENTRY vx_status VX_API_CALL vxuHistogram(vx_context context, vx_image input, vx_distribution distribution)
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

VX_API_ENTRY vx_status VX_API_CALL vxuEqualizeHist(vx_context context, vx_image input, vx_image output)
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

VX_API_ENTRY vx_status VX_API_CALL vxuAbsDiff(vx_context context, vx_image in1, vx_image in2, vx_image out)
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

VX_API_ENTRY vx_status VX_API_CALL vxuMeanStdDev(vx_context context, vx_image input, vx_float32 *mean, vx_float32 *stddev)
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
        vxCopyScalar(s_mean, (void*)mean, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
        vxCopyScalar(s_stddev, (void*)stddev, VX_READ_ONLY, VX_MEMORY_TYPE_HOST);
    }

    vxReleaseNode(&node);
    vxReleaseScalar(&s_mean);
    vxReleaseScalar(&s_stddev);
    vxReleaseGraph(&graph);

    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuThreshold(vx_context context, vx_image input, vx_threshold thresh, vx_image output)
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

VX_API_ENTRY vx_status VX_API_CALL vxuIntegralImage(vx_context context, vx_image input, vx_image output)
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

VX_API_ENTRY vx_status VX_API_CALL vxuErode3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxErode3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_3_e, gcmCOUNTOF(vx_border_modes_3_e));

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

VX_API_ENTRY vx_status VX_API_CALL vxuDilate3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxDilate3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_3_e, gcmCOUNTOF(vx_border_modes_3_e));
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

VX_API_ENTRY vx_status VX_API_CALL vxuMedian3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxMedian3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_3_e, gcmCOUNTOF(vx_border_modes_3_e));
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

VX_API_ENTRY vx_status VX_API_CALL vxuBox3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxBox3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_3_e, gcmCOUNTOF(vx_border_modes_3_e));

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

VX_API_ENTRY vx_status VX_API_CALL vxuGaussian3x3(vx_context context, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxGaussian3x3Node(graph, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_3_e, gcmCOUNTOF(vx_border_modes_3_e));

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

VX_API_ENTRY vx_status VX_API_CALL vxuNonLinearFilter(vx_context context, vx_enum function, vx_image input, vx_matrix mask, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph graph = vxCreateGraph(context);
    if (vxGetStatus((vx_reference)graph) == VX_SUCCESS)
    {
        vx_node node = vxNonLinearFilterNode(graph, function, input, mask, output);
        if (vxGetStatus((vx_reference)node) == VX_SUCCESS)
        {
            status = vx_useImmediateBorderMode(context, node, vx_border_modes_3_e, gcmCOUNTOF(vx_border_modes_3_e));

            if (status == VX_SUCCESS)
                status = vxVerifyGraph(graph);

            if (status == VX_SUCCESS)
                status = vxProcessGraph(graph);

            vxReleaseNode(&node);
        }
        vxReleaseGraph(&graph);
    }
    return status;
}

VX_API_ENTRY vx_status VX_API_CALL vxuConvolve(vx_context context, vx_image input, vx_convolution conv, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxConvolveNode(graph, input, conv, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_3_e, gcmCOUNTOF(vx_border_modes_3_e));
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

VX_API_ENTRY vx_status VX_API_CALL vxuGaussianPyramid(vx_context context, vx_image input, vx_pyramid gaussian)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

   graph = vxCreateGraph(context);
   if (graph == NULL) return VX_FAILURE;

    node = vxGaussianPyramidNode(graph, input, gaussian);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_2_e, gcmCOUNTOF(vx_border_modes_2_e));

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

VX_API_ENTRY vx_status VX_API_CALL vxuAccumulateImage(vx_context context, vx_image input, vx_image accum)
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

VX_API_ENTRY vx_status VX_API_CALL vxuAccumulateWeightedImage(vx_context context, vx_image input, vx_scalar scale, vx_image accum)
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

VX_API_ENTRY vx_status VX_API_CALL vxuAccumulateSquareImage(vx_context context, vx_image input, vx_scalar scale, vx_image accum)
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

VX_API_ENTRY vx_status VX_API_CALL vxuMinMaxLoc(vx_context context, vx_image input,
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

VX_API_ENTRY vx_status VX_API_CALL vxuConvertDepth(vx_context context, vx_image input, vx_image output, vx_enum policy, vx_int32 shift)
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

VX_API_ENTRY vx_status VX_API_CALL vxuCannyEdgeDetector(vx_context context, vx_image input, vx_threshold hyst,
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

VX_API_ENTRY vx_status VX_API_CALL vxuHalfScaleGaussian(vx_context context, vx_image input, vx_image output, vx_int32 kernel_size)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxHalfScaleGaussianNode(graph, input, output, kernel_size);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_2_e, gcmCOUNTOF(vx_border_modes_2_e));

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

VX_API_ENTRY vx_status VX_API_CALL vxuAnd(vx_context context, vx_image in1, vx_image in2, vx_image out)
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

VX_API_ENTRY vx_status VX_API_CALL vxuOr(vx_context context, vx_image in1, vx_image in2, vx_image out)
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

VX_API_ENTRY vx_status VX_API_CALL vxuXor(vx_context context, vx_image in1, vx_image in2, vx_image out)
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

VX_API_ENTRY vx_status VX_API_CALL vxuNot(vx_context context, vx_image input, vx_image out)
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

VX_API_ENTRY vx_status VX_API_CALL vxuMultiply(vx_context context, vx_image in1, vx_image in2, vx_float32 scale, vx_enum overflow_policy, vx_enum rounding_policy, vx_image out)
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

VX_API_ENTRY vx_status VX_API_CALL vxuAdd(vx_context context, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
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

VX_API_ENTRY vx_status VX_API_CALL vxuSubtract(vx_context context, vx_image in1, vx_image in2, vx_enum policy, vx_image out)
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

VX_API_ENTRY vx_status VX_API_CALL vxuWarpAffine(vx_context context, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxWarpAffineNode(graph, input, matrix, type, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_2_e, gcmCOUNTOF(vx_border_modes_2_e));

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

VX_API_ENTRY vx_status VX_API_CALL vxuWarpPerspective(vx_context context, vx_image input, vx_matrix matrix, vx_enum type, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxWarpPerspectiveNode(graph, input, matrix, type, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_2_e, gcmCOUNTOF(vx_border_modes_2_e));

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

VX_API_ENTRY vx_status VX_API_CALL vxuHarrisCorners(vx_context context, vx_image input,
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

VX_API_ENTRY vx_status VX_API_CALL vxuFastCorners(vx_context context, vx_image input, vx_scalar sens, vx_bool nonmax, vx_array corners, vx_scalar num_corners)
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

VX_API_ENTRY vx_status VX_API_CALL vxuOpticalFlowPyrLK(vx_context context, vx_pyramid old_images,
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

VX_API_ENTRY vx_status VX_API_CALL vxuRemap(vx_context context, vx_image input, vx_remap table, vx_enum policy, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

    graph = vxCreateGraph(context);
    if (graph == NULL) return VX_FAILURE;

    node = vxRemapNode(graph, input, table, policy, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_2_e, gcmCOUNTOF(vx_border_modes_2_e));

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

VX_API_ENTRY vx_status VX_API_CALL vxuLaplacianPyramid(vx_context context, vx_image input, vx_pyramid laplacian, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

   graph = vxCreateGraph(context);
   if (graph == NULL) return VX_FAILURE;

    node = vxLaplacianPyramidNode(graph, input, laplacian, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_2_e, gcmCOUNTOF(vx_border_modes_2_e));

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


VX_API_ENTRY vx_status VX_API_CALL vxuLaplacianReconstruct(vx_context context, vx_pyramid laplacian, vx_image input, vx_image output)
{
    vx_status status = VX_FAILURE;
    vx_graph  graph  = NULL;
    vx_node   node   = NULL;

   graph = vxCreateGraph(context);
   if (graph == NULL) return VX_FAILURE;

    node = vxLaplacianReconstructNode(graph, laplacian, input, output);
    if (node == NULL) return VX_FAILURE;

    status = vx_useImmediateBorderMode(context, node, vx_border_modes_2_e, gcmCOUNTOF(vx_border_modes_2_e));

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
