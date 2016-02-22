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


LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
VIV_TARGET_ABI ?= $(TARGET_ARCH)
ifneq ($(findstring 64,$(VIV_TARGET_ABI)),)
    VIV_MULTILIB=64
else
    VIV_MULTILIB=32
endif


COVERFLOW_TOP := ../..

LOCAL_MODULE:= libcover_flow
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := \
			-DANDROID \
			-DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION) \
			-DLOG_TAG=\"cover_flow\"

LOCAL_SRC_FILES:= \
	cover_flow_android.c \
	pkm.c \
	tga.c

LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE)

LOCAL_LDFLAGS += -lGLESv1_CM -ldl -llog

LOCAL_PRELINK_MODULE:= false
LOCAL_ARM_MODE := $(VIV_TARGET_ABI)
LOCAL_MODULE_PATH := $(LOCAL_PATH)/../libs/$(VIV_TARGET_ABI)
LOCAL_MULTILIB := $(VIV_MULTILIB)
include $(BUILD_SHARED_LIBRARY)

