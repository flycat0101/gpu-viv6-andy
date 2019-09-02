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
include $(LOCAL_PATH)/../../../Android.mk.def


#
# libEGL_$(TAG)
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	src/chip/gc_halti5_chip.c \
	src/chip/gc_halti2_chip.c \
	src/chip/gc_halti3_chip.c \
	src/chip/gc_halti5_cmdbuf.c \
	src/chip/gc_halti5_pipeline.c \
	src/chip/gc_halti5_resource.c \
	src/chip/gc_halti5_computeblit.c \
	src/chip/gc_halti5_tweak.c \
	src/gc_vk_api.c \
	src/gc_vk_chip.c \
	src/gc_vk_cmdbuf.c \
	src/gc_vk_context.c \
	src/gc_vk_devqueue.c \
	src/gc_vk_descriptor.c \
	src/gc_vk_dispatch.c \
	src/gc_vk_framebuffer.c \
	src/gc_vk_icd.c \
	src/gc_vk_instance.c \
	src/gc_vk_noop.c \
	src/gc_vk_object.c \
	src/gc_vk_phydevice.c \
	src/gc_vk_pipeline.c \
	src/gc_vk_query.c \
	src/gc_vk_resource.c \
	src/gc_vk_shader.c \
	src/gc_vk_sync.c \
	src/gc_vk_utils.c \
	src/gc_vk_validation.c \
	src/wsi/gc_wsi_surface.c \
	src/wsi/gc_wsi_swapchain.c \
	src/wsi/gc_wsi_display.c \
	src/wsi/gc_wsi_surface_android.c

LOCAL_SRC_FILES += \
	src/wsi/gc_wsi_display_fbdev.c

LOCAL_GENERATED_SOURCES := \
    $(AQREG)

ifndef FIXED_ARCH_TYPE

VULKAN_DIR := $(LOCAL_PATH)





endif

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-Werror \
	-DLOG_TAG=\"vulkan\"

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/compiler/libVSC/include \
	$(AQROOT)/compiler/libSPIRV/spvconverter

LOCAL_C_INCLUDES += \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/user/arch

LOCAL_C_INCLUDES += \
	$(AQARCH)/cmodel/inc

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 26),1)
LOCAL_C_INCLUDES += \
	frameworks/native/libs/nativewindow/include \
	frameworks/native/libs/nativebase/include \
	frameworks/native/libs/arect/include
endif

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/libVulkan.map

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libhardware \
	libsync

LOCAL_SHARED_LIBRARIES += \
	libSPIRV_viv \
	libVSC \
	libGAL \
	libGLSLC

ifeq ($(DRM_GRALLOC),1)
LOCAL_C_INCLUDES += \
	$(AQROOT)/driver/android/gralloc_drm

LOCAL_CFLAGS += \
	-DDRM_GRALLOC
else
LOCAL_C_INCLUDES += \
	$(AQROOT)/driver/android/gralloc
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 28),1)
LOCAL_C_INCLUDES += \
	frameworks/native/include
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 24),1)
  LOCAL_MODULE         := libvulkan_$(GPU_VENDOR)
else
  LOCAL_MODULE         := libvulkan
endif

LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

