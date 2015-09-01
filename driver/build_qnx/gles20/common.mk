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
PINFO DESCRIPTION="Vivante OpenGLES 2.0"
endef

NAME=libGLESv2_viv

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libEGL/inc

# from libGLESv2 (trunk/driver/khronos/libGLESv2/driver/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_buffer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_clear.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_compiler.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_debug.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_database.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_draw.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_egl.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_flush.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_framebuffer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_object.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_patch.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_pixels.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_profiler.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_query.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_renderbuffer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_shader.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_state.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_texture.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh_vertex.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv2/driver/gc_glsh.o
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libGLESv2/driver

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

STATIC_LIBS += khronosS
$(foreach lib, $(STATIC_LIBS), $(eval LIBPREF_$(lib) = -Bstatic))
$(foreach lib, $(STATIC_LIBS), $(eval LIBPOST_$(lib) = -Bdynamic))

LIBS += VSC GAL-$(HARDWARENAME) EGL_viv $(STATIC_LIBS)

ifneq ($(filter v7, $(VARIANT_LIST)), v7)
	CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
	LIBS += m-vfp
else
	LIBS += m
endif

LDFLAGS += -Wl,--version-script=$(driver_root)/driver/khronos/libGLESv2/driver/libGLESv2.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
