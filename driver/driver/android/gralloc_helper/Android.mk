#
# libv_gralloc.a
#
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../Android.mk.def

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    gralloc_helper_drm.c

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DDRM_GRALLOC=1 \
    -DLOG_TAG=\"gralloc-helper\"

# imx specific
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 25),1)
ifneq ($(findstring x7.1.1,x$(PLATFORM_VERSION)), x7.1.1)
    LOCAL_C_INCLUDES += $(IMX_PATH)/imx/include
    LOCAL_CFLAGS += -DFSL_YUV_EXT
endif
endif

LOCAL_C_INCLUDES := \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/hal/inc \
	$(AQROOT)/compiler/libVSC/include \
	$(AQROOT)/driver/android/gralloc_drm

ifeq ($(LIBDRM_IMX),1)
LOCAL_C_INCLUDES += \
       $(IMX_PATH)/libdrm-imx/vivante \
       $(IMX_PATH)/libdrm-imx/include/drm
else
LOCAL_C_INCLUDES += \
        external/libdrm/vivante \
        external/libdrm/include/drm
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 28),1)
LOCAL_C_INCLUDES += \
        hardware/libhardware/include \
        system/core/include
endif

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdrm_vivante \
    libGAL

ifeq ($(LIBDRM_IMX),1)
LOCAL_SHARED_LIBRARIES += \
       libdrm_android
else
LOCAL_SHARED_LIBRARIES += \
        libdrm
endif

LOCAL_C_INCLUDES += \
    $(IMX_PATH)/imx/include

LOCAL_MODULE         := libv_gralloc
LOCAL_VENDOR_MODULE  := true
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)

