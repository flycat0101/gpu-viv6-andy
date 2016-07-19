LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../driver/Android.mk.def

include $(CLEAR_VARS)

LOCAL_SRC_FILES := gralloc_helper.cpp hwc_helper.cpp

LOCAL_CFLAGS += $(CFLAGS)
LOCAL_CFLAGS += -DLINUX -DgcdENABLE_VG=1
LOCAL_CFLAGS += -DLOG_TAG=\"helper\"

LOCAL_SHARED_LIBRARIES := libutils libc liblog libbinder libGAL libEGL

LOCAL_STATIC_LIBRARIES := libv_gralloc

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include\
    $(AQROOT)/driver/android/gralloc \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/compiler/libVSC/include \
    $(AQROOT)/sdk/inc \
    $(AQROOT)/hal/inc \
    $(AQROOT)/../fsl_drv/g2d/include

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 26),1)
LOCAL_C_INCLUDES += frameworks/native/libs/nativewindow/include
LOCAL_C_INCLUDES += frameworks/native/libs/nativebase/include
LOCAL_C_INCLUDES += frameworks/native/libs/arect/include
endif

LOCAL_MODULE := libgpuhelper
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS    := optional
LOCAL_VENDOR_MODULE  := true

include $(BUILD_SHARED_LIBRARY)
