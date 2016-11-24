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

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libEGL/inc

# from libGLESv11 (trunk/driver/khronos/libGLESv11/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_alpha.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_basic_types.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_buffer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_clear.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_clip_plane.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_context.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_cull.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_depth.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_draw.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_enable.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_fixed_func.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_fog.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_frag_proc.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_fragment_shader.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_framebuffer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_hash.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_lighting.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_line.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_matrix.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_multisample.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_named_object.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_pixel.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_point.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_profiler.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_query.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_renderbuffer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_states.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_stream.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_texture.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_vertex_shader.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff_viewport.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv11/gc_glff.o
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libGLESv11

EXTRA_LIBVPATH += $(LOCAL_INSTALL)

OBJECTS_FROM_SRCVPATH := $(basename $(wildcard $(foreach dir, $(EXTRA_SRCVPATH), $(addprefix $(dir)/*., s S c cc cpp))))
MISSING_OBJECTS := $(filter-out $(OBJECTS_FROM_SRCVPATH), $(basename $(SOURCE_OBJECTS)))
ifneq ($(MISSING_OBJECTS), )
$(error ***** Missing source objects:  $(MISSING_OBJECTS))
endif

EXCLUDE_OBJS += $(addsuffix .o, $(notdir $(filter-out $(basename $(SOURCE_OBJECTS)), $(OBJECTS_FROM_SRCVPATH))))
#$(warning ***** Excluded objects: $(EXCLUDE_OBJS))

include $(MKFILES_ROOT)/qmacros.mk

ifeq ($(findstring lite, $(VARIANT_LIST)), lite)
EXTRA_SILENT_VARIANTS+=lite
define PINFO
PINFO DESCRIPTION="Vivante OpenGL ES Common-Lite profile"
endef
CCFLAGS += -DCOMMON_LITE
NAME=libGLES_CL_viv
else
define PINFO
PINFO DESCRIPTION="Vivante OpenGL ES Common profile"
endef
NAME=libGLES_CM_viv
endif

CCFLAGS += -D__GL_EXPORTS

LIBS += VSC GAL EGL_viv

ifneq ($(filter v7, $(VARIANT_LIST)), v7)
	CCFLAGS += -mfpu=vfp -mfloat-abi=softfp
	LIBS += m-vfp
else
	LIBS += m
endif

LDFLAGS += -Wl,--version-script=$(driver_root)/driver/khronos/libGLESv11/libGLESv11.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
