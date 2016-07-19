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

qnx_build_dir:=$(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../../../..)

ifndef QCONFIG
QCONFIG=qconfig.mk
endif
include $(QCONFIG)

define PINFO
PINFO DESCRIPTION="es11 tutorials"
endef

NAME=tutorial6

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc

SOURCE_OBJECTS += $(VDK_TEST_DIR)/es11/tutorial6/main.o
SOURCE_OBJECTS += $(VDK_TEST_DIR)/es11/tutorial6/font.o
SOURCE_OBJECTS += $(VDK_TEST_DIR)/es11/tutorial6/render.o
SOURCE_OBJECTS += $(VDK_TEST_DIR)/es11/tutorial6/mesh.o
SOURCE_OBJECTS += $(VDK_TEST_DIR)/es11/tutorial6/particles.o
SOURCE_OBJECTS += $(VDK_TEST_DIR)/es11/tutorial6/texture.o
SOURCE_OBJECTS += $(VDK_TEST_DIR)/es11/tutorial6/timer.o

EXTRA_SRCVPATH += $(VDK_TEST_DIR)/es11/tutorial6

EXTRA_LIBVPATH += $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LIBS += GAL-$(HARDWARENAME) VDK EGL GLESv1_CM

ifneq ($(filter v7, $(VARIANT_LIST)), v7)
	CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
	LIBS += m-vfp
else
	LIBS += m
endif

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) samples/es11/tutorial)

include $(MKFILES_ROOT)/qtargets.mk
