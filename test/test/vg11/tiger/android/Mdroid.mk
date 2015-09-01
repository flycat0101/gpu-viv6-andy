LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
VIV_TARGET_ABI ?= $(TARGET_ARCH)
ifneq ($(findstring 64,$(VIV_TARGET_ABI)),)
    VIV_MULTILIB=64
else
    VIV_MULTILIB=32
endif

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JNI_SHARED_LIBRARIES := libTiger2DVG

LOCAL_PACKAGE_NAME := tiger2DVG
LOCAL_MULTILIB := $(VIV_MULTILIB)
LOCAL_MODULE_PATH := $(AQROOT)/bin/$(VIV_TARGET_ABI)/vg11

include $(BUILD_PACKAGE)
