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
include $(LOCAL_PATH)/../../../Android.mk.def

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	gc_cl.c \
	gc_cl_command.c \
	gc_cl_context.c \
	gc_cl_device.c \
	gc_cl_enqueue.c \
	gc_cl_event.c \
	gc_cl_extension.c \
	gc_cl_gl.c \
	gc_cl_icd.c \
	gc_cl_kernel.c \
	gc_cl_mem.c \
	gc_cl_platform.c \
	gc_cl_program.c \
	gc_cl_profiler.c \
	gc_cl_sampler.c

LOCAL_CFLAGS := \
	$(CFLAGS)

LOCAL_C_INCLUDES := \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/compiler/libVSC/include \
    $(AQROOT)/hal/user \
	$(LOCAL_PATH)

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/libOpenCL12.map

CFLAGS         += -DCL_USE_DEPRECATED_OPENCL_1_0_APIS -DCL_USE_DEPRECATED_OPENCL_1_1_APIS -DBUILD_OPENCL_12=1
LOCAL_CFLAGS += \
         $(CFLAGS)

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libVSC \
	libGLESv2 \
	libGAL

ifeq ($(BUILD_OPENCL_ICD),1)
LOCAL_CFLAGS         += -DBUILD_OPENCL_ICD=1
LOCAL_MODULE         := libVivanteOpenCL
else
LOCAL_MODULE         := libOpenCL
endif

ifeq ($(BUILD_OPENCL_FP),1)
LOCAL_CFLAGS         += -DBUILD_OPENCL_FP=1
endif

LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

