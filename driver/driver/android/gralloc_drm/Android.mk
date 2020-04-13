##############################################################################
#
#    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


#
# gralloc.<variant>.so
#
# This is a Google drm_gralloc (gralloc.drm.so) adaptor to private_handle_t
#

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

USE_DRM_ADAPTOR := 0

ifeq ($(USE_DRM_ADAPTOR),1)
LOCAL_SRC_FILES := \
    gralloc_drm_adaptor.cpp

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DDRM_GRALLOC=1 \
    -DLOG_TAG=\"gralloc-adp\"

ifeq ($(LIBDRM_IMX),1)
LOCAL_C_INCLUDES := \
    $(IMX_PATH)/libdrm-imx/vivante \
    $(IMX_PATH)/libdrm-imx/include/drm \
    external/drm_gralloc

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdrm_android \
    libdrm_vivante \
    libgralloc_drm
else
LOCAL_C_INCLUDES := \
    external/libdrm/vivante \
    external/libdrm/include/drm \
    external/drm_gralloc

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdrm \
    libdrm_vivante \
    libgralloc_drm
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 28),1)
LOCAL_C_INCLUDES += \
        hardware/libhardware/include \
        system/core/include
endif

else
# Vivante drm based gralloc
LOCAL_SRC_FILES := \
    gralloc.cpp \
    gralloc_vivante.cpp

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DDRM_GRALLOC=1 \
    -DLOG_TAG=\"gralloc-viv\"

ifeq ($(LIBDRM_IMX),1)
LOCAL_C_INCLUDES := \
    $(IMX_PATH)/libdrm-imx/vivante \
    $(IMX_PATH)/libdrm-imx/include/drm \

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdrm_android \
    libdrm_vivante

else

LOCAL_C_INCLUDES := \
    external/libdrm/vivante \
    external/libdrm/include/drm

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdrm \
    libdrm_vivante
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 28),1)
LOCAL_C_INCLUDES += \
        hardware/libhardware/include \
        system/core/include
endif

endif

LOCAL_C_INCLUDES += \
    $(IMX_PATH)/imx/include

LOCAL_MODULE_RELATIVE_PATH := hw

LOCAL_MODULE         := gralloc_viv.$(HAL_MODULE_VARIANT)
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 27),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)

