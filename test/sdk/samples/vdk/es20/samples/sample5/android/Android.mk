##############################################################################
#
#    Copyright 2012 - 2016 Vivante Corporation, Santa Clara, California.
#    All Rights Reserved.
#
#    Permission is hereby granted, free of charge, to any person obtaining
#    a copy of this software and associated documentation files (the
#    'Software'), to deal in the Software without restriction, including
#    without limitation the rights to use, copy, modify, merge, publish,
#    distribute, sub license, and/or sell copies of the Software, and to
#    permit persons to whom the Software is furnished to do so, subject
#    to the following conditions:
#
#    The above copyright notice and this permission notice (including the
#    next paragraph) shall be included in all copies or substantial
#    portions of the Software.
#
#    THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
#    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
#    IN NO EVENT SHALL VIVANTE AND/OR ITS SUPPLIERS BE LIABLE FOR ANY
#    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
#    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
#    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
##############################################################################


#########################################################################
# OpenGL ES JNI sample
# This makefile builds both an activity and a shared library.
#########################################################################
ifneq ($(TARGET_SIMULATOR),true) # not 64 bit clean


# Build activity

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
VIV_TARGET_ABI ?= $(TARGET_ARCH)
ifneq ($(findstring 64,$(VIV_TARGET_ABI)),)
    VIV_MULTILIB=64
else
    VIV_MULTILIB=32
endif


LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := GL2Sample5

LOCAL_JNI_SHARED_LIBRARIES := libgl2sample5_jni

LOCAL_MULTILIB := $(VIV_MULTILIB)
LOCAL_MODULE_PATH := $(AQROOT)/bin/$(VIV_TARGET_ABI)
include $(BUILD_PACKAGE)

#########################################################################
# Build JNI Shared Library
# #########################################################################

LOCAL_PATH:= $(LOCAL_PATH)/jni

include $(CLEAR_VARS)
VIV_TARGET_ABI ?= $(TARGET_ARCH)
ifneq ($(findstring 64,$(VIV_TARGET_ABI)),)
    VIV_MULTILIB=64
else
    VIV_MULTILIB=32
endif


# Optional tag would mean it doesn't get installed by default
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := -Werror -Wno-unused-parameter -Wno-strict-aliasing -DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_SRC_FILES:= 	 \
					sample5.cpp \
					../../sphere.c \
					../../main.c \
					../../../Common/vdk_sample_common.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../.. \
					$(LOCAL_PATH)/../../../Common \
#LOCAL_SRC_FILES:=  gl_code.cpp

LOCAL_SHARED_LIBRARIES := \
	libutils \
	libEGL \
	libGLESv2

#LOCAL_STATIC_LIBRAIES := libvdk_sample_common libsphere
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 18),1)
LOCAL_SHARED_LIBRARIES += \
        liblog
endif

LOCAL_MODULE := libgl2sample5_jni

LOCAL_PRELINK_MODULE := false


LOCAL_ARM_MODE := $(VIV_TARGET_ABI)
LOCAL_MODULE_PATH := $(LOCAL_PATH)/../libs/$(VIV_TARGET_ABI)
LOCAL_MULTILIB := $(VIV_MULTILIB)
include $(BUILD_SHARED_LIBRARY)

endif # TARGET_SIMULATOR
