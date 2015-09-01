/****************************************************************************
*
*    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
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
