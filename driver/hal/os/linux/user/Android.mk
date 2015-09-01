##############################################################################
#
#    Copyright (c) 2005 - 2015 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../../Android.mk.def

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    gc_hal_user_debug.c \
    gc_hal_user_os.c \
    gc_hal_user_math.c

LOCAL_CFLAGS := \
    $(CFLAGS) \
	-Werror

LOCAL_C_INCLUDES := \
    $(AQROOT)/hal/inc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user

ifeq ($(VIVANTE_ENABLE_3D),1)
LOCAL_C_INCLUDES += \
    $(AQROOT)/compiler/libVSC/include
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 20),1)
LOCAL_C_INCLUDES += \
	system/core/libsync/include
endif

LOCAL_MODULE         := libhalosuser
LOCAL_MODULE_TAGS    := optional
include $(BUILD_STATIC_LIBRARY)

