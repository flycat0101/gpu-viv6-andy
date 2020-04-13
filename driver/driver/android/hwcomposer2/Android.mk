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

#
# hwcomposer.<property>.so
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    hwc2_common.cpp \
    hwc2_blitter.cpp \
    hwc2_fbdev.cpp

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -Wno-unused-variable \
    -Wno-unused-function \
    -DLOG_TAG=\"hwc\"

LOCAL_C_INCLUDES := \
    $(AQROOT)/hal/inc \
    $(AQROOT)/driver/android/gralloc

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    libutils \
    liblog \
    libhardware \
    libsync \
    liblog \
    libGAL

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MULTILIB             := 32

LOCAL_MODULE         := hwcomposer.$(HAL_MODULE_VARIANT)
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)


