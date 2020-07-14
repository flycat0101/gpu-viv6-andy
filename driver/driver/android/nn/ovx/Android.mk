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


LOCAL_PATH:= $(call my-dir)
include $(LOCAL_PATH)/../../../../Android.mk.def
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../sdk/inc \
                    frameworks/ml/nn/common/include/ \
                    frameworks/ml/nn/runtime/include/    \

LOCAL_SRC_FILES:= \
    OvxDevice.cpp   \
    OvxDriver.cpp   \
    OvxExecutor.cpp

LOCAL_SHARED_LIBRARIES := \
    libbase \
    libdl   \
    libhardware \
    libhidlbase \
    libhidlmemory   \
    libhidltransport    \
    liblog  \
    libutils    \
    android.hardware.neuralnetworks@1.0 \
    android.hidl.allocator@1.0  \
    android.hidl.memory@1.0 \
    libneuralnetworks   \
    libOpenVX
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 28),1)

LOCAL_SHARED_LIBRARIES += android.hardware.neuralnetworks@1.1
LOCAL_STATIC_LIBRARIES += libneuralnetworks_common

LOCAL_CFLAGS += -Wno-error=unused-variable -Wno-error=unused-function -Wno-error=unused-parameter -Wno-error=return-type

LOCAL_CFLAGS += -DMULTI_CONTEXT

LOCAL_MODULE      := android.hardware.neuralnetworks@1.1-service-ovx-driver

else

LOCAL_MODULE      := android.hardware.neuralnetworks@1.0-service-ovx-driver

endif

ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif

LOCAL_CFLAGS += -DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_MODULE_TAGS := optional

include $(BUILD_EXECUTABLE)
