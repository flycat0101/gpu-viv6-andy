##############################################################################
#
#    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
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
	gc_hwc.cpp \
	gc_hwc_debug.cpp \
	gc_hwc_prepare.cpp \
	gc_hwc_set.cpp \
	gc_hwc_compose.cpp \
	gc_hwc_overlay.cpp

# vsync supported after jellybean
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 16),1)
LOCAL_SRC_FILES += \
	gc_hwc_vsync.cpp

endif

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-Wall \
	-Wextra \
	-DLOG_TAG=\"v_hwc\"

LOCAL_C_INCLUDES := \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/driver/android/gralloc \
	$(AQROOT)/hal/inc \
	$(AQROOT)/compiler/libVSC/include

LOCAL_LDFLAGS := \
	-Wl,--version-script=$(LOCAL_PATH)/hwcomposer.map

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libEGL \
	libGAL

LOCAL_MODULE         := hwcomposer_viv.$(HAL_MODULE_VARIANT)
LOCAL_MODULE_PATH    := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
LOCAL_VENDOR_MODULE  := true
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

