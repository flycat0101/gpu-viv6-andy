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


#ifndef _OPENVX_INTERFACE_H_
#define _OPENVX_INTERFACE_H_

#include <gc_vxk_common.h>

VX_PRIVATE_API vx_status vxoBaseKernel_Invalid(vx_node node, vx_reference paramTable[], vx_uint32 num);
VX_PRIVATE_API vx_status vxoInvalid_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoInvalid_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *info);
VX_PRIVATE_API vx_param_description_s  invalid_kernel_params[1];
VX_PRIVATE_API vx_kernel_description_s invalid_kernel = {
    VX_KERNEL_INVALID,
    "org.khronos.openvx.invalid",
    vxoBaseKernel_Invalid,
    invalid_kernel_params, 0,
    vxoInvalid_ValidateInput,
    vxoInvalid_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_ColorConvert(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoColorConvert_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoColorConvert_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_df_image colorConvert_InputOutputFormat[][2] = {
    {VX_DF_IMAGE_RGB, VX_DF_IMAGE_RGBX},
    {VX_DF_IMAGE_RGB, VX_DF_IMAGE_NV12},
    {VX_DF_IMAGE_RGB, VX_DF_IMAGE_YUV4},
    {VX_DF_IMAGE_RGB, VX_DF_IMAGE_IYUV},
    {VX_DF_IMAGE_RGBX,VX_DF_IMAGE_RGB},
    {VX_DF_IMAGE_RGBX,VX_DF_IMAGE_NV12},
    {VX_DF_IMAGE_RGBX,VX_DF_IMAGE_YUV4},
    {VX_DF_IMAGE_RGBX,VX_DF_IMAGE_IYUV},
    {VX_DF_IMAGE_NV12,VX_DF_IMAGE_RGB},
    {VX_DF_IMAGE_NV12,VX_DF_IMAGE_RGBX},
    {VX_DF_IMAGE_NV12,VX_DF_IMAGE_NV21},
    {VX_DF_IMAGE_NV12,VX_DF_IMAGE_YUV4},
    {VX_DF_IMAGE_NV12,VX_DF_IMAGE_IYUV},
    {VX_DF_IMAGE_NV21,VX_DF_IMAGE_RGB},
    {VX_DF_IMAGE_NV21,VX_DF_IMAGE_RGBX},
    {VX_DF_IMAGE_NV21,VX_DF_IMAGE_NV12},
    {VX_DF_IMAGE_NV21,VX_DF_IMAGE_YUV4},
    {VX_DF_IMAGE_NV21,VX_DF_IMAGE_IYUV},
    {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_RGB},
    {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_RGBX},
    {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_NV12},
    {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_YUV4},
    {VX_DF_IMAGE_UYVY,VX_DF_IMAGE_IYUV},
    {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_RGB},
    {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_RGBX},
    {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_NV12},
    {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_YUV4},
    {VX_DF_IMAGE_YUYV,VX_DF_IMAGE_IYUV},
    {VX_DF_IMAGE_IYUV,VX_DF_IMAGE_RGB},
    {VX_DF_IMAGE_IYUV,VX_DF_IMAGE_RGBX},
    {VX_DF_IMAGE_IYUV,VX_DF_IMAGE_NV12},
    {VX_DF_IMAGE_IYUV,VX_DF_IMAGE_YUV4},
};

VX_PRIVATE_API vx_param_description_s basekernel_colorConvert_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_colorconvert = {
    VX_KERNEL_COLOR_CONVERT,
    "org.khronos.openvx.color_convert",
    vxoBaseKernel_ColorConvert,
    basekernel_colorConvert_params, vxmLENGTH_OF(basekernel_colorConvert_params),
    vxoColorConvert_ValidateInput,
    vxoColorConvert_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_ChannelExtract(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoChannelExtract_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoChannelExtract_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_channelExtract_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_ChannelCombine(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoChannelCombine_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoChannelCombine_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_channelextract = {
    VX_KERNEL_CHANNEL_EXTRACT,
    "org.khronos.openvx.channel_extract",
    vxoBaseKernel_ChannelExtract,
    basekernel_channelExtract_params, vxmLENGTH_OF(basekernel_channelExtract_params),
    vxoChannelExtract_ValidateInput,
    vxoChannelExtract_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_param_description_s basekernel_channelCombine_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_channelcombine = {
    VX_KERNEL_CHANNEL_COMBINE,
    "org.khronos.openvx.channel_combine",
    vxoBaseKernel_ChannelCombine,
    basekernel_channelCombine_params, vxmLENGTH_OF(basekernel_channelCombine_params),
    vxoChannelCombine_ValidateInput,
    vxoChannelCombine_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_Sobel3x3(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoSobel3x3_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoSobel3x3_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_Sobel3x3_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
};

vx_kernel_description_s basekernel_sobel3x3 = {
    VX_KERNEL_SOBEL_3x3,
    "org.khronos.openvx.sobel3x3",
    vxoBaseKernel_Sobel3x3,
    basekernel_Sobel3x3_params, vxmLENGTH_OF(basekernel_Sobel3x3_params),
    vxoSobel3x3_ValidateInput,
    vxoSobel3x3_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_Magnitude(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMagnitude_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMagnitude_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_magnitude_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_magnitude = {
    VX_KERNEL_MAGNITUDE,
    "org.khronos.openvx.magnitude",
    vxoBaseKernel_Magnitude,
    basekernel_magnitude_params, vxmLENGTH_OF(basekernel_magnitude_params),
    vxoMagnitude_ValidateInput,
    vxoMagnitude_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 2, 1 },
#endif
};

VX_PRIVATE_API vx_status vxoBaseKernel_Phase(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoPhase_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoPhase_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_phase_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_phase = {
    VX_KERNEL_PHASE,
    "org.khronos.openvx.phase",
    vxoBaseKernel_Phase,
    basekernel_phase_params, vxmLENGTH_OF(basekernel_phase_params),
    vxoPhase_ValidateInput,
    vxoPhase_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 2, 1 },
#endif
};

VX_PRIVATE_API vx_status vxoBaseKernel_TableLookup(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoTableLookup_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoTableLookup_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_lut_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_LUT,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_lut = {
    VX_KERNEL_TABLE_LOOKUP,
    "org.khronos.openvx.table_lookup",
    vxoBaseKernel_TableLookup,
    basekernel_lut_params, vxmLENGTH_OF(basekernel_lut_params),
    vxoTableLookup_ValidateInput,
    vxoTableLookup_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_ScaleImage(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoScaleImage_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoScaleImage_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoScaleImage_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_scale_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

vx_kernel_description_s basekernel_scale_image = {
    VX_KERNEL_SCALE_IMAGE,
    "org.khronos.openvx.scale_image",
    vxoBaseKernel_ScaleImage,
    basekernel_scale_params, vxmLENGTH_OF(basekernel_scale_params),
    vxoScaleImage_ValidateInput,
    vxoScaleImage_ValidateOutput,
    vxoScaleImage_Initializer,
    NULL,
};

VX_PRIVATE_API vx_status vxoBasekernel_HalfscaleGaussian(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoHalfscaleGaussian_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoHalfscaleGaussian_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoHalfscaleGaussian_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoHalfscaleGaussian_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_halfscale_gaussian = {
    VX_KERNEL_HALFSCALE_GAUSSIAN,
    "org.khronos.openvx.halfscale_gaussian",
    vxoBasekernel_HalfscaleGaussian,
    basekernel_scale_params, vxmLENGTH_OF(basekernel_scale_params),
    vxoHalfscaleGaussian_ValidateInput,
    vxoHalfscaleGaussian_ValidateOutput,
    vxoHalfscaleGaussian_Initializer,
    vxoHalfscaleGaussian_Deinitializer,
};

VX_PRIVATE_API vx_status vxoBaseKernel_Histogram(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoHistogram_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoHistogram_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_histogram_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_DISTRIBUTION, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_histogram = {
    VX_KERNEL_HISTOGRAM,
    "org.khronos.openvx.histogram",
    vxoBaseKernel_Histogram,
    basekernel_histogram_params, vxmLENGTH_OF(basekernel_histogram_params),
    vxoHistogram_ValidateInput,
    vxoHistogram_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_EqualizeHist(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEqualizeHist_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEqualizeHist_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoEqualizeHist_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEqualizeHist_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_equalize_hist_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_equalize_hist = {
    VX_KERNEL_EQUALIZE_HISTOGRAM,
    "org.khronos.openvx.equalize_histogram",
    vxoBaseKernel_EqualizeHist,
    basekernel_equalize_hist_params, vxmLENGTH_OF(basekernel_equalize_hist_params),
    vxoEqualizeHist_ValidateInput,
    vxoEqualizeHist_ValidateOutput,
    vxoEqualizeHist_Initializer,
    vxoEqualizeHist_Deinitializer,
};

VX_PRIVATE_API vx_status vxoBaseKernel_AbsDiff(vx_node node, vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status vxoAbsDiff_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoAbsDiff_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_absdiff_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_absdiff = {
    VX_KERNEL_ABSDIFF,
    "org.khronos.openvx.absdiff",
    vxoBaseKernel_AbsDiff,
    basekernel_absdiff_params, vxmLENGTH_OF(basekernel_absdiff_params),
    vxoAbsDiff_ValidateInput,
    vxoAbsDiff_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_MeanStdDev(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMeanStdDev_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMeanStdDev_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_mean_stddev_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_mean_stddev = {
    VX_KERNEL_MEAN_STDDEV,
    "org.khronos.openvx.mean_stddev",
    vxoBaseKernel_MeanStdDev,
    basekernel_mean_stddev_params, vxmLENGTH_OF(basekernel_mean_stddev_params),
    vxoMeanStdDev_ValidateInput,
    vxoMeanStdDev_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_Threshold(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoThreshold_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoThreshold_ValidatorOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_threshold_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_THRESHOLD,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_threshold = {
    VX_KERNEL_THRESHOLD,
    "org.khronos.openvx.threshold",
    vxoBaseKernel_Threshold,
    basekernel_threshold_params, vxmLENGTH_OF(basekernel_threshold_params),
    vxoThreshold_ValidateInput,
    vxoThreshold_ValidatorOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_IntegralImage(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoIntegral_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoIntegral_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoIntegral_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoIntegral_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_integral_image_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_integral_image = {
    VX_KERNEL_INTEGRAL_IMAGE,
    "org.khronos.openvx.integral_image",
    vxoBaseKernel_IntegralImage,
    basekernel_integral_image_params, vxmLENGTH_OF(basekernel_integral_image_params),
    vxoIntegral_ValidateInput,
    vxoIntegral_ValidateOutput,
    vxoIntegral_Initializer,
    vxoIntegral_Deinitializer,
};

VX_PRIVATE_API vx_status vxoMorphology_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMorphology_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s*ptr);
VX_PRIVATE_API vx_param_description_s basekernel_morphology_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_Erode3x3(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_erode3x3 = {
    VX_KERNEL_ERODE_3x3,
    "org.khronos.openvx.erode3x3",
    vxoBaseKernel_Erode3x3,
    basekernel_morphology_params, vxmLENGTH_OF(basekernel_morphology_params),
    vxoMorphology_ValidateInput,
    vxoMorphology_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_Dilate3x3(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_dilate3x3 = {
    VX_KERNEL_DILATE_3x3,
    "org.khronos.openvx.dilate3x3",
    vxoBaseKernel_Dilate3x3,
    basekernel_morphology_params, vxmLENGTH_OF(basekernel_morphology_params),
    vxoMorphology_ValidateInput,
    vxoMorphology_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoFilter_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoFilter_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_filter_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_Box3x3(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_box3x3 = {
    VX_KERNEL_BOX_3x3,
    "org.khronos.openvx.box3x3:default",
    vxoBaseKernel_Box3x3,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
};

vx_kernel_description_s basekernel_box3x3_2 = {
    VX_KERNEL_BOX_3x3,
    "org.khronos.openvx.box3x3:duplicate",
    vxoBaseKernel_Box3x3,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
};

VX_PRIVATE_API vx_status vxoBaseKernel_Median3x3(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_median3x3 = {
    VX_KERNEL_MEDIAN_3x3,
    "org.khronos.openvx.median3x3",
    vxoBaseKernel_Median3x3,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
};

VX_PRIVATE_API vx_status vxoBaseKernel_Gaussian3x3(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_gaussian3x3 = {
    VX_KERNEL_GAUSSIAN_3x3,
    "org.khronos.openvx.gaussian3x3",
    vxoBaseKernel_Gaussian3x3,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
};

VX_PRIVATE_API vx_param_description_s basekernel_convolution_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_CONVOLUTION, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBasekernel_Convolve(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoConvolve_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoConvolve_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_convolution = {
    VX_KERNEL_CUSTOM_CONVOLUTION,
    "org.khronos.openvx.custom_convolution",
    vxoBasekernel_Convolve,
    basekernel_convolution_params, vxmLENGTH_OF(basekernel_convolution_params),
    vxoConvolve_ValidateInput,
    vxoConvolve_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_pyramid_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_Pyramid(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoPyramid_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoPyramid_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoPyramid_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoPyramid_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_pyramid = {
    VX_KERNEL_GAUSSIAN_PYRAMID,
    "org.khronos.openvx.pyramid",
    vxoBaseKernel_Pyramid,
    basekernel_pyramid_params, vxmLENGTH_OF(basekernel_pyramid_params),
    vxoPyramid_ValidateInput,
    vxoPyramid_ValidateOutput,
    vxoPyramid_Initializer,
    vxoPyramid_Deinitializer,
};

static vx_param_description_s accumulate_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_BIDIRECTIONAL, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

static vx_param_description_s accumulate_scaled_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_BIDIRECTIONAL, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_Accumulate(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoAccumulate_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoAccumulate_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_accumulate = {
    VX_KERNEL_ACCUMULATE,
    "org.khronos.openvx.accumulate",
    vxoBaseKernel_Accumulate,
    accumulate_kernel_params, vxmLENGTH_OF(accumulate_kernel_params),
    vxoAccumulate_ValidateInput,
    vxoAccumulate_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_AccumulateWeighted(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoAccumulateWeighted_ValidateInput(vx_node node, vx_uint32 index);
vx_kernel_description_s basekernel_accumulate_weighted = {
    VX_KERNEL_ACCUMULATE_WEIGHTED,
    "org.khronos.openvx.accumulate_weighted",
    vxoBaseKernel_AccumulateWeighted,
    accumulate_scaled_kernel_params, vxmLENGTH_OF(accumulate_scaled_kernel_params),
    vxoAccumulateWeighted_ValidateInput,
    vxoAccumulate_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_AccumulateSquare(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoAccumulateSquared_ValidateInput(vx_node node, vx_uint32 index);
vx_kernel_description_s basekernel_accumulate_square = {
    VX_KERNEL_ACCUMULATE_SQUARE,
    "org.khronos.openvx.accumulate_square",
    vxoBaseKernel_AccumulateSquare,
    accumulate_scaled_kernel_params, vxmLENGTH_OF(accumulate_scaled_kernel_params),
    vxoAccumulateSquared_ValidateInput,
    vxoAccumulate_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_minmaxloc_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

VX_PRIVATE_API vx_status vxoBaseKernel_MinMaxLoc(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMinMaxLoc_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMinMaxLoc_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoMinMaxLoc_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMinMaxLoc_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_minmaxloc = {
    VX_KERNEL_MINMAXLOC,
    "org.khronos.openvx.min_max_loc",
    vxoBaseKernel_MinMaxLoc,
    basekernel_minmaxloc_params, vxmLENGTH_OF(basekernel_minmaxloc_params),
    vxoMinMaxLoc_ValidateInput,
    vxoMinMaxLoc_ValidateOutput,
    vxoMinMaxLoc_Initializer,
    vxoMinMaxLoc_Deinitializer,
};

VX_PRIVATE_API vx_df_image convertDepth_InputOutputFormat[][2] = {
    {VX_DF_IMAGE_U8,  VX_DF_IMAGE_U16},
    {VX_DF_IMAGE_U8,  VX_DF_IMAGE_S16},
    {VX_DF_IMAGE_U8,  VX_DF_IMAGE_U32},
    {VX_DF_IMAGE_U16, VX_DF_IMAGE_U8 },
    {VX_DF_IMAGE_S16, VX_DF_IMAGE_U8 },
    {VX_DF_IMAGE_U16, VX_DF_IMAGE_U32},
    {VX_DF_IMAGE_S16, VX_DF_IMAGE_S32},
    {VX_DF_IMAGE_U32, VX_DF_IMAGE_U8 },
    {VX_DF_IMAGE_U32, VX_DF_IMAGE_U16},
    {VX_DF_IMAGE_S32, VX_DF_IMAGE_S16},
    {VX_DF_IMAGE_F32, VX_DF_IMAGE_U8 }
};

static vx_param_description_s basekernel_convertdepth_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR,  VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR,  VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_ConvertDepth(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoConvertDepth_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoConvertDepth_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_convertdepth = {
    VX_KERNEL_CONVERTDEPTH,
    "org.khronos.openvx.convert_depth",
    vxoBaseKernel_ConvertDepth,
    basekernel_convertdepth_params, vxmLENGTH_OF(basekernel_convertdepth_params),
    vxoConvertDepth_ValidateInput,
    vxoConvertDepth_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_canny_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_THRESHOLD, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_CannyEdge(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoCannyEdge_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoCannyEdge_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoCannyEdge_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoCannyEdge_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_canny = {
    VX_KERNEL_CANNY_EDGE_DETECTOR,
    "org.khronos.openvx.canny_edge_detector",
    vxoBaseKernel_CannyEdge,
    basekernel_canny_params, vxmLENGTH_OF(basekernel_canny_params),
    vxoCannyEdge_ValidateInput,
    vxoCannyEdge_ValidateOutput,
    vxoCannyEdge_Initializer,
    vxoCannyEdge_Deinitializer,
};

static vx_param_description_s basekernel_binary_bitwise_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_And(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoBinaryBitwise_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoBinaryBitwise_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_and = {
    VX_KERNEL_AND,
    "org.khronos.openvx.and",
    vxoBaseKernel_And,
    basekernel_binary_bitwise_params, vxmLENGTH_OF(basekernel_binary_bitwise_params),
    vxoBinaryBitwise_ValidateInput,
    vxoBinaryBitwise_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_Or(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_or = {
    VX_KERNEL_OR,
    "org.khronos.openvx.or",
    vxoBaseKernel_Or,
    basekernel_binary_bitwise_params, vxmLENGTH_OF(basekernel_binary_bitwise_params),
    vxoBinaryBitwise_ValidateInput,
    vxoBinaryBitwise_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBasekernel_Xor(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_xor = {
    VX_KERNEL_XOR,
    "org.khronos.openvx.xor",
    vxoBasekernel_Xor,
    basekernel_binary_bitwise_params, vxmLENGTH_OF(basekernel_binary_bitwise_params),
    vxoBinaryBitwise_ValidateInput,
    vxoBinaryBitwise_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s unary_bitwise_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_Not(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoUnaryBitwise_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoUnaryBitwise_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_not = {
    VX_KERNEL_NOT,
    "org.khronos.openvx.not",
    vxoBaseKernel_Not,
    unary_bitwise_kernel_params, vxmLENGTH_OF(unary_bitwise_kernel_params),
    vxoUnaryBitwise_ValidateInput,
    vxoUnaryBitwise_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_multiply_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_Multiply(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMultiply_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMultiply_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_multiply = {
    VX_KERNEL_MULTIPLY,
    "org.khronos.openvx.multiply",
    vxoBaseKernel_Multiply,
    basekernel_multiply_params, vxmLENGTH_OF(basekernel_multiply_params),
    vxoMultiply_ValidateInput,
    vxoMultiply_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_add_subtract_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_Add(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoAddSubtract_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoAddSubtract_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_add = {
    VX_KERNEL_ADD,
    "org.khronos.openvx.add",
    vxoBaseKernel_Add,
    basekernel_add_subtract_params, vxmLENGTH_OF(basekernel_add_subtract_params),
    vxoAddSubtract_ValidateInput,
    vxoAddSubtract_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_Sub(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_subtract = {
    VX_KERNEL_SUBTRACT,
    "org.khronos.openvx.subtract",
    vxoBaseKernel_Sub,
    basekernel_add_subtract_params, vxmLENGTH_OF(basekernel_add_subtract_params),
    vxoAddSubtract_ValidateInput,
    vxoAddSubtract_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_warp_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_MATRIX, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxWarpAffineKernel(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoWarpAffine_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoWarp_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_warp_affine = {
    VX_KERNEL_WARP_AFFINE,
    "org.khronos.openvx.warp_affine",
    vxWarpAffineKernel,
    basekernel_warp_params, vxmLENGTH_OF(basekernel_warp_params),
    vxoWarpAffine_ValidateInput,
    vxoWarp_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status vxoBaseKernel_WarpPerspective(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoWarpPerspective_ValidateInput(vx_node node, vx_uint32 index);
vx_kernel_description_s basekernel_warp_perspective = {
    VX_KERNEL_WARP_PERSPECTIVE,
    "org.khronos.openvx.warp_perspective",
    vxoBaseKernel_WarpPerspective,
    basekernel_warp_params, vxmLENGTH_OF(basekernel_warp_params),
    vxoWarpPerspective_ValidateInput,
    vxoWarp_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_harris_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

VX_PRIVATE_API vx_status vxoBaseKernel_HarrisCorners(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoHarris_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoHarris_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoHarris_Initializer(vx_node node, vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status vxoHarris_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_harris = {
    VX_KERNEL_HARRIS_CORNERS,
    "org.khronos.openvx.harris_corners",
    vxoBaseKernel_HarrisCorners,
    basekernel_harris_params, vxmLENGTH_OF(basekernel_harris_params),
    vxoHarris_ValidateInput,
    vxoHarris_ValidateOutput,
    vxoHarris_Initializer,
    vxoHarris_Deinitializer,
};

static vx_param_description_s basekernel_fast9_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

VX_PRIVATE_API vx_status vxoBaseKernel_Fast9Corners(vx_node node, vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status vxoFast9_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoFast9_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoFast9_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoFast9_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_fast9 = {
    VX_KERNEL_FAST_CORNERS,
    "org.khronos.openvx.fast_corners",
    vxoBaseKernel_Fast9Corners,
    basekernel_fast9_params, vxmLENGTH_OF(basekernel_fast9_params),
    vxoFast9_ValidateInput,
    vxoFast9_ValidateOutput,
    vxoFast9_Initializer,
    vxoFast9_Deinitializer
};

static vx_param_description_s basekernel_optpyrlk_params[] = {
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_OpticalFlowPyrLK(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoOpticalFlowPyrLK_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoOpticalFlowPyrLK_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoOpticalFlowPyrLK_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoOpticalFlowPyrLK_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_optpyrlk = {
    VX_KERNEL_OPTICAL_FLOW_PYR_LK,
    "org.khronos.openvx.opticalflow_pyr_lk",
    vxoBaseKernel_OpticalFlowPyrLK,
    basekernel_optpyrlk_params, vxmLENGTH_OF(basekernel_optpyrlk_params),
    vxoOpticalFlowPyrLK_ValidateInput,
    vxoOpticalFlowPyrLK_ValidateOutput,
    vxoOpticalFlowPyrLK_Initializer,
    vxoOpticalFlowPyrLK_Deinitializer
};

static vx_param_description_s remap_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_REMAP, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoBaseKernel_Remap(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoRemap_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoRemap_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_remap = {
    VX_KERNEL_REMAP,
    "org.khronos.openvx.remap",
    vxoBaseKernel_Remap,
    remap_kernel_params, vxmLENGTH_OF(remap_kernel_params),
    vxoRemap_ValidateInput,
    vxoRemap_ValidateOutput,
    NULL,
    NULL,
};

/* internal kernel */
static vx_param_description_s internlkernel_gradientMxN_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
};

VX_PRIVATE_API vx_status vxoInternalKernel_SobelMxN(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoGradientMxN_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoGradientMxN_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_sobelMxN = {
    VX_KERNEL_INTERNAL_SOBEL_MxN,
    "org.khronos.internal.sobelMxN",
    vxoInternalKernel_SobelMxN,
    internlkernel_gradientMxN_params, vxmLENGTH_OF(internlkernel_gradientMxN_params),
    vxoGradientMxN_ValidateInput,
    vxoGradientMxN_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 1, 2 },
#endif
};

static vx_param_description_s internalkernel_harrisscore_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_HarrisScore(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoHarrisScore_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoHarrisScore_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_harris_score = {
    VX_KERNEL_INTERNAL_HARRIS_SCORE,
    "org.khronos.internal.harris_score",
    vxoInternalKernel_HarrisScore,
    internalkernel_harrisscore_params, vxmLENGTH_OF(internalkernel_harrisscore_params),
    vxoHarrisScore_ValidateInput,
    vxoHarrisScore_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s intenralkernel_euclidean_non_max_suppression_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_EuclideanNonMaxSuppression(vx_node node, vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status vxoEuclideanNonMax_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEuclideanNonMax_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_euclidian_nonmax = {
    VX_KERNEL_INTERNAL_EUCLIDEAN_NONMAXSUPPRESSION,
    "org.khronos.internal.euclidean_nonmaxsuppression",
    vxoInternalKernel_EuclideanNonMaxSuppression,
    intenralkernel_euclidean_non_max_suppression_params, vxmLENGTH_OF(intenralkernel_euclidean_non_max_suppression_params),
    vxoEuclideanNonMax_ValidateInput,
    vxoEuclideanNonMax_ValidateOutput,
};

static vx_param_description_s internalkernel_lister_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

VX_PRIVATE_API vx_status vxoInternalKernel_ImageLister(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoLister_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoLister_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
VX_PRIVATE_API vx_status vxoLister_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoLister_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_lister = {
    VX_KERNEL_INTERNAL_IMAGE_LISTER,
    "org.khronos.internal.image_to_list",
    vxoInternalKernel_ImageLister,
    internalkernel_lister_params, vxmLENGTH_OF(internalkernel_lister_params),
    vxoLister_ValidateInput,
    vxoLister_ValidateOutput,
    vxoLister_Initializer,
    vxoLister_Deinitializer,
};

static vx_param_description_s internalkernel_norm_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_Norm(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoNorm_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoNorm_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_norm = {
    VX_KERNEL_INTERNAL_ELEMENTWISE_NORM,
    "org.khronos.internal.elementwise_norm",
    vxoInternalKernel_Norm,
    internalkernel_norm_params, vxmLENGTH_OF(internalkernel_norm_params),
    vxoNorm_ValidateInput,
    vxoNorm_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 2, 1 },
#endif
};

static vx_param_description_s internalkernel_nonmaxsuppression_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_NonMaxSuppression(vx_node node, vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status vxoNonMaxSuppression_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoNonMaxSuppression_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_nonmax = {
    VX_KERNEL_INTERNAL_NONMAXSUPPRESSION,
    "org.khronos.internal.nonmaximasuppression",
    vxoInternalKernel_NonMaxSuppression,
    internalkernel_nonmaxsuppression_params, vxmLENGTH_OF(internalkernel_nonmaxsuppression_params),
    vxoNonMaxSuppression_ValidateInput,
    vxoNonMaxSuppression_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 1, 1 },
#endif
};

static vx_param_description_s internalkernel_edge_trace_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_THRESHOLD,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_EdgeTrace(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEdgeTrace_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEdgeTrace_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
VX_PRIVATE_API vx_status vxoEdgeTrace_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEdgeTrace_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_edge_trace = {
    VX_KERNEL_INTERNAL_EDGE_TRACE,
    "org.khronos.extra.edge_trace",
    vxoInternalKernel_EdgeTrace,
    internalkernel_edge_trace_params, vxmLENGTH_OF(internalkernel_edge_trace_params),
    vxoEdgeTrace_ValidateInput,
    vxoEdgeTrace_ValidateOutput,
    vxoEdgeTrace_Initializer,
    vxoEdgeTrace_Deinitializer,
};

static vx_param_description_s internalkernel_example_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_Example(vx_node node, vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status vxoExample_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoExample_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_example = {
    VX_KERNEL_INTERNAL_EXAMPLE,
    "vivante.internal.example",
    vxoInternalKernel_Example,
    internalkernel_example_params, vxmLENGTH_OF(internalkernel_example_params),
    vxoExample_ValidateInput,
    vxoExample_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_copy_image_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_CopyImage(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoCopyImage_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoCopyImage_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
VX_PRIVATE_API vx_status vxoCopyImage_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoCopyImage_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_copy_image = {
    VX_KERNEL_INTERNAL_COPY_IMAGE,
    "vivante.internal.copy_image",
    vxoInternalKernel_CopyImage,
    internalkernel_copy_image_params, vxmLENGTH_OF(internalkernel_copy_image_params),
    vxoCopyImage_ValidateInput,
    vxoCopyImage_ValidateOutput,
    NULL,
    NULL,
};


static vx_param_description_s internalkernel_fast9corners_strength_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},

};

VX_PRIVATE_API vx_status vxoInternalKernel_Fast9CornersStrength(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoFast9CornersStrength_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoFast9CornersStrength_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
VX_PRIVATE_API vx_status vxoFast9CornersStrength_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoFast9CornersStrength_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_Fast9Corners_Strength = {
    VX_KERNEL_INTERNAL_FAST9CORNERS_STRENGTH,
    "vivante.internal.fast9corners_strength",
    vxoInternalKernel_Fast9CornersStrength,
    internalkernel_fast9corners_strength_params, vxmLENGTH_OF(internalkernel_fast9corners_strength_params),
    vxoFast9CornersStrength_ValidateInput,
    vxoFast9CornersStrength_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_fast9corners_nonmax_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_Fast9CornersNonMax(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoFast9CornersNonMax_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoFast9CornersNonMax_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
VX_PRIVATE_API vx_status vxoFast9CornersNonMax_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoFast9CornersNonMax_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalKernel_Fast9Corners_NonMax = {
    VX_KERNEL_INTERNAL_FAST9CORNERS_NONMAX,
    "vivante.internal.fast9corners_nonmax",
    vxoInternalKernel_Fast9CornersNonMax,
    internalkernel_fast9corners_nonmax_params, vxmLENGTH_OF(internalkernel_fast9corners_nonmax_params),
    vxoFast9CornersNonMax_ValidateInput,
    vxoFast9CornersNonMax_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_createlister_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status vxoInternalKernel_CreateLister(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoCreateLister_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoCreateLister_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_CreateLister = {
    VX_KERNEL_INTERNAL_CREATE_LISTER,
    "vivante.internal.createlister",
    vxoInternalKernel_CreateLister,
    internalkernel_createlister_params, vxmLENGTH_OF(internalkernel_createlister_params),
    vxoCreateLister_ValidateInput,
    vxoCreateLister_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_pack_arrays_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL}
};

VX_PRIVATE_API vx_status vxoInternalKernel_PackArrays(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoPackArrays_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoPackArrays_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_PackArrays = {
    VX_KERNEL_INTERNAL_PACK_ARRAYS,
    "vivante.internal.packarrays",
    vxoInternalKernel_PackArrays,
    internalkernel_pack_arrays_params, vxmLENGTH_OF(internalkernel_pack_arrays_params),
    vxoPackArrays_ValidateInput,
    vxoPackArrays_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_minmaxloc_pack_arrays_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL}
};

VX_PRIVATE_API vx_status vxoInternalKernel_MinMaxlocPackArrays(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMinMaxLocPackArrays_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMinMaxLocPackArrays_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_MinMacLocPackArrays = {
    VX_KERNEL_INTERNAL_MIN_MAX_LOC_PACK_ARRAYS,
    "vivante.internal.minmaxloc.packarrays",
    vxoInternalKernel_MinMaxlocPackArrays,
    internalkernel_minmaxloc_pack_arrays_params, vxmLENGTH_OF(internalkernel_minmaxloc_pack_arrays_params),
    vxoMinMaxLocPackArrays_ValidateInput,
    vxoMinMaxLocPackArrays_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_minmaxloc_filter_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL}
};

VX_PRIVATE_API vx_status vxoInternalKernel_MinMaxLocFilter(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMinMaxLocFilter_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMinMaxLocFilter_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_MinMaxLocFilter = {
    VX_KERNEL_INTERNAL_MINMAXLOC_FILTER,
    "vivante.internal.minmaxlocfilter",
    vxoInternalKernel_MinMaxLocFilter,
    internalkernel_minmaxloc_filter_params, vxmLENGTH_OF(internalkernel_minmaxloc_filter_params),
    vxoMinMaxLocFilter_ValidateInput,
    vxoMinMaxLocFilter_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_minmax_get_location_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL}
};

VX_PRIVATE_API vx_status vxoInternalKernel_MinMaxGetLocation(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMinMaxGetLocation_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMinMaxGetLocation_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoMinMaxGetLocation_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMinMaxGetLocation_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalKernel_MinMaxGetLocation = {
    VX_KERNEL_INTERNAL_GET_LOCATION,
    "vivante.internal.minmax.getlocation",
    vxoInternalKernel_MinMaxGetLocation,
    internalkernel_minmax_get_location_params, vxmLENGTH_OF(internalkernel_minmax_get_location_params),
    vxoMinMaxGetLocation_ValidateInput,
    vxoMinMaxGetLocation_ValidateOutput
};

static vx_param_description_s internalkernel_edgeTrace_threshold_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_THRESHOLD, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status vxoInternalKernel_EdgeTraceThreshold(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEdgeTraceThreshold_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEdgeTraceThreshold_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EdgeTraceThreshold = {
    VX_KERNEL_INTERNAL_EDGE_TRACE_THRESHOLD,
    "vivante.internal.edge.trace.threshold",
    vxoInternalKernel_EdgeTraceThreshold,
    internalkernel_edgeTrace_threshold_params, vxmLENGTH_OF(internalkernel_edgeTrace_threshold_params),
    vxoEdgeTraceThreshold_ValidateInput,
    vxoEdgeTraceThreshold_ValidateOutput
};

static vx_param_description_s internalkernel_edgeTrace_hysteresis_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status vxoInternalKernel_EdgeTraceHysteresis(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEdgeTraceHysteresis_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEdgeTraceHysteresis_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EdgeTraceHysteresis = {
    VX_KERNEL_INTERNAL_EDGE_TRACE_HYSTERESIS,
    "vivante.internal.edge.trace.hysteresis",
    vxoInternalKernel_EdgeTraceHysteresis,
    internalkernel_edgeTrace_hysteresis_params, vxmLENGTH_OF(internalkernel_edgeTrace_hysteresis_params),
    vxoEdgeTraceHysteresis_ValidateInput,
    vxoEdgeTraceHysteresis_ValidateOutput
};

static vx_param_description_s internalkernel_edgeTrace_clamp_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status vxoInternalKernel_EdgeTraceClamp(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEdgeTraceClamp_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEdgeTraceClamp_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EdgeTraceClamp = {
    VX_KERNEL_INTERNAL_EDGE_TRACE_CLAMP,
    "vivante.internal.edge.trace.clamp",
    vxoInternalKernel_EdgeTraceClamp,
    internalkernel_edgeTrace_clamp_params, vxmLENGTH_OF(internalkernel_edgeTrace_clamp_params),
    vxoEdgeTraceClamp_ValidateInput,
    vxoEdgeTraceClamp_ValidateOutput
};

static vx_param_description_s internalkernel_integral_image_step_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status vxoInternalKernel_IntegralImageStep(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoIntegralImageStep_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoIntegralImageStep_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_IntegralImageStep = {
    VX_KERNEL_INTERNAL_INTEGRAL_IMAGE_STEP,
    "vivante.internal.integral.image.step",
    vxoInternalKernel_IntegralImageStep,
    internalkernel_integral_image_step_params, vxmLENGTH_OF(internalkernel_integral_image_step_params),
    vxoIntegralImageStep_ValidateInput,
    vxoIntegralImageStep_ValidateOutput
};

static vx_param_description_s internalkernel_scharr3x3_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status vxoInternalKernel_Scharr3x3(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoScharr3x3_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoScharr3x3_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_Scharr3x3 = {
    VX_KERNEL_INTERNAL_SCHARR3x3,
    "vivante.internal.scharr3x3",
    vxoInternalKernel_Scharr3x3,
    internalkernel_scharr3x3_params, vxmLENGTH_OF(internalkernel_scharr3x3_params),
    vxoScharr3x3_ValidateInput,
    vxoScharr3x3_ValidateOutput
};

static vx_param_description_s internalkernel_vlk_tracker_params[] = {
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_VLKTracker(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoVLKTracker_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoVLKTracker_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_VLKTracker = {
    VX_KERNEL_INTERNAL_VLK_TRACKER,
    "vivante.internal.vlk.tracker",
    vxoInternalKernel_VLKTracker,
    internalkernel_vlk_tracker_params, vxmLENGTH_OF(internalkernel_vlk_tracker_params),
    vxoVLKTracker_ValidateInput,
    vxoVLKTracker_ValidateOutput
};

static vx_param_description_s internalkernel_equalize_histogram_hist_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status vxoInternalKernel_EqualizeHistogramHist(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEqualizeHistogramHist_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEqualizeHistogramHist_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EqualizeHistogramHist = {
    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_HIST,
    "vivante.internal.equalizehistogram.hist",
    vxoInternalKernel_EqualizeHistogramHist,
    internalkernel_equalize_histogram_hist_params, vxmLENGTH_OF(internalkernel_equalize_histogram_hist_params),
    vxoEqualizeHistogramHist_ValidateInput,
    vxoEqualizeHistogramHist_ValidateOutput
};

static vx_param_description_s internalkernel_equalize_histogram_gcdf_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status vxoInternalKernel_EqualizeHistogramGcdf(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEqualizeHistogramGcdf_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEqualizeHistogramGcdf_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EqualizeHistogramGcdf = {
    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_GCDF,
    "vivante.internal.equalizehistogram.gcdf",
    vxoInternalKernel_EqualizeHistogramGcdf,
    internalkernel_equalize_histogram_gcdf_params, vxmLENGTH_OF(internalkernel_equalize_histogram_gcdf_params),
    vxoEqualizeHistogramGcdf_ValidateInput,
    vxoEqualizeHistogramGcdf_ValidateOutput
};

static vx_param_description_s internalkernel_equalize_histogram_cdf_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoInternalKernel_EqualizeHistogramCdf(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEqualizeHistogramCdf_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEqualizeHistogramCdf_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EqualizeHistogramCdf = {
    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_CDF,
    "vivante.internal.equalizehistogram.cdf",
    vxoInternalKernel_EqualizeHistogramCdf,
    internalkernel_equalize_histogram_cdf_params, vxmLENGTH_OF(internalkernel_equalize_histogram_cdf_params),
    vxoEqualizeHistogramCdf_ValidateInput,
    vxoEqualizeHistogramCdf_ValidateOutput
};

static vx_param_description_s internalkernel_equalize_histogram_lut_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

VX_PRIVATE_API vx_status vxoInternalKernel_EqualizeHistogramLut(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoEqualizeHistogramLut_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoEqualizeHistogramLut_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EqualizeHistogramLut = {
    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_LUT,
    "vivante.internal.equalizehistogram.lut",
    vxoInternalKernel_EqualizeHistogramLut,
    internalkernel_equalize_histogram_lut_params, vxmLENGTH_OF(internalkernel_equalize_histogram_lut_params),
    vxoEqualizeHistogramLut_ValidateInput,
    vxoEqualizeHistogramLut_ValidateOutput
};

#endif