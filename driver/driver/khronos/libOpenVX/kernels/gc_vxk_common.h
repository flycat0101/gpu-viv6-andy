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



#ifndef __GC_VXK_COMMON_H__
#define __GC_VXK_COMMON_H__

#include <gc_hal.h>
#include <gc_hal_user.h>

#include "gc_hal_vx.h"

#include "gc_vxk_be_kernel.h"

#define _GC_OBJ_ZONE            gcvZONE_API_VX

#include <VX/vx.h>
#include <gc_vx_common.h>
#include <math.h>

#include <assert.h>


/*! \brief The largest convolution matrix the specification requires support for is 15x15.
 */
#define C_MAX_CONVOLUTION_DIM (15)

#ifdef __cplusplus
extern "C" {
#endif

#define FORMAT_VALUE(value) ((value) | ((value) << 8) | ((value) << 16) | ((value) << 24))

#define GC_VX_UNIFORM_SH    0x30000
#define GC_VX_UNIFORM_GPIPE 0x34000
#define GC_VX_UNIFORM_PIXEL 0x36000

#define GC_VX_MAX_ARRAY 10

typedef struct _gcoVX_Index
{
    vx_uint32 index;
    vx_uint32 num;
    vx_uint32 bin[4];
}
gcoVX_Index;

#define FV(t) ((t) | ((t) << 8) | ((t) << 16) | ((t) << 24))
#define FV2(t) ((t) | ((t) << 16))
#define FV4(a0, a8, a16, a24) ((a0) | ((a8) << 8) | ((a16) << 16) | ((a24) << 24))

#define GC_VX_INDEX_AUTO (~0U)

#define VIV_HARRIS_SCORE        1
#define VIV_EUCLIDEAN_NONMAX    1
#define VIV_SOBEL_MXN           1

#define VIV_NONMAX_SUPPRESSION  1
#define VIV_NORM                1
#define VIV_EDGE                1


typedef enum
{
    GC_VX_CONTEXT_OBJECT_IMAGE_INPUT = 0,
    GC_VX_CONTEXT_OBJECT_IMAGE_OUTPUT,
    GC_VX_CONTEXT_OBJECT_DISTRIBUTION,
    GC_VX_CONTEXT_OBJECT_REMAP,
    GC_VX_CONTEXT_OBJECT_LUT,
    GC_VX_CONTEXT_OBJECT_SCALAR,
    GC_VX_CONTEXT_OBJECT_ARRAY,
}
gc_vx_object_type_t;

typedef struct _gcoVX_Kernel_Context_Object
{
    gc_vx_object_type_t type;

	void*				obj;
    gcsVX_IMAGE_INFO    info;
    vx_uint32           index;
    vx_uint32			num;
}
gcoVX_Kernel_Context_Object;

typedef struct _gcoVX_Kernel_Context
{
    gcsVX_KERNEL_PARAMETERS         params;

    gcoVX_Kernel_Context_Object     obj[GC_VX_MAX_ARRAY * GC_VX_MAX_ARRAY];
    gctUINT32                       objects_num;

    gcoVX_Kernel_Context_Uniform    uniforms[GC_VX_MAX_ARRAY * GC_VX_MAX_ARRAY];
    gctUINT32						uniform_num;
}
gcoVX_Kernel_Context;

gceSTATUS gcfVX_Kernel_ConvertFormat(
    IN vx_df_image Format,
    OUT gcsVX_IMAGE_INFO_PTR Info
    );

gceSTATUS gcfVX_Kernel_Upload(
    IN vx_image Image,
	IN vx_bool Upload,
    OUT gcsVX_IMAGE_INFO_PTR Info
    );

vx_status
gcfVX_Kernel(
    IN gcoVX_Kernel_Context *Context
    );

vx_status gcfVX_Flush(
    IN gctBOOL      Stall
    );

gcoVX_Kernel_Context_Object*
gcoVX_AddObject(
	IN OUT gcoVX_Kernel_Context* context,
	IN gc_vx_object_type_t type,
	IN void* object,
	IN gctUINT32 index
	);

/* base kernel function */
vx_status Convolve(vx_image src, vx_image dst, vx_int16* matrix,  vx_uint32 scale, vx_bool clamp,
                   vx_size conv_width, vx_size conv_height, vx_border_mode_t *bordermode);

vx_status vxAbsDiff(vx_image in1, vx_image in2, vx_image output);

vx_status vxAccumulate(vx_image input, vx_image accum);
vx_status vxAccumulateWeighted(vx_image input, vx_scalar scalar, vx_image accum);
vx_status vxAccumulateSquare(vx_image input, vx_scalar scalar, vx_image accum);

vx_status vxAddition(vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output);
vx_status vxSubtraction(vx_image in0, vx_image in1, vx_scalar policy_param, vx_image output);

vx_status vxAnd(vx_image in0, vx_image in1, vx_image output);
vx_status vxOr(vx_image in0, vx_image in1, vx_image output);
vx_status vxXor(vx_image in0, vx_image in1, vx_image output);
vx_status vxNot(vx_image input, vx_image output);

vx_status vxChannelCombine(vx_image inputs[4], vx_image output);
vx_status vxChannelExtract(vx_image src, vx_scalar channel, vx_image dst);

vx_status vxConvertColor(vx_image src, vx_image dst);
vx_status vxConvertDepth(vx_image input, vx_image output, vx_scalar spol, vx_scalar sshf);

vx_status vxConvolve(vx_image src, vx_convolution conv, vx_image dst, vx_border_mode_t *bordermode);
vx_status vxConvolution3x3(vx_image src, vx_image dst, vx_int16 conv[3][3], const vx_border_mode_t *borders);

vx_status vxFast9Corners(vx_image src, vx_scalar sens, vx_scalar nonm,
                         vx_array points, vx_scalar num_corners, vx_border_mode_t *bordermode, vx_reference* staging);

vx_status vxMedian3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);
vx_status vxBox3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);
vx_status vxGaussian3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);

vx_status vxHistogram(vx_image src, vx_distribution dist, vx_reference* staging);
vx_status vxEqualizeHist(vx_image src, vx_image dst, vx_reference* staging);

vx_status vxIntegralImage(vx_image src, vx_image dst);
vx_status vxTableLookup(vx_image src, vx_lut lut, vx_image dst);

vx_status vxMeanStdDev(vx_image input, vx_scalar mean, vx_scalar stddev);
vx_status vxMinMaxLoc(vx_image input, vx_scalar minVal, vx_scalar maxVal, vx_array minLoc, vx_array maxLoc, vx_scalar minCount, vx_scalar maxCount, vx_reference* staging);

vx_status vxErode3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);
vx_status vxDilate3x3(vx_image src, vx_image dst, vx_border_mode_t *bordermode);

vx_status vxMagnitude(vx_image grad_x, vx_image grad_y, vx_image output);

vx_status vxMultiply(vx_image in0, vx_image in1, vx_scalar scale_param, vx_scalar opolicy_param, vx_scalar rpolicy_param, vx_image output);

vx_status vxOpticalFlowPyrLK(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_status VLKTracker(const vx_image prevImg[], const vx_image nextImg[], vx_reference* stagings,
                                            const vx_array prevPts,  const vx_array estimatedPts, vx_array nextPts,
                                            vx_scalar criteriaScalar, vx_scalar epsilonScalar, vx_scalar numIterationsScalar, vx_bool isUseInitialEstimate, vx_scalar winSizeScalar,
                                            vx_int32 maxLevel, vx_float32 pyramidScale
                                            );

vx_status vxPhase(vx_image grad_x, vx_image grad_y, vx_image output);
vx_status vxRemap(vx_image input, vx_remap remap, vx_enum policy, vx_border_mode_t *borders, vx_image output);

vx_status vxScaleImage(vx_image src_image, vx_image dst_image, vx_scalar stype, vx_border_mode_t *bordermode, vx_float64 *interm, vx_size size);

vx_status vxSobel3x3(vx_image input, vx_image grad_x, vx_image grad_y, vx_border_mode_t *bordermode);
vx_status vxScharr3x3(vx_image input, vx_image grad_x, vx_image grad_y, vx_border_mode_t *bordermode);

vx_status vxThreshold(vx_image src_image, vx_threshold threshold, vx_image dst_image);

vx_status vxWarpPerspective(vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image, const vx_border_mode_t *borders);
vx_status vxWarpAffine(vx_image src_image, vx_matrix matrix, vx_scalar stype, vx_image dst_image, const vx_border_mode_t *borders);

/* internal kernel function */
vx_status vxSobelMxN(vx_image input, vx_scalar win, vx_image grad_x, vx_image grad_y, vx_border_mode_t *bordermode);
vx_status vxNorm(vx_image input_x, vx_image input_y, vx_scalar norm_type, vx_image output);
vx_status vxNonMaxSuppression(vx_image i_mag, vx_image i_ang, vx_image i_edge, vx_border_mode_t *borders);
vx_status vxEdgeTrace(vx_image norm, vx_threshold threshold, vx_image output, vx_reference* staging);

vx_status vxHarrisScore(vx_image grad_x, vx_image grad_y, vx_image dst, vx_scalar winds, vx_scalar sens, vx_border_mode_t borders);
vx_status vxEuclideanNonMaxSuppression(vx_image src, vx_scalar thr, vx_scalar rad, vx_image dst);

vx_status vxImageLister(vx_image src, vx_array arrays, vx_scalar num, vx_reference* staging);

#ifdef __cplusplus
}
#endif

#endif
