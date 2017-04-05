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
# libGLESv2_$(GPU_VENDOR)
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/glcore/gc_es_api.c \
    src/glcore/gc_es_api_profiler.c \
    src/glcore/gc_es_bufobj.c \
    src/glcore/gc_es_context.c \
    src/glcore/gc_es_dispatch.c \
    src/glcore/gc_es_draw.c \
    src/glcore/gc_es_egl.c \
    src/glcore/gc_es_extensions.c \
    src/glcore/gc_es_fbo.c \
    src/glcore/gc_es_formats.c \
    src/glcore/gc_es_fragop.c \
    src/glcore/gc_es_misc.c \
    src/glcore/gc_es_names.c \
    src/glcore/gc_es_pixelop.c \
    src/glcore/gc_es_query.c \
    src/glcore/gc_es_raster.c \
    src/glcore/gc_es_shader.c \
    src/glcore/gc_es_teximage.c \
    src/glcore/gc_es_texstate.c \
    src/glcore/gc_es_varray.c \
    src/glcore/gc_es_profiler.c \
    src/chip/gc_chip_basic_types.c \
    src/chip/gc_chip_buffer.c \
    src/chip/gc_chip_clear.c \
    src/chip/gc_chip_codec.c \
    src/chip/gc_chip_context.c \
    src/chip/gc_chip_depth.c \
    src/chip/gc_chip_device.c \
    src/chip/gc_chip_draw.c \
    src/chip/gc_chip_drawable.c \
    src/chip/gc_chip_fbo.c \
    src/chip/gc_chip_misc.c \
    src/chip/gc_chip_patch.c \
    src/chip/gc_chip_pixel.c \
    src/chip/gc_chip_profiler.c \
    src/chip/gc_chip_shader.c \
    src/chip/gc_chip_state.c \
    src/chip/gc_chip_texture.c \
    src/chip/gc_chip_utils.c

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-Werror \
	-DGL_GLEXT_PROTOTYPES \
	-fms-extensions \
	-D_LINUX_ \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include/glcore \
	$(LOCAL_PATH)/src/glcore \
	$(LOCAL_PATH)/include/chip \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/user \
	$(AQROOT)/compiler/libVSC/include \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/driver/android/gralloc \
	$(AQROOT)/driver/khronos/libEGL/inc \
	$(AQROOT)/driver/khronos/libEGL/source

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/libGLESv3.map

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libVSC \
	libGAL

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 21),1)
  LOCAL_MODULE_RELATIVE_PATH := egl
else
  LOCAL_MODULE_PATH          := $(TARGET_OUT_SHARED_LIBRARIES)/egl
endif

LOCAL_MODULE         := libGLESv2_$(GPU_VENDOR)
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

