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


#ifndef _GC_INTERFACE_H_
#define _GC_INTERFACE_H_

#include <gc_vxk_common.h>

extern vx_kernel_description_s internalkernel_NNConvolutionReluCnnLayer;
extern vx_kernel_description_s internalkernel_NNConvolutionReluPoolingCnnLayer;
extern vx_kernel_description_s internalkernel_NNFullyConnectedReluLayer;
extern vx_kernel_description_s internalkernel_NNSoftmaxLayer;
extern vx_kernel_description_s internalkernel_NNNormalization;
extern vx_kernel_description_s internalkernel_NNPoolingLayer;
extern vx_kernel_description_s internalkernel_NNFullyConnectedLayer;
extern vx_kernel_description_s internalkernel_NNFullyConnectedLayer11;
extern vx_kernel_description_s internalkernel_NNActivationLayer;
extern vx_kernel_description_s internalkernel_NNTensorAdd;
extern vx_kernel_description_s internalkernel_NNTensorSub;
extern vx_kernel_description_s internalkernel_NNTensorMul;
extern vx_kernel_description_s internalkernel_NNTensorDiv;
extern vx_kernel_description_s internalkernel_NNTensorTrans;
extern vx_kernel_description_s internalkernel_NNLeakyReluLayer;
extern vx_kernel_description_s internalkernel_NNBatchNormLayer;
extern vx_kernel_description_s internalkernel_NNRPNLayer;
extern vx_kernel_description_s internalkernel_NNROIPoolLayer;
extern vx_kernel_description_s internalkernel_NNConcat2Layer;
extern vx_kernel_description_s internalkernel_NNConvolutionLayer;
extern vx_kernel_description_s internalkernel_NNConcatIndefiniteLayer;
extern vx_kernel_description_s internalkernel_NNReorgLayer;
extern vx_kernel_description_s internalkernel_NNDeConvolutionLayer;
extern vx_kernel_description_s internalkernel_NNL2NormalizeLayer;
extern vx_kernel_description_s internalkernel_NNL2NormalizeLayer2;
extern vx_kernel_description_s internalkernel_NNTensorCopy;
extern vx_kernel_description_s internalkernel_NNConvolutionReluPoolingCnnLayer2;
extern vx_kernel_description_s internalkernel_NNPoolingLayer2;
extern vx_kernel_description_s internalkernel_NNTensorReduceSum;
extern vx_kernel_description_s internalkernel_NNTensorPad;
extern vx_kernel_description_s internalkernel_NN_LSTMUnit;
extern vx_kernel_description_s internalkernel_NN_LSTMLayer;
extern vx_kernel_description_s internalkernel_NNReOrg2;
extern vx_kernel_description_s internalkernel_NNTensorRounding;
extern vx_kernel_description_s internalkernel_NNHashLUT;
extern vx_kernel_description_s internalkernel_NNLSHProjection;
extern vx_kernel_description_s internalkernel_NNReshape;
extern vx_kernel_description_s internalkernel_NNTensorScale;
extern vx_kernel_description_s internalkernel_NNRNNLayer;
extern vx_kernel_description_s internalkernel_NNSoftmaxLayer2;
extern vx_kernel_description_s internalkernel_NNSVDFLayer;
extern vx_kernel_description_s internalkernel_NNLUT2;
extern vx_kernel_description_s internalkernel_NNNormalizationLayer2;
extern vx_kernel_description_s internalkernel_NNAdapter;
extern vx_kernel_description_s internalkernel_NNTensorReverse;
extern vx_kernel_description_s internalkernel_NNYUV2RGBScale;
extern vx_kernel_description_s internalkernel_NNTensorMean;
extern vx_kernel_description_s internalkernel_NNTensorStrideSlice;
extern vx_kernel_description_s internalkernel_NNTensorSqueeze;
extern vx_kernel_description_s internalkernel_NNTensorPad2;
extern vx_kernel_description_s internalkernel_PReluLayer;
extern vx_kernel_description_s internalkernel_NNROIPoolReluLayer;

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Invalid(vx_node node, const vx_reference paramTable[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoInvalid_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoInvalid_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *info);
VX_PRIVATE_API vx_param_description_s  invalid_kernel_params[1];
VX_PRIVATE_API vx_kernel_description_s invalid_kernel = {
    VX_KERNEL_INVALID,
    "org.khronos.openvx.invalid",
    vxoBaseKernel_Invalid,
    invalid_kernel_params, 0,
    NULL,
    vxoInvalid_ValidateInput,
    vxoInvalid_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_ColorConvert(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoColorConvert_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoColorConvert_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
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
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_colorconvert = {
    VX_KERNEL_COLOR_CONVERT,
    "org.khronos.openvx.color_convert",
    vxoBaseKernel_ColorConvert,
    basekernel_colorConvert_params, vxmLENGTH_OF(basekernel_colorConvert_params),
    NULL,
    vxoColorConvert_ValidateInput,
    vxoColorConvert_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_ChannelExtract(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoChannelExtract_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoChannelExtract_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_channelExtract_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_ChannelCombine(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoChannelCombine_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoChannelCombine_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_channelextract = {
    VX_KERNEL_CHANNEL_EXTRACT,
    "org.khronos.openvx.channel_extract",
    vxoBaseKernel_ChannelExtract,
    basekernel_channelExtract_params, vxmLENGTH_OF(basekernel_channelExtract_params),
    NULL,
    vxoChannelExtract_ValidateInput,
    vxoChannelExtract_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_param_description_s basekernel_channelCombine_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_channelcombine = {
    VX_KERNEL_CHANNEL_COMBINE,
    "org.khronos.openvx.channel_combine",
    vxoBaseKernel_ChannelCombine,
    basekernel_channelCombine_params, vxmLENGTH_OF(basekernel_channelCombine_params),
    NULL,
    vxoChannelCombine_ValidateInput,
    vxoChannelCombine_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Sobel3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSobel3x3_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSobel3x3_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_Sobel3x3_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
};

vx_kernel_description_s basekernel_sobel3x3 = {
    VX_KERNEL_SOBEL_3x3,
    "org.khronos.openvx.sobel_3x3",
    vxoBaseKernel_Sobel3x3,
    basekernel_Sobel3x3_params, vxmLENGTH_OF(basekernel_Sobel3x3_params),
    NULL,
    vxoSobel3x3_ValidateInput,
    vxoSobel3x3_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Magnitude(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMagnitude_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMagnitude_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_magnitude_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_magnitude = {
    VX_KERNEL_MAGNITUDE,
    "org.khronos.openvx.magnitude",
    vxoBaseKernel_Magnitude,
    basekernel_magnitude_params, vxmLENGTH_OF(basekernel_magnitude_params),
    NULL,
    vxoMagnitude_ValidateInput,
    vxoMagnitude_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 2, 1 },
#endif
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Phase(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPhase_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPhase_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_phase_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_phase = {
    VX_KERNEL_PHASE,
    "org.khronos.openvx.phase",
    vxoBaseKernel_Phase,
    basekernel_phase_params, vxmLENGTH_OF(basekernel_phase_params),
    NULL,
    vxoPhase_ValidateInput,
    vxoPhase_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 2, 1 },
#endif
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_TableLookup(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTableLookup_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTableLookup_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_lut_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_LUT, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_lut = {
    VX_KERNEL_TABLE_LOOKUP,
    "org.khronos.openvx.table_lookup",
    vxoBaseKernel_TableLookup,
    basekernel_lut_params, vxmLENGTH_OF(basekernel_lut_params),
    NULL,
    vxoTableLookup_ValidateInput,
    vxoTableLookup_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_ScaleImage(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoScaleImage_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoScaleImage_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoScaleImage_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_scale_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e},
};

vx_kernel_description_s basekernel_scale_image = {
    VX_KERNEL_SCALE_IMAGE,
    "org.khronos.openvx.scale_image",
    vxoBaseKernel_ScaleImage,
    basekernel_scale_params, vxmLENGTH_OF(basekernel_scale_params),
    NULL,
    vxoScaleImage_ValidateInput,
    vxoScaleImage_ValidateOutput,
    vxoScaleImage_Initializer,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBasekernel_HalfscaleGaussian(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHalfscaleGaussian_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHalfscaleGaussian_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHalfscaleGaussian_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHalfscaleGaussian_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_halfscale_gaussian = {
    VX_KERNEL_HALFSCALE_GAUSSIAN,
    "org.khronos.openvx.halfscale_gaussian",
    vxoBasekernel_HalfscaleGaussian,
    basekernel_scale_params, vxmLENGTH_OF(basekernel_scale_params),
    NULL,
    vxoHalfscaleGaussian_ValidateInput,
    vxoHalfscaleGaussian_ValidateOutput,
    vxoHalfscaleGaussian_Initializer,
    vxoHalfscaleGaussian_Deinitializer,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Histogram(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHistogram_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHistogram_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_histogram_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_DISTRIBUTION, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_histogram = {
    VX_KERNEL_HISTOGRAM,
    "org.khronos.openvx.histogram",
    vxoBaseKernel_Histogram,
    basekernel_histogram_params, vxmLENGTH_OF(basekernel_histogram_params),
    NULL,
    vxoHistogram_ValidateInput,
    vxoHistogram_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_EqualizeHist(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHist_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHist_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHist_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHist_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_equalize_hist_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_equalize_hist = {
    VX_KERNEL_EQUALIZE_HISTOGRAM,
    "org.khronos.openvx.equalize_histogram",
    vxoBaseKernel_EqualizeHist,
    basekernel_equalize_hist_params, vxmLENGTH_OF(basekernel_equalize_hist_params),
    NULL,
    vxoEqualizeHist_ValidateInput,
    vxoEqualizeHist_ValidateOutput,
    vxoEqualizeHist_Initializer,
    vxoEqualizeHist_Deinitializer,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_AbsDiff(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAbsDiff_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAbsDiff_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_absdiff_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_absdiff = {
    VX_KERNEL_ABSDIFF,
    "org.khronos.openvx.absdiff",
    vxoBaseKernel_AbsDiff,
    basekernel_absdiff_params, vxmLENGTH_OF(basekernel_absdiff_params),
    NULL,
    vxoAbsDiff_ValidateInput,
    vxoAbsDiff_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_MeanStdDev(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMeanStdDev_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMeanStdDev_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_mean_stddev_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_mean_stddev = {
    VX_KERNEL_MEAN_STDDEV,
    "org.khronos.openvx.mean_stddev",
    vxoBaseKernel_MeanStdDev,
    basekernel_mean_stddev_params, vxmLENGTH_OF(basekernel_mean_stddev_params),
    NULL,
    vxoMeanStdDev_ValidateInput,
    vxoMeanStdDev_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Threshold(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoThreshold_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoThreshold_ValidatorOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_threshold_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_THRESHOLD, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s basekernel_threshold = {
    VX_KERNEL_THRESHOLD,
    "org.khronos.openvx.threshold",
    NULL,
    basekernel_threshold_params, vxmLENGTH_OF(basekernel_threshold_params),
    NULL,
    vxoThreshold_ValidateInput,
    vxoThreshold_ValidatorOutput,
    vxoBaseKernel_Threshold,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"threshold.vx"},
};


VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_IntegralImage(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoIntegral_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoIntegral_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoIntegral_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoIntegral_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_integral_image_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

vx_kernel_description_s basekernel_integral_image = {
    VX_KERNEL_INTEGRAL_IMAGE,
    "org.khronos.openvx.integral_image",
    vxoBaseKernel_IntegralImage,
    basekernel_integral_image_params, vxmLENGTH_OF(basekernel_integral_image_params),
    NULL,
    vxoIntegral_ValidateInput,
    vxoIntegral_ValidateOutput,
    vxoIntegral_Initializer,
    vxoIntegral_Deinitializer,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoMorphology_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMorphology_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s*ptr);
VX_PRIVATE_API vx_param_description_s basekernel_morphology_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Erode3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_erode3x3 = {
    VX_KERNEL_ERODE_3x3,
    "org.khronos.openvx.erode_3x3",
    vxoBaseKernel_Erode3x3,
    basekernel_morphology_params, vxmLENGTH_OF(basekernel_morphology_params),
    NULL,
    vxoMorphology_ValidateInput,
    vxoMorphology_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Dilate3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_dilate3x3 = {
    VX_KERNEL_DILATE_3x3,
    "org.khronos.openvx.dilate_3x3",
    vxoBaseKernel_Dilate3x3,
    basekernel_morphology_params, vxmLENGTH_OF(basekernel_morphology_params),
    NULL,
    vxoMorphology_ValidateInput,
    vxoMorphology_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoFilter_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoFilter_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_filter_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Box3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_box3x3 = {
    VX_KERNEL_BOX_3x3,
    "org.khronos.openvx.box_3x3:default",
    vxoBaseKernel_Box3x3,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    NULL,
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
};

vx_kernel_description_s basekernel_box3x3_2 = {
    VX_KERNEL_BOX_3x3,
    "org.khronos.openvx.box_3x3:duplicate",
    vxoBaseKernel_Box3x3,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    NULL,
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Median3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_median3x3 = {
    VX_KERNEL_MEDIAN_3x3,
    "org.khronos.openvx.median_3x3",
    vxoBaseKernel_Median3x3,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    NULL,
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Gaussian3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_gaussian3x3 = {
    VX_KERNEL_GAUSSIAN_3x3,
    "org.khronos.openvx.gaussian_3x3",
    vxoBaseKernel_Gaussian3x3,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    NULL,
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
};

VX_PRIVATE_API vx_param_description_s basekernel_convolution_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_CONVOLUTION, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBasekernel_Convolve(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoConvolve_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoConvolve_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_convolution = {
    VX_KERNEL_CUSTOM_CONVOLUTION,
    "org.khronos.openvx.custom_convolution",
    vxoBasekernel_Convolve,
    basekernel_convolution_params, vxmLENGTH_OF(basekernel_convolution_params),
    NULL,
    vxoConvolve_ValidateInput,
    vxoConvolve_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_pyramid_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Pyramid(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPyramid_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPyramid_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPyramid_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPyramid_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_pyramid = {
    VX_KERNEL_GAUSSIAN_PYRAMID,
    "org.khronos.openvx.gaussian_pyramid",
    vxoBaseKernel_Pyramid,
    basekernel_pyramid_params, vxmLENGTH_OF(basekernel_pyramid_params),
    NULL,
    vxoPyramid_ValidateInput,
    vxoPyramid_ValidateOutput,
    vxoPyramid_Initializer,
    vxoPyramid_Deinitializer,
};

#define OPENV_CTS_1_2_CONV5x5_ON 0

static vx_param_description_s basekernel_laplacian_pyramid_params[] =
{
    { VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_LaplacianPyramid(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacianPyramid_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacianPyramid_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacianPyramid_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacianPyramid_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_laplacian_pyramid = {
    VX_KERNEL_LAPLACIAN_PYRAMID,
    "org.khronos.openvx.laplacianpyramid",
    vxoBaseKernel_LaplacianPyramid,
    basekernel_laplacian_pyramid_params, vxmLENGTH_OF(basekernel_laplacian_pyramid_params),
    NULL,
    vxoLaplacianPyramid_ValidateInput,
    vxoLaplacianPyramid_ValidateOutput,
    vxoLaplacianPyramid_Initializer,
    vxoLaplacianPyramid_Deinitializer,
};

static vx_param_description_s basekernel_laplacian_reconstruct_params[] =
{
    { VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_LaplacianReconstruct(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacianReconstruct_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacianReconstruct_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacianReconstruct_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacianReconstruct_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_laplacian_reconstruct = {
    VX_KERNEL_LAPLACIAN_RECONSTRUCT,
    "org.khronos.openvx.laplacianreconstruct",
    vxoBaseKernel_LaplacianReconstruct,
    basekernel_laplacian_reconstruct_params, vxmLENGTH_OF(basekernel_laplacian_reconstruct_params),
    NULL,
    vxoLaplacianReconstruct_ValidateInput,
    vxoLaplacianReconstruct_ValidateOutput,
    vxoLaplacianReconstruct_Initializer,
    vxoLaplacianReconstruct_Deinitializer,
};


static vx_param_description_s basekernel_filter_kernel_params[] = {
    { VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e },
    { VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_INPUT, VX_TYPE_MATRIX, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
    { VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e },
};

VX_PRIVATE_API vx_status VX_CALLBACK vxNonLinearFilterInputValidator(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxNonLinearFilterOutputValidator(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxNonLinearFilterKernel(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_nonlinear_filter = {
    VX_KERNEL_NON_LINEAR_FILTER,
    "org.khronos.openvx.non_linear_filter",
    vxNonLinearFilterKernel,
    basekernel_filter_kernel_params, vxmLENGTH_OF(basekernel_filter_kernel_params),
    NULL,
    vxNonLinearFilterInputValidator,
    vxNonLinearFilterOutputValidator,
    NULL,
    NULL,
};

static vx_param_description_s accumulate_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_BIDIRECTIONAL, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

static vx_param_description_s accumulate_scaled_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_BIDIRECTIONAL, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Accumulate(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAccumulate_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAccumulate_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_accumulate = {
    VX_KERNEL_ACCUMULATE,
    "org.khronos.openvx.accumulate",
    vxoBaseKernel_Accumulate,
    accumulate_kernel_params, vxmLENGTH_OF(accumulate_kernel_params),
    NULL,
    vxoAccumulate_ValidateInput,
    vxoAccumulate_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_AccumulateWeighted(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAccumulateWeighted_ValidateInput(vx_node node, vx_uint32 index);
vx_kernel_description_s basekernel_accumulate_weighted = {
    VX_KERNEL_ACCUMULATE_WEIGHTED,
    "org.khronos.openvx.accumulate_weighted",
    vxoBaseKernel_AccumulateWeighted,
    accumulate_scaled_kernel_params, vxmLENGTH_OF(accumulate_scaled_kernel_params),
    NULL,
    vxoAccumulateWeighted_ValidateInput,
    vxoAccumulate_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_AccumulateSquare(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAccumulateSquared_ValidateInput(vx_node node, vx_uint32 index);
vx_kernel_description_s basekernel_accumulate_square = {
    VX_KERNEL_ACCUMULATE_SQUARE,
    "org.khronos.openvx.accumulate_square",
    vxoBaseKernel_AccumulateSquare,
    accumulate_scaled_kernel_params, vxmLENGTH_OF(accumulate_scaled_kernel_params),
    NULL,
    vxoAccumulateSquared_ValidateInput,
    vxoAccumulate_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_minmaxloc_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_MinMaxLoc(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxLoc_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxLoc_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxLoc_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxLoc_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_minmaxloc = {
    VX_KERNEL_MINMAXLOC,
    "org.khronos.openvx.minmaxloc",
    vxoBaseKernel_MinMaxLoc,
    basekernel_minmaxloc_params, vxmLENGTH_OF(basekernel_minmaxloc_params),
    NULL,
    vxoMinMaxLoc_ValidateInput,
    vxoMinMaxLoc_ValidateOutput,
    vxoMinMaxLoc_Initializer,
    vxoMinMaxLoc_Deinitializer,
};

VX_PRIVATE_API vx_df_image convertDepth_InputOutputFormat[][2] = {
    {VX_DF_IMAGE_U8, VX_DF_IMAGE_U16},
    {VX_DF_IMAGE_U8, VX_DF_IMAGE_S16},
    {VX_DF_IMAGE_U8, VX_DF_IMAGE_U32},
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
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_ConvertDepth(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoConvertDepth_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoConvertDepth_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_convertdepth = {
    VX_KERNEL_CONVERTDEPTH,
    "org.khronos.openvx.convertdepth",
    vxoBaseKernel_ConvertDepth,
    basekernel_convertdepth_params, vxmLENGTH_OF(basekernel_convertdepth_params),
    NULL,
    vxoConvertDepth_ValidateInput,
    vxoConvertDepth_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_canny_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_THRESHOLD, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_CannyEdge(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCannyEdge_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCannyEdge_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCannyEdge_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCannyEdge_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_canny = {
    VX_KERNEL_CANNY_EDGE_DETECTOR,
    "org.khronos.openvx.canny_edge_detector",
    vxoBaseKernel_CannyEdge,
    basekernel_canny_params, vxmLENGTH_OF(basekernel_canny_params),
    NULL,
    vxoCannyEdge_ValidateInput,
    vxoCannyEdge_ValidateOutput,
    vxoCannyEdge_Initializer,
    vxoCannyEdge_Deinitializer,
};

static vx_param_description_s basekernel_binary_bitwise_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_And(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoBinaryBitwise_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoBinaryBitwise_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_and = {
    VX_KERNEL_AND,
    "org.khronos.openvx.and",
    vxoBaseKernel_And,
    basekernel_binary_bitwise_params, vxmLENGTH_OF(basekernel_binary_bitwise_params),
    NULL,
    vxoBinaryBitwise_ValidateInput,
    vxoBinaryBitwise_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Or(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_or = {
    VX_KERNEL_OR,
    "org.khronos.openvx.or",
    vxoBaseKernel_Or,
    basekernel_binary_bitwise_params, vxmLENGTH_OF(basekernel_binary_bitwise_params),
    NULL,
    vxoBinaryBitwise_ValidateInput,
    vxoBinaryBitwise_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBasekernel_Xor(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_xor = {
    VX_KERNEL_XOR,
    "org.khronos.openvx.xor",
    vxoBasekernel_Xor,
    basekernel_binary_bitwise_params, vxmLENGTH_OF(basekernel_binary_bitwise_params),
    NULL,
    vxoBinaryBitwise_ValidateInput,
    vxoBinaryBitwise_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s unary_bitwise_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Not(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoUnaryBitwise_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoUnaryBitwise_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_not = {
    VX_KERNEL_NOT,
    "org.khronos.openvx.not",
    vxoBaseKernel_Not,
    unary_bitwise_kernel_params, vxmLENGTH_OF(unary_bitwise_kernel_params),
    NULL,
    vxoUnaryBitwise_ValidateInput,
    vxoUnaryBitwise_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_multiply_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Multiply(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMultiply_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMultiply_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_multiply = {
    VX_KERNEL_MULTIPLY,
    "org.khronos.openvx.multiply",
    vxoBaseKernel_Multiply,
    basekernel_multiply_params, vxmLENGTH_OF(basekernel_multiply_params),
    NULL,
    vxoMultiply_ValidateInput,
    vxoMultiply_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_add_subtract_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Add(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAddSubtract_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoAddSubtract_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_add = {
    VX_KERNEL_ADD,
    "org.khronos.openvx.add",
    vxoBaseKernel_Add,
    basekernel_add_subtract_params, vxmLENGTH_OF(basekernel_add_subtract_params),
    NULL,
    vxoAddSubtract_ValidateInput,
    vxoAddSubtract_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Sub(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_subtract = {
    VX_KERNEL_SUBTRACT,
    "org.khronos.openvx.subtract",
    vxoBaseKernel_Sub,
    basekernel_add_subtract_params, vxmLENGTH_OF(basekernel_add_subtract_params),
    NULL,
    vxoAddSubtract_ValidateInput,
    vxoAddSubtract_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_warp_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_MATRIX, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxWarpAffineKernel(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoWarpAffine_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoWarp_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_warp_affine = {
    VX_KERNEL_WARP_AFFINE,
    "org.khronos.openvx.warp_affine",
    vxWarpAffineKernel,
    basekernel_warp_params, vxmLENGTH_OF(basekernel_warp_params),
    NULL,
    vxoWarpAffine_ValidateInput,
    vxoWarp_ValidateOutput,
    NULL,
    NULL,
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_WarpPerspective(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoWarpPerspective_ValidateInput(vx_node node, vx_uint32 index);
vx_kernel_description_s basekernel_warp_perspective = {
    VX_KERNEL_WARP_PERSPECTIVE,
    "org.khronos.openvx.warp_perspective",
    vxoBaseKernel_WarpPerspective,
    basekernel_warp_params, vxmLENGTH_OF(basekernel_warp_params),
    NULL,
    vxoWarpPerspective_ValidateInput,
    vxoWarp_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s basekernel_harris_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_HarrisCorners(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHarris_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHarris_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHarris_Initializer(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHarris_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_harris = {
    VX_KERNEL_HARRIS_CORNERS,
    "org.khronos.openvx.harris_corners",
    vxoBaseKernel_HarrisCorners,
    basekernel_harris_params, vxmLENGTH_OF(basekernel_harris_params),
    NULL,
    vxoHarris_ValidateInput,
    vxoHarris_ValidateOutput,
    vxoHarris_Initializer,
    vxoHarris_Deinitializer,
};

static vx_param_description_s basekernel_fast9_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Fast9Corners(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoFast9_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoFast9_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoFast9_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoFast9_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_fast9 = {
    VX_KERNEL_FAST_CORNERS,
    "org.khronos.openvx.fast_corners",
    vxoBaseKernel_Fast9Corners,
    basekernel_fast9_params, vxmLENGTH_OF(basekernel_fast9_params),
    NULL,
    vxoFast9_ValidateInput,
    vxoFast9_ValidateOutput,
    vxoFast9_Initializer,
    vxoFast9_Deinitializer
};

static vx_param_description_s basekernel_optpyrlk_params[] = {
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_true_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_OpticalFlowPyrLK(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoOpticalFlowPyrLK_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoOpticalFlowPyrLK_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoOpticalFlowPyrLK_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoOpticalFlowPyrLK_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_optpyrlk = {
    VX_KERNEL_OPTICAL_FLOW_PYR_LK,
    "org.khronos.openvx.optical_flow_pyr_lk",
    vxoBaseKernel_OpticalFlowPyrLK,
    basekernel_optpyrlk_params, vxmLENGTH_OF(basekernel_optpyrlk_params),
    NULL,
    vxoOpticalFlowPyrLK_ValidateInput,
    vxoOpticalFlowPyrLK_ValidateOutput,
    vxoOpticalFlowPyrLK_Initializer,
    vxoOpticalFlowPyrLK_Deinitializer
};

static vx_param_description_s remap_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_REMAP, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_true_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Remap(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoRemap_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoRemap_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s basekernel_remap = {
    VX_KERNEL_REMAP,
    "org.khronos.openvx.remap",
    vxoBaseKernel_Remap,
    remap_kernel_params, vxmLENGTH_OF(remap_kernel_params),
    NULL,
    vxoRemap_ValidateInput,
    vxoRemap_ValidateOutput,
    NULL,
    NULL,
};

/* internal kernel */
static vx_param_description_s internlkernel_gradientMxN_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_SobelMxN(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoGradientMxN_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoGradientMxN_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_sobelMxN = {
    VX_KERNEL_INTERNAL_SOBEL_MxN,
    "org.khronos.internal.sobelMxN",
    vxoInternalKernel_SobelMxN,
    internlkernel_gradientMxN_params, vxmLENGTH_OF(internlkernel_gradientMxN_params),
    NULL,
    vxoGradientMxN_ValidateInput,
    vxoGradientMxN_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 1, 2 },
#endif
};

static vx_param_description_s internlkernel_gradientMxN_F16_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoGradientMxN_F16_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_SobelMxNF16(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_sobelMxN_f16 = {
    VX_KERNEL_INTERNAL_SOBEL_MxN_F16,
    "org.khronos.internal.sobelMxN_f16",
    vxoInternalKernel_SobelMxNF16,
    internlkernel_gradientMxN_F16_params, vxmLENGTH_OF(internlkernel_gradientMxN_F16_params),
    NULL,
    vxoGradientMxN_ValidateInput,
    vxoGradientMxN_F16_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 1, 2 },
#endif
};

static vx_param_description_s internalkernel_harrisscore_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_HarrisScore(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHarrisScore_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHarrisScore_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_harris_score = {
    VX_KERNEL_INTERNAL_HARRIS_SCORE,
    "org.khronos.internal.harris_score",
    vxoInternalKernel_HarrisScore,
    internalkernel_harrisscore_params, vxmLENGTH_OF(internalkernel_harrisscore_params),
    NULL,
    vxoHarrisScore_ValidateInput,
    vxoHarrisScore_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s intenralkernel_euclidean_non_max_suppression_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_EuclideanNonMaxSuppression(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEuclideanNonMax_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEuclideanNonMax_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_euclidian_nonmax = {
    VX_KERNEL_INTERNAL_EUCLIDEAN_NONMAXSUPPRESSION,
    "org.khronos.internal.euclidean_nonmaxsuppression",
    vxoInternalKernel_EuclideanNonMaxSuppression,
    intenralkernel_euclidean_non_max_suppression_params, vxmLENGTH_OF(intenralkernel_euclidean_non_max_suppression_params),
    NULL,
    vxoEuclideanNonMax_ValidateInput,
    vxoEuclideanNonMax_ValidateOutput,
};

static vx_param_description_s internalkernel_lister_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_ImageLister(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLister_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLister_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLister_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLister_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_lister = {
    VX_KERNEL_INTERNAL_IMAGE_LISTER,
    "org.khronos.internal.image_to_list",
    vxoInternalKernel_ImageLister,
    internalkernel_lister_params, vxmLENGTH_OF(internalkernel_lister_params),
    NULL,
    vxoLister_ValidateInput,
    vxoLister_ValidateOutput,
    vxoLister_Initializer,
    vxoLister_Deinitializer,
};

static vx_param_description_s internalkernel_norm_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_Norm(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNorm_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNorm_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_norm = {
    VX_KERNEL_INTERNAL_ELEMENTWISE_NORM,
    "org.khronos.internal.elementwise_norm",
    vxoInternalKernel_Norm,
    internalkernel_norm_params, vxmLENGTH_OF(internalkernel_norm_params),
    NULL,
    vxoNorm_ValidateInput,
    vxoNorm_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 2, 1 },
#endif
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_NormF16(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_norm_f16 = {
    VX_KERNEL_INTERNAL_ELEMENTWISE_NORM_F16,
    "org.khronos.internal.elementwise_norm_f16",
    vxoInternalKernel_NormF16,
    internalkernel_norm_params, vxmLENGTH_OF(internalkernel_norm_params),
    NULL,
    vxoNorm_ValidateInput,
    vxoNorm_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 2, 1 },
#endif
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_PhaseF16(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_kernel_description_s internalkernel_phase_f16 = {
    VX_KERNEL_INTERNAL_PHASE_F16,
    "org.khronos.internal.phase_f16",
    vxoInternalKernel_PhaseF16,
    basekernel_phase_params, vxmLENGTH_OF(basekernel_phase_params),
    NULL,
    vxoPhase_ValidateInput,
    vxoPhase_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 2, 1 },
#endif
};

static vx_param_description_s internalkernel_nonmaxsuppressioncanny_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_NonMaxSuppressionCanny(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNonMaxSuppressionCanny_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNonMaxSuppressionCanny_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_nonmaxcanny = {
    VX_KERNEL_INTERNAL_NONMAXSUPPRESSION_CANNY,
    "org.khronos.internal.nonmaximasuppressioncanny",
    vxoInternalKernel_NonMaxSuppressionCanny,
    internalkernel_nonmaxsuppressioncanny_params, vxmLENGTH_OF(internalkernel_nonmaxsuppressioncanny_params),
    NULL,
    vxoNonMaxSuppressionCanny_ValidateInput,
    vxoNonMaxSuppressionCanny_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_true_e, vx_true_e, 2, 0, 0, 0, 1, 1 },
#endif
};

static vx_param_description_s internalkernel_edge_trace_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_THRESHOLD, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_EdgeTrace(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTrace_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTrace_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTrace_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTrace_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_edge_trace = {
    VX_KERNEL_INTERNAL_EDGE_TRACE,
    "org.khronos.extra.edge_trace",
    vxoInternalKernel_EdgeTrace,
    internalkernel_edge_trace_params, vxmLENGTH_OF(internalkernel_edge_trace_params),
    NULL,
    vxoEdgeTrace_ValidateInput,
    vxoEdgeTrace_ValidateOutput,
    vxoEdgeTrace_Initializer,
    vxoEdgeTrace_Deinitializer,
};

static vx_param_description_s internalkernel_sgm_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_SGM(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSGM_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSGM_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSGM_Initializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSGM_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s internalkernel_sgm = {
    VX_KERNEL_INTERNAL_SGM,
    "vivante.internal.sgm",
    vxoInternalKernel_SGM,
    internalkernel_sgm_params, vxmLENGTH_OF(internalkernel_sgm_params),
    NULL,
    vxoSGM_ValidateInput,
    vxoSGM_ValidateOutput,
    vxoSGM_Initializer,
    vxoSGM_Deinitializer,
};

static vx_param_description_s internalkernel_copy_image_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_CopyImage(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCopyImage_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCopyImage_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_copy_image = {
    VX_KERNEL_INTERNAL_COPY_IMAGE,
    "vivante.internal.copy_image",
    vxoInternalKernel_CopyImage,
    internalkernel_copy_image_params, vxmLENGTH_OF(internalkernel_copy_image_params),
    NULL,
    vxoCopyImage_ValidateInput,
    vxoCopyImage_ValidateOutput,
    NULL,
    NULL,
};


static vx_param_description_s internalkernel_fast9corners_strength_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},

};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_Fast9CornersStrength(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoFast9CornersStrength_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoFast9CornersStrength_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_Fast9Corners_Strength = {
    VX_KERNEL_INTERNAL_FAST9CORNERS_STRENGTH,
    "vivante.internal.fast9corners_strength",
    vxoInternalKernel_Fast9CornersStrength,
    internalkernel_fast9corners_strength_params, vxmLENGTH_OF(internalkernel_fast9corners_strength_params),
    NULL,
    vxoFast9CornersStrength_ValidateInput,
    vxoFast9CornersStrength_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_fast9corners_nonmax_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_Fast9CornersNonMax(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoFast9CornersNonMax_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoFast9CornersNonMax_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalKernel_Fast9Corners_NonMax = {
    VX_KERNEL_INTERNAL_FAST9CORNERS_NONMAX,
    "vivante.internal.fast9corners_nonmax",
    vxoInternalKernel_Fast9CornersNonMax,
    internalkernel_fast9corners_nonmax_params, vxmLENGTH_OF(internalkernel_fast9corners_nonmax_params),
    NULL,
    vxoFast9CornersNonMax_ValidateInput,
    vxoFast9CornersNonMax_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_createlister_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_CreateLister(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCreateLister_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCreateLister_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_CreateLister = {
    VX_KERNEL_INTERNAL_CREATE_LISTER,
    "vivante.internal.createlister",
    vxoInternalKernel_CreateLister,
    internalkernel_createlister_params, vxmLENGTH_OF(internalkernel_createlister_params),
    NULL,
    vxoCreateLister_ValidateInput,
    vxoCreateLister_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_pack_arrays_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_PackArrays(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPackArrays_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPackArrays_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_PackArrays = {
    VX_KERNEL_INTERNAL_PACK_ARRAYS,
    "vivante.internal.packarrays",
    vxoInternalKernel_PackArrays,
    internalkernel_pack_arrays_params, vxmLENGTH_OF(internalkernel_pack_arrays_params),
    NULL,
    vxoPackArrays_ValidateInput,
    vxoPackArrays_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_minmaxloc_pack_arrays_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_MinMaxlocPackArrays(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxLocPackArrays_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxLocPackArrays_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_MinMacLocPackArrays = {
    VX_KERNEL_INTERNAL_MIN_MAX_LOC_PACK_ARRAYS,
    "vivante.internal.minmaxloc.packarrays",
    vxoInternalKernel_MinMaxlocPackArrays,
    internalkernel_minmaxloc_pack_arrays_params, vxmLENGTH_OF(internalkernel_minmaxloc_pack_arrays_params),
    NULL,
    vxoMinMaxLocPackArrays_ValidateInput,
    vxoMinMaxLocPackArrays_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_minmaxloc_filter_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_MinMaxLocFilter(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxLocFilter_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxLocFilter_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_MinMaxLocFilter = {
    VX_KERNEL_INTERNAL_MINMAXLOC_FILTER,
    "vivante.internal.minmaxlocfilter",
    vxoInternalKernel_MinMaxLocFilter,
    internalkernel_minmaxloc_filter_params, vxmLENGTH_OF(internalkernel_minmaxloc_filter_params),
    NULL,
    vxoMinMaxLocFilter_ValidateInput,
    vxoMinMaxLocFilter_ValidateOutput,
    NULL,
    NULL,
};

static vx_param_description_s internalkernel_minmax_get_location_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_MinMaxGetLocation(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxGetLocation_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMinMaxGetLocation_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_MinMaxGetLocation = {
    VX_KERNEL_INTERNAL_GET_LOCATION,
    "vivante.internal.minmax.getlocation",
    vxoInternalKernel_MinMaxGetLocation,
    internalkernel_minmax_get_location_params, vxmLENGTH_OF(internalkernel_minmax_get_location_params),
    NULL,
    vxoMinMaxGetLocation_ValidateInput,
    vxoMinMaxGetLocation_ValidateOutput
};

static vx_param_description_s internalkernel_edgeTrace_threshold_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_THRESHOLD, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_EdgeTraceThreshold(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTraceThreshold_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTraceThreshold_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EdgeTraceThreshold = {
    VX_KERNEL_INTERNAL_EDGE_TRACE_THRESHOLD,
    "vivante.internal.edge.trace.threshold",
    vxoInternalKernel_EdgeTraceThreshold,
    internalkernel_edgeTrace_threshold_params, vxmLENGTH_OF(internalkernel_edgeTrace_threshold_params),
    NULL,
    vxoEdgeTraceThreshold_ValidateInput,
    vxoEdgeTraceThreshold_ValidateOutput
};

static vx_param_description_s internalkernel_edgeTrace_hysteresis_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_EdgeTraceHysteresis(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTraceHysteresis_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTraceHysteresis_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EdgeTraceHysteresis = {
    VX_KERNEL_INTERNAL_EDGE_TRACE_HYSTERESIS,
    "vivante.internal.edge.trace.hysteresis",
    vxoInternalKernel_EdgeTraceHysteresis,
    internalkernel_edgeTrace_hysteresis_params, vxmLENGTH_OF(internalkernel_edgeTrace_hysteresis_params),
    NULL,
    vxoEdgeTraceHysteresis_ValidateInput,
    vxoEdgeTraceHysteresis_ValidateOutput
};

static vx_param_description_s internalkernel_edgeTrace_clamp_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_EdgeTraceClamp(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTraceClamp_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEdgeTraceClamp_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EdgeTraceClamp = {
    VX_KERNEL_INTERNAL_EDGE_TRACE_CLAMP,
    "vivante.internal.edge.trace.clamp",
    vxoInternalKernel_EdgeTraceClamp,
    internalkernel_edgeTrace_clamp_params, vxmLENGTH_OF(internalkernel_edgeTrace_clamp_params),
    NULL,
    vxoEdgeTraceClamp_ValidateInput,
    vxoEdgeTraceClamp_ValidateOutput
};

static vx_param_description_s internalkernel_integral_image_step_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_IntegralImageStep(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoIntegralImageStep_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoIntegralImageStep_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_IntegralImageStep = {
    VX_KERNEL_INTERNAL_INTEGRAL_IMAGE_STEP,
    "vivante.internal.integral.image.step",
    vxoInternalKernel_IntegralImageStep,
    internalkernel_integral_image_step_params, vxmLENGTH_OF(internalkernel_integral_image_step_params),
    NULL,
    vxoIntegralImageStep_ValidateInput,
    vxoIntegralImageStep_ValidateOutput
};

static vx_param_description_s internalkernel_scharr3x3_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_Scharr3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoScharr3x3_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoScharr3x3_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_Scharr3x3 = {
    VX_KERNEL_INTERNAL_SCHARR3x3,
    "vivante.internal.scharr3x3",
    vxoInternalKernel_Scharr3x3,
    internalkernel_scharr3x3_params, vxmLENGTH_OF(internalkernel_scharr3x3_params),
    NULL,
    vxoScharr3x3_ValidateInput,
    vxoScharr3x3_ValidateOutput
};

static vx_param_description_s internalkernel_vlk_tracker_params[] = {
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_PYRAMID, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_VLKTracker(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoVLKTracker_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoVLKTracker_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_VLKTracker = {
    VX_KERNEL_INTERNAL_VLK_TRACKER,
    "vivante.internal.vlk.tracker",
    vxoInternalKernel_VLKTracker,
    internalkernel_vlk_tracker_params, vxmLENGTH_OF(internalkernel_vlk_tracker_params),
    NULL,
    vxoVLKTracker_ValidateInput,
    vxoVLKTracker_ValidateOutput
};

static vx_param_description_s internalkernel_equalize_histogram_hist_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_EqualizeHistogramHist(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHistogramHist_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHistogramHist_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EqualizeHistogramHist = {
    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_HIST,
    "vivante.internal.equalizehistogram.hist",
    vxoInternalKernel_EqualizeHistogramHist,
    internalkernel_equalize_histogram_hist_params, vxmLENGTH_OF(internalkernel_equalize_histogram_hist_params),
    NULL,
    vxoEqualizeHistogramHist_ValidateInput,
    vxoEqualizeHistogramHist_ValidateOutput
};

static vx_param_description_s internalkernel_equalize_histogram_gcdf_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_EqualizeHistogramGcdf(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHistogramGcdf_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHistogramGcdf_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EqualizeHistogramGcdf = {
    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_GCDF,
    "vivante.internal.equalizehistogram.gcdf",
    vxoInternalKernel_EqualizeHistogramGcdf,
    internalkernel_equalize_histogram_gcdf_params, vxmLENGTH_OF(internalkernel_equalize_histogram_gcdf_params),
    NULL,
    vxoEqualizeHistogramGcdf_ValidateInput,
    vxoEqualizeHistogramGcdf_ValidateOutput
};

static vx_param_description_s internalkernel_equalize_histogram_cdf_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_EqualizeHistogramCdf(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHistogramCdf_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHistogramCdf_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EqualizeHistogramCdf = {
    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_CDF,
    "vivante.internal.equalizehistogram.cdf",
    vxoInternalKernel_EqualizeHistogramCdf,
    internalkernel_equalize_histogram_cdf_params, vxmLENGTH_OF(internalkernel_equalize_histogram_cdf_params),
    NULL,
    vxoEqualizeHistogramCdf_ValidateInput,
    vxoEqualizeHistogramCdf_ValidateOutput
};

static vx_param_description_s internalkernel_equalize_histogram_lut_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_EqualizeHistogramLut(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHistogramLut_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoEqualizeHistogramLut_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_EqualizeHistogramLut = {
    VX_KERNEL_INTERNAL_EQUALIZE_HISTOGRAM_LUT,
    "vivante.internal.equalizehistogram.lut",
    vxoInternalKernel_EqualizeHistogramLut,
    internalkernel_equalize_histogram_lut_params, vxmLENGTH_OF(internalkernel_equalize_histogram_lut_params),
    NULL,
    vxoEqualizeHistogramLut_ValidateInput,
    vxoEqualizeHistogramLut_ValidateOutput
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Laplacian3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s basekernel_laplacian3x3 = {
    VX_KERNEL_INTERNAL_LAPLACIAN3x3,
    "org.khronos.openvx.laplacian3x3",
    vxoBaseKernel_Laplacian3x3,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    NULL,
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
};

static vx_param_description_s internalkernel_sgm_cost_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_SgmCost(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmCost_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmCost_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_sgm_cost = {
    VX_KERNEL_INTERNAL_SGM_COST,
    "vivante.internal.sgm.cost",
    vxoInternalKernel_SgmCost,
    internalkernel_sgm_cost_params, vxmLENGTH_OF(internalkernel_sgm_cost_params),
    NULL,
    vxoSgmCost_ValidateInput,
    vxoSgmCost_ValidateOutput
};

static vx_param_description_s internalkernel_sgm_path90_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_SgmPath90(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmPath90_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmPath90_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_sgm_path90 = {
    VX_KERNEL_INTERNAL_SGM_PATH90,
    "vivante.internal.sgm.path90",
    vxoInternalKernel_SgmPath90,
    internalkernel_sgm_path90_params, vxmLENGTH_OF(internalkernel_sgm_path90_params),
    NULL,
    vxoSgmPath90_ValidateInput,
    vxoSgmPath90_ValidateOutput
};

static vx_param_description_s internalkernel_sgm_path45_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_SgmPath45(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmPath45_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmPath45_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_sgm_path45 = {
    VX_KERNEL_INTERNAL_SGM_PATH45,
    "vivante.internal.sgm.path45",
    vxoInternalKernel_SgmPath45,
    internalkernel_sgm_path45_params, vxmLENGTH_OF(internalkernel_sgm_path45_params),
    NULL,
    vxoSgmPath45_ValidateInput,
    vxoSgmPath45_ValidateOutput
};

static vx_param_description_s internalkernel_sgm_path135_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_SgmPath135(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmPath135_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmPath135_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_sgm_path135 = {
    VX_KERNEL_INTERNAL_SGM_PATH135,
    "vivante.internal.sgm.path135",
    vxoInternalKernel_SgmPath135,
    internalkernel_sgm_path135_params, vxmLENGTH_OF(internalkernel_sgm_path135_params),
    NULL,
    vxoSgmPath135_ValidateInput,
    vxoSgmPath135_ValidateOutput
};

static vx_param_description_s internalkernel_sgm_path0_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_SgmPath0(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmPath0_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmPath0_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_sgm_path0 = {
    VX_KERNEL_INTERNAL_SGM_PATH0,
    "vivante.internal.sgm.path0",
    vxoInternalKernel_SgmPath0,
    internalkernel_sgm_path0_params, vxmLENGTH_OF(internalkernel_sgm_path0_params),
    NULL,
    vxoSgmPath0_ValidateInput,
    vxoSgmPath0_ValidateOutput
};

static vx_param_description_s internalkernel_sgm_disp_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_SgmDisp(vx_node node, const vx_reference parameters[], vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmDisp_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSgmDisp_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format meta);
vx_kernel_description_s internalkernel_sgm_disp = {
    VX_KERNEL_INTERNAL_SGM_DISP,
    "vivante.internal.sgm.disp",
    vxoInternalKernel_SgmDisp,
    internalkernel_sgm_disp_params, vxmLENGTH_OF(internalkernel_sgm_disp_params),
    NULL,
    vxoSgmDisp_ValidateInput,
    vxoSgmDisp_ValidateOutput
};

static vx_param_description_s internalkernel_laplacian3x3_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_Laplacian3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacian3x3_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLaplacian3x3_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_Laplacian3x3 = {
    VX_KERNEL_INTERNAL_LAPLACIAN3x3,
    "vivante.internal.laplacian3x3",
    vxoInternalKernel_Laplacian3x3,
    internalkernel_laplacian3x3_params, vxmLENGTH_OF(internalkernel_laplacian3x3_params),
    NULL,
    vxoLaplacian3x3_ValidateInput,
    vxoLaplacian3x3_ValidateOutput
};

static vx_param_description_s internalkernel_census3x3_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED, vx_false_e}
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalKernel_Census3x3(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCensus3x3_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCensus3x3_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalKernel_census3x3 = {
    VX_KERNEL_INTERNAL_CENSUS3x3,
    "vivante.internal.census3x3",
    vxoInternalKernel_Census3x3,
    internalkernel_census3x3_params, vxmLENGTH_OF(internalkernel_census3x3_params),
    NULL,
    vxoCensus3x3_ValidateInput,
    vxoCensus3x3_ValidateOutput
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoMax_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMax_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMax_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_max_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s basekernel_max = {
    VX_KERNEL_MAX,
    "org.khronos.openvx.max",
    NULL,
    basekernel_max_params, vxmLENGTH_OF(basekernel_max_params),
    VX_NULL,
    vxoMax_ValidateInput,
    vxoMax_ValidateOutput,
    vxoMax_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"max.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoMin_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMin_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMin_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_min_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s basekernel_min = {
    VX_KERNEL_MIN,
    "org.khronos.openvx.min",
    NULL,
    basekernel_min_params, vxmLENGTH_OF(basekernel_min_params),
    VX_NULL,
    vxoMin_ValidateInput,
    vxoMin_ValidateOutput,
    vxoMin_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"min.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoNon_max_suppression_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNon_max_suppression_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoNon_max_suppression_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_non_max_suppression_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s basekernel_non_max_suppression = {
    VX_KERNEL_NON_MAX_SUPPRESSION,
    "org.khronos.openvx.non_max_suppression",
    VX_NULL,
    basekernel_non_max_suppression_params, vxmLENGTH_OF(basekernel_non_max_suppression_params),
    VX_NULL,
    vxoNon_max_suppression_ValidateInput,
    vxoNon_max_suppression_ValidateOutput,
    vxoNon_max_suppression_Initialize,
    VX_NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"non_max_suppression.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoMatch_template_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMatch_template_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMatch_template_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_match_template_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s basekernel_match_template = {
    VX_KERNEL_MATCH_TEMPLATE,
    "org.khronos.openvx.match_template",
    NULL,
    basekernel_match_template_params, vxmLENGTH_OF(basekernel_match_template_params),
    VX_NULL,
    vxoMatch_template_ValidateInput,
    vxoMatch_template_ValidateOutput,
    vxoMatch_template_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"match_template.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoLbp_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLbp_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLbp_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_lbp_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s basekernel_lbp = {
    VX_KERNEL_LBP,
    "org.khronos.openvx.lbp",
    NULL,
    basekernel_lbp_params, vxmLENGTH_OF(basekernel_lbp_params),
    VX_NULL,
    vxoLbp_ValidateInput,
    vxoLbp_ValidateOutput,
    vxoLbp_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"lbp.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoHoughMakepoints_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHoughMakepoints_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHoughMakepoints_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_hough_makepoints_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s internalkernel_hough_makepoints = {
    VX_KERNEL_INTERNAL_HOUGH_MAKEPOINTS,
    "org.khronos.internal.hough.makepoints",
    NULL,
    internalkernel_hough_makepoints_params, vxmLENGTH_OF(internalkernel_hough_makepoints_params),
    VX_NULL,
    vxoHoughMakepoints_ValidateInput,
    vxoHoughMakepoints_ValidateOutput,
    vxoHoughMakepoints_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"makepoints.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoHoughFillaccum_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHoughFillaccum_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHoughFillaccum_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_hough_fillaccum_params[] = {
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s internalkernel_hough_fillaccum = {
    VX_KERNEL_INTERNAL_HOUGH_FILLACCUM,
    "org.khronos.internal.hough.fillaccum",
    NULL,
    internalkernel_hough_fillaccum_params, vxmLENGTH_OF(internalkernel_hough_fillaccum_params),
    VX_NULL,
    vxoHoughFillaccum_ValidateInput,
    vxoHoughFillaccum_ValidateOutput,
    vxoHoughFillaccum_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"fillaccum.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoHoughGetlines_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHoughGetlines_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHoughGetlines_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_hough_getlines_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s internalkernel_hough_getlines = {
    VX_KERNEL_INTERNAL_HOUGH_GETLINES,
    "org.khronos.internal.hough.getlines",
    NULL,
    internalkernel_hough_getlines_params, vxmLENGTH_OF(internalkernel_hough_getlines_params),
    VX_NULL,
    vxoHoughGetlines_ValidateInput,
    vxoHoughGetlines_ValidateOutput,
    vxoHoughGetlines_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"getlines.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_hough_lines_p(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHough_lines_p_Deinitialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHough_lines_p_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHough_lines_p_Input_Validate(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHough_lines_p_Output_Validate(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_hough_lines_p_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s basekernel_hough_lines_p = {
    VX_KERNEL_HOUGH_LINES_P,
    "org.khronos.openvx.hough_lines_p",
    vxoBaseKernel_hough_lines_p,
    basekernel_hough_lines_p_params, vxmLENGTH_OF(basekernel_hough_lines_p_params),
    NULL,
    vxoHough_lines_p_Input_Validate,
    vxoHough_lines_p_Output_Validate,
    vxoHough_lines_p_Initialize,
    vxoHough_lines_p_Deinitialize,
    {NULL},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorLUT_Validate(vx_node node, const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[]);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorLUT_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);

VX_PRIVATE_API vx_param_description_s basekernel_tensorlut_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_LUT, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s basekernel_tensorlut = {
    VX_KERNEL_TENSOR_TABLE_LOOKUP,
    "org.khronos.openvx.tensor_table_lookup",
    NULL,
    basekernel_tensorlut_params, vxmLENGTH_OF(basekernel_tensorlut_params),
    vxoTensorLUT_Validate,
    NULL,
    NULL,
    vxoTensorLUT_Initialize,
    NULL,
    {"tensorlut.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensor_convert_depth_Validate(vx_node node, const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[]);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTensor_convert_depth_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_tensor_convert_depth_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s basekernel_tensor_convert_depth = {
    VX_KERNEL_TENSOR_CONVERT_DEPTH,
    "org.khronos.openvx.tensor_convert_depth",
    NULL,
    basekernel_tensor_convert_depth_params, vxmLENGTH_OF(basekernel_tensor_convert_depth_params),
    vxoTensor_convert_depth_Validate,
    NULL,
    NULL,
    vxoTensor_convert_depth_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"tensor_convert_depth.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensor_matrix_multiply(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTensor_matrix_multiply_Validate(vx_node node, const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[]);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTensor_matrix_multiply_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTensor_matrix_multiply_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_tensor_matrix_multiply_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s basekernel_tensor_matrix_multiply = {
    VX_KERNEL_TENSOR_MATRIX_MULTIPLY,
    "org.khronos.openvx.tensor_matrix_multiply",
    vxoTensor_matrix_multiply,
    basekernel_tensor_matrix_multiply_params, vxmLENGTH_OF(basekernel_tensor_matrix_multiply_params),
    vxoTensor_matrix_multiply_Validate,
    NULL,
    NULL,
    vxoTensor_matrix_multiply_Initialize,
    vxoTensor_matrix_multiply_Deinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {NULL},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoImageCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoImageCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoImageCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_image_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s internalkernel_image_copy = {
    VX_KERNEL_INTERNAL_IMAGE_COPY,
    "org.khronos.internal.image.copy",
    NULL,
    internalkernel_image_copy_params, vxmLENGTH_OF(internalkernel_image_copy_params),
    VX_NULL,
    vxoImageCopy_ValidateInput,
    vxoImageCopy_ValidateOutput,
    vxoImageCopy_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"imageCopy.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoScalarCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoScalarCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoScalarCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_scalar_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s internalkernel_scalar_copy = {
    VX_KERNEL_INTERNAL_SCALAR_COPY,
    "org.khronos.internal.scalar.copy",
    NULL,
    internalkernel_scalar_copy_params, vxmLENGTH_OF(internalkernel_scalar_copy_params),
    VX_NULL,
    vxoScalarCopy_ValidateInput,
    vxoScalarCopy_ValidateOutput,
    vxoScalarCopy_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"scalarCopy.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoArrayCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoArrayCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoArrayCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_array_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internalkernel_array_copy = {
    VX_KERNEL_INTERNAL_ARRAY_COPY,
    "org.khronos.internal.array.copy",
    NULL,
    internalkernel_array_copy_params, vxmLENGTH_OF(internalkernel_array_copy_params),
    VX_NULL,
    vxoArrayCopy_ValidateInput,
    vxoArrayCopy_ValidateOutput,
    vxoArrayCopy_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"arrayCopy.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoLutCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLutCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLutCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_lut_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internalkernel_lut_copy = {
    VX_KERNEL_INTERNAL_LUT_COPY,
    "org.khronos.internal.lut.copy",
    NULL,
    internalkernel_lut_copy_params, vxmLENGTH_OF(internalkernel_lut_copy_params),
    VX_NULL,
    vxoLutCopy_ValidateInput,
    vxoLutCopy_ValidateOutput,
    vxoLutCopy_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"lutCopy.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoMatrixCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMatrixCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMatrixCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_matrix_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internalkernel_matrix_copy = {
    VX_KERNEL_INTERNAL_MATRIX_COPY,
    "org.khronos.internal.matrix.copy",
    NULL,
    internalkernel_matrix_copy_params, vxmLENGTH_OF(internalkernel_matrix_copy_params),
    VX_NULL,
    vxoMatrixCopy_ValidateInput,
    vxoMatrixCopy_ValidateOutput,
    vxoMatrixCopy_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"copy.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoConvolutionCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoConvolutionCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoConvolutionCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_convolution_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internalkernel_convolution_copy = {
    VX_KERNEL_INTERNAL_CONVOLUTION_COPY,
    "org.khronos.internal.convolution.copy",
    NULL,
    internalkernel_convolution_copy_params, vxmLENGTH_OF(internalkernel_convolution_copy_params),
    VX_NULL,
    vxoConvolutionCopy_ValidateInput,
    vxoConvolutionCopy_ValidateOutput,
    vxoConvolutionCopy_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"copy.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoDistributtionCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoDistributtionCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoDistributtionCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_distributtion_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internalkernel_distributtion_copy = {
    VX_KERNEL_INTERNAL_DISTRIBUTION_COPY,
    "org.khronos.internal.distributtion.copy",
    NULL,
    internalkernel_distributtion_copy_params, vxmLENGTH_OF(internalkernel_distributtion_copy_params),
    VX_NULL,
    vxoDistributtionCopy_ValidateInput,
    vxoDistributtionCopy_ValidateOutput,
    vxoDistributtionCopy_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"copy.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTensorCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_tensor_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internalkernel_tensor_copy = {
    VX_KERNEL_INTERNAL_TENSOR_COPY,
    "org.khronos.internal.tensor.copy",
    NULL,
    internalkernel_tensor_copy_params, vxmLENGTH_OF(internalkernel_tensor_copy_params),
    VX_NULL,
    vxoTensorCopy_ValidateInput,
    vxoTensorCopy_ValidateOutput,
    vxoTensorCopy_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"copy.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoThresholdCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoThresholdCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoThresholdCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_threshold_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internalkernel_threshold_copy = {
    VX_KERNEL_INTERNAL_THRESHOLD_COPY,
    "org.khronos.internal.threshold.copy",
    vxoThresholdCopy_Initialize,
    internalkernel_threshold_copy_params, vxmLENGTH_OF(internalkernel_threshold_copy_params),
    VX_NULL,
    vxoThresholdCopy_ValidateInput,
    vxoThresholdCopy_ValidateOutput,
    NULL,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {NULL},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoRemapCopy_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoRemapCopy_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoRemapCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_remap_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internalkernel_remap_copy = {
    VX_KERNEL_INTERNAL_REMAP_COPY,
    "org.khronos.internal.remap.copy",
    NULL,
    internalkernel_remap_copy_params, vxmLENGTH_OF(internalkernel_remap_copy_params),
    VX_NULL,
    vxoRemapCopy_ValidateInput,
    vxoRemapCopy_ValidateOutput,
    vxoRemapCopy_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"remapCopy.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Copy(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCopy_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCopy_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoCopy_Validate(vx_node node, const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[]);
VX_PRIVATE_API vx_param_description_s basekernel_copy_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s basekernel_copy = {
    VX_KERNEL_COPY,
    "org.khronos.openvx.copy",
    vxoBaseKernel_Copy,
    basekernel_copy_params, vxmLENGTH_OF(basekernel_copy_params),
    vxoCopy_Validate,
    NULL,
    NULL,
    vxoCopy_Initialize,
    vxoCopy_Deinitializer,
    {NULL},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxBaseKernelScalarOperation_Validator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[]);
VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernelScalarOperation_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernelScalarOperation_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_ScalarOperation(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_scalar_operation_params[] = {
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s basekernel_scalar_operation = {
    VX_KERNEL_SCALAR_OPERATION,
    "org.khronos.openvx.scalar_operation",
    vxoBaseKernel_ScalarOperation,
    basekernel_scalar_operation_params, vxmLENGTH_OF(basekernel_scalar_operation_params),
    vxBaseKernelScalarOperation_Validator,
    NULL,
    NULL,
    vxoBaseKernelScalarOperation_Initialize,
    vxoBaseKernelScalarOperation_Deinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {NULL},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalScalar_operation_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalScalar_operation_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalScalar_operation_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_scalar_operation_params[] = {
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internalkernel_scalar_operation = {
    VX_KERNEL_INTERNAL_SCALAR_OPERATION,
    "org.khronos.internal.scalar_operation",
    NULL,
    internalkernel_scalar_operation_params, vxmLENGTH_OF(internalkernel_scalar_operation_params),
    VX_NULL,
    vxoInternalScalar_operation_ValidateInput,
    vxoInternalScalar_operation_ValidateOutput,
    vxoInternalScalar_operation_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"scalar_operation.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoHog_cells_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHog_cells_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHog_cells_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_hog_cells_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s basekernel_hog_cells = {
    VX_KERNEL_HOG_CELLS,
    "org.khronos.openvx.hog_cells",
    NULL,
    basekernel_hog_cells_params, vxmLENGTH_OF(basekernel_hog_cells_params),
    VX_NULL,
    vxoHog_cells_ValidateInput,
    vxoHog_cells_ValidateOutput,
    vxoHog_cells_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"hog_cells.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoHog_features_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHog_features_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoHog_features_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_hog_features_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s basekernel_hog_features = {
    VX_KERNEL_HOG_FEATURES,
    "org.khronos.openvx.hog_features",
    NULL,
    basekernel_hog_features_params, vxmLENGTH_OF(basekernel_hog_features_params),
    VX_NULL,
    vxoHog_features_ValidateInput,
    vxoHog_features_ValidateOutput,
    vxoHog_features_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"hog_features.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxBilateralFilterValidator(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[]);
VX_PRIVATE_API vx_status VX_CALLBACK vxoBilateral_filter_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoBilateral_filter_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_BilateralFilter(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s basekernel_bilateral_filter_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s basekernel_bilateral_filter = {
    VX_KERNEL_BILATERAL_FILTER,
    "org.khronos.openvx.bilateral_filter",
    vxoBaseKernel_BilateralFilter,
    basekernel_bilateral_filter_params, vxmLENGTH_OF(basekernel_bilateral_filter_params),
    vxBilateralFilterValidator,
    NULL,
    NULL,
    vxoBilateral_filter_Initialize,
    vxoBilateral_filter_Deinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {NULL},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalBilateral_filter_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalBilateral_filter_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status VX_CALLBACK vxoInternalBilateral_filter_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_param_description_s internalkernel_bilateral_filter_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s internalkernel_bilateral_filter = {
    VX_KERNEL_INTERNAL_BILATERAL_FILTER,
    "org.khronos.internal.bilateral_filter",
    NULL,
    internalkernel_bilateral_filter_params, vxmLENGTH_OF(internalkernel_bilateral_filter_params),
    NULL,
    vxoInternalBilateral_filter_ValidateInput,
    vxoInternalBilateral_filter_ValidateOutput,
    vxoInternalBilateral_filter_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"bilateral_filter.vx"},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoBaseKernel_Select(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSelect_Deinitializer(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSelect_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoSelect_Validate(vx_node node, const vx_reference parameters[], vx_uint32 num, vx_meta_format metas[]);
VX_PRIVATE_API vx_param_description_s basekernel_select_params[] = {
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_REFERENCE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s basekernel_select = {
    VX_KERNEL_SELECT,
    "org.khronos.openvx.select",
    vxoBaseKernel_Select,
    basekernel_select_params, vxmLENGTH_OF(basekernel_select_params),
    vxoSelect_Validate,
    NULL,
    NULL,
    vxoSelect_Initialize,
    vxoSelect_Deinitializer,
    {NULL},
};

VX_PRIVATE_API vx_param_description_s internalkernel_upsample_padding_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoUpSamplePadding_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoUpSamplePadding_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoUpSamplePadding_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalkernel_upsample_padding = {
    VX_KERNEL_INTERNAL_UPSAMPLE_PADDING,
    "org.khronos.internal.upsample_padding",
    NULL,
    internalkernel_upsample_padding_params, vxmLENGTH_OF(internalkernel_upsample_padding_params),
    NULL,
    vxoUpSamplePadding_ValidateInput,
    vxoUpSamplePadding_ValidateOutput,
    vxoUpSamplePadding_Initialize,
    NULL,
    {"upsample_padding.vx"},
};

VX_PRIVATE_API vx_param_description_s internalkernel_upsample_convert_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoUpSampleConvert_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoUpSampleConvert_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoUpSampleConvert_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalkernel_upsample_convert = {
    VX_KERNEL_INTERNAL_UPSAMPLE_CONVERT,
    "org.khronos.internal.upsample_convert",
    NULL,
    internalkernel_upsample_convert_params, vxmLENGTH_OF(internalkernel_upsample_convert_params),
    NULL,
    vxoUpSampleConvert_ValidateInput,
    vxoUpSampleConvert_ValidateOutput,
    vxoUpSampleConvert_Initialize,
    NULL,
    {"upsample_convert.vx"},
};

VX_PRIVATE_API vx_param_description_s internalkernel_pyramid_copy_image_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoPyramidCopyImage_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPyramidCopyImage_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoPyramidCopyImage_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalkernel_pyramid_copy_image = {
    VX_KERNEL_INTERNAL_PYRAMID_COPY_IMAGE,
    "org.khronos.internal.pyramid_copy_image",
    NULL,
    internalkernel_pyramid_copy_image_params, vxmLENGTH_OF(internalkernel_pyramid_copy_image_params),
    NULL,
    vxoPyramidCopyImage_ValidateInput,
    vxoPyramidCopyImage_ValidateOutput,
    vxoPyramidCopyImage_Initialize,
    NULL,
    {"pyramid_copy_image.vx"},
};

VX_PRIVATE_API vx_param_description_s internalkernel_transpose_2d_tensor_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoTransPose2DTensor_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoTransPose2DTensor_Validate(vx_node node, const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[]);
vx_kernel_description_s internalkernel_transpose_2d_tensor = {
    VX_KERNEL_INTERNAL_TRANSPOSE_2D_TENSOR,
    "org.khronos.internal.transpose_2d_tensor",
    NULL,
    internalkernel_transpose_2d_tensor_params, vxmLENGTH_OF(internalkernel_transpose_2d_tensor_params),
    vxoTransPose2DTensor_Validate,
    NULL,
    NULL,
    vxoTransPose2DTensor_Initialize,
    NULL,
    {"transpose_2d_tensor.vx"},
};

VX_PRIVATE_API vx_param_description_s internalkernel_multiply_2d_matrixes_params[] = {
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_TENSOR, VX_PARAMETER_STATE_REQUIRED},
};
VX_PRIVATE_API vx_status VX_CALLBACK vxoMultiply2DMatrixes_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoMultiply2DMatrixes_Validate(vx_node node, const vx_reference parameters[ ], vx_uint32 num, vx_meta_format metas[]);
vx_kernel_description_s internalkernel_multiply_2d_matrixes = {
    VX_KERNEL_INTERNAL_MULTIPLY_2D_MATRIXES,
    "org.khronos.internal.multiply_2d_matrixes",
    NULL,
    internalkernel_multiply_2d_matrixes_params, vxmLENGTH_OF(internalkernel_multiply_2d_matrixes_params),
    vxoMultiply2DMatrixes_Validate,
    NULL,
    NULL,
    vxoMultiply2DMatrixes_Initialize,
    NULL,
    {"multiply_2d_matrixes.vx"},
};

VX_PRIVATE_API vx_param_description_s internalkernel_convolve5x5_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_CONVOLUTION, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status VX_CALLBACK vxoConvolve5x5_Initialize(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxoConvolve5x5_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoConvolve5x5_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s internalkernel_convolve5x5 = {
    VX_KERNEL_INTERNAL_CONVOLVE5X5,
    "org.khronos.internal.convolve5x5",
    NULL,
    internalkernel_convolve5x5_params, vxmLENGTH_OF(internalkernel_convolve5x5_params),
    NULL,
    vxoConvolve5x5_ValidateInput,
    vxoConvolve5x5_ValidateOutput,
    vxoConvolve5x5_Initialize,
    NULL,
    {"convolve5x5.vx"},
};
#endif

