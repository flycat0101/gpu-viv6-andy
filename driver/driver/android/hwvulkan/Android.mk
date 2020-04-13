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
# hwvulkan.<variant>.so for android-n and later.
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    viv_vulkan.cpp

LOCAL_C_INCLUDES := \
    frameworks/native/vulkan/include

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 28),1)
LOCAL_C_INCLUDES += \
		hardware/libhardware/include \
		system/core/include
endif


ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 29),1)
LOCAL_C_INCLUDES += \
       external/vulkan-headers/include \
       $(AQROOT)/sdk/inc
endif

LOCAL_CFLAGS := \
    -Wall \
    -Wextra \
    -Wno-unused-parameter \
    -DGPU_VENDOR=\"$(GPU_VENDOR)\" \
    -DLOG_TAG=\"v_vulkan\"

LOCAL_SHARED_LIBRARIES := \
    libdl \
    liblog

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE               := vulkan.$(HAL_MODULE_VARIANT)
LOCAL_MODULE_TAGS          := optional
LOCAL_PRELINK_MODULE       := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE        := true
endif
include $(BUILD_SHARED_LIBRARY)

