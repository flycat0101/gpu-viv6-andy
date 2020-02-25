#
# Copyright 2010, QNX Software Systems Ltd.  All Rights Reserved
#
# This source code has been published by QNX Software Systems Ltd.
# (QSSL).  However, any use, reproduction, modification, distribution
# or transfer of this software, or any software which includes or is
# based upon any of this code, is only permitted under the terms of
# the QNX Open Community License version 1.0 (see licensing.qnx.com for
# details) or as otherwise expressly authorized by a written license
# agreement from QSSL.  For more information, please email licensing@qnx.com.
#

# find the driver's root directory, which is 3 levels below the current make file
driver_root:=$(abspath ../../$(dir $(lastword $(MAKEFILE_LIST))))
qnx_build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/..)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="ArchModel Utilities"
endef

NAME=libNNArchPerf


include $(qnx_build_dir)/common.mk

# Core
SOURCE_OBJECTS += $(driver_root)/arch/vipArchPerfMdl_dev/vipArchPerf/nnArchPerf.o
EXTRA_SRCVPATH += $(driver_root)/arch/vipArchPerfMdl_dev/vipArchPerf

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

CCFLAGS += -Wno-error=switch -Wno-error=missing-braces -Wno-unused-function -Wno-unused-variable -Wno-sign-compare -Wno-narrowing
CCFLAGS += -Wno-unused-local-typedefs
CCFLAGS += -D_LITTLE_ENDIAN_ \
           -D__linux__ \
           -fPIC \

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

include $(qnx_build_dir)/math.mk

PRE_TARGET:= prebuild

prebuild:
	@cp -f $(driver_root)/tools/bin/gc_feature_database.h $(driver_root)/arch/vipArchPerfMdl_dev/vipArchPerf

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
