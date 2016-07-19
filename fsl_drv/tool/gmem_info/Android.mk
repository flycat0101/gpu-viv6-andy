ifeq ($(TARGET_BOARD_PLATFORM),imx6)

LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../driver/Android.mk.def

# Share library
include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
	gmem_info.c

LOCAL_CFLAGS += -DBUILD_FOR_ANDROID -DIMX6Q -DLINUX

LOCAL_SHARED_LIBRARIES := libutils libc liblog libGAL

LOCAL_C_INCLUDES := $(LOCAL_PATH)\
	$(AQROOT)/sdk/inc \
	$(AQROOT)/hal/inc

LOCAL_MODULE := gmem_info
LOCAL_LD_FLAGS += -nostartfiles
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS := eng
LOCAL_VENDOR_MODULE  := true
include $(BUILD_EXECUTABLE)

endif
