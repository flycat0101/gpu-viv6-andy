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
PINFO DESCRIPTION="Vivante OpenGLES 3.0"
endef

NAME=libGLESv2_viv

include $(qnx_build_dir)/common.mk

EXTRA_INCVPATH += $(driver_root)/hal/inc
EXTRA_INCVPATH += $(driver_root)/hal/user
EXTRA_INCVPATH += $(driver_root)/hal/os/qnx/user
EXTRA_INCVPATH += $(driver_root)/sdk/inc
EXTRA_INCVPATH += $(driver_root)/compiler/libVSC/include
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libGLESv3/include/glcore
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libGLESv3/include/chip
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libGLESv3/src/glcore
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libEGL/inc
EXTRA_INCVPATH += $(driver_root)/driver/khronos/libEGL/source
EXTRA_INCVPATH += $(driver_root)/hal/user/arch
EXTRA_INCVPATH += $(driver_root)/arch/XAQ2/cmodel/inc

# from libGLESv3 (trunk/driver/khronos/libGLESv3/Makefile.linux)
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_api.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_api_profiler.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_bufobj.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_context.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_dispatch.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_draw.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_egl.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_extensions.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_fbo.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_formats.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_fragop.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_misc.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_names.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_pixelop.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_profiler.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_query.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_raster.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_shader.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_teximage.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_texstate.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/glcore/gc_es_varray.o
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libGLESv3/src/glcore

SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_basic_types.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_buffer.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_clear.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_codec.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_context.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_depth.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_device.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_draw.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_drawable.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_fbo.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_misc.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_patch.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_pixel.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_profiler.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_shader.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_state.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_texture.o
SOURCE_OBJECTS += $(driver_root)/driver/khronos/libGLESv3/src/chip/gc_chip_utils.o
EXTRA_SRCVPATH += $(driver_root)/driver/khronos/libGLESv3/src/chip

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

LIBS += VSC GAL EGL_viv $(STATIC_LIBS)

CCFLAGS += -DGL_GLEXT_PROTOTYPES -fms-extensions

include $(qnx_build_dir)/math.mk

LDFLAGS += -Wl,--version-script=$(driver_root)/driver/khronos/libGLESv3/libGLESv3.map

ifeq ($(filter so dll, $(VARIANT_LIST)),)
INSTALLDIR=/dev/null
endif

include $(MKFILES_ROOT)/qtargets.mk
