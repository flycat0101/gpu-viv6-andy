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
	gc_hwc.cpp \
	gc_hwc_debug.cpp \
	gc_hwc_prepare.cpp \
	gc_hwc_set.cpp \
	gc_hwc_compose.cpp \
	gc_hwc_display.cpp \
	gc_hwc_vsync.cpp \
	gc_hwc_overlay.cpp \
	gc_hwc_cursor.cpp

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-Wall \
	-Wextra \
	-Wno-gnu-designator \
	-Wno-unused-parameter \
	-DLOG_TAG=\"v_hwc\"

LOCAL_C_INCLUDES := \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/driver/android/gralloc \
	$(AQROOT)/hal/inc \
	$(AQROOT)/compiler/libVSC/include

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 26),1)
LOCAL_C_INCLUDES += \
	frameworks/native/libs/nativewindow/include \
	frameworks/native/libs/nativebase/include \
	frameworks/native/libs/arect/include
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 28),1)
LOCAL_C_INCLUDES += \
	frameworks/native/include
endif

LOCAL_LDFLAGS := \
	-Wl,--version-script=$(LOCAL_PATH)/hwcomposer.map

LOCAL_SHARED_LIBRARIES := \
	libhardware_legacy \
	libhardware \
	libsync \
	libcutils \
	libutils \
	liblog \
	libEGL \
	libGAL

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 21),1)
  LOCAL_MODULE_RELATIVE_PATH := hw
else
  LOCAL_MODULE_PATH          := $(TARGET_OUT_SHARED_LIBRARIES)/hw
endif

LOCAL_MODULE         := hwcomposer_viv.$(HAL_MODULE_VARIANT)
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 27),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)

