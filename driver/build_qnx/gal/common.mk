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
PINFO DESCRIPTION="Vivante GAL"
endef

NAME=GAL

include $(qnx_build_dir)/common.mk

#PUBLIC_INCVPATH := $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user

# from libGAL (trunk/hal/user/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_brush_cache.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_brush.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_buffer.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_dump.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_heap.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_query.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_queue.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_raster.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_rect.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_surface.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user.o

ifeq ($(VIVANTE_ENABLE_VG), 1)
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_vg.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_buffer_vg.o
endif

ifeq ($(VIVANTE_ENABLE_3D), 1)
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_bufobj.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_engine.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_index.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_vertex_array.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_vertex.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_shader.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_format.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_texture.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_mem.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_statistics.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_resource.o

ifeq ($(USE_OPENCL), 1)
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_cl.o
endif
else
ifeq ($(VIVANTE_ENABLE_VG), 1)
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_mem.o
endif
endif

EXTRA_SRCVPATH += $(driver_root)/hal/os/qnx/user

SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_profiler.o
SOURCE_OBJECTS += $(driver_root)/hal/user/gc_hal_user_bitmask.o

EXTRA_SRCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

STATIC_LIBS += halarchuserS halosuserS khronosS
ifeq ($(VIVANTE_ENABLE_VG), 1)
STATIC_LIBS += halarchuser_vgS
endif
$(foreach lib, $(STATIC_LIBS), $(eval LIBPREF_$(lib) = -Bstatic))
$(foreach lib, $(STATIC_LIBS), $(eval LIBPOST_$(lib) = -Bdynamic))

LIBS += $(STATIC_LIBS)
LIBS += socket
ifeq ($(USE_FAST_MEM_COPY), 1)
LIBS += fastmemcpyS
endif
LIBS += screen

ifneq ($(filter v7, $(VARIANT_LIST)), v7)
	CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
	LIBS += m-vfp
else
	LIBS += m
endif

ifeq ($(filter dll so, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
else

INSTALLDIR=$(firstword $(INSTALLDIR_$(OS)) usr/lib/graphics/$(PLATFORM))
POST_INSTALL=$(RM_HOST) $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)/libGALS.a

endif

include $(MKFILES_ROOT)/qtargets.mk
