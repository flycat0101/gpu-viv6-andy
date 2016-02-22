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


LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../Android.mk.def


#
# VDK library backend for Android.
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	gc_vdk_egl.c \
	gc_vdk_gl.c \
	gc_vdk_android.cpp

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-DLOG_TAG=\"vdk\" \
	-D__VDK_EXPORT

LOCAL_C_INCLUDES += \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/hal/inc

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/libVDK.map \
	-DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_SHARED_LIBRARIES := \
	libdl \
	libui \
	libutils \
	libcutils \
	libbinder \
	libEGL

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 14),1)
  LOCAL_SHARED_LIBRARIES += \
	libgui

else
  LOCAL_SHARED_LIBRARIES += \
	libsurfaceflinger_client

endif

LOCAL_MODULE         := libVDK
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

