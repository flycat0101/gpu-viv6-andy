##############################################################################
#
#    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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
# libv_gralloc
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	gc_gralloc.cpp \
	gc_gralloc_helper.cpp

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-DLOG_TAG=\"v_gralloc\"

LOCAL_C_INCLUDES := \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/hal/inc \
	$(AQROOT)/compiler/libVSC/include

# Where to find gralloc_priv.h
LOCAL_C_INCLUDES += \


LOCAL_MODULE         := libv_gralloc
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)


#
# gralloc.<property>.so
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	gralloc.cpp

LOCAL_SRC_FILES += \
	framebuffer.cpp

# ION allocator reference code in ion_gralloc.cpp
# LOCAL_SRC_FILES += ion_gralloc.cpp

# With front buffer rendering, gralloc always provides the same buffer  when
# GRALLOC_USAGE_HW_FB. Obviously there is no synchronization with the display.
# Can be used to test non-VSYNC-locked rendering.
LOCAL_CFLAGS := \
	$(CFLAGS) \
	-DLOG_TAG=\"gralloc\" \
	-DDISABLE_FRONT_BUFFER

ifneq ($(NUM_FRAMEBUFFER_SURFACE_BUFFERS),)
LOCAL_CFLAGS += \
	-DNUM_FRAMEBUFFER_SURFACE_BUFFERS=$(NUM_FRAMEBUFFER_SURFACE_BUFFERS)
endif

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libbinder \
	libutils \
	libcutils \
	libGAL

LOCAL_STATIC_LIBRARIES := \
	libv_gralloc

# ION allocator needs libion
# LOCAL_SHARED_LIBRARIES += libion

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 21),1)
  LOCAL_MODULE_RELATIVE_PATH := hw
else
  LOCAL_MODULE_PATH          := $(TARGET_OUT_SHARED_LIBRARIES)/hw
endif

LOCAL_MODULE         := gralloc.$(HAL_MODULE_VARIANT)
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

