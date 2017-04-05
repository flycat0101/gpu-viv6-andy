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
PINFO DESCRIPTION="Vivante HAL kernel"
endef

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/kernel
EXTRA_INCVPATH += $(driver_root)/hal/kernel/inc
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/inc
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/kernel
EXTRA_INCVPATH += $(driver_root)/hal/kernel/arch

ifeq ($(VIVANTE_ENABLE_VG), 1)
EXTRA_INCVPATH += $(driver_root)/hal/kernel/archvg
endif

# from libhalkernel (trunk/hal/kernel/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_async_command.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_command.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_db.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_debug.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_event.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_heap.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_mmu.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_video_memory.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_power.o

EXTRA_SRCVPATH += $(driver_root)/hal/kernel

ifeq ($(VIVANTE_ENABLE_VG), 1)

EXTRA_INCVPATH += $(driver_root)/hal/kernel/archvg

SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_command_vg.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_interrupt_vg.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_mmu_vg.o
SOURCE_OBJECTS += $(driver_root)/hal/kernel/gc_hal_kernel_vg.o

endif

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

ifeq ($(USE_FAST_MEM_COPY), 1)
LIBS += fastmemcpyS
endif

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

include $(qnx_build_dir)/math.mk

ifneq ($(filter so, $(VARIANT_LIST)), so)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
