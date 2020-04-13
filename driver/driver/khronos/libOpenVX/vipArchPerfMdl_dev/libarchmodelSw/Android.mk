##############################################################################
#
#    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../../../Android.mk.def

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    archSwCommon.cpp \
    archSwPerf.cpp \

LOCAL_CFLAGS := \
    $(CFLAGS) \
	-fPIC	\
	-Wall	\
    -Wno-unused-parameter	\
	-DLOG_TAG=\"ArchModelSw\"	\

LOCAL_C_INCLUDES := \
    $(AQROOT)/compiler/libVSC/include \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/libarchmodelSw/include \
    $(AQROOT)/driver/khronos/libOpenVX/libarchmodelInterface/include \
    $(AQROOT)/driver/khronos/libOpenVX/vipArchPerfMdl_dev/vipArchPerf \

LOCAL_SHARED_LIBRARIES := \
    libNNArchPerf

LOCAL_LDFLAGS := \
	-Wl,-z,defs	\
	-Wl,--version-script=$(LOCAL_PATH)/libarchmodelSw.map

LOCAL_MODULE         := libarchmodelSw
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)
