LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../../driver/Android.mk.def

COMMITNR :=$(shell awk 'NR==1' $(LOCAL_PATH)/../../../.git/FETCH_HEAD | cut -c -7)

# Share library
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	g2d.c

LOCAL_CFLAGS += -DLINUX -Dgco2D_SetStateU32_Fix=0 -DgcdENABLE_VG=1 -Dg2dBUILD_FOR_VIVANTE=0
LOCAL_CFLAGS += -DGIT_STRING=$(COMMITNR)
LOCAL_CFLAGS += -Wno-date-time

LOCAL_SHARED_LIBRARIES := libutils libc liblog libGAL libEGL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include\
	$(AQROOT)/driver/android/gralloc \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/hal/inc

#CAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_MODULE := libg2d

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_TAGS    := optional
LOCAL_VENDOR_MODULE  := true

include $(BUILD_SHARED_LIBRARY)
