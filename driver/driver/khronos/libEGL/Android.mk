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
# libEGL_$(GPU_VENDOR)
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
	api/gc_egl_android.c \
	api/gc_egl_nullws.c

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-Werror \
	-DLOG_TAG=\"v_egl\"

LOCAL_CFLAGS += \
    -DGPU_VENDOR=\"$(GPU_VENDOR)\"

ifneq ($(NUM_FRAMEBUFFER_SURFACE_BUFFERS),)
LOCAL_CFLAGS += \
	-DNUM_FRAMEBUFFER_SURFACE_BUFFERS=$(NUM_FRAMEBUFFER_SURFACE_BUFFERS)
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
	$(AQROOT)/compiler/libVSC/include \

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/source/libEGL.map

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libcutils \
	libhardware \
	libGAL

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 17),1)
LOCAL_SHARED_LIBRARIES += \
	libsync
endif

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

ifeq ($(DRM_GRALLOC),1)
LOCAL_C_INCLUDES += \
	$(AQROOT)/driver/android/gralloc_drm

ifeq ($(LIBDRM_IMX),1)
LOCAL_C_INCLUDES += \
	$(IMX_PATH)/libdrm-imx/vivante \
	$(IMX_PATH)/libdrm-imx/include/drm
else
LOCAL_C_INCLUDES += \
        external/libdrm/vivante \
        external/libdrm/include/drm
endif

LOCAL_SHARED_LIBRARIES += \
	libdrm_vivante

ifeq ($(LIBDRM_IMX),1)
LOCAL_SHARED_LIBRARIES += \
       libdrm_android
else
LOCAL_SHARED_LIBRARIES += \
        libdrm
endif

LOCAL_CFLAGS += \
	-DDRM_GRALLOC
else
LOCAL_C_INCLUDES += \
	$(AQROOT)/driver/android/gralloc
endif

# imx specific
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 25),1)
  ifneq ($(findstring x7.1.1,x$(PLATFORM_VERSION)), x7.1.1)
    LOCAL_C_INCLUDES += $(IMX_PATH)/imx/include
    LOCAL_CFLAGS += -DFSL_YUV_EXT
  endif
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 21),1)
  LOCAL_MODULE_RELATIVE_PATH := egl
else
  LOCAL_MODULE_PATH          := $(TARGET_OUT_SHARED_LIBRARIES)/egl
endif

LOCAL_MODULE         := libEGL_$(GPU_VENDOR)
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
LOCAL_VENDOR_MODULE  := true
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

