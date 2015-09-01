LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
VIV_TARGET_ABI ?= $(TARGET_ARCH)
ifneq ($(findstring 64,$(VIV_TARGET_ABI)),)
    VIV_MULTILIB=64
else
    VIV_MULTILIB=32
endif
LOCAL_MODULE_TAGS := optional
TIGER_ROOT_PATH := ..

LOCAL_MODULE := libTiger2DVG

LOCAL_CFLAGS := -O1 \
                -DGC500 \
                -DANDROID \

LOCAL_SRC_FILES := \
	VGTigerNative.cpp\
	$(TIGER_ROOT_PATH)/../tiger.cpp \
	$(TIGER_ROOT_PATH)/Common.c \
	$(TIGER_ROOT_PATH)/VGTiger.cpp


LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/$(TIGER_ROOT_PATH)/../  \
	$(LOCAL_PATH)/$(TIGER_ROOT_PATH)/

LOCAL_PRELINK_MODULE :=false

LOCAL_SHARED_LIBRARIES :=	\
	libutils	\
	libcutils   \
	libEGL_VIVANTE	\
	libOpenVG  \
	libui

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 14),1)
LOCAL_SHARED_LIBRARIES += \
               libgui
else
LOCAL_SHARED_LIBRARIES += \
	libsurfaceflinger_client
endif

LOCAL_CFLAGS += -DANDROID_SDK_VERSION=$(PLATFORM_SDK_VERSION)

LOCAL_ARM_MODE := $(VIV_TARGET_ABI)
LOCAL_MODULE_PATH := $(LOCAL_PATH)/../libs/$(VIV_TARGET_ABI)
LOCAL_MULTILIB := $(VIV_MULTILIB)

include $(BUILD_SHARED_LIBRARY)





