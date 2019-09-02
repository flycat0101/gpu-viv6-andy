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
# libGLSLC
#
include $(CLEAR_VARS)

ifndef FIXED_ARCH_TYPE
LOCAL_SRC_FILES := \
	common/gc_glsl_common.c \
	compiler/gc_glsl_built_ins.c \
	compiler/gc_glsl_compiler.c \
	compiler/gc_glsl_emit_code.c \
	compiler/gc_glsl_gen_code.c \
	compiler/gc_glsl_ir.c \
	compiler/gc_glsl_parser.c \
	compiler/gc_glsl_parser_misc.c \
	compiler/gc_glsl_scanner.c \
	compiler/gc_glsl_scanner_misc.c \
	compiler/gc_glsl_ast_walk.c \
	preprocessor/gc_glsl_api.c \
	preprocessor/gc_glsl_base.c \
	preprocessor/gc_glsl_expression.c \
	preprocessor/gc_glsl_hide_set.c \
	preprocessor/gc_glsl_input_stream.c \
	preprocessor/gc_glsl_macro_expand.c \
	preprocessor/gc_glsl_macro_manager.c \
	preprocessor/gc_glsl_preprocessor.c \
	preprocessor/gc_glsl_syntax.c \
	preprocessor/gc_glsl_syntax_util.c \
	preprocessor/gc_glsl_token.c \
	entry/gc_glsl_entry.c

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-Werror

LOCAL_C_INCLUDES := \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/os/linux/user \
	$(AQROOT)/compiler/libVSC/include \
	$(LOCAL_PATH)/inc

LOCAL_LDFLAGS := \
	-Wl,-z,defs

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libGAL \
	libVSC

LOCAL_MODULE         := libGLSLC
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_SHARED_LIBRARY)


else

LOCAL_SRC_FILES := \
    $(FIXED_ARCH_TYPE)/libGLSLC.so

LOCAL_MODULE         := libGLSLC
LOCAL_MODULE_SUFFIX  := .so
LOCAL_MODULE_TAGS    := optional
LOCAL_MODULE_CLASS   := SHARED_LIBRARIES
ifeq ($(PLATFORM_VENDOR),1)
LOCAL_VENDOR_MODULE  := true
endif
include $(BUILD_PREBUILT)

endif

include $(AQROOT)/copy_installed_module.mk

