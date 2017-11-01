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
include $(LOCAL_PATH)/Android.mk.def

ifndef FIXED_ARCH_TYPE

  ifeq ($(VIVANTE_ENABLE_VG),1)
  endif
endif

LOCAL_PATH := $(AQROOT)

VIVANTE_MAKEFILES := $(LOCAL_PATH)/hal/kernel/Android.mk \
                     $(LOCAL_PATH)/hal/user/Android.mk

ifneq ($(DRM_GRALLOC),1)
  VIVANTE_MAKEFILES += $(LOCAL_PATH)/driver/android/gralloc/Android.mk
endif

ifeq ($(VIVANTE_ENABLE_2D),1)
  # Build hwcomposer for Honeycomb and later
  ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 11),1)
    HWC_MAKEFILE += $(LOCAL_PATH)/driver/android/hwcomposer/Android.mk
  endif

  # Build hwcomposer_v1 for Jellybean 4.2 and later
  ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 17),1)
    HWC_MAKEFILE := $(LOCAL_PATH)/driver/android/hwcomposer_v1/Android.mk
  endif

  # Build hwcomposer_v2 Androi-N and later
  ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 24),1)
    # HWC_MAKEFILE := $(LOCAL_PATH)/driver/android/hwcomposer_v2/Android.mk
  endif

  VIVANTE_MAKEFILES += $(HWC_MAKEFILE)
endif

ifeq ($(VIVANTE_ENABLE_3D)_$(USE_OPENCL),1_1)
  VIVANTE_MAKEFILES += $(LOCAL_PATH)/compiler/libCLC/Android.mk \
                       $(LOCAL_PATH)/driver/khronos/libCL/Android.mk

  ifeq ($(BUILD_OPENCL_ICD),1)
    VIVANTE_MAKEFILES += $(LOCAL_PATH)/driver/khronos/libCL/icdloader12/Android.mk
  endif
endif

ifeq ($(USE_OPENVX),1)
  VIVANTE_MAKEFILES += $(LOCAL_PATH)/driver/khronos/libOpenVX/Android.mk
endif

ifeq ($(VIVANTE_ENABLE_3D),1)
  EGL_MAKEFILE := $(LOCAL_PATH)/driver/khronos/libEGL/Android.mk

  VIVANTE_MAKEFILES += $(LOCAL_PATH)/driver/khronos/libGLESv11/Android.mk \
                       $(LOCAL_PATH)/driver/khronos/libGLESv3/Android.mk \
                       $(LOCAL_PATH)/compiler/libVSC/Android.mk \
                       $(LOCAL_PATH)/compiler/libGLSLC/Android.mk
endif

ifeq ($(VIVANTE_ENABLE_VG),1)
  EGL_MAKEFILE := $(LOCAL_PATH)/driver/khronos/libEGL/Android.mk

  VIVANTE_MAKEFILES += $(LOCAL_PATH)/driver/khronos/libOpenVG/Android.mk
endif

VIVANTE_MAKEFILES += $(EGL_MAKEFILE)

ifeq ($(VIVANTE_ENABLE_3D)_$(USE_VULKAN),1_1)
  VIVANTE_MAKEFILES += $(LOCAL_PATH)/compiler/libSPIRV/Android.mk \
                       $(LOCAL_PATH)/driver/khronos/libVulkan10/Android.mk

  # Build hwvulkan for android-n and later
  ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 24),1)
      VIVANTE_MAKEFILES += $(LOCAL_PATH)/driver/android/hwvulkan/Android.mk
  endif
endif

ifeq ($(RECORDER),1)
  VIVANTE_MAKEFILES += $(LOCAL_PATH)/tools/vdb/recorder/Android.mk
endif

ifeq ($(SECURITY),1)
  VIVANTE_MAKEFILES += $(LOCAL_PATH)/hal/security/Android.mk
endif

include $(VIVANTE_MAKEFILES)

