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


VV_TOP := ../..

LOCAL_MODULE:= libvv_launcher
LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS := \
				-DLINUX \
				-fno-strict-aliasing \
				-I $(LOCAL_PATH)/$(VV_TOP)/lib \
				-I $(LOCAL_PATH)/$(VV_TOP)/src \
				-DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION) \
				-DLOG_TAG=\"vv_launcher\"

LOCAL_SRC_FILES:= \
		$(VV_TOP)/lib/math/icosa.c \
		$(VV_TOP)/lib/math/Mat.c \
		$(VV_TOP)/lib/math/misc.c \
		$(VV_TOP)/lib/math/Vec.c \
		\
		$(VV_TOP)/lib/glutils/check.c \
		$(VV_TOP)/lib/glutils/dds.c \
		$(VV_TOP)/lib/glutils/log.c \
		$(VV_TOP)/lib/glutils/program.c \
		$(VV_TOP)/lib/glutils/Texture2D.c \
		$(VV_TOP)/lib/glutils/Texture2DRenderSurface.c \
		$(VV_TOP)/lib/glutils/TextureCube.c \
		\
		$(VV_TOP)/lib/rc/resource.c \
		\
		$(VV_TOP)/src/Chiclet.c \
		$(VV_TOP)/src/ChicletProg.c \
		$(VV_TOP)/src/ContentProg.c \
		$(VV_TOP)/src/EnvProg.c \
		$(VV_TOP)/src/Geodesic.c \
		$(VV_TOP)/src/LauncherApp.c \
		$(VV_TOP)/src/resources.c \
		\
		vv_main_android.c

LOCAL_LDFLAGS += -lEGL -lGLESv2 -ldl -llog

LOCAL_PRELINK_MODULE:= false
LOCAL_ARM_MODE := $(VIV_TARGET_ABI)
LOCAL_MODULE_PATH := $(LOCAL_PATH)/../libs/$(VIV_TARGET_ABI)
LOCAL_MULTILIB := $(VIV_MULTILIB)
include $(BUILD_SHARED_LIBRARY)

