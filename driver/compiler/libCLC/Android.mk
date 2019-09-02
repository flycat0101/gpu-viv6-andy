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

GC_CLC_DIR  := .

ifdef FIXED_ARCH_TYPE

include $(CLEAR_VARS)
LOCAL_SRC_FILES := \
    $(FIXED_ARCH_TYPE)/libCLC.so

LOCAL_MODULE         := libCLC
LOCAL_MODULE_SUFFIX  := .so
LOCAL_MODULE_TAGS    := optional
LOCAL_MODULE_CLASS   := SHARED_LIBRARIES
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_PREBUILT)

include $(AQROOT)/copy_installed_module.mk

else
#
# libclCommon.a
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    $(GC_CLC_DIR)/common/gc_cl_common.c

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-D_GNU_SOURCE \
	-D__STDC_LIMIT_MACROS \
	-D__STDC_CONSTANT_MACROS

LOCAL_C_INCLUDES := \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/compiler/libVSC/include \
	$(LOCAL_PATH)/$(GC_CLC_DIR)/inc

LOCAL_MODULE         := libclCommon
LOCAL_MODULE_TAGS    := optional
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_STATIC_LIBRARY)

#
# libclCompiler.a
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    $(GC_CLC_DIR)/compiler/gc_cl_scanner.c \
    $(GC_CLC_DIR)/compiler/gc_cl_compiler.c \
    $(GC_CLC_DIR)/compiler/gc_cl_ir.c \
    $(GC_CLC_DIR)/compiler/gc_cl_gen_code.c \
    $(GC_CLC_DIR)/compiler/gc_cl_parser_misc.c \
    $(GC_CLC_DIR)/compiler/gc_cl_emit_code.c \
    $(GC_CLC_DIR)/compiler/gc_cl_parser.c \
    $(GC_CLC_DIR)/compiler/gc_cl_built_ins.c \
    $(GC_CLC_DIR)/compiler/gc_cl_scanner_misc.c \
    $(GC_CLC_DIR)/compiler/gc_cl_tune.c

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-DCL_USE_DEPRECATED_OPENCL_1_0_APIS \
	-DCL_USE_DEPRECATED_OPENCL_1_1_APIS \
	-D_GNU_SOURCE \
	-D__STDC_LIMIT_MACROS \
	-D__STDC_CONSTANT_MACROS \
	-Wno-macro-redefined \
	-Wno-enum-conversion

LOCAL_C_INCLUDES := \
	bionic \
	$(AQROOT)/sdk/inc \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/compiler/libVSC/include \
	$(LOCAL_PATH)/$(GC_CLC_DIR)/inc

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) "<" 23),1)
LOCAL_C_INCLUDES += \
	external/stlport/stlport
endif

LOCAL_MODULE         := libclCompiler
LOCAL_MODULE_TAGS    := optional
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_STATIC_LIBRARY)

#
# libclPreprocessor.a
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    $(GC_CLC_DIR)/preprocessor/gc_cl_input_stream.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_base.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_preprocessor.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_expression.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_macro_expand.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_syntax_util.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_api.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_token.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_syntax.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_hide_set.c \
    $(GC_CLC_DIR)/preprocessor/gc_cl_macro_manager.c

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-D_GNU_SOURCE \
	-D__STDC_LIMIT_MACROS \
	-D__STDC_CONSTANT_MACROS

LOCAL_C_INCLUDES := \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/os/linux/user \
	bionic \
	$(AQROOT)/compiler/libVSC/include \
	$(LOCAL_PATH)/$(GC_CLC_DIR)/inc

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) "<" 23),1)
LOCAL_C_INCLUDES += \
	external/stlport/stlport
endif

LOCAL_MODULE         := libclPreprocessor
LOCAL_MODULE_TAGS    := optional
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_STATIC_LIBRARY)

#
# libCLC
#
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    $(GC_CLC_DIR)/entry/gc_cl_entry.c

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-D_GNU_SOURCE \
	-D__STDC_LIMIT_MACROS \
	-D__STDC_CONSTANT_MACROS

LOCAL_CFLAGS += \
	-fPIC -w

LOCAL_C_INCLUDES := \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/compiler/libVSC/include \
	$(LOCAL_PATH)/$(GC_CLC_DIR)/inc

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/$(GC_CLC_DIR)/entry/libCLC.map

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libVSC \
	libGAL

ifeq ($(shell expr $(PLATFORM_SDK_VERSION) "<" 23),1)
LOCAL_SHARED_LIBRARIES += \
	libstlport
endif

LOCAL_STATIC_LIBRARIES := \
	libclCompiler \
	libclPreprocessor \
	libclCommon

LOCAL_MODULE         := libCLC
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)

include $(AQROOT)/copy_installed_module.mk

endif
