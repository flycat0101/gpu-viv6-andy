##############################################################################
#
#    Copyright (c) 2005 - 2019 by Vivante Corp.  All rights reserved.
#
#    The material in this file is confidential and contains trade secrets
#    of Vivante Corporation. This is proprietary information owned by
#    Vivante Corporation. No part of this work may be disclosed,
#    reproduced, copied, transmitted, or used in any way for any purpose,
#    without the express written permission of Vivante Corporation.
#
##############################################################################


LOCAL_PATH := $(call my-dir)
include $(LOCAL_PATH)/../../Android.mk.def

#
# libSPIRV_viv
#
include $(CLEAR_VARS)

ifndef FIXED_ARCH_TYPE
LOCAL_SRC_FILES := \
	spvconverter/gc_spirv_mempool.c \
	spvconverter/gc_spirv_to_vir.c \
	spvconverter/gc_spirv_spec_constant_op.c \
	spvconverter/gc_spriv_disassmble.c

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-Wno-tautological-constant-out-of-range-compare \
	-Wno-tautological-compare \
	-Wno-sign-compare \
	-Wno-switch \
	-Wno-incompatible-pointer-types \
	-Wno-bitfield-constant-conversion

LOCAL_C_INCLUDES := \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/user/arch \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/compiler/libVSC/include \
	$(AQARCH)/cmodel/inc \
	$(LOCAL_PATH)/spvconverter

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/spvconverter/spvconverter.map

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libGAL \
	libVSC

LOCAL_MODULE         := libSPIRV_viv
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)


else

LOCAL_SRC_FILES := \
    $(FIXED_ARCH_TYPE)/libSPIRV_viv.so

LOCAL_MODULE         := libSPIRV_viv
LOCAL_MODULE_SUFFIX  := .so
LOCAL_MODULE_TAGS    := optional
LOCAL_MODULE_CLASS   := SHARED_LIBRARIES
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_PREBUILT)

endif

include $(AQROOT)/copy_installed_module.mk

