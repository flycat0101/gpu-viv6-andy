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
include $(LOCAL_PATH)/Android.mk.def

ifndef FIXED_ARCH_TYPE

  ifeq ($(VIVANTE_ENABLE_VG), 1)
  endif
endif

LOCAL_PATH := $(AQROOT)

VIVANTE_MAKEFILES := \
    $(LOCAL_PATH)/hal/kernel/Android.mk \
    $(LOCAL_PATH)/driver/android/gralloc/Android.mk

ifeq ($(VIVANTE_ENABLE_2D), 1)
# Build hwcomposer_v1 for Jellybean 4.2 and later
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 17),1)
VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/driver/android/hwcomposer_v1/Android.mk

else
  ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 11),1)
# Build hwcomposer for Honeycomb and later
VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/driver/android/hwcomposer/Android.mk

  endif
endif

ifeq ($(USE_2D_COMPOSITION),1)
VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/driver/android/copybit/Android.mk

endif

endif

ifeq ($(VIVANTE_ENABLE_3D), 1)
endif

ifeq ($(USE_OPENVX),1)
VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/driver/khronos/libOpenVX/reference/Android.mk
endif

ifeq ($(VIVANTE_ENABLE_3D), 1)
VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/driver/khronos/libEGL/Android.mk \
    $(LOCAL_PATH)/driver/khronos/libGLESv11/Android.mk \
    $(LOCAL_PATH)/compiler/libVSC/Android.mk \
    $(LOCAL_PATH)/compiler/libGLSLC/Android.mk

VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/driver/khronos/libGLESv3/Android.mk
else
ifeq ($(VIVANTE_ENABLE_VG), 1)
VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/driver/khronos/libEGL/Android.mk
endif
endif

ifeq ($(VIVANTE_ENABLE_VG), 1)
VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/driver/khronos/libOpenVG/Android.mk
endif

ifeq ($(RECORDER), 1)
VIVANTE_MAKEFILES += $(LOCAL_PATH)/tools/vdb/recorder/Android.mk
endif

ifeq ($(SECURITY),1)
VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/hal/security/Android.mk
endif

# libGAL
VIVANTE_MAKEFILES += \
    $(LOCAL_PATH)/hal/user/Android.mk

include $(VIVANTE_MAKEFILES)

