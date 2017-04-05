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

qnx_build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../../../..)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="unsupported_extensions"
endef

NAME=unsupported_extensions

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc

SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/main.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/test_extension_support_cles_khr_int64.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/test_extension_support_d3d10_sharing.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/test_extension_support_fp16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/test_extension_support_fp16_func.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/test_extension_support_fp64.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/test_extension_support_fp64_func.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/test_extension_support_gl_event.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/test_extension_support_gl_sharing.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions/test_extension_support_write_image3d.o
EXTRA_SRCVPATH += $(CL11_TEST_DIR)/UnitTest/test_vivante/unsupported_extensions

EXTRA_LIBVPATH += $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LDOPTS += -lGAL -lVSC -lLLVM_viv -lCLC -lOpenCL -lLLVM_viv

include $(qnx_build_dir)/math.mk

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) samples/cl11/UnitTest/test_vivante)

include $(MKFILES_ROOT)/qtargets.mk
