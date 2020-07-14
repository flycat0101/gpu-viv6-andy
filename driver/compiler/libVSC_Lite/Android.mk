##############################################################################
#
#    Copyright (c) 2005 - 2020 by Vivante Corp.  All rights reserved.
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
# libVSC_Lite
#
include $(CLEAR_VARS)

CFLAGS        += -DVSC_LITE_BUILD=1

ifndef FIXED_ARCH_TYPE
LOCAL_SRC_FILES := \
	../libVSC/old_impl/optimizer/gc_vsc_old_optimizer_util.c \
	../libVSC/old_impl/gc_vsc_old_compiler.c \
	../libVSC/old_impl/gc_vsc_old_preprocess.c \
	../libVSC/utils/array/gc_vsc_utils_array.c \
	../libVSC/utils/base/gc_vsc_utils_base_node.c \
	../libVSC/utils/base/gc_vsc_utils_bit_op.c \
	../libVSC/utils/base/gc_vsc_utils_data_digest.c \
	../libVSC/utils/base/gc_vsc_utils_dump.c \
	../libVSC/utils/base/gc_vsc_utils_err.c \
	../libVSC/utils/base/gc_vsc_utils_math.c \
	../libVSC/utils/bitvector/gc_vsc_utils_bm.c \
	../libVSC/utils/bitvector/gc_vsc_utils_bv.c \
	../libVSC/utils/bitvector/gc_vsc_utils_sv.c \
	../libVSC/utils/graph/gc_vsc_utils_dg.c \
	../libVSC/utils/graph/gc_vsc_utils_udg.c \
	../libVSC/utils/hash/gc_vsc_utils_hash.c \
	../libVSC/utils/io/gc_vsc_utils_io.c \
	../libVSC/utils/list/gc_vsc_utils_bi_list.c \
	../libVSC/utils/list/gc_vsc_utils_uni_list.c \
	../libVSC/utils/mm/gc_vsc_utils_mm.c \
	../libVSC/utils/mm/gc_vsc_utils_mm_arena.c \
	../libVSC/utils/mm/gc_vsc_utils_mm_buddy.c \
	../libVSC/utils/mm/gc_vsc_utils_mm_primary_pool.c \
	../libVSC/utils/string/gc_vsc_utils_string.c \
	../libVSC/utils/table/gc_vsc_utils_block_table.c \
	../libVSC/utils/tree/gc_vsc_utils_tree.c \

LOCAL_GENERATED_SOURCES := \
	$(AQREG)

LOCAL_CFLAGS := \
	$(CFLAGS) \
	-Wno-enum-conversion \
	-Wno-sign-compare \
	-Wno-tautological-constant-out-of-range-compare \
	-Wno-tautological-compare

LOCAL_C_INCLUDES := \
	$(AQROOT)/hal/inc \
	$(AQROOT)/hal/user \
	$(AQROOT)/hal/user/arch \
	$(AQROOT)/hal/user/archvg \
	$(AQROOT)/hal/os/linux/user \
	$(AQARCH)/cmodel/inc \
	$(LOCAL_PATH)/../libVSC/include

LOCAL_LDFLAGS := \
	-Wl,-z,defs \
	-Wl,--version-script=$(LOCAL_PATH)/libVSC_Lite.map

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libdl \
	libGAL

LOCAL_MODULE         := libVSC_Lite
LOCAL_MODULE_TAGS    := optional
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)


else

LOCAL_SRC_FILES := \
    $(FIXED_ARCH_TYPE)/libVSC_Lite.so

LOCAL_MODULE         := libVSC_Lite
LOCAL_MODULE_SUFFIX  := .so
LOCAL_MODULE_TAGS    := optional
LOCAL_MODULE_CLASS   := SHARED_LIBRARIES
include $(BUILD_PREBUILT)

endif

include $(AQROOT)/copy_installed_module.mk

