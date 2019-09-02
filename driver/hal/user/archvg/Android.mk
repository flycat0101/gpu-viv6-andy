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

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    gc_hal_user_hardware_context_vg.c \
    gc_hal_user_hardware_vg.c \
    gc_hal_user_hardware_vg_software.c

LOCAL_GENERATED_SOURCES := \
    $(VG_AQREG)

LOCAL_CFLAGS := \
    $(CFLAGS) \
	-Wno-unused-function \
	-Werror

LOCAL_C_INCLUDES := \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQVGARCH)/cmodel/inc \
    $(AQROOT)/compiler/libVSC/include

LOCAL_MODULE         := libhalarchuser_vg
LOCAL_MODULE_TAGS    := optional
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_STATIC_LIBRARY)

