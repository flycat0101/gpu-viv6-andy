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


#ifndef _OPENVXC_INTERFACE_H_
#define _OPENVXC_INTERFACE_H_

#include <gc_vxk_common.h>

#if defined(__ANDROID__)
#define VX_SHADER_SOURCE_PATH				"/sdcard/vx/";
#elif defined(__linux__)
#define VX_SHADER_SOURCE_PATH				"/home/vx/";
#elif defined(_WIN32) || defined(UNDER_CE)
#define VX_SHADER_SOURCE_PATH				"e:\\vx1\\";
#endif

enum
{
    VX_KERNEL_EXTENSION_MINMAX = 200,
    VX_KERNEL_EXTENSION_MINMAXLOCATION,
};

typedef struct _vx_object_data
{
    vx_enum                                 objType;

    union
    {
        struct
        {
            vx_uint32                       width;
            vx_uint32                       height;
            vx_df_image                     format;
        }
        imageInfo;

        struct
        {
            vx_enum                         dataType;
            void *                          scalarValuePtr;
        }
        scalarInfo;

        struct
        {
            vx_enum                         dataType;
        }
        lutArrayInfo;

        struct
        {
            vx_size                         numBins;
        }
        distributionInfo;

        struct
        {
            vx_enum                         dataType;
        }
        thresholdInfo;

        struct
        {
            vx_size                         rows;
            vx_size                         columns;
        }
        convolutionInfo;

        struct
        {
            vx_enum                         dataType;
            vx_size                         rows;
            vx_size                         columns;
        }
        matrixInfo;

        struct
        {
            vx_uint32                       srcWidth;
            vx_uint32                       srcHeight;
            vx_uint32                       dstWidth;
            vx_uint32                       dstHeight;
        }
        remapInfo;

        struct
        {
            vx_size                         numLevels;
            vx_float32                      scale;
        }
        pyramidInfo;

        struct
        {
            vx_enum                         dataType;
            vx_size                         capacity;
        }
        arrayInfo;
    }
    u;
}
vx_object_data_s;

VX_PRIVATE_API vx_status vxoAddParameterToGraphByIndex(vx_graph graph, vx_node node, vx_uint32 index);

VX_PRIVATE_API vx_status vxoGetObjAttributeByNodeIndex(vx_node node, vx_uint32 index, vx_enum type, vx_object_data_s* objData);

VX_PRIVATE_API vx_param_description_s basekernel_minmaxloc_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

VX_PRIVATE_API vx_param_description_s programkernel_minmaxloc_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

VX_PRIVATE_API vx_param_description_s programkernel_minmaxlocation_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
};

VX_PRIVATE_API vx_status vxoProgrameKernel_MinMaxLoc(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoMinMaxLoc_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMinMaxLoc_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoProgrameMinMaxLoc_Initializer(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoProgrameMinMaxLoc_Deinitializer(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_minmaxloc = {
    VX_KERNEL_MINMAXLOC,
    "com.vivantecorp.openvx.min_max_loc",
    vxoProgrameKernel_MinMaxLoc,
    basekernel_minmaxloc_params, vxmLENGTH_OF(basekernel_minmaxloc_params),
    vxoMinMaxLoc_ValidateInput,
    vxoMinMaxLoc_ValidateOutput,
    vxoProgrameMinMaxLoc_Initializer,
    vxoProgrameMinMaxLoc_Deinitializer,
};

VX_PRIVATE_API vx_status vxoMinMax_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMinMax_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoMinMax_Initialize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_minmax = {
    VX_KERNEL_EXTENSION_MINMAX,
    "com.vivantecorp.openvx.minmax",
    NULL,
    programkernel_minmaxloc_params, vxmLENGTH_OF(programkernel_minmaxloc_params),
    vxoMinMax_ValidateInput,
    vxoMinMax_ValidateOutput,
    vxoMinMax_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"minmax.vx"},
};

VX_PRIVATE_API vx_status vxoMinMaxLocation_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMinMaxLocation_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s programkernel_minmaxlocation = {
    VX_KERNEL_EXTENSION_MINMAXLOCATION,
    "com.vivantecorp.openvx.minmaxlocation",
    NULL,
    programkernel_minmaxlocation_params, vxmLENGTH_OF(programkernel_minmaxlocation_params),
    vxoMinMaxLocation_ValidateInput,
    vxoMinMaxLocation_ValidateOutput,
    vxoMinMax_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"minmaxlocation.vx"},
};

VX_PRIVATE_API vx_param_description_s basekernel_lut_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_LUT,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoTableLookup_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoTableLookup_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoLUT_Initialize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_lut = {
    VX_KERNEL_TABLE_LOOKUP,
    "com.vivantecorp.openvx.table_lookup",
    NULL,
    basekernel_lut_params, vxmLENGTH_OF(basekernel_lut_params),
    vxoTableLookup_ValidateInput,
    vxoTableLookup_ValidateOutput,
    vxoLUT_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"lut.vx"},
};

VX_PRIVATE_API vx_status vxoAbsDiff_Initialize(vx_node node, vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status vxoAbsDiff_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoAbsDiff_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_param_description_s basekernel_absdiff_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

vx_kernel_description_s programkernel_absdiff = {
    VX_KERNEL_ABSDIFF,
    "com.vivantecorp.openvx.abs_diff",
    NULL,
    basekernel_absdiff_params, vxmLENGTH_OF(basekernel_absdiff_params),
    vxoAbsDiff_ValidateInput,
    vxoAbsDiff_ValidateOutput,
    vxoAbsDiff_Initialize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"absdiff.vx"},
};

VX_PRIVATE_API vx_param_description_s basekernel_morphology_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoMorphology_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMorphology_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s*ptr);
VX_PRIVATE_API vx_status vxoMorphology_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_dilate3x3 = {
    VX_KERNEL_DILATE_3x3,
    "com.vivantecorp.openvx.dilate3x3",
    NULL,
    basekernel_morphology_params, vxmLENGTH_OF(basekernel_morphology_params),
    vxoMorphology_ValidateInput,
    vxoMorphology_ValidateOutput,
    vxoMorphology_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"dilate3x3.vx"},
};

vx_kernel_description_s programkernel_erode3x3 = {
    VX_KERNEL_ERODE_3x3,
    "com.vivantecorp.openvx.erode3x3",
    NULL,
    basekernel_morphology_params, vxmLENGTH_OF(basekernel_morphology_params),
    vxoMorphology_ValidateInput,
    vxoMorphology_ValidateOutput,
    vxoMorphology_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"erode3x3.vx"},
};

VX_PRIVATE_API vx_param_description_s basekernel_filter_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoFilter_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoFilter_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s programkernel_median3x3 = {
    VX_KERNEL_MEDIAN_3x3,
    "com.vivantecorp.openvx.median3x3",
    NULL,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
    vxoMorphology_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"median3x3.vx"},
};

vx_kernel_description_s programkernel_gaussian3x3 = {
    VX_KERNEL_GAUSSIAN_3x3,
    "com.vivantecorp.openvx.gaussian3x3",
    NULL,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
    vxoMorphology_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"gaussian3x3.vx"},
};

VX_PRIVATE_API vx_param_description_s basekernel_Sobel3x3_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_OPTIONAL},
};

VX_PRIVATE_API vx_status vxoSobel3x3_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoSobel3x3_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoSobel3x3_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_sobel3x3 = {
    VX_KERNEL_SOBEL_3x3,
    "com.vivantecorp.openvx.sobel3x3",
    NULL,
    basekernel_Sobel3x3_params, vxmLENGTH_OF(basekernel_Sobel3x3_params),
    vxoSobel3x3_ValidateInput,
    vxoSobel3x3_ValidateOutput,
    vxoSobel3x3_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"sobel3x3.vx"},
};

VX_PRIVATE_API vx_status vxoBox3x3_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_box3x3 = {
    VX_KERNEL_BOX_3x3,
    "com.vivantecorp.openvx.box3x3:default",
    NULL,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
    vxoBox3x3_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"box3x3.vx"},
};

vx_kernel_description_s programkernel_box3x3_2 = {
    VX_KERNEL_BOX_3x3,
    "com.vivantecorp.openvx.box3x3:duplicate",
    NULL,
    basekernel_filter_params, vxmLENGTH_OF(basekernel_filter_params),
    vxoFilter_ValidateInput,
    vxoFilter_ValidateOutput,
    vxoBox3x3_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"box3x3.vx"},
};

static vx_param_description_s basekernel_multiply_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoMultiply_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoMultiply_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoMultiply_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_multiply = {
    VX_KERNEL_MULTIPLY,
    "com.vivantecorp.openvx.multiply",
    NULL,
    basekernel_multiply_params, vxmLENGTH_OF(basekernel_multiply_params),
    vxoMultiply_ValidateInput,
    vxoMultiply_ValidateOutput,
    vxoMultiply_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"multiply.vx"},
};

static vx_param_description_s remap_kernel_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_REMAP, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoRemap_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoRemap_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoRemap_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_remap = {
    VX_KERNEL_REMAP,
    "com.vivantecorp.openvx.remap",
    NULL,
    remap_kernel_params, vxmLENGTH_OF(remap_kernel_params),
    vxoRemap_ValidateInput,
    vxoRemap_ValidateOutput,
    vxoRemap_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"remap.vx"},
};

VX_PRIVATE_API vx_param_description_s basekernel_threshold_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_THRESHOLD,   VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoThreshold_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoThreshold_ValidatorOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s programkernel_threshold = {
    VX_KERNEL_THRESHOLD,
    "com.vivantecorp.openvx.threshold",
    NULL,
    basekernel_threshold_params, vxmLENGTH_OF(basekernel_threshold_params),
    vxoThreshold_ValidateInput,
    vxoThreshold_ValidatorOutput,
    vxoRemap_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"threshold.vx"},
};

static vx_param_description_s basekernel_warp_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_MATRIX, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_OPTIONAL},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoWarpAffine_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoWarp_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
vx_kernel_description_s programkernel_warp_affine = {
    VX_KERNEL_WARP_AFFINE,
    "com.vivantecorp.openvx.warp_affine",
    NULL,
    basekernel_warp_params, vxmLENGTH_OF(basekernel_warp_params),
    vxoWarpAffine_ValidateInput,
    vxoWarp_ValidateOutput,
    vxoRemap_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"warpaffine.vx"},
};

VX_PRIVATE_API vx_param_description_s basekernel_histogram_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_DISTRIBUTION, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoHistogram_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoHistogram_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoHistogram_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_histogram = {
    VX_KERNEL_HISTOGRAM,
    "com.vivantecorp.openvx.histogram",
    NULL,
    basekernel_histogram_params, vxmLENGTH_OF(basekernel_histogram_params),
    vxoHistogram_ValidateInput,
    vxoHistogram_ValidateOutput,
    vxoHistogram_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"histogram.vx"},
};

VX_PRIVATE_API vx_param_description_s basekernel_convolution_params[] = {
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_CONVOLUTION, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
};

VX_PRIVATE_API vx_status vxoConvolve_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status vxoConvolve_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format_s *ptr);
VX_PRIVATE_API vx_status vxoConvolve_Initilize(vx_node node, vx_reference *parameters, vx_uint32 num);
vx_kernel_description_s programkernel_convolution = {
    VX_KERNEL_CUSTOM_CONVOLUTION,
    "com.vivantecorp.openvx.custom_convolution",
    NULL,
    basekernel_convolution_params, vxmLENGTH_OF(basekernel_convolution_params),
    vxoConvolve_ValidateInput,
    vxoConvolve_ValidateOutput,
    vxoConvolve_Initilize,
    NULL,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"convolution.vx"},
};

#endif
