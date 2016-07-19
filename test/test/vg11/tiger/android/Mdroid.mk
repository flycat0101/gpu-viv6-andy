LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JNI_SHARED_LIBRARIES := libTiger2DVG

LOCAL_MULTILIB     := both
LOCAL_PACKAGE_NAME := tiger2DVG
LOCAL_MODULE_PATH  := $(LOCAL_PATH)/../bin/
include $(BUILD_PACKAGE)

