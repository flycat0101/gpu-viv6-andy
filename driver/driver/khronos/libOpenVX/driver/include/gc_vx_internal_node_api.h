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


#ifndef __GC_VX_INTERNAL_NODE_API_H__
#define __GC_VX_INTERNAL_NODE_API_H__

EXTERN_C_BEGIN

#include <gc_vx_common.h>
#include <VX/vx_lib_debug.h>

VX_INTERNAL_API vx_node vxSobelMxNNode(vx_graph graph, vx_image input, vx_scalar ws, vx_image gx, vx_image gy);

VX_INTERNAL_API vx_node vxHarrisScoreNode(
        vx_graph graph, vx_image gx, vx_image gy, vx_scalar sensitivity, vx_scalar blockSize, vx_image score);

VX_INTERNAL_API vx_node vxEuclideanNonMaxNode(
        vx_graph graph, vx_image input, vx_scalar strengthThresh, vx_scalar minDistance, vx_image output);

VX_INTERNAL_API vx_node vxImageListerNode(vx_graph graph, vx_image input, vx_array arr, vx_scalar numPoints);

VX_INTERNAL_API vx_node vxElementwiseNormNode(
        vx_graph graph, vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output);

VX_INTERNAL_API vx_node vxNonMaxSuppressionNode(vx_graph graph, vx_image mag, vx_image phase, vx_image edge);

VX_INTERNAL_API vx_node vxEdgeTraceNode(vx_graph graph, vx_image norm, vx_threshold threshold, vx_image output);

EXTERN_C_END

#endif /* __GC_VX_INTERNAL_NODE_API_H__ */
