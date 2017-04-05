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
PINFO DESCRIPTION="Vivante VDK"
endef

NAME=VDK

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/sdk/inc

# from libVDK (trunk/sdk/vdk/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/sdk/vdk/gc_vdk_egl.o
SOURCE_OBJECTS += $(driver_root)/sdk/vdk/gc_vdk_gl.o
SOURCE_OBJECTS += $(driver_root)/sdk/vdk/gc_vdk_qnx.o

EXTRA_SRCVPATH += $(driver_root)/sdk/vdk

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

CCFLAGS += -D__VDK_EXPORT

STATIC_LIBS += khronosS
$(foreach lib, $(STATIC_LIBS), $(eval LIBPREF_$(lib) = -Bstatic))
$(foreach lib, $(STATIC_LIBS), $(eval LIBPOST_$(lib) = -Bdynamic))

LIBS += screen $(STATIC_LIBS) GAL EGL_viv

include $(qnx_build_dir)/math.mk

LDFLAGS += -Wl,--version-script=$(driver_root)/sdk/vdk/libVDK.map

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib/graphics/$(PLATFORM))

include $(MKFILES_ROOT)/qtargets.mk
