##############################################################################
#
#    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################

LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../Android.mk.def

VX_KENRELS_HW := 1

#
# libOpenVX
#
include $(CLEAR_VARS)

# Core
LOCAL_SRC_FILES := \
    driver/src/gc_vx_array.c \
    driver/src/gc_vx_json.c \
    driver/src/gc_vx_binary.c \
    driver/src/gc_vx_context.c \
    driver/src/gc_vx_convolution.c \
    driver/src/gc_vx_delay.c \
    driver/src/gc_vx_distribution.c \
    driver/src/gc_vx_error.c \
    driver/src/gc_vx_graph.c \
    driver/src/gc_vx_graph_optimization.c \
    driver/src/gc_vx_image.c \
    driver/src/gc_vx_kernel.c \
    driver/src/gc_vx_layer.c \
    driver/src/gc_vx_log.c \
    driver/src/gc_vx_lut.c \
    driver/src/gc_vx_matrix.c \
    driver/src/gc_vx_memory.c \
    driver/src/gc_vx_meta_format.c \
    driver/src/gc_vx_node_api.c \
    driver/src/gc_vx_node.c \
    driver/src/gc_vx_object_array.c \
    driver/src/gc_vx_parameter.c \
    driver/src/gc_vx_profiler.c \
    driver/src/gc_vx_program.c \
    driver/src/gc_vx_pyramid.c \
    driver/src/gc_vx_reference.c \
    driver/src/gc_vx_remap.c \
    driver/src/gc_vx_runtime.c \
    driver/src/gc_vx_scalar.c \
    driver/src/gc_vx_target.c \
    driver/src/gc_vx_tensor.c \
    driver/src/gc_vx_threshold.c \
    driver/src/gc_vx_gpu_layer.c \
    driver/src/gc_vx_sys.c

# Operations
LOCAL_SRC_FILES += \
    driver/src/ops/gc_vx_op_fc.c \
    driver/src/ops/gc_vx_op_debug.c \
    driver/src/ops/gc_vx_op_tensor_copy.c

# Layers
LOCAL_SRC_FILES += \
    driver/src/layers/gc_vx_layer_lstm.c \
    driver/src/layers/gc_vx_layer_activation.c \
    driver/src/layers/gc_vx_layer_conv.c \
    driver/src/layers/gc_vx_layer_fc.c \
    driver/src/layers/gc_vx_layer_adapter.c \
    driver/src/layers/gc_vx_layer_batch_norm.c \
    driver/src/layers/gc_vx_layer_concat.c \
    driver/src/layers/gc_vx_layer_deconv.c \
    driver/src/layers/gc_vx_layer_depthwise_conv.c \
    driver/src/layers/gc_vx_layer_div.c \
    driver/src/layers/gc_vx_layer_eltwise.c \
    driver/src/layers/gc_vx_layer_rnn.c \
    driver/src/layers/gc_vx_layer_hash_lut.c \
    driver/src/layers/gc_vx_layer_l2norm.c \
    driver/src/layers/gc_vx_layer_lrn.c \
    driver/src/layers/gc_vx_layer_lsh_project.c \
    driver/src/layers/gc_vx_layer_lut.c \
    driver/src/layers/gc_vx_layer_pool.c \
    driver/src/layers/gc_vx_layer_reduce_sum.c \
    driver/src/layers/gc_vx_layer_reorg.c \
    driver/src/layers/gc_vx_layer_roi_pool.c \
    driver/src/layers/gc_vx_layer_rpn.c \
    driver/src/layers/gc_vx_layer_tensor_mean.c \
    driver/src/layers/gc_vx_layer_tensor_reshape.c \
    driver/src/layers/gc_vx_layer_tensor_round.c \
    driver/src/layers/gc_vx_layer_tensor_scale.c \
    driver/src/layers/gc_vx_layer_tensor_squeeze.c \
    driver/src/layers/gc_vx_layer_tensor_stride_slice.c \
    driver/src/layers/gc_vx_layer_tensor_transpose.c \
    driver/src/layers/gc_vx_layer_tensor_pad.c \
    driver/src/layers/gc_vx_layer_softmax.c \
    driver/src/layers/gc_vx_layer_tensor_copy.c \
    driver/src/layers/gc_vx_layer_tensor_reverse.c \

# API
LOCAL_SRC_FILES += \
    driver/src/gc_vx_interface.c \
    driver/src/gc_vx_nn_extension_interface.c \
    driver/src/gc_vx_internal_node_api.c \
    driver/src/gc_vx_nn_util.c \
    driver/src/gc_vx_nn_encoder.c \
    driver/src/gc_vx_nn_wb.c \
    driver/src/gc_vx_nn_command.c \

# Kernel
LOCAL_SRC_FILES += \
    kernels/gc_vxk_absdiff.c \
    kernels/gc_vxk_accumulate.c \
    kernels/gc_vxk_addsub.c \
    kernels/gc_vxk_bitwise.c \
    kernels/gc_vxk_channel.c \
    kernels/gc_vxk_common.c \
    kernels/gc_vxk_convertcolor.c \
    kernels/gc_vxk_convertdepth.c \
    kernels/gc_vxk_convolve.c \
    kernels/gc_vxk_fast9.c \
    kernels/gc_vxk_filter.c \
    kernels/gc_vxk_harris.c \
    kernels/gc_vxk_histogram.c \
    kernels/gc_vxk_integralimage.c \
    kernels/gc_vxk_lut.c \
    kernels/gc_vxk_magnitude.c \
    kernels/gc_vxk_morphology.c \
    kernels/gc_vxk_multiply.c \
    kernels/gc_vxk_nn_layers.c \
    kernels/gc_vxk_nonmax.c \
    kernels/gc_vxk_norm.c \
    kernels/gc_vxk_optpyrlk.c \
    kernels/gc_vxk_phase.c \
    kernels/gc_vxk_remap.c \
    kernels/gc_vxk_scale.c \
    kernels/gc_vxk_sobel3x3.c \
    kernels/gc_vxk_statistics.c \
    kernels/gc_vxk_threshold.c \
    kernels/gc_vxk_warp.c

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DLOG_TAG=\"vx\"

LOCAL_CFLAGS += \
    -DOPENVX_USE_LIST \
    -DOPENVX_USE_NODE_MEMORY \
    -DOPENVX_USE_TILING \
    -DOPENVX_USE_DOT  \
    -DOPENVX_USE_TARGET

#For original nnarchperf
LOCAL_CFLAGS += \
    -DUSE_LIB_NN_ARCH_PERF

ifeq ($(ORI_NNARCHPERF),1)
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/driver/include \
    $(LOCAL_PATH)/libarchmodel/include \
    $(LOCAL_PATH)/kernels \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/compiler/libVSC/include \
    $(AQARCH)/cmodel/inc \
    $(AQARCH)/../libNNArchPerf/libNNArchPerf
else
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/driver/include \
    $(LOCAL_PATH)/libarchmodelInterface/include \
    $(LOCAL_PATH)/kernels \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/compiler/libVSC/include \
    $(AQARCH)/cmodel/inc \
    $(AQROOT)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/vipArchPerf \
    $(AQROOT)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/libarchmodelSw/include
endif

ifeq ($(USE_VXC_BINARY),1)
LOCAL_C_INCLUDES += $(LOCAL_C_INCLUDES) \
                    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libnnvxc/ \
                    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libnnvxc/${GPU_CONFIG} \
                    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libovx12/ \
                    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libovx12/${GPU_CONFIG}  \
                    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libnngpu/ \
                    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libnngpu/${GPU_CONFIG}
endif

LOCAL_LDFLAGS := \
    -Wl,-z,defs

ifeq ($(ORI_NNARCHPERF),1)
LOCAL_STATIC_LIBRARIES += \
    libarchmodel
else
LOCAL_STATIC_LIBRARIES += \
    libarchmodelInterface
endif

ifeq ($(ORI_NNARCHPERF),1)
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libdl \
    libcutils \
    libVSC \
    libGAL \
    libNNArchPerf
else
LOCAL_SHARED_LIBRARIES := \
    liblog \
    libdl \
    libcutils \
    libVSC \
    libGAL \
    libNNArchPerf \
    libarchmodelSw
endif

LOCAL_MODULE         := libOpenVX
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

#
# libOpenVXU
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    utility/vx_utility.c

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DLOG_TAG=\"vxu\"

ifeq ($(ORI_NNARCHPERF),1)
LOCAL_C_INCLUDES := \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/driver/khronos/libOpenVX/driver/include \
    $(AQROOT)/driver/khronos/libOpenVX/kernels \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/compiler/libVSC/include \
    $(AQARCH)/../libNNArchPerf/libNNArchPerf
else
LOCAL_C_INCLUDES := \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/driver/khronos/libOpenVX/driver/include \
    $(AQROOT)/driver/khronos/libOpenVX/kernels \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/compiler/libVSC/include \
    $(AQROOT)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/vipArchPerf \
    $(AQROOT)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/libarchmodelSw/include
endif

LOCAL_LDFLAGS := \
    -Wl,-z,defs

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libOpenVX

LOCAL_MODULE         := libOpenVXU
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

#
# libNNVXCBinaries
#
ifeq ($(USE_VXC_BINARY),1)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    libkernel/libnnvxc/nnvxc_binary_interface.c

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DLOG_TAG=\"libnnvxc\"

LOCAL_C_INCLUDES := \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libnnvxc/ \
    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libnnvxc/${GPU_CONFIG}

LOCAL_LDFLAGS := \
    -Wl,-z,defs

LOCAL_MODULE         := libNNVXCBinary
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk
endif

#
# libOvx12VXCBinaries
#
ifeq ($(USE_VXC_BINARY),1)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    libkernel/libovx12/ovx12_vxc_binary_interface.c

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DLOG_TAG=\"ovx12vxc\"

LOCAL_C_INCLUDES := \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/driver/khronos/libOpenVX/driver/include \
    $(AQROOT)/driver/khronos/libOpenVX/kernels \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/compiler/libVSC/include \
    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libovx12/ \
    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libovx12/${GPU_CONFIG}

LOCAL_LDFLAGS := \
    -Wl,-z,defs

LOCAL_MODULE         := libOvx12VXCBinary
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk
endif

# libNNGPUBinary
#
ifeq ($(USE_VXC_BINARY),1)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    libkernel/libnngpu/nngpu_binary_interface.c

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DLOG_TAG=\"libnngpu\"

LOCAL_C_INCLUDES := \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libnngpu/ \
    $(AQROOT)/driver/khronos/libOpenVX/libkernel/libnngpu/${GPU_CONFIG}

LOCAL_LDFLAGS := \
    -Wl,-z,defs

LOCAL_MODULE         := libNNGPUBinary
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk
endif

# libNNArchPerf
ifeq ($(ORI_NNARCHPERF),1)
include $(AQARCH)/../libNNArchPerf/libNNArchPerf/Android.mk
else
include $(AQROOT)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/vipArchPerf/Android.mk
include $(AQROOT)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/libarchmodelSw/Android.mk
endif
# libarchmodel
ifeq ($(ORI_NNARCHPERF),1)
include $(AQROOT)/driver/khronos/libOpenVX/libarchmodel/Android.mk
else
include $(AQROOT)/driver/khronos/libOpenVX/libarchmodelInterface/Android.mk
endif
