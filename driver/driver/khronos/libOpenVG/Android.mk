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
# libOpenVG
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
           gc_vg_arc.c \
           gc_vg_context.c \
           gc_vg_debug.c \
           gc_vg_filter.c \
           gc_vg_format.c \
           gc_vg_image.c \
           gc_vg_main.c \
           gc_vg_mask.c \
           gc_vg_matrix.c \
           gc_vg_memory_manager.c \
           gc_vg_object.c \
           gc_vg_paint.c \
           gc_vg_path.c \
           gc_vg_path_append.c \
           gc_vg_path_coordinate.c \
           gc_vg_path_import.c \
           gc_vg_path_interpolate.c \
           gc_vg_path_modify.c \
           gc_vg_path_normalize.c \
           gc_vg_path_point_along.c \
           gc_vg_path_storage.c \
           gc_vg_path_transform.c \
           gc_vg_path_walker.c \
           gc_vg_state.c \
           gc_vg_stroke.c \
           gc_vg_text.c \
           gc_vgu_library.c
LOCAL_CFLAGS := \
    $(CFLAGS) \
	-Werror

LOCAL_C_INCLUDES += \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/driver/khronos/libEGL/inc

LOCAL_LDFLAGS := \
    -Wl,-z,defs \
    -Wl,--version-script=$(LOCAL_PATH)/libOpenVG.map

LOCAL_SHARED_LIBRARIES := \
    liblog \
    libdl \
    libGAL

LOCAL_MODULE        := libOpenVG
LOCAL_MODULE_TAGS   := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

