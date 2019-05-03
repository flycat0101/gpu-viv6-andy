##############################################################################
#
#    Copyright 2012 - 2019 Vivante Corporation, Santa Clara, California.
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

# Build activity

LOCAL_PATH:= $(call my-dir)
ifdef TARGET_2ND_ARCH

  # build for 64
  include $(CLEAR_VARS)

  LOCAL_SRC_FILES := $(call all-subdir-java-files)

  LOCAL_JNI_SHARED_LIBRARIES := libgl11tutorial5_jni
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 23),1)
  LOCAL_JNI_SHARED_LIBRARIES += libc++
endif
  LOCAL_PACKAGE_NAME := GL11Tutorial5_android-64
  LOCAL_MODULE_PATH  := $(LOCAL_PATH)/bin/
  LOCAL_MULTILIB     := 64
  include $(BUILD_PACKAGE)

  # build for 32
  include $(CLEAR_VARS)

  LOCAL_SRC_FILES := $(call all-subdir-java-files)

  LOCAL_JNI_SHARED_LIBRARIES := libgl11tutorial5_jni
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 23),1)
  LOCAL_JNI_SHARED_LIBRARIES += libc++
endif
  LOCAL_PACKAGE_NAME := GL11Tutorial5_android-32
  LOCAL_MODULE_PATH  := $(LOCAL_PATH)/bin/
  LOCAL_MULTILIB     := 32
  include $(BUILD_PACKAGE)

  # build for multilib
  include $(CLEAR_VARS)

  LOCAL_SRC_FILES := $(call all-subdir-java-files)

  LOCAL_JNI_SHARED_LIBRARIES := libgl11tutorial5_jni
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 23),1)
  LOCAL_JNI_SHARED_LIBRARIES += libc++
endif
  LOCAL_PACKAGE_NAME := GL11Tutorial5_android-multilib
  LOCAL_MODULE_PATH  := $(LOCAL_PATH)/bin/
  LOCAL_MULTILIB     := both
  include $(BUILD_PACKAGE)

else

include $(CLEAR_VARS)

  LOCAL_SRC_FILES := $(call all-subdir-java-files)
  LOCAL_JNI_SHARED_LIBRARIES := libgl11tutorial5_jni
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 23),1)
  LOCAL_JNI_SHARED_LIBRARIES += libc++
endif
  LOCAL_PACKAGE_NAME := GL11Tutorial5_android
  LOCAL_MODULE_PATH  := $(LOCAL_PATH)/bin/
  include $(BUILD_PACKAGE)
endif
#########################################################################
# Build JNI Shared Library
#########################################################################

LOCAL_PATH:= $(LOCAL_PATH)/..

include $(CLEAR_VARS)

LOCAL_CFLAGS := -Werror -Wno-unused-parameter -DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION) -DANDROID_JNI

LOCAL_SRC_FILES:= \
	android/jni/tutorial5.cpp	\
	android/jni/tga.cpp	\
	render.cpp	\
	font.cpp	\

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)

LOCAL_SHARED_LIBRARIES := \
	libEGL \
	libGLESv1_CM

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 18),1)
LOCAL_SHARED_LIBRARIES += \
	liblog
endif

LOCAL_MODULE := libgl11tutorial5_jni
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MULTILIB := both
include $(BUILD_SHARED_LIBRARY)
