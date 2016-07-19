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
PINFO DESCRIPTION="Vivante HAL arch-vg user"
endef

include $(qnx_build_dir)/common.mk

ifeq ($(VIVANTE_ENABLE_VG), 1)
EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/arch/GC350/cmodel/inc

ifeq ($(VIVANTE_ENABLE_3D), 1)
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
endif
ifeq ($(VIVANTE_ENABLE_2D), 1)
EXTRA_INCVPATH += $(driver_root)/hal/user/arch/thirdparty_special
endif
ifeq ($(VIVANTE_ENABLE_VG), 1)
EXTRA_INCVPATH += $(driver_root)/hal/user/archvg
endif

SOURCE_OBJECTS += $(driver_root)/hal/user/archvg/gc_hal_user_hardware_context_vg.o
SOURCE_OBJECTS += $(driver_root)/hal/user/archvg/gc_hal_user_hardware_vg.o
SOURCE_OBJECTS += $(driver_root)/hal/user/archvg/gc_hal_user_hardware_vg_software.o

EXTRA_SRCVPATH += $(driver_root)/hal/user/archvg

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

ifneq ($(filter v7, $(VARIANT_LIST)), v7)
	CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
	LIBS += m-vfp
else
	LIBS += m
endif

ifneq ($(filter so, $(VARIANT_LIST)), so)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
endif
