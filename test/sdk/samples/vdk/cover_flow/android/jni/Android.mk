##############################################################################
#
#    Copyright 2012 - 2017 Vivante Corporation, Santa Clara, California.
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


LOCAL_PATH:= $(call my-dir)/../..
include $(CLEAR_VARS)
LOCAL_CFLAGS := \
			-DANDROID \
			-DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION) \
			-DLOG_TAG=\"cover_flow\"

LOCAL_SRC_FILES:= \
	android/jni/cover_flow_android.c \
	android/jni/pkm.c \
	android/jni/tga.c

LOCAL_C_INCLUDES += \
	$(JNI_H_INCLUDE)

LOCAL_SHARED_LIBRARIES := \
    libEGL \
    libGLESv1_CM \
    liblog

LOCAL_MODULE:= libcover_flow
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE:= false
LOCAL_MULTILIB := both
include $(BUILD_SHARED_LIBRARY)

