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
PINFO DESCRIPTION="initializers"
endef

NAME=initializers

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc

SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/main.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_scalar_array.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_scalar_array_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_scalar.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_scalar_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_struct_multiple.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_struct_multiple_vector16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_struct_multiple_vector2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_struct_multiple_vector3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_struct_multiple_vector4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_struct_multiple_vector8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_union_multiple.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_union_multiple_vector16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_union_multiple_vector2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_union_multiple_vector3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_union_multiple_vector4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_union_multiple_vector8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector16_array.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector16_array_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector16.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector16_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector2_array.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector2_array_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector2.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector2_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector3_array.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector3_array_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector3.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector3_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector4_array.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector4_array_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector4.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector4_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector8_array.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector8_array_union.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector8.o
SOURCE_OBJECTS += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers/initializers_vector8_union.o
EXTRA_SRCVPATH += $(CL11_TEST_DIR)/UnitTest/test_vivante/initializers

EXTRA_LIBVPATH += $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LDOPTS += -lLLVM_viv -lCLC -lOpenCL
CCOPTS += -Wno-narrowing

include $(qnx_build_dir)/math.mk

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) samples/cl11/UnitTest/test_vivante)

include $(MKFILES_ROOT)/qtargets.mk
