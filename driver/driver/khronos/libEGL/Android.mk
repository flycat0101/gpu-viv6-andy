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
# libEGL_$(TAG)
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	source/gc_egl.c \
	source/gc_egl_config.c \
	source/gc_egl_context.c \
	source/gc_egl_image.c \
	source/gc_egl_init.c \
	source/gc_egl_query.c \
	source/gc_egl_surface.c \
	source/gc_egl_swap.c \
	source/gc_egl_sync.c \
	api/gc_egl_android.c

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-Werror \
	-DLOG_TAG=\"v_egl\"

ifeq ($(gcdDUMP_FRAME_TGA),1)
LOCAL_CFLAGS += \
	-DgcdDUMP_FRAME_TGA=1
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 20),1)
LOCAL_C_INCLUDES += \
	system/core/libsync/include
endif

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/inc \
	$(LOCAL_PATH)/api \
	$(LOCAL_PATH)/os \
	$(LOCAL_PATH)/source \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/driver/android/gralloc \
	$(AQROOT)/compiler/libVSC/include \

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/source/libEGL.map

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libhardware \
	libpixelflinger \
	libGAL

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 17),1)
LOCAL_SHARED_LIBRARIES += \
	libsync
endif

ifeq ($(gcdDUMP_FRAME_TGA),1)
LOCAL_SHARED_LIBRARIES += \
	libcutils
endif

ifneq ($(TARGET_2ND_ARCH),)
  LOCAL_MODULE_RELATIVE_PATH := egl
else
  LOCAL_MODULE_PATH          := $(TARGET_OUT_SHARED_LIBRARIES)/egl
endif

LOCAL_MODULE         := libEGL_$(TAG)
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

