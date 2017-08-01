/****************************************************************************
*
*    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Vivante Corporation. This is proprietary information owned by
*    Vivante Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Vivante Corporation.
*
*****************************************************************************/


#ifndef _GC_NN_INTERFACE_H_
#define _GC_NN_INTERFACE_H_

#include <gc_vxk_common.h>

#if defined(__ANDROID__)
#define VX_SHADER_SOURCE_PATH               "/sdcard/vx/";
#elif defined(__linux__)
#define VX_SHADER_SOURCE_PATH               "/home/vx/";
#elif defined(_WIN32) || defined(UNDER_CE)
#define VX_SHADER_SOURCE_PATH               "e:\\vx1\\";
#endif

enum
{
    VX_KERNEL_ENUM_LRN          = 10000,
    VX_KERNEL_ENUM_HORIPOOL     = 10001,
    VX_KERNEL_ENUM_VERTPOOL     = 10002,
    VX_KERNEL_ENUM_ROIPOOL        = 10003,
    VX_KERNEL_ENUM_FASTERRCNNRESHUFFLEIMAGE    = 10004,
    VX_KERNEL_ENUM_FASTERRCNNINTERLEAVE = 10005,
    VX_KERNEL_ENUM_FASTERRCNNRESHUFFLEDATA = 10006,
    VX_KERNEL_ENUM_NMS            = 1007 ,
    VX_KERNEL_ENUM_RECTPROCESS    = 1008,
    VX_KERNEL_ENUM_RESORTING      = 1009,
    VX_KERNEL_ENUM_MAXPOOL3x3     = 1010
};

#define USE_LRN_VXC
#define VX_KERNEL_NAME_LRN  "com.vivantecorp.extension.vxcLrn"

#define USE_HORIPOOL_VXC
#define VX_KERNEL_NAME_HORIPOOL  "com.vivantecorp.extension.vxcHoripool"

#define USE_VERTPOOL_VXC
#define VX_KERNEL_NAME_VERTPOOL  "com.vivantecorp.extension.vxcVertpool"

#define USE_ROIPOOL_VXC
#define VX_KERNEL_NAME_ROIPOOL  "com.vivantecorp.extension.vxcRoipool"

#define USE_NMS_VXC
#define VX_KERNEL_NAME_NMS  "com.vivantecorp.extension.vxcNms"

#define USE_FASTERRCNNRESHUFFLEIMAGE_VXC
#define VX_KERNEL_NAME_FASTERRCNNRESHUFFLEIMAGE  "com.vivantecorp.extension.fasterrcnnreshuffleimageVXC"

#define USE_FASTERRCNNINTERLEAVE_VXC
#define VX_KERNEL_NAME_FASTERRCNNINTERLEAVE  "com.vivantecorp.extension.fasterrcnninterleaveVXC"

#define USE_FASTERRCNNRESHUFFLEDATA_VXC
#define VX_KERNEL_NAME_FASTERRCNNRESHUFFLEDATA  "com.vivantecorp.extension.fasterrcnnreshuffledataVXC"

//#define USE_RECTPROCESS_VXC
#define  VX_KERNEL_NAME_RECTPROCESS  "com.vivantecorp.extension.vxcRectprocess"
//#define USE_RESORTING_VXC
#define  VX_KERNEL_NAME_RESORTING  "com.vivantecorp.extension.vxcResorting"

#define USE_MAXPOOL3x3_VXC
#define VX_KERNEL_NAME_MAXPOOL3x3  "com.vivantecorp.extension.maxpool3x3VXC"
//LRN**********************************
VX_PRIVATE_API vx_status VX_CALLBACK vxoLRN_ValidateInput(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxoLRN_ValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj);
VX_PRIVATE_API vx_status VX_CALLBACK vxLrnInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxLrnDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxLrnFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

vx_param_description_s basekernel_lrn_params[] = {
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s internal_nn_lrn = {
    VX_KERNEL_INTERNAL_LRN,
    VX_KERNEL_NAME_LRN,
#ifdef USE_LRN_VXC
    vxLrnFunction,
#else
     vxoInternalKernel_LRN,
#endif
    basekernel_lrn_params,
    (sizeof(basekernel_lrn_params)/sizeof(basekernel_lrn_params[0])),
    VX_NULL,
    vxoLRN_ValidateInput,
    vxoLRN_ValidateOutput,
    vxLrnInitializer,
    vxLrnDeinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"lrn.vx"},
};
//RECTPROCESS**********************************
vx_status VX_CALLBACK vxRectprocessInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxRectprocessDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxRectprocessValidateInput(vx_node node, vx_uint32 index);
vx_status VX_CALLBACK vxRectprocessValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj);
vx_status VX_CALLBACK vxRectprocessInternalKernel(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_param_description_s basekernel_rectprocess_params[] = {
    {VX_INPUT,  VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s internalkernel_rectprocess = {
    VX_KERNEL_ENUM_RECTPROCESS,
    VX_KERNEL_NAME_RECTPROCESS,
#ifdef USE_RECTPROCESS_VXC
    NULL,
#else
     vxRectprocessInternalKernel,
#endif
    basekernel_rectprocess_params,
    (sizeof(basekernel_rectprocess_params)/sizeof(basekernel_rectprocess_params[0])),
    VX_NULL,
    vxRectprocessValidateInput,
    vxRectprocessValidateOutput,
    vxRectprocessInitializer,
    vxRectprocessDeinitializer
};
//RESORTING**********************************

vx_status VX_CALLBACK vxResortingInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxResortingDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxResortingValidateInput(vx_node node, vx_uint32 index);
vx_status VX_CALLBACK vxResortingValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj);
vx_status VX_CALLBACK vxResortingInternalKernel(vx_node node, const vx_reference *parameters, vx_uint32 num);

vx_param_description_s basekernel_resorting_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s internalkernel_resorting = {
    VX_KERNEL_ENUM_RESORTING,
    VX_KERNEL_NAME_RESORTING,

#ifdef USE_RESORTING_VXC
    NULL,
#else
     vxResortingInternalKernel,
#endif
    basekernel_resorting_params,
    (sizeof(basekernel_resorting_params)/sizeof(basekernel_resorting_params[0])),
    VX_NULL,
    vxResortingValidateInput,
    vxResortingValidateOutput,
    vxResortingInitializer,
    vxResortingDeinitializer
};

//HORIPOOL**********************************
vx_status VX_CALLBACK vxHoripoolInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxHoripoolDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxHoripoolValidateInput(vx_node node, vx_uint32 index);
vx_status VX_CALLBACK vxHoripoolValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj);
vx_status VX_CALLBACK vxHoripoolInternalKernel(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxHoripoolFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

vx_param_description_s basekernel_horipool_params[] = {
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s internal_kernel_horipool = {
    VX_KERNEL_ENUM_HORIPOOL,
    VX_KERNEL_NAME_HORIPOOL,

#ifdef USE_HORIPOOL_VXC
    vxHoripoolFunction,
#else
     vxHoripoolInternalKernel,
#endif
    basekernel_horipool_params,
    (sizeof(basekernel_horipool_params)/sizeof(basekernel_horipool_params[0])),
    VX_NULL,
    vxHoripoolValidateInput,
    vxHoripoolValidateOutput,
    vxHoripoolInitializer,
    vxHoripoolDeinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"horipool.vx"},
};
//VERTPOOL**********************************

vx_status VX_CALLBACK vxVertpoolInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxVertpoolDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxVertpoolValidateInput(vx_node node, vx_uint32 index);
vx_status VX_CALLBACK vxVertpoolValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj);
vx_status VX_CALLBACK vxVertpoolInternalKernel(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxVertpoolFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

vx_param_description_s basekernel_vertpool_params[] = {
    {VX_INPUT,  VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s internal_kernel_vertpool = {
    VX_KERNEL_ENUM_VERTPOOL,
    VX_KERNEL_NAME_VERTPOOL,

#ifdef USE_VERTPOOL_VXC
    vxVertpoolFunction,
#else
     vxVertpoolInternalKernel,
#endif
    basekernel_vertpool_params,
    (sizeof(basekernel_vertpool_params)/sizeof(basekernel_vertpool_params[0])),
    VX_NULL,
    vxVertpoolValidateInput,
    vxVertpoolValidateOutput,
    vxVertpoolInitializer,
    vxVertpoolDeinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"vertpool.vx"},
};

//ROIPOOL = Horipool+Vertpool**********************************
vx_status VX_CALLBACK vxRoipoolInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxRoipoolDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxRoipoolValidateInput(vx_node node, vx_uint32 index);
vx_status VX_CALLBACK vxRoipoolValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj);
vx_status VX_CALLBACK vxRoipoolInternalKernel(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxRoipoolFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

vx_param_description_s basekernel_roipool_params[] = {
    {VX_INPUT,  VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
};
vx_kernel_description_s internal_kernel_roipool = {
    VX_KERNEL_INTERNAL_ROI_POOLING,
//    VX_KERNEL_ENUM_ROIPOOL,
    VX_KERNEL_NAME_ROIPOOL,
    vxRoipoolInternalKernel,
    basekernel_roipool_params,
    (sizeof(basekernel_roipool_params)/sizeof(basekernel_roipool_params[0])),
    VX_NULL,
    vxRoipoolValidateInput,
    vxRoipoolValidateOutput,
    vxRoipoolInitializer,
    vxRoipoolDeinitializer,
};

//NMS**********************************

vx_status VX_CALLBACK vxNmsInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxNmsDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
vx_status VX_CALLBACK vxNmsValidateInput(vx_node node, vx_uint32 index);
vx_status VX_CALLBACK vxNmsValidateOutput(vx_node node, vx_uint32 index, vx_meta_format  metaObj);
vx_status VX_CALLBACK vxNmsInternalKernel(vx_node node, const vx_reference *parameters, vx_uint32 num);
VX_PRIVATE_API vx_status VX_CALLBACK vxNmsFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

vx_param_description_s basekernel_nms_params[] = {
    {VX_INPUT,  VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT,  VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED}
};

vx_kernel_description_s internal_kernel_nms = {
    VX_KERNEL_ENUM_NMS,
    VX_KERNEL_NAME_NMS,

#ifdef USE_NMS_VXC
    vxNmsFunction,
#else
     vxNmsInternalKernel,
#endif
    basekernel_nms_params,
    (sizeof(basekernel_nms_params)/sizeof(basekernel_nms_params[0])),
    VX_NULL,
    vxNmsValidateInput,
    vxNmsValidateOutput,
    vxNmsInitializer,
    vxNmsDeinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"nms.vx"},
};

//ReshuffleImage*********************
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageInputValidator(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageOutputValidator(vx_node node, vx_uint32 index, vx_meta_format metaObj);
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleImageFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

vx_param_description_s vxfasterRcnnReshuffleImageKernelParam[] =
{
    {VX_INPUT, VX_TYPE_IMAGE, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_OPTIONAL},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED}

};
vx_kernel_description_s vxfasterRcnnReshuffleImageKernelInfo =
{
    /*VX_KERNEL_ENUM_FASTERRCNNRESHUFFLEIMAGE,*/
    VX_KERNEL_INTERNAL_CNN_RESHUFFLE_IMAGE,
    VX_KERNEL_NAME_FASTERRCNNRESHUFFLEIMAGE,
#ifdef USE_FASTERRCNNRESHUFFLEIMAGE_VXC
    vxfasterRcnnReshuffleImageFunction,
#else
    vxoInternalKernel_CnnReshuffleImage,
#endif
    vxfasterRcnnReshuffleImageKernelParam,
    (sizeof(vxfasterRcnnReshuffleImageKernelParam)/sizeof(vxfasterRcnnReshuffleImageKernelParam[0])),
    VX_NULL,
    vxfasterRcnnReshuffleImageInputValidator,
    vxfasterRcnnReshuffleImageOutputValidator,
    vxfasterRcnnReshuffleImageInitializer,
    vxfasterRcnnReshuffleImageDeinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"fasterRcnnReshuffleImage.vx"},
};
//Interleave*********************
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveInputValidator(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveOutputValidator(vx_node node, vx_uint32 index, vx_meta_format metaObj);
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveKernel(vx_node node, const vx_reference* paramObj, vx_uint32 paramNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxFasterRcnnInterleaveFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

vx_param_description_s vxFasterRcnnInterleaveKernelParam[] =
{
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED}
};
vx_kernel_description_s vxFasterRcnnInterleaveKernelInfo =
{
    VX_KERNEL_INTERNAL_CNN_INTERLEAVE_BUFFERS,
    VX_KERNEL_NAME_FASTERRCNNINTERLEAVE,
#ifdef USE_FASTERRCNNINTERLEAVE_VXC
    vxFasterRcnnInterleaveFunction,
#else
    vxoInternalKernel_CnnInterleave_Buffers,
#endif
    vxFasterRcnnInterleaveKernelParam,
    (sizeof(vxFasterRcnnInterleaveKernelParam)/sizeof(vxFasterRcnnInterleaveKernelParam[0])),
    VX_NULL,
    vxFasterRcnnInterleaveInputValidator,
    vxFasterRcnnInterleaveOutputValidator,
    vxFasterRcnnInterleaveInitializer,
    vxFasterRcnnInterleaveDeinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"fasterRcnnInterleave.vx"},
};
//ReshuffleData*********************
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataInputValidator(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataOutputValidator(vx_node node, vx_uint32 index, vx_meta_format metaObj);
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataInitializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataDeinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxfasterRcnnReshuffleDataFunction(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

vx_param_description_s vxfasterRcnnReshuffleDataKernelParam[] =
{
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED}

};
vx_kernel_description_s vxfasterRcnnReshuffleDataKernelInfo =
{
    /*VX_KERNEL_ENUM_FASTERRCNNRESHUFFLEDATA,*/
    VX_KERNEL_INTERNAL_FASTERRCNN_RESHUFFLE_DATA,
    VX_KERNEL_NAME_FASTERRCNNRESHUFFLEDATA,
#ifdef USE_FASTERRCNNRESHUFFLEIMAGE_VXC
    vxfasterRcnnReshuffleDataFunction,
#else
    vxoInternalKernel_FasterRcnnReshuffleData,
#endif
    vxfasterRcnnReshuffleDataKernelParam,
    (sizeof(vxfasterRcnnReshuffleDataKernelParam)/sizeof(vxfasterRcnnReshuffleDataKernelParam[0])),
    VX_NULL,
    vxfasterRcnnReshuffleDataInputValidator,
    vxfasterRcnnReshuffleDataOutputValidator,
    vxfasterRcnnReshuffleDataInitializer,
    vxfasterRcnnReshuffleDataDeinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"fasterRcnnReshuffleData.vx"},
};
//MaxPool3x3*****************************/
VX_PRIVATE_API vx_status VX_CALLBACK vxMaxPool3x3InputValidator(vx_node node, vx_uint32 index);
VX_PRIVATE_API vx_status VX_CALLBACK vxMaxPool3x3OutputValidator(vx_node node, vx_uint32 index, vx_meta_format metaObj);
VX_PRIVATE_API vx_status VX_CALLBACK vxMaxPool3x3Initializer(vx_node nodObj, const vx_reference *paramObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxMaxPool3x3Deinitializer(vx_node nodObj, const vx_reference *paraObj, vx_uint32 paraNum);
VX_PRIVATE_API vx_status VX_CALLBACK vxMaxPool3x3Function(vx_node node, const vx_reference parameters[], vx_uint32 paramCount);

vx_param_description_s vxMaxPool3x3KernelParam[] =
{
    {VX_INPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_INPUT, VX_TYPE_SCALAR, VX_PARAMETER_STATE_REQUIRED},
    {VX_OUTPUT, VX_TYPE_ARRAY, VX_PARAMETER_STATE_REQUIRED}

};
vx_kernel_description_s vxMaxPool3x3KernelInfo =
{
    /*VX_KERNEL_ENUM_MAXPOOL3x3,*/
    VX_KERNEL_INTERNAL_MAX_POOL3x3,
    VX_KERNEL_NAME_MAXPOOL3x3,
#ifdef USE_MAXPOOL3x3_VXC
    vxMaxPool3x3Function,
#else
    vxMaxPool3x3Kernel,
#endif
    vxMaxPool3x3KernelParam,
    (sizeof(vxMaxPool3x3KernelParam)/sizeof(vxMaxPool3x3KernelParam[0])),
    VX_NULL,
    vxMaxPool3x3InputValidator,
    vxMaxPool3x3OutputValidator,
    vxMaxPool3x3Initializer,
    vxMaxPool3x3Deinitializer,
#if gcdVX_OPTIMIZER
    {vx_false_e, vx_false_e, 0, 0, 0, 0, 0, 0},
#endif
    {"MaxPool3x3.vx"},
};


#endif
