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

include $(qnx_build_dir)/platform_config/$(PLATFORM)/platform_config

define PINFO
PINFO DESCRIPTION="Vivante GPU module"
endef

NAME=libGalcore

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/kernel/inc
EXTRA_INCVPATH += $(driver_root)/hal/kernel
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/kernel/arch
EXTRA_INCVPATH += $(driver_root)/hal/kernel/archvg
EXTRA_INCVPATH += $(driver_root)/build_qnx/platform_config/$(PLATFORM)

# from galcore (trunk/hal/os/qnx/kernel/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/hal/os/qnx/kernel/gc_hal_kernel_driver.o
SOURCE_OBJECTS += $(driver_root)/hal/os/qnx/kernel/gc_hal_kernel_os.o
SOURCE_OBJECTS += $(driver_root)/hal/os/qnx/kernel/gc_hal_kernel_qnx.o
SOURCE_OBJECTS += $(driver_root)/hal/os/qnx/kernel/gc_hal_kernel_device.o
SOURCE_OBJECTS += $(driver_root)/hal/os/qnx/kernel/gc_hal_kernel_math.o
SOURCE_OBJECTS += $(driver_root)/hal/os/qnx/kernel/gc_hal_kernel_debugfs.o
EXTRA_SRCVPATH += $(driver_root)/hal/os/qnx/kernel

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

ifeq ($(filter g, $(VARIANT_LIST)), g)
STATIC_LIBS += halkernel_gS halarchkernel_gS
ifeq ($(VIVANTE_ENABLE_VG),1)
STATIC_LIBS += halarchkernel_vg_gS
endif
else
STATIC_LIBS += halkernelS halarchkernelS
ifeq ($(VIVANTE_ENABLE_VG),1)
STATIC_LIBS += halarchkernel_vgS
endif
endif

ifeq ($(USE_FAST_MEM_COPY), 1)
STATIC_LIBS += fastmemcpyS
endif
STATIC_LIBS += khronosS

$(foreach lib, $(STATIC_LIBS), $(eval LIBPREF_$(lib) = -Bstatic))
$(foreach lib, $(STATIC_LIBS), $(eval LIBPOST_$(lib) = -Bdynamic))

LIBS += $(STATIC_LIBS)

include $(qnx_build_dir)/math.mk

LDFLAGS += -Wl,--version-script=$(driver_root)/hal/os/qnx/kernel/libGalcore.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
