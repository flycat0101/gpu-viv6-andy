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
PINFO DESCRIPTION="OpenVX nngpu libraries"
endef

NAME=libnngpu


include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/driver/include
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/kernels
EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/linux/user
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/libkernel/libnngpu
ifeq ($(USE_VXC_BINARY),1)
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libOpenVX/libkernel/libnngpu/$(GPU_CONFIG)
endif

# from libCL (trunk/driver/khronos/libCL/makefile.linux)
# Core
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVX/libkernel/libnngpu/nngpu_binary_interface.o

EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libOpenVX/libkernel/libnngpu

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

#-Wno-error=switch -Wno-error=missing-braces

CCFLAGS += -D_LITTLE_ENDIAN_ \
           -DOPENVX_USE_LIST \
           -DOPENVX_USE_NODE_MEMORY \
           -DOPENVX_USE_TILING \
           -DOPENVX_USE_DOT \
           -DOPENVX_USE_TARGET \
           -D__linux__ \
           -fPIC

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LIBS += OpenVX CLC VSC GAL

include $(qnx_build_dir)/math.mk

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk

