##############################################################################
#
#    Copyright (c) 2005 - 2018 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


LOCAL_PATH := $(call my-dir)

ifdef AQROOT
include $(AQROOT)/Android.mk.def
else
include $(LOCAL_PATH)/../../../../Android.mk.def
endif

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	icd.c \
	icd_dispatch.c \
	icd_linux.c

ifeq ($(ENABLE_CL_GL), 1)
CFLAGS += -DgcdENABLE_CL_GL=1
else
CFLAGS += -DgcdENABLE_CL_GL=0
endif

LOCAL_CFLAGS := \
	$(CFLAGS)

LOCAL_CFLAGS += \
	-DCL_USE_DEPRECATED_OPENCL_1_0_APIS \
	-DCL_USE_DEPRECATED_OPENCL_1_1_APIS

LOCAL_C_INCLUDES := \
	$(AQROOT)/sdk/inc \
	$(LOCAL_PATH)

ifeq ($(ENABLE_CL_GL), 1)
LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/icd_exports_GL.map
else
LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/icd_exports.map
endif

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl

LOCAL_MODULE         := libOpenCL
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

