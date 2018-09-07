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


#
# libgralloc_helper.so
#
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../Android.mk.def

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    gralloc_helper_drm.c

LOCAL_CFLAGS := \
    -DDRM_GRALLOC=1 \
    -DLOG_TAG=\"gralloc-helper\"

LOCAL_C_INCLUDES := \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/hal/inc \
	$(AQROOT)/compiler/libVSC/include \
	$(AQROOT)/driver/android/gralloc_drm \
        $(IMX_PATH)/libdrm-imx/vivante \
        $(IMX_PATH)/libdrm-imx/include/drm

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdrm_android \
    libdrm_vivante \
    libGAL

LOCAL_C_INCLUDES += \
    $(IMX_PATH)/imx/include

LOCAL_MODULE         := libv_gralloc
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)

