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


#ifndef __GC_VX_BE_KERNEL_H__
#define __GC_VX_BE_KERNEL_H__


#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
****************************** Object Declarations *****************************
\******************************************************************************/

typedef enum _gceVX_KERNEL
{
    gcvVX_KERNEL_COLOR_CONVERT,
    gcvVX_KERNEL_CHANNEL_EXTRACT,
    gcvVX_KERNEL_CHANNEL_COMBINE,
    gcvVX_KERNEL_SOBEL_3x3,
    gcvVX_KERNEL_MAGNITUDE,
    gcvVX_KERNEL_PHASE,
    gcvVX_KERNEL_SCALE_IMAGE,
    gcvVX_KERNEL_TABLE_LOOKUP,
    gcvVX_KERNEL_HISTOGRAM,
    gcvVX_KERNEL_EQUALIZE_HISTOGRAM,
    gcvVX_KERNEL_ABSDIFF,
    gcvVX_KERNEL_MEAN_STDDEV,
    gcvVX_KERNEL_THRESHOLD,
    gcvVX_KERNEL_INTEGRAL_IMAGE,
    gcvVX_KERNEL_DILATE_3x3,
    gcvVX_KERNEL_ERODE_3x3,
    gcvVX_KERNEL_MEDIAN_3x3,
    gcvVX_KERNEL_BOX_3x3,
    gcvVX_KERNEL_GAUSSIAN_3x3,
    gcvVX_KERNEL_CUSTOM_CONVOLUTION,
    gcvVX_KERNEL_GAUSSIAN_PYRAMID,
    gcvVX_KERNEL_ACCUMULATE,
    gcvVX_KERNEL_ACCUMULATE_WEIGHTED,
    gcvVX_KERNEL_ACCUMULATE_SQUARE,
    gcvVX_KERNEL_MINMAXLOC,
    gcvVX_KERNEL_CONVERTDEPTH,
    gcvVX_KERNEL_CANNY_EDGE_DETECTOR,
    gcvVX_KERNEL_AND,
    gcvVX_KERNEL_OR,
    gcvVX_KERNEL_XOR,
    gcvVX_KERNEL_NOT,
    gcvVX_KERNEL_MULTIPLY,
    gcvVX_KERNEL_ADD,
    gcvVX_KERNEL_SUBTRACT,
    gcvVX_KERNEL_WARP_AFFINE,
    gcvVX_KERNEL_WARP_PERSPECTIVE,
    gcvVX_KERNEL_HARRIS_CORNERS,
    gcvVX_KERNEL_FAST_CORNERS,
    gcvVX_KERNEL_OPTICAL_FLOW_PYR_LK,
    gcvVX_KERNEL_REMAP,
    gcvVX_KERNEL_HALFSCALE_GAUSSIAN,

    gcvVX_KERNEL_SCHARR_3x3,
    gcvVX_KERNEL_ELEMENTWISE_NORM,
    gcvVX_KERNEL_NONMAXSUPPRESSION,
    gcvVX_KERNEL_EUCLIDEAN_NONMAXSUPPRESSION,
    gcvVX_KERNEL_EDGE_TRACE,
    gcvVX_KERNEL_IMAGE_LISTER,
}
gceVX_KERNEL;

typedef enum _gceVX_InterPolicy
{
    gcvVX_INTERPOLATION_TYPE_NEAREST_NEIGHBOR,
    gcvVX_INTERPOLATION_TYPE_BILINEAR,
    gcvVX_INTERPOLATION_TYPE_AREA,
}
gceVX_InterPolicy;

typedef enum _gceVX_BorderMode
{
    gcvVX_BORDER_MODE_UNDEFINED,
    gcvVX_BORDER_MODE_CONSTANT,
    gcvVX_BORDER_MODE_REPLACEMENT,
}
gceVX_BorderMode;

#ifdef __cplusplus
}
#endif

#endif /* __gc_vx_be_kernel_h_ */
