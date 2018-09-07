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
PINFO DESCRIPTION="Vivante OpenCL"
endef

NAME=libOpenCL

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user

# from libCL (trunk/driver/khronos/libCL/makefile.linux)
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_log.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_command.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_context.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_device.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_enqueue.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_event.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_extension.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_icd.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_kernel.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_mem.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_platform.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_program.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_profiler.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_sampler.o
ifeq ($(ENABLE_CL_GL), 1)
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libCL/gc_cl_gl.o
endif

EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libCL

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LIBS += GAL GLESv2 EGL VSC $(STATIC_LIBS)

CCFLAGS += -DCL_USE_DEPRECATED_OPENCL_1_0_APIS
CCFLAGS += -DCL_USE_DEPRECATED_OPENCL_1_1_APIS

include $(qnx_build_dir)/math.mk

LDFLAGS += -Wl,--version-script=$(driver_root)/driver/khronos/libCL/libOpenCL12.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
