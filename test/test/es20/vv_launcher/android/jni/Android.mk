##############################################################################
#
#    Copyright 2012 - 2020 Vivante Corporation, Santa Clara, California.
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


LOCAL_PATH := $(call my-dir)/../..
include $(CLEAR_VARS)

LOCAL_CFLAGS := \
   -fno-strict-aliasing \
   -DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION) \
   -DLOG_TAG=\"vv_launcher\"

LOCAL_C_INCLUDES := \
   $(LOCAL_PATH)/lib \
   $(LOCAL_PATH)/src

LOCAL_SRC_FILES := \
   lib/math/icosa.c \
   lib/math/Mat.c \
   lib/math/misc.c \
   lib/math/Vec.c \
   \
   lib/glutils/check.c \
   lib/glutils/dds.c \
   lib/glutils/log.c \
   lib/glutils/program.c \
   lib/glutils/Texture2D.c \
   lib/glutils/Texture2DRenderSurface.c \
   lib/glutils/TextureCube.c \
   \
   lib/rc/resource.c \
   \
   src/Chiclet.c \
   src/ChicletProg.c \
   src/ContentProg.c \
   src/EnvProg.c \
   src/Geodesic.c \
   src/LauncherApp.c \
   src/resources.c \
   \
   android/jni/vv_main_android.c

LOCAL_SHARED_LIBRARIES := \
   libEGL \
   libGLESv2 \
   liblog

LOCAL_MODULE         := libvv_launcher
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MULTILIB       := both
include $(BUILD_SHARED_LIBRARY)

