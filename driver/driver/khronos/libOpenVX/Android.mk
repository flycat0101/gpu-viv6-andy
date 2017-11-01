##############################################################################
#
#    Copyright (c) 2005 - 2017 by Vivante Corp.  All rights reserved.
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

#
# libOpenVX
#
include $(CLEAR_VARS)

# Core
LOCAL_SRC_FILES := \
    driver/src/gc_vx_array.c \
    driver/src/gc_vx_context.c \
    driver/src/gc_vx_convolution.c \
    driver/src/gc_vx_delay.c \
    driver/src/gc_vx_distribution.c \
    driver/src/gc_vx_error.c \
    driver/src/gc_vx_graph.c \
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
    driver/src/gc_vx_threshold.c

# API
LOCAL_SRC_FILES += \
    driver/src/gc_vx_interface.c \
    driver/src/gc_vx_internal_node_api.c \
    driver/src/gc_vx_nn_extension_interface.c \
    driver/src/gc_vx_nn_util.c \
    driver/src/ops/gc_vx_nn_extension_concat.c

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
    kernels/gc_vxk_rpn.c \
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

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/driver/include \
    $(LOCAL_PATH)/kernels \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/compiler/libVSC/include \
    $(AQARCH)/cmodel/inc

LOCAL_LDFLAGS := \
    -Wl,-z,defs

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libdl \
    libcutils \
    libVSC \
    libGAL

LOCAL_MODULE         := libOpenVX
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
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

LOCAL_C_INCLUDES := \
    $(AQROOT)/sdk/inc

LOCAL_LDFLAGS := \
    -Wl,-z,defs

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libOpenVX

LOCAL_MODULE         := libOpenVXU
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

#
# libOpenVXC
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    extension/gc_vxc_interface.c

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DLOG_TAG=\"vxc\"

LOCAL_CFLAGS += \
    -DOPENVX_USE_LIST \
    -DOPENVX_USE_NODE_MEMORY \
    -DOPENVX_USE_TILING \
    -DOPENVX_USE_DOT  \
    -DOPENVX_USE_TARGET

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/driver/include \
    $(LOCAL_PATH)/kernels \
    $(LOCAL_PATH)/extension \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/compiler/libVSC/include \
    $(AQARCH)/cmodel/inc

LOCAL_LDFLAGS := \
    -Wl,-z,defs

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libOpenVX \
    libVSC \
    libGAL

LOCAL_MODULE         := libOpenVXC
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

