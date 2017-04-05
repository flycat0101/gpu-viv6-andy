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
PINFO DESCRIPTION="Vivante EGL"
endef

NAME=EGL_viv

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libEGL/os
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libEGL/source
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libEGL/api
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libEGL/inc

# from libeglapi (trunk/driver/khronos/libEGL/api/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/api/gc_egl_qnx.o

ifeq ($(CUSTOM_PIXMAP), 1)
    CCFLAGS += -DCUSTOM_PIXMAP
endif

# from libEGL_viv (trunk/driver/khronos/libEGL/source/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/source/gc_egl_config.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/source/gc_egl_context.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/source/gc_egl_image.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/source/gc_egl_init.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/source/gc_egl_query.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/source/gc_egl_surface.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/source/gc_egl_swap.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/source/gc_egl_sync.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/source/gc_egl.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libEGL/api/gc_egl_nullws.o

EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libEGL/os
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libEGL/api
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libEGL/source

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LIBS += screen GAL

include $(qnx_build_dir)/math.mk

LDFLAGS += -Wl,--version-script=$(driver_root)/driver/khronos/libEGL/source/libEGL.map

ifeq ($(filter dll so, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
else
POST_INSTALL=$(RM_HOST) $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)/libEGL_vivS.a
endif

include $(MKFILES_ROOT)/qtargets.mk
