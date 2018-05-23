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
PINFO DESCRIPTION="Vivante OpenVG"
endef

include $(qnx_build_dir)/common.mk
ifeq ($(VIVANTE_ENABLE_3D), 1)
NAME=libOpenVG_viv
EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libEGL/inc

# from libOpenVG (trunk/driver/khronos/libOpenVG_3D/vg11/driver/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_context.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_dump.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_egl.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_font.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_hardware.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_image.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_mask_layer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_math.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_matrix.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_object.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_paint.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_path.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_path.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_profiler.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_scanline_tessellator.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_scanline.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_shader.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_tessellator.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/gc_vgsh_vgu.o
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver

#EXTRA_LIBVPATH += $(INSTALL_ROOT_$(OS))/$(CPU)$(filter le be, $(VARIANT_LIST))$(CPUVARDIR_SUFFIX)/$(INSTALLDIR)
EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

LIBS += VSC GAL EGL_viv
CCFLAGS += -O1


include $(qnx_build_dir)/math.mk

LDFLAGS += -Wl,--version-script=$(driver_root)/driver/khronos/libOpenVG_3D/vg11/driver/libOpenVG.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

POST_BUILD =$(CP_HOST) $@ $(LOCAL_INSTALL)/$(@F).3d && $(CP_HOST) $@ $(LOCAL_INSTALL)/$(@F)

include $(MKFILES_ROOT)/qtargets.mk
endif
