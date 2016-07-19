LOCAL_PATH := $(call my-dir)/../../
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    tiger.cpp \
    android/Common.c \
    android/VGTiger.cpp \
    android/jni/VGTigerNative.cpp

LOCAL_CFLAGS += \
    -DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/android

LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils \
    libui \
    libEGL_VIVANTE \
    libOpenVG

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 14),1)
  LOCAL_SHARED_LIBRARIES += \
      libgui
else
  LOCAL_SHARED_LIBRARIES += \
      libsurfaceflinger_client
endif

ifdef TARGET_2ND_ARCH
  LOCAL_MODULE_PATH_64 := $(LOCAL_PATH)/androidlibs/$(TARGET_ARCH)
  LOCAL_MODULE_PATH_32 := $(LOCAL_PATH)/android/libs/$(TARGET_2ND_ARCH)
else
  LOCAL_MODULE_PATH := $(LOCAL_PATH)/android/libs/
endif

LOCAL_MULTILIB := both

LOCAL_MODULE := libTiger2DVG
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

