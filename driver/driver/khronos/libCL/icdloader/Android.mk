##############################################################################
#
#    Copyright (c) 2005 - 2016 by Vivante Corp.  All rights reserved.
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

LOCAL_CFLAGS := \
	$(CFLAGS)

LOCAL_C_INCLUDES := \
	$(AQROOT)/sdk/inc \
	$(LOCAL_PATH)

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/icd_exports.map

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl


LOCAL_MODULE         := libOpenCL
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

