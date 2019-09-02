#
# libv_gralloc.so
#
LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../../Android.mk.def

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    gralloc_helper_drm.c

LOCAL_CFLAGS := \
    $(CFLAGS) \
    -DDRM_GRALLOC=1 \
    -DLOG_TAG=\"gralloc-helper\"

ifeq ($(LIBDRM_IMX),1)
LOCAL_C_INCLUDES := \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/hal/inc \
    $(AQROOT)/compiler/libVSC/include \
    $(AQROOT)/driver/android/gralloc_drm \
    $(IMX_PATH)/libdrm-imx/vivante \
    $(IMX_PATH)/libdrm-imx/include/drm

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdrm_android \
    libdrm_vivante \
    libGAL

else
LOCAL_C_INCLUDES := \
    $(AQROOT)/hal/user \
    $(AQROOT)/hal/os/linux/user \
    $(AQROOT)/hal/inc \
    $(AQROOT)/compiler/libVSC/include \
    $(AQROOT)/driver/android/gralloc_drm \
    external/libdrm/vivante \
    external/libdrm/include/drm

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libdrm \
    libdrm_vivante \
    libGAL
endif

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 28),1)
LOCAL_C_INCLUDES += \
        hardware/libhardware/include \
        system/core/include
endif

# imx specific
ifeq ($(shell expr $(PLATFORM_SDK_VERSION) ">=" 25),1)
ifneq ($(findstring x7.1.1,x$(PLATFORM_VERSION)), x7.1.1)
    LOCAL_C_INCLUDES += $(IMX_PATH)/imx/include
    LOCAL_CFLAGS += -DFSL_YUV_EXT
endif
endif

LOCAL_C_INCLUDES += \
    $(IMX_PATH)/imx/include

LOCAL_MODULE         := libv_gralloc
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_STATIC_LIBRARY)

