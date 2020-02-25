##############################################################################
#
#    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../../../Android.mk.def

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    nnArchPerf.cpp \

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -fPIC \
    -Wall \
    -fno-builtin \
    -Wno-unused-parameter \
    -Wno-unused-function \
    -DLOG_TAG=\"nnArchPerf\" \

ifndef FIXED_ARCH_TYPE
LOCAL_C_INCLUDES := \
    $(AQROOT)/tools/bin \
	$(AQROOT)/hal/inc
else
LOCAL_C_INCLUDES := \
    $(AQROOT)/hal/inc
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 20),1)
LOCAL_C_INCLUDES += \
	system/core/libsync/include
endif

LOCAL_LDFLAGS := \
    -Wl,-z,defs
    -Wl,--version-script=$(LOCAL_PATH)/libNNArchPerf.map

LOCAL_MODULE         := libNNArchPerf
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk
